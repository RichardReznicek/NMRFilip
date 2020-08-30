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

#include "plotgen.h"

#if __cplusplus >= 201103L
#include <cstdint>
#else
#include <stdint.h>
#endif
#include <cstdlib>
#include <cmath>

#include "nmrdata.h"
#include "doc.h"
#include "plotwin.h"


#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(NFGCurveArray);
WX_DEFINE_OBJARRAY(NFGPointArray);

WX_DEFINE_OBJARRAY(wxPointArray);


NFGDataseries::NFGDataseries()
{
	Curve.PointArray = NULL;
	FreeScaledPoints();
	
	RealBBox.minx = 0.0;
	RealBBox.maxx = 0.0;
	RealBBox.miny = 0.0;
	RealBBox.maxy = 0.0;
	RealBBoxValid = false;
	
	No = ALL_STEPS;
}

NFGDataseries::~NFGDataseries()
{
	FreeScaledPoints();
}

void NFGDataseries::FreeScaledPoints()
{
	std::free(Curve.PointArray);
	Curve.PointArray = NULL;
	Curve.PointCount = 0;
	Curve.BufferLength = 0;
	Curve.ElisionCount = 0;
	Curve.BoundingBox.x = 0;
	Curve.BoundingBox.y = 0;
	Curve.BoundingBox.width = 0;
	Curve.BoundingBox.height = 0;

	CurveScale.xoffset = 0;
	CurveScale.yoffset = 0;
	CurveScale.xfactor = 0.0;
	CurveScale.yfactor = 0.0;
	
	CurveValid = false;
}

void NFGDataseries::SimplifyCurve()
{
	if ((Curve.PointCount != Curve.BufferLength) || (Curve.PointArray == NULL) || (Curve.PointCount < 2))
		return;
	
	wxPointArray Elisions;
	bool InElision = false;
	unsigned long index = 0;

	for (unsigned long i = 0; i < (Curve.PointCount - 1); i++) {
		if (!InElision) 
			Curve.PointArray[index++] = Curve.PointArray[i];
		
		if (!InElision && (Curve.PointArray[i] == Curve.PointArray[i + 1])) {
			InElision = true;
			Elisions.Add(wxPoint(i, i + 1));
		}
		
		if (InElision && (Curve.PointArray[i] != Curve.PointArray[i + 1])) {
			InElision = false;
			Elisions.Last().y = i;
		}
	}

	if (InElision) 	/// handle the last point
		Elisions.Last().y = Curve.PointCount - 1;
	else 
		Curve.PointArray[index++] = Curve.PointArray[Curve.PointCount - 1];

	Curve.PointCount = index;
	Curve.ElisionCount = Elisions.GetCount();
	
	/// store the elisions
	for (unsigned long i = 0; i < Curve.ElisionCount; i++) 
		(Curve.PointArray + Curve.PointCount)[i] = Elisions[i];
	
}

long NFGDataseries::SimplifiedCurveIndex(long Index, bool Backwards) 
{
	if ((Index < 0) || (((unsigned long) Index) >= Curve.BufferLength))
		return -1;
	
	if (Curve.PointCount == Curve.BufferLength)	/// curve not simplified
		return Index;
	
	unsigned long sIndex = 0;
	
	if (!Backwards) {
		sIndex = Index;
		for (unsigned long i = 0; i < Curve.ElisionCount; i++) {
			if (Index >= (Curve.PointArray + Curve.PointCount)[i].y)
				sIndex -= (Curve.PointArray + Curve.PointCount)[i].y - (Curve.PointArray + Curve.PointCount)[i].x;
			else
			if (Index > (Curve.PointArray + Curve.PointCount)[i].x)
				sIndex -= Index - (Curve.PointArray + Curve.PointCount)[i].x;
			else 
				break;
		}
	} else {
		sIndex = Curve.BufferLength - Index;
		for (unsigned long i = 0; i < Curve.ElisionCount; i++) {
			if (Index <= (Curve.PointArray + Curve.PointCount)[Curve.ElisionCount - 1 - i].x)
				sIndex -= (Curve.PointArray + Curve.PointCount)[Curve.ElisionCount - 1 - i].y - (Curve.PointArray + Curve.PointCount)[Curve.ElisionCount - 1 - i].x;
			else
			if (Index < (Curve.PointArray + Curve.PointCount)[Curve.ElisionCount - 1 - i].y)
				sIndex -= (Curve.PointArray + Curve.PointCount)[Curve.ElisionCount - 1 - i].y - Index;
			else 
				break;
		}
		sIndex = Curve.PointCount - sIndex;
	}
	
	return sIndex;
}


NFGDataseriesGroup::NFGDataseriesGroup()
{
	NMRDataPointer = NULL;
	WatchedNMRData = 0;
	
	GetNMRPts = NULL;
	GetNMRRPtBB = NULL;
	GetNMRFlag = NULL;
	GetNMRIndexRange = NULL;
	
	KeyItem.DatasetFlag = 0;

	DataseriesArray = NULL;
	DataseriesCount = 0;
	DataseriesValid = false;

	NumberedDataseries = false;
	NumberedCurvePoints = false;

	HasHeadAndTail	= false;
	
	SymmetricYRange = false;
	IncludeYZero = false;
	NondecreasingX = false;
	DominantBBox = false;
	
	RealBBox.minx = 0.0;
	RealBBox.maxx = 0.0;
	RealBBox.miny = 0.0;
	RealBBox.maxy = 0.0;
	RealBBoxValid = false;
}

NFGDataseriesGroup::~NFGDataseriesGroup()
{
	FreeDataseries();
}

bool NFGDataseriesGroup::GetDataseries()
{
	if (DataseriesValid)
		return true;
	
	if (NMRDataPointer == NULL)
		return false;
	
	if ((GetNMRPts == NULL) || (GetNMRFlag == NULL) || (GetNMRIndexRange == NULL) || (GetNMRRPtBB == NULL))
		return false;

	if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_StepSet, ALL_STEPS) != DATA_OK) {
		FreeDataseries();
		return false;
	}
	
	if (((NumberedDataseries)?(NFGNMRData::GetStepNoRange(NMRDataPointer)):(1)) != DataseriesCount) {
		FreeDataseries();
		DataseriesCount = (NumberedDataseries)?(NFGNMRData::GetStepNoRange(NMRDataPointer)):(1);
		DataseriesArray = new (std::nothrow) NFGDataseries[DataseriesCount];
		if (DataseriesArray == NULL) {
			DataseriesCount = 0;
			NFGErrorReportCustom(NMRDataPointer, "Memory allocation error", "Preparing dataseries for plotting");
			return false;
		}
	} else {
		/// Clean up existing dataseries
		for (unsigned long i = 0; i < DataseriesCount; i++) {
			DataseriesArray[i].FreeScaledPoints();
			DataseriesArray[i].RealBBoxValid = false;
		}
	}
	
	/// (Re-)Initialize the dataseries
	for (unsigned long i = 0; i < DataseriesCount; i++) {
		DataseriesArray[i].No = (NumberedDataseries)?(i):(ALL_STEPS);
		
		DataseriesArray[i].Curve.NondecreasingX = NondecreasingX;
		DataseriesArray[i].Curve.Pen = KeyItem.Pen;
		DataseriesArray[i].Curve.PenThick = KeyItem.PenThick;
		DataseriesArray[i].Curve.PenBW = KeyItem.PenBW;
	}
	
	DataseriesValid = true;
	
	return true;
}

void NFGDataseriesGroup::FreeDataseries()
{
	delete[] DataseriesArray;
	DataseriesArray = NULL;
	DataseriesCount = 0;
	DataseriesValid = false;
	
	InvalidateRealBBox();
}

unsigned long NFGDataseriesGroup::GetDataseriesCount()
{
	return ((GetDataseries())?(DataseriesCount):(0));
}

void NFGDataseriesGroup::FreeScaledPoints()
{
	if (DataseriesArray == NULL)
		return;
	
	for (unsigned long i = 0; i < DataseriesCount; i++)
		DataseriesArray[i].FreeScaledPoints();
}

void NFGDataseriesGroup::InvalidateScaledPoints(long No)
{
	if (DataseriesArray != NULL) {
		if ((NumberedDataseries) && (No >= 0) && ((unsigned long) No < DataseriesCount)) {	/// Individual Dataseries
			DataseriesArray[No].CurveValid = false;
			
		} else {	/// All Dataseries together
			for (unsigned long i = 0; i < DataseriesCount; i++)
				DataseriesArray[i].CurveValid = false;
		} 
	}
}

bool NFGDataseriesGroup::DoGetRealBBox(NFGRealBBox &BBox, long Index) 
{
	if (!GetDataseries())
		return false;
	
	/// TODO: consider removing this
	if ((DataseriesArray == NULL) || (DataseriesCount == 0))
		return false;
	
	if ((Index >= 0) && ((unsigned long) Index < DataseriesCount)) {	/// Individual Dataseries
		
		if (DataseriesArray[Index].RealBBoxValid) {
			BBox = DataseriesArray[Index].RealBBox;
			return true;
		}
		
		if (NMRDataPointer == NULL)
			return false;
		
		/// Make sure the NMR data are available
		if (NFGNMRData::CheckNMRData(NMRDataPointer, WatchedNMRData, DataseriesArray[Index].No) != DATA_OK) 
			return false;
		
		if ((GetNMRIndexRange == NULL) || (GetNMRRPtBB == NULL))
			return false;
		
		if (GetNMRIndexRange(NMRDataPointer, DataseriesArray[Index].No) == 0) {
			DataseriesArray[Index].RealBBox.minx = NAN;
			DataseriesArray[Index].RealBBox.maxx = NAN;
			DataseriesArray[Index].RealBBox.miny = NAN;
			DataseriesArray[Index].RealBBox.maxy = NAN;
		} else
			GetNMRRPtBB(NMRDataPointer, DataseriesArray[Index].No, DataseriesArray[Index].RealBBox.minx, DataseriesArray[Index].RealBBox.maxx, DataseriesArray[Index].RealBBox.miny, DataseriesArray[Index].RealBBox.maxy);
		
		DataseriesArray[Index].RealBBoxValid = true;
		
		BBox = DataseriesArray[Index].RealBBox;
		
	} else 
	if (Index < 0) {	/// All Dataseries together

		if (RealBBoxValid) {
			BBox = RealBBox;
			return true;
		}

		if ((NMRDataPointer == NULL) || (GetNMRFlag == NULL)) 
			return false;
		
		RealBBox.minx = HUGE_VAL; 
		RealBBox.maxx = - HUGE_VAL; 
		RealBBox.miny = HUGE_VAL; 
		RealBBox.maxy = - HUGE_VAL; 
		
		NFGRealBBox AuxBBox;
		unsigned long hits = 0;
		for (unsigned long i = 0; i < DataseriesCount; i++) {
			/// Make sure the NMR step set is available
			if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_StepSet, DataseriesArray[i].No) != DATA_OK) 
				return false;

			/// Make sure the NMR step is valid
			if ((GetNMRFlag(NMRDataPointer, DataseriesArray[i].No) & 0x0f) != STEP_OK)
				continue;

			/// Get the bounding box of individual Dataseries
			if (!DoGetRealBBox(AuxBBox, i))
				return false;
			
			if (AuxBBox.minx < RealBBox.minx)
				RealBBox.minx = AuxBBox.minx;
			if (AuxBBox.maxx > RealBBox.maxx)
				RealBBox.maxx = AuxBBox.maxx;
			
			if (AuxBBox.miny < RealBBox.miny)
				RealBBox.miny = AuxBBox.miny;
			if (AuxBBox.maxy > RealBBox.maxy)
				RealBBox.maxy = AuxBBox.maxy;
			
			hits++;
		}
	
		if ((hits == 0) || (RealBBox.minx > RealBBox.maxx)) {
			RealBBox.minx = NAN; 
			RealBBox.maxx = NAN; 
		}
		
		if ((hits == 0) || (RealBBox.miny > RealBBox.maxy)) {
			RealBBox.miny = NAN; 
			RealBBox.maxy = NAN; 
		}
		
		RealBBoxValid = true;
		
		BBox = RealBBox;
		
	} else {	/// Invalid Index
		return false;
	}
	
	return true;
}

bool NFGDataseriesGroup::GetRealBBox(NFGRealBBox &BBox, long No)
{
	return DoGetRealBBox(BBox, (NumberedDataseries)?(No):(ALL_STEPS));
}

void NFGDataseriesGroup::InvalidateRealBBox(long No)
{
	if (DataseriesArray != NULL) {
		if ((NumberedDataseries) && (No >= 0) && ((unsigned long) No < DataseriesCount)) {	/// Individual Dataseries
			DataseriesArray[No].RealBBoxValid = false;
				
		} else {	/// All Dataseries together
			for (unsigned long i = 0; i < DataseriesCount; i++)
				DataseriesArray[i].RealBBoxValid = false;
		} 
	}
	
	RealBBoxValid = false;
}

