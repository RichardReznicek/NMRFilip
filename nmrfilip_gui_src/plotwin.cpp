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

#include "plotwin.h"

#include "plotgen.h"
#include "nmrdata.h"
#include "doc.h"


IMPLEMENT_CLASS(NFGGraphWindow, wxWindow)

BEGIN_EVENT_TABLE(NFGGraphWindow, wxWindow)
	EVT_MOUSE_EVENTS(NFGGraphWindow::OnMouse)
	EVT_KEY_DOWN(NFGGraphWindow::OnChar)
	EVT_SIZE(NFGGraphWindow::OnSize)
	EVT_SCROLLWIN(NFGGraphWindow::OnScroll)
END_EVENT_TABLE()

NFGGraphWindow::NFGGraphWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxWindow(parent, id, pos, size, (style | wxVSCROLL | wxHSCROLL) & (~wxALWAYS_SHOW_SB))
{
	Graph = NULL;

	SetBackgroundColour(*wxWHITE);

	Refreshing = false;
	WasFocusedByClick = false;
	Frozen = 0;
	ToBeRefreshed = false;
	
	CurveWindow = new NFGCurveWindow(this);
	XAxisWindow = new NFGXAxisWindow(this);
	YAxisWindow = new NFGYAxisWindow(this);
	CornerWindow = new NFGCornerWindow(this);

	GraphSizer = new wxFlexGridSizer(2, 2, 0, 0);

	GraphSizer->AddGrowableCol(1, 1);
	GraphSizer->AddGrowableRow(0, 1);
	
	GraphSizer->Add(YAxisWindow, 1, wxEXPAND);
	GraphSizer->Add(CurveWindow, 1, wxEXPAND);
	GraphSizer->Add(CornerWindow, 0, 0);
	GraphSizer->Add(XAxisWindow, 1, wxEXPAND);
	
	GraphSizer->SetItemMinSize(CornerWindow, FromDIP(AxisBarWidth), FromDIP(AxisBarWidth));
	GraphSizer->SetItemMinSize(CurveWindow, FromDIP(200), FromDIP(100));
	
	SetSizer(GraphSizer);
	
	if (parent != NULL) {
		wxSize MinClientSize = GraphSizer->GetMinSize();
		int sw = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);
		int sh = wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y, this);
		MinClientSize.IncBy(sw, sh);	/// reserve space for scrollbars

		wxSize MinWindowSize = parent->ClientToWindowSize(MinClientSize);
		parent->SetMinSize(MinWindowSize);
	}

	OldWinSize = wxSize(0, 0);
	Layout();
	
	SetBackgroundStyle(wxBG_STYLE_PAINT);
}

NFGGraphWindow::~NFGGraphWindow()
{
	if (Graph != NULL)
		Graph->SelectedInto(NULL);	/// normally should not be called
}

void NFGGraphWindow::OnSize(wxSizeEvent& event)
{
	Layout();
	
	if (event.GetSize() != OldWinSize)
		RefreshGraph();
	
	OldWinSize = event.GetSize();
}

void NFGGraphWindow::OnScroll(wxScrollWinEvent& event)
{
	int position = event.GetPosition();
	int thumbSize = GetScrollThumb(event.GetOrientation());
	int range = GetScrollRange(event.GetOrientation());

	int dx = 0;
	int dy = 0;
	
	if ((Graph != NULL) && (CurveWindow != NULL) && (XAxisWindow != NULL) && (YAxisWindow != NULL)) {
		wxSize ClientSize = Graph->GetClientSize();
		wxSize VirtualSize = Graph->GetVirtualSize();
		wxPoint ViewStart = Graph->GetViewStart();
		
		if (event.GetOrientation() == wxHORIZONTAL) {
			if (event.GetEventType() == wxEVT_SCROLLWIN_TOP) {
				dx = ViewStart.x;
			} else 
			if (event.GetEventType() == wxEVT_SCROLLWIN_BOTTOM) {
				dx = - (VirtualSize - ClientSize).GetWidth() + ViewStart.x;
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_LINEUP) {
				dx = FromDIP(PixelsPerLine);
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN) {
				dx = - FromDIP(PixelsPerLine);
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP) {
				dx = ClientSize.GetWidth();
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN) {
				dx = - ClientSize.GetWidth();
			} else
			if ((event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK) || (event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE)) {
				dx = - (int) ((((long long) position)*((long long) ((VirtualSize - ClientSize).GetWidth())))/((long long) (range - thumbSize))) + ViewStart.x;
			}
		} else {
			if (event.GetEventType() == wxEVT_SCROLLWIN_TOP) {
				dy = ViewStart.y;
			} else 
			if (event.GetEventType() == wxEVT_SCROLLWIN_BOTTOM) {
				dy = - (VirtualSize - ClientSize).GetHeight() + ViewStart.y;
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_LINEUP) {
				dy = FromDIP(PixelsPerLine);
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN) {
				dy = - FromDIP(PixelsPerLine);
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP) {
				dy = ClientSize.GetHeight();
			} else
			if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN) {
				dy = - ClientSize.GetHeight();
			} else
			if ((event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK) || (event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE)) {
				dy = - (int) ((((long long) position)*((long long) ((VirtualSize - ClientSize).GetHeight())))/((long long) (range - thumbSize))) + ViewStart.y;
			}
		}
		
		CurveWindow->Update();
		CurveWindow->ScrollWindow(dx, dy);
		if (dx != 0)
			XAxisWindow->RefreshGraph();
		if (dy != 0)
			YAxisWindow->RefreshGraph();
	}
	
	AdjustScrollPos(event.GetOrientation());
}

void NFGGraphWindow::OnMouse(wxMouseEvent& event)
{
	if ((event.GetEventType() == wxEVT_MOUSEWHEEL) && (Graph != NULL) && !event.AltDown()) {
		
		if (event.CmdDown() || event.ShiftDown()) {
			if (wxRect(GetClientSize()).Contains(event.GetPosition()) && (CurveWindow != NULL)) {
				wxPoint Position = CurveWindow->ScreenToClient(ClientToScreen(event.GetPosition()));

				if (wxRect(CurveWindow->GetClientSize()).Contains(Position)) {
					if (event.GetWheelRotation() >= event.GetWheelDelta())
						Graph->Zoom(true, event.CmdDown(), event.ShiftDown(), Position);

					if (event.GetWheelRotation() <= -event.GetWheelDelta())
						Graph->Zoom(false, event.CmdDown(), event.ShiftDown(), Position);
				}
			}
		} else {
			/// minimalistic approach
			if (event.GetWheelRotation() >= event.GetWheelDelta())
				Graph->SelectPrevStep();

			if (event.GetWheelRotation() <= -event.GetWheelDelta())
				Graph->SelectNextStep();
		}
	}
	
	if (event.GetEventType() == wxEVT_LEFT_DOWN) {
		SetFocus();
		WasFocusedByClick = true;
	}
}

void NFGGraphWindow::OnChar(wxKeyEvent& event)
{
	bool processed = false;
	
	if ((event.GetEventType() == wxEVT_KEY_DOWN) && (Graph != NULL) && !event.AltDown()) {
		
		if (event.CmdDown() || event.ShiftDown()) {
			if ((event.GetKeyCode() == WXK_PAGEUP) || (event.GetKeyCode() == WXK_NUMPAD_PAGEUP)) {
				Graph->Zoom(true, event.CmdDown(), event.ShiftDown());
				processed = true;
			}

			if ((event.GetKeyCode() == WXK_PAGEDOWN) || (event.GetKeyCode() == WXK_NUMPAD_PAGEDOWN)) {
				Graph->Zoom(false, event.CmdDown(), event.ShiftDown());
				processed = true;
			}
			
		} else {
			if ((event.GetKeyCode() == WXK_PAGEUP) || (event.GetKeyCode() == WXK_NUMPAD_PAGEUP)) {
				Graph->SelectPrevStep();
				processed = true;
			}

			if ((event.GetKeyCode() == WXK_PAGEDOWN) || (event.GetKeyCode() == WXK_NUMPAD_PAGEDOWN)) {
				Graph->SelectNextStep();
				processed = true;
			}
			
			
			if ((event.GetKeyCode() == WXK_LEFT) || (event.GetKeyCode() == WXK_NUMPAD_LEFT)) {
				wxScrollWinEvent ScrollEvt(0, 0, wxHORIZONTAL);
				ScrollEvt.SetEventType(wxEVT_SCROLLWIN_LINEUP);
				OnScroll(ScrollEvt);
				processed = true;
			}
			
			if ((event.GetKeyCode() == WXK_RIGHT) || (event.GetKeyCode() == WXK_NUMPAD_RIGHT)) {
				wxScrollWinEvent ScrollEvt(0, 0, wxHORIZONTAL);
				ScrollEvt.SetEventType(wxEVT_SCROLLWIN_LINEDOWN);
				OnScroll(ScrollEvt);
				processed = true;
			}
			
			if ((event.GetKeyCode() == WXK_UP) || (event.GetKeyCode() == WXK_NUMPAD_UP)) {
				wxScrollWinEvent ScrollEvt(0, 0, wxVERTICAL);
				ScrollEvt.SetEventType(wxEVT_SCROLLWIN_LINEUP);
				OnScroll(ScrollEvt);
				processed = true;
			}
			
			if ((event.GetKeyCode() == WXK_DOWN) || (event.GetKeyCode() == WXK_NUMPAD_DOWN)) {
				wxScrollWinEvent ScrollEvt(0, 0, wxVERTICAL);
				ScrollEvt.SetEventType(wxEVT_SCROLLWIN_LINEDOWN);
				OnScroll(ScrollEvt);
				processed = true;
			}
		}
		
	}
	
	if (!processed)
		event.Skip();
}


void NFGGraphWindow::AdjustScrollbars()
{
	int h_position = 0;
	int h_thumbSize = 0;
	int h_range = 0;
	
	int v_position = 0;
	int v_thumbSize = 0;
	int v_range = 0;
	
	if ((Graph != NULL) && (CurveWindow != NULL)) {
		wxSize ClientSize = Graph->GetClientSize();
		wxSize VirtualSize = Graph->GetVirtualSize();
		wxPoint ViewStart = Graph->GetViewStart();
		
		if (VirtualSize.GetWidth() > ClientSize.GetWidth()) {
			h_thumbSize = ClientSize.GetWidth()/FromDIP(PixelsPerLine);
			if (h_thumbSize < 1)	/// should never happen due to min size set
				h_thumbSize = 1;
			h_range = (VirtualSize.GetWidth() + FromDIP(PixelsPerLine) - 1)/FromDIP(PixelsPerLine);
			h_position = NFGMSTD lround(((double) ViewStart.x)/((double) (VirtualSize - ClientSize).GetWidth())*((double) (h_range - h_thumbSize)));
		}
		
		if (VirtualSize.GetHeight() > ClientSize.GetHeight()) {
			v_thumbSize = ClientSize.GetHeight()/FromDIP(PixelsPerLine);
			if (v_thumbSize < 1)	/// should never happen due to min size set
				v_thumbSize = 1;
			v_range = (VirtualSize.GetHeight() + FromDIP(PixelsPerLine) - 1)/FromDIP(PixelsPerLine);
			v_position = NFGMSTD lround(((double) ViewStart.y)/((double) (VirtualSize - ClientSize).GetHeight())*((double) (v_range - v_thumbSize)));
		}
	}
	
	SetScrollbar(wxHORIZONTAL, h_position, h_thumbSize, h_range);
	SetScrollbar(wxVERTICAL, v_position, v_thumbSize, v_range);
}


