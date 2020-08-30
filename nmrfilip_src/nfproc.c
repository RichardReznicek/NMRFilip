/* 
 * NMRFilip LIB - the NMR data processing software - core library
 * Copyright (C) 2010, 2011, 2020 Richard Reznicek
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#include "fftw3.h"

#include "nmrfilip.h"

#include "nfio.h"
#include "nfproc.h"
#include "nfexport.h"


typedef struct {
	intptr_t Offset;
	size_t Length;	/** in 2x long (Re, Im) (8 B) **/
	size_t DataLength;
	size_t Count;
	intptr_t Start;
	size_t FalseNegative;
	size_t FalsePositive;
} SignalPattern;

/** Single ChunkSet is common for all steps; used as array of offsets with respect to step beginning **/
int GetChunkSet(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t MaxLength = 0;
	unsigned char AddPattern = 0;
	size_t i = 0, j = 0, k = 0;
	intptr_t m = 0;
	size_t AuxChunkCount = 0;
	int32_t AuxLong = 0;
	int32_t *OrPad = NULL;
	SignalWindow *AuxPointer = NULL;

	SignalPattern Pattern = {0, 0, 0, 1, 0, 0, 0};
	size_t BufferLength = 0;
	SignalPattern* Patterns = NULL;
	size_t ValidPatterns = 0;
	SignalPattern* AuxPointer2 = NULL;
	intptr_t Start = 0;
	intptr_t End = 0;

	size_t MinLength = SIZE_MAX;
	size_t MaxCount = 0;
	size_t MaxCount2 = 0;
	size_t MinFalseNegative = SIZE_MAX;
	size_t MinFalsePositive = SIZE_MAX;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if ((NMRDataStruct->Steps == NULL) || (NMRDataStruct->StepCount == 0)) {
		free(NMRDataStruct->ChunkSet);
		NMRDataStruct->ChunkSet = NULL;
		NMRDataStruct->ChunkCount = 0;
		return DATA_OK;
	}
	
	/** Finding MaxLength **/
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) 
		if ((TDDIndexRange(NMRDataStruct, i) > MaxLength) && (!(StepFlag(NMRDataStruct, i) & STEP_IGNORE)))
			MaxLength = TDDIndexRange(NMRDataStruct, i);

	if (MaxLength == 0) {
		free(NMRDataStruct->ChunkSet);
		NMRDataStruct->ChunkSet = NULL;
		NMRDataStruct->ChunkCount = 0;
		return DATA_OK;
	}
		

	OrPad = (int32_t *) malloc(MaxLength*sizeof(int32_t));

	if (OrPad == NULL) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating auxiliary memory space during the creation of chunk set");
		return (MEM_ALLOC_ERROR | DATA_OLD);
	}
		
	/** zero-ing OrPad **/
	for (i = 0; i < MaxLength; i++) 
		OrPad[i] = 0;
	
	/** OR-ing all steps into OrPad **/
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
		if (!(StepFlag(NMRDataStruct, i) & STEP_IGNORE))
			/* for (j = 0; j < TDDIndexRange(NMRDataStruct, i); j++) { */
			for (j = NMRDataStruct->SkipPoints; j < TDDIndexRange(NMRDataStruct, i); j++) {	/** skip the digital filter artifacts **/
				OrPad[j] |= TDDReal(NMRDataStruct, i, j);
				OrPad[j] |= TDDImag(NMRDataStruct, i, j);
			}
	}
	
	/** Counting non-zero chunks in OrPad **/
	for (j = 0; j < MaxLength; j++) {
		/** "rising edge" detection **/
		if ((AuxLong == 0) && (OrPad[j] !=0))
			AuxChunkCount++;
		AuxLong = OrPad[j];
	}
	
	if ((AuxChunkCount != NMRDataStruct->ChunkCount) || (NMRDataStruct->ChunkSet == NULL)) {
		AuxPointer = NMRDataStruct->ChunkSet;
		NMRDataStruct->ChunkSet = (SignalWindow *) realloc(NMRDataStruct->ChunkSet, AuxChunkCount*sizeof(SignalWindow));
		
		if (NMRDataStruct->ChunkSet == NULL) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating chunk set memory space");
			free(AuxPointer);
			AuxPointer = NULL;
			NMRDataStruct->ChunkCount = 0;
			free(OrPad);
			OrPad = NULL;
			return (MEM_ALLOC_ERROR | DATA_EMPTY);
		}
		
		NMRDataStruct->ChunkCount = AuxChunkCount;
	}
	
	/** Creating chunk set **/
	i = 0;
	AuxLong = 0;
	for (j = 0; j < MaxLength; j++) {
		/** "rising edge" detection **/
		if ((AuxLong == 0) && (OrPad[j] !=0)) {
			ChunkDataStart(NMRDataStruct, i) = 2*((intptr_t) j);
			ChunkIndexRange(NMRDataStruct, i) = 1;
			i++;
		} else
		if ((AuxLong != 0) && (OrPad[j] !=0)) {
			ChunkIndexRange(NMRDataStruct, i - 1)++;
		} 
		
		AuxLong = OrPad[j];
	}
	

	/** Construct a set of concievable patterns **/
	MinLength = SIZE_MAX;
	MaxCount = 2;
	for (i = 0; i < NMRDataStruct->ChunkCount; i++) {	/** extra extent **/

		/** Any pattern found from now on will be longer than the shortest pattern from the last cycle **/
		/** Try to avoid the need to allocate more memory: check the patterns of Length <= the shortest pattern from the last cycle and keep only the ones with the highest Count **/
		if (i == (NMRDataStruct->ChunkCount - 1))		/** no more patterns are going to be found in the last iteration, so keep only the patterns with the highest Count for further processing **/
			MinLength = SIZE_MAX;
		
		if (((BufferLength - ValidPatterns) < (2*(NMRDataStruct->ChunkCount - i - 1))) || (i == (NMRDataStruct->ChunkCount - 1))) {
			for (MaxCount2 = 0, k = 0; k < ValidPatterns; k++) 
				if ((Patterns[k].Length <= MinLength) && (Patterns[k].Count > MaxCount2))
					MaxCount2 = Patterns[k].Count;

			for (j = 0, k = 0; k < ValidPatterns; k++) 
				if ((Patterns[k].Length > MinLength) || (Patterns[k].Count == MaxCount2)) 	/** keep the pattern **/
					Patterns[j++] = Patterns[k];
			ValidPatterns = j;
		}
		
		/** (re)allocate a buffer if necessary **/
		if (((BufferLength - ValidPatterns) < (2*(NMRDataStruct->ChunkCount - i - 1))) || (Patterns == NULL)) {
			if (Patterns == NULL)
				BufferLength = 4 * NMRDataStruct->ChunkCount;	/** rough initial guess **/
			else
				BufferLength *= 2;
			
			AuxPointer2 = Patterns;
			Patterns = (SignalPattern *) realloc(Patterns, BufferLength*sizeof(SignalPattern));
			
			if (Patterns == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating chunk pattern array memory space");
				free(AuxPointer2);
				AuxPointer2 = NULL;
				free(OrPad);
				OrPad = NULL;
				return (MEM_ALLOC_ERROR | DATA_INVALID);
			}
			
			/** Add all chunks together as the first entry **/
			if (AuxPointer2 == NULL) {
				Patterns[ValidPatterns].Offset = ChunkDataStart(NMRDataStruct, 0)/2;
				Patterns[ValidPatterns].Length = ChunkDataStart(NMRDataStruct, NMRDataStruct->ChunkCount - 1)/2 + ChunkIndexRange(NMRDataStruct, NMRDataStruct->ChunkCount - 1) - ChunkDataStart(NMRDataStruct, 0)/2;
				Patterns[ValidPatterns].DataLength = ChunkDataStart(NMRDataStruct, NMRDataStruct->ChunkCount - 1)/2 + ChunkIndexRange(NMRDataStruct, NMRDataStruct->ChunkCount - 1) - ChunkDataStart(NMRDataStruct, 0)/2;
				Patterns[ValidPatterns].Count = 2;
				Patterns[ValidPatterns].Start = 0;
				Patterns[ValidPatterns].FalseNegative = 0;
				Patterns[ValidPatterns].FalsePositive = 0;
				ValidPatterns++;
			}
		}
		
		MinLength = SIZE_MAX;
		
		for (j = 0; j < (NMRDataStruct->ChunkCount - i - 1); j++) {	/** chunk **/
			/** starts **/
			Start = ChunkDataStart(NMRDataStruct, j)/2;
			Pattern.Length = ChunkDataStart(NMRDataStruct, j + i + 1)/2 - ChunkDataStart(NMRDataStruct, j)/2;
			Pattern.DataLength = ChunkDataStart(NMRDataStruct, j + i)/2 + ChunkIndexRange(NMRDataStruct, j + i) - ChunkDataStart(NMRDataStruct, j)/2;
			
			Pattern.Offset = Start%Pattern.Length;
			
			if (Pattern.Length < MinLength)
				MinLength = Pattern.Length;
			
			for (k = 0, AddPattern = 1; (k < ValidPatterns) && AddPattern; k++) {	/** backward search for a better efficiency **/
				
				if ((Patterns[ValidPatterns - 1 - k].Offset == Pattern.Offset) && 
					(Patterns[ValidPatterns - 1 - k].Length == Pattern.Length) && 
					(Patterns[ValidPatterns - 1 - k].DataLength == Pattern.DataLength)) {
						
					/** Count the repeated pattern occurrence instead of producing duplicities **/
					AddPattern = 0;
					Patterns[ValidPatterns - 1 - k].Count++;
					/** keep track of maximal Count so far **/
					if (Patterns[ValidPatterns - 1 - k].Count > MaxCount)
						MaxCount = Patterns[ValidPatterns - 1 - k].Count;
				}
			}
			
			if (AddPattern) {
				Patterns[ValidPatterns] = Pattern;
				ValidPatterns++;
			}
		}
		
		for (j = 0; j < (NMRDataStruct->ChunkCount - i - 1); j++) {	/** chunk **/
			/** ends **/
			Pattern.Length = (ChunkDataStart(NMRDataStruct, j + i + 1)/2 + ChunkIndexRange(NMRDataStruct, j + i + 1)) - (ChunkDataStart(NMRDataStruct, j)/2 + ChunkIndexRange(NMRDataStruct, j));
			Pattern.DataLength = ChunkDataStart(NMRDataStruct, j + i + 1)/2 + ChunkIndexRange(NMRDataStruct, j + i + 1) - ChunkDataStart(NMRDataStruct, j + 1)/2;
			Start = ChunkDataStart(NMRDataStruct, j + 1)/2;	/** = End - DataLength **/
			
			Pattern.Offset = Start%Pattern.Length;
			
			if (Pattern.Length < MinLength)
				MinLength = Pattern.Length;

			for (k = 0, AddPattern = 1; (k < ValidPatterns) && AddPattern; k++) {	/** backward search for a better efficiency **/
				
				if ((Patterns[ValidPatterns - 1 - k].Offset == Pattern.Offset) && 
					(Patterns[ValidPatterns - 1 - k].Length == Pattern.Length) && 
					(Patterns[ValidPatterns - 1 - k].DataLength == Pattern.DataLength)) {
						
					/** Count the repeated pattern occurrence instead of producing duplicities **/
					AddPattern = 0;
					Patterns[ValidPatterns - 1 - k].Count++;
					/** keep track of maximal Count so far **/
					if (Patterns[ValidPatterns - 1 - k].Count > MaxCount)
						MaxCount = Patterns[ValidPatterns - 1 - k].Count;
				}
			}
			
			if (AddPattern) {
				Patterns[ValidPatterns] = Pattern;
				ValidPatterns++;
			}
		}
		
		/** Any pattern found in further cycles will be longer than the shortest pattern from this cycle **/
		/** If the rough upper estimate of the corresponding Count based on the fraction of the first&last chunk start (& end) difference and the Length is lower than any (or the maximal) Pattern Count found so far, go to the last iteration **/
		if ((MaxCount > (
				(ChunkDataStart(NMRDataStruct, NMRDataStruct->ChunkCount - 1)/2 - ChunkDataStart(NMRDataStruct, 0)/2)/MinLength + 
				((ChunkDataStart(NMRDataStruct, NMRDataStruct->ChunkCount - 1)/2 + ChunkIndexRange(NMRDataStruct, NMRDataStruct->ChunkCount - 1)) - (ChunkDataStart(NMRDataStruct, 0)/2 + ChunkIndexRange(NMRDataStruct, 0)))/MinLength
			)
		) && (i < (NMRDataStruct->ChunkCount - 1)))
			i = NMRDataStruct->ChunkCount - 2;	/** Jump to the last iteration **/

	}
	
	/** Classify the most common patterns **/
	for (k = 0; k < ValidPatterns; k++) {
		
		Patterns[k].Start = Patterns[k].Offset;
		if (ChunkDataStart(NMRDataStruct, 0)/2 > Patterns[k].Offset)
			Patterns[k].Start += ((ChunkDataStart(NMRDataStruct, 0)/2 - Patterns[k].Offset)/Patterns[k].Length)*Patterns[k].Length;
		
		/** ! Count changes meaning here ! It means the number of chunks now. **/
		Patterns[k].Count = (ChunkDataStart(NMRDataStruct, NMRDataStruct->ChunkCount - 1)/2 + ChunkIndexRange(NMRDataStruct, NMRDataStruct->ChunkCount - 1) - Patterns[k].Start + Patterns[k].Length - Patterns[k].DataLength)/Patterns[k].Length;
		if ((ChunkDataStart(NMRDataStruct, NMRDataStruct->ChunkCount - 1)/2 + ChunkIndexRange(NMRDataStruct, NMRDataStruct->ChunkCount - 1) - Patterns[k].Start + Patterns[k].Length - Patterns[k].DataLength)%Patterns[k].Length > 0)
			Patterns[k].Count++;
		if (Patterns[k].Count > (MaxLength - Patterns[k].Start + Patterns[k].Length - Patterns[k].DataLength)/Patterns[k].Length)
			Patterns[k].Count = (MaxLength - Patterns[k].Start + Patterns[k].Length - Patterns[k].DataLength)/Patterns[k].Length;
		
		if ((Patterns[k].Count > 0) && (Patterns[k].DataLength > 0))	/** probably unnecessary check **/
			End = Patterns[k].Start + (Patterns[k].Count - 1) * Patterns[k].Length + Patterns[k].DataLength - 1;
		else 
			End = Patterns[k].Start;
		
		for (m = 0; ((size_t) m) < MaxLength; m++) {
			
			if ((m < Patterns[k].Start) && (OrPad[m] != 0)) {
				Patterns[k].FalseNegative++;
			}
			
			if ((m >= Patterns[k].Start) && (m <= End)) {
				if ((((m - Patterns[k].Offset) % Patterns[k].Length) < Patterns[k].DataLength) && (OrPad[m] == 0))
					Patterns[k].FalsePositive++;
				
				if ((((m - Patterns[k].Offset) % Patterns[k].Length) >= Patterns[k].DataLength) && (OrPad[m] != 0))
					Patterns[k].FalseNegative++;
			}
			
			if ((m > End) && (OrPad[m] != 0)) {
				Patterns[k].FalseNegative++;
			}
		}
		
	}
	
	free(OrPad);
	OrPad = NULL;
	
	/** Check the false negative match ratios and keep only the patterns with the lowest one **/
	for (k = 0; k < ValidPatterns; k++) 
		if (Patterns[k].FalseNegative < MinFalseNegative)
			MinFalseNegative = Patterns[k].FalseNegative;
	
	for (j = 0, k = 0; k < ValidPatterns; k++) 
		if (Patterns[k].FalseNegative == MinFalseNegative) 
			Patterns[j++] = Patterns[k];
	ValidPatterns = j;
	
	/** Check the false positive match ratios and keep only the patterns with the lowest one **/
	for (k = 0; k < ValidPatterns; k++) 
		if (Patterns[k].FalsePositive < MinFalsePositive)
			MinFalsePositive = Patterns[k].FalsePositive;
	
	for (j = 0, k = 0; k < ValidPatterns; k++) 
		if (Patterns[k].FalsePositive == MinFalsePositive) 
			Patterns[j++] = Patterns[k];
	ValidPatterns = j;
	
	if (ValidPatterns > 0) {
		/** If there are two or more of them, pick the first one, probably with shorter Length and DataLength. **/
		AuxChunkCount = Patterns[0].Count;
		
		if ((AuxChunkCount != (NMRDataStruct->ChunkCount)) || ((NMRDataStruct->ChunkSet) == NULL)) {
			AuxPointer = NMRDataStruct->ChunkSet;
			NMRDataStruct->ChunkSet = (SignalWindow *) realloc(NMRDataStruct->ChunkSet, AuxChunkCount*sizeof(SignalWindow));
			
			if (NMRDataStruct->ChunkSet == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating chunk set memory space");
				free(AuxPointer);
				AuxPointer = NULL;
				NMRDataStruct->ChunkCount = 0;
				free(Patterns);
				Patterns = NULL;
				return (MEM_ALLOC_ERROR | DATA_EMPTY);
			}
			
			NMRDataStruct->ChunkCount = AuxChunkCount;
		}
		
		/** Recreate chunk set **/
		for (i = 0, j = Patterns[0].Start; i < Patterns[0].Count; i++, j += Patterns[0].Length) {
			ChunkDataStart(NMRDataStruct, i) = 2*((intptr_t) j);
			ChunkIndexRange(NMRDataStruct, i) = Patterns[0].DataLength;
		}
		
	} else {
		NMRDataStruct->ChunkCount = 0;
		free(NMRDataStruct->ChunkSet);
		NMRDataStruct->ChunkSet = NULL;
	}
	
	free(Patterns);
	Patterns = NULL;

	return DATA_OK;
}