bool NFGDataseriesGroup::GetCurve(NFGScale scale, long Index, NFGCurve &curve)
{
	if (!GetDataseries())
		return false;
	
	if ((DataseriesArray == NULL) || (DataseriesCount == 0))
		return false;
	
	if ((Index >= 0) && ((unsigned long) Index < DataseriesCount)) {
		/// Check whether the scale matches the previous one and if the curve is already available
		if (DataseriesArray[Index].CurveValid && 
			(DataseriesArray[Index].CurveScale.xoffset == scale.xoffset) && (DataseriesArray[Index].CurveScale.yoffset == scale.yoffset) &&
			(DataseriesArray[Index].CurveScale.xfactor == scale.xfactor) && (DataseriesArray[Index].CurveScale.yfactor == scale.yfactor)
		) {
			curve = DataseriesArray[Index].Curve;
			return true;
		}
		
		if (NMRDataPointer == NULL) 
			return false;
		
		if ((GetNMRIndexRange == NULL) || (GetNMRPts == NULL))
			return false;
		
		NFGRealBBox BBox;
		if (!DoGetRealBBox(BBox, Index)) 
			return false;

		/// Make sure the NMR data are available
		if (NFGNMRData::CheckNMRData(NMRDataPointer, WatchedNMRData, DataseriesArray[Index].No) != DATA_OK) 
			return false;
		
		if (GetNMRIndexRange(NMRDataPointer, DataseriesArray[Index].No) != DataseriesArray[Index].Curve.BufferLength) {
			wxPoint* auxptr = (wxPoint*) std::realloc(DataseriesArray[Index].Curve.PointArray, GetNMRIndexRange(NMRDataPointer, DataseriesArray[Index].No)*sizeof(wxPoint));
			if ((auxptr == NULL) && (GetNMRIndexRange(NMRDataPointer, DataseriesArray[Index].No) > 0)) {
				NFGErrorReportCustom(NMRDataPointer, "Memory allocation error", "Preparing curves for plotting");
				return false;
			}
			
			DataseriesArray[Index].Curve.PointArray = auxptr;
			DataseriesArray[Index].Curve.BufferLength = GetNMRIndexRange(NMRDataPointer, DataseriesArray[Index].No);
		}
		
		GetNMRPts(NMRDataPointer, DataseriesArray[Index].No, DataseriesArray[Index].Curve.PointArray, scale);
		
		DataseriesArray[Index].Curve.PointCount = DataseriesArray[Index].Curve.BufferLength;
		DataseriesArray[Index].Curve.ElisionCount = 0;
		
		if (wxFinite(BBox.minx) && wxFinite(BBox.maxx)) {
			DataseriesArray[Index].Curve.BoundingBox.x = NFGNMRData::llroundnu(BBox.minx * scale.xfactor) - scale.xoffset;
			DataseriesArray[Index].Curve.BoundingBox.width = NFGNMRData::llroundnu(BBox.maxx * scale.xfactor) - scale.xoffset - DataseriesArray[Index].Curve.BoundingBox.x + 1;
		} else {
			DataseriesArray[Index].Curve.BoundingBox.x = - scale.xoffset;
			DataseriesArray[Index].Curve.BoundingBox.width = (GetNMRIndexRange(NMRDataPointer, DataseriesArray[Index].No) > 0) ? 1 : 0;
		}
		
		if (wxFinite(BBox.miny) && wxFinite(BBox.maxy)) {
			DataseriesArray[Index].Curve.BoundingBox.y = NFGNMRData::llroundnu(BBox.maxy * scale.yfactor) - scale.yoffset;
			DataseriesArray[Index].Curve.BoundingBox.height = NFGNMRData::llroundnu(BBox.miny * scale.yfactor) - scale.yoffset - DataseriesArray[Index].Curve.BoundingBox.y + 1;
		} else {
			DataseriesArray[Index].Curve.BoundingBox.y = - scale.yoffset;
			DataseriesArray[Index].Curve.BoundingBox.height = (GetNMRIndexRange(NMRDataPointer, DataseriesArray[Index].No) > 0) ? 1 : 0;
		}
		
		DataseriesArray[Index].CurveScale = scale;
		DataseriesArray[Index].CurveValid = true;
		
		/// Heuristic guess whether there is a significant overlap of subsequent curve points
		if (DataseriesArray[Index].Curve.PointCount > 3*((unsigned long) std::labs(DataseriesArray[Index].Curve.BoundingBox.width) + (unsigned long) std::labs(DataseriesArray[Index].Curve.BoundingBox.height)))
			DataseriesArray[Index].SimplifyCurve();
		
		curve = DataseriesArray[Index].Curve;
		return true;
		
	} else
		return false;
}

bool NFGDataseriesGroup::GetCurve(NFGScale scale, long Index, NFGCurve &curvehead, NFGCurve &curvetail, NFGCurve &curvebody, long start, long end)
{
	if (!GetCurve(scale, Index, curvebody))
		return false;
	
	curvehead = curvebody;
	curvetail = curvebody;
	
	curvehead.Pen = KeyItem.AltPen;
	curvehead.PenThick = KeyItem.AltPenThick;
	curvehead.PenBW = KeyItem.AltPenBW;
	
	curvetail.Pen = KeyItem.AltPen;
	curvetail.PenThick = KeyItem.AltPenThick;
	curvetail.PenBW = KeyItem.AltPenBW;
	
	if (curvebody.PointCount == 0) /// Nothing more to be done
		return true;
	
	/// Translate the indices if the curve is simplified
	start = DataseriesArray[Index].SimplifiedCurveIndex(start, false);
	end = DataseriesArray[Index].SimplifiedCurveIndex(end, true);
	
	if ((start < 0) || (((unsigned long) start) >= curvebody.PointCount) || !HasHeadAndTail)
		start = 0;
	
	if ((end < 0) || (((unsigned long) end) >= curvebody.PointCount) || !HasHeadAndTail)
		end = curvebody.PointCount - 1;
	
	if (start > end)
		start = end;
	
	curvehead.PointCount = start + 1;
	curvehead.BufferLength = curvehead.PointCount;
	
	curvebody.PointCount = end + 1 - start;
	curvebody.BufferLength = curvebody.PointCount;
	curvebody.PointArray += start;
	
	curvetail.PointCount -= end;
	curvetail.BufferLength = curvetail.PointCount;
	curvetail.PointArray += end;
	
	if (curvebody.NondecreasingX) {
		curvehead.BoundingBox.x = curvehead.PointArray[0].x;
		curvehead.BoundingBox.width = curvehead.PointArray[curvehead.PointCount - 1].x - curvehead.PointArray[0].x + 1;
		
		curvebody.BoundingBox.x = curvebody.PointArray[0].x;
		curvebody.BoundingBox.width = curvebody.PointArray[curvebody.PointCount - 1].x - curvebody.PointArray[0].x + 1;
		
		curvetail.BoundingBox.x = curvetail.PointArray[0].x;
		curvetail.BoundingBox.width = curvetail.PointArray[curvetail.PointCount - 1].x - curvetail.PointArray[0].x + 1;
	}
	
	return true;
}

bool NFGDataseriesGroup::GetPoint(NFGScale scale, long Index, long PointIndex, NFGPoint &point) 
{
	NFGCurve curve;
	
	if (!GetCurve(scale, Index, curve))
		return false;

	/// Translate the index if the curve is simplified
	long sIndex = DataseriesArray[Index].SimplifiedCurveIndex(PointIndex);
	if (sIndex < 0)
		return false;
	
	point.Point = curve.PointArray[sIndex];
	point.Pen = curve.Pen;
	point.PenThick = curve.PenThick;
	point.PenBW = curve.PenBW;
	
	return true;
}

void NFGDataseriesGroup::MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo)
{
	if (ClearedFlags & Flag(CHECK_StepSet)) {
		DataseriesValid = false;
		InvalidateRealBBox(ALL_STEPS);
		InvalidateScaledPoints(ALL_STEPS);
		return;
	}

	if (ClearedFlags & Flag(WatchedNMRData)) {
		InvalidateRealBBox(StepNo);
		InvalidateScaledPoints(StepNo);
	}
}

void NFGDataseriesGroup::ChangeProcParamCallback(unsigned int ParamType, long StepNo)
{
	if (Flag(ParamType) & (Flag(PROC_PARAM_StepFlag) | Flag(PROC_PARAM_SetStepFlag) | Flag(PROC_PARAM_ClearStepFlag))) 
		RealBBoxValid = false;
}




NFGGraph::NFGGraph(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style, unsigned long WatchNMRFlags, unsigned long WatchProcParamFlags)
{
	GraphWindow = NULL;
	SerDoc = document;
	
	DisplayedDatasets = 0;
	DisplayedDatasetsMask = 0;
	
	DataseriesGroupArray = NULL;
	DataseriesGroupCount = 0;
	
	WatchedNMRFlags = WatchNMRFlags;
	WatchedProcParamFlags = WatchProcParamFlags;
	
	GraphRealRect.x = 0.0;
	GraphRealRect.y = 0.0;
	GraphRealRect.width = 0.0;
	GraphRealRect.height = 0.0;
	
	VisibleRealRect = GraphRealRect;
	
	RequestedVisibleRealRect = GraphRealRect;
	
	BoundingRealRect = GraphRealRect;
	BoundingRealRectValid = false;
	
	GWClientSize = wxSize(0, 0);
	GWVirtualSize = wxSize(0, 0);
	GWViewStart = wxPoint(0, 0);

	AutoZoomVSelected = false;
	AutoZoomVAll = true;
	AutoZoomHSelected = false;
	AutoZoomHAll = true;
	
	StyleFlag = style;
	DrawPoints = ((StyleFlag & GraphStyle_Points) == GraphStyle_Points);
	ThickLines = ((StyleFlag & GraphStyle_Thick) == GraphStyle_Thick);

	ConstrainZoomLeft = false;
	ConstrainZoomRight = false;
	ConstrainZoomTop = false;
	ConstrainZoomBottom = false;

	ConstrainZoomLeftAllowOverride = false;
	ConstrainZoomRightAllowOverride = false;
	ConstrainZoomTopAllowOverride = false;
	ConstrainZoomBottomAllowOverride = false;
	
	ConstrainZoomOverride.minx = NAN;
	ConstrainZoomOverride.maxx = NAN;
	ConstrainZoomOverride.miny = NAN;
	ConstrainZoomOverride.maxy = NAN;

	StickyZoomLeft = false;
	StickyZoomBottom = false;

	DisableZoomInH = false;
	DisableZoomInV = false;
	
	DataError = false;
	
	NMRDataPointer = NMRDataPtr;
	SelectedStep = 0;
	
	CurveSet.CurveArray = NULL;
	CurveSet.CurveCount = 0;
	CurveSet.DrawPoints = DrawPoints;
	CurveSet.ThickLines = ThickLines;
	CurveSet.PointRadius = 4;
	CurveSetValid = false;
	
	OptimizedCurveSet = CurveSet;
	OptimizedCurveSetValid = false;
	OptimizedCurveSetRect = wxRect(0, 0, 0, 0);
	
	HighlightedCurveSet.CurveArray = NULL;
	HighlightedCurveSet.CurveCount = 0;
	HighlightedCurveSet.DrawPoints = DrawPoints;
	HighlightedCurveSet.ThickLines = ThickLines;
	HighlightedCurveSet.PointRadius = 6;
	HighlightedCurveSetValid = false;
	
	OptimizedHighlightedCurveSet = HighlightedCurveSet;
	OptimizedHighlightedCurveSetValid = false;
	OptimizedHighlightedCurveSetRect = wxRect(0, 0, 0, 0);
	
	HighlightedPointSet.PointArray = NULL;
	HighlightedPointSet.PointCount = 0;
	HighlightedPointSet.ThickLines = ThickLines;
	HighlightedPointSet.PointRadius = 6;
	HighlightedPointSetValid = false;

	OptimizedHighlightedPointSet = HighlightedPointSet;
	OptimizedHighlightedPointSetValid = false;
	OptimizedHighlightedPointSetRect = wxRect(0, 0, 0, 0);

	
	XAxisBufferRect = wxRect(0, 0, 0, 0);

	XAxis.MajorTics = NULL;
	XAxis.MajorTicsCount = 0;
	XAxis.Labels = NULL;
	
	XAxis.MinorTics = NULL;
	XAxis.MinorTicsCount = 0;
	
	XAxis.AxisLabel = wxEmptyString;
	XAxis.ZeroAxisPos = 0;
	XAxis.ShowZeroAxis = false;

	XAxisUnits = wxEmptyString;
	XAxisZeroExponent = false;
	XAxisMajorTicsDistMin = 50;
	
	XAxisValid = false;
	
	
	YAxisBufferRect = wxRect(0, 0, 0, 0);
	
	YAxis.MajorTics = NULL;
	YAxis.MajorTicsCount = 0;
	YAxis.Labels = NULL;
	
	YAxis.MinorTics = NULL;
	YAxis.MinorTicsCount = 0;
	
	YAxis.AxisLabel = wxEmptyString;
	YAxis.ZeroAxisPos = 0;
	YAxis.ShowZeroAxis = false;

	YAxisUnits = wxEmptyString;
	YAxisZeroExponent = false;
	YAxisMajorTicsDistMin = 50;

	YAxisValid = false;
	
	MousePos = wxPoint(-1, -1);
	ReferenceRealPointValid = false;
	ReferenceRealPoint.x = 0.0;
	ReferenceRealPoint.y = 0.0;
}


NFGGraph::~NFGGraph()
{
	FreeScaledPoints();
	delete[] DataseriesGroupArray;
	DataseriesGroupArray = NULL;
	DataseriesGroupCount = 0;
}


bool NFGGraph::CheckData(bool refresh)
{
	if ((GraphWindow != NULL) && refresh)
		GraphWindow->FreezeGraph();

	DataError = false;
	
	/// Just in case the number of steps changed
	SelectStep(SelectedStep);
	
	AutoScaleAll(AutoZoomHAll, AutoZoomVAll);
	
	AutoScaleSelected(AutoZoomHSelected, AutoZoomVSelected);

	CheckConstrains();

	if ((GraphWindow != NULL) && refresh) {
		GraphWindow->ThawGraph(true);
		GraphWindow->RefreshGraph();
	}
	
	return true;
}


void NFGGraph::InvalidateScaledPoints()
{
 	CurveSetValid = false;
	OptimizedCurveSetValid = false;
	HighlightedCurveSetValid = false;
	OptimizedHighlightedCurveSetValid = false;
	HighlightedPointSetValid = false;
	OptimizedHighlightedPointSetValid = false;
	XAxisValid = false;
	YAxisValid = false;
}


void NFGGraph::FreeScaledPoints()
{
	for (unsigned long i = 0; i < DataseriesGroupCount; i++) 
		DataseriesGroupArray[i].FreeScaledPoints();
	
	delete[] CurveSet.CurveArray;
	CurveSet.CurveArray = NULL;
	CurveSet.CurveCount = 0;
	CurveSetValid = false;
	
	delete[] OptimizedCurveSet.CurveArray;
	OptimizedCurveSet.CurveArray = NULL;
	OptimizedCurveSet.CurveCount = 0;
	OptimizedCurveSetValid = false;

	delete[] HighlightedCurveSet.CurveArray;
	HighlightedCurveSet.CurveArray = NULL;
	HighlightedCurveSet.CurveCount = 0;
	HighlightedCurveSetValid = false;
	
	delete[] OptimizedHighlightedCurveSet.CurveArray;
	OptimizedHighlightedCurveSet.CurveArray = NULL;
	OptimizedHighlightedCurveSet.CurveCount = 0;
	OptimizedHighlightedCurveSetValid = false;
	
	delete[] HighlightedPointSet.PointArray;
	HighlightedPointSet.PointArray = NULL;
	HighlightedPointSet.PointCount = 0;
	HighlightedPointSetValid = false;
	
	delete[] OptimizedHighlightedPointSet.PointArray;
	OptimizedHighlightedPointSet.PointArray = NULL;
	OptimizedHighlightedPointSet.PointCount = 0;
	OptimizedHighlightedPointSetValid = false;
	

	XAxisBufferRect = wxRect(0, 0, 0, 0);

	delete[] XAxis.MajorTics;
	XAxis.MajorTics = NULL;
	XAxis.MajorTicsCount = 0;
	delete[] XAxis.Labels;
	XAxis.Labels = NULL;
	
	delete[] XAxis.MinorTics;
	XAxis.MinorTics = NULL;
	XAxis.MinorTicsCount = 0;
	
	XAxis.AxisLabel = wxEmptyString;
	XAxis.ZeroAxisPos = 0;

	XAxisValid = false;


	YAxisBufferRect = wxRect(0, 0, 0, 0);

	delete[] YAxis.MajorTics;
	YAxis.MajorTics = NULL;
	YAxis.MajorTicsCount = 0;
	delete[] YAxis.Labels;
	YAxis.Labels = NULL;
	
	delete[] YAxis.MinorTics;
	YAxis.MinorTics = NULL;
	YAxis.MinorTicsCount = 0;
	
	YAxis.AxisLabel = wxEmptyString;
	YAxis.ZeroAxisPos = 0;

	YAxisValid = false;
}


