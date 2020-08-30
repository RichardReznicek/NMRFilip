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

#include "nmrfilipgui.h"
#include <wx/clipbrd.h>
#include <wx/filename.h>

#include "nmrdata.h"
#include "doc.h"
#include "view.h"
#include "panels.h"
#include "artpr.h"


IMPLEMENT_APP(NMRFilipGUIApp)

NMRFilipGUIApp::NMRFilipGUIApp()
{
	m_docManager = NULL;
	NMRFilipCoreDll = NULL;
	frame = NULL;
	
	/// pointers to functions in nmrfilip dll
	NFGNMRData::InitNMRData = NULL;
	NFGNMRData::CheckNMRData = NULL;
	NFGNMRData::FreeNMRData = NULL;
	NFGNMRData::RefreshNMRData = NULL;
	NFGNMRData::ReloadNMRData = NULL;
	
	NFGNMRData::CheckProcParam = NULL;
	NFGNMRData::GetProcParam = NULL;
	NFGNMRData::SetProcParam = NULL;
	NFGNMRData::ImportProcParams = NULL;
	
	NFGNMRData::DataToText = NULL;
	
	NFGNMRData::ReadUserlist = NULL;
	NFGNMRData::WriteUserlist = NULL;

	NFGNMRData::CleanupOnExit = NULL;
}

NMRFilipGUIApp::~NMRFilipGUIApp()
{
	/// just in case of OnInit() failure
	delete NMRFilipCoreDll;
}