int FreeChunkSet(NMRData *NMRDataStruct) {

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	free(NMRDataStruct->ChunkSet);
	NMRDataStruct->ChunkSet = NULL;
	NMRDataStruct->ChunkCount = 0;

	return DATA_EMPTY;
}



int GetEchoPeaksEnvelope(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	double Amp = 0.0;
	unsigned long UpperEstimate = 0;
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	size_t Index = 0;
	size_t IndexMin = 0;
	size_t IndexMax = 0;
	size_t IndexRange = 0;
	double *AuxPointerDouble = NULL;
	int RetVal = DATA_OK;
	long Val = 0;
	size_t Start = 0;
	size_t Range = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if ((NMRDataStruct->Steps == NULL) || (NMRDataStruct->StepCount == 0)) 
		return DATA_OK;

	if (StepNo < 0) {
		Start = 0;
		Range = StepNoRange(NMRDataStruct);
	} else 
	if ((unsigned long) StepNo < StepNoRange(NMRDataStruct)) {
		Start = StepNo;
		Range = StepNo + 1;
	} else 
		return INVALID_PARAMETER;

	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_ChunkStart, PARAM_LONG, &Val, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'ChunkStart' check failed", "Creating echo peaks envelope");
		return RetVal;
	}
	
	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_ChunkEnd, PARAM_LONG, &Val, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'ChunkEnd' check failed", "Creating echo peaks envelope");
		return RetVal;
	}

	
	for (k = Start; k < Range; k++) {
		if (NMRDataStruct->Steps[k].Flags & Flag(CHECK_EchoPeaksEnvelope))
			continue;	/** This step is already done **/
		
		if ((ChunkNoRange(NMRDataStruct) != EchoPeaksEnvelopeIndexRange(NMRDataStruct, k)) || (EchoPeaksEnvelopeDataStart(NMRDataStruct, k) == NULL)) {
			AuxPointerDouble = (double *) EchoPeaksEnvelopeDataStart(NMRDataStruct, k);
			EchoPeaksEnvelopeDataStart(NMRDataStruct, k) = (double *) realloc(EchoPeaksEnvelopeDataStart(NMRDataStruct, k), 2*ChunkNoRange(NMRDataStruct)*sizeof(double));
	
			if (EchoPeaksEnvelopeDataStart(NMRDataStruct, k) == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating echo peaks envelope data memory space");
				free(AuxPointerDouble);
				AuxPointerDouble = NULL;
				EchoPeaksEnvelopeIndexRange(NMRDataStruct, k) = 0;
				return (MEM_ALLOC_ERROR | DATA_INVALID);
			}
	
			EchoPeaksEnvelopeIndexRange(NMRDataStruct, k) = ChunkNoRange(NMRDataStruct);
		}
		
		for (i = 0; i < EchoPeaksEnvelopeIndexRange(NMRDataStruct, k); i++) {
			EchoPeaksEnvelopeTime(NMRDataStruct, k, i) = 0.0;
			EchoPeaksEnvelopeAmp(NMRDataStruct, k, i) = 0.0;
			
			/** bound just into the chosen part of chunks **/
			IndexMin = NMRDataStruct->ChunkStart;
			IndexMax = (NMRDataStruct->ChunkEnd < ChunkIndexRange(NMRDataStruct, i))?(NMRDataStruct->ChunkEnd + 1):(ChunkIndexRange(NMRDataStruct, i));
			IndexRange = (IndexMax > IndexMin)?(IndexMax - IndexMin):(0);
			
			/** maximum of echo amplitude is supposed to be near the centre of chunk - start at the centre and proceed alternately towards both ends -> cca 2-times faster execution for complete spectrum measurement **/
			for (j = 0, Index = IndexMin + IndexRange/2; j < IndexRange; j++) {
				if (j%2)
					Index -= j;
				else 
					Index += j;
				
				/** computes estimate first for faster execution **/
				UpperEstimate = (unsigned long) labs(ChunkReal(NMRDataStruct, k, i, Index)) + (unsigned long) labs(ChunkImag(NMRDataStruct, k, i, Index));
				
				if (UpperEstimate > (unsigned long) EchoPeaksEnvelopeAmp(NMRDataStruct, k, i)) {
					/** may take some time **/
					Amp = hypot(ChunkReal(NMRDataStruct, k, i, Index), ChunkImag(NMRDataStruct, k, i, Index));
					
					if (Amp > EchoPeaksEnvelopeAmp(NMRDataStruct, k, i)) {
						EchoPeaksEnvelopeTime(NMRDataStruct, k, i) = ChunkTime(NMRDataStruct, k, i, Index);	/** time [us] **/
						EchoPeaksEnvelopeAmp(NMRDataStruct, k, i) = Amp;
					}
				}
			}
		}
		
	}
	
	return DATA_OK;
}



