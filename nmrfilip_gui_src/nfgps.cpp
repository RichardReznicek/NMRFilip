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

#include "nfgps.h"

/** A simple single-purpose PS device context written just to avoid the limitations and 
complications connected with the use of wxPostScriptDC for PS-file generation, especially on msw **/

/** Please ensure that the following is placed in the application initialization code:
#ifdef __WXGTK__
	setlocale(LC_NUMERIC, "C");
#endif
**/

/** Avoid using SetUserScale() function. **/

IMPLEMENT_DYNAMIC_CLASS(NFGPSDC, wxDC)

NFGPSDC::NFGPSDC(wxSize size, bool landscape, bool encapsulated, int ppi) : wxDC(new NFGPSDCImpl(this, size, landscape, encapsulated, ppi))
{
	
}

bool NFGPSDC::WriteToFile(wxString filename) { 
	return wxDynamicCast(m_pimpl, NFGPSDCImpl)->WriteToFile(filename); 
}

bool NFGPSDC::WriteToStream(wxOutputStream* stream) { 
	return wxDynamicCast(m_pimpl, NFGPSDCImpl)->WriteToStream(stream); 
}


IMPLEMENT_ABSTRACT_CLASS(NFGPSDCImpl, wxDCImpl)

NFGPSDCImpl::NFGPSDCImpl(NFGPSDC *owner, wxSize size, bool landscape, bool encapsulated, int ppi) : wxDCImpl(owner)
{
	m_memBuf = new wxMemoryBuffer(64*1024);
	m_memBufHeader = new wxMemoryBuffer(2*1024);
	m_usedFonts = new wxArrayString();
	
	m_paperSize = size;
	m_size = (landscape)?(wxSize(size.GetHeight(), size.GetWidth())):(size);
	m_landscape = landscape;
	m_encapsulated = encapsulated;
	m_ppi = (ppi > 0)?(ppi):(72);
	m_scale = m_ppi / 72.0;
	
	m_pageNumber = 0;

	m_clipping = false;
	
	m_PSFontFamily = 0;
	m_PSFontStyle = 0;
	m_PSFontWeight = 0;

	SetAxisOrientation(true, true);
	
	SetDeviceOrigin(0, 0);

	m_ok = true;
}

NFGPSDCImpl::~NFGPSDCImpl()
{
	delete m_memBuf;
	delete m_memBufHeader;
	delete m_usedFonts;
}

bool NFGPSDCImpl::WriteToFile(wxString filename)
{
	if (m_memBufHeader == NULL)	/// should never happen
		return false;
	
	if (m_memBuf == NULL)	/// should never happen
		return false;

	wxFFile OutputFile(filename, "wb");

	if (!OutputFile.IsOpened())
		return false;
	
	if (OutputFile.Error()) {
		OutputFile.Close();
		return false;
	}
	
	if (OutputFile.Write(m_memBufHeader->GetData(), m_memBufHeader->GetDataLen()) != (m_memBufHeader->GetDataLen())) {
		OutputFile.Close();
		return false;
	}
		
	if (OutputFile.Write(m_memBuf->GetData(), m_memBuf->GetDataLen()) != (m_memBuf->GetDataLen())) {
		OutputFile.Close();
		return false;
	}
	
	if (!OutputFile.Close())
		return false;

	return true;
}

bool NFGPSDCImpl::WriteToStream(wxOutputStream* stream)
{
	if (m_memBufHeader == NULL)	/// should never happen
		return false;
	
	if (m_memBuf == NULL)	/// should never happen
		return false;
	
	if (!stream)
		return false;
	
	for (size_t written = 0, towrite = m_memBufHeader->GetDataLen(); towrite > 0; towrite -= stream->LastWrite(), written += stream->LastWrite())
		stream->Write(((char*) m_memBufHeader->GetData()) + written, towrite);
		
	for (size_t written = 0, towrite = m_memBuf->GetDataLen(); towrite > 0; towrite -= stream->LastWrite(), written += stream->LastWrite())
		stream->Write(((char*) m_memBuf->GetData()) + written, towrite);
		
	if (!stream->Close())
		return false;
	
	return true;
}

bool NFGPSDCImpl::AppendString(wxString str, bool header)
{
	if (header) {
		if (m_memBufHeader == NULL)	/// should never happen
			return false;
		
		m_memBufHeader->AppendData(str.char_str(), strlen(str.char_str()));

	} else {
		if (m_memBuf == NULL)	/// should never happen
			return false;
		
		if ((str.Find("grestore") != wxNOT_FOUND) || (str.Find("showpage") != wxNOT_FOUND)) {	/// initgraphics, setgstate and setpagedevice not used here
			PenCommands[0] = wxEmptyString;
			PenCommands[1] = wxEmptyString;
			PenCommands[2] = wxEmptyString;
			PenCommands[3] = wxEmptyString;
			PenCommands[4] = wxEmptyString;
		}
		
		if ((str.Find('\n') != wxNOT_FOUND) && (((size_t) str.Find('\n') + 1) == str.Len())) {
			int index = -1;
			
			if (str.EndsWith("setlinewidth\n")) 
				index = 0;
			if (str.EndsWith("setdash\n")) 
				index = 1;
			if (str.EndsWith("setlinecap\n")) 
				index = 2;
			if (str.EndsWith("setlinejoin\n")) 
				index = 3;
			if (str.EndsWith("setrgbcolor\n")) 
				index = 4;
			
			if (index >= 0) {
				if (str.IsSameAs(PenCommands[index])) 
					return true;	/// avoid repeatedly setting the same properties to reduce output size
			
				PenCommands[index] = str;
			}
		}
		
		m_memBuf->AppendData(str.char_str(), strlen(str.char_str()));
	}
	
	return true;
}


