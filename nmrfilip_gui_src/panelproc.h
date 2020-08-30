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

#ifndef __panelproc__
#define __panelproc__

#include "wx_pch.h"

#include <wx/notebook.h>
#include <wx/filepicker.h>
#include <wx/spinbutt.h>
#include <wx/radiobut.h>
#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/docview.h>

#include "cd.h"
#include "panelproc_cd.h"

#include "validators_cd.h"
#include "doc.h"
#include "nmrfilipgui_cd.h"


class NFGProcessPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnDefaultFirstLastChunk(wxCommandEvent& event);
		void OnDefaultFirstLastChunkPoint(wxCommandEvent& event);
		void OnProcessCommand(wxCommandEvent& event);
		void OnProcessChangeCommand(wxCommandEvent& event);
		void OnAdjustValues(wxCommandEvent& event);
		void OnUpdateUI(wxUpdateUIEvent& event);
	
		int FirstChunkSpinVal, LastChunkSpinVal, FirstChunkPointSpinVal, LastChunkPointSpinVal, FFTLengthSpinVal, FilterSpinVal;
		bool PostponeChanges;
	
		long MaxLastChunk, MaxLastChunkPoint;
	
		void OnSpin(wxSpinEvent& event);
		void OnChar(wxKeyEvent& event);
		void OnMouseWheel(wxMouseEvent& event);
	
		void SpinChunks(int step, long max, NFGAutoValidatedTextCtrl* control);
		void SpinFFTLength(int step);
		void SpinFilter(int step);
		
	protected:
		NFGSerDocument* CurrentSerDocument;
		NFGDocManager *DocMan;

		ProcParams params;
		
		wxStaticText* FirstChunkLabelST;
		wxStaticText* LastChunkLabelST;
		wxCheckBox* EnableFirstLastChunkCheckBox;
		NFGAutoValidatedTextCtrl* FirstChunkTextCtrl;
		wxSpinButton* FirstChunkSpinButton;
		NFGAutoValidatedTextCtrl* LastChunkTextCtrl;
		wxSpinButton* LastChunkSpinButton;
		wxButton* DefaultFirstLastChunkButton;
	
		wxStaticText* FirstChunkPointLabelST;
		wxStaticText* LastChunkPointLabelST;
		wxCheckBox* EnableFirstLastChunkPointCheckBox;
		NFGAutoValidatedTextCtrl* FirstChunkPointTextCtrl;
		wxSpinButton* FirstChunkPointSpinButton;
		NFGAutoValidatedTextCtrl* LastChunkPointTextCtrl;
		wxSpinButton* LastChunkPointSpinButton;
		wxButton* DefaultFirstLastChunkPointButton;
	
		wxStaticText* FFTLengthLabelST;
		wxCheckBox* FilterCheckBox;
		
		NFGAutoValidatedTextCtrl* FFTLengthTextCtrl;
		wxSpinButton* FFTLengthSpinButton;
		
		NFGAutoValidatedTextCtrl* FilterTextCtrl;
		wxSpinButton* FilterSpinButton;
		
		wxCheckBox* ScaleFirstTDPointCheckBox;
		
		wxButton* ProcessLoadButton;
		wxButton* ProcessStoreButton;
		
		wxListBox* OmitListBox;
		
		wxToggleButton* ProcessAutoApplyTgButton;
		wxButton* ProcessApplyButton;
		wxButton* ProcessRevertButton;
		
	public:
		NFGProcessPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGProcessPanel();
};

class NFGPhasePanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnPhaseCommand(wxCommandEvent& event);
		void OnPhaseChangeCommand(wxCommandEvent& event);
		void OnUpdateUI(wxUpdateUIEvent& event);
		int SpinVal0, SpinVal1;
		bool PostponeChanges;
	
		void OnSpin(wxSpinEvent& event);
		void OnChar(wxKeyEvent& event);
		void OnMouseWheel(wxMouseEvent& event);
	
		void SpinZeroOrder(double coefficient);
		void SpinFirstOrder(double coefficient);
		
	protected:
		NFGSerDocument* CurrentSerDocument;
		NFGDocManager *DocMan;

		PhaseCorrParams pparams;
			
		wxStaticText* ZeroOrderPhaseCorrLabelST;
		wxStaticText* ZeroOrderPhaseCorrLabelUnitsST;
		wxCheckBox* FirstOrderPhaseCorrCheckBox;
		wxStaticText* FirstOrderPhaseCorrLabelUnitsST;
		
		wxFlexGridSizer* ZeroOrderFGSizer;
		wxFlexGridSizer* ZeroOrderFollowAutoFGSizer;
		wxFlexGridSizer* FirstOrderFGSizer;
		
		NFGAutoValidatedTextCtrl* ZeroOrderPhaseCorrTextCtrl;
		wxSpinButton* ZeroOrderPhaseCorrSpinButton;
		NFGAutoValidatedTextCtrl* FirstOrderPhaseCorrTextCtrl;
		wxSpinButton* FirstOrderPhaseCorrSpinButton;

		wxCheckBox* ZeroOrderSameValuesForAllCheckBox;
		wxCheckBox* FirstOrderSameValuesForAllCheckBox;
		wxToggleButton* ZeroOrderAutoAllTogetherToogleButton;
		wxToggleButton* ZeroOrderAutoToogleButton;
		wxToggleButton* ZeroOrderFollowAutoToogleButton;
		wxTextCtrl* PilotStepTextCtrl;
		wxButton* ZeroOrderSetAllAutoButton;
		wxButton* ZeroOrderSetAllManualButton;

		wxCheckBox* RemoveOffsetCheckBox;

		wxButton* PhaseLoadButton;
		wxButton* PhaseStoreButton;
		
		wxToggleButton* PhaseAutoApplyTgButton;
		wxButton* PhaseApplyButton;
		wxButton* PhaseRevertButton;
		
	public:
		NFGPhasePanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGPhasePanel();
};

class NFGPlotPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnMinMaxLoadStore(wxCommandEvent& event);
		void OnMinMaxInput(wxCommandEvent& event);
		void OnPlotCommand(wxCommandEvent& event);
		void OnUpdateUI(wxUpdateUIEvent& event);
	
	protected:
		NFGSerDocument* CurrentSerDocument;
		NFGDocManager *DocMan;
	
		ZoomParams params;
		bool ManualH;
		bool ManualV;

		NFGAutoRadioButton* PlotAllAutoHRadioButton;
		NFGAutoRadioButton* PlotSelectedAutoHRadioButton;
		NFGAutoRadioButton* PlotManualHRadioButton;
		wxStaticText* MinimumHLabelST;
		NFGAutoValidatedTextCtrl* MinimumHTextCtrl;
		wxButton* MinimumHLoadButton;
		wxButton* MinimumHStoreButton;
		wxStaticText* MaximumHLabelST;
		NFGAutoValidatedTextCtrl* MaximumHTextCtrl;
		wxButton* MaximumHLoadButton;
		wxButton* MaximumHStoreButton;
		NFGAutoRadioButton* PlotAllAutoVRadioButton;
		NFGAutoRadioButton* PlotSelectedAutoVRadioButton;
		NFGAutoRadioButton* PlotManualVRadioButton;
		wxStaticText* MinimumVLabelST;
		NFGAutoValidatedTextCtrl* MinimumVTextCtrl;
		wxButton* MinimumVLoadButton;
		wxButton* MinimumVStoreButton;
		wxStaticText* MaximumVLabelST;
		NFGAutoValidatedTextCtrl* MaximumVTextCtrl;
		wxButton* MaximumVLoadButton;
		wxButton* MaximumVStoreButton;
		wxCheckBox* DrawPointsCheckBox;
		wxCheckBox* ThickLinesCheckBox;
		wxButton* PlotLoadButton;
		wxButton* PlotStoreButton;
		wxButton* PlotApplyButton;
		wxButton* PlotRevertButton;
		
	public:
		NFGPlotPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGPlotPanel();
};

class NFGViewsPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnViewNameEnter(wxCommandEvent& event);
		void OnStoredViewsSelect(wxCommandEvent& event);
		void OnStoredViewsDblClick(wxCommandEvent& event);
		void OnViewsCommand(wxCommandEvent& event);
		void OnUpdateUI(wxUpdateUIEvent& event);
	
	protected:
		NFGSerDocument* CurrentSerDocument;
		NFGDocManager *DocMan;

		wxTextCtrl* ViewNameTextCtrl;
		wxListBox* StoredViewsListBox;
		wxButton* LoadViewButton;
		wxButton* StoreViewButton;
		wxButton* LoadOtherViewButton;
		wxButton* DeleteViewButton;
		
	public:
		NFGViewsPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGViewsPanel();
};

class NFGExportPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnExportCommand(wxCommandEvent& event);
		void OnUpdateUI(wxUpdateUIEvent& event);
	
	protected:
		NFGSerDocument* CurrentSerDocument;
		NFGDocManager *DocMan;

		wxFilePickerCtrl *ExportTDDFilePickerCtrl;
		wxBitmapButton *ExportTDDBmpButton, *ExportTDDBmpButton2;
		wxFilePickerCtrl *ExportEchoPeaksEnvelopeFilePickerCtrl;
		wxBitmapButton *ExportEchoPeaksEnvelopeBmpButton, *ExportEchoPeaksEnvelopeBmpButton2;
		wxFilePickerCtrl *ExportChunkAvgFilePickerCtrl;
		wxBitmapButton *ExportChunkAvgBmpButton, *ExportChunkAvgBmpButton2;
		wxFilePickerCtrl *ExportFFTFilePickerCtrl;
		wxBitmapButton *ExportFFTBmpButton, *ExportFFTBmpButton2;
		wxFilePickerCtrl *ExportFFTEnvelopeFilePickerCtrl;
		wxBitmapButton *ExportFFTEnvelopeBmpButton, *ExportFFTEnvelopeBmpButton2;
		wxFilePickerCtrl *ExportFFTRealEnvelopeFilePickerCtrl;
		wxBitmapButton *ExportFFTRealEnvelopeBmpButton, *ExportFFTRealEnvelopeBmpButton2;
		wxFilePickerCtrl *ExportEvaluationFilePickerCtrl;
		wxBitmapButton *ExportEvaluationBmpButton, *ExportEvaluationBmpButton2;
		
	public:
		NFGExportPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGExportPanel();
};

class NFGPrintPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnPrintCommand(wxCommandEvent& event);
		void OnUpdateUI(wxUpdateUIEvent& event);
	
	protected:
		NFGSerDocument* CurrentSerDocument;
		NFGDocManager *DocMan;
	
		PrintRequest PrintReq;

		wxCheckBox* PrintTitleCheckBox;
		wxCheckBox* PrintBWCheckBox;
		wxCheckBox* PrintKeyCheckBox;
		wxCheckBox* PrintParamsCheckBox;
		wxCheckBox* PrintParamsAndKeyAtRightCheckBox;
		NFGAutoRadioButton* PrintPortraitRadioButton;
		NFGAutoRadioButton* PrintLandscapeRadioButton;
		wxButton* PrintButton;
		wxButton* PrintPSButton;
		wxStaticLine* PrintStaticLine1;
		wxStaticText* PrintEPSWidthLabelST;
		NFGAutoValidatedTextCtrl* PrintEPSWidthTextCtrl;
		wxStaticText* PrintEPSWmmLabelST;
		wxStaticText* PrintEPSHeightLabelST;
		NFGAutoValidatedTextCtrl* PrintEPSHeightTextCtrl;
		wxStaticText* PrintEPSHmmLabelST;
		wxButton* PrintEPSButton;
		wxStaticLine* PrintStaticLine2;
		wxStaticText* PrintPNGWidthLabelST;
		NFGAutoValidatedTextCtrl* PrintPNGWidthTextCtrl;
		wxStaticText* PrintPNGWpxLabelST;
		wxStaticText* PrintPNGHeightLabelST;
		NFGAutoValidatedTextCtrl* PrintPNGHeightTextCtrl;
		wxStaticText* PrintPNGHpxLabelST;
		wxStaticText* PrintPNGResolutionLabelST;
		NFGAutoValidatedTextCtrl* PrintPNGResolutionTextCtrl;
		wxStaticText* PrintPNGdpiLabelST;
		wxButton* PrintMetafile2ClipboardButton;
		wxButton* PrintPNGButton;
		wxButton* PrintBitmap2ClipboardButton;
		
	public:
		NFGPrintPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGPrintPanel();
};


class NFGProcPanel : public wxPanel 
{
	private:
	
	protected:
		NFGDocManager *DocMan;

		wxNotebook* ProcNotebook;
		NFGProcessPanel* ProcessPanel;
		NFGPhasePanel* PhasePanel;
		NFGPlotPanel* PlotPanel;
		NFGViewsPanel* ViewsPanel;
		NFGExportPanel* ExportPanel;
		NFGPrintPanel* PrintPanel;
	
	public:
		NFGProcPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGProcPanel();
};

#endif