int GetChunkAvg(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t MaxChunkLength = 0;
	size_t Counter = 0;
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	double *AuxPointerDouble = NULL;
	long Val = 0;
	size_t Start = 0;
	size_t Range = 0;
	int RetVal = DATA_OK;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if (StepNo < 0) {
		Start = 0;
		Range = StepNoRange(NMRDataStruct);
	} else 
	if ((unsigned long) StepNo < StepNoRange(NMRDataStruct)) {
		Start = StepNo;
		Range = StepNo + 1;
	} else 
		return INVALID_PARAMETER;
	
	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_FirstChunk, PARAM_LONG, &Val, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'FirstChunk' check failed", "Chunk average calculation");
		return RetVal;
	}
	
	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_LastChunk, PARAM_LONG, &Val, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'LastChunk' check failed", "Chunk average calculation");
		return RetVal;
	}

	/** Determines MaxChunkLength (actually, all chunks should have the same length anyway) **/
	for (i = NMRDataStruct->FirstChunk; (i <= NMRDataStruct->LastChunk) && (i < ChunkNoRange(NMRDataStruct)); i++) 
		if (ChunkIndexRange(NMRDataStruct, i) > MaxChunkLength)
			MaxChunkLength = ChunkIndexRange(NMRDataStruct, i);
	
	if (MaxChunkLength == 0) {
		/** Deleting old data **/
		for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
			free(ChunkAvgDataStart(NMRDataStruct, i));
			ChunkAvgDataStart(NMRDataStruct, i) = NULL;
			free(ChunkAvgDataAmpStart(NMRDataStruct, i));
			ChunkAvgDataAmpStart(NMRDataStruct, i) = NULL;
			ChunkAvgIndexRange(NMRDataStruct, i) = 0;
		}

		return DATA_OK;
	}

	for (k = Start; k < Range; k++) {
		if (NMRDataStruct->Steps[k].Flags & Flag(CHECK_ChunkAvg))
			continue;	/** This step is already done **/
		
		if ((MaxChunkLength != ChunkAvgIndexRange(NMRDataStruct, k)) || (ChunkAvgDataStart(NMRDataStruct, k) == NULL) || (ChunkAvgDataAmpStart(NMRDataStruct, k) == NULL)) {
			AuxPointerDouble = ChunkAvgDataStart(NMRDataStruct, k);
			ChunkAvgDataStart(NMRDataStruct, k) = (double *) realloc(ChunkAvgDataStart(NMRDataStruct, k), 2*MaxChunkLength*sizeof(double));
	
			if (ChunkAvgDataStart(NMRDataStruct, k) == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating chunk average data memory space");
				free(AuxPointerDouble);
				AuxPointerDouble = NULL;
				ChunkAvgIndexRange(NMRDataStruct, k) = 0;
				return (MEM_ALLOC_ERROR | DATA_INVALID);
			}
	
			AuxPointerDouble = ChunkAvgDataAmpStart(NMRDataStruct, k);
			ChunkAvgDataAmpStart(NMRDataStruct, k) = (double *) realloc(ChunkAvgDataAmpStart(NMRDataStruct, k), MaxChunkLength*sizeof(double));

			if (ChunkAvgDataAmpStart(NMRDataStruct, k) == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating chunk average data memory space");
				free(AuxPointerDouble);
				AuxPointerDouble = NULL;
				ChunkAvgIndexRange(NMRDataStruct, k) = 0;
				return (MEM_ALLOC_ERROR | DATA_INVALID);
			}
			
			ChunkAvgIndexRange(NMRDataStruct, k) = MaxChunkLength;
		}
			
		for (j = 0; j < MaxChunkLength; j++) {
			ChunkAvgReal(NMRDataStruct, k, j) = 0.0;
			ChunkAvgImag(NMRDataStruct, k, j) = 0.0;
		}

		if (!(StepFlag(NMRDataStruct, k) & STEP_BLANK)) {
			Counter = 0;
			for (i = NMRDataStruct->FirstChunk; (i <= NMRDataStruct->LastChunk) && (i < ChunkNoRange(NMRDataStruct)); i++) {
				if ((ChunkDataStart(NMRDataStruct, i)/2 + ChunkIndexRange(NMRDataStruct, i)) <= TDDIndexRange(NMRDataStruct, k)) {	/** It shouldn't be really necessary to test this **/
					Counter++;
				
					for (j = 0; j < ChunkIndexRange(NMRDataStruct, i); j++) {
						ChunkAvgReal(NMRDataStruct, k, j) += (double) ChunkReal(NMRDataStruct, k, i, j);
						ChunkAvgImag(NMRDataStruct, k, j) += (double) ChunkImag(NMRDataStruct, k, i, j);
					}
				}
			}

			if (Counter > 0)
				for (j = 0; j < ChunkAvgIndexRange(NMRDataStruct, k); j++) {
					ChunkAvgReal(NMRDataStruct, k, j) /= (double) Counter;
					ChunkAvgImag(NMRDataStruct, k, j) /= (double) Counter;
				}
			
			for (j = 0; j < ChunkAvgIndexRange(NMRDataStruct, k); j++) 
				ChunkAvgAmp(NMRDataStruct, k, j) = hypot(ChunkAvgReal(NMRDataStruct, k, j), ChunkAvgImag(NMRDataStruct, k, j));
			
		} else {
			for (j = 0; j < MaxChunkLength; j++) 
				ChunkAvgAmp(NMRDataStruct, k, j) = 0.0;
		}
		
	}
	
	return DATA_OK;
}



