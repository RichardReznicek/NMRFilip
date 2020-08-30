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

#include "view.h"

#include "doc.h"
#include "plotwin.h"
#include "userlist.h"
#include "infopanel.h"
#include "nmrfilipgui.h"


IMPLEMENT_DYNAMIC_CLASS(NFGTextView, wxView)

NFGTextView::NFGTextView() : wxView() 
{
	frame = NULL; 
	textsw = NULL; 
}

bool NFGTextView::OnCreate(wxDocument *doc, long flags)
{
	if (!wxView::OnCreate(doc, flags))
		return false;

	frame = new NFGDocDIChildFrame(doc, this, wxDynamicCast(wxGetApp().GetTopWindow(), wxDIParentFrame), wxID_ANY, "text", 
				wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);

	
	int width, height;
	frame->GetClientSize(&width, &height);
	textsw = new NFGTextWindow(doc, frame, wxPoint(0, 0), wxSize(width, height), wxTE_MULTILINE | wxTE_NOHIDESEL);
	
	frame->Show(true);
	
	return true;
}

void NFGTextView::OnDraw(wxDC *WXUNUSED(dc) )
{
}

bool NFGTextView::OnClose(bool deleteWindow)
{
	if (!wxView::OnClose(deleteWindow))
		return false;
	
	Activate(false);
	
	if (deleteWindow) {
		GetFrame()->Destroy();
		SetFrame(NULL);
	}
	
	return true;
}


IMPLEMENT_DYNAMIC_CLASS(NFGSerView, wxView)

NFGSerView::NFGSerView() : wxView()
{
	frame = NULL;
	GraphWindow = NULL;
}

bool NFGSerView::OnCreate(wxDocument *doc, long flags)
{
	if (!wxView::OnCreate(doc, flags))
		return false;

	frame = new NFGDocDIChildFrame(doc, this, wxDynamicCast(::wxGetApp().GetTopWindow(), wxDIParentFrame), wxID_ANY, "ser", 
				wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);
	
	int width, height;
	frame->GetClientSize(&width, &height);

	GraphWindow = new NFGGraphWindow(frame, -1, wxPoint(0, 0), wxSize(width, height));

	frame->Show(true);
	
	return true;
}

void NFGSerView::OnDraw(wxDC *WXUNUSED(dc) )
{
}

bool NFGSerView::OnClose(bool deleteWindow)
{
	if (!wxView::OnClose(deleteWindow))
		return false;
	
	if (GraphWindow)
		GraphWindow->SelectGraph(NULL);
	
	Activate(false);

	if (deleteWindow) {
		GetFrame()->Destroy();
		SetFrame(NULL);
	}
	
	return true;
}


IMPLEMENT_DYNAMIC_CLASS(NFGInfoView, wxView)

NFGInfoView::NFGInfoView() : wxView()
{
	frame = NULL;
	InfoPanel = NULL;
}

