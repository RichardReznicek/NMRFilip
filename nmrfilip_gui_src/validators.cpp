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
 
#include "validators.h"


BEGIN_EVENT_TABLE(NFGDoubleValidator, wxValidator)
	EVT_CHAR(NFGDoubleValidator::OnChar)
END_EVENT_TABLE()

static bool IsNumeric(const wxString& val);

NFGDoubleValidator::NFGDoubleValidator(double *val) : wxValidator()
{
	m_doubleValue = val;
}

NFGDoubleValidator::NFGDoubleValidator(const NFGDoubleValidator& val) : wxValidator()
{
	Copy(val);
}

bool NFGDoubleValidator::Copy(const NFGDoubleValidator& val)
{
	wxValidator::Copy(val);
	
	m_doubleValue = val.m_doubleValue;
	
	return true;
}

bool NFGDoubleValidator::Validate(wxWindow *parent)
{
	if (m_validatorWindow == NULL)
		return false;
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)))	/// nothing to be done
		return true;
	if (!m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
		return false;
		
	wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
	
	if (!control->IsEnabled())
		return true;
	
	wxString val(control->GetValue());
	val.Replace(",", ".");	/// accept both '.' and ',' decimal point
	
	double dvalue = 0.0;
	if ((!val.ToCDouble(&dvalue)) || (!IsNumeric(val))) {
		wxBell();
		return false;
	}
	
	return true;
}

bool NFGDoubleValidator::TransferToWindow(void)
{
	if (m_validatorWindow == NULL)
		return false;
	
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {
	
		if (m_doubleValue != NULL) {
			wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
			wxString str;
			str.Printf("%.15g", *m_doubleValue);
			control->ChangeValue(str);
		}
	
		return true;
	}
	
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText))) {
	
		if (m_doubleValue != NULL) {
			wxStaticText *control = (wxStaticText *) m_validatorWindow;
			wxString str;
			str.Printf("%.15g", *m_doubleValue);
			control->SetLabel(str);
		}
	
		return true;
	}
	
	return false;
}

bool NFGDoubleValidator::TransferFromWindow(void)
{
	if (m_validatorWindow == NULL)
		return false;
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)))	/// nothing to be done
		return true;
	if (!m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
		return false;
	
	if (m_doubleValue != NULL) {
		wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
		wxString str = control->GetValue();
		str.Replace(",", ".");	/// accept both '.' and ',' decimal point
		str.ToCDouble(m_doubleValue);
	}
	
	return true;
}

inline bool IsDigit(char c) {
	return ((c <= '9') && (c >= '0'));
}

