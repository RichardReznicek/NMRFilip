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

#ifndef __panels__
#define __panels__

#include "wx_pch.h"

#include <wx/mdi.h>
#include <wx/docview.h>
#include <wx/docmdi.h>

#ifdef __NMRFilipGUI_SDI__
#include <wx/display.h>
#endif

#include <cmath>

#include "cd.h"
#include "panels_cd.h"
#include "panelmain_cd.h"
#include "panelproc_cd.h"

#include "validators_cd.h"
#include "doc.h"
#include "nmrfilipgui_cd.h"


class NMRFilipGUIFrame : public wxDocDIParentFrame
{
	DECLARE_CLASS(NMRFilipGUIFrame)
	
	DECLARE_EVENT_TABLE()

	private:
		/// Event handlers
		void OnChildFrameActivation(wxCommandEvent& event);
		void OnChildFrameExistence(wxCommandEvent& event);
		void OnChildFrameLabel(wxCommandEvent& event);
#ifdef MSW_FLICKER_FIX
		void OnCloseWindow(wxCloseEvent& event);
#endif
#ifdef GTK_OXYGEN_FIX
		void OnSize(wxSizeEvent& event);
#endif
	
		wxListBox *WindowsListBox;
#ifdef GTK_OXYGEN_FIX
	public:
		void RepeatLayout();
		
	private:
		bool RedoInitialLayout;
#endif
	protected:
		NFGMainPanel* MainPanel;
		NFGProcPanel* ProcPanel;
		NFGDocManager *DocMan;

	public:
		NMRFilipGUIFrame(NFGDocManager *manager, wxFrame* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxPoint(-1,-1), const wxSize& size = wxSize(-1,-1), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL);
		~NMRFilipGUIFrame();

		void SetWindowsListBox(wxListBox *listbox);
};

#endif
