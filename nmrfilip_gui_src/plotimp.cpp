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

#include "plotimp.h"

#include "nmrdata.h"
#include "doc.h"
#include "plotwin.h"
#include "gui_ids.h"


NFGGraphTDD::NFGGraphTDD(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style) : NFGGraph(NMRDataPtr, document, style, Flag(CHECK_EchoPeaksEnvelope) | Flag(CHECK_ChunkSet), 0)
{
	DisplayedDatasets = 	(1ul << (ID_TDDReal - DatasetIDMin)) | 
					(1ul << (ID_TDDImag - DatasetIDMin));
	
	DisplayedDatasetsMask = 	(1ul << (ID_TDDReal - DatasetIDMin)) | 
						(1ul << (ID_TDDImag - DatasetIDMin)) |
						(1ul << (ID_TDDModule - DatasetIDMin)) |
						(1ul << (ID_TDDEchoPeaksEnvelope - DatasetIDMin));
	
	DrawPoints = false;
	ThickLines = false;

	DataseriesGroupArray = new NFGDataseriesGroup[4];
	DataseriesGroupCount = 4;
	
	wxWindow *win = (document != NULL)?(document->GetDocumentWindow()):(NULL);
	
	wxPen CurvePen(wxColour(255, 0, 0), wxWindow::FromDIP(1, win));

	wxPen CurvePenThick(CurvePen);
	CurvePenThick.SetWidth(wxWindow::FromDIP(2, win));
	
	wxPen CurvePenBW = *wxBLACK_PEN;
	CurvePenBW.SetStyle(wxPENSTYLE_LONG_DASH);


	wxPen AltCurvePen(CurvePen);
	AltCurvePen.SetColour(wxColour(255, 224, 224));
	
	wxPen AltCurvePenThick(CurvePenThick);
	AltCurvePenThick.SetColour(wxColour(255, 224, 224));
	
	wxPen AltCurvePenBW(CurvePenBW);
	AltCurvePenBW.SetStyle(wxPENSTYLE_TRANSPARENT);

	DataseriesGroupArray[1].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[1].WatchedNMRData = CHECK_ChunkSet;
	DataseriesGroupArray[1].GetNMRPts = NFGNMRData::GetTDDRealPts;
	DataseriesGroupArray[1].GetNMRRPtBB = NFGNMRData::GetTDDRealRPtBB;
	DataseriesGroupArray[1].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[1].GetNMRIndexRange = NFGNMRData::GetTDDIndexRange;

	DataseriesGroupArray[1].NumberedDataseries = true;
	DataseriesGroupArray[1].NumberedCurvePoints = false;

	DataseriesGroupArray[1].HasHeadAndTail = true;

	DataseriesGroupArray[1].SymmetricYRange = true;
	DataseriesGroupArray[1].IncludeYZero = true;
	DataseriesGroupArray[1].NondecreasingX = true;
	DataseriesGroupArray[1].DominantBBox = false;

	DataseriesGroupArray[1].KeyItem.Label = wxString("Real");
	DataseriesGroupArray[1].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[1].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[1].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[1].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[1].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[1].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[1].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[1].KeyItem.PointRadius = 4;
	DataseriesGroupArray[1].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[1].KeyItem.DatasetFlag = 1ul << (ID_TDDReal - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 255, 0));
	CurvePenThick.SetColour(wxColour(0, 255, 0));
	CurvePenBW.SetStyle(wxPENSTYLE_DOT);
	AltCurvePen.SetColour(wxColour(224, 255, 224));
	AltCurvePenThick.SetColour(wxColour(224, 255, 224));
	
	DataseriesGroupArray[0].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[0].WatchedNMRData = CHECK_ChunkSet;
	DataseriesGroupArray[0].GetNMRPts = NFGNMRData::GetTDDImagPts;
	DataseriesGroupArray[0].GetNMRRPtBB = NFGNMRData::GetTDDImagRPtBB;
	DataseriesGroupArray[0].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[0].GetNMRIndexRange = NFGNMRData::GetTDDIndexRange;

	DataseriesGroupArray[0].NumberedDataseries = true;
	DataseriesGroupArray[0].NumberedCurvePoints = false;

	DataseriesGroupArray[0].HasHeadAndTail = true;

	DataseriesGroupArray[0].SymmetricYRange = true;
	DataseriesGroupArray[0].IncludeYZero = true;
	DataseriesGroupArray[0].NondecreasingX = true;
	DataseriesGroupArray[0].DominantBBox = false;

	DataseriesGroupArray[0].KeyItem.Label = wxString("Imaginary");
	DataseriesGroupArray[0].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[0].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[0].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[0].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[0].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[0].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[0].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[0].KeyItem.PointRadius = 4;
	DataseriesGroupArray[0].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[0].KeyItem.DatasetFlag = 1ul << (ID_TDDImag - DatasetIDMin);

	CurvePen.SetColour(wxColour(0, 0, 255));
	CurvePenThick.SetColour(wxColour(0, 0, 255));
	CurvePenBW.SetStyle(wxPENSTYLE_SOLID);
	AltCurvePen.SetColour(wxColour(224, 224, 255));
	AltCurvePenThick.SetColour(wxColour(224, 224, 255));

	DataseriesGroupArray[2].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[2].WatchedNMRData = CHECK_ChunkSet;
	DataseriesGroupArray[2].GetNMRPts = NFGNMRData::GetTDDAmpPts;
	DataseriesGroupArray[2].GetNMRRPtBB = NFGNMRData::GetTDDAmpRPtBB;
	DataseriesGroupArray[2].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[2].GetNMRIndexRange = NFGNMRData::GetTDDIndexRange;

	DataseriesGroupArray[2].NumberedDataseries = true;
	DataseriesGroupArray[2].NumberedCurvePoints = false;

	DataseriesGroupArray[2].HasHeadAndTail = true;

	DataseriesGroupArray[2].SymmetricYRange = false;
	DataseriesGroupArray[2].IncludeYZero = true;
	DataseriesGroupArray[2].NondecreasingX = true;
	DataseriesGroupArray[2].DominantBBox = true;

	DataseriesGroupArray[2].KeyItem.Label = wxString("Modulus");
	DataseriesGroupArray[2].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[2].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[2].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[2].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[2].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[2].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[2].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[2].KeyItem.PointRadius = 4;
	DataseriesGroupArray[2].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[2].KeyItem.DatasetFlag = 1ul << (ID_TDDModule - DatasetIDMin);

	CurvePen.SetColour(wxColour(128, 128, 0));
	CurvePenThick.SetColour(wxColour(128, 128, 0));
	CurvePenBW.SetStyle(wxPENSTYLE_DOT_DASH);

	DataseriesGroupArray[3].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[3].WatchedNMRData = CHECK_EchoPeaksEnvelope;
	DataseriesGroupArray[3].GetNMRPts = NFGNMRData::GetEchoPeaksEnvelopeAmpPts;
	DataseriesGroupArray[3].GetNMRRPtBB = NFGNMRData::GetEchoPeaksEnvelopeAmpRPtBB;
	DataseriesGroupArray[3].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[3].GetNMRIndexRange = NFGNMRData::GetEchoPeaksEnvelopeIndexRange;

	DataseriesGroupArray[3].NumberedDataseries = true;
	DataseriesGroupArray[3].NumberedCurvePoints = false;

	DataseriesGroupArray[3].HasHeadAndTail = false;

	DataseriesGroupArray[3].SymmetricYRange = false;
	DataseriesGroupArray[3].IncludeYZero = true;
	DataseriesGroupArray[3].NondecreasingX = true;
	DataseriesGroupArray[3].DominantBBox = false;

	DataseriesGroupArray[3].KeyItem.Label = wxString("Echo peaks envelope");
	DataseriesGroupArray[3].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[3].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[3].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[3].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[3].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[3].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[3].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[3].KeyItem.PointRadius = 4;
	DataseriesGroupArray[3].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[3].KeyItem.DatasetFlag = 1ul << (ID_TDDEchoPeaksEnvelope - DatasetIDMin);


	StickyZoomLeft = true;

	ConstrainZoomLeft = true;
	ConstrainZoomLeftAllowOverride = true;
	ConstrainZoomRight = true;
	ConstrainZoomRightAllowOverride = true;
	ConstrainZoomBottomAllowOverride = true;

	CurveSet.PointRadius = 4;
	
	HighlightedCurveSet.PointRadius = 6;
	
	XAxisUnits = wxString("us");

	YAxisUnits = wxString("a.u.");
	YAxisZeroExponent = false;
}