void NFGDoubleValidator::OnChar(wxKeyEvent& event)
{

	if (m_validatorWindow == NULL)
		return;
	if (!m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
		return;
	
	int keyCode = event.GetKeyCode();
	
	/// don't filter special keys and Delete
	if (!(keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode > WXK_START) && 
	     !IsDigit(keyCode) && keyCode != '.' && keyCode != ',' && keyCode != '-' && keyCode != '+' && keyCode != 'e' && keyCode != 'E')
	{
		if (!wxValidator::IsSilent())
			wxBell();
		/// discard the event
		return;
	}
	
	event.Skip();
}


static bool IsNumeric(const wxString& val)
{
	for (int i = 0; i < (int) val.length(); i++) {
		/// accept both '.' and ',' decimal point
		if ((!IsDigit(val[i])) && (val[i] != '.') && (val[i] != ',') && (val[i] != 'e') && (val[i] != 'E') && (val[i] != '+') && (val[i] != '-'))
		    return false;
	}
	
	return true;
}


static bool IsSignedInteger(const wxString& val)
{
	for (int i = 0; i < (int) val.length(); i++) {
		if ((!IsDigit(val[i])) && (val[i] != '-'))
		    return false;
	}
	
	return true;
}

static bool IsUnsignedInteger(const wxString& val)
{
	for (int i = 0; i < (int) val.length(); i++) {
		if (!IsDigit(val[i]))
		    return false;
	}
	
	return true;
}



BEGIN_EVENT_TABLE(NFGLongValidator, wxValidator)
	EVT_CHAR(NFGLongValidator::OnChar)
END_EVENT_TABLE()

NFGLongValidator::NFGLongValidator(long *val, unsigned char type) : wxValidator()
{
	m_longValue = val;
	m_type = type;
}

NFGLongValidator::NFGLongValidator(const NFGLongValidator& val) : wxValidator()
{
	Copy(val);
}

bool NFGLongValidator::Copy(const NFGLongValidator& val)
{
	wxValidator::Copy(val);
	
	m_longValue = val.m_longValue;
	m_type = val.m_type;
	
	return true;
}

bool NFGLongValidator::Validate(wxWindow *parent)
{
	if (m_validatorWindow == NULL)
		return false;
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)))	/// nothing to be done
		return true;
	
	wxString val = wxEmptyString;
	
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {
		wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
		
		if (!control->IsEnabled())
			return true;
		
		val = control->GetValue();

		long lvalue = 0;
		
		switch (m_type) {
			case NON_NEGATIVE_NUMBER:
				if ((!val.ToLong(&lvalue)) || (!IsUnsignedInteger(val))) {
					wxBell();
					return false;
				}
				
				if (lvalue < 0) {
					wxBell();
					return false;
				}
				
				break;
			
			case POSITIVE_NUMBER:
				if ((!val.ToLong(&lvalue)) || (!IsUnsignedInteger(val))) {
					wxBell();
					return false;
				}
				
				if (lvalue <= 0) {
					wxBell();
					return false;
				}
				
				break;
			
			case ANY_NUMBER:
			default:
				if ((!val.ToLong(&lvalue)) || (!IsSignedInteger(val))) {
					wxBell();
					return false;
				}
		}
		
		return true;
	} else
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl))) {
		wxSpinCtrl *control = (wxSpinCtrl *) m_validatorWindow;
		
		if (!control->IsEnabled())
			return true;
		
		int value = control->GetValue();
	
		switch (m_type) {
			case NON_NEGATIVE_NUMBER:
				if (value < 0) {
					wxBell();
					return false;
				}
				
				break;
			
			case POSITIVE_NUMBER:
				if (value <= 0) {
					wxBell();
					return false;
				}
				
				break;
			
			case ANY_NUMBER:
			default:
				;
		}
		
		return true;
	} else
		return false;

	return true;
}

bool NFGLongValidator::TransferToWindow(void)
{
	if (m_validatorWindow == NULL)
		return false;
	
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {
	
		if (m_longValue != NULL) {
			wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
			wxString str;
			str.Printf("%ld", *m_longValue);
			control->ChangeValue(str);
		}

		return true;
	}
	
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText))) {
		
		if (m_longValue != NULL) {
			wxStaticText *control = (wxStaticText *) m_validatorWindow;
			wxString str;
			str.Printf("%ld", *m_longValue);
			control->SetLabel(str);
		}

		return true;
	}
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl))) {
	
		if (m_longValue != NULL) {
			wxSpinCtrl *control = (wxSpinCtrl *) m_validatorWindow;
			control->SetValue(*m_longValue);
		}

		return true;
	}
	
	return false;
}

bool NFGLongValidator::TransferFromWindow(void)
{
	if (m_validatorWindow == NULL)
		return false;
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)))	/// nothing to be done
		return true;
	
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {
		if (m_longValue != NULL) {
			wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
			wxString str = control->GetValue();
			str.ToLong(m_longValue);
		}
	} else
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl))) {
		if (m_longValue != NULL) {
			wxSpinCtrl *control = (wxSpinCtrl *) m_validatorWindow;
			*m_longValue = control->GetValue();
		}
	} else
		return false;
	
	return true;
}

void NFGLongValidator::OnChar(wxKeyEvent& event)
{

	if (m_validatorWindow == NULL)
		return;
	if (!m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) && !m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl)))
		return;
	
	int keyCode = event.GetKeyCode();
	
	/// don't filter special keys and Delete
	if ( (!(keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode > WXK_START) && 
	     !IsDigit(keyCode) && keyCode != '+' && (keyCode != '-')) || ((m_type != ANY_NUMBER) && (keyCode == '-')) )
	{
		if (!wxValidator::IsSilent())
			wxBell();
		/// discard the event
		return;
	}
	
	event.Skip();
}