int GetDFTResult(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	double *aux_in = NULL;
	double *aux_out = NULL;
	double *aux_amp = NULL;
	fftw_plan DFTPlan;
	size_t i = 0;
	size_t j = 0;
	long Val = 0;
	int RetVal = DATA_OK;
	int FFTlength = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;


	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_ChunkStart, PARAM_LONG, &Val, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'ChunkStart' check failed", "Carrying out Fourier transform");
		return RetVal;
	}
	
	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_ChunkEnd, PARAM_LONG, &Val, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'ChunkEnd' check failed", "Carrying out Fourier transform");
		return RetVal;
	}

	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_DFTLength, PARAM_LONG, &Val, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'DFTLength' check failed", "Carrying out Fourier transform");
		return RetVal;
	}

	if (NMRDataStruct->DFTLength == 0) {	/** should not normally happen **/
		FreeDFTResult(NMRDataStruct);
		return DATA_OK;
	}
	
	if (NMRDataStruct->DFTLength != DFTIndexRange(NMRDataStruct, 0)) {
		FreeDFTResult(NMRDataStruct);

		aux_in = (double *) fftw_malloc((NMRDataStruct->DFTLength)*StepNoRange(NMRDataStruct)*2*sizeof(double));
		aux_out = (double *) fftw_malloc((NMRDataStruct->DFTLength)*StepNoRange(NMRDataStruct)*2*sizeof(double));
		aux_amp = (double *) malloc((NMRDataStruct->DFTLength)*StepNoRange(NMRDataStruct)*sizeof(double));
		
		if ( (aux_in == NULL) || (aux_out == NULL) || (aux_amp == NULL) ) {
			fftw_free(aux_in);
			fftw_free(aux_out);
			free(aux_amp);

			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating DFT data memory space");
			return (MEM_ALLOC_ERROR | DATA_EMPTY);
		}
		
		for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
			DFTIndexRange(NMRDataStruct, i) = NMRDataStruct->DFTLength;
			NMRDataStruct->Steps[i].DFTInput = aux_in + 2*i*(NMRDataStruct->DFTLength);
			NMRDataStruct->Steps[i].DFTOutput = aux_out + 2*i*(NMRDataStruct->DFTLength);
			NMRDataStruct->Steps[i].DFTPhaseCorrOutput = aux_out + 2*i*(NMRDataStruct->DFTLength);
			NMRDataStruct->Steps[i].DFTOutAmp = aux_amp + i*(NMRDataStruct->DFTLength);
			NMRDataStruct->Steps[i].DFTPhaseCorrOutAmp = aux_amp + i*(NMRDataStruct->DFTLength);
		}
	}

	FFTlength = NMRDataStruct->DFTLength;
	
	DFTPlan = fftw_plan_many_dft(1, &FFTlength, NMRDataStruct->StepCount , 
								(fftw_complex *) (NMRDataStruct->Steps->DFTInput), NULL, 1, NMRDataStruct->DFTLength, 
								(fftw_complex *) (NMRDataStruct->Steps->DFTOutput), NULL, 1, NMRDataStruct->DFTLength, 
								FFTW_FORWARD, FFTW_ESTIMATE | FFTW_DESTROY_INPUT);
	
	/** Copying input data **/
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
		memcpy(NMRDataStruct->Steps[i].DFTInput, ChunkAvgProcStart(NMRDataStruct, i), ChunkAvgProcIndexRange(NMRDataStruct, i)*2*sizeof(double));
		/** zero-padding **/
		for (j = ChunkAvgProcIndexRange(NMRDataStruct, i); j < DFTIndexRange(NMRDataStruct, i); j++) {
			NMRDataStruct->Steps[i].DFTInput[2*j + 0] = 0.0;
			NMRDataStruct->Steps[i].DFTInput[2*j + 1] = 0.0;
		}
	}
	
	if (NMRDataStruct->ScaleFirstTDPoint) {
		for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
			NMRDataStruct->Steps[i].DFTInput[0] *= 0.5;
			NMRDataStruct->Steps[i].DFTInput[1] *= 0.5;
		}
	}

	fftw_execute(DFTPlan);

	fftw_destroy_plan(DFTPlan);

	/** Computing amplitude **/
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) 
		for (j = 0; j < DFTIndexRange(NMRDataStruct, i); j++) 
			DFTAmp(NMRDataStruct, i, j) = hypot(DFTReal(NMRDataStruct, i, j), DFTImag(NMRDataStruct, i, j));
	
	return DATA_OK;
}


int FreeDFTResult(NMRData *NMRDataStruct) {
	size_t i = 0;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if (NMRDataStruct->Steps != NULL) {
		/** Free DFT in/out space **/
		fftw_free(NMRDataStruct->Steps->DFTInput);
		fftw_free(NMRDataStruct->Steps->DFTOutput);
		free(NMRDataStruct->Steps->DFTOutAmp);
		
		/** Free phase-corrected DFT output, if there is any (i.e. if its not just a pointer to the DFT out memory space) **/
		if (NMRDataStruct->Steps->DFTPhaseCorrOutput != NMRDataStruct->Steps->DFTOutput)
			fftw_free(NMRDataStruct->Steps->DFTPhaseCorrOutput);

		/** Free phase- and offset-corrected DFT output amplitude, if there is any (i.e. if its not just a pointer to the DFT output amplitude memory space) **/
		if (NMRDataStruct->Steps->DFTPhaseCorrOutAmp != NMRDataStruct->Steps->DFTOutAmp)
			free(NMRDataStruct->Steps->DFTPhaseCorrOutAmp);
		
		for (i = 0; i < NMRDataStruct->StepCount; i++) {
			NMRDataStruct->Steps[i].DFTInput = NULL;
			NMRDataStruct->Steps[i].DFTOutput = NULL;
			NMRDataStruct->Steps[i].DFTOutAmp = NULL;
			NMRDataStruct->Steps[i].DFTLength = 0;
			NMRDataStruct->Steps[i].DFTPhaseCorrOutput = NULL;
			NMRDataStruct->Steps[i].DFTPhaseCorrOutAmp = NULL;
		}
	}

	return DATA_EMPTY;
}



