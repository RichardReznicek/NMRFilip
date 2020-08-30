/* 
 * NMRFilip GUI - the NMR data processing software - graphical user interface
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

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cctype>

#include "doc.h"
#include <wx/dir.h>
#include <wx/printdlg.h>

#include "view.h"
#include "plotwin.h"
#include "plotgen.h"
#include "plotimp.h"
#include "print.h"
#include "nfgps.h"
#include "userlist.h"
#include "infopanel.h"
#include "nmrfilipgui.h"
#include "nmrdata.h"
#include "gui_ids.h"


IMPLEMENT_DYNAMIC_CLASS(NFGTextDocument, wxDocument)

/// Created document shall be immediatelly saved (or discarded otherwise)
bool NFGTextDocument::OnNewDocument() {
	wxString path = GetFilename();
	if (!path.empty()) {	/// setup up the title and save the fresh-made text file if the fllename is set
		wxFileName FN = wxFileName(path);
		FN.MakeAbsolute();
		wxArrayString dirs = FN.GetDirs();
		wxString title;
		
		for (size_t i = ((dirs.GetCount() >= 2)?(dirs.GetCount() - 2):(0)); i < dirs.GetCount(); i++) 
			title.Append(dirs.Item(i) + " | ");
		
		title.Append(FN.GetFullName());
		SetTitle(title);
		
		Modify(true);
		if (OnSaveDocument(path))
			return true;
	}
	
	Modify(false);
	return false;
}

bool NFGTextDocument::DoSaveDocument(const wxString& filename)
{
	NFGTextView *view = (NFGTextView *) GetFirstView();
	if (!view)
		return false;
	if (!(view->GetTextWin()))
		return false;
	
	if (!view->GetTextWin()->SaveFile(filename))
		return false;
	
	return true;
}

bool NFGTextDocument::DoOpenDocument(const wxString& filename)
{
	NFGTextView *view = (NFGTextView *) GetFirstView();
	if (!view)
		return false;
	if (!(view->GetTextWin()))
		return false;
	
	if (!view->GetTextWin()->LoadFile(filename))
		return false;
	
	if (!filename.empty()) {
		wxFileName FN = wxFileName(filename);
		FN.MakeAbsolute();
		wxArrayString dirs = FN.GetDirs();
		wxString title = wxEmptyString;
		
		for (size_t i = ((dirs.GetCount() >= 2)?(dirs.GetCount() - 2):(0)); i < dirs.GetCount(); i++) 
			title.Append(dirs.Item(i) + " | ");
		
		title.Append(FN.GetFullName());
		SetTitle(title);
	}
		
	Modify(false);
	return true;
}

bool NFGTextDocument::IsModified(void) const
{
	NFGTextView *view = (NFGTextView *) GetFirstView();
	
	if (view && view->GetTextWin())
		return (wxDocument::IsModified() || view->GetTextWin()->IsModified());
	
	return wxDocument::IsModified();
}

void NFGTextDocument::Modify(bool mod)
{
	NFGTextView *view = (NFGTextView *) GetFirstView();
	
	wxDocument::Modify(mod);
	
	if (!mod && view && view->GetTextWin())
		view->GetTextWin()->DiscardEdits();
}

bool NFGTextDocument::Revert()
{
	if (IsModified()) {
		if (::wxMessageBox("Reloading will discard unsaved changes made in document " + GetTitle() + ". Do you wish to proceed?", "Warning", wxYES_NO | wxICON_QUESTION) == wxNO)
			return false;
	}
	
	return OnOpenDocument(GetFilename());
}



char *NFGErrorReport(void *NMRDataStruct, int ErrorNumber, char *Activity) {
	char *ErrorDesc = std::strerror(ErrorNumber);

	if (errno == EINVAL)	/// invalid ErrorNumber
		return NULL;
	
	if (Activity != NULL) {
		wxLogError("%hs failed: %hs\n", Activity, ErrorDesc);
	} else {
		wxLogError("%hs\n", ErrorDesc);
	}
	
	return ErrorDesc;
}

char *NFGErrorReportCustom(void *NMRDataStruct, char *ErrorDesc, char *Activity) {
	if (Activity != NULL) {
		wxLogError("%hs failed: %hs\n", Activity, ErrorDesc);
	} else {
		wxLogError("%hs\n", ErrorDesc);
	}
	
	return ErrorDesc;
}

int MarkNMRDataOldCallbackFn(void* NMRDataStruct, unsigned long ClearedFlags, long StepNo) {
	if (NMRDataStruct != NULL) {
		NFGSerDocument *SerDoc = wxDynamicCast(((NMRData *) NMRDataStruct)->AuxPointer, NFGSerDocument);
		if (SerDoc != NULL)
			return SerDoc->MarkNMRDataOldCallback(ClearedFlags, StepNo);
	}
	
	return INVALID_PARAMETER;
}

int ChangeProcParamCallbackFn(void* NMRDataStruct, unsigned int ParamType, long StepNo) {
	if (NMRDataStruct != NULL) {
		NFGSerDocument *SerDoc = wxDynamicCast(((NMRData *) NMRDataStruct)->AuxPointer, NFGSerDocument);
		if (SerDoc != NULL)
			return SerDoc->ChangeProcParamCallback(ParamType, StepNo);
	}
	
	return INVALID_PARAMETER;
}


IMPLEMENT_DYNAMIC_CLASS(NFGSerDocument, wxDocument)

NFGSerDocument::NFGSerDocument()
{
	NFGNMRData::InitNMRData(&SerNMRData);
	SerNMRData.ErrorReport = NFGErrorReport;
	SerNMRData.ErrorReportCustom = NFGErrorReportCustom;
	SerNMRData.MarkNMRDataOldCallback = MarkNMRDataOldCallbackFn;
	SerNMRData.ChangeProcParamCallback = ChangeProcParamCallbackFn;
	SerNMRData.AuxPointer = this;
	
	SelectedGraphType = 0;
	Graph = NULL;
	GraphTDD = NULL;
	GraphChunkAvg = NULL;
	GraphFFT = NULL;
	GraphSpectrum = NULL;
	GraphEvaluation = NULL;
	
	params.UseFirstLastChunk = true;
	params.FirstChunk = 0;
	params.LastChunk = INT_MAX;
	params.UseFirstLastChunkPoint = true;
	params.FirstChunkPoint = 0;
	params.LastChunkPoint = INT_MAX;
	params.FFTLength = 128;
	params.UseFilter = false;
	params.Filter = 0.1;
	params.ScaleFirstTDPoint = false;
	params.AutoApply = false;
	
	PhaseParams.PhaseCorr0 = 0.0;
	PhaseParams.PhaseCorr1 = 0.0;
	PhaseParams.ZeroOrderSameValuesForAll = true;
	PhaseParams.ZeroOrderAutoAllTogether = false;
	PhaseParams.ZeroOrderAuto = false;
	PhaseParams.ZeroOrderFollowAuto = false;
	PhaseParams.PilotStep = 0;
	PhaseParams.ZeroOrderSetAllAuto = false;
	PhaseParams.ZeroOrderSetAllManual = false;
	PhaseParams.FirstOrderSameValuesForAll = true;
	PhaseParams.ApplyFirstOrderCorr = false;
	PhaseParams.RemoveOffset = false;
	PhaseParams.AutoApply = false;
	
	ParamsChanged = true;
	PhaseParamsChanged = true;
	ZParamsChanged = true;
	SelectedStepChanged = true;
	StepStatesChanged = true;
	ViewListChanged = true;

	FileName_ExportTDD = wxEmptyString;
	FileName_ExportEchoPeaksEnvelope = wxEmptyString;
	FileName_ExportChunkAvg = wxEmptyString;
	FileName_ExportFFT = wxEmptyString;
	FileName_ExportFFTEnvelope = wxEmptyString;
	FileName_ExportFFTRealEnvelope = wxEmptyString;
	FileName_ExportEvaluation = wxEmptyString;

}

NFGSerDocument::~NFGSerDocument()
{
	if (Graph != NULL)
		Graph->SelectedInto(NULL);
	
	delete GraphTDD;
	delete GraphChunkAvg;
	delete GraphFFT;
	delete GraphSpectrum;
	delete GraphEvaluation;
	
	NFGNMRData::FreeNMRData(&SerNMRData);
	SerNMRData.ErrorReport = NULL;	/// not really necessary here
	SerNMRData.ErrorReportCustom = NULL;	/// not really necessary here
	
	free(SerNMRData.SerName);
	SerNMRData.SerName = NULL;	/// not really necessary here
	
	ViewListChoices.Clear();
	ViewListPaths.Clear();
}

bool NFGSerDocument::DoSaveDocument(const wxString& filename)
{
	/// Do nothing
	return true;
}

bool NFGSerDocument::DoOpenDocument(const wxString& filename)
{
	NFGSerView *view = wxDynamicCast(GetFirstView(), NFGSerView);
	
	if (view == NULL)
		return false;
	
	wxFileName FN = wxFileName(filename);
	FN.MakeAbsolute();
	wxString path = FN.GetPathWithSep();
	PathName = wxFileName::DirName(path);

	/// free() is probably not necessary, unless a reuse of the SerNMRData structure takes place
	free(SerNMRData.SerName);
	if (GetDocumentTemplate()->GetDocumentName() == "Ser File")
		SerNMRData.SerName = strdup(wxString(path).Append("ser").char_str(*wxConvFileName));
	else	/// expecting "Fid File"
		SerNMRData.SerName = strdup(wxString(path).Append("fid").char_str(*wxConvFileName));

	/// Call CheckProcParam here for all params to get reasonable proc param values and preload the data.
	NFGNMRData::CheckProcParam(&SerNMRData, PROC_PARAM_FirstChunk, PARAM_LONG, &(params.FirstChunk), NULL);
	NFGNMRData::CheckProcParam(&SerNMRData, PROC_PARAM_LastChunk, PARAM_LONG, &(params.LastChunk), NULL);
	NFGNMRData::CheckProcParam(&SerNMRData, PROC_PARAM_ChunkStart, PARAM_LONG, &(params.FirstChunkPoint), NULL);
	NFGNMRData::CheckProcParam(&SerNMRData, PROC_PARAM_ChunkEnd, PARAM_LONG, &(params.LastChunkPoint), NULL);
	NFGNMRData::CheckProcParam(&SerNMRData, PROC_PARAM_DFTLength, PARAM_LONG, &(params.FFTLength), NULL);
	
	ParamsChanged = true;
	PhaseParamsChanged = true;
	ZParamsChanged = true;
	SelectedStepChanged = true;
	StepStatesChanged = true;


	GraphTDD = new NFGGraphTDD(&SerNMRData, this, GraphStyle_Line);
	GraphChunkAvg = new NFGGraphChunkAvg(&SerNMRData, this, GraphStyle_Line);
	GraphFFT = new NFGGraphFFT(&SerNMRData, this, GraphStyle_Line);
	GraphSpectrum = new NFGGraphSpectrum(&SerNMRData, this, GraphStyle_Line);
	GraphEvaluation = new NFGGraphEvaluation(&SerNMRData, this, GraphStyle_Line);
	
	
	wxFileName FName = FN;
	FName.AppendDir("export");
	wxString ExportPath = FName.GetPathWithSep();
	
	FileName_ExportTDD = ExportPath + "tddata.txt";
	FileName_ExportEchoPeaksEnvelope = ExportPath + "echopeaks.txt";
	FileName_ExportChunkAvg = ExportPath + "chunkavg.txt";
	FileName_ExportFFT = ExportPath + "fft.txt";
	FileName_ExportFFTEnvelope = ExportPath + "spectrum.txt";
	FileName_ExportFFTRealEnvelope = ExportPath + "realspectrum.txt";
	FileName_ExportEvaluation = ExportPath + "evaluation.txt";

	LoadViewList();
	
	if (view->GetGraphWin() != NULL) {
		view->GetGraphWin()->SelectGraph(GraphSpectrum);
		Graph = GraphSpectrum;
		SelectedGraphType = GraphTypeSpectrum;
	} else
		return false;
	
	if (!filename.empty()) {
		wxFileName FN = wxFileName(filename);
		FN.MakeAbsolute();
		wxArrayString dirs = FN.GetDirs();
		
		wxString title = wxEmptyString;
		
		for (int i = (dirs.GetCount() >= 2)?(((int) dirs.GetCount()) - 2):(0); (i >= 0) && (i < ((int) dirs.GetCount())); i++) {
			title.Append(dirs.Item(i));
			title.Append(" | ");
		}
		
		title.Append(FN.GetFullName());
		SetTitle(title);
	}

	return true;
}

bool NFGSerDocument::IsModified(void) const
{
	return wxDocument::IsModified();
}

int NFGSerDocument::MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo)
{
	if (GraphTDD != NULL)
		GraphTDD->MarkNMRDataOldCallback(ClearedFlags, StepNo);
	if (GraphChunkAvg != NULL)
		GraphChunkAvg->MarkNMRDataOldCallback(ClearedFlags, StepNo);
	if (GraphFFT != NULL)
		GraphFFT->MarkNMRDataOldCallback(ClearedFlags, StepNo);
	if (GraphSpectrum != NULL)
		GraphSpectrum->MarkNMRDataOldCallback(ClearedFlags, StepNo);
	if (GraphEvaluation != NULL)
		GraphEvaluation->MarkNMRDataOldCallback(ClearedFlags, StepNo);
	
	return DATA_OK;
}
	
int NFGSerDocument::ChangeProcParamCallback(unsigned int ParamType, long StepNo)
{
	if (GraphTDD != NULL)
		GraphTDD->ChangeProcParamCallback(ParamType, StepNo);
	if (GraphChunkAvg != NULL)
		GraphChunkAvg->ChangeProcParamCallback(ParamType, StepNo);
	if (GraphFFT != NULL)
		GraphFFT->ChangeProcParamCallback(ParamType, StepNo);
	if (GraphSpectrum != NULL)
		GraphSpectrum->ChangeProcParamCallback(ParamType, StepNo);
	if (GraphEvaluation != NULL)
		GraphEvaluation->ChangeProcParamCallback(ParamType, StepNo);

	/// Let the UI reflect the flag change
	SelectedStepChanged = SelectedStepChanged || ((Flag(ParamType) & (Flag(PROC_PARAM_StepFlag) | Flag(PROC_PARAM_SetStepFlag) | Flag(PROC_PARAM_ClearStepFlag))) && ((StepNo < 0) || (((unsigned long) StepNo) == Graph->GetSelectedStep())));

	StepStatesChanged = StepStatesChanged || ((Flag(ParamType) & (Flag(PROC_PARAM_StepFlag) | Flag(PROC_PARAM_SetStepFlag) | Flag(PROC_PARAM_ClearStepFlag))) && ((StepNo < 0) || (((unsigned long) StepNo) == Graph->GetSelectedStep())));
	
	return DATA_OK;
}


void NFGSerDocument::Modify(bool mod)
{
	wxDocument::Modify(false);
}

void NFGSerDocument::ZoomCommand(int command)
{
	if (Graph == NULL)
		return;
	
	switch (command) {
		case ID_ZoomOutH:
			Graph->Zoom(false, true, false);
			break;
		case ID_ZoomInH:
			Graph->Zoom(true, true, false);
			break;
		case ID_AutoZoomH:
			Graph->AutoScaleSelected(true, false);
			break;
		case ID_AutoZoomHAll:
			Graph->AutoScaleAll(true, false);
			break;
		case ID_ZoomOutV:
			Graph->Zoom(false, false, true);
			break;
		case ID_ZoomInV:
			Graph->Zoom(true, false, true);
			break;
		case ID_AutoZoomV:
			Graph->AutoScaleSelected(false, true);
			break;
		case ID_AutoZoomVAll:
			Graph->AutoScaleAll(false, true);
			break;
		
		default:
			;
	}
}


unsigned long NFGSerDocument::SetSelectedStep(unsigned long StepNo)
{
	if (Graph == NULL)
		return 0;
	
	Graph->SelectStep(StepNo);
	return Graph->GetSelectedStep();
}


unsigned long NFGSerDocument::GetSelectedStep()
{
	if (Graph == NULL)
		return 0;
	
	return Graph->GetSelectedStep();
}

wxString NFGSerDocument::GetSelectedStepState()
{
	if (Graph == NULL)
		return wxEmptyString;
	
	return Graph->GetSelectedStepState();
}

unsigned char NFGSerDocument::GetSelectedStepStateFlag()
{
	if (Graph == NULL)
		return 0;
	
	return Graph->GetSelectedStepStateFlag();
}

wxString NFGSerDocument::GetSelectedStepLabel()
{
	if (Graph == NULL)
		return wxEmptyString;
	
	return Graph->GetSelectedStepLabel();
}

bool NFGSerDocument::SelectedStepChangedQuery()
{
	bool RetVal = SelectedStepChanged;
	SelectedStepChanged = false;
	
	return RetVal;
}

void NFGSerDocument::SelectedStepChangedNotify()
{
	SelectedStepChanged = true;
	
	if ((!PhaseParams.ZeroOrderSameValuesForAll) || (!PhaseParams.FirstOrderSameValuesForAll))
		PhaseParamsChanged = true;
}

bool NFGSerDocument::StepStatesChangedQuery()
{
	bool RetVal = StepStatesChanged;
	StepStatesChanged = false;
	
	return RetVal;
}


wxString NFGSerDocument::CursorXQuery()
{
	if (Graph == NULL)
		return wxEmptyString;
	
	return Graph->GetCursorX();
}

wxString NFGSerDocument::CursorYQuery()
{
	if (Graph == NULL)
		return wxEmptyString;
	
	return Graph->GetCursorY();
}

wxString NFGSerDocument::CursorDeltaXQuery()
{
	if (Graph == NULL)
		return wxEmptyString;
	
	return Graph->GetCursorDeltaX();
}

wxString NFGSerDocument::CursorYRatioQuery()
{
	if (Graph == NULL)
		return wxEmptyString;
	
	return Graph->GetCursorYRatio();
}

void NFGSerDocument::CopyCursorPosCommand(wxString &CursorPosStr, bool Append)
{
	NFGDocManager *DocMan = wxDynamicCast(GetDocumentManager(), NFGDocManager);
	if (DocMan != NULL)
		DocMan->CopyCursorPos(CursorPosStr, Append);
}

unsigned char NFGSerDocument::GraphTypeCommand(unsigned char GraphType)
{
	if (GraphType == SelectedGraphType)
		return SelectedGraphType;

	NFGSerView *view = wxDynamicCast(GetFirstView(), NFGSerView);
	
	if (view == NULL)
		return SelectedGraphType;

	if (view->GetGraphWin() == NULL) 
		return SelectedGraphType;
		
	unsigned long Selected = GetSelectedStep();
	
	switch (GraphType) {
		case GraphTypeTDD:
			Graph = GraphTDD;
			SelectedGraphType = GraphTypeTDD;
			SetSelectedStep(Selected);
			view->GetGraphWin()->SelectGraph(GraphTDD);
			break;
		case GraphTypeChunkAvg:
			Graph = GraphChunkAvg;
			SelectedGraphType = GraphTypeChunkAvg;
			SetSelectedStep(Selected);
			view->GetGraphWin()->SelectGraph(GraphChunkAvg);
			break;
		case GraphTypeFFT:
			Graph = GraphFFT;
			SelectedGraphType = GraphTypeFFT;
			SetSelectedStep(Selected);
			view->GetGraphWin()->SelectGraph(GraphFFT);
			break;
		case GraphTypeSpectrum:
			Graph = GraphSpectrum;
			SelectedGraphType = GraphTypeSpectrum;
			SetSelectedStep(Selected);
			view->GetGraphWin()->SelectGraph(GraphSpectrum);
			break;
		case GraphTypeEvaluation:
			Graph = GraphEvaluation;
			SelectedGraphType = GraphTypeEvaluation;
			SetSelectedStep(Selected);
			view->GetGraphWin()->SelectGraph(GraphEvaluation);
			break;
		default:
			;
	}

	ZParamsChanged = true;	/// just to make sure ...
	
	return SelectedGraphType;
}

unsigned char NFGSerDocument::GraphTypeQuery()
{
	return SelectedGraphType;
}


unsigned long NFGSerDocument::DatasetDisplayCommand(unsigned long flag)
{
	if (Graph == NULL)
		return 0;
	
	return Graph->DisplayDatasets(flag);
}

unsigned long NFGSerDocument::DatasetDisplayQuery()
{
	if (Graph == NULL)
		return 0;
	
	return Graph->GetDisplayedDatasets();
}

ProcParams NFGSerDocument::SetProcParams(ProcParams Parameters, bool NoRefresh, bool Coerce)
{
	long Val = 0;
	int RetVal = DATA_OK;
	
	/// coerce the parameters (beyond what NMRFilip LIB does) to improve user experience
	if (Parameters.FirstChunk < 0) 
		Parameters.FirstChunk = 0;
	if (Parameters.FirstChunk > LastChunkQuery()) 
		Parameters.FirstChunk = LastChunkQuery();

	if (Parameters.LastChunk < 0) 
		Parameters.LastChunk = 0;
	if (Parameters.LastChunk > LastChunkQuery()) 
		Parameters.LastChunk = LastChunkQuery();
	
	if (Parameters.FirstChunkPoint < 0) 
		Parameters.FirstChunkPoint = 0;
	if (Parameters.FirstChunkPoint > LastChunkPointQuery()) 
		Parameters.FirstChunkPoint = LastChunkPointQuery();
	
	if (Parameters.LastChunkPoint < 0) 
		Parameters.LastChunkPoint = 0;
	if (Parameters.LastChunkPoint > LastChunkPointQuery()) 
		Parameters.LastChunkPoint = LastChunkPointQuery();
	
	if (Coerce) {
		if ((Parameters.UseFirstLastChunk) && (Parameters.LastChunk < Parameters.FirstChunk)) {
			if (Parameters.FirstChunk == params.FirstChunk)
				Parameters.FirstChunk = Parameters.LastChunk;
			else
			if (Parameters.LastChunk == params.LastChunk)
				Parameters.LastChunk = Parameters.FirstChunk;
			else {
				Val = Parameters.LastChunk;
				Parameters.LastChunk = Parameters.FirstChunk;
				Parameters.FirstChunk = Val;
			}
		}
		
		if ((Parameters.UseFirstLastChunkPoint) && (Parameters.LastChunkPoint < Parameters.FirstChunkPoint)) {
			if (Parameters.FirstChunkPoint == params.FirstChunkPoint)
				Parameters.FirstChunkPoint = Parameters.LastChunkPoint;
			else
			if (Parameters.LastChunkPoint == params.LastChunkPoint)
				Parameters.LastChunkPoint = Parameters.FirstChunkPoint;
			else {
				Val = Parameters.LastChunkPoint;
				Parameters.LastChunkPoint = Parameters.FirstChunkPoint;
				Parameters.FirstChunkPoint = Val;
			}
		}
	}

	params = Parameters;
	
	if (params.UseFirstLastChunk) {
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_FirstChunk, PARAM_LONG, &(params.FirstChunk), NULL);
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_LastChunk, PARAM_LONG, &(params.LastChunk), NULL);
	} else {
		Val = 0;
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_FirstChunk, PARAM_LONG, &Val, NULL);
		Val = INT_MAX;
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_LastChunk, PARAM_LONG, &Val, NULL);
	}

	if (params.UseFirstLastChunkPoint) {
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ChunkStart, PARAM_LONG, &(params.FirstChunkPoint), NULL);
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ChunkEnd, PARAM_LONG, &(params.LastChunkPoint), NULL);
	} else {
		Val = 0;
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ChunkStart, PARAM_LONG, &Val, NULL);
		Val = INT_MAX;
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ChunkEnd, PARAM_LONG, &Val, NULL);
	}

	RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_DFTLength, PARAM_LONG, &(params.FFTLength), NULL);

	if (params.UseFilter)
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_Filter, PARAM_DOUBLE, &(params.Filter), NULL);
	else {
		Val = 2000000000;
		RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_Filter, PARAM_LONG, &Val, NULL);
	}
	
	Val = (params.ScaleFirstTDPoint)?(1):(0);
	RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ScaleFirstTDPoint, PARAM_LONG, &Val, NULL);
	params.ScaleFirstTDPoint = Val;
	
	PhaseParamsChanged = true;
	
	if (!NoRefresh)
		CheckGraphData();	/// Make sure the plot is up to date

	/// get everything from SerNMRData if something went wrong
	return GetProcParams(true, RetVal == DATA_OK);
}


ProcParams NFGSerDocument::GetProcParams(bool MarkUnchanged, bool Quick)
{
	long Val = 0;

	if (!Quick) {	/// get everything from SerNMRData
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_FirstChunk, PARAM_LONG, &(params.FirstChunk), NULL);
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_LastChunk, PARAM_LONG, &(params.LastChunk), NULL);
		params.UseFirstLastChunk = (params.FirstChunk > 0) || (params.LastChunk < LastChunkQuery());

		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_ChunkStart, PARAM_LONG, &(params.FirstChunkPoint), NULL);
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_ChunkEnd, PARAM_LONG, &(params.LastChunkPoint), NULL);
		params.UseFirstLastChunkPoint = (params.FirstChunkPoint > 0) || (params.LastChunkPoint < LastChunkPointQuery());

		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_DFTLength, PARAM_LONG, &(params.FFTLength), NULL);

		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_Filter, PARAM_LONG, &Val, NULL);
		if (Val < 2000000000) {
			params.UseFilter = true;
			NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_Filter, PARAM_DOUBLE, &(params.Filter), NULL);
		} else
			params.UseFilter = false;
		
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_ScaleFirstTDPoint, PARAM_LONG, &Val, NULL);
		params.ScaleFirstTDPoint = Val;
		
		ParamsChanged = true;
	}

	if (MarkUnchanged)
		ParamsChanged = false;
	
	return params;
}

bool NFGSerDocument::ProcParamsChangedQuery(bool MarkUnchanged)
{
	bool RetVal = ParamsChanged;
	
	if (MarkUnchanged)
		ParamsChanged = false;
	
	return RetVal;
}


PhaseCorrParams NFGSerDocument::SetPhaseCorrParams(PhaseCorrParams Parameters, bool NoRefresh)
{
	unsigned long i = 0;
	long CurStep = GetSelectedStep();
	long Val = 0;
	int RetVal = DATA_OK;

	if ((Parameters.PilotStep > 0) && (((size_t) Parameters.PilotStep) >= StepNoRange(&SerNMRData)))
		Parameters.PilotStep = ((long)  StepNoRange(&SerNMRData)) - 1;
	
	if (Parameters.PilotStep < 0)
		Parameters.PilotStep = 0;


	if (Parameters.ZeroOrderSameValuesForAll) {
		if (Parameters.ZeroOrderAutoAllTogether) {
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0AutoAllTogether, PARAM_LONG, &Val, NULL);
			Parameters.ZeroOrderFollowAuto = false;
		} else
		if (Parameters.ZeroOrderFollowAuto) {
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0FollowAuto, PARAM_LONG, &Val, &(Parameters.PilotStep));
		} else 
		{	/// manual
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0Manual, PARAM_DOUBLE, &(Parameters.PhaseCorr0), NULL);
		}
		Parameters.ZeroOrderAuto = false;
	} else {
		if (Parameters.ZeroOrderSetAllAuto) {	/// set the flags accordingly
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0Auto, PARAM_LONG, &Val, NULL);
		} else 
		if (Parameters.ZeroOrderSetAllManual) {	/// set all the flags accordingly + set the selected step manually
			for (i = 0; i < StepNoRange(&SerNMRData); i++) 
				DFTPhaseCorrFlag(&SerNMRData, i) = (DFTPhaseCorrFlag(&SerNMRData, i) & 0xF0) | PHASE0_Manual;

			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0Manual, PARAM_DOUBLE, &(Parameters.PhaseCorr0), &CurStep);
		} else 
		if (Parameters.ZeroOrderAuto) {	/// check all the flags + set the flag of the selected step accordingly
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0Auto, PARAM_LONG, &Val, &CurStep);
		} else
		{	/// check all the flags + set the flag of the selected step accordingly + set the step manually
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0Manual, PARAM_DOUBLE, &(Parameters.PhaseCorr0), &CurStep);
		}
	}

	if (Parameters.FirstOrderSameValuesForAll) {
		if (Parameters.ApplyFirstOrderCorr) {
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr1ManualRefDataStart, PARAM_DOUBLE, &(Parameters.PhaseCorr1), NULL);
		} else {
			Val = 0;
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr1ManualRefProcStart, PARAM_LONG, &Val, NULL);
		}
	} else {
		if (Parameters.ApplyFirstOrderCorr) {
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr1ManualRefDataStart, PARAM_DOUBLE, &(Parameters.PhaseCorr1), &CurStep);
		} else {
			Val = 0;
			RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr1ManualRefProcStart, PARAM_LONG, &Val, &CurStep);
		}
	}

	
	Val = (Parameters.RemoveOffset)?(1):(0);
	RetVal |= NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_RemoveOffset, PARAM_LONG, &Val, NULL);
	
	if (Parameters.RemoveOffset && params.ScaleFirstTDPoint) {
		RetVal |= NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_ScaleFirstTDPoint, PARAM_LONG, &Val, NULL);
		params.ScaleFirstTDPoint = Val;
		ParamsChanged = true;
	}
	
	PhaseParams = Parameters;
	
	PhaseParams.ZeroOrderSetAllAuto = false;
	PhaseParams.ZeroOrderSetAllManual = false;

	if (!NoRefresh)
		CheckGraphData();	/// Make sure the plot is up to date
	
	/// get the flag and corr. values for selected step 
	/// get everything based on SerNMRData if something went wrong
	return GetPhaseCorrParams(true, RetVal == DATA_OK);
}


PhaseCorrParams NFGSerDocument::GetPhaseCorrParams(bool MarkUnchanged, bool Quick)
{
	size_t i = 0;
	long CorrVal = 0;
	long RefVal = 0;
	long Val = 0;
	long Step = 0;
	long CurStep = 0;

	if (!Quick) {	/// get everything based on SerNMRData
		PhaseParams.PhaseCorr0 = 0.0;
		PhaseParams.PhaseCorr1 = 0.0;
		PhaseParams.ApplyFirstOrderCorr = false;
		PhaseParams.ZeroOrderSameValuesForAll = false;
		PhaseParams.FirstOrderSameValuesForAll = false;
		PhaseParams.ZeroOrderAutoAllTogether = false;
		PhaseParams.ZeroOrderAuto = false;
		PhaseParams.ZeroOrderFollowAuto = false;
		PhaseParams.PilotStep = 0;
		PhaseParams.ZeroOrderSetAllAuto = false;
		PhaseParams.ZeroOrderSetAllManual = false;

		if (NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0FollowAuto, PARAM_LONG, &Val, &Step) == DATA_OK) {
			if (Val) {
				PhaseParams.ZeroOrderFollowAuto = true;
				PhaseParams.ZeroOrderSameValuesForAll = true;			
				PhaseParams.PilotStep = Step;
			}
		}
		
		if (!PhaseParams.ZeroOrderFollowAuto) {
			if (NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0AutoAllTogether, PARAM_LONG, &Val, NULL) == DATA_OK) {
				if (Val) {
					PhaseParams.ZeroOrderAutoAllTogether = true;
					PhaseParams.ZeroOrderSameValuesForAll = true;			
				}
			}
		}
		
		if ((!PhaseParams.ZeroOrderFollowAuto) && (!PhaseParams.ZeroOrderAutoAllTogether) && (StepNoRange(&SerNMRData) > 0)) {
			CorrVal = DFTPhaseCorr0(&SerNMRData, 0);
			PhaseParams.ZeroOrderSameValuesForAll = true;			
			for (i = 0; i < StepNoRange(&SerNMRData); i++) {
				if ((DFTPhaseCorr0(&SerNMRData, i) != CorrVal) || ((DFTPhaseCorrFlag(&SerNMRData, i) & 0x0F) != PHASE0_Manual)) {
					PhaseParams.ZeroOrderSameValuesForAll = false;
					break;
				}
			}
		}
		
		if (StepNoRange(&SerNMRData) > 0) {
			CorrVal = DFTPhaseCorr1(&SerNMRData, 0);
			RefVal = DFTPhaseCorr1Ref(&SerNMRData, 0);
			PhaseParams.FirstOrderSameValuesForAll = true;			
			for (i = 0; i < StepNoRange(&SerNMRData); i++) {
				if ((DFTPhaseCorr1(&SerNMRData, i) != CorrVal) || (DFTPhaseCorr1Ref(&SerNMRData, i) != RefVal) || ((DFTPhaseCorrFlag(&SerNMRData, i) & 0xF0) != PHASE1_Manual)) {
					PhaseParams.FirstOrderSameValuesForAll = false;
					break;
				}
			}
		}
		
		PhaseParamsChanged = true;
	}
	
	/// get the flag and corr. values for Selected exp.
	CurStep = GetSelectedStep();
	if (((unsigned long) CurStep) < StepNoRange(&SerNMRData)) {
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr0, PARAM_DOUBLE, &(PhaseParams.PhaseCorr0), &CurStep);
		
		if ((DFTPhaseCorr1(&SerNMRData, CurStep) == 0) && (DFTPhaseCorr1Ref(&SerNMRData, CurStep) == (-1))) {
			PhaseParams.PhaseCorr1 = 0.0;
			PhaseParams.ApplyFirstOrderCorr = false;
		} else {
			NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_PhaseCorr1ManualRefDataStart, PARAM_DOUBLE, &(PhaseParams.PhaseCorr1), &CurStep);
			PhaseParams.ApplyFirstOrderCorr = true;
		}
		
		PhaseParams.ZeroOrderAuto = ((!PhaseParams.ZeroOrderSameValuesForAll) && ((DFTPhaseCorrFlag(&SerNMRData, CurStep) & 0x0F) == PHASE0_Auto));
	} else {
		PhaseParams.PhaseCorr0 = 0.0;
		PhaseParams.PhaseCorr1 = 0.0;
		PhaseParams.ApplyFirstOrderCorr = false;
		PhaseParams.ZeroOrderAuto = false;
	}

	if ((PhaseParams.PilotStep > 0) && (((size_t) PhaseParams.PilotStep) >= StepNoRange(&SerNMRData)))
		PhaseParams.PilotStep = ((long)  StepNoRange(&SerNMRData)) - 1;
	
	if (PhaseParams.PilotStep < 0)
		PhaseParams.PilotStep = 0;
	
	NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_RemoveOffset, PARAM_LONG, &Val, NULL);
	PhaseParams.RemoveOffset = Val;

	if (MarkUnchanged)
		PhaseParamsChanged = false;
	
	return PhaseParams;
}

bool NFGSerDocument::PhaseCorrParamsChangedQuery(bool MarkUnchanged)
{
	bool RetVal = PhaseParamsChanged;
	
	if (MarkUnchanged)
		PhaseParamsChanged = false;
	
	return RetVal;
}


ZoomParams NFGSerDocument::SetZoomParams(ZoomParams Parameters)
{
	ZoomParams params;
	/// just set some reasonable default values
	params.AutoZoomHAll = true;
	params.AutoZoomHSelected = false;
	params.AutoZoomVAll = true;
	params.AutoZoomVSelected = false;
	params.DrawPoints = false;
	params.ThickLines = false;
	params.minx = 0.0;
	params.miny = 0.0;
	params.maxx = 1.0e3;
	params.maxy = 1.0e3;
	
	if (Graph == NULL)
		return params;
	
	return Graph->SetZoomParams(Parameters);
}


ZoomParams NFGSerDocument::GetZoomParams()
{
	ZoomParams params;
	/// just some reasonable default values
	params.AutoZoomHAll = true;
	params.AutoZoomHSelected = false;
	params.AutoZoomVAll = true;
	params.AutoZoomVSelected = false;
	params.DrawPoints = false;
	params.ThickLines = false;
	params.minx = 0.0;
	params.miny = 0.0;
	params.maxx = 1.0e3;
	params.maxy = 1.0e3;
	
	ZParamsChanged = false;
	
	if (Graph == NULL)
		return params;
	
	return Graph->GetZoomParams();
}

bool NFGSerDocument::ZoomParamsChangedQuery()
{
	bool RetVal = ZParamsChanged;
	ZParamsChanged = false;
	
	return RetVal;
}

void NFGSerDocument::ZoomParamsChangedNotify()
{
	ZParamsChanged = true;
}


long NFGSerDocument::LastChunkQuery()
{
	if (NFGNMRData::CheckNMRData(&SerNMRData, CHECK_ChunkSet, ALL_STEPS) != DATA_OK) 
		return INT_MAX;
		
	return (ChunkNoRange(&SerNMRData) > 0)?(ChunkNoRange(&SerNMRData) - 1):(0);
}


long NFGSerDocument::LastChunkPointQuery()
{
	if (NFGNMRData::CheckNMRData(&SerNMRData, CHECK_ChunkAvg, 0) != DATA_OK) 
		return INT_MAX;
	
	if (StepNoRange(&SerNMRData) > 0)
		return (ChunkAvgIndexRange(&SerNMRData, 0) > 0)?(ChunkAvgIndexRange(&SerNMRData, 0) - 1):(0);
	else
		return 0;
}


unsigned long NFGSerDocument::StepCountQuery()
{
	if (NFGNMRData::CheckNMRData(&SerNMRData, CHECK_StepSet, ALL_STEPS) != DATA_OK)
		return 0;
	
	return StepNoRange(&SerNMRData);
}


wxString NFGSerDocument::StepLabelQuery(unsigned long n)
{
	if (NFGNMRData::CheckNMRData(&SerNMRData, CHECK_StepSet, ALL_STEPS) != DATA_OK)
		return wxEmptyString;
	
	if (n < StepNoRange(&SerNMRData)) {
		wxString Label = wxString::Format("%ld : %.10g", n, StepAssocValue(&SerNMRData, n));

		if (NFGNMRData::CheckNMRData(&SerNMRData, CHECK_AcquParams, ALL_STEPS) == DATA_OK) {
			if (SerNMRData.AcquInfo.AssocValueUnits != NULL) 
				Label += " " + wxString(SerNMRData.AcquInfo.AssocValueUnits);
		}
		
		return Label;
	} else
		return wxEmptyString;
}


bool NFGSerDocument::StepNoFFTEnvelopeQuery(unsigned long n)
{
	if (NFGNMRData::CheckNMRData(&SerNMRData, CHECK_StepSet, ALL_STEPS) != DATA_OK)
		return false;
	
	if (n < StepNoRange(&SerNMRData)) 
		return (StepFlag(&SerNMRData, n) & 0x0f & STEP_NO_ENVELOPE);
	else
		return false;
}


void NFGSerDocument::StepNoFFTEnvelopeCommand(unsigned long n, bool omit, bool NoRefresh)
{
	long Step = n;
	long Val = STEP_NO_ENVELOPE;

	if (omit) 
		NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_SetStepFlag, PARAM_LONG, &Val, &Step);
	else
		NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ClearStepFlag, PARAM_LONG, &Val, &Step);

	if (!NoRefresh)
		CheckGraphData();	/// Make sure the plot is up to date
}


int NFGSerDocument::ClipboardExport(unsigned int DataType)
{
	wxString buf = wxEmptyString;
	char *auxbuf = NULL;
	size_t auxlen = 0;
	
	int RetVal = NFGNMRData::DataToText(&SerNMRData, NULL, &auxbuf, &auxlen, DataType);

	if ((RetVal == DATA_OK) && (auxbuf != NULL) && (auxlen != 0)) 
		buf = wxString(auxbuf, auxlen);
	
	free(auxbuf);
	auxbuf = NULL;
	auxlen = 0;
	
	/// Copy to clipboard
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(buf));
		wxTheClipboard->Close();
	}

	return RetVal;
}


int NFGSerDocument::TextExport(wxString filename, unsigned int DataType)
{
	int RetVal = DATA_OK;
	
	wxFileName FName(filename);

	if (!FName.MakeAbsolute())
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	
	if (FName.IsDir())
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	
	wxFileName Path = wxFileName::DirName(FName.GetPath());
	
	if (!FName.IsOk())
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);

	if (!Path.IsOk())
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	
	if (!FName.FileExists()) {
		/// maybe the dir also doesn't exist
		if (!Path.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
			return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	
		/// just in case
		if (!Path.DirExists())
			return (FILE_OPEN_ERROR | INVALID_PARAMETER);
		
		/// just in case
		if (!Path.IsDirWritable())
			return (FILE_OPEN_ERROR | INVALID_PARAMETER);
		
	} else {
		
		if (!FName.IsFileWritable())
			return FILE_OPEN_ERROR;
		
		if (::wxMessageBox("File \'" + FName.GetFullPath() + "\' already exists. \nPress OK if you wish to overwrite it, otherwise cancel the operation.", "Confirm", wxOK | wxCANCEL | wxICON_QUESTION) != wxOK)
			return (FILE_OPEN_ERROR | DATA_OLD);
		
	}
	
	/// open the file
	wxFFile file(FName.GetFullPath(), "w");

	if (!file.IsOpened())
		return FILE_OPEN_ERROR;
	
	if (file.Error()) {
		file.Close();
		return FILE_OPEN_ERROR;
	}
	

	switch (DataType) {
		case EXPORT_TDD:
			FileName_ExportTDD = filename;
			break;
		case EXPORT_ChunkAvg:
			FileName_ExportChunkAvg = filename;
			break;
		case EXPORT_DFTResult:
		case EXPORT_DFTPhaseCorrResult:
			FileName_ExportFFT = filename;
			break;
		case EXPORT_Evaluation:
			FileName_ExportEvaluation = filename;
			break;
		case EXPORT_DFTEnvelope:
			FileName_ExportFFTEnvelope = filename;
			break;
		case EXPORT_DFTPhaseCorrRealEnvelope:
			FileName_ExportFFTRealEnvelope = filename;
			break;
		case EXPORT_EchoPeaksEnvelope:
			FileName_ExportEchoPeaksEnvelope = filename;
			break;
		default:
			;
	}


	switch (DataType) {
		case EXPORT_TDD:
		case EXPORT_ChunkSet:
/*			RetVal |= NFGNMRData::DataToText(&SerNMRData, file.fp(), NULL, NULL, EXPORT_AcquInfo);
			RetVal |= NFGNMRData::DataToText(&SerNMRData, file.fp(), NULL, NULL, DataType);
			break;
*/		
		case EXPORT_ChunkAvg:
		case EXPORT_DFTResult:
		case EXPORT_DFTPhaseCorrResult:
		case EXPORT_Evaluation:
		case EXPORT_DFTEnvelope:
		case EXPORT_DFTPhaseCorrRealEnvelope:
		case EXPORT_EchoPeaksEnvelope:
			RetVal |= NFGNMRData::DataToText(&SerNMRData, file.fp(), NULL, NULL, EXPORT_AcquInfo);
			RetVal |= NFGNMRData::DataToText(&SerNMRData, file.fp(), NULL, NULL, EXPORT_ProcParams);
			RetVal |= NFGNMRData::DataToText(&SerNMRData, file.fp(), NULL, NULL, DataType);
			break;
		
		case EXPORT_AcquInfo:
		case EXPORT_ProcParams:
		default:
			RetVal |= NFGNMRData::DataToText(&SerNMRData, file.fp(), NULL, NULL, DataType);
	}
	
	if (!file.Close())
		RetVal |= FILE_NOT_CLOSED;
	
	return RetVal;
}