void NFGGraphWindow::AdjustScrollPos(int orientation)
{
	int position = 0;
	int thumbSize = GetScrollThumb(orientation);
	int range = GetScrollRange(orientation);
	
	if ((Graph != NULL) && (CurveWindow != NULL)) {
		wxSize ClientSize = Graph->GetClientSize();
		wxSize VirtualSize = Graph->GetVirtualSize();
		wxPoint ViewStart = Graph->GetViewStart();
		
		if (orientation == wxHORIZONTAL) 
			position = NFGMSTD lround(((double) ViewStart.x)/((double) (VirtualSize - ClientSize).GetWidth())*((double) (range - thumbSize)));
		else
			position = NFGMSTD lround(((double) ViewStart.y)/((double) (VirtualSize - ClientSize).GetHeight())*((double) (range - thumbSize)));
	}
	
	SetScrollPos(orientation, position);
}


void NFGGraphWindow::SelectGraph(NFGGraph* graph2select)
{
	FreezeGraph();
	
	if (Graph != NULL)
		Graph->SelectedInto(NULL);
	
	Graph = graph2select;
	
	if (Graph != NULL)
		Graph->SelectedInto(this);
	
	ThawGraph(true);
	RefreshGraph();
}


NFGGraph* NFGGraphWindow::GetGraph()
{
	return Graph;
}


void NFGGraphWindow::RefreshGraph()
{
	if (Frozen > 0) {
		ToBeRefreshed = true;
		return;
	}
	
	ToBeRefreshed = false;
	
	/// maybe not necessary
	if (Refreshing == true)
		return;
	Refreshing = true;
	
	if ((Graph != NULL) && (CurveWindow != NULL) && (GetParent() != NULL)) {
		wxSize ClientSize = GetSize();
		ClientSize.DecBy(FromDIP(AxisBarWidth));

		wxSize VirtualSize = Graph->SetScale(ClientSize);
		
		CurveWindow->SetVirtualSize(VirtualSize);

		AdjustScrollbars();
	}
	
	if (CurveWindow != NULL)
		CurveWindow->RefreshGraph();
	if (XAxisWindow != NULL)
		XAxisWindow->RefreshGraph();
	if (YAxisWindow != NULL)
		YAxisWindow->RefreshGraph();
	
	/// make sure that no residue of scrollbar remains in the area of corner window when the scrollbar gets hidden
	if (CornerWindow != NULL)
		CornerWindow->ClearBackground();

	Refreshing = false;
}


void NFGGraphWindow::FreezeGraph()
{
	Frozen++;
}

void NFGGraphWindow::ThawGraph(bool CancelPendingRefresh)
{
	Frozen--;
	if (Frozen < 0)
		Frozen = 0;
	
	if (CancelPendingRefresh)
		ToBeRefreshed = false;
	
	if (ToBeRefreshed && (Frozen == 0))
		RefreshGraph();
}


NFGCurveWindow* NFGGraphWindow::GetCurveWindow()
{
	return CurveWindow;
}

NFGXAxisWindow* NFGGraphWindow::GetXAxisWindow()
{
	return XAxisWindow;
}

NFGYAxisWindow* NFGGraphWindow::GetYAxisWindow()
{
	return YAxisWindow;
}

NFGCornerWindow* NFGGraphWindow::GetCornerWindow()
{
	return CornerWindow;
}

bool NFGGraphWindow::WasFocusedByUser()
{
	bool RetVal = WasFocusedByClick;
	WasFocusedByClick = false;
	return RetVal;
}



BEGIN_EVENT_TABLE(NFGCurveWindow, wxWindow)
	EVT_MOUSE_EVENTS(NFGCurveWindow::OnMouse)
	EVT_MOUSE_CAPTURE_LOST(NFGCurveWindow::OnMouseCaptureLost)
	EVT_PAINT(NFGCurveWindow::OnPaint)
END_EVENT_TABLE()

NFGCurveWindow::NFGCurveWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxWindow(parent, id, pos, size, style)
{
	BufferBmp = NULL;
	BufferDC = NULL;
	BufferRect.x = 0;
	BufferRect.y = 0;
	BufferRect.width = 0;
	BufferRect.height = 0;
	
	GraphWin = wxDynamicCast(parent, NFGGraphWindow);

	DraggingOrigin = wxPoint(-1, -1);
	Dragging = false;
	
	AxisPen = wxPen(*wxBLACK_PEN);
	AxisPen.SetCap(wxCAP_PROJECTING);
	if (FromDIP(1) > 1)
		AxisPen.SetWidth(FromDIP(1));
	
	SetBackgroundColour(*wxWHITE);
	SetBackgroundStyle(wxBG_STYLE_PAINT);
}

NFGCurveWindow::~NFGCurveWindow()
{
	FreeBuffers();
}

void NFGCurveWindow::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	if ((GraphWin == NULL) || (BufferDC == NULL)) {
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.Clear();
		return;
	}
	
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();

	if ((!BufferDC->IsOk()) || (Graph == NULL)) {
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.Clear();
		return;
	}
	
	wxPoint ViewStart = Graph->GetViewStart();
	
	wxSize ClientSize = GetClientSize();
	
	dc.SetDeviceOrigin(-ViewStart.x, -ViewStart.y);
	
	dc.Blit(ViewStart.x, ViewStart.y, ClientSize.GetWidth(), ClientSize.GetHeight(), BufferDC, ViewStart.x, ViewStart.y);
	
	NFGCurveSet curveset = Graph->GetOptimizedHighlightedCurveSet();

	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	
	for (unsigned long i = 0; i < curveset.CurveCount; i++) {
		dc.SetPen((curveset.ThickLines)?(curveset.CurveArray[i].PenThick):(curveset.CurveArray[i].Pen));
		wxPoint* PointArray = curveset.CurveArray[i].PointArray;
		unsigned long PointCount = curveset.CurveArray[i].PointCount;

		/// splitting the curves is a heuristic workaround for performance issue observed on wxp for many-point curves
		for (unsigned long j = 0; (j + 1) < PointCount; j += 127) 
			dc.DrawLines(((j + 127) < PointCount)?(128):(PointCount - j), PointArray + j);
		
		if (PointCount > 0)	/// draws the last point which wouldn't be drawn otherwise - see the wxWidget's documentation
			dc.DrawLine(PointArray[PointCount - 1], PointArray[PointCount - 1]);
			
		if (curveset.DrawPoints) 
			for (unsigned long j = 0; j < PointCount; j++)
				dc.DrawCircle(PointArray[j], FromDIP(curveset.PointRadius));
	}
		
	NFGPointSet pointset = Graph->GetOptimizedHighlightedPointSet();
	
	for (unsigned long i = 0; i < pointset.PointCount; i++) {
		dc.SetPen((pointset.ThickLines)?(pointset.PointArray[i].PenThick):(pointset.PointArray[i].Pen));
		dc.SetBrush(dc.GetPen().GetColour());
		
		dc.DrawCircle(pointset.PointArray[i].Point, FromDIP(pointset.PointRadius));
	}
	
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(*wxBLACK_PEN);
}


void NFGCurveWindow::OnMouse(wxMouseEvent& event)
{
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL) 
		Graph = GraphWin->GetGraph();
	
	if (Graph != NULL) {
		/// Zoom with fixed point - this code is typically reached under win 10
		if ((event.GetEventType() == wxEVT_MOUSEWHEEL) && !event.AltDown() && (event.CmdDown() || event.ShiftDown())) {
			if (wxRect(GetClientSize()).Contains(event.GetPosition())) {
				if (event.GetWheelRotation() >= event.GetWheelDelta())
					Graph->Zoom(true, event.CmdDown(), event.ShiftDown(), event.GetPosition());

				if (event.GetWheelRotation() <= -event.GetWheelDelta())
					Graph->Zoom(false, event.CmdDown(), event.ShiftDown(), event.GetPosition());
			}
		}

		/// Cursor position tracking
		if (event.GetEventType() == wxEVT_LEAVE_WINDOW)
			Graph->TrackMousePos(wxPoint(-1, -1));
		
		if (event.GetEventType() == wxEVT_MOTION)
			Graph->TrackMousePos(event.GetPosition());
		
		if (event.GetEventType() == wxEVT_RIGHT_DOWN)
			Graph->RegisterMousePos(event.GetPosition());
		
		if (event.GetEventType() == wxEVT_LEFT_DOWN) {
			if (event.CmdDown()) 
				Graph->CopyMousePos(event.GetPosition(), event.ShiftDown());
			else 
				Graph->DiscardMousePos();
		}
	}

	
	/// Start dragging
	if ((!Dragging) && (event.GetEventType() == wxEVT_MIDDLE_DOWN)) {
		if (wxRect(GetClientSize()).Contains(event.GetPosition())) {
			DraggingOrigin = event.GetPosition();
			Dragging = true;
			SetCursor(wxCursor(wxCURSOR_SIZING));
			//~ if (!HasCapture())
				CaptureMouse();
		}
	}
	
	/// Process dragging
	if (Dragging && (event.GetEventType() == wxEVT_MOTION) && event.Dragging() && event.MiddleIsDown()) {
		if (GraphWin != NULL) {
			int dx = event.GetPosition().x - DraggingOrigin.x;
			int dy = event.GetPosition().y - DraggingOrigin.y;
			
			Update();
			ScrollWindow(dx, dy);
			if ((dx != 0) && (GraphWin->GetXAxisWindow()))
				GraphWin->GetXAxisWindow()->RefreshGraph();
			if ((dy != 0) && (GraphWin->GetYAxisWindow()))
				GraphWin->GetYAxisWindow()->RefreshGraph();
			
			GraphWin->AdjustScrollPos(wxHORIZONTAL);
			GraphWin->AdjustScrollPos(wxVERTICAL);
			
			DraggingOrigin = event.GetPosition();
		}
	}
	
	/// End dragging
	if (Dragging && 
		( (event.GetEventType() == wxEVT_MIDDLE_UP) || 
		((event.GetEventType() == wxEVT_MOTION) && (!event.MiddleIsDown())) )
	) {
		DraggingOrigin = wxPoint(-1, -1);
		Dragging = false;
		SetCursor(wxNullCursor);
		ReleaseMouse();
	}

	/// Forward only specific events to NFGGraphWindow
	if (	(event.GetEventType() == wxEVT_LEFT_DOWN) || 
		((event.GetEventType() == wxEVT_MOUSEWHEEL) && !event.AltDown() && !event.CmdDown() && !event.ShiftDown())	) {
		event.ResumePropagation(1);
		event.Skip();
	}
}

void NFGCurveWindow::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
	/// End dragging
	DraggingOrigin = wxPoint(-1, -1);
	Dragging = false;
	SetCursor(wxNullCursor);

//	event.Skip();
}


void NFGCurveWindow::OnChar(wxKeyEvent& event)
{
	event.Skip();
}


