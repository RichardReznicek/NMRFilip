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

#include "userlist.h"
#include <wx/tokenzr.h> 
#include <cstring>

#include "nmrdata.h"
#include "validators.h"
#include "gui_ids.h"


BEGIN_EVENT_TABLE(NFGUserlistPanel, wxPanel)
	EVT_UPDATE_UI(ID_UserlistGen, NFGUserlistPanel::OnUpdateUI)
END_EVENT_TABLE()

NFGUserlistPanel::NFGUserlistPanel(wxWindow* parent, unsigned long AssocValueType) : wxPanel(parent, ID_UserlistGen, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
	const char* Units[ASSOC_UserlistHighest+1] = 		{"", "", "dB", "s", "s", "s", "us", "MHz"};
	const double DefStart[ASSOC_UserlistHighest+1] = 		{0.0, 0.0, 22.0, 0.05, 0.1, 0.001, 0.2, 70.00132};
	const double DefStep[ASSOC_UserlistHighest+1] = 		{0.0, 1.0, -2.0, 0.01, 0.02, 0.002, 0.2, 0.1};
	const double DefCoef[ASSOC_UserlistHighest+1] = 		{1.0, 1.0, 1.0, 1.5, 1.5, 1.5, 1.0, 1.0};
	const long DefStepCount[ASSOC_UserlistHighest+1] = 	{1, 10, 10, 10, 10, 10, 10, 21};

	AssocType = ((AssocValueType > ASSOC_UserlistHighest)?(ASSOC_NOT_SET):(AssocValueType));

	StepCount = DefStepCount[AssocType];
	Start = DefStart[AssocType];
	Step = DefStep[AssocType];
	Coef = DefCoef[AssocType];
	End = 0.0;
	Sum = 0.0;

	StepOrder = 0;
	UseValueList = false;
	TuneBeforeStart = false;
	
	DoNotTune = (AssocType == ASSOC_FREQ_MHZ);
	TuneOnce = false;
	TuneEvery = false;

	TuneEveryValue = 1;

	Legacy = false;

	UserlistPanelFGSizer = new wxFlexGridSizer(((AssocType == ASSOC_FREQ_MHZ)?(4):(2)), 1, 0, 0);
	UserlistPanelFGSizer->AddGrowableCol(0);
	UserlistPanelFGSizer->SetFlexibleDirection(wxBOTH);
	UserlistPanelFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


	InnerFGSizer = new wxFlexGridSizer(5, 6, 0, 0);
	InnerFGSizer->AddGrowableCol(0);
	InnerFGSizer->AddGrowableCol(1);
	InnerFGSizer->AddGrowableCol(2);
	InnerFGSizer->AddGrowableCol(3);
	InnerFGSizer->AddGrowableCol(4);
	InnerFGSizer->AddGrowableCol(5);
	InnerFGSizer->SetFlexibleDirection(wxBOTH);
	InnerFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	StartLabelST = new wxStaticText(this, wxID_ANY, "Start");
	InnerFGSizer->Add(StartLabelST, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	StartTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_Start, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGDoubleValidator(&Start));
	InnerFGSizer->Add(StartTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	StartsLabelST = new wxStaticText(this, wxID_ANY, Units[AssocType]);
	InnerFGSizer->Add(StartsLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	EndLabelST = new wxStaticText(this, wxID_ANY, "End");
	InnerFGSizer->Add(EndLabelST, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	EndST = new NFGValidatedStaticText(this, ID_End, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGDoubleValidator(&End));
	InnerFGSizer->Add(EndST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	EndsLabelST = new wxStaticText(this, wxID_ANY, Units[AssocType]);
	InnerFGSizer->Add(EndsLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	NumberLabelST = new wxStaticText(this, wxID_ANY, "Number of steps");
	InnerFGSizer->Add(NumberLabelST, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	NumberTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_Number, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGLongValidator(&StepCount, POSITIVE_NUMBER));
	InnerFGSizer->Add(NumberTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	InnerFGSizer->AddStretchSpacer();
	
	StepLabelST = new wxStaticText(this, wxID_ANY, "Step");
	InnerFGSizer->Add(StepLabelST, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	StepTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_Step, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGDoubleValidator(&Step));
	InnerFGSizer->Add(StepTextCtrl, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	StepsLabelST = new wxStaticText(this, wxID_ANY, Units[AssocType]);
	InnerFGSizer->Add(StepsLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));

	if ((AssocType != ASSOC_FREQ_MHZ) && (AssocType != ASSOC_NUTATION_US) && (AssocType != ASSOC_PWR_DB)) {
		CoefficientLabelST = new wxStaticText(this, wxID_ANY, "Coefficient");
		InnerFGSizer->Add(CoefficientLabelST, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|((AssocType == ASSOC_VARIABLE)?(wxRESERVE_SPACE_EVEN_IF_HIDDEN):(0)), FromDIP(5));
		
		CoefficientTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_Coefficient, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGDoubleValidator(&Coef));
		InnerFGSizer->Add(CoefficientTextCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND|((AssocType == ASSOC_VARIABLE)?(wxRESERVE_SPACE_EVEN_IF_HIDDEN):(0)), FromDIP(5));
		InnerFGSizer->AddStretchSpacer();
	} else {
		CoefficientLabelST = NULL;
		CoefficientTextCtrl = NULL;
		InnerFGSizer->AddStretchSpacer();
		InnerFGSizer->AddStretchSpacer();
		InnerFGSizer->AddStretchSpacer();
	}
	
	SumLabelST = NULL;
	SumST = NULL;
	SumsLabelST = NULL;
	VariableNameLabelST = NULL;
	VariableNameTextCtrl = NULL;

	if (AssocType == ASSOC_TRIG_S) {
		SumLabelST = new wxStaticText(this, wxID_ANY, "Sum");
		InnerFGSizer->Add(SumLabelST, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, FromDIP(5));
		
		SumST = new NFGValidatedStaticText(this, ID_Sum, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGDoubleValidator(&Sum));
		InnerFGSizer->Add(SumST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
		
		SumsLabelST = new wxStaticText(this, wxID_ANY, Units[AssocType]);
		InnerFGSizer->Add(SumsLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
		
	} else 
	if (AssocType == ASSOC_VARIABLE) {
		VariableNameLabelST = new wxStaticText(this, wxID_ANY, "Variable name");
		InnerFGSizer->Add(VariableNameLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, FromDIP(5));
		
		NFGTextValidator VariableValidator = NFGTextValidator(wxFILTER_INCLUDE_CHAR_LIST, &Variable);
		VariableValidator.SetCharIncludes("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/_ ,;");
		
		VariableNameTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_VariableName, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, VariableValidator);
		VariableNameTextCtrl->SetToolTip("Separate multiple variables by semicolon or comma");
		InnerFGSizer->Add(VariableNameTextCtrl, 0, wxALL|wxEXPAND, FromDIP(5));
		InnerFGSizer->AddStretchSpacer();
		
	} else {
		InnerFGSizer->AddStretchSpacer();
		InnerFGSizer->AddStretchSpacer();
		InnerFGSizer->AddStretchSpacer();
	}

	
	UseValueListCheckBox = new NFGAutoCheckBox(this, wxID_ANY, "Use value list", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&UseValueList));
	UseValueListCheckBox->SetToolTip("Enter values separated by space, tab, semicolon or newline");
	InnerFGSizer->Add(UseValueListCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));

	if (AssocType == ASSOC_FREQ_MHZ) {
		TuneBeforeStartCheckBox = NULL;
		InnerFGSizer->AddStretchSpacer();
		
	} else {
		TuneBeforeStartCheckBox = new NFGAutoCheckBox(this, wxID_ANY, "Tune before start", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&TuneBeforeStart));;
		InnerFGSizer->Add(TuneBeforeStartCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	}
	InnerFGSizer->AddStretchSpacer();
	
	
	wxString choices[5] = {"sequential", "reversed", "interlaced", "expanding", "spread"};
	StepOrderST = new wxStaticText(this, wxID_ANY, "Step order");
	InnerFGSizer->Add(StepOrderST, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, FromDIP(5));
	
	StepOrderChoice = new NFGAutoChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, choices, 0, wxGenericValidator(&StepOrder));
	InnerFGSizer->Add(StepOrderChoice, 0, wxALL|wxEXPAND, FromDIP(5));
	
	InnerFGSizer->AddStretchSpacer();
	
	
	NFGTextValidator ValueListValidator = NFGTextValidator(wxFILTER_INCLUDE_CHAR_LIST, &ValueList);
	ValueListValidator.SetCharIncludes("+-0123456789.,e ;\t\r\n");
	wxSize ValueListSize;
	if ((AssocType == ASSOC_TRIG_S) || (AssocType == ASSOC_INVREC_S) || (AssocType == ASSOC_T2_S))
		ValueListSize = wxSize(-1, 3*StartTextCtrl->GetSize().GetHeight() + 4*FromDIP(5));
	else
		ValueListSize = wxSize(-1, 2*StartTextCtrl->GetSize().GetHeight() + 2*FromDIP(5));
	ValueListTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_ValueList, wxEmptyString, wxDefaultPosition, ValueListSize, wxTE_MULTILINE, ValueListValidator);
	UserlistPanelFGSizer->Add(ValueListTextCtrl, 1, wxALL|wxEXPAND, FromDIP(5));
	
	
	UserlistPanelFGSizer->Add(InnerFGSizer, 1, wxEXPAND, 0);
	
	if (AssocType == ASSOC_FREQ_MHZ) {
		StaticLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
		UserlistPanelFGSizer->Add(StaticLine, 0, wxALL|wxEXPAND, FromDIP(5));

		
		wxFlexGridSizer* BottomFGSizer;
		BottomFGSizer = new wxFlexGridSizer(1, 5, 0, 0);
		BottomFGSizer->AddGrowableCol(0);
		BottomFGSizer->AddGrowableCol(1);
		BottomFGSizer->AddGrowableCol(2);
		BottomFGSizer->AddGrowableCol(4);
		BottomFGSizer->AddGrowableRow(0);
		BottomFGSizer->SetFlexibleDirection(wxBOTH);
		BottomFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
		
		DoNotTuneRadioButton = new NFGAutoRadioButton(this, ID_DoNotTune, "Do not tune", wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxGenericValidator(&DoNotTune));
		DoNotTuneRadioButton->SetValue(true); 
		BottomFGSizer->Add(DoNotTuneRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, FromDIP(5));
		
		TuneOnceRadioButton = new NFGAutoRadioButton(this, ID_TuneOnce, "Tune before start", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&TuneOnce));
		BottomFGSizer->Add(TuneOnceRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER, FromDIP(5));
		
		TuneEveryRadioButton = new NFGAutoRadioButton(this, ID_TuneEvery, "Tune every", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&TuneEvery));
		BottomFGSizer->Add(TuneEveryRadioButton, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, FromDIP(5));
		
		TuneEveryTextCtrl = new NFGAutoValidatedTextCtrl(this, ID_TuneEveryValue, wxEmptyString, wxDefaultPosition, wxSize(60,-1), 0, NFGLongValidator(&TuneEveryValue, POSITIVE_NUMBER));
		BottomFGSizer->Add(TuneEveryTextCtrl, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, FromDIP(5));
		
		TuneEverystepLabelST = new wxStaticText(this, wxID_ANY, "step(s)");
		BottomFGSizer->Add(TuneEverystepLabelST, 0, wxALL|wxALIGN_CENTER_VERTICAL, FromDIP(5));
		
		UserlistPanelFGSizer->Add(BottomFGSizer, 1, wxEXPAND, 0);

	} else {
		StaticLine = NULL;
		DoNotTuneRadioButton = NULL;
		TuneOnceRadioButton = NULL;
		TuneEveryRadioButton = NULL;
		TuneEveryTextCtrl = NULL;
		TuneEverystepLabelST = NULL;
	}
	
	UserlistPanelFGSizer->Show((size_t) 0, UseValueList);
	for (size_t i = 0; i < 12; i++)
		InnerFGSizer->Show(i, !UseValueList);
	if (AssocType == ASSOC_VARIABLE) 
		for (size_t i = 12; i < 14; i++)
			InnerFGSizer->Show(i, !UseValueList);
	if ((AssocType == ASSOC_TRIG_S) || (AssocType == ASSOC_INVREC_S) || (AssocType == ASSOC_T2_S))
		for (size_t i = 12; i < 18; i++)
			InnerFGSizer->Show(i, !UseValueList);

	InnerFGSizer->Show(18, !Legacy);
	InnerFGSizer->Show(21, !Legacy);
	InnerFGSizer->Show(22, !Legacy);
	InnerFGSizer->Show(23, !Legacy);
	
	SetSizer(UserlistPanelFGSizer);
	Layout();
}