int GetDFTPhaseCorrPrep(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	double *aux_phased = NULL;
	size_t i = 0;
	size_t j = 0;
	double ReCoef = 0.0;
	double ImCoef = 0.0;
	unsigned char DoPhaseCorrection = 0;
	int RetVal = DATA_OK;
	
	long PilotStep = 0;
	long Follow = 0;
	long AutoAllTogether = 0;
	double RealSum = 0.0;
	double ImagSum = 0.0;
	double Angle = 0.0;
	
	long Val = 0;
	
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	

	if ((Components & Flag(CHECK_DFTPhaseCorrPrep_AutoCorr)) && !(NMRDataStruct->Flags & Flag(CHECK_DFTPhaseCorrPrep_AutoCorr))) {
		if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_Filter, PARAM_LONG, &Val, NULL)) != DATA_OK) {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'Filter' check failed", "Preparing phase correction");
			return RetVal;
		}
		
		if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr0FollowAuto, PARAM_LONG, &Follow, &PilotStep)) != DATA_OK) {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr0FollowAuto' check failed", "Preparing phase correction");
			return RetVal;
		}
		
		if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr0AutoAllTogether, PARAM_LONG, &AutoAllTogether, NULL)) != DATA_OK) {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr0AutoAllTogether' check failed", "Preparing phase correction");
			return RetVal;
		}
		
		if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr1ManualRefDataStart, PARAM_LONG, &Val, NULL)) != DATA_OK) {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr1ManualRefDataStart' check failed", "Preparing phase correction");
			return RetVal;
		}

		/** Carry out automatic determination of phase correction values if necessary. (Automatic 1st-order phase correction not implemented.) **/
		if (Follow) {
			/** Find out the 0th-order phase corection value automatically for the PilotStep and set it to all steps with PHASE0_FollowAuto flag set. **/
			if (DFTPhaseCorr1Relative(NMRDataStruct, PilotStep) == 0) {
				for (j = 0; j < DFTProcIndexRange(NMRDataStruct, PilotStep); j++) {
					RealSum += DFTProcReal(NMRDataStruct, PilotStep, j);
					ImagSum += DFTProcImag(NMRDataStruct, PilotStep, j);
				}
			} else {
				for (j = 0; j < DFTProcIndexRange(NMRDataStruct, PilotStep); j++) {
					ReCoef = cos(2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, PilotStep)*(DFTProcFreq(NMRDataStruct, PilotStep, j) - StepFreq(NMRDataStruct, PilotStep)));
					ImCoef = sin(2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, PilotStep)*(DFTProcFreq(NMRDataStruct, PilotStep, j) - StepFreq(NMRDataStruct, PilotStep)));
					
					RealSum += ReCoef*DFTProcReal(NMRDataStruct, PilotStep, j) - ImCoef*DFTProcImag(NMRDataStruct, PilotStep, j);
					ImagSum += ReCoef*DFTProcImag(NMRDataStruct, PilotStep, j) + ImCoef*DFTProcReal(NMRDataStruct, PilotStep, j);
				}
			}
			
			Angle = atan2(ImagSum, RealSum);
			DFTPhaseCorr0(NMRDataStruct, PilotStep) = lround(-Angle*1000.0*180.0/M_PI);
			
			for (i = 0; i < StepNoRange(NMRDataStruct); i++) 
				if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) == PHASE0_FollowAuto) 
					DFTPhaseCorr0(NMRDataStruct, i) = lround(-Angle*1000.0*180.0/M_PI);
			
		} else 
		if (AutoAllTogether) {
			/** Find out the common 0th-order phase corection value automatically for all steps with PHASE0_AutoAllTogether flag set. **/
			for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
			/*	if ((StepFlag(NMRDataStruct, i) & 0x0f) != STEP_OK)	*/
				if (StepFlag(NMRDataStruct, i) & (STEP_BLANK | STEP_IGNORE))
					continue;

				if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) == PHASE0_AutoAllTogether) {
					if (DFTPhaseCorr1Relative(NMRDataStruct, i) == 0) {
						for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
							RealSum += DFTProcReal(NMRDataStruct, i, j);
							ImagSum += DFTProcImag(NMRDataStruct, i, j);
						}
					} else {
						for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
							ReCoef = cos(2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(DFTProcFreq(NMRDataStruct, i, j) - StepFreq(NMRDataStruct, i)));
							ImCoef = sin(2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(DFTProcFreq(NMRDataStruct, i, j) - StepFreq(NMRDataStruct, i)));
							
							RealSum += ReCoef*DFTProcReal(NMRDataStruct, i, j) - ImCoef*DFTProcImag(NMRDataStruct, i, j);
							ImagSum += ReCoef*DFTProcImag(NMRDataStruct, i, j) + ImCoef*DFTProcReal(NMRDataStruct, i, j);
						}
					}
				}
			}
			Angle = atan2(ImagSum, RealSum);

			for (i = 0; i < StepNoRange(NMRDataStruct); i++) 
				if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) == PHASE0_AutoAllTogether) 
					DFTPhaseCorr0(NMRDataStruct, i) = lround(-Angle*1000.0*180.0/M_PI);
			
		} else {
			/** Find out the 0th-order phase corection values automatically for all steps with PHASE0_Auto flag set. **/
			for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
				if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) == PHASE0_Auto) {
					RealSum = 0.0;
					ImagSum = 0.0;
					if (DFTPhaseCorr1Relative(NMRDataStruct, i) == 0) {
						for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
							RealSum += DFTProcReal(NMRDataStruct, i, j);
							ImagSum += DFTProcImag(NMRDataStruct, i, j);
						}
					} else {
						for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
							ReCoef = cos(2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(DFTProcFreq(NMRDataStruct, i, j) - StepFreq(NMRDataStruct, i)));
							ImCoef = sin(2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(DFTProcFreq(NMRDataStruct, i, j) - StepFreq(NMRDataStruct, i)));
							
							RealSum += ReCoef*DFTProcReal(NMRDataStruct, i, j) - ImCoef*DFTProcImag(NMRDataStruct, i, j);
							ImagSum += ReCoef*DFTProcImag(NMRDataStruct, i, j) + ImCoef*DFTProcReal(NMRDataStruct, i, j);
						}
					}
					Angle = atan2(ImagSum, RealSum);
					DFTPhaseCorr0(NMRDataStruct, i) = lround(-Angle*1000.0*180.0/M_PI);
				}
			}
		}
	}
	
	if ((Components & Flag(CHECK_DFTPhaseCorrPrep_MemReIm)) && !(NMRDataStruct->Flags & Flag(CHECK_DFTPhaseCorrPrep_MemReIm))) {
		/** Check if any phase correction has to be actually done **/
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && (!DoPhaseCorrection); i++) {
			if (DFTPhaseCorr0(NMRDataStruct, i) != 0)
				DoPhaseCorrection = 1;
			if (DFTPhaseCorr1Relative(NMRDataStruct, i) != 0)
				DoPhaseCorrection = 1;
		}
		
		/** Allocate memory if necessary and not already available **/
		if ((DoPhaseCorrection || NMRDataStruct->RemoveOffset) && (NMRDataStruct->Steps->DFTPhaseCorrOutput == NMRDataStruct->Steps->DFTOutput)) {
			MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr_ReIm, ALL_STEPS);
			aux_phased = (double *) fftw_malloc((NMRDataStruct->DFTLength)*StepNoRange(NMRDataStruct)*2*sizeof(double));
			
			if (aux_phased == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating memory for phase corrected DFT data");
				return (MEM_ALLOC_ERROR | DATA_OLD);
			}
			
			for (i = 0; i < StepNoRange(NMRDataStruct); i++) 
				NMRDataStruct->Steps[i].DFTPhaseCorrOutput = aux_phased + 2*i*(NMRDataStruct->DFTLength);
		}
	}
	
	if ((Components & Flag(CHECK_DFTPhaseCorrPrep_MemAmp)) && !(NMRDataStruct->Flags & Flag(CHECK_DFTPhaseCorrPrep_MemAmp))) {
		/** Allocate memory if necessary and not already available **/
		if (NMRDataStruct->RemoveOffset && (NMRDataStruct->Steps->DFTPhaseCorrOutAmp == NMRDataStruct->Steps->DFTOutAmp)) {
			MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr_Amp, ALL_STEPS);
			aux_phased = (double *) malloc((NMRDataStruct->DFTLength)*StepNoRange(NMRDataStruct)*sizeof(double));
			
			if (aux_phased == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating memory for phase corrected DFT data");
				return (MEM_ALLOC_ERROR | DATA_OLD);
			}
			
			for (i = 0; i < StepNoRange(NMRDataStruct); i++) 
				NMRDataStruct->Steps[i].DFTPhaseCorrOutAmp = aux_phased + i*(NMRDataStruct->DFTLength);
		}
	}
	
	return DATA_OK;
}