bool NMRFilipGUIApp::OnInit()
{
#ifdef __WXGTK__
	setlocale(LC_NUMERIC, "C");
#endif

	if (!wxApp::OnInit()) 
		return false;

	/// load nmrfilip dll
	NMRFilipCoreDll = new wxDynamicLibrary("libnmrfilip");
	if ((NMRFilipCoreDll == NULL) || (!(NMRFilipCoreDll->IsLoaded()))) {	/// critical error
		wxLogError("Unable to load the NMRFilip core library.");
		return false;
	}
	
	/// initialization of pointers to functions in nmrfilip dll
	NFGNMRData::InitNMRData = (InitNMRDataFunc) NMRFilipCoreDll->GetSymbol("InitNMRData");
	NFGNMRData::CheckNMRData = (CheckNMRDataFunc) NMRFilipCoreDll->GetSymbol("CheckNMRData");
	NFGNMRData::FreeNMRData = (FreeNMRDataFunc) NMRFilipCoreDll->GetSymbol("FreeNMRData");
	NFGNMRData::RefreshNMRData = (RefreshNMRDataFunc) NMRFilipCoreDll->GetSymbol("RefreshNMRData");
	NFGNMRData::ReloadNMRData = (ReloadNMRDataFunc) NMRFilipCoreDll->GetSymbol("ReloadNMRData");
	
	NFGNMRData::CheckProcParam = (CheckProcParamFunc) NMRFilipCoreDll->GetSymbol("CheckProcParam");
	NFGNMRData::GetProcParam = (GetProcParamFunc) NMRFilipCoreDll->GetSymbol("GetProcParam");
	NFGNMRData::SetProcParam = (SetProcParamFunc) NMRFilipCoreDll->GetSymbol("SetProcParam");
	NFGNMRData::ImportProcParams = (ImportProcParamsFunc) NMRFilipCoreDll->GetSymbol("ImportProcParams");
	
	NFGNMRData::DataToText = (DataToTextFunc) NMRFilipCoreDll->GetSymbol("DataToText");
	
	NFGNMRData::InitUserlist = (InitUserlistFunc) NMRFilipCoreDll->GetSymbol("InitUserlist");
	NFGNMRData::ReadUserlist = (ReadUserlistFunc) NMRFilipCoreDll->GetSymbol("ReadUserlist");
	NFGNMRData::WriteUserlist = (WriteUserlistFunc) NMRFilipCoreDll->GetSymbol("WriteUserlist");
	NFGNMRData::FreeUserlist = (FreeUserlistFunc) NMRFilipCoreDll->GetSymbol("FreeUserlist");
	
	NFGNMRData::CleanupOnExit = (CleanupOnExitFunc) NMRFilipCoreDll->GetSymbol("CleanupOnExit");
	
	if ( 
		(NFGNMRData::InitNMRData == NULL) || (NFGNMRData::CheckNMRData == NULL) || (NFGNMRData::FreeNMRData == NULL) || 
		(NFGNMRData::RefreshNMRData == NULL) || (NFGNMRData::ReloadNMRData == NULL) ||
		(NFGNMRData::CheckProcParam == NULL) || (NFGNMRData::GetProcParam == NULL) || (NFGNMRData::SetProcParam == NULL) ||
		(NFGNMRData::ImportProcParams == NULL) || (NFGNMRData::DataToText == NULL) ||
		(NFGNMRData::InitUserlist == NULL) || (NFGNMRData::ReadUserlist == NULL) || 
		(NFGNMRData::WriteUserlist == NULL) || (NFGNMRData::FreeUserlist == NULL) || 
		(NFGNMRData::CleanupOnExit == NULL)
	) {
		wxLogError("Some functions of the NMRFilip core library not found.");
		return false;
	}
	
	/// Create the document manager and templates
	m_docManager = new NFGDocManager;
	new NFGDocTemplate(m_docManager, "Text", "*.txt", wxEmptyString, "txt", "Text File", "Text View", CLASSINFO(NFGTextDocument), CLASSINFO(NFGTextView));
	new NFGDocTemplate(m_docManager, "Userlist File", "ulist;userlist", wxEmptyString, wxEmptyString, "Userlist File", "Userlist View", CLASSINFO(NFGUserlistDocument), CLASSINFO(NFGUserlistView));
	new NFGDocTemplate(m_docManager, "Fid File", "fid", wxEmptyString, wxEmptyString, "Fid File", "Fid View", CLASSINFO(NFGSerDocument), CLASSINFO(NFGSerView));
	new NFGDocTemplate(m_docManager, "Ser File", "ser", wxEmptyString, wxEmptyString, "Ser File", "Ser View", CLASSINFO(NFGSerDocument), CLASSINFO(NFGSerView));

	wxImage::AddHandler(new wxPNGHandler);

        wxArtProvider::Push(new NFGArtProvider);

#ifdef __NMRFilipGUI_MDI__
	long style = wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxVSCROLL | wxHSCROLL | wxFRAME_NO_WINDOW_MENU;
#else
	long style = wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxRESIZE_BORDER;
#endif
	
	SetVendorName("Richard Reznicek");
	SetAppName("NMRFilip GUI");
	SetAppDisplayName("NMRFilip GUI beta");
	
	/// Create the application frame
	frame = new NMRFilipGUIFrame(m_docManager, NULL, wxID_ANY, GetAppDisplayName(), wxPoint(0, 0), wxSize(500, 400), style);
	
#ifdef __NMRFilipGUI_MDI__
	frame->Centre(wxBOTH);
#endif
	
#ifndef __WXMAC__
	frame->Show(true);
#endif

#ifdef GTK_OXYGEN_FIX
	frame->RepeatLayout();
#endif
	
	return true;
}

int NMRFilipGUIApp::OnExit(void)
{
	delete m_docManager;
	if (NFGNMRData::CleanupOnExit != NULL)
		NFGNMRData::CleanupOnExit();
	delete NMRFilipCoreDll;
	NMRFilipCoreDll = NULL;

	return wxApp::OnExit();
}


IMPLEMENT_DYNAMIC_CLASS(NFGDocManager, wxDocManager)

NFGDocManager::NFGDocManager(long flags, bool initialize) : wxDocManager(flags, initialize)
{
	SerProcParams.FirstChunk = 0;
	SerProcParams.LastChunk = INT_MAX;
	SerProcParams.UseFirstLastChunkPoint = true;
	SerProcParams.FirstChunkPoint = 0;
	SerProcParams.LastChunkPoint = INT_MAX;
	SerProcParams.FFTLength = 128;
	SerProcParams.UseFilter = false;
	SerProcParams.Filter = 0.1;
	
	SerPhaseParams.PhaseCorr0 = 0.0;
	SerPhaseParams.PhaseCorr1 = 0.0;
	SerPhaseParams.ZeroOrderSameValuesForAll = true;
	SerPhaseParams.FirstOrderSameValuesForAll = true;
	SerPhaseParams.ZeroOrderAutoAllTogether = false;
	SerPhaseParams.ZeroOrderAuto = false;
	SerPhaseParams.ZeroOrderFollowAuto = false;
	SerPhaseParams.PilotStep = 0;
	SerPhaseParams.ZeroOrderSetAllAuto = false;
	SerPhaseParams.ZeroOrderSetAllManual = false;

	ZParams.AutoZoomHAll = true;
	ZParams.AutoZoomHSelected = false;
	ZParams.AutoZoomVAll = true;
	ZParams.AutoZoomVSelected = false;
	ZParams.DrawPoints = false;
	ZParams.ThickLines = false;
	ZParams.minx = 0.0;
	ZParams.miny = 0.0;
	ZParams.maxx = 1.0e3;
	ZParams.maxy = 1.0e3;

	LastActiveView = NULL;
}

