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

#ifndef __panelmain__
#define __panelmain__

#include "wx_pch.h"

#include <wx/notebook.h>
#include <wx/toolbook.h>
#include <wx/filepicker.h>
#include <wx/treectrl.h>
#include <wx/tglbtn.h>

#include "cd.h"
#include "panelmain_cd.h"

#include "validators_cd.h"
#include "doc.h"
#include "nmrfilipgui_cd.h"


class NFGColourTag : public wxWindow
{
	public:
		NFGColourTag(wxWindow* parent, wxColour colour, wxSize size);
		~NFGColourTag();
};


class NFGDisplayInnerPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnUpdateUI(wxUpdateUIEvent& event);
		void OnUpdateUIDataSets(wxUpdateUIEvent& event);
		void OnDisplayChange(wxToolbookEvent& event);
		void OnDataSetDisplayChange(wxCommandEvent& event);
		
	protected:
		NFGDocManager *DocMan; 
	
		wxToolbook* DisplayToolbook;
		wxPanel* ReloadPanel;
		wxPanel* ParamsPanel;
		wxStaticText* ParamsLabelST;
		wxPanel* TDDPanel;
		wxStaticText* TDDLabelST;
		NFGColourTag* TDDRealColourTag;
		wxCheckBox* TDDRealCheckBox;
		NFGColourTag* TDDImagColourTag;
		wxCheckBox* TDDImagCheckBox;
		NFGColourTag* TDDModuleColourTag;
		wxCheckBox* TDDModuleCheckBox;
		NFGColourTag* TDDEchoPeaksEnvelopeColourTag;
		wxCheckBox* TDDEchoPeaksEnvelopeCheckBox;
		wxPanel* ChunkAvgPanel;
		wxStaticText* ChunkAvgLabelST;
		NFGColourTag* ChunkAvgRealColourTag;
		wxCheckBox* ChunkAvgRealCheckBox;
		NFGColourTag* ChunkAvgImagColourTag;
		wxCheckBox* ChunkAvgImagCheckBox;
		NFGColourTag* ChunkAvgModuleColourTag;
		wxCheckBox* ChunkAvgModuleCheckBox;
		wxPanel* FFTPanel;
		wxStaticText* FFTLabelST;
		NFGColourTag* FFTRealColourTag;
		wxCheckBox* FFTRealCheckBox;
		NFGColourTag* FFTImagColourTag;
		wxCheckBox* FFTImagCheckBox;
		NFGColourTag* FFTModuleColourTag;
		wxCheckBox* FFTModuleCheckBox;
		wxPanel* SpectrumPanel;
		wxStaticText* SpectrumLabelST;
		NFGColourTag* SpectrumFFTEnvelopeColourTag;
		wxCheckBox* SpectrumFFTEnvelopeCheckBox;
		NFGColourTag* SpectrumParticularFFTModulesColourTag;
		wxCheckBox* SpectrumParticularFFTModulesCheckBox;
		NFGColourTag* SpectrumFFTRealEnvelopeColourTag;
		wxCheckBox* SpectrumFFTRealEnvelopeCheckBox;
		NFGColourTag* SpectrumParticularFFTRealPartsColourTag;
		wxCheckBox* SpectrumParticularFFTRealPartsCheckBox;
		wxPanel* EvaluationPanel;
		wxStaticText* EvaluationLabelST;
		NFGColourTag* EvaluationFFTMaxColourTag;
		wxCheckBox* EvaluationFFTMaxCheckBox;
		NFGColourTag* EvaluationFFT0ColourTag;
		wxCheckBox* EvaluationFFT0CheckBox;
		NFGColourTag* EvaluationFFTMeanColourTag;
		wxCheckBox* EvaluationFFTMeanCheckBox;
		NFGColourTag* EvaluationFFTRealMaxColourTag;
		wxCheckBox* EvaluationFFTRealMaxCheckBox;
		NFGColourTag* EvaluationFFTReal0ColourTag;
		wxCheckBox* EvaluationFFTReal0CheckBox;
		NFGColourTag* EvaluationFFTRealMeanColourTag;
		wxCheckBox* EvaluationFFTRealMeanCheckBox;
		NFGColourTag* EvaluationChunkAvgModMaxColourTag;
		wxCheckBox* EvaluationChunkAvgModMaxCheckBox;
		NFGColourTag* EvaluationChunkAvgModIntColourTag;
		wxCheckBox* EvaluationChunkAvgModIntCheckBox;
		
	public:
		NFGDisplayInnerPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGDisplayInnerPanel();
		
};


