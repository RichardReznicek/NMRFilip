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

#ifndef __nmrfilipcmn__
#define __nmrfilipcmn__

#ifdef __cplusplus

#if __cplusplus >= 201103L
#include <cstdint>
#else 
#include <stdint.h>
#endif

#include <cstddef>
#include <cmath>

#ifdef __WIN32__
#ifdef BUILD_DLL
/** DLL export **/
#define EXPORT extern "C" __declspec(dllexport)
#else
/** EXE import **/
#define EXPORT extern "C" __declspec(dllimport)
#endif
#else
#define EXPORT extern "C"
#endif

#else

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#ifdef __WIN32__
#ifdef BUILD_DLL
/** DLL export **/
#define EXPORT extern __declspec(dllexport)
#else
/** EXE import **/
#define EXPORT extern __declspec(dllimport)
#endif
#else
#define EXPORT extern
#endif

#endif

/** Function return values **/
#define DATA_OK			(0 << 0)
#define DATA_OLD		(1 << 0)
#define DATA_VOID		(2 << 0)
#define DATA_EMPTY		(4 << 0)
#define DATA_INVALID		(8 << 0)

#define FILE_LOADED_OK		(0 << 4)
#define FILE_NOT_CLOSED		(1 << 4)
#define FILE_OPEN_ERROR		(2 << 4)
#define FILE_WRONG_SIZE		(4 << 4)
#define FILE_IO_ERROR		(8 << 4)

#define INVALID_PARAMETER	(1 << 8)
#define MEM_ALLOC_ERROR		(2 << 8)
#define NMR_DATA_STRUCT_VOID	(4 << 8)
#define ERROR_REPORT_VOID	(8 << 8)

#define ERROR_REPORTED		(1 << 12)


/** Associated value flags **/
#define ASSOC_NOT_SET		0
#define ASSOC_VARIABLE		1
#define ASSOC_PWR_DB		2
#define ASSOC_TRIG_S		3
#define ASSOC_INVREC_S		4	/** variable D3 **/
#define ASSOC_T2_S		5	/** variable D3 **/
#define ASSOC_NUTATION_US	6	/** variable P1 **/
#define ASSOC_FREQ_MHZ		7

#define ASSOC_UserlistHighest	ASSOC_FREQ_MHZ


#define ASSOC_VALIST		(1 << 8)
#define ASSOC_VCLIST		(2 << 8)
#define ASSOC_VDLIST		(4 << 8)
#define ASSOC_VPLIST		(8 << 8)
#define ASSOC_VTLIST		(16 << 8)

#define ASSOC_VLIST_MASK	(31 << 8)

#define ASSOC_FQ1LIST		(1 << 16)
#define ASSOC_FQ2LIST		(2 << 16)
#define ASSOC_FQ3LIST		(4 << 16)
#define ASSOC_FQ4LIST		(8 << 16)
#define ASSOC_FQ5LIST		(16 << 16)
#define ASSOC_FQ6LIST		(32 << 16)
#define ASSOC_FQ7LIST		(64 << 16)
#define ASSOC_FQ8LIST		(128 << 16)

#define ASSOC_FQLIST_MASK	(255 << 16)

/** Step order options for userlist **/
#define STEP_ORDER_SEQUENTIAL	0	/** from zero upward **/
#define STEP_ORDER_REVERSED	1	/** from last step downward **/
#define STEP_ORDER_INTERLACED	2	/** even steps upward, then odd steps downward **/
#define STEP_ORDER_EXPANDING	3	/** from the middle alternately in both directions **/
#define STEP_ORDER_SPREAD	4	/** attempting even coverage of the step range throughout the run **/ 

#define STEP_ORDER_Highest	STEP_ORDER_SPREAD

/** AcquParams flags **/
#define ACQU_None		0
#define ACQU_Counts		1
#define ACQU_Userlist		2
#define ACQU_vlist		4
#define ACQU_Acqus		8

/** Step flags **/
#define STEP_BLANK		8
#define STEP_IGNORE		4
#define STEP_NO_ENVELOPE	2
#define STEP_NO_SHOW		1
#define STEP_OK			0

/** Phase flags **/
#define PHASE0_Manual		0
#define PHASE0_Auto		1
#define PHASE0_AutoAllTogether	2
#define PHASE0_FollowAuto	4
#define PHASE1_Manual		(0 << 4)
/** the following flags are not implemented **/
#define PHASE1_Auto		(1 << 4)
#define PHASE1_AutoAllTogether	(2 << 4)
#define PHASE1_FollowAuto	(4 << 4)


/** Parameters in the first group should be set/checked in the following order: **/
#define PROC_PARAM_FirstChunk			1
#define PROC_PARAM_LastChunk			2
#define PROC_PARAM_ChunkStart			3
#define PROC_PARAM_ChunkEnd			4
#define PROC_PARAM_DFTLength			5
#define PROC_PARAM_ScaleFirstTDPoint		6
#define PROC_PARAM_RemoveOffset			7
#define PROC_PARAM_Filter			8

