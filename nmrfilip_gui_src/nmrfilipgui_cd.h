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

#ifndef __nmrfilipgui_cd__
#define __nmrfilipgui_cd__

#include <wx/docview.h>

#include "cd.h"

class NFGDocDIChildFrame;
#ifdef GTK_OXYGEN_FIX
class NFGUserlistDIChildFrame;
typedef NFGUserlistDIChildFrame NFGInfoDIChildFrame;
#else
typedef NFGDocDIChildFrame NFGUserlistDIChildFrame;
typedef NFGDocDIChildFrame NFGInfoDIChildFrame;
#endif

#ifdef __NMRFilipGUI_MDI__
#include <wx/mdi.h>
#include <wx/docmdi.h>
#define wxDocDIChildFrame	wxDocMDIChildFrame
#define wxDocDIParentFrame	wxDocMDIParentFrame
#define wxDIParentFrame	wxMDIParentFrame
#else
#define wxDocDIChildFrame	wxDocChildFrame
#define wxDocDIParentFrame	wxDocParentFrame
#define wxDIParentFrame	wxFrame
#endif


class NFGDocTemplate;
class NFGDocManager;

class NMRFilipGUIApp;

#endif