void NFGGraph::MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo)
{
	if (ClearedFlags & (Flag(CHECK_StepSet) | WatchedNMRFlags)) {
		for (unsigned long i = 0; i < DataseriesGroupCount; i++) 
			DataseriesGroupArray[i].MarkNMRDataOldCallback(ClearedFlags, StepNo);
		
		InvalidateScaledPoints();
		BoundingRealRectValid = false;
	}
}
	
void NFGGraph::ChangeProcParamCallback(unsigned int ParamType, long StepNo)
{
	if (Flag(ParamType) & WatchedProcParamFlags) {
		CurveSetValid = false;
		OptimizedCurveSetValid = false;
		HighlightedCurveSetValid = false;
		OptimizedHighlightedCurveSetValid = false;
		HighlightedPointSetValid = false;
		OptimizedHighlightedPointSetValid = false;
	}
	
	if (Flag(ParamType) & (Flag(PROC_PARAM_StepFlag) | Flag(PROC_PARAM_SetStepFlag) | Flag(PROC_PARAM_ClearStepFlag))) {
		for (unsigned long i = 0; i < DataseriesGroupCount; i++) 
			DataseriesGroupArray[i].ChangeProcParamCallback(ParamType, StepNo);
		
		BoundingRealRectValid = false;
	}
}


unsigned long NFGGraph::DisplayDatasets(unsigned long flag)
{
	flag &= DisplayedDatasetsMask;

	if (flag == DisplayedDatasets)
		return DisplayedDatasets;
	
	DisplayedDatasets = flag;

	/// Content and possibly also the scale of the graph changes
	InvalidateScaledPoints();
	BoundingRealRectValid = false;
	CheckData(true);
	
	return DisplayedDatasets;
}


unsigned long NFGGraph::GetDisplayedDatasets()
{
	return DisplayedDatasets;
}


long NFGGraph::GetKeyItemCount()
{
	unsigned long ItemCount = 0;
	unsigned long ScratchPad = DisplayedDatasets;
	
	/// counts all bits in DisplayedDatasets which are set to '1' to get the number of distinct visible curve types
	for (unsigned long i = 0; i < sizeof(DisplayedDatasets)*8; i++) {
		if (ScratchPad & 0x1)
			ItemCount++;
		ScratchPad >>= 1;
	}
	
	if (DataseriesGroupCount <  ItemCount)	/// should not happen
		return DataseriesGroupCount;
	
	return ItemCount;
}

NFGKeyItem NFGGraph::GetKeyItem(long index, bool DrawingOrder)
{
	NFGKeyItem EmptyKeyItem;
	EmptyKeyItem.Label = wxEmptyString;
	EmptyKeyItem.Pen = wxNullPen;
	EmptyKeyItem.PenThick = wxNullPen;
	EmptyKeyItem.PenBW = wxNullPen;
	EmptyKeyItem.AltPen = wxNullPen;
	EmptyKeyItem.AltPenThick = wxNullPen;
	EmptyKeyItem.AltPenBW = wxNullPen;
	EmptyKeyItem.DrawPoints = false;
	EmptyKeyItem.PointRadius = 0;
	EmptyKeyItem.HighlightedPointRadius = 0;
	EmptyKeyItem.DatasetFlag = 0;

	if (index < 0)
		return EmptyKeyItem;
	
	long index2 = 0;
	
	if (DrawingOrder) {
		for (unsigned long i = 0; i < DataseriesGroupCount; i++) {
			if (DataseriesGroupArray[i].KeyItem.DatasetFlag & DisplayedDatasets) {
				if (index == index2) {
					DataseriesGroupArray[i].KeyItem.DrawPoints = DrawPoints;
					return DataseriesGroupArray[i].KeyItem;
				}
				index2++;
			}
		}
	} else {
		unsigned long MatchFlag = 1;
		/// find the proper bit in DisplayedDatasets
		for (unsigned long j = 0; j < sizeof(MatchFlag)*8; j++) {
			if (DisplayedDatasets & MatchFlag)
				index2++;
			
			if ((index + 1) == index2) {
				for (unsigned long i = 0; i < DataseriesGroupCount; i++) {
					if (DataseriesGroupArray[i].KeyItem.DatasetFlag & MatchFlag) {
						DataseriesGroupArray[i].KeyItem.DrawPoints = DrawPoints;
						return DataseriesGroupArray[i].KeyItem;
					}
				}
			}
			
			MatchFlag <<= 1;
		}
	}

	return EmptyKeyItem;
}

wxString NFGGraph::GetGraphLabel()
{
	return wxEmptyString;
}


void NFGGraph::SetStyle(unsigned char style)
{
	
}


void NFGGraph::SelectedInto(NFGGraphWindow* window)
{
	GraphWindow = window;
	if (window != NULL)
		CheckData();
}


unsigned char NFGGraph::GetStyle()
{
	return GraphStyle_Line;
}


wxSize NFGGraph::SetScale(wxSize ClientSize)
{
	DisableZoomInH = false;
	DisableZoomInV = false;
	
	VisibleRealRect = RequestedVisibleRealRect;

	GWClientSize = ClientSize;
	/// The aim is to map window client area's <0; width/height - 1> onto visible real <min; max> range.
	ClientSize.DecBy(1);

	bool HRangeValid = false;
	bool VRangeValid = false;
	
	/// Provide for scrollbars - not desired for print or image export
#ifdef GTK_SCROLL_FIX
	int sw = 0;
	int sh = 0;
	if (GraphWindow != NULL) {
		GraphWindow->SetScrollbar(wxHORIZONTAL, 0, 0, 0);
		GraphWindow->SetScrollbar(wxVERTICAL, 0, 0, 0);
		wxSize Full = GraphWindow->GetClientSize();
		
		GraphWindow->SetScrollbar(wxHORIZONTAL, 0, 1, 2);
		GraphWindow->SetScrollbar(wxVERTICAL, 0, 1, 2);
		wxSize Cropped = GraphWindow->GetClientSize();
		
		wxSize ScrollbarSize = Full - Cropped;
		sw = ScrollbarSize.GetWidth();
		sh = ScrollbarSize.GetHeight();
	}
#else
	int sw = (GraphWindow != NULL)?(wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, GraphWindow)):(0);
	int sh = (GraphWindow != NULL)?(wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y, GraphWindow)):(0);
#endif
	bool HScrollbar = false;
	bool VScrollbar = false;
	
	/// Avoid the loss of precission due to too high magnitude
	const double PrecLimit = (double) (1ull << 53);
	/// Make sure point coordinates fit in device space on win - see http://msdn.microsoft.com/en-us/library/dd145139%28v=VS.85%29.aspx
	const double DevLimit = (double) ((1ul << 27) - 1);
	
	if (	(ClientSize.GetWidth() >= sw) && wxFinite(GraphRealRect.x) && wxFinite(GraphRealRect.width) && (GraphRealRect.width > 0.0) && 
		((ClientSize.GetWidth() / GraphRealRect.width * NFGMSTD fmax(std::fabs(GraphRealRect.x), std::fabs(GraphRealRect.x + GraphRealRect.width))) < PrecLimit) ) {
			
		if (!wxFinite(VisibleRealRect.width) || !(VisibleRealRect.width > 0.0) || (VisibleRealRect.width > GraphRealRect.width)) 
			VisibleRealRect.width = GraphRealRect.width;
		
		double InitialWidth = VisibleRealRect.width;
		
		/// make sure point coordinates fit in device space on win
		if (!((ClientSize.GetWidth() / VisibleRealRect.width * GraphRealRect.width) < (DevLimit - 1.0))) {	/// - 1.0 to provide for possible consequences of rounding
			VisibleRealRect.width = ClientSize.GetWidth() / (DevLimit - 1.0) * GraphRealRect.width;
			DisableZoomInH = true;
		}
		
		/// avoid reduction of precission
		if (!((ClientSize.GetWidth() / VisibleRealRect.width * NFGMSTD fmax(std::fabs(GraphRealRect.x), std::fabs(GraphRealRect.x + GraphRealRect.width))) < PrecLimit)) {
			VisibleRealRect.width = ClientSize.GetWidth() / PrecLimit * NFGMSTD fmax(std::fabs(GraphRealRect.x), std::fabs(GraphRealRect.x + GraphRealRect.width));
			DisableZoomInH = true;
		}
		
		/// check if a scrollbar is necessary
		const volatile double coef = (ClientSize.GetWidth() - sw) / VisibleRealRect.width;
		if ((NFGNMRData::llroundnu(coef * (GraphRealRect.x + GraphRealRect.width)) - NFGNMRData::llroundnu(coef * GraphRealRect.x)) <= (ClientSize.GetWidth() - sw)) 
			VisibleRealRect.width = GraphRealRect.width;
		else 
			HScrollbar = true;
		
		/// try to keep the centre point fixed
		if (VisibleRealRect.width > InitialWidth) 
			VisibleRealRect.x -= (VisibleRealRect.width - InitialWidth) / 2.0;
		
		if (!wxFinite(VisibleRealRect.x) || (VisibleRealRect.x < GraphRealRect.x)) 
			VisibleRealRect.x = GraphRealRect.x;
		
		if ((VisibleRealRect.x + VisibleRealRect.width) > (GraphRealRect.x + GraphRealRect.width)) 
			VisibleRealRect.x = GraphRealRect.x + GraphRealRect.width - VisibleRealRect.width;
		
		HRangeValid = true;
	}
	
	if (	(ClientSize.GetHeight() >= sh) && wxFinite(GraphRealRect.y) && wxFinite(GraphRealRect.height) && (GraphRealRect.height > 0.0) && 
		((ClientSize.GetHeight() / GraphRealRect.height * NFGMSTD fmax(std::fabs(GraphRealRect.y), std::fabs(GraphRealRect.y + GraphRealRect.height))) < PrecLimit)) {
	
		if (!wxFinite(VisibleRealRect.height) || !(VisibleRealRect.height > 0.0) || (VisibleRealRect.height > GraphRealRect.height))
			VisibleRealRect.height = GraphRealRect.height;
		
		double InitialHeight = VisibleRealRect.height;
		
		/// make sure it fits in device space on win
		if (!((ClientSize.GetHeight() / VisibleRealRect.height * GraphRealRect.height) < (DevLimit - 1.0))) {	/// - 1.0 to provide for possible consequences of rounding
			VisibleRealRect.height = ClientSize.GetHeight() / (DevLimit - 1.0) * GraphRealRect.height;
			DisableZoomInV = true;
		}
		
		/// avoid reduction of precission
		if (!((ClientSize.GetHeight() / VisibleRealRect.height * NFGMSTD fmax(std::fabs(GraphRealRect.y), std::fabs(GraphRealRect.y + GraphRealRect.height))) < PrecLimit)) {
			VisibleRealRect.height = ClientSize.GetHeight() / PrecLimit * std::fabs(GraphRealRect.y + GraphRealRect.height);
			DisableZoomInV = true;
		}
		
		/// check if a scrollbar is necessary
		const volatile double coef = - (ClientSize.GetHeight() - sh) / VisibleRealRect.height;
		if ((NFGNMRData::llroundnu(coef * (GraphRealRect.y)) - NFGNMRData::llroundnu(coef * (GraphRealRect.y + GraphRealRect.height))) <= (ClientSize.GetHeight() - sh)) 
			VisibleRealRect.height = GraphRealRect.height;
		else 
			VScrollbar = true;
		
		/// try to keep the centre point fixed
		if (VisibleRealRect.height > InitialHeight) 
			VisibleRealRect.y -= (VisibleRealRect.height - InitialHeight) / 2.0;
		
		if (!wxFinite(VisibleRealRect.y) || (VisibleRealRect.y < GraphRealRect.y)) 
			VisibleRealRect.y = GraphRealRect.y;
		
		if ((VisibleRealRect.y + VisibleRealRect.height) > (GraphRealRect.y + GraphRealRect.height)) 
			VisibleRealRect.y = GraphRealRect.y + GraphRealRect.height - VisibleRealRect.height;
		
		VRangeValid = true;
	}
	
	/// Provide for scrollbars if necessary
	GWClientSize.DecBy(((VScrollbar)?(sw):(0)), ((HScrollbar)?(sh):(0)));
	ClientSize = GWClientSize;
	/// The aim is to map window client area's <0; width/height - 1> onto visible real <min; max> range.
	ClientSize.DecBy(1);
		
	if (HRangeValid) {
		/// This surely fits in the ranges checked above
		ScaleValue.xfactor = ClientSize.GetWidth() / VisibleRealRect.width;
		ScaleValue.xoffset = NFGNMRData::llroundnu(GraphRealRect.x * ScaleValue.xfactor);
		GWVirtualSize.SetWidth(NFGNMRData::llroundnu((GraphRealRect.x + GraphRealRect.width)*ScaleValue.xfactor) - ScaleValue.xoffset + 1);
		GWViewStart.x = NFGNMRData::llroundnu(VisibleRealRect.x * ScaleValue.xfactor) - ScaleValue.xoffset;
		
	} else {
		VisibleRealRect.x = GraphRealRect.x;
		VisibleRealRect.width = GraphRealRect.width;
		
		ScaleValue.xfactor = 0.0;
		ScaleValue.xoffset = NFGNMRData::llroundnu(- ClientSize.GetWidth() / 2.0);
		GWVirtualSize.SetWidth(ClientSize.GetWidth() + 1);
		GWViewStart.x = 0;
	}
	
	if (VRangeValid) {
		/// This surely fits in the ranges checked above
		ScaleValue.yfactor = - ClientSize.GetHeight() / VisibleRealRect.height;
		ScaleValue.yoffset = NFGNMRData::llroundnu((GraphRealRect.y + GraphRealRect.height) * ScaleValue.yfactor);
		GWVirtualSize.SetHeight(NFGNMRData::llroundnu(GraphRealRect.y*ScaleValue.yfactor) - ScaleValue.yoffset + 1);
		GWViewStart.y = NFGNMRData::llroundnu((VisibleRealRect.y + VisibleRealRect.height) * ScaleValue.yfactor) - ScaleValue.yoffset;
		
	} else {
		VisibleRealRect.y = GraphRealRect.y;
		VisibleRealRect.height = GraphRealRect.height;
		
		ScaleValue.yfactor = 0.0;
		ScaleValue.yoffset = NFGNMRData::llroundnu(- ClientSize.GetHeight() / 2.0);
		GWVirtualSize.SetHeight(ClientSize.GetHeight() + 1);
		GWViewStart.y = 0;
	}
	
	InvalidateScaledPoints();

	if ((SerDoc != NULL) && (GraphWindow != NULL))
		SerDoc->ZoomParamsChangedNotify();
	
	return GWVirtualSize;
}