NFGUserlistPanel::~NFGUserlistPanel()
{
}

void NFGUserlistPanel::OnUpdateUI(wxUpdateUIEvent& event) 
{
	if (UseValueList != UserlistPanelFGSizer->IsShown((size_t) 0)) {
		UserlistPanelFGSizer->Show((size_t) 0, UseValueList);
		for (size_t i = 0; i < 12; i++)
			InnerFGSizer->Show(i, !UseValueList);
		if ((AssocType == ASSOC_TRIG_S) || (AssocType == ASSOC_INVREC_S) || (AssocType == ASSOC_T2_S))
			for (size_t i = 12; i < 18; i++)
				InnerFGSizer->Show(i, !UseValueList);
		if (AssocType == ASSOC_VARIABLE) 
			for (size_t i = 12; i < 14; i++)
				InnerFGSizer->Show(i, !UseValueList);
		
		Layout();
	}
}

bool NFGUserlistPanel::TransferDataToWindow()
{
	CalcEnd();
	
	return wxPanel::TransferDataToWindow();
}

/// Generic userlist panel data loading function - subclasses may use this or override it
bool NFGUserlistPanel::Load(UserlistParams* UParams, bool LegacyFormat)
{
	if (AssocType != UParams->AssocValueType)
		return false;
	
	SetLegacy(LegacyFormat);
	
	StepCount = UParams->StepCount;
	
	if (Legacy || (UParams->AssocValues == NULL)) {
		UseValueList = false;
		ValueList = wxEmptyString;
	} else {
		UseValueList = true;
		ValueList = wxEmptyString;
		for (long i = 0; i < StepCount; i++)
			ValueList.Append(wxString::Format("%.14g ", UParams->AssocValues[i]));
	}
	
	Start = ((wxFinite(UParams->AssocValueStart))?(UParams->AssocValueStart):(0.0));
	Step = ((wxFinite(UParams->AssocValueStep))?(UParams->AssocValueStep):(0.0));
	Coef = ((wxFinite(UParams->AssocValueCoef))?(UParams->AssocValueCoef):(1.0));
	if ((AssocType == ASSOC_FREQ_MHZ) || (AssocType == ASSOC_NUTATION_US) || (AssocType == ASSOC_PWR_DB)) 
		Coef = 1.0;
	

	if (AssocType == ASSOC_FREQ_MHZ) {
		TuneBeforeStart = false;
		
		DoNotTune = ((UParams->WobbStep > StepCount) || (UParams->WobbStep <= 0));
		TuneOnce = ((UParams->WobbStep == StepCount) && (UParams->WobbStep > 0));
		TuneEvery = !(DoNotTune || TuneOnce);
		TuneEveryValue = ((TuneEvery)?(UParams->WobbStep):(1));
		
	} else {
		TuneBeforeStart = ((UParams->WobbStep == StepCount) && (UParams->WobbStep > 0));
		
		DoNotTune = false;
		TuneOnce = false;
		TuneEvery = false;
		TuneEveryValue = 1;
	}

	Variable = (((UParams->AssocValueVariable != NULL) && (AssocType == ASSOC_VARIABLE))?(wxString(UParams->AssocValueVariable)):(wxString("")));
	
	StepOrder = ((Legacy)?(0):(UParams->StepOrder));
	
	UpdateWindowUI();
	
	return TransferDataToWindow();
}