void NFGCurveWindow::ScrollWindow(int dx, int dy, const wxRect* rect)
{
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();
	
	if (Graph != NULL) {
		wxPoint OldPos = Graph->GetViewStart();
		wxPoint NewPos = OldPos - wxPoint(dx, dy);
	
		wxSize ClientSize = GetClientSize();
		wxSize VirtualSize = GetVirtualSize();
		
		if ((NewPos.x + ClientSize.GetWidth() - 1) > (VirtualSize.GetWidth() - 1))
			NewPos.x = VirtualSize.GetWidth() - ClientSize.GetWidth();
		if (NewPos.x < 0)
			NewPos.x = 0;
	
		if ((NewPos.y + ClientSize.GetHeight() - 1) > (VirtualSize.GetHeight() - 1))
			NewPos.y = VirtualSize.GetHeight() - ClientSize.GetHeight();
		if (NewPos.y < 0)
			NewPos.y = 0;
		
		Graph->SetViewStart(NewPos);
		
		NewPos = Graph->GetViewStart();
		
		dx = - (NewPos.x - OldPos.x);
		dy = - (NewPos.y - OldPos.y);
	}
	
	UpdateBuffers();
	
	wxWindow::ScrollWindow(dx, dy, NULL);
}


void NFGCurveWindow::FreeBuffers()
{
	if (BufferDC != NULL) {
		BufferDC->SelectObject(wxNullBitmap);
		delete BufferDC;
		BufferDC = NULL;
	}
	
	if (BufferBmp != NULL) {
		delete BufferBmp;
		BufferBmp = NULL;
	}
	
	BufferRect.x = 0;
	BufferRect.y = 0;
	BufferRect.width = 0;
	BufferRect.height = 0;
}


void NFGCurveWindow::DrawToBufferDC(NFGGraph* Graph)
{
	if ((Graph == NULL) || (BufferDC == NULL) || (!BufferDC->IsOk())) 
		return;
	
	BufferDC->SetBackground(*wxWHITE_BRUSH);
	BufferDC->Clear();
	
	NFGCurveSet curveset = Graph->GetOptimizedCurveSet();
	
	BufferDC->SetBrush(*wxTRANSPARENT_BRUSH);
	
	for (unsigned long i = 0; i < curveset.CurveCount; i++) {
		BufferDC->SetPen((curveset.ThickLines)?(curveset.CurveArray[i].PenThick):(curveset.CurveArray[i].Pen));
		wxPoint* PointArray = curveset.CurveArray[i].PointArray;
		unsigned long PointCount = curveset.CurveArray[i].PointCount;

		/// splitting the curves is a heuristic workaround for performance issue observed on wxp for many-point curves
		for (unsigned long j = 0; (j + 1) < PointCount; j += 127) 
			BufferDC->DrawLines(((j + 127) < PointCount)?(128):(PointCount - j), PointArray + j);
		
		if (PointCount > 0)	/// draws the last point which wouldn't be drawn otherwise - see the wxWidget's documentation
			BufferDC->DrawLine(PointArray[PointCount - 1], PointArray[PointCount - 1]);
			
		if (curveset.DrawPoints) 
			for (unsigned long j = 0; j < PointCount; j++)
				BufferDC->DrawCircle(PointArray[j], FromDIP(curveset.PointRadius));
	}
	
	BufferDC->SetBrush(*wxWHITE_BRUSH);
	
	NFGAxis XAxis = Graph->GetXAxis(FromDIP(50));
	NFGAxis YAxis = Graph->GetYAxis(FromDIP(50));
	
	BufferDC->SetPen(AxisPen);
	
	if (XAxis.ShowZeroAxis) {
		BufferDC->DrawLine(BufferRect.x, XAxis.ZeroAxisPos, BufferRect.x + BufferRect.width, XAxis.ZeroAxisPos);
	
		for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++) 
			BufferDC->DrawLine(XAxis.MajorTics[i], XAxis.ZeroAxisPos + FromDIP(5), XAxis.MajorTics[i], XAxis.ZeroAxisPos - FromDIP(5));
		
		for (unsigned long i = 0; i < XAxis.MinorTicsCount; i++) 
			BufferDC->DrawLine(XAxis.MinorTics[i], XAxis.ZeroAxisPos + FromDIP(3), XAxis.MinorTics[i], XAxis.ZeroAxisPos - FromDIP(3));
	}

	if (YAxis.ShowZeroAxis) {
		BufferDC->DrawLine(YAxis.ZeroAxisPos, BufferRect.y, YAxis.ZeroAxisPos, BufferRect.y + BufferRect.height);
	
		for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++) 
			BufferDC->DrawLine(YAxis.ZeroAxisPos + FromDIP(5), YAxis.MajorTics[i], YAxis.ZeroAxisPos - FromDIP(5), YAxis.MajorTics[i]);
		
		for (unsigned long i = 0; i < YAxis.MinorTicsCount; i++) 
			BufferDC->DrawLine(YAxis.ZeroAxisPos + FromDIP(3), YAxis.MinorTics[i], YAxis.ZeroAxisPos - FromDIP(3), YAxis.MinorTics[i]);
	}
	
	BufferDC->SetPen(*wxBLACK_PEN);
}

void NFGCurveWindow::RefreshGraph()
{
	if (Dragging) {	/// Unlikely
		DraggingOrigin = wxPoint(-1, -1);
		Dragging = false;
		SetCursor(wxNullCursor);
		ReleaseMouse();
	}

	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();
	
	if (Graph == NULL) {
		FreeBuffers();
		return;
	}
	
	BufferRect = Graph->GetBufferRect();

	if (BufferBmp == NULL) {
		BufferBmp = new wxBitmap(BufferRect.GetWidth(), BufferRect.GetHeight());
		if ((BufferBmp == NULL) || (!BufferBmp->IsOk())) {
			FreeBuffers();
			return;
		}
		
		if (BufferDC != NULL)
			BufferDC->SelectObject(*BufferBmp);
	}
	
	if (BufferDC == NULL) {
		BufferDC = new wxMemoryDC(*BufferBmp);
		if ((BufferDC == NULL) || (!BufferDC->IsOk())) {
			FreeBuffers();
			return;
		}
	}
	
	if (BufferDC->GetSize() != BufferRect.GetSize()) {
		BufferDC->SelectObject(wxNullBitmap);
		delete BufferBmp;
		BufferBmp = NULL;
		
		BufferBmp = new wxBitmap(BufferRect.GetWidth(), BufferRect.GetHeight());
		if ((BufferBmp == NULL) || (!BufferBmp->IsOk())) {
			FreeBuffers();
			return;
		}
		
		BufferDC->SelectObject(*BufferBmp);
		if (!BufferDC->IsOk()) {
			FreeBuffers();
			return;
		}
	}
	
	
	BufferDC->SetDeviceOrigin(-BufferRect.x, -BufferRect.y);

	BufferDC->DestroyClippingRegion();
	BufferDC->SetClippingRegion(BufferRect);

	DrawToBufferDC(Graph);

	Refresh();
	Update();	/// maybe not necessary
}


void NFGCurveWindow::UpdateBuffers()
{
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();
	
	if (Graph == NULL) {
		FreeBuffers();
		return;
	}
	
	wxRect VisibleRect(Graph->GetViewStart(), GetClientSize());
	
	if (BufferRect.Contains(VisibleRect))	/// BufferBmp is up to date
		return;
	
	wxRect OldBufferRect = BufferRect;
	BufferRect = Graph->GetBufferRect();

	wxBitmap* OldBufferBmp = BufferBmp;
	
	BufferBmp = new wxBitmap(BufferRect.GetWidth(), BufferRect.GetHeight());
	if (BufferBmp == NULL) {
		BufferBmp = OldBufferBmp;
		FreeBuffers();
		return;
	}
	
	if (!BufferBmp->IsOk()) {
		delete BufferBmp;
		BufferBmp = OldBufferBmp;
		FreeBuffers();
		return;
	}
	
	if (BufferDC == NULL) {
		BufferDC = new wxMemoryDC(*BufferBmp);
		if ((BufferDC == NULL) || (!BufferDC->IsOk())) {
			delete BufferBmp;
			BufferBmp = OldBufferBmp;
			FreeBuffers();
			return;
		}
	} else
		BufferDC->SelectObject(*BufferBmp);
	
	BufferDC->SetDeviceOrigin(-BufferRect.x, -BufferRect.y);

	/// It seems that the only option to set the clipping region as wxRegion (as needed below) is to use SetDeviceClippingRegion(), which requires device coordinates.
	wxRect ClipRect(
		BufferDC->LogicalToDeviceX(BufferRect.x), 
		BufferDC->LogicalToDeviceY(BufferRect.y), 
		BufferDC->LogicalToDeviceXRel(BufferRect.width), 
		BufferDC->LogicalToDeviceYRel(BufferRect.height)
	);
	
	wxRegion ClipRgn(ClipRect);
	
	BufferDC->DestroyClippingRegion();
	BufferDC->SetDeviceClippingRegion(ClipRgn);
		
	if ((OldBufferBmp != NULL) && OldBufferRect.Intersects(BufferRect)) {
		wxRect OBR = OldBufferRect;
		wxRect Intersection = OBR.Intersect(BufferRect);
		wxRect BmpIntersection = Intersection;
		BmpIntersection.Offset(- OldBufferRect.x, - OldBufferRect.y);
		
		BufferDC->DrawBitmap(
			OldBufferBmp->GetSubBitmap(BmpIntersection),
			Intersection.x,
			Intersection.y,
			false
		);
		
		/// see the note above
		wxRect DeviceIntersection(
			BufferDC->LogicalToDeviceX(Intersection.x), 
			BufferDC->LogicalToDeviceY(Intersection.y), 
			BufferDC->LogicalToDeviceXRel(Intersection.width), 
			BufferDC->LogicalToDeviceYRel(Intersection.height)
		);
		
		ClipRgn.Subtract(DeviceIntersection);
		
		BufferDC->DestroyClippingRegion();
		BufferDC->SetDeviceClippingRegion(ClipRgn);
	}
	
	delete OldBufferBmp;
	
	DrawToBufferDC(Graph);
}



BEGIN_EVENT_TABLE(NFGXAxisWindow, wxWindow)
	EVT_MOUSE_EVENTS(NFGXAxisWindow::OnMouse)
	EVT_PAINT(NFGXAxisWindow::OnPaint)
END_EVENT_TABLE()

NFGXAxisWindow::NFGXAxisWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxWindow(parent, id, pos, size, style)
{
	BufferBmp = NULL;
	BufferDC = NULL;
	BufferRect.x = 0;
	BufferRect.y = 0;
	BufferRect.width = 0;
	BufferRect.height = 0;
	
	GraphWin = wxDynamicCast(parent, NFGGraphWindow);

	AxisPen = wxPen(*wxBLACK_PEN);
	AxisPen.SetCap(wxCAP_PROJECTING);
	if (FromDIP(1) > 1)
		AxisPen.SetWidth(FromDIP(1));
	
	SetBackgroundColour(*wxWHITE);
	SetBackgroundStyle(wxBG_STYLE_PAINT);
}

NFGXAxisWindow::~NFGXAxisWindow()
{
	FreeBuffers();
}

