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

#ifndef __infopanel__
#define __infopanel__

#include "wx_pch.h"

#include <wx/datetime.h>

#include "cd.h"
#include "infopanel_cd.h"

#include "nmrdata_cd.h"


class NFGInfoPanel : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* PathLabelST;
		wxStaticText* PathST;
		wxStaticText* AcqusTitleLabelST;
		wxStaticText* AcqusTitleST;
		wxStaticText* AcqusDateLabelST;
		wxStaticText* AcqusDateST;
		wxStaticText* ExpTypeLabelST;
		wxStaticText* ExpTypeST;
		
		wxStaticText* MinLabelST;
		wxStaticText* MinST;
		wxStaticText* ExpCountLabelST;
		wxStaticText* StepCountST;
		
		wxStaticText* CoefficientLabelST;
		wxStaticText* CoefST;
		
		wxStaticText* TunedLabelST;
		wxStaticText* TunedST;
		wxStaticText* MaxLabelST;
		wxStaticText* MaxST;
		wxStaticText* StepLabelST;
		wxStaticText* StepST;
		wxStaticText* VariableNameLabelST;
		wxStaticText* VariableNameST;
		
		wxStaticText* PulseProgramLabelST;
		wxStaticText* PulseProgramST;
		
		wxStaticText* SampleRateLabelST;
		wxStaticText* SampleRateST;
		wxStaticText* DWLabelST;
		wxStaticText* DWST;
		wxStaticText* FilterWidthLabelST;
		wxStaticText* FilterWidthST;
		wxStaticText* ReceiverGainLabelST;
		wxStaticText* ReceiverGainST;
		wxStaticText* FrequencyLabelST;
		wxStaticText* FrequencyST;
		
		wxStaticText* NumberOfScansLabelST;
		wxStaticText* NumberOfScansST;
		wxStaticText* D1LabelST;
		wxStaticText* D1ST;
		wxStaticText* D3LabelST;
		wxStaticText* D3ST;
		wxStaticText* D6LabelST;
		wxStaticText* D6ST;
		wxStaticText* DELabelST;
		wxStaticText* DEST;
		wxStaticText* NumberOfChunksLabelST;
		wxStaticText* NumberOfChunksST;
		
		wxStaticText* P1LabelST;
		wxStaticText* P1ST;
		wxStaticText* P2LabelST;
		wxStaticText* P2ST;
		wxStaticText* P3LabelST;
		wxStaticText* P3ST;
		wxStaticText* P4LabelST;
		wxStaticText* P4ST;
		wxStaticText* PL1LabelST;
		wxStaticText* PL1ST;
		wxStaticText* PL2LabelST;
		wxStaticText* PL2ST;
		wxStaticText* PL21LabelST;
		wxStaticText* PL21ST;
		wxStaticText* PL22LabelST;
		wxStaticText* PL22ST;
	
		wxFlexGridSizer* fgSizer0;
		wxFlexGridSizer* fgSizer21;
		wxFlexGridSizer* fgSizer22;

	public:
		NFGInfoPanel(AcquParams* AcquInfo, wxString path, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 466,507 ), long style = wxTAB_TRAVERSAL);
		~NFGInfoPanel();
	
		void LoadAcquInfo(AcquParams* AcquInfo, wxString path);
};

#endif
