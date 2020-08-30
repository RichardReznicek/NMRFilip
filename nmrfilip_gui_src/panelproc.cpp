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

#include "panelproc.h"
#include <wx/artprov.h>
#include <wx/valgen.h>
#include <cmath>

#include "validators.h"
#include "view.h"
#include "doc.h"
#include "nmrfilipgui.h"
#include "gui_ids.h"


#define SpinHalfRange	32000

BEGIN_EVENT_TABLE(NFGProcessPanel, wxPanel)
	EVT_BUTTON(ID_DefaultFirstLastChunk, NFGProcessPanel::OnDefaultFirstLastChunk)
	EVT_BUTTON(ID_DefaultFirstLastChunkPoint, NFGProcessPanel::OnDefaultFirstLastChunkPoint)
	EVT_UPDATE_UI(ID_Process_Panel, NFGProcessPanel::OnUpdateUI)
	EVT_BUTTON(ID_ProcessLoad, NFGProcessPanel::OnProcessCommand)
	EVT_BUTTON(ID_ProcessStore, NFGProcessPanel::OnProcessCommand)
	EVT_BUTTON(ID_ProcessApply, NFGProcessPanel::OnProcessCommand)
	EVT_BUTTON(ID_ProcessRevert, NFGProcessPanel::OnProcessCommand)
	EVT_TOGGLEBUTTON(ID_ProcessAutoApply, NFGProcessPanel::OnProcessCommand)


	EVT_TEXT(ID_FirstChunk, NFGProcessPanel::OnProcessChangeCommand)
	EVT_TEXT(ID_LastChunk, NFGProcessPanel::OnProcessChangeCommand)
	EVT_TEXT(ID_FirstChunkPoint, NFGProcessPanel::OnProcessChangeCommand)
	EVT_TEXT(ID_LastChunkPoint, NFGProcessPanel::OnProcessChangeCommand)
	EVT_TEXT(ID_FFTLength, NFGProcessPanel::OnProcessChangeCommand)
	EVT_TEXT(ID_Filter, NFGProcessPanel::OnProcessChangeCommand)

	EVT_COMMAND(wxID_ANY, NFGEVT_ADJUST_VALUES, NFGProcessPanel::OnAdjustValues)

	EVT_CHECKBOX(ID_EnableFirstLastChunk, NFGProcessPanel::OnProcessChangeCommand)
	EVT_CHECKBOX(ID_EnableFirstLastChunkPoint, NFGProcessPanel::OnProcessChangeCommand)
	EVT_CHECKBOX(ID_FilterEnable, NFGProcessPanel::OnProcessChangeCommand)
	EVT_CHECKBOX(ID_ScaleFirstTDPointEnable, NFGProcessPanel::OnProcessChangeCommand)

	EVT_LISTBOX(ID_FFTEnvelopeOmit, NFGProcessPanel::OnProcessChangeCommand)

	EVT_SPIN_UP(ID_FirstChunkSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_DOWN(ID_FirstChunkSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_UP(ID_LastChunkSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_DOWN(ID_LastChunkSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_UP(ID_FirstChunkPointSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_DOWN(ID_LastChunkPointSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_UP(ID_LastChunkPointSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_DOWN(ID_LastChunkPointSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_UP(ID_FFTLengthSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_DOWN(ID_FFTLengthSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_UP(ID_FilterSpin, NFGProcessPanel::OnSpin)
	EVT_SPIN_DOWN(ID_FilterSpin, NFGProcessPanel::OnSpin)

	EVT_MOUSEWHEEL(NFGProcessPanel::OnMouseWheel)
	EVT_CHAR_HOOK(NFGProcessPanel::OnChar)
END_EVENT_TABLE()

NFGProcessPanel::NFGProcessPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	CurrentSerDocument = NULL;
	
	params.UseFirstLastChunk = false;
	params.FirstChunk = 0;
	params.LastChunk = INT_MAX;
	params.UseFirstLastChunkPoint = false;
	params.FirstChunkPoint = 0;
	params.LastChunkPoint = INT_MAX;
	params.FFTLength = 128;
	params.UseFilter = false;
	params.Filter = 0.1;
	params.ScaleFirstTDPoint = false;
	params.AutoApply = false;
	
	PostponeChanges = false;

	MaxLastChunk = 0;
	MaxLastChunkPoint = 0;
	
	wxFlexGridSizer* ProcessFGSizer;
	ProcessFGSizer = new wxFlexGridSizer(5, 1, FromDIP(7), 0);
	ProcessFGSizer->SetFlexibleDirection(wxBOTH);
	ProcessFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ProcessFGSizer->AddGrowableCol(0);
	ProcessFGSizer->AddGrowableRow(3);
	
	wxFlexGridSizer* ProcessTopFGSizer;
	ProcessTopFGSizer = new wxFlexGridSizer(6, 3, FromDIP(2), FromDIP(12));
	ProcessTopFGSizer->SetFlexibleDirection(wxBOTH);
	ProcessTopFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ProcessTopFGSizer->AddGrowableCol(0);
	ProcessTopFGSizer->AddGrowableCol(1);

	FirstChunkLabelST = new wxStaticText(this, wxID_ANY, "First chunk");
	ProcessTopFGSizer->Add(FirstChunkLabelST, 0, wxALL|wxALIGN_BOTTOM, 0);
	
	LastChunkLabelST = new wxStaticText(this, wxID_ANY, "Last chunk");
	ProcessTopFGSizer->Add(LastChunkLabelST, 0, wxALL|wxALIGN_BOTTOM, 0);
	
	EnableFirstLastChunkCheckBox = new wxCheckBox(this, ID_EnableFirstLastChunk, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(params.UseFirstLastChunk)));
	
	EnableFirstLastChunkCheckBox->SetToolTip("Enable first and last chunk settings");
	
	ProcessTopFGSizer->Add(EnableFirstLastChunkCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 0);
	
	wxFlexGridSizer* FirstChunkSizer;
	FirstChunkSizer = new wxFlexGridSizer(1, 2, 0, 0);
	FirstChunkSizer->SetFlexibleDirection(wxBOTH);
	FirstChunkSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	FirstChunkSizer->AddGrowableCol(0);

	FirstChunkTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_FirstChunk, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(80,-1)), 0,
							 NFGLongValidator(&(params.FirstChunk), NON_NEGATIVE_NUMBER), wxTextCtrlNameStr, false, false, true, &PostponeChanges);
	
	FirstChunkSpinButton = new wxSpinButton(this, ID_FirstChunkSpin, wxDefaultPosition, wxSize(-1, FirstChunkTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP);
#ifdef __WXGTK__
	FirstChunkSpinButton->SetCanFocus(false);
#endif
	FirstChunkSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	FirstChunkSpinButton->SetValue(0);
	FirstChunkSpinVal = 0;
	
	FirstChunkSizer->Add(FirstChunkTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	FirstChunkSizer->Add(FirstChunkSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ProcessTopFGSizer->Add(FirstChunkSizer, 0, wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(5));
	

	wxFlexGridSizer* LastChunkSizer;
	LastChunkSizer = new wxFlexGridSizer(1, 2, 0, 0);
	LastChunkSizer->SetFlexibleDirection(wxBOTH);
	LastChunkSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	LastChunkSizer->AddGrowableCol(0);

	LastChunkTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_LastChunk, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(80,-1)), 0,
							 NFGLongValidator(&(params.LastChunk), NON_NEGATIVE_NUMBER), wxTextCtrlNameStr, false, false, true, &PostponeChanges);
	
	LastChunkSpinButton = new wxSpinButton(this, ID_LastChunkSpin, wxDefaultPosition, wxSize(-1, LastChunkTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP);
#ifdef __WXGTK__
	LastChunkSpinButton->SetCanFocus(false);
#endif
	LastChunkSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	LastChunkSpinButton->SetValue(0);
	LastChunkSpinVal = 0;
	
	LastChunkSizer->Add(LastChunkTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	LastChunkSizer->Add(LastChunkSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ProcessTopFGSizer->Add(LastChunkSizer, 0, wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(5));
	
	
	DefaultFirstLastChunkButton = new wxButton(this, ID_DefaultFirstLastChunk, "Def", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	DefaultFirstLastChunkButton->SetToolTip("Default setting");
	
	ProcessTopFGSizer->Add(DefaultFirstLastChunkButton, 0, wxBOTTOM|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	
	FirstChunkPointLabelST = new wxStaticText(this, wxID_ANY, "First chunk point");
	ProcessTopFGSizer->Add(FirstChunkPointLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	LastChunkPointLabelST = new wxStaticText(this, wxID_ANY, "Last chunk point");
	ProcessTopFGSizer->Add(LastChunkPointLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EnableFirstLastChunkPointCheckBox = new wxCheckBox(this, ID_EnableFirstLastChunkPoint, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(params.UseFirstLastChunkPoint)));
	
	EnableFirstLastChunkPointCheckBox->SetToolTip("Enable first and last chunk point settings");
	
	ProcessTopFGSizer->Add(EnableFirstLastChunkPointCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 0);


	wxFlexGridSizer* FirstChunkPointSizer;
	FirstChunkPointSizer = new wxFlexGridSizer(1, 2, 0, 0);
	FirstChunkPointSizer->SetFlexibleDirection(wxBOTH);
	FirstChunkPointSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	FirstChunkPointSizer->AddGrowableCol(0);

	FirstChunkPointTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_FirstChunkPoint, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(80,-1)), 0,
							 NFGLongValidator(&(params.FirstChunkPoint), NON_NEGATIVE_NUMBER), wxTextCtrlNameStr, false, false, true, &PostponeChanges);
	
	FirstChunkPointSpinButton = new wxSpinButton(this, ID_FirstChunkPointSpin, wxDefaultPosition, wxSize(-1, FirstChunkPointTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP);
#ifdef __WXGTK__
	FirstChunkPointSpinButton->SetCanFocus(false);
#endif
	FirstChunkPointSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	FirstChunkPointSpinButton->SetValue(0);
	FirstChunkPointSpinVal = 0;
	
	FirstChunkPointSizer->Add(FirstChunkPointTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	FirstChunkPointSizer->Add(FirstChunkPointSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ProcessTopFGSizer->Add(FirstChunkPointSizer, 0, wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(5));
	
	
	wxFlexGridSizer* LastChunkPointSizer;
	LastChunkPointSizer = new wxFlexGridSizer(1, 2, 0, 0);
	LastChunkPointSizer->SetFlexibleDirection(wxBOTH);
	LastChunkPointSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	LastChunkPointSizer->AddGrowableCol(0);

	LastChunkPointTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_LastChunkPoint, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(80,-1)), 0,
							 NFGLongValidator(&(params.LastChunkPoint), NON_NEGATIVE_NUMBER), wxTextCtrlNameStr, false, false, true, &PostponeChanges);

	LastChunkPointSpinButton = new wxSpinButton(this, ID_LastChunkPointSpin, wxDefaultPosition, wxSize(-1, LastChunkPointTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP);
#ifdef __WXGTK__
	LastChunkPointSpinButton->SetCanFocus(false);
#endif
	LastChunkPointSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	LastChunkPointSpinButton->SetValue(0);
	LastChunkPointSpinVal = 0;
	
	LastChunkPointSizer->Add(LastChunkPointTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	LastChunkPointSizer->Add(LastChunkPointSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ProcessTopFGSizer->Add(LastChunkPointSizer, 0, wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(5));
	
	
	DefaultFirstLastChunkPointButton = new wxButton(this, ID_DefaultFirstLastChunkPoint, "Def", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	DefaultFirstLastChunkPointButton->SetToolTip("Default setting");
	
	ProcessTopFGSizer->Add(DefaultFirstLastChunkPointButton, 0, wxBOTTOM|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	FFTLengthLabelST = new wxStaticText(this, wxID_ANY, "FFT length");
	ProcessTopFGSizer->Add(FFTLengthLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	FilterCheckBox = new wxCheckBox(this, ID_FilterEnable, "Filter [MHz]", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(params.UseFilter)));
	
	ProcessTopFGSizer->Add(FilterCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ProcessTopFGSizer->Add(0, 0, 1, wxEXPAND, 0);
	
	
	wxFlexGridSizer* FFTLengthSizer;
	FFTLengthSizer = new wxFlexGridSizer(1, 2, 0, 0);
	FFTLengthSizer->SetFlexibleDirection(wxBOTH);
	FFTLengthSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	FFTLengthSizer->AddGrowableCol(0);

	FFTLengthTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_FFTLength, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(64,-1)), 0,
							 NFGLongValidator(&(params.FFTLength), POSITIVE_NUMBER), wxTextCtrlNameStr, false, false, false, &PostponeChanges);
	
	FFTLengthSpinButton = new wxSpinButton(this, ID_FFTLengthSpin, wxDefaultPosition, wxSize(-1, FFTLengthTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP);
#ifdef __WXGTK__
	FFTLengthSpinButton->SetCanFocus(false);
#endif
	FFTLengthSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	FFTLengthSpinButton->SetValue(0);
	FFTLengthSpinVal = 0;
	
	FFTLengthSizer->Add(FFTLengthTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	FFTLengthSizer->Add(FFTLengthSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	ProcessTopFGSizer->Add(FFTLengthSizer, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);


	wxFlexGridSizer* FilterSizer;
	FilterSizer = new wxFlexGridSizer(1, 2, 0, 0);
	FilterSizer->SetFlexibleDirection(wxBOTH);
	FilterSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	FilterSizer->AddGrowableCol(0);

	FilterTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_Filter, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(80,-1)), 0,
							 NFGDoubleValidator(&(params.Filter)), wxTextCtrlNameStr, false, false, false, &PostponeChanges);

	FilterSpinButton = new wxSpinButton(this, ID_FilterSpin, wxDefaultPosition, wxSize(-1, FilterTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP);
#ifdef __WXGTK__
	FilterSpinButton->SetCanFocus(false);
#endif
	FilterSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	FilterSpinButton->SetValue(0);
	FilterSpinVal = 0;
	
	FilterSizer->Add(FilterTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	FilterSizer->Add(FilterSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	ProcessTopFGSizer->Add(FilterSizer, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	
	ProcessTopFGSizer->Add(0, 0, 1, wxEXPAND, 0);
	
	FilterCheckBox->MoveBeforeInTabOrder(FilterTextCtrl);
	
	ProcessFGSizer->Add(ProcessTopFGSizer, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, FromDIP(5));
	

	ScaleFirstTDPointCheckBox = new wxCheckBox(this, ID_ScaleFirstTDPointEnable, "Scale the first chunk point by 0.5", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(params.ScaleFirstTDPoint)));

	ProcessFGSizer->Add(ScaleFirstTDPointCheckBox, 0, wxLEFT|wxRIGHT, FromDIP(8));


	wxFlexGridSizer* ProcessLoadStoreFGSizer;
	ProcessLoadStoreFGSizer = new wxFlexGridSizer(1, 2, 0, FromDIP(6));
	ProcessLoadStoreFGSizer->AddGrowableCol(0);
	ProcessLoadStoreFGSizer->AddGrowableCol(1);
	ProcessLoadStoreFGSizer->SetFlexibleDirection(wxHORIZONTAL);
	ProcessLoadStoreFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


	ProcessLoadButton = new wxButton(this, ID_ProcessLoad, "Load <");
	ProcessLoadStoreFGSizer->Add(ProcessLoadButton, 0, wxALL|wxEXPAND, 0);
	
	ProcessStoreButton = new wxButton(this, ID_ProcessStore, "> Store");
	ProcessLoadStoreFGSizer->Add(ProcessStoreButton, 0, wxALL|wxEXPAND, 0);
	
	ProcessFGSizer->Add(ProcessLoadStoreFGSizer, 1, wxEXPAND|wxLEFT|wxRIGHT, FromDIP(5));
	
	
	wxStaticBoxSizer* ProcessCenterSBSizer;
	ProcessCenterSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Omit from FFT envelope"), wxVERTICAL);
	
	OmitListBox = new wxListBox(this, ID_FFTEnvelopeOmit, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE); 
	/// just to prevent strange sizing behavior
	OmitListBox->SetMinSize(wxSize(10, 10));
	
	ProcessCenterSBSizer->Add(OmitListBox, 1, wxALL|wxEXPAND, FromDIP(3));
	
	ProcessFGSizer->Add(ProcessCenterSBSizer, 1, wxEXPAND|wxLEFT|wxRIGHT, FromDIP(6));
	

	wxGridSizer* ProcessBottomFGSizer;
	ProcessBottomFGSizer = new wxGridSizer(1, 3, 0, FromDIP(7));
	
	ProcessAutoApplyTgButton = new wxToggleButton(this, ID_ProcessAutoApply, "Auto-Apply", wxDefaultPosition, wxSize(-1, ProcessLoadButton->GetSize().GetHeight()), wxBU_EXACTFIT, wxGenericValidator(&(params.AutoApply)));
	
	ProcessBottomFGSizer->Add(ProcessAutoApplyTgButton, 0, wxALL|wxEXPAND, 0);
	
	ProcessApplyButton = new wxButton(this, ID_ProcessApply, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ProcessBottomFGSizer->Add(ProcessApplyButton, 0, wxALL|wxEXPAND, 0);
	
	ProcessRevertButton = new wxButton(this, ID_ProcessRevert, "Revert", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ProcessBottomFGSizer->Add(ProcessRevertButton, 0, wxALL|wxEXPAND, 0);
	

	ProcessFGSizer->Add(ProcessBottomFGSizer, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, FromDIP(5));
	
	this->SetSizer(ProcessFGSizer);
	this->Layout();
	ProcessFGSizer->Fit(this);

	/// will be enabled by OnUpdateUI() handler if needed
	Disable();
}

NFGProcessPanel::~NFGProcessPanel()
{
}

void NFGProcessPanel::OnDefaultFirstLastChunk(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;

	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;
	
	wxString str("0");
	FirstChunkTextCtrl->SetValue(str);
	
	long LastChunk = CurrentSerDocument->LastChunkQuery();
	str.Printf("%ld", LastChunk);
	LastChunkTextCtrl->SetValue(str);
}

void NFGProcessPanel::OnDefaultFirstLastChunkPoint(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	wxString str("0");
	FirstChunkPointTextCtrl->SetValue(str);
	
	long LastChunkPoint = CurrentSerDocument->LastChunkPointQuery();
	str.Printf("%ld", LastChunkPoint);
	LastChunkPointTextCtrl->SetValue(str);
}

void NFGProcessPanel::OnSpin(wxSpinEvent& event) 
{
	int SpinValue = event.GetPosition();
	int increment = 0;

#define CalcIncrement(incr, curval, spinval)	\
	{	\
		(incr) = (curval) - (spinval);	\
		if ((incr) > SpinHalfRange) 	\
			(incr) -= 2*SpinHalfRange;	\
		if ((incr) < -SpinHalfRange) 	\
			(incr) += 2*SpinHalfRange;	\
		\
		(spinval) = (curval);	\
	}
	
	switch (event.GetId()) {
		case ID_FirstChunkSpin:
			CalcIncrement(increment, SpinValue, FirstChunkSpinVal);
			SpinChunks(increment, MaxLastChunk, FirstChunkTextCtrl);
			break;
		
		case ID_LastChunkSpin:
			CalcIncrement(increment, SpinValue, LastChunkSpinVal);
			SpinChunks(increment, MaxLastChunk, LastChunkTextCtrl);
			break;
		
		case ID_FirstChunkPointSpin:
			CalcIncrement(increment, SpinValue, FirstChunkPointSpinVal);
			SpinChunks(increment, MaxLastChunkPoint, FirstChunkPointTextCtrl);
			break;
		
		case ID_LastChunkPointSpin:
			CalcIncrement(increment, SpinValue, LastChunkPointSpinVal);
			SpinChunks(increment, MaxLastChunkPoint, LastChunkPointTextCtrl);
			break;
		
		case ID_FFTLengthSpin:
			CalcIncrement(increment, SpinValue, FFTLengthSpinVal);
			SpinFFTLength(increment);
			break;
		
		case ID_FilterSpin:
			CalcIncrement(increment, SpinValue, FilterSpinVal);
			SpinFilter(increment);
			break;
		
		default:
			;
	}
	
#undef CalcIncrement
	
	event.Skip();
}

void NFGProcessPanel::OnChar(wxKeyEvent& event) 
{
	int increment = 0;
	NFGAutoValidatedTextCtrl *FocusedWin = wxDynamicCast(wxWindow::FindFocus(), NFGAutoValidatedTextCtrl);
	if (FocusedWin == NULL) {
		event.Skip();
		return;
	}
	
	switch (event.GetKeyCode()) {
		case WXK_UP:
		case WXK_NUMPAD_UP:
			increment = 1;
			break;

		case WXK_DOWN:
		case WXK_NUMPAD_DOWN:
			increment = -1;
			break;

		case WXK_PAGEUP:
		case WXK_NUMPAD_PAGEUP:
			increment = 4;
			break;

		case WXK_PAGEDOWN:
		case WXK_NUMPAD_PAGEDOWN:
			increment = -4;
			break;

		default:
			event.Skip();
			return;
	}
	
	if ((FocusedWin == FirstChunkTextCtrl) || (FocusedWin == LastChunkTextCtrl))
		SpinChunks(increment, MaxLastChunk, FocusedWin);
	else 
	if ((FocusedWin == FirstChunkPointTextCtrl) || (FocusedWin == LastChunkPointTextCtrl))
		SpinChunks(increment, MaxLastChunkPoint, FocusedWin);
	else 
	if (FocusedWin == FFTLengthTextCtrl)
		SpinFFTLength(increment);
	else
	if (FocusedWin == FilterTextCtrl)
		SpinFilter(increment);
	else
		event.Skip();
}

void NFGProcessPanel::OnMouseWheel(wxMouseEvent& event) 
{
	NFGAutoValidatedTextCtrl *FocusedWin = wxDynamicCast(wxWindow::FindFocus(), NFGAutoValidatedTextCtrl);
	if (FocusedWin == NULL) {
		event.Skip();
		return;
	}
	
	if ((FocusedWin == FirstChunkTextCtrl) || (FocusedWin == LastChunkTextCtrl))
		SpinChunks(event.GetWheelRotation()/event.GetWheelDelta(), MaxLastChunk, FocusedWin);
	else 
	if ((FocusedWin == FirstChunkPointTextCtrl) || (FocusedWin == LastChunkPointTextCtrl))
		SpinChunks(event.GetWheelRotation()/event.GetWheelDelta(), MaxLastChunkPoint, FocusedWin);
	else 
	if (FocusedWin == FFTLengthTextCtrl)
		SpinFFTLength(event.GetWheelRotation()/event.GetWheelDelta());
	else
	if (FocusedWin == FilterTextCtrl)
		SpinFilter(event.GetWheelRotation()/event.GetWheelDelta());
	else
		event.Skip();
}

void NFGProcessPanel::SpinChunks(int step, long max, NFGAutoValidatedTextCtrl* control) 
{
	if ((step == 0) || (control == NULL)) 
		return;
	
	wxValidator *validator = control->GetValidator();
	if (validator == NULL) 
		return;

	if (!(validator->Validate(this))) {
		PostponeChanges = false;
		validator->TransferToWindow();
		PostponeChanges = params.AutoApply;
		return;
	}

	long value, oldvalue, auxvalue;
	wxString str = control->GetValue();
	if (!(str.ToLong(&value))) {
		wxBell();
		oldvalue = 0;
		value = 0;
	} else {
		oldvalue = value;
		value += step;
	}
	
	if (value < 0) 
		value = 0;
	if (value > max) 
		value = max;
	
	/// adjust the pair value for convenience
	if (control == FirstChunkTextCtrl) {
		str = LastChunkTextCtrl->GetValue();
		if (str.ToLong(&auxvalue) && (value > auxvalue) && (oldvalue <= auxvalue)) {
			str.Printf("%ld", value);
			LastChunkTextCtrl->ChangeValue(str);	/// it should not have a focus so we can avoid another event if AutoApply is on
		}
	}
	
	if (control == LastChunkTextCtrl) {
		str = FirstChunkTextCtrl->GetValue();
		if (str.ToLong(&auxvalue) && (value < auxvalue) && (oldvalue >= auxvalue)) {
			str.Printf("%ld", value);
			FirstChunkTextCtrl->ChangeValue(str);	/// it should not have a focus so we can avoid another event if AutoApply is on
		}
	}
	
	if (control == FirstChunkPointTextCtrl) {
		str = LastChunkPointTextCtrl->GetValue();
		if (str.ToLong(&auxvalue) && (value > auxvalue) && (oldvalue <= auxvalue)) {
			str.Printf("%ld", value);
			LastChunkPointTextCtrl->ChangeValue(str);	/// it should not have a focus so we can avoid another event if AutoApply is on
		}
	}
	
	if (control == LastChunkPointTextCtrl) {
		str = FirstChunkPointTextCtrl->GetValue();
		if (str.ToLong(&auxvalue) && (value < auxvalue) && (oldvalue >= auxvalue)) {
			str.Printf("%ld", value);
			FirstChunkPointTextCtrl->ChangeValue(str);	/// it should not have a focus so we can avoid another event if AutoApply is on
		}
	}
	
	str.Printf("%ld", value);
	control->SetValue(str);
}

void NFGProcessPanel::SpinFFTLength(int step) 
{
	if (step == 0) 
		return;

	long FFTLen;
	wxString str = FFTLengthTextCtrl->GetValue();
	if (!(str.ToLong(&FFTLen))) {
		wxBell();
		FFTLen = params.FFTLength;
		if (FFTLen < 1) 
			FFTLen = 1;
		
	} else {
		long order = 0;
		
		if (FFTLen < 1) {
			order = 0;
			step = 0;
		}
		
		if (FFTLen > 0x200000) {
			order = 2*21;
			step = 0;
		}
		
		if (step > 0) {
			order = 2 * NFGMSTD lround(std::floor(NFGMSTD log2(FFTLen)));
			if ((3ul << (order/2))/2 <= (unsigned long) FFTLen) 
				order++;
		}
		
		if (step < 0) {
			order = 2 * NFGMSTD lround(std::ceil(NFGMSTD log2(FFTLen)));
			if ((3ul << (order/2))/4 >= (unsigned long) FFTLen) 
				order--;
		}
		
		order += step;
		if (order < 0)
			order = 0;
		if (order > 2*21)
			order = 2*21;
		
		FFTLen = 1ul << (order/2);
		if (order%2)
			FFTLen += FFTLen/2;
	}
	
	str.Printf("%ld", FFTLen);
	FFTLengthTextCtrl->SetValue(str);
}

void NFGProcessPanel::SpinFilter(int step) 
{
	if ((step == 0) || (FilterTextCtrl == NULL)) 
		return;

	double dvalue = 0.0;
	wxString val(FilterTextCtrl->GetValue());
	val.Replace(",", ".");	/// accept both '.' and ',' decimal point
	if (!val.ToCDouble(&dvalue) || !wxFinite(dvalue)) {
		wxBell();
		dvalue = params.Filter;
		step = 0;
	}
	
	if (dvalue > 2.0e3) {
		dvalue = 2.0e3;
		step = 0;
	}
	
	if (dvalue < 1.0e-6) {
		dvalue = 1.0e-6;
		step = 0;
	}
	
	double order = std::floor(std::log10(dvalue));
	long mant = NFGMSTD lround(dvalue*std::pow(10.0, 2.0 - order));
	
	const double presel[64] = {	/// the difference between adjacent values is 5% or less
	100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 
	150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 
	200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 
	300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 
	400, 420, 440, 460, 480, 500, 520, 540, 560, 580, 
	600, 620, 650, 670, 700, 720, 750, 770, 800, 830, 
	860, 900, 930, 960};
	
	int index = 0;
	
	if (step > 0)
		for (index = 63; (index >= 0) && (mant < presel[index]); index--)
			;
	else
		for (index = 0; (index < 64) && (mant > presel[index]); index++)
			;
	
	index += step;
	order += index/64;
	index %= 64;
	if (index < 0) {
		index += 64;
		order -= 1.0;
	}
	
	dvalue = presel[index]*std::pow(10.0, order - 2.0);

	if (dvalue > 2.0e3)
		dvalue = 2.0e3;
	if (dvalue < 1.0e-6)
		dvalue = 1.0e-6;

	val.Printf("%.14g", dvalue);
	FilterTextCtrl->SetValue(val);
}

void NFGProcessPanel::OnProcessCommand(wxCommandEvent& event)
{
	unsigned long StepCnt = 0;

	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	
	switch(event.GetId()) {
		case ID_ProcessLoad:
			params = DocMan->LoadSerProcParams();
			/// apply loaded params immediately
			params = CurrentSerDocument->SetProcParams(params);
			TransferDataToWindow();
			break;
			
		case ID_ProcessStore:
			if (Validate()) {
				TransferDataFromWindow();
				DocMan->StoreSerProcParams(params);
			} else
				wxBell();
			break;
		
		case ID_ProcessApply:
			if (Validate()) {
				TransferDataFromWindow();
				
				StepCnt = OmitListBox->GetCount();
				for (unsigned long i = 0; i < StepCnt; i++) 
					CurrentSerDocument->StepNoFFTEnvelopeCommand(i, OmitListBox->IsSelected(i), true);
				CurrentSerDocument->StepStatesChangedQuery();	/// just clear StepStatesChanged
				
				params = CurrentSerDocument->SetProcParams(params);
				TransferDataToWindow();
			} else
				wxBell();
			break;
			
		case ID_ProcessRevert:
			params = CurrentSerDocument->GetProcParams();
			TransferDataToWindow();
			
			StepCnt = CurrentSerDocument->StepCountQuery();
			for (unsigned long i = 0; i < StepCnt; i++) 
				if (CurrentSerDocument->StepNoFFTEnvelopeQuery(i))
					OmitListBox->Select(i);
				else
					OmitListBox->Deselect(i);
			
			break;
			
		case ID_ProcessAutoApply:
			params.AutoApply = event.IsChecked();
			ProcessApplyButton->Enable(!params.AutoApply);
			ProcessRevertButton->Enable(!params.AutoApply);
		
			PostponeChanges = params.AutoApply;
		
			if (Validate()) {
				TransferDataFromWindow();
				
				StepCnt = OmitListBox->GetCount();
				for (unsigned long i = 0; i < StepCnt; i++) 
					CurrentSerDocument->StepNoFFTEnvelopeCommand(i, OmitListBox->IsSelected(i), true);
				CurrentSerDocument->StepStatesChangedQuery();	/// just clear StepStatesChanged
				
				params = CurrentSerDocument->SetProcParams(params);
				TransferDataToWindow();
			} else
				wxBell();
			break;
			
		default:
			;
	}
	
	FirstChunkTextCtrl->Enable(EnableFirstLastChunkCheckBox->IsChecked());
	FirstChunkSpinButton->Enable(FirstChunkTextCtrl->IsEnabled());
	LastChunkTextCtrl->Enable(EnableFirstLastChunkCheckBox->IsChecked());
	LastChunkSpinButton->Enable(LastChunkTextCtrl->IsEnabled());
	
	FirstChunkPointTextCtrl->Enable(EnableFirstLastChunkPointCheckBox->IsChecked());
	FirstChunkPointSpinButton->Enable(FirstChunkPointTextCtrl->IsEnabled());
	LastChunkPointTextCtrl->Enable(EnableFirstLastChunkPointCheckBox->IsChecked());
	LastChunkPointSpinButton->Enable(LastChunkPointTextCtrl->IsEnabled());
	
	FilterTextCtrl->Enable(FilterCheckBox->IsChecked());
	FilterSpinButton->Enable(FilterTextCtrl->IsEnabled());
}

void NFGProcessPanel::OnProcessChangeCommand(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	FirstChunkTextCtrl->Enable(EnableFirstLastChunkCheckBox->IsChecked());
	FirstChunkSpinButton->Enable(FirstChunkTextCtrl->IsEnabled());
	LastChunkTextCtrl->Enable(EnableFirstLastChunkCheckBox->IsChecked());
	LastChunkSpinButton->Enable(LastChunkTextCtrl->IsEnabled());
	
	FirstChunkPointTextCtrl->Enable(EnableFirstLastChunkPointCheckBox->IsChecked());
	FirstChunkPointSpinButton->Enable(FirstChunkPointTextCtrl->IsEnabled());
	LastChunkPointTextCtrl->Enable(EnableFirstLastChunkPointCheckBox->IsChecked());
	LastChunkPointSpinButton->Enable(LastChunkPointTextCtrl->IsEnabled());
	
	FilterTextCtrl->Enable(FilterCheckBox->IsChecked());
	FilterSpinButton->Enable(FilterTextCtrl->IsEnabled());

	if (!params.AutoApply)
		return;
	
	if (event.GetEventType() == wxEVT_LISTBOX) {
		if (event.IsSelection()) {
			CurrentSerDocument->StepNoFFTEnvelopeCommand(event.GetSelection(), true, false);
		} else {
			unsigned long StepCnt = OmitListBox->GetCount();
			for (unsigned long i = 0; i < StepCnt; i++) 
				CurrentSerDocument->StepNoFFTEnvelopeCommand(i, OmitListBox->IsSelected(i), (i+1) < StepCnt);
		}
		CurrentSerDocument->StepStatesChangedQuery();	/// just clear StepStatesChanged
		
	} else {
		if ((event.GetEventType() == wxEVT_TEXT) && event.GetString().IsEmpty())
			return;	/// do not beep when user deletes the value before a new one
		
		if (Validate()) {
			TransferDataFromWindow();
			params = CurrentSerDocument->SetProcParams(params, false, false);
			TransferDataToWindow();
		} else {
			wxBell();
		}
	}
}

void NFGProcessPanel::OnAdjustValues(wxCommandEvent& event)
{
	NFGAutoValidatedTextCtrl *PrimaryCtrl = NULL, *SecondaryCtrl = NULL;
	bool PrimaryLarger = true;
	long value = 0, auxvalue = 0;
	
	switch (event.GetId()) {
		case ID_FirstChunk:
			PrimaryCtrl = FirstChunkTextCtrl;
			SecondaryCtrl = LastChunkTextCtrl;
			PrimaryLarger = false;
			break;
		
		case ID_LastChunk:
			PrimaryCtrl = LastChunkTextCtrl;
			SecondaryCtrl = FirstChunkTextCtrl;
			PrimaryLarger = true;
			break;
		
		case ID_FirstChunkPoint: 
			PrimaryCtrl = FirstChunkPointTextCtrl;
			SecondaryCtrl = LastChunkPointTextCtrl;
			PrimaryLarger = false;
			break;
		
		case ID_LastChunkPoint:
			PrimaryCtrl = LastChunkPointTextCtrl;
			SecondaryCtrl = FirstChunkPointTextCtrl;
			PrimaryLarger = true;
			break;
		
		default: 
			return;
	}
	
	wxString str = PrimaryCtrl->GetValue();
	if (!(str.ToLong(&value)))
		return;
		
	str = SecondaryCtrl->GetValue();
	if (!str.ToLong(&auxvalue)) 
		return;
	
	/// adjust the pair value for convenience
	if ((PrimaryLarger && (value < auxvalue)) || (!PrimaryLarger && (value > auxvalue))) {
		str.Printf("%ld", value);
		SecondaryCtrl->SetValue(str);
	}
}

void NFGProcessPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if (SerDoc != CurrentSerDocument) 
		OmitListBox->Clear();

	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) {
		CurrentSerDocument = NULL;
		Disable();
		return;
	}

	Enable();
	
	if (SerDoc->ProcParamsChangedQuery() || (SerDoc != CurrentSerDocument)) {
		
		params = SerDoc->GetProcParams();
		
		PostponeChanges = false;
		
		MaxLastChunk = SerDoc->LastChunkQuery(); 
		MaxLastChunkPoint = SerDoc->LastChunkPointQuery();
		
		TransferDataToWindow();
		
		PostponeChanges = params.AutoApply;
		
		ProcessApplyButton->Enable(!params.AutoApply);
		ProcessRevertButton->Enable(!params.AutoApply);
		
		FirstChunkTextCtrl->Enable(EnableFirstLastChunkCheckBox->IsChecked());
		FirstChunkSpinButton->Enable(FirstChunkTextCtrl->IsEnabled());
		LastChunkTextCtrl->Enable(EnableFirstLastChunkCheckBox->IsChecked());
		LastChunkSpinButton->Enable(LastChunkTextCtrl->IsEnabled());
		
		FirstChunkPointTextCtrl->Enable(EnableFirstLastChunkPointCheckBox->IsChecked());
		FirstChunkPointSpinButton->Enable(FirstChunkPointTextCtrl->IsEnabled());
		LastChunkPointTextCtrl->Enable(EnableFirstLastChunkPointCheckBox->IsChecked());
		LastChunkPointSpinButton->Enable(LastChunkPointTextCtrl->IsEnabled());
		
		FilterTextCtrl->Enable(FilterCheckBox->IsChecked());
		FilterSpinButton->Enable(FilterTextCtrl->IsEnabled());
	}
	
	if (SerDoc->StepStatesChangedQuery() || (SerDoc != CurrentSerDocument)) {
		unsigned long StepCnt = SerDoc->StepCountQuery();
		
		if (StepCnt > 0) {
			wxString *StepLabels = new wxString[StepCnt];
			
			for (unsigned long i = 0; i < StepCnt; i++) 
				StepLabels[i] = SerDoc->StepLabelQuery(i);
			
			OmitListBox->Set(StepCnt, StepLabels);
			
			delete[] StepLabels;
			
			for (unsigned long i = 0; i < StepCnt; i++) 
				if (SerDoc->StepNoFFTEnvelopeQuery(i))
					OmitListBox->Select(i);
				else /// just in case
					OmitListBox->Deselect(i);
		} else
			OmitListBox->Clear();

	}
	
	CurrentSerDocument = SerDoc;
}


BEGIN_EVENT_TABLE(NFGPhasePanel, wxPanel)
	EVT_UPDATE_UI(ID_Phase_Panel, NFGPhasePanel::OnUpdateUI)
	EVT_CHECKBOX(ID_PhaseCorr0SameValuesForAll, NFGPhasePanel::OnPhaseCommand)
	EVT_BUTTON(ID_PhaseCorr0SetAllAuto, NFGPhasePanel::OnPhaseCommand)
	EVT_BUTTON(ID_PhaseCorr0SetAllManual, NFGPhasePanel::OnPhaseCommand)
	EVT_BUTTON(ID_PhaseLoad, NFGPhasePanel::OnPhaseCommand)
	EVT_BUTTON(ID_PhaseStore, NFGPhasePanel::OnPhaseCommand)
	EVT_BUTTON(ID_PhaseApply, NFGPhasePanel::OnPhaseCommand)
	EVT_BUTTON(ID_PhaseRevert, NFGPhasePanel::OnPhaseCommand)
	EVT_TOGGLEBUTTON(ID_PhaseAutoApply, NFGPhasePanel::OnPhaseCommand)
	
	EVT_TEXT(ID_PhaseCorr0, NFGPhasePanel::OnPhaseChangeCommand)
	EVT_TEXT(ID_PhaseCorr1, NFGPhasePanel::OnPhaseChangeCommand)
	EVT_TEXT(ID_PhaseCorr0PilotStep, NFGPhasePanel::OnPhaseChangeCommand)
	
	EVT_TOGGLEBUTTON(ID_PhaseCorr0Auto, NFGPhasePanel::OnPhaseChangeCommand)
	EVT_TOGGLEBUTTON(ID_PhaseCorr0AutoAllTogether, NFGPhasePanel::OnPhaseChangeCommand)
	EVT_TOGGLEBUTTON(ID_PhaseCorr0FollowAuto, NFGPhasePanel::OnPhaseChangeCommand)
	
	EVT_CHECKBOX(ID_PhaseCorr1SameValuesForAll, NFGPhasePanel::OnPhaseChangeCommand)
	EVT_CHECKBOX(ID_PhaseCorr1Apply, NFGPhasePanel::OnPhaseChangeCommand)
	EVT_CHECKBOX(ID_RemoveOffset, NFGPhasePanel::OnPhaseChangeCommand)

	EVT_SPIN_UP(ID_PhaseCorr0Spin, NFGPhasePanel::OnSpin)
	EVT_SPIN_DOWN(ID_PhaseCorr0Spin, NFGPhasePanel::OnSpin)
	EVT_SPIN_UP(ID_PhaseCorr1Spin, NFGPhasePanel::OnSpin)
	EVT_SPIN_DOWN(ID_PhaseCorr1Spin, NFGPhasePanel::OnSpin)
	EVT_MOUSEWHEEL(NFGPhasePanel::OnMouseWheel)
	EVT_CHAR_HOOK(NFGPhasePanel::OnChar)
END_EVENT_TABLE()

NFGPhasePanel::NFGPhasePanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	CurrentSerDocument = NULL;
	
	pparams.PhaseCorr0 = 0.0;
	pparams.PhaseCorr1 = 0.0;
	pparams.ZeroOrderSameValuesForAll = true;
	pparams.FirstOrderSameValuesForAll = true;
	pparams.ZeroOrderAutoAllTogether = false;
	pparams.ZeroOrderAuto = false;
	pparams.ZeroOrderFollowAuto = false;
	pparams.PilotStep = 0;
	pparams.ZeroOrderSetAllAuto = false;
	pparams.ZeroOrderSetAllManual = false;
	pparams.ApplyFirstOrderCorr = false;
	pparams.RemoveOffset = false;
	pparams.AutoApply = false;
	
	PostponeChanges = false;
	
	
	wxFlexGridSizer* PhaseFGSizer;
	PhaseFGSizer = new wxFlexGridSizer(2, 1, 0, 0);
	PhaseFGSizer->SetFlexibleDirection(wxBOTH);
	PhaseFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PhaseFGSizer->AddGrowableCol(0);
	
	wxFlexGridSizer* PhaseTopFGSizer;
	PhaseTopFGSizer = new wxFlexGridSizer(3, 1, FromDIP(5), 0);
	PhaseTopFGSizer->SetFlexibleDirection(wxBOTH);
	PhaseTopFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PhaseTopFGSizer->AddGrowableCol(0);


	wxStaticBoxSizer* ZeroOrderPhaseCorrSBSizer;
	ZeroOrderPhaseCorrSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Zero order phase correction"), wxVERTICAL);
	
	wxFlexGridSizer* ZeroOrderAuxFGSizer;
	ZeroOrderAuxFGSizer = new wxFlexGridSizer(3, 1, 0, 0);
	ZeroOrderAuxFGSizer->SetFlexibleDirection(wxBOTH);
	ZeroOrderAuxFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ZeroOrderAuxFGSizer->AddGrowableCol(0);

	ZeroOrderSameValuesForAllCheckBox = new wxCheckBox(this, ID_PhaseCorr0SameValuesForAll, "Same values for all steps", wxDefaultPosition,wxDefaultSize, 0, wxGenericValidator(&(pparams.ZeroOrderSameValuesForAll)));
	ZeroOrderAuxFGSizer->Add(ZeroOrderSameValuesForAllCheckBox, 0, wxLEFT|wxRIGHT|wxBOTTOM | wxEXPAND, FromDIP(5));


	wxFlexGridSizer* ZeroOrderValueFGSizer;
	ZeroOrderValueFGSizer = new wxFlexGridSizer(1, 3, 0, 0);
	ZeroOrderValueFGSizer->SetFlexibleDirection(wxBOTH);
	ZeroOrderValueFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ZeroOrderValueFGSizer->AddGrowableCol(2);
	ZeroOrderValueFGSizer->AddGrowableRow(0);

	ZeroOrderPhaseCorrLabelST = new wxStaticText(this, wxID_ANY, "Phase shift");
	ZeroOrderValueFGSizer->Add(ZeroOrderPhaseCorrLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(3));


	wxFlexGridSizer* PhaseShiftSizer;
	PhaseShiftSizer = new wxFlexGridSizer(1, 2, 0, 0);
	PhaseShiftSizer->SetFlexibleDirection(wxBOTH);
	PhaseShiftSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PhaseShiftSizer->AddGrowableCol(0);

	ZeroOrderPhaseCorrTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_PhaseCorr0, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(60, -1)), 0,
							 NFGDoubleValidator(&(pparams.PhaseCorr0)), wxTextCtrlNameStr, false, false, false, &PostponeChanges);
	
	ZeroOrderPhaseCorrSpinButton = new wxSpinButton(this, ID_PhaseCorr0Spin, wxDefaultPosition, wxSize(-1, ZeroOrderPhaseCorrTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP/*|wxSP_ARROW_KEYS*/);
#ifdef __WXGTK__
	ZeroOrderPhaseCorrSpinButton->SetCanFocus(false);
#endif
	ZeroOrderPhaseCorrSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	ZeroOrderPhaseCorrSpinButton->SetValue(0);
	SpinVal0 = 0;
	
	PhaseShiftSizer->Add(ZeroOrderPhaseCorrTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	PhaseShiftSizer->Add(ZeroOrderPhaseCorrSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	ZeroOrderValueFGSizer->Add(PhaseShiftSizer, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));


	ZeroOrderPhaseCorrLabelUnitsST = new wxStaticText(this, wxID_ANY, "deg");
	ZeroOrderValueFGSizer->Add(ZeroOrderPhaseCorrLabelUnitsST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(3));
	
	ZeroOrderAuxFGSizer->Add(ZeroOrderValueFGSizer, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));


	ZeroOrderFGSizer = new wxFlexGridSizer(1, 2, 0, 0);
	ZeroOrderFGSizer->SetFlexibleDirection(wxBOTH);
	ZeroOrderFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ZeroOrderFGSizer->AddGrowableCol(0);
	ZeroOrderFGSizer->AddGrowableCol(1);
	
	
	wxFlexGridSizer* ZeroOrderFGSizer0;
	ZeroOrderFGSizer0 = new wxFlexGridSizer(2, 1, 0, 0);
	ZeroOrderFGSizer0->SetFlexibleDirection(wxBOTH);
	ZeroOrderFGSizer0->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ZeroOrderFGSizer0->AddGrowableCol(0);

	ZeroOrderAutoAllTogetherToogleButton = new wxToggleButton(this, ID_PhaseCorr0AutoAllTogether, "Auto all together", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(pparams.ZeroOrderAutoAllTogether)));

	ZeroOrderFollowAutoFGSizer = new wxFlexGridSizer(1, 2, 0, FromDIP(4));
	ZeroOrderFollowAutoFGSizer->SetFlexibleDirection(wxBOTH);
	ZeroOrderFollowAutoFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ZeroOrderFollowAutoFGSizer->AddGrowableCol(0);

	ZeroOrderFollowAutoToogleButton = new wxToggleButton(this, ID_PhaseCorr0Auto, "Auto by step #", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(pparams.ZeroOrderFollowAuto)));
	PilotStepTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_PhaseCorr0PilotStep, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(60, -1)), 0, NFGLongValidator(&(pparams.PilotStep)), wxTextCtrlNameStr, false, false, false, &PostponeChanges);

	ZeroOrderFollowAutoFGSizer->Add(ZeroOrderFollowAutoToogleButton, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	ZeroOrderFollowAutoFGSizer->Add(PilotStepTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);

	ZeroOrderFGSizer0->Add(ZeroOrderAutoAllTogetherToogleButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));
	ZeroOrderFGSizer0->Add(ZeroOrderFollowAutoFGSizer, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));
	
	
	wxFlexGridSizer* ZeroOrderFGSizer1;
	ZeroOrderFGSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	ZeroOrderFGSizer1->SetFlexibleDirection(wxBOTH);
	ZeroOrderFGSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ZeroOrderFGSizer1->AddGrowableCol(0);

	ZeroOrderAutoToogleButton = new wxToggleButton(this, ID_PhaseCorr0Auto, "Auto", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(pparams.ZeroOrderAuto)));
	
	wxGridSizer* ZeroOrderSetAllGSizer = new wxGridSizer(1, 2, 0, FromDIP(4));

	ZeroOrderSetAllAutoButton = new wxButton(this, ID_PhaseCorr0SetAllAuto, "Set all to auto");
	ZeroOrderSetAllManualButton = new wxButton(this, ID_PhaseCorr0SetAllManual, "Set all to manual");

	ZeroOrderFGSizer1->Add(ZeroOrderAutoToogleButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));
	ZeroOrderSetAllGSizer->Add(ZeroOrderSetAllAutoButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	ZeroOrderSetAllGSizer->Add(ZeroOrderSetAllManualButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);

	ZeroOrderFGSizer1->Add(ZeroOrderSetAllGSizer, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));
	
	
	ZeroOrderFGSizer->Add(ZeroOrderFGSizer0, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	ZeroOrderFGSizer->Add(ZeroOrderFGSizer1, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	
	ZeroOrderAuxFGSizer->Add(ZeroOrderFGSizer, 0, wxEXPAND, 0);	
	
	ZeroOrderPhaseCorrSBSizer->Add(ZeroOrderAuxFGSizer, 0, wxEXPAND, 0);
	PhaseTopFGSizer->Add(ZeroOrderPhaseCorrSBSizer, 0, wxEXPAND, 0);
	
	
	wxStaticBoxSizer* FirstOrderPhaseCorrSBSizer;
	FirstOrderPhaseCorrSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "First order phase correction"), wxVERTICAL);
	
	FirstOrderFGSizer = new wxFlexGridSizer(2, 1, 0, 0);
	FirstOrderFGSizer->SetFlexibleDirection(wxBOTH);
	FirstOrderFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	FirstOrderFGSizer->AddGrowableCol(0);
	
	FirstOrderSameValuesForAllCheckBox = new wxCheckBox(this, ID_PhaseCorr1SameValuesForAll, "Same values for all steps", wxDefaultPosition,wxDefaultSize, 0, wxGenericValidator(&(pparams.FirstOrderSameValuesForAll)));
	FirstOrderFGSizer->Add(FirstOrderSameValuesForAllCheckBox, 0, wxLEFT|wxRIGHT|wxBOTTOM | wxEXPAND, FromDIP(5));
	

	wxFlexGridSizer* FirstOrderValueFGSizer;
	FirstOrderValueFGSizer = new wxFlexGridSizer(1, 3, 0, 0);
	FirstOrderValueFGSizer->SetFlexibleDirection(wxBOTH);
	FirstOrderValueFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	FirstOrderValueFGSizer->AddGrowableCol(2);
	FirstOrderValueFGSizer->AddGrowableRow(0);

	FirstOrderPhaseCorrCheckBox = new wxCheckBox(this, ID_PhaseCorr1Apply, "Assume FID start at", wxDefaultPosition, wxDefaultSize, 0 , wxGenericValidator(&(pparams.ApplyFirstOrderCorr)));
	FirstOrderValueFGSizer->Add(FirstOrderPhaseCorrCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(3));
	

	wxFlexGridSizer* FIDStartSizer;
	FIDStartSizer = new wxFlexGridSizer(1, 2, 0, 0);
	FIDStartSizer->SetFlexibleDirection(wxBOTH);
	FIDStartSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	FIDStartSizer->AddGrowableCol(0);

	FirstOrderPhaseCorrTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_PhaseCorr1, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(60, -1)), 0,
							 NFGDoubleValidator(&(pparams.PhaseCorr1)), wxTextCtrlNameStr, false, false, false, &PostponeChanges);
	
	FirstOrderPhaseCorrSpinButton = new wxSpinButton(this, ID_PhaseCorr1Spin, wxDefaultPosition, wxSize(-1, FirstOrderPhaseCorrTextCtrl->GetSize().GetHeight()), wxSP_VERTICAL|wxSP_WRAP/*|wxSP_ARROW_KEYS*/);
#ifdef __WXGTK__
	FirstOrderPhaseCorrSpinButton->SetCanFocus(false);
#endif
	FirstOrderPhaseCorrSpinButton->SetRange(-SpinHalfRange, SpinHalfRange);
	FirstOrderPhaseCorrSpinButton->SetValue(0);
	SpinVal1 = 0;
	
	FIDStartSizer->Add(FirstOrderPhaseCorrTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	FIDStartSizer->Add(FirstOrderPhaseCorrSpinButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	FirstOrderValueFGSizer->Add(FIDStartSizer, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));


	FirstOrderPhaseCorrLabelUnitsST = new wxStaticText(this, wxID_ANY, "us");
	FirstOrderValueFGSizer->Add(FirstOrderPhaseCorrLabelUnitsST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(3));

	FirstOrderFGSizer->Add(FirstOrderValueFGSizer, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(2));


	FirstOrderPhaseCorrSBSizer->Add(FirstOrderFGSizer, 0, wxEXPAND, 0);
	PhaseTopFGSizer->Add(FirstOrderPhaseCorrSBSizer, 0, wxEXPAND|wxBOTTOM, FromDIP(3));
	
	/// reserve the space
	ZeroOrderFGSizer->Show((size_t) 0, !pparams.ZeroOrderSameValuesForAll);
	ZeroOrderFGSizer->Show((size_t) 1, pparams.ZeroOrderSameValuesForAll);


	RemoveOffsetCheckBox = new wxCheckBox(this, ID_RemoveOffset, "Remove offset", wxDefaultPosition,wxDefaultSize, 0, wxGenericValidator(&(pparams.RemoveOffset)));
	PhaseTopFGSizer->Add(RemoveOffsetCheckBox, 0, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(10));


	PhaseFGSizer->Add(PhaseTopFGSizer, 1, wxALL|wxEXPAND, FromDIP(6));
	

	wxGridSizer* PhaseBottomGSizer;
	PhaseBottomGSizer = new wxGridSizer(2, 1, FromDIP(6), 0);


	wxGridSizer* PhaseBottomFGSizer1;
	PhaseBottomFGSizer1 = new wxGridSizer(1, 2, 0, FromDIP(6));
	
	PhaseLoadButton = new wxButton(this, ID_PhaseLoad, "Load <");
	PhaseBottomFGSizer1->Add(PhaseLoadButton, 0, wxALL|wxEXPAND, 0);
	
	PhaseStoreButton = new wxButton(this, ID_PhaseStore, "> Store");
	PhaseBottomFGSizer1->Add(PhaseStoreButton, 0, wxALL|wxEXPAND, 0);
	
	PhaseBottomGSizer->Add(PhaseBottomFGSizer1, 1, wxEXPAND, 0);
	
	wxGridSizer* PhaseBottomFGSizer2;
	PhaseBottomFGSizer2 = new wxGridSizer(1, 3, 0, FromDIP(7));
	
	PhaseAutoApplyTgButton = new wxToggleButton(this, ID_PhaseAutoApply, "Auto-Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxGenericValidator(&(pparams.AutoApply)));
	
	PhaseBottomFGSizer2->Add(PhaseAutoApplyTgButton, 0, wxALL|wxEXPAND, 0);
	
	PhaseApplyButton = new wxButton(this, ID_PhaseApply, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	PhaseBottomFGSizer2->Add(PhaseApplyButton, 0, wxALL|wxEXPAND, 0);
	
	PhaseRevertButton = new wxButton(this, ID_PhaseRevert, "Revert", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	PhaseBottomFGSizer2->Add(PhaseRevertButton, 0, wxALL|wxEXPAND, 0);
	
	PhaseBottomGSizer->Add(PhaseBottomFGSizer2, 1, wxEXPAND, 0);


	PhaseFGSizer->Add(PhaseBottomGSizer, 1, wxALL|wxEXPAND|wxALIGN_TOP, FromDIP(5));
	
	this->SetSizer(PhaseFGSizer);
	this->Layout();
	PhaseFGSizer->Fit(this);

	/// will be enabled by OnUpdateUI() handler if needed
	Disable();
}

NFGPhasePanel::~NFGPhasePanel()
{
}

void NFGPhasePanel::OnSpin(wxSpinEvent& event) 
{
	int SpinValue = event.GetPosition();
	int increment = 0;
	
	if (event.GetId() == ID_PhaseCorr0Spin) {
		increment = SpinValue - SpinVal0;
		if (increment > SpinHalfRange)	/// wrapping occured
			increment -= 2*SpinHalfRange;
		if (increment < -SpinHalfRange)	/// wrapping occured
			increment += 2*SpinHalfRange;
		
		SpinVal0 = SpinValue;
		
		SpinZeroOrder(1.0*increment);
	}
	
	if (event.GetId() == ID_PhaseCorr1Spin) {
		increment = SpinValue - SpinVal1;
		if (increment > SpinHalfRange)	/// wrapping occured
			increment -= 2*SpinHalfRange;
		if (increment < -SpinHalfRange)	/// wrapping occured
			increment += 2*SpinHalfRange;
		
		SpinVal1 = SpinValue;
		
		SpinFirstOrder(1.0*increment);
	}
	
	event.Skip();
}

void NFGPhasePanel::OnChar(wxKeyEvent& event) 
{
	double increment = 0.0;
	wxWindow *FocusedWin = wxWindow::FindFocus();
	
	switch (event.GetKeyCode()) {
		case WXK_UP:
		case WXK_NUMPAD_UP:
			increment = 1.0;
			break;

		case WXK_DOWN:
		case WXK_NUMPAD_DOWN:
			increment = -1.0;
			break;

		case WXK_PAGEUP:
		case WXK_NUMPAD_PAGEUP:
			increment = 10.0;
			break;

		case WXK_PAGEDOWN:
		case WXK_NUMPAD_PAGEDOWN:
			increment = -10.0;
			break;

		default:
			event.Skip();
			return;
	}
	
	if ((FocusedWin != NULL) && (FocusedWin == ZeroOrderPhaseCorrTextCtrl)) 
		SpinZeroOrder(increment);
	else 
	if ((FocusedWin != NULL) && (FocusedWin == FirstOrderPhaseCorrTextCtrl)) 
		SpinFirstOrder(increment);
	else 
		event.Skip();
}

void NFGPhasePanel::OnMouseWheel(wxMouseEvent& event) 
{
	wxWindow *FocusedWin = wxWindow::FindFocus();
	
	if ((FocusedWin != NULL) && (FocusedWin == ZeroOrderPhaseCorrTextCtrl)) {
		SpinZeroOrder((1.0*event.GetWheelRotation())/event.GetWheelDelta());
		return;
	}
	
	if ((FocusedWin != NULL) && (FocusedWin == FirstOrderPhaseCorrTextCtrl)) {
		SpinFirstOrder((1.0*event.GetWheelRotation())/event.GetWheelDelta());
		return;
	}
	
	event.Skip();	
}

void NFGPhasePanel::SpinZeroOrder(double coefficient) 
{
	double dvalue = 0.0;
	double IntPart = 0.0;
	wxString val(ZeroOrderPhaseCorrTextCtrl->GetValue());
	val.Replace(",", ".");	/// accept both '.' and ',' decimal point
	if (!val.ToCDouble(&dvalue) || !wxFinite(dvalue)) {
		wxBell();
		dvalue = pparams.PhaseCorr0;
		coefficient = 0.0;
	}
	
	dvalue = std::modf(dvalue/360.0, &IntPart)*360.0;
	dvalue += 0.5*coefficient;
	dvalue = std::modf(dvalue/360.0, &IntPart)*360.0;
	dvalue = 1.0e-3 * NFGMSTD round(dvalue*1.0e3);
	val.Printf("%.15g", dvalue);
	ZeroOrderPhaseCorrTextCtrl->SetValue(val);
}

void NFGPhasePanel::SpinFirstOrder(double coefficient) 
{
	double dvalue = 0.0;
	wxString val(FirstOrderPhaseCorrTextCtrl->GetValue());
	val.Replace(",", ".");	/// accept both '.' and ',' decimal point
	if (!val.ToCDouble(&dvalue) || !wxFinite(dvalue)) {
		wxBell();
		dvalue = pparams.PhaseCorr1;
		coefficient = 0.0;
	}
	
	dvalue += 0.2*coefficient;
	dvalue = 1.0e-3 * NFGMSTD round(dvalue*1.0e3);

	if (dvalue > 2.0e6)
		dvalue = 2.0e6;
	if (dvalue < -2.0e6)
		dvalue = -2.0e6;

	val.Printf("%.15g", dvalue);
	FirstOrderPhaseCorrTextCtrl->SetValue(val);
}


void NFGPhasePanel::OnPhaseCommand(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	
	switch(event.GetId()) {
		case ID_PhaseCorr0SameValuesForAll:
			ZeroOrderFGSizer->Show((size_t) 0, event.IsChecked());
			ZeroOrderFGSizer->Show((size_t) 1, !event.IsChecked());
			ZeroOrderFGSizer->Layout();
		
			OnPhaseChangeCommand(event);
			break;
		
		case ID_PhaseCorr0SetAllAuto:
		//	if (Validate()) 	/// do at least a partial transfer of data - the crucial parameter is pparams.ZeroOrderSameValuesForAll
				TransferDataFromWindow();
			pparams.ZeroOrderSetAllAuto = true;
			pparams = CurrentSerDocument->SetPhaseCorrParams(pparams);
			TransferDataToWindow();
			break;
		
		case ID_PhaseCorr0SetAllManual:
		//	if (Validate()) 	/// do at least a partial transfer of data - the crucial parameter is pparams.ZeroOrderSameValuesForAll
				TransferDataFromWindow();
			pparams.ZeroOrderSetAllManual = true;
			pparams = CurrentSerDocument->SetPhaseCorrParams(pparams);
			TransferDataToWindow();
			break;
		
		case ID_PhaseLoad:
			pparams = DocMan->LoadSerPhaseCorrParams();
			/// apply loaded params immediately
			pparams = CurrentSerDocument->SetPhaseCorrParams(pparams);
			TransferDataToWindow();
			ZeroOrderFGSizer->Show((size_t) 0, pparams.ZeroOrderSameValuesForAll);
			ZeroOrderFGSizer->Show((size_t) 1, !pparams.ZeroOrderSameValuesForAll);
			ZeroOrderFGSizer->Layout();
			break;
			
		case ID_PhaseStore:
			if (Validate()) {
				TransferDataFromWindow();
				DocMan-> StoreSerPhaseCorrParams(pparams);
			} else
				wxBell();
			break;
		
		case ID_PhaseApply:
			if (Validate()) {
				TransferDataFromWindow();
				pparams = CurrentSerDocument->SetPhaseCorrParams(pparams);
				TransferDataToWindow();
			} else
				wxBell();
			break;
			
		case ID_PhaseRevert:
			pparams = CurrentSerDocument->GetPhaseCorrParams();
			TransferDataToWindow();
			ZeroOrderFGSizer->Show((size_t) 0, pparams.ZeroOrderSameValuesForAll);
			ZeroOrderFGSizer->Show((size_t) 1, !pparams.ZeroOrderSameValuesForAll);
			ZeroOrderFGSizer->Layout();
			break;
			
		case ID_PhaseAutoApply:
			pparams.AutoApply = event.IsChecked();
			PhaseApplyButton->Enable(!pparams.AutoApply);
			PhaseRevertButton->Enable(!pparams.AutoApply);
		
			PostponeChanges = pparams.AutoApply;
		
			if (Validate()) {
				TransferDataFromWindow();
				pparams = CurrentSerDocument->SetPhaseCorrParams(pparams);
				TransferDataToWindow();
			} else
				wxBell();
			break;
			
		default:
			;
	}

	ZeroOrderPhaseCorrTextCtrl->Enable(!((ZeroOrderSameValuesForAllCheckBox->IsChecked())?(ZeroOrderAutoAllTogetherToogleButton->GetValue() || ZeroOrderFollowAutoToogleButton->GetValue()):(ZeroOrderAutoToogleButton->GetValue())));
	ZeroOrderPhaseCorrSpinButton->Enable(ZeroOrderPhaseCorrTextCtrl->IsEnabled());
	FirstOrderPhaseCorrTextCtrl->Enable(FirstOrderPhaseCorrCheckBox->IsChecked());
	FirstOrderPhaseCorrSpinButton->Enable(FirstOrderPhaseCorrTextCtrl->IsEnabled());
	
}

void NFGPhasePanel::OnPhaseChangeCommand(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	if ((event.GetEventObject() == ZeroOrderAutoAllTogetherToogleButton) && event.IsChecked()) 
		ZeroOrderFollowAutoToogleButton->SetValue(false);
	
	if ((event.GetEventObject() == ZeroOrderFollowAutoToogleButton) && event.IsChecked()) 
		ZeroOrderAutoAllTogetherToogleButton->SetValue(false);
	
	ZeroOrderPhaseCorrTextCtrl->Enable(!((ZeroOrderSameValuesForAllCheckBox->IsChecked())?(ZeroOrderAutoAllTogetherToogleButton->GetValue() || ZeroOrderFollowAutoToogleButton->GetValue()):(ZeroOrderAutoToogleButton->GetValue())));
	ZeroOrderPhaseCorrSpinButton->Enable(ZeroOrderPhaseCorrTextCtrl->IsEnabled());
	FirstOrderPhaseCorrTextCtrl->Enable(FirstOrderPhaseCorrCheckBox->IsChecked());
	FirstOrderPhaseCorrSpinButton->Enable(FirstOrderPhaseCorrTextCtrl->IsEnabled());

	if (!pparams.AutoApply)
		return;
	
	if (Validate()) {
		TransferDataFromWindow();
		pparams = CurrentSerDocument->SetPhaseCorrParams(pparams);
		TransferDataToWindow();
	} else {
		wxBell();
	}
	
}

void NFGPhasePanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) {
		CurrentSerDocument = NULL;
		Disable();
		return;
	}

	Enable();
	
	if (SerDoc == CurrentSerDocument)
		if (!(SerDoc->PhaseCorrParamsChangedQuery()))
			return;
	
	CurrentSerDocument = SerDoc;
	
	pparams = CurrentSerDocument->GetPhaseCorrParams();

	PostponeChanges = false;
	
	TransferDataToWindow();
	
	PostponeChanges = pparams.AutoApply;
	
	PhaseApplyButton->Enable(!pparams.AutoApply);
	PhaseRevertButton->Enable(!pparams.AutoApply);
	
	ZeroOrderFGSizer->Show((size_t) 0, pparams.ZeroOrderSameValuesForAll);
	ZeroOrderFGSizer->Show((size_t) 1, !pparams.ZeroOrderSameValuesForAll);
	ZeroOrderFGSizer->Layout();
	
	ZeroOrderPhaseCorrTextCtrl->Enable(!((ZeroOrderSameValuesForAllCheckBox->IsChecked())?(ZeroOrderAutoAllTogetherToogleButton->GetValue() || ZeroOrderFollowAutoToogleButton->GetValue()):(ZeroOrderAutoToogleButton->GetValue())));
	ZeroOrderPhaseCorrSpinButton->Enable(ZeroOrderPhaseCorrTextCtrl->IsEnabled());
	FirstOrderPhaseCorrTextCtrl->Enable(FirstOrderPhaseCorrCheckBox->IsChecked());
	FirstOrderPhaseCorrSpinButton->Enable(FirstOrderPhaseCorrTextCtrl->IsEnabled());
}


BEGIN_EVENT_TABLE(NFGPlotPanel, wxPanel)
	EVT_BUTTON(ID_MinimumHLoad, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_MinimumHStore, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_MaximumHLoad, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_MaximumHStore, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_MinimumVLoad, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_MinimumVStore, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_MaximumVLoad, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_MaximumVStore, NFGPlotPanel::OnMinMaxLoadStore)
	EVT_BUTTON(ID_PlotLoad, NFGPlotPanel::OnPlotCommand)
	EVT_BUTTON(ID_PlotStore, NFGPlotPanel::OnPlotCommand)
	EVT_BUTTON(ID_PlotApply, NFGPlotPanel::OnPlotCommand)
	EVT_BUTTON(ID_PlotRevert, NFGPlotPanel::OnPlotCommand)
	EVT_TEXT(ID_MinimumH, NFGPlotPanel::OnMinMaxInput)
	EVT_TEXT(ID_MaximumH, NFGPlotPanel::OnMinMaxInput)
	EVT_TEXT(ID_MinimumV, NFGPlotPanel::OnMinMaxInput)
	EVT_TEXT(ID_MaximumV, NFGPlotPanel::OnMinMaxInput)
	EVT_UPDATE_UI(ID_Plot_Panel, NFGPlotPanel::OnUpdateUI)
END_EVENT_TABLE()

NFGPlotPanel::NFGPlotPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	CurrentSerDocument = NULL;
	
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
	
	ManualH = false;
	ManualV = false;
	
	wxFlexGridSizer* PlotFGSizer;
	PlotFGSizer = new wxFlexGridSizer(3, 1, FromDIP(5), FromDIP(5));
	PlotFGSizer->SetFlexibleDirection(wxBOTH);
	PlotFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PlotFGSizer->AddGrowableCol(0);
	
	wxStaticBoxSizer* PlotTopSBSizer;
	PlotTopSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Horizontal axis range"), wxVERTICAL);
	
	wxFlexGridSizer* PlotHorizontalFGSizer;
	PlotHorizontalFGSizer = new wxFlexGridSizer(2, 1, 0, 0);
	PlotHorizontalFGSizer->SetFlexibleDirection(wxBOTH);
	PlotHorizontalFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PlotHorizontalFGSizer->AddGrowableCol(0);
	
	wxFlexGridSizer* PlotHRadioFGSizer;
	PlotHRadioFGSizer = new wxFlexGridSizer(1, 3, 0, 0);
	PlotHRadioFGSizer->SetFlexibleDirection(wxBOTH);
	PlotHRadioFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PlotHRadioFGSizer->AddGrowableCol(0);
	PlotHRadioFGSizer->AddGrowableCol(1);
	PlotHRadioFGSizer->AddGrowableCol(2);
	
	PlotAllAutoHRadioButton = new NFGAutoRadioButton(this, ID_PlotAllAutoH, "All auto", wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxGenericValidator(&params.AutoZoomHAll), false);
	PlotAllAutoHRadioButton->SetValue(true); 
	PlotHRadioFGSizer->Add(PlotAllAutoHRadioButton, 0, wxALL|wxEXPAND, FromDIP(3));
	
	PlotSelectedAutoHRadioButton = new NFGAutoRadioButton(this, ID_PlotSelectedAutoH, "Selected auto", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&params.AutoZoomHSelected), false);
	PlotHRadioFGSizer->Add(PlotSelectedAutoHRadioButton, 0, wxALL|wxEXPAND, FromDIP(3));
	
	PlotManualHRadioButton = new NFGAutoRadioButton(this, ID_PlotManualH, "Manual", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&ManualH), false);
	PlotHRadioFGSizer->Add(PlotManualHRadioButton, 0, wxALL|wxEXPAND, FromDIP(3));
	
	PlotHorizontalFGSizer->Add(PlotHRadioFGSizer, 1, wxEXPAND, 0);
	
	wxFlexGridSizer* PlotHRangeFGSizer;
	PlotHRangeFGSizer = new wxFlexGridSizer(2, 4, 0, 0);
	PlotHRangeFGSizer->SetFlexibleDirection(wxBOTH);
	PlotHRangeFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PlotHRangeFGSizer->AddGrowableCol(1);

	MinimumHLabelST = new wxStaticText(this, wxID_ANY, "Minimum");
	PlotHRangeFGSizer->Add(MinimumHLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(6));
	
	MinimumHTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_MinimumH, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(85,-1)), 0,
							 NFGDoubleValidator(&(params.minx)), wxTextCtrlNameStr, false);
	PlotHRangeFGSizer->Add(MinimumHTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	MinimumHLoadButton = new wxButton(this, ID_MinimumHLoad, "<", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MinimumHLoadButton->SetToolTip("Load");
	
	PlotHRangeFGSizer->Add(MinimumHLoadButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	MinimumHStoreButton = new wxButton(this, ID_MinimumHStore, ">", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MinimumHStoreButton->SetToolTip("Store");
	
	PlotHRangeFGSizer->Add(MinimumHStoreButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	MaximumHLabelST = new wxStaticText(this, wxID_ANY, "Maximum");
	PlotHRangeFGSizer->Add(MaximumHLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(6));
	
	MaximumHTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_MaximumH, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(85,-1)), 0,
							 NFGDoubleValidator(&(params.maxx)), wxTextCtrlNameStr, false);
	PlotHRangeFGSizer->Add(MaximumHTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	MaximumHLoadButton = new wxButton(this, ID_MaximumHLoad, "<", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MaximumHLoadButton->SetToolTip("Load");
	
	PlotHRangeFGSizer->Add(MaximumHLoadButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	MaximumHStoreButton = new wxButton(this, ID_MaximumHStore, ">", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MaximumHStoreButton->SetToolTip("Store");
	
	PlotHRangeFGSizer->Add(MaximumHStoreButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	PlotHorizontalFGSizer->Add(PlotHRangeFGSizer, 1, wxEXPAND, 0);
	
	PlotTopSBSizer->Add(PlotHorizontalFGSizer, 1, wxEXPAND, 0);
	
	PlotFGSizer->Add(PlotTopSBSizer, 1, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, FromDIP(6));
	
	wxStaticBoxSizer* PlotCenterSBSizer;
	PlotCenterSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Vertical axis range"), wxVERTICAL);
	
	wxFlexGridSizer* PlotVerticalFGSizer;
	PlotVerticalFGSizer = new wxFlexGridSizer(2, 1, 0, 0);
	PlotVerticalFGSizer->SetFlexibleDirection(wxBOTH);
	PlotVerticalFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PlotVerticalFGSizer->AddGrowableCol(0);
	
	wxFlexGridSizer* PlotVRadioFGSizer;
	PlotVRadioFGSizer = new wxFlexGridSizer(1, 3, 0, 0);
	PlotVRadioFGSizer->SetFlexibleDirection(wxBOTH);
	PlotVRadioFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PlotVRadioFGSizer->AddGrowableCol(0);
	PlotVRadioFGSizer->AddGrowableCol(1);
	PlotVRadioFGSizer->AddGrowableCol(2);

	PlotAllAutoVRadioButton = new NFGAutoRadioButton(this, ID_PlotAllAutoV, "All auto", wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxGenericValidator(&params.AutoZoomVAll), false);
	PlotAllAutoVRadioButton->SetValue(true); 
	PlotVRadioFGSizer->Add(PlotAllAutoVRadioButton, 0, wxALL, FromDIP(3));
	
	PlotSelectedAutoVRadioButton = new NFGAutoRadioButton(this, ID_PlotSelectedAutoV, "Selected auto", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&params.AutoZoomVSelected), false);
	PlotVRadioFGSizer->Add(PlotSelectedAutoVRadioButton, 0, wxALL, FromDIP(3));
	
	PlotManualVRadioButton = new NFGAutoRadioButton(this, ID_PlotManualV, "Manual", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&ManualV), false);
	PlotVRadioFGSizer->Add(PlotManualVRadioButton, 0, wxALL, FromDIP(3));
	
	PlotVerticalFGSizer->Add(PlotVRadioFGSizer, 1, wxEXPAND, 0);
	
	wxFlexGridSizer* PlotVRangeFGSizer;
	PlotVRangeFGSizer = new wxFlexGridSizer(2, 4, 0, 0);
	PlotVRangeFGSizer->SetFlexibleDirection(wxBOTH);
	PlotVRangeFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	PlotVRangeFGSizer->AddGrowableCol(1);

	MinimumVLabelST = new wxStaticText(this, wxID_ANY, "Minimum");
	PlotVRangeFGSizer->Add(MinimumVLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(6));
	
	MinimumVTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_MinimumV, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(85,-1)), 0,
							 NFGDoubleValidator(&(params.miny)), wxTextCtrlNameStr, false);
	PlotVRangeFGSizer->Add(MinimumVTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	MinimumVLoadButton = new wxButton(this, ID_MinimumVLoad, "<", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MinimumVLoadButton->SetToolTip("Load");
	
	PlotVRangeFGSizer->Add(MinimumVLoadButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	MinimumVStoreButton = new wxButton(this, ID_MinimumVStore, ">", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MinimumVStoreButton->SetToolTip("Store");
	
	PlotVRangeFGSizer->Add(MinimumVStoreButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	MaximumVLabelST = new wxStaticText(this, wxID_ANY, "Maximum");
	PlotVRangeFGSizer->Add(MaximumVLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(6));
	
	MaximumVTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_MaximumV, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(85,-1)), 0,
							 NFGDoubleValidator(&(params.maxy)), wxTextCtrlNameStr, false);
	PlotVRangeFGSizer->Add(MaximumVTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	MaximumVLoadButton = new wxButton(this, ID_MaximumVLoad, "<", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MaximumVLoadButton->SetToolTip("Load");
	
	PlotVRangeFGSizer->Add(MaximumVLoadButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	MaximumVStoreButton = new wxButton(this, ID_MaximumVStore, ">", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	MaximumVStoreButton->SetToolTip("Store");
	
	PlotVRangeFGSizer->Add(MaximumVStoreButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	PlotVerticalFGSizer->Add(PlotVRangeFGSizer, 1, wxEXPAND, 0);
	
	PlotCenterSBSizer->Add(PlotVerticalFGSizer, 1, wxEXPAND, 0);
	
	PlotFGSizer->Add(PlotCenterSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(6));
	
	wxFlexGridSizer* PlotBottomFGSizer;
	PlotBottomFGSizer = new wxFlexGridSizer(3, 2, FromDIP(6), FromDIP(6));
	PlotBottomFGSizer->AddGrowableCol(0);
	PlotBottomFGSizer->AddGrowableCol(1);
	PlotBottomFGSizer->SetFlexibleDirection(wxBOTH);
	PlotBottomFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	DrawPointsCheckBox = new wxCheckBox(this, ID_DrawPoints, "Draw points", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(params.DrawPoints)));
	PlotBottomFGSizer->Add(DrawPointsCheckBox, 0, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	ThickLinesCheckBox = new wxCheckBox(this, ID_ThickLines, "Thick lines", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(params.ThickLines)));
	PlotBottomFGSizer->Add(ThickLinesCheckBox, 0, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, FromDIP(5));

	PlotLoadButton = new wxButton(this, ID_PlotLoad, "Load <");
	PlotBottomFGSizer->Add(PlotLoadButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PlotStoreButton = new wxButton(this, ID_PlotStore, "> Store");
	PlotBottomFGSizer->Add(PlotStoreButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PlotApplyButton = new wxButton(this, ID_PlotApply, "Apply");
	PlotBottomFGSizer->Add(PlotApplyButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PlotRevertButton = new wxButton(this, ID_PlotRevert, "Revert");
	PlotBottomFGSizer->Add(PlotRevertButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PlotFGSizer->Add(PlotBottomFGSizer, 1, wxALL|wxEXPAND, FromDIP(5));
	
	this->SetSizer(PlotFGSizer);
	this->Layout();
	PlotFGSizer->Fit(this);
}

NFGPlotPanel::~NFGPlotPanel()
{
}

void NFGPlotPanel::OnMinMaxLoadStore(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	ZoomParams zparams = DocMan->LoadZoomParams();
	
	switch (event.GetId()) {
		case ID_MinimumHLoad:
			if (Validate()) 
				TransferDataFromWindow();
			params.minx = zparams.minx;
			TransferDataToWindow();
			break;
		
		case ID_MaximumHLoad:
			if (Validate()) 
				TransferDataFromWindow();
			params.maxx = zparams.maxx;
			TransferDataToWindow();
			break;
		
		case ID_MinimumVLoad:
			if (Validate()) 
				TransferDataFromWindow();
			params.miny = zparams.miny;
			TransferDataToWindow();
			break;
		
		case ID_MaximumVLoad:
			if (Validate()) 
				TransferDataFromWindow();
			params.maxy = zparams.maxy;
			TransferDataToWindow();
			break;
		
		
		case ID_MinimumHStore:
			if (Validate()) {
				TransferDataFromWindow();
				zparams.minx = params.minx;
				DocMan->StoreZoomParams(zparams);
			} else
				wxBell();
			break;
		
		case ID_MaximumHStore:
			if (Validate()) {
				TransferDataFromWindow();
				zparams.maxx = params.maxx;
				DocMan->StoreZoomParams(zparams);
			} else
				wxBell();
			break;
		
		case ID_MinimumVStore:
			if (Validate()) {
				TransferDataFromWindow();
				zparams.miny = params.miny;
				DocMan->StoreZoomParams(zparams);
			} else
				wxBell();
			break;
		
		case ID_MaximumVStore:
			if (Validate()) {
				TransferDataFromWindow();
				zparams.maxy = params.maxy;
				DocMan->StoreZoomParams(zparams);
			} else
				wxBell();
			break;
		
		default:
			;
	}
}

void NFGPlotPanel::OnMinMaxInput(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	switch (event.GetId()) {
		case ID_MinimumH:
		case ID_MaximumH:
			PlotManualHRadioButton->SetValue(true);
			break;
		
		case ID_MinimumV:
		case ID_MaximumV:
			PlotManualVRadioButton->SetValue(true);
			break;

		default:
			;
	}
}

void NFGPlotPanel::OnPlotCommand(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	switch(event.GetId()) {
		case ID_PlotLoad:
			params = DocMan->LoadZoomParams();
			/// apply loaded params immediately
			params = CurrentSerDocument->SetZoomParams(params);
			ManualH = !(params.AutoZoomHAll || params.AutoZoomHSelected);
			ManualV = !(params.AutoZoomVAll || params.AutoZoomVSelected);
			TransferDataToWindow();
			break;
			
		case ID_PlotStore:
			if (Validate()) {
				TransferDataFromWindow();
				DocMan->StoreZoomParams(params);
			} else
				wxBell();
			break;
		
		case ID_PlotApply:
			if (Validate()) {
				TransferDataFromWindow();
				params = CurrentSerDocument->SetZoomParams(params);
				ManualH = !(params.AutoZoomHAll || params.AutoZoomHSelected);
				ManualV = !(params.AutoZoomVAll || params.AutoZoomVSelected);
				TransferDataToWindow();
			} else
				wxBell();
			break;
			
		case ID_PlotRevert:
			params = CurrentSerDocument->GetZoomParams();
			ManualH = !(params.AutoZoomHAll || params.AutoZoomHSelected);
			ManualV = !(params.AutoZoomVAll || params.AutoZoomVSelected);
			TransferDataToWindow();
			break;
			
		default:
			;
	}
	
}


void NFGPlotPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);
	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) {
		CurrentSerDocument = NULL;
		Disable();
		return;
	}

	Enable();
	
	if (SerDoc == CurrentSerDocument)
		if (!(SerDoc->ZoomParamsChangedQuery()))
			return;
	
	CurrentSerDocument = SerDoc;
	
	params = CurrentSerDocument->GetZoomParams();

	ManualH = !(params.AutoZoomHAll || params.AutoZoomHSelected);
	ManualV = !(params.AutoZoomVAll || params.AutoZoomVSelected);

	TransferDataToWindow();
}


BEGIN_EVENT_TABLE(NFGViewsPanel, wxPanel)
	EVT_TEXT_ENTER(ID_ViewName, NFGViewsPanel::OnViewNameEnter)
	EVT_LISTBOX(ID_StoredViews, NFGViewsPanel::OnStoredViewsSelect)
	EVT_LISTBOX_DCLICK(ID_StoredViews, NFGViewsPanel::OnStoredViewsDblClick)
	EVT_BUTTON(ID_LoadView, NFGViewsPanel::OnViewsCommand)
	EVT_BUTTON(ID_StoreView, NFGViewsPanel::OnViewsCommand)
	EVT_BUTTON(ID_LoadOtherView, NFGViewsPanel::OnViewsCommand)
	EVT_BUTTON(ID_DeleteView, NFGViewsPanel::OnViewsCommand)
	EVT_UPDATE_UI(ID_Views_Panel, NFGViewsPanel::OnUpdateUI)
END_EVENT_TABLE()

NFGViewsPanel::NFGViewsPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	CurrentSerDocument = NULL;
	
	wxFlexGridSizer* ViewsFGSizer;
	ViewsFGSizer = new wxFlexGridSizer(3, 1, 0, 0);
	ViewsFGSizer->AddGrowableCol(0);
	ViewsFGSizer->AddGrowableRow(1);
	ViewsFGSizer->SetFlexibleDirection(wxBOTH);
	ViewsFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	ViewNameTextCtrl = new wxTextCtrl(this, ID_ViewName, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	ViewsFGSizer->Add(ViewNameTextCtrl, 0, wxALL|wxEXPAND, FromDIP(5));
	
	StoredViewsListBox = new wxListBox(this, ID_StoredViews, wxDefaultPosition, wxDefaultSize, 0, NULL, 0); 
	StoredViewsListBox->SetMinSize(wxSize(10, 10));
	ViewsFGSizer->Add(StoredViewsListBox, 0, wxALL|wxEXPAND, FromDIP(5));
	
	wxFlexGridSizer* BottomViewsFGSizer;
	BottomViewsFGSizer = new wxFlexGridSizer(2, 2, FromDIP(6), FromDIP(6));
	BottomViewsFGSizer->AddGrowableCol(0);
	BottomViewsFGSizer->AddGrowableCol(1);
	BottomViewsFGSizer->SetFlexibleDirection(wxBOTH);
	BottomViewsFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	LoadViewButton = new wxButton(this, ID_LoadView, "Load");
	BottomViewsFGSizer->Add(LoadViewButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	StoreViewButton = new wxButton(this, ID_StoreView, "Store");
	BottomViewsFGSizer->Add(StoreViewButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	LoadOtherViewButton = new wxButton(this, ID_LoadOtherView, "Load other...");
	BottomViewsFGSizer->Add(LoadOtherViewButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	DeleteViewButton = new wxButton(this, ID_DeleteView, "Delete");
	BottomViewsFGSizer->Add(DeleteViewButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	ViewsFGSizer->Add(BottomViewsFGSizer, 1, wxALL|wxEXPAND, FromDIP(5));
	
	this->SetSizer(ViewsFGSizer);
	this->Layout();
	ViewsFGSizer->Fit(this);
	
	/// will be enabled by OnUpdateUI() handler if needed
	Disable();
}

NFGViewsPanel::~NFGViewsPanel()
{
}

void NFGViewsPanel::OnViewNameEnter(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;

	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	wxString ViewName = ViewNameTextCtrl->GetValue();
	/// No point to accidentally try to overwrite exported files (would not happen anyway)
	if (ViewName.StartsWith(">")) {
		wxBell();
		return;
	}

	if (CurrentSerDocument->StoreView(ViewName) != DATA_OK)
		wxBell();
	
	CurrentSerDocument->LoadViewList();
	StoredViewsListBox->Clear();
	StoredViewsListBox->Set(CurrentSerDocument->ViewListChoicesQuery());
}

void NFGViewsPanel::OnStoredViewsSelect(wxCommandEvent& event)
{ 
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	if (event.IsSelection())
		ViewNameTextCtrl->ChangeValue(event.GetString());
}

void NFGViewsPanel::OnStoredViewsDblClick(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	if (event.IsSelection()) {
		ViewNameTextCtrl->ChangeValue(event.GetString());

		int n = event.GetSelection();
		if (n == wxNOT_FOUND)
			return;
		
		wxString ViewFullPath = CurrentSerDocument->ViewFullPathQuery(n);
		if (CurrentSerDocument->LoadViewCommand(ViewFullPath) == false) {
			wxBell();
			
			CurrentSerDocument->LoadViewList();
			StoredViewsListBox->Clear();
			StoredViewsListBox->Set(CurrentSerDocument->ViewListChoicesQuery());
		}
	}
}

void NFGViewsPanel::OnViewsCommand(wxCommandEvent& event)
{
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	wxString ViewFullPath = wxEmptyString;
	wxString ViewName = wxEmptyString;
	int n = wxNOT_FOUND;
	
	switch(event.GetId()) {
		case ID_LoadView:
			n = StoredViewsListBox->GetSelection();
			if (n == wxNOT_FOUND)
				return;
			
			ViewFullPath = CurrentSerDocument->ViewFullPathQuery(n);
			if (CurrentSerDocument->LoadViewCommand(ViewFullPath) == false) {
				wxBell();
				
				CurrentSerDocument->LoadViewList();
				StoredViewsListBox->Clear();
				StoredViewsListBox->Set(CurrentSerDocument->ViewListChoicesQuery());
			}
			break;
		
		case ID_StoreView:
			ViewName = ViewNameTextCtrl->GetValue();
			/// No point to accidentally try to overwrite exported files (would not happen anyway)
			if (ViewName.StartsWith(">")) {
				wxBell();
				return;
			}
			
			if (CurrentSerDocument->StoreView(ViewName) != DATA_OK)
				wxBell();
			
			CurrentSerDocument->LoadViewList();
			StoredViewsListBox->Clear();
			StoredViewsListBox->Set(CurrentSerDocument->ViewListChoicesQuery());
			break;
		
		case ID_LoadOtherView:
			ViewFullPath = ::wxFileSelector("Choose the view file", wxEmptyString, wxEmptyString, wxEmptyString, "*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (ViewFullPath.IsEmpty() == true)
				return;
			
			if (CurrentSerDocument->LoadViewCommand(ViewFullPath) == false)
				wxBell();
			else {
				wxFileName OtherViewFileName = wxFileName::FileName(ViewFullPath);
				ViewNameTextCtrl->ChangeValue(OtherViewFileName.GetFullName());
			}
			break;
			
		case ID_DeleteView:
			n = StoredViewsListBox->GetSelection();
			if (n == wxNOT_FOUND)
				return;
			
			/// Do not delete exported files
			if ((StoredViewsListBox->GetStringSelection()).StartsWith(">")) {
				wxBell();
				return;
			}
			
			ViewFullPath = CurrentSerDocument->ViewFullPathQuery(n);
			if (ViewFullPath.IsEmpty())
				return;
			
			if (::wxRemoveFile(ViewFullPath) == false)
				wxBell();
			
			CurrentSerDocument->LoadViewList();
			StoredViewsListBox->Clear();
			StoredViewsListBox->Set(CurrentSerDocument->ViewListChoicesQuery());
			break;
			
		default:
			;
	}

}


void NFGViewsPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) {
		CurrentSerDocument = NULL;
		Disable();
		return;
	}
	
	Enable();
	
	if (SerDoc == CurrentSerDocument)
		if (!CurrentSerDocument->ViewListChangedQuery())
			return;
	
	CurrentSerDocument = SerDoc;
	
	ViewNameTextCtrl->Clear();
	ViewNameTextCtrl->DiscardEdits();
	StoredViewsListBox->Clear();
	
	StoredViewsListBox->Set(CurrentSerDocument->ViewListChoicesQuery());
}



BEGIN_EVENT_TABLE(NFGExportPanel, wxPanel)
	EVT_COMMAND_RANGE(ID_ExportTDD, ID_ExportEvaluation, wxEVT_BUTTON, NFGExportPanel::OnExportCommand)
	EVT_COMMAND_RANGE(ID_ExportTDD2, ID_ExportEvaluation2, wxEVT_BUTTON, NFGExportPanel::OnExportCommand)
	EVT_UPDATE_UI(ID_Export_Panel, NFGExportPanel::OnUpdateUI)
END_EVENT_TABLE()

NFGExportPanel::NFGExportPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	CurrentSerDocument = NULL;
	
	wxFlexGridSizer* ExportFGSizer;
	ExportFGSizer = new wxFlexGridSizer(7, 1, 0, 0);
	ExportFGSizer->SetFlexibleDirection(wxBOTH);
	ExportFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportFGSizer->AddGrowableCol(0);
	
	wxBitmap GoBmp = wxArtProvider::GetBitmap("go", wxART_OTHER, FromDIP(wxSize(16, 16)));
	wxBitmap ClipboardBmp = wxArtProvider::GetBitmap("clipboard", wxART_OTHER, FromDIP(wxSize(16, 16)));
	
	wxStaticBoxSizer* ExportTDDSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Time domain data"), wxVERTICAL);
	wxFlexGridSizer* ExportTDDFGSizer = new wxFlexGridSizer(1, 4, 0, 0);
	ExportTDDFGSizer->SetFlexibleDirection(wxBOTH);
	ExportTDDFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportTDDFGSizer->AddGrowableCol(2);
	
	ExportTDDBmpButton2 = new wxBitmapButton(this, ID_ExportTDD2, ClipboardBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportTDDBmpButton2->SetToolTip("Simplified export to clipboard");
	ExportTDDFGSizer->Add(ExportTDDBmpButton2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportTDDFGSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(2, 16)), wxLI_VERTICAL), 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	ExportTDDFilePickerCtrl = new wxFilePickerCtrl(this, ID_ExportTDD_FilePicker, wxEmptyString, "Choose a filename", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
	ExportTDDFGSizer->Add(ExportTDDFilePickerCtrl, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(1));
	
	ExportTDDBmpButton = new wxBitmapButton(this, ID_ExportTDD, GoBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportTDDBmpButton->SetToolTip("Export to file");
	ExportTDDFGSizer->Add(ExportTDDBmpButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportTDDSBSizer->Add(ExportTDDFGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(2));
	ExportFGSizer->Add(ExportTDDSBSizer, 1, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, FromDIP(6));
	
	
	wxStaticBoxSizer* ExportEchoPeaksEnvelopeSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Echo peaks envelope"), wxVERTICAL);
	wxFlexGridSizer* ExportEchoPeaksEnvelopeFGSizer = new wxFlexGridSizer(1, 4, 0, 0);
	ExportEchoPeaksEnvelopeFGSizer->SetFlexibleDirection(wxBOTH);
	ExportEchoPeaksEnvelopeFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportEchoPeaksEnvelopeFGSizer->AddGrowableCol(2);
	
	ExportEchoPeaksEnvelopeBmpButton2 = new wxBitmapButton(this, ID_ExportEchoPeaksEnvelope2, ClipboardBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportEchoPeaksEnvelopeBmpButton2->SetToolTip("Simplified export to clipboard");
	ExportEchoPeaksEnvelopeFGSizer->Add(ExportEchoPeaksEnvelopeBmpButton2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportEchoPeaksEnvelopeFGSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(2, 16)), wxLI_VERTICAL), 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	ExportEchoPeaksEnvelopeFilePickerCtrl = new wxFilePickerCtrl(this, ID_ExportEchoPeaksEnvelope_FilePicker, wxEmptyString, "Choose a filename", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
	ExportEchoPeaksEnvelopeFGSizer->Add(ExportEchoPeaksEnvelopeFilePickerCtrl, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(1));
	
	ExportEchoPeaksEnvelopeBmpButton = new wxBitmapButton(this, ID_ExportEchoPeaksEnvelope, GoBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportEchoPeaksEnvelopeBmpButton->SetToolTip("Export to file");
	ExportEchoPeaksEnvelopeFGSizer->Add(ExportEchoPeaksEnvelopeBmpButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportEchoPeaksEnvelopeSBSizer->Add(ExportEchoPeaksEnvelopeFGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(2));
	ExportFGSizer->Add(ExportEchoPeaksEnvelopeSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(6));
	

	wxStaticBoxSizer* ExportChunkAvgSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Chunk average"), wxVERTICAL);
	wxFlexGridSizer* ExportChunkAvgFGSizer = new wxFlexGridSizer(1, 4, 0, 0);
	ExportChunkAvgFGSizer->SetFlexibleDirection(wxBOTH);
	ExportChunkAvgFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportChunkAvgFGSizer->AddGrowableCol(2);
	
	ExportChunkAvgBmpButton2 = new wxBitmapButton(this, ID_ExportChunkAvg2, ClipboardBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportChunkAvgBmpButton2->SetToolTip("Simplified export to clipboard");
	ExportChunkAvgFGSizer->Add(ExportChunkAvgBmpButton2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportChunkAvgFGSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(2, 16)), wxLI_VERTICAL), 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	ExportChunkAvgFilePickerCtrl = new wxFilePickerCtrl(this, ID_ExportChunkAvg_FilePicker, wxEmptyString, "Choose a filename", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
	ExportChunkAvgFGSizer->Add(ExportChunkAvgFilePickerCtrl, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(1));
	
	ExportChunkAvgBmpButton = new wxBitmapButton(this, ID_ExportChunkAvg, GoBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportChunkAvgBmpButton->SetToolTip("Export to file");
	ExportChunkAvgFGSizer->Add(ExportChunkAvgBmpButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	ExportChunkAvgSBSizer->Add(ExportChunkAvgFGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(2));
	ExportFGSizer->Add(ExportChunkAvgSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(6));


	wxStaticBoxSizer* ExportFFTSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "FFT"), wxVERTICAL);
	wxFlexGridSizer* ExportFFTFGSizer = new wxFlexGridSizer(1, 4, 0, 0);
	ExportFFTFGSizer->SetFlexibleDirection(wxBOTH);
	ExportFFTFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportFFTFGSizer->AddGrowableCol(2);
	
	ExportFFTBmpButton2 = new wxBitmapButton(this, ID_ExportFFT2, ClipboardBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportFFTBmpButton2->SetToolTip("Simplified export to clipboard");
	ExportFFTFGSizer->Add(ExportFFTBmpButton2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportFFTFGSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(2, 16)), wxLI_VERTICAL), 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	ExportFFTFilePickerCtrl = new wxFilePickerCtrl(this, ID_ExportFFT_FilePicker, wxEmptyString, "Choose a filename", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
	ExportFFTFGSizer->Add(ExportFFTFilePickerCtrl, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(1));
	
	ExportFFTBmpButton = new wxBitmapButton(this, ID_ExportFFT, GoBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportFFTBmpButton->SetToolTip("Export to file");
	ExportFFTFGSizer->Add(ExportFFTBmpButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportFFTSBSizer->Add(ExportFFTFGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(2));
	ExportFGSizer->Add(ExportFFTSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(6));
	
	
	wxStaticBoxSizer* ExportFFTEnvelopeSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "FFT moduli envelope"), wxVERTICAL);
	wxFlexGridSizer* ExportFFTEnvelopeFGSizer = new wxFlexGridSizer(1, 4, 0, 0);
	ExportFFTEnvelopeFGSizer->SetFlexibleDirection(wxBOTH);
	ExportFFTEnvelopeFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportFFTEnvelopeFGSizer->AddGrowableCol(2);
	
	ExportFFTEnvelopeBmpButton2 = new wxBitmapButton(this, ID_ExportFFTEnvelope2, ClipboardBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportFFTEnvelopeBmpButton2->SetToolTip("Simplified export to clipboard");
	ExportFFTEnvelopeFGSizer->Add(ExportFFTEnvelopeBmpButton2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportFFTEnvelopeFGSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(2, 16)), wxLI_VERTICAL), 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	ExportFFTEnvelopeFilePickerCtrl = new wxFilePickerCtrl(this, ID_ExportFFTEnvelope_FilePicker, wxEmptyString, "Choose a filename", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
	ExportFFTEnvelopeFGSizer->Add(ExportFFTEnvelopeFilePickerCtrl, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(1));
	
	ExportFFTEnvelopeBmpButton = new wxBitmapButton(this, ID_ExportFFTEnvelope, GoBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportFFTEnvelopeBmpButton->SetToolTip("Export to file");
	ExportFFTEnvelopeFGSizer->Add(ExportFFTEnvelopeBmpButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportFFTEnvelopeSBSizer->Add(ExportFFTEnvelopeFGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(2));
	ExportFGSizer->Add(ExportFFTEnvelopeSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(6));
	
	
	wxStaticBoxSizer* ExportFFTRealEnvelopeSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "FFT real parts envelope"), wxVERTICAL);
	wxFlexGridSizer* ExportFFTRealEnvelopeFGSizer = new wxFlexGridSizer(1, 4, 0, 0);
	ExportFFTRealEnvelopeFGSizer->SetFlexibleDirection(wxBOTH);
	ExportFFTRealEnvelopeFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportFFTRealEnvelopeFGSizer->AddGrowableCol(2);
	
	ExportFFTRealEnvelopeBmpButton2 = new wxBitmapButton(this, ID_ExportFFTRealEnvelope2, ClipboardBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportFFTRealEnvelopeBmpButton2->SetToolTip("Simplified export to clipboard");
	ExportFFTRealEnvelopeFGSizer->Add(ExportFFTRealEnvelopeBmpButton2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportFFTRealEnvelopeFGSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(2, 16)), wxLI_VERTICAL), 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	ExportFFTRealEnvelopeFilePickerCtrl = new wxFilePickerCtrl(this, ID_ExportFFTRealEnvelope_FilePicker, wxEmptyString, "Choose a filename", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
	ExportFFTRealEnvelopeFGSizer->Add(ExportFFTRealEnvelopeFilePickerCtrl, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(1));
	
	ExportFFTRealEnvelopeBmpButton = new wxBitmapButton(this, ID_ExportFFTRealEnvelope, GoBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportFFTRealEnvelopeBmpButton->SetToolTip("Export to file");
	ExportFFTRealEnvelopeFGSizer->Add(ExportFFTRealEnvelopeBmpButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportFFTRealEnvelopeSBSizer->Add(ExportFFTRealEnvelopeFGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(2));
	ExportFGSizer->Add(ExportFFTRealEnvelopeSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(6));
	
	
	wxStaticBoxSizer* ExportEvaluationSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Evaluation"), wxVERTICAL);
	wxFlexGridSizer* ExportEvaluationFGSizer = new wxFlexGridSizer(1, 4, 0, 0);
	ExportEvaluationFGSizer->SetFlexibleDirection(wxBOTH);
	ExportEvaluationFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	ExportEvaluationFGSizer->AddGrowableCol(2);
	
	ExportEvaluationBmpButton2 = new wxBitmapButton(this, ID_ExportEvaluation2, ClipboardBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportEvaluationBmpButton2->SetToolTip("Simplified export to clipboard");
	ExportEvaluationFGSizer->Add(ExportEvaluationBmpButton2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportEvaluationFGSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(2, 16)), wxLI_VERTICAL), 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(3));
	
	ExportEvaluationFilePickerCtrl = new wxFilePickerCtrl(this, ID_ExportEvaluation_FilePicker, wxEmptyString, "Choose a filename", "*.*", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
	ExportEvaluationFGSizer->Add(ExportEvaluationFilePickerCtrl, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, FromDIP(1));
	
	ExportEvaluationBmpButton = new wxBitmapButton(this, ID_ExportEvaluation, GoBmp, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	ExportEvaluationBmpButton->SetToolTip("Export to file");
	ExportEvaluationFGSizer->Add(ExportEvaluationBmpButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ExportEvaluationSBSizer->Add(ExportEvaluationFGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(2));
	ExportFGSizer->Add(ExportEvaluationSBSizer, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, FromDIP(6));
	
	this->SetSizer(ExportFGSizer);
	this->Layout();
	ExportFGSizer->Fit(this);
	
	/// will be enabled by OnUpdateUI() handler if needed
	Disable();
}

NFGExportPanel::~NFGExportPanel()
{
}

void NFGExportPanel::OnExportCommand(wxCommandEvent& event)
{
	int RetVal = DATA_OK;
	
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	switch (event.GetId()) {
		case ID_ExportTDD:
			RetVal = CurrentSerDocument->TextExport(ExportTDDFilePickerCtrl->GetPath(), EXPORT_TDD);
			break;
			
		case ID_ExportEchoPeaksEnvelope:
			RetVal = CurrentSerDocument->TextExport(ExportEchoPeaksEnvelopeFilePickerCtrl->GetPath(), EXPORT_EchoPeaksEnvelope);
			break;
			
		case ID_ExportChunkAvg:
			RetVal = CurrentSerDocument->TextExport(ExportChunkAvgFilePickerCtrl->GetPath(), EXPORT_ChunkAvg);
			break;
			
		case ID_ExportFFT:
			RetVal = CurrentSerDocument->TextExport(ExportFFTFilePickerCtrl->GetPath(), EXPORT_DFTPhaseCorrResult);
			break;
			
		case ID_ExportFFTEnvelope:
			RetVal = CurrentSerDocument->TextExport(ExportFFTEnvelopeFilePickerCtrl->GetPath(), EXPORT_DFTEnvelope);
			break;
			
		case ID_ExportFFTRealEnvelope:
			RetVal = CurrentSerDocument->TextExport(ExportFFTRealEnvelopeFilePickerCtrl->GetPath(), EXPORT_DFTPhaseCorrRealEnvelope);
			break;
			
		case ID_ExportEvaluation:
			RetVal = CurrentSerDocument->TextExport(ExportEvaluationFilePickerCtrl->GetPath(), EXPORT_Evaluation);
			break;
			
		
		case ID_ExportTDD2:
			RetVal = CurrentSerDocument->ClipboardExport(EXPORT_TDD);
			break;
			
		case ID_ExportEchoPeaksEnvelope2:
			RetVal = CurrentSerDocument->ClipboardExport(EXPORT_EchoPeaksEnvelope);
			break;
			
		case ID_ExportChunkAvg2:
			RetVal = CurrentSerDocument->ClipboardExport(EXPORT_ChunkAvg);
			break;
			
		case ID_ExportFFT2:
			RetVal = CurrentSerDocument->ClipboardExport(EXPORT_DFTPhaseCorrResult);
			break;
			
		case ID_ExportFFTEnvelope2:
			RetVal = CurrentSerDocument->ClipboardExport(EXPORT_DFTEnvelope);
			break;
			
		case ID_ExportFFTRealEnvelope2:
			RetVal = CurrentSerDocument->ClipboardExport(EXPORT_DFTPhaseCorrRealEnvelope);
			break;
			
		case ID_ExportEvaluation2:
			RetVal = CurrentSerDocument->ClipboardExport(EXPORT_Evaluation);
			break;
			
		default:
			return;
	}
	
	if ((RetVal != DATA_OK) && (RetVal != (FILE_OPEN_ERROR | DATA_OLD) /** overwrite prevented by user **/))
		::wxMessageBox("Data export failed.", "Error", wxOK | wxICON_ERROR);
	
	/// Update the Views list
	if (RetVal == DATA_OK)
		CurrentSerDocument->LoadViewList();
	
}

void NFGExportPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) {

		CurrentSerDocument = NULL;
		
		ExportTDDFilePickerCtrl->SetPath(wxEmptyString);
		ExportEchoPeaksEnvelopeFilePickerCtrl->SetPath(wxEmptyString);
		ExportChunkAvgFilePickerCtrl->SetPath(wxEmptyString);
		ExportFFTFilePickerCtrl->SetPath(wxEmptyString);
		ExportFFTEnvelopeFilePickerCtrl->SetPath(wxEmptyString);
		ExportFFTRealEnvelopeFilePickerCtrl->SetPath(wxEmptyString);
		ExportEvaluationFilePickerCtrl->SetPath(wxEmptyString);

		Disable();
		
		return;
	}
	
	Enable();
	
	if (SerDoc == CurrentSerDocument)
		return;
	
	CurrentSerDocument = SerDoc;
	
	ExportTDDFilePickerCtrl->SetPath(CurrentSerDocument->TextExportFilenameQuery(EXPORT_TDD));
	ExportEchoPeaksEnvelopeFilePickerCtrl->SetPath(CurrentSerDocument->TextExportFilenameQuery(EXPORT_EchoPeaksEnvelope));
	ExportChunkAvgFilePickerCtrl->SetPath(CurrentSerDocument->TextExportFilenameQuery(EXPORT_ChunkAvg));
	ExportFFTFilePickerCtrl->SetPath(CurrentSerDocument->TextExportFilenameQuery(EXPORT_DFTResult));
	ExportFFTEnvelopeFilePickerCtrl->SetPath(CurrentSerDocument->TextExportFilenameQuery(EXPORT_DFTEnvelope));
	ExportFFTRealEnvelopeFilePickerCtrl->SetPath(CurrentSerDocument->TextExportFilenameQuery(EXPORT_DFTPhaseCorrRealEnvelope));
	ExportEvaluationFilePickerCtrl->SetPath(CurrentSerDocument->TextExportFilenameQuery(EXPORT_Evaluation));
}


BEGIN_EVENT_TABLE(NFGPrintPanel, wxPanel)
	EVT_BUTTON(ID_Print, NFGPrintPanel::OnPrintCommand)
	EVT_BUTTON(ID_PrintPS, NFGPrintPanel::OnPrintCommand)
	EVT_BUTTON(ID_PrintEPS, NFGPrintPanel::OnPrintCommand)
	EVT_BUTTON(ID_PrintMetafile2Clipboard, NFGPrintPanel::OnPrintCommand)
	EVT_BUTTON(ID_PrintPNG, NFGPrintPanel::OnPrintCommand)
	EVT_BUTTON(ID_PrintBitmap2Clipboard, NFGPrintPanel::OnPrintCommand)
	EVT_UPDATE_UI(ID_Print_Panel, NFGPrintPanel::OnUpdateUI)
END_EVENT_TABLE()

NFGPrintPanel::NFGPrintPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	CurrentSerDocument = NULL;
	
	PrintReq.width_mm = 160;
	PrintReq.height_mm = 60;
	PrintReq.width_px = 800;
	PrintReq.height_px = 300;
	PrintReq.dpi = 100;
	PrintReq.point_size = 10;
	PrintReq.portrait = false;
	PrintReq.landscape = true;
	PrintReq.params_and_key_at_right = false;
	PrintReq.black_and_white = false;
	PrintReq.print_params = false;
	PrintReq.print_key = false;
	PrintReq.print_title = false;
	PrintReq.request_type = ID_Print;

	wxFlexGridSizer* PrintFGSizer;
	PrintFGSizer = new wxFlexGridSizer(9, 1, FromDIP(6), 0);
	PrintFGSizer->AddGrowableCol(0);
	PrintFGSizer->SetFlexibleDirection(wxBOTH);
	PrintFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	wxGridSizer* PrintCheckGSizer;
	PrintCheckGSizer = new wxGridSizer(2, 2, FromDIP(7), FromDIP(20));
	
	PrintTitleCheckBox = new wxCheckBox(this, wxID_ANY, "Print title", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(PrintReq.print_title)));
	
	PrintCheckGSizer->Add(PrintTitleCheckBox, 0, wxLEFT|wxEXPAND, FromDIP(7));
	
	PrintParamsCheckBox = new wxCheckBox(this, wxID_ANY, "Print parameters", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(PrintReq.print_params)));
	
	PrintCheckGSizer->Add(PrintParamsCheckBox, 0, wxRIGHT|wxEXPAND, FromDIP(7));
	
	PrintKeyCheckBox = new wxCheckBox(this, wxID_ANY, "Print key", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(PrintReq.print_key)));
	
	PrintCheckGSizer->Add(PrintKeyCheckBox, 0, wxLEFT|wxEXPAND, FromDIP(7));
	
	PrintBWCheckBox = new wxCheckBox(this, wxID_ANY, "Black and white", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(PrintReq.black_and_white)));
	
	PrintCheckGSizer->Add(PrintBWCheckBox, 0, wxRIGHT|wxEXPAND, FromDIP(7));
	
	PrintFGSizer->Add(PrintCheckGSizer, 1, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, FromDIP(7));
	
	wxStaticBoxSizer* PrintOrientationSBSizer;
	PrintOrientationSBSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Page orientation and layout"), wxVERTICAL);
	
	wxGridSizer* PrintOrientationAndLayoutGSizer;
	PrintOrientationAndLayoutGSizer = new wxGridSizer(2, 1, 0, FromDIP(20));
	
	wxGridSizer* PrintOrientationGSizer;
	PrintOrientationGSizer = new wxGridSizer(1, 2, 0, FromDIP(20));
	
	PrintPortraitRadioButton = new NFGAutoRadioButton(this, wxID_ANY, "Portrait", wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxGenericValidator(&PrintReq.portrait), false);
	PrintOrientationGSizer->Add(PrintPortraitRadioButton, 0, wxALL|wxEXPAND, 0);
	
	PrintLandscapeRadioButton = new NFGAutoRadioButton(this, wxID_ANY, "Landscape", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&PrintReq.landscape), false);
	PrintOrientationGSizer->Add(PrintLandscapeRadioButton, 0, wxALL|wxEXPAND, 0);
	
	PrintOrientationAndLayoutGSizer->Add(PrintOrientationGSizer, 1, wxALL|wxEXPAND, FromDIP(3));

	PrintParamsAndKeyAtRightCheckBox = new wxCheckBox(this, wxID_ANY, "Parameters and key at right side", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&(PrintReq.params_and_key_at_right)));
	
	PrintOrientationAndLayoutGSizer->Add(PrintParamsAndKeyAtRightCheckBox, 1, wxALL|wxEXPAND, FromDIP(3));
	
	PrintOrientationSBSizer->Add(PrintOrientationAndLayoutGSizer, 0, wxALL|wxEXPAND, 0);
	
	PrintFGSizer->Add(PrintOrientationSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(6));
	
	wxFlexGridSizer* PrintBtnFGSizer;
	PrintBtnFGSizer = new wxFlexGridSizer(1, 2, 0, FromDIP(6));
	PrintBtnFGSizer->AddGrowableCol(0);
	PrintBtnFGSizer->AddGrowableCol(1);
	PrintBtnFGSizer->SetFlexibleDirection(wxVERTICAL);
	PrintBtnFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PrintButton = new wxButton(this, ID_Print, "Print...");
	PrintBtnFGSizer->Add(PrintButton, 0, wxALL|wxEXPAND, 0);
	
	PrintPSButton = new wxButton(this, ID_PrintPS, "Print to PS file...");
	PrintBtnFGSizer->Add(PrintPSButton, 0, wxALL|wxEXPAND, 0);
	
	PrintFGSizer->Add(PrintBtnFGSizer, 0, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(5));
	
	PrintStaticLine1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	PrintFGSizer->Add(PrintStaticLine1, 0, wxEXPAND | wxLEFT|wxRIGHT, FromDIP(5));

	wxGridSizer* PrintEPSGSizer;
	PrintEPSGSizer = new wxGridSizer(1, 2, 0, FromDIP(12));
	
	wxFlexGridSizer* PrintEPSFGSizer1;
	PrintEPSFGSizer1 = new wxFlexGridSizer(1, 3, 0, FromDIP(6));
	PrintEPSFGSizer1->AddGrowableCol(1, 1);
	PrintEPSFGSizer1->SetFlexibleDirection(wxBOTH);
	PrintEPSFGSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PrintEPSWidthLabelST = new wxStaticText(this, wxID_ANY, "Width");
	PrintEPSFGSizer1->Add(PrintEPSWidthLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	PrintEPSWidthTextCtrl = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(30,-1)), 0,
							 NFGLongValidator(&(PrintReq.width_mm), POSITIVE_NUMBER), wxTextCtrlNameStr, false);
	PrintEPSFGSizer1->Add(PrintEPSWidthTextCtrl, 1, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PrintEPSWmmLabelST = new wxStaticText(this, wxID_ANY, "mm");
	PrintEPSFGSizer1->Add(PrintEPSWmmLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	PrintEPSGSizer->Add(PrintEPSFGSizer1, 1, wxEXPAND, 0);
	
	wxFlexGridSizer* PrintEPSFGSizer2;
	PrintEPSFGSizer2 = new wxFlexGridSizer(1, 3, 0, FromDIP(4));
	PrintEPSFGSizer2->AddGrowableCol(1, 1);
	PrintEPSFGSizer2->SetFlexibleDirection(wxBOTH);
	PrintEPSFGSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PrintEPSHeightLabelST = new wxStaticText(this, wxID_ANY, "Height");
	PrintEPSFGSizer2->Add(PrintEPSHeightLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	PrintEPSHeightTextCtrl = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(30,-1)), 0,
							 NFGLongValidator(&(PrintReq.height_mm), POSITIVE_NUMBER), wxTextCtrlNameStr, false);
	PrintEPSFGSizer2->Add(PrintEPSHeightTextCtrl, 1, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PrintEPSHmmLabelST = new wxStaticText(this, wxID_ANY, "mm");
	PrintEPSFGSizer2->Add(PrintEPSHmmLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	PrintEPSGSizer->Add(PrintEPSFGSizer2, 1, wxEXPAND, 0);
	
	PrintFGSizer->Add(PrintEPSGSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(5));
	
	
	PrintEPSButton = new wxButton(this, ID_PrintEPS, "Print to EPS file...");
	PrintFGSizer->Add(PrintEPSButton, 0, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(5));
	
	PrintStaticLine2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	PrintFGSizer->Add(PrintStaticLine2, 0, wxEXPAND | wxLEFT|wxRIGHT, FromDIP(5));
	
	
	wxGridSizer* PrintPNGGSizer;
	PrintPNGGSizer = new wxGridSizer(3, 2, FromDIP(6), FromDIP(6));
	
	wxFlexGridSizer* PrintPNGFGSizer1;
	PrintPNGFGSizer1 = new wxFlexGridSizer(1, 3, 0, FromDIP(4));
	PrintPNGFGSizer1->AddGrowableCol(1, 1);
	PrintPNGFGSizer1->SetFlexibleDirection(wxBOTH);
	PrintPNGFGSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PrintPNGWidthLabelST = new wxStaticText(this, wxID_ANY, "Width");
	PrintPNGFGSizer1->Add(PrintPNGWidthLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	PrintPNGWidthTextCtrl = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(30,-1)), 0,
							 NFGLongValidator(&(PrintReq.width_px), POSITIVE_NUMBER), wxTextCtrlNameStr, false);
	PrintPNGFGSizer1->Add(PrintPNGWidthTextCtrl, 1, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PrintPNGWpxLabelST = new wxStaticText(this, wxID_ANY, "px");
	PrintPNGFGSizer1->Add(PrintPNGWpxLabelST, 0, wxALIGN_CENTER_VERTICAL, 0);
	
	PrintPNGGSizer->Add(PrintPNGFGSizer1, 1, wxRIGHT|wxEXPAND, FromDIP(6));
	
	
	wxFlexGridSizer* PrintPNGFGSizer2;
	PrintPNGFGSizer2 = new wxFlexGridSizer(1, 3, 0, FromDIP(4));
	PrintPNGFGSizer2->AddGrowableCol(1, 1);
	PrintPNGFGSizer2->SetFlexibleDirection(wxBOTH);
	PrintPNGFGSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PrintPNGHeightLabelST = new wxStaticText(this, wxID_ANY, "Height");
	PrintPNGFGSizer2->Add(PrintPNGHeightLabelST, 0, wxALIGN_CENTER_VERTICAL, 0);
	
	PrintPNGHeightTextCtrl = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(30,-1)), 0,
							 NFGLongValidator(&(PrintReq.height_px), POSITIVE_NUMBER), wxTextCtrlNameStr, false);
	PrintPNGFGSizer2->Add(PrintPNGHeightTextCtrl, 1, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PrintPNGHpxLabelST = new wxStaticText(this, wxID_ANY, "px");
	PrintPNGFGSizer2->Add(PrintPNGHpxLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	PrintPNGGSizer->Add(PrintPNGFGSizer2, 1, wxLEFT|wxEXPAND, FromDIP(6));


	wxFlexGridSizer* PrintPNGFGSizer3;
	PrintPNGFGSizer3 = new wxFlexGridSizer(1, 3, 0, FromDIP(4));
	PrintPNGFGSizer3->AddGrowableCol(1, 1);
	PrintPNGFGSizer3->SetFlexibleDirection(wxBOTH);
	PrintPNGFGSizer3->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PrintPNGResolutionLabelST = new wxStaticText(this, wxID_ANY, "Resolution");
	PrintPNGFGSizer3->Add(PrintPNGResolutionLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	PrintPNGResolutionTextCtrl = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(49,-1)), 0,
							 NFGLongValidator(&(PrintReq.dpi), POSITIVE_NUMBER), wxTextCtrlNameStr, false);
	PrintPNGFGSizer3->Add(PrintPNGResolutionTextCtrl, 1, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
	
	PrintPNGdpiLabelST = new wxStaticText(this, wxID_ANY, "dpi");
	PrintPNGFGSizer3->Add(PrintPNGdpiLabelST, 0, wxALIGN_CENTER_VERTICAL, 0);

	PrintPNGGSizer->Add(PrintPNGFGSizer3, 1, wxRIGHT|wxEXPAND, FromDIP(6));
	
#ifdef __WXMSW__
	PrintMetafile2ClipboardButton = new wxButton(this, ID_PrintMetafile2Clipboard, "Metafile to clipboard", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	PrintPNGGSizer->Add(PrintMetafile2ClipboardButton, 0, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND, 0);
#else
	PrintPNGGSizer->AddStretchSpacer();
#endif

	PrintPNGButton = new wxButton(this, ID_PrintPNG, "Print to PNG file...");
	PrintPNGGSizer->Add(PrintPNGButton, 0, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND, 0);
	
	PrintBitmap2ClipboardButton = new wxButton(this, ID_PrintBitmap2Clipboard, "Bitmap to clipboard", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	PrintPNGGSizer->Add(PrintBitmap2ClipboardButton, 0, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND, 0);
	
	
	PrintFGSizer->Add(PrintPNGGSizer, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, FromDIP(5));
	
	this->SetSizer(PrintFGSizer);
	this->Layout();
	PrintFGSizer->Fit(this);
	
	TransferDataToWindow();
}

NFGPrintPanel::~NFGPrintPanel()
{
}

void NFGPrintPanel::OnPrintCommand(wxCommandEvent& event)
{
	int RetVal = DATA_OK;
	
	if ((DocMan == NULL) || (CurrentSerDocument == NULL))
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if ((SerDoc == NULL) || (SerDoc != CurrentSerDocument))
		return;

	if (Validate()) {
		TransferDataFromWindow();
		
		PrintReq.request_type = event.GetId();
		
		TransferDataToWindow();
		
		RetVal = CurrentSerDocument->PrintGraph(PrintReq);
		if ((RetVal != DATA_OK) && ((RetVal & ERROR_REPORTED) != ERROR_REPORTED))
			::wxMessageBox("Printing failed.", "Error", wxOK | wxICON_ERROR);

	} else
		wxBell();

}

void NFGPrintPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);
	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	if ((SerDoc == NULL) || (view == NULL)) {
		CurrentSerDocument = NULL;
		Disable();
		return;
	}
	
	Enable();
	
	if (SerDoc == CurrentSerDocument)
		return;
	
	CurrentSerDocument = SerDoc;
}


NFGProcPanel::NFGProcPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	wxBoxSizer* ProcBSizer;
	ProcBSizer = new wxBoxSizer(wxVERTICAL);
	
	ProcNotebook = new wxNotebook(this, wxID_ANY);

	ProcessPanel = new NFGProcessPanel(DocMan, ProcNotebook, ID_Process_Panel, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	ProcNotebook->AddPage(ProcessPanel, "Process", true);
	PhasePanel = new NFGPhasePanel(DocMan, ProcNotebook, ID_Phase_Panel, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	ProcNotebook->AddPage(PhasePanel, "Phase", false);
	PlotPanel = new NFGPlotPanel(DocMan, ProcNotebook, ID_Plot_Panel, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	ProcNotebook->AddPage(PlotPanel, "Plot", false);
	ViewsPanel = new NFGViewsPanel(DocMan, ProcNotebook, ID_Views_Panel, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	ProcNotebook->AddPage(ViewsPanel, "Views", false);
	ExportPanel = new NFGExportPanel(DocMan, ProcNotebook, ID_Export_Panel, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	ProcNotebook->AddPage(ExportPanel, "Export", false);
	PrintPanel = new NFGPrintPanel(DocMan, ProcNotebook, ID_Print_Panel, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	ProcNotebook->AddPage(PrintPanel, "Print", false);
	
	ProcBSizer->Add(ProcNotebook, 1, wxEXPAND | wxALL, FromDIP(5));
	
	this->SetSizer(ProcBSizer);
	this->Layout();
	ProcBSizer->Fit(this);
}

NFGProcPanel::~NFGProcPanel()
{
}