wxString NFGSerDocument::TextExportFilenameQuery(unsigned int DataType)
{
	switch (DataType) {
		case EXPORT_TDD:
			return FileName_ExportTDD;
		case EXPORT_ChunkAvg:
			return FileName_ExportChunkAvg;
		case EXPORT_DFTResult:
		case EXPORT_DFTPhaseCorrResult:
			return FileName_ExportFFT;
		case EXPORT_Evaluation:
			return FileName_ExportEvaluation;
		case EXPORT_DFTEnvelope:
			return FileName_ExportFFTEnvelope;
		case EXPORT_DFTPhaseCorrRealEnvelope:
			return FileName_ExportFFTRealEnvelope;
		case EXPORT_EchoPeaksEnvelope:
			return FileName_ExportEchoPeaksEnvelope;
		default:
			;
	}
	
	return wxEmptyString;
}


bool NFGSerDocument::LoadViewList()
{
	ViewListChanged = true;

	ViewListChoices.Clear();
	ViewListPaths.Clear();
	
	if (!PathName.IsOk())
		return false;
	
	wxFileName ViewPath = PathName;
	ViewPath.AppendDir("views");

	if (ViewPath.IsDirReadable()) {
		wxDir::GetAllFiles(ViewPath.GetPathWithSep(), &ViewListPaths, wxEmptyString, wxDIR_FILES);
		
		ViewListPaths.Sort();
		
		for (unsigned int i = 0; i < ViewListPaths.GetCount(); i++)
			ViewListChoices.Add(wxFileName::FileName(ViewListPaths.Item(i)).GetFullName());
	}
	
	/// Add exported files among views
	wxArrayString ExportListPaths;
	
	wxFileName ExportPath = PathName;
	ExportPath.AppendDir("export");

	if (ExportPath.IsDirReadable()) {
		wxDir::GetAllFiles(ExportPath.GetPathWithSep(), &ExportListPaths, wxEmptyString, wxDIR_FILES);
		
		ExportListPaths.Sort();
		
		for (unsigned int i = 0; i < ExportListPaths.GetCount(); i++) {
			ViewListPaths.Add(ExportListPaths.Item(i));
			ViewListChoices.Add("> " + wxFileName::FileName(ExportListPaths.Item(i)).GetFullName());
		}	
	}
	
	return true;
}