bool NFGPSDCImpl::StartDoc(const wxString& message)
{
	SetBrush(*wxBLACK_BRUSH);
	SetPen(*wxBLACK_PEN);
	SetBackground(*wxWHITE_BRUSH);
	SetTextForeground(*wxBLACK);

	PenCommands[0] = wxEmptyString;
	PenCommands[1] = wxEmptyString;
	PenCommands[2] = wxEmptyString;
	PenCommands[3] = wxEmptyString;
	PenCommands[4] = wxEmptyString;
	
	return true;
}

void NFGPSDCImpl::EndDoc()
{
	/// Create header info
	if (m_encapsulated)
	//	AppendString("%!PS-Adobe-3.0 EPSF-3.0\n", true);
		AppendString("%!PS-Adobe-2.0 EPSF-1.2\n", true);
	else
	//	AppendString("%!PS-Adobe-3.0\n", true);
		AppendString("%!PS-Adobe-2.0\n", true);
	
	if (!m_encapsulated) {
		if (m_landscape)
			AppendString("%%Orientation: Landscape\n", true);
		else
			AppendString("%%Orientation: Portrait\n", true);
		
		if (m_paperSize == wxSize(210, 297))
			AppendString("%%DocumentPaperSizes: A4\n", true);
	
		AppendString(wxString::Format("%%%%Pages: %d\n", m_pageNumber), true);
	}
	
	if (m_usedFonts->GetCount() > 0) {
		AppendString("%%DocumentNeededResources: font", true);
		for (size_t i = 0; i < m_usedFonts->GetCount(); i++)
			AppendString(" " + m_usedFonts->Item(i), true);
		AppendString("\n", true);
	}
	
	/// Calculate and write BoundingBox
	double Min_X = LogicalToDeviceX(MinX())/m_scale;
	double Max_X = LogicalToDeviceX(MaxX())/m_scale;
	double Min_Y = LogicalToDeviceY(MinY())/m_scale;
	double Max_Y = LogicalToDeviceY(MaxY())/m_scale;
	
	if (m_landscape) {
		Min_Y = m_size.GetHeight()*72.0/25.4 - Min_Y;
		Max_Y = m_size.GetHeight()*72.0/25.4 - Max_Y;
		
		/// Swap X and Y coordinates
		double aux_X = Min_X;
		Min_X = Min_Y;
		Min_Y = aux_X;
		
		aux_X = Max_X;
		Max_X = Max_Y;
		Max_Y = aux_X;
	}
	
	if (Min_X > Max_X) {
		double aux = Max_X;
		Max_X = Min_X;
		Min_X = aux;
	}

	if (Min_Y > Max_Y) {
		double aux = Max_Y;
		Max_Y = Min_Y;
		Min_Y = aux;
	}

	AppendString(wxString::Format("%%%%BoundingBox: %ld %ld %ld %ld\n", NFGMSTD lround(std::floor(Min_X)), NFGMSTD lround(std::floor(Min_Y)), NFGMSTD lround(std::ceil(Max_X)), NFGMSTD lround(std::ceil(Max_Y))), true);
	
	AppendString("%%EndComments\n\n", true);
	
	if (m_encapsulated) 
		AppendString(wxString::Format("%.8g %.8g scale\n", 1.0/m_scale, 1.0/m_scale), true);
	
	/// Finish the document
	if (m_clipping) {
		AppendString("grestore\n");
		m_clipping = false;
	}
	
	AppendString("%%EOF\n");
}

void NFGPSDCImpl::StartPage()
{
	if (!m_encapsulated) {
		m_pageNumber++;
		
		AppendString(wxString::Format("%%%%Page: Page_%d %d\n", m_pageNumber, m_pageNumber));

		if (m_landscape) {
			AppendString("90 rotate\n");
			AppendString(wxString::Format("%d %d translate\n", 0, - (int) (m_size.GetHeight()*72.0/25.4)));
		}
		
		AppendString(wxString::Format("%.8g %.8g scale\n", 1.0/m_scale, 1.0/m_scale));
	}
}

void NFGPSDCImpl::EndPage()
{
	if (!m_encapsulated)
		AppendString("showpage\n\n");
}


void NFGPSDCImpl::DestroyClippingRegion()
{
	if (m_clipping) {
		AppendString("grestore\n");
		m_clipping = false;
	}
}


