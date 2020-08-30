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

#ifndef __plotwin__
#define __plotwin__

#include "wx_pch.h"

#if __cplusplus >= 201103L
#include <cstdint>
#else
#include <stdint.h>
#endif
#include <cmath>

#include "cd.h"
#include "plotwin_cd.h"

#include "doc_cd.h"
#include "plotgen_cd.h"
#include "nmrdata_cd.h"


#define AxisBarWidth	25
#define PixelsPerLine 10

class NFGGraphWindow : public wxWindow
{
	DECLARE_CLASS(NFGGraphWindow)
	
	DECLARE_EVENT_TABLE();
	private:
		wxSize OldWinSize;
	
		NFGGraph* Graph;

		bool Refreshing;
		bool WasFocusedByClick;
		long Frozen;
		bool ToBeRefreshed;

		NFGCurveWindow* CurveWindow;
		NFGXAxisWindow* XAxisWindow;
		NFGYAxisWindow* YAxisWindow;
		NFGCornerWindow* CornerWindow;
	
		wxFlexGridSizer* GraphSizer;
	
		void OnSize(wxSizeEvent& event);
		void OnScroll(wxScrollWinEvent& event);
		void OnMouse(wxMouseEvent& event);
		void OnChar(wxKeyEvent& event);
	
	public:
		NFGGraphWindow(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHSCROLL | wxVSCROLL);
		~NFGGraphWindow();
	
		void SelectGraph(NFGGraph* graph2select);
		NFGGraph* GetGraph();
	
		void RefreshGraph();
		void FreezeGraph();
		void ThawGraph(bool CancelPendingRefresh = false);
	
		NFGCurveWindow* GetCurveWindow();
		NFGXAxisWindow* GetXAxisWindow();
		NFGYAxisWindow* GetYAxisWindow();
		NFGCornerWindow* GetCornerWindow();
		
		void AdjustScrollbars();
		void AdjustScrollPos(int orientation);
	
		bool WasFocusedByUser();
};

class NFGCurveWindow : public wxWindow
{
	DECLARE_EVENT_TABLE();
	private:
		wxBitmap* BufferBmp;
		wxMemoryDC* BufferDC;
		wxRect BufferRect;
	
		NFGGraphWindow* GraphWin;

		wxPoint DraggingOrigin;
		bool Dragging;

		wxPen AxisPen;
	
		void OnPaint(wxPaintEvent& event);
		void OnMouse(wxMouseEvent& event);
		void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
		void OnChar(wxKeyEvent& event);
	
		void DrawToBufferDC(NFGGraph* Graph);

	public:
		NFGCurveWindow(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		~NFGCurveWindow();
	
		virtual void ScrollWindow(int dx, int dy, const wxRect* rect = NULL);
	
		void FreeBuffers();
		void RefreshGraph();
		void UpdateBuffers();
	
};

class NFGXAxisWindow : public wxWindow
{
	DECLARE_EVENT_TABLE();
	private:
		wxBitmap* BufferBmp;
		wxMemoryDC* BufferDC;
		wxRect BufferRect;

		NFGGraphWindow* GraphWin;
	
		wxPen AxisPen;
	
		void OnPaint(wxPaintEvent& event);
		void OnMouse(wxMouseEvent& event);
	
	public:
		NFGXAxisWindow(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		~NFGXAxisWindow();
	
		void FreeBuffers();
		void RefreshGraph();
};

class NFGYAxisWindow : public wxWindow
{
	DECLARE_EVENT_TABLE();
	private:
		wxBitmap* BufferBmp;
		wxMemoryDC* BufferDC;
		wxRect BufferRect;

		NFGGraphWindow* GraphWin;
	
		wxPen AxisPen;
	
		void OnPaint(wxPaintEvent& event);
		void OnMouse(wxMouseEvent& event);
	
	public:
		NFGYAxisWindow(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		~NFGYAxisWindow();
	
		void FreeBuffers();
		void RefreshGraph();
};

class NFGCornerWindow : public wxWindow
{
	DECLARE_EVENT_TABLE();
	private:
		void OnMouse(wxMouseEvent& event);

	public:
		NFGCornerWindow(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		~NFGCornerWindow();
};



class NFGGraphRenderer
{
	private:
		/// translating generic linestyles to user defined ones in order to 
		/// provide consistent rendering across various DCs
		static void AdjustPenStyle(wxPen &pen);
	
	public:
		/// linestyle definitions for black and white print
		static const wxDash NFG_dot[2];
		static const int NFG_dot_n;

		static const wxDash NFG_long_dash[2];
		static const int NFG_long_dash_n;

		static const wxDash NFG_short_dash[2];
		static const int NFG_short_dash_n;

		static const wxDash NFG_dot_dash[4];
		static const int NFG_dot_dash_n;

		static bool RenderGraph(NFGGraph* Graph, wxDC* DC, AcquParams* AcquInfo, ProcParams* PParams, 
							wxString Title = wxEmptyString, wxString Path = wxEmptyString, 
							int PointSize = 11, int OutputDPI = 72, int ScreenDPI = 72, bool PrinterMargins = false, 
							bool BlackAndWhite = false, bool ParamsAndKeyAtRight = true, 
							bool PrintParams = true, bool PrintTitle = true, bool PrintKey = true, 
							wxFontFamily FontFamily = wxFONTFAMILY_SWISS);
};

#endif