void NFGXAxisWindow::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	if ((GraphWin == NULL) || (BufferDC == NULL)) {
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.Clear();
		return;
	}
	
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();

	if ((!BufferDC->IsOk()) || (Graph == NULL)) {
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.Clear();
		return;
	}
	
	wxPoint ViewStart = Graph->GetViewStart();
	ViewStart.y = 0;

	wxSize ClientSize = GetClientSize();
	
	dc.SetDeviceOrigin(-ViewStart.x, -ViewStart.y);
	
	dc.Blit(ViewStart.x, ViewStart.y, ClientSize.GetWidth(), ClientSize.GetHeight(), BufferDC, ViewStart.x, ViewStart.y);
}

void NFGXAxisWindow::OnMouse(wxMouseEvent& event)
{
	/// Forward only specific events to NFGGraphWindow
	if (	(event.GetEventType() == wxEVT_LEFT_DOWN) || 
		((event.GetEventType() == wxEVT_MOUSEWHEEL) && !event.AltDown() && !event.CmdDown() && !event.ShiftDown())	) {
		event.ResumePropagation(1);
		event.Skip();
	}
}

void NFGXAxisWindow::FreeBuffers()
{
	if (BufferDC != NULL) {
		BufferDC->SelectObject(wxNullBitmap);
		delete BufferDC;
		BufferDC = NULL;
	}
	
	if (BufferBmp != NULL) {
		delete BufferBmp;
		BufferBmp = NULL;
	}
	
	BufferRect.x = 0;
	BufferRect.y = 0;
	BufferRect.width = 0;
	BufferRect.height = 0;
}

void NFGXAxisWindow::RefreshGraph()
{
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();
	
	if (Graph == NULL) {
		FreeBuffers();
		return;
	}
	
	wxSize ClientSize = Graph->GetClientSize();
	ClientSize.SetHeight(GetClientSize().GetHeight());

	wxPoint ViewStart = Graph->GetViewStart();
	ViewStart.y = 0;

	BufferRect = wxRect(ViewStart, ClientSize);

	if (BufferBmp == NULL) {
		BufferBmp = new wxBitmap(BufferRect.GetWidth(), BufferRect.GetHeight());
		if (BufferBmp == NULL) {
			FreeBuffers();
			return;
		}
		
		if (!BufferBmp->IsOk()) {
			FreeBuffers();
			return;
		}
		
		if (BufferDC != NULL)
			BufferDC->SelectObject(*BufferBmp);
	}
	
	if (BufferDC == NULL) {
		BufferDC = new wxMemoryDC(*BufferBmp);
		if (BufferDC == NULL) {
			FreeBuffers();
			return;
		}
		
		if (!BufferDC->IsOk()) {
			FreeBuffers();
			return;
		}
	}
	
	if (BufferDC->GetSize() != BufferRect.GetSize()) {
		BufferDC->SelectObject(wxNullBitmap);
		delete BufferBmp;
		BufferBmp = NULL;
		
		BufferBmp = new wxBitmap(BufferRect.GetWidth(), BufferRect.GetHeight());
		if (BufferBmp == NULL) {
			FreeBuffers();
			return;
		}
		
		if (!BufferBmp->IsOk()) {
			FreeBuffers();
			return;
		}
		
		BufferDC->SelectObject(*BufferBmp);
		if (!BufferDC->IsOk()) {
			FreeBuffers();
			return;
		}
	}
	
	
	BufferDC->SetDeviceOrigin(-BufferRect.x, -BufferRect.y);

	BufferDC->DestroyClippingRegion();
	BufferDC->SetClippingRegion(BufferRect);

	BufferDC->SetBackground(*wxWHITE_BRUSH);
	BufferDC->Clear();
	
	BufferDC->SetPen(AxisPen);
	
	BufferDC->DrawLine(BufferRect.x, 0, BufferRect.x + BufferRect.width, 0);
	
	NFGAxis XAxis = Graph->GetXAxis(FromDIP(50));
	
	for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++) 
		BufferDC->DrawLine(XAxis.MajorTics[i], 0, XAxis.MajorTics[i], 2*ClientSize.GetHeight()/5 - 1);
	
	for (unsigned long i = 0; i < XAxis.MinorTicsCount; i++) 
		BufferDC->DrawLine( XAxis.MinorTics[i], 0, XAxis.MinorTics[i], ClientSize.GetHeight()/5 - 1);
	
	BufferDC->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	
	wxSize AxisLabelExtent = BufferDC->GetTextExtent(XAxis.AxisLabel);
	wxCoord AxisLabelLeft = BufferRect.GetRight() - AxisLabelExtent.GetWidth();
	BufferDC->DrawText(XAxis.AxisLabel, AxisLabelLeft, 2*ClientSize.GetHeight()/5);
	
	wxCoord LastLabelRight = BufferRect.x;
	
	if (XAxis.Labels != NULL) {
		for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++) {
			wxSize LabelExtent = BufferDC->GetTextExtent(XAxis.Labels[i]);
			wxCoord LabelLeft = XAxis.MajorTics[i] - LabelExtent.GetWidth()/2 /*- 1*/;
			if (LabelLeft > LastLabelRight) {
				LastLabelRight = LabelLeft + LabelExtent.GetWidth() + FromDIP(7);					
				if (LastLabelRight < AxisLabelLeft)
					BufferDC->DrawText(XAxis.Labels[i], LabelLeft, 2*ClientSize.GetHeight()/5);
			}
		}
	}

	BufferDC->SetPen(*wxBLACK_PEN);

	Refresh();
	Update();
}



BEGIN_EVENT_TABLE(NFGYAxisWindow, wxWindow)
	EVT_MOUSE_EVENTS(NFGYAxisWindow::OnMouse)
	EVT_PAINT(NFGYAxisWindow::OnPaint)
END_EVENT_TABLE()


NFGYAxisWindow::NFGYAxisWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxWindow(parent, id, pos, size, style)
{
	BufferBmp = NULL;
	BufferDC = NULL;
	BufferRect.x = 0;
	BufferRect.y = 0;
	BufferRect.width = 0;
	BufferRect.height = 0;
	
	GraphWin = wxDynamicCast(parent, NFGGraphWindow);

	AxisPen = wxPen(*wxBLACK_PEN);
	AxisPen.SetCap(wxCAP_PROJECTING);
	if (FromDIP(1) > 1)
		AxisPen.SetWidth(FromDIP(1));
	
	SetBackgroundColour(*wxWHITE);
	SetBackgroundStyle(wxBG_STYLE_PAINT);
}

NFGYAxisWindow::~NFGYAxisWindow()
{
	FreeBuffers();
}

void NFGYAxisWindow::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	if ((GraphWin == NULL) || (BufferDC == NULL)) {
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.Clear();
		return;
	}
	
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();

	if ((!BufferDC->IsOk()) || (Graph == NULL)) {
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.Clear();
		return;
	}
	
	wxPoint ViewStart = Graph->GetViewStart();
	ViewStart.x = 0;

	wxSize ClientSize = GetClientSize();
	
	dc.SetDeviceOrigin(-ViewStart.x, -ViewStart.y);
	
	dc.Blit(ViewStart.x, ViewStart.y, ClientSize.GetWidth(), ClientSize.GetHeight(), BufferDC, ViewStart.x, ViewStart.y);
}

void NFGYAxisWindow::OnMouse(wxMouseEvent& event)
{
	/// Forward only specific events to NFGGraphWindow
	if (	(event.GetEventType() == wxEVT_LEFT_DOWN) || 
		((event.GetEventType() == wxEVT_MOUSEWHEEL) && !event.AltDown() && !event.CmdDown() && !event.ShiftDown())	) {
		event.ResumePropagation(1);
		event.Skip();
	}
}

void NFGYAxisWindow::FreeBuffers()
{
	if (BufferDC != NULL) {
		BufferDC->SelectObject(wxNullBitmap);
		delete BufferDC;
		BufferDC = NULL;
	}
	
	if (BufferBmp != NULL) {
		delete BufferBmp;
		BufferBmp = NULL;
	}
	
	BufferRect.x = 0;
	BufferRect.y = 0;
	BufferRect.width = 0;
	BufferRect.height = 0;
}

void NFGYAxisWindow::RefreshGraph()
{
	NFGGraph* Graph = NULL;
	if (GraphWin != NULL)
		Graph = GraphWin->GetGraph();
	
	if (Graph == NULL) {
		FreeBuffers();
		return;
	}
	
	wxSize ClientSize = Graph->GetClientSize();
	ClientSize.SetWidth(GetClientSize().GetWidth());

	wxPoint ViewStart = Graph->GetViewStart();
	ViewStart.x = 0;

	BufferRect = wxRect(ViewStart, ClientSize);

	if (BufferBmp == NULL) {
		BufferBmp = new wxBitmap(BufferRect.GetWidth(), BufferRect.GetHeight());
		if (BufferBmp == NULL) {
			FreeBuffers();
			return;
		}
		
		if (!BufferBmp->IsOk()) {
			FreeBuffers();
			return;
		}
		
		if (BufferDC != NULL)
			BufferDC->SelectObject(*BufferBmp);
	}
	
	if (BufferDC == NULL) {
		BufferDC = new wxMemoryDC(*BufferBmp);
		if (BufferDC == NULL) {
			FreeBuffers();
			return;
		}
		
		if (!BufferDC->IsOk()) {
			FreeBuffers();
			return;
		}
	}
	
	if (BufferDC->GetSize() != BufferRect.GetSize()) {
		BufferDC->SelectObject(wxNullBitmap);
		delete BufferBmp;
		BufferBmp = NULL;
		
		BufferBmp = new wxBitmap(BufferRect.GetWidth(), BufferRect.GetHeight());
		if (BufferBmp == NULL) {
			FreeBuffers();
			return;
		}
		
		if (!BufferBmp->IsOk()) {
			FreeBuffers();
			return;
		}
		
		BufferDC->SelectObject(*BufferBmp);
		if (!BufferDC->IsOk()) {
			FreeBuffers();
			return;
		}
	}
	
	
	BufferDC->SetDeviceOrigin(-BufferRect.x, -BufferRect.y);

	BufferDC->DestroyClippingRegion();
	BufferDC->SetClippingRegion(BufferRect);

	BufferDC->SetBackground(*wxWHITE_BRUSH);
	BufferDC->Clear();
	
	
	BufferDC->SetPen(AxisPen);
	
	BufferDC->DrawLine(ClientSize.GetWidth() - 1, BufferRect.y, ClientSize.GetWidth() - 1, BufferRect.y + BufferRect.height);
	
	NFGAxis YAxis = Graph->GetYAxis(FromDIP(50));
	
	for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++) 
		BufferDC->DrawLine(ClientSize.GetWidth() - 1, YAxis.MajorTics[i], ClientSize.GetWidth() - 2*ClientSize.GetWidth()/5, YAxis.MajorTics[i]);
	
	for (unsigned long i = 0; i < YAxis.MinorTicsCount; i++) 
		BufferDC->DrawLine(ClientSize.GetWidth() - 1, YAxis.MinorTics[i], ClientSize.GetWidth() - ClientSize.GetWidth()/5, YAxis.MinorTics[i]);

	BufferDC->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	
	wxSize AxisLabelExtent = BufferDC->GetTextExtent(YAxis.AxisLabel);
	wxCoord AxisLabelLeft = BufferRect.y + AxisLabelExtent.GetWidth() + 1;
	BufferDC->DrawRotatedText(YAxis.AxisLabel, 0, AxisLabelLeft, 90.0);
	
	wxCoord LastLabelRight = BufferRect.GetBottom();
	
	if (YAxis.Labels != NULL) {
		for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++) {
			wxSize LabelExtent = BufferDC->GetTextExtent(YAxis.Labels[i]);
			wxCoord LabelLeft = YAxis.MajorTics[i] + LabelExtent.GetWidth()/2 /*- 1*/;
			if (LabelLeft < LastLabelRight) {
				LastLabelRight = LabelLeft - LabelExtent.GetWidth() - FromDIP(7);
				if (LastLabelRight > AxisLabelLeft)
					BufferDC->DrawRotatedText(YAxis.Labels[i], 0, LabelLeft, 90.0);
			}
		}
	}
	
	BufferDC->SetPen(*wxBLACK_PEN);
	
	Refresh();
	Update();

}