int GetDFTPhaseCorr(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t i = 0;
	size_t j = 0;
	size_t Start = 0;
	size_t Range = 0;
	unsigned char DoPhaseCorrection = 0;
	double ReCoef = 0.0;
	double ImCoef = 0.0;
	double ReOffset = 0.0;
	double ImOffset = 0.0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if (StepNo < 0) {
		Start = 0;
		Range = StepNoRange(NMRDataStruct);
	} else 
	if ((unsigned long) StepNo < StepNoRange(NMRDataStruct)) {
		Start = StepNo;
		Range = StepNo + 1;
	} else 
		return INVALID_PARAMETER;
	
	for (i = Start; i < Range; i++) {
		if (NMRDataStruct->Steps[i].Flags & Flag(CHECK_DFTPhaseCorr))
			continue;	/** This step is already done **/

		/** Get the real and imaginary parts **/
		if ((Components & Flag(CHECK_DFTPhaseCorr_ReIm)) && !(NMRDataStruct->Steps[i].Flags & Flag(CHECK_DFTPhaseCorr_ReIm))) {
			DoPhaseCorrection = 0;
			if (DFTPhaseCorr0(NMRDataStruct, i) != 0)
				DoPhaseCorrection = 1;
			if (DFTPhaseCorr1Relative(NMRDataStruct, i) != 0)
				DoPhaseCorrection = 1;
			
			if ((!DoPhaseCorrection) && (NMRDataStruct->Steps->DFTPhaseCorrOutput != NMRDataStruct->Steps->DFTOutput)) 				
				memcpy(NMRDataStruct->Steps[i].DFTPhaseCorrOutput, NMRDataStruct->Steps[i].DFTOutput, (NMRDataStruct->DFTLength)*2*sizeof(double));	/** Just copy the unphased data **/
			
			if (DoPhaseCorrection) {
				if (DFTPhaseCorr1Relative(NMRDataStruct, i) == 0) {
					ReCoef = cos(M_PI/180.0*0.001*DFTPhaseCorr0(NMRDataStruct, i));
					ImCoef = sin(M_PI/180.0*0.001*DFTPhaseCorr0(NMRDataStruct, i));
					
					for (j = 0; j < DFTIndexRange(NMRDataStruct, i); j++) {
						DFTPhaseCorrReal(NMRDataStruct, i, j) = ReCoef*DFTReal(NMRDataStruct, i, j) - ImCoef*DFTImag(NMRDataStruct, i, j);
						DFTPhaseCorrImag(NMRDataStruct, i, j) = ReCoef*DFTImag(NMRDataStruct, i, j) + ImCoef*DFTReal(NMRDataStruct, i, j);
					}			
				} else {
					for (j = 0; j < DFTIndexRange(NMRDataStruct, i); j++) {
						ReCoef = cos(M_PI/180.0*0.001*DFTPhaseCorr0(NMRDataStruct, i) + 2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(DFTFreq(NMRDataStruct, i, j) - StepFreq(NMRDataStruct, i)));
						ImCoef = sin(M_PI/180.0*0.001*DFTPhaseCorr0(NMRDataStruct, i) + 2*M_PI*0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(DFTFreq(NMRDataStruct, i, j) - StepFreq(NMRDataStruct, i)));
						
						DFTPhaseCorrReal(NMRDataStruct, i, j) = ReCoef*DFTReal(NMRDataStruct, i, j) - ImCoef*DFTImag(NMRDataStruct, i, j);
						DFTPhaseCorrImag(NMRDataStruct, i, j) = ReCoef*DFTImag(NMRDataStruct, i, j) + ImCoef*DFTReal(NMRDataStruct, i, j);
					}			
				}
			}
			
			if (NMRDataStruct->RemoveOffset) {
				ReCoef = cos(M_PI/180.0*0.001*DFTPhaseCorr0(NMRDataStruct, i))*(0.5 + 0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(NMRDataStruct->SWMh));
				ImCoef = sin(M_PI/180.0*0.001*DFTPhaseCorr0(NMRDataStruct, i))*(0.5 + 0.001*DFTPhaseCorr1Relative(NMRDataStruct, i)*(NMRDataStruct->SWMh));
				
				ReOffset = ReCoef*NMRDataStruct->Steps[i].DFTInput[0] - ImCoef*NMRDataStruct->Steps[i].DFTInput[1];
				ImOffset = ReCoef*NMRDataStruct->Steps[i].DFTInput[1] + ImCoef*NMRDataStruct->Steps[i].DFTInput[0];
				
				for (j = 0; j < DFTIndexRange(NMRDataStruct, i); j++) {
					DFTPhaseCorrReal(NMRDataStruct, i, j) -= ReOffset;
					DFTPhaseCorrImag(NMRDataStruct, i, j) -= ImOffset;
				}
			}
		}
		
		/** Get the amplitude **/
		if ((Components & Flag(CHECK_DFTPhaseCorr_Amp)) && !(NMRDataStruct->Steps[i].Flags & Flag(CHECK_DFTPhaseCorr_Amp))) {
			if ((!(NMRDataStruct->RemoveOffset)) && (NMRDataStruct->Steps->DFTPhaseCorrOutAmp != NMRDataStruct->Steps->DFTOutAmp))
				memcpy(NMRDataStruct->Steps[i].DFTPhaseCorrOutAmp, NMRDataStruct->Steps[i].DFTOutAmp, (NMRDataStruct->DFTLength)*sizeof(double));	/** Just copy the uncorrected data **/

			if (NMRDataStruct->RemoveOffset) 
				for (j = 0; j < DFTIndexRange(NMRDataStruct, i); j++) 
					DFTPhaseCorrAmp(NMRDataStruct, i, j) = hypot(DFTPhaseCorrReal(NMRDataStruct, i, j), DFTPhaseCorrImag(NMRDataStruct, i, j));
		}
	}
	
	return DATA_OK;
}



int CompareDouble(const void * dVal1, const void * dVal2) {
	if ( (*((double *) dVal1)) < (*((double *) dVal2)) )
		return -1;
	
	if ( (*((double *) dVal1)) > (*((double *) dVal2)) )
		return 1;
	
	return 0;
}


int GetDFTEnvelope(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	size_t Step = 0;
	short Uniform = 0;
	unsigned char Valid = 1;
	double *AuxPointer = NULL;
	
	size_t ValidSteps = 0;
	size_t ValidPoints = 0;
	size_t BufferLength = 0;

	double Amp1 = 0.0;
	double Amp2 = 0.0;
	double Amp3 = 0.0;
	
	double IdxOffset = 0.0;
	double IdxOffFrac = 0.0;
	long long IdxOffInt = 0;
	
	long long Index2 = 0;
	
	int RetVal = DATA_OK;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if ((NMRDataStruct->Steps == NULL) || (NMRDataStruct->StepCount == 0)) {
		FreeDFTEnvelope(NMRDataStruct);
		return DATA_OK;
	}

	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_Filter, PARAM_LONG, NULL, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'Filter' check failed", "Calculating DFT moduli envelope");
		return RetVal;
	}
	
	
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
	/*	if ((StepFlag(NMRDataStruct, i) & 0x0f) == STEP_OK)	*/
		if (!(StepFlag(NMRDataStruct, i) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE)))
			ValidSteps++;
	}
	
	if (ValidSteps == 0) {
		FreeDFTEnvelope(NMRDataStruct);
		return DATA_OK;
	}
	
	if (NMRDataStruct->AcquInfo.AssocValueType != ASSOC_FREQ_MHZ) 
		BufferLength = DFTProcIndexRange(NMRDataStruct, 0);
	else 
	if ((NMRDataStruct->DFTEnvelopeArray != NULL) && (NMRDataStruct->DFTEnvelopeCount > 0))
		BufferLength = NMRDataStruct->DFTEnvelopeCount;
	else
		BufferLength = DFTProcIndexRange(NMRDataStruct, 0) * (ValidSteps / 16 + 4);	/** initial guess **/
	
	if (BufferLength != NMRDataStruct->DFTEnvelopeCount) {
		AuxPointer = NMRDataStruct->DFTEnvelopeArray;
		NMRDataStruct->DFTEnvelopeArray = (double *) realloc(NMRDataStruct->DFTEnvelopeArray, 2*BufferLength*sizeof(double));
		
		if (NMRDataStruct->DFTEnvelopeArray == NULL) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating envelope point array memory space");
			free(AuxPointer);
			AuxPointer = NULL;
			NMRDataStruct->DFTEnvelopeCount = 0;
			
			return (MEM_ALLOC_ERROR | DATA_EMPTY);
		}
	}

	
	/** The simple case - all steps share the same set of frequencies **/
	if (NMRDataStruct->AcquInfo.AssocValueType != ASSOC_FREQ_MHZ) {
		for (j = 0; j < DFTProcIndexRange(NMRDataStruct, 0); j++) {
			DFTEnvelopeFreq(NMRDataStruct, j) = DFTProcFreq(NMRDataStruct, 0, j);
			DFTEnvelopeAmp(NMRDataStruct, j) = 0.0;
		}
		
		for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
		/*	if ((StepFlag(NMRDataStruct, i) & 0x0f) != STEP_OK)	*/
			if (StepFlag(NMRDataStruct, i) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE))
				continue;
			
			for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
				if (DFTProcPhaseCorrAmp(NMRDataStruct, i, j) > DFTEnvelopeAmp(NMRDataStruct, j))
					DFTEnvelopeAmp(NMRDataStruct, j) = DFTProcPhaseCorrAmp(NMRDataStruct, i, j);
			}
		}
		
		NMRDataStruct->DFTEnvelopeCount = BufferLength;
		return DATA_OK;
	}
	
	
	/** The complicated case - each step has its own set of frequencies (shifted with respect to other steps) **/
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
	/*	if ((StepFlag(NMRDataStruct, i) & 0x0f) != STEP_OK)	*/
		if (StepFlag(NMRDataStruct, i) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE))
			continue;
		
		for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
			Valid = 1;
			
			Amp1 = DFTProcPhaseCorrAmp(NMRDataStruct, i, j);

			for (k = 1, Step = i, Uniform = 0; (k < StepNoRange(NMRDataStruct)) && (Valid); k++) {
				/** If the point is going to be rejected, it should be done soon: start with the neighbouring steps and proceed farther alternately in both directions as long as possible. **/
				if ((!Uniform) && (k%2) && (Step < k))
					Uniform = 1;

				if ((!Uniform) && (!(k%2)) && ((StepNoRange(NMRDataStruct) - Step) <= k))
					Uniform = -1;
				
				if (Uniform > 0) 
					Step++;
				else
				if (Uniform < 0) 
					Step--;
				else {
					if (k%2) 
						Step -= k;
					else 
						Step += k;
				}
				
			/*	if ((StepFlag(NMRDataStruct, Step) & 0x0f) != STEP_OK)	*/
				if (StepFlag(NMRDataStruct, Step) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE))
					continue;
				
				/** Get the frequency offset between the steps in terms of indices **/
				IdxOffset = (NMRDataStruct->Steps[i].Freq - NMRDataStruct->Steps[Step].Freq) * ((double) NMRDataStruct->Steps[i].DFTLength / NMRDataStruct->SWMh);
				IdxOffInt = llround(floor(IdxOffset));
				IdxOffFrac = IdxOffset - IdxOffInt;
				
				Index2 = (long long) j + IdxOffInt;
	
				if ((Index2 >= 0) && ((size_t) (Index2 + 1) < DFTProcIndexRange(NMRDataStruct, Step))) {
					/** Compare Amp1 with linear interpolation between the two closest points **/
					Amp2 = DFTProcPhaseCorrAmp(NMRDataStruct, Step, Index2 + 0);
					Amp3 = DFTProcPhaseCorrAmp(NMRDataStruct, Step, Index2 + 1);

					if (((1.0 - IdxOffFrac)*Amp2 + IdxOffFrac*Amp3) > Amp1)
						Valid = 0;
				} else 
				if ((Index2 >= 0) && ((size_t) (Index2 + 1) == DFTProcIndexRange(NMRDataStruct, Step))) {
					/** Compare Amp1 with the end point at the very same frequency (rare case) **/
					Amp2 = DFTProcPhaseCorrAmp(NMRDataStruct, Step, Index2);

					if ((IdxOffFrac == 0.0) && (Amp2 > Amp1)) 
						Valid = 0;
				}
			}
			
			if (Valid) {
				/** Add the point **/
				ValidPoints++;
				
				if (ValidPoints > BufferLength) {
					BufferLength *= 2;
					AuxPointer = NMRDataStruct->DFTEnvelopeArray;
					NMRDataStruct->DFTEnvelopeArray = (double *) realloc(NMRDataStruct->DFTEnvelopeArray, 2*BufferLength*sizeof(double));
					
					if (NMRDataStruct->DFTEnvelopeArray == NULL) {
						NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating DFT envelope point array memory space");
						free(AuxPointer);
						AuxPointer = NULL;
						NMRDataStruct->DFTEnvelopeCount = 0;
						
						return (MEM_ALLOC_ERROR | DATA_EMPTY);
					}
				}
				
				DFTEnvelopeFreq(NMRDataStruct, ValidPoints - 1) = DFTProcFreq(NMRDataStruct, i, j);
				DFTEnvelopeAmp(NMRDataStruct, ValidPoints - 1) = Amp1;
			}
			
		}
		
	}

	if (ValidPoints == 0) {	/** usually should not happen **/
		FreeDFTEnvelope(NMRDataStruct);
		return DATA_OK;
	}

	/** Shrink to appropriate size **/
	NMRDataStruct->DFTEnvelopeCount = BufferLength = ValidPoints;
	AuxPointer = (double *) realloc(NMRDataStruct->DFTEnvelopeArray, 2*BufferLength*sizeof(double));
	if (AuxPointer != NULL) /** otherwise the data at NMRDataStruct->DFTEnvelopeArray are intact **/
		NMRDataStruct->DFTEnvelopeArray = AuxPointer;

	/** Sort **/
	qsort(NMRDataStruct->DFTEnvelopeArray, BufferLength, 2*sizeof(double), CompareDouble);

	return DATA_OK;
}