void NFGGraph::SetViewStart(wxPoint ViewStart)
{
	if (ViewStart.x > (GWVirtualSize.GetWidth() - GWClientSize.GetWidth())) 
		ViewStart.x = GWVirtualSize.GetWidth() - GWClientSize.GetWidth();
	if (ViewStart.x < 0) 
		ViewStart.x = 0;
	if (ViewStart.y > (GWVirtualSize.GetHeight() - GWClientSize.GetHeight())) 
		ViewStart.y = GWVirtualSize.GetHeight() - GWClientSize.GetHeight();
	if (ViewStart.y < 0) 
		ViewStart.y = 0;

	if (ViewStart.x != GWViewStart.x) {
		GWViewStart.x = ViewStart.x;
		
		if (ScaleValue.xfactor > 0.0) {
			RequestedVisibleRealRect.x = VisibleRealRect.x = ((double) (((long long) GWViewStart.x) + ScaleValue.xoffset))/ScaleValue.xfactor;
			
			/// "sticky" edges (useful e.g. when resizing)
			if ((GWViewStart.x + GWClientSize.GetWidth()) == GWVirtualSize.GetWidth())
				RequestedVisibleRealRect.x = VisibleRealRect.x = GraphRealRect.x + GraphRealRect.width - VisibleRealRect.width;
			
			if (GWViewStart.x == 0)
				RequestedVisibleRealRect.x = VisibleRealRect.x = GraphRealRect.x;
			
			/// extra check
			if ((VisibleRealRect.x + VisibleRealRect.width) > (GraphRealRect.x + GraphRealRect.width))
				RequestedVisibleRealRect.x = VisibleRealRect.x = GraphRealRect.x + GraphRealRect.width - VisibleRealRect.width;
			if (VisibleRealRect.x < GraphRealRect.x)
				RequestedVisibleRealRect.x = VisibleRealRect.x = GraphRealRect.x;
		} else {
			RequestedVisibleRealRect.x = VisibleRealRect.x = GraphRealRect.x;
		}
		
		RequestedVisibleRealRect.width = VisibleRealRect.width;
	}
	
	if (ViewStart.y != GWViewStart.y) {
		GWViewStart.y = ViewStart.y;
		
		if (ScaleValue.yfactor < 0.0) {
			RequestedVisibleRealRect.y = VisibleRealRect.y = ((double) (((long long) GWViewStart.y) + ScaleValue.yoffset))/ScaleValue.yfactor - VisibleRealRect.height;
			
			/// "sticky" edges (usefull e.g. when resizing)
			if ((GWViewStart.y + GWClientSize.GetHeight()) == GWVirtualSize.GetHeight())
				RequestedVisibleRealRect.y = VisibleRealRect.y = GraphRealRect.y;
			
			if (GWViewStart.y == 0)
				RequestedVisibleRealRect.y = VisibleRealRect.y = GraphRealRect.y + GraphRealRect.height - VisibleRealRect.height;
			
			/// extra check
			if ((VisibleRealRect.y + VisibleRealRect.height) > (GraphRealRect.y + GraphRealRect.height))
				RequestedVisibleRealRect.y = VisibleRealRect.y = GraphRealRect.y + GraphRealRect.height - VisibleRealRect.height;
			if (VisibleRealRect.y < GraphRealRect.y)
				RequestedVisibleRealRect.y = VisibleRealRect.y = GraphRealRect.y;
		} else {
			RequestedVisibleRealRect.y = VisibleRealRect.y = GraphRealRect.y;
		}
		
		RequestedVisibleRealRect.height = VisibleRealRect.height;
	}
	
	if (SerDoc != NULL)
		SerDoc->ZoomParamsChangedNotify();
}

wxPoint NFGGraph::GetViewStart()
{
	return GWViewStart;
}

wxRect NFGGraph::GetBufferRect()
{
	if (DataError)
		return wxRect(GWClientSize);

	if ((GraphWindow == NULL) || (GraphWindow->GetCurveWindow() == NULL)) /// typically during printing
		return wxRect(GWViewStart, GWClientSize);	/// consider only the print area
	
	int minx = GWViewStart.x - GWClientSize.GetWidth()/4;
	int maxx = GWViewStart.x + (GWClientSize.GetWidth() - 1) + GWClientSize.GetWidth()/4;
	int miny = GWViewStart.y - GWClientSize.GetHeight()/4;
	int maxy = GWViewStart.y + (GWClientSize.GetHeight() - 1) + GWClientSize.GetHeight()/4;
	
	if (minx < 0)
		minx = 0;
	if (miny < 0)
		miny = 0;
	if (maxx > (GWVirtualSize.GetWidth() - 1))
		maxx = GWVirtualSize.GetWidth() - 1;
	if (maxy > (GWVirtualSize.GetHeight() - 1))
		maxy = GWVirtualSize.GetHeight() - 1;
	
	return wxRect(minx, miny, maxx - minx + 1, maxy - miny + 1);
}

wxSize NFGGraph::GetVirtualSize()
{
	return GWVirtualSize;
}

wxSize NFGGraph::GetClientSize()
{
	return GWClientSize;
}


bool NFGGraph::DoGetCurveSet(NFGCurveSet &CS, long No, bool IsHighlighted, long start, long end)
{
	bool HeadBodyTail = !(IsHighlighted || (start < 0) || (end < 0));
	
	unsigned long CurveCount = 0;
	if (No < 0) {
		for (unsigned long i = 0; i < DataseriesGroupCount; i++) 
			CurveCount += DataseriesGroupArray[i].GetDataseriesCount();
	} else {
		for (unsigned long i = 0; i < DataseriesGroupCount; i++) 
			if (DataseriesGroupArray[i].NumberedDataseries)
				CurveCount++;
	}
	
	if (HeadBodyTail)
		CurveCount *= 3;
	
	if (CurveCount != CS.CurveCount) {
		delete[] CS.CurveArray;
		CS.CurveArray = new NFGCurve[CurveCount];
		CS.CurveCount = CurveCount;
	}

	CS.ThickLines = ThickLines;
	CS.DrawPoints = DrawPoints;

	NFGCurve EmptyCurve;
	EmptyCurve.PointArray = NULL;
	EmptyCurve.PointCount = 0;
	EmptyCurve.BufferLength = 0;
	EmptyCurve.ElisionCount = 0;
	EmptyCurve.NondecreasingX = false;
	EmptyCurve.BoundingBox = wxRect(0, 0, 0, 0);
	EmptyCurve.Pen = wxNullPen;
	EmptyCurve.PenThick = wxNullPen;
	EmptyCurve.PenBW = wxNullPen;
	
	for (unsigned long i = 0; i < CurveCount; i++) 
		CS.CurveArray[i] = EmptyCurve;
	
	unsigned long index = 0;
	for (unsigned long i = 0; i < DataseriesGroupCount; i++) {
		if (No < 0) {
			if (DataseriesGroupArray[i].KeyItem.DatasetFlag & DisplayedDatasets) 
				for (unsigned long j = 0; j < DataseriesGroupArray[i].GetDataseriesCount(); j++) {
					if (HeadBodyTail) {
						if (!DataseriesGroupArray[i].GetCurve(ScaleValue, j, CS.CurveArray[3*index + 0], CS.CurveArray[3*index + 1], CS.CurveArray[3*index + 2], start, end))
							return false;
					} else {
						if (!DataseriesGroupArray[i].GetCurve(ScaleValue, j, CS.CurveArray[index]))
							return false;
					}
					
					index++;
				}
			else
				index += DataseriesGroupArray[i].GetDataseriesCount();
			
		} else {
			if (DataseriesGroupArray[i].NumberedDataseries) {
				if (DataseriesGroupArray[i].KeyItem.DatasetFlag & DisplayedDatasets) {
					if (HeadBodyTail) {
						if (!DataseriesGroupArray[i].GetCurve(ScaleValue, No, CS.CurveArray[3*index + 0], CS.CurveArray[3*index + 1], CS.CurveArray[3*index + 2], start, end))
							return false;
					} else {
						if (!DataseriesGroupArray[i].GetCurve(ScaleValue, No, CS.CurveArray[index]))
							return false;
						/// Set alternative pens
						if (IsHighlighted) {
							CS.CurveArray[index].Pen = DataseriesGroupArray[i].KeyItem.AltPen;
							CS.CurveArray[index].PenThick = DataseriesGroupArray[i].KeyItem.AltPenThick;
							CS.CurveArray[index].PenBW = DataseriesGroupArray[i].KeyItem.AltPenBW;
						}
					}
				}
				index++;
			}
		}
	}
	
	return true;
}

void NFGGraph::DoGetOptimizedCurveSet(NFGCurveSet &OptimizedCS, wxRect &OptimizedCSRect, NFGCurveSet &InputCS)
{
	NFGCurveSet OrigCurveSet = InputCS;
	
	/// Unless printing, take into account the points and lines extending into the BufferRect from outside
	wxCoord extra = 0;
	NFGCurveWindow* CW = NULL;
	if ((GraphWindow != NULL) && (CW = GraphWindow->GetCurveWindow())) {
		for (unsigned long i = 0; i < OrigCurveSet.CurveCount; i++) {
			if (OrigCurveSet.CurveArray[i].PointCount > 0) {
				int linewidth = (OrigCurveSet.ThickLines)?(OrigCurveSet.CurveArray[i].PenThick.GetWidth()):(OrigCurveSet.CurveArray[i].Pen.GetWidth());
				wxCoord extraspace = ((linewidth > 0)?(linewidth):(1));
				if (OrigCurveSet.DrawPoints) 
					extraspace += CW->FromDIP(OrigCurveSet.PointRadius) + 1;
				
				if (extraspace > extra) 
					extra = extraspace;
			}
		}
	}
	
	OptimizedCSRect = GetBufferRect();
	OptimizedCSRect.Inflate(extra, extra);
	OptimizedCSRect.Intersect(wxRect(GWVirtualSize));
	
	NFGCurveArray* CurveArr = new NFGCurveArray;
	CurveArr->Alloc(OrigCurveSet.CurveCount);
	
	int minx, maxx;
	
	for (unsigned long i = 0; i < OrigCurveSet.CurveCount; i++) {
		if ((!(OptimizedCSRect.Intersects(OrigCurveSet.CurveArray[i].BoundingBox))) || (OrigCurveSet.CurveArray[i].PointCount == 0)) 
			continue;
		else if (OptimizedCSRect.Contains(OrigCurveSet.CurveArray[i].BoundingBox)) {
			CurveArr->Add(OrigCurveSet.CurveArray[i]);
			continue;
		} else {
			CurveArr->Add(OrigCurveSet.CurveArray[i]);
			minx = OrigCurveSet.CurveArray[i].BoundingBox.GetLeft();
			maxx = OrigCurveSet.CurveArray[i].BoundingBox.GetRight();
			
			if (OrigCurveSet.CurveArray[i].NondecreasingX) {
				
				if (OrigCurveSet.CurveArray[i].BoundingBox.GetLeft() < OptimizedCSRect.GetLeft()) {
				
					unsigned long leftindex = 0;
					unsigned long rightindex = OrigCurveSet.CurveArray[i].PointCount - 1;
					unsigned long index = 0;
					wxCoord left, right, val, x;
					
					left = OrigCurveSet.CurveArray[i].PointArray[leftindex].x;
					right = OrigCurveSet.CurveArray[i].PointArray[rightindex].x;
					val = OptimizedCSRect.GetLeft();
					x = left;
					
					if ((val > left) && (val < right) && (leftindex < rightindex)) {
				
						while ((leftindex < rightindex) && (left < right)) {
							index = (val - left)*(rightindex - leftindex)/(right - left) + leftindex;
							if ((index > rightindex) || (index < leftindex)) {	/// should not happen
								index = 0;
								leftindex = index;
								rightindex = index;
							}
							
							if ((index == rightindex) || (index == leftindex)) {
								leftindex = index;
								rightindex = index;
							}
							
							x = OrigCurveSet.CurveArray[i].PointArray[index].x;
							if (x < val) {
								leftindex = index;
								left = x;
							} else 
							if (x > val) {
								rightindex = index;
								right = x;
							} else {
								leftindex = index;
								rightindex = index;
							}
						}
						
						long j;
						
						/// quick refinement
						if (x < val) {
							for (j = index; (((unsigned long) j) < (OrigCurveSet.CurveArray[i].PointCount - 1)) && !((OrigCurveSet.CurveArray[i].PointArray[j].x < val) && (OrigCurveSet.CurveArray[i].PointArray[j+1].x >= val)); j++) 
								;
						} else {
							for (j = index; (j >= 0) && !((OrigCurveSet.CurveArray[i].PointArray[j].x < val) && (OrigCurveSet.CurveArray[i].PointArray[j+1].x >= val)) ; j--) 
								;
						}
						
						minx = OrigCurveSet.CurveArray[i].PointArray[j].x;
						
						(CurveArr->Last()).PointArray += j;
						(CurveArr->Last()).PointCount -= j;
					}
				}
				
				if (OrigCurveSet.CurveArray[i].BoundingBox.GetRight() > OptimizedCSRect.GetRight()) {
				
					unsigned long leftindex = 0;
					unsigned long rightindex = OrigCurveSet.CurveArray[i].PointCount - 1;
					unsigned long index = 0;
					wxCoord left, right, val, x;
					
					left = OrigCurveSet.CurveArray[i].PointArray[leftindex].x;
					right = OrigCurveSet.CurveArray[i].PointArray[rightindex].x;
					val = OptimizedCSRect.GetRight();
					x = left;
					
					if ((val > left) && (val < right) && (leftindex < rightindex)) {
				
						while ((leftindex < rightindex) && (left < right)) {
							index = (val - left)*(rightindex - leftindex)/(right - left) + leftindex;
							if ((index > rightindex) || (index < leftindex)) {	/// should not happen
								index = OrigCurveSet.CurveArray[i].PointCount - 1;
								leftindex = index;
								rightindex = index;
							}
							
							if ((index == rightindex) || (index == leftindex)) {
								leftindex = index;
								rightindex = index;
							}
							
							x = OrigCurveSet.CurveArray[i].PointArray[index].x;
							if (x < val) {
								leftindex = index;
								left = x;
							} else 
							if (x > val) {
								rightindex = index;
								right = x;
							} else {
								leftindex = index;
								rightindex = index;
							}
						}
						
						long j;
						
						/// quick refinement
						if (x <= val) {
							for (j = index; (((unsigned long) j) < OrigCurveSet.CurveArray[i].PointCount) && !((OrigCurveSet.CurveArray[i].PointArray[j].x > val) && (OrigCurveSet.CurveArray[i].PointArray[j-1].x <= val)); j++) 
								;
						} else {
							for (j = index; (j >= 1) && !((OrigCurveSet.CurveArray[i].PointArray[j].x > val) && (OrigCurveSet.CurveArray[i].PointArray[j-1].x <= val)) ; j--) 
								;
						}
						
						maxx = OrigCurveSet.CurveArray[i].PointArray[j].x;
						
						(CurveArr->Last()).PointCount -= OrigCurveSet.CurveArray[i].PointCount - 1 - (unsigned long) j;
						
					}
					
				}
				
				(CurveArr->Last()).BoundingBox.x = minx;
				(CurveArr->Last()).BoundingBox.width = maxx - minx + 1;
				
				
			}

		}
	}

	/// get rid of the extra space
	OptimizedCSRect = GetBufferRect();
	OptimizedCSRect.Intersect(wxRect(GWVirtualSize));

	if (CurveArr->GetCount() != OptimizedCS.CurveCount) {
		delete[] OptimizedCS.CurveArray;
		OptimizedCS.CurveCount = CurveArr->GetCount();
		OptimizedCS.CurveArray = new NFGCurve[OptimizedCS.CurveCount];
	}
	OptimizedCS.ThickLines = OrigCurveSet.ThickLines;
	OptimizedCS.DrawPoints = OrigCurveSet.DrawPoints;
	OptimizedCS.PointRadius = OrigCurveSet.PointRadius;
	for (unsigned long k = 0; k < OptimizedCS.CurveCount; k++)
		OptimizedCS.CurveArray[k] = CurveArr->Item(k);

	CurveArr->Empty();
	
#ifdef __WXGTK__
	OrigCurveSet = OptimizedCS;
	int miny, maxy;

	/// the check of possible signed 16-bit integer range overflow required due to the X interface limits
	/// unwanted side-effect: missing lines if the nearest curve point outside the rendered area exceeds the 16-bit limit (proper handling based on interpolation would be quite resource-expensive)

	wxRect BufRect = GetBufferRect();

	minx = BufRect.x + INT16_MIN + extra;
	maxx = BufRect.x + INT16_MAX - extra;
	miny = BufRect.y + INT16_MIN + extra;
	maxy = BufRect.y + INT16_MAX - extra;
	wxRect LimitRect = wxRect(wxPoint(minx, miny), wxPoint(maxx, maxy));

	for (unsigned long i = 0; i < OrigCurveSet.CurveCount; i++) {
		if (!(LimitRect.Intersects(OrigCurveSet.CurveArray[i].BoundingBox))) 
			continue;
		else if (LimitRect.Contains(OrigCurveSet.CurveArray[i].BoundingBox)) {
			CurveArr->Add(OrigCurveSet.CurveArray[i]);
			continue;
		} else {
			bool InRange = false;
			for (unsigned long j = 0; j < OrigCurveSet.CurveArray[i].PointCount; j++) {
				if (!InRange && 
					((OrigCurveSet.CurveArray[i].PointArray[j].x <= maxx) && (OrigCurveSet.CurveArray[i].PointArray[j].x >= minx) &&
					(OrigCurveSet.CurveArray[i].PointArray[j].y <= maxy) && (OrigCurveSet.CurveArray[i].PointArray[j].y >= miny))
				) {
					InRange = true;
					CurveArr->Add(OrigCurveSet.CurveArray[i]);
					(CurveArr->Last()).BoundingBox.Intersect(LimitRect);	/// not exact
					(CurveArr->Last()).PointArray += j;
					(CurveArr->Last()).PointCount -= j;
				}
				
				if (InRange && 
					((OrigCurveSet.CurveArray[i].PointArray[j].x > maxx) || (OrigCurveSet.CurveArray[i].PointArray[j].x < minx) ||
					(OrigCurveSet.CurveArray[i].PointArray[j].y > maxy) || (OrigCurveSet.CurveArray[i].PointArray[j].y < miny))
				) {
					InRange = false;
					(CurveArr->Last()).PointCount = &(OrigCurveSet.CurveArray[i].PointArray[j]) - (CurveArr->Last()).PointArray;
				}
			}
		}
	}
	
	if (CurveArr->GetCount() != OptimizedCS.CurveCount) {
		delete[] OptimizedCS.CurveArray;
		OptimizedCS.CurveCount = CurveArr->GetCount();
		OptimizedCS.CurveArray = new NFGCurve[OptimizedCS.CurveCount];
	}
	for (unsigned long k = 0; k < OptimizedCS.CurveCount; k++)
		OptimizedCS.CurveArray[k] = CurveArr->Item(k);

#endif
	
	CurveArr->Clear();
	delete CurveArr;
	
}