/// Generic userlist panel data storing function - subclasses may use this or override it
bool NFGUserlistPanel::Store(UserlistParams* UParams, bool LegacyFormat)
{
	if (!TransferDataFromWindow())
		return false;
	
	if (UseValueList && !LegacyFormat) {
		ValueList.Replace(",", ".");	/// accept both '.' and ',' decimal point
		wxArrayString ValStrArr = wxStringTokenize(ValueList, " ;\t\r\n", wxTOKEN_STRTOK);
		if (ValStrArr.GetCount() == 0)
			return false;
		
		double aux, *auxptr;
		for (size_t i = 0; i < ValStrArr.GetCount(); i++)
			if (!ValStrArr[i].ToCDouble(&aux))
				return false;
		
		auxptr = (double *) realloc(UParams->AssocValues, ValStrArr.GetCount()*sizeof(double));
		if (auxptr == NULL)
			return false;
		
		UParams->AssocValues = auxptr;
		UParams->StepCount = ValStrArr.GetCount();
		
		for (size_t i = 0; i < ValStrArr.GetCount(); i++)
			ValStrArr[i].ToCDouble(&(UParams->AssocValues[i]));
		
	} else {
		free(UParams->AssocValues);
		UParams->AssocValues = NULL;
		UParams->StepCount = StepCount;
	}
	
	UParams->AssocValueStart = ((UseValueList)?(NAN):(Start));
	UParams->AssocValueStep = ((UseValueList)?(NAN):(Step));
	UParams->AssocValueCoef = ((UseValueList)?(NAN):(Coef));
	
	UParams->AssocValueType = AssocType;
	
	if (AssocType == ASSOC_FREQ_MHZ) {
		if (TuneEvery && ((TuneEveryValue >= UParams->StepCount) || (TuneEveryValue <= 0))) {
			TuneEvery = false;
			TuneOnce = (TuneEveryValue > 0);
			DoNotTune = !TuneOnce;
		}
		
		if (TuneEvery) 
			UParams->WobbStep = TuneEveryValue;
		else 
			UParams->WobbStep = UParams->StepCount + ((TuneOnce)?(0):(1));
		
	} else {
		UParams->WobbStep = UParams->StepCount + ((TuneBeforeStart)?(0):(1));
	}

	if (Variable.IsEmpty()) {
		free(UParams->AssocValueVariable);
		UParams->AssocValueVariable = NULL;
	} else 
	if ((UParams->AssocValueVariable == NULL) || (std::strcmp(Variable.char_str(), UParams->AssocValueVariable) != 0)) {
		free(UParams->AssocValueVariable);
		UParams->AssocValueVariable = strdup(Variable.char_str());
	}
	
	UParams->StepOrder = ((LegacyFormat)?(0):(StepOrder));
	
	TransferDataToWindow();	/// especially because of ValueList
	
	return true;
}