NFGGraphTDD::~NFGGraphTDD()
{
}

wxString NFGGraphTDD::GetGraphLabel()
{
	return wxString::Format("Time domain data plot - step %ld: ", GetSelectedStep()) + GetSelectedStepLabel();
}

NFGRealRect NFGGraphTDD::GetBoundingRealRect()
{
	return NFGGraph::GetBoundingRealRect();
}

NFGCurveSet NFGGraphTDD::GetCurveSet()
{
	if (CurveSetValid)
		return CurveSet;
	
	long Start = 0, End = INT_MAX;
	if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_ChunkSet, ALL_STEPS) == DATA_OK) {
		long FirstChunk = 0, LastChunk = INT_MAX;
		NFGNMRData::CheckProcParam(NMRDataPointer, PROC_PARAM_FirstChunk, PARAM_LONG, &FirstChunk, NULL);
		NFGNMRData::CheckProcParam(NMRDataPointer, PROC_PARAM_LastChunk, PARAM_LONG, &LastChunk, NULL);
		
		if (((size_t) FirstChunk < ChunkNoRange(NMRDataPointer)) && ((size_t) LastChunk < ChunkNoRange(NMRDataPointer)) && (FirstChunk >= 0) && (LastChunk >= 0)) {
			Start = ChunkDataStart(NMRDataPointer, FirstChunk)/2;
			End = ChunkDataStart(NMRDataPointer, LastChunk)/2 + ChunkIndexRange(NMRDataPointer, LastChunk) - 1;
			if (End < Start) 	/// unlikely
				End = Start;
		} else 	/// no chunk to process
			End = Start = 0;
	}
	
	if (DataError || !DoGetCurveSet(CurveSet, SelectedStep, false, Start, End)) {
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

NFGRealRect NFGGraphTDD::GetSelectedStepBoundingRealRect()
{
	return NFGGraph::GetSelectedStepBoundingRealRect();
}

NFGCurveSet NFGGraphTDD::GetHighlightedCurveSet()
{
	/// Actually no highlighted curves
	return HighlightedCurveSet;
}

NFGPointSet NFGGraphTDD::GetHighlightedPointSet()
{
	/// No highlighted points
	return HighlightedPointSet;
}

void NFGGraphTDD::SelectStep(unsigned long index)
{
	DoSelectStep(index, true);
}



NFGGraphChunkAvg::NFGGraphChunkAvg(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style) : NFGGraph(NMRDataPtr, document, style, Flag(CHECK_ChunkAvg), Flag(PROC_PARAM_ChunkStart) | Flag(PROC_PARAM_ChunkEnd))
{
	DisplayedDatasets = 	(1ul << (ID_ChunkAvgReal - DatasetIDMin)) | 
					(1ul << (ID_ChunkAvgImag - DatasetIDMin)) |
					(1ul << (ID_ChunkAvgModule - DatasetIDMin));
	
	DisplayedDatasetsMask = 	(1ul << (ID_ChunkAvgReal - DatasetIDMin)) | 
						(1ul << (ID_ChunkAvgImag - DatasetIDMin)) |
						(1ul << (ID_ChunkAvgModule - DatasetIDMin));
	
	DrawPoints = false;
	ThickLines = false;
	
	DataseriesGroupArray = new NFGDataseriesGroup[3];
	DataseriesGroupCount = 3;
	
	wxWindow *win = (document != NULL)?(document->GetDocumentWindow()):(NULL);

	wxPen CurvePen(wxColour(255, 0, 0), wxWindow::FromDIP(1, win));

	wxPen CurvePenThick(CurvePen);
	CurvePenThick.SetWidth(wxWindow::FromDIP(2, win));
	
	wxPen CurvePenBW = *wxBLACK_PEN;
	CurvePenBW.SetStyle(wxPENSTYLE_LONG_DASH);

	
	wxPen AltCurvePen(CurvePen);
	AltCurvePen.SetColour(wxColour(255, 224, 224));
	
	wxPen AltCurvePenThick(CurvePenThick);
	AltCurvePenThick.SetColour(wxColour(255, 224, 224));
	
	wxPen AltCurvePenBW(CurvePenBW);
	AltCurvePenBW.SetStyle(wxPENSTYLE_TRANSPARENT);

	DataseriesGroupArray[1].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[1].WatchedNMRData = CHECK_ChunkAvg;
	DataseriesGroupArray[1].GetNMRPts = NFGNMRData::GetChunkAvgRealPts;
	DataseriesGroupArray[1].GetNMRRPtBB = NFGNMRData::GetChunkAvgRealRPtBB;
	DataseriesGroupArray[1].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[1].GetNMRIndexRange = NFGNMRData::GetChunkAvgIndexRange;
	
	DataseriesGroupArray[1].NumberedDataseries = true;
	DataseriesGroupArray[1].NumberedCurvePoints = false;

	DataseriesGroupArray[1].HasHeadAndTail = true;
	
	DataseriesGroupArray[1].SymmetricYRange = true;
	DataseriesGroupArray[1].IncludeYZero = true;
	DataseriesGroupArray[1].NondecreasingX = true;
	DataseriesGroupArray[1].DominantBBox = false;

	DataseriesGroupArray[1].KeyItem.Label = wxString("Real");
	DataseriesGroupArray[1].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[1].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[1].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[1].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[1].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[1].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[1].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[1].KeyItem.PointRadius = 4;
	DataseriesGroupArray[1].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[1].KeyItem.DatasetFlag = 1ul << (ID_ChunkAvgReal - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 255, 0));
	CurvePenThick.SetColour(wxColour(0, 255, 0));
	CurvePenBW.SetStyle(wxPENSTYLE_DOT);
	AltCurvePen.SetColour(wxColour(224, 255, 224));
	AltCurvePenThick.SetColour(wxColour(224, 255, 224));
	
	DataseriesGroupArray[0].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[0].WatchedNMRData = CHECK_ChunkAvg;
	DataseriesGroupArray[0].GetNMRRPtBB = NFGNMRData::GetChunkAvgImagRPtBB;
	DataseriesGroupArray[0].GetNMRPts = NFGNMRData::GetChunkAvgImagPts;
	DataseriesGroupArray[0].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[0].GetNMRIndexRange = NFGNMRData::GetChunkAvgIndexRange;
	
	DataseriesGroupArray[0].NumberedDataseries = true;
	DataseriesGroupArray[0].NumberedCurvePoints = false;

	DataseriesGroupArray[0].HasHeadAndTail = true;

	DataseriesGroupArray[0].SymmetricYRange = true;
	DataseriesGroupArray[0].IncludeYZero = true;
	DataseriesGroupArray[0].NondecreasingX = true;
	DataseriesGroupArray[0].DominantBBox = false;

	DataseriesGroupArray[0].KeyItem.Label = wxString("Imaginary");
	DataseriesGroupArray[0].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[0].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[0].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[0].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[0].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[0].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[0].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[0].KeyItem.PointRadius = 4;
	DataseriesGroupArray[0].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[0].KeyItem.DatasetFlag = 1ul << (ID_ChunkAvgImag - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 0, 255));
	CurvePenThick.SetColour(wxColour(0, 0, 255));
	CurvePenBW.SetStyle(wxPENSTYLE_SOLID);
	AltCurvePen.SetColour(wxColour(224, 224, 255));
	AltCurvePenThick.SetColour(wxColour(224, 224, 255));

	DataseriesGroupArray[2].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[2].WatchedNMRData = CHECK_ChunkAvg;
	DataseriesGroupArray[2].GetNMRRPtBB = NFGNMRData::GetChunkAvgAmpRPtBB;
	DataseriesGroupArray[2].GetNMRPts = NFGNMRData::GetChunkAvgAmpPts;
	DataseriesGroupArray[2].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[2].GetNMRIndexRange = NFGNMRData::GetChunkAvgIndexRange;
	
	DataseriesGroupArray[2].NumberedDataseries = true;
	DataseriesGroupArray[2].NumberedCurvePoints = false;

	DataseriesGroupArray[2].HasHeadAndTail = true;
	
	DataseriesGroupArray[2].SymmetricYRange = false;
	DataseriesGroupArray[2].IncludeYZero = true;
	DataseriesGroupArray[2].NondecreasingX = true;
	DataseriesGroupArray[2].DominantBBox = true;

	DataseriesGroupArray[2].KeyItem.Label = wxString("Modulus");
	DataseriesGroupArray[2].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[2].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[2].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[2].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[2].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[2].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[2].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[2].KeyItem.PointRadius = 4;
	DataseriesGroupArray[2].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[2].KeyItem.DatasetFlag = 1ul << (ID_ChunkAvgModule - DatasetIDMin);


	StickyZoomLeft = true;

	ConstrainZoomLeft = true;
	ConstrainZoomLeftAllowOverride = true;
	ConstrainZoomRight = true;
	ConstrainZoomRightAllowOverride = true;
	ConstrainZoomBottomAllowOverride = true;

	CurveSet.PointRadius = 4;
	
	HighlightedCurveSet.PointRadius = 6;
	
	XAxisUnits = wxString("us");
	XAxisZeroExponent = true;
	
	YAxisUnits = wxString("a.u.");
	YAxisZeroExponent = false;
}