void NFGPSDCImpl::SetFont(const wxFont& font)
{
	if (!font.Ok())
		return;

	m_font = font;
	
	switch (GetFont().GetFamily()) {
	//	case wxFONTFAMILY_SCRIPT:
	//		m_PSFontFamily = 3;
	//	break;
		
		case wxFONTFAMILY_SWISS:
			m_PSFontFamily = 2;
		break;

		case wxFONTFAMILY_ROMAN:
			m_PSFontFamily = 1;
		break;

		case wxFONTFAMILY_TELETYPE:
		case wxFONTFAMILY_MODERN:
		default:
			m_PSFontFamily = 0;
	}
	
	switch (GetFont().GetStyle()) {
		case wxFONTSTYLE_ITALIC:
			m_PSFontStyle = 1;
		break;

		default:
			m_PSFontStyle = 0;
	}
	
	switch (GetFont().GetWeight()) {
		case wxFONTWEIGHT_BOLD:
			m_PSFontWeight = 1;
		break;

		default:
			m_PSFontWeight = 0;
	}
	
	AppendString(wxString::Format("%hs findfont\n", FontInfo[m_PSFontFamily][m_PSFontStyle][m_PSFontWeight].PSFontName));
	AppendString(wxString::Format("%.8g scalefont setfont\n", (double) GetFont().GetPointSize()));

	if (m_usedFonts->Index(FontInfo[m_PSFontFamily][m_PSFontStyle][m_PSFontWeight].FontName) == wxNOT_FOUND)
		m_usedFonts->Add(FontInfo[m_PSFontFamily][m_PSFontStyle][m_PSFontWeight].FontName);
}

void NFGPSDCImpl::SetPen(const wxPen& pen)
{
	if (!pen.Ok())
		return;

	m_pen = pen;
}

void NFGPSDCImpl::UsePen(double scale)
{
	if (!GetPen().Ok())
		return;

	AppendString(wxString::Format("%.8g setlinewidth\n", scale*GetPen().GetWidth()));

	wxDash NFG_dot[2] = {1, 2};
	const int NFG_dot_n = 2;

	wxDash NFG_long_dash[2] = {8, 3};
	const int NFG_long_dash_n = 2;

	wxDash NFG_short_dash[2] = {4, 3};
	const int NFG_short_dash_n = 2;

	wxDash NFG_dot_dash[4] = {6, 3, 1, 3};
	const int NFG_dot_dash_n = 4;

	wxDash *DashArr = NULL;
	int DashArr_n = 0;
	
	switch (GetPen().GetStyle()) {
		case wxDOT:
			DashArr = NFG_dot;
			DashArr_n = NFG_dot_n;
		break;
		
		case wxLONG_DASH:
			DashArr = NFG_long_dash;
			DashArr_n = NFG_long_dash_n;
		break;
		
		case wxSHORT_DASH:
			DashArr = NFG_short_dash;
			DashArr_n = NFG_short_dash_n;
		break;
		
		case wxDOT_DASH:
			DashArr = NFG_dot_dash;
			DashArr_n = NFG_dot_dash_n;
		break;
		
		case wxUSER_DASH:
			DashArr_n = GetPen().GetDashes(&DashArr);
		break;
		
		case wxSOLID:
		case wxTRANSPARENT:
		default:
			;
	}

	wxString DashStr = "[";
	for (int i = 0; i < DashArr_n; i++)
		DashStr += wxString::Format("%.8g ", DashArr[i]*scale*GetPen().GetWidth());
	DashStr += "] 0 setdash\n";
	
	AppendString(DashStr);
	
	switch (GetPen().GetCap()) {
		case wxCAP_ROUND:
			AppendString("1 setlinecap\n");
		break;
		
		case wxCAP_PROJECTING:
			AppendString("2 setlinecap\n");
		break;
		
		case wxCAP_BUTT:
			AppendString("0 setlinecap\n");
		break;
		
		default:
			;
	}

	switch (GetPen().GetJoin()) {
		case wxJOIN_BEVEL:
			AppendString("2 setlinejoin\n");
		break;
		
		case wxJOIN_ROUND:
			AppendString("1 setlinejoin\n");
		break;
		
		case wxJOIN_MITER:
			AppendString("0 setlinejoin\n");
		break;
		
		default:
			;
	}
	
	double red = GetPen().GetColour().Red()/255.0;
	double blue = GetPen().GetColour().Blue()/255.0;
	double green = GetPen().GetColour().Green()/255.0;

	AppendString(wxString::Format("%.8g %.8g %.8g setrgbcolor\n", red, green, blue));
}


void NFGPSDCImpl::SetBrush(const wxBrush& brush)
{
	if (!brush.Ok())
		return;

	m_brush = brush;
}

void NFGPSDCImpl::UseBrush()
{
	if (!GetBrush().Ok())
		return;

	double red = GetBrush().GetColour().Red()/255.0;
	double blue = GetBrush().GetColour().Blue()/255.0;
	double green = GetBrush().GetColour().Green()/255.0;

	AppendString(wxString::Format("%.8g %.8g %.8g setrgbcolor\n", red, green, blue));
}

void NFGPSDCImpl::SetBackground(const wxBrush& brush)
{
	m_backgroundBrush = brush;
}


wxCoord NFGPSDCImpl::GetCharHeight() const
{
	if (GetFont().Ok())
		return GetFont().GetPointSize();

	return 10;
}

wxCoord NFGPSDCImpl::GetCharWidth() const
{
	return NFGMSTD lround(GetCharHeight()*72.0/120.0);
}


