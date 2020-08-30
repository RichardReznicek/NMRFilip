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

#include "infopanel.h"

#include "nmrdata.h"


NFGInfoPanel::NFGInfoPanel(AcquParams* AcquInfo, wxString path, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	fgSizer0 = new wxFlexGridSizer(4, 1, FromDIP(5), 0);
	fgSizer0->SetFlexibleDirection(wxBOTH);
	fgSizer0->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "General information"), wxVERTICAL);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(3, 2, FromDIP(5), FromDIP(8));
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PathLabelST = new wxStaticText(this, wxID_ANY, "Path");
	fgSizer1->Add(PathLabelST);
	
	PathST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer1->Add(PathST);
	
	
	AcqusTitleLabelST = new wxStaticText(this, wxID_ANY, "Acqus title");
	fgSizer1->Add(AcqusTitleLabelST);
	
	AcqusTitleST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer1->Add(AcqusTitleST);
	
	
	AcqusDateLabelST = new wxStaticText(this, wxID_ANY, "Acqus date");
	fgSizer1->Add(AcqusDateLabelST);
	
	AcqusDateST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer1->Add(AcqusDateST);
	
	sbSizer1->Add(fgSizer1, 1, wxEXPAND|wxALL, FromDIP(5));
	
	fgSizer0->Add(sbSizer1, 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, FromDIP(5));
	
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Experiment details"), wxVERTICAL);
	
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer(1, 2, 0, FromDIP(10));
	
	fgSizer21 = new wxFlexGridSizer(4, 2, FromDIP(5), FromDIP(8));
	fgSizer21->SetFlexibleDirection(wxBOTH);
	fgSizer21->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	ExpTypeLabelST = new wxStaticText(this, wxID_ANY, "Experiment type");
	fgSizer21->Add(ExpTypeLabelST);
	
	ExpTypeST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer21->Add(ExpTypeST);
	
	
	MinLabelST = new wxStaticText(this, wxID_ANY, "Minimum");
	fgSizer21->Add(MinLabelST);
	
	MinST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer21->Add(MinST);
	
	
	ExpCountLabelST = new wxStaticText(this, wxID_ANY, "Number of steps");
	fgSizer21->Add(ExpCountLabelST);
	
	StepCountST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer21->Add(StepCountST);
	
	
	CoefficientLabelST = new wxStaticText(this, wxID_ANY, "Coefficient");
	fgSizer21->Add(CoefficientLabelST);
	
	CoefST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer21->Add(CoefST);
	
	gSizer2->Add(fgSizer21, 1, wxEXPAND|wxALL, FromDIP(5));
	

	fgSizer22 = new wxFlexGridSizer(4, 2, FromDIP(5), FromDIP(8));
	fgSizer22->SetFlexibleDirection(wxBOTH);
	fgSizer22->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	VariableNameLabelST = new wxStaticText(this, wxID_ANY, "Variable");
	fgSizer22->Add(VariableNameLabelST);
	
	VariableNameST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer22->Add(VariableNameST);
	
	
	MaxLabelST = new wxStaticText(this, wxID_ANY, "Maximum");
	fgSizer22->Add(MaxLabelST);
	
	MaxST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer22->Add(MaxST);
	
	
	StepLabelST = new wxStaticText(this, wxID_ANY, "Step");
	fgSizer22->Add(StepLabelST);
	
	StepST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer22->Add(StepST);
	
	
	TunedLabelST = new wxStaticText(this, wxID_ANY, "Tuned every");
	fgSizer22->Add(TunedLabelST);
	
	TunedST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer22->Add(TunedST);
	
	gSizer2->Add(fgSizer22, 1, wxEXPAND|wxALL, FromDIP(5));
	
	sbSizer2->Add(gSizer2, 1, wxEXPAND, 0);
	
	fgSizer0->Add(sbSizer2, 1, wxEXPAND|wxLEFT|wxRIGHT, FromDIP(5));
	
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Pulse sequence details"), wxVERTICAL);
	
	wxGridSizer* gSizer3;
	gSizer3 = new wxGridSizer(1, 2, 0, FromDIP(10));
	
	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer(6, 2, FromDIP(5), FromDIP(8));
	fgSizer31->SetFlexibleDirection(wxBOTH);
	fgSizer31->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PulseProgramLabelST = new wxStaticText(this, wxID_ANY, "Pulse program");
	fgSizer31->Add(PulseProgramLabelST);
	
	PulseProgramST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer31->Add(PulseProgramST);
	
	
	SampleRateLabelST = new wxStaticText(this, wxID_ANY, "Sample rate");
	fgSizer31->Add(SampleRateLabelST);
	
	SampleRateST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer31->Add(SampleRateST);
	
	
	DWLabelST = new wxStaticText(this, wxID_ANY, "Dwell time DW");
	fgSizer31->Add(DWLabelST);
	
	DWST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer31->Add(DWST);
	
	
	FilterWidthLabelST = new wxStaticText(this, wxID_ANY, "Filter width");
	fgSizer31->Add(FilterWidthLabelST);
	
	FilterWidthST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer31->Add(FilterWidthST);
	
	
	ReceiverGainLabelST = new wxStaticText(this, wxID_ANY, "Receiver gain");
	fgSizer31->Add(ReceiverGainLabelST);
	
	ReceiverGainST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer31->Add(ReceiverGainST);
	
	
	FrequencyLabelST = new wxStaticText(this, wxID_ANY, "Frequency");
	fgSizer31->Add(FrequencyLabelST);
	
	FrequencyST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer31->Add(FrequencyST);
	
	gSizer3->Add(fgSizer31, 1, wxEXPAND|wxALL, FromDIP(5));
	
	
	wxFlexGridSizer* fgSizer32;
	fgSizer32 = new wxFlexGridSizer(6, 2, FromDIP(5), FromDIP(8));
	fgSizer32->SetFlexibleDirection(wxBOTH);
	fgSizer32->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	

	NumberOfScansLabelST = new wxStaticText(this, wxID_ANY, "Number of scans");
	fgSizer32->Add(NumberOfScansLabelST);
	
	NumberOfScansST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer32->Add(NumberOfScansST);


	D1LabelST = new wxStaticText(this, wxID_ANY, "Trigger delay D1");
	fgSizer32->Add(D1LabelST);
	
	D1ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer32->Add(D1ST);
	
	
	D3LabelST = new wxStaticText(this, wxID_ANY, "Ringdown delay D3");
	fgSizer32->Add(D3LabelST);
	
	D3ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer32->Add(D3ST);
	
	
	D6LabelST = new wxStaticText(this, wxID_ANY, "Delay D6 (1/2 of echo)");
	fgSizer32->Add(D6LabelST);
	
	D6ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer32->Add(D6ST);
	
	
	DELabelST = new wxStaticText(this, wxID_ANY, "Pre-scan delay DE");
	fgSizer32->Add(DELabelST);
	
	DEST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer32->Add(DEST);
	
	
	NumberOfChunksLabelST = new wxStaticText(this, wxID_ANY, "Number of chunks");
	fgSizer32->Add(NumberOfChunksLabelST);
	
	NumberOfChunksST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer32->Add(NumberOfChunksST);
	
	
	gSizer3->Add(fgSizer32, 1, wxEXPAND|wxALL, FromDIP(5));
	
	sbSizer3->Add(gSizer3, 1, wxEXPAND, 0);
	
	fgSizer0->Add(sbSizer3, 1, wxEXPAND|wxLEFT|wxRIGHT, FromDIP(5));
	
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, "Pulse parameters"), wxVERTICAL);
	
	wxGridSizer* gSizer4;
	gSizer4 = new wxGridSizer(1, 2, 0, FromDIP(10));
	
	wxFlexGridSizer* fgSizer41;
	fgSizer41 = new wxFlexGridSizer(4, 2, FromDIP(5), FromDIP(8));
	fgSizer41->SetFlexibleDirection(wxBOTH);
	fgSizer41->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	P1LabelST = new wxStaticText(this, wxID_ANY, "Pulse length P 1 (echo 90)");
	fgSizer41->Add(P1LabelST);
	
	P1ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer41->Add(P1ST);
	

	P2LabelST = new wxStaticText(this, wxID_ANY, "Pulse length P 2 (echo 180)");
	fgSizer41->Add(P2LabelST);
	
	P2ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer41->Add(P2ST);
	
	
	P3LabelST = new wxStaticText(this, wxID_ANY, "Pulse length P 3 (cpmg 90)");
	fgSizer41->Add(P3LabelST);
	
	P3ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer41->Add(P3ST);
	
	
	P4LabelST = new wxStaticText(this, wxID_ANY, "Pulse length P 4 (cpmg 180)");
	fgSizer41->Add(P4LabelST);
	
	P4ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer41->Add(P4ST);
	
	gSizer4->Add(fgSizer41, 1, wxEXPAND|wxALL, FromDIP(5));
	
	
	wxFlexGridSizer* fgSizer42;
	fgSizer42 = new wxFlexGridSizer(4, 2, FromDIP(5), FromDIP(8));
	fgSizer42->SetFlexibleDirection(wxBOTH);
	fgSizer42->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	PL1LabelST = new wxStaticText(this, wxID_ANY, "RF power level PL 1");
	fgSizer42->Add(PL1LabelST);
	
	PL1ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer42->Add(PL1ST);
	
	
	PL2LabelST = new wxStaticText(this, wxID_ANY, "RF power level PL 2");
	fgSizer42->Add(PL2LabelST);
	
	PL2ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer42->Add(PL2ST);
	
	
	PL21LabelST = new wxStaticText(this, wxID_ANY, "RF power level PL 21");
	fgSizer42->Add(PL21LabelST);
	
	PL21ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer42->Add(PL21ST);
	
	
	PL22LabelST = new wxStaticText(this, wxID_ANY, "RF power level PL 22");
	fgSizer42->Add(PL22LabelST);
	
	PL22ST = new wxStaticText(this, wxID_ANY, wxEmptyString);
	fgSizer42->Add(PL22ST);
	
	gSizer4->Add(fgSizer42, 1, wxEXPAND|wxALL, FromDIP(5));
	
	sbSizer4->Add(gSizer4, 1, wxEXPAND, 0);
	
	fgSizer0->Add(sbSizer4, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, FromDIP(5));
	
	this->SetSizer(fgSizer0);
	
	/// set the appropriate values
	LoadAcquInfo(AcquInfo, path);
	
}
	