NFGDocManager::~NFGDocManager()
{
	
}

wxDEFINE_EVENT(NFGEVT_CHILDFRAME_ACTIVATION, wxCommandEvent);
wxDEFINE_EVENT(NFGEVT_CHILDFRAME_EXISTENCE, wxCommandEvent);
wxDEFINE_EVENT(NFGEVT_CHILDFRAME_LABEL, wxCommandEvent);

void NFGDocManager::ActivateView(wxView* view, bool activate) 
{
	if (activate)
		LastActiveView = view;

	if (view && view->GetFrame()) {
		if (activate) {
			wxCommandEvent ActivateIconChangeEvt(NFGEVT_CHILDFRAME_ACTIVATION);
			ActivateIconChangeEvt.SetInt(1);
			ActivateIconChangeEvt.SetEventObject(view->GetFrame());
			wxPostEvent(view->GetFrame()->GetEventHandler(), ActivateIconChangeEvt);
		
			if (view->GetFrame()->GetParent()) {
				wxList::const_iterator iter, iter2;
				for (iter = m_docs.begin(); iter != m_docs.end(); ++iter) {
					wxDocument* doc = (wxDocument *)*iter;
					
					for (iter2 = doc->GetViews().begin(); iter2 != doc->GetViews().end(); ++iter2) {
						wxWindow* win = ((wxView *)*iter2)->GetFrame();
				
						if (win && (win != view->GetFrame())) {
							wxCommandEvent DeActivateIconChangeEvt(NFGEVT_CHILDFRAME_ACTIVATION);
							DeActivateIconChangeEvt.SetInt(0);
							DeActivateIconChangeEvt.SetEventObject(view->GetFrame());
							wxPostEvent(win->GetEventHandler(), DeActivateIconChangeEvt);
						}
					}
				}
			}
		} 
	}
	
	wxDocManager::ActivateView(view, activate); 
};

/// Allows to treat the last active view as the current view in the case when none of the views has focus (typically whe the main application frame or some other window is active).
wxView* NFGDocManager::GetCurrentView() const
{
	wxView* view = wxDocManager::GetCurrentView();
	if (view != NULL) 
		return view;
	
	if (LastActiveView == NULL)
		return NULL;
	
	wxList::const_iterator iter;
	for (iter = m_docs.begin(); iter != m_docs.end(); ++iter) {
		wxDocument* doc = (wxDocument *) *iter;
		
		if (doc->GetViews().IndexOf(LastActiveView) != wxNOT_FOUND)
			return LastActiveView;
	}
/// would be meaningful, but cannot be done in const method
//	LastActiveView = NULL;
	
	return NULL;
}

wxDocument* NFGDocManager::GetCurrentDocument()
{
	wxView *view = GetCurrentView();
	if (view == NULL)
		return NULL;
	else 
		return view->GetDocument();
}

void NFGDocManager::StoreSerProcParams(ProcParams params)
{
	SerProcParams = params;
}

ProcParams NFGDocManager::LoadSerProcParams()
{
	return SerProcParams;
}


void NFGDocManager::StoreSerPhaseCorrParams(PhaseCorrParams params)
{
	SerPhaseParams = params;
}

PhaseCorrParams NFGDocManager::LoadSerPhaseCorrParams()
{
	return SerPhaseParams;
}


void NFGDocManager::StoreZoomParams(ZoomParams params)
{
	ZParams = params;
}

ZoomParams NFGDocManager::LoadZoomParams()
{
	return ZParams;
}