bool NFGSerDocument::ViewListChangedQuery()
{
	return ViewListChanged;
}

wxArrayString NFGSerDocument::ViewListChoicesQuery()
{
	ViewListChanged = false;
	
	return ViewListChoices;
}

wxString NFGSerDocument::ViewFullPathQuery(int index)
{
	if ((index < 0) || ((unsigned int) index >= ViewListPaths.GetCount()))
		return wxEmptyString;
	
	return ViewListPaths.Item(index);
}

bool NFGSerDocument::LoadViewCommand(wxString filename, bool NoRefresh)
{
	bool RetVal = true;
	
	if (NFGNMRData::ImportProcParams(&SerNMRData, filename.char_str(*wxConvFileName)) != DATA_OK)
		RetVal = false;

	GetProcParams(false, false);	/// sets also ParamsChanged = 1;

	GetPhaseCorrParams(false, false);	/// sets also PhaseParamsChanged = 1;

	if (!NoRefresh) 
		CheckGraphData();	/// Make sure the plot is up to date
	
	return RetVal;
}

int NFGSerDocument::StoreView(wxString viewname)
{
	wxFileName ViewPath = PathName;
	ViewPath.AppendDir("views");
	
	if (viewname.IsEmpty())
		return FILE_OPEN_ERROR;

	ViewPath.SetFullName(viewname);
	if (ViewPath.IsDir())	/// i.e. viewname is not a filename
		return FILE_OPEN_ERROR;
	
	return TextExport(ViewPath.GetFullPath(), EXPORT_ProcParams);
}


