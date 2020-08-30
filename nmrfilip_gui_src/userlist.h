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

#ifndef __userlist__
#define __userlist__

#include "wx_pch.h"

#include <wx/notebook.h>
#include <wx/toolbook.h>
#include <wx/imaglist.h>
#include <wx/filepicker.h>
#include <wx/treectrl.h>
#include <wx/statline.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/mdi.h>
#include <wx/docview.h>
#include <wx/docmdi.h>
#include <wx/valgen.h>

#include "cd.h"
#include "userlist_cd.h"

#include "nmrdata_cd.h"
#include "validators_cd.h"


class NFGUserlistPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	
	private:
		void OnUpdateUI(wxUpdateUIEvent& event);
		
	protected:
		
		unsigned long AssocType;
	
		double Start, End, Step, Coef, Sum;
		long StepCount;

		wxString Variable;
		wxString ValueList;

		int StepOrder;
		bool UseValueList;
		bool TuneBeforeStart;
	
		bool DoNotTune;
		bool TuneOnce;
		bool TuneEvery;
	
		long TuneEveryValue;
	
		bool Legacy;
	

		wxFlexGridSizer* UserlistPanelFGSizer;
		wxFlexGridSizer* InnerFGSizer;
	
		NFGAutoValidatedTextCtrl* ValueListTextCtrl;
	
		wxStaticText* StartLabelST;
		NFGAutoValidatedTextCtrl* StartTextCtrl;
		wxStaticText* StartsLabelST;
		wxStaticText* EndLabelST;
		NFGValidatedStaticText* EndST;
		wxStaticText* EndsLabelST;
		wxStaticText* NumberLabelST;
		NFGAutoValidatedTextCtrl* NumberTextCtrl;
		
		wxStaticText* StepLabelST;
		NFGAutoValidatedTextCtrl* StepTextCtrl;
		wxStaticText* StepsLabelST;
		wxStaticText* CoefficientLabelST;
		NFGAutoValidatedTextCtrl* CoefficientTextCtrl;
	
		wxStaticText* SumLabelST;
		NFGValidatedStaticText* SumST;
		wxStaticText* SumsLabelST;
		
		wxStaticText* VariableNameLabelST;
		NFGAutoValidatedTextCtrl* VariableNameTextCtrl;
	
		wxStaticText* StepOrderST;
		NFGAutoChoice* StepOrderChoice;
		
		NFGAutoCheckBox* UseValueListCheckBox;
		NFGAutoCheckBox* TuneBeforeStartCheckBox;
		
		wxStaticLine* StaticLine;
		NFGAutoRadioButton* DoNotTuneRadioButton;
		NFGAutoRadioButton* TuneOnceRadioButton;
		NFGAutoRadioButton* TuneEveryRadioButton;
		NFGAutoValidatedTextCtrl* TuneEveryTextCtrl;
		wxStaticText* TuneEverystepLabelST;

	public:
		NFGUserlistPanel(wxWindow* parent, unsigned long AssocValueType = ASSOC_NOT_SET);
		~NFGUserlistPanel();
	
		virtual unsigned char GetAssocType();
	
		virtual void CalcEnd();

		virtual bool Load(UserlistParams* UParams, bool LegacyFormat = false);
		virtual bool Store(UserlistParams* UParams, bool LegacyFormat = false);
	
		virtual void SetLegacy(bool LegacyFormat);
	
		virtual bool IsModified();
		virtual void Modify(bool mod);
	
		virtual bool TransferDataToWindow();
};


class NFGUserlistMainPanel : public wxPanel 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Event handlers
		void OnChange(wxCommandEvent& event);
		void OnPageChange(wxBookCtrlEvent& event);
	
		void OnUpdateUISave(wxUpdateUIEvent& event);
		void OnUpdateUI(wxUpdateUIEvent& event);
	
		void OnUserlistCommand(wxCommandEvent& event);
	
		wxDocument* Doc;
	
		wxFlexGridSizer* UserlistFGSizer;
		wxFlexGridSizer* OptionalFGSizer;
	
		wxNotebook* UserlistNotebook;

		NFGUserlistPanel* RFPowerPanel;
		NFGUserlistPanel* TriggerPanel;
		NFGUserlistPanel* SpectrumPanel;
		NFGUserlistPanel* VariablePanel;
		NFGUserlistPanel* InvRecPanel;
		NFGUserlistPanel* T2Panel;
		NFGUserlistPanel* NutationPanel;

		wxStaticText* DestinationST;
		NFGAutoValidatedTextCtrl* DestinationTC;
	
		wxStaticText* CommandST;
		wxStaticText* WrkST;
		wxStaticText* DstST;
		wxStaticText* BeforeExpST;
		wxStaticText* AfterExpST;
		wxStaticText* BeforeStepST;
		wxStaticText* AfterStepST;
	
		NFGAutoValidatedTextCtrl *RunBeforeExpWrkTC;
		NFGAutoValidatedTextCtrl *RunBeforeExpDstTC;
		NFGAutoValidatedTextCtrl *RunAfterExpWrkTC;
		NFGAutoValidatedTextCtrl *RunAfterExpDstTC;
		NFGAutoValidatedTextCtrl *RunBeforeStepWrkTC;
		NFGAutoValidatedTextCtrl *RunBeforeStepDstTC;
		NFGAutoValidatedTextCtrl *RunAfterStepWrkTC;
		NFGAutoValidatedTextCtrl *RunAfterStepDstTC;

		wxString Destination;
		
		wxString RunBeforeExpWrk;
		wxString RunBeforeExpDst;
		wxString RunAfterExpWrk;
		wxString RunAfterExpDst;
		wxString RunBeforeStepWrk;
		wxString RunBeforeStepDst;
		wxString RunAfterStepWrk;
		wxString RunAfterStepDst;
		
		bool UseOptionalSettings;

		bool Legacy;
		
		NFGAutoCheckBox* UseOptionalSettingsCheckBox;
		
		wxButton* UserlistSaveButton;
		wxButton* UserlistRevertButton;
		
		unsigned char AssocValueType;
	
	public:
		NFGUserlistMainPanel(wxDocument* document, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1,-1), long style = wxTAB_TRAVERSAL);
		~NFGUserlistMainPanel();

		NFGUserlistPanel* FindPanelByType(unsigned char AssocType, bool Select = false);
	
		bool Load(UserlistParams* UParams, bool LegacyFormat = false);
		bool Store(UserlistParams* UParams, bool LegacyFormat = false);
		
		bool IsModified();
		void Modify(bool mod);
};

#endif