void NFGPSDCImpl::SetAxisOrientation(bool xLeftRight, bool yBottomUp)
{
	wxDCImpl::SetAxisOrientation(xLeftRight, yBottomUp);
}

void NFGPSDCImpl::SetDeviceOrigin(wxCoord x, wxCoord y)
{
	int width = 0;
	int height = 0;
	GetSize(&width, &height);

	wxDCImpl::SetDeviceOrigin(x, height - y);
}



void NFGPSDCImpl::CreatePath(int n, const wxPoint points[], wxCoord xoffset, wxCoord yoffset, bool fill, bool eofill, bool close, bool stroke, bool clip, bool calc_bbox, int bbox_extra)
{
	if (n <= 0) 
		return;
	
	if (calc_bbox && (bbox_extra < 0))
		return;

	AppendString("newpath\n");
	AppendString(wxString::Format("%d %d moveto\n", LogicalToDeviceX(points[0].x + xoffset), LogicalToDeviceY(points[0].y + yoffset)));

	for (int i = 1; i < n ; i++)
		AppendString(wxString::Format("%d %d lineto\n", LogicalToDeviceX(points[i].x + xoffset), LogicalToDeviceY(points[i].y + yoffset)));

	/// if m_clipping == true, vertices of clipping path are taken into the bounding box calculation instead of these points
	if (calc_bbox && !m_clipping) {
		wxCoord xmin = points[0].x;
		wxCoord xmax = points[0].x;
		wxCoord ymin = points[0].y;
		wxCoord ymax = points[0].y;
		
		for (int i = 1; i < n ; i++) {
			if (points[i].x < xmin)
				xmin = points[i].x;
			if (points[i].x > xmax)
				xmax = points[i].x;
			if (points[i].y < ymin)
				ymin = points[i].y;
			if (points[i].y > ymax)
				ymax = points[i].y;
		}
		
		CalcBoundingBox(xmin + xoffset - bbox_extra, ymin + yoffset - bbox_extra);
		CalcBoundingBox(xmax + xoffset + bbox_extra, ymax + yoffset + bbox_extra);
	}
	
	if (fill)
		AppendString("fill\n");
	else 
	if (eofill)
		AppendString("eofill\n");
	
	if (close)
		AppendString("closepath\n");
	if (stroke)
		AppendString("stroke\n");
	if (clip)
		AppendString("clip\n");
}


void NFGPSDCImpl::DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
{
	if (!(GetPen().IsNonTransparent()))
		return;

	UsePen();
	
	wxPoint points[2] = {wxPoint(x1, y1), wxPoint(x2, y2)};
	
	int bbox_extra = GetPen().GetWidth()/* - GetPen().GetWidth()/2*/;
	CreatePath(2, points, 0, 0, false, false, false, true, false, true, bbox_extra);
}

void NFGPSDCImpl::DoDrawLines(int n, const wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
	if (n <= 0) 
		return;

	if (!(GetPen().IsNonTransparent()))
		return;

	UsePen();

	int bbox_extra = GetPen().GetWidth()/* - GetPen().GetWidth()/2*/;
	CreatePath(n, points, xoffset, yoffset, false, false, false, true, false, true, bbox_extra);
}

void NFGPSDCImpl::DoDrawPolygon(int n, const wxPoint points[], wxCoord xoffset, wxCoord yoffset, wxPolygonFillMode fillStyle)
{
	if (n <= 0) 
		return;

	if (GetBrush().IsNonTransparent()) {
		UseBrush();
		CreatePath(n, points, xoffset, yoffset, fillStyle != wxODDEVEN_RULE, fillStyle == wxODDEVEN_RULE, false, false);
	}

	if (GetPen().IsNonTransparent()) {
		UsePen();
		int bbox_extra = GetPen().GetWidth()/* - GetPen().GetWidth()/2*/;
		CreatePath(n, points, xoffset, yoffset, false, false, true, true, false, true, bbox_extra);
	}
}


