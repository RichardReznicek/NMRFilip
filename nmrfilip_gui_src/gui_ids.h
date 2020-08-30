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

#ifndef __gui_ids__
#define __gui_ids__

enum NFGID 
{
	DatasetIDMin = wxID_HIGHEST + 1, 
	
	ID_TDDReal = DatasetIDMin, 
	ID_TDDImag, 
	ID_TDDModule, 
	ID_TDDEchoPeaksEnvelope, 
	ID_ChunkAvgReal, 
	ID_ChunkAvgImag, 
	ID_ChunkAvgModule, 
	ID_FFTReal, 
	ID_FFTImag, 
	ID_FFTModule, 
	ID_SpectrumFFTEnvelope, 
	ID_SpectrumParticularFFTModules, 
	ID_SpectrumFFTRealEnvelope, 
	ID_SpectrumParticularFFTRealParts, 
	ID_EvaluationFFTMax, 
	ID_EvaluationFFT0, 
	ID_EvaluationFFTMean, 
	ID_EvaluationFFTRealMax, 
	ID_EvaluationFFTReal0, 
	ID_EvaluationFFTRealMean, 
	ID_EvaluationChunkAvgModMax, 
	ID_EvaluationChunkAvgModInt, 
	
	DatasetIDMax = ID_EvaluationChunkAvgModInt, 
	
	BT_NONE, 
	BT_SERFILE, 
	BT_TDDFILE, 
	BT_USERLIST, 
	BT_TEXTFILE, 
	BT_DIR, 
	
	ID_MainNotebook, 
	
	ID_RefreshBrowseTree, 

	ID_SelectedStepNumber, 
	ID_SelectedStepState, 
	ID_SelectedStepLabel, 

	ID_xCoordST, 
	ID_yCoordST, 
	ID_dxCoordST, 
	ID_yratioCoordST, 

	ID_ZoomOutH, 
	ID_ZoomInH, 
	ID_AutoZoomH, 
	ID_AutoZoomHAll, 
	ID_ZoomOutV, 
	ID_ZoomInV, 
	ID_AutoZoomV, 
	ID_AutoZoomVAll, 

	ID_DisplayToolbook, 
	

	ID_Tile, 
	ID_Cascade, 
	ID_WindowsListBox, 
	
	
	ID_Process_Panel, 
	
	ID_DefaultFirstLastChunk,  
	ID_DefaultFirstLastChunkPoint,  
	ID_EnableFirstLastChunk,  
	ID_EnableFirstLastChunkPoint, 
	ID_FilterEnable, 
	
	ID_FirstChunk, 
	ID_LastChunk, 
	ID_FirstChunkPoint, 
	ID_LastChunkPoint, 
	ID_FFTLength, 
	ID_Filter, 
	
	ID_FirstChunkSpin, 
	ID_LastChunkSpin, 
	ID_FirstChunkPointSpin, 
	ID_LastChunkPointSpin, 
	ID_FFTLengthSpin, 
	ID_FilterSpin, 
	
	ID_FFTEnvelopeOmit, 
	ID_ScaleFirstTDPointEnable, 
	
	ID_ProcessLoad, 
	ID_ProcessStore, 
	ID_ProcessApply, 
	ID_ProcessRevert, 
	ID_ProcessAutoApply, 
	
	
	ID_Phase_Panel, 
	
	ID_PhaseCorr0, 
	ID_PhaseCorr1, 
	ID_PhaseCorr0SameValuesForAll, 
	ID_PhaseCorr0AutoAllTogether, 
	ID_PhaseCorr0Auto, 
	ID_PhaseCorr0FollowAuto, 
	ID_PhaseCorr0PilotStep, 
	ID_PhaseCorr0SetAllAuto, 
	ID_PhaseCorr0SetAllManual, 
	ID_PhaseCorr1SameValuesForAll, 
	ID_PhaseCorr1Apply, 
	ID_RemoveOffset, 
	ID_PhaseCorr0Spin, 
	ID_PhaseCorr1Spin, 
	
	ID_PhaseLoad, 
	ID_PhaseStore, 
	ID_PhaseApply, 
	ID_PhaseRevert, 
	ID_PhaseAutoApply, 
	
	
	ID_Plot_Panel, 
	
	ID_PlotAllAutoH, 
	ID_PlotSelectedAutoH, 
	ID_PlotManualH, 
	ID_MinimumH, 
	ID_MaximumH, 
	ID_PlotAllAutoV, 
	ID_PlotSelectedAutoV, 
	ID_PlotManualV, 
	ID_MinimumV, 
	ID_MaximumV, 
	ID_DrawPoints, 
	ID_ThickLines, 
	ID_MinimumHLoad, 
	ID_MinimumHStore, 
	ID_MaximumHLoad, 
	ID_MaximumHStore, 
	ID_MinimumVLoad, 
	ID_MinimumVStore, 
	ID_MaximumVLoad, 
	ID_MaximumVStore, 

	ID_PlotLoad, 
	ID_PlotStore, 
	ID_PlotApply, 
	ID_PlotRevert, 


	ID_Views_Panel, 
	
	ID_ViewName, 
	ID_StoredViews, 
	ID_LoadView, 
	ID_StoreView, 
	ID_LoadOtherView, 
	ID_DeleteView,


	ID_Export_Panel, 
	
	ID_ExportTDD, 
	ID_ExportEchoPeaksEnvelope, 
	ID_ExportChunkAvg, 
	ID_ExportFFT, 
	ID_ExportFFTEnvelope, 
	ID_ExportFFTRealEnvelope, 
	ID_ExportEvaluation, 

	ID_ExportTDD2, 
	ID_ExportEchoPeaksEnvelope2, 
	ID_ExportChunkAvg2, 
	ID_ExportFFT2, 
	ID_ExportFFTEnvelope2, 
	ID_ExportFFTRealEnvelope2, 
	ID_ExportEvaluation2, 

	ID_ExportTDD_FilePicker, 
	ID_ExportEchoPeaksEnvelope_FilePicker, 
	ID_ExportChunkAvg_FilePicker, 
	ID_ExportFFT_FilePicker, 
	ID_ExportFFTEnvelope_FilePicker, 
	ID_ExportFFTRealEnvelope_FilePicker, 
	ID_ExportEvaluation_FilePicker, 

	
	ID_Print_Panel, 
	
	ID_Print, 
	ID_PrintPS, 
	ID_PrintEPS, 
	ID_PrintMetafile2Clipboard, 
	ID_PrintPNG, 
	ID_PrintBitmap2Clipboard, 

	
	ID_UserlistGen, 
	ID_ValueList, 
	ID_Start, 
	ID_End, 
	ID_Number, 
	ID_Step, 
	ID_Coefficient, 
	ID_Sum, 
	ID_VariableName, 
	ID_DoNotTune, 
	ID_TuneOnce, 
	ID_TuneEvery, 
	ID_TuneEveryValue, 
	
	ID_UserlistSave, 
	ID_UserlistRevert,
	ID_UserlistMainPanel

};

#endif
