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
#include "nfload.h"
#include "nfproc.h"
#include "nfexport.h"


typedef int (*NMRProcFunc)(NMRData *, long, unsigned long);

typedef struct {
	NMRProcFunc method;
	unsigned short collective;
	unsigned short requires;
	unsigned long components;
	unsigned long enables;
} NMRDataRelation;

/** component entries allow for efficiency improvements by fine-grained access **/
const NMRDataRelation NMRDataRelations[22] = {
	/** CHECK_AcquParams **/
	{&GetAcquParams, 1, CHECK_AcquParams, Flag(CHECK_AcquParams), Flag(CHECK_AcquParams) | 
		Flag(CHECK_RawData) | Flag(CHECK_StepSet) | Flag(CHECK_ChunkSet) | 
		Flag(CHECK_ChunkAvg) | Flag(CHECK_DFTResult) | 
		Flag(CHECK_DFTPhaseCorrPrep) | Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep_MemAmp) | 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTAmp) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp) | 
		Flag(CHECK_EchoPeaksEnvelope) | Flag(CHECK_AcquInfo)},
	/** CHECK_RawData **/
	{&GetRawData, 1, CHECK_AcquParams, Flag(CHECK_RawData), Flag(CHECK_RawData) | 
		Flag(CHECK_StepSet) | Flag(CHECK_ChunkSet) | Flag(CHECK_ChunkAvg) | 
		Flag(CHECK_DFTResult) | 
		Flag(CHECK_DFTPhaseCorrPrep) | Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep_MemAmp) | 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTAmp) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp) | 
		Flag(CHECK_EchoPeaksEnvelope) | Flag(CHECK_AcquInfo)},
	/** CHECK_StepSet **/
	{&GetStepSet, 1, CHECK_RawData, Flag(CHECK_StepSet), Flag(CHECK_StepSet) | 
		Flag(CHECK_ChunkSet) | Flag(CHECK_ChunkAvg) | Flag(CHECK_DFTResult) | 
		Flag(CHECK_DFTPhaseCorrPrep) | Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep_MemAmp) | 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTAmp) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp) | 
		Flag(CHECK_EchoPeaksEnvelope) | Flag(CHECK_AcquInfo)},
	/** CHECK_ChunkSet **/
	{&GetChunkSet, 1, CHECK_StepSet, Flag(CHECK_ChunkSet), Flag(CHECK_ChunkSet) | 
		Flag(CHECK_ChunkAvg) | Flag(CHECK_DFTResult) | 
		Flag(CHECK_DFTPhaseCorrPrep) | Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep_MemAmp) | 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTAmp) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp) | 
		Flag(CHECK_EchoPeaksEnvelope) | Flag(CHECK_AcquInfo)},
	/** CHECK_ChunkAvg **/
	{&GetChunkAvg, 0, CHECK_ChunkSet, Flag(CHECK_ChunkAvg), Flag(CHECK_ChunkAvg) | 
		Flag(CHECK_DFTResult) | 
		Flag(CHECK_DFTPhaseCorrPrep) | Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep_MemAmp) | 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTAmp) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp)}, 
	/** CHECK_DFTResult **/
	{&GetDFTResult, 1, CHECK_ChunkAvg, Flag(CHECK_DFTResult), Flag(CHECK_DFTResult) | 
		Flag(CHECK_DFTPhaseCorrPrep) | Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep_MemAmp) | 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTAmp) | Flag(CHECK_Evaluation_DFTPhaseCorrReal) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrAmp)}, 
	/** CHECK_DFTPhaseCorrPrep **/
	{&GetDFTPhaseCorrPrep, 1, CHECK_DFTResult, Flag(CHECK_DFTPhaseCorrPrep) | 
		Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemAmp), 
		Flag(CHECK_DFTPhaseCorrPrep) | Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | 
		Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep_MemAmp) |
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp)}, 
		/** component CHECK_DFTPhaseCorrPrep_AutoCorr **/
		{&GetDFTPhaseCorrPrep, 1, CHECK_DFTResult, Flag(CHECK_DFTPhaseCorrPrep_AutoCorr), 
			Flag(CHECK_DFTPhaseCorrPrep_AutoCorr) | Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | 
			Flag(CHECK_DFTPhaseCorrPrep) | 
			Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTRealEnvelope) | 
			Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTPhaseCorrReal)}, 
		/** component CHECK_DFTPhaseCorrPrep_MemReIm **/
		{&GetDFTPhaseCorrPrep, 1, CHECK_DFTPhaseCorrPrep_AutoCorr, Flag(CHECK_DFTPhaseCorrPrep_MemReIm), 
			Flag(CHECK_DFTPhaseCorrPrep_MemReIm) | Flag(CHECK_DFTPhaseCorrPrep) | 
			Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | 
			Flag(CHECK_DFTRealEnvelope) | 
			Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTPhaseCorrReal)}, 
		/** component CHECK_DFTPhaseCorrPrep_MemAmp **/
		{&GetDFTPhaseCorrPrep, 1, CHECK_DFTResult, Flag(CHECK_DFTPhaseCorrPrep_MemAmp), 
			Flag(CHECK_DFTPhaseCorrPrep_MemAmp) | Flag(CHECK_DFTPhaseCorrPrep) | 
			Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_Amp) | 
			Flag(CHECK_DFTEnvelope) | 
			Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp)}, 
	/** CHECK_DFTPhaseCorr **/
	{&GetDFTPhaseCorr, 0, CHECK_DFTPhaseCorrPrep, 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp), 
		Flag(CHECK_DFTPhaseCorr) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | 
		Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope) | 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp)}, 
		/** component CHECK_DFTPhaseCorr_ReIm **/
		{&GetDFTPhaseCorr, 0, CHECK_DFTPhaseCorrPrep, Flag(CHECK_DFTPhaseCorr_ReIm), 
			Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr) | 
			Flag(CHECK_DFTRealEnvelope) | Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTPhaseCorrReal)}, 
		/** component CHECK_DFTPhaseCorr_Amp **/
		{&GetDFTPhaseCorr, 0, CHECK_DFTPhaseCorr_ReIm, Flag(CHECK_DFTPhaseCorr_Amp), 
			Flag(CHECK_DFTPhaseCorr_Amp) | Flag(CHECK_DFTPhaseCorr) | 
			Flag(CHECK_DFTEnvelope) | Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp)}, 
	/** CHECK_AcquInfo **/
	{&GetAcquInfo, 1, CHECK_ChunkSet, Flag(CHECK_AcquInfo), Flag(CHECK_AcquInfo)}, 
	/** CHECK_EchoPeaksEnvelope **/
	{&GetEchoPeaksEnvelope, 0, CHECK_ChunkSet, Flag(CHECK_EchoPeaksEnvelope), Flag(CHECK_EchoPeaksEnvelope)}, 
	/** CHECK_DFTEnvelope **/
	{&GetDFTEnvelope, 1, CHECK_DFTPhaseCorr, Flag(CHECK_DFTEnvelope), Flag(CHECK_DFTEnvelope)}, 
	/** CHECK_DFTRealEnvelope **/
	{&GetDFTRealEnvelope, 1, CHECK_DFTPhaseCorr, Flag(CHECK_DFTRealEnvelope), Flag(CHECK_DFTRealEnvelope)}, 
	/** CHECK_Evaluation **/
	{&GetEvaluation, 0, CHECK_DFTPhaseCorr, Flag(CHECK_Evaluation) | 
		Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTAmp) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp), 
		Flag(CHECK_Evaluation) | Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTAmp) | 
		Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp)},
		/** component CHECK_Evaluation_ChunkAvgAmp **/
		{&GetEvaluation, 0, CHECK_ChunkAvg, Flag(CHECK_Evaluation_ChunkAvgAmp), 
			Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation)},
		/** component CHECK_Evaluation_DFTAmp **/
		{&GetEvaluation, 0, CHECK_DFTResult, Flag(CHECK_Evaluation_DFTAmp), 
			Flag(CHECK_Evaluation_DFTAmp) | Flag(CHECK_Evaluation)},
		/** component CHECK_Evaluation_DFTPhaseCorrReal **/
		{&GetEvaluation, 0, CHECK_DFTPhaseCorr_ReIm, Flag(CHECK_Evaluation_DFTPhaseCorrReal), 
			Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation)},
		/** component CHECK_Evaluation_DFTPhaseCorrAmp **/
		{&GetEvaluation, 0, CHECK_DFTPhaseCorr_Amp, Flag(CHECK_Evaluation_DFTPhaseCorrAmp), 
			Flag(CHECK_Evaluation_DFTPhaseCorrAmp) | Flag(CHECK_Evaluation)}
};