void NFGPSDCImpl::DoDrawArc(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, wxCoord xc, wxCoord yc)
{
	wxCoord dx1 = x1 - xc;
	wxCoord dy1 = y1 - yc;
	wxCoord dx2 = x2 - xc;
	wxCoord dy2 = y2 - yc;
	
	double radius = std::sqrt(LogicalToDeviceXRel(dx1)*LogicalToDeviceXRel(dx1) + LogicalToDeviceYRel(dy1)*LogicalToDeviceYRel(dy1));
	
	double sa = 180.0*atan2(-dy1, dx1)/M_PI;
	double ea = 180.0*atan2(-dy2, dx2)/M_PI;
	
	if (GetBrush().IsNonTransparent()) {
		UseBrush();
		
		AppendString("newpath\n");
		AppendString(wxString::Format("%.8g %.8g %.8g %.8g %.8g arc\n", (double) LogicalToDeviceX(xc), (double) LogicalToDeviceY(yc), radius, sa, ea));
		if ((x1 != x2) || (y1 != y2)) 	/// not a full circle
			AppendString(wxString::Format("%.8g %.8g lineto\n", (double) LogicalToDeviceX(xc), (double) LogicalToDeviceY(yc)));
 		AppendString("closepath\n");
 		AppendString("fill\n");
		
		if (!m_clipping) {
			/// TODO: Calculate it precisely instead of assuming a full circle:
			CalcBoundingBox(xc - DeviceToLogicalXRel(std::ceil(radius)), yc - DeviceToLogicalYRel(std::ceil(radius)));
			CalcBoundingBox(xc + DeviceToLogicalXRel(std::ceil(radius)), yc + DeviceToLogicalYRel(std::ceil(radius)));
		}
	}

	if (GetPen().IsNonTransparent()) {
		UsePen();

		AppendString("newpath\n");
		AppendString(wxString::Format("%.8g %.8g %.8g %.8g %.8g arc\n", (double) LogicalToDeviceX(xc), (double) LogicalToDeviceY(yc), radius, sa, ea));
		if (GetBrush().IsNonTransparent()) {
			if ((x1 != x2) || (y1 != y2)) 	/// not a full circle
				AppendString(wxString::Format("%.8g %.8g lineto\n", (double) LogicalToDeviceX(xc), (double) LogicalToDeviceY(yc)));
			AppendString("closepath\n");
		}
 		AppendString("stroke\n");
		
		if (!m_clipping) {
			/// TODO: Calculate it precisely instead of assuming a full circle:
			CalcBoundingBox(xc - DeviceToLogicalXRel(std::ceil(radius)) - GetPen().GetWidth(), yc - DeviceToLogicalYRel(std::ceil(radius)) - GetPen().GetWidth());
			CalcBoundingBox(xc + DeviceToLogicalXRel(std::ceil(radius)) + GetPen().GetWidth(), yc + DeviceToLogicalYRel(std::ceil(radius)) + GetPen().GetWidth());
		}
	}
}

void NFGPSDCImpl::DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h, double sa, double ea)
{
	/// Note: In contrast to the msw version of wxDC in wxW-2.8.11, this 
	/// function does not swap the start and end points if ea < sa.
	
	if ((w < 1) || (h < 1))
		return;
	
	double radius = (LogicalToDeviceXRel(w) + LogicalToDeviceYRel(h))/4.0;
	
	double scale_x = LogicalToDeviceXRel(w)/2.0/radius;
	double scale_y = LogicalToDeviceYRel(h)/2.0/radius;
	
	double center_x = ((LogicalToDeviceX(x) + LogicalToDeviceX(x + w))/2.0)/scale_x;
	double center_y = ((LogicalToDeviceY(y) + LogicalToDeviceY(y + h))/2.0)/scale_y;
	
	bool FullEllipse = false;
	if (wxIsSameDouble(sa, ea)) {	/// Draw a full ellipse
		FullEllipse = true;
		ea += 360.0;
	}

	if (w != h) 	/// no need to scale by 1
		AppendString(wxString::Format("%.8g %.8g scale\n", scale_x, scale_y));
	
	if (GetBrush().IsNonTransparent()) {
		UseBrush();
		
		AppendString("newpath\n");
		AppendString(wxString::Format("%.8g %.8g %.8g %.8g %.8g arc\n", center_x, center_y, radius, sa, ea));
		if (!FullEllipse)
			AppendString(wxString::Format("%.8g %.8g lineto\n", center_x, center_y));
 		AppendString("closepath\n");
 		AppendString("fill\n");
		
		if (!m_clipping) {
			/// TODO: Calculate it precisely instead of assuming a full ellipse:
			CalcBoundingBox(x, y);
			CalcBoundingBox(x + w, y + h);
		}
	}

	if (GetPen().IsNonTransparent()) {
		UsePen();

		AppendString("newpath\n");
		AppendString(wxString::Format("%.8g %.8g %.8g %.8g %.8g arc\n", center_x, center_y, radius, sa, ea));
 		AppendString("stroke\n");
		
		if (!m_clipping) {
			/// TODO: Calculate it precisely instead of assuming a full ellipse:
			CalcBoundingBox(x - GetPen().GetWidth(), y - GetPen().GetWidth());
			CalcBoundingBox(x + w + GetPen().GetWidth(), y + h + GetPen().GetWidth());
		}
	}

	if (w != h) 	/// no need to scale by 1
		AppendString(wxString::Format("%.8g %.8g scale\n", 1.0/scale_x, 1.0/scale_y));
}


void NFGPSDCImpl::DoDrawText(const wxString& text, wxCoord x, wxCoord y)
{
	if (GetTextForeground().Ok()) {
		double red = GetTextForeground().Red()/255.0;
		double blue = GetTextForeground().Blue()/255.0;
		double green = GetTextForeground().Green()/255.0;

		AppendString(wxString::Format("%.8g %.8g %.8g setrgbcolor\n", red, green, blue));
	}
	
	wxCoord text_width, text_height, text_descent;

	DoGetTextExtent(text, &text_width, &text_height, &text_descent);

	int size = GetFont().GetPointSize();

	wxCoord bly = y + size - text_descent; /// baseline

	AppendString(wxString::Format("%d %d moveto\n", LogicalToDeviceX(x), LogicalToDeviceY(bly)));
	AppendString("(");

	wxString TextToDraw = text;
	/// replacement of special chars
	TextToDraw.Replace("\\", "\\\\");
	TextToDraw.Replace("(", "\\(");
	TextToDraw.Replace(")", "\\)");
	
	/// TODO: add a support for non-ascii chars
	if (TextToDraw.IsAscii())
		AppendString(TextToDraw);

	AppendString(") show\n");

	/// TODO: implement underline

	CalcBoundingBox(x, y);
	CalcBoundingBox(x + text_width, y + text_height);
}