int NFGSerDocument::PrintGraph(PrintRequest request)
{
	if (Graph == NULL)
		return DATA_INVALID;
	
	NFGSerView *view = wxDynamicCast(GetFirstView(), NFGSerView);
	
	if (view == NULL)
		return DATA_INVALID;

	if (view->GetGraphWin() == NULL) 
		return DATA_INVALID;
		
	bool RetVal = true;
	
	switch (request.request_type) {
		case ID_Print:
			{
				wxPrintData pdata;
				
				if (request.landscape)
					pdata.SetOrientation(wxLANDSCAPE);
				else
					pdata.SetOrientation(wxPORTRAIT);
				
				if (request.black_and_white)
					pdata.SetColour(false);
				else
					pdata.SetColour(true);
				
				wxPrintDialogData printDialogData(pdata);

				printDialogData.SetMinPage(1);
				printDialogData.SetMaxPage(1);

				NFGPrinter printer(&printDialogData);

				view->GetGraphWin()->SelectGraph(NULL);

				NFGPrintout printout(GetTitle() + " - " + Graph->GetGraphLabel(), Graph, &request, AcquInfoQuery(), &params, PathStringQuery());

				if (!printer.Print(GetDocumentWindow(), &printout, true)) {
					view->GetGraphWin()->SelectGraph(Graph);
					if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
						return DATA_EMPTY;
					else
						return DATA_OK;
				}
				
				view->GetGraphWin()->SelectGraph(Graph);
				
				return DATA_OK;
			}
			break;
		
		case ID_PrintPS:
			{	
				wxString filename = wxFileSelector("Choose a filename", wxEmptyString, wxEmptyString, "ps", "*.ps", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				if (filename.empty())
					return DATA_OK;
				
				NFGPSDC *PSDC = new NFGPSDC(wxSize(210, 297), request.landscape, false, 4*72);
				
				if (!PSDC->IsOk()) {
					delete PSDC;
					return DATA_EMPTY;
				}
				
				PSDC->StartDoc(GetTitle() + " - " + Graph->GetGraphLabel());
				PSDC->StartPage();

				view->GetGraphWin()->SelectGraph(NULL);
				
				wxSize DPI = PSDC->GetPPI();
				
				/// draw the graph
				RetVal = NFGGraphRenderer::RenderGraph(Graph, PSDC, AcquInfoQuery(), &params, 
											GetTitle() + " - " + Graph->GetGraphLabel(), PathStringQuery(), 
											request.point_size, DPI.GetWidth(), 72, 
											true, request.black_and_white, request.params_and_key_at_right, 
											request.print_params, request.print_title, request.print_key, wxFONTFAMILY_ROMAN);
				
				PSDC->EndPage();
				PSDC->EndDoc();
				
				view->GetGraphWin()->SelectGraph(Graph);

				if (RetVal)
					RetVal = PSDC->WriteToFile(filename);
				
				delete PSDC;
				
				if (RetVal)
					return DATA_OK;
				else {
					::wxMessageBox("Writing graph to PS file failed.", "Error", wxOK | wxICON_ERROR);
					return (DATA_EMPTY | ERROR_REPORTED);
				}
			}
			break;
		
		case ID_PrintEPS:
			{	
				wxString filename = wxFileSelector("Choose a filename", wxEmptyString, wxEmptyString, "eps", "*.eps", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				if (filename.empty())
					return DATA_OK;
				
				NFGPSDC *PSDC = new NFGPSDC(wxSize(request.width_mm, request.height_mm), false, true, 4*72);
				
				if (!PSDC->IsOk()) {
					delete PSDC;
					return DATA_EMPTY;
				}
				
				PSDC->StartDoc(GetTitle() + " - " + Graph->GetGraphLabel());

				view->GetGraphWin()->SelectGraph(NULL);
				
				wxSize DPI = PSDC->GetPPI();
				
				/// draw the graph
				RetVal = NFGGraphRenderer::RenderGraph(Graph, PSDC, AcquInfoQuery(), &params, 
											GetTitle() + " - " + Graph->GetGraphLabel(), PathStringQuery(), 
											request.point_size, DPI.GetWidth(), 72, 
											false, request.black_and_white, request.params_and_key_at_right, 
											request.print_params, request.print_title, request.print_key, wxFONTFAMILY_ROMAN);
				
				PSDC->EndDoc();
				
				view->GetGraphWin()->SelectGraph(Graph);

				if (RetVal)
					RetVal = PSDC->WriteToFile(filename);
				
				delete PSDC;
				
				if (RetVal)
					return DATA_OK;
				else {
					::wxMessageBox("Writing graph to EPS file failed.", "Error", wxOK | wxICON_ERROR);
					return (DATA_EMPTY | ERROR_REPORTED);
				}
			}			
			break;
		
#ifdef __WXMSW__
		case ID_PrintMetafile2Clipboard:
			{
#ifdef wxMETAFILE_IS_ENH
				/// actually using wxEnhMetaFileDC
				wxMetafileDC* MetaDC = new wxMetafileDC(wxEmptyString, request.width_px, request.height_px);
#else
				/// using wxMetafileDC
				wxMetafileDC* MetaDC = new wxMetafileDC(wxEmptyString, request.width_px, request.height_px, 0, 0);
#endif
			
				if (!MetaDC->IsOk()) {
					delete MetaDC;
					return DATA_EMPTY;
				}
				
				view->GetGraphWin()->SelectGraph(NULL);
				
				wxSize DPI = MetaDC->GetPPI();
				
				/// draw the graph
				RetVal = NFGGraphRenderer::RenderGraph(Graph, MetaDC, AcquInfoQuery(), &params, 
											GetTitle() + " - " + Graph->GetGraphLabel(), PathStringQuery(), 
											request.point_size, request.dpi, DPI.GetWidth(), 
											false, request.black_and_white, request.params_and_key_at_right, 
											request.print_params, request.print_title, request.print_key);
				
				view->GetGraphWin()->SelectGraph(Graph);

				wxMetafile *mf = MetaDC->Close();
				if (mf && RetVal) 
					RetVal = mf->SetClipboard(request.width_px, request.height_px);
				
				if (mf)
					delete mf;
				
				delete MetaDC;
				
				if (RetVal)
					return DATA_OK;
				else {
					::wxMessageBox("Creating metafile failed.", "Error", wxOK | wxICON_ERROR);
					return (DATA_EMPTY | ERROR_REPORTED);
				}
				
			}
			break;
#endif		
		case ID_PrintPNG:
			{
				wxString filename = wxFileSelector("Choose a filename", wxEmptyString, wxEmptyString, "png", "*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				if (filename.empty())
					return DATA_OK;
				
				wxBitmap *bmp = new wxBitmap(request.width_px, request.height_px, (request.black_and_white)?(1):(wxBITMAP_SCREEN_DEPTH));
				if (!bmp->IsOk()) {
					delete bmp;
					return DATA_EMPTY;
				}
				
				wxMemoryDC *MemDC = new wxMemoryDC(*bmp);
				if (!MemDC->IsOk()) {
					delete MemDC;
					delete bmp;
					return DATA_EMPTY;
				}

				view->GetGraphWin()->SelectGraph(NULL);
				
				wxSize DPI = MemDC->GetPPI();
				
				/// draw the graph
				RetVal = NFGGraphRenderer::RenderGraph(Graph, MemDC, AcquInfoQuery(), &params, 
											GetTitle() + " - " + Graph->GetGraphLabel(), PathStringQuery(), 
											request.point_size, request.dpi, DPI.GetWidth(), 
											false, request.black_and_white, request.params_and_key_at_right, 
											request.print_params, request.print_title, request.print_key);
				
				view->GetGraphWin()->SelectGraph(Graph);

				delete MemDC;
				
				if (RetVal)
					RetVal = bmp->SaveFile(filename, wxBITMAP_TYPE_PNG);
				
				delete bmp;
				
				if (RetVal)
					return DATA_OK;
				else {
					::wxMessageBox("Writing image to file failed.", "Error", wxOK | wxICON_ERROR);
					return (DATA_EMPTY | ERROR_REPORTED);
				}
			}
			break;
		
		case ID_PrintBitmap2Clipboard:
			{
				wxBitmap *bmp = new wxBitmap(request.width_px, request.height_px, (request.black_and_white)?(1):(-1));
				if (!bmp->IsOk()) {
					delete bmp;
					return DATA_EMPTY;
				}
				
				wxMemoryDC *MemDC = new wxMemoryDC(*bmp);
				if (!MemDC->IsOk()) {
					delete MemDC;
					delete bmp;
					return DATA_EMPTY;
				}

				view->GetGraphWin()->SelectGraph(NULL);
				
				wxSize DPI = MemDC->GetPPI();
				
				/// draw the graph
				RetVal = NFGGraphRenderer::RenderGraph(Graph, MemDC, AcquInfoQuery(), &params, GetTitle() + " - " + Graph->GetGraphLabel(), PathStringQuery(), 
											request.point_size, request.dpi, DPI.GetWidth(), 
											false, request.black_and_white, request.params_and_key_at_right, 
											request.print_params, request.print_title, request.print_key);
				
				view->GetGraphWin()->SelectGraph(Graph);

				delete MemDC;

				if (RetVal) {
					if (wxTheClipboard->Open()) {
						wxTheClipboard->SetData(new wxBitmapDataObject(*bmp));
						wxTheClipboard->Close();
					}
				}
				
				delete bmp;
				
				if (RetVal)
					return DATA_OK;
				else {
					::wxMessageBox("Creating bitmap failed.", "Error", wxOK | wxICON_ERROR);
					return (DATA_EMPTY | ERROR_REPORTED);
				}
			}
			break;
		
		default:
			;
	}

	return DATA_OK;
}


AcquParams* NFGSerDocument::AcquInfoQuery()
{
	if (NFGNMRData::CheckNMRData(&SerNMRData, CHECK_AcquInfo, ALL_STEPS) != DATA_OK) 
		return NULL;
	
	return &(SerNMRData.AcquInfo);
}

wxString NFGSerDocument::PathStringQuery()
{
	return PathName.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

void NFGSerDocument::ReloadData()
{
	long Val = 0;
	
	NFGNMRData::ReloadNMRData(&SerNMRData);
	
	if (params.UseFirstLastChunk) {
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_FirstChunk, PARAM_LONG, &(params.FirstChunk), NULL);
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_LastChunk, PARAM_LONG, &(params.LastChunk), NULL);
	} else {
		Val = 0;
		NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_FirstChunk, PARAM_LONG, &Val, NULL);
		Val = INT_MAX;
		NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_LastChunk, PARAM_LONG, &Val, NULL);
	}

	if (params.UseFirstLastChunkPoint) {
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_ChunkStart, PARAM_LONG, &(params.FirstChunkPoint), NULL);
		NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_ChunkEnd, PARAM_LONG, &(params.LastChunkPoint), NULL);
	} else {
		Val = 0;
		NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ChunkStart, PARAM_LONG, &Val, NULL);
		Val = INT_MAX;
		NFGNMRData::SetProcParam(&SerNMRData, PROC_PARAM_ChunkEnd, PARAM_LONG, &Val, NULL);
	}

	NFGNMRData::GetProcParam(&SerNMRData, PROC_PARAM_DFTLength, PARAM_LONG, &(params.FFTLength), NULL);

	ParamsChanged = true;
	PhaseParamsChanged = true;

	SelectedStepChanged = true;	/// update displayed associated value etc. if necessary
	StepStatesChanged = true;	/// update displayed associated value etc. if necessary
	
	if (Graph != NULL)
		Graph->CheckData(true);

	/// update NFGInfoView if any is opened
	wxList::iterator iter;
	for (iter = GetViews().begin(); iter != GetViews().end(); ++iter) {
		NFGInfoView* iview = wxDynamicCast(*iter, NFGInfoView);
		if (iview) {
			if (iview->GetInfoWin())
				iview->GetInfoWin()->LoadAcquInfo(AcquInfoQuery(), PathStringQuery());
		}
	}
}