NFGCurveSet NFGGraph::GetCurveSet()
{
	if (CurveSetValid)
		return CurveSet;
	
	if (DataError || !DoGetCurveSet(CurveSet, ALL_STEPS, false)) {
		DataError = true;
		
		NFGCurveSet EmptyCurveSet;
		EmptyCurveSet.CurveArray = NULL;
		EmptyCurveSet.CurveCount = 0;
		EmptyCurveSet.DrawPoints = DrawPoints;
		EmptyCurveSet.ThickLines = ThickLines;
		EmptyCurveSet.PointRadius = 4;
		
		return EmptyCurveSet;
	}

	CurveSetValid = true;

	return CurveSet;
}

NFGCurveSet NFGGraph::GetOptimizedCurveSet()
{
	if (OptimizedCurveSetValid) 
		if (OptimizedCurveSetRect.Contains(GetBufferRect()))
			return OptimizedCurveSet;
	
	NFGCurveSet OrigCurveSet = GetCurveSet();
	
	DoGetOptimizedCurveSet(OptimizedCurveSet, OptimizedCurveSetRect, OrigCurveSet);
	
	OptimizedCurveSetValid = !DataError;

	return OptimizedCurveSet;
}

NFGCurveSet NFGGraph::GetHighlightedCurveSet()
{
	if (HighlightedCurveSetValid)
		return HighlightedCurveSet;
	
	if (DataError || !DoGetCurveSet(HighlightedCurveSet, SelectedStep, true)) {
		DataError = true;
		
		NFGCurveSet EmptyCurveSet;
		EmptyCurveSet.CurveArray = NULL;
		EmptyCurveSet.CurveCount = 0;
		EmptyCurveSet.DrawPoints = DrawPoints;
		EmptyCurveSet.ThickLines = ThickLines;
		EmptyCurveSet.PointRadius = 4;
		
		return EmptyCurveSet;
	}
	
	HighlightedCurveSetValid = true;

	return HighlightedCurveSet;
}

NFGCurveSet NFGGraph::GetOptimizedHighlightedCurveSet()
{
	if (OptimizedHighlightedCurveSetValid) 
		if (OptimizedHighlightedCurveSetRect.Contains(GetBufferRect()))
			return OptimizedHighlightedCurveSet;
	
	NFGCurveSet OrigCurveSet = GetHighlightedCurveSet();

	DoGetOptimizedCurveSet(OptimizedHighlightedCurveSet, OptimizedHighlightedCurveSetRect, OrigCurveSet);

	OptimizedHighlightedCurveSetValid = !DataError;
	
	return OptimizedHighlightedCurveSet;
}

void NFGGraph::DoGetHighlightedPointSet(NFGPointSet &PS)
{
	unsigned long PointCount = 0;
	for (unsigned long i = 0; i < DataseriesGroupCount; i++) 
		if ((DataseriesGroupArray[i].NumberedCurvePoints) && (DataseriesGroupArray[i].KeyItem.DatasetFlag & DisplayedDatasets))
			PointCount += DataseriesGroupArray[i].GetDataseriesCount();
	
	if (PointCount != PS.PointCount) {
		delete[] PS.PointArray;
		PS.PointArray = new NFGPoint[PointCount];
		PS.PointCount = PointCount;
	}

	PS.ThickLines = ThickLines;
	
	NFGCurve AuxCurve;
	unsigned long index = 0;
	for (unsigned long i = 0; i < DataseriesGroupCount; i++) {
		if ((DataseriesGroupArray[i].NumberedCurvePoints) && (DataseriesGroupArray[i].KeyItem.DatasetFlag & DisplayedDatasets)) {
			for (unsigned long j = 0; j < DataseriesGroupArray[i].GetDataseriesCount(); j++) {
				if (DataseriesGroupArray[i].GetPoint(ScaleValue, j, SelectedStep, PS.PointArray[index])) 
					index++;
			}
		}
	}

	/// index should be eqaul to PointCount under standard circumstances
	if (index < PS.PointCount)
		PS.PointCount = index;
}

NFGPointSet NFGGraph::GetHighlightedPointSet()
{
	if (HighlightedPointSetValid)
		return HighlightedPointSet;
	
	if (DataError) {
		NFGPointSet EmptyPointSet;
		EmptyPointSet.PointArray = NULL;
		EmptyPointSet.PointCount = 0;
		EmptyPointSet.ThickLines = ThickLines;
		EmptyPointSet.PointRadius = 6;
		
		return EmptyPointSet;
	}
	
	DoGetHighlightedPointSet(HighlightedPointSet);
	
	HighlightedPointSetValid = true;
	
	return HighlightedPointSet;
}

NFGPointSet NFGGraph::GetOptimizedHighlightedPointSet()
{
#ifdef __WXGTK__
	wxRect ViewRect = wxRect(GetViewStart(), GetClientSize());
	
	if (OptimizedHighlightedPointSetValid) 
		if (OptimizedHighlightedPointSetRect.Intersects(ViewRect))
			return OptimizedHighlightedPointSet;

	delete[] OptimizedHighlightedPointSet.PointArray;
	OptimizedHighlightedPointSet.PointArray = NULL;
	OptimizedHighlightedPointSet.PointCount = 0;

	NFGPointSet OrigHighlightedPointSet = GetHighlightedPointSet();

	/// Unless printing, take into account the point radius
	wxCoord extra = 0;
	NFGCurveWindow* CW = NULL;
	if ((GraphWindow != NULL) && (CW = GraphWindow->GetCurveWindow())) {
		for (unsigned long i = 0; i < OrigHighlightedPointSet.PointCount; i++) {
			int linewidth = (OrigHighlightedPointSet.ThickLines)?(OrigHighlightedPointSet.PointArray[i].PenThick.GetWidth()):(OrigHighlightedPointSet.PointArray[i].Pen.GetWidth());
			wxCoord extraspace = CW->FromDIP(OrigHighlightedPointSet.PointRadius) + 1 + ((linewidth > 0)?(linewidth):(1));
			if (extraspace > extra) 
				extra = extraspace;
		}
	}
	
	OptimizedHighlightedPointSetRect = ViewRect;
	
	int minx, maxx, miny, maxy;
	wxRect LimitRect;

	/// check of signed 16bit integer range required due to the X interface limits
	NFGPointArray* PointArr = new NFGPointArray;
	PointArr->Alloc(OrigHighlightedPointSet.PointCount);

	minx = ViewRect.x + INT16_MIN + ViewRect.width + extra;
	maxx = ViewRect.x + INT16_MAX - ViewRect.width - extra;
	miny = ViewRect.y + INT16_MIN + ViewRect.height + extra;
	maxy = ViewRect.y + INT16_MAX - ViewRect.height - extra;
	
	LimitRect = wxRect(wxPoint(minx, miny), wxPoint(maxx, maxy));

	for (unsigned long i = 0; i < OrigHighlightedPointSet.PointCount; i++) 
		if (LimitRect.Contains(OrigHighlightedPointSet.PointArray[i].Point))
			PointArr->Add(OrigHighlightedPointSet.PointArray[i]);
	
	OptimizedHighlightedPointSet = OrigHighlightedPointSet;
	OptimizedHighlightedPointSet.PointCount = PointArr->GetCount();
	OptimizedHighlightedPointSet.PointArray = new NFGPoint[OptimizedHighlightedPointSet.PointCount];
	for (unsigned long k = 0; k < OptimizedHighlightedPointSet.PointCount; k++)
		OptimizedHighlightedPointSet.PointArray[k] = PointArr->Item(k);

	PointArr->Clear();
	delete PointArr;

	OptimizedHighlightedPointSetValid = !DataError;
	
	return OptimizedHighlightedPointSet;
#else
	return GetHighlightedPointSet();
#endif
}

wxString NFGGraph::GetXAxisUnits()
{
	return XAxisUnits;
}

wxString NFGGraph::GetYAxisUnits()
{
	return YAxisUnits;
}