/** Populates the NMRData structure with reasonable initial values **/
EXPORT int InitNMRData(NMRData *NMRDataStruct) {

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	NMRDataStruct->AcqusData = NULL;
	NMRDataStruct->AcqusLength = 0;
	
	NMRDataStruct->SerName = NULL;
	NMRDataStruct->ByteOrder = 0;
	
	NMRDataStruct->DataSpace = NULL;
	NMRDataStruct->DataSize = 0;
	NMRDataStruct->TimeDomain = 0;
	NMRDataStruct->PointLine = 0;
	
	NMRDataStruct->SWMh = 1.0;

	NMRDataStruct->SkipPoints = 0;
	NMRDataStruct->TimeOffset = 0.0;

	NMRDataStruct->FirstChunk = 0;
	NMRDataStruct->LastChunk = INT_MAX;
	NMRDataStruct->ChunkStart = 0;
	NMRDataStruct->ChunkEnd = INT_MAX;
	NMRDataStruct->DFTLength = 128;
	NMRDataStruct->FilterHz = 2000000000;
	NMRDataStruct->filter = 0;
	NMRDataStruct->filter2 = 0;

	NMRDataStruct->ScaleFirstTDPoint = 0;
	NMRDataStruct->RemoveOffset = 0;
	
	NMRDataStruct->Steps = NULL;
	NMRDataStruct->StepCount = 0;
	NMRDataStruct->Flags = 0ul;
	
	NMRDataStruct->ChunkSet = NULL;
	NMRDataStruct->ChunkCount = 0;
	
	NMRDataStruct->DFTEnvelopeArray = NULL;
	NMRDataStruct->DFTEnvelopeCount = 0;
	
	NMRDataStruct->DFTRealEnvelopeArray = NULL;
	NMRDataStruct->DFTRealEnvelopeCount = 0;
	
	InitAcquInfo(NMRDataStruct);
	
	
	/** Application dependent function pointers **/
	NMRDataStruct->ErrorReport = DefErrorReport;
	NMRDataStruct->ErrorReportCustom = DefErrorReportCustom;
	NMRDataStruct->MarkNMRDataOldCallback = NULL;
	NMRDataStruct->ChangeProcParamCallback = NULL;
	
	/** Auxiliary application dependent pointers and values **/
	NMRDataStruct->AuxPointer = NULL;
	NMRDataStruct->AuxLong = 0;
	NMRDataStruct->AuxFlag = 0;
	
	return DATA_OK;
}

/** If requested, marks particular data and all dependent data no longer valid **/
int MarkNMRDataOld(NMRData *NMRDataStruct, unsigned int NMRDataType, long StepNo) {
	size_t i = 0;
	unsigned long Changed = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if (NMRDataType > HighestNMRDataType) 
		return INVALID_PARAMETER;
	
	Changed |= NMRDataStruct->Flags & NMRDataRelations[NMRDataType].enables;
	NMRDataStruct->Flags &= ~NMRDataRelations[NMRDataType].enables;
	
	if (NMRDataStruct->Steps != NULL) {
		if ((StepNo >= 0) && ((size_t) StepNo < NMRDataStruct->StepCount)) {
			Changed |= NMRDataStruct->Steps[StepNo].Flags & NMRDataRelations[NMRDataType].enables;
			NMRDataStruct->Steps[StepNo].Flags &= ~NMRDataRelations[NMRDataType].enables;
		} else {
			for (i = 0; i < NMRDataStruct->StepCount; i++) {
				Changed |= NMRDataStruct->Steps[i].Flags & NMRDataRelations[NMRDataType].enables;
				NMRDataStruct->Steps[i].Flags &= ~NMRDataRelations[NMRDataType].enables;
			}
		}
	}

	if (Changed && (NMRDataStruct->MarkNMRDataOldCallback != NULL))
		NMRDataStruct->MarkNMRDataOldCallback(NMRDataStruct, NMRDataRelations[NMRDataType].enables, StepNo);
		
	return DATA_OK;
}