bool NFGTextValidator::TransferToWindow()
{
	wxTextEntry *control = GetTextEntry();
	if (control != NULL) {
		if (m_stringValue != NULL) 
			control->ChangeValue(*m_stringValue);	/// avoid sending event
	} else
		return false;

	return true;
}


IMPLEMENT_DYNAMIC_CLASS(NFGAutoValidatedTextCtrl, wxTextCtrl)

BEGIN_EVENT_TABLE(NFGAutoValidatedTextCtrl, wxTextCtrl)
	EVT_KILL_FOCUS(NFGAutoValidatedTextCtrl::OnKillFocus)
END_EVENT_TABLE()

NFGAutoValidatedTextCtrl::NFGAutoValidatedTextCtrl() : wxTextCtrl()
{
	m_AutoExchange = true;
	m_AutoRevert = false;
	m_SendEvent = false;
	m_PostponeChanges = NULL;
	m_PendingChange = false;
	m_ChangeToApply = wxEmptyString;
}

NFGAutoValidatedTextCtrl::NFGAutoValidatedTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name, bool AutoExchange, bool AutoRevert, bool SendEvent, bool *PostponeChanges) : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
{
	m_AutoExchange = AutoExchange;
	m_AutoRevert = AutoRevert;
	m_SendEvent = SendEvent;
	m_PostponeChanges = PostponeChanges;
	m_PendingChange = false;
	m_ChangeToApply = wxEmptyString;
}

wxDEFINE_EVENT(NFGEVT_ADJUST_VALUES, wxCommandEvent);

void NFGAutoValidatedTextCtrl::OnKillFocus(wxFocusEvent& event)
{
	wxValidator *validator = GetValidator();
	wxWindow *parent = GetParent();

	if (m_SendEvent && (parent != NULL)) {
		wxCommandEvent AdjustValuesEvt(NFGEVT_ADJUST_VALUES);
		AdjustValuesEvt.SetId(GetId());
		AdjustValuesEvt.SetEventObject(this);
		parent->ProcessWindowEvent(AdjustValuesEvt);
	}
	
	if (validator) {
		if (m_AutoRevert || !(validator->Validate(parent)))
			validator->TransferToWindow();
		else if (m_AutoExchange) {
 			validator->TransferFromWindow();
 			if (parent != NULL)
 				parent->TransferDataToWindow();
		}
	}
	
	if (m_PendingChange) {
		bool mod = IsModified();
		wxTextCtrl::ChangeValue(m_ChangeToApply);
		m_PendingChange = false;
		m_ChangeToApply = wxEmptyString;
		/// NOTE: This is a non-standard behaviour required by the AutoExchange feature.
		if (mod)
			MarkDirty();
	}
	
	event.Skip();
}

void NFGAutoValidatedTextCtrl::SetValue(const wxString& value)
{
	m_PendingChange = false;
	m_ChangeToApply = wxEmptyString;

	bool mod = IsModified();
	wxTextCtrl::SetValue(value);
	/// NOTE: This is a non-standard behaviour required by the AutoExchange feature.
	if (mod)
		MarkDirty();
}

void NFGAutoValidatedTextCtrl::ChangeValue(const wxString& value)
{
	if ((m_PostponeChanges != NULL) && (*m_PostponeChanges) && HasFocus()) {
		m_ChangeToApply = value;
		m_PendingChange = true;
	} else {
		bool mod = IsModified();
		m_PendingChange = false;
		m_ChangeToApply = wxEmptyString;
		wxTextCtrl::ChangeValue(value);
		/// NOTE: This is a non-standard behaviour required by the AutoExchange feature.
		if (mod)
			MarkDirty();
	}
}



IMPLEMENT_DYNAMIC_CLASS(NFGValidatedStaticText, wxStaticText)

NFGValidatedStaticText::NFGValidatedStaticText() : wxStaticText()
{
}

NFGValidatedStaticText::NFGValidatedStaticText(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxStaticText(parent, id, label, pos, size, style, name)
{
	SetValidator(validator);
}

void NFGValidatedStaticText::SetLabel(const wxString& label)
{
	/// avoid flicker
	if ((label.Len() > 1024) || (label != GetLabel())) 
		wxStaticText::SetLabel(label);
}



IMPLEMENT_DYNAMIC_CLASS(NFGAutoRadioButton, wxRadioButton)

BEGIN_EVENT_TABLE(NFGAutoRadioButton, wxRadioButton)
	EVT_RADIOBUTTON(wxID_ANY, NFGAutoRadioButton::OnSelect)
END_EVENT_TABLE()

NFGAutoRadioButton::NFGAutoRadioButton() : wxRadioButton()
{
	m_AutoExchange = true;
	m_modified = false;
}

NFGAutoRadioButton::NFGAutoRadioButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, bool AutoExchange) : wxRadioButton(parent, id, label, pos, size, style, validator)
{
	m_AutoExchange = AutoExchange;
	m_modified = false;
}

