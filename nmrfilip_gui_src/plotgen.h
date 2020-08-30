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

#ifndef __plotgen__
#define __plotgen__

#include "wx_pch.h"

#include "cd.h"
#include "plotgen_cd.h"

#include "doc_cd.h"
#include "plotwin_cd.h"
#include "nmrdata_cd.h"


struct NFGRealRect
{
	/// [x,y] is left bottom corner
	double x, y, width, height;
};

struct NFGRealBBox
{
	double minx, maxx, miny, maxy;
};

struct NFGScale
{
	/// xoffset - value assigned to the left edge of the plot
	/// xfactor - horizontal scaling factor
	/// yoffset - negative value assigned to the _top_ edge of the plot
	/// yfactor - negative vertical scaling factor
	long long xoffset, yoffset;
	double xfactor, yfactor;
};

struct NFGCurve
{
	wxPoint* PointArray;
	unsigned long PointCount;
	unsigned long ElisionCount;
	unsigned long BufferLength;
	bool NondecreasingX;
	wxRect BoundingBox;	/// ignoring linewidth and point radius
	wxPen Pen;
	wxPen PenThick;
	wxPen PenBW;	/// pen for black and white print
};

/// used just during curveset and pointset optimization
WX_DECLARE_OBJARRAY(NFGCurve, NFGCurveArray);
WX_DECLARE_OBJARRAY(NFGPoint, NFGPointArray);

WX_DECLARE_OBJARRAY(wxPoint, wxPointArray);

struct NFGCurveSet
{
	NFGCurve* CurveArray;
	unsigned long CurveCount;

	bool ThickLines;
	bool DrawPoints;
	wxCoord PointRadius;
};

/// Note the difference between NFGCurve + NFGCurveSet and NFGPoint + NFGPointSet.
struct NFGPoint
{
	wxPoint Point;
	wxPen Pen;
	wxPen PenThick;
	wxPen PenBW;
};

struct NFGPointSet
{
	NFGPoint* PointArray;
	unsigned long PointCount;
	
	bool ThickLines;
	wxCoord PointRadius;
};


struct NFGAxis
{
	wxCoord* MajorTics;
	unsigned long MajorTicsCount;
	wxString* Labels;
	
	wxCoord* MinorTics;
	unsigned long MinorTicsCount;
	
	wxString AxisLabel;
	wxCoord ZeroAxisPos;
	bool ShowZeroAxis;
};


struct NFGKeyItem
{
	wxString Label;
	wxPen Pen;	/// standard pen
	wxPen PenThick;	/// thick pen
	wxPen PenBW;	/// pen for black and white print
	wxPen AltPen;	/// alternative pen for suppressed parts of the curves
	wxPen AltPenThick;	/// alternative thick pen for suppressed parts of the curves
	wxPen AltPenBW;	/// alternative pen for black and white print of suppressed parts of the curves
	bool DrawPoints;
	wxCoord PointRadius;
	wxCoord HighlightedPointRadius;
	unsigned long DatasetFlag;
};


class NFGDataseries
{
	public:
		NFGCurve Curve;
		bool CurveValid;
		
		NFGScale CurveScale;
	
		NFGRealBBox RealBBox;
		bool RealBBoxValid;
		
		long No;
	
		NFGDataseries();
		~NFGDataseries();
	
		void FreeScaledPoints();
	
		void SimplifyCurve();
		long SimplifiedCurveIndex(long Index, bool Backwards = false);
};

class NFGDataseriesGroup
{
	private:
		NFGDataseries *DataseriesArray;
		unsigned long DataseriesCount;
		bool DataseriesValid;
		
		NFGRealBBox RealBBox;
		bool RealBBoxValid;
	
		bool GetDataseries();
		void FreeDataseries();
	
		bool DoGetRealBBox(NFGRealBBox &BBox, long Index = ALL_STEPS);
		
	protected:
		
	public:
		NMRData* NMRDataPointer;
		unsigned int WatchedNMRData;
		
		GetNMRPtsFuncPtr GetNMRPts;
		GetNMRRPtBBFuncPtr GetNMRRPtBB;
		GetNMRFlagFuncPtr GetNMRFlag;
		GetNMRIndexRangeFuncPtr GetNMRIndexRange;

		NFGKeyItem KeyItem;
		bool NumberedDataseries;
		bool NumberedCurvePoints;
	
		bool HasHeadAndTail;
	
		bool SymmetricYRange;
		bool IncludeYZero;
		bool NondecreasingX;
		bool DominantBBox;
	
		NFGDataseriesGroup();
		~NFGDataseriesGroup();
	