void NFGUserlistPanel::SetLegacy(bool LegacyFormat)
{
	if (Legacy == LegacyFormat) 
		return;

	Legacy = LegacyFormat;
	
	if (Legacy) {
		UseValueList = false;
		ValueList = wxEmptyString;
		StepOrder = 0;
		
		if (VariableNameTextCtrl != NULL) 
			VariableNameTextCtrl->UnsetToolTip();
	} else
		if (VariableNameTextCtrl != NULL) 
			VariableNameTextCtrl->SetToolTip("Separate multiple variables by semicolon or comma");

	
	InnerFGSizer->Show(18, !Legacy);
	InnerFGSizer->Show(21, !Legacy);
	InnerFGSizer->Show(22, !Legacy);
	InnerFGSizer->Show(23, !Legacy);
	Layout();
}


bool NFGUserlistPanel::IsModified()
{
	bool modified = false;
	
	wxWindowList children = GetChildren();
	wxWindowListNode *node = children.GetFirst();

	while (node)
	{
		wxWindow *child = (wxWindow*) node->GetData();
		if (child) {
			if (child->IsKindOf(CLASSINFO(NFGAutoValidatedTextCtrl)))
				modified = modified || ((NFGAutoValidatedTextCtrl *) child)->IsModified();

			if (child->IsKindOf(CLASSINFO(NFGAutoRadioButton)))
				modified = modified || ((NFGAutoRadioButton *) child)->IsModified();

			if (child->IsKindOf(CLASSINFO(NFGAutoCheckBox)))
				modified = modified || ((NFGAutoCheckBox *) child)->IsModified();
			
			if (child->IsKindOf(CLASSINFO(NFGAutoChoice)))
				modified = modified || ((NFGAutoChoice *) child)->IsModified();
		}

		node = node->GetNext();
	}

	return modified;
}