NFGGraphChunkAvg::~NFGGraphChunkAvg()
{
}

wxString NFGGraphChunkAvg::GetGraphLabel()
{
	return wxString::Format("Chunk average plot - step %ld: ", GetSelectedStep()) + GetSelectedStepLabel();
}

NFGRealRect NFGGraphChunkAvg::GetBoundingRealRect()
{
	return NFGGraph::GetBoundingRealRect();
}

NFGCurveSet NFGGraphChunkAvg::GetCurveSet()
{
	if (CurveSetValid)
		return CurveSet;

	long ChunkStart = 0, ChunkEnd = INT_MAX;
	NFGNMRData::CheckProcParam(NMRDataPointer, PROC_PARAM_ChunkStart, PARAM_LONG, &ChunkStart, NULL);
	NFGNMRData::CheckProcParam(NMRDataPointer, PROC_PARAM_ChunkEnd, PARAM_LONG, &ChunkEnd, NULL);
	
	if (DataError || !DoGetCurveSet(CurveSet, SelectedStep, false, ChunkStart, ChunkEnd)) {
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

NFGRealRect NFGGraphChunkAvg::GetSelectedStepBoundingRealRect()
{
	return NFGGraph::GetSelectedStepBoundingRealRect();
}

NFGCurveSet NFGGraphChunkAvg::GetHighlightedCurveSet()
{
	/// Actually no highlighted curves
	return HighlightedCurveSet;
}

NFGPointSet NFGGraphChunkAvg::GetHighlightedPointSet()
{
	/// No highlighted points
	return HighlightedPointSet;
}

void NFGGraphChunkAvg::SelectStep(unsigned long index)
{
	DoSelectStep(index, true);
}



NFGGraphFFT::NFGGraphFFT(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style) : NFGGraph(NMRDataPtr, document, style, Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp), Flag(PROC_PARAM_Filter))
{
	DisplayedDatasets = 	(1ul << (ID_FFTReal - DatasetIDMin)) | 
					(1ul << (ID_FFTImag - DatasetIDMin)) |
					(1ul << (ID_FFTModule - DatasetIDMin));
	
	DisplayedDatasetsMask = 	(1ul << (ID_FFTReal - DatasetIDMin)) | 
						(1ul << (ID_FFTImag - DatasetIDMin)) |
						(1ul << (ID_FFTModule - DatasetIDMin));
	
	DrawPoints = false;
	ThickLines = false;
	
	DataseriesGroupArray = new NFGDataseriesGroup[3];
	DataseriesGroupCount = 3;
	
	wxWindow *win = (document != NULL)?(document->GetDocumentWindow()):(NULL);
	
	wxPen CurvePen(wxColour(255, 0, 0), wxWindow::FromDIP(1, win));

	wxPen CurvePenThick(CurvePen);
	CurvePenThick.SetWidth(wxWindow::FromDIP(2, win));
	
	wxPen CurvePenBW = *wxBLACK_PEN;
	CurvePenBW.SetStyle(wxPENSTYLE_LONG_DASH);

	
	wxPen AltCurvePen(CurvePen);
	AltCurvePen.SetColour(wxColour(255, 224, 224));
	
	wxPen AltCurvePenThick(CurvePenThick);
	AltCurvePenThick.SetColour(wxColour(255, 224, 224));
	
	wxPen AltCurvePenBW(CurvePenBW);
	AltCurvePenBW.SetStyle(wxPENSTYLE_TRANSPARENT);


	DataseriesGroupArray[1].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[1].WatchedNMRData = CHECK_DFTPhaseCorr_ReIm;
	DataseriesGroupArray[1].GetNMRPts = NFGNMRData::GetDFTProcNoFilterPhaseCorrRealPts;
	DataseriesGroupArray[1].GetNMRRPtBB = NFGNMRData::GetDFTProcNoFilterPhaseCorrRealRPtBB;
	DataseriesGroupArray[1].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[1].GetNMRIndexRange = NFGNMRData::GetDFTProcNoFilterPhaseCorrIndexRange;
	
	DataseriesGroupArray[1].NumberedDataseries = true;
	DataseriesGroupArray[1].NumberedCurvePoints = false;

	DataseriesGroupArray[1].HasHeadAndTail = true;
	
	DataseriesGroupArray[1].SymmetricYRange = true;
	DataseriesGroupArray[1].IncludeYZero = true;
	DataseriesGroupArray[1].NondecreasingX = true;
	DataseriesGroupArray[1].DominantBBox = false;

	DataseriesGroupArray[1].KeyItem.Label = wxString("Real");
	DataseriesGroupArray[1].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[1].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[1].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[1].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[1].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[1].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[1].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[1].KeyItem.PointRadius = 4;
	DataseriesGroupArray[1].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[1].KeyItem.DatasetFlag = 1ul << (ID_FFTReal - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 255, 0));
	CurvePenThick.SetColour(wxColour(0, 255, 0));
	CurvePenBW.SetStyle(wxPENSTYLE_DOT);
	AltCurvePen.SetColour(wxColour(224, 255, 224));
	AltCurvePenThick.SetColour(wxColour(224, 255, 224));
	
	DataseriesGroupArray[0].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[0].WatchedNMRData = CHECK_DFTPhaseCorr_ReIm;
	DataseriesGroupArray[0].GetNMRPts = NFGNMRData::GetDFTProcNoFilterPhaseCorrImagPts;
	DataseriesGroupArray[0].GetNMRRPtBB = NFGNMRData::GetDFTProcNoFilterPhaseCorrImagRPtBB;
	DataseriesGroupArray[0].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[0].GetNMRIndexRange = NFGNMRData::GetDFTProcNoFilterPhaseCorrIndexRange;
	
	DataseriesGroupArray[0].NumberedDataseries = true;
	DataseriesGroupArray[0].NumberedCurvePoints = false;

	DataseriesGroupArray[0].HasHeadAndTail = true;
	
	DataseriesGroupArray[0].SymmetricYRange = true;
	DataseriesGroupArray[0].IncludeYZero = true;
	DataseriesGroupArray[0].NondecreasingX = true;
	DataseriesGroupArray[0].DominantBBox = false;

	DataseriesGroupArray[0].KeyItem.Label = wxString("Imaginary");
	DataseriesGroupArray[0].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[0].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[0].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[0].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[0].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[0].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[0].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[0].KeyItem.PointRadius = 4;
	DataseriesGroupArray[0].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[0].KeyItem.DatasetFlag = 1ul << (ID_FFTImag - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 0, 255));
	CurvePenThick.SetColour(wxColour(0, 0, 255));
	CurvePenBW.SetStyle(wxPENSTYLE_SOLID);
	AltCurvePen.SetColour(wxColour(224, 224, 255));
	AltCurvePenThick.SetColour(wxColour(224, 224, 255));

	DataseriesGroupArray[2].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[2].WatchedNMRData = CHECK_DFTPhaseCorr_Amp;
	DataseriesGroupArray[2].GetNMRPts = NFGNMRData::GetDFTProcNoFilterPhaseCorrAmpPts;
	DataseriesGroupArray[2].GetNMRRPtBB = NFGNMRData::GetDFTProcNoFilterPhaseCorrAmpRPtBB;
	DataseriesGroupArray[2].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[2].GetNMRIndexRange = NFGNMRData::GetDFTProcNoFilterPhaseCorrIndexRange;
	
	DataseriesGroupArray[2].NumberedDataseries = true;
	DataseriesGroupArray[2].NumberedCurvePoints = false;

	DataseriesGroupArray[2].HasHeadAndTail = true;
	
	DataseriesGroupArray[2].SymmetricYRange = false;
	DataseriesGroupArray[2].IncludeYZero = true;
	DataseriesGroupArray[2].NondecreasingX = true;
	DataseriesGroupArray[2].DominantBBox = true;

	DataseriesGroupArray[2].KeyItem.Label = wxString("Modulus");
	DataseriesGroupArray[2].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[2].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[2].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[2].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[2].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[2].KeyItem.AltPenBW = AltCurvePenBW;
	DataseriesGroupArray[2].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[2].KeyItem.PointRadius = 4;
	DataseriesGroupArray[2].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[2].KeyItem.DatasetFlag = 1ul << (ID_FFTModule - DatasetIDMin);


	ConstrainZoomLeft = true;
	ConstrainZoomLeftAllowOverride = true;
	ConstrainZoomRight = true;
	ConstrainZoomRightAllowOverride = true;
	ConstrainZoomBottomAllowOverride = true;

	CurveSet.PointRadius = 4;
	
	HighlightedCurveSet.PointRadius = 6;
	
	XAxisUnits = wxString("MHz");
	XAxisZeroExponent = true;
	
	YAxisUnits = wxString("a.u.");
	YAxisZeroExponent = false;
}