		unsigned long GetDataseriesCount();
		
		void FreeScaledPoints();
		void InvalidateScaledPoints(long No = ALL_STEPS);
	
		bool GetRealBBox(NFGRealBBox &BBox, long No = ALL_STEPS);
		void InvalidateRealBBox(long No = ALL_STEPS);

		bool GetCurve(NFGScale scale, long Index, NFGCurve &curve);
		bool GetCurve(NFGScale scale, long Index, NFGCurve &curvehead, NFGCurve &curvetail, NFGCurve &curvebody, long start, long end);

		bool GetPoint(NFGScale scale, long Index, long PointIndex, NFGPoint &point);
		
		void MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo);
		void ChangeProcParamCallback(unsigned int ParamType, long StepNo);
};


#define GraphStyle_Line	1
#define GraphStyle_Points	2
#define GraphStyle_Large	4
#define GraphStyle_Thick	8
// #define GraphStyle_X2Axis	16
// #define GraphStyle_Y2Axis	32
#define GraphStyle_Print	64
#define GraphStyle_BlackAndWhite	128

class NFGGraph
{
	private:
		void InvalidateScaledPoints();
		void FreeScaledPoints();
	
		wxRealPoint UnscrolledPoint2RealPoint(wxPoint point, bool CoarserRounding = false);

		void CheckConstrains(bool AllowMove = true); 	/// adjusts VisibleRealRect and applies changes
		
		void SetVisibleRealRect(NFGRealRect rect, bool AllowMove = true, bool Force = false);
		NFGRealRect GetVisibleRealRect();

	protected:
		NMRData* NMRDataPointer;
		NFGSerDocument* SerDoc;
		NFGGraphWindow* GraphWindow;
	
		unsigned long DisplayedDatasets;
		unsigned long DisplayedDatasetsMask;
	
		NFGDataseriesGroup* DataseriesGroupArray;
		unsigned long DataseriesGroupCount;
	
		unsigned long WatchedNMRFlags;
		unsigned int WatchedProcParamFlags;

		NFGRealRect VisibleRealRect;
		NFGRealRect RequestedVisibleRealRect;
		NFGRealRect GraphRealRect;
		NFGRealRect BoundingRealRect;
		bool BoundingRealRectValid;
	
		NFGScale ScaleValue;
		
		wxSize GWClientSize;
		wxSize GWVirtualSize;
		wxPoint GWViewStart;
	
		bool AutoZoomVSelected;
		bool AutoZoomVAll;
		bool AutoZoomHSelected;
		bool AutoZoomHAll;
		
		bool DrawPoints;
		bool ThickLines;

		bool ConstrainZoomLeft;
		bool ConstrainZoomRight;
		bool ConstrainZoomTop;
		bool ConstrainZoomBottom;
		
		bool ConstrainZoomLeftAllowOverride;
		bool ConstrainZoomRightAllowOverride;
		bool ConstrainZoomTopAllowOverride;
		bool ConstrainZoomBottomAllowOverride;
		
		NFGRealBBox ConstrainZoomOverride;

		bool StickyZoomLeft;
		bool StickyZoomBottom;
		
		bool DisableZoomInH;
		bool DisableZoomInV;
		
		/// Set DataError to true if data (curves or bounding boxes) cannot be acquired (typically due to memory allocation failure)
		/// to prevent further futile attempts and show a blank graph
		bool DataError;
	
		NFGCurveSet CurveSet;
		bool CurveSetValid;
		NFGCurveSet HighlightedCurveSet;
		bool HighlightedCurveSetValid;
		NFGPointSet HighlightedPointSet;
		bool HighlightedPointSetValid;
	
		NFGCurveSet OptimizedCurveSet;
		bool OptimizedCurveSetValid;
		wxRect OptimizedCurveSetRect;	/// reference rect used for optimization - not bounding box
		NFGCurveSet OptimizedHighlightedCurveSet;
		bool OptimizedHighlightedCurveSetValid;
		wxRect OptimizedHighlightedCurveSetRect;	/// reference rect used for optimization - not bounding box
		NFGPointSet OptimizedHighlightedPointSet;
		bool OptimizedHighlightedPointSetValid;
		wxRect OptimizedHighlightedPointSetRect;	/// reference rect used for optimization - not bounding box
	
		unsigned long SelectedStep;
		unsigned char StyleFlag;

		wxRect XAxisBufferRect;
		wxString XAxisUnits;
		bool XAxisZeroExponent;
		NFGAxis XAxis;
		bool XAxisValid;
		int XAxisMajorTicsDistMin;
		