#define PROC_PARAM_PhaseCorr0			9
#define PROC_PARAM_PhaseCorr0Manual		9
#define PROC_PARAM_PhaseCorr0Auto		10
#define PROC_PARAM_PhaseCorr0AutoAllTogether	11
#define PROC_PARAM_PhaseCorr0FollowAuto		12

#define PROC_PARAM_PhaseCorr1ManualRefDataStart	13
#define PROC_PARAM_PhaseCorr1ManualRefProcStart	14
#define PROC_PARAM_PhaseCorr1Ref		15
/** reserved, not implemented: **/
#define PROC_PARAM_PhaseCorr1Auto		0
#define PROC_PARAM_PhaseCorr1AutoAllTogether	0
#define PROC_PARAM_PhaseCorr1FollowAuto		0

#define PROC_PARAM_StepFlag			19
#define PROC_PARAM_SetStepFlag			20
#define PROC_PARAM_ClearStepFlag		21


/** NMR data types **/
#define CHECK_AcquParams	0
#define CHECK_RawData		1
#define CHECK_StepSet		2
#define CHECK_ChunkSet		3
#define CHECK_ChunkAvg		4
#define CHECK_DFTResult		5
#define CHECK_DFTPhaseCorrPrep	6
#define 	CHECK_DFTPhaseCorrPrep_AutoCorr		7
#define 	CHECK_DFTPhaseCorrPrep_MemReIm		8
#define 	CHECK_DFTPhaseCorrPrep_MemAmp		9
#define CHECK_DFTPhaseCorr	10
#define 	CHECK_DFTPhaseCorr_ReIm			11
#define 	CHECK_DFTPhaseCorr_Amp			12
#define CHECK_AcquInfo		13
#define CHECK_EchoPeaksEnvelope	14
#define CHECK_DFTEnvelope	15
#define CHECK_DFTRealEnvelope	16
#define CHECK_Evaluation	17
#define 	CHECK_Evaluation_ChunkAvgAmp		18
#define 	CHECK_Evaluation_DFTAmp			19
#define 	CHECK_Evaluation_DFTPhaseCorrReal	20
#define 	CHECK_Evaluation_DFTPhaseCorrAmp	21

#define HighestNMRDataType	CHECK_Evaluation_DFTPhaseCorrAmp

#define Flag(N)	(1ul << (N))


#define ALL_STEPS	(-1)


/** Text data export constants **/
#define EXPORT_AcquInfo			0
#define EXPORT_ProcParams		1
#define EXPORT_TDD			2
#define EXPORT_ChunkSet			3
#define EXPORT_ChunkAvg			4
#define EXPORT_DFTResult		5
#define EXPORT_DFTPhaseCorrResult	6
#define EXPORT_DFTEnvelope		7
#define EXPORT_DFTPhaseCorrRealEnvelope	8
#define EXPORT_EchoPeaksEnvelope	9
#define EXPORT_Evaluation		10

#define EXPORT_Highest	EXPORT_Evaluation

/** Parameter type flags **/
#define PARAM_NONE	0
#define PARAM_LONG	1
#define PARAM_ULONG	2
#define PARAM_DOUBLE	4
#define PARAM_STRING	8


#define PARAM_SET_SWh		(8 << 0)
#define PARAM_SET_ByteOrder	(4 << 0)
#define PARAM_SET_PointLine	(2 << 0)
#define PARAM_SET_TimeDomain	(1 << 0)
#define PARAM_SET_Freq		(8 << 8)

#define PARAM_SET_MASK		(PARAM_SET_SWh | PARAM_SET_ByteOrder | PARAM_SET_PointLine | PARAM_SET_TimeDomain | PARAM_SET_Freq)


typedef struct {
	intptr_t start;
	size_t length;	/** in 2x long (Re, Im) (8 B) **/
} SignalWindow;