int FreeDFTEnvelope(NMRData *NMRDataStruct) {

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	free(NMRDataStruct->DFTEnvelopeArray);
	NMRDataStruct->DFTEnvelopeArray = NULL;
	NMRDataStruct->DFTEnvelopeCount = 0;

	return DATA_EMPTY;
}



int GetDFTRealEnvelope(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	size_t Step = 0;
	short Uniform = 0;
	unsigned char Valid = 1;
	double *AuxPointer = NULL;
	
	size_t ValidSteps = 0;
	size_t ValidPoints = 0;
	size_t BufferLength = 0;

	double Real1 = 0.0;
	double Real2 = 0.0;
	double Real3 = 0.0;
	
	double IdxOffset = 0.0;
	double IdxOffFrac = 0.0;
	long long IdxOffInt = 0;
	
	long long Index2 = 0;
	
	int RetVal = DATA_OK;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if ((NMRDataStruct->Steps == NULL) || (NMRDataStruct->StepCount == 0)) {
		FreeDFTRealEnvelope(NMRDataStruct);
		return DATA_OK;
	}

	if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_Filter, PARAM_LONG, NULL, NULL)) != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'Filter' check failed", "Calculating DFT real envelope");
		return RetVal;
	}
	
	
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
	/*	if ((StepFlag(NMRDataStruct, i) & 0x0f) == STEP_OK)	*/
		if (!(StepFlag(NMRDataStruct, i) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE)))
			ValidSteps++;
	}
	
	if (ValidSteps == 0) {
		FreeDFTRealEnvelope(NMRDataStruct);
		return DATA_OK;
	}
	
	if (NMRDataStruct->AcquInfo.AssocValueType != ASSOC_FREQ_MHZ) 
		BufferLength = DFTProcIndexRange(NMRDataStruct, 0);
	else 
	if ((NMRDataStruct->DFTRealEnvelopeArray != NULL) && (NMRDataStruct->DFTRealEnvelopeCount > 0))
		BufferLength = NMRDataStruct->DFTRealEnvelopeCount;
	else
		BufferLength = DFTProcIndexRange(NMRDataStruct, 0) * (ValidSteps / 16 + 4);	/** initial guess **/
	
	if (BufferLength != NMRDataStruct->DFTRealEnvelopeCount) {
		AuxPointer = NMRDataStruct->DFTRealEnvelopeArray;
		NMRDataStruct->DFTRealEnvelopeArray = (double *) realloc(NMRDataStruct->DFTRealEnvelopeArray, 2*BufferLength*sizeof(double));
		
		if (NMRDataStruct->DFTRealEnvelopeArray == NULL) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating DFT real envelope point array memory space");
			free(AuxPointer);
			AuxPointer = NULL;
			NMRDataStruct->DFTRealEnvelopeCount = 0;
			
			return (MEM_ALLOC_ERROR | DATA_EMPTY);
		}
	}

	
	/** The simple case - all steps share the same set of frequencies **/
	if (NMRDataStruct->AcquInfo.AssocValueType != ASSOC_FREQ_MHZ) {
		for (j = 0; j < DFTProcIndexRange(NMRDataStruct, 0); j++) {
			DFTRealEnvelopeFreq(NMRDataStruct, j) = DFTProcFreq(NMRDataStruct, 0, j);
			DFTRealEnvelopeReal(NMRDataStruct, j) = - HUGE_VAL;
		}
		
		for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
		/*	if ((StepFlag(NMRDataStruct, i) & 0x0f) != STEP_OK)	*/
			if (StepFlag(NMRDataStruct, i) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE))
				continue;
			
			for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
				if (DFTProcPhaseCorrReal(NMRDataStruct, i, j) > DFTRealEnvelopeReal(NMRDataStruct, j))
					DFTRealEnvelopeReal(NMRDataStruct, j) = DFTProcPhaseCorrReal(NMRDataStruct, i, j);
			}
		}
		
		NMRDataStruct->DFTRealEnvelopeCount = BufferLength;
		return DATA_OK;
	}
	
	
	/** The complicated case - each step has its own set of frequencies (shifted with respect to other steps) **/
	for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
	/*	if ((StepFlag(NMRDataStruct, i) & 0x0f) != STEP_OK)	*/
		if (StepFlag(NMRDataStruct, i) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE))
			continue;
		
		for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
			Valid = 1;
			
			Real1 = DFTProcPhaseCorrReal(NMRDataStruct, i, j);

			for (k = 1, Step = i, Uniform = 0; (k < StepNoRange(NMRDataStruct)) && (Valid); k++) {
				/** If the point is going to be rejected, it should be done soon: start with the neighbouring steps end proceed farther alternately in both directions as long as possible. **/
				if ((!Uniform) && (k%2) && (Step < k))
					Uniform = 1;

				if ((!Uniform) && (!(k%2)) && ((StepNoRange(NMRDataStruct) - Step) <= k))
					Uniform = -1;
				
				if (Uniform > 0) 
					Step++;
				else
				if (Uniform < 0) 
					Step--;
				else {
					if (k%2) 
						Step -= k;
					else 
						Step += k;
				}
				
			/*	if ((StepFlag(NMRDataStruct, Step) & 0x0f) != STEP_OK)	*/
				if (StepFlag(NMRDataStruct, Step) & (STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE))
					continue;
				
				/** Get the frequency offset between the steps in terms of indices **/
				IdxOffset = (NMRDataStruct->Steps[i].Freq - NMRDataStruct->Steps[Step].Freq) * ((double) NMRDataStruct->Steps[i].DFTLength / NMRDataStruct->SWMh);
				IdxOffInt = llround(floor(IdxOffset));
				IdxOffFrac = IdxOffset - IdxOffInt;
				
				Index2 = (long long) j + IdxOffInt;
	
				if ((Index2 >= 0) && ((size_t) (Index2 + 1) < DFTProcIndexRange(NMRDataStruct, Step))) {
					/** Compare Real1 with linear interpolation between the two closest points **/
					Real2 = DFTProcPhaseCorrReal(NMRDataStruct, Step, Index2 + 0);
					Real3 = DFTProcPhaseCorrReal(NMRDataStruct, Step, Index2 + 1);

					if (((1.0 - IdxOffFrac)*Real2 + IdxOffFrac*Real3) > Real1)
						Valid = 0;
				} else 
				if ((Index2 >= 0) && ((size_t) (Index2 + 1) == DFTProcIndexRange(NMRDataStruct, Step))) {
					/** Compare Real1 with the end point at the very same frequency (rare case) **/
					Real2 = DFTProcPhaseCorrReal(NMRDataStruct, Step, Index2);

					if ((IdxOffFrac == 0.0) && (Real2 > Real1)) 
						Valid = 0;
				}
			}

			if (Valid) {
				/** Add the point **/
				ValidPoints++;
				
				if (ValidPoints > BufferLength) {
					BufferLength *= 2;
					AuxPointer = NMRDataStruct->DFTRealEnvelopeArray;
					NMRDataStruct->DFTRealEnvelopeArray = (double *) realloc(NMRDataStruct->DFTRealEnvelopeArray, 2*BufferLength*sizeof(double));
					
					if (NMRDataStruct->DFTRealEnvelopeArray == NULL) {
						NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating envelope point array memory space");
						free(AuxPointer);
						AuxPointer = NULL;
						NMRDataStruct->DFTRealEnvelopeCount = 0;
						
						return (MEM_ALLOC_ERROR | DATA_EMPTY);
					}
				}
				
				DFTRealEnvelopeFreq(NMRDataStruct, ValidPoints - 1) = DFTProcFreq(NMRDataStruct, i, j);
				DFTRealEnvelopeReal(NMRDataStruct, ValidPoints - 1) = Real1;
			}
			
		}
		
	}

	if (ValidPoints == 0) {	/** usually should not happen **/
		FreeDFTRealEnvelope(NMRDataStruct);
		return DATA_OK;
	}

	/** Shrink to appropriate size **/
	NMRDataStruct->DFTRealEnvelopeCount = BufferLength = ValidPoints;
	AuxPointer = (double *) realloc(NMRDataStruct->DFTRealEnvelopeArray, 2*BufferLength*sizeof(double));
	if (AuxPointer != NULL) 
		NMRDataStruct->DFTRealEnvelopeArray = AuxPointer;	/** otherwise the data at NMRDataStruct->DFTRealEnvelopeArray are intact **/

	/** Sort **/
	qsort(NMRDataStruct->DFTRealEnvelopeArray, BufferLength, 2*sizeof(double), CompareDouble);

	return DATA_OK;
}