NFGGraphFFT::~NFGGraphFFT()
{
}

wxString NFGGraphFFT::GetGraphLabel()
{
	return wxString::Format("FFT plot - step %ld: ", GetSelectedStep()) + GetSelectedStepLabel();
}

NFGRealRect NFGGraphFFT::GetBoundingRealRect()
{
	return NFGGraph::GetBoundingRealRect();
}

NFGCurveSet NFGGraphFFT::GetCurveSet()
{
	if (CurveSetValid)
		return CurveSet;
	
	NFGNMRData::CheckProcParam(NMRDataPointer, PROC_PARAM_DFTLength, PARAM_LONG, NULL, NULL);
	NFGNMRData::CheckProcParam(NMRDataPointer, PROC_PARAM_Filter, PARAM_LONG, NULL, NULL);
	long Start = (NMRDataPointer)?(NMRDataPointer->filter):(0);
	long End = (NMRDataPointer)?(NMRDataPointer->DFTLength - NMRDataPointer->filter2 - 1):(0);
	
	if (DataError || !DoGetCurveSet(CurveSet, SelectedStep, false, Start, End)) {
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

NFGRealRect NFGGraphFFT::GetSelectedStepBoundingRealRect()
{
	return NFGGraph::GetSelectedStepBoundingRealRect();
}

NFGCurveSet NFGGraphFFT::GetHighlightedCurveSet()
{
	/// Actually no highlighted curves
	return HighlightedCurveSet;
}

NFGPointSet NFGGraphFFT::GetHighlightedPointSet()
{
	/// No highlighted points
	return HighlightedPointSet;
}

void NFGGraphFFT::SelectStep(unsigned long index)
{
	DoSelectStep(index, true);
}



NFGGraphSpectrum::NFGGraphSpectrum(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style) : NFGGraph(NMRDataPtr, document, style, Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp) | Flag(CHECK_DFTEnvelope) | Flag(CHECK_DFTRealEnvelope), 0)
{
	DisplayedDatasets = (1ul << (ID_SpectrumFFTEnvelope - DatasetIDMin)) | (1ul << (ID_SpectrumParticularFFTModules - DatasetIDMin));
	DisplayedDatasetsMask = (1ul << (ID_SpectrumFFTEnvelope - DatasetIDMin)) | (1ul << (ID_SpectrumParticularFFTModules - DatasetIDMin)) | (1ul << (ID_SpectrumFFTRealEnvelope - DatasetIDMin)) | (1ul << (ID_SpectrumParticularFFTRealParts - DatasetIDMin));
	
	DrawPoints = false;
	ThickLines = false;

	DataseriesGroupArray = new NFGDataseriesGroup[4];
	DataseriesGroupCount = 4;
	
	wxWindow *win = (document != NULL)?(document->GetDocumentWindow()):(NULL);
	
	wxPen EnvelopePen(wxColour(255, 0, 128), wxWindow::FromDIP(1, win));

	wxPen EnvelopePenThick(EnvelopePen);
	EnvelopePenThick.SetWidth(wxWindow::FromDIP(2, win));

	wxPen EnvelopePenBW = *wxBLACK_PEN;
	EnvelopePenBW.SetStyle(wxPENSTYLE_SOLID);

	DataseriesGroupArray[2].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[2].WatchedNMRData = CHECK_DFTEnvelope;
	DataseriesGroupArray[2].GetNMRPts = NFGNMRData::GetDFTEnvelopeAmpPts;
	DataseriesGroupArray[2].GetNMRRPtBB = NFGNMRData::GetDFTEnvelopeAmpRPtBB;
	DataseriesGroupArray[2].GetNMRFlag = NFGNMRData::GetDFTEnvelopeFlag;
	DataseriesGroupArray[2].GetNMRIndexRange = NFGNMRData::GetDFTEnvelopeIndexRange;
	
	DataseriesGroupArray[2].NumberedDataseries = false;
	DataseriesGroupArray[2].NumberedCurvePoints = false;

	DataseriesGroupArray[2].HasHeadAndTail = false;
	
	DataseriesGroupArray[2].SymmetricYRange = false;
	DataseriesGroupArray[2].IncludeYZero = true;
	DataseriesGroupArray[2].NondecreasingX = true;
	DataseriesGroupArray[2].DominantBBox = false;

	DataseriesGroupArray[2].KeyItem.Label = wxString("FFT moduli envelope");
	DataseriesGroupArray[2].KeyItem.Pen = EnvelopePen;
	DataseriesGroupArray[2].KeyItem.PenThick = EnvelopePenThick;
	DataseriesGroupArray[2].KeyItem.PenBW = EnvelopePenBW;
	DataseriesGroupArray[2].KeyItem.AltPen = EnvelopePen;
	DataseriesGroupArray[2].KeyItem.AltPenThick = EnvelopePenThick;
	DataseriesGroupArray[2].KeyItem.AltPenBW = EnvelopePenBW;
	DataseriesGroupArray[2].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[2].KeyItem.PointRadius = 4;
	DataseriesGroupArray[2].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[2].KeyItem.DatasetFlag = 1ul << (ID_SpectrumFFTEnvelope - DatasetIDMin);


	wxPen CurvePen(wxColour(0, 0, 255), wxWindow::FromDIP(1, win));

	wxPen CurvePenThick(CurvePen);
	CurvePenThick.SetWidth(wxWindow::FromDIP(2, win));

	wxPen CurvePenBW = *wxBLACK_PEN;
	CurvePenBW.SetStyle(wxPENSTYLE_DOT);
	
	wxPen AltCurvePen(CurvePen);
	AltCurvePen.SetColour(wxColour(240, 200, 70));
	
	wxPen AltCurvePenThick(CurvePenThick);
	AltCurvePenThick.SetColour(wxColour(240, 200, 70));
	
	DataseriesGroupArray[0].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[0].WatchedNMRData = CHECK_DFTPhaseCorr_Amp;
	DataseriesGroupArray[0].GetNMRPts = NFGNMRData::GetDFTProcNoFilterPhaseCorrAmpPts;
	DataseriesGroupArray[0].GetNMRRPtBB = NFGNMRData::GetDFTProcNoFilterPhaseCorrAmpRPtBB;
	DataseriesGroupArray[0].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[0].GetNMRIndexRange = NFGNMRData::GetDFTProcNoFilterPhaseCorrIndexRange;
	
	DataseriesGroupArray[0].NumberedDataseries = true;
	DataseriesGroupArray[0].NumberedCurvePoints = false;

	DataseriesGroupArray[0].HasHeadAndTail = false;
	
	DataseriesGroupArray[0].SymmetricYRange = false;
	DataseriesGroupArray[0].IncludeYZero = true;
	DataseriesGroupArray[0].NondecreasingX = true;
	DataseriesGroupArray[0].DominantBBox = false;

	DataseriesGroupArray[0].KeyItem.Label = wxString("Particular FFT moduli");
	DataseriesGroupArray[0].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[0].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[0].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[0].KeyItem.AltPen = AltCurvePen;
	DataseriesGroupArray[0].KeyItem.AltPenThick = AltCurvePenThick;
	DataseriesGroupArray[0].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[0].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[0].KeyItem.PointRadius = 4;
	DataseriesGroupArray[0].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[0].KeyItem.DatasetFlag = 1ul << (ID_SpectrumParticularFFTModules - DatasetIDMin);


	wxPen RealEnvelopePen(wxColour(64, 255, 0), wxWindow::FromDIP(1, win));

	wxPen RealEnvelopePenThick(RealEnvelopePen);
	RealEnvelopePenThick.SetWidth(wxWindow::FromDIP(2, win));

	wxPen RealEnvelopePenBW = *wxBLACK_PEN;
	RealEnvelopePenBW.SetStyle(wxPENSTYLE_SOLID);

	DataseriesGroupArray[3].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[3].WatchedNMRData = CHECK_DFTRealEnvelope;
	DataseriesGroupArray[3].GetNMRPts = NFGNMRData::GetDFTRealEnvelopeRealPts;
	DataseriesGroupArray[3].GetNMRRPtBB = NFGNMRData::GetDFTRealEnvelopeRealRPtBB;
	DataseriesGroupArray[3].GetNMRFlag = NFGNMRData::GetDFTRealEnvelopeFlag;
	DataseriesGroupArray[3].GetNMRIndexRange = NFGNMRData::GetDFTRealEnvelopeIndexRange;
	
	DataseriesGroupArray[3].NumberedDataseries = false;
	DataseriesGroupArray[3].NumberedCurvePoints = false;

	DataseriesGroupArray[3].HasHeadAndTail = false;
	
	DataseriesGroupArray[3].SymmetricYRange = true;
	DataseriesGroupArray[3].IncludeYZero = true;
	DataseriesGroupArray[3].NondecreasingX = true;
	DataseriesGroupArray[3].DominantBBox = false;

	DataseriesGroupArray[3].KeyItem.Label = wxString("FFT real parts envelope");
	DataseriesGroupArray[3].KeyItem.Pen = RealEnvelopePen;
	DataseriesGroupArray[3].KeyItem.PenThick = RealEnvelopePenThick;
	DataseriesGroupArray[3].KeyItem.PenBW = RealEnvelopePenBW;
	DataseriesGroupArray[3].KeyItem.AltPen = RealEnvelopePen;
	DataseriesGroupArray[3].KeyItem.AltPenThick = RealEnvelopePenThick;
	DataseriesGroupArray[3].KeyItem.AltPenBW = RealEnvelopePenBW;
	DataseriesGroupArray[3].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[3].KeyItem.PointRadius = 4;
	DataseriesGroupArray[3].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[3].KeyItem.DatasetFlag = 1ul << (ID_SpectrumFFTRealEnvelope - DatasetIDMin);
	
	
	wxPen RealCurvePen(wxColour(255, 0, 0), wxWindow::FromDIP(1, win));

	wxPen RealCurvePenThick(RealCurvePen);
	RealCurvePenThick.SetWidth(wxWindow::FromDIP(2, win));
	
	wxPen RealCurvePenBW = *wxBLACK_PEN;
	RealCurvePenBW.SetStyle(wxPENSTYLE_DOT);
	
	wxPen AltRealCurvePen(RealCurvePen);
	AltRealCurvePen.SetColour(wxColour(70, 240, 200));
	
	wxPen AltRealCurvePenThick(RealCurvePenThick);
	AltRealCurvePenThick.SetColour(wxColour(70, 240, 200));
	
	DataseriesGroupArray[1].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[1].WatchedNMRData = CHECK_DFTPhaseCorr_ReIm;
	DataseriesGroupArray[1].GetNMRPts = NFGNMRData::GetDFTProcNoFilterPhaseCorrRealPts;
	DataseriesGroupArray[1].GetNMRRPtBB = NFGNMRData::GetDFTProcNoFilterPhaseCorrRealRPtBB;
	DataseriesGroupArray[1].GetNMRFlag = NFGNMRData::GetStepFlag;
	DataseriesGroupArray[1].GetNMRIndexRange = NFGNMRData::GetDFTProcNoFilterPhaseCorrIndexRange;
	
	DataseriesGroupArray[1].NumberedDataseries = true;
	DataseriesGroupArray[1].NumberedCurvePoints = false;

	DataseriesGroupArray[1].HasHeadAndTail = false;
	
	DataseriesGroupArray[1].SymmetricYRange = true;
	DataseriesGroupArray[1].IncludeYZero = true;
	DataseriesGroupArray[1].NondecreasingX = true;
	DataseriesGroupArray[1].DominantBBox = false;

	DataseriesGroupArray[1].KeyItem.Label = wxString("Particular FFT real parts");
	DataseriesGroupArray[1].KeyItem.Pen = RealCurvePen;
	DataseriesGroupArray[1].KeyItem.PenThick = RealCurvePenThick;
	DataseriesGroupArray[1].KeyItem.PenBW = RealCurvePenBW;
	DataseriesGroupArray[1].KeyItem.AltPen = AltRealCurvePen;
	DataseriesGroupArray[1].KeyItem.AltPenThick = AltRealCurvePenThick;
	DataseriesGroupArray[1].KeyItem.AltPenBW = RealCurvePenBW;
	DataseriesGroupArray[1].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[1].KeyItem.PointRadius = 4;
	DataseriesGroupArray[1].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[1].KeyItem.DatasetFlag = 1ul << (ID_SpectrumParticularFFTRealParts - DatasetIDMin);

	
	StickyZoomBottom = true;

	ConstrainZoomLeft = true;
	ConstrainZoomLeftAllowOverride = true;
	ConstrainZoomRight = true;
	ConstrainZoomRightAllowOverride = true;
	ConstrainZoomBottomAllowOverride = true;

	CurveSet.PointRadius = 4;
	
	HighlightedCurveSet.PointRadius = 6;
	
	XAxisUnits = wxString("MHz");
	XAxisZeroExponent = true;
	
	YAxisUnits = wxString("a.u.");
	YAxisZeroExponent = false;
}

NFGGraphSpectrum::~NFGGraphSpectrum()
{
}

wxString NFGGraphSpectrum::GetGraphLabel()
{
	return wxString("Spectrum plot");
}

NFGRealRect NFGGraphSpectrum::GetBoundingRealRect()
{
	return NFGGraph::GetBoundingRealRect();
}

NFGCurveSet NFGGraphSpectrum::GetCurveSet()
{
	return NFGGraph::GetCurveSet();
}

NFGRealRect NFGGraphSpectrum::GetSelectedStepBoundingRealRect()
{
	return GetBoundingRealRect();
}

NFGCurveSet NFGGraphSpectrum::GetHighlightedCurveSet()
{
	return NFGGraph::GetHighlightedCurveSet();
}

NFGPointSet NFGGraphSpectrum::GetHighlightedPointSet()
{
	/// No highlighted points
	return HighlightedPointSet;
}

void NFGGraphSpectrum::SelectStep(unsigned long index)
{
	NFGGraph::SelectStep(index);
}




NFGGraphEvaluation::NFGGraphEvaluation(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style) : NFGGraph(NMRDataPtr, document, style, Flag(CHECK_Evaluation_ChunkAvgAmp) | Flag(CHECK_Evaluation_DFTPhaseCorrReal) | Flag(CHECK_Evaluation_DFTPhaseCorrAmp) | Flag(CHECK_DFTPhaseCorr_ReIm) | Flag(CHECK_DFTPhaseCorr_Amp), 0)
{
	DisplayedDatasets = 	(1ul << (ID_EvaluationFFTMax - DatasetIDMin)) | 
					(1ul << (ID_EvaluationFFT0 - DatasetIDMin)) |
					(1ul << (ID_EvaluationChunkAvgModMax - DatasetIDMin));

	DisplayedDatasetsMask = 	(1ul << (ID_EvaluationFFTMax - DatasetIDMin)) | 
						(1ul << (ID_EvaluationFFT0 - DatasetIDMin)) |
						(1ul << (ID_EvaluationFFTMean - DatasetIDMin)) |
						(1ul << (ID_EvaluationFFTRealMax - DatasetIDMin)) | 
						(1ul << (ID_EvaluationFFTReal0 - DatasetIDMin)) |
						(1ul << (ID_EvaluationFFTRealMean - DatasetIDMin)) |
						(1ul << (ID_EvaluationChunkAvgModInt - DatasetIDMin)) |
						(1ul << (ID_EvaluationChunkAvgModMax - DatasetIDMin));
	
	DrawPoints = true;
	ThickLines = false;
	
	DataseriesGroupArray = new NFGDataseriesGroup[8];
	DataseriesGroupCount = 8;
	
	wxWindow *win = (document != NULL)?(document->GetDocumentWindow()):(NULL);
	
	wxPen CurvePen(wxColour(0, 128, 64), wxWindow::FromDIP(1, win));
	
	wxPen CurvePenThick(CurvePen);
	CurvePenThick.SetWidth(wxWindow::FromDIP(2, win));
	
	wxPen CurvePenBW = *wxBLACK_PEN;
	CurvePenBW.SetStyle(wxPENSTYLE_SHORT_DASH);

	DataseriesGroupArray[0].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[0].WatchedNMRData = CHECK_Evaluation_DFTPhaseCorrAmp;
	DataseriesGroupArray[0].GetNMRPts = NFGNMRData::GetDFTMaxPhaseCorrAmpPts;
	DataseriesGroupArray[0].GetNMRRPtBB = NFGNMRData::GetDFTMaxPhaseCorrAmpRPtBB;
	DataseriesGroupArray[0].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[0].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[0].NumberedDataseries = false;
	DataseriesGroupArray[0].NumberedCurvePoints = true;

	DataseriesGroupArray[0].HasHeadAndTail = false;
	
	DataseriesGroupArray[0].SymmetricYRange = false;
	DataseriesGroupArray[0].IncludeYZero = true;
	DataseriesGroupArray[0].NondecreasingX = false;
	DataseriesGroupArray[0].DominantBBox = false;

	DataseriesGroupArray[0].KeyItem.Label = wxString("FFT modulus maximum");
	DataseriesGroupArray[0].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[0].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[0].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[0].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[0].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[0].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[0].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[0].KeyItem.PointRadius = 4;
	DataseriesGroupArray[0].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[0].KeyItem.DatasetFlag = 1ul << (ID_EvaluationFFTMax - DatasetIDMin);


	CurvePen.SetColour(wxColour(128, 0, 0));
	CurvePenThick.SetColour(wxColour(128, 0, 0));
	CurvePenBW.SetStyle(wxPENSTYLE_SOLID);

	DataseriesGroupArray[1].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[1].WatchedNMRData = CHECK_DFTPhaseCorr_Amp;
	DataseriesGroupArray[1].GetNMRPts = NFGNMRData::GetDFTPhaseCorrAmpAtZeroPts;
	DataseriesGroupArray[1].GetNMRRPtBB = NFGNMRData::GetDFTPhaseCorrAmpAtZeroRPtBB;
	DataseriesGroupArray[1].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[1].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[1].NumberedDataseries = false;
	DataseriesGroupArray[1].NumberedCurvePoints = true;

	DataseriesGroupArray[1].HasHeadAndTail = false;
	
	DataseriesGroupArray[1].SymmetricYRange = false;
	DataseriesGroupArray[1].IncludeYZero = true;
	DataseriesGroupArray[1].NondecreasingX = false;
	DataseriesGroupArray[1].DominantBBox = false;
	
	DataseriesGroupArray[1].KeyItem.Label = wxString("FFT modulus at excitation frequency");
	DataseriesGroupArray[1].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[1].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[1].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[1].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[1].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[1].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[1].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[1].KeyItem.PointRadius = 4;
	DataseriesGroupArray[1].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[1].KeyItem.DatasetFlag = 1ul << (ID_EvaluationFFT0 - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 128, 192));
	CurvePenThick.SetColour(wxColour(0, 128, 192));
	CurvePenBW.SetStyle(wxPENSTYLE_LONG_DASH);

	DataseriesGroupArray[2].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[2].WatchedNMRData = CHECK_Evaluation_DFTPhaseCorrAmp;
	DataseriesGroupArray[2].GetNMRPts = NFGNMRData::GetDFTMeanPhaseCorrAmpPts;
	DataseriesGroupArray[2].GetNMRRPtBB = NFGNMRData::GetDFTMeanPhaseCorrAmpRPtBB;
	DataseriesGroupArray[2].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[2].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[2].NumberedDataseries = false;
	DataseriesGroupArray[2].NumberedCurvePoints = true;

	DataseriesGroupArray[2].HasHeadAndTail = false;
	
	DataseriesGroupArray[2].SymmetricYRange = false;
	DataseriesGroupArray[2].IncludeYZero = true;
	DataseriesGroupArray[2].NondecreasingX = false;
	DataseriesGroupArray[2].DominantBBox = false;

	DataseriesGroupArray[2].KeyItem.Label = wxString("FFT modulus mean");
	DataseriesGroupArray[2].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[2].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[2].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[2].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[2].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[2].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[2].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[2].KeyItem.PointRadius = 4;
	DataseriesGroupArray[2].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[2].KeyItem.DatasetFlag = 1ul << (ID_EvaluationFFTMean - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 255, 0));
	CurvePenThick.SetColour(wxColour(0, 255, 0));
	CurvePenBW.SetStyle(wxPENSTYLE_SHORT_DASH);

	DataseriesGroupArray[3].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[3].WatchedNMRData = CHECK_Evaluation_DFTPhaseCorrReal;
	DataseriesGroupArray[3].GetNMRPts = NFGNMRData::GetDFTMaxPhaseCorrRealPts;
	DataseriesGroupArray[3].GetNMRRPtBB = NFGNMRData::GetDFTMaxPhaseCorrRealRPtBB;
	DataseriesGroupArray[3].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[3].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[3].NumberedDataseries = false;
	DataseriesGroupArray[3].NumberedCurvePoints = true;

	DataseriesGroupArray[3].HasHeadAndTail = false;
	
	DataseriesGroupArray[3].SymmetricYRange = true;
	DataseriesGroupArray[3].IncludeYZero = true;
	DataseriesGroupArray[3].NondecreasingX = false;
	DataseriesGroupArray[3].DominantBBox = false;

	DataseriesGroupArray[3].KeyItem.Label = wxString("FFT real maximum/minimum");
	DataseriesGroupArray[3].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[3].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[3].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[3].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[3].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[3].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[3].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[3].KeyItem.PointRadius = 4;
	DataseriesGroupArray[3].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[3].KeyItem.DatasetFlag = 1ul << (ID_EvaluationFFTRealMax - DatasetIDMin);


	CurvePen.SetColour(wxColour(255, 0, 0));
	CurvePenThick.SetColour(wxColour(255, 0, 0));
	CurvePenBW.SetStyle(wxPENSTYLE_SOLID);
	
	DataseriesGroupArray[4].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[4].WatchedNMRData = CHECK_DFTPhaseCorr_ReIm;
	DataseriesGroupArray[4].GetNMRPts = NFGNMRData::GetDFTPhaseCorrRealAtZeroPts;
	DataseriesGroupArray[4].GetNMRRPtBB = NFGNMRData::GetDFTPhaseCorrRealAtZeroRPtBB;
	DataseriesGroupArray[4].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[4].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[4].NumberedDataseries = false;
	DataseriesGroupArray[4].NumberedCurvePoints = true;

	DataseriesGroupArray[4].HasHeadAndTail = false;
	
	DataseriesGroupArray[4].SymmetricYRange = true;
	DataseriesGroupArray[4].IncludeYZero = true;
	DataseriesGroupArray[4].NondecreasingX = false;
	DataseriesGroupArray[4].DominantBBox = false;

	DataseriesGroupArray[4].KeyItem.Label = wxString("FFT real at excitation frequency");
	DataseriesGroupArray[4].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[4].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[4].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[4].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[4].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[4].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[4].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[4].KeyItem.PointRadius = 4;
	DataseriesGroupArray[4].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[4].KeyItem.DatasetFlag = 1ul << (ID_EvaluationFFTReal0 - DatasetIDMin);


	CurvePen.SetColour(wxColour(0, 0, 255));
	CurvePenThick.SetColour(wxColour(0, 0, 255));
	CurvePenBW.SetStyle(wxPENSTYLE_LONG_DASH);

	DataseriesGroupArray[5].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[5].WatchedNMRData = CHECK_Evaluation_DFTPhaseCorrReal;
	DataseriesGroupArray[5].GetNMRPts = NFGNMRData::GetDFTMeanPhaseCorrRealPts;
	DataseriesGroupArray[5].GetNMRRPtBB = NFGNMRData::GetDFTMeanPhaseCorrRealRPtBB;
	DataseriesGroupArray[5].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[5].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[5].NumberedDataseries = false;
	DataseriesGroupArray[5].NumberedCurvePoints = true;

	DataseriesGroupArray[5].HasHeadAndTail = false;
	
	DataseriesGroupArray[5].SymmetricYRange = true;
	DataseriesGroupArray[5].IncludeYZero = true;
	DataseriesGroupArray[5].NondecreasingX = false;
	DataseriesGroupArray[5].DominantBBox = false;

	DataseriesGroupArray[5].KeyItem.Label = wxString("FFT real mean");
	DataseriesGroupArray[5].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[5].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[5].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[5].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[5].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[5].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[5].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[5].KeyItem.PointRadius = 4;
	DataseriesGroupArray[5].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[5].KeyItem.DatasetFlag = 1ul << (ID_EvaluationFFTRealMean - DatasetIDMin);


	CurvePen.SetColour(wxColour(128, 0, 255));
	CurvePenThick.SetColour(wxColour(128, 0, 255));
	CurvePenBW.SetStyle(wxPENSTYLE_DOT);

	DataseriesGroupArray[6].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[6].WatchedNMRData = CHECK_Evaluation_ChunkAvgAmp;
	DataseriesGroupArray[6].GetNMRPts = NFGNMRData::GetChunkAvgMaxAmpPts;
	DataseriesGroupArray[6].GetNMRRPtBB = NFGNMRData::GetChunkAvgMaxAmpRPtBB;
	DataseriesGroupArray[6].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[6].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[6].NumberedDataseries = false;
	DataseriesGroupArray[6].NumberedCurvePoints = true;

	DataseriesGroupArray[6].HasHeadAndTail = false;
	
	DataseriesGroupArray[6].SymmetricYRange = false;
	DataseriesGroupArray[6].IncludeYZero = true;
	DataseriesGroupArray[6].NondecreasingX = false;
	DataseriesGroupArray[6].DominantBBox = false;

	DataseriesGroupArray[6].KeyItem.Label = wxString("Chunk average modulus maximum");
	DataseriesGroupArray[6].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[6].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[6].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[6].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[6].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[6].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[6].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[6].KeyItem.PointRadius = 4;
	DataseriesGroupArray[6].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[6].KeyItem.DatasetFlag = 1ul << (ID_EvaluationChunkAvgModMax - DatasetIDMin);


	CurvePen.SetColour(wxColour(255, 128, 192));
	CurvePenThick.SetColour(wxColour(255, 128, 192));
	CurvePenBW.SetStyle(wxPENSTYLE_DOT_DASH);
	
	DataseriesGroupArray[7].NMRDataPointer = NMRDataPointer;
	DataseriesGroupArray[7].WatchedNMRData = CHECK_Evaluation_ChunkAvgAmp;
	DataseriesGroupArray[7].GetNMRPts = NFGNMRData::GetChunkAvgIntAmpPts;
	DataseriesGroupArray[7].GetNMRRPtBB = NFGNMRData::GetChunkAvgIntAmpRPtBB;
	DataseriesGroupArray[7].GetNMRFlag = NFGNMRData::GetEvaluationFlag;
	DataseriesGroupArray[7].GetNMRIndexRange = NFGNMRData::GetEvaluationIndexRange;
	
	DataseriesGroupArray[7].NumberedDataseries = false;
	DataseriesGroupArray[7].NumberedCurvePoints = true;

	DataseriesGroupArray[7].HasHeadAndTail = false;
	
	DataseriesGroupArray[7].SymmetricYRange = false;
	DataseriesGroupArray[7].IncludeYZero = true;
	DataseriesGroupArray[7].NondecreasingX = false;
	DataseriesGroupArray[7].DominantBBox = false;

	DataseriesGroupArray[7].KeyItem.Label = wxString("Chunk average modulus integral");
	DataseriesGroupArray[7].KeyItem.Pen = CurvePen;
	DataseriesGroupArray[7].KeyItem.PenThick = CurvePenThick;
	DataseriesGroupArray[7].KeyItem.PenBW = CurvePenBW;
	DataseriesGroupArray[7].KeyItem.AltPen = CurvePen;
	DataseriesGroupArray[7].KeyItem.AltPenThick = CurvePenThick;
	DataseriesGroupArray[7].KeyItem.AltPenBW = CurvePenBW;
	DataseriesGroupArray[7].KeyItem.DrawPoints = DrawPoints;
	DataseriesGroupArray[7].KeyItem.PointRadius = 4;
	DataseriesGroupArray[7].KeyItem.HighlightedPointRadius = 6;
	DataseriesGroupArray[7].KeyItem.DatasetFlag = 1ul << (ID_EvaluationChunkAvgModInt - DatasetIDMin);

	
	StickyZoomLeft = true;	/// should be set depending on experiment type
	StickyZoomBottom = true;

	ConstrainZoomLeft = true;
	ConstrainZoomLeftAllowOverride = true;
	ConstrainZoomRight = true;
	ConstrainZoomRightAllowOverride = true;
	ConstrainZoomBottomAllowOverride = true;

	CurveSet.DrawPoints = true;
	CurveSet.PointRadius = 4;
	
	HighlightedCurveSet.DrawPoints = true;
	HighlightedCurveSet.PointRadius = 6;

	HighlightedPointSet.PointRadius = 6;

	XAxisUnitsValid = false;
	XAxisUnits = wxEmptyString;
	XAxisZeroExponent = true;
	
	YAxisUnits = wxString("a.u.");
	YAxisZeroExponent = false;
}