void NFGSerDocument::CheckGraphData()
{
	if (Graph == NULL)
		return;
	
	Graph->CheckData(true);
}



IMPLEMENT_DYNAMIC_CLASS(NFGUserlistDocument, wxDocument)

NFGUserlistDocument::NFGUserlistDocument()
{
	NFGNMRData::InitNMRData(&EmptyNMRData);
	EmptyNMRData.ErrorReport = NFGErrorReport;
	EmptyNMRData.ErrorReportCustom = NFGErrorReportCustom;

	NFGNMRData::InitUserlist(&EmptyNMRData, &UserlistParamsData);
}

NFGUserlistDocument::~NFGUserlistDocument()
{
	NFGNMRData::FreeUserlist(&EmptyNMRData, &UserlistParamsData);
}

/// Created document shall be immediatelly saved (or discarded otherwise)
bool NFGUserlistDocument::OnNewDocument() {
	wxString path = GetFilename();
	if (!path.empty()) {	/// setup up the title and save the fresh-made userlist if the fllename is set
		wxFileName FN = wxFileName(path);
		FN.MakeAbsolute();
		wxArrayString dirs = FN.GetDirs();
		wxString title;
		
		for (size_t i = ((dirs.GetCount() >= 2)?(dirs.GetCount() - 2):(0)); i < dirs.GetCount(); i++) 
			title.Append(dirs.Item(i) + " | ");
		
		title.Append(FN.GetFullName());
		SetTitle(title);
		
		Modify(true);
		if (OnSaveDocument(path))
			return true;
	}
		
	Modify(false);
	return false;
}

