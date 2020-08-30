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

#ifndef __nmrdata_h__
#define __nmrdata_h__

#include "wx_pch.h"

#include "cd.h"
#include "nmrdata_cd.h"

#include "plotgen.h"


namespace NFGNMRData
{
	/// pointers to functions in nmrfilip dll
	extern InitNMRDataFunc InitNMRData;
	extern CheckNMRDataFunc CheckNMRData;
	extern FreeNMRDataFunc FreeNMRData;
	extern RefreshNMRDataFunc RefreshNMRData;
	extern ReloadNMRDataFunc ReloadNMRData;

	extern CheckProcParamFunc CheckProcParam;
	extern GetProcParamFunc GetProcParam;
	extern SetProcParamFunc SetProcParam;
	extern ImportProcParamsFunc ImportProcParams;

	extern DataToTextFunc DataToText;

	extern InitUserlistFunc InitUserlist;
	extern ReadUserlistFunc ReadUserlist;
	extern WriteUserlistFunc WriteUserlist;
	extern FreeUserlistFunc FreeUserlist;

	extern CleanupOnExitFunc CleanupOnExit;


	long long llroundnu(double val);
	
	/// Wrapper functions for data access macros used to simplify the code of NFGGraph and NFGDataset derived classes
	/// Note: No function argument checking is performed.
	/// time in us, freq in MHz
	size_t GetStepNoRange(NMRData* NMRDataPtr);
	unsigned long GetStepFlag(NMRData* NMRDataPtr, size_t StepNo);
	double GetStepAssocValue(NMRData* NMRDataPtr, size_t StepNo);
	unsigned long GetStepAssocType(NMRData* NMRDataPtr, size_t StepNo);

	size_t GetChunkNoRange(NMRData* NMRDataPtr);
	size_t GetChunkIndexRange(NMRData* NMRDataPtr, size_t ChunkNo);

#define GET_DATA_IdxRng_DECL(Data, Xdom) 	\
	size_t Get##Data##IndexRange(NMRData* NMRDataPtr, size_t No); 
#define GET_DATA_Xdom_DECL(Data, Xdom) 	\
	double Get##Data##Xdom(NMRData* NMRDataPtr, size_t No, size_t Index); 
#define GET_DATA_Val_DECL(Data, Xdom, Comp) 	\
	double Get##Data##Comp(NMRData* NMRDataPtr, size_t No, size_t Index); \
	wxPoint Get##Data##Comp##Pt(NMRData* NMRDataPtr, size_t No, size_t Index, NFGScale Scale); \
	wxRealPoint Get##Data##Comp##RPt(NMRData* NMRDataPtr, size_t No, size_t Index); \
	void Get##Data##Comp##RPtBB(NMRData* NMRDataPtr, size_t No, double &minx, double &maxx, double &miny, double &maxy); \
	void Get##Data##Comp##Pts(NMRData* NMRDataPtr, size_t No, wxPoint* PointArray, NFGScale Scale);

	GET_DATA_IdxRng_DECL(TDD, Time)
	GET_DATA_Xdom_DECL(TDD, Time)
	GET_DATA_Val_DECL(TDD, Time, Real)
	GET_DATA_Val_DECL(TDD, Time, Imag)
	double GetTDDAmp(NMRData* NMRDataPtr, size_t No, size_t Index); 
	wxPoint GetTDDAmpPt(NMRData* NMRDataPtr, size_t No, size_t Index, NFGScale Scale); 
	wxRealPoint GetTDDAmpRPt(NMRData* NMRDataPtr, size_t No, size_t Index); 
	void GetTDDAmpRPtBB(NMRData* NMRDataPtr, size_t No, double &minx, double &maxx, double &miny, double &maxy); 
	void GetTDDAmpPts(NMRData* NMRDataPtr, size_t No, wxPoint* PointArray, NFGScale Scale); 

#define GET_DATA_DECL(Data, Xdom) 	\
	GET_DATA_IdxRng_DECL(Data, Xdom) \
	GET_DATA_Xdom_DECL(Data, Xdom) 	\
	GET_DATA_Val_DECL(Data, Xdom, Real) 	\
	GET_DATA_Val_DECL(Data, Xdom, Imag) 	\
	GET_DATA_Val_DECL(Data, Xdom, Amp) 

	GET_DATA_DECL(ChunkAvg, Time)
	
	GET_DATA_DECL(DFTProcNoFilterPhaseCorr, Freq)
	