void NFGUserlistPanel::Modify(bool mod)
{
	if (mod)
		return;
	
	wxWindowList children = GetChildren();
	wxWindowListNode *node = children.GetFirst();

	while (node)
	{
		wxWindow *child = (wxWindow*) node->GetData();
		if (child) {
			if (child->IsKindOf(CLASSINFO(NFGAutoValidatedTextCtrl)))
				((NFGAutoValidatedTextCtrl *) child)->DiscardEdits();

			if (child->IsKindOf(CLASSINFO(NFGAutoRadioButton)))
				((NFGAutoRadioButton *) child)->DiscardEdits();

			if (child->IsKindOf(CLASSINFO(NFGAutoCheckBox)))
				((NFGAutoCheckBox *) child)->DiscardEdits();

			if (child->IsKindOf(CLASSINFO(NFGAutoChoice)))
				((NFGAutoChoice *) child)->DiscardEdits();
		}

		node = node->GetNext();
	}
	
}

unsigned char NFGUserlistPanel::GetAssocType()
{
	return AssocType;
}

/// Generic userlist panel end value calculation function - subclasses may use this or override it
void NFGUserlistPanel::CalcEnd()
{
	double AssocValue = Start;
	double AssocStep = Step;
	Sum = 0.0;
	
	if (Coef == 1.0) {
		End = (StepCount - 1)*Step + Start;
		Sum = StepCount*(Start + End)/2.0;
	} else 
	/** Equation for sum of geometric series not used to avoid rounding errors different from those in AU program during measurement. **/
	for (int i = 0; i < StepCount; i++) {
		End = AssocValue;
		Sum += AssocValue;
		
		AssocValue += AssocStep;
		AssocStep *= Coef;
	}
}



BEGIN_EVENT_TABLE(NFGUserlistMainPanel, wxPanel)
	EVT_TEXT(wxID_ANY, NFGUserlistMainPanel::OnChange)
	EVT_RADIOBUTTON(wxID_ANY, NFGUserlistMainPanel::OnChange)
	EVT_CHECKBOX(wxID_ANY, NFGUserlistMainPanel::OnChange)
	EVT_CHOICE(wxID_ANY, NFGUserlistMainPanel::OnChange)
	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, NFGUserlistMainPanel::OnPageChange)
	EVT_UPDATE_UI(ID_UserlistSave, NFGUserlistMainPanel::OnUpdateUISave)
	EVT_UPDATE_UI(ID_UserlistMainPanel, NFGUserlistMainPanel::OnUpdateUI)
	EVT_BUTTON(ID_UserlistSave, NFGUserlistMainPanel::OnUserlistCommand)
	EVT_BUTTON(ID_UserlistRevert, NFGUserlistMainPanel::OnUserlistCommand)
END_EVENT_TABLE()