typedef struct {
	unsigned long Flags;	/** flags of valid data parts **/
	unsigned long StepFlag;	/** user flag indicating step usability **/
	
	/** Basic parameters **/
	double AssocValue;
	double Freq;
	
	/** Raw data of the step **/
	int32_t *RawData;
	size_t RawDataLength; 	/** in 2x long (Re, Im) (8 B) **/

	/** Chunk average **/
	double *ChunkAvgData;	/** pointer to start of the whole (Re, Im) chunk average field **/
	double *ChunkAvgAmp;	/** pointer to start of the whole chunk average amplitude field **/
	size_t ChunkAvgLength;	/** in 2x double (Re, Im) (16 B) - length of the whole chunk average field **/

	/** Echo peaks envelope **/
	double *EchoPeaksEnvelope;
	size_t EchoPeaksEnvelopeLength;	/** in 2x long (Re, Im) (8 B) **/

	/** DFT data **/
	double *DFTInput;	/** pointer to start of the whole (Re, Im) DFT input field **/
	size_t DFTInputLength;	/** in 2x double (Re, Im) (16 B) - length of the whole DFT input field **/
	
	/** NOTE: 
	1. Output data are not normalized.
	2. Use the corresponding macro for data access or see the FFTW Reference for a description of the output ordering. **/
	double *DFTOutput;	/** pointer to start of the whole (Re, Im) DFT output field **/
	double *DFTOutAmp;	/** pointer to start of the whole DFT amplitude field **/
	size_t DFTLength;	/** in 2x double (Re, Im) (16 B) - length of the whole DFT field **/

	/** Phase-correction parameters and results **/
	unsigned char PhaseCorrFlag;
	long PhaseCorr0;	/** additional phase shift in units of 0.001 deg **/
	long PhaseCorr1;	/** time position of FID origin or echo center in units of 1 ns **/
	long PhaseCorr1Ref;	/** reference point in chunk average data (-1 for proc start, 0 for data start) **/

	double *DFTPhaseCorrOutput;	/** pointer to start of the whole (Re, Im) phase- and offset-corrected DFT output **/
	double *DFTPhaseCorrOutAmp;	/** pointer to start of the whole phase- and offset-corrected DFT amplitude **/

	/** Evaluation parameters **/
	double ChunkAvgAmpMax;	/** maximum of amplitude of the processed part of the chunk average **/
	double ChunkAvgAmpInt;	/** integral of amplitude of the processed part of the chunk average **/
	
	double DFTAmpMax;	/** maximum of amplitude of the processed part of the DFT output **/
	size_t DFTAmpMaxPoint;
	double DFTAmpMean;	/** integral of amplitude of the processed part of the DFT output **/
	
	double DFTPhaseCorrRealMax;	/** maximum of real part of the processed part of the DFT output **/
	size_t DFTPhaseCorrRealMaxPoint;
	double DFTPhaseCorrRealMean;	/** integral of real part of the processed part of the DFT output **/
	
	double DFTPhaseCorrAmpMax;	/** maximum of amplitude of the processed part of the DFT output **/
	size_t DFTPhaseCorrAmpMaxPoint;
	double DFTPhaseCorrAmpMean;	/** integral of amplitude of the processed part of the DFT output **/
} StepStruct;



typedef struct {
	unsigned char AcquFlag;
	
	/** Info acquired from data processing **/
	long StepCount;
	long ChunkCount;
	
	/** Experiment info from ulist/userlist file for setting values associated to particular steps - not applicable in the case of fid file data **/
	unsigned long AssocValueType;
	char *AssocValueTypeName;
	char *AssocValueVariable;
	char *AssocValueUnits;
	double AssocValueStart;
	double AssocValueStep;
	double AssocValueCoef;
	double *AssocValues;	/** set of values provided by v?list/fq?list or ulist **/
	size_t AssocValuesLength;
	
	double AssocValueMin;
	double AssocValueMax;
	
	/** wobbler is started if (StepNo%WobbStep == 0) => 
		if (WobbStep == StepCount) "Tuned before start" else 
		if (WobbStep > StepCount) "Not tuned" else 
		if (WobbStep < StepCount) "Tuned every {WobbStep} step(s)" 
		(WobbStep <= 0 not allowed) **/
	long WobbStep;

	/** General info from acqus file **/
	char *Title;	/** TITLE - contains acqus parameter file generator name (xwin-nmr, topspin) and version **/
	long Date;	/** $DATE - timestamp of the end of acquisition (of the first step in the case of ulist/userlist-based experiments) **/
	
	/** Important acqu params */
	double Freq;	/** $SFO1 - frequency [MHz] - usually ignore if AssocValueType == ASSOC_FREQ_MHZ **/
	double SWh;	/** $SW_h - sample rate [Hz] **/
	char *PulProg;	/** $PULPROG - pulse program name **/
	
	long NS;		/** $NS - number of scans **/
	
	double *D;
	size_t Dlength;
	/** $D [1] - recycle delay (trigger) [s] - might be confusing if AssocValueType == ASSOC_TRIG_S **/
	/** $D [3] - time for pulse ringdown [s] **/
	/** $D [6] - duration of FID (half of chunk window) [s] **/
	
	double *P;
	size_t Plength;
	/** $P [1] - spin echo 90 degree pulse length [us] **/
	/** $P [2] - spin echo 180 degree pulse length [us] **/
	/** $P [3] - CPMG 90 degree pulse length [us] **/
	/** $P [4] - CPMG 180 degree pulse length [us] **/
	
	double *PL;
	size_t PLlength;
	/** $PL [21] - RF power level [dB] - might be confusing if AssocValueType == ASSOC_PWR_DB **/

	double *PLW;
	size_t PLWlength;
	/** $PLW - RF power levels in Watt - might be confusing if AssocValueType == ASSOC_PWR_DB **/

	double RG;	/** $RG - receiver gain - might be confusing if AssocValueType == ASSOC_RG **/
	
	double FW;	/** $FW - filter width [Hz] **/

	double DE;	/** $DE - pre-scan delay [us] **/
	
	/** parameters related to the digital filter used during the acquisition **/
	long DigMod; 	/** $DIGMOD - digitizer mode: analog, digital, homodecoupling-digital, baseopt **/
	long DSPFirm;	/** $DSPFIRM - firmware: sharp, user_defined, smooth, medium, rectangle **/
	long DSPFVS;	/** $DSPFVS - firmware version: 10 to 23 **/
	long Decim;	/** $DECIM - decimation factor **/
	double GrpDly;	/** $GRPDLY - group delay **/
	
} AcquParams;

