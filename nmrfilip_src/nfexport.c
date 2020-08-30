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
#include <ctype.h>
#include "fftw3.h"

#include "nmrfilip.h"

#include "nfio.h"
#include "nfproc.h"
#include "nfexport.h"


/** Text data export functions **/
/** Note: Do NOT call these functions directly! They shall be called only through DataToText(), 
which performs most of argument and error checks, as well as makes sure 
that necessary data are available and carries out crucial preparations and clean up. **/

int AcquInfoToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;

	if (foutput != NULL) {
		if ((NMRDataStruct->AcquInfo.AcquFlag & ACQU_Acqus) == ACQU_Acqus) {
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "TITLE", &(NMRDataStruct->AcquInfo.Title), PARAM_STRING, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$DATE", &(NMRDataStruct->AcquInfo.Date), PARAM_LONG, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$SFO1", &(NMRDataStruct->AcquInfo.Freq), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$SW_h", &(NMRDataStruct->AcquInfo.SWh), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$PULPROG", &(NMRDataStruct->AcquInfo.PulProg), PARAM_STRING, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$NS", &(NMRDataStruct->AcquInfo.NS), PARAM_LONG, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$RG", &(NMRDataStruct->AcquInfo.RG), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$FW", &(NMRDataStruct->AcquInfo.FW), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "$DE", &(NMRDataStruct->AcquInfo.DE), PARAM_DOUBLE, &RetVal);
			
			WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "$D", NMRDataStruct->AcquInfo.D, NMRDataStruct->AcquInfo.Dlength, PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "$P", NMRDataStruct->AcquInfo.P, NMRDataStruct->AcquInfo.Plength, PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "$PL", NMRDataStruct->AcquInfo.PL, NMRDataStruct->AcquInfo.PLlength, PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "$PLW", NMRDataStruct->AcquInfo.PLW, NMRDataStruct->AcquInfo.PLWlength, PARAM_DOUBLE, &RetVal);
		}
		
		WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueType", &(NMRDataStruct->AcquInfo.AssocValueType), PARAM_ULONG, &RetVal);
		if (NMRDataStruct->AcquInfo.AssocValueTypeName != NULL) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueTypeName", &(NMRDataStruct->AcquInfo.AssocValueTypeName), PARAM_STRING, &RetVal);
		if (NMRDataStruct->AcquInfo.AssocValueVariable != NULL) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueVariable", &(NMRDataStruct->AcquInfo.AssocValueVariable), PARAM_STRING, &RetVal);
		if (NMRDataStruct->AcquInfo.AssocValueUnits != NULL) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueUnits", &(NMRDataStruct->AcquInfo.AssocValueUnits), PARAM_STRING, &RetVal);
		
		if (isfinite(NMRDataStruct->AcquInfo.AssocValueStart) && isfinite(NMRDataStruct->AcquInfo.AssocValueStep) && isfinite(NMRDataStruct->AcquInfo.AssocValueCoef)) {
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueStart", &(NMRDataStruct->AcquInfo.AssocValueStart), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueStep", &(NMRDataStruct->AcquInfo.AssocValueStep), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueCoef", &(NMRDataStruct->AcquInfo.AssocValueCoef), PARAM_DOUBLE, &RetVal);
		}
		
		if ((NMRDataStruct->AcquInfo.AssocValues != NULL) && (NMRDataStruct->AcquInfo.AssocValuesLength > 0))
			WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "%AssocValues", NMRDataStruct->AcquInfo.AssocValues, NMRDataStruct->AcquInfo.AssocValuesLength, PARAM_DOUBLE, &RetVal);
		
		if (isfinite(NMRDataStruct->AcquInfo.AssocValueMin) && isfinite(NMRDataStruct->AcquInfo.AssocValueMax)) {
			//~ WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueEnd", &(NMRDataStruct->AcquInfo.AssocValueEnd), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueMin", &(NMRDataStruct->AcquInfo.AssocValueMin), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueMax", &(NMRDataStruct->AcquInfo.AssocValueMax), PARAM_DOUBLE, &RetVal);
		}
		
		if ((NMRDataStruct->AcquInfo.AcquFlag & ACQU_Userlist) == ACQU_Userlist) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%WobbStep", &(NMRDataStruct->AcquInfo.WobbStep), PARAM_LONG, &RetVal);

		if ((NMRDataStruct->AcquInfo.AcquFlag & ACQU_Counts) == ACQU_Counts) {
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%StepCount", &(NMRDataStruct->AcquInfo.StepCount), PARAM_LONG, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%ChunkCount", &(NMRDataStruct->AcquInfo.ChunkCount), PARAM_LONG, &RetVal);
		}
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		/** no simplified text export implemented **/
		if ((*soutput = malloc(1*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;
	
		*soutput[0] = '\0';
		*slength = 0;
	}
	
	return RetVal;
}


int ProcParamsToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	int RetValW = DATA_OK;
	int RVal = DATA_OK;
	long ProcParam = 0;
	long Step = ALL_STEPS;
	unsigned long *Flags = NULL;
	size_t FlagsCount = 0;
	double *PhaseValues = NULL;
	size_t PhaseValuesCount = 0;
	long *PhaseRefValues = NULL;
	size_t PhaseRefValuesCount = 0;
	size_t i = 0;
	
	if (foutput != NULL) {
		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_FirstChunk, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%FirstChunk", &ProcParam, PARAM_LONG, &RetValW);

		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_LastChunk, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%LastChunk", &ProcParam, PARAM_LONG, &RetValW);

		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_ChunkStart, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%ChunkStart", &ProcParam, PARAM_LONG, &RetValW);

		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_ChunkEnd, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%ChunkEnd", &ProcParam, PARAM_LONG, &RetValW);

		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_DFTLength, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%DFTLength", &ProcParam, PARAM_LONG, &RetValW);

		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_ScaleFirstTDPoint, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%ScaleFirstTDPoint", &ProcParam, PARAM_LONG, &RetValW);

		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_RemoveOffset, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RemoveOffset", &ProcParam, PARAM_LONG, &RetValW);

		RetVal |= RVal = GetProcParam(NMRDataStruct, PROC_PARAM_Filter, PARAM_LONG, &ProcParam, &Step);
		if (RVal == DATA_OK)
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%FilterHz", &ProcParam, PARAM_LONG, &RetValW);
		
		
		if (StepNoRange(NMRDataStruct) > 0) {
			Flags = (unsigned long *) malloc(StepNoRange(NMRDataStruct) * sizeof(unsigned long));
			if (Flags == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating auxiliary memory space during exporting processing parameters");
				RetVal |= MEM_ALLOC_ERROR;
			} else {
				FlagsCount = StepNoRange(NMRDataStruct);

				/** Export particular step flags **/
				for (i = 0; i < FlagsCount; i++)
					Flags[i] = StepFlag(NMRDataStruct, i);

				WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "%StepFlags", Flags, FlagsCount, PARAM_ULONG, &RetValW);
				
				/** Export phase correction flags **/
				for (i = 0; i < FlagsCount; i++)
					Flags[i] = DFTPhaseCorrFlag(NMRDataStruct, i);
				
				WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "%PhaseCorrFlags", Flags, FlagsCount, PARAM_ULONG, &RetValW);
				
				free(Flags);
				Flags = NULL;
				FlagsCount = 0;
			}
			
			/** Export also phase correction values **/
			PhaseValues = (double *) malloc(StepNoRange(NMRDataStruct) * sizeof(double));
			if (PhaseValues == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating auxiliary memory space during exporting processing parameters");
				RetVal |= MEM_ALLOC_ERROR;
			} else {
				PhaseValuesCount = StepNoRange(NMRDataStruct);
				
				for (i = 0; i < PhaseValuesCount; i++)
					PhaseValues[i] = 0.001*DFTPhaseCorr0(NMRDataStruct, i);
				
				WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "%PhaseCorr0", PhaseValues, PhaseValuesCount, PARAM_DOUBLE, &RetValW);
				
				for (i = 0; i < PhaseValuesCount; i++)
					PhaseValues[i] = 0.001*DFTPhaseCorr1(NMRDataStruct, i);
				
				WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "%PhaseCorr1", PhaseValues, PhaseValuesCount, PARAM_DOUBLE, &RetValW);
				
				free(PhaseValues);
				PhaseValues = NULL;
				PhaseValuesCount = 0;
			}
			
			PhaseRefValues = (long *) malloc(StepNoRange(NMRDataStruct) * sizeof(long));
			if (PhaseRefValues == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating auxiliary memory space during exporting processing parameters");
				RetVal |= MEM_ALLOC_ERROR;
			} else {
				PhaseRefValuesCount = StepNoRange(NMRDataStruct);
				
				for (i = 0; i < PhaseRefValuesCount; i++)
					PhaseRefValues[i] = DFTPhaseCorr1Ref(NMRDataStruct, i);
				
				WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "%PhaseCorr1Ref", PhaseRefValues, PhaseRefValuesCount, PARAM_LONG, &RetValW);
				
				free(PhaseRefValues);
				PhaseRefValues = NULL;
				PhaseRefValuesCount = 0;
			}
		}
		
		RetVal |= RetValW;
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		/** no simplified text export implemented **/
		if ((*soutput = malloc(1*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;
	
		*soutput[0] = '\0';
		*slength = 0;
	}
	
	return RetVal;
}


#define AssocUnits(NMRDataPtr)		(((NMRDataPtr)->AcquInfo.AssocValueUnits)?((NMRDataPtr)->AcquInfo.AssocValueUnits):(""))
#define AssocVariable(NMRDataPtr)		(((NMRDataPtr)->AcquInfo.AssocValueVariable)?((NMRDataPtr)->AcquInfo.AssocValueVariable):(""))

int TDDToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Time domain data";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 4 + 7+20+3+21+1+strlen(AssocUnits(NMRDataStruct))+2 + 22;
	const size_t rowlen = 22+2*11+4;
	const size_t buflen = StepNoRange(NMRDataStruct)*(TDDIndexRange(NMRDataStruct, 0)*rowlen + headlen) + titlelen + 1;
	size_t i = 0, j = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;

	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n", title);
		ferr = ferr || (written < 0);

		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Step\tPoint\tTime [us]\tReal\tImag\tFlag\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			ferr = ferr || (written < 0);
			
			for (j = 0; (j < TDDIndexRange(NMRDataStruct, i)) && !ferr; j++) {
				written = fprintf(foutput, "%" PRIu64 "\t%" PRIu64 "\t%.15g\t%" PRIi32 "\t%" PRIi32 "\t%lu\n", 
					(uint64_t) i, (uint64_t) j, 
					TDDTime(NMRDataStruct, i, j), 
					TDDReal(NMRDataStruct, i, j), 
					TDDImag(NMRDataStruct, i, j), 
					StepFlag(NMRDataStruct, i));
				ferr = ferr || (written < 0);
			}
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + 1, "# %s\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > titlelen));
		
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, headlen + 1, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Time [us]\tReal\tImag\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			serr = serr || ((written < 0) || (((size_t) written) > headlen));
			
			for (j = 0; (j < TDDIndexRange(NMRDataStruct, i)) && !serr; j++) {
				s += written = snprintf(s, rowlen + 1, "%.15g\t%" PRIi32 "\t%" PRIi32 "\n", 
					TDDTime(NMRDataStruct, i, j), 
					TDDReal(NMRDataStruct, i, j), 
					TDDImag(NMRDataStruct, i, j));
				serr = serr || ((written < 0) || (((size_t) written) > rowlen));
			}
		}
		
		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}
	
	return RetVal;
}


int ChunkSetToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Chunk listing";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 35;
	const size_t rowlen = 4*20+5;
	const size_t buflen = ChunkNoRange(NMRDataStruct)*rowlen + headlen + titlelen + 1;
	size_t i = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;

	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n"  "#Chunk\tStartpoint\tEndpoint\tLength\n", title);
		ferr = ferr || (written < 0);
		
		for (i = 0; (i < ChunkNoRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n", 
				(uint64_t) i, 
				(uint64_t) ChunkDataStart(NMRDataStruct, i)/2, 
				(uint64_t) (ChunkDataStart(NMRDataStruct, i)/2 + ChunkIndexRange(NMRDataStruct, i) - 1), 
				(uint64_t) ChunkIndexRange(NMRDataStruct, i));
			ferr = ferr || (written < 0);
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + headlen + 1, "# %s\n"  "#Chunk\tStartpoint\tEndpoint\tLength\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > (titlelen + headlen)));
		
		for (i = 0; (i < ChunkNoRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, rowlen + 1, "%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n", 
				(uint64_t) i, 
				(uint64_t) ChunkDataStart(NMRDataStruct, i)/2, 
				(uint64_t) (ChunkDataStart(NMRDataStruct, i)/2 + ChunkIndexRange(NMRDataStruct, i) - 1), 
				(uint64_t) ChunkIndexRange(NMRDataStruct, i));
			serr = serr || ((written < 0) || (((size_t) written) > rowlen));
		}
		
		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}
	
	return RetVal;
}


int ChunkAvgToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Chunk average data";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 4 + 7+20+3+21+1+strlen(AssocUnits(NMRDataStruct))+2 + 30;
	const size_t rowlen = 4*22+5;
	const size_t buflen = StepNoRange(NMRDataStruct)*(ChunkAvgIndexRange(NMRDataStruct, 0)*rowlen + headlen) + titlelen + 1;
	size_t i = 0, j = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;

	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n", title);
		ferr = ferr || (written < 0);

		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Step\tPoint\tTime [us]\tReal\tImag\tModulus\tProcessed\tFlag\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			ferr = ferr || (written < 0);
			
			for (j = 0; (j < ChunkAvgIndexRange(NMRDataStruct, i)) && !ferr; j++) {
				written = fprintf(foutput, "%" PRIu64 "\t%" PRIu64 "\t%.15g\t%.15g\t%.15g\t%.15g\t%u\t%lu\n", 
					(uint64_t) i, (uint64_t) j, 
					ChunkAvgTime(NMRDataStruct, i, j), 
					ChunkAvgReal(NMRDataStruct, i, j), 
					ChunkAvgImag(NMRDataStruct, i, j), 
					ChunkAvgAmp(NMRDataStruct, i, j), 
					(unsigned int) ChunkAvgIsProc(NMRDataStruct, i, j), 
					StepFlag(NMRDataStruct, i));
				ferr = ferr || (written < 0);
			}
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + 1, "# %s\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > titlelen));
		
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, headlen + 1, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Time [us]\tReal\tImag\tModulus\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			serr = serr || ((written < 0) || (((size_t) written) > headlen));
			
			for (j = 0; (j < ChunkAvgIndexRange(NMRDataStruct, i)) && !serr; j++) {
				s += written = snprintf(s, rowlen + 1, "%.15g\t%.15g\t%.15g\t%.15g\n", 
					ChunkAvgTime(NMRDataStruct, i, j), 
					ChunkAvgReal(NMRDataStruct, i, j), 
					ChunkAvgImag(NMRDataStruct, i, j), 
					ChunkAvgAmp(NMRDataStruct, i, j));
				serr = serr || ((written < 0) || (((size_t) written) > rowlen));
			}
		}
		
		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}	
	
	return RetVal;
}


int DFTResultToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Fourier transform output data";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 4 + 7+20+3+21+1+strlen(AssocUnits(NMRDataStruct))+2 + 36;
	const size_t rowlen = 4*22+5;
	const size_t buflen = StepNoRange(NMRDataStruct)*(DFTProcNoFilterIndexRange(NMRDataStruct, 0)*rowlen + headlen) + titlelen + 1;
	size_t i = 0, j = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;

	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n", title);
		ferr = ferr || (written < 0);

		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Step\tPoint\tFrequency [MHz]\tReal\tImag\tModulus\tProcessed\tFlag\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			ferr = ferr || (written < 0);
		
			for (j = 0; (j < DFTProcNoFilterIndexRange(NMRDataStruct, i)) && !ferr; j++) {
				written = fprintf(foutput, "%" PRIu64 "\t%" PRIu64 "\t%.15g\t%.15g\t%.15g\t%.15g\t%u\t%lu\n", 
					(uint64_t) i, (uint64_t) j, 
					DFTProcNoFilterFreq(NMRDataStruct, i, j),
					DFTProcNoFilterReal(NMRDataStruct, i, j),
					DFTProcNoFilterImag(NMRDataStruct, i, j),
					DFTProcNoFilterAmp(NMRDataStruct, i, j),
					(unsigned int) DFTIsProcNoFilter(NMRDataStruct, i, j),
					StepFlag(NMRDataStruct, i));
				ferr = ferr || (written < 0);
			}
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + 1, "# %s\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > titlelen));
		
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, headlen + 1, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Frequency [MHz]\tReal\tImag\tModulus\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			serr = serr || ((written < 0) || (((size_t) written) > headlen));
			
			for (j = 0; (j < DFTProcNoFilterIndexRange(NMRDataStruct, i)) && !serr; j++) {
				s += written = snprintf(s, rowlen + 1, "%.15g\t%.15g\t%.15g\t%.15g\n", 
					DFTProcNoFilterFreq(NMRDataStruct, i, j), 
					DFTProcNoFilterReal(NMRDataStruct, i, j), 
					DFTProcNoFilterImag(NMRDataStruct, i, j), 
					DFTProcNoFilterAmp(NMRDataStruct, i, j));
				serr = serr || ((written < 0) || (((size_t) written) > rowlen));
			}
		}

		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}
	
	return RetVal;
}


int DFTPhaseCorrectedResultToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Phase corrected Fourier transform output data";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 4 + 7+20+3+21+1+strlen(AssocUnits(NMRDataStruct))+2 + 36;
	const size_t rowlen = 4*22+5;
	const size_t buflen = StepNoRange(NMRDataStruct)*(DFTProcNoFilterIndexRange(NMRDataStruct, 0)*rowlen + headlen) + titlelen + 1;
	size_t i = 0, j = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;

	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n", title);
		ferr = ferr || (written < 0);

		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Step\tPoint\tFrequency [MHz]\tReal\tImag\tModulus\tProcessed\tFlag\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			ferr = ferr || (written < 0);
		
			for (j = 0; (j < DFTProcNoFilterIndexRange(NMRDataStruct, i)) && !ferr; j++) {
				written = fprintf(foutput, "%" PRIu64 "\t%" PRIu64 "\t%.15g\t%.15g\t%.15g\t%.15g\t%u\t%lu\n", 
					(uint64_t) i, (uint64_t) j, 
					DFTProcNoFilterFreq(NMRDataStruct, i, j),
					DFTProcNoFilterPhaseCorrReal(NMRDataStruct, i, j),
					DFTProcNoFilterPhaseCorrImag(NMRDataStruct, i, j),
					DFTProcNoFilterPhaseCorrAmp(NMRDataStruct, i, j),
					(unsigned int) DFTIsProcNoFilter(NMRDataStruct, i, j),
					StepFlag(NMRDataStruct, i));
				ferr = ferr || (written < 0);
			}
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + 1, "# %s\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > titlelen));
		
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, headlen + 1, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Frequency [MHz]\tReal\tImag\tModulus\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			serr = serr || ((written < 0) || (((size_t) written) > headlen));
			
			for (j = 0; (j < DFTProcNoFilterIndexRange(NMRDataStruct, i)) && !serr; j++) {
				s += written = snprintf(s, rowlen + 1, "%.15g\t%.15g\t%.15g\t%.15g\n", 
					DFTProcNoFilterFreq(NMRDataStruct, i, j), 
					DFTProcNoFilterPhaseCorrReal(NMRDataStruct, i, j), 
					DFTProcNoFilterPhaseCorrImag(NMRDataStruct, i, j), 
					DFTProcNoFilterPhaseCorrAmp(NMRDataStruct, i, j));
				serr = serr || ((written < 0) || (((size_t) written) > rowlen));
			}
		}

		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}
	
	return RetVal;
}


int DFTEnvelopeToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Spectrum - envelope of Fourier transform moduli";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 28;
	const size_t rowlen = 2*22+3;
	const size_t buflen = DFTEnvelopeIndexRange(NMRDataStruct)*rowlen + headlen + titlelen + 1;
	size_t i = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;
	
	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n"  "#Frequency [MHz]\tIntensity\n", title);
		ferr = ferr || (written < 0);

		for (i = 0; (i < DFTEnvelopeIndexRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%.15g\t%.15g\n", DFTEnvelopeFreq(NMRDataStruct, i), DFTEnvelopeAmp(NMRDataStruct, i));
			ferr = ferr || (written < 0);
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + headlen + 1, "# %s\n"  "#Frequency [MHz]\tIntensity\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > (titlelen + headlen)));
		
		for (i = 0; (i < DFTEnvelopeIndexRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, rowlen + 1, "%.15g\t%.15g\n", DFTEnvelopeFreq(NMRDataStruct, i), DFTEnvelopeAmp(NMRDataStruct, i));
			serr = serr || ((written < 0) || (((size_t) written) > rowlen));
		}
		
		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}
	
	return RetVal;
}
	

int DFTPhaseCorrRealEnvelopeToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Spectrum - envelope of Fourier transform real parts";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 28;
	const size_t rowlen = 2*22+3;
	const size_t buflen = DFTRealEnvelopeIndexRange(NMRDataStruct)*rowlen + headlen + titlelen + 1;
	size_t i = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;
	
	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n"  "#Frequency [MHz]\tIntensity\n", title);
		ferr = ferr || (written < 0);

		for (i = 0; (i < DFTRealEnvelopeIndexRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%.15g\t%.15g\n", DFTRealEnvelopeFreq(NMRDataStruct, i), DFTRealEnvelopeReal(NMRDataStruct, i));
			ferr = ferr || (written < 0);
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + headlen + 1, "# %s\n"  "#Frequency [MHz]\tIntensity\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > (titlelen + headlen)));
		
		for (i = 0; (i < DFTRealEnvelopeIndexRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, rowlen + 1, "%.15g\t%.15g\n", DFTRealEnvelopeFreq(NMRDataStruct, i), DFTRealEnvelopeReal(NMRDataStruct, i));
			serr = serr || ((written < 0) || (((size_t) written) > rowlen));
		}
		
		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}
	
	return RetVal;
}


int EvaluationToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Chunk average and Fourier transform evaluation";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 1 + strlen(AssocVariable(NMRDataStruct))+8 + strlen(AssocUnits(NMRDataStruct))+3 + 139;
	const size_t rowlen = 11*22+12;
	const size_t buflen = StepNoRange(NMRDataStruct)*rowlen + headlen + titlelen + 1;
	size_t i = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;

	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n"  
			"#Step\t%c%s%s%s%s\t"
			"ChunkAvg_amp_max\tChunkAvg_amp_integral\tChunkAvg_flag\t"
			"FT0_amp\tFT_amp_max_Freq\tFT_amp_max\tFT_amp_mean\t"
			"FT0_real\tFT_real_max_Freq\tFT_real_max\tFT_real_mean\tFlag\n", 
			title, 
			((strlen(AssocVariable(NMRDataStruct)) > 0)?(toupper(NMRDataStruct->AcquInfo.AssocValueVariable[0])):((int) 'V')), 
			((strlen(AssocVariable(NMRDataStruct)) > 0)?(&(NMRDataStruct->AcquInfo.AssocValueVariable[1])):("ariable")), 
			((strlen(AssocUnits(NMRDataStruct)) > 0)?(" ["):("")), AssocUnits(NMRDataStruct), ((strlen(AssocUnits(NMRDataStruct)) > 0)?("]"):("")) );
		ferr = ferr || (written < 0);
		
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%" PRIu64 "\t%.15g\t%.15g\t%.15g\t%lu\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%lu\n", 
				(uint64_t) i, StepAssocValue(NMRDataStruct, i), 
				ChunkAvgMaxAmp(NMRDataStruct, i), ChunkAvgIntAmp(NMRDataStruct, i), StepFlag(NMRDataStruct, i), 
				DFTPhaseCorrAmpAtZero(NMRDataStruct, i), DFTFreq(NMRDataStruct, i, DFTMaxPhaseCorrAmpIndex(NMRDataStruct, i)),
				DFTMaxPhaseCorrAmp(NMRDataStruct, i), DFTMeanPhaseCorrAmp(NMRDataStruct, i), 
				DFTPhaseCorrRealAtZero(NMRDataStruct, i), DFTFreq(NMRDataStruct, i, DFTMaxPhaseCorrRealIndex(NMRDataStruct, i)),
				DFTMaxPhaseCorrReal(NMRDataStruct, i), DFTMeanPhaseCorrReal(NMRDataStruct, i), StepFlag(NMRDataStruct, i));
			ferr = ferr || (written < 0);
		}			
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + headlen + 1, "# %s\n"  
			"#%c%s%s%s%s\t" 
			"ChunkAvg_amp_max\tChunkAvg_amp_integral\t" 
			"FT0_amp\tFT_amp_max_Freq\tFT_amp_max\tFT_amp_mean\t" 
			"FT0_real\tFT_real_max_Freq\tFT_real_max\tFT_real_mean\n", 
			title, 
			((strlen(AssocVariable(NMRDataStruct)) > 0)?(toupper(NMRDataStruct->AcquInfo.AssocValueVariable[0])):((int) 'V')), 
			((strlen(AssocVariable(NMRDataStruct)) > 0)?(&(NMRDataStruct->AcquInfo.AssocValueVariable[1])):("ariable")), 
			((strlen(AssocUnits(NMRDataStruct)) > 0)?(" ["):("")), AssocUnits(NMRDataStruct), ((strlen(AssocUnits(NMRDataStruct)) > 0)?("]"):("")) );
		serr = serr || ((written < 0) || (((size_t) written) > (titlelen + headlen)));
		
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, rowlen + 1, "%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\t%.15g\n", 
				StepAssocValue(NMRDataStruct, i), 
				ChunkAvgMaxAmp(NMRDataStruct, i), ChunkAvgIntAmp(NMRDataStruct, i), 
				DFTPhaseCorrAmpAtZero(NMRDataStruct, i), DFTFreq(NMRDataStruct, i, DFTMaxPhaseCorrAmpIndex(NMRDataStruct, i)),
				DFTMaxPhaseCorrAmp(NMRDataStruct, i), DFTMeanPhaseCorrAmp(NMRDataStruct, i), 
				DFTPhaseCorrRealAtZero(NMRDataStruct, i), DFTFreq(NMRDataStruct, i, DFTMaxPhaseCorrRealIndex(NMRDataStruct, i)),
				DFTMaxPhaseCorrReal(NMRDataStruct, i), DFTMeanPhaseCorrReal(NMRDataStruct, i) );
			serr = serr || ((written < 0) || (((size_t) written) > rowlen));
		}
		
		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}

	return RetVal;
}


int EchoPeaksEnvelopeToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength) {
	int RetVal = DATA_OK;
	char *title = "Echo peak moduli envelope";
	const size_t titlelen = 2+strlen(title)+2;
	const size_t headlen = 4 + 7+20+3+21+1+strlen(AssocUnits(NMRDataStruct))+2 + 22;
	const size_t rowlen = 2*22+3;
	const size_t buflen = StepNoRange(NMRDataStruct)*(EchoPeaksEnvelopeIndexRange(NMRDataStruct, 0)*rowlen + headlen) + titlelen + 1;
	size_t i = 0, j = 0;
	int ferr = 0, serr = 0, written = 0;
	char *s = NULL;
	
	if (foutput != NULL) {
		written = fprintf(foutput, "# %s\n", title);
		ferr = ferr || (written < 0);

		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !ferr; i++) {
			written = fprintf(foutput, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Step\t%c%s%s%s%s\tTime [us]\tAmpl\tFlag\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct), 
				((strlen(AssocVariable(NMRDataStruct)) > 0)?(toupper(NMRDataStruct->AcquInfo.AssocValueVariable[0])):((int) 'V')), 
				((strlen(AssocVariable(NMRDataStruct)) > 0)?(&(NMRDataStruct->AcquInfo.AssocValueVariable[1])):("ariable")), 
				((strlen(AssocUnits(NMRDataStruct)) > 0)?(" ["):("")), AssocUnits(NMRDataStruct), ((strlen(AssocUnits(NMRDataStruct)) > 0)?("]"):("")) );
			ferr = ferr || (written < 0);

			for (j = 0; (j < EchoPeaksEnvelopeIndexRange(NMRDataStruct, i)) && !ferr; j++) {
				written = fprintf(foutput, "%" PRIu64 "\t%.15g\t%.15g\t%.15g\t%lu\n", 
					(uint64_t) i, StepAssocValue(NMRDataStruct, i),
					EchoPeaksEnvelopeTime(NMRDataStruct, i, j), EchoPeaksEnvelopeAmp(NMRDataStruct, i, j), 
					StepFlag(NMRDataStruct, i));
				ferr = ferr || (written < 0);
			}
		}
		
		RetVal |= ((ferr)?(FILE_IO_ERROR):(0));
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((s = *soutput = malloc(buflen*sizeof(char))) == NULL) 
			return RetVal | MEM_ALLOC_ERROR | DATA_VOID;

		s += written = snprintf(s, titlelen + 1, "# %s\n", title);
		serr = serr || ((written < 0) || (((size_t) written) > titlelen));
		
		for (i = 0; (i < StepNoRange(NMRDataStruct)) && !serr; i++) {
			s += written = snprintf(s, headlen + 1, "%s# Step %" PRIu64 " : %.14g %s\n"  "#Time [us]\tAmplitude\n", 
				((i > 0)?("\n\n"):("")), (uint64_t) i, StepAssocValue(NMRDataStruct, i), AssocUnits(NMRDataStruct));
			serr = serr || ((written < 0) || (((size_t) written) > headlen));
			
			for (j = 0; (j < EchoPeaksEnvelopeIndexRange(NMRDataStruct, i)) && !serr; j++) {
				s += written = snprintf(s, rowlen + 1, "%.15g\t%.15g\n", EchoPeaksEnvelopeTime(NMRDataStruct, i, j), EchoPeaksEnvelopeAmp(NMRDataStruct, i, j));
				serr = serr || ((written < 0) || (((size_t) written) > rowlen));
			}
		}
		
		if (!serr) {
			*s = '\0';
			*slength = s - *soutput;
		}
		
		RetVal |= ((serr)?(DATA_VOID):(0));
	}
	
	return RetVal;
}


#undef AssocUnits
#undef AssocVariable
