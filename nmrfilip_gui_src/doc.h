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

#ifndef __doc__
#define __doc__

#include "wx_pch.h"

#include <wx/docview.h>
#include <wx/cmdproc.h>
#include <wx/dynlib.h>
#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/clipbrd.h>
#ifdef __WXMSW__
#include <wx/metafile.h>
#endif
#include <wx/bookctrl.h>

#include "cd.h"
#include "doc_cd.h"

#include "nmrdata_cd.h"
#include "plotgen_cd.h"


struct ProcParams
{
	public:
		bool UseFirstLastChunk;
		long FirstChunk;
		long LastChunk;
		bool UseFirstLastChunkPoint;
		long FirstChunkPoint;
		long LastChunkPoint;
		long FFTLength;
		bool UseFilter;
		double Filter;	/// [MHz]
		bool ScaleFirstTDPoint;
	
		bool AutoApply;
};

struct PhaseCorrParams
{
	public:
		double PhaseCorr0;	/// [deg]
		double PhaseCorr1;	/// [us]
	
		bool ZeroOrderSameValuesForAll;
		bool ZeroOrderAutoAllTogether;
		bool ZeroOrderAuto;
		bool ZeroOrderFollowAuto;
		long PilotStep;
			
		bool ZeroOrderSetAllAuto;
		bool ZeroOrderSetAllManual;
	
		bool FirstOrderSameValuesForAll;
		bool ApplyFirstOrderCorr;
	
		bool RemoveOffset;
	
		bool AutoApply;
};

struct ZoomParams {
	public:
		bool AutoZoomVAll, AutoZoomVSelected;
		bool AutoZoomHAll, AutoZoomHSelected;
		bool DrawPoints, ThickLines;
		double minx, maxx, miny, maxy;
};

struct PrintRequest {
	public:
		long width_mm, height_mm;
		long width_px, height_px, dpi, point_size;
		bool portrait, landscape, params_and_key_at_right, black_and_white;
		bool print_params, print_key, print_title;
		int request_type;
};


class NFGTextDocument : public wxDocument
{
	DECLARE_DYNAMIC_CLASS(NFGTextDocument)
	
	public:
		bool OnNewDocument();
		bool DoSaveDocument(const wxString& filename);
		bool DoOpenDocument(const wxString& filename);
		bool IsModified(void) const;
		void Modify(bool mod);
	
		bool Revert();
		
		NFGTextDocument(void) {}
		~NFGTextDocument(void) {}
};


#define PROC_PARAM_UseFirstLastChunkPoint	256
#define PROC_PARAM_UseFilter	512

extern "C" {
	char *NFGErrorReport(void *NMRDataStruct, int ErrorNumber, char *Activity);
	char *NFGErrorReportCustom(void *NMRDataStruct, char *ErrorDesc, char *Activity);

	int MarkNMRDataOldCallbackFn(void* NMRDataStruct, unsigned long ClearedFlags, long StepNo);
	int ChangeProcParamCallbackFn(void* NMRDataStruct, unsigned int ParamType, long StepNo);
}

class NFGSerDocument : public wxDocument
{
	DECLARE_DYNAMIC_CLASS(NFGSerDocument)

	protected:
		NMRData SerNMRData;
		wxFileName PathName;
	
		unsigned char SelectedGraphType;
		NFGGraph *Graph;	/// just a pointer to selected graph
		NFGGraph *GraphTDD;
		NFGGraph *GraphChunkAvg;
		NFGGraph *GraphFFT;
		NFGGraph *GraphSpectrum;
		NFGGraph *GraphEvaluation;

		ProcParams params;
		bool ParamsChanged;	/// proc params
		PhaseCorrParams PhaseParams;
		bool PhaseParamsChanged;	/// PhaseCorr params
		bool ZParamsChanged;	/// zoom params
		bool SelectedStepChanged;
		bool StepStatesChanged;
		bool ViewListChanged;