#ifdef __cplusplus
extern "C" {
#endif
typedef char*		(*ErrorReportFunc)(void*, int, char*);
typedef char*		(*ErrorReportCustomFunc)(void*, char*, char*);
typedef int		(*MarkNMRDataOldCallbackFunc)(void*, unsigned long, long);
typedef int		(*ChangeProcParamCallbackFunc)(void*, unsigned int, long);
#ifdef __cplusplus
}
#endif

typedef struct {
	/** Parameter file **/
	char *AcqusData;
	size_t AcqusLength;
	
	/** Datafile **/
	char *SerName;
	char ByteOrder;	/** $BYTORDA - datafile byte order: 1 -> little endian; 0 -> big endian (crucial parameter) **/
	char DTypA;	/** $DTYPA - raw data are integers (0) or doubles (2) **/
	
	/** Raw data loaded from ser file **/
	int32_t *DataSpace;
	size_t DataSize;
	
	size_t TimeDomain;	/** $TD - time domain (crucial parameter) **/
	size_t PointLine;	/** PointLine - line length in 2x long (Re, Im) (8 B), including padding zeroes **/

	double SWMh;	/** Sampling spectral width [MHz] (crucial parameter) **/
	
	/** Parameters for handling digital filter artifacts - based on the digital filter setting used during the acquisition **/
	size_t SkipPoints;	/** number of points containing filter artifacts **/
	double TimeOffset;	/** time offset of the first valid point relative to actual acqusition start [us] **/
	
	/** Processing params **/
	unsigned long FirstChunk;
	unsigned long LastChunk;
	unsigned long ChunkStart;
	unsigned long ChunkEnd;
	unsigned long DFTLength;
	unsigned long FilterHz;
	unsigned long filter;
	unsigned long filter2;

	/** Do not use both at once **/
	unsigned char ScaleFirstTDPoint;
	unsigned char RemoveOffset;
	
	/** Structures with pointers to data of particular steps **/
	StepStruct *Steps;
	size_t StepCount;
	unsigned long Flags;
	
	/** Structures with pseudo-pointers to starts of particular chunks in step **/
	SignalWindow *ChunkSet;
	size_t ChunkCount;
	
	/** Sorted array of DFT envelope points **/
	double *DFTEnvelopeArray;
	size_t DFTEnvelopeCount;
	
	/** Sorted array of phase-corrected real DFT envelope points **/
	double *DFTRealEnvelopeArray;
	size_t DFTRealEnvelopeCount;
	
	/** Structure with the most important acqusition parameters **/
	AcquParams AcquInfo;
	
	/** Application-dependent function pointers **/
	ErrorReportFunc ErrorReport;
	ErrorReportCustomFunc ErrorReportCustom;
	MarkNMRDataOldCallbackFunc MarkNMRDataOldCallback;
	ChangeProcParamCallbackFunc ChangeProcParamCallback;
	
	/** Auxiliary application-dependent pointers and values **/
	void *AuxPointer;
	long AuxLong;
	unsigned long AuxFlag;
	
} NMRData;


/** Used just for userlist reading/writing. **/
typedef struct {
	/** mandatory: **/
	unsigned long AssocValueType;
	char *AssocValueVariable;	/** used if AssocValueType == ASSOC_VARIABLE **/
	
	/** mandatory (unless AssocValues is used and no legacy compatibility is needed): **/
	double AssocValueStart;
	double AssocValueStep;
	double AssocValueCoef;
	
	/** optional: **/
	double *AssocValues;
	
	/** mandatory: **/
	long StepCount;
	
	/** optional: **/
	unsigned long StepOrder;
	
	/** wobbler is started if (StepNo%WobbStep == 0) => 
		if (WobbStep == StepCount) "Tuned before start" else 
		if (WobbStep > StepCount) "Not tuned" else 
		if (WobbStep < StepCount) "Tuned every {WobbStep} step(s)" 
		(WobbStep <= 0 not allowed) **/
	long WobbStep;
	
	char *Destination;	/** required if legacy compatibility is needed **/
	
	char *RunBeforeExpWrk;
	char *RunBeforeExpDst;
	char *RunAfterExpWrk;
	char *RunAfterExpDst;
	char *RunBeforeStepWrk;
	char *RunBeforeStepDst;
	char *RunAfterStepWrk;
	char *RunAfterStepDst;
	
} UserlistParams;