		wxRect YAxisBufferRect;
		wxString YAxisUnits;
		bool YAxisZeroExponent;
		NFGAxis YAxis;
		bool YAxisValid;
		int YAxisMajorTicsDistMin;
		
		wxPoint MousePos;
		wxRealPoint ReferenceRealPoint;
		bool ReferenceRealPointValid;

		bool DoGetCurveSet(NFGCurveSet &CS, long No = ALL_STEPS, bool IsHighlighted = false, long start = -1, long end = -1);
		void DoGetOptimizedCurveSet(NFGCurveSet &OptimizedCS, wxRect &OptimizedCSRect, NFGCurveSet &InputCS);
		void DoGetHighlightedPointSet(NFGPointSet &PS);
		void DoSelectStep(unsigned long index, bool OnlySelectedPlotted = false);
		bool DoGetBoundingRealRect(NFGRealRect &BRealRect, long No = ALL_STEPS);
		
		virtual wxString GetXAxisUnits();
		virtual wxString GetYAxisUnits();

	public:
		NFGGraph(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style, unsigned long WatchNMRFlags = 0, unsigned long WatchProcParamFlags = 0);
		virtual ~NFGGraph();

		bool CheckData(bool refresh = false);

		virtual void MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo);
		virtual void ChangeProcParamCallback(unsigned int ParamType, long StepNo);
	
		unsigned long DisplayDatasets(unsigned long flag);
		unsigned long GetDisplayedDatasets();
		
		long GetKeyItemCount();
		NFGKeyItem GetKeyItem(long index, bool DrawingOrder = false);
	
		virtual wxString GetGraphLabel();
	
		/// Called by NFGGraphWindow immediately after selecting this into it - NFGGraph must know the NFGGraphWindow
		void SelectedInto(NFGGraphWindow* window);
	
		void SetStyle(unsigned char style);
		unsigned char GetStyle();

		/// Called by NFGGraphWindow::RefreshGraph()
		/// Performs calculation of curve sets and axes scale
		wxSize SetScale(wxSize ClientSize);
		void SetViewStart(wxPoint ViewStart);	/// notification function called by NFGGraphWindow::OnScroll() event handler
		wxPoint GetViewStart();
		wxRect GetBufferRect();
		wxSize GetVirtualSize();
		wxSize GetClientSize();

		/// Called by NFGGraphWindow::RefreshGraph()
		virtual NFGCurveSet GetCurveSet();
		virtual NFGCurveSet GetOptimizedCurveSet();

		/// Called by NFGGraphWindow::OnPaint event handler
		virtual NFGCurveSet GetHighlightedCurveSet();
		virtual NFGCurveSet GetOptimizedHighlightedCurveSet();
		virtual NFGPointSet GetHighlightedPointSet();
		virtual NFGPointSet GetOptimizedHighlightedPointSet();
	
		/// Called by NFGGraphWindow::RefreshGraph()
		NFGAxis GetXAxis(const int MajorXTicsDistMin = 50, bool NoReuse = false);
		NFGAxis GetYAxis(const int MajorYTicsDistMin = 50, bool NoReuse = false);
		
		/// Called by NFGGraphWindow::OnMouse event handler or directly
		virtual void SelectStep(unsigned long index);
		virtual void SelectNextStep();
		virtual void SelectPrevStep();

		/// Scale-change commands
		void Zoom(bool ZoomIn, bool ZoomHorizontal, bool ZoomVertical, wxPoint point = wxDefaultPosition);
		void AutoScaleAll(bool AutoScaleHorizontal, bool AutoScaleVertical, bool SetFlag = true);
		void AutoScaleSelected(bool AutoScaleHorizontal, bool AutoScaleVertical, bool SetFlag = true);
		
		ZoomParams GetZoomParams();
		ZoomParams SetZoomParams(ZoomParams zparams);
		
		/// Info functions
		virtual NFGRealRect GetBoundingRealRect();
		virtual NFGRealRect GetSelectedStepBoundingRealRect();
		NFGRealRect GetGraphRealRect();	/// can be larger than BoundingRealRect as it may contain empty space
		
		unsigned long GetSelectedStep();
		wxString GetSelectedStepLabel();
		wxString GetSelectedStepState();
		unsigned char GetSelectedStepStateFlag();
		
		void TrackMousePos(wxPoint pt);
		void RegisterMousePos(wxPoint pt);
		void DiscardMousePos();
		void CopyMousePos(wxPoint pt, bool Append = false);
		
		wxString GetCursorX();
		wxString GetCursorY();
		wxString GetCursorDeltaX();
		wxString GetCursorYRatio();
};

#endif