NFGAxis NFGGraph::GetXAxis(const int MajorXTicsDistMin, bool NoReuse)
{
	if (	XAxisValid && (MajorXTicsDistMin == XAxisMajorTicsDistMin) &&
		(XAxisBufferRect.x <= GWViewStart.x) && 
		((XAxisBufferRect.x + XAxisBufferRect.width) >= (GWViewStart.x + GWClientSize.GetWidth()))
	)
		return XAxis;
	
	double MinMajorXTicsOrder = 0.0, MinMajorXTicsLog = 0.0, MinMajorXTicsSize = 1.0;
	double MajorXTicsSize = 1.0, MinorXTicsSize = 1.0;
	double FirstMajorXTic = 0.0, FirstMinorXTic = 0.0, LastMajorXTic = 0.0, LastMinorXTic = 0.0;
	unsigned long MajorXTicsCount = 0, MinorXTicsCount = 0;
	
	XAxisMajorTicsDistMin = MajorXTicsDistMin;
	if (XAxisMajorTicsDistMin <= 1)
		XAxisMajorTicsDistMin = 50;
	
	if (ScaleValue.xfactor > 0.0) {
		MinMajorXTicsSize = ((double) XAxisMajorTicsDistMin) / ScaleValue.xfactor;
		MinMajorXTicsOrder = std::floor(std::log10(MinMajorXTicsSize));
		MinMajorXTicsLog = std::log10(MinMajorXTicsSize) - MinMajorXTicsOrder;
		
		if (MinMajorXTicsLog > std::log10(5.0)) {
			MinMajorXTicsOrder += 1.0;
			MajorXTicsSize = 1.0*std::pow(10.0, MinMajorXTicsOrder);
			MinorXTicsSize = 0.25*std::pow(10.0, MinMajorXTicsOrder);
		//	MinorXTicsSize = 0.2*std::pow(10.0, MinMajorXTicsOrder);
		} else
		if (MinMajorXTicsLog > std::log10(2.0)) {
			MajorXTicsSize = 5.0*std::pow(10.0, MinMajorXTicsOrder);
			MinorXTicsSize = 1.0*std::pow(10.0, MinMajorXTicsOrder);
		} else {
			MajorXTicsSize = 2.0*std::pow(10.0, MinMajorXTicsOrder);
			MinorXTicsSize = 0.5*std::pow(10.0, MinMajorXTicsOrder);
		}
		
		XAxisBufferRect = GetBufferRect();
		/// Concerning rounding errors, considering the BufferRect extended on both sides by 1 pixel (instead of just 0.5 pixel) is safer. 
		/// (Any tics just outside the BufferRect do not cause any trouble.)
		long long extra = 1;
		
		/// Unless printing, take into account pen linewith possibly larger than 1
		NFGCurveWindow* CW = NULL;
		if ((GraphWindow != NULL) && (CW = GraphWindow->GetCurveWindow())) 
			if (CW->FromDIP(1) > 1)
				extra += CW->FromDIP(1) / 2;
		
		double RealXMin = ((double) (((long long) XAxisBufferRect.GetLeft()) - extra + ScaleValue.xoffset)) / ScaleValue.xfactor;
		double RealXMax = ((double) (((long long) XAxisBufferRect.GetRight()) + extra + ScaleValue.xoffset)) / ScaleValue.xfactor;

		FirstMajorXTic = std::ceil(RealXMin / MajorXTicsSize)*MajorXTicsSize;
		LastMajorXTic = std::floor(RealXMax / MajorXTicsSize)*MajorXTicsSize;
		MajorXTicsCount = NFGMSTD lround((LastMajorXTic - FirstMajorXTic) / MajorXTicsSize) + 1;
		
		FirstMinorXTic = std::ceil(RealXMin / MinorXTicsSize)*MinorXTicsSize;
		LastMinorXTic = std::floor(RealXMax / MinorXTicsSize)*MinorXTicsSize;
		MinorXTicsCount = NFGMSTD lround((LastMinorXTic - FirstMinorXTic) / MinorXTicsSize) + 1;
		
	} else {
		FirstMajorXTic = GraphRealRect.x + GraphRealRect.width/2.0;
		MajorXTicsCount = (wxFinite(FirstMajorXTic))?(1):(0);
	}
	
	/// possible overflow/underflow does no harm since NFGGraph::SetVisibleRealRect() takes care of XAxis.ShowZeroAxis accordingly
	XAxis.ZeroAxisPos = - ScaleValue.yoffset;
	
	XAxisValid = !NoReuse;
		
	if (XAxis.MajorTicsCount != MajorXTicsCount) {
		delete[] XAxis.MajorTics;
		XAxis.MajorTics = new (std::nothrow) wxCoord[MajorXTicsCount];
		XAxis.MajorTicsCount = (XAxis.MajorTics)?(MajorXTicsCount):(0);
		
		delete[] XAxis.Labels;
		XAxis.Labels = new (std::nothrow) wxString[XAxis.MajorTicsCount];
	}

	if (XAxis.MinorTicsCount != MinorXTicsCount) {
		delete[] XAxis.MinorTics;
		XAxis.MinorTics = new (std::nothrow) wxCoord[MinorXTicsCount];
		XAxis.MinorTicsCount = (XAxis.MinorTics)?(MinorXTicsCount):(0);
	}

	for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++) 
		XAxis.MajorTics[i] = NFGNMRData::llroundnu((FirstMajorXTic + MajorXTicsSize*i)*ScaleValue.xfactor) - ScaleValue.xoffset;

	for (unsigned long i = 0; i < XAxis.MinorTicsCount; i++) 
		XAxis.MinorTics[i] = NFGNMRData::llroundnu((FirstMinorXTic + MinorXTicsSize*i)*ScaleValue.xfactor) - ScaleValue.xoffset;

	if (XAxisZeroExponent || (NFGMSTD lround(MinMajorXTicsOrder) == 0)) {
		if (XAxis.Labels != NULL) 
			for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++) 
				XAxis.Labels[i] = wxString::Format("%.14g", FirstMajorXTic + MajorXTicsSize*i); 
		
		XAxis.AxisLabel = (GetXAxisUnits().IsEmpty()) ? ((wxString) wxEmptyString) : ("[" + GetXAxisUnits() + "]");

	} else {
		if (XAxis.Labels != NULL) 
			for (unsigned long i = 0; i < XAxis.MajorTicsCount; i++) 
				XAxis.Labels[i] = wxString::Format("%.14g", (FirstMajorXTic + MajorXTicsSize*i) / pow(10.0, MinMajorXTicsOrder)); 
		
		XAxis.AxisLabel = wxString::Format("[*1e%.14g", MinMajorXTicsOrder) + ((GetXAxisUnits().IsEmpty()) ? ("]") : (" " + GetXAxisUnits() + "]"));
	}
	
	return XAxis;
}


NFGAxis NFGGraph::GetYAxis(const int MajorYTicsDistMin, bool NoReuse)
{
	if (	YAxisValid && (MajorYTicsDistMin == YAxisMajorTicsDistMin) &&
		(YAxisBufferRect.y <= GWViewStart.y) && 
		((YAxisBufferRect.y + YAxisBufferRect.height) >= (GWViewStart.y + GWClientSize.GetHeight()))
	)
		return YAxis;
	
	double MinMajorYTicsOrder = 0.0, MinMajorYTicsLog = 0.0, MinMajorYTicsSize = 1.0;
	double MajorYTicsSize = 1.0, MinorYTicsSize = 1.0;
	double FirstMajorYTic = 0.0, FirstMinorYTic = 0.0, LastMajorYTic = 0.0, LastMinorYTic = 0.0;
	unsigned long MajorYTicsCount = 0, MinorYTicsCount = 0;
	
	YAxisMajorTicsDistMin = MajorYTicsDistMin;
	if (YAxisMajorTicsDistMin <= 1)
		YAxisMajorTicsDistMin = 50;
	
	if (ScaleValue.yfactor < 0.0) {
		MinMajorYTicsSize = - ((double) YAxisMajorTicsDistMin) / ScaleValue.yfactor;
		MinMajorYTicsOrder = std::floor(std::log10(MinMajorYTicsSize));
		MinMajorYTicsLog = std::log10(MinMajorYTicsSize) - MinMajorYTicsOrder;
		
		if (MinMajorYTicsLog > std::log10(5.0)) {
			MinMajorYTicsOrder += 1.0;
			MajorYTicsSize = 1.0*std::pow(10.0, MinMajorYTicsOrder);
			MinorYTicsSize = 0.25*std::pow(10.0, MinMajorYTicsOrder);
		//	MinorYTicsSize = 0.2*std::pow(10.0, MinMajorYTicsOrder);
		} else
		if (MinMajorYTicsLog > std::log10(2.0)) {
			MajorYTicsSize = 5.0*std::pow(10.0, MinMajorYTicsOrder);
			MinorYTicsSize = 1.0*std::pow(10.0, MinMajorYTicsOrder);
		} else {
			MajorYTicsSize = 2.0*std::pow(10.0, MinMajorYTicsOrder);
			MinorYTicsSize = 0.5*std::pow(10.0, MinMajorYTicsOrder);
		}
		
		YAxisBufferRect = GetBufferRect();
		/// Concerning rounding errors, considering the BufferRect extended on both sides by 1 pixel (instead of just 0.5 pixel) is safer. 
		/// (Any tics just outside the BufferRect do not cause any trouble.)
		long long extra = 1;
		
		/// Unless printing, take into account pen linewith possibly larger than 1
		NFGCurveWindow* CW = NULL;
		if ((GraphWindow != NULL) && (CW = GraphWindow->GetCurveWindow())) 
			if (CW->FromDIP(1) > 1)
				extra += CW->FromDIP(1) / 2;
		
		double RealYMin = ((double) (((long long) YAxisBufferRect.GetBottom()) + extra + ScaleValue.yoffset)) / ScaleValue.yfactor;
		double RealYMax = ((double) (((long long) YAxisBufferRect.GetTop()) - extra + ScaleValue.yoffset)) / ScaleValue.yfactor;

		FirstMajorYTic = std::ceil(RealYMin / MajorYTicsSize)*MajorYTicsSize;
		LastMajorYTic = std::floor(RealYMax / MajorYTicsSize)*MajorYTicsSize;
		MajorYTicsCount = NFGMSTD lround((LastMajorYTic - FirstMajorYTic) / MajorYTicsSize) + 1;
		
		FirstMinorYTic = std::ceil(RealYMin / MinorYTicsSize)*MinorYTicsSize;
		LastMinorYTic = std::floor(RealYMax / MinorYTicsSize)*MinorYTicsSize;
		MinorYTicsCount = NFGMSTD lround((LastMinorYTic - FirstMinorYTic) / MinorYTicsSize) + 1;
		
	} else {
		FirstMajorYTic = GraphRealRect.y + GraphRealRect.height/2.0;
		MajorYTicsCount = (wxFinite(FirstMajorYTic))?(1):(0);
	}
	
	/// possible overflow/underflow does no harm since YAxis.ShowZeroAxis is false
	YAxis.ZeroAxisPos = - ScaleValue.xoffset;
	
	YAxisValid = !NoReuse;
	
	if (YAxis.MajorTicsCount != MajorYTicsCount) {
		delete[] YAxis.MajorTics;
		YAxis.MajorTics = new (std::nothrow) wxCoord[MajorYTicsCount];
		YAxis.MajorTicsCount = (YAxis.MajorTics)?(MajorYTicsCount):(0);
		
		delete[] YAxis.Labels;
		YAxis.Labels = new (std::nothrow) wxString[YAxis.MajorTicsCount];
	}

	if (YAxis.MinorTicsCount != MinorYTicsCount) {
		delete[] YAxis.MinorTics;
		YAxis.MinorTics = new (std::nothrow) wxCoord[MinorYTicsCount];
		YAxis.MinorTicsCount = (YAxis.MinorTics)?(MinorYTicsCount):(0);
	}

	for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++) 
		YAxis.MajorTics[i] = NFGNMRData::llroundnu((FirstMajorYTic + MajorYTicsSize*i)*ScaleValue.yfactor) - ScaleValue.yoffset;

	for (unsigned long i = 0; i < YAxis.MinorTicsCount; i++) 
		YAxis.MinorTics[i] = NFGNMRData::llroundnu((FirstMinorYTic + MinorYTicsSize*i)*ScaleValue.yfactor) - ScaleValue.yoffset;

	if (YAxisZeroExponent || (NFGMSTD lround(MinMajorYTicsOrder) == 0)) {
		if (YAxis.Labels != NULL) 
			for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++) 
				YAxis.Labels[i] = wxString::Format("%.14g", FirstMajorYTic + MajorYTicsSize*i); 
		
		YAxis.AxisLabel = (GetYAxisUnits().IsEmpty()) ? ((wxString) wxEmptyString) : ("[" + GetYAxisUnits() + "]");

	} else {
		if (YAxis.Labels != NULL) 
			for (unsigned long i = 0; i < YAxis.MajorTicsCount; i++) 
				YAxis.Labels[i] = wxString::Format("%.14g", (FirstMajorYTic + MajorYTicsSize*i) / pow(10.0, MinMajorYTicsOrder)); 
		
		YAxis.AxisLabel = wxString::Format("[*1e%.14g", MinMajorYTicsOrder) + ((GetYAxisUnits().IsEmpty()) ? ("]") : (" " + GetYAxisUnits() + "]"));
	}

	return YAxis;
}


void NFGGraph::DoSelectStep(unsigned long index, bool OnlySelectedPlotted) 
{
	/// Basic range check
	if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_StepSet, ALL_STEPS) != DATA_OK) 
		index = 0;
	else
		if (index >= NFGNMRData::GetStepNoRange(NMRDataPointer))
			index = (NFGNMRData::GetStepNoRange(NMRDataPointer) > 0)?(NFGNMRData::GetStepNoRange(NMRDataPointer) - 1):(0);

	if (index == SelectedStep) 	/// Nothing to do
		return;
	
	if (OnlySelectedPlotted) {
		SelectedStep = index;
		
		CurveSetValid = false;
		OptimizedCurveSetValid = false;
		DataError = false;
		
		if (GraphWindow != NULL)
			GraphWindow->FreezeGraph();
		
		AutoScaleSelected(AutoZoomHSelected, AutoZoomVSelected);
		
		if (GraphWindow != NULL) {
			GraphWindow->ThawGraph(true);
			GraphWindow->RefreshGraph();
		}
		
	} else {
		NFGCurveWindow* CW = NULL;
		if (GraphWindow != NULL)
			CW = GraphWindow->GetCurveWindow();

		if (CW == NULL) {
			SelectedStep = index;
			
			HighlightedCurveSetValid = false;
			HighlightedPointSetValid = false;
			OptimizedHighlightedCurveSetValid = false;
			OptimizedHighlightedPointSetValid = false;
			
		} else {
			NFGCurveSet OldHighlightedCurveSet = GetOptimizedHighlightedCurveSet();
			NFGPointSet OldHighlightedPointSet = GetOptimizedHighlightedPointSet();

			SelectedStep = index;
		
			wxPoint ViewStart = GetViewStart();
			wxSize ClientSize = GetClientSize();
			wxRect VisibleRect(ViewStart, ClientSize);
			
			for (unsigned long i = 0; i < OldHighlightedCurveSet.CurveCount; i++) {
				if (OldHighlightedCurveSet.CurveArray[i].PointCount == 0) 
					continue;
				
				wxRect BBox = OldHighlightedCurveSet.CurveArray[i].BoundingBox;
				
				int linewidth = (OldHighlightedCurveSet.ThickLines)?(OldHighlightedCurveSet.CurveArray[i].PenThick.GetWidth()):(OldHighlightedCurveSet.CurveArray[i].Pen.GetWidth());
				wxCoord extraspace = ((linewidth > 0)?(linewidth):(1));
				if (OldHighlightedCurveSet.DrawPoints) 
					extraspace += CW->FromDIP(OldHighlightedCurveSet.PointRadius) + 1;

				BBox.Inflate(extraspace);
				
				BBox.Intersect(VisibleRect);
				
				if (!BBox.IsEmpty()) {
					BBox.Offset(-ViewStart.x, -ViewStart.y);
					CW->RefreshRect(BBox);
				}
			}
			
			for (unsigned long i = 0; i < OldHighlightedPointSet.PointCount; i++) {
				wxRect BBox(OldHighlightedPointSet.PointArray[i].Point, wxSize(0, 0));
				int linewidth = (OldHighlightedPointSet.ThickLines)?(OldHighlightedPointSet.PointArray[i].PenThick.GetWidth()):(OldHighlightedPointSet.PointArray[i].Pen.GetWidth());
				BBox.Inflate(CW->FromDIP(OldHighlightedPointSet.PointRadius) + 1 + ((linewidth > 0)?(linewidth):(1)));
				
				BBox.Intersect(VisibleRect);
				
				if (!BBox.IsEmpty()) {
					BBox.Offset(-ViewStart.x, -ViewStart.y);
					CW->RefreshRect(BBox);
				}
			}
			
			HighlightedCurveSetValid = false;
			HighlightedPointSetValid = false;
			OptimizedHighlightedCurveSetValid = false;
			OptimizedHighlightedPointSetValid = false;

			GetOptimizedHighlightedCurveSet();
			GetOptimizedHighlightedPointSet();
			
			for (unsigned long i = 0; i < HighlightedCurveSet.CurveCount; i++) {
				if (HighlightedCurveSet.CurveArray[i].PointCount == 0) 
					continue;
				
				wxRect BBox = HighlightedCurveSet.CurveArray[i].BoundingBox;
				
				int linewidth = (HighlightedCurveSet.ThickLines)?(HighlightedCurveSet.CurveArray[i].PenThick.GetWidth()):(HighlightedCurveSet.CurveArray[i].Pen.GetWidth());
				wxCoord extraspace = ((linewidth > 0)?(linewidth):(1));
				if (HighlightedCurveSet.DrawPoints) 
					extraspace += CW->FromDIP(HighlightedCurveSet.PointRadius) + 1;

				BBox.Inflate(extraspace);
				
				BBox.Intersect(VisibleRect);
				
				if (!BBox.IsEmpty()) {
					BBox.Offset(-ViewStart.x, -ViewStart.y);
					CW->RefreshRect(BBox);
				}
			}
			
			for (unsigned long i = 0; i < HighlightedPointSet.PointCount; i++) {
				wxRect BBox(HighlightedPointSet.PointArray[i].Point, wxSize(0, 0));
				int linewidth = (HighlightedPointSet.ThickLines)?(HighlightedPointSet.PointArray[i].PenThick.GetWidth()):(HighlightedPointSet.PointArray[i].Pen.GetWidth());
				BBox.Inflate(CW->FromDIP(HighlightedPointSet.PointRadius) + 1 + ((linewidth > 0)?(linewidth):(1)));
				
				BBox.Intersect(VisibleRect);
				
				if (!BBox.IsEmpty()) {
					BBox.Offset(-ViewStart.x, -ViewStart.y);
					CW->RefreshRect(BBox);
				}
			}
		}
	}
	
	if (SerDoc != NULL)
		SerDoc->SelectedStepChangedNotify();

}