/** Data access macros **/
/** time in us, frequency in MHz **/

#define StepNoRange(NMRDataPtr)						((NMRDataPtr)->StepCount)
#define StepAssocType(NMRDataPtr, StepNo)				((NMRDataPtr)->AcquInfo.AssocValueType)
#define StepAssocValue(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].AssocValue)
#define StepFreq(NMRDataPtr, StepNo)					(((NMRDataPtr)->Steps)[StepNo].Freq)
#define StepFlag(NMRDataPtr, StepNo)					(((NMRDataPtr)->Steps)[StepNo].StepFlag)

#define TDDIndexRange(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].RawDataLength)
#define TDDDataStart(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].RawData)
#define TDDTime(NMRDataPtr, StepNo, Index)				(((double) (Index) - (double) ((NMRDataPtr)->SkipPoints)) / (NMRDataPtr)->SWMh + (NMRDataPtr)->TimeOffset)
#define TDDReal(NMRDataPtr, StepNo, Index)				((((NMRDataPtr)->Steps)[StepNo].RawData)[2*(Index) + 0])
#define TDDImag(NMRDataPtr, StepNo, Index)				((((NMRDataPtr)->Steps)[StepNo].RawData)[2*(Index) + 1])

#ifdef __cplusplus
#define TDDAmp(NMRDataPtr, StepNo, Index)				(std::sqrt(((double) TDDReal((NMRDataPtr), (StepNo), (Index)))*((double) TDDReal((NMRDataPtr), (StepNo), (Index))) + ((double) TDDImag((NMRDataPtr), (StepNo), (Index)))*((double) TDDImag((NMRDataPtr), (StepNo), (Index)))))
#else
#define TDDAmp(NMRDataPtr, StepNo, Index)				(sqrt(((double) TDDReal((NMRDataPtr), (StepNo), (Index)))*((double) TDDReal((NMRDataPtr), (StepNo), (Index))) + ((double) TDDImag((NMRDataPtr), (StepNo), (Index)))*((double) TDDImag((NMRDataPtr), (StepNo), (Index)))))
#endif


#define ChunkNoRange(NMRDataPtr)					((NMRDataPtr)->ChunkCount)
#define ChunkIndexRange(NMRDataPtr, ChunkNo)				(((NMRDataPtr)->ChunkSet)[ChunkNo].length)
#define ChunkDataStart(NMRDataPtr, ChunkNo)				(((NMRDataPtr)->ChunkSet)[ChunkNo].start)
#define ChunkTime(NMRDataPtr, StepNo, ChunkNo, Index)			TDDTime((NMRDataPtr), (StepNo), (Index) + ChunkDataStart((NMRDataPtr), (ChunkNo))/2)
#define ChunkReal(NMRDataPtr, StepNo, ChunkNo, Index)			((((NMRDataPtr)->Steps)[StepNo].RawData + ((NMRDataPtr)->ChunkSet)[ChunkNo].start)[2*(Index) + 0])
#define ChunkImag(NMRDataPtr, StepNo, ChunkNo, Index)			((((NMRDataPtr)->Steps)[StepNo].RawData + ((NMRDataPtr)->ChunkSet)[ChunkNo].start)[2*(Index) + 1])



#define ChunkAvgIndexRange(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].ChunkAvgLength)
#define ChunkAvgProcIndexRange(NMRDataPtr, StepNo)			((((NMRDataPtr)->Steps)[StepNo].ChunkAvgLength > 0)?((NMRDataPtr)->ChunkEnd + 1 - (NMRDataPtr)->ChunkStart):(0))
#define ChunkAvgIsProc(NMRDataPtr, StepNo, Index)			(((Index) >= (NMRDataPtr)->ChunkStart) && ((Index) <= (NMRDataPtr)->ChunkEnd))
#define ChunkAvgTime(NMRDataPtr, StepNo, Index)				(((double) (Index)) / (NMRDataPtr)->SWMh + (NMRDataPtr)->TimeOffset)
#define ChunkAvgDataStart(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].ChunkAvgData)
#define ChunkAvgProcStart(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].ChunkAvgData + 2*((NMRDataPtr)->ChunkStart))
#define ChunkAvgReal(NMRDataPtr, StepNo, Index)				((((NMRDataPtr)->Steps)[StepNo].ChunkAvgData)[2*(Index) + 0])
#define ChunkAvgImag(NMRDataPtr, StepNo, Index)				((((NMRDataPtr)->Steps)[StepNo].ChunkAvgData)[2*(Index) + 1])
#define ChunkAvgAmp(NMRDataPtr, StepNo, Index)				((((NMRDataPtr)->Steps)[StepNo].ChunkAvgAmp)[Index])
#define ChunkAvgDataAmpStart(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].ChunkAvgAmp)
#define ChunkAvgProcAmpStart(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].ChunkAvgAmp + ((NMRDataPtr)->ChunkStart))
#define ChunkAvgProcReal(NMRDataPtr, StepNo, Index)			((((NMRDataPtr)->Steps)[StepNo].ChunkAvgData + 2*((NMRDataPtr)->ChunkStart))[2*(Index) + 0])
#define ChunkAvgProcImag(NMRDataPtr, StepNo, Index)			((((NMRDataPtr)->Steps)[StepNo].ChunkAvgData + 2*((NMRDataPtr)->ChunkStart))[2*(Index) + 1])
#define ChunkAvgProcAmp(NMRDataPtr, StepNo, Index)			((((NMRDataPtr)->Steps)[StepNo].ChunkAvgAmp + ((NMRDataPtr)->ChunkStart))[Index])
#define ChunkAvgMaxAmp(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].ChunkAvgAmpMax)
#define ChunkAvgIntAmp(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].ChunkAvgAmpInt)