	GET_DATA_IdxRng_DECL(EchoPeaksEnvelope, Time) 
	GET_DATA_Xdom_DECL(EchoPeaksEnvelope, Time) 
	GET_DATA_Val_DECL(EchoPeaksEnvelope, Time, Amp) 
	
#undef GET_DATA_DECL
#undef GET_DATA_IdxRng_DECL
#undef GET_DATA_Xdom_DECL
#undef GET_DATA_Val_DECL


#define GET_DATA_DFTEnv_IdxRng_DECL(Data, Xdom) 	\
	size_t Get##Data##IndexRange(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed)); 
#define GET_DATA_DFTEnv_Xdom_DECL(Data, Xdom) 	\
	double Get##Data##Xdom(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index); 
#define GET_DATA_DFTEnv_Val_DECL(Data, Xdom, Comp) 	\
	double Get##Data##Comp(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index); \
	wxPoint Get##Data##Comp##Pt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index, NFGScale Scale); \
	wxRealPoint Get##Data##Comp##RPt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index); \
	void Get##Data##Comp##RPtBB(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), double &minx, double &maxx, double &miny, double &maxy); \
	void Get##Data##Comp##Pts(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), wxPoint* PointArray, NFGScale Scale); 
	
	size_t GetDFTEnvelopeNoRange(NMRData* WXUNUSED(NMRDataPtr)); 
	unsigned long GetDFTEnvelopeFlag(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed)); 
	GET_DATA_DFTEnv_IdxRng_DECL(DFTEnvelope, Freq) 
	GET_DATA_DFTEnv_Xdom_DECL(DFTEnvelope, Freq) 
	GET_DATA_DFTEnv_Val_DECL(DFTEnvelope, Freq, Amp) 
	
	size_t GetDFTRealEnvelopeNoRange(NMRData* WXUNUSED(NMRDataPtr)); 
	unsigned long GetDFTRealEnvelopeFlag(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed)); 
	GET_DATA_DFTEnv_IdxRng_DECL(DFTRealEnvelope, Freq) 
	GET_DATA_DFTEnv_Xdom_DECL(DFTRealEnvelope, Freq) 
	GET_DATA_DFTEnv_Val_DECL(DFTRealEnvelope, Freq, Real) 

#undef GET_DATA_DFTEnv_IdxRng_DECL
#undef GET_DATA_DFTEnv_Xdom_DECL
#undef GET_DATA_DFTEnv_Val_DECL

	
#define GET_DATA_Eval_DECL(Data) 	\
	double Get##Data(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index); \
	wxPoint Get##Data##Pt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index, NFGScale Scale); \
	wxRealPoint Get##Data##RPt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t Index); \
	void Get##Data##RPtBB(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), double &minx, double &maxx, double &miny, double &maxy); \
	void Get##Data##Pts(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), wxPoint* PointArray, NFGScale Scale); 
	
	unsigned long GetEvaluationFlag(NMRData* WXUNUSED(NMRDataPtr), size_t WXUNUSED(NotUsed)); 
	size_t GetEvaluationIndexRange(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed)); 
	
	GET_DATA_Eval_DECL(DFTMeanPhaseCorrReal)
	GET_DATA_Eval_DECL(DFTPhaseCorrRealAtZero)
	GET_DATA_Eval_DECL(DFTMeanPhaseCorrAmp)
	GET_DATA_Eval_DECL(DFTPhaseCorrAmpAtZero)
	GET_DATA_Eval_DECL(ChunkAvgMaxAmp)
	GET_DATA_Eval_DECL(ChunkAvgIntAmp)
	
#undef GET_DATA_Eval_DECL
	
#define GET_DATA_Eval_FreqMax_DECL(Data) 	\
	double Get##Data(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t index); \
	wxPoint Get##Data##Pt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t index, NFGScale Scale); \
	wxRealPoint Get##Data##RPt(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), size_t index); \
	void Get##Data##RPtBB(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), double &minx, double &maxx, double &miny, double &maxy); \
	void Get##Data##Pts(NMRData* NMRDataPtr, size_t WXUNUSED(NotUsed), wxPoint* PointArray, NFGScale Scale); 
	
	GET_DATA_Eval_FreqMax_DECL(DFTMaxPhaseCorrReal)
	GET_DATA_Eval_FreqMax_DECL(DFTMaxPhaseCorrAmp)
	
#undef GET_DATA_Eval_FreqMax_DECL
};

#endif