///	Called by NFGGraphWindow::OnMouse event handler or directly
void NFGGraph::SelectStep(unsigned long index) 
{
	DoSelectStep(index, false);
}

void NFGGraph::SelectNextStep()
{
	SelectStep(SelectedStep + 1);
}

void NFGGraph::SelectPrevStep()
{
	if (SelectedStep > 0)
		SelectStep(SelectedStep - 1);
//	else
//		SelectedStep = 0;
}

///	Scale-change commands
void NFGGraph::Zoom(bool ZoomIn, bool ZoomHorizontal, bool ZoomVertical, wxPoint point)
{
	ZoomHorizontal = ZoomHorizontal && (!ZoomIn || !DisableZoomInH);
	ZoomVertical = ZoomVertical && (!ZoomIn || !DisableZoomInV);
	
	if (!ZoomHorizontal && !ZoomVertical)
		return;
	
	NFGRealRect rect = RequestedVisibleRealRect;
	
	wxRealPoint FixedPoint;

	if (wxRect(GWClientSize).Contains(point)) {
		FixedPoint = UnscrolledPoint2RealPoint(point + GWViewStart, true);
		
		/// Sticky axes behavoiur for convenience
		if (fabs(FixedPoint.y/VisibleRealRect.height) < 0.1)	/// Close enough to x-axis
			if ((VisibleRealRect.y <= 0.0) && ((VisibleRealRect.y + VisibleRealRect.height) >= 0.0))	/// x-axis visible
				FixedPoint.y = 0.0;
		
		if (fabs(FixedPoint.x/VisibleRealRect.width) < 0.1)	/// Close enough to y-axis
			if ((VisibleRealRect.x <= 0.0) && ((VisibleRealRect.x + VisibleRealRect.width) >= 0.0))	/// y-axis visible
				FixedPoint.x = 0.0;
			
	} else {
		FixedPoint.x = VisibleRealRect.x + VisibleRealRect.width/2.0;
		FixedPoint.y = VisibleRealRect.y + VisibleRealRect.height/2.0;
		
		if (StickyZoomLeft && (GWViewStart.x == 0)) 
			FixedPoint.x = GraphRealRect.x;
		
		if (StickyZoomBottom && ((GWViewStart.y + GWClientSize.GetHeight()) == GWVirtualSize.GetHeight())) 
			FixedPoint.y = GraphRealRect.y;
	}
	
	if (ZoomHorizontal) {
		AutoZoomHSelected = false;
		AutoZoomHAll = false;

		if (ZoomIn) {
			rect.x = FixedPoint.x - (FixedPoint.x - VisibleRealRect.x)/M_SQRT2;
			rect.width = VisibleRealRect.width/M_SQRT2;
		} else {	/// Zoom out
			rect.x = FixedPoint.x - (FixedPoint.x - VisibleRealRect.x)*M_SQRT2;
			rect.width = VisibleRealRect.width*M_SQRT2;
		}
	}
	
	if (ZoomVertical) {
		AutoZoomVSelected = false;
		AutoZoomVAll = false;

		if (ZoomIn) {
			rect.y = FixedPoint.y - (FixedPoint.y - VisibleRealRect.y)/M_SQRT2;
			rect.height = VisibleRealRect.height/M_SQRT2;
		} else {
			rect.y = FixedPoint.y - (FixedPoint.y - VisibleRealRect.y)*M_SQRT2;
			rect.height = VisibleRealRect.height*M_SQRT2;
		}
	}
	
	SetVisibleRealRect(rect);
}

void NFGGraph::AutoScaleAll(bool AutoScaleHorizontal, bool AutoScaleVertical, bool SetFlag) 
{
	if ((!AutoScaleHorizontal) && (!AutoScaleVertical)) 
		return;
	
	GetBoundingRealRect();
	NFGRealRect NewVisibleRealRect = RequestedVisibleRealRect;
	
	if (AutoScaleHorizontal) {
		NewVisibleRealRect.x = BoundingRealRect.x;
		NewVisibleRealRect.width = BoundingRealRect.width;
		
		AutoZoomHSelected = false;
		if (SetFlag)
			AutoZoomHAll = true;

		ConstrainZoomOverride.minx = NAN;
		ConstrainZoomOverride.maxx = NAN;
	}

	if (AutoScaleVertical) {
		NewVisibleRealRect.y = BoundingRealRect.y;
		NewVisibleRealRect.height = BoundingRealRect.height;
		
		AutoZoomVSelected = false;
		if (SetFlag)
			AutoZoomVAll = true;

		ConstrainZoomOverride.miny = NAN;
		ConstrainZoomOverride.maxy = NAN;
	}
	
	SetVisibleRealRect(NewVisibleRealRect);
}

void NFGGraph::AutoScaleSelected(bool AutoScaleHorizontal, bool AutoScaleVertical, bool SetFlag) 
{
	if ((!AutoScaleHorizontal) && (!AutoScaleVertical))
		return;
	
	NFGRealRect SelectedStepBoundingRealRect = GetSelectedStepBoundingRealRect();
	NFGRealRect NewVisibleRealRect = RequestedVisibleRealRect;
	
	if (AutoScaleHorizontal) {
		NewVisibleRealRect.x = SelectedStepBoundingRealRect.x;
		NewVisibleRealRect.width = SelectedStepBoundingRealRect.width;

		if (SetFlag)
			AutoZoomHSelected = true;
		AutoZoomHAll = false;
		
		ConstrainZoomOverride.minx = NAN;
		ConstrainZoomOverride.maxx = NAN;
	}
	
	if (AutoScaleVertical) {
		NewVisibleRealRect.y = SelectedStepBoundingRealRect.y;
		NewVisibleRealRect.height = SelectedStepBoundingRealRect.height;

		if (SetFlag)
			AutoZoomVSelected = true;
		AutoZoomVAll = false;
		
		ConstrainZoomOverride.miny = NAN;
		ConstrainZoomOverride.maxy = NAN;
	}
	
	SetVisibleRealRect(NewVisibleRealRect);
}


void NFGGraph::CheckConstrains(bool AllowMove)
{
	SetVisibleRealRect(RequestedVisibleRealRect, AllowMove);
}

ZoomParams NFGGraph::GetZoomParams()
{
	ZoomParams zparams;
	
	zparams.AutoZoomHAll = false;
	zparams.AutoZoomHSelected = false;
	zparams.AutoZoomVAll = false;
	zparams.AutoZoomVSelected = false;
	
	if (AutoZoomVAll)
		zparams.AutoZoomVAll = true;
	else if (AutoZoomVSelected)
		zparams.AutoZoomVSelected = true;
	
	if (AutoZoomHAll)
		zparams.AutoZoomHAll = true;
	else if (AutoZoomHSelected)
		zparams.AutoZoomHSelected = true;
	
	zparams.DrawPoints = DrawPoints;
	zparams.ThickLines = ThickLines;

	zparams.minx = VisibleRealRect.x;
	zparams.miny = VisibleRealRect.y;
	zparams.maxx = VisibleRealRect.x + VisibleRealRect.width;
	zparams.maxy = VisibleRealRect.y + VisibleRealRect.height;
	
	return zparams;
}


ZoomParams NFGGraph::SetZoomParams(ZoomParams zparams)
{
	NFGRealRect NewVisibleRealRect;
	
	NewVisibleRealRect.x = fmin(zparams.minx, zparams.maxx);
	NewVisibleRealRect.y = fmin(zparams.miny, zparams.maxy);
	NewVisibleRealRect.width = fabs(zparams.maxx - zparams.minx);
	NewVisibleRealRect.height = fabs(zparams.maxy - zparams.miny);

	ConstrainZoomOverride.minx = NAN;
	ConstrainZoomOverride.maxx = NAN;
	ConstrainZoomOverride.miny = NAN;
	ConstrainZoomOverride.maxy = NAN;

	GetBoundingRealRect();

	/// some checks to prevent strange behaviour if unexpected parameters were passed
	if (!wxFinite(NewVisibleRealRect.x) || !wxFinite(NewVisibleRealRect.width) || (NewVisibleRealRect.width < 1.0e-10))
		zparams.AutoZoomHAll = true;
	
	if (!wxFinite(NewVisibleRealRect.y) || !wxFinite(NewVisibleRealRect.height) || (NewVisibleRealRect.height < 1.0e-10)) 
		zparams.AutoZoomVAll = true;
	
	if (zparams.AutoZoomHAll) {
		NewVisibleRealRect.x = BoundingRealRect.x;
		NewVisibleRealRect.width = BoundingRealRect.width;
		
		AutoZoomHSelected = false;
		AutoZoomHAll = true;
	} else if (zparams.AutoZoomHSelected) {
		NFGRealRect SelectedStepBoundingRealRect = GetSelectedStepBoundingRealRect();
		
		NewVisibleRealRect.x = SelectedStepBoundingRealRect.x;
		NewVisibleRealRect.width = SelectedStepBoundingRealRect.width;
		
		AutoZoomHSelected = true;
		AutoZoomHAll = false;
	} else {
		AutoZoomHSelected = false;
		AutoZoomHAll = false;
		
		if ((NewVisibleRealRect.x < BoundingRealRect.x) && ConstrainZoomLeftAllowOverride)
			ConstrainZoomOverride.minx = NewVisibleRealRect.x;
		
		if (((NewVisibleRealRect.x + NewVisibleRealRect.width) > (BoundingRealRect.x + BoundingRealRect.width)) && ConstrainZoomRightAllowOverride)
			ConstrainZoomOverride.maxx = NewVisibleRealRect.x + NewVisibleRealRect.width;
	}
	
	if (zparams.AutoZoomVAll) {
		NewVisibleRealRect.y = BoundingRealRect.y;
		NewVisibleRealRect.height = BoundingRealRect.height;
		
		AutoZoomVSelected = false;
		AutoZoomVAll = true;
	} else if (zparams.AutoZoomVSelected) {
		NFGRealRect SelectedStepBoundingRealRect = GetSelectedStepBoundingRealRect();
		
		NewVisibleRealRect.y = SelectedStepBoundingRealRect.y;
		NewVisibleRealRect.height = SelectedStepBoundingRealRect.height;
		
		AutoZoomVSelected = true;
		AutoZoomVAll = false;
	} else {
		AutoZoomVSelected = false;
		AutoZoomVAll = false;
		
		if ((NewVisibleRealRect.y < BoundingRealRect.y) && ConstrainZoomBottomAllowOverride)
			ConstrainZoomOverride.miny = NewVisibleRealRect.y;
		
		if (((NewVisibleRealRect.y + NewVisibleRealRect.height) > (BoundingRealRect.y + BoundingRealRect.height)) && ConstrainZoomTopAllowOverride)
			ConstrainZoomOverride.maxy = NewVisibleRealRect.y + NewVisibleRealRect.height;
	}
	
	DrawPoints = zparams.DrawPoints;
	ThickLines = zparams.ThickLines;
	
	SetVisibleRealRect(NewVisibleRealRect, false, false);
	
	return GetZoomParams();
}