NFGUserlistMainPanel::NFGUserlistMainPanel(wxDocument* document, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, ID_UserlistMainPanel, pos, size, style)
{
	Doc = document;
	
	AssocValueType = ASSOC_NOT_SET;
	UseOptionalSettings = false;
	Legacy = false;
	
	UserlistFGSizer = new wxFlexGridSizer(3, 1, 0, 0);
	UserlistFGSizer->AddGrowableCol(0);
	UserlistFGSizer->AddGrowableRow(0);
	UserlistFGSizer->SetFlexibleDirection(wxBOTH);
	UserlistFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	//~ UserlistNotebook = new wxNotebook(this, wxID_ANY);
	UserlistNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
	
	RFPowerPanel = new NFGUserlistPanel(UserlistNotebook, ASSOC_PWR_DB);
	UserlistNotebook->AddPage(RFPowerPanel, "RF power", false);
	TriggerPanel = new NFGUserlistPanel(UserlistNotebook, ASSOC_TRIG_S);
	UserlistNotebook->AddPage(TriggerPanel, "Trigger delay", false);
	SpectrumPanel = new NFGUserlistPanel(UserlistNotebook, ASSOC_FREQ_MHZ);
	UserlistNotebook->AddPage(SpectrumPanel, "Spectrum", true);
	VariablePanel = new NFGUserlistPanel(UserlistNotebook, ASSOC_VARIABLE);
	UserlistNotebook->AddPage(VariablePanel, "Variable", false);
	InvRecPanel = new NFGUserlistPanel(UserlistNotebook, ASSOC_INVREC_S);
	UserlistNotebook->AddPage(InvRecPanel, "Inversion recovery", false);
	T2Panel = new NFGUserlistPanel(UserlistNotebook, ASSOC_T2_S);
	UserlistNotebook->AddPage(T2Panel, "T2 relaxation", false);
	NutationPanel = new NFGUserlistPanel(UserlistNotebook, ASSOC_NUTATION_US);
	UserlistNotebook->AddPage(NutationPanel, "Nutation", false);

	UserlistNotebook->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
	UserlistNotebook->InitDialog();

	UserlistFGSizer->Add(UserlistNotebook, 0, wxALL|wxEXPAND, FromDIP(5));


	OptionalFGSizer = new wxFlexGridSizer(6, 3, FromDIP(10), FromDIP(10));
	OptionalFGSizer->AddGrowableCol(1);
	OptionalFGSizer->AddGrowableCol(2);
	OptionalFGSizer->SetFlexibleDirection(wxBOTH);
	OptionalFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	DestinationST = new wxStaticText(this, wxID_ANY, "Destination name");
	OptionalFGSizer->Add(DestinationST, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM, FromDIP(5));
	DestinationTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_NONE, &Destination));
	OptionalFGSizer->Add(DestinationTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, FromDIP(5));
	OptionalFGSizer->AddStretchSpacer();

	CommandST = new wxStaticText(this, wxID_ANY, "Commands to run");
	OptionalFGSizer->Add(CommandST, 0, wxALIGN_CENTER_VERTICAL);
	WrkST = new wxStaticText(this, wxID_ANY, "in working dataset");
	OptionalFGSizer->Add(WrkST, 0, wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL);
	DstST = new wxStaticText(this, wxID_ANY, "in destination dataset");
	OptionalFGSizer->Add(DstST, 0, wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL);

	BeforeExpST = new wxStaticText(this, wxID_ANY, "before experiment");
	OptionalFGSizer->Add(BeforeExpST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
	RunBeforeExpWrkTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunBeforeExpWrk));
	OptionalFGSizer->Add(RunBeforeExpWrkTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	RunBeforeExpDstTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunBeforeExpDst));
	OptionalFGSizer->Add(RunBeforeExpDstTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	
	AfterExpST = new wxStaticText(this, wxID_ANY, "after experiment");
	OptionalFGSizer->Add(AfterExpST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
	RunAfterExpWrkTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunAfterExpWrk));
	OptionalFGSizer->Add(RunAfterExpWrkTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	RunAfterExpDstTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunAfterExpDst));
	OptionalFGSizer->Add(RunAfterExpDstTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	
	BeforeStepST = new wxStaticText(this, wxID_ANY, "before each step");
	OptionalFGSizer->Add(BeforeStepST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
	RunBeforeStepWrkTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunBeforeStepWrk));
	OptionalFGSizer->Add(RunBeforeStepWrkTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	RunBeforeStepDstTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunBeforeStepDst));
	OptionalFGSizer->Add(RunBeforeStepDstTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	
	AfterStepST = new wxStaticText(this, wxID_ANY, "after each step");
	OptionalFGSizer->Add(AfterStepST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
	RunAfterStepWrkTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunAfterStepWrk));
	OptionalFGSizer->Add(RunAfterStepWrkTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	RunAfterStepDstTC = new NFGAutoValidatedTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NFGTextValidator(wxFILTER_ASCII, &RunAfterStepDst));
	OptionalFGSizer->Add(RunAfterStepDstTC, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL);


	wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);
	bSizer->Add(OptionalFGSizer, 0, wxEXPAND|wxALL, FromDIP(10));
	UserlistFGSizer->Add(bSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, FromDIP(5));


	wxFlexGridSizer* UserlistCmdFGSizer;
	UserlistCmdFGSizer = new wxFlexGridSizer(1, 3, 0, FromDIP(10));
	UserlistCmdFGSizer->AddGrowableCol(0);
	UserlistCmdFGSizer->SetFlexibleDirection(wxBOTH);
	UserlistCmdFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	UseOptionalSettingsCheckBox = new NFGAutoCheckBox(this, wxID_ANY, "Use optional settings", wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&UseOptionalSettings));
	UserlistCmdFGSizer->Add(UseOptionalSettingsCheckBox, 1, wxLEFT|wxALIGN_CENTER_VERTICAL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, FromDIP(10));
	
	UserlistSaveButton = new wxButton(this, ID_UserlistSave, "Save");
	UserlistSaveButton->SetDefault(); 
	UserlistCmdFGSizer->Add(UserlistSaveButton, 0, wxALL, 0);
	
	UserlistRevertButton = new wxButton(this, ID_UserlistRevert, "Revert");
	UserlistCmdFGSizer->Add(UserlistRevertButton, 0, wxALL, 0);
	
	UserlistFGSizer->Add(UserlistCmdFGSizer, 1, wxEXPAND|wxALL, FromDIP(5));
	
	SetSizer(UserlistFGSizer);
	
#ifndef MSW_MLNB_SIZE_FIX
	UserlistFGSizer->Show(1, UseOptionalSettings);