/** Makes sure that requested data are available, taking care of all prerequisities **/
EXPORT int CheckNMRData(NMRData *NMRDataStruct, unsigned int NMRDataType, long StepNo) {
	size_t i = 0;
	int RetVal = DATA_OK;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if (NMRDataType > HighestNMRDataType) 
		return INVALID_PARAMETER;

	/** Check the flag **/
	if (NMRDataStruct->Flags & Flag(NMRDataType))
		return DATA_OK;
	
	if ((StepNo >= 0) && ((size_t) StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) {
		if (NMRDataStruct->Steps[StepNo].Flags & Flag(NMRDataType))
			return DATA_OK;
	}
	
	if (NMRDataRelations[NMRDataType].method == NULL)
		return DATA_EMPTY;
	
	if ((StepNo < 0) || ((size_t) StepNo >= NMRDataStruct->StepCount) || (NMRDataRelations[NMRDataType].collective)/* || (NMRDataStruct->Steps == NULL)*/) 
		StepNo = ALL_STEPS;
	
	/** Prepare prerequisities **/
	if (NMRDataRelations[NMRDataType].requires != NMRDataType)	/** avoid endless cycles in case of methods without actual prerequisities **/
		if ((RetVal = CheckNMRData(NMRDataStruct, NMRDataRelations[NMRDataType].requires, StepNo)) != DATA_OK)
			return RetVal;

	/** Obtain the data **/
	if ((RetVal = NMRDataRelations[NMRDataType].method(NMRDataStruct, StepNo, NMRDataRelations[NMRDataType].components)) != DATA_OK)
		return RetVal;
	
	/** Set the flag **/
	if (StepNo == ALL_STEPS) {
		NMRDataStruct->Flags |= NMRDataRelations[NMRDataType].components;
	
		if (NMRDataStruct->Steps != NULL) {
			for (i = 0; i < NMRDataStruct->StepCount; i++)
				NMRDataStruct->Steps[i].Flags |= NMRDataRelations[NMRDataType].components;
		}
	} else
	if ((StepNo >= 0) && ((size_t) StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) /** normally should not fail **/
		NMRDataStruct->Steps[StepNo].Flags |= NMRDataRelations[NMRDataType].components;
	
	return RetVal;
}

/** If any data representation is displayed in user application then RefreshNMRData() or ReloadNMRData() function call should be followed by calling CheckNMData() with appropriate parameter and refreshing the data representation. **/
EXPORT int RefreshNMRData(NMRData *NMRDataStruct) {
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	return MarkNMRDataOld(NMRDataStruct, CHECK_AcquParams, ALL_STEPS);
}

/** Reloads the data completely, taking care of checking validity of processing parameters. **/
EXPORT int ReloadNMRData(NMRData *NMRDataStruct) {
	int RetVal = DATA_OK;
	long Val = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	RetVal = RefreshNMRData(NMRDataStruct);
	if (RetVal != DATA_OK)
		return RetVal;
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_FirstChunk, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'FirstChunk' check failed", "Reloading NMR data");
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_LastChunk, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'LastChunk' check failed", "Reloading NMR data");
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_ChunkStart, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'ChunkStart' check failed", "Reloading NMR data");
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_ChunkEnd, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'ChunkEnd' check failed", "Reloading NMR data");
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_DFTLength, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'DFTLength' check failed", "Reloading NMR data");
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_Filter, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'Filter' check failed", "Reloading NMR data");
	
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr0AutoAllTogether, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr0AutoAllTogether' check failed", "Reloading NMR data");
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr0FollowAuto, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr0FollowAuto' check failed", "Reloading NMR data");
	
	if ((RetVal |= CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr1ManualRefDataStart, PARAM_LONG, &Val, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr1ManualRefDataStart' check failed", "Reloading NMR data");
	
	return RetVal;
}

EXPORT int FreeNMRData(NMRData *NMRDataStruct) {
	int RetVal = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	RetVal |= FreeDFTRealEnvelope(NMRDataStruct);
	RetVal |= FreeDFTEnvelope(NMRDataStruct);
	RetVal |= FreeChunkSet(NMRDataStruct);
	RetVal |= FreeStepSet(NMRDataStruct);
	RetVal |= FreeRawData(NMRDataStruct);
	RetVal |= FreeText(NMRDataStruct, &(NMRDataStruct->AcqusData), &(NMRDataStruct->AcqusLength));
	RetVal |= FreeAcquInfo(NMRDataStruct);
	
	return RetVal;
}

/** Should be called on exit of program **/
EXPORT void CleanupOnExit() {
	fftw_cleanup();
}


EXPORT int GetProcParam(NMRData *NMRDataStruct, unsigned int ParamType, unsigned int type, void *ParamValue, long *StepNo) {
	size_t i = 0, j = 0;
	long Val = 0;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if (/*(StepNo == NULL) || */(ParamValue == NULL) || ((type != PARAM_LONG) && (type != PARAM_DOUBLE)))
		return INVALID_PARAMETER;
	
	switch (ParamType) {
		case PROC_PARAM_FirstChunk:
			Val = NMRDataStruct->FirstChunk;
			break;
		case PROC_PARAM_LastChunk:
			Val = NMRDataStruct->LastChunk;
			break;
		case PROC_PARAM_ChunkStart:
			Val = NMRDataStruct->ChunkStart;
			break;
		case PROC_PARAM_ChunkEnd:
			Val = NMRDataStruct->ChunkEnd;
			break;
		case PROC_PARAM_DFTLength:
			Val = NMRDataStruct->DFTLength;
			break;
		case PROC_PARAM_ScaleFirstTDPoint:
			Val = NMRDataStruct->ScaleFirstTDPoint;
			break;
		case PROC_PARAM_RemoveOffset:
			Val = NMRDataStruct->RemoveOffset;
			break;
		case PROC_PARAM_Filter:
			Val = NMRDataStruct->FilterHz;
			break;

		case PROC_PARAM_PhaseCorr0:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) {
				if ((DFTPhaseCorrFlag(NMRDataStruct, *StepNo) & 0x0F) == PHASE0_Manual)
					Val = DFTPhaseCorr0(NMRDataStruct, *StepNo);
				else 
				if (CheckNMRData(NMRDataStruct, CHECK_DFTPhaseCorrPrep, *StepNo) == DATA_OK) 
					Val = DFTPhaseCorr0(NMRDataStruct, *StepNo);
			}
			break;
			
		case PROC_PARAM_PhaseCorr0Auto:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) 
				Val = ((DFTPhaseCorrFlag(NMRDataStruct, *StepNo) & 0x0F) == PHASE0_Auto)?(1):(0);
			break;
			
		case PROC_PARAM_PhaseCorr0AutoAllTogether:
			if (NMRDataStruct->Steps != NULL) {
				for (i = 0; (i < NMRDataStruct->StepCount) && (!Val); i++) 
					if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) == PHASE0_AutoAllTogether) 
						Val = 1;
			}
			break;
		
		case PROC_PARAM_PhaseCorr0FollowAuto:
			if (NMRDataStruct->Steps != NULL) 
				for (i = 0; (i < NMRDataStruct->StepCount) && (!Val); i++) 
					if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) == PHASE0_FollowAuto) {
						for (j = 0; (j < NMRDataStruct->StepCount) && (!Val); j++) 
							if ((DFTPhaseCorrFlag(NMRDataStruct, j) & 0x0F) == PHASE0_Auto) {
								if (StepNo != NULL)
									*StepNo = j;	/** pilot step **/
								Val = 1;
							}
					}
			break;

		case PROC_PARAM_PhaseCorr1ManualRefDataStart:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL) 
				&& (CheckNMRData(NMRDataStruct, CHECK_AcquParams, *StepNo) == DATA_OK)) 
				Val = DFTPhaseCorr1Absolute(NMRDataStruct, *StepNo);
			break;
			
		case PROC_PARAM_PhaseCorr1ManualRefProcStart:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL) 
				&& (CheckNMRData(NMRDataStruct, CHECK_AcquParams, *StepNo) == DATA_OK)) 
				Val = DFTPhaseCorr1Relative(NMRDataStruct, *StepNo);
			break;
			
		case PROC_PARAM_PhaseCorr1Ref:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) 
				Val = DFTPhaseCorr1Ref(NMRDataStruct, *StepNo);
			else
				Val = -1;
			break;
			
		case PROC_PARAM_StepFlag:
		case PROC_PARAM_SetStepFlag:
		case PROC_PARAM_ClearStepFlag:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) 
				Val = StepFlag(NMRDataStruct, *StepNo);
			break;
		
		default:
			return INVALID_PARAMETER;
	}
	
	if (type == PARAM_LONG)
		*((long *) ParamValue) = Val;
	else 
		switch (ParamType) {
			case PROC_PARAM_PhaseCorr0:
				*((double *) ParamValue) = 1.0e-3*Val;	/** deg **/
				break;
			
			case PROC_PARAM_PhaseCorr1ManualRefDataStart:
			case PROC_PARAM_PhaseCorr1ManualRefProcStart:
				*((double *) ParamValue) = 1.0e-3*Val;	/** us **/
				break;
			
			case PROC_PARAM_Filter:
				*((double *) ParamValue) = 1.0e-6*Val;	/** MHz **/
				break;
			
			default:
				*((double *) ParamValue) = Val;
				break;
		}
	
	return DATA_OK;
}