void NFGPSDCImpl::DoDrawRotatedText(const wxString& text, wxCoord x, wxCoord y, double angle)
{
	if (GetTextForeground().Ok()) {
		double red = GetTextForeground().Red()/255.0;
		double blue = GetTextForeground().Blue()/255.0;
		double green = GetTextForeground().Green()/255.0;

		AppendString(wxString::Format("%.8g %.8g %.8g setrgbcolor\n", red, green, blue));
	}
	
	wxCoord text_width, text_height, text_descent;

	DoGetTextExtent(text, &text_width, &text_height, &text_descent);

	int size = GetFont().GetPointSize();

	wxCoord bly = y + size - text_descent; /// baseline

	AppendString(wxString::Format("%d %d moveto\n", LogicalToDeviceX(x), LogicalToDeviceY(y)));
	AppendString(wxString::Format("%.8g rotate\n", angle));
	AppendString(wxString::Format("%d %d rmoveto\n", - LogicalToDeviceXRel(0), - LogicalToDeviceYRel(bly - y)));
	
	AppendString("(");

	wxString TextToDraw = text;
	/// replacement of special chars
	TextToDraw.Replace("\\", "\\\\");
	TextToDraw.Replace("(", "\\(");
	TextToDraw.Replace(")", "\\)");
	
	/// TODO: add a support for non-ascii chars
	if (TextToDraw.IsAscii())
		AppendString(TextToDraw);
	
	AppendString(") show\n");

	/// TODO: implement underline

	AppendString(wxString::Format("%d %d rmoveto\n", LogicalToDeviceXRel(0), LogicalToDeviceYRel(bly - y)));
	AppendString(wxString::Format("%.8g rotate\n", - angle));


	CalcBoundingBox(x, y);
	CalcBoundingBox(NFGMSTD lround(x + text_width*std::cos(M_PI*angle/180.0)), NFGMSTD lround(y - text_width*std::sin(M_PI*angle/180.0)));
	CalcBoundingBox(NFGMSTD lround(x + text_height*std::sin(M_PI*angle/180.0)), NFGMSTD lround(y + text_height*std::cos(M_PI*angle/180.0)));
	CalcBoundingBox(NFGMSTD lround(x + text_width*std::cos(M_PI*angle/180.0) + text_height*std::sin(M_PI*angle/180.0)), NFGMSTD lround(y - text_width*std::sin(M_PI*angle/180.0) + text_height*std::cos(M_PI*angle/180.0)));
}

void NFGPSDCImpl::DoSetClippingRegion(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
	if (m_clipping)
		DestroyClippingRegion();

	AppendString("gsave\n");
	
	wxPoint points[4] = {wxPoint(x, y), wxPoint(x + width, y), wxPoint(x + width, y + height), wxPoint(x, y + height)};
	
	CreatePath(4, points, 0, 0, false, false, true, false, true, true);	/// include corners into the bounding box

	m_clipping = true;
}


void NFGPSDCImpl::DoGetTextExtent(const wxString& string, wxCoord *x, wxCoord *y, wxCoord *descent, wxCoord *externalLeading, const wxFont *theFont) const
{
	wxCoord FontSize = 0;
	int PSFontFamily = 0;
	int PSFontStyle = 0;
	int PSFontWeight = 0;
	
	if (theFont) {
		if (theFont->IsOk()) {
			FontSize = theFont->GetPointSize();
			
			switch (theFont->GetFamily()) {
			//	case wxFONTFAMILY_SCRIPT:
			//		PSFontFamily = 3;
			//	break;
				
				case wxFONTFAMILY_SWISS:
					PSFontFamily = 2;
				break;

				case wxFONTFAMILY_ROMAN:
					PSFontFamily = 1;
				break;

				case wxFONTFAMILY_TELETYPE:
				case wxFONTFAMILY_MODERN:
				default:
					PSFontFamily = 0;
			}

			switch (theFont->GetStyle()) {
				case wxFONTSTYLE_ITALIC:
					PSFontStyle = 1;
				break;

				default:
					PSFontStyle = 0;
			}

			switch (theFont->GetWeight()) {
				case wxFONTWEIGHT_BOLD:
					PSFontWeight = 1;
				break;

				default:
					PSFontWeight = 0;
			}
		}
	} else 
	if (GetFont().IsOk()) {
		FontSize = GetFont().GetPointSize();
		
		PSFontFamily = m_PSFontFamily;
		PSFontStyle = m_PSFontStyle;
		PSFontWeight = m_PSFontStyle;
	} else {
		if (x) 
			*x = 0;
		if (y) 
			*y = 0;
		if (descent) 
			*descent = 0;
		if (externalLeading) 
			*externalLeading = 0;

		return;
	}
	
	if (x) {
		*x = 0;
		double width = 0.0;
		if (string.IsAscii()) 
			for (size_t i = 0; i < string.Len(); i++)
				if (((unsigned char) (string[i])) < 128)	/// just in case...
					width += FontInfo[PSFontFamily][PSFontStyle][PSFontWeight].CharWidth[(unsigned char) (string[i])];
	
		width *= FontSize;
		*x = NFGMSTD lround(width);
	}
	if (y) 
		*y = FontSize;
	if (descent) 
		*descent = NFGMSTD lround(- FontInfo[PSFontFamily][PSFontStyle][PSFontWeight].Descender * FontSize);
	if (externalLeading) 
		*externalLeading = 0;
}