void NFGGraph::SetVisibleRealRect(NFGRealRect rect, bool AllowMove, bool Force)
{
	/// just in case...
	if (rect.width < 0.0) {
		rect.x += rect.width;
		rect.width *= -1.0;
	}
	if (rect.height < 0.0) {
		rect.y += rect.height;
		rect.height *= -1.0;
	}
	
	GetBoundingRealRect();

	double minx = NFGMSTD fmin(BoundingRealRect.x, ConstrainZoomOverride.minx);
	double maxx = NFGMSTD fmax(BoundingRealRect.x + BoundingRealRect.width, ConstrainZoomOverride.maxx);
	double miny = NFGMSTD fmin(BoundingRealRect.y, ConstrainZoomOverride.miny);
	double maxy = NFGMSTD fmax(BoundingRealRect.y + BoundingRealRect.height, ConstrainZoomOverride.maxy);
	
	ConstrainZoomBottom = (BoundingRealRect.y == 0.0) && wxIsNaN(ConstrainZoomOverride.miny);
	
	if (!Force) {
		if (AllowMove) {
			/// try to move the range to left if necessary
			if (ConstrainZoomRight && ((rect.x + rect.width) > maxx))
				rect.x = maxx - rect.width;
			
			/// try to move the range to right if necessary
			if (ConstrainZoomLeft && (rect.x < minx))	
				rect.x = minx;
			
			/// try to move the range to bottom if necessary
			if (ConstrainZoomTop && ((rect.y + rect.height) > maxy))
				rect.y = maxy - rect.height;
			
			/// try to move the range to top if necessary
			if (ConstrainZoomBottom && (rect.y < miny))	
				rect.y = miny;
		}
		
		/// trim the range if required
		if (ConstrainZoomRight && ((rect.x + rect.width) > maxx)) {
			if (rect.x > maxx) {
				rect.x = maxx;
				rect.width = 0.0;
			} else {
				rect.width = maxx - rect.x;
			}
		}
		
		/// trim the range if required
		if (ConstrainZoomLeft && (rect.x < minx))	{
			if ((rect.x + rect.width) < minx) {
				rect.x = minx;
				rect.width = 0.0;
			} else {
				rect.width -= minx - rect.x;
				rect.x = minx;
			}
		}
		
		/// trim the range if required
		if (ConstrainZoomTop && ((rect.y + rect.height) > maxy)) {
			if (rect.y > maxy) {
				rect.y = maxy;
				rect.height = 0.0;
			} else {
				rect.height = maxy - rect.y;
			}
		}
		
		/// trim the range if required
		if (ConstrainZoomBottom && (rect.y < miny)) {
			if ((rect.y + rect.height) < miny) {
				rect.y = miny;
				rect.height = 0.0;
			} else {
				rect.height -= miny - rect.y;
				rect.y = miny;
			}
		}
	}

	minx = NFGMSTD fmin(minx, rect.x);
	maxx = NFGMSTD fmax(maxx, rect.x + rect.width);
	miny = NFGMSTD fmin(miny, rect.y);
	maxy = NFGMSTD fmax(maxy, rect.y + rect.height);

	RequestedVisibleRealRect = rect;
	VisibleRealRect = rect;	/// VisibleRealRect can be overridden in SetScale()
	
	GraphRealRect.x = minx;
	GraphRealRect.width = maxx - minx;
	GraphRealRect.y = miny;
	GraphRealRect.height = maxy - miny;

	XAxis.ShowZeroAxis = ((GraphRealRect.y < 0.0) && ((GraphRealRect.y + GraphRealRect.height) > 0.0)) || ((GraphRealRect.y == 0.0) && (GraphRealRect.height == 0.0));
	StickyZoomBottom = (GraphRealRect.y == 0.0);
	
	if (GraphWindow != NULL)
		GraphWindow->RefreshGraph();
	
	if (SerDoc != NULL)
		SerDoc->ZoomParamsChangedNotify();
}

///	Info functions
NFGRealRect NFGGraph::GetVisibleRealRect()
{
	return VisibleRealRect;
}

bool NFGGraph::DoGetBoundingRealRect(NFGRealRect &BRealRect, long No)
{
	bool SymmetricYRange = false;
	bool IncludeYZero = false;
	bool DominantBBox = false;
	
	NFGRealBBox RealBBox, AuxBBox;
	RealBBox.minx = HUGE_VAL; 
	RealBBox.maxx = - HUGE_VAL; 
	RealBBox.miny = HUGE_VAL; 
	RealBBox.maxy = - HUGE_VAL; 
	
	for (unsigned long i = 0; i < DataseriesGroupCount; i++) {
		if (!(DataseriesGroupArray[i].KeyItem.DatasetFlag & DisplayedDatasets)) 
			continue;
		
		SymmetricYRange = SymmetricYRange || DataseriesGroupArray[i].SymmetricYRange;
		IncludeYZero = IncludeYZero || DataseriesGroupArray[i].IncludeYZero;
		DominantBBox = DominantBBox || DataseriesGroupArray[i].DominantBBox;
	}
	
	unsigned long hits = 0;
	for (unsigned long i = 0; i < DataseriesGroupCount; i++) {
		
		if (!(DataseriesGroupArray[i].KeyItem.DatasetFlag & DisplayedDatasets)) 
			continue;
		
		if (DominantBBox && !(DataseriesGroupArray[i].DominantBBox))
			continue;
		
		if (!DataseriesGroupArray[i].GetRealBBox(AuxBBox, No)) 
			return false;
		
		if (AuxBBox.minx < RealBBox.minx)
			RealBBox.minx = AuxBBox.minx;
		if (AuxBBox.maxx > RealBBox.maxx)
			RealBBox.maxx = AuxBBox.maxx;
		
		if (AuxBBox.miny < RealBBox.miny)
			RealBBox.miny = AuxBBox.miny;
		if (AuxBBox.maxy > RealBBox.maxy)
			RealBBox.maxy = AuxBBox.maxy;
		
		hits++;
	}

	if ((hits == 0) || (RealBBox.minx > RealBBox.maxx)) {
		RealBBox.minx = NAN; 
		RealBBox.maxx = NAN; 
	}
	
	if ((hits == 0) || (RealBBox.miny > RealBBox.maxy)) {
		RealBBox.miny = NAN; 
		RealBBox.maxy = NAN; 
	}
	
	if (SymmetricYRange) {
		RealBBox.maxy = NFGMSTD fmax(std::fabs(RealBBox.miny), std::fabs(RealBBox.maxy));
		RealBBox.miny = -RealBBox.maxy;
	}
	
	if (IncludeYZero) {
		if (RealBBox.miny > 0.0) 
			RealBBox.miny = 0.0;
		if (RealBBox.maxy < 0.0)
			RealBBox.maxy = 0.0;
	}

	BRealRect.x = RealBBox.minx;
	BRealRect.y = RealBBox.miny;
	BRealRect.width = RealBBox.maxx - RealBBox.minx;
	BRealRect.height = RealBBox.maxy - RealBBox.miny;

	return true;
}

NFGRealRect NFGGraph::GetBoundingRealRect()
{
	if (BoundingRealRectValid)
		return BoundingRealRect;
	
	if (DataError || !DoGetBoundingRealRect(BoundingRealRect, ALL_STEPS)) {
		DataError = true;
		
		BoundingRealRect.x = NAN; 
		BoundingRealRect.y = NAN; 
		BoundingRealRect.width = 0.0; 
		BoundingRealRect.height = 0.0; 
		return BoundingRealRect;
	}
	
	BoundingRealRectValid = true;

	return BoundingRealRect;
}

NFGRealRect NFGGraph::GetSelectedStepBoundingRealRect()
{
	NFGRealRect BRealRect;
	
	if (DataError || !DoGetBoundingRealRect(BRealRect, SelectedStep)) {
		DataError = true;
		
		NFGRealRect EmptyRealRect;
		EmptyRealRect.x = NAN; 
		EmptyRealRect.y = NAN; 
		EmptyRealRect.width = 0.0; 
		EmptyRealRect.height = 0.0; 
		return EmptyRealRect;
	}
		
	return BRealRect;
}

NFGRealRect NFGGraph::GetGraphRealRect()
{
	return GraphRealRect;
}


unsigned long NFGGraph::GetSelectedStep()
{
	return SelectedStep;
}

wxString NFGGraph::GetSelectedStepLabel()
{
	if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_StepSet, ALL_STEPS) != DATA_OK) 
		return wxEmptyString;
	
	if (SelectedStep < StepNoRange(NMRDataPointer)) {
		wxString Label = wxString::Format("%.10g", StepAssocValue(NMRDataPointer, SelectedStep));
		
		if (NMRDataPointer->AcquInfo.AssocValueUnits != NULL) {
			Label += wxString(" ");
			Label += wxString(NMRDataPointer->AcquInfo.AssocValueUnits);
		}
		return Label;
	} else
		return wxEmptyString;
}

wxString NFGGraph::GetSelectedStepState()
{
	if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_StepSet, ALL_STEPS) != DATA_OK) 
		return wxEmptyString;
	
	if (SelectedStep < StepNoRange(NMRDataPointer)) {
		unsigned char flag = StepFlag(NMRDataPointer, SelectedStep) & 0x0f;
		
		if (flag & STEP_NO_ENVELOPE)
			return "not in FFT envelope";
		else
			
		if (flag & STEP_IGNORE)
			return "ignored";
		else
		if (flag & STEP_BLANK)
			return "blank";
		else
		if (flag & STEP_NO_SHOW)
			return "not shown";
		else
			return "OK";
	} 
	
	return wxEmptyString;
}

unsigned char NFGGraph::GetSelectedStepStateFlag()
{
	if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_StepSet, ALL_STEPS) != DATA_OK) 
		return 0;
	
	if (SelectedStep < StepNoRange(NMRDataPointer)) {
		unsigned char flag = StepFlag(NMRDataPointer, SelectedStep) & 0x0f;
		
		if (flag & STEP_NO_ENVELOPE)
			return STEP_NO_ENVELOPE;
		else
		/// normalize the flag
		if (flag & STEP_IGNORE)
			return STEP_IGNORE;
		else
		if (flag & STEP_BLANK)
			return STEP_BLANK;
		else
		if (flag & STEP_NO_SHOW)
			return STEP_NO_SHOW;
		else
			return STEP_OK;
	} else
		return 0;
}

wxRealPoint NFGGraph::UnscrolledPoint2RealPoint(wxPoint point, bool CoarserRounding)
{
	wxRealPoint realpoint;

	if (ScaleValue.xfactor > 0.0)
		realpoint.x = ((double) (((long long) point.x) + ScaleValue.xoffset))/ScaleValue.xfactor;
	else
		realpoint.x = GraphRealRect.x + GraphRealRect.width/2.0;
	
	if (ScaleValue.yfactor < 0.0)
		realpoint.y = ((double) (((long long) point.y) + ScaleValue.yoffset))/ScaleValue.yfactor;
	else
		realpoint.y = GraphRealRect.y + GraphRealRect.height/2.0;
	
	/// round within the tolerance corresponding to ScaleValue
	if ((ScaleValue.xfactor > 0.0) && wxFinite(realpoint.x)) {
		double tol = 0.5/ScaleValue.xfactor;
		double order = std::floor(std::log10(tol));
		tol *= std::pow(10.0, -order);
		double val = std::fabs(realpoint.x)*std::pow(10.0, -order);
		double aux = 10.0 * NFGMSTD trunc(0.1*val);
		
		if (CoarserRounding && ((((val - aux) - tol) < 0.0) || (((val - aux) + tol) > 10.0))) 
			order += 1.0;
		
		realpoint.x = NFGMSTD round(realpoint.x*std::pow(10.0, -order))*std::pow(10.0, order);
	}
	
	if ((ScaleValue.yfactor < 0.0) && wxFinite(realpoint.y)) {
		double tol = -0.5/ScaleValue.yfactor;
		double order = std::floor(std::log10(tol));
		tol *= std::pow(10.0, -order);
		double val = std::fabs(realpoint.y)*std::pow(10.0, -order);
		double aux = 10.0 * NFGMSTD trunc(0.1*val);
		
		if (CoarserRounding && ((((val - aux) - tol) < 0.0) || (((val - aux) + tol) > 10.0))) 
			order += 1.0;
		
		realpoint.y = NFGMSTD round(realpoint.y*std::pow(10.0, -order))*std::pow(10.0, order);
	}
	
	/// "sticky" edges
	if (point.x == 0)
		realpoint.x = GraphRealRect.x;
	if (point.x == (GWVirtualSize.GetWidth() - 1))
		realpoint.x = GraphRealRect.x + GraphRealRect.width;
	
	if (point.y == 0)
		realpoint.y = GraphRealRect.y + GraphRealRect.height;
	if (point.y == (GWVirtualSize.GetHeight() - 1))
		realpoint.y = GraphRealRect.y;
	
	return realpoint;
}

void NFGGraph::TrackMousePos(wxPoint pt)
{
	MousePos = pt;
}

void NFGGraph::RegisterMousePos(wxPoint pt)
{
	if (wxRect(GWClientSize).Contains(pt)) {
		ReferenceRealPoint = UnscrolledPoint2RealPoint(pt + GWViewStart);
		ReferenceRealPointValid = true;
	} else
		DiscardMousePos();
}

void NFGGraph::DiscardMousePos()
{
	ReferenceRealPointValid = false;

	ReferenceRealPoint.x = 0.0;
	ReferenceRealPoint.y = 0.0;
}

void NFGGraph::CopyMousePos(wxPoint pt, bool Append)
{
	TrackMousePos(pt);
	
	wxString Label = wxString("x = ") + GetCursorX() + wxString("\t") + wxString("y = ") + GetCursorY();
	if (ReferenceRealPointValid)
		Label += wxString("\t") + wxString("x-x' = ") + GetCursorDeltaX() + wxString("\t") + wxString("y/y' = ") + GetCursorYRatio();
	Label += wxString("\n");
	
	if (SerDoc != NULL) 
		SerDoc->CopyCursorPosCommand(Label, Append);
}

wxString NFGGraph::GetCursorX()
{
	if (wxRect(GWClientSize).Contains(MousePos)) 
		return wxString::Format("%.8g ", UnscrolledPoint2RealPoint(MousePos + GWViewStart).x) + GetXAxisUnits();
	else 
		return wxEmptyString;
}

wxString NFGGraph::GetCursorY()
{
	if (wxRect(GWClientSize).Contains(MousePos)) 
		return wxString::Format("%.4g ", UnscrolledPoint2RealPoint(MousePos + GWViewStart).y) + GetYAxisUnits();
	else
		return wxEmptyString;
}

wxString NFGGraph::GetCursorDeltaX()
{
	if (ReferenceRealPointValid && wxRect(GWClientSize).Contains(MousePos)) {
		if (XAxisZeroExponent)
			return wxString::Format("%.6f ", UnscrolledPoint2RealPoint(MousePos + GWViewStart).x - ReferenceRealPoint.x) + GetXAxisUnits();
		else
			return wxString::Format("%.6g ", UnscrolledPoint2RealPoint(MousePos + GWViewStart).x - ReferenceRealPoint.x) + GetXAxisUnits();
	} 
	
	return wxEmptyString;
}

wxString NFGGraph::GetCursorYRatio()
{
	if (ReferenceRealPointValid && wxRect(GWClientSize).Contains(MousePos)) 
		return wxString::Format("%g", UnscrolledPoint2RealPoint(MousePos + GWViewStart).y / ReferenceRealPoint.y);
	else 
		return wxEmptyString;
}
