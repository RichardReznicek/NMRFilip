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

#ifndef __plotimp__
#define __plotimp__

#include "wx_pch.h"

#include "cd.h"
#include "plotimp_cd.h"

#include "doc_cd.h"
#include "plotgen.h"


#define GraphTypeTDD	2

class NFGGraphTDD : public NFGGraph
{
	public:
		NFGGraphTDD(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style);
		~NFGGraphTDD();

		wxString GetGraphLabel();

		NFGRealRect GetBoundingRealRect();
		NFGCurveSet GetCurveSet();

		NFGRealRect GetSelectedStepBoundingRealRect();
		NFGCurveSet GetHighlightedCurveSet();
		NFGPointSet GetHighlightedPointSet();
	
		void SelectStep(unsigned long index);
};


#define GraphTypeChunkAvg	3

class NFGGraphChunkAvg : public NFGGraph
{
	public:
		NFGGraphChunkAvg(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style);
		~NFGGraphChunkAvg();

		wxString GetGraphLabel();
	
		NFGRealRect GetBoundingRealRect();
		NFGCurveSet GetCurveSet();

		NFGRealRect GetSelectedStepBoundingRealRect();
		NFGCurveSet GetHighlightedCurveSet();
		NFGPointSet GetHighlightedPointSet();
	
		void SelectStep(unsigned long index);
};


#define GraphTypeFFT	4

class NFGGraphFFT : public NFGGraph
{
	public:
		NFGGraphFFT(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style);
		~NFGGraphFFT();

		wxString GetGraphLabel();
	
		NFGRealRect GetBoundingRealRect();
		NFGCurveSet GetCurveSet();

		NFGRealRect GetSelectedStepBoundingRealRect();
		NFGCurveSet GetHighlightedCurveSet();
		NFGPointSet GetHighlightedPointSet();
	
		void SelectStep(unsigned long index);
};


#define GraphTypeSpectrum	5

class NFGGraphSpectrum : public NFGGraph
{
	public:
		NFGGraphSpectrum(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style);
		~NFGGraphSpectrum();

		wxString GetGraphLabel();
	
		NFGRealRect GetBoundingRealRect();
		NFGCurveSet GetCurveSet();

		NFGRealRect GetSelectedStepBoundingRealRect();
		NFGCurveSet GetHighlightedCurveSet();
		NFGPointSet GetHighlightedPointSet();
	
		void SelectStep(unsigned long index);
};


#define GraphTypeEvaluation	6

class NFGGraphEvaluation : public NFGGraph
{
	private:
		bool XAxisUnitsValid;
	
	protected:
		wxString GetXAxisUnits();

	public:
		NFGGraphEvaluation(NMRData* NMRDataPtr, NFGSerDocument* document, unsigned char style);
		~NFGGraphEvaluation();

		wxString GetGraphLabel();

		NFGRealRect GetBoundingRealRect();
		NFGCurveSet GetCurveSet();

		NFGRealRect GetSelectedStepBoundingRealRect();
		NFGCurveSet GetHighlightedCurveSet();
		NFGPointSet GetHighlightedPointSet();
	
		void SelectStep(unsigned long index);
	
		void MarkNMRDataOldCallback(unsigned long ClearedFlags, long StepNo);
};


#endif