EXPORT int SetProcParam(NMRData *NMRDataStruct, unsigned int ParamType, unsigned int type, void *ParamValue, long *StepNo) {
	size_t MaxChunkLength = 0;
	size_t MinDFTLength = 0;
	size_t i = 0;
	unsigned char Changed = 0;
	unsigned char Changed2 = 0;
	long Val = 0;
	long AuxVal = 0;
	double IntPart = 0.0;
	int RetVal = DATA_OK;
	size_t Start = 0;
	size_t Range = 0;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if (/*(StepNo == NULL) || */(ParamValue == NULL) || ((type != PARAM_LONG) && (type != PARAM_DOUBLE)))
		return INVALID_PARAMETER;

	
	if (ParamType > PROC_PARAM_Filter) {
		if ((RetVal = CheckNMRData(NMRDataStruct, CHECK_StepSet, ALL_STEPS)) != DATA_OK) 
			return RetVal;
		
		if (NMRDataStruct->Steps != NULL) {
			if ((StepNo == NULL) || (*StepNo < 0)) {
				Start = 0;
				Range = StepNoRange(NMRDataStruct);
			} else 
			if ((unsigned long) (*StepNo) < StepNoRange(NMRDataStruct)) {
				Start = *StepNo;
				Range = *StepNo + 1;
			} else 
				return INVALID_PARAMETER;
		}
	}
	
	
	if (type == PARAM_LONG) {
		Val = *((long *) ParamValue);
		
		switch (ParamType) {
			case PROC_PARAM_PhaseCorr0:
				Val %= 360000;
				break;
			
			case PROC_PARAM_PhaseCorr1ManualRefDataStart:
			case PROC_PARAM_PhaseCorr1ManualRefProcStart:
				if (Val > 2000000000)
					Val = 2000000000;
				else
				if (Val < -2000000000)
					Val = -2000000000;
				break;
				
			case PROC_PARAM_Filter:
				if (Val > 2000000000)
					Val = 2000000000;
				else
				if (Val <= 0)
					Val = 1;
				break;
				
			case PROC_PARAM_ScaleFirstTDPoint:
			case PROC_PARAM_RemoveOffset:
				Val = (Val != 0)?(1):(0);
				break;
			
			case PROC_PARAM_PhaseCorr1Ref:
				Val = (Val != 0)?(-1):(0);
				break;
			
			default:
				if (Val < 0)
					Val = 0;
				break;
		}
		
	} else {	/** PARAM_DOUBLE **/
		if (!isfinite(*((double *) ParamValue))) {
			*((double *) ParamValue) = 0.0;
		/*	return INVALID_PARAMETER;	*/
		} 

		switch (ParamType) {
			case PROC_PARAM_PhaseCorr0:
				Val = lround(modf((*((double *) ParamValue))/360.0, &IntPart)*360.0*1.0e3);
			/*	Val = lround(*((double *) ParamValue)*1.0e3);	*/
				break;
			
			case PROC_PARAM_PhaseCorr1ManualRefDataStart:
			case PROC_PARAM_PhaseCorr1ManualRefProcStart:
				if ((*((double *) ParamValue)) > 2.0e6)
					*((double *) ParamValue) = 2.0e6;
				else
				if ((*((double *) ParamValue)) < -2.0e6)
					*((double *) ParamValue) = -2.0e6;
				
				Val = lround(*((double *) ParamValue)*1.0e3);
				break;
			
			case PROC_PARAM_Filter:
				if ((*((double *) ParamValue)) > 2.0e3)
					*((double *) ParamValue) = 2.0e3;
				else
				if ((*((double *) ParamValue)) < 0.0)
					*((double *) ParamValue) = 0.0;
				
				Val = lround(*((double *) ParamValue)*1.0e6);
				if (Val <= 0)
					Val = 1;
				break;
				
			case PROC_PARAM_ScaleFirstTDPoint:
			case PROC_PARAM_RemoveOffset:
				if ((*((double *) ParamValue)) > 2.0e9)
					*((double *) ParamValue) = 2.0e9;
				else
				if ((*((double *) ParamValue)) < -2.0e9)
					*((double *) ParamValue) = -2.0e9;
				
				Val = lround(*((double *) ParamValue));
				Val = (Val != 0)?(1):(0);
				break;
			
			case PROC_PARAM_PhaseCorr1Ref:
				if ((*((double *) ParamValue)) > 2.0e9)
					*((double *) ParamValue) = 2.0e9;
				else
				if ((*((double *) ParamValue)) < -2.0e9)
					*((double *) ParamValue) = -2.0e9;
				
				Val = lround(*((double *) ParamValue));
				Val = (Val != 0)?(-1):(0);
				break;

			default:
				if ((*((double *) ParamValue)) > 2.0e9)
					*((double *) ParamValue) = 2.0e9;
				else
				if ((*((double *) ParamValue)) < -2.0e9)
					*((double *) ParamValue) = -2.0e9;
				
				Val = lround(*((double *) ParamValue));
				if (Val < 0)
					Val = 0;
				break;
		}
	}

		
	switch (ParamType) {
		case PROC_PARAM_FirstChunk:
			if ((RetVal = CheckNMRData(NMRDataStruct, CHECK_ChunkSet, ALL_STEPS)) != DATA_OK) 
				return RetVal;
			
			if ((size_t) Val >= ChunkNoRange(NMRDataStruct)) {
				if (NMRDataStruct->FirstChunk >= ChunkNoRange(NMRDataStruct))
					Val = 0;	/** reasonable default **/
				else
					Val = NMRDataStruct->FirstChunk;	/** revert to original value **/
			}

			if ((unsigned long) Val != NMRDataStruct->FirstChunk) {
				NMRDataStruct->FirstChunk = Val;
				MarkNMRDataOld(NMRDataStruct, CHECK_ChunkAvg, ALL_STEPS);
				Changed = 1;
			}
			
			break;
		
		case PROC_PARAM_LastChunk:
			if ((RetVal = CheckNMRData(NMRDataStruct, CHECK_ChunkSet, ALL_STEPS)) != DATA_OK) 
				return RetVal;
			
			if ((unsigned long) Val < NMRDataStruct->FirstChunk) {
				if (NMRDataStruct->LastChunk < NMRDataStruct->FirstChunk)
					Val = NMRDataStruct->FirstChunk;	/** reasonable adjustment **/
				else
					Val = NMRDataStruct->LastChunk;	/** revert to original value **/
			}
			
			if ((size_t) Val >= ChunkNoRange(NMRDataStruct))
				Val = (ChunkNoRange(NMRDataStruct) > 0)?(ChunkNoRange(NMRDataStruct) - 1):(0);
			
			if ((unsigned long) Val != NMRDataStruct->LastChunk) {
				NMRDataStruct->LastChunk = Val;
				MarkNMRDataOld(NMRDataStruct, CHECK_ChunkAvg, ALL_STEPS);
				Changed = 1;
			}
			
			break;
		
		case PROC_PARAM_ChunkStart:
			if ((RetVal = CheckNMRData(NMRDataStruct, CHECK_ChunkAvg, ALL_STEPS)) != DATA_OK) 
				return RetVal;
			
			MaxChunkLength = (StepNoRange(NMRDataStruct) > 0)?(ChunkAvgIndexRange(NMRDataStruct, 0)):(0);
			
			if ((size_t) Val >= MaxChunkLength) {
				if (NMRDataStruct->ChunkStart >= MaxChunkLength)
					Val = 0;	/** reasonable default **/
				else 
					Val = NMRDataStruct->ChunkStart;	/** revert to original value **/
			}

			if ((unsigned long) Val != NMRDataStruct->ChunkStart) {
				NMRDataStruct->ChunkStart = Val;
				MarkNMRDataOld(NMRDataStruct, CHECK_EchoPeaksEnvelope, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_Evaluation_ChunkAvgAmp, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTResult, ALL_STEPS);
				Changed = 1;
			}

			break;
			
		case PROC_PARAM_ChunkEnd:
			if ((RetVal = CheckNMRData(NMRDataStruct, CHECK_ChunkAvg, ALL_STEPS)) != DATA_OK) 
				return RetVal;
			
			MaxChunkLength = (StepNoRange(NMRDataStruct) > 0)?(ChunkAvgIndexRange(NMRDataStruct, 0)):(0);
			
			if ((unsigned long) Val < NMRDataStruct->ChunkStart) {
				if (NMRDataStruct->ChunkEnd < NMRDataStruct->ChunkStart)
					Val = NMRDataStruct->ChunkStart;	/** reasonable adjustment **/
				else
					Val = NMRDataStruct->ChunkEnd;	/** revert to original value **/
			}
			
			if ((size_t) Val >= MaxChunkLength) 
				Val = (MaxChunkLength > 0)?(MaxChunkLength - 1):(0);
			
			if ((unsigned long) Val != NMRDataStruct->ChunkEnd) {
				NMRDataStruct->ChunkEnd = Val;
				MarkNMRDataOld(NMRDataStruct, CHECK_EchoPeaksEnvelope, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_Evaluation_ChunkAvgAmp, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTResult, ALL_STEPS);
				Changed = 1;
			}
			
			break;		
			
		case PROC_PARAM_DFTLength:
			if ((RetVal = CheckNMRData(NMRDataStruct, CHECK_ChunkAvg, ALL_STEPS)) != DATA_OK) 
				return RetVal;
			
			if (StepNoRange(NMRDataStruct) > 0)
				MinDFTLength = (StepNoRange(NMRDataStruct) > 0)?(ChunkAvgProcIndexRange(NMRDataStruct, 0)):(0);
			
			if ((size_t) Val < MinDFTLength)
				Val = MinDFTLength;
			
			if ((unsigned long) Val != NMRDataStruct->DFTLength) {
				NMRDataStruct->DFTLength = Val;
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTResult, ALL_STEPS);
				Changed = 1;
			}
			
			break;
		
		case PROC_PARAM_ScaleFirstTDPoint:
			if ((NMRDataStruct->ScaleFirstTDPoint != 0) != (Val != 0)) {
				NMRDataStruct->ScaleFirstTDPoint = (Val)?(1):(0);
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTResult, ALL_STEPS);
				Changed = 1;
			} 
			
			if (NMRDataStruct->ScaleFirstTDPoint && NMRDataStruct->RemoveOffset) {
				AuxVal = 0;
				RetVal = SetProcParam(NMRDataStruct, PROC_PARAM_RemoveOffset, PARAM_LONG, &AuxVal, NULL);
			}
			
			break;
		
		case PROC_PARAM_RemoveOffset:
			if ((NMRDataStruct->RemoveOffset != 0) != (Val != 0)) {
				NMRDataStruct->RemoveOffset = (Val)?(1):(0);
				if (NMRDataStruct->RemoveOffset && ((NMRDataStruct->Steps->DFTPhaseCorrOutput == NMRDataStruct->Steps->DFTOutput) || (NMRDataStruct->Steps->DFTPhaseCorrOutAmp == NMRDataStruct->Steps->DFTOutAmp))) {
					/** need to allocate memory for phase corrected and amplitude data **/
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemReIm, ALL_STEPS);
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemAmp, ALL_STEPS);
				} else	
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr, ALL_STEPS);
				
				Changed = 1;
			} 
		
			if (NMRDataStruct->ScaleFirstTDPoint && NMRDataStruct->RemoveOffset) {
				AuxVal = 0;
				RetVal = SetProcParam(NMRDataStruct, PROC_PARAM_ScaleFirstTDPoint, PARAM_LONG, &AuxVal, NULL);
			}
			
			break;

		case PROC_PARAM_Filter:
			if (((unsigned long) Val != NMRDataStruct->FilterHz) || ((unsigned long) DFTFreqToFilter(NMRDataStruct, 1.0e-6*Val) != NMRDataStruct->filter) || ((unsigned long) DFTFreqToFilter2(NMRDataStruct, 1.0e-6*Val) != NMRDataStruct->filter2)) {
				NMRDataStruct->FilterHz = Val;
				NMRDataStruct->filter = DFTFreqToFilter(NMRDataStruct, 1.0e-6*Val);
				NMRDataStruct->filter2 = DFTFreqToFilter2(NMRDataStruct, 1.0e-6*Val);
				
				if ((NMRDataStruct->StepCount > 0) && (NMRDataStruct->Steps != NULL)) {
					for (i = 0; i < NMRDataStruct->StepCount; i++) {
						if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Manual)
							MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, i);
					}
				} else 
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, ALL_STEPS);
				
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTEnvelope, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTRealEnvelope, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_Evaluation_DFTAmp, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_Evaluation_DFTPhaseCorrReal, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_Evaluation_DFTPhaseCorrAmp, ALL_STEPS);
				Changed = 1;
			}

			break;


		case PROC_PARAM_PhaseCorr0Manual:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) {
				for (i = 0; i < NMRDataStruct->StepCount; i++) {
					if (((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Manual) && ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Auto)) 
						DFTPhaseCorrFlag(NMRDataStruct, i) = (DFTPhaseCorrFlag(NMRDataStruct, i) & 0xF0) | PHASE0_Manual;
				}
			}
			
			for (i = Start; i < Range; i++) {
				DFTPhaseCorrFlag(NMRDataStruct, i) = (DFTPhaseCorrFlag(NMRDataStruct, i) & 0xF0) | PHASE0_Manual;
				if (DFTPhaseCorr0(NMRDataStruct, i) != Val) {
					DFTPhaseCorr0(NMRDataStruct, i) = Val;
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr_ReIm, i);
					Changed = 1;
				}
			}
			
			if (Changed && (NMRDataStruct->Steps->DFTPhaseCorrOutput == NMRDataStruct->Steps->DFTOutput))
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemReIm, ALL_STEPS);	/** need to allocate memory for phase corrected data **/
		
			break;
			
		case PROC_PARAM_PhaseCorr0Auto:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) {
				for (i = 0; i < NMRDataStruct->StepCount; i++) {
					if (((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Manual) && ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Auto)) 
						DFTPhaseCorrFlag(NMRDataStruct, i) = (DFTPhaseCorrFlag(NMRDataStruct, i) & 0xF0) | PHASE0_Manual;
				}
			}
				
			for (i = Start; i < Range; i++) {
				if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Auto) {
					DFTPhaseCorrFlag(NMRDataStruct, i) = (DFTPhaseCorrFlag(NMRDataStruct, i) & 0xF0) | PHASE0_Auto;
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, i);
					Changed = 1;
				}
			}
			
			break;
			
		case PROC_PARAM_PhaseCorr0AutoAllTogether:
			if (NMRDataStruct->Steps != NULL) {
				for (i = 0; i < NMRDataStruct->StepCount; i++) {
					Changed |= ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_AutoAllTogether);
					DFTPhaseCorrFlag(NMRDataStruct, i) = (DFTPhaseCorrFlag(NMRDataStruct, i) & 0xF0) | PHASE0_AutoAllTogether;
				}
			}
			
			if (Changed)
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, ALL_STEPS);
				
			break;
		
		case PROC_PARAM_PhaseCorr0FollowAuto:
			if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < NMRDataStruct->StepCount) && (NMRDataStruct->Steps != NULL)) {
				for (i = 0; i < (size_t) *StepNo; i++) {
					Changed |= ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_FollowAuto);
					DFTPhaseCorrFlag(NMRDataStruct, i) = (DFTPhaseCorrFlag(NMRDataStruct, i) & 0xF0) | PHASE0_FollowAuto;
				}
				
				Changed |= ((DFTPhaseCorrFlag(NMRDataStruct, *StepNo) & 0x0F) != PHASE0_Auto);
				DFTPhaseCorrFlag(NMRDataStruct, *StepNo) = (DFTPhaseCorrFlag(NMRDataStruct, *StepNo) & 0xF0) | PHASE0_Auto;
				
				for (i = *StepNo + 1; i < NMRDataStruct->StepCount; i++) {
					Changed |= ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_FollowAuto);
					DFTPhaseCorrFlag(NMRDataStruct, i) = (DFTPhaseCorrFlag(NMRDataStruct, i) & 0xF0) | PHASE0_FollowAuto;
				}
			} else {
				/* Val = 0; */
				RetVal = INVALID_PARAMETER;
			}
			
			if (Changed)
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, ALL_STEPS);
			
			break;

		case PROC_PARAM_PhaseCorr1ManualRefDataStart:
			for (i = Start; i < Range; i++) {
				if ((DFTPhaseCorr1Ref(NMRDataStruct, i) != 0) || (DFTPhaseCorr1(NMRDataStruct, i) != Val)) {
					DFTPhaseCorr1Ref(NMRDataStruct, i) = 0;
					DFTPhaseCorr1(NMRDataStruct, i) = Val;
				
					if (NMRDataStruct->RemoveOffset)
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr, i);
					else 
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr_ReIm, i);

					Changed = 1;
					if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Manual)
						Changed2 = 1;
				}
			}
			
			if (Changed && (NMRDataStruct->Steps->DFTPhaseCorrOutput == NMRDataStruct->Steps->DFTOutput))
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemReIm, ALL_STEPS);	/** need to allocate memory for phase corrected data **/

			if (Changed && NMRDataStruct->RemoveOffset && (NMRDataStruct->Steps->DFTPhaseCorrOutAmp == NMRDataStruct->Steps->DFTOutAmp)) 
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemAmp, ALL_STEPS);	/** need to allocate memory for phase corrected data **/
			
			if (Changed2)
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, ALL_STEPS);
			
			break;
			
		case PROC_PARAM_PhaseCorr1ManualRefProcStart:
			for (i = Start; i < Range; i++) {
				if ((DFTPhaseCorr1Ref(NMRDataStruct, i) != -1) || (DFTPhaseCorr1(NMRDataStruct, i) != Val)) {
					DFTPhaseCorr1Ref(NMRDataStruct, i) = -1;
					DFTPhaseCorr1(NMRDataStruct, i) = Val;
					
					if (NMRDataStruct->RemoveOffset)
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr, i);
					else 
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr_ReIm, i);

					Changed = 1;
					if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Manual)
						Changed2 = 1;
				}
			}
			
			if (Changed && (NMRDataStruct->Steps->DFTPhaseCorrOutput == NMRDataStruct->Steps->DFTOutput))
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemReIm, ALL_STEPS);	/** need to allocate memory for phase corrected data **/

			if (Changed && NMRDataStruct->RemoveOffset && (NMRDataStruct->Steps->DFTPhaseCorrOutAmp == NMRDataStruct->Steps->DFTOutAmp)) 
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemAmp, ALL_STEPS);	/** need to allocate memory for phase corrected data **/
			
			if (Changed2)
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, ALL_STEPS);
			
			break;
		
		case PROC_PARAM_PhaseCorr1Ref:
			for (i = Start; i < Range; i++) {
				if (DFTPhaseCorr1Ref(NMRDataStruct, i) != Val) {
					DFTPhaseCorr1Ref(NMRDataStruct, i) = Val;
					
					if (NMRDataStruct->RemoveOffset)
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr, i);
					else 
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorr_ReIm, i);

					Changed = 1;
					if ((DFTPhaseCorrFlag(NMRDataStruct, i) & 0x0F) != PHASE0_Manual)
						Changed2 = 1;
				}
			}
			
			if (Changed && (NMRDataStruct->Steps->DFTPhaseCorrOutput == NMRDataStruct->Steps->DFTOutput))
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemReIm, ALL_STEPS);	/** need to allocate memory for phase corrected data **/

			if (Changed && NMRDataStruct->RemoveOffset && (NMRDataStruct->Steps->DFTPhaseCorrOutAmp == NMRDataStruct->Steps->DFTOutAmp)) 
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_MemAmp, ALL_STEPS);	/** need to allocate memory for phase corrected data **/
			
			if (Changed2)
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep_AutoCorr, ALL_STEPS);
			
			break;

			
		case PROC_PARAM_StepFlag:
			Val &= /*STEP_BLANK |*/ STEP_IGNORE | STEP_NO_ENVELOPE | STEP_NO_SHOW | STEP_OK;	/** STEP_BLANK is not to be set by the user code **/
			
			for (i = Start; i < Range; i++) {
				if ((StepFlag(NMRDataStruct, i) & (STEP_IGNORE | STEP_NO_ENVELOPE | STEP_NO_SHOW | STEP_OK)) != (unsigned long) Val) {
					if ((StepFlag(NMRDataStruct, i) & STEP_IGNORE) != (((unsigned long) Val) & STEP_IGNORE))
						Changed |= 1;
					
					if ((StepFlag(NMRDataStruct, i) & STEP_NO_ENVELOPE) != (((unsigned long) Val) & STEP_NO_ENVELOPE)) 
						Changed2 |= 1;
					
					StepFlag(NMRDataStruct, i) = Val;
				}
			}
			
			if (Changed)
				MarkNMRDataOld(NMRDataStruct, CHECK_ChunkSet, ALL_STEPS);
			
			if (Changed2) {
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTEnvelope, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTRealEnvelope, ALL_STEPS);
			}
			
			break;
			
		case PROC_PARAM_SetStepFlag:
			Val &= /*STEP_BLANK |*/ STEP_IGNORE | STEP_NO_ENVELOPE | STEP_NO_SHOW | STEP_OK;	/** STEP_BLANK is not to be set by the user code **/
			
			for (i = Start; i < Range; i++) {
				if ((StepFlag(NMRDataStruct, i) & (unsigned long) Val) != (unsigned long) Val) {
					if (!(StepFlag(NMRDataStruct, i) & STEP_IGNORE) && (((unsigned long) Val) & STEP_IGNORE))
						Changed |= 1;
					
					if (!(StepFlag(NMRDataStruct, i) & STEP_NO_ENVELOPE) && (((unsigned long) Val) & STEP_NO_ENVELOPE)) 
						Changed2 |= 1;
					
					StepFlag(NMRDataStruct, i) |= Val;
				}
			}
			
			if (Changed)
				MarkNMRDataOld(NMRDataStruct, CHECK_ChunkSet, ALL_STEPS);
			
			if (Changed2) {
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTEnvelope, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTRealEnvelope, ALL_STEPS);
			}
			
			break;
			
		case PROC_PARAM_ClearStepFlag:
			Val &= /*STEP_BLANK |*/ STEP_IGNORE | STEP_NO_ENVELOPE | STEP_NO_SHOW | STEP_OK;	/** STEP_BLANK is not to be set by the user code **/
			
			for (i = Start; i < Range; i++) {
				if (StepFlag(NMRDataStruct, i) & ((unsigned long) Val)) {
					if ((StepFlag(NMRDataStruct, i) & STEP_IGNORE) && (((unsigned long) Val) & STEP_IGNORE))
						Changed |= 1;
					
					if ((StepFlag(NMRDataStruct, i) & STEP_NO_ENVELOPE) && (((unsigned long) Val) & STEP_NO_ENVELOPE)) 
						Changed2 |= 1;
					
					StepFlag(NMRDataStruct, i) &= ~((unsigned long) Val);
				}
			}
			
			if (Changed)
				MarkNMRDataOld(NMRDataStruct, CHECK_ChunkSet, ALL_STEPS);
			
			if (Changed2) {
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTEnvelope, ALL_STEPS);
				MarkNMRDataOld(NMRDataStruct, CHECK_DFTRealEnvelope, ALL_STEPS);
			}
			
			break;

		default:
		/*	RetVal = INVALID_PARAMETER;
			break;	*/
			return INVALID_PARAMETER;
	}

	if ((Changed | Changed2) && (NMRDataStruct->ChangeProcParamCallback != NULL))
		NMRDataStruct->ChangeProcParamCallback(NMRDataStruct, ParamType, ALL_STEPS);
	