int FreeDFTRealEnvelope(NMRData *NMRDataStruct) {

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	free(NMRDataStruct->DFTRealEnvelopeArray);
	NMRDataStruct->DFTRealEnvelopeArray = NULL;
	NMRDataStruct->DFTRealEnvelopeCount = 0;

	return DATA_EMPTY;
}



int GetEvaluation(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t i = 0;
	size_t j = 0;
	size_t Start = 0;
	size_t Range = 0;
	int RetVal = DATA_OK;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if (StepNo < 0) {
		Start = 0;
		Range = StepNoRange(NMRDataStruct);
	} else 
	if ((unsigned long) StepNo < StepNoRange(NMRDataStruct)) {
		Start = StepNo;
		Range = StepNo + 1;
	} else 
		return INVALID_PARAMETER;

	if (Components & (Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTAmp) | Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp))) {
		if ((RetVal = CheckProcParam(NMRDataStruct, PROC_PARAM_Filter, PARAM_LONG, NULL, NULL)) != DATA_OK) {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'Filter' check failed", "Evaluation of NMR data");
			return RetVal;
		}
	}

	for (i = Start; i < Range; i++) {
		if (NMRDataStruct->Steps[i].Flags & Flag(CHECK_Evaluation))
			continue;	/** This step is already done **/
		
		if ((Components & Flag(CHECK_Evaluation_ChunkAvgAmp)) && !(NMRDataStruct->Steps[i].Flags & Flag(CHECK_Evaluation_ChunkAvgAmp))) {
			ChunkAvgIntAmp(NMRDataStruct, i) = 0.0;
			ChunkAvgMaxAmp(NMRDataStruct, i) = 0.0;
			for (j = 0; j < ChunkAvgProcIndexRange(NMRDataStruct, i); j++) {
				ChunkAvgIntAmp(NMRDataStruct, i) += ChunkAvgProcAmp(NMRDataStruct, i, j);
				if (ChunkAvgProcAmp(NMRDataStruct, i, j) >  ChunkAvgMaxAmp(NMRDataStruct, i)) 
					ChunkAvgMaxAmp(NMRDataStruct, i) = ChunkAvgProcAmp(NMRDataStruct, i, j);
			}
		}
			
		if ((Components & Flag(CHECK_Evaluation_DFTAmp)) && !(NMRDataStruct->Steps[i].Flags & Flag(CHECK_Evaluation_DFTAmp))) {
			DFTMeanAmp(NMRDataStruct, i) = 0.0;
			DFTMaxAmp(NMRDataStruct, i) = 0.0;
			DFTMaxAmpIndex(NMRDataStruct, i)  = 0;
			for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
				DFTMeanAmp(NMRDataStruct, i) += DFTProcAmp(NMRDataStruct, i, j);
				if (DFTProcAmp(NMRDataStruct, i, j) >  DFTMaxAmp(NMRDataStruct, i)) {
					DFTMaxAmpIndex(NMRDataStruct, i) = DFTIndexToRawIndexWithFilter(NMRDataStruct, i, j);
					DFTMaxAmp(NMRDataStruct, i) = DFTProcAmp(NMRDataStruct, i, j);
				}
				
			}
			DFTMeanAmp(NMRDataStruct, i) /= (double) DFTIndexRange(NMRDataStruct, i);
		}	
			
		if ((Components & Flag(CHECK_Evaluation_DFTPhaseCorrReal)) && !(NMRDataStruct->Steps[i].Flags & Flag(CHECK_Evaluation_DFTPhaseCorrReal))) {
			DFTMaxPhaseCorrReal(NMRDataStruct, i) = 0.0;
			DFTMeanPhaseCorrReal(NMRDataStruct, i) = 0.0;
			DFTMaxPhaseCorrRealIndex(NMRDataStruct, i)  = 0;
			
			for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
				DFTMeanPhaseCorrReal(NMRDataStruct, i) += DFTProcPhaseCorrReal(NMRDataStruct, i, j);
				/** Actually looking for an extreme in absolute value **/
				if (fabs(DFTProcPhaseCorrReal(NMRDataStruct, i, j)) >  fabs(DFTMaxPhaseCorrReal(NMRDataStruct, i))) {
					DFTMaxPhaseCorrRealIndex(NMRDataStruct, i) = DFTIndexToRawIndexWithFilter(NMRDataStruct, i, j);
					DFTMaxPhaseCorrReal(NMRDataStruct, i) = DFTProcPhaseCorrReal(NMRDataStruct, i, j);
				}
			}
			DFTMeanPhaseCorrReal(NMRDataStruct, i) /= (double) DFTIndexRange(NMRDataStruct, i);
		}
			
		if ((Components & Flag(CHECK_Evaluation_DFTPhaseCorrAmp)) && !(NMRDataStruct->Steps[i].Flags & Flag(CHECK_Evaluation_DFTPhaseCorrAmp))) {
			DFTMaxPhaseCorrAmp(NMRDataStruct, i) = 0.0;
			DFTMeanPhaseCorrAmp(NMRDataStruct, i) = 0.0;
			DFTMaxPhaseCorrAmpIndex(NMRDataStruct, i)  = 0;
			
			for (j = 0; j < DFTProcIndexRange(NMRDataStruct, i); j++) {
				DFTMeanPhaseCorrAmp(NMRDataStruct, i) += DFTProcPhaseCorrAmp(NMRDataStruct, i, j);
				if (DFTProcPhaseCorrAmp(NMRDataStruct, i, j) >  DFTMaxPhaseCorrAmp(NMRDataStruct, i)) {
					DFTMaxPhaseCorrAmpIndex(NMRDataStruct, i) = DFTIndexToRawIndexWithFilter(NMRDataStruct, i, j);
					DFTMaxPhaseCorrAmp(NMRDataStruct, i) = DFTProcPhaseCorrAmp(NMRDataStruct, i, j);
				}
			}
			DFTMeanPhaseCorrAmp(NMRDataStruct, i) /= (double) DFTIndexRange(NMRDataStruct, i);
		}
	}
	
	return DATA_OK;
}
