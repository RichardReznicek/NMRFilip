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

#ifndef __view__
#define __view__

#include "wx_pch.h"

#include <wx/docview.h>

#include "cd.h"
#include "view_cd.h"

#include "userlist_cd.h"
#include "nmrfilipgui_cd.h"
#include "plotwin_cd.h"
#include "infopanel_cd.h"


class NFGTextWindow : public wxTextCtrl
{
	DECLARE_EVENT_TABLE()

	private:
		void OnContextMenu(wxContextMenuEvent& event);
		void OnContextMenuCommand(wxCommandEvent& event);
	
		void OnChange(wxCommandEvent& event);
	
		wxMenu *ContextMenu;
		wxDocument *Doc;
	
	public:
		virtual void SetFocusFromKbd();
	
		NFGTextWindow(wxDocument *document, wxDocDIChildFrame *frame, const wxPoint& pos, const wxSize& size, long style);
		~NFGTextWindow();
};


class NFGTextView : public wxView
{
	DECLARE_DYNAMIC_CLASS(NFGTextView)

	private: 
		NFGDocDIChildFrame *frame;
		NFGTextWindow *textsw;
		
	public:
		NFGTextView();
		~NFGTextView() {}
		
		bool OnCreate(wxDocument *doc, long flags);
		void OnDraw(wxDC *dc);
		bool OnClose(bool deleteWindow = true);
		NFGTextWindow *GetTextWin() { return textsw; }
};


class NFGSerView : public wxView
{
	DECLARE_DYNAMIC_CLASS(NFGSerView)

	private: 
		NFGDocDIChildFrame *frame;
		NFGGraphWindow *GraphWindow;
		
	public:
		NFGSerView();
		~NFGSerView() {}
		
		bool OnCreate(wxDocument *doc, long flags);
		void OnDraw(wxDC *dc);
		bool OnClose(bool deleteWindow = true);
		NFGGraphWindow *GetGraphWin() { return GraphWindow; }
};



class NFGInfoView : public wxView
{
	DECLARE_DYNAMIC_CLASS(NFGInfoView)
	
	private: 
		NFGInfoDIChildFrame *frame;
		NFGInfoPanel *InfoPanel;
		
	public:
		NFGInfoView();
		~NFGInfoView() {}
		
		bool OnCreate(wxDocument *doc, long flags);
		void OnDraw(wxDC *dc);
		bool OnClose(bool deleteWindow = true);
		void OnChangeFilename();
		NFGInfoPanel *GetInfoWin() { return InfoPanel; }
};



class NFGUserlistView : public wxView
{
	DECLARE_DYNAMIC_CLASS(NFGUserlistView)
	
	private: 
		NFGUserlistDIChildFrame *frame;
		NFGUserlistMainPanel *UserlistMainPanel;
	
	public:
		NFGUserlistView();
		~NFGUserlistView() {}
		
		bool OnCreate(wxDocument *doc, long flags);
		void OnDraw(wxDC *dc);
		bool OnClose(bool deleteWindow = true);
		NFGUserlistMainPanel *GetUserlistWin() { return UserlistMainPanel; }
};

#endif
