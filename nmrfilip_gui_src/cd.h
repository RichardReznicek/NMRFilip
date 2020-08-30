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

#ifndef __cd__
#define __cd__

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You have to set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif


#if !wxUSE_MDI_ARCHITECTURE
#define __NMRFilipGUI_SDI__	1
#endif

#ifndef __WXMSW__
#define __NMRFilipGUI_SDI__	1
#endif

#ifndef __NMRFilipGUI_SDI__
#define __NMRFilipGUI_MDI__	1
#endif

#if __cplusplus >= 201103L
#define NFGMSTD	std::
#else
#define NFGMSTD
#endif

#ifdef __WXMSW__
#ifdef __NMRFilipGUI_MDI__
/// use the workaround to avoid the flicker issues when sizing the app window observed with wxW 3.1.2 under MSW
/** The wxMDIParentFrame::Create() function simply calls wxApp::GetRegisteredClassName(wxT("wxMDIFrame")) without providing apropriate flags parameter depending on the presence/absence of the wxFULL_REPAINT_ON_RESIZE flag in style. Thus, the MDI parent frame is created using the window class _with_ the CS_HREDRAW | CS_VREDRAW style flags set. Choosing the appropriate value for the flags parameter of the wxApp::GetRegisteredClassName() function (like in the wxWindowMSW::GetMSWClassName()) function) would fix the flicker issue. **/
#define MSW_FLICKER_FIX	1
#endif
/// use the workaround to avoid troubles with wrong initial size of notebook with wxNB_MULTILINE style observed with wxW 3.1.2 under MSW
#define MSW_MLNB_SIZE_FIX
#endif


#ifdef __WXGTK__
/// use the workaround for the static box sizer issues observed with wxW 3.1.2 under GTK with gtk-oxygen theme
#define GTK_OXYGEN_FIX	1
/// use the workaround for unprecise values from wxSystemSettings::GetMetric(wxSYS_VSCROLL_X/Y) observed with wxW 3.1.2 under GTK with gtk-oxygen theme
#define GTK_SCROLL_FIX	1
#endif


#endif