/** "raw" index **/
#define DFTIndexRange(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].DFTLength)
#define DFTProcIndexRange(NMRDataPtr, StepNo)				((NMRDataPtr)->Steps[StepNo].DFTLength - (NMRDataPtr)->filter - (NMRDataPtr)->filter2)
#define DFTProcNoFilterIndexRange(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].DFTLength)

#define DFTPhaseCorrIndexRange(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].DFTLength)
#define DFTProcPhaseCorrIndexRange(NMRDataPtr, StepNo)			((NMRDataPtr)->Steps[StepNo].DFTLength - (NMRDataPtr)->filter - (NMRDataPtr)->filter2)
#define DFTProcNoFilterPhaseCorrIndexRange(NMRDataPtr, StepNo)		(((NMRDataPtr)->Steps)[StepNo].DFTLength)

#define DFTPhaseCorrFlag(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].PhaseCorrFlag)
#define DFTPhaseCorr0(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].PhaseCorr0)
#define DFTPhaseCorr1(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1)
#define DFTPhaseCorr1Ref(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1Ref)
/** relative to processed chunk average data start **/
#ifdef __cplusplus
#define DFTPhaseCorr1Relative(NMRDataPtr, StepNo)			((((NMRDataPtr)->Steps)[StepNo].PhaseCorr1Ref)?(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1):(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1 - std::lround((((double) ((NMRDataPtr)->ChunkStart))/(NMRDataPtr)->SWMh + (NMRDataPtr)->TimeOffset)*1.0e3)))
#define DFTPhaseCorr1Absolute(NMRDataPtr, StepNo)			((((NMRDataPtr)->Steps)[StepNo].PhaseCorr1Ref == 0)?(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1):(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1 + std::lround((((double) ((NMRDataPtr)->ChunkStart))/(NMRDataPtr)->SWMh + (NMRDataPtr)->TimeOffset)*1.0e3)))
#else
#define DFTPhaseCorr1Relative(NMRDataPtr, StepNo)			((((NMRDataPtr)->Steps)[StepNo].PhaseCorr1Ref)?(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1):(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1 - lround((((double) ((NMRDataPtr)->ChunkStart))/(NMRDataPtr)->SWMh + (NMRDataPtr)->TimeOffset)*1.0e3)))
#define DFTPhaseCorr1Absolute(NMRDataPtr, StepNo)			((((NMRDataPtr)->Steps)[StepNo].PhaseCorr1Ref == 0)?(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1):(((NMRDataPtr)->Steps)[StepNo].PhaseCorr1 + lround((((double) ((NMRDataPtr)->ChunkStart))/(NMRDataPtr)->SWMh + (NMRDataPtr)->TimeOffset)*1.0e3)))
#endif

#define DFTFreq(NMRDataPtr, StepNo, rawIndex)				((((rawIndex) > ((NMRDataPtr)->Steps[StepNo].DFTLength / 2))?((double) (rawIndex) - (double) (NMRDataPtr)->Steps[StepNo].DFTLength):((double) (rawIndex))) * (NMRDataPtr)->SWMh / ((double) (NMRDataPtr)->Steps[StepNo].DFTLength) + (NMRDataPtr)->Steps[StepNo].Freq)

#define DFTReal(NMRDataPtr, StepNo, rawIndex)				((((NMRDataPtr)->Steps)[StepNo].DFTOutput)[2*(rawIndex) + 0])
#define DFTImag(NMRDataPtr, StepNo, rawIndex)				((((NMRDataPtr)->Steps)[StepNo].DFTOutput)[2*(rawIndex) + 1])
#define DFTAmp(NMRDataPtr, StepNo, rawIndex)				((((NMRDataPtr)->Steps)[StepNo].DFTOutAmp)[rawIndex])
#define DFTPhaseCorrReal(NMRDataPtr, StepNo, rawIndex)			((((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrOutput)[2*(rawIndex) + 0])
#define DFTPhaseCorrImag(NMRDataPtr, StepNo, rawIndex)			((((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrOutput)[2*(rawIndex) + 1])
#define DFTPhaseCorrAmp(NMRDataPtr, StepNo, rawIndex)			((((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrOutAmp)[rawIndex])