BEGIN_EVENT_TABLE(NFGCornerWindow, wxWindow)
	EVT_MOUSE_EVENTS(NFGCornerWindow::OnMouse)
END_EVENT_TABLE()

NFGCornerWindow::NFGCornerWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxWindow(parent, id, pos, size, style)
{
	SetBackgroundColour(*wxWHITE);
}

NFGCornerWindow::~NFGCornerWindow()
{
	
}

void NFGCornerWindow::OnMouse(wxMouseEvent& event)
{
	/// Forward only specific events to NFGGraphWindow
	if (	(event.GetEventType() == wxEVT_LEFT_DOWN) || 
		((event.GetEventType() == wxEVT_MOUSEWHEEL) && !event.AltDown() && !event.CmdDown() && !event.ShiftDown())	) {
		event.ResumePropagation(1);
		event.Skip();
	}
}



/// pen style definitions for black and white print
const wxDash NFGGraphRenderer::NFG_dot[2] = {1, 2};
const int NFGGraphRenderer::NFG_dot_n = 2;

const wxDash NFGGraphRenderer::NFG_long_dash[2] = {8, 3};
const int NFGGraphRenderer::NFG_long_dash_n = 2;

const wxDash NFGGraphRenderer::NFG_short_dash[2] = {4, 3};
const int NFGGraphRenderer::NFG_short_dash_n = 2;

const wxDash NFGGraphRenderer::NFG_dot_dash[4] = {6, 3, 1, 3};
const int NFGGraphRenderer::NFG_dot_dash_n = 4;

void NFGGraphRenderer::AdjustPenStyle(wxPen &pen)
{
	/// translating generic pen styles to user defined ones in order to 
	/// provide consistent rendering across various DCs
	switch (pen.GetStyle()) {
		case wxDOT:
			pen.SetDashes(NFG_dot_n, NFG_dot);
			pen.SetStyle(wxPENSTYLE_USER_DASH);
			break;
		
		case wxLONG_DASH:
			pen.SetDashes(NFG_long_dash_n, NFG_long_dash);
			pen.SetStyle(wxPENSTYLE_USER_DASH);
			break;
		
		case wxSHORT_DASH:
			pen.SetDashes(NFG_short_dash_n, NFG_short_dash);
			pen.SetStyle(wxPENSTYLE_USER_DASH);
			break;
		
		case wxDOT_DASH:
			pen.SetDashes(NFG_dot_dash_n, NFG_dot_dash);
			pen.SetStyle(wxPENSTYLE_USER_DASH);
			break;
		
		default:
			;
	}
}

