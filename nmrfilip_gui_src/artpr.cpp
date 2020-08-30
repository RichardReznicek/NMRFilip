/* 
 * NMRFilip GUI - the NMR data processing software - graphical user interface
 * Copyright (C) 2020 Richard Reznicek
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

#include "artpr.h"

#include "xpmicons.h"


wxImage NFGArtProvider::DoCreateImage(const wxArtID& id, const wxSize& size) 
{
	char *const *bits = NULL;
	
	if (id == "nmrfilipgui") bits = nmrfilipgui; 
	else if (id == "nmrfilipgui_16") bits = nmrfilipgui_16; 
	
	else if (id == "serfile") bits = serfile_32; 
	else if (id == "serfile_g") bits = serfile_g_32; 
	else if (id == "fidfile") bits = fidfile_32; 
	else if (id == "fidfile_g") bits = fidfile_g_32; 
	else if (id == "userlist") bits = userlist_32; 
	else if (id == "userlist_g") bits = userlist_g_32; 
	else if (id == "textfile") bits = textfile_32; 
	else if (id == "textfile_g") bits = textfile_g_32; 
	
	else if (id == "folder") bits = folder; 
	
	else if (id == "autozoomh") bits = autozoomh; 
	else if (id == "autozoomv") bits = autozoomv; 
	else if (id == "autozoomh2") bits = autozoomh2; 
	else if (id == "autozoomv2") bits = autozoomv2; 
	else if (id == "unzoomh") bits = unzoomh; 
	else if (id == "unzoomv") bits = unzoomv; 
	else if (id == "zoomh") bits = zoomh; 
	else if (id == "zoomv") bits = zoomv; 
	
	else if (id == "refresh_icon") bits = refresh_icon; 
	else if (id == "params") bits = params; 
	
	else if (id == "tdd") bits = tdd; 
	else if (id == "chunkavg") bits = chunkavg; 
	else if (id == "ft") bits = ft; 
	else if (id == "ftenv") bits = ftenv; 
	else if (id == "eval") bits = eval; 
	
	else if (id == "clipboard") bits = clipboard; 
	else if (id == "go") bits = go; 
	else if (id == "refresh_smallicon") bits = refresh_smallicon; 

	else if (id == "ignored") bits = ignored; 
	else if (id == "blank") bits = blank; 
	else if (id == "nie") bits = nie; 
	else if (id == "notshown") bits = notshown; 
	else if (id == "ok") bits = ok; 

	if (bits == NULL)
		return wxNullImage;
	
	wxImage image = wxImage(bits);
	if (image.IsOk()) {
		if (size.IsFullySpecified() && (size != image.GetSize())) {
			image.InitAlpha();
			image.Rescale(size.GetWidth(), size.GetHeight(), wxIMAGE_QUALITY_HIGH);
		}
		return image;
	}
	
	return wxNullImage;
}

wxBitmap NFGArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) 
{
	wxImage image;
	wxArtID id0;
	if (id.EndsWith("_disabled", &id0)) 
		image = DoCreateImage(id0, size).ConvertToDisabled();
	else
		image = DoCreateImage(id, size);
	
	if (image.IsOk()) 
		return image;
	
	return wxNullBitmap;
}

wxIconBundle NFGArtProvider::CreateIconBundle(const wxArtID& id, const wxArtClient& client) 
{
	wxImage image16, image32;
	
	if (id == "NMRFilipGUI") {
		image16 = DoCreateImage("nmrfilipgui_16");
		image32 = DoCreateImage("nmrfilipgui");
	}
	else if (id == "Ser File") image16 = image32 = DoCreateImage("serfile");
	else if (id == "Ser File Inactive") image16 = image32 = DoCreateImage("serfile_g");
	else if (id == "Fid File") image16 = image32 = DoCreateImage("fidfile");
	else if (id == "Fid File Inactive") image16 = image32 = DoCreateImage("fidfile_g");
	else if (id == "Userlist File") image16 = image32 = DoCreateImage("userlist");
	else if (id == "Userlist File Inactive") image16 = image32 = DoCreateImage("userlist_g");
	else if (id == "Text File") image16 = image32 = DoCreateImage("textfile");
	else if (id == "Text File Inactive") image16 = image32 = DoCreateImage("textfile_g");
	else return wxNullIconBundle;
	
	if (image16.IsOk() && image32.IsOk()) {
		if (image16.GetSize() != wxSize(16, 16)) {
			image16.InitAlpha();
			image16.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);
		}
		wxIcon icon16;
		icon16.CopyFromBitmap(image16);
		
		if (image32.GetSize() != wxSize(32, 32)) {
			image32.InitAlpha();
			image32.Rescale(32, 32, wxIMAGE_QUALITY_HIGH);
		}
		wxIcon icon32;
		icon32.CopyFromBitmap(image32);
		
		wxIconBundle icons;
		icons.AddIcon(icon16);
		icons.AddIcon(icon32);
		return icons;
	}
	
	return wxNullIconBundle;
}