#define DFTIndexToRawIndexNoFilter(NMRDataPtr, StepNo, Index) 		((((NMRDataPtr)->Steps[StepNo].DFTLength)/2 + 1 + (Index))%((NMRDataPtr)->Steps[StepNo].DFTLength))
#define DFTIndexToRawIndexWithFilter(NMRDataPtr, StepNo, Index) 	((((NMRDataPtr)->Steps[StepNo].DFTLength)/2 + 1 + (Index) + (NMRDataPtr)->filter)%((NMRDataPtr)->Steps[StepNo].DFTLength))

#define DFTIsProcNoFilter(NMRDataPtr, StepNo, Index)			(((Index) >= (NMRDataPtr)->filter) && ((Index) < (DFTProcNoFilterIndexRange((NMRDataPtr), (StepNo)) - (NMRDataPtr)->filter2)))

#define DFTProcNoFilterFreq(NMRDataPtr, StepNo, Index)			((double) ((long) (Index) - ((long) (NMRDataPtr)->Steps[StepNo].DFTLength - 1)/2) * ((NMRDataPtr)->SWMh / ((double) (NMRDataPtr)->Steps[StepNo].DFTLength)) + (NMRDataPtr)->Steps[StepNo].Freq)
#define DFTProcNoFilterPhaseCorrFreq(NMRDataPtr, StepNo, Index)	DFTProcNoFilterFreq((NMRDataPtr), (StepNo), (Index))

#define DFTProcNoFilterReal(NMRDataPtr, StepNo, Index)			DFTReal(NMRDataPtr, StepNo, DFTIndexToRawIndexNoFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcNoFilterImag(NMRDataPtr, StepNo, Index)			DFTImag(NMRDataPtr, StepNo, DFTIndexToRawIndexNoFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcNoFilterAmp(NMRDataPtr, StepNo, Index)			DFTAmp(NMRDataPtr, StepNo, DFTIndexToRawIndexNoFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcNoFilterPhaseCorrReal(NMRDataPtr, StepNo, Index)	DFTPhaseCorrReal(NMRDataPtr, StepNo, DFTIndexToRawIndexNoFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcNoFilterPhaseCorrImag(NMRDataPtr, StepNo, Index)	DFTPhaseCorrImag(NMRDataPtr, StepNo, DFTIndexToRawIndexNoFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcNoFilterPhaseCorrAmp(NMRDataPtr, StepNo, Index)		DFTPhaseCorrAmp(NMRDataPtr, StepNo, DFTIndexToRawIndexNoFilter((NMRDataPtr), (StepNo), (Index)))

#define ChooseMax(a, b)							(((a) > (b))?(a):(b))

#define DFTProcFreq(NMRDataPtr, StepNo, Index)				((double) ((long) (Index) + (long) (NMRDataPtr)->filter - ((long) (NMRDataPtr)->Steps[StepNo].DFTLength - 1)/2) * ((NMRDataPtr)->SWMh / ((double) (NMRDataPtr)->Steps[StepNo].DFTLength)) + (NMRDataPtr)->Steps[StepNo].Freq)
#define DFTProcReal(NMRDataPtr, StepNo, Index)				DFTReal(NMRDataPtr, StepNo, DFTIndexToRawIndexWithFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcImag(NMRDataPtr, StepNo, Index)				DFTImag(NMRDataPtr, StepNo, DFTIndexToRawIndexWithFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcAmp(NMRDataPtr, StepNo, Index)				DFTAmp(NMRDataPtr, StepNo, DFTIndexToRawIndexWithFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcPhaseCorrReal(NMRDataPtr, StepNo, Index)			DFTPhaseCorrReal(NMRDataPtr, StepNo, DFTIndexToRawIndexWithFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcPhaseCorrImag(NMRDataPtr, StepNo, Index)			DFTPhaseCorrImag(NMRDataPtr, StepNo, DFTIndexToRawIndexWithFilter((NMRDataPtr), (StepNo), (Index)))
#define DFTProcPhaseCorrAmp(NMRDataPtr, StepNo, Index)			DFTPhaseCorrAmp(NMRDataPtr, StepNo, DFTIndexToRawIndexWithFilter((NMRDataPtr), (StepNo), (Index)))