class NFGBrowseTreeItemData : public wxTreeItemData
{
	public:
		NFGBrowseTreeItemData() { }
		virtual ~NFGBrowseTreeItemData() { }

		int Type;
		wxString Name;
		wxString FullPath;
		bool IsNew;
};


class NFGBrowseTreeCtrl :  public wxTreeCtrl
{
	DECLARE_EVENT_TABLE()
	private:
		void OnItemExpanding(wxTreeEvent& event);
		void OnItemCollapsed(wxTreeEvent& event);
		
		bool IterateDir(const wxString& dir, const wxTreeItemId& parent, bool SetNew = false);
		bool ReiterateDir(const wxString& dir, const wxTreeItemId& parent);

		bool IsInterestingDir(const wxString& dir);
	
		wxTreeItemId AddEntry(const wxString& name, const wxString& path, const wxTreeItemId& parent, int type, int image, bool SetNew);
	
	protected:
		int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2);
		
	public:
		NFGBrowseTreeCtrl() { }
		NFGBrowseTreeCtrl(wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
		virtual ~NFGBrowseTreeCtrl() { }

		void DisplayDirTree();
		void RefreshDirTree();
		void RefreshDirItem(const wxTreeItemId& item);
		
	DECLARE_DYNAMIC_CLASS(NFGBrowseTreeCtrl)
};
	

class NFGMainPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnBrowseChangeDir(wxFileDirPickerEvent& event);
		void OnBrowseRefresh(wxCommandEvent& event);
		void OnBrowseOpen(wxTreeEvent& event);
		void OnBrowseMenu(wxTreeEvent& event);
		void OnContextMenuCommand(wxCommandEvent& event);
		void OnSetSelectedStep(wxCommandEvent& event);
		void OnSetSelectedStepState(wxCommandEvent& event);
		void OnZoom(wxCommandEvent& event);
		void OnWindowsShow(wxCommandEvent& event);
#ifdef __NMRFilipGUI_MDI__
		void OnWindowsCommand(wxCommandEvent& event);
#endif
		void OnUpdateUI(wxUpdateUIEvent& event);
		
		long SelectedStep;
		unsigned char SelectedStepFlag;
		wxString SelectedStepLabel;
		
		wxString xCoord, yCoord, dxCoord, yratioCoord;
	
		wxBitmap bmpIgnored, bmpBlank, bmpNotInEnvelope, bmpNotShown, bmpOk;
	
		wxMenu *ContextMenuFile;
		wxMenu *ContextMenuDir;
		wxTreeItemId CMItem;
		
		static const wxString AboutText;
		static const wxString HintsText;
	
	protected:
		NFGSerDocument* CurrentSerDocument;
		NFGDocManager *DocMan;

		wxNotebook* MainNotebook;
		wxPanel* BrowsePanel;
		wxBitmapButton* BrowseRefreshButton;
		wxDirPickerCtrl* BrowseDirPicker;
		NFGBrowseTreeCtrl* BrowseTreeCtrl;
		wxPanel* DisplayPanel;

		wxStaticText* NumberLabelST;
		NFGAutoValidatedTextCtrl* NumberTextCtrl;
		wxToggleButton* StateTgButton;
		NFGValidatedStaticText* AssocValST;
	
		wxStaticText* xCoordLabelST;
		NFGValidatedStaticText* xCoordST;
		wxStaticText* yCoordLabelST;
		NFGValidatedStaticText* yCoordST;
		wxStaticText* dxCoordLabelST;
		NFGValidatedStaticText* dxCoordST;
		wxStaticText* dyCoordLabelST;
		NFGValidatedStaticText* yratioCoordST;
		
		wxToolBar* ZoomTBar;
		NFGDisplayInnerPanel* DisplayInnerPanel;
		wxPanel* WindowsPanel;
		wxListBox* WindowsListBox;
		wxButton* WindowsTileButton;
		wxButton* WindowsCascadeButton;
		wxPanel* HintsPanel;
		wxTextCtrl* HintsTextCtrl;
		wxPanel* AboutPanel;
		wxTextCtrl* AboutTextCtrl;
		
	public:
		NFGMainPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGMainPanel();
};

#endif