#endif

	Layout();
	UserlistFGSizer->Fit(this);
	
	if (parent)
		parent->Fit();
}

NFGUserlistMainPanel::~NFGUserlistMainPanel()
{
}

void NFGUserlistMainPanel::OnChange(wxCommandEvent& event)
{
	if (Doc != NULL) 
		Doc->Modify(true);

	event.Skip();
}

void NFGUserlistMainPanel::OnPageChange(wxBookCtrlEvent& event)
{
	if (Doc != NULL) 
		Doc->Modify(true);

	event.Skip();
}

void NFGUserlistMainPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	bool refit = (UseOptionalSettings != UserlistFGSizer->IsShown(1)) || (UseOptionalSettings && (Legacy == OptionalFGSizer->IsShown(3)));
	
	if (refit) 
		Freeze();
	
	if (UseOptionalSettings != UserlistFGSizer->IsShown(1)) 
		UserlistFGSizer->Show(1, UseOptionalSettings);
	
	if (UseOptionalSettings && (Legacy == OptionalFGSizer->IsShown(3))) {
		for (size_t i = 3; i < 18; i++)
			OptionalFGSizer->Show(i, !Legacy);
	}
	
	if (refit) {
		UserlistNotebook->InvalidateBestSize();	/// make the resize of the notebook possible
		UserlistFGSizer->Fit(this);
		wxWindow* parent = GetParent();
		if (parent)
			parent->Fit();
		
		Thaw();
	}
}

void NFGUserlistMainPanel::OnUpdateUISave(wxUpdateUIEvent& event) 
{
	event.Enable((Doc == NULL) || (Doc->IsModified()));
}

void NFGUserlistMainPanel::OnUserlistCommand(wxCommandEvent& event)
{
	int id = event.GetId();
	
	wxCommandEvent evt(wxEVT_MENU);
	evt.SetEventObject(this);
	
	if (id == ID_UserlistSave) 
		evt.SetId(wxID_SAVE);
		
	if (id == ID_UserlistRevert) 
		evt.SetId(wxID_REVERT);

	ProcessWindowEvent(evt);
}


NFGUserlistPanel* NFGUserlistMainPanel::FindPanelByType(unsigned char AssocType, bool Select)
{
	for(size_t i = 0; i < UserlistNotebook->GetPageCount(); i++) {
		NFGUserlistPanel *Panel = (NFGUserlistPanel *) (UserlistNotebook->GetPage(i));
		if (Panel)
			if (Panel->GetAssocType() == AssocType) {
				if (Select)
					UserlistNotebook->ChangeSelection(i);
					
				return Panel;
			}
	}

	return NULL;
}

bool NFGUserlistMainPanel::Load(UserlistParams* UParams, bool LegacyFormat)
{
	bool RetVal = true;
	
	if (UParams == NULL)
		return false;
	
	if (Legacy != LegacyFormat) {
		if (LegacyFormat && (UserlistNotebook->GetPageCount() == 7))
			UserlistNotebook->RemovePage(6);
		
		if ((!LegacyFormat) && (UserlistNotebook->GetPageCount() == 6))
			UserlistNotebook->AddPage(NutationPanel, "Nutation", false);
		
		UserlistNotebook->InvalidateBestSize();	/// make the resize of the notebook possible
	}
	
	for(size_t i = 0; i < UserlistNotebook->GetPageCount(); i++) {
		NFGUserlistPanel *Panel = (NFGUserlistPanel *) (UserlistNotebook->GetPage(i));
		if (Panel)
			Panel->SetLegacy(LegacyFormat);
	}
	
	Legacy = LegacyFormat;
	
	NFGUserlistPanel *Panel = FindPanelByType(UParams->AssocValueType, true);

	if ((Panel != NULL) && (Panel->Load(UParams, LegacyFormat))) {
		AssocValueType = UParams->AssocValueType;
		
		Destination = ((UParams->Destination != NULL)?(wxString(UParams->Destination)):(wxString("")));
		
		RunBeforeExpWrk = (((!Legacy) && (UParams->RunBeforeExpWrk != NULL))?(wxString(UParams->RunBeforeExpWrk)):(wxString("")));
		RunBeforeExpDst = (((!Legacy) && (UParams->RunBeforeExpDst != NULL))?(wxString(UParams->RunBeforeExpDst)):(wxString("")));
		RunAfterExpWrk = (((!Legacy) && (UParams->RunAfterExpWrk != NULL))?(wxString(UParams->RunAfterExpWrk)):(wxString("")));
		RunAfterExpDst = (((!Legacy) && (UParams->RunAfterExpDst != NULL))?(wxString(UParams->RunAfterExpDst)):(wxString("")));
		RunBeforeStepWrk = (((!Legacy) && (UParams->RunBeforeStepWrk != NULL))?(wxString(UParams->RunBeforeStepWrk)):(wxString("")));
		RunBeforeStepDst = (((!Legacy) && (UParams->RunBeforeStepDst != NULL))?(wxString(UParams->RunBeforeStepDst)):(wxString("")));
		RunAfterStepWrk = (((!Legacy) && (UParams->RunAfterStepWrk != NULL))?(wxString(UParams->RunAfterStepWrk)):(wxString("")));
		RunAfterStepDst = (((!Legacy) && (UParams->RunAfterStepDst != NULL))?(wxString(UParams->RunAfterStepDst)):(wxString("")));

		UseOptionalSettings = !Destination.IsEmpty() || 
			!RunBeforeExpWrk.IsEmpty() || !RunBeforeExpDst.IsEmpty() || !RunAfterExpWrk.IsEmpty() || !RunAfterExpDst.IsEmpty() || 
			!RunBeforeStepWrk.IsEmpty() || !RunBeforeStepDst.IsEmpty() || !RunAfterStepWrk.IsEmpty() || !RunAfterStepDst.IsEmpty();
			
	} else 
		RetVal = false;
		
	UpdateWindowUI();
	
	UserlistFGSizer->Fit(this);
	wxWindow* parent = GetParent();
	if (parent)
		parent->Fit();
	
	return (TransferDataToWindow() && RetVal);
}

