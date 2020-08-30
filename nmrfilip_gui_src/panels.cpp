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

#include "panels.h"
#include <wx/artprov.h>
#include <wx/display.h>

#include "panelmain.h"
#include "panelproc.h"
#include "view.h"
#include "doc.h"
#include "nmrfilipgui.h"
#include "gui_ids.h"


IMPLEMENT_CLASS(NMRFilipGUIFrame, wxDocDIParentFrame)
BEGIN_EVENT_TABLE(NMRFilipGUIFrame, wxDocDIParentFrame)
	EVT_COMMAND(wxID_ANY, NFGEVT_CHILDFRAME_ACTIVATION, NMRFilipGUIFrame::OnChildFrameActivation)
	EVT_COMMAND(wxID_ANY, NFGEVT_CHILDFRAME_EXISTENCE, NMRFilipGUIFrame::OnChildFrameExistence)
	EVT_COMMAND(wxID_ANY, NFGEVT_CHILDFRAME_LABEL, NMRFilipGUIFrame::OnChildFrameLabel)
#ifdef MSW_FLICKER_FIX
	EVT_CLOSE(NMRFilipGUIFrame::OnCloseWindow)
#endif
#ifdef GTK_OXYGEN_FIX
	EVT_SIZE(NMRFilipGUIFrame::OnSize)
#endif
END_EVENT_TABLE()