NFGGraphEvaluation::~NFGGraphEvaluation()
{
}

wxString NFGGraphEvaluation::GetGraphLabel()
{
	return wxString("Evaluation plot");
}

NFGRealRect NFGGraphEvaluation::GetBoundingRealRect()
{
	return NFGGraph::GetBoundingRealRect();
}

NFGCurveSet NFGGraphEvaluation::GetCurveSet()
{
	return NFGGraph::GetCurveSet();
}

NFGRealRect NFGGraphEvaluation::GetSelectedStepBoundingRealRect()
{
	return GetBoundingRealRect();
}

NFGCurveSet NFGGraphEvaluation::GetHighlightedCurveSet()
{
	/// No highlighted curves
	return HighlightedCurveSet;
}

NFGPointSet NFGGraphEvaluation::GetHighlightedPointSet()
{
	return NFGGraph::GetHighlightedPointSet();
}

void NFGGraphEvaluation::SelectStep(unsigned long index)
{
	NFGGraph::SelectStep(index);
}

void NFGGraphEvaluation::MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo)
{
	/// The x axis units may change when reloading the NMR data
	if (ClearedFlags & Flag(CHECK_AcquParams)) 
		XAxisUnitsValid = false;
	
	NFGGraph::MarkNMRDataOldCallback(ClearedFlags, StepNo);
}

wxString NFGGraphEvaluation::GetXAxisUnits()
{
	/// The x axis units may change when reloading the NMR data
	if (!XAxisUnitsValid) {
		if (NFGNMRData::CheckNMRData(NMRDataPointer, CHECK_AcquParams, ALL_STEPS) == DATA_OK) {
			if (NMRDataPointer->AcquInfo.AssocValueUnits != NULL) 
				XAxisUnits = wxString(NMRDataPointer->AcquInfo.AssocValueUnits);
			else
				XAxisUnits = wxEmptyString;
				
			XAxisUnitsValid = true;
		} else 
			XAxisUnits = wxEmptyString;
	}
	
	return XAxisUnits;
}