bool NFGGraphRenderer::RenderGraph(NFGGraph* Graph, wxDC* DC, AcquParams* AcquInfo, ProcParams* PParams, wxString Title, wxString Path, 
						int PointSize, int OutputDPI, int ScreenDPI, bool PrinterMargins, 
						bool BlackAndWhite, bool ParamsAndKeyAtRight, 
						bool PrintParams, bool PrintTitle, bool PrintKey, wxFontFamily FontFamily)
{
	if (Graph == NULL)
		return false;
	
	if (DC == NULL)
		return false;
	
	if (!DC->IsOk())
		return false;
	
	if (PrintParams && ((AcquInfo == NULL) || (PParams == NULL)))
		return false;
	
	if (PointSize < 4)
		return false;
	
	if (OutputDPI < 36)
		return false;
	
	if (ScreenDPI < 36)
		return false;
	
	BlackAndWhite = BlackAndWhite || (DC->GetDepth() == 1);
	
	PrintKey = PrintKey && (Graph->GetKeyItemCount() > 0);
	
	DC->DestroyClippingRegion();
	DC->SetBackground(*wxWHITE_BRUSH);
	DC->Clear();

	wxSize DCSize = DC->GetSize();
	wxRect DCRect(DCSize);
	wxSize ClientSize(DCSize);
	
	int FontPixelSize = wxRound(1.0*PointSize*OutputDPI/72.0);
	int ParamFontPixelSize = wxRound(0.7*PointSize*OutputDPI/72.0);
	int TitleFontPixelSize = wxRound(1.3*PointSize*OutputDPI/72.0);
	
	/// workaround for the wxW bug linking the font sizing always to screen DPI
	float FontPointSize = FontPixelSize*72.0/ScreenDPI;
	float ParamFontPointSize = ParamFontPixelSize*72.0/ScreenDPI;
	float TitleFontPointSize = TitleFontPixelSize*72.0/ScreenDPI;


	wxCoord LineWidth = wxRound(0.75*OutputDPI/72.0);	/// 3/4 pt
	if (LineWidth <= 0)
		LineWidth = 1;
	
	wxCoord AxisWidth = 2*FontPixelSize;

	wxCoord Margins = LineWidth/2;
	if (PrinterMargins)
		Margins += 3*OutputDPI/4;	/// 3/4 in
	
	wxCoord LMargin = Margins;
	wxCoord RMargin = Margins;
	wxCoord TMargin = Margins;
	wxCoord BMargin = Margins;

#ifndef __UNIX__
	wxPrinterDC *PrintDC = wxDynamicCast(DC, wxPrinterDC);
	if (PrintDC && PrinterMargins) {
		wxRect PrintRect = PrintDC->GetPaperRect();
		LMargin += PrintRect.GetLeft();
		if (LMargin < 0)
			LMargin = 0;
		TMargin += PrintRect.GetTop();
		if (TMargin < 0)
			TMargin = 0;
		
		RMargin += DCRect.GetRight() - PrintRect.GetRight();
		if (RMargin < 0)
			RMargin = 0;
		BMargin += DCRect.GetBottom() - PrintRect.GetBottom();
		if (BMargin < 0)
			BMargin = 0;
	}
#endif
	
	ClientSize.DecBy(LMargin + RMargin, TMargin + BMargin);
	
	wxPen pen = *wxBLACK_PEN;
	
	wxPen AxisPen = *wxBLACK_PEN;
	AxisPen.SetWidth(LineWidth);
	AxisPen.SetJoin(wxJOIN_MITER);
	AxisPen.SetCap(wxCAP_PROJECTING);
	DC->SetPen(AxisPen);
	
	wxSize TitleExtent(0, 0);
	wxCoord TitleHeight = 0;
	wxCoord ParamWidth = 0;
	wxCoord ParamHeight = 0;
	wxCoord ParamSpacing = ParamFontPixelSize;
	wxCoord KeyWidth = 0;
	wxCoord KeyHeight = 0;
	wxCoord KeyParamSpacing = (PrintKey && PrintParams)?(12*OutputDPI/72):(0);	/// 1/6 in
	wxCoord GraphParamKeySpacing = (PrintKey || PrintParams)?(12*OutputDPI/72):(0); /// 1/6 in
	wxCoord KeyLineLength = 36*OutputDPI/72;	/// 1/2 in


	if (PrintTitle) {
		wxFont font(wxRound(TitleFontPointSize), FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
		font.SetFractionalPointSize(TitleFontPointSize);
		DC->SetFont(font);
		
		TitleExtent = DC->GetTextExtent(Title);
		TitleHeight = 3*TitleExtent.GetHeight()/2;
	}
	
	ClientSize.DecBy(0, TitleHeight);


	if (PrintKey) {
		wxFont font(wxRound(FontPointSize), FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		font.SetFractionalPointSize(FontPointSize);
		DC->SetFont(font);
		
		long KeyItemCnt = Graph->GetKeyItemCount();
		for (long i = 0; i < KeyItemCnt; i++) {
			wxSize KeyLabelExtent = DC->GetTextExtent(Graph->GetKeyItem(i).Label);
			
			if (KeyLabelExtent.GetWidth() > KeyWidth)
				KeyWidth = KeyLabelExtent.GetWidth();
			
			KeyHeight += 3*KeyLabelExtent.GetHeight()/2;
		}
		
		KeyWidth += KeyLineLength + 3*FontPixelSize/4;
	}
	

	wxString ParamReport0 = wxEmptyString;
	wxString ParamReport1 = wxEmptyString;
	wxString ParamReport2 = wxEmptyString;
	wxString ParamReport3 = wxEmptyString;
	wxString ParamReport4 = wxEmptyString;
	
	wxRect ParamRect0, ParamRect1, ParamRect2, ParamRect3, ParamRect4; 
	
	if (PrintParams) {
		wxFont font(wxRound(ParamFontPointSize), FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		font.SetFractionalPointSize(ParamFontPointSize);
		DC->SetFont(font);
		
		if (AcquInfo->AcquFlag & ACQU_Acqus) {
			ParamReport0 += "> General information";
			
			ParamReport0 += "\nPath \"" + Path + "\"";
			wxString ATitle(AcquInfo->Title);
			ATitle.Replace("\t", " ");
			ParamReport0 += "\nAcqus title \"" + ATitle + "\"";
			ParamReport0 += "\nAcqus date " + wxDateTime((time_t) (AcquInfo->Date)).Format("%d %b %Y %H:%M:%S");
		}
		
		if ((AcquInfo->AcquFlag & ACQU_Userlist) || (AcquInfo->AcquFlag & ACQU_vlist)) {
			wxString units = wxEmptyString;
			
			ParamReport1 += "> Experiment details";
			
			if (AcquInfo->AssocValueUnits != NULL)
				units = " " + wxString(AcquInfo->AssocValueUnits);
			
			if (AcquInfo->AssocValueType == ASSOC_VARIABLE) {
					ParamReport1 += "\nVariable ";
					if (AcquInfo->AssocValueVariable != NULL)
						ParamReport1 += wxString(AcquInfo->AssocValueVariable);
			} else {
				if (AcquInfo->AssocValueTypeName != NULL) 
					ParamReport1 += "\n" + wxString(AcquInfo->AssocValueTypeName);
				else 
					ParamReport1 += "\nUnknown";
			}

			ParamReport1 += " experiment";
			
			if ((AcquInfo->AssocValueType != ASSOC_FREQ_MHZ) && (AcquInfo->AssocValueType != ASSOC_VARIABLE) && (AcquInfo->AssocValueVariable != NULL)) {
				ParamReport1 += "\nVariable ";
				ParamReport1 += wxString(AcquInfo->AssocValueVariable);
			}

			if (AcquInfo->AcquFlag & ACQU_Counts) {
				if ((AcquInfo->AcquFlag & ACQU_Userlist) && (AcquInfo->AssocValueType == ASSOC_FREQ_MHZ)) {
					if (AcquInfo->WobbStep == AcquInfo->StepCount) {
						ParamReport1 += "\nTuned before start";
					} else
					if (AcquInfo->WobbStep > AcquInfo->StepCount) {
						ParamReport1 += "\nNot tuned";
					} else 
					if (AcquInfo->WobbStep == 1) {
						ParamReport1 += "\nTuned every step";
					} else {
						ParamReport1 += wxString::Format("\nTuned every %ld steps", AcquInfo->WobbStep);
					}
				}
			}
			
			ParamReport1 += wxString::Format("\nFrom %.6g to %.6g", AcquInfo->AssocValueMin, AcquInfo->AssocValueMax) + units;
			if (AcquInfo->AcquFlag & ACQU_Counts) 
				ParamReport1 += wxString::Format("\nNumber of steps %ld", AcquInfo->StepCount);
			if (AcquInfo->AcquFlag & ACQU_Userlist) {
				ParamReport1 += wxString::Format("\nStep %.12g", AcquInfo->AssocValueStep) + units;
				ParamReport1 += wxString::Format("\nCoefficient %.6g", AcquInfo->AssocValueCoef);
			}
		}
		
		if (AcquInfo->AcquFlag & ACQU_Acqus) {
			
			ParamReport2 += "> Pulse sequence details";

			ParamReport2 += "\nPulse program " + wxString(AcquInfo->PulProg);
			ParamReport2 += wxString::Format("\nSample rate %.6g MSps", 1.0e-6*AcquInfo->SWh);
			ParamReport2 += wxString::Format("\nFilter width %.12g kHz", 1.0e-3*AcquInfo->FW);
			ParamReport2 += wxString::Format("\nReceiver gain %.12g", AcquInfo->RG);
			ParamReport2 += wxString::Format("\nFrequency %.12g MHz", AcquInfo->Freq);
			ParamReport2 += wxString::Format("\nNumber of scans %ld", AcquInfo->NS);
			
			if (AcquInfo->Dlength > 6) {
				ParamReport2 += wxString::Format("\nTrigger delay D1 %.12g s", AcquInfo->D[1]);
				ParamReport2 += wxString::Format("\nRingdown delay D3 %.12g us", 1.0e6*AcquInfo->D[3]);
				ParamReport2 += wxString::Format("\nDelay D6 (1/2 of echo) %.12g us", 1.0e6*AcquInfo->D[6]);
			}
			
			ParamReport2 += wxString::Format("\nPre-scan delay DE %.12g us", AcquInfo->DE);
			
			if (AcquInfo->AcquFlag & ACQU_Counts) {
				ParamReport2 += wxString::Format("\nNumber of chunks %ld", AcquInfo->ChunkCount);
			}
			

			ParamReport3 += "> Pulse parameters";

			if (AcquInfo->Plength > 4) {
				ParamReport3 += "\n>> Pulse lengths";
				ParamReport3 += wxString::Format("\nP1 (echo 90) %.12g us", AcquInfo->P[1]);
				ParamReport3 += wxString::Format("\nP2 (echo 180) %.12g us", AcquInfo->P[2]);
				ParamReport3 += wxString::Format("\nP3 (cpmg 90) %.12g us", AcquInfo->P[3]);
				ParamReport3 += wxString::Format("\nP4 (cpmg 180) %.12g us", AcquInfo->P[4]);
			}
			
			if (AcquInfo->PLWlength > 22) {
				ParamReport3 += "\n>> RF power levels";
				ParamReport3 += wxString::Format("\nPLW1 %.12g W", AcquInfo->PLW[1]);
				ParamReport3 += wxString::Format("\nPLW2 %.12g W", AcquInfo->PLW[2]);
				ParamReport3 += wxString::Format("\nPLW21 %.12g W", AcquInfo->PLW[21]);
				ParamReport3 += wxString::Format("\nPLW22 %.12g W", AcquInfo->PLW[22]);
			} else
			if (AcquInfo->PLlength > 22) {
				ParamReport3 += "\n>> RF power levels";
				ParamReport3 += wxString::Format("\nPL1 %.12g dB", AcquInfo->PL[1]);
				ParamReport3 += wxString::Format("\nPL2 %.12g dB", AcquInfo->PL[2]);
				ParamReport3 += wxString::Format("\nPL21 %.12g dB", AcquInfo->PL[21]);
				ParamReport3 += wxString::Format("\nPL22 %.12g dB", AcquInfo->PL[22]);
			}
		}
		
		ParamReport4 += "> Processing parameters";
		ParamReport4 += wxString::Format("\nProcessed chunks %ld - %ld", PParams->FirstChunk, PParams->LastChunk);

		if (PParams->UseFirstLastChunkPoint)
			ParamReport4 += wxString::Format("\nProcessed chunk points %ld - %ld", PParams->FirstChunkPoint, PParams->LastChunkPoint);
		else
			ParamReport4 += "\nProcessed all chunk points";
		ParamReport4+= wxString::Format("\nFFT length %ld", PParams->FFTLength);

		if (PParams->UseFilter)
			ParamReport4 += wxString::Format("\nSW filter width %.12g kHz", 1.0e3*PParams->Filter);
		else
			ParamReport4 += "\nSW filter not applied";

		
		/// Simple text layout

		ParamRect0 = wxRect(DC->GetMultiLineTextExtent(ParamReport0));
		ParamRect1 = wxRect(DC->GetMultiLineTextExtent(ParamReport1));
		ParamRect2 = wxRect(DC->GetMultiLineTextExtent(ParamReport2));
		ParamRect3 = wxRect(DC->GetMultiLineTextExtent(ParamReport3));
		ParamRect4 = wxRect(DC->GetMultiLineTextExtent(ParamReport4));

		
		/// There are several layout scenarios to try.
		bool ParamLayoutChosen = false;
		if (ParamsAndKeyAtRight) {
		/// Minimal width is the target; height is the constraint.
			if (!ParamLayoutChosen) {
				//~ ===========0 General===========
				
				//~ =1 Experiment=
				//~ ==============
				
				//~ ==2 Sequence==
				//~ ==============
				//~ ==============
				//~ ==============
				
				//~ ===3 Pulses===
				//~ ==============
				//~ ==============
				//~ ==============
				
				//~ =4 Processing=
				//~ ==============
				
				ParamWidth = ParamRect0.GetWidth();
				if (ParamRect1.GetWidth() > ParamWidth)
					ParamWidth = ParamRect1.GetWidth();
				if (ParamRect2.GetWidth() > ParamWidth)
					ParamWidth = ParamRect2.GetWidth();
				if (ParamRect3.GetWidth() > ParamWidth)
					ParamWidth = ParamRect3.GetWidth();
				if (ParamRect4.GetWidth() > ParamWidth)
					ParamWidth = ParamRect4.GetWidth();
				
				ParamHeight = ParamRect0.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect1.SetY(ParamHeight);
				ParamHeight += ParamRect1.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect2.SetY(ParamHeight);
				ParamHeight += ParamRect2.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect3.SetY(ParamHeight);
				ParamHeight += ParamRect3.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect4.SetY(ParamHeight);
				ParamHeight += ParamRect4.GetHeight();
				
				if ((ParamHeight + KeyParamSpacing + KeyHeight) <= ClientSize.GetHeight()) 
					ParamLayoutChosen = true;
				else {	/// undo the layout
					ParamRect0.SetX(0);
					ParamRect0.SetY(0);
					ParamRect1.SetX(0);
					ParamRect1.SetY(0);
					ParamRect2.SetX(0);
					ParamRect2.SetY(0);
					ParamRect3.SetX(0);
					ParamRect3.SetY(0);
					ParamRect4.SetX(0);
					ParamRect4.SetY(0);
				}
			}
				
			if (!ParamLayoutChosen) {
				//~ ===========0 General===========
				
				//~ =1 Experiment=
				//~ ==============
				
				//~ ==2 Sequence==   ===3 Pulses===
				//~ ==============   ==============
				//~ ==============   ==============
				//~ ==============   ==============
				
				//~ =4 Processing=
				//~ ==============
				
				ParamWidth = ParamRect0.GetWidth();
				if (ParamRect1.GetWidth() > ParamWidth)
					ParamWidth = ParamRect1.GetWidth();
				if ((ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth()) > ParamWidth)
					ParamWidth = ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth();
				if (ParamRect4.GetWidth() > ParamWidth)
					ParamWidth = ParamRect4.GetWidth();
				
				ParamHeight = ParamRect0.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect1.SetY(ParamHeight);
				ParamHeight += ParamRect1.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect2.SetY(ParamHeight);
				ParamRect3.SetY(ParamHeight);
				ParamRect3.SetX(ParamRect2.GetWidth() + ParamSpacing);
				ParamHeight += (ParamRect2.GetHeight() > ParamRect3.GetHeight())?(ParamRect2.GetHeight()):(ParamRect3.GetHeight());
				ParamHeight += ParamFontPixelSize;
				ParamRect4.SetY(ParamHeight);
				ParamHeight += ParamRect4.GetHeight();
				
				if ((ParamHeight + KeyParamSpacing + KeyHeight) <= ClientSize.GetHeight()) 
					ParamLayoutChosen = true;
				else {	/// undo the layout
					ParamRect0.SetX(0);
					ParamRect0.SetY(0);
					ParamRect1.SetX(0);
					ParamRect1.SetY(0);
					ParamRect2.SetX(0);
					ParamRect2.SetY(0);
					ParamRect3.SetX(0);
					ParamRect3.SetY(0);
					ParamRect4.SetX(0);
					ParamRect4.SetY(0);
				}
			}
				
			if (!ParamLayoutChosen) {
				//~ ===========0 General===========
				
				//~ =1 Experiment=   =4 Processing=
				//~ ==============   ==============
				
				//~ ==2 Sequence==   ===3 Pulses===
				//~ ==============   ==============
				//~ ==============   ==============
				//~ ==============   ==============
				
				ParamWidth = ParamRect0.GetWidth();
				if ((ParamRect1.GetWidth() + ParamSpacing + ParamRect4.GetWidth()) > ParamWidth)
					ParamWidth = ParamRect1.GetWidth() + ParamSpacing + ParamRect4.GetWidth();
				if ((ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth()) > ParamWidth)
					ParamWidth = ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth();
				
				ParamHeight = ParamRect0.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect1.SetY(ParamHeight);
				ParamRect4.SetY(ParamHeight);
				ParamRect4.SetX(ParamRect1.GetWidth() + ParamSpacing);
				ParamHeight += (ParamRect1.GetHeight() > ParamRect4.GetHeight())?(ParamRect1.GetHeight()):(ParamRect4.GetHeight());
				ParamHeight += ParamFontPixelSize;
				ParamRect2.SetY(ParamHeight);
				ParamRect3.SetY(ParamHeight);
				ParamRect3.SetX(ParamRect2.GetWidth() + ParamSpacing);
				ParamHeight += (ParamRect2.GetHeight() > ParamRect3.GetHeight())?(ParamRect2.GetHeight()):(ParamRect3.GetHeight());
				
				if ((ParamHeight + KeyParamSpacing + KeyHeight) <= ClientSize.GetHeight()) 
					ParamLayoutChosen = true;
				else {	/// undo the layout
					ParamRect0.SetX(0);
					ParamRect0.SetY(0);
					ParamRect1.SetX(0);
					ParamRect1.SetY(0);
					ParamRect2.SetX(0);
					ParamRect2.SetY(0);
					ParamRect3.SetX(0);
					ParamRect3.SetY(0);
					ParamRect4.SetX(0);
					ParamRect4.SetY(0);
				}
			}

		} else {
		/// Minimal height is the target; width is the constraint.
			if (!ParamLayoutChosen) {
				//~ ===========0 General===========   ==2 Sequence==   ===3 Pulses===
								  //~ ==============   ==============
				//~ =1 Experiment=   =4 Processing=   ==============   ==============
				//~ ==============   ==============   ==============   ==============
				
				ParamHeight = ParamRect0.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect1.SetY(ParamHeight);
				ParamRect4.SetY(ParamHeight);
				ParamHeight += (ParamRect1.GetHeight() > ParamRect4.GetHeight())?(ParamRect1.GetHeight()):(ParamRect4.GetHeight());
				if (ParamRect2.GetHeight() > ParamHeight)
					ParamHeight = ParamRect2.GetHeight();
				if (ParamRect3.GetHeight() > ParamHeight)
					ParamHeight = ParamRect3.GetHeight();
				
				ParamWidth = ParamRect0.GetWidth();
				if ((ParamRect1.GetWidth() + ParamSpacing + ParamRect4.GetWidth()) > ParamWidth)
					ParamWidth = ParamRect1.GetWidth() + ParamSpacing + ParamRect4.GetWidth();
				ParamWidth += ParamSpacing;
				ParamRect4.SetX(ParamRect1.GetWidth() + ParamSpacing);
				ParamRect2.SetX(ParamWidth);
				ParamWidth += ParamRect2.GetWidth();
				ParamWidth += ParamSpacing;
				ParamRect3.SetX(ParamWidth);
				ParamWidth += ParamRect3.GetWidth();
				
				if ((ParamWidth + KeyParamSpacing + KeyWidth) <= ClientSize.GetWidth()) 
					ParamLayoutChosen = true;
				else {	/// undo the layout
					ParamRect0.SetX(0);
					ParamRect0.SetY(0);
					ParamRect1.SetX(0);
					ParamRect1.SetY(0);
					ParamRect2.SetX(0);
					ParamRect2.SetY(0);
					ParamRect3.SetX(0);
					ParamRect3.SetY(0);
					ParamRect4.SetX(0);
					ParamRect4.SetY(0);
				}
			}
				
			if (!ParamLayoutChosen) {
				//~ ===========0 General===========   =1 Experiment=
								  //~ ==============
				//~ ==2 Sequence==   ===3 Pulses===   
				//~ ==============   ==============   =4 Processing=
				//~ ==============   ==============   ==============
				//~ ==============   ==============
				
				ParamHeight = ParamRect0.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect2.SetY(ParamHeight);
				ParamRect3.SetY(ParamHeight);
				ParamRect4.SetY(ParamRect1.GetHeight() + ParamFontPixelSize);
				ParamHeight += (ParamRect2.GetHeight() > ParamRect3.GetHeight())?(ParamRect2.GetHeight()):(ParamRect3.GetHeight());
				if ((ParamRect1.GetHeight() + ParamFontPixelSize + ParamRect4.GetHeight()) > ParamHeight)
					ParamHeight = ParamRect1.GetHeight() + ParamFontPixelSize + ParamRect4.GetHeight();
				
				ParamWidth = ParamRect0.GetWidth();
				if ((ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth()) > ParamWidth)
					ParamWidth = ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth();
				ParamWidth += ParamSpacing;
				ParamRect1.SetX(ParamWidth);
				ParamRect4.SetX(ParamWidth);
				ParamRect3.SetX(ParamRect2.GetWidth() + ParamSpacing);
				ParamWidth += (ParamRect1.GetWidth() > ParamRect4.GetWidth())?(ParamRect1.GetWidth()):(ParamRect4.GetWidth());
				
				if ((ParamWidth + KeyParamSpacing + KeyWidth) <= ClientSize.GetWidth()) 
					ParamLayoutChosen = true;
				else {	/// undo the layout
					ParamRect0.SetX(0);
					ParamRect0.SetY(0);
					ParamRect1.SetX(0);
					ParamRect1.SetY(0);
					ParamRect2.SetX(0);
					ParamRect2.SetY(0);
					ParamRect3.SetX(0);
					ParamRect3.SetY(0);
					ParamRect4.SetX(0);
					ParamRect4.SetY(0);
				}
			}
				
			if (!ParamLayoutChosen) {
				//~ ===========0 General===========
				
				//~ =1 Experiment=   =4 Processing=
				//~ ==============   ==============
				
				//~ ==2 Sequence==   ===3 Pulses===
				//~ ==============   ==============
				//~ ==============   ==============
				//~ ==============   ==============
				
				ParamHeight = ParamRect0.GetHeight();
				ParamHeight += ParamFontPixelSize;
				ParamRect1.SetY(ParamHeight);
				ParamRect4.SetY(ParamHeight);
				ParamHeight += (ParamRect1.GetHeight() > ParamRect4.GetHeight())?(ParamRect1.GetHeight()):(ParamRect4.GetHeight());
				ParamHeight += ParamFontPixelSize;
				ParamRect2.SetY(ParamHeight);
				ParamRect3.SetY(ParamHeight);
				ParamHeight += (ParamRect2.GetHeight() > ParamRect3.GetHeight())?(ParamRect2.GetHeight()):(ParamRect3.GetHeight());
				
				ParamWidth = ParamRect0.GetWidth();
				if ((ParamRect1.GetWidth() + ParamSpacing + ParamRect4.GetWidth()) > ParamWidth)
					ParamWidth = ParamRect1.GetWidth() + ParamSpacing + ParamRect4.GetWidth();
				if ((ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth()) > ParamWidth)
					ParamWidth = ParamRect2.GetWidth() + ParamSpacing + ParamRect3.GetWidth();
				ParamRect4.SetX(ParamRect1.GetWidth() + ParamSpacing);
				ParamRect3.SetX(ParamRect2.GetWidth() + ParamSpacing);
				
				if ((ParamWidth + KeyParamSpacing + KeyWidth) <= ClientSize.GetWidth()) 
					ParamLayoutChosen = true;
				else {	/// undo the layout
					ParamRect0.SetX(0);
					ParamRect0.SetY(0);
					ParamRect1.SetX(0);
					ParamRect1.SetY(0);
					ParamRect2.SetX(0);
					ParamRect2.SetY(0);
					ParamRect3.SetX(0);
					ParamRect3.SetY(0);
					ParamRect4.SetX(0);
					ParamRect4.SetY(0);
				}
			}
			
		}
		
		if (!ParamLayoutChosen) {
			::wxMessageBox("Requested image size is too small.", "Error", wxOK | wxICON_ERROR);
			return false;
		}
	}
	

	if (ParamsAndKeyAtRight) {
		ClientSize.DecBy(((ParamWidth > KeyWidth)?(ParamWidth):(KeyWidth)) + GraphParamKeySpacing, 0);
		
		if ((KeyHeight + KeyParamSpacing + ParamHeight) > ClientSize.GetHeight()) {	/// does not fit in the DC
			::wxMessageBox("Requested image size is too small.", "Error", wxOK | wxICON_ERROR );
			return false;
		}
	} else {
		ClientSize.DecBy(0, ((ParamHeight > KeyHeight)?(ParamHeight):(KeyHeight)) + GraphParamKeySpacing);
		
		if ((ParamWidth + KeyParamSpacing + KeyWidth) > ClientSize.GetWidth()) {	/// does not fit in the DC
			::wxMessageBox("Requested image size is too small.", "Error", wxOK | wxICON_ERROR );
			return false;
		}
	}
	
	if (TitleExtent.GetWidth() > ClientSize.GetWidth()) {	/// does not fit in the DC
		::wxMessageBox("Requested image size is too small.", "Error", wxOK | wxICON_ERROR );
		return false;
	}
	
	ClientSize.DecBy(AxisWidth);

	if ((ClientSize.GetWidth() < 100) || (ClientSize.GetHeight() < 100)) {
		::wxMessageBox("Requested image size is too small.", "Error", wxOK | wxICON_ERROR );
		return false;
	}
	

	/// Prepare the curves

	Graph->SetScale(ClientSize);
	wxPoint ViewStart = Graph->GetViewStart();
	
	NFGCurveSet curveset = Graph->GetOptimizedCurveSet();

	
	/// Start drawing
	
	DC->SetDeviceOrigin(0, 0);
	
	if (PrintTitle) {
		wxFont font(wxRound(TitleFontPointSize), FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
		font.SetFractionalPointSize(TitleFontPointSize);
		DC->SetFont(font);
		DC->DrawText(Title, LMargin + (ClientSize.GetWidth() + AxisWidth - TitleExtent.GetWidth())/2, TMargin + 0*TitleHeight/4);
	}
	
	if (PrintParams) {
		wxFont font(wxRound(ParamFontPointSize), FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		font.SetFractionalPointSize(ParamFontPointSize);
		DC->SetFont(font);
		
		if (ParamsAndKeyAtRight) {
			ParamRect0.Offset(LMargin + AxisWidth + ClientSize.GetWidth() + GraphParamKeySpacing, TMargin + TitleHeight + KeyHeight + KeyParamSpacing);
			ParamRect1.Offset(LMargin + AxisWidth + ClientSize.GetWidth() + GraphParamKeySpacing, TMargin + TitleHeight + KeyHeight + KeyParamSpacing);
			ParamRect2.Offset(LMargin + AxisWidth + ClientSize.GetWidth() + GraphParamKeySpacing, TMargin + TitleHeight + KeyHeight + KeyParamSpacing);
			ParamRect3.Offset(LMargin + AxisWidth + ClientSize.GetWidth() + GraphParamKeySpacing, TMargin + TitleHeight + KeyHeight + KeyParamSpacing);
			ParamRect4.Offset(LMargin + AxisWidth + ClientSize.GetWidth() + GraphParamKeySpacing, TMargin + TitleHeight + KeyHeight + KeyParamSpacing);
		} else {
			ParamRect0.Offset(LMargin, TMargin + AxisWidth + TitleHeight + ClientSize.GetHeight() + GraphParamKeySpacing);
			ParamRect1.Offset(LMargin, TMargin + AxisWidth + TitleHeight + ClientSize.GetHeight() + GraphParamKeySpacing);
			ParamRect2.Offset(LMargin, TMargin + AxisWidth + TitleHeight + ClientSize.GetHeight() + GraphParamKeySpacing);
			ParamRect3.Offset(LMargin, TMargin + AxisWidth + TitleHeight + ClientSize.GetHeight() + GraphParamKeySpacing);
			ParamRect4.Offset(LMargin, TMargin + AxisWidth + TitleHeight + ClientSize.GetHeight() + GraphParamKeySpacing);
		}
		
		DC->DrawLabel(ParamReport0, ParamRect0);
		DC->DrawLabel(ParamReport1, ParamRect1);
		DC->DrawLabel(ParamReport2, ParamRect2);
		DC->DrawLabel(ParamReport3, ParamRect3);
		DC->DrawLabel(ParamReport4, ParamRect4);
	}
	
	if (PrintKey) {
		wxFont font(wxRound(FontPointSize), FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		font.SetFractionalPointSize(FontPointSize);
		DC->SetFont(font);
		
		long KeyItemCnt = Graph->GetKeyItemCount();
		wxCoord item_x = LMargin + AxisWidth + ClientSize.GetWidth() + GraphParamKeySpacing;
		wxCoord item_y = TMargin + TitleHeight;
		
		if (!ParamsAndKeyAtRight) {
			item_x = LMargin + AxisWidth + ClientSize.GetWidth() - KeyWidth;	/// right alignment
			item_y = TMargin + AxisWidth + TitleHeight + ClientSize.GetHeight() + GraphParamKeySpacing;
		}

		for (long i = 0; i < KeyItemCnt; i++) {
			NFGKeyItem KeyItem = Graph->GetKeyItem(i);
			
			pen = (BlackAndWhite)?(KeyItem.PenBW):(KeyItem.Pen);
			pen.SetWidth(LineWidth);
			
			AdjustPenStyle(pen);

			DC->SetPen(pen);
			DC->DrawLine(item_x, item_y + KeyHeight/(3*KeyItemCnt), item_x + KeyLineLength, item_y + KeyHeight/(3*KeyItemCnt));
			if (KeyItem.DrawPoints) {
				DC->SetBrush(*wxTRANSPARENT_BRUSH);
				DC->DrawCircle(item_x + KeyLineLength/2, item_y + KeyHeight/(3*KeyItemCnt), KeyItem.PointRadius*LineWidth);
				DC->SetBrush(*wxWHITE_BRUSH);
			}

			DC->SetPen(AxisPen);
			DC->DrawText(KeyItem.Label, item_x + KeyLineLength + 3*FontPixelSize/4, item_y);
			item_y += KeyHeight/KeyItemCnt;
		}
	}
	
	wxFont font(wxRound(FontPointSize), FontFamily, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	font.SetFractionalPointSize(FontPointSize);
	DC->SetFont(font);
	
	/// Draw the curves
	
	DC->SetDeviceOrigin(-ViewStart.x + AxisWidth + LMargin, -ViewStart.y + TMargin + TitleHeight);

	/// to clip the curve area (including the parts covered by a graph border)
	wxRect ClipRect(
		ViewStart.x, 
		ViewStart.y, 
		ClientSize.GetWidth() - 1, 
		ClientSize.GetHeight() - 1
	);
	
	DC->DestroyClippingRegion();
	DC->SetClippingRegion(ClipRect);


	DC->SetBrush(*wxTRANSPARENT_BRUSH);
	
	for (unsigned long i = 0; i < curveset.CurveCount; i++) {
		pen = (BlackAndWhite)?(curveset.CurveArray[i].PenBW):(curveset.CurveArray[i].Pen);
		pen.SetWidth(LineWidth);
		AdjustPenStyle(pen);
		DC->SetPen(pen);
		
		wxPoint* PointArray = curveset.CurveArray[i].PointArray;
		unsigned long PointCount = curveset.CurveArray[i].PointCount;

		/// splitting the curves is a heuristic workaround for performance issue observed on wxp for many-point curves
		for (unsigned long j = 0; (j + 1) < PointCount; j += 127) 
			DC->DrawLines(((j + 127) < PointCount)?(128):(PointCount - j), PointArray + j);
		
		if (curveset.CurveArray[i].PointCount > 0)	/// draws the last point which wouldn't be drawn otherwise - see the wxWidget's documentation
			DC->DrawLine(PointArray[PointCount - 1], PointArray[PointCount - 1]);
			
		if (curveset.DrawPoints) 
			for (unsigned long j = 0; j < PointCount; j++)
				DC->DrawCircle(PointArray[j], curveset.PointRadius*LineWidth);
	}
	
	DC->SetBrush(*wxWHITE_BRUSH);

	
	/// Draw the axes
	
	NFGAxis XAxis = Graph->GetXAxis(48*OutputDPI/72, true);
	NFGAxis YAxis = Graph->GetYAxis(48*OutputDPI/72, true);
	
	DC->SetPen(AxisPen);
	
	if (XAxis.ShowZeroAxis) {
		DC->DrawLine(ViewStart.x, XAxis.ZeroAxisPos, ViewStart.x + ClientSize.GetWidth(), XAxis.ZeroAxisPos);
	
		for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++)
			DC->DrawLine(XAxis.MajorTics[i], XAxis.ZeroAxisPos + 5*LineWidth, XAxis.MajorTics[i], XAxis.ZeroAxisPos - 5*LineWidth);
		
		for (unsigned long i = 0; i < XAxis.MinorTicsCount; i++)
			DC->DrawLine(XAxis.MinorTics[i], XAxis.ZeroAxisPos + 3*LineWidth, XAxis.MinorTics[i], XAxis.ZeroAxisPos - 3*LineWidth);
	}

	if (YAxis.ShowZeroAxis) {
		DC->DrawLine(YAxis.ZeroAxisPos, ViewStart.y, YAxis.ZeroAxisPos, ViewStart.y + ClientSize.GetHeight());
	
		for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++)
			DC->DrawLine(YAxis.ZeroAxisPos + 5*LineWidth, YAxis.MajorTics[i], YAxis.ZeroAxisPos - 5*LineWidth, YAxis.MajorTics[i]);
		
		for (unsigned long i = 0; i < YAxis.MinorTicsCount; i++)
			DC->DrawLine(YAxis.ZeroAxisPos + 3*LineWidth, YAxis.MinorTics[i], YAxis.ZeroAxisPos - 3*LineWidth, YAxis.MinorTics[i]);
	}
	

	/// Draw the X-axis

	DC->SetDeviceOrigin(-ViewStart.x + AxisWidth + LMargin, ClientSize.GetHeight() - 1 + TMargin + TitleHeight);

	ClipRect = wxRect(
		ViewStart.x - LineWidth/2, 
		- LineWidth/2, 
		ClientSize.GetWidth() + LineWidth - 1, 
		AxisWidth + LineWidth
	);
	
	DC->DestroyClippingRegion();
	DC->SetClippingRegion(ClipRect);

	for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++)
		DC->DrawLine(XAxis.MajorTics[i], 0, XAxis.MajorTics[i], AxisWidth/3);
	
	for (unsigned long i = 0; i < XAxis.MinorTicsCount; i++)
		DC->DrawLine(XAxis.MinorTics[i], 0, XAxis.MinorTics[i], AxisWidth/6);
	
	ClipRect = wxRect(
		ViewStart.x - LineWidth/2 - AxisWidth/2, 
		- LineWidth/2, 
		ClientSize.GetWidth() + LineWidth - 1 + AxisWidth/2, 
		AxisWidth + LineWidth
	);
	
	DC->DestroyClippingRegion();
	DC->SetClippingRegion(ClipRect);
	
	wxSize AxisLabelExtent = DC->GetTextExtent(XAxis.AxisLabel);
	wxCoord AxisLabelLeft = ViewStart.x + ClientSize.GetWidth() - AxisLabelExtent.GetWidth();
	DC->DrawText(XAxis.AxisLabel, AxisLabelLeft, 3*AxisWidth/7);
	
	wxCoord LastLabelRight = ViewStart.x - AxisWidth/3;
	
	if (XAxis.Labels != NULL) {
		for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++) {
			wxSize LabelExtent = DC->GetTextExtent(XAxis.Labels[i]);
			wxCoord LabelLeft = XAxis.MajorTics[i] - LabelExtent.GetWidth()/2 /*- 1*/;
			if (LabelLeft > LastLabelRight) {
				LastLabelRight = LabelLeft + LabelExtent.GetWidth() + FontPixelSize/2;
				if (LastLabelRight < AxisLabelLeft) {
					DC->DrawText(XAxis.Labels[i], LabelLeft, 3*AxisWidth/7);
				}
			}
		}
	}

	/// Draw the Y-axis
	
	DC->SetDeviceOrigin(LMargin, -ViewStart.y + TMargin + TitleHeight);

	ClipRect = wxRect(
		- LineWidth/2, 
		ViewStart.y - LineWidth/2, 
		AxisWidth + LineWidth, 
		ClientSize.GetHeight() + LineWidth - 1
	);
	
	DC->DestroyClippingRegion();
	DC->SetClippingRegion(ClipRect);
	
	for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++)
		DC->DrawLine(AxisWidth, YAxis.MajorTics[i], AxisWidth - AxisWidth/3, YAxis.MajorTics[i]);
	
	for (unsigned long i = 0; i < YAxis.MinorTicsCount; i++)
		DC->DrawLine(AxisWidth, YAxis.MinorTics[i], AxisWidth - AxisWidth/6, YAxis.MinorTics[i]);
	
	ClipRect = wxRect(
		- LineWidth/2, 
		ViewStart.y - LineWidth/2, 
		AxisWidth + LineWidth, 
		ClientSize.GetHeight() + LineWidth - 1 + AxisWidth/2
	);
	
	DC->DestroyClippingRegion();
	DC->SetClippingRegion(ClipRect);

	wxCoord AxisLabelWidth, AxisLabelHeight, AxisLabelDescent;
	DC->GetTextExtent(YAxis.AxisLabel, &AxisLabelWidth, &AxisLabelHeight, &AxisLabelDescent);
	AxisLabelLeft = ViewStart.y + AxisLabelWidth + 1;
	DC->DrawRotatedText(YAxis.AxisLabel, AxisWidth - 3*AxisWidth/7 - (AxisLabelHeight - AxisLabelDescent) - FontPixelSize/5, AxisLabelLeft, 90.0);
	
	LastLabelRight = ViewStart.y + ClientSize.GetHeight() + AxisWidth/3;
	
	if (YAxis.Labels != NULL) {
		for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++) {
			wxCoord LabelWidth, LabelHeight, LabelDescent;
			DC->GetTextExtent(YAxis.Labels[i], &LabelWidth, &LabelHeight, &LabelDescent);
			wxCoord LabelLeft = YAxis.MajorTics[i] + LabelWidth/2 /*- 1*/;
			if (LabelLeft < LastLabelRight) {
				LastLabelRight = LabelLeft - LabelWidth - FontPixelSize/2;
				if ((LastLabelRight > AxisLabelLeft) && (YAxis.MajorTics[i] < (ClientSize.GetHeight() + ViewStart.y)))
					DC->DrawRotatedText(YAxis.Labels[i], AxisWidth - 3*AxisWidth/7 - (AxisLabelHeight - AxisLabelDescent) - FontPixelSize/5, LabelLeft, 90.0);
			}
		}
	}
	
	/// Draw the axes lines and a graph border

	DC->DestroyClippingRegion();
	DC->SetBrush(*wxTRANSPARENT_BRUSH);
	DC->SetDeviceOrigin(0, 0);
	wxPoint corners[4];
	corners[0] = wxPoint(AxisWidth + LMargin, TMargin + TitleHeight);
	corners[1] = wxPoint(AxisWidth + LMargin, ClientSize.GetHeight() - 1 + TMargin + TitleHeight);
	corners[2] = wxPoint(AxisWidth + LMargin + ClientSize.GetWidth() - 1, ClientSize.GetHeight() - 1 + TMargin + TitleHeight);
	corners[3] = wxPoint(AxisWidth + LMargin + ClientSize.GetWidth() - 1, TMargin + TitleHeight);
	/// avoiding the use of DrawRectangle due to the off-by-one-pixel problems on some DC types
	DC->DrawPolygon(4, corners);
	
	DC->SetPen(*wxBLACK_PEN);
	
	return true;
}
