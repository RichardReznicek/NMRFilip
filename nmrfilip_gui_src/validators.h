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

#ifndef __validators__
#define __validators__

#include "wx_pch.h"

#include <wx/spinctrl.h>

#include "cd.h"
#include "validators_cd.h"


class NFGDoubleValidator : public wxValidator
{
	DECLARE_EVENT_TABLE()
	private:
		/// Declare private copy constructor to prevent the compiler from defining it:
		NFGDoubleValidator& operator=(const NFGDoubleValidator&);
		
	protected:
		double *m_doubleValue;
	
	public:
		NFGDoubleValidator(double *val = 0);
		NFGDoubleValidator(const NFGDoubleValidator& val);
		virtual ~NFGDoubleValidator() {}
		
		virtual wxObject *Clone() const { return new NFGDoubleValidator(*this); }
		bool Copy(const NFGDoubleValidator& val);
		
		virtual bool Validate(wxWindow *parent);
		virtual bool TransferToWindow();
		virtual bool TransferFromWindow();
		
		/// Filter keystrokes
		void OnChar(wxKeyEvent& event);
};


#define ANY_NUMBER 0
#define POSITIVE_NUMBER	1
#define NON_NEGATIVE_NUMBER	2

class NFGLongValidator : public wxValidator 
{
	DECLARE_EVENT_TABLE()
	private:
		/// Declare private copy constructor to prevent the compiler from defining it:
		NFGLongValidator& operator=(const NFGLongValidator&);
		
	protected:
		long *m_longValue;
		unsigned char m_type;
	
	public:
		NFGLongValidator(long *val = 0, unsigned char type = ANY_NUMBER);
		NFGLongValidator(const NFGLongValidator& val);
		virtual ~NFGLongValidator() {}
		
		virtual wxObject *Clone() const { return new NFGLongValidator(*this); }
		bool Copy(const NFGLongValidator& val);

		virtual bool Validate(wxWindow *parent);
		virtual bool TransferToWindow();
		virtual bool TransferFromWindow();
		
		/// Filter keystrokes
		void OnChar(wxKeyEvent& event);	
};


class NFGTextValidator : public wxTextValidator 
{
	private:
		/// Declare private copy constructor to prevent the compiler from defining it:
		NFGLongValidator& operator=(const NFGLongValidator&);
		
	public:
		NFGTextValidator(long style = wxFILTER_NONE, wxString *val = NULL) : wxTextValidator(style, val) {}
		NFGTextValidator(const NFGTextValidator& val) : wxTextValidator(val) {}
		virtual ~NFGTextValidator() {}
		
		virtual wxObject *Clone() const { return new NFGTextValidator(*this); }
		bool Copy(const NFGTextValidator& val) { return wxTextValidator::Copy(val); }

		virtual bool TransferToWindow();
};


wxDECLARE_EVENT(NFGEVT_ADJUST_VALUES, wxCommandEvent);

class NFGAutoValidatedTextCtrl : public wxTextCtrl 
{
	DECLARE_DYNAMIC_CLASS(NFGAutoValidatedTextCtrl)
	
	DECLARE_EVENT_TABLE();
	private:
		void OnKillFocus(wxFocusEvent& event);
		
		bool m_AutoExchange;
		bool m_AutoRevert;
		bool m_SendEvent;
		bool *m_PostponeChanges;
		bool m_PendingChange;
		wxString m_ChangeToApply;
		
	public:
		NFGAutoValidatedTextCtrl();
		NFGAutoValidatedTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxTextCtrlNameStr, bool AutoExchange = true, bool AutoRevert = false, bool SendEvent = false, bool *PostponeChanges = NULL);
		~NFGAutoValidatedTextCtrl() {}
		
		virtual void SetValue(const wxString& value);
		virtual void ChangeValue(const wxString& value);
};


class NFGValidatedStaticText : public wxStaticText 
{
	DECLARE_DYNAMIC_CLASS(NFGValidatedStaticText)

	public:
		NFGValidatedStaticText();
		NFGValidatedStaticText(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxStaticTextNameStr);
		~NFGValidatedStaticText() {}
		
		virtual void SetLabel(const wxString& label);
};


class NFGAutoRadioButton : public wxRadioButton 
{
	DECLARE_DYNAMIC_CLASS(NFGAutoRadioButton)

	DECLARE_EVENT_TABLE();
	private:
		void OnSelect(wxCommandEvent& event);
		bool m_AutoExchange;
		bool m_modified;
		
	public:
		NFGAutoRadioButton();
		NFGAutoRadioButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, bool AutoExchange = true);
		~NFGAutoRadioButton() {}
		
		void SetValue(const bool value);
		bool IsModified();
		void MarkDirty();
		void DiscardEdits();
};


class NFGAutoCheckBox : public wxCheckBox 
{
	DECLARE_DYNAMIC_CLASS(NFGAutoCheckBox)

	DECLARE_EVENT_TABLE();
	private:
		void OnSelect(wxCommandEvent& event);
		bool m_AutoExchange;
		bool m_modified;
	
	public:
		NFGAutoCheckBox();
		NFGAutoCheckBox(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, bool AutoExchange = true);
		~NFGAutoCheckBox() {}
		
		void SetValue(const bool value);
		bool IsModified();
		void MarkDirty();
		void DiscardEdits();
};


class NFGAutoChoice : public wxChoice 
{
	DECLARE_DYNAMIC_CLASS(NFGAutoChoice)

	DECLARE_EVENT_TABLE();
	private:
		void OnSelect(wxCommandEvent& event);
		bool m_AutoExchange;
		bool m_modified;
		
	public:
		NFGAutoChoice();
		NFGAutoChoice(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n=0, const wxString choices[]=NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString &name=wxChoiceNameStr, bool AutoExchange = true);
		~NFGAutoChoice() {}
		
		void SetSelection(int n);
		bool IsModified();
		void MarkDirty();
		void DiscardEdits();
};

#endif