#ifdef MSW_FLICKER_FIX
NMRFilipGUIFrame::NMRFilipGUIFrame(NFGDocManager* manager, wxFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDocDIParentFrame()
{
	/// this would be normally done in wxDocParentFrameAny::Create()
        m_docManager = manager;

	/// this would be normally done in wxMDIParentFrame::Create()
	if (!parent)
		wxTopLevelWindows.Append(this);

	SetName(wxFrameNameStr);
	SetWindowStyleFlag(style | wxFRAME_NO_WINDOW_MENU);

	if (parent)
		parent->AddChild(this);

	SetId((id == wxID_ANY)?(NewControlId()):(id));

	WXDWORD exflags;
	WXDWORD msflags = MSWGetCreateWindowFlags(&exflags) & (~(WS_VSCROLL | WS_HSCROLL));

	if (wxWindow::MSWCreate(wxApp::GetRegisteredClassName(wxT("wxMDIFrame"), -1, 0, (style & wxFULL_REPAINT_ON_RESIZE)?(wxApp::RegClass_Default):(wxApp::RegClass_ReturnNR)), title.t_str(), pos, size, msflags, exflags)) 
		SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

	m_isShown = false;
#else
NMRFilipGUIFrame::NMRFilipGUIFrame(NFGDocManager* manager, wxFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDocDIParentFrame(manager, parent, id, title, pos, size, style)
{
#endif
	DocMan = manager;
	WindowsListBox = NULL;
	
#ifdef GTK_OXYGEN_FIX
	RedoInitialLayout = false;
//	PostSizeEvent();
#endif
	
	SetIcons(wxArtProvider::GetIconBundle("NMRFilipGUI", wxART_FRAME_ICON));
	
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	
	wxFlexGridSizer* NMRFilipGUIFGSizer;
	NMRFilipGUIFGSizer = new wxFlexGridSizer(2, 1, 0, 0);
	NMRFilipGUIFGSizer->AddGrowableRow(0);
	NMRFilipGUIFGSizer->SetFlexibleDirection(wxBOTH);
	NMRFilipGUIFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	MainPanel = new NFGMainPanel(DocMan, this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	NMRFilipGUIFGSizer->Add(MainPanel, 1, wxFIXED_MINSIZE | wxEXPAND | wxALL, 0);
	
	ProcPanel = new NFGProcPanel(DocMan, this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	NMRFilipGUIFGSizer->Add(ProcPanel, 0, wxEXPAND | wxALL, 0);
	
#ifdef __NMRFilipGUI_MDI__
	wxFlexGridSizer* NMRFilipGUIMDIFGSizer;
	NMRFilipGUIMDIFGSizer = new wxFlexGridSizer(1, 2, 0, 0);
	NMRFilipGUIMDIFGSizer->AddGrowableCol(1);
	NMRFilipGUIMDIFGSizer->AddGrowableRow(0);
	NMRFilipGUIMDIFGSizer->SetFlexibleDirection(wxBOTH);
	NMRFilipGUIMDIFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	NMRFilipGUIMDIFGSizer->Add(NMRFilipGUIFGSizer, 0, wxEXPAND | wxALL, 0);
	if (GetClientWindow())
		NMRFilipGUIMDIFGSizer->Add(GetClientWindow(), 1, wxEXPAND | wxALL, 0);

	this->SetSizer(NMRFilipGUIMDIFGSizer);
	this->Layout();
	wxSize MinSize;
	MinSize = NMRFilipGUIMDIFGSizer->Fit(this);
	this->SetMinSize(MinSize);
	this->SetSize(MinSize.Scale(4, 1));
#else
	this->SetSizer(NMRFilipGUIFGSizer);
	this->Layout();
	wxSize MinSize;
	MinSize = NMRFilipGUIFGSizer->Fit(this);
	this->SetMinSize(MinSize);
	MinSize.SetHeight(-1);
	this->SetMaxSize(MinSize);
	
	/// In SDI mode, this application is intended to use the whole desktop. 
	/// (Use of multiple desktops while dedicating one for it is therefore recommended.)
	wxDisplay Display(wxDisplay::GetFromWindow(this));
	wxRect ClientRect = Display.GetClientArea();
	this->SetSize(wxSize(MinSize.GetWidth(), ClientRect.GetHeight()));
#endif
}

NMRFilipGUIFrame::~NMRFilipGUIFrame()
{
}

void NMRFilipGUIFrame::SetWindowsListBox(wxListBox *listbox)
{
	WindowsListBox = listbox;
}

void NMRFilipGUIFrame::OnChildFrameActivation(wxCommandEvent& event) 
{
	if (event.GetInt() && WindowsListBox) {	/// activation
		int max = WindowsListBox->GetCount();
		for (int n = 0; n < max; n++) {
			if (WindowsListBox->GetClientData(n) == ((void *) event.GetEventObject())) {
				WindowsListBox->SetSelection(n);
				break;
			}
				
		}
	}
}

void NMRFilipGUIFrame::OnChildFrameExistence(wxCommandEvent& event) 
{
	if (WindowsListBox) {
		if (event.GetInt()) {	/// creation
			int n = WindowsListBox->Append(event.GetString(), (void *) event.GetEventObject());
			WindowsListBox->SetSelection(n);	// expects activation after creation; maybe shouldn't be here
		} else {	/// destruction
			int max = WindowsListBox->GetCount();
			for (int n = 0; n < max; n++) {
				if (WindowsListBox->GetClientData(n) == ((void *) event.GetEventObject())) {
					WindowsListBox->Delete(n);
					break;
				}
					
			}
		}
	}
}

void NMRFilipGUIFrame::OnChildFrameLabel(wxCommandEvent& event) 
{
	if (WindowsListBox) {
		int max = WindowsListBox->GetCount();
		for (int n = 0; n < max; n++) {
			if (WindowsListBox->GetClientData(n) == ((void *) event.GetEventObject())) {
				WindowsListBox->SetString(n,  event.GetString());
				break;
			}
				
		}
	}
}

#ifdef MSW_FLICKER_FIX
/// this would be normally done in wxDocParentFrameAny::OnCloseWindow()
void NMRFilipGUIFrame::OnCloseWindow(wxCloseEvent& event)
{
	if (m_docManager && !(m_docManager->Clear(!event.CanVeto())))
		event.Veto();
	else
		event.Skip();
}
#endif

#ifdef GTK_OXYGEN_FIX
void NMRFilipGUIFrame::RepeatLayout()
{
	RedoInitialLayout = true;
	PostSizeEvent();
}
#endif

#ifdef GTK_OXYGEN_FIX
void NMRFilipGUIFrame::OnSize(wxSizeEvent& event)
{
	if (RedoInitialLayout) {
		InvalidateBestSize();
		wxWindowList list1 = GetChildren();	/// MainPanel, ProcPanel
		wxWindowList::iterator iter1;
		for (iter1 = list1.begin(); iter1 != list1.end(); ++iter1)
		{
			wxWindow *current1 = *iter1;
			current1->InvalidateBestSize();
			
			wxWindowList list2 = current1->GetChildren();	/// MainNotebook, ProcNotebook
			wxWindowList::iterator iter2;
			for (iter2 = list2.begin(); iter2 != list2.end(); ++iter2)
			{
				wxWindow *current2 = *iter2;
				current2->InvalidateBestSize();
				
				wxNotebook *nb = wxDynamicCast(current2, wxNotebook);
				if (nb) {
					int sel = nb->GetSelection();
					
					for (size_t i = 0; i < nb->GetPageCount(); i++) {
						nb->GetPage(i)->InvalidateBestSize();
						nb->ChangeSelection(i);
						nb->GetPage(i)->Layout();
					}
					
					nb->ChangeSelection((sel >= 0)?(sel):(0));
				}
				
				current2->Layout();
			}
			
			current1->Layout();
		}
		
		Layout();
		
		wxSize MinSize = GetBestSize();
		SetMinSize(MinSize);
		MinSize.SetHeight(-1);
		SetMaxSize(MinSize);
		wxDisplay Display(wxDisplay::GetFromWindow(this));
		wxRect ClientRect = Display.GetClientArea();
		SetSize(wxSize(MinSize.GetWidth(), ClientRect.GetHeight()));
		
		RedoInitialLayout = false;
	}

	event.Skip();
}
#endif