/// Note: doc has to be a pointer to a valid wxDocument otherwise a segfault will occur during deletition of the object (wxW 2.8.8)
bool NFGInfoView::OnCreate(wxDocument *doc, long flags)
{
	if (!wxView::OnCreate(doc, flags))
		return false;

	NFGSerDocument* SerDoc = wxDynamicCast(doc, NFGSerDocument);

	if (!SerDoc)
		return false;
	
	AcquParams* info = SerDoc->AcquInfoQuery();
	wxString path = SerDoc->PathStringQuery();
	if (!info)
		return false;
	
	frame = new NFGInfoDIChildFrame(doc, this, wxDynamicCast(::wxGetApp().GetTopWindow(), wxDIParentFrame), wxID_ANY, "info", 
				wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE);

	if (!frame)
		return false;

	InfoPanel = new NFGInfoPanel(info, path, frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	if (!InfoPanel)
		return false;

	
	wxBoxSizer* BSizer;
	BSizer = new wxBoxSizer(wxVERTICAL);
	BSizer->Add(InfoPanel, 1, wxEXPAND | wxALL, 0);
	frame->SetSizer(BSizer);

	InfoPanel->LoadAcquInfo(info, path);

	frame->Show(true);
	
	return true;
}

void NFGInfoView::OnDraw(wxDC *WXUNUSED(dc) )
{
}

bool NFGInfoView::OnClose(bool deleteWindow)
{
	if (!wxView::OnClose(deleteWindow))
		return false;
	
	Activate(false);

	if (deleteWindow) {
		GetFrame()->Destroy();
		SetFrame(NULL);
	}
	
	return true;
}

void NFGInfoView::OnChangeFilename()
{
	if (m_viewFrame == NULL) 
		return;

	if (m_viewDocument == NULL) 
		return;

	wxString label = m_viewDocument->GetTitle();
	m_viewFrame->SetLabel(label + " - info");
}



IMPLEMENT_DYNAMIC_CLASS(NFGUserlistView, wxView)

NFGUserlistView::NFGUserlistView() : wxView() 
{
	frame = NULL;
	UserlistMainPanel = NULL;
}

bool NFGUserlistView::OnCreate(wxDocument *doc, long flags)
{
	if (!wxView::OnCreate(doc, flags))
		return false;

	frame = new NFGUserlistDIChildFrame(doc, this, wxDynamicCast(::wxGetApp().GetTopWindow(), wxDIParentFrame), wxID_ANY, "userlist", 
				wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE);

	if (!frame)
		return false;

	UserlistMainPanel = new NFGUserlistMainPanel(doc, frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	if (!UserlistMainPanel)
		return false;

	wxBoxSizer* UserlistBSizer;
	UserlistBSizer = new wxBoxSizer(wxVERTICAL);
	UserlistBSizer->Add(UserlistMainPanel, 1, wxEXPAND | wxALL, 0);
	frame->SetSizer(UserlistBSizer);

	frame->Show(true);
	
	return true;
}

void NFGUserlistView::OnDraw(wxDC *WXUNUSED(dc) )
{
}

bool NFGUserlistView::OnClose(bool deleteWindow)
{
	if (!wxView::OnClose(deleteWindow))
		return false;
	
	Activate(false);

	if (deleteWindow) {
		GetFrame()->Destroy();
		SetFrame(NULL);
	}
	
	return true;
}



BEGIN_EVENT_TABLE(NFGTextWindow, wxTextCtrl)
	EVT_TEXT(wxID_ANY, NFGTextWindow::OnChange)
	EVT_CONTEXT_MENU(NFGTextWindow::OnContextMenu)
	EVT_MENU(wxID_UNDO, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_REDO, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_CUT, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_COPY, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_PASTE, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_SELECTALL, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_SAVE, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_REVERT, NFGTextWindow::OnContextMenuCommand)
	EVT_MENU(wxID_CLOSE, NFGTextWindow::OnContextMenuCommand)
END_EVENT_TABLE()

NFGTextWindow::NFGTextWindow(wxDocument *document, wxDocDIChildFrame *frame, const wxPoint& pos, const wxSize& size, long style) : wxTextCtrl(frame, wxID_ANY, wxEmptyString, pos, size, style)
{
	Doc = document;
	
	ContextMenu = new wxMenu;

	ContextMenu->Append(wxID_UNDO, "Undo\tCtrl+Z");
	ContextMenu->Append(wxID_REDO, "Redo\tCtrl+Y");
	ContextMenu->AppendSeparator();
	ContextMenu->Append(wxID_CUT, "Cut\tCtrl+X");
	ContextMenu->Append(wxID_COPY, "Copy\tCtrl+C");
	ContextMenu->Append(wxID_PASTE, "Paste\tCtrl+V");
	ContextMenu->AppendSeparator();
	ContextMenu->Append(wxID_SELECTALL, "Select All\tCtrl+A");
	ContextMenu->AppendSeparator();
	ContextMenu->Append(wxID_SAVE, "Save\tCtrl+S");
	ContextMenu->Append(wxID_REVERT, "Revert\tCtrl+R");
	ContextMenu->Append(wxID_CLOSE, "Close\tCtrl+W");

	wxAcceleratorEntry entries[9];

	entries[0].Set(wxACCEL_CTRL, (int) 'Z', wxID_UNDO);
	entries[1].Set(wxACCEL_CTRL, (int) 'Y', wxID_REDO);

	entries[2].Set(wxACCEL_CTRL, (int) 'X', wxID_CUT);
	entries[3].Set(wxACCEL_CTRL, (int) 'C', wxID_COPY);
	entries[4].Set(wxACCEL_CTRL, (int) 'V', wxID_PASTE);

	entries[5].Set(wxACCEL_CTRL, (int) 'A', wxID_SELECTALL);

	entries[6].Set(wxACCEL_CTRL, (int) 'S', wxID_SAVE);
	entries[7].Set(wxACCEL_CTRL, (int) 'R', wxID_REVERT);
	entries[8].Set(wxACCEL_CTRL, (int) 'W', wxID_CLOSE);

	wxAcceleratorTable accel(9, entries);
	SetAcceleratorTable(accel);
}

NFGTextWindow::~NFGTextWindow()
{
	delete ContextMenu;
}


void NFGTextWindow::OnContextMenu(wxContextMenuEvent& WXUNUSED(event))
{
	ContextMenu->Enable(wxID_UNDO, CanUndo());
	ContextMenu->Enable(wxID_REDO, CanRedo());
	PopupMenu(ContextMenu);
}

void NFGTextWindow::OnContextMenuCommand(wxCommandEvent& event)
{
	switch(event.GetId()) {
		case wxID_UNDO:
		case wxID_REDO:
			event.Skip();
			break;

		case wxID_CUT:
		case wxID_COPY:
		case wxID_PASTE:
			event.Skip();
			break;

		case wxID_SELECTALL:
			SetSelection(-1, -1);
			break;

		case wxID_SAVE:
		case wxID_REVERT:
			if (Doc && Doc->GetDocumentManager())
				Doc->GetDocumentManager()->ProcessEvent(event);
			break;

		case wxID_CLOSE:
			if (Doc && Doc->GetDocumentManager())
				Doc->GetDocumentManager()->AddPendingEvent(event);
			break;
		
		default:
			;
	}
}

void NFGTextWindow::OnChange(wxCommandEvent& event)
{
	if (Doc != NULL) 
		Doc->Modify(true);

	event.Skip();
}

/// suppresses automatic selection of all text on other focus events than a mouse click
void NFGTextWindow::SetFocusFromKbd()
{
	// SetFocus();	/// direct approach
	wxWindowBase::SetFocusFromKbd();
}

