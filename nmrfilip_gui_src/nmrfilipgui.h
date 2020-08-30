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

#ifndef __nmrfilipgui__
#define __nmrfilipgui__

#include "wx_pch.h"

#include <wx/mdi.h>
#include <wx/docview.h>
#include <wx/docmdi.h>
#include <wx/dynlib.h>
#include <wx/tokenzr.h>

#include "cd.h"
#include "nmrfilipgui_cd.h"

#include "panels_cd.h"
#include "doc.h"


class NMRFilipGUIApp : public wxApp
{
	private:
		NFGDocManager* m_docManager;
		wxDynamicLibrary* NMRFilipCoreDll;
	
		NMRFilipGUIFrame* frame;
	
	public:
		NMRFilipGUIApp();
		~NMRFilipGUIApp();
		bool OnInit();
		int OnExit();
};

DECLARE_APP(NMRFilipGUIApp)


class NFGDocManager : public wxDocManager
{
	DECLARE_DYNAMIC_CLASS(NFGDocManager)
	private:
		ProcParams SerProcParams;
		PhaseCorrParams SerPhaseParams;
		ZoomParams ZParams;
		wxString CursorPosCopy;
		wxView* LastActiveView;
	
	public:
		NFGDocManager(long flags = 0, bool initialize = true);
		~NFGDocManager();
		
		virtual void ActivateView(wxView* view, bool activate = true);
		virtual wxView* GetCurrentView() const;
		wxDocument* GetCurrentDocument();
	
		virtual void StoreSerProcParams(ProcParams params);
		virtual ProcParams LoadSerProcParams();
		virtual void StoreSerPhaseCorrParams(PhaseCorrParams params);
		virtual PhaseCorrParams LoadSerPhaseCorrParams();
	
		virtual void StoreZoomParams(ZoomParams params);
		virtual ZoomParams LoadZoomParams();
	
		virtual void CopyCursorPos(wxString &CursorPosStr, bool Append = false);
};


wxDECLARE_EVENT(NFGEVT_CHILDFRAME_ACTIVATION, wxCommandEvent);
wxDECLARE_EVENT(NFGEVT_CHILDFRAME_EXISTENCE, wxCommandEvent);
wxDECLARE_EVENT(NFGEVT_CHILDFRAME_LABEL, wxCommandEvent);

class NFGDocDIChildFrame : public wxDocDIChildFrame
{
	DECLARE_EVENT_TABLE()
	private:
		void OnMaximize(wxMaximizeEvent& event);
		void OnActivation(wxCommandEvent& event);
	
		wxString FrameTitle;
		bool Active;
		wxIconBundle icons, icons_g;
	
	public:
		NFGDocDIChildFrame(wxDocument* doc, wxView* view, wxDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, const wxString& name = wxFrameNameStr);
		~NFGDocDIChildFrame();
	
		void SetLabel(const wxString& label);
		void SetTitle(const wxString& label) { SetLabel(label); }
};


#ifdef GTK_OXYGEN_FIX
class NFGUserlistDIChildFrame : public NFGDocDIChildFrame
{
	DECLARE_EVENT_TABLE()
	private:
		void OnSize(wxSizeEvent& event);
	
	public:
		NFGUserlistDIChildFrame(wxDocument* doc, wxView* view, wxDIParentFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, const wxString& name = wxFrameNameStr);
		~NFGUserlistDIChildFrame();
};
#endif

class NFGDocTemplate : public wxDocTemplate
{
	public:
		NFGDocTemplate(NFGDocManager* manager, const wxString& descr, const wxString& filter, const wxString& dir, const wxString& ext, const wxString& docTypeName, const wxString& viewTypeName, wxClassInfo* docClassInfo = NULL, wxClassInfo* viewClassInfo = NULL, long flags = wxDEFAULT_TEMPLATE_FLAGS);
		~NFGDocTemplate();
		
		/// necessary to overide this function to distinguish ser/fid/ulist/userlist files correctly
		bool FileMatchesTemplate(const wxString& path);
};

#endif