bool NFGUserlistMainPanel::Store(UserlistParams* UParams, bool LegacyFormat)
{
	if (UParams == NULL)
		return false;
	
	int Selection = UserlistNotebook->GetSelection();
	NFGUserlistPanel *Panel = (NFGUserlistPanel *) (UserlistNotebook->GetPage(Selection));
	if (Panel == NULL)
		return false;

	if (!TransferDataFromWindow())
		return false;
	
	if (!Panel->Store(UParams, LegacyFormat)) 
		return false;
	
	if (!UseOptionalSettings || Destination.IsEmpty()) {
		free(UParams->Destination);
		UParams->Destination = NULL;
	} else 
	if ((UParams->Destination == NULL) || (std::strcmp(Destination.char_str(), UParams->Destination) != 0)) {
		free(UParams->Destination);
		UParams->Destination = strdup(Destination.char_str());
	}
	
#define StoreParam(param)	\
	if (Legacy || !UseOptionalSettings || param.IsEmpty()) { \
		free(UParams->param); \
		UParams->param = NULL; \
	} else \
	if ((UParams->param == NULL) || (std::strcmp(param.char_str(), UParams->param) != 0)) { \
		free(UParams->param); \
		UParams->param = strdup(param.char_str()); \
	}
	
	StoreParam(RunBeforeExpWrk);
	StoreParam(RunBeforeExpDst);
	StoreParam(RunAfterExpWrk);
	StoreParam(RunAfterExpDst);
	StoreParam(RunBeforeStepWrk);
	StoreParam(RunBeforeStepDst);
	StoreParam(RunAfterStepWrk);
	StoreParam(RunAfterStepDst);

#undef StoreParam
	
	return true;
}


bool NFGUserlistMainPanel::IsModified()
{
	bool modified = false;

	wxWindowList children = GetChildren();
	wxWindowListNode *node = children.GetFirst();

	while (node && !modified)
	{
		wxWindow *child = (wxWindow*) node->GetData();
		if (child) {
			if (child->IsKindOf(CLASSINFO(NFGAutoValidatedTextCtrl)))
				modified = modified || ((NFGAutoValidatedTextCtrl *) child)->IsModified();

			if (child->IsKindOf(CLASSINFO(NFGAutoCheckBox)))
				modified = modified || ((NFGAutoCheckBox *) child)->IsModified();
		}

		node = node->GetNext();
	}

	if (modified)
		return true;
	
	
	int Selection = UserlistNotebook->GetSelection();
	NFGUserlistPanel *Panel = (NFGUserlistPanel *) (UserlistNotebook->GetPage(Selection));
	if (Panel == NULL)
		return false;
	
	if (Panel->GetAssocType() != AssocValueType)
		return true;
	
	return Panel->IsModified();
}


void NFGUserlistMainPanel::Modify(bool mod)
{
	if (!mod) {
		wxWindowList children = GetChildren();
		wxWindowListNode *node = children.GetFirst();

		while (node) {
			wxWindow *child = (wxWindow*) node->GetData();
			if (child) {
				if (child->IsKindOf(CLASSINFO(NFGAutoValidatedTextCtrl)))
					((NFGAutoValidatedTextCtrl *) child)->DiscardEdits();

				if (child->IsKindOf(CLASSINFO(NFGAutoCheckBox)))
					((NFGAutoCheckBox *) child)->DiscardEdits();
			}

			node = node->GetNext();
		}
	}

	for(size_t i = 0; i < UserlistNotebook->GetPageCount(); i++) {
		NFGUserlistPanel *Panel = (NFGUserlistPanel *) (UserlistNotebook->GetPage(i));
		if (Panel)
			Panel->Modify(mod);
	}

	int Selection = UserlistNotebook->GetSelection();
	NFGUserlistPanel *Panel = (NFGUserlistPanel *) (UserlistNotebook->GetPage(Selection));

	if (mod)
		return;
	
	if (Panel)
		AssocValueType = Panel->GetAssocType();
	else
		AssocValueType = ASSOC_NOT_SET;
}