		wxString FileName_ExportTDD;
		wxString FileName_ExportEchoPeaksEnvelope;
		wxString FileName_ExportChunkAvg;
		wxString FileName_ExportFFT;
		wxString FileName_ExportFFTEnvelope;
		wxString FileName_ExportFFTRealEnvelope;
		wxString FileName_ExportEvaluation;
		
		wxArrayString ViewListChoices;
		wxArrayString ViewListPaths;

	public:
		bool DoSaveDocument(const wxString& filename);
		bool DoOpenDocument(const wxString& filename);
		bool IsModified(void) const;
		void Modify(bool mod);
	
		int MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo);
		int ChangeProcParamCallback(unsigned int ParamType, long StepNo);
	
		void ZoomCommand(int command);
	
		unsigned long SetSelectedStep(unsigned long StepNo);
		unsigned long GetSelectedStep();
		wxString GetSelectedStepState();
		unsigned char GetSelectedStepStateFlag();
		wxString GetSelectedStepLabel();
		bool SelectedStepChangedQuery();
		void SelectedStepChangedNotify();
		bool StepStatesChangedQuery();
	
		wxString CursorXQuery();
		wxString CursorYQuery();
		wxString CursorDeltaXQuery();
		wxString CursorYRatioQuery();
	
		void CopyCursorPosCommand(wxString &CursorPosStr, bool Append = false);
	
		unsigned char GraphTypeCommand(unsigned char GraphType);
		unsigned char GraphTypeQuery();
		
		unsigned long DatasetDisplayCommand(unsigned long flag);
		unsigned long DatasetDisplayQuery();
		
		ProcParams SetProcParams(ProcParams Parameters, bool NoRefresh = false, bool Coerce = true);
		ProcParams GetProcParams(bool MarkUnchanged = true, bool Quick = true);
		bool ProcParamsChangedQuery(bool MarkUnchanged = true);
		
		PhaseCorrParams SetPhaseCorrParams(PhaseCorrParams Parameters, bool NoRefresh = false);
		PhaseCorrParams GetPhaseCorrParams(bool MarkUnchanged = true, bool Quick = true);
		bool PhaseCorrParamsChangedQuery(bool MarkUnchanged = true);
		
		ZoomParams SetZoomParams(ZoomParams Parameters);
		ZoomParams GetZoomParams();
		bool ZoomParamsChangedQuery();
		void ZoomParamsChangedNotify();
		
		long LastChunkQuery();
		long LastChunkPointQuery();
		
		unsigned long StepCountQuery();
		wxString StepLabelQuery(unsigned long n);
		bool StepNoFFTEnvelopeQuery(unsigned long n);
		void StepNoFFTEnvelopeCommand(unsigned long n, bool omit, bool NoRefresh = false);
		
		int ClipboardExport(unsigned int DataType);
		int TextExport(wxString filename, unsigned int DataType);
		wxString TextExportFilenameQuery(unsigned int DataType);
		
		bool LoadViewList();
		bool ViewListChangedQuery();
		wxArrayString ViewListChoicesQuery();
		wxString ViewFullPathQuery(int index);
		bool LoadViewCommand(wxString filename, bool NoRefresh = false);
		int StoreView(wxString viewname);
		
		int PrintGraph(PrintRequest request);
		
		AcquParams* AcquInfoQuery();
		wxString PathStringQuery();
		
		void ReloadData();
		void CheckGraphData();
		
		NFGSerDocument(void);
		~NFGSerDocument(void);
};

class NFGUserlistDocument : public wxDocument
{
	DECLARE_DYNAMIC_CLASS(NFGUserlistDocument)
	
	private:
		NMRData EmptyNMRData;
		UserlistParams UserlistParamsData;

	public:
		bool OnNewDocument();
		bool DoSaveDocument(const wxString& filename);
		bool DoOpenDocument(const wxString& filename);
		bool IsModified(void) const;
		void Modify(bool mod);
		
		bool Revert();
		
		NFGUserlistDocument(void);
		~NFGUserlistDocument(void);
};

#endif