/*	return GetProcParam(NMRDataStruct, ParamType, type, ParamValue, StepNo);	*/
		
	if (type == PARAM_LONG)
		*((long *) ParamValue) = Val;
	else 
		switch (ParamType) {
			case PROC_PARAM_PhaseCorr0:
				*((double *) ParamValue) = 1.0e-3*Val;	/** deg **/
				break;
			
			case PROC_PARAM_PhaseCorr1ManualRefDataStart:
			case PROC_PARAM_PhaseCorr1ManualRefProcStart:
				*((double *) ParamValue) = 1.0e-3*Val;	/** us **/
				break;
			
			case PROC_PARAM_Filter:
				*((double *) ParamValue) = 1.0e-6*Val;	/** MHz **/
				break;
			
			default:
				*((double *) ParamValue) = Val;
				break;
		}
	
	return RetVal;
}

/** Checks and (if necessary) adjusts the processing prameters **/
EXPORT int CheckProcParam(NMRData *NMRDataStruct, unsigned int ParamType, unsigned int type, void *ParamValue, long *StepNo) {
	long Val = 0;
	int RetVal = DATA_OK;
	long AllSteps = ALL_STEPS;
	long *pStep = &AllSteps;
	size_t Step = 0;
	size_t Start = 0;
	size_t Range = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((ParamValue != NULL) && ((type != PARAM_LONG) && (type != PARAM_DOUBLE)))
		return INVALID_PARAMETER;

	if ((StepNo != NULL) && (*StepNo >= 0) && ((size_t) *StepNo < StepNoRange(NMRDataStruct))) {
		pStep = StepNo;
		Start = *StepNo;
		Range = *StepNo + 1;
	} else {
		Start = 0;
		Range = StepNoRange(NMRDataStruct);
	}

	switch (ParamType) {
		case PROC_PARAM_FirstChunk:
		case PROC_PARAM_LastChunk:
		case PROC_PARAM_ChunkStart:
		case PROC_PARAM_ChunkEnd:
		case PROC_PARAM_DFTLength:
		case PROC_PARAM_ScaleFirstTDPoint:
		case PROC_PARAM_RemoveOffset:
		case PROC_PARAM_Filter:
			if ((RetVal = GetProcParam(NMRDataStruct, ParamType, PARAM_LONG, &Val, pStep)) != DATA_OK)
				return RetVal;
			
			if ((RetVal = SetProcParam(NMRDataStruct, ParamType, PARAM_LONG, &Val, pStep)) != DATA_OK)
				return RetVal;
			
			break;

		case PROC_PARAM_PhaseCorr0AutoAllTogether:
		case PROC_PARAM_PhaseCorr0FollowAuto:
			if ((RetVal = GetProcParam(NMRDataStruct, ParamType, PARAM_LONG, &Val, pStep)) != DATA_OK)
				return RetVal;
			
			if (Val) {
				/** set the flags again to make sure that no invalid combination is present **/
				if ((RetVal = SetProcParam(NMRDataStruct, ParamType, PARAM_LONG, &Val, pStep)) != DATA_OK)
					return RetVal;
			} else {
				if ((NMRDataStruct->Steps != NULL) && (ParamType == PROC_PARAM_PhaseCorr0FollowAuto)) {
					for (Step = 0; Step < StepNoRange(NMRDataStruct); Step++) {
						/** clear all PHASE0_FollowAuto flags since there is no PHASE0_Auto flag set **/
						if ((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) == PHASE0_FollowAuto)
							DFTPhaseCorrFlag(NMRDataStruct, Step) = (DFTPhaseCorrFlag(NMRDataStruct, Step) & 0xF0) | PHASE0_Manual;
					}
				}
			}
			
			/** check validity of all flags and normalize DFTPhaseCorr0 values **/
			Start = 0;
			Range = StepNoRange(NMRDataStruct);
			
			if (NMRDataStruct->Steps != NULL) {
				for (Step = Start; Step < Range; Step++) {
					/** clear invalid flag combinations **/
					if (((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_Manual) && 
						((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_Auto) && 
						((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_AutoAllTogether) && 
						((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_FollowAuto))
							DFTPhaseCorrFlag(NMRDataStruct, Step) = (DFTPhaseCorrFlag(NMRDataStruct, Step) & 0xF0) | PHASE0_Manual;
					 
					DFTPhaseCorr0(NMRDataStruct, Step) %= 360000;
				}
			}
			
			break;

		case PROC_PARAM_PhaseCorr0:
		case PROC_PARAM_PhaseCorr0Auto:
			if (NMRDataStruct->Steps != NULL) {
				for (Step = Start; Step < Range; Step++) {
					/** clear invalid flag combinations **/
					if (((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_Manual) && 
						((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_Auto) && 
						((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_AutoAllTogether) && 
						((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) != PHASE0_FollowAuto))
							DFTPhaseCorrFlag(NMRDataStruct, Step) = (DFTPhaseCorrFlag(NMRDataStruct, Step) & 0xF0) | PHASE0_Manual;
					 
					DFTPhaseCorr0(NMRDataStruct, Step) %= 360000;
				}
			}
			
			break;
			
		case PROC_PARAM_PhaseCorr1ManualRefDataStart:
		case PROC_PARAM_PhaseCorr1ManualRefProcStart:
		case PROC_PARAM_PhaseCorr1Ref:
			if (NMRDataStruct->Steps != NULL) {
				for (Step = Start; Step < Range; Step++) {
					if ((DFTPhaseCorrFlag(NMRDataStruct, Step) & 0xF0) != PHASE1_Manual)
						DFTPhaseCorrFlag(NMRDataStruct, Step) = (DFTPhaseCorrFlag(NMRDataStruct, Step) & 0x0F) | PHASE1_Manual;
					 
					if (DFTPhaseCorr1(NMRDataStruct, Step) > 2000000000) {
						DFTPhaseCorr1(NMRDataStruct, Step) = 2000000000;
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep, Step);
					}
					if (DFTPhaseCorr1(NMRDataStruct, Step) < -2000000000) {
						DFTPhaseCorr1(NMRDataStruct, Step) = -2000000000;
						MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep, Step);
					}
					
					if (DFTPhaseCorr1Ref(NMRDataStruct, Step) != 0)
						DFTPhaseCorr1Ref(NMRDataStruct, Step) = -1;
				}
			}
			
			break;
			
		case PROC_PARAM_StepFlag:
		case PROC_PARAM_SetStepFlag:
		case PROC_PARAM_ClearStepFlag:
			if (NMRDataStruct->Steps != NULL) {
				for (Step = Start; Step < Range; Step++) 
					StepFlag(NMRDataStruct, Step) &= STEP_BLANK | STEP_IGNORE | STEP_NO_ENVELOPE | STEP_NO_SHOW | STEP_OK;
			}
			
			break;
		
		default:
			return INVALID_PARAMETER;
	}		
	
	if (ParamValue != NULL)
		return GetProcParam(NMRDataStruct, ParamType, type, ParamValue, StepNo);
	
	return DATA_OK;
}

EXPORT int ImportProcParams(NMRData *NMRDataStruct, char *TextFileName) {
	int RetVal = DATA_OK;
	int RVal = DATA_OK;
	char *TextData = NULL;
	size_t TextLength = 0;
	long ProcParam = 0;
	long Step = -1;
	unsigned char AuxFlag = 0;
	long AuxPhaseCorr = 0;
	unsigned long *ExpFlags = NULL;
	size_t ExpFlagsCount = 0;
	unsigned long *PhaseFlags = NULL;
	size_t PhaseFlagsCount = 0;
	double *PhaseValues = NULL;
	size_t PhaseValuesCount = 0;
	long *PhaseRefValues = NULL;
	size_t PhaseRefValuesCount = 0;
	unsigned char Changed = 0;
	unsigned char Changed2 = 0;
	size_t i = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if (TextFileName == NULL)
		return INVALID_PARAMETER;
	
	RetVal = LoadTextFile(NMRDataStruct, &TextData, &TextLength, TextFileName);
	if (RetVal != FILE_LOADED_OK)
		return RetVal;
	
	
	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%FirstChunk" , &ProcParam, PARAM_LONG);
	if (RVal != DATA_OK)
		RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%FirstEcho" , &ProcParam, PARAM_LONG);
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_FirstChunk, PARAM_LONG, &ProcParam, &Step);
	RetVal |= RVal;

	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%LastChunk" , &ProcParam, PARAM_LONG);
	if (RVal != DATA_OK)
		RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%LastEcho" , &ProcParam, PARAM_LONG);
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_LastChunk, PARAM_LONG, &ProcParam, &Step);
	RetVal |= RVal;

	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%ChunkStart" , &ProcParam, PARAM_LONG);
	if (RVal != DATA_OK)
		RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%EchoBegin" , &ProcParam, PARAM_LONG);
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_ChunkStart, PARAM_LONG, &ProcParam, &Step);
	RetVal |= RVal;

	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%ChunkEnd" , &ProcParam, PARAM_LONG);
	if (RVal != DATA_OK)
		RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%EchoEnd" , &ProcParam, PARAM_LONG);
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_ChunkEnd, PARAM_LONG, &ProcParam, &Step);
	RetVal |= RVal;

	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%DFTLength" , &ProcParam, PARAM_LONG);
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_DFTLength, PARAM_LONG, &ProcParam, &Step);
	RetVal |= RVal;

	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%ScaleFirstTDPoint" , &ProcParam, PARAM_LONG);
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_ScaleFirstTDPoint, PARAM_LONG, &ProcParam, &Step);
	RetVal |= (RVal & ~DATA_EMPTY);

	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%RemoveOffset" , &ProcParam, PARAM_LONG);
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_RemoveOffset, PARAM_LONG, &ProcParam, &Step);
	RetVal |= (RVal & ~DATA_EMPTY);

	RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%FilterHz", &ProcParam, PARAM_LONG);
	if (RVal != DATA_OK) {
		RVal = GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength,"%filter", &ProcParam, PARAM_LONG);
		if (RVal == DATA_OK)
			ProcParam = lround(1.0e6*DFTFilterToFreq(NMRDataStruct, ProcParam));
	}
	if (RVal == DATA_OK)
		SetProcParam(NMRDataStruct, PROC_PARAM_Filter, PARAM_LONG, &ProcParam, &Step);
	RetVal |= RVal;

	
	RVal = CheckNMRData(NMRDataStruct, CHECK_StepSet, ALL_STEPS);

	if (RVal == DATA_OK) {
		RVal = GetAcqusStyleParamSet(NMRDataStruct, TextData, TextLength, "%StepFlags", (void **) &ExpFlags, &ExpFlagsCount, PARAM_ULONG);
		if (RVal != DATA_OK) 
			RVal = GetAcqusStyleParamSet(NMRDataStruct, TextData, TextLength, "%ExpFlags", (void **) &ExpFlags, &ExpFlagsCount, PARAM_ULONG);
		if (RVal == DATA_OK) {
			if (ExpFlagsCount == StepNoRange(NMRDataStruct)) {
				for (i = 0; i < ExpFlagsCount; i++) {
					ExpFlags[i] &= /*STEP_BLANK |*/ STEP_IGNORE | STEP_NO_ENVELOPE | STEP_NO_SHOW | STEP_OK;	/** STEP_BLANK is not to be set by the user code **/
					if ((StepFlag(NMRDataStruct, i) & (STEP_IGNORE | STEP_NO_ENVELOPE | STEP_NO_SHOW | STEP_OK)) != ExpFlags[i]) {
						if ((StepFlag(NMRDataStruct, i) & STEP_IGNORE) != (ExpFlags[i] & STEP_IGNORE))
							Changed |= 1;
						
						if ((StepFlag(NMRDataStruct, i) & STEP_NO_ENVELOPE) != (ExpFlags[i] & STEP_NO_ENVELOPE)) 
							Changed2 |= 1;
						
						StepFlag(NMRDataStruct, i) = ExpFlags[i];
					}
				}
				
				if (Changed)
					MarkNMRDataOld(NMRDataStruct, CHECK_ChunkSet, ALL_STEPS);
				
				if (Changed2) {
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTEnvelope, ALL_STEPS);
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTRealEnvelope, ALL_STEPS);
				}
				
				if ((Changed | Changed2) && (NMRDataStruct->ChangeProcParamCallback != NULL))
					NMRDataStruct->ChangeProcParamCallback(NMRDataStruct, PROC_PARAM_StepFlag, ALL_STEPS);
			}
		}
		free(ExpFlags);
		ExpFlags = NULL;
		ExpFlagsCount = 0;
		Changed = 0;
		Changed2 = 0;
		
		RVal &= ~DATA_EMPTY;
		RetVal |= RVal;
		
		
		RVal = GetAcqusStyleParamSet(NMRDataStruct, TextData, TextLength, "%PhaseCorrFlags", (void **) &PhaseFlags, &PhaseFlagsCount, PARAM_ULONG);
		if (RVal == DATA_OK) {
			if (PhaseFlagsCount == StepNoRange(NMRDataStruct)) {
				for (i = 0; i < PhaseFlagsCount; i++) {
					AuxFlag = DFTPhaseCorrFlag(NMRDataStruct, i);
					DFTPhaseCorrFlag(NMRDataStruct, i) = PhaseFlags[i];
					if (DFTPhaseCorrFlag(NMRDataStruct, i) != AuxFlag)
						Changed = 1;
				}
				
				if (Changed) 
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep, ALL_STEPS);
				
				if ((Changed | Changed2) && (NMRDataStruct->ChangeProcParamCallback != NULL))
					NMRDataStruct->ChangeProcParamCallback(NMRDataStruct, PROC_PARAM_PhaseCorr0AutoAllTogether, ALL_STEPS);
			}
		}
		free(PhaseFlags);
		PhaseFlags = NULL;
		PhaseFlagsCount = 0;
		Changed = 0;
		
		RVal = GetAcqusStyleParamSet(NMRDataStruct, TextData, TextLength, "%PhaseCorr0", (void **) &PhaseValues, &PhaseValuesCount, PARAM_DOUBLE);
		if (RVal == DATA_OK) {
			if (PhaseValuesCount == StepNoRange(NMRDataStruct)) {
				for (i = 0; i < PhaseValuesCount; i++) {
					AuxPhaseCorr = DFTPhaseCorr0(NMRDataStruct, i);
					DFTPhaseCorr0(NMRDataStruct, i) = lround(1000.0*PhaseValues[i]);
					if (DFTPhaseCorr0(NMRDataStruct, i) != AuxPhaseCorr)
						Changed = 1;
				}
				
				if (Changed) 
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep, ALL_STEPS);
				
				if ((Changed | Changed2) && (NMRDataStruct->ChangeProcParamCallback != NULL))
					NMRDataStruct->ChangeProcParamCallback(NMRDataStruct, PROC_PARAM_PhaseCorr0, ALL_STEPS);
			}
		}
		free(PhaseValues);
		PhaseValues = NULL;
		PhaseValuesCount = 0;
		Changed = 0;
		
		RVal = GetAcqusStyleParamSet(NMRDataStruct, TextData, TextLength, "%PhaseCorr1", (void **) &PhaseValues, &PhaseValuesCount, PARAM_DOUBLE);
		if (RVal == DATA_OK) {
			if (PhaseValuesCount == StepNoRange(NMRDataStruct)) {
				for (i = 0; i < PhaseValuesCount; i++) {
					AuxPhaseCorr = DFTPhaseCorr1(NMRDataStruct, i);
					DFTPhaseCorr1(NMRDataStruct, i) = lround(1000.0*PhaseValues[i]);
					if (DFTPhaseCorr1(NMRDataStruct, i) != AuxPhaseCorr)
						Changed = 1;
				}
				
				if (Changed) 
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep, ALL_STEPS);
				
				if ((Changed | Changed2) && (NMRDataStruct->ChangeProcParamCallback != NULL))
					NMRDataStruct->ChangeProcParamCallback(NMRDataStruct, PROC_PARAM_PhaseCorr1ManualRefDataStart, ALL_STEPS);
			}
		}
		free(PhaseValues);
		PhaseValues = NULL;
		PhaseValuesCount = 0;
		Changed = 0;
		
		RVal = GetAcqusStyleParamSet(NMRDataStruct, TextData, TextLength, "%PhaseCorr1Ref", (void **) &PhaseRefValues, &PhaseRefValuesCount, PARAM_LONG);
		if (RVal == DATA_OK) {
			if (PhaseRefValuesCount == StepNoRange(NMRDataStruct)) {
				for (i = 0; i < PhaseRefValuesCount; i++) {
					AuxPhaseCorr = DFTPhaseCorr1Ref(NMRDataStruct, i);
					DFTPhaseCorr1Ref(NMRDataStruct, i) = PhaseRefValues[i];
					if (DFTPhaseCorr1Ref(NMRDataStruct, i) != AuxPhaseCorr)
						Changed = 1;
				}
				
				if (Changed) 
					MarkNMRDataOld(NMRDataStruct, CHECK_DFTPhaseCorrPrep, ALL_STEPS);
				
				if ((Changed | Changed2) && (NMRDataStruct->ChangeProcParamCallback != NULL))
					NMRDataStruct->ChangeProcParamCallback(NMRDataStruct, PROC_PARAM_PhaseCorr1Ref, ALL_STEPS);
			}
		}
		free(PhaseRefValues);
		PhaseRefValues = NULL;
		PhaseRefValuesCount = 0;
		Changed = 0;
	}
	
	if ((RVal = CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr0AutoAllTogether, PARAM_LONG, &ProcParam, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr0AutoAllTogether' check failed", "Importing processing parameters");
	RetVal |= RVal;
	
	if ((RVal = CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr0FollowAuto, PARAM_LONG, &ProcParam, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr0FollowAuto' check failed", "Importing processing parameters");
	RetVal |= RVal;
	
	if ((RVal = CheckProcParam(NMRDataStruct, PROC_PARAM_PhaseCorr1ManualRefDataStart, PARAM_LONG, &ProcParam, NULL)) != DATA_OK)
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Processing parameter 'PhaseCorr1ManualRefDataStart' check failed", "Importing processing parameters");
	RetVal |= RVal;
	
	
	if((RVal = FreeText(NMRDataStruct, &TextData, &TextLength)) != DATA_EMPTY)
		return (RetVal | RVal);

	return RetVal;
}



typedef int (*NMRExportFunc)(NMRData *, FILE *, char **, size_t *);

typedef struct {
	NMRExportFunc method;
	unsigned short requires;
} NMRExportRelation;

/** Exports selected data as a text in full form to a file (FILE *foutput) and/or in simplified form 
to a char buffer (char **soutput, size_t *slength - the buffer will be allocated by the function). **/
EXPORT int DataToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength, unsigned int DataType) {
	int RetVal = DATA_OK;
	char *auxptr = NULL;
	
	const NMRExportRelation NMRExportFuncSet[11] = {
		{&AcquInfoToText, CHECK_AcquInfo},
		{&ProcParamsToText, CHECK_DFTPhaseCorrPrep /** All proc params are verified at this stage **/}, 
		{&TDDToText, CHECK_ChunkSet}, 
		{&ChunkSetToText, CHECK_ChunkSet}, 
		{&ChunkAvgToText, CHECK_ChunkAvg}, 
		{&DFTResultToText, CHECK_DFTResult /* CHECK_DFTPhaseCorrPrep */ /** All proc params are verified at this stage **/}, 
		{&DFTPhaseCorrectedResultToText, CHECK_DFTPhaseCorr}, 
		{&DFTEnvelopeToText, CHECK_DFTEnvelope}, 
		{&DFTPhaseCorrRealEnvelopeToText, CHECK_DFTRealEnvelope}, 
		{&EchoPeaksEnvelopeToText, CHECK_EchoPeaksEnvelope}, 
		{&EvaluationToText, CHECK_Evaluation}
	};
		
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if ((foutput == NULL) && ((soutput == NULL) || (slength == NULL))) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid output pointer supplied", "Exporting data to text");
		return INVALID_PARAMETER;
	}
	
	if (DataType > EXPORT_Highest) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid data type requested", "Exporting data to text");
		return INVALID_PARAMETER;
	}
	
	RetVal = CheckNMRData(NMRDataStruct, NMRExportFuncSet[DataType].requires, ALL_STEPS);
	if (RetVal != DATA_OK) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Cannot acquire data", "Exporting data to text");
		return RetVal;
	}

	if ((soutput != NULL) && (slength != NULL)) {
		free(*soutput);
		*soutput = NULL;
		*slength = 0;
	}

	if (StepNoRange(NMRDataStruct) > 0) {
		RetVal = NMRExportFuncSet[DataType].method(NMRDataStruct, foutput, soutput, slength);
		if (RetVal != DATA_OK) 
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Data to text conversion failed", "Exporting data to text");
	} else {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "There are no data to export", "Exporting data to text");
		return DATA_EMPTY | DATA_VOID;
	}
	
	if ((soutput != NULL) && (slength != NULL)) {
		if ((RetVal & DATA_VOID) == DATA_OK) {
			/** shrink the buffer **/
			auxptr = realloc(*soutput, ((*slength) + 1) * sizeof(char));
			if (auxptr != NULL)
				*soutput = auxptr;
		} else {
			/** free the buffer **/
			free(*soutput);
			*soutput = NULL;
			*slength = 0;
		}
	}

	return RetVal;
}