/** raw index **/
#define DFTMaxAmpIndex(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].DFTAmpMaxPoint)
#define DFTMaxAmp(NMRDataPtr, StepNo)					(((NMRDataPtr)->Steps)[StepNo].DFTAmpMax)
#define DFTMeanAmp(NMRDataPtr, StepNo)					(((NMRDataPtr)->Steps)[StepNo].DFTAmpMean)
#define DFTAmpAtZero(NMRDataPtr, StepNo)				((((NMRDataPtr)->Steps)[StepNo].DFTOutAmp)[0])
#define DFTMaxPhaseCorrRealIndex(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrRealMaxPoint)
#define DFTMaxPhaseCorrReal(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrRealMax)
#define DFTMeanPhaseCorrReal(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrRealMean)
#define DFTPhaseCorrRealAtZero(NMRDataPtr, StepNo)			((((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrOutput)[0])
#define DFTMaxPhaseCorrAmpIndex(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrAmpMaxPoint)
#define DFTMaxPhaseCorrAmp(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrAmpMax)
#define DFTMeanPhaseCorrAmp(NMRDataPtr, StepNo)				(((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrAmpMean)
#define DFTPhaseCorrAmpAtZero(NMRDataPtr, StepNo)			((((NMRDataPtr)->Steps)[StepNo].DFTPhaseCorrOutAmp)[0])

#ifdef __cplusplus
#define DFTFreqToFilter(NMRDataPtr, Freq)				((((Freq) < (((NMRDataPtr)->SWMh)/2)) && ((Freq) > 0.0))?(ChooseMax(0, std::lround(std::ceil(- ((double) (NMRDataPtr)->DFTLength) / (NMRDataPtr)->SWMh * (Freq) + ((long) (NMRDataPtr)->DFTLength - 1)/2)))):(0))
#define DFTFreqToFilter2(NMRDataPtr, Freq)				((((Freq) < (((NMRDataPtr)->SWMh)/2)) && ((Freq) > 0.0))?(ChooseMax(0, std::lround(std::ceil(- ((double) (NMRDataPtr)->DFTLength) / (NMRDataPtr)->SWMh * (Freq) + (NMRDataPtr)->DFTLength/2)))):(0))
#else
#define DFTFreqToFilter(NMRDataPtr, Freq)				((((Freq) < (((NMRDataPtr)->SWMh)/2)) && ((Freq) > 0.0))?(ChooseMax(0, lround(ceil(- ((double) (NMRDataPtr)->DFTLength) / (NMRDataPtr)->SWMh * (Freq) + ((long) (NMRDataPtr)->DFTLength - 1)/2)))):(0))
#define DFTFreqToFilter2(NMRDataPtr, Freq)				((((Freq) < (((NMRDataPtr)->SWMh)/2)) && ((Freq) > 0.0))?(ChooseMax(0, lround(ceil(- ((double) (NMRDataPtr)->DFTLength) / (NMRDataPtr)->SWMh * (Freq) + (NMRDataPtr)->DFTLength/2)))):(0))
#endif
/** set the filter frequency in between the points at the edge **/
#define DFTFilterToFreq(NMRDataPtr, filter)				((0.5 - (double) (filter) + ((long) (NMRDataPtr)->DFTLength - 1)/2) * ((NMRDataPtr)->SWMh / ((double) (NMRDataPtr)->DFTLength)))


#define EchoPeaksEnvelopeIndexRange(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].EchoPeaksEnvelopeLength)
#define EchoPeaksEnvelopeDataStart(NMRDataPtr, StepNo)			(((NMRDataPtr)->Steps)[StepNo].EchoPeaksEnvelope)
#define EchoPeaksEnvelopeTime(NMRDataPtr, StepNo, Index)		((((NMRDataPtr)->Steps)[StepNo].EchoPeaksEnvelope)[2*(Index) + 0])
#define EchoPeaksEnvelopeAmp(NMRDataPtr, StepNo, Index)			((((NMRDataPtr)->Steps)[StepNo].EchoPeaksEnvelope)[2*(Index) + 1])


#define DFTEnvelopeIndexRange(NMRDataPtr)				((NMRDataPtr)->DFTEnvelopeCount)
#define DFTEnvelopeFreq(NMRDataPtr, Index)				(((NMRDataPtr)->DFTEnvelopeArray)[2*(Index) + 0])
#define DFTEnvelopeAmp(NMRDataPtr, Index)				(((NMRDataPtr)->DFTEnvelopeArray)[2*(Index) + 1])

#define DFTRealEnvelopeIndexRange(NMRDataPtr)				((NMRDataPtr)->DFTRealEnvelopeCount)
#define DFTRealEnvelopeFreq(NMRDataPtr, Index)				(((NMRDataPtr)->DFTRealEnvelopeArray)[2*(Index) + 0])
#define DFTRealEnvelopeReal(NMRDataPtr, Index)				(((NMRDataPtr)->DFTRealEnvelopeArray)[2*(Index) + 1])


#define SWMh(NMRDataPtr)						((NMRDataPtr)->SWMh)

#endif