void NFGDocManager::CopyCursorPos(wxString &CursorPosStr, bool Append)
{
	if (Append)
		CursorPosCopy += CursorPosStr;
	else 
		CursorPosCopy = CursorPosStr;
	
	/// Copy to clipboard
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(CursorPosCopy));
		wxTheClipboard->Close();
	}
}



BEGIN_EVENT_TABLE(NFGDocDIChildFrame, wxDocDIChildFrame)
	EVT_MAXIMIZE(NFGDocDIChildFrame::OnMaximize) 
	EVT_COMMAND(wxID_ANY, NFGEVT_CHILDFRAME_ACTIVATION, NFGDocDIChildFrame::OnActivation)
END_EVENT_TABLE()

NFGDocDIChildFrame::NFGDocDIChildFrame(wxDocument* doc, wxView* view, wxDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxDocDIChildFrame(doc, view, parent, id, title, pos, size, style, name)
{
	FrameTitle = title;
	Active = false;

	if (doc && doc->GetDocumentTemplate()) {
		icons = wxArtProvider::GetIconBundle(doc->GetDocumentTemplate()->GetDocumentName(), wxART_FRAME_ICON);
		icons_g = wxArtProvider::GetIconBundle(doc->GetDocumentTemplate()->GetDocumentName() + " Inactive", wxART_FRAME_ICON);
	}
	
	
	if (GetParent()) {
		wxCommandEvent ChildFrameCreationEvt(NFGEVT_CHILDFRAME_EXISTENCE);
		ChildFrameCreationEvt.SetInt(1);
		ChildFrameCreationEvt.SetEventObject(this);
		ChildFrameCreationEvt.SetString(GetTitle());
		wxPostEvent(GetParent()->GetEventHandler(), ChildFrameCreationEvt);
			
#ifdef __NMRFilipGUI_SDI__
		/// In SDI mode, this application is intended to use the whole desktop. 
		/// (Use of multiple desktops while dedicating one for it is therefore recommended.)
		/// The following is just the simplest approach to lay out the windows in some reasonable way - do not expect too much:
		if (parent && doc && doc->GetDocumentManager() && (wxDisplay::GetFromWindow(parent) == wxDisplay::GetFromWindow(this))) {
			wxDisplay Display(wxDisplay::GetFromWindow(parent));
			wxRect ClientRect = Display.GetClientArea();
			
			size_t n = 0;
			wxList &docs = doc->GetDocumentManager()->GetDocuments();
			wxList::const_iterator iter;
			for (iter = docs.begin(); iter != docs.end(); ++iter) {
				wxDocument* doc = (wxDocument *)*iter;
				n += doc->GetViews().GetCount();
			}
			
			if (n > 0)	/// should be always true
				n--;

			wxSize size = wxSize(ClientRect.GetRight() - (parent->GetScreenRect()).GetRight(), (parent->GetScreenRect()).GetHeight());
			size.DecBy(wxSystemSettings::GetMetric(wxSYS_FRAMESIZE_X, parent) * 2, - wxSystemSettings::GetMetric(wxSYS_FRAMESIZE_Y, parent) - wxSystemSettings::GetMetric(wxSYS_CAPTION_Y, parent));

			/// window size should be at least 400x250 px
			int nx = size.GetWidth() / 400;
			int ny = size.GetHeight() / 250;

			if ((nx > 0) && (ny > 0)) {
				int x = ((n%(nx*ny))/ny) * (size.GetWidth() / nx) + (parent->GetScreenRect()).GetRight() + wxSystemSettings::GetMetric(wxSYS_FRAMESIZE_X, parent) * 2;
 				int y = ((n%(nx*ny))%ny) * (size.GetHeight() / ny) + (parent->GetScreenRect()).GetTop();
				int width = size.GetWidth() / nx;
				int height = size.GetHeight() / ny;

				SetSize(x, y, width, height);
			}
		}
#endif
	}
}

NFGDocDIChildFrame::~NFGDocDIChildFrame()
{
	if (GetParent()) {
		wxCommandEvent ChildFrameDestructionEvt(NFGEVT_CHILDFRAME_EXISTENCE);
		ChildFrameDestructionEvt.SetInt(0);
		ChildFrameDestructionEvt.SetEventObject(this);
		wxPostEvent(GetParent()->GetEventHandler(), ChildFrameDestructionEvt);
	}
}

void NFGDocDIChildFrame::OnMaximize(wxMaximizeEvent& event)
{
#ifdef __NMRFilipGUI_MDI__
	if (GetParent() != NULL) {
		SetSize(((wxDocDIParentFrame*) GetParent())->GetClientWindow()->GetClientSize());
		Move(0, 0);
	}
#endif
	event.Skip();
}

void NFGDocDIChildFrame::OnActivation(wxCommandEvent& event)
{
	Active = (event.GetInt() != 0);
	SetIcons((Active)?(icons):(icons_g));
	
#ifdef __WXGTK__
	wxTopLevelWindow::SetTitle((Active)?("> " + FrameTitle + " <"):(FrameTitle));
#else
	wxDocDIChildFrame::SetLabel((Active)?("> " + FrameTitle + " <"):(FrameTitle));
#endif

	event.Skip();
}

void NFGDocDIChildFrame::SetLabel(const wxString& label)
{
	if (GetParent()) {
		wxCommandEvent ChildFrameLabelEvt(NFGEVT_CHILDFRAME_LABEL);
		ChildFrameLabelEvt.SetEventObject(this);
		ChildFrameLabelEvt.SetString(label);
		wxPostEvent(GetParent()->GetEventHandler(), ChildFrameLabelEvt);
	}	

	FrameTitle = label;

#ifdef __WXGTK__
	wxTopLevelWindow::SetTitle((Active)?("> " + FrameTitle + " <"):(FrameTitle));
#else
	wxDocDIChildFrame::SetLabel((Active)?("> " + FrameTitle + " <"):(FrameTitle));
#endif
}


#ifdef GTK_OXYGEN_FIX
BEGIN_EVENT_TABLE( NFGUserlistDIChildFrame, NFGDocDIChildFrame )
	EVT_SIZE(NFGUserlistDIChildFrame::OnSize) 
END_EVENT_TABLE()

NFGUserlistDIChildFrame::NFGUserlistDIChildFrame(wxDocument* doc, wxView* view, wxDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : NFGDocDIChildFrame(doc, view, parent, id, title, pos, size, style, name)
{

}

NFGUserlistDIChildFrame::~NFGUserlistDIChildFrame()
{

}

void NFGUserlistDIChildFrame::OnSize(wxSizeEvent& event)
{
	//~ Fit();	/// required just because wxMDIChildFrame::HandleGetMinMaxInfo() in MSW port of wxW 2.8.8 ignores max size
	Fit();	/// In wxGTK 3.1.2 in oxygen-gtk theme, static box size info for sizers does not seem to be correct before the window is shown, so the Fit() has to be called later.
	
	event.Skip();
}
#endif

NFGDocTemplate::NFGDocTemplate(NFGDocManager* manager, const wxString& descr, const wxString& filter, const wxString& dir, const wxString& ext, const wxString& docTypeName, const wxString& viewTypeName, wxClassInfo* docClassInfo, wxClassInfo* viewClassInfo, long flags) : wxDocTemplate(manager, descr, filter, dir, ext, docTypeName, viewTypeName, docClassInfo, viewClassInfo, flags)
{
	
}

NFGDocTemplate::~NFGDocTemplate() 
{ 
	
}

bool NFGDocTemplate::FileMatchesTemplate(const wxString& path)
{
	wxString name;
	wxString ext;
	
	wxFileName::SplitPath(path, NULL, &name, &ext);
	ext.MakeLower();
	
	wxStringTokenizer parser(GetFileFilter(), ";");
	wxString anything = "*";
	while (parser.HasMoreTokens()) {
		wxString filter = parser.GetNextToken();
		
		wxString filterName;
		wxString filterExt;
		wxFileName::SplitPath(filter, NULL, &filterName, &filterExt);
		filterExt.MakeLower();
		
		if (filter.IsSameAs(anything))
			return true;
			
		if (filterName.IsSameAs(anything) && filterExt.IsSameAs(anything))
			return true;
			
		if (filterName.IsSameAs(anything) && filterExt.IsSameAs(ext))
			return true;
			
		if (filterName.IsSameAs(name) && filterExt.IsSameAs(anything))
			return true;
			
		if (filterName.IsSameAs(name) && filterExt.IsSameAs(ext))
			return true;
	}
	
	return false;
}