void NFGInfoPanel::LoadAcquInfo(AcquParams* AcquInfo, wxString path) {
	
	wxStaticText* ToClearST[33] = {
		PathST, AcqusTitleST, AcqusDateST, PulseProgramST, SampleRateST, DWST, 
		FilterWidthST, ReceiverGainST, NumberOfScansST, FrequencyST, DEST, D1ST, 
		D3ST, D6ST, P1ST, P2ST, P3ST, P4ST, 
		PL1ST, PL2ST, PL21ST, PL22ST, NumberOfChunksST, StepCountST, 
		ExpTypeST, VariableNameLabelST, VariableNameST, MinST, CoefST, MaxST, 
		StepST, TunedLabelST, TunedST
	};
	
	Freeze();
	
	/// clear what is necessary
	for (size_t i = 0; i < 33; i++) 
		ToClearST[i]->SetLabel(wxEmptyString);
	
	
	PathST->SetLabel(path);
	
	if (AcquInfo != NULL) {
		if (AcquInfo->AcquFlag & ACQU_Acqus) {
			wxString ATitle(AcquInfo->Title);
			ATitle.Replace("\t", " ");
			AcqusTitleST->SetLabel(ATitle);
			AcqusDateST->SetLabel(wxDateTime((time_t) (AcquInfo->Date)).Format("%#c"));
			
			PulseProgramST->SetLabel(wxString(AcquInfo->PulProg));
			SampleRateST->SetLabel(wxString::Format("%.12g MSps", 1.0e-6*AcquInfo->SWh));	/// [MSps]
			DWST->SetLabel(wxString::Format("%.12g us", 0.5e6/AcquInfo->SWh));	/// [us]
			FilterWidthST->SetLabel(wxString::Format("%.12g kHz", 1.0e-3*AcquInfo->FW));		/// [kHz]
			ReceiverGainST->SetLabel(wxString::Format("%.12g", AcquInfo->RG));
			NumberOfScansST->SetLabel(wxString::Format("%ld", AcquInfo->NS));
			FrequencyST->SetLabel(wxString::Format("%.12g MHz", AcquInfo->Freq));
			DEST->SetLabel(wxString::Format("%.12g us", AcquInfo->DE));
			
			if (AcquInfo->Dlength > 6) {
				D1ST->SetLabel(wxString::Format("%.12g s", AcquInfo->D[1]));
				D3ST->SetLabel(wxString::Format("%.12g us", 1.0e6*AcquInfo->D[3]));
				D6ST->SetLabel(wxString::Format("%.12g us", 1.0e6*AcquInfo->D[6]));
			}
			
			if (AcquInfo->Plength > 4) {
				P1ST->SetLabel(wxString::Format("%.12g us", AcquInfo->P[1]));
				P2ST->SetLabel(wxString::Format("%.12g us", AcquInfo->P[2]));
				P3ST->SetLabel(wxString::Format("%.12g us", AcquInfo->P[3]));
				P4ST->SetLabel(wxString::Format("%.12g us", AcquInfo->P[4]));
			}
			
			if (AcquInfo->PLWlength > 22) {
				PL1ST->SetLabel(wxString::Format("%.12g W", AcquInfo->PLW[1]));
				PL2ST->SetLabel(wxString::Format("%.12g W", AcquInfo->PLW[2]));
				PL21ST->SetLabel(wxString::Format("%.12g W", AcquInfo->PLW[21]));
				PL22ST->SetLabel(wxString::Format("%.12g W", AcquInfo->PLW[22]));
			} else
			if (AcquInfo->PLlength > 22) {
				PL1ST->SetLabel(wxString::Format("%.12g dB", AcquInfo->PL[1]));
				PL2ST->SetLabel(wxString::Format("%.12g dB", AcquInfo->PL[2]));
				PL21ST->SetLabel(wxString::Format("%.12g dB", AcquInfo->PL[21]));
				PL22ST->SetLabel(wxString::Format("%.12g dB", AcquInfo->PL[22]));
			}
		}
		
		/// show/hide Experiment details
		fgSizer0->Show((size_t) 1, ((AcquInfo->AcquFlag & ACQU_Counts) && (AcquInfo->StepCount > 1)) || (AcquInfo->AcquFlag & ACQU_Userlist) || (AcquInfo->AcquFlag & ACQU_vlist));
		
		if (AcquInfo->AcquFlag & ACQU_Counts) {
			NumberOfChunksST->SetLabel(wxString::Format("%ld", AcquInfo->ChunkCount));
			StepCountST->SetLabel(wxString::Format("%ld", AcquInfo->StepCount));
		}
		
		if ((AcquInfo->AcquFlag & ACQU_Userlist) || (AcquInfo->AcquFlag & ACQU_vlist)) {
			fgSizer21->Show((size_t) 0);
			fgSizer21->Show((size_t) 1);
			
			fgSizer21->Show((size_t) 2);
			fgSizer21->Show((size_t) 3);
			
			
			fgSizer22->Show((size_t) 0);
			fgSizer22->Show((size_t) 1);

			fgSizer22->Show((size_t) 2);
			fgSizer22->Show((size_t) 3);

			wxString AssocUnits;
			if (AcquInfo->AssocValueUnits != NULL) 
				AssocUnits = " " + wxString(AcquInfo->AssocValueUnits);
			
			if (AcquInfo->AssocValueTypeName != NULL) {
				ExpTypeST->SetLabel(wxString(AcquInfo->AssocValueTypeName));
				VariableNameLabelST->SetLabel("Variable");
			} else {
				ExpTypeST->SetLabel("not set");
			}
			
			if (AcquInfo->AssocValueVariable != NULL)
				VariableNameST->SetLabel(wxString(AcquInfo->AssocValueVariable));

			MinST->SetLabel(wxString::Format("%.12g", AcquInfo->AssocValueMin) + AssocUnits);
			MaxST->SetLabel(wxString::Format("%.12g", AcquInfo->AssocValueMax) + AssocUnits);
			
			if (AcquInfo->AcquFlag & ACQU_Userlist) {
				CoefST->SetLabel(wxString::Format("%.12g", AcquInfo->AssocValueCoef));
				StepST->SetLabel(wxString::Format("%.12g", AcquInfo->AssocValueStep) + AssocUnits);
				
				fgSizer21->Show((size_t) 6);
				fgSizer21->Show((size_t) 7);
				
				fgSizer22->Show((size_t) 4);
				fgSizer22->Show((size_t) 5);
				
			} else {	/// (AcquInfo->AcquFlag & ACQU_vlist)
				fgSizer21->Hide((size_t) 6);
				fgSizer21->Hide((size_t) 7);
				
				fgSizer22->Hide((size_t) 4);
				fgSizer22->Hide((size_t) 5);
			}
			
			if (AcquInfo->AcquFlag & ACQU_Counts) {
				if ((AcquInfo->AcquFlag & ACQU_Userlist) && ((AcquInfo->AssocValueType == ASSOC_FREQ_MHZ) || !(AcquInfo->WobbStep > AcquInfo->StepCount))) {
					if (AcquInfo->WobbStep == AcquInfo->StepCount) {
						TunedLabelST->SetLabel("Tuned");
						TunedST->SetLabel("before start");
					} else
					if (AcquInfo->WobbStep > AcquInfo->StepCount) {
						TunedLabelST->SetLabel("Not tuned");
						TunedST->SetLabel(wxEmptyString);
					} else 
					if (AcquInfo->WobbStep == 1) {
						TunedLabelST->SetLabel("Tuned every");
						TunedST->SetLabel("step");
					} else {
						TunedLabelST->SetLabel("Tuned every");
						TunedST->SetLabel(wxString::Format("%ld steps", AcquInfo->WobbStep));
					}
				}
			}
			
		} else {
			fgSizer21->Hide((size_t) 0);
			fgSizer21->Hide((size_t) 1);

			fgSizer21->Hide((size_t) 2);
			fgSizer21->Hide((size_t) 3);
			
			fgSizer21->Hide((size_t) 6);
			fgSizer21->Hide((size_t) 7);
			
			
			fgSizer22->Hide((size_t) 0);
			fgSizer22->Hide((size_t) 1);
			
			fgSizer22->Hide((size_t) 2);
			fgSizer22->Hide((size_t) 3);

			fgSizer22->Hide((size_t) 4);
			fgSizer22->Hide((size_t) 5);
		}
		
	}
	
	/// Layout
	wxWindow* parent = GetParent();
	
	if (parent)
		parent->Fit();

	Thaw();

#ifndef GTK_OXYGEN_FIX
	if (parent && parent->IsShown())
#endif
		PostSizeEventToParent();
}

NFGInfoPanel::~NFGInfoPanel()
{
}
