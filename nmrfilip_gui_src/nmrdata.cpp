/* 
 * NMRFilip GUI - the NMR data processing software - graphical user interface
 * Copyright (C) 2010, 2020 Richard Reznicek
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

#include "nmrdata.h"

#include "plotgen.h"

/// pointers to functions in nmrfilip dll
InitNMRDataFunc NFGNMRData::InitNMRData;
CheckNMRDataFunc NFGNMRData::CheckNMRData;
FreeNMRDataFunc NFGNMRData::FreeNMRData;
RefreshNMRDataFunc NFGNMRData::RefreshNMRData;
ReloadNMRDataFunc NFGNMRData::ReloadNMRData;

CheckProcParamFunc NFGNMRData::CheckProcParam;
GetProcParamFunc NFGNMRData::GetProcParam;
SetProcParamFunc NFGNMRData::SetProcParam;
ImportProcParamsFunc NFGNMRData::ImportProcParams;

DataToTextFunc NFGNMRData::DataToText;

InitUserlistFunc NFGNMRData::InitUserlist;
ReadUserlistFunc NFGNMRData::ReadUserlist;
WriteUserlistFunc NFGNMRData::WriteUserlist;
FreeUserlistFunc NFGNMRData::FreeUserlist;

CleanupOnExitFunc NFGNMRData::CleanupOnExit;


/// Round to the nearest integer, round half up, errors ignored
long long NFGNMRData::llroundnu(double val) 
{
	double retval = std::ceil(val);
	
	if ((retval - val) > 0.5)
		retval -= 1.0;
	
	return (long long) retval;
}


/// Wrapper functions for data access macros used to simplify the code of NFGGraph and NFGDataset derived classes
/// Note: No function argument checking is performed.
/// time in us, freq in MHz
size_t NFGNMRData::GetStepNoRange(NMRData* NMRDataPtr)													{ return StepNoRange(NMRDataPtr); }
unsigned long NFGNMRData::GetStepFlag(NMRData* NMRDataPtr, size_t StepNo)										{ return StepFlag(NMRDataPtr, StepNo); }
double NFGNMRData::GetStepAssocValue(NMRData* NMRDataPtr, size_t StepNo)										{ return StepAssocValue(NMRDataPtr, StepNo); }
unsigned long NFGNMRData::GetStepAssocType(NMRData* NMRDataPtr, size_t StepNo)								{ return StepAssocType(NMRDataPtr, StepNo); }

size_t NFGNMRData::GetChunkNoRange(NMRData* NMRDataPtr)													{ return ChunkNoRange(NMRDataPtr); }
size_t NFGNMRData::GetChunkIndexRange(NMRData* NMRDataPtr, size_t ChunkNo)								{ return ChunkIndexRange(NMRDataPtr, ChunkNo); }

#define GET_DATA_IdxRng_IMPL(Data, Xdom) 	\
size_t NFGNMRData::Get##Data##IndexRange(NMRData* NMRDataPtr, size_t No)						{ return Data##IndexRange(NMRDataPtr, No); } 
#define GET_DATA_Xdom_IMPL(Data, Xdom) 	\
double NFGNMRData::Get##Data##Xdom(NMRData* NMRDataPtr, size_t No, size_t Index)				{ return Data##Xdom(NMRDataPtr, No, Index); } 
#define GET_DATA_Val_IMPL(Data, Xdom, Comp) 	\
double NFGNMRData::Get##Data##Comp(NMRData* NMRDataPtr, size_t No, size_t Index)				{ return Data##Comp(NMRDataPtr, No, Index); } \
wxPoint NFGNMRData::Get##Data##Comp##Pt(NMRData* NMRDataPtr, size_t No, size_t Index, NFGScale Scale)	{ return wxPoint(NFGNMRData::llroundnu(Data##Xdom(NMRDataPtr, No, Index)*Scale.xfactor) - Scale.xoffset, NFGNMRData::llroundnu(Data##Comp(NMRDataPtr, No, Index)*Scale.yfactor) - Scale.yoffset); } \
wxRealPoint NFGNMRData::Get##Data##Comp##RPt(NMRData* NMRDataPtr, size_t No, size_t Index)			{ return wxRealPoint(Data##Xdom(NMRDataPtr, No, Index), Data##Comp(NMRDataPtr, No, Index)); } \
void NFGNMRData::Get##Data##Comp##RPtBB(NMRData* NMRDataPtr, size_t No, double &minx, double &maxx, double &miny, double &maxy) { \
	minx = NFGMSTD fmin(Data##Xdom(NMRDataPtr, No, 0), Data##Xdom(NMRDataPtr, No, Data##IndexRange(NMRDataPtr, No) - 1)); \
	maxx = NFGMSTD fmax(Data##Xdom(NMRDataPtr, No, 0), Data##Xdom(NMRDataPtr, No, Data##IndexRange(NMRDataPtr, No) - 1)); \
	miny = HUGE_VAL; \
	maxy = - HUGE_VAL; \
	for (size_t i = 0; i < Data##IndexRange(NMRDataPtr, No); i++) { \
		if (Data##Comp(NMRDataPtr, No, i) < miny) \
			miny = Data##Comp(NMRDataPtr, No, i); \
		if (Data##Comp(NMRDataPtr, No, i) > maxy) \
			maxy = Data##Comp(NMRDataPtr, No, i); \
	} \
} \
void NFGNMRData::Get##Data##Comp##Pts(NMRData* NMRDataPtr, size_t No, wxPoint* PointArray, NFGScale Scale) { \
	for (size_t i = 0; i < Data##IndexRange(NMRDataPtr, No); i++) { \
		PointArray[i].x = NFGNMRData::llroundnu(Data##Xdom(NMRDataPtr, No, i)*Scale.xfactor) - Scale.xoffset; \
		PointArray[i].y = NFGNMRData::llroundnu(Data##Comp(NMRDataPtr, No, i)*Scale.yfactor) - Scale.yoffset; \
	} \
}

GET_DATA_IdxRng_IMPL(TDD, Time)
GET_DATA_Xdom_IMPL(TDD, Time)
GET_DATA_Val_IMPL(TDD, Time, Real)
GET_DATA_Val_IMPL(TDD, Time, Imag)
double NFGNMRData::GetTDDAmp(NMRData* NMRDataPtr, size_t No, size_t Index)				{ return TDDAmp(NMRDataPtr, No, Index); } 
wxPoint NFGNMRData::GetTDDAmpPt(NMRData* NMRDataPtr, size_t No, size_t Index, NFGScale Scale)	{ return wxPoint(NFGNMRData::llroundnu(TDDTime(NMRDataPtr, No, Index)*Scale.xfactor) - Scale.xoffset, NFGNMRData::llroundnu(TDDAmp(NMRDataPtr, No, Index)*Scale.yfactor) - Scale.yoffset); } 
wxRealPoint NFGNMRData::GetTDDAmpRPt(NMRData* NMRDataPtr, size_t No, size_t Index)			{ return wxRealPoint(TDDTime(NMRDataPtr, No, Index), TDDAmp(NMRDataPtr, No, Index)); } 
void NFGNMRData::GetTDDAmpRPtBB(NMRData* NMRDataPtr, size_t No, double &minx, double &maxx, double &miny, double &maxy) { 
	minx = NFGMSTD fmin(TDDTime(NMRDataPtr, No, 0), TDDTime(NMRDataPtr, No, TDDIndexRange(NMRDataPtr, No) - 1)); 
	maxx = NFGMSTD fmax(TDDTime(NMRDataPtr, No, 0), TDDTime(NMRDataPtr, No, TDDIndexRange(NMRDataPtr, No) - 1)); 
	miny = 0.0; /// checking for actual minimum omited
	maxy = 0.0; 
	for (size_t i = 0; i < TDDIndexRange(NMRDataPtr, No); i++) {
		/// computes upper estimation first for faster execution
		if (((unsigned long) std::labs(TDDReal(NMRDataPtr, No, i)) + (unsigned long) std::labs(TDDImag(NMRDataPtr, No, i))) > ((unsigned long) maxy))
			if (TDDAmp(NMRDataPtr, No, i) > maxy) 
				maxy = TDDAmp(NMRDataPtr, No, i); 
	} 
} 
void NFGNMRData::GetTDDAmpPts(NMRData* NMRDataPtr, size_t No, wxPoint* PointArray, NFGScale Scale) { 
	for (size_t i = 0; i < TDDIndexRange(NMRDataPtr, No); i++) { 
		PointArray[i].x = NFGNMRData::llroundnu(TDDTime(NMRDataPtr, No, i)*Scale.xfactor) - Scale.xoffset; 
		PointArray[i].y = NFGNMRData::llroundnu(TDDAmp(NMRDataPtr, No, i)*Scale.yfactor) - Scale.yoffset; 
	} 
}




#define GET_DATA_IMPL(Data, Xdom) 	\
GET_DATA_IdxRng_IMPL(Data, Xdom) \
GET_DATA_Xdom_IMPL(Data, Xdom) 	\
GET_DATA_Val_IMPL(Data, Xdom, Real) 	\
GET_DATA_Val_IMPL(Data, Xdom, Imag) 	\
GET_DATA_Val_IMPL(Data, Xdom, Amp) 

GET_DATA_IMPL(ChunkAvg, Time)

GET_DATA_IMPL(DFTProcNoFilterPhaseCorr, Freq)

GET_DATA_IdxRng_IMPL(EchoPeaksEnvelope, Time) 
GET_DATA_Xdom_IMPL(EchoPeaksEnvelope, Time) 
GET_DATA_Val_IMPL(EchoPeaksEnvelope, Time, Amp) 

#undef GET_DATA_IMPL
#undef GET_DATA_IdxRng_IMPL
#undef GET_DATA_Xdom_IMPL
#undef GET_DATA_Val_IMPL


#define GET_DATA_DFTEnv_IdxRng_IMPL(Data, Xdom) 	\
size_t NFGNMRData::Get##Data##IndexRange(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed))						{ return Data##IndexRange(NMRDataPtr); } 
#define GET_DATA_DFTEnv_Xdom_IMPL(Data, Xdom) 	\
double NFGNMRData::Get##Data##Xdom(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index)				{ return Data##Xdom(NMRDataPtr, Index); } 
#define GET_DATA_DFTEnv_Val_IMPL(Data, Xdom, Comp) 	\
double NFGNMRData::Get##Data##Comp(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index)				{ return Data##Comp(NMRDataPtr, Index); } \
wxPoint NFGNMRData::Get##Data##Comp##Pt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index, NFGScale Scale)	{ return wxPoint(NFGNMRData::llroundnu(Data##Xdom(NMRDataPtr, Index)*Scale.xfactor) - Scale.xoffset, NFGNMRData::llroundnu(Data##Comp(NMRDataPtr, Index)*Scale.yfactor) - Scale.yoffset); } \
wxRealPoint NFGNMRData::Get##Data##Comp##RPt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index)			{ return wxRealPoint(Data##Xdom(NMRDataPtr, Index), Data##Comp(NMRDataPtr, Index)); } \
void NFGNMRData::Get##Data##Comp##RPtBB(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), double &minx, double &maxx, double &miny, double &maxy) { \
	minx = NFGMSTD fmin(Data##Xdom(NMRDataPtr, 0), Data##Xdom(NMRDataPtr, Data##IndexRange(NMRDataPtr) - 1)); \
	maxx = NFGMSTD fmax(Data##Xdom(NMRDataPtr, 0), Data##Xdom(NMRDataPtr, Data##IndexRange(NMRDataPtr) - 1)); \
	miny = HUGE_VAL; \
	maxy = - HUGE_VAL; \
	for (size_t i = 0; i < Data##IndexRange(NMRDataPtr); i++) { \
		if (Data##Comp(NMRDataPtr, i) < miny) \
			miny = Data##Comp(NMRDataPtr, i); \
		if (Data##Comp(NMRDataPtr, i) > maxy) \
			maxy = Data##Comp(NMRDataPtr, i); \
	} \
} \
void NFGNMRData::Get##Data##Comp##Pts(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), wxPoint* PointArray, NFGScale Scale) { \
	for (size_t i = 0; i < Data##IndexRange(NMRDataPtr); i++) { \
		PointArray[i].x = NFGNMRData::llroundnu(Data##Xdom(NMRDataPtr, i)*Scale.xfactor) - Scale.xoffset; \
		PointArray[i].y = NFGNMRData::llroundnu(Data##Comp(NMRDataPtr, i)*Scale.yfactor) - Scale.yoffset; \
	} \
}

size_t NFGNMRData::GetDFTEnvelopeNoRange(NMRData* WXUNUSED(NMRDataPtr))											{ return 1; }
unsigned long NFGNMRData::GetDFTEnvelopeFlag(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed))								{ return STEP_OK; }
GET_DATA_DFTEnv_IdxRng_IMPL(DFTEnvelope, Freq) 
GET_DATA_DFTEnv_Xdom_IMPL(DFTEnvelope, Freq) 
GET_DATA_DFTEnv_Val_IMPL(DFTEnvelope, Freq, Amp) 

size_t NFGNMRData::GetDFTRealEnvelopeNoRange(NMRData* WXUNUSED(NMRDataPtr))											{ return 1; }
unsigned long NFGNMRData::GetDFTRealEnvelopeFlag(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed))								{ return STEP_OK; }
GET_DATA_DFTEnv_IdxRng_IMPL(DFTRealEnvelope, Freq) 
GET_DATA_DFTEnv_Xdom_IMPL(DFTRealEnvelope, Freq) 
GET_DATA_DFTEnv_Val_IMPL(DFTRealEnvelope, Freq, Real) 

#undef GET_DATA_DFTEnv_IdxRng_IMPL
#undef GET_DATA_DFTEnv_Xdom_IMPL
#undef GET_DATA_DFTEnv_Val_IMPL


#define GET_DATA_Eval_IMPL(Data) 	\
double NFGNMRData::Get##Data(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index)				{ return Data(NMRDataPtr, Index); } \
wxPoint NFGNMRData::Get##Data##Pt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index, NFGScale Scale)	{ return wxPoint(NFGNMRData::llroundnu(StepAssocValue(NMRDataPtr, Index)*Scale.xfactor) - Scale.xoffset, NFGNMRData::llroundnu(Data(NMRDataPtr, Index)*Scale.yfactor) - Scale.yoffset); } \
wxRealPoint NFGNMRData::Get##Data##RPt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index)			{ return wxRealPoint(StepAssocValue(NMRDataPtr, Index), Data(NMRDataPtr, Index)); } \
void NFGNMRData::Get##Data##RPtBB(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), double &minx, double &maxx, double &miny, double &maxy) { \
	minx = HUGE_VAL; \
	maxx = - HUGE_VAL; \
	for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
		if (StepAssocValue(NMRDataPtr, i) < minx) \
			minx = StepAssocValue(NMRDataPtr, i); \
		if (StepAssocValue(NMRDataPtr, i) > maxx) \
			maxx = StepAssocValue(NMRDataPtr, i); \
	} \
	miny = HUGE_VAL; \
	maxy = - HUGE_VAL; \
	for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
		if (Data(NMRDataPtr, i) < miny) \
			miny = Data(NMRDataPtr, i); \
		if (Data(NMRDataPtr, i) > maxy) \
			maxy = Data(NMRDataPtr, i); \
	} \
} \
void NFGNMRData::Get##Data##Pts(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), wxPoint* PointArray, NFGScale Scale) { \
	for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
		PointArray[i].x = NFGNMRData::llroundnu(StepAssocValue(NMRDataPtr, i)*Scale.xfactor) - Scale.xoffset; \
		PointArray[i].y = NFGNMRData::llroundnu(Data(NMRDataPtr, i)*Scale.yfactor) - Scale.yoffset; \
	} \
}

unsigned long NFGNMRData::GetEvaluationFlag(NMRData* WXUNUSED(NMRDataPtr), size_t WXUNUSED(NotUsed))								{ return STEP_OK; }
size_t NFGNMRData::GetEvaluationIndexRange(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed))							{ return StepNoRange(NMRDataPtr); }

GET_DATA_Eval_IMPL(DFTMeanPhaseCorrReal)
GET_DATA_Eval_IMPL(DFTPhaseCorrRealAtZero)
GET_DATA_Eval_IMPL(DFTMeanPhaseCorrAmp)
GET_DATA_Eval_IMPL(DFTPhaseCorrAmpAtZero)
GET_DATA_Eval_IMPL(ChunkAvgMaxAmp)
GET_DATA_Eval_IMPL(ChunkAvgIntAmp)

#undef GET_DATA_Eval_IMPL

#define GET_DATA_Eval_FreqMax_IMPL(Data) 	\
double NFGNMRData::Get##Data(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t index)				{ return Data(NMRDataPtr, index); } \
wxPoint NFGNMRData::Get##Data##Pt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t index, NFGScale Scale)	{ return wxPoint(NFGNMRData::llroundnu(((StepAssocType(NMRDataPtr, index) == ASSOC_FREQ_MHZ)?(DFTFreq(NMRDataPtr, index, Data##Index(NMRDataPtr, index))):(StepAssocValue(NMRDataPtr, index)))*Scale.xfactor) - Scale.xoffset, NFGNMRData::llroundnu(Data(NMRDataPtr, index)*Scale.yfactor) - Scale.yoffset); } \
wxRealPoint NFGNMRData::Get##Data##RPt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t index)			{ return wxRealPoint(((StepAssocType(NMRDataPtr, index) == ASSOC_FREQ_MHZ)?(DFTFreq(NMRDataPtr, index, Data##Index(NMRDataPtr, index))):(StepAssocValue(NMRDataPtr, index))), Data(NMRDataPtr, index)); } \
void NFGNMRData::Get##Data##RPtBB(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), double &minx, double &maxx, double &miny, double &maxy) { \
	minx = HUGE_VAL; \
	maxx = - HUGE_VAL; \
	if (StepAssocType(NMRDataPtr, 0) == ASSOC_FREQ_MHZ) \
		for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
			if (DFTFreq(NMRDataPtr, i, Data##Index(NMRDataPtr, i)) < minx) \
				minx = DFTFreq(NMRDataPtr, i, Data##Index(NMRDataPtr, i)); \
			if (DFTFreq(NMRDataPtr, i, Data##Index(NMRDataPtr, i)) > maxx) \
				maxx = DFTFreq(NMRDataPtr, i, Data##Index(NMRDataPtr, i)); \
		} \
	else \
		for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
			if (StepAssocValue(NMRDataPtr, i) < minx) \
				minx = StepAssocValue(NMRDataPtr, i); \
			if (StepAssocValue(NMRDataPtr, i) > maxx) \
				maxx = StepAssocValue(NMRDataPtr, i); \
		} \
	miny = HUGE_VAL; \
	maxy = - HUGE_VAL; \
	for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
		if (Data(NMRDataPtr, i) < miny) \
			miny = Data(NMRDataPtr, i); \
		if (Data(NMRDataPtr, i) > maxy) \
			maxy = Data(NMRDataPtr, i); \
	} \
} \
void NFGNMRData::Get##Data##Pts(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), wxPoint* PointArray, NFGScale Scale) { \
	if (StepAssocType(NMRDataPtr, 0) == ASSOC_FREQ_MHZ) \
		for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
			PointArray[i].x = NFGNMRData::llroundnu(DFTFreq(NMRDataPtr, i, Data##Index(NMRDataPtr, i))*Scale.xfactor) - Scale.xoffset; \
			PointArray[i].y = NFGNMRData::llroundnu(Data(NMRDataPtr, i)*Scale.yfactor) - Scale.yoffset; \
		} \
	else \
		for (size_t i = 0; i < StepNoRange(NMRDataPtr); i++) { \
			PointArray[i].x = NFGNMRData::llroundnu(StepAssocValue(NMRDataPtr, i)*Scale.xfactor) - Scale.xoffset; \
			PointArray[i].y = NFGNMRData::llroundnu(Data(NMRDataPtr, i)*Scale.yfactor) - Scale.yoffset; \
		} \
}

GET_DATA_Eval_FreqMax_IMPL(DFTMaxPhaseCorrReal)
GET_DATA_Eval_FreqMax_IMPL(DFTMaxPhaseCorrAmp)

#undef GET_DATA_Eval_FreqMax_IMPL