bool NFGUserlistDocument::DoSaveDocument(const wxString& filename)
{
	NFGUserlistView *view = (NFGUserlistView *) GetFirstView();
	if (!view)
		return false;
	if (!(view->GetUserlistWin()))
		return false;

	if (filename.IsEmpty()) 
		return false;
	
	wxFileName FN = wxFileName(filename);
	FN.MakeAbsolute();

	wxString path = FN.GetPathWithSep();
	if (wxFileName(path, "ser").FileExists()) {
		if (::wxMessageBox("The directory of this userlist seems to contain an already performed experiment. Do you really want to save the userlist?", GetTitle(), wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION) != wxYES) 
			return false;
	}
	
	if (!view->GetUserlistWin()->Store(&UserlistParamsData, FN.GetFullName().Lower() == wxString("userlist")))
		return false;
	
	/// newname variable handling for legacy userlist
	if ((FN.GetFullName().Lower() == wxString("userlist")) && (UserlistParamsData.Destination == NULL)) {
		wxString Destination;
		wxArrayString dirs = FN.GetDirs();
	
		if (dirs.GetCount() >= 2) {
			Destination = dirs.Item(dirs.GetCount() - 2);
			if (!(Destination.EndsWith("_h"))) 
				Destination.Append("_h");
		} else {
			Destination = ::wxGetTextFromUser("Please enter the \'Destination\' variable value: ", GetTitle(), wxString((UserlistParamsData.Destination)?(UserlistParamsData.Destination):("")));
			if (Destination.IsEmpty())
				return false;
		}
		
		UserlistParamsData.Destination = strdup(Destination.char_str());
	}

	if (NFGNMRData::WriteUserlist(&EmptyNMRData, filename.char_str(*wxConvFileName), &UserlistParamsData) != DATA_OK)
		return false;
	
	return true;
}