void NFGAutoRadioButton::SetValue(const bool value)
{
	wxRadioButton::SetValue(value);
}

bool NFGAutoRadioButton::IsModified()
{
	return m_modified;
}

void NFGAutoRadioButton::MarkDirty()
{
	m_modified = true;
}

void NFGAutoRadioButton::DiscardEdits()
{
	m_modified = false;
}

void NFGAutoRadioButton::OnSelect(wxCommandEvent& event)
{
	m_modified = true;
	
	wxWindow *parent = GetParent();

	if (m_AutoExchange && (parent != NULL)) {
 		parent->TransferDataFromWindow();	/// the easiest way (although not optimal) to store the values of all the RadioButtons from the same group
 		parent->TransferDataToWindow();
 	}
	
	event.Skip();
}


IMPLEMENT_DYNAMIC_CLASS(NFGAutoCheckBox, wxCheckBox)

BEGIN_EVENT_TABLE(NFGAutoCheckBox, wxCheckBox)
	EVT_CHECKBOX(wxID_ANY, NFGAutoCheckBox::OnSelect)
END_EVENT_TABLE()

NFGAutoCheckBox::NFGAutoCheckBox() : wxCheckBox()
{
	m_AutoExchange = true;
	m_modified = false;
}

NFGAutoCheckBox::NFGAutoCheckBox(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, bool AutoExchange) : wxCheckBox(parent, id, label, pos, size, style, validator)
{
	m_AutoExchange = AutoExchange;
	m_modified = false;
}

void NFGAutoCheckBox::SetValue(const bool value)
{
	wxCheckBox::SetValue(value);
}

bool NFGAutoCheckBox::IsModified()
{
	return m_modified;
}

void NFGAutoCheckBox::MarkDirty()
{
	m_modified = true;
}

void NFGAutoCheckBox::DiscardEdits()
{
	m_modified = false;
}

void NFGAutoCheckBox::OnSelect(wxCommandEvent& event)
{
	m_modified = true;
	
	wxWindow *parent = GetParent();
	wxValidator *validator = GetValidator();

	if (m_AutoExchange && (validator != NULL)) {
		validator->TransferFromWindow();
		if (parent != NULL)
			parent->TransferDataToWindow();
	}

	event.Skip();
}


IMPLEMENT_DYNAMIC_CLASS(NFGAutoChoice, wxChoice)

BEGIN_EVENT_TABLE(NFGAutoChoice, wxChoice)
	EVT_CHOICE(wxID_ANY, NFGAutoChoice::OnSelect)
END_EVENT_TABLE()

NFGAutoChoice::NFGAutoChoice() : wxChoice()
{
	m_AutoExchange = true;
	m_modified = false;
}

NFGAutoChoice::NFGAutoChoice(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, int n, const wxString choices[], long style, const wxValidator& validator, const wxString &name, bool AutoExchange) : wxChoice (parent, id, pos, size, n, choices, style, validator, name)
{
	m_AutoExchange = AutoExchange;
	m_modified = false;
}

void NFGAutoChoice::SetSelection(int n)
{
	wxChoice::SetSelection(n);
}

bool NFGAutoChoice::IsModified()
{
	return m_modified;
}

void NFGAutoChoice::MarkDirty()
{
	m_modified = true;
}

void NFGAutoChoice::DiscardEdits()
{
	m_modified = false;
}

void NFGAutoChoice::OnSelect(wxCommandEvent& event)
{
	m_modified = true;
	
	wxWindow *parent = GetParent();
	wxValidator *validator = GetValidator();

	if (m_AutoExchange && (validator != NULL)) {
		validator->TransferFromWindow();
		if (parent != NULL)
			parent->TransferDataToWindow();
	}

	event.Skip();
}