const PSFontInfo NFGPSDCImpl::FontInfo[3][2][2] = {
	{
		{
			{
				"/Courier", 
				"Courier", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.0}, 
				-0.157
			},
			{
				"/Courier-Bold", 
				"Courier-Bold", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.0}, 
				-0.157
			}
		},
		{
			{
				"/Courier-Oblique", 
				"Courier-Oblique", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.0}, 
				-0.157
			},
			{
				"/Courier-BoldOblique", 
				"Courier-BoldOblique", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.600, 0.0}, 
				-0.157
			}
		}
	},
	{
		{
			{
				"/Times-Roman", 
				"Times-Roman", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.250, 0.333, 0.408, 0.500, 0.500, 0.833, 0.778, 0.333, 0.333, 0.333, 0.500, 0.564, 0.250, 0.333, 0.250, 0.278, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.278, 0.278, 0.564, 0.564, 0.564, 0.444, 0.921, 0.722, 0.667, 0.667, 0.722, 0.611, 0.556, 0.722, 0.722, 0.333, 0.389, 0.722, 0.611, 0.889, 0.722, 0.722, 0.556, 0.722, 0.667, 0.556, 0.611, 0.722, 0.722, 0.944, 0.722, 0.722, 0.611, 0.333, 0.278, 0.333, 0.469, 0.500, 0.333, 0.444, 0.500, 0.444, 0.500, 0.444, 0.333, 0.500, 0.500, 0.278, 0.278, 0.500, 0.278, 0.778, 0.500, 0.500, 0.500, 0.500, 0.333, 0.389, 0.278, 0.500, 0.500, 0.722, 0.500, 0.500, 0.444, 0.480, 0.200, 0.480, 0.541, 0.0}, 
				-0.217
			},
			{
				"/Times-Bold", 
				"Times-Bold", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.250, 0.333, 0.555, 0.500, 0.500, 1.000, 0.833, 0.333, 0.333, 0.333, 0.500, 0.570, 0.250, 0.333, 0.250, 0.278, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.333, 0.333, 0.570, 0.570, 0.570, 0.500, 0.930, 0.722, 0.667, 0.722, 0.722, 0.667, 0.611, 0.778, 0.778, 0.389, 0.500, 0.778, 0.667, 0.944, 0.722, 0.778, 0.611, 0.778, 0.722, 0.556, 0.667, 0.722, 0.722, 1.000, 0.722, 0.722, 0.667, 0.333, 0.278, 0.333, 0.581, 0.500, 0.333, 0.500, 0.556, 0.444, 0.556, 0.444, 0.333, 0.500, 0.556, 0.278, 0.333, 0.556, 0.278, 0.833, 0.556, 0.500, 0.556, 0.556, 0.444, 0.389, 0.333, 0.556, 0.500, 0.722, 0.500, 0.500, 0.444, 0.394, 0.220, 0.394, 0.520, 0.0}, 
				-0.217
			}
		},
		{
			{
				"/Times-Italic", 
				"Times-Italic", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.250, 0.333, 0.420, 0.500, 0.500, 0.833, 0.778, 0.333, 0.333, 0.333, 0.500, 0.675, 0.250, 0.333, 0.250, 0.278, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.333, 0.333, 0.675, 0.675, 0.675, 0.500, 0.920, 0.611, 0.611, 0.667, 0.722, 0.611, 0.611, 0.722, 0.722, 0.333, 0.444, 0.667, 0.556, 0.833, 0.667, 0.722, 0.611, 0.722, 0.611, 0.500, 0.556, 0.722, 0.611, 0.833, 0.611, 0.556, 0.556, 0.389, 0.278, 0.389, 0.422, 0.500, 0.333, 0.500, 0.500, 0.444, 0.500, 0.444, 0.278, 0.500, 0.500, 0.278, 0.278, 0.444, 0.278, 0.722, 0.500, 0.500, 0.500, 0.500, 0.389, 0.389, 0.278, 0.500, 0.444, 0.667, 0.444, 0.444, 0.389, 0.400, 0.275, 0.400, 0.541, 0.0}, 
				-0.217
			},
			{
				"/Times-BoldItalic", 
				"Times-BoldItalic", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.250, 0.389, 0.555, 0.500, 0.500, 0.833, 0.778, 0.333, 0.333, 0.333, 0.500, 0.570, 0.250, 0.333, 0.250, 0.278, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.333, 0.333, 0.570, 0.570, 0.570, 0.500, 0.832, 0.667, 0.667, 0.667, 0.722, 0.667, 0.667, 0.722, 0.778, 0.389, 0.500, 0.667, 0.611, 0.889, 0.722, 0.722, 0.611, 0.722, 0.667, 0.556, 0.611, 0.722, 0.667, 0.889, 0.667, 0.611, 0.611, 0.333, 0.278, 0.333, 0.570, 0.500, 0.333, 0.500, 0.500, 0.444, 0.500, 0.444, 0.333, 0.500, 0.556, 0.278, 0.278, 0.500, 0.278, 0.778, 0.556, 0.500, 0.500, 0.500, 0.389, 0.389, 0.278, 0.556, 0.444, 0.667, 0.500, 0.444, 0.389, 0.348, 0.220, 0.348, 0.570, 0.0}, 
				-0.217
			}
		}
	},
	{
		{
			{
				"/Helvetica", 
				"Helvetica", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.278, 0.278, 0.355, 0.556, 0.556, 0.889, 0.667, 0.222, 0.333, 0.333, 0.389, 0.584, 0.278, 0.333, 0.278, 0.278, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.278, 0.278, 0.584, 0.584, 0.584, 0.556, 1.015, 0.667, 0.667, 0.722, 0.722, 0.667, 0.611, 0.778, 0.722, 0.278, 0.500, 0.667, 0.556, 0.833, 0.722, 0.778, 0.667, 0.778, 0.722, 0.667, 0.611, 0.722, 0.667, 0.944, 0.667, 0.667, 0.611, 0.278, 0.278, 0.278, 0.469, 0.556, 0.222, 0.556, 0.556, 0.500, 0.556, 0.556, 0.278, 0.556, 0.556, 0.222, 0.222, 0.500, 0.222, 0.833, 0.556, 0.556, 0.556, 0.556, 0.333, 0.500, 0.278, 0.556, 0.500, 0.722, 0.500, 0.500, 0.500, 0.334, 0.260, 0.334, 0.584, 0.0}, 
				-0.207
			},
			{
				"/Helvetica-Bold", 
				"Helvetica-Bold", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.278, 0.333, 0.474, 0.556, 0.556, 0.889, 0.722, 0.278, 0.333, 0.333, 0.389, 0.584, 0.278, 0.333, 0.278, 0.278, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.333, 0.333, 0.584, 0.584, 0.584, 0.611, 0.975, 0.722, 0.722, 0.722, 0.722, 0.667, 0.611, 0.778, 0.722, 0.278, 0.556, 0.722, 0.611, 0.833, 0.722, 0.778, 0.667, 0.778, 0.722, 0.667, 0.611, 0.722, 0.667, 0.944, 0.667, 0.667, 0.611, 0.333, 0.278, 0.333, 0.584, 0.556, 0.278, 0.556, 0.611, 0.556, 0.611, 0.556, 0.333, 0.611, 0.611, 0.278, 0.278, 0.556, 0.278, 0.889, 0.611, 0.611, 0.611, 0.611, 0.389, 0.556, 0.333, 0.611, 0.556, 0.778, 0.556, 0.556, 0.500, 0.389, 0.280, 0.389, 0.584, 0.0}, 
				-0.207
			}
		},
		{
			{
				"/Helvetica-Oblique", 
				"Helvetica-Oblique", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.278, 0.278, 0.355, 0.556, 0.556, 0.889, 0.667, 0.222, 0.333, 0.333, 0.389, 0.584, 0.278, 0.333, 0.278, 0.278, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.278, 0.278, 0.584, 0.584, 0.584, 0.556, 1.015, 0.667, 0.667, 0.722, 0.722, 0.667, 0.611, 0.778, 0.722, 0.278, 0.500, 0.667, 0.556, 0.833, 0.722, 0.778, 0.667, 0.778, 0.722, 0.667, 0.611, 0.722, 0.667, 0.944, 0.667, 0.667, 0.611, 0.278, 0.278, 0.278, 0.469, 0.556, 0.222, 0.556, 0.556, 0.500, 0.556, 0.556, 0.278, 0.556, 0.556, 0.222, 0.222, 0.500, 0.222, 0.833, 0.556, 0.556, 0.556, 0.556, 0.333, 0.500, 0.278, 0.556, 0.500, 0.722, 0.500, 0.500, 0.500, 0.334, 0.260, 0.334, 0.584, 0.0}, 
				-0.207
			},
			{
				"/Helvetica-BoldOblique", 
				"Helvetica-BoldOblique", 
				{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.278, 0.333, 0.474, 0.556, 0.556, 0.889, 0.722, 0.278, 0.333, 0.333, 0.389, 0.584, 0.278, 0.333, 0.278, 0.278, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.556, 0.333, 0.333, 0.584, 0.584, 0.584, 0.611, 0.975, 0.722, 0.722, 0.722, 0.722, 0.667, 0.611, 0.778, 0.722, 0.278, 0.556, 0.722, 0.611, 0.833, 0.722, 0.778, 0.667, 0.778, 0.722, 0.667, 0.611, 0.722, 0.667, 0.944, 0.667, 0.667, 0.611, 0.333, 0.278, 0.333, 0.584, 0.556, 0.278, 0.556, 0.611, 0.556, 0.611, 0.556, 0.333, 0.611, 0.611, 0.278, 0.278, 0.556, 0.278, 0.889, 0.611, 0.611, 0.611, 0.611, 0.389, 0.556, 0.333, 0.611, 0.556, 0.778, 0.556, 0.556, 0.500, 0.389, 0.280, 0.389, 0.584, 0.0}, 
				-0.207
			}
		}
	}
};