bool NFGUserlistDocument::DoOpenDocument(const wxString& filename)
{
	NFGUserlistView *view = (NFGUserlistView *) GetFirstView();
	if (!view)
		return false;
	if (!(view->GetUserlistWin()))
		return false;

	bool DefaultsUsed = (NFGNMRData::ReadUserlist(&EmptyNMRData, filename.char_str(*wxConvFileName), &UserlistParamsData) != DATA_OK);
	
	wxFileName FN = wxFileName(filename);
	
	/// convenient newname variable handling for legacy userlist
	if ((FN.GetFullName().Lower() == "userlist") && (UserlistParamsData.Destination != NULL)) {
		wxString Destination;
		wxArrayString dirs = FN.GetDirs();
	
		if (dirs.GetCount() >= 2) {
			Destination = dirs.Item(dirs.GetCount() - 2);
			Destination.Append("_h");
			
			if (Destination == wxString(UserlistParamsData.Destination)) {
				free(UserlistParamsData.Destination);
				UserlistParamsData.Destination = NULL;
			}
		}
	}

	DefaultsUsed = DefaultsUsed || !(view->GetUserlistWin()->Load(&UserlistParamsData, FN.GetFullName().Lower() == wxString("userlist")));
	
	if (DefaultsUsed)
		::wxMessageBox("Problems occured when reading userlist. \nDefault values were used to a certain extent.", "Error", wxOK | wxICON_ERROR);

	if (!filename.empty()) {
		wxFileName FN = wxFileName(filename);
		FN.MakeAbsolute();
		wxArrayString dirs = FN.GetDirs();
		
		wxString title = wxEmptyString;
		
		for (int i = (dirs.GetCount() >= 2)?(((int) dirs.GetCount()) - 2):(0); (i >= 0) && (i < ((int) dirs.GetCount())); i++) {
			title.Append(dirs.Item(i));
			title.Append(" | ");
		}
		
		title.Append(FN.GetFullName());
		SetTitle(title);
	}
	
	Modify(DefaultsUsed);
	return true;
}

