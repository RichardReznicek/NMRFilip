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

#ifndef __artpr__
#define __artpr__

#include "wx_pch.h"

#include <wx/artprov.h>

#include "cd.h"
#include "artpr_cd.h"


class NFGArtProvider : public wxArtProvider
{
	private:
		wxImage DoCreateImage(const wxArtID& id, const wxSize& size = wxDefaultSize);
	
	protected:
		virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size);
		virtual wxIconBundle CreateIconBundle(const wxArtID& id, const wxArtClient& client);
};

#endif
