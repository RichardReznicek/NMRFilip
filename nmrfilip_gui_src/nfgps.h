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

#ifndef __nfgps__
#define __nfgps__

#include "wx_pch.h"

#include <wx/buffer.h>
#include <wx/ffile.h>
#include <cmath>

#include "cd.h"
#include "nfgps_cd.h"


class NFGPSDC : public wxDC
{
	private:
		DECLARE_DYNAMIC_CLASS(NFGPSDC)
	
	public:
		NFGPSDC(wxSize size = wxSize(210, 297), bool landscape = false, bool encapsulated = false, int ppi = 72);	/// size in mm

		bool WriteToFile(wxString filename);
		bool WriteToStream(wxOutputStream* stream);
};

class NFGPSDCImpl : public wxDCImpl
{
	DECLARE_DYNAMIC_CLASS(NFGPSDCImpl)

	private:
		static const PSFontInfo FontInfo[3][2][2];
		wxString PenCommands[5];

	protected:
		void UsePen(double scale = 1.0);
		void UseBrush();
	
		void CreatePath(int n, const wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0, bool fill = false, bool eofill = false, bool close = false, bool stroke = true, bool clip = false, bool calc_bbox = true, int bbox_extra = 0);
		
		bool DoFloodFill(wxCoord x1, wxCoord y1, const wxColour &col, wxFloodFillStyle style = wxFLOOD_SURFACE) { return false; }
		bool DoGetPixel(wxCoord x1, wxCoord y1, wxColour *col) const { return false; }
		void DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2);
		void DoCrossHair(wxCoord x, wxCoord y) {}
		void DoDrawArc(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, wxCoord xc, wxCoord yc);
		void DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h, double sa, double ea);
		void DoDrawPoint(wxCoord x, wxCoord y) {}
		void DoDrawLines(int n, const wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0);
		void DoDrawPolygon(int n, const wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0, wxPolygonFillMode fillStyle = wxODDEVEN_RULE);
		void DoDrawPolyPolygon(int n, const int count[], const wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0, wxPolygonFillMode fillStyle = wxODDEVEN_RULE) {}
		void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) {}
		void DoDrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height, double radius = 20) {}
		void DoDrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height) { DoDrawEllipticArc(x, y, width, height, 0.0, 0.0); }
		void DoDrawSpline(wxList *points) {}
		bool DoBlit(wxCoord xdest, wxCoord ydest, wxCoord width, wxCoord height,
			wxDC *source, wxCoord xsrc, wxCoord ysrc, wxRasterOperationMode rop = wxCOPY, bool useMask = false,
			wxCoord xsrcMask = wxDefaultCoord, wxCoord ysrcMask = wxDefaultCoord) { return false; }
		void DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y) {}
		void DoDrawBitmap(const wxBitmap& bitmap, wxCoord x, wxCoord y, bool useMask = false) {}
		void DoDrawText(const wxString& text, wxCoord x, wxCoord y);
		void DoDrawRotatedText(const wxString& text, wxCoord x, wxCoord y, double angle);
		void DoSetClippingRegion(wxCoord x, wxCoord y, wxCoord width, wxCoord height);
		void DoSetClippingRegionAsRegion(const wxRegion &clip) {}
		void DoSetDeviceClippingRegion(const wxRegion& region) {}
		void DoGetTextExtent(const wxString& string, wxCoord *x, wxCoord *y, wxCoord *descent = NULL, wxCoord *externalLeading = NULL, const wxFont *theFont = NULL) const;
		void DoGetSize(int *width, int *height) const { if (width) *width = (int) (m_size.GetWidth() * m_ppi / 25.4); if (height) *height = (int) (m_size.GetHeight() * m_ppi / 25.4); }
		void DoGetSizeMM(int *width, int *height) const { if (width) *width = m_size.GetWidth(); if (height) *height = m_size.GetHeight(); }

		void SetLogicalFunction(wxRasterOperationMode function) {}
		
		wxMemoryBuffer* m_memBuf;
		wxMemoryBuffer* m_memBufHeader;
		wxArrayString* m_usedFonts;
		wxSize m_size, m_paperSize;
		int m_ppi;
		double m_scale;
		int m_pageNumber;
		bool m_encapsulated;
		bool m_landscape;
		int m_PSFontFamily, m_PSFontStyle, m_PSFontWeight;

	public:
		NFGPSDCImpl(NFGPSDC *owner = NULL, wxSize size = wxSize(210, 297), bool landscape = false, bool encapsulated = false, int ppi = 72);	/// size in mm
		~NFGPSDCImpl();
	
		bool WriteToFile(wxString filename);
		bool WriteToStream(wxOutputStream* stream);
		bool AppendString(wxString str, bool header = false);
	
		bool StartDoc(const wxString& message);
		void EndDoc();
		void StartPage();
		void EndPage();
	
		void Clear() {}
		
		void DestroyClippingRegion();

		void SetFont(const wxFont& font);
		void SetPen(const wxPen& pen);
		void SetBrush(const wxBrush& brush);
		void SetBackground(const wxBrush& brush);

		void SetLogicalFunction(int function) {}
		
		wxCoord GetCharHeight() const;
		wxCoord GetCharWidth() const;
		bool CanGetTextExtent() const { return true; }
		bool CanDrawBitmap() const { return false; }
		
		/// Resolution in pixels per logical inch
		wxSize GetPPI() const { return wxSize(m_ppi, m_ppi); }

		void SetAxisOrientation(bool xLeftRight, bool yBottomUp);
		void SetDeviceOrigin(wxCoord x, wxCoord y);

		void SetBackgroundMode(int WXUNUSED(mode)) { }
		void SetPalette(const wxPalette& WXUNUSED(palette)) { }

		int GetDepth() const { return 24; }
};


struct PSFontInfo 
{
	char* PSFontName;
	wxString FontName;
	double CharWidth[128];
	double Descender;
};

#endif