bool NFGUserlistDocument::IsModified(void) const
{
	return wxDocument::IsModified();
}

void NFGUserlistDocument::Modify(bool mod)
{
	NFGUserlistView *view = (NFGUserlistView *) GetFirstView();
	if (view && view->GetUserlistWin())
		view->GetUserlistWin()->Modify(mod);
	
	wxDocument::Modify(mod);
}


bool NFGUserlistDocument::Revert()
{
	NFGUserlistView *view = (NFGUserlistView *) GetFirstView();
	if (!view)
		return false;
	if (!(view->GetUserlistWin()))
		return false;
	
	bool NotReverted = true;
	wxString filename = GetFilename();
	wxFileName FN = wxFileName(filename);
	if (!filename.IsEmpty()) {
		NotReverted = (NFGNMRData::ReadUserlist(&EmptyNMRData, filename.char_str(*wxConvFileName), &UserlistParamsData) != DATA_OK);

		/// convenient newname variable handling for legacy userlist
		if ((FN.GetFullName().Lower() == "userlist") && (UserlistParamsData.Destination != NULL)) {
			wxString Destination;
			wxArrayString dirs = FN.GetDirs();
		
			if (dirs.GetCount() >= 2) {
				Destination = dirs.Item(dirs.GetCount() - 2);
				Destination.Append("_h");
				
				if (Destination == wxString(UserlistParamsData.Destination)) {
					free(UserlistParamsData.Destination);
					UserlistParamsData.Destination = NULL;
				}
			}
		}
	}
	
	NotReverted = NotReverted || !view->GetUserlistWin()->Load(&UserlistParamsData, FN.GetFullName().Lower() == wxString("userlist"));
	
	if (NotReverted)
		::wxMessageBox("Problems occured when reading userlist. \nSome values were not reverted.", "Error", wxOK | wxICON_ERROR);
	
	Modify(NotReverted);
	SetDocumentSaved(true);
	UpdateAllViews();
	return true;
}
