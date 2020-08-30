/* 
 * NMRFilip GUI - the NMR data processing software - graphical user interface
 * Copyright (C) 2010, 2011, 2020 Richard Reznicek
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

#include "panelmain.h"
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/valgen.h>
#include <wx/display.h>
#include <wx/filefn.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/docview.h>

#include "panels.h"
#include "validators.h"
#include "view.h"
#include "doc.h"
#include "plotgen.h"
#include "plotimp.h"
#include "plotwin.h"
#include "nmrfilipgui.h"
#include "gui_ids.h"


NFGColourTag::NFGColourTag(wxWindow* parent, wxColour colour, wxSize size) : wxWindow(parent, wxID_ANY, wxDefaultPosition, size)
{
	SetBackgroundColour(colour);
//	SetBackgroundStyle(wxBG_STYLE_COLOUR);
}

NFGColourTag::~NFGColourTag()
{
}



BEGIN_EVENT_TABLE(NFGDisplayInnerPanel, wxPanel)
	EVT_UPDATE_UI_RANGE(DatasetIDMin, DatasetIDMax, NFGDisplayInnerPanel::OnUpdateUIDataSets)
	EVT_UPDATE_UI(ID_DisplayToolbook, NFGDisplayInnerPanel::OnUpdateUI)
	EVT_TOOLBOOK_PAGE_CHANGED(wxID_ANY, NFGDisplayInnerPanel::OnDisplayChange)
	EVT_COMMAND_RANGE(DatasetIDMin, DatasetIDMax, wxEVT_CHECKBOX, NFGDisplayInnerPanel::OnDataSetDisplayChange)
END_EVENT_TABLE()

NFGDisplayInnerPanel::NFGDisplayInnerPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	DocMan = DocManager;
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxVERTICAL);
	
	DisplayToolbook = new wxToolbook(this, ID_DisplayToolbook);
#ifdef __WXGTK__
	if (DisplayToolbook->GetToolBar()) 
		DisplayToolbook->GetToolBar()->SetWindowStyle(DisplayToolbook->GetToolBar()->GetWindowStyle() & ~wxTB_TEXT);
#endif
	
	const wxArtID BmpID[] = {"refresh_icon", "params", "tdd", "chunkavg", "ft", "ftenv", "eval"};
	wxSize DisplayToolbookImageSize = FromDIP(wxSize(24, 24));
	int DisplayToolbookIndex = 0;
	wxImageList* DisplayToolbookImages = new wxImageList(DisplayToolbookImageSize.GetWidth(), DisplayToolbookImageSize.GetHeight());
	
	for (size_t i = 0; i < 7; i++) {
		wxBitmap DisplayToolbookBmp = wxArtProvider::GetBitmap(BmpID[i], wxART_TOOLBAR, DisplayToolbookImageSize);
		if (DisplayToolbookBmp.IsOk()) 
			DisplayToolbookImages->Add(DisplayToolbookBmp);
	}

	DisplayToolbook->AssignImageList(DisplayToolbookImages);
	
	ReloadPanel = new wxPanel(DisplayToolbook, DisplayToolbookIndex, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	DisplayToolbook->AddPage(ReloadPanel, wxEmptyString, false, DisplayToolbookIndex++);

	
	ParamsPanel = new wxPanel(DisplayToolbook, DisplayToolbookIndex, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	
	wxBoxSizer* ParamsBSizer;
	ParamsBSizer = new wxBoxSizer(wxVERTICAL);
	
	ParamsLabelST = new wxStaticText(ParamsPanel, wxID_ANY, "Acquisition parameters");
	ParamsLabelST->SetFont(ParamsLabelST->GetFont().MakeBold());
	
	ParamsBSizer->Add(ParamsLabelST, 0, wxLEFT|wxRIGHT, FromDIP(5));
	
	ParamsPanel->SetSizer(ParamsBSizer);
	ParamsPanel->Layout();
	ParamsBSizer->Fit(ParamsPanel);
	DisplayToolbook->AddPage(ParamsPanel, wxEmptyString, false, DisplayToolbookIndex++);

	TDDPanel = new wxPanel(DisplayToolbook, DisplayToolbookIndex, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	
	wxFlexGridSizer* TDDFGSizer;
	TDDFGSizer = new wxFlexGridSizer(2, 1, FromDIP(2), 0);
	TDDFGSizer->SetFlexibleDirection(wxBOTH);
	TDDFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	TDDLabelST = new wxStaticText(TDDPanel, wxID_ANY, "Time domain data");
	TDDLabelST->SetFont(TDDLabelST->GetFont().MakeBold());
	
	TDDFGSizer->Add(TDDLabelST, 0, wxLEFT|wxRIGHT, FromDIP(5));
	
	wxFlexGridSizer* TDDInnerFGSizer;
	TDDInnerFGSizer = new wxFlexGridSizer(4, 2, FromDIP(5), FromDIP(5));
	TDDInnerFGSizer->SetFlexibleDirection(wxBOTH);
	TDDInnerFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	TDDRealColourTag = new NFGColourTag(TDDPanel, wxColour(255, 0, 0), FromDIP(wxSize(6,12)));
	TDDInnerFGSizer->Add(TDDRealColourTag, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 0);

	TDDRealCheckBox = new wxCheckBox(TDDPanel, ID_TDDReal, "Real");
	TDDRealCheckBox->SetValue(true);
	TDDInnerFGSizer->Add(TDDRealCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	TDDImagColourTag = new NFGColourTag(TDDPanel, wxColour(0, 255, 0), FromDIP(wxSize(6,12)));
	TDDInnerFGSizer->Add(TDDImagColourTag, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 0);
	
	TDDImagCheckBox = new wxCheckBox(TDDPanel, ID_TDDImag, "Imaginary");
	TDDImagCheckBox->SetValue(true);
	TDDInnerFGSizer->Add(TDDImagCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	TDDModuleColourTag = new NFGColourTag(TDDPanel, wxColour(0, 0, 255), FromDIP(wxSize(6,12)));
	TDDInnerFGSizer->Add(TDDModuleColourTag, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 0);
	
	TDDModuleCheckBox = new wxCheckBox(TDDPanel, ID_TDDModule, "Modulus");
	TDDModuleCheckBox->SetValue(true);
	TDDInnerFGSizer->Add(TDDModuleCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	TDDEchoPeaksEnvelopeColourTag = new NFGColourTag(TDDPanel, wxColour(128, 128, 0), FromDIP(wxSize(6,12)));
	TDDInnerFGSizer->Add(TDDEchoPeaksEnvelopeColourTag, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 0);
	
	TDDEchoPeaksEnvelopeCheckBox = new wxCheckBox(TDDPanel, ID_TDDEchoPeaksEnvelope, "Echo peaks envelope");
	TDDInnerFGSizer->Add(TDDEchoPeaksEnvelopeCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	TDDFGSizer->Add(TDDInnerFGSizer, 1, wxALL|wxEXPAND, FromDIP(5));
	
	TDDPanel->SetSizer(TDDFGSizer);
	TDDPanel->Layout();
	TDDFGSizer->Fit(TDDPanel);
	DisplayToolbook->AddPage(TDDPanel, wxEmptyString, true, DisplayToolbookIndex++);

	ChunkAvgPanel = new wxPanel(DisplayToolbook, DisplayToolbookIndex, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	
	wxFlexGridSizer* ChunkAvgFGSizer;
	ChunkAvgFGSizer = new wxFlexGridSizer(2, 1, FromDIP(2), 0);
	ChunkAvgFGSizer->SetFlexibleDirection(wxBOTH);
	ChunkAvgFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	ChunkAvgLabelST = new wxStaticText(ChunkAvgPanel, wxID_ANY, "Chunk average");
	ChunkAvgLabelST->SetFont(ChunkAvgLabelST->GetFont().MakeBold());
	
	ChunkAvgFGSizer->Add(ChunkAvgLabelST, 0, wxLEFT|wxRIGHT, FromDIP(5));
	
	wxFlexGridSizer* ChunkAvgInnerFGSizer;
	ChunkAvgInnerFGSizer = new wxFlexGridSizer(3, 2, FromDIP(5), FromDIP(5));
	ChunkAvgInnerFGSizer->SetFlexibleDirection(wxBOTH);
	ChunkAvgInnerFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	ChunkAvgRealColourTag = new NFGColourTag(ChunkAvgPanel, wxColour(255, 0, 0), FromDIP(wxSize(6,12)));
	ChunkAvgInnerFGSizer->Add(ChunkAvgRealColourTag, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 0);
	
	ChunkAvgRealCheckBox = new wxCheckBox(ChunkAvgPanel, ID_ChunkAvgReal, "Real");
	ChunkAvgRealCheckBox->SetValue(true);
	ChunkAvgInnerFGSizer->Add(ChunkAvgRealCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ChunkAvgImagColourTag = new NFGColourTag(ChunkAvgPanel, wxColour(0, 255, 0), FromDIP(wxSize(6,12)));
	ChunkAvgInnerFGSizer->Add(ChunkAvgImagColourTag, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 0);
	
	ChunkAvgImagCheckBox = new wxCheckBox(ChunkAvgPanel, ID_ChunkAvgImag, "Imaginary");
	ChunkAvgImagCheckBox->SetValue(true);
	ChunkAvgInnerFGSizer->Add(ChunkAvgImagCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ChunkAvgModuleColourTag = new NFGColourTag(ChunkAvgPanel, wxColour(0, 0, 255), FromDIP(wxSize(6,12)));
	ChunkAvgInnerFGSizer->Add(ChunkAvgModuleColourTag, 0, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	
	ChunkAvgModuleCheckBox = new wxCheckBox(ChunkAvgPanel, ID_ChunkAvgModule, "Modulus");
	ChunkAvgModuleCheckBox->SetValue(true);
	ChunkAvgInnerFGSizer->Add(ChunkAvgModuleCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	ChunkAvgFGSizer->Add(ChunkAvgInnerFGSizer, 1, wxALL|wxEXPAND, FromDIP(5));
	
	ChunkAvgPanel->SetSizer(ChunkAvgFGSizer);
	ChunkAvgPanel->Layout();
	ChunkAvgFGSizer->Fit(ChunkAvgPanel);
	DisplayToolbook->AddPage(ChunkAvgPanel, wxEmptyString, false, DisplayToolbookIndex++);

	FFTPanel = new wxPanel(DisplayToolbook, DisplayToolbookIndex, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	
	wxFlexGridSizer* FFTFGSizer;
	FFTFGSizer = new wxFlexGridSizer(2, 1, FromDIP(2), 0);
	FFTFGSizer->SetFlexibleDirection(wxBOTH);
	FFTFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	FFTLabelST = new wxStaticText(FFTPanel, wxID_ANY, "FFT");
	FFTLabelST->SetFont(FFTLabelST->GetFont().MakeBold());
	
	FFTFGSizer->Add(FFTLabelST, 0, wxLEFT|wxRIGHT, FromDIP(5));
	
	wxFlexGridSizer* FFTInnerFGSizer;
	FFTInnerFGSizer = new wxFlexGridSizer(3, 2, FromDIP(5), FromDIP(5));
	FFTInnerFGSizer->SetFlexibleDirection(wxBOTH);
	FFTInnerFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	FFTRealColourTag = new NFGColourTag(FFTPanel, wxColour(255, 0, 0), FromDIP(wxSize(6,12)));
	FFTInnerFGSizer->Add(FFTRealColourTag, 0, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	
	FFTRealCheckBox = new wxCheckBox(FFTPanel, ID_FFTReal, "Real");
	FFTRealCheckBox->SetValue(true);
	FFTInnerFGSizer->Add(FFTRealCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	FFTImagColourTag = new NFGColourTag(FFTPanel, wxColour(0, 255, 0), FromDIP(wxSize(6,12)));
	FFTInnerFGSizer->Add(FFTImagColourTag, 0, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	
	FFTImagCheckBox = new wxCheckBox(FFTPanel, ID_FFTImag, "Imaginary");
	FFTImagCheckBox->SetValue(true);
	FFTInnerFGSizer->Add(FFTImagCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	FFTModuleColourTag = new NFGColourTag(FFTPanel, wxColour(0, 0, 255), FromDIP(wxSize(6,12)));
	FFTInnerFGSizer->Add(FFTModuleColourTag, 0, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	
	FFTModuleCheckBox = new wxCheckBox(FFTPanel, ID_FFTModule, "Modulus");
	FFTModuleCheckBox->SetValue(true);
	FFTInnerFGSizer->Add(FFTModuleCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	FFTFGSizer->Add(FFTInnerFGSizer, 1, wxALL|wxEXPAND, FromDIP(5));
	
	FFTPanel->SetSizer(FFTFGSizer);
	FFTPanel->Layout();
	FFTFGSizer->Fit(FFTPanel);
	DisplayToolbook->AddPage(FFTPanel, wxEmptyString, false, DisplayToolbookIndex++);

	SpectrumPanel = new wxPanel(DisplayToolbook, DisplayToolbookIndex, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	
	wxFlexGridSizer* SpectrumFGSizer;
	SpectrumFGSizer = new wxFlexGridSizer(2, 1, FromDIP(2), 0);
	SpectrumFGSizer->SetFlexibleDirection(wxBOTH);
	SpectrumFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	SpectrumLabelST = new wxStaticText(SpectrumPanel, wxID_ANY, "Spectrum");
	SpectrumLabelST->SetFont(SpectrumLabelST->GetFont().MakeBold());
	
	SpectrumFGSizer->Add(SpectrumLabelST, 0, wxLEFT|wxRIGHT, FromDIP(5));
	
	wxFlexGridSizer* SpectrumInnerFGSizer;
	SpectrumInnerFGSizer = new wxFlexGridSizer(4, 2, FromDIP(5), FromDIP(5));
	SpectrumInnerFGSizer->SetFlexibleDirection(wxBOTH);
	SpectrumInnerFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	SpectrumFFTEnvelopeColourTag = new NFGColourTag(SpectrumPanel, wxColour(255, 0, 128), FromDIP(wxSize(6,12)));
	SpectrumInnerFGSizer->Add(SpectrumFFTEnvelopeColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	SpectrumFFTEnvelopeCheckBox = new wxCheckBox(SpectrumPanel, ID_SpectrumFFTEnvelope, "FFT moduli envelope");
	SpectrumFFTEnvelopeCheckBox->SetValue(true);
	SpectrumInnerFGSizer->Add(SpectrumFFTEnvelopeCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);

	SpectrumParticularFFTModulesColourTag = new NFGColourTag(SpectrumPanel, wxColour(0, 0, 255), FromDIP(wxSize(6,12)));
	SpectrumInnerFGSizer->Add(SpectrumParticularFFTModulesColourTag, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 0);
	
	SpectrumParticularFFTModulesCheckBox = new wxCheckBox(SpectrumPanel, ID_SpectrumParticularFFTModules, "Particular FFT moduli");
	SpectrumParticularFFTModulesCheckBox->SetValue(true);
	SpectrumInnerFGSizer->Add(SpectrumParticularFFTModulesCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	SpectrumFFTRealEnvelopeColourTag = new NFGColourTag(SpectrumPanel, wxColour(64, 255, 0), FromDIP(wxSize(6,12)));
	SpectrumInnerFGSizer->Add(SpectrumFFTRealEnvelopeColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	SpectrumFFTRealEnvelopeCheckBox = new wxCheckBox(SpectrumPanel, ID_SpectrumFFTRealEnvelope, "FFT real parts envelope");
	SpectrumFFTRealEnvelopeCheckBox->SetValue(false);
	SpectrumInnerFGSizer->Add(SpectrumFFTRealEnvelopeCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	SpectrumParticularFFTRealPartsColourTag = new NFGColourTag(SpectrumPanel, wxColour(255, 0 ,0), FromDIP(wxSize(6,12)));
	SpectrumInnerFGSizer->Add(SpectrumParticularFFTRealPartsColourTag, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 0);
	
	SpectrumParticularFFTRealPartsCheckBox = new wxCheckBox(SpectrumPanel, ID_SpectrumParticularFFTRealParts, "Particular FFT real parts");
	SpectrumParticularFFTRealPartsCheckBox->SetValue(false);
	SpectrumInnerFGSizer->Add(SpectrumParticularFFTRealPartsCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	SpectrumFGSizer->Add(SpectrumInnerFGSizer, 1, wxALL|wxEXPAND, FromDIP(5));
	
	SpectrumPanel->SetSizer(SpectrumFGSizer);
	SpectrumPanel->Layout();
	SpectrumFGSizer->Fit(SpectrumPanel);
	DisplayToolbook->AddPage(SpectrumPanel, wxEmptyString, false, DisplayToolbookIndex++);


	EvaluationPanel = new wxPanel(DisplayToolbook, DisplayToolbookIndex, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	
	wxFlexGridSizer* EvaluationFGSizer;
	EvaluationFGSizer = new wxFlexGridSizer(2, 1, FromDIP(2), 0);
	EvaluationFGSizer->SetFlexibleDirection(wxBOTH);
	EvaluationFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	EvaluationLabelST = new wxStaticText(EvaluationPanel, wxID_ANY, "Evaluation");
	EvaluationLabelST->SetFont(EvaluationLabelST->GetFont().MakeBold());
	
	EvaluationFGSizer->Add(EvaluationLabelST, 0, wxLEFT|wxRIGHT, FromDIP(5));
	
	wxFlexGridSizer* EvaluationInnerFGSizer;
	EvaluationInnerFGSizer = new wxFlexGridSizer(8, 2, FromDIP(5), FromDIP(5));
	EvaluationInnerFGSizer->SetFlexibleDirection(wxBOTH);
	EvaluationInnerFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	EvaluationFFTMaxColourTag = new NFGColourTag(EvaluationPanel, wxColour(0, 128, 64), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationFFTMaxColourTag, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 0);
	
	EvaluationFFTMaxCheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationFFTMax, "FFT modulus maximum");
	EvaluationFFTMaxCheckBox->SetValue(true);
	EvaluationInnerFGSizer->Add(EvaluationFFTMaxCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationFFT0ColourTag = new NFGColourTag(EvaluationPanel, wxColour(128, 0, 0), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationFFT0ColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	EvaluationFFT0CheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationFFT0, "FFT modulus at excitation frequency");
	EvaluationFFT0CheckBox->SetValue(true);
	EvaluationInnerFGSizer->Add(EvaluationFFT0CheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationFFTMeanColourTag = new NFGColourTag(EvaluationPanel, wxColour(0, 128, 192), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationFFTMeanColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	EvaluationFFTMeanCheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationFFTMean, "FFT modulus mean");
	EvaluationInnerFGSizer->Add(EvaluationFFTMeanCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationFFTRealMaxColourTag = new NFGColourTag(EvaluationPanel, wxColour(0, 255, 0), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationFFTRealMaxColourTag, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 0);
	
	EvaluationFFTRealMaxCheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationFFTRealMax, "FFT real maximum/minimum");
	EvaluationInnerFGSizer->Add(EvaluationFFTRealMaxCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationFFTReal0ColourTag = new NFGColourTag(EvaluationPanel, wxColour(255, 0, 0), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationFFTReal0ColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	EvaluationFFTReal0CheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationFFTReal0, "FFT real at excitation frequency");
	EvaluationInnerFGSizer->Add(EvaluationFFTReal0CheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationFFTRealMeanColourTag = new NFGColourTag(EvaluationPanel, wxColour(0, 0, 255), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationFFTRealMeanColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	EvaluationFFTRealMeanCheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationFFTRealMean, "FFT real mean");
	EvaluationInnerFGSizer->Add(EvaluationFFTRealMeanCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationChunkAvgModMaxColourTag = new NFGColourTag(EvaluationPanel, wxColour(128, 0, 255), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationChunkAvgModMaxColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	EvaluationChunkAvgModMaxCheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationChunkAvgModMax, "Chunk average modulus maximum");
	EvaluationChunkAvgModMaxCheckBox->SetValue(true);
	EvaluationInnerFGSizer->Add(EvaluationChunkAvgModMaxCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationChunkAvgModIntColourTag = new NFGColourTag(EvaluationPanel, wxColour(255, 128, 192), FromDIP(wxSize(6,12)));
	EvaluationInnerFGSizer->Add(EvaluationChunkAvgModIntColourTag, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 0);
	
	EvaluationChunkAvgModIntCheckBox = new wxCheckBox(EvaluationPanel, ID_EvaluationChunkAvgModInt, "Chunk average modulus integral");
	EvaluationInnerFGSizer->Add(EvaluationChunkAvgModIntCheckBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	
	EvaluationFGSizer->Add(EvaluationInnerFGSizer, 1, wxALL|wxEXPAND, FromDIP(5));

	EvaluationPanel->SetSizer(EvaluationFGSizer);
	EvaluationPanel->Layout();
	EvaluationFGSizer->Fit(EvaluationPanel);
	DisplayToolbook->AddPage(EvaluationPanel, wxEmptyString, false, DisplayToolbookIndex++);

	/// just to improve user experience
	if (DisplayToolbook->GetToolBar()) {
		DisplayToolbookIndex = 0;
		
		DisplayToolbook->GetToolBar()->SetToolShortHelp(DisplayToolbookIndex++, "Reload");
		DisplayToolbook->GetToolBar()->SetToolShortHelp(DisplayToolbookIndex++, "Acquisition parameters");
		DisplayToolbook->GetToolBar()->SetToolShortHelp(DisplayToolbookIndex++, "Time domain data");
		DisplayToolbook->GetToolBar()->SetToolShortHelp(DisplayToolbookIndex++, "Chunk average");
		DisplayToolbook->GetToolBar()->SetToolShortHelp(DisplayToolbookIndex++, "FFT");
		DisplayToolbook->GetToolBar()->SetToolShortHelp(DisplayToolbookIndex++, "Spectrum");
		DisplayToolbook->GetToolBar()->SetToolShortHelp(DisplayToolbookIndex++, "Evaluation");
		
		DisplayToolbook->GetToolBar()->Realize();

#ifdef __WXMSW__
		if (parent) {
			wxColour col = parent->GetBackgroundColour();
			if (col.IsOk()) 
				DisplayToolbook->GetToolBar()->SetBackgroundColour(col);
		}
#endif
	}

	bSizer11->Add(DisplayToolbook, 1, wxEXPAND | wxALL, 0);
	
	this->SetSizer(bSizer11);
	this->Layout();
	bSizer11->Fit(this);
}

NFGDisplayInnerPanel::~NFGDisplayInnerPanel()
{
}


void NFGDisplayInnerPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);
	
	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) 
		return;

	switch (SerDoc->GraphTypeQuery()) {
		case GraphTypeTDD:
			DisplayToolbook->ChangeSelection(2);
			break;
		case GraphTypeChunkAvg:
			DisplayToolbook->ChangeSelection(3);
			break;
		case GraphTypeFFT:
			DisplayToolbook->ChangeSelection(4);
			break;
		case GraphTypeSpectrum:
			DisplayToolbook->ChangeSelection(5);
			break;
		case GraphTypeEvaluation:
			DisplayToolbook->ChangeSelection(6);
			break;
		default:
			;
	}
}


void NFGDisplayInnerPanel::OnUpdateUIDataSets(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);
	
	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) 
		return;

	if ((event.GetId() >= DatasetIDMin) && (event.GetId() <= DatasetIDMax))
		event.Check(SerDoc->DatasetDisplayQuery() & (1ul << (event.GetId() - DatasetIDMin)));
}


void NFGDisplayInnerPanel::OnDisplayChange(wxToolbookEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if (SerDoc == NULL)
		return;
	
	wxList::iterator iter;
	NFGInfoView* ExpInfoView = NULL;
	
	switch (event.GetSelection()) {
		case 0:
			SerDoc->ReloadData();
			break;
		case 1:
			/// quite non-standard use of document/view framework
			
			/// check if this kind of view is already opened
			for (iter = (SerDoc->GetViews()).begin(); iter != (SerDoc->GetViews()).end(); ++iter)
			{
				wxView* current = (wxView *)*iter;
				
				NFGInfoView* iview = wxDynamicCast(current, NFGInfoView);
				if (iview) {
					iview->Activate(true);	/// probably not necessary
					wxWindow *win = iview->GetFrame();
					if (win) {
						wxDocDIChildFrame* iframe = wxDynamicCast(win, wxDocDIChildFrame);
						if (iframe)
#ifdef __NMRFilipGUI_MDI__
							iframe->Activate();
#else
							iframe->Raise();
#endif
						win->SetFocus();	/// actually activates the window
					}
					
					return;
				}
			}
			
			/// create the view object
			ExpInfoView = new NFGInfoView();
			if (ExpInfoView == NULL)
				return;
			
			/// sets the wxView::m_viewDocument variable and calls wxDocument::AddView() function
			ExpInfoView->SetDocument(SerDoc);

			/// create the view window etc.
			if (!(ExpInfoView->OnCreate(SerDoc, 0))) {
				delete ExpInfoView;
				return;
			}
			
			/// sets the proper window title
			ExpInfoView->OnChangeFilename();
			
			ExpInfoView->Activate(true);
			
			break;
		case 2:
			SerDoc->GraphTypeCommand(GraphTypeTDD);
			break;
		case 3:
			SerDoc->GraphTypeCommand(GraphTypeChunkAvg);
			break;
		case 4:
			SerDoc->GraphTypeCommand(GraphTypeFFT);
			break;
		case 5:
			SerDoc->GraphTypeCommand(GraphTypeSpectrum);
			break;
		case 6:
			SerDoc->GraphTypeCommand(GraphTypeEvaluation);
			break;
		default:
			;
	}
}


void NFGDisplayInnerPanel::OnDataSetDisplayChange(wxCommandEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if (SerDoc == NULL)
		return;
	
	if (!((event.GetId() >= DatasetIDMin) && (event.GetId() <= DatasetIDMax)))
		return;
	
	unsigned long flag;
	
	if (event.IsChecked()) 
		flag = SerDoc->DatasetDisplayQuery() | (1ul << (event.GetId() - DatasetIDMin));
	else
		flag = SerDoc->DatasetDisplayQuery() & ~(1ul << (event.GetId() - DatasetIDMin));
	
	flag = SerDoc->DatasetDisplayCommand(flag);
}



IMPLEMENT_DYNAMIC_CLASS(NFGBrowseTreeCtrl, wxTreeCtrl)

BEGIN_EVENT_TABLE(NFGBrowseTreeCtrl, wxTreeCtrl)
	EVT_TREE_ITEM_EXPANDING(wxID_ANY, NFGBrowseTreeCtrl::OnItemExpanding)
	EVT_TREE_ITEM_COLLAPSED(wxID_ANY, NFGBrowseTreeCtrl::OnItemCollapsed)
END_EVENT_TABLE()

NFGBrowseTreeCtrl::NFGBrowseTreeCtrl(wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxTreeCtrl(parent, id, pos, size, style) 
{
	wxSize ImageSize = FromDIP(wxSize(16, 16));
	wxImageList *images = new wxImageList(ImageSize.GetWidth(), ImageSize.GetHeight());

	wxBitmap bmp;
	bmp = wxArtProvider::GetBitmap("folder", wxART_OTHER, ImageSize);
	if (bmp.Ok())
		images->Add(bmp);

	bmp = wxArtProvider::GetBitmap("serfile", wxART_OTHER, ImageSize);
	if (bmp.Ok())
		images->Add(bmp);

	bmp = wxArtProvider::GetBitmap("fidfile", wxART_OTHER, ImageSize);
	if (bmp.Ok())
		images->Add(bmp);

	bmp = wxArtProvider::GetBitmap("userlist", wxART_OTHER, ImageSize);
	if (bmp.Ok())
		images->Add(bmp);

	bmp = wxArtProvider::GetBitmap("textfile", wxART_OTHER, ImageSize);
	if (bmp.Ok())
		images->Add(bmp);

	AssignImageList(images);
	
	/// just to prevent strange sizing behavior - if there was at least one MDIChildFrame present and the virtual size of this was larger than client area then the first sizing event in this configuration often caused this to expand and made the ProcPanel shrink
	CacheBestSize(wxSize(10, 10));
}

void NFGBrowseTreeCtrl::OnItemExpanding(wxTreeEvent& event) 
{
	NFGBrowseTreeItemData *nodedata = (NFGBrowseTreeItemData *) GetItemData(event.GetItem());
	
	if (nodedata == NULL)
		return;
	
	SetItemHasChildren(event.GetItem(), IterateDir(nodedata->FullPath, event.GetItem()));
}

void NFGBrowseTreeCtrl::OnItemCollapsed(wxTreeEvent& event) 
{
	DeleteChildren(event.GetItem());
}

bool NFGBrowseTreeCtrl::IsInterestingDir(const wxString& dir) 
{
	if (!wxFileName::IsDirReadable(dir))
		return false;
	
	wxDir CDir(dir);
	if (!CDir.IsOpened())
		return false;

	if (CDir.HasSubDirs())
		return true;
	
	if (CDir.HasFiles("ser"))
		return true;
	
	if (CDir.HasFiles("fid"))
		return true;

	if (CDir.HasFiles("ulist"))
		return true;
	
	if (CDir.HasFiles("userlist"))
		return true;
	
	if (CDir.HasFiles("*.txt"))
		return true;
	
	return false;
}

wxTreeItemId NFGBrowseTreeCtrl::AddEntry(const wxString& name, const wxString& path, const wxTreeItemId& parent, int type, int image, bool SetNew) 
{
	wxTreeItemId node = this->AppendItem(parent, name, image);
	NFGBrowseTreeItemData *nodedata = new NFGBrowseTreeItemData();
	nodedata->Type = type;
	nodedata->Name = name;
	nodedata->FullPath = path;
	nodedata->IsNew = SetNew;
	nodedata->SetId(node);
	SetItemData(node, nodedata);
	
	return node;
}

bool NFGBrowseTreeCtrl::IterateDir(const wxString& dir, const wxTreeItemId& parent, bool SetNew) 
{
	bool retval = false;
	
	if (!wxFileName::IsDirReadable(dir))
		return false;

	wxDir CDir(dir);
	if (!CDir.IsOpened())
		return false;
	
	if (CDir.HasSubDirs()) {
		wxString dirname;
		bool cont = CDir.GetFirst(&dirname, wxEmptyString, wxDIR_DIRS);
		while (cont) {
			wxFileName FName = wxFileName::DirName(dir);
			FName.AppendDir(dirname);
			wxTreeItemId node = AddEntry(dirname, FName.GetFullPath(), parent, BT_DIR, 0, SetNew);
			SetItemHasChildren(node, IsInterestingDir(FName.GetFullPath()));
			
			cont = CDir.GetNext(&dirname);
		}
		
		retval = true;
	}

	if (CDir.HasFiles("ser")) {
		wxFileName FName(dir, "ser");
		AddEntry(FName.GetFullName(), FName.GetFullPath(), parent, BT_SERFILE, 1, SetNew);
		retval = true;
	}
	
	if (CDir.HasFiles("fid")) {
		wxFileName FName(dir, "fid");
		AddEntry(FName.GetFullName(), FName.GetFullPath(), parent, BT_TDDFILE, 2, SetNew);
		retval = true;
	}
	
	if (CDir.HasFiles("ulist")) {
		wxFileName FName(dir, "ulist");
		AddEntry(FName.GetFullName(), FName.GetFullPath(), parent, BT_USERLIST, 3, SetNew);
		retval = true;
	}
	
	if (CDir.HasFiles("userlist")) {
		wxFileName FName(dir, "userlist");
		AddEntry(FName.GetFullName(), FName.GetFullPath(), parent, BT_USERLIST, 3, SetNew);
		retval = true;
	}
	
	if (CDir.HasFiles("*.txt")) {
		wxString filename;
		bool cont = CDir.GetFirst(&filename, "*.txt", wxDIR_FILES);
		while (cont) {
			wxFileName FName(dir, filename);
			AddEntry(FName.GetFullName(), FName.GetFullPath(), parent, BT_TEXTFILE, 4, SetNew);
			cont = CDir.GetNext(&filename);
		}
		
		retval = true;
	}
	
	this->SortChildren(parent);
	return retval;
}


bool NFGBrowseTreeCtrl::ReiterateDir(const wxString& dir, const wxTreeItemId& parent) 
{
	NFGBrowseTreeItemData *nodedata, *nodedata2;
	wxTreeItemIdValue cookie;
	wxTreeItemId ItemID, ItemID2;
	
	if (!IsExpanded(parent))
		return IsInterestingDir(dir);
	
	if (!IterateDir(dir, parent, true)) {
		DeleteChildren(parent);
		return false;
	}
	
	ItemID = GetFirstChild(parent, cookie);
	while (ItemID.IsOk()) {
		
		nodedata = (NFGBrowseTreeItemData *) GetItemData(ItemID);
		
		if (nodedata == NULL) {
			DeleteChildren(parent);
			return false;
		}
		
		ItemID2 = GetNextSibling(ItemID);
		
		if (!nodedata->IsNew) {
			Delete(ItemID);
		} else {
			nodedata->IsNew = false;
			
			if (!ItemID2.IsOk())
				break;

			nodedata2 = (NFGBrowseTreeItemData *) GetItemData(ItemID2);
			if (nodedata2 == NULL) {
				DeleteChildren(parent);
				return false;
			}
			
			if ((!nodedata2->IsNew) && (nodedata2->Type == nodedata->Type) && ((nodedata2->Name).Cmp(nodedata->Name) == 0)) {
				if (nodedata2->Type == BT_DIR) {
					if (ItemHasChildren(ItemID) && IsExpanded(ItemID2)) 
						ReiterateDir(nodedata2->FullPath, ItemID2);
					else 
					if (!ItemHasChildren(ItemID) && ItemHasChildren(ItemID2)) {
						DeleteChildren(ItemID2);
						SetItemHasChildren(ItemID2, false);
					}
				}
				ItemID2 = GetNextSibling(ItemID2);
				Delete(ItemID);
			}
		}
	
		ItemID = ItemID2;
	}
	
	return true;
}


int NFGBrowseTreeCtrl::OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2) 
{
	NFGBrowseTreeItemData *nodedata1 = (NFGBrowseTreeItemData *) GetItemData(item1);
	NFGBrowseTreeItemData *nodedata2 = (NFGBrowseTreeItemData *) GetItemData(item2);
	long val1, val2;
	int StringCompRes;
	
	if ((nodedata1->Type == BT_DIR) && (nodedata2->Type != BT_DIR))
		return -1;
	
	if ((nodedata1->Type != BT_DIR) && (nodedata2->Type == BT_DIR))
		return 1;

	if ((nodedata1->Type == BT_SERFILE) && (nodedata2->Type != BT_SERFILE))
		return -1;
	
	if ((nodedata1->Type != BT_SERFILE) && (nodedata2->Type == BT_SERFILE))
		return 1;
	
	if ((nodedata1->Type == BT_TDDFILE) && (nodedata2->Type != BT_TDDFILE))
		return -1;
	
	if ((nodedata1->Type != BT_TDDFILE) && (nodedata2->Type == BT_TDDFILE))
		return 1;
	
	if ((nodedata1->Type == BT_USERLIST) && (nodedata2->Type != BT_USERLIST))
		return -1;
	
	if ((nodedata1->Type != BT_USERLIST) && (nodedata2->Type == BT_USERLIST))
		return 1;
	
	/// Reaching this implies (BT_NONE is never assigned) the same type of items.
	if ((nodedata1->Type == BT_DIR) && (nodedata2->Type == BT_DIR)) {
		if ((nodedata1->Name).ToLong(&val1) && (nodedata2->Name).ToLong(&val2)) {
			if (val1 > val2)
				return 1;
			if (val1 < val2)
				return -1;
		}
	}

	StringCompRes = (nodedata1->Name).CmpNoCase(nodedata2->Name);
	if (StringCompRes == 0) {
		if (nodedata1->IsNew && !nodedata2->IsNew)
			return -1;
		if (!nodedata1->IsNew && nodedata2->IsNew)
			return 1;
		return 0;	/// should not happen
	}
	
	return StringCompRes;
}


void NFGBrowseTreeCtrl::DisplayDirTree()
{
	wxWindow *TopWin = ::wxGetApp().GetTopWindow();
	if (TopWin)
		TopWin->SetLabel(::wxGetApp().GetAppDisplayName() + wxString(" - ") + ::wxGetCwd());

	this->DeleteAllItems();
	
	AddRoot(::wxGetCwd(), 0);
	
	NFGBrowseTreeItemData *nodedata = new NFGBrowseTreeItemData();
	nodedata->Type = BT_DIR;
	nodedata->Name = ::wxGetCwd();
	nodedata->FullPath = ::wxGetCwd();
	nodedata->IsNew = false;
	nodedata->SetId(GetRootItem());
	SetItemData(GetRootItem(), nodedata);
	
	SetItemHasChildren(GetRootItem(), IsInterestingDir(nodedata->FullPath));
	if (ItemHasChildren(GetRootItem())) 
		Expand(GetRootItem());
}


void NFGBrowseTreeCtrl::RefreshDirTree()
{
	RefreshDirItem(GetRootItem());
}

void NFGBrowseTreeCtrl::RefreshDirItem(const wxTreeItemId& item) {
	if (!item.IsOk())
		return;
	
	NFGBrowseTreeItemData *nodedata = (NFGBrowseTreeItemData *) GetItemData(item);

	if ((nodedata != NULL) && (nodedata->Type == BT_DIR)) {
		Freeze();
		SetItemHasChildren(item, ReiterateDir(nodedata->FullPath, item));
		Thaw();
	}
}



BEGIN_EVENT_TABLE(NFGMainPanel, wxPanel)
	EVT_DIRPICKER_CHANGED(wxID_ANY, NFGMainPanel::OnBrowseChangeDir)
	EVT_BUTTON(ID_RefreshBrowseTree, NFGMainPanel::OnBrowseRefresh)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, NFGMainPanel::OnBrowseOpen)
	EVT_TREE_ITEM_MENU(wxID_ANY, NFGMainPanel::OnBrowseMenu)
	EVT_MENU(wxID_OPEN, NFGMainPanel::OnContextMenuCommand)
	EVT_MENU(wxID_DELETE, NFGMainPanel::OnContextMenuCommand)
	EVT_MENU(BT_TEXTFILE, NFGMainPanel::OnContextMenuCommand)
	EVT_MENU(BT_USERLIST, NFGMainPanel::OnContextMenuCommand)
	EVT_MENU(BT_DIR, NFGMainPanel::OnContextMenuCommand)

	EVT_TEXT_ENTER(ID_SelectedStepNumber, NFGMainPanel::OnSetSelectedStep)
	EVT_TOGGLEBUTTON(ID_SelectedStepState, NFGMainPanel::OnSetSelectedStepState)
	EVT_UPDATE_UI(ID_MainNotebook, NFGMainPanel::OnUpdateUI)
	EVT_TOOL_RANGE(ID_ZoomOutH, ID_AutoZoomVAll, NFGMainPanel::OnZoom)
	EVT_LISTBOX(wxID_ANY, NFGMainPanel::OnWindowsShow)
#ifdef __NMRFilipGUI_MDI__
	EVT_BUTTON(ID_Tile, NFGMainPanel::OnWindowsCommand)
	EVT_BUTTON(ID_Cascade, NFGMainPanel::OnWindowsCommand)
#endif
END_EVENT_TABLE()

NFGMainPanel::NFGMainPanel(NFGDocManager *DocManager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxPanel(parent, id, pos, size, style)
{
	CurrentSerDocument = NULL;
	DocMan = DocManager;
	
	SelectedStep = 0;
	SelectedStepFlag = STEP_OK;
	SelectedStepLabel = wxEmptyString;
	
	xCoord = wxEmptyString;
	yCoord = wxEmptyString;
	dxCoord = wxEmptyString;
	yratioCoord = wxEmptyString;
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer(wxVERTICAL);
	
	MainNotebook = new wxNotebook(this, ID_MainNotebook);
	BrowsePanel = new wxPanel(MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxFlexGridSizer* BrowseFGSizer;
	BrowseFGSizer = new wxFlexGridSizer(2, 1, 0, 0);
	BrowseFGSizer->AddGrowableCol(0);
	BrowseFGSizer->AddGrowableRow(1);
	BrowseFGSizer->SetFlexibleDirection(wxBOTH);
	BrowseFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	wxFlexGridSizer* BrowseTopFGSizer;
	BrowseTopFGSizer = new wxFlexGridSizer(1, 2, 0, 0);
	BrowseTopFGSizer->AddGrowableCol(1);
	BrowseTopFGSizer->SetFlexibleDirection(wxHORIZONTAL );
	BrowseTopFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	BrowseRefreshButton = new wxBitmapButton(BrowsePanel, ID_RefreshBrowseTree, wxArtProvider::GetBitmap("refresh_smallicon", wxART_BUTTON, FromDIP(wxSize(16, 16))), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	BrowseRefreshButton->SetToolTip("Refresh the folder tree");
	BrowseTopFGSizer->Add(BrowseRefreshButton, 0, wxLEFT|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, FromDIP(5));

	BrowseDirPicker = new wxDirPickerCtrl(BrowsePanel, wxID_ANY, wxGetCwd(), "Select a folder", wxDefaultPosition, wxDefaultSize, /*wxDIRP_CHANGE_DIR|*/wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL|wxDIRP_SMALL);
#ifdef __WXGTK__
	BrowseDirPicker->SetPath(wxGetCwd());
#endif
	
	BrowseTopFGSizer->Add(BrowseDirPicker, 1, wxALL|wxEXPAND, FromDIP(5));

	BrowseFGSizer->Add(BrowseTopFGSizer, 1, wxALL|wxEXPAND, 0);

	
	BrowseTreeCtrl = new NFGBrowseTreeCtrl(BrowsePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | /* wxTR_HIDE_ROOT |*/ wxTR_HAS_BUTTONS /*| wxVSCROLL | wxHSCROLL*/);
	BrowseFGSizer->Add(BrowseTreeCtrl, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND|wxFIXED_MINSIZE, FromDIP(5));
	BrowseTreeCtrl->DisplayDirTree();
	
	BrowsePanel->SetSizer(BrowseFGSizer);
	BrowsePanel->Layout();
	BrowseFGSizer->Fit(BrowsePanel);
	MainNotebook->AddPage(BrowsePanel, "Browse", true);


	DisplayPanel = new wxPanel(MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxFlexGridSizer* DisplayFGSizer;
	DisplayFGSizer = new wxFlexGridSizer(4, 1, FromDIP(2), 0);
	DisplayFGSizer->SetFlexibleDirection(wxBOTH);
	DisplayFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	DisplayFGSizer->AddGrowableCol(0);
	
	wxFlexGridSizer* StepNoFGSizer;
	StepNoFGSizer = new wxFlexGridSizer(1, 4, 0, FromDIP(5));
	StepNoFGSizer->SetFlexibleDirection(wxBOTH);
	StepNoFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	StepNoFGSizer->AddGrowableCol(2);
	
	NumberLabelST = new wxStaticText(DisplayPanel, wxID_ANY, "Selected step");
	StepNoFGSizer->Add(NumberLabelST, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, FromDIP(5));
	
	NumberTextCtrl = new NFGAutoValidatedTextCtrl(DisplayPanel, ID_SelectedStepNumber, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(50,-1)), wxTE_PROCESS_ENTER, NFGLongValidator(&SelectedStep, NON_NEGATIVE_NUMBER), wxTextCtrlNameStr, false, true);
	StepNoFGSizer->Add(NumberTextCtrl, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, FromDIP(2));
	
	AssocValST = new NFGValidatedStaticText(DisplayPanel, ID_SelectedStepLabel, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(90,-1)), 0, wxGenericValidator(&SelectedStepLabel));
	AssocValST->SetToolTip("Associated value of the selected step");
	StepNoFGSizer->Add(AssocValST, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
	
	wxSize StateBmpSize = FromDIP(wxSize(16, 16));
	bmpIgnored = wxArtProvider::GetBitmap("ignored", wxART_OTHER, StateBmpSize);
	bmpBlank = wxArtProvider::GetBitmap("blank", wxART_OTHER, StateBmpSize);
	bmpNotInEnvelope = wxArtProvider::GetBitmap("nie", wxART_OTHER, StateBmpSize);
	bmpNotShown = wxArtProvider::GetBitmap("notshown", wxART_OTHER, StateBmpSize);
	bmpOk = wxArtProvider::GetBitmap("ok", wxART_OTHER, StateBmpSize);
	
	StateTgButton = new wxToggleButton(DisplayPanel, ID_SelectedStepState, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT | wxBU_NOTEXT /*| wxBORDER_NONE*/);
	StateTgButton->SetBitmapLabel(bmpOk);
	StateTgButton->SetBitmapPressed(bmpNotInEnvelope);
	StateTgButton->SetToolTip("Step is OK");
	StepNoFGSizer->Add(StateTgButton, 0, wxALIGN_CENTER_VERTICAL, 0);
	
	DisplayFGSizer->Add(StepNoFGSizer, 1, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, FromDIP(5));
	
	wxStaticBoxSizer* CursorSBSizer;
	CursorSBSizer = new wxStaticBoxSizer(new wxStaticBox(DisplayPanel, wxID_ANY, "Cursor"), wxVERTICAL);
	
	wxFlexGridSizer* CoordFGSizer;
	CoordFGSizer = new wxFlexGridSizer(2, 4, FromDIP(4), FromDIP(4));
	CoordFGSizer->SetFlexibleDirection(wxBOTH);
	CoordFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	CoordFGSizer->AddGrowableCol(1);
	CoordFGSizer->AddGrowableCol(3);
	
	xCoordLabelST = new wxStaticText(DisplayPanel, wxID_ANY, "x =");
	CoordFGSizer->Add(xCoordLabelST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);

	xCoordST = new NFGValidatedStaticText(DisplayPanel, ID_xCoordST, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(65,-1)), 0, wxGenericValidator(&xCoord));
	CoordFGSizer->Add(xCoordST, 0, wxALIGN_CENTER_VERTICAL, 0);
	
	yCoordLabelST = new wxStaticText(DisplayPanel, wxID_ANY, "y =");
	CoordFGSizer->Add(yCoordLabelST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	
	yCoordST = new NFGValidatedStaticText(DisplayPanel, ID_yCoordST, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(65,-1)), 0, wxGenericValidator(&yCoord));
	CoordFGSizer->Add(yCoordST, 0, wxALIGN_CENTER_VERTICAL, 0);
	
	dxCoordLabelST = new wxStaticText(DisplayPanel, wxID_ANY, "x-x' =");
	CoordFGSizer->Add(dxCoordLabelST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	
	dxCoordST = new NFGValidatedStaticText(DisplayPanel, ID_dxCoordST, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(65,-1)), 0, wxGenericValidator(&dxCoord));
	CoordFGSizer->Add(dxCoordST, 0, wxALIGN_CENTER_VERTICAL, 0);
	
	dyCoordLabelST = new wxStaticText(DisplayPanel, wxID_ANY, "y/y' =");
	CoordFGSizer->Add(dyCoordLabelST, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	
	yratioCoordST = new NFGValidatedStaticText(DisplayPanel, ID_yratioCoordST, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(65,-1)), 0, wxGenericValidator(&yratioCoord));
	CoordFGSizer->Add(yratioCoordST, 0, wxALIGN_CENTER_VERTICAL, 0);
	
	CursorSBSizer->Add(CoordFGSizer, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, FromDIP(4));
	
	DisplayFGSizer->Add(CursorSBSizer, 1, wxLEFT|wxRIGHT|wxEXPAND, FromDIP(5));
	
	
	wxColour col = MainNotebook->GetThemeBackgroundColour();
	if (col.IsOk()) 
		DisplayPanel->SetBackgroundColour(col);

	ZoomTBar = new wxToolBar(DisplayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | /*wxNO_BORDER |*/ wxTB_FLAT | wxTB_NODIVIDER);
	
	wxSize ToolSize = FromDIP(wxSize(24, 24));
	ZoomTBar->SetToolBitmapSize(ToolSize);

#define ZoomTBarAddTool(toolid, label, shortcut, bmpid)	\
	ZoomTBar->AddTool((toolid), label, \
		wxArtProvider::GetBitmap(#bmpid, wxART_TOOLBAR, ToolSize), \
		wxArtProvider::GetBitmap(#bmpid "_disabled", wxART_TOOLBAR, ToolSize), \
		wxITEM_NORMAL, label shortcut);
	
	ZoomTBarAddTool(ID_ZoomOutH, "Zoom out horizontally", " (Ctrl+PageDown)", unzoomh);
	ZoomTBarAddTool(ID_ZoomInH, "Zoom in horizontally", " (Ctrl+PageUp)", zoomh);
	ZoomTBarAddTool(ID_AutoZoomH, "Horizontal autozoom - selected step", "", autozoomh);
	ZoomTBarAddTool(ID_AutoZoomHAll, "Horizontal autozoom - all steps", "", autozoomh2);
	
	ZoomTBar->AddSeparator();

	ZoomTBarAddTool(ID_ZoomOutV, "Zoom out vertically", " (Shift+PageDown)", unzoomv);
	ZoomTBarAddTool(ID_ZoomInV, "Zoom in vertically", " (Shift+PageUp)", zoomv);
	ZoomTBarAddTool(ID_AutoZoomV, "Vertical autozoom - selected step", "", autozoomv);
	ZoomTBarAddTool(ID_AutoZoomVAll, "Vertical autozoom - all steps", "", autozoomv2);
	
#undef ZoomTBarAddTool

	ZoomTBar->Realize();
	
	if (col.IsOk()) 
		ZoomTBar->SetBackgroundColour(col);

	DisplayFGSizer->Add(ZoomTBar, 1, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(5));
	

	DisplayInnerPanel = new NFGDisplayInnerPanel(DocMan, DisplayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	DisplayFGSizer->Add(DisplayInnerPanel, 1, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, FromDIP(5));
	
	DisplayPanel->SetSizer(DisplayFGSizer);
	DisplayPanel->Layout();
	DisplayFGSizer->Fit(DisplayPanel);
	MainNotebook->AddPage(DisplayPanel, "Display", false);


	WindowsPanel = new wxPanel(MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxFlexGridSizer* WindowsFGSizer;
	WindowsFGSizer = new wxFlexGridSizer(2, 1, 0, 0);
	WindowsFGSizer->AddGrowableCol(0);
	WindowsFGSizer->AddGrowableRow(0);
	WindowsFGSizer->SetFlexibleDirection(wxBOTH);
	WindowsFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	WindowsListBox = new wxListBox(WindowsPanel, ID_WindowsListBox, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_HSCROLL | wxLB_NEEDED_SB); 
	if (wxDynamicCast(GetParent(), NMRFilipGUIFrame))
		wxDynamicCast(GetParent(), NMRFilipGUIFrame)->SetWindowsListBox(WindowsListBox);

	WindowsFGSizer->Add(WindowsListBox, 1, wxALL|wxEXPAND, FromDIP(5));
	
#ifdef __NMRFilipGUI_MDI__
	wxFlexGridSizer* WindowsCmdFGSizer;
	WindowsCmdFGSizer = new wxFlexGridSizer(2, 2, 0, 0);
	WindowsCmdFGSizer->AddGrowableCol(0);
	WindowsCmdFGSizer->AddGrowableCol(1);
	WindowsCmdFGSizer->SetFlexibleDirection(wxBOTH);
	WindowsCmdFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	
	WindowsTileButton = new wxButton(WindowsPanel, ID_Tile, "Tile");
	WindowsCmdFGSizer->Add(WindowsTileButton, 0, wxALL|wxEXPAND, FromDIP(5));
	
	WindowsCascadeButton = new wxButton(WindowsPanel, ID_Cascade, "Cascade");
	WindowsCmdFGSizer->Add(WindowsCascadeButton, 0, wxALL|wxEXPAND, FromDIP(5));
	
	WindowsFGSizer->Add(WindowsCmdFGSizer, 1, wxEXPAND, FromDIP(5));
#endif

	WindowsPanel->SetSizer(WindowsFGSizer);
	WindowsPanel->Layout();
	WindowsFGSizer->Fit(WindowsPanel);
	MainNotebook->AddPage(WindowsPanel, "Windows", false);


	HintsPanel = new wxPanel(MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxFlexGridSizer* HintsFGSizer;
	HintsFGSizer = new wxFlexGridSizer(1, 1, 0, 0);
	HintsFGSizer->AddGrowableCol(0);
	HintsFGSizer->AddGrowableRow(0);
	HintsFGSizer->SetFlexibleDirection(wxBOTH);
	HintsFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	HintsTextCtrl = new wxTextCtrl(HintsPanel, wxID_ANY, HintsText, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTE_READONLY | wxTE_MULTILINE);
	HintsFGSizer->Add(HintsTextCtrl, 1, wxALL|wxEXPAND, FromDIP(5));

	HintsPanel->SetSizer(HintsFGSizer);
	HintsPanel->Layout();
	HintsFGSizer->Fit(HintsPanel);

	MainNotebook->AddPage(HintsPanel, "Hints", false);
	
	
	AboutPanel = new wxPanel(MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxFlexGridSizer* AboutFGSizer;
	AboutFGSizer = new wxFlexGridSizer(1, 1, 0, 0);
	AboutFGSizer->AddGrowableCol(0);
	AboutFGSizer->AddGrowableRow(0);
	AboutFGSizer->SetFlexibleDirection(wxBOTH);
	AboutFGSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	AboutTextCtrl = new wxTextCtrl(AboutPanel, wxID_ANY, AboutText, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTE_READONLY | wxTE_MULTILINE);
	AboutFGSizer->Add(AboutTextCtrl, 1, wxALL|wxEXPAND, FromDIP(5));

	AboutPanel->SetSizer(AboutFGSizer);
	AboutPanel->Layout();
	AboutFGSizer->Fit(AboutPanel);

	MainNotebook->AddPage(AboutPanel, "About", false);


	bSizer13->Add(MainNotebook, 1, wxEXPAND | wxALL, FromDIP(5));
	
	this->SetSizer(bSizer13);
	this->Layout();
	bSizer13->Fit(this);
	
	/// will be enabled by OnUpdateUI() handler if needed
	DisplayPanel->Disable();
	

	ContextMenuFile = new wxMenu;
	ContextMenuFile->Append(wxID_OPEN, "Open");
	ContextMenuFile->AppendSeparator();
	ContextMenuFile->Append(wxID_DELETE, "Delete");
	
	ContextMenuDir = new wxMenu;
	ContextMenuDir->Append(BT_TEXTFILE, "New text file");
	ContextMenuDir->Append(BT_USERLIST, "New ulist file");
	ContextMenuDir->Append(BT_DIR, "New directory");
	ContextMenuDir->AppendSeparator();
	ContextMenuDir->Append(wxID_DELETE, "Delete");

}

NFGMainPanel::~NFGMainPanel()
{
	if (wxDynamicCast(GetParent(), NMRFilipGUIFrame))
		wxDynamicCast(GetParent(), NMRFilipGUIFrame)->SetWindowsListBox(NULL);
	
	delete ContextMenuFile;
	delete ContextMenuDir;
}

void NFGMainPanel::OnBrowseChangeDir(wxFileDirPickerEvent& event)
{ 
	wxString path = event.GetPath();
	if (wxFileName::IsDirReadable(path)) {
		wxSetWorkingDirectory(path);
		BrowseTreeCtrl->DisplayDirTree();
	}
	
	event.Skip(); 
}

void NFGMainPanel::OnBrowseRefresh(wxCommandEvent& event)
{
	BrowseTreeCtrl->RefreshDirTree();
}

void NFGMainPanel::OnBrowseOpen(wxTreeEvent& event)
{
	if (DocMan == NULL)
		return;
	
	wxTreeItemId node = event.GetItem();
	if (!(node.IsOk()))
		return;
	
	NFGBrowseTreeItemData *nodedata = (NFGBrowseTreeItemData *) BrowseTreeCtrl->GetItemData(node);
	if (nodedata == NULL)
		return;
	
	switch (nodedata->Type) {
		case BT_SERFILE:
		case BT_TDDFILE:
		case BT_USERLIST:
		case BT_TEXTFILE:
			DocMan->CreateDocument(nodedata->FullPath, wxDOC_SILENT);
			break;
		
		default:
			;
	}

	event.Skip(); 
}

void NFGMainPanel::OnBrowseMenu(wxTreeEvent& event)
{
	if (DocMan == NULL)
		return;
	
	CMItem = event.GetItem();
	if (!(CMItem.IsOk()))
		return;

	NFGBrowseTreeItemData *nodedata = (NFGBrowseTreeItemData *) BrowseTreeCtrl->GetItemData(CMItem);
	if (nodedata == NULL)
		return;
	
	switch (nodedata->Type) {
		case BT_SERFILE:
		case BT_TDDFILE:
		case BT_USERLIST:
		case BT_TEXTFILE:
			BrowseTreeCtrl->PopupMenu(ContextMenuFile);
			break;
		
		case BT_DIR:
			BrowseTreeCtrl->PopupMenu(ContextMenuDir);
			break;
		
		default:
			;
	}
}

void NFGMainPanel::OnContextMenuCommand(wxCommandEvent& event)
{
	if (DocMan == NULL)
		return;
	
	if (!(CMItem.IsOk()))
		return;

	NFGBrowseTreeItemData *nodedata = (NFGBrowseTreeItemData *) BrowseTreeCtrl->GetItemData(CMItem);
	if (nodedata == NULL)
		return;

	switch(event.GetId()) {
		case wxID_OPEN:
			switch (nodedata->Type) {
				case BT_SERFILE:
				case BT_TDDFILE:
				case BT_USERLIST:
				case BT_TEXTFILE:
					DocMan->CreateDocument(nodedata->FullPath, wxDOC_SILENT);
					break;
				
				default:
					break;
			}
			break;
		
		case wxID_DELETE:
			switch (nodedata->Type) {
				case BT_SERFILE:
				case BT_TDDFILE:
				case BT_USERLIST:
				case BT_TEXTFILE:
					if (DocMan->FindDocumentByPath(nodedata->FullPath) != NULL) {
						::wxMessageBox("The file \'" + nodedata->FullPath + "\' cannot be deleted because it is open.", "Error", wxOK | wxICON_ERROR);
						return;
					}
					
					/// ask for confirmation
					if (::wxMessageBox("Do you really want to delete the file \'" + nodedata->FullPath + "\'?", "Confirm", wxYES_NO | wxICON_QUESTION) != wxYES) 
						return;
				
					if (wxRemoveFile(nodedata->FullPath)) 
						BrowseTreeCtrl->RefreshDirItem(BrowseTreeCtrl->GetItemParent(CMItem));
					else 
						::wxMessageBox("Deleting the file \'" + nodedata->FullPath + "\' failed.", "Error", wxOK | wxICON_ERROR);
					
					break;
				
				case BT_DIR:
					if (CMItem == BrowseTreeCtrl->GetRootItem()) {
						::wxMessageBox("Current working directory cannot be deleted.", "Error", wxOK | wxICON_ERROR);
						return;
					}
					
					for (wxList::const_iterator iter = DocMan->GetDocuments().begin(); iter != DocMan->GetDocuments().end(); ++iter) {
						wxDocument * const doc = wxStaticCast(*iter, wxDocument);
						if (wxFileName(doc->GetFilename()).GetFullPath().StartsWith(nodedata->FullPath)) {
							::wxMessageBox("The directory \'" + nodedata->FullPath + "\' cannot be deleted because a file contained in the directory is open.", "Error", wxOK | wxICON_ERROR);
							return;
						}
					}

					/// ask for confirmation
					if (::wxMessageBox("Do you really want to delete the directory \'" + nodedata->FullPath + "\'?", "Confirm", wxYES_NO | wxICON_QUESTION) != wxYES) 
						return;
				
					
					if (!wxFileName::Rmdir(nodedata->FullPath, wxPATH_RMDIR_RECURSIVE)) 
						::wxMessageBox("Deleting the directory \'" + nodedata->FullPath + "\' failed.", "Error", wxOK | wxICON_ERROR);
					
					BrowseTreeCtrl->RefreshDirItem(BrowseTreeCtrl->GetItemParent(CMItem));
					
					break;
					
				default:
					break;
			}
			break;

		case BT_TEXTFILE:
			if (nodedata->Type == BT_DIR) {
				if (!wxFileName::IsDirWritable(nodedata->FullPath)) {
					::wxMessageBox("The directory \'" + nodedata->FullPath + "\' is not writable.", "Error", wxOK | wxICON_ERROR);
					return;
				}

				/// ask for name
				wxString name = ::wxGetTextFromUser("Enter the name of the new text file:", "File name");
				if (name.IsEmpty())
					return;
				
				wxString forbidden = wxFileName::GetForbiddenChars();
				for (wxString::const_iterator iter = forbidden.begin(); iter != forbidden.end(); ++iter) {
					wxUniChar c = (wxUniChar) *iter;
					if (name.Find(c) != wxNOT_FOUND) {
						::wxMessageBox("The name of the new file is invalid.", "Error", wxOK | wxICON_ERROR);
						return;
					}
				}
				
				wxFileName FName(nodedata->FullPath, name);
				if (!FName.HasExt())
					FName.SetExt("txt");
				
				if (FName.GetExt() != "txt") {
					::wxMessageBox("Only \'txt\' extension is supported.", "Error", wxOK | wxICON_ERROR);
					return;
				}
				
				if (FName.Exists()) {
					::wxMessageBox("The directory \'" + nodedata->FullPath + "\' already contains \'" + name + "\'.", "Error", wxOK | wxICON_ERROR);
					return;
				}

				wxDocument *doc = DocMan->CreateDocument(FName.GetFullPath(), wxDOC_NEW | wxDOC_SILENT);
				if (doc != NULL) {
					BrowseTreeCtrl->RefreshDirItem(CMItem);
					if (BrowseTreeCtrl->ItemHasChildren(CMItem) && !BrowseTreeCtrl->IsExpanded(CMItem)) 
						BrowseTreeCtrl->Expand(CMItem);
				} else
					::wxMessageBox("Creating the file \'" + name + "\' in the directory \'" + nodedata->FullPath + "\' failed.", "Error", wxOK | wxICON_ERROR);
					
			}
			break;
			
		case BT_USERLIST:
			if (nodedata->Type == BT_DIR) {
				if (!wxFileName::IsDirWritable(nodedata->FullPath)) {
					::wxMessageBox("The directory \'" + nodedata->FullPath + "\' is not writable.", "Error", wxOK | wxICON_ERROR);
					return;
				}

				wxFileName FName(nodedata->FullPath, "ulist");
				if (FName.Exists()) {
					::wxMessageBox("The directory \'" + nodedata->FullPath + "\' already contains ulist.", "Error", wxOK | wxICON_ERROR);
					return;
				}

				if (DocMan->CreateDocument(FName.GetFullPath(), wxDOC_NEW | wxDOC_SILENT) != NULL) {
					BrowseTreeCtrl->RefreshDirItem(CMItem);
					if (BrowseTreeCtrl->ItemHasChildren(CMItem) && !BrowseTreeCtrl->IsExpanded(CMItem)) 
						BrowseTreeCtrl->Expand(CMItem);
				} else
					::wxMessageBox("Creating ulist in the directory \'" + nodedata->FullPath + "\' failed.", "Error", wxOK | wxICON_ERROR);

			}
			break;
			
		case BT_DIR:
			if (nodedata->Type == BT_DIR) {
				if (!wxFileName::IsDirWritable(nodedata->FullPath)) {
					::wxMessageBox("The directory \'" + nodedata->FullPath + "\' is not writable.", "Error", wxOK | wxICON_ERROR);
					return;
				}

				/// ask for name
				wxString name = ::wxGetTextFromUser("Enter the name of the new directory:", "Directory name");
				if (name.IsEmpty())
					return;
				
				wxString forbidden = wxFileName::GetForbiddenChars();
				for (wxString::const_iterator iter = forbidden.begin(); iter != forbidden.end(); ++iter) {
					wxUniChar c = (wxUniChar) *iter;
					if (name.Find(c) != wxNOT_FOUND) {
						::wxMessageBox("The name of the new directory is invalid.", "Error", wxOK | wxICON_ERROR);
						return;
					}
				}
				
				wxFileName DName = wxFileName::DirName(nodedata->FullPath);
				if (!DName.AppendDir(name)) {
					::wxMessageBox("The name of the new directory is invalid.", "Error", wxOK | wxICON_ERROR);
					return;
				}
				
				if (DName.Exists()) {
					::wxMessageBox("The directory \'" + nodedata->FullPath + "\' already contains \'" + name + "\'.", "Error", wxOK | wxICON_ERROR);
					return;
				}
				
				if (DName.Mkdir()) {
					BrowseTreeCtrl->RefreshDirItem(CMItem);
					if (BrowseTreeCtrl->ItemHasChildren(CMItem) && !BrowseTreeCtrl->IsExpanded(CMItem)) 
						BrowseTreeCtrl->Expand(CMItem);
				} else
					::wxMessageBox("Creating the directory \'" + name + "\' in the directory \'" + nodedata->FullPath + "\' failed.", "Error", wxOK | wxICON_ERROR);

			}
			break;

		default:
			;
	}
}

void NFGMainPanel::OnSetSelectedStep(wxCommandEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if (SerDoc == NULL)
		return;
	
	wxValidator *validator = NumberTextCtrl->GetValidator();
	wxWindow *parent = NumberTextCtrl->GetParent();

	if (validator) {
		if (!(validator->Validate(parent)))
			validator->TransferToWindow();
		else {
 			validator->TransferFromWindow();
			
			/// Select the step
			unsigned long NewSelectedStep = SerDoc->SetSelectedStep(SelectedStep);
			/// If the actual selected step does not change, OnUpdateUI() does not adjust the value in NumberTextCtrl.
			if (NewSelectedStep != (unsigned long) SelectedStep) {
				SelectedStep = NewSelectedStep;
				validator->TransferToWindow();
			}
		}
	}
}

void NFGMainPanel::OnSetSelectedStepState(wxCommandEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if (SerDoc == NULL)
		return;

	SerDoc->StepNoFFTEnvelopeCommand(SelectedStep, event.IsChecked(), false);
}

void NFGMainPanel::OnZoom(wxCommandEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	if (SerDoc == NULL)
		return;
	
	SerDoc->ZoomCommand(event.GetId());
}

void NFGMainPanel::OnWindowsShow(wxCommandEvent& event)
{
	int n = WindowsListBox->GetSelection();
	if (n != wxNOT_FOUND) {
		wxDocDIChildFrame *window = (wxDocDIChildFrame *) WindowsListBox->GetClientData(n);
#ifdef __NMRFilipGUI_MDI__
		window->Activate();
#else
		window->Raise();
#endif
		if (window->IsIconized())
			window->Iconize(false);
	}
}

#ifdef __NMRFilipGUI_MDI__
void NFGMainPanel::OnWindowsCommand(wxCommandEvent& event)
{
	wxDocDIParentFrame *DIParent = (wxDocDIParentFrame *) GetParent();
	if (DIParent == NULL)
		return;
		
	if (event.GetId() == ID_Tile)
		DIParent->Tile();
	else 
	if (event.GetId() == ID_Cascade)
		DIParent->Cascade();
}
#endif	


void NFGMainPanel::OnUpdateUI(wxUpdateUIEvent& event)
{
	if (DocMan == NULL)
		return;
	
	NFGSerDocument* SerDoc = wxDynamicCast(DocMan->GetCurrentDocument(), NFGSerDocument);

	NFGSerView* view = wxDynamicCast(DocMan->GetCurrentView(), NFGSerView);
	
	if ((SerDoc == NULL) || (view == NULL)) {
		CurrentSerDocument = NULL;
		DisplayPanel->Disable();
		return;
	}
	
	DisplayPanel->Enable();


	NFGGraphWindow* GW = wxDynamicCast(wxWindow::FindFocus(), NFGGraphWindow);
	if (GW != NULL)
		if (GW->WasFocusedByUser() && (MainNotebook->GetSelection() != 1)) {	/// show Display panel if some NFGGraphWindow recently obtained focus by click in the graph area
			MainNotebook->SetSelection(1);
			GW->SetFocus();
		}	

	if (SerDoc->SelectedStepChangedQuery() || (SerDoc != CurrentSerDocument)) {
		wxWindow *parent = NumberTextCtrl->GetParent();
		
		SelectedStep = SerDoc->GetSelectedStep();
		SelectedStepLabel = SerDoc->GetSelectedStepLabel();
		
		if (SelectedStepFlag != SerDoc->GetSelectedStepStateFlag()) {
			SelectedStepFlag = SerDoc->GetSelectedStepStateFlag();
			StateTgButton->SetToolTip("Step is " + SerDoc->GetSelectedStepState());
			
			bool StateTgButtonPressed = false;
			switch (SelectedStepFlag) {
				case STEP_IGNORE:
					StateTgButton->SetBitmapLabel(bmpIgnored);
					break;
				
				case STEP_BLANK:
					StateTgButton->SetBitmapLabel(bmpBlank);
					break;
					
				case STEP_NO_ENVELOPE:
					StateTgButtonPressed = true;
					break;
					
				case STEP_NO_SHOW:
					StateTgButton->SetBitmapLabel(bmpNotShown);
					break;
					
				case STEP_OK:
				default:
					StateTgButton->SetBitmapLabel(bmpOk);
			}
			
			StateTgButton->SetValue(StateTgButtonPressed);
#ifdef __WXGTK__
			/// ensure the update of the bitmap on the button
			if (StateTgButtonPressed) 
				StateTgButton->GTKPressed();
			else 
				StateTgButton->GTKReleased();
#endif
		}
		
		if (parent)
			parent->TransferDataToWindow();
	}
	
	CurrentSerDocument = SerDoc;

	wxValidator* val = NULL;
	val = xCoordST->GetValidator();
	if (val) {
		xCoord = SerDoc->CursorXQuery();
		val->TransferToWindow();
	}

	val = yCoordST->GetValidator();
	if (val) {
		yCoord = SerDoc->CursorYQuery();
		val->TransferToWindow();
	}

	val = dxCoordST->GetValidator();
	if (val) {
		dxCoord = SerDoc->CursorDeltaXQuery();
		val->TransferToWindow();
	}

	val = yratioCoordST->GetValidator();
	if (val) {
		yratioCoord = SerDoc->CursorYRatioQuery();
		val->TransferToWindow();
	}
}


const wxString NFGMainPanel::AboutText = "\
NMRFilip GUI - the NMR data processing software - graphical user interface\n\
Copyright (C) 2010, 2011, 2020 Richard Reznicek\n\
\n\
This program is free software; you can redistribute it and/or \
modify it under the terms of the GNU General Public License \
as published by the Free Software Foundation; either version 2 \
of the License, or (at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License \
along with this program; if not, write to the Free Software \
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n\
\n\
----------------------------------------\n\
\n\
This program is based on the NMRFilip LIB core library (Copyright (C) 2010, 2011, 2020 \
Richard Reznicek) licensed under the terms of the GNU General Public License \
version 2 or any later version.\n\
\n\
----------------------------------------\n\
\n\
The NMRFilip LIB core library uses the FFTW library (http://www.fftw.org) \
(Copyright (c) 2003, 2007-14 Matteo Frigo, Copyright (c) 2003, 2007-14 Massachusetts \
Institute of Technology) licensed under the terms of the GNU General Public \
License version 2 or any later version.\n\
\n\
----------------------------------------\n\
\n\
The NMRFilip GUI program uses the wxWidgets library (http://www.wxwidgets.org/) \
(Copyright (c) 1992-2018 Julian Smart, Vadim Zeitlin, Stefan Csomor, Robert Roebling, \
and other members of the wxWidgets team, \
Portions (c) 1996 Artificial Intelligence Applications Institute) \
licensed under the terms of the wxWindows Library License.\
";


const wxString NFGMainPanel::HintsText = "\
=== General ===\n\
\n"
"All indices used in the program start from zero (e.g. let each step contain 8 chunks, then the index of the first chunk is 0 and it is 7 for the last one).\n\
\n\
Mouse wheel (as well as PageUp/PageDown and up/down arrow keys) can be used to gradually increase or decrease the parameters in the fields with spin button. This is usefull especially in combination with the Auto-Apply function.\n\
\n"
#ifdef __NMRFilipGUI_SDI__
"The active child window has a bright icon and a title enclosed between the symbols > and <. \n\
\n\
Running the NMRFilip GUI on a dedicated desktop helps to avoid a clutter of numerous windows.\n\
\n"
#endif
"The content of a directory is scanned every time it is expanded in the Browse panel. The whole displayed folder tree can be refreshed by the designated button. \n\
\n\
Directories, ulist and text files can be created and deleted using context menu (accessible by a secondary mouse button click) in the Browse panel.\n\
\n\
Text file edited in the built-in editor can be saved using context menu (accessible by a secondary mouse button click).\n\
\n\
=== Plots ===\n\
\n\
A primary mouse button click in the graph area of ser or fid child window shows the Display panel.\n\
\n\
Particular step to display or highlight can be selected by entering its index in the corresponding field in the Display panel (and confirming by Enter) or from the plot using mouse wheel or PageUp/PageDown keys.\n\
\n\
Zoom is controled by mouse wheel (and PageUp/PageDown keys) in combination with Ctrl (horizontal zoom) and Shift (vertical zoom) keys, in addition to the designated buttons in the Display panel.\n\
\n"
"Scrolling is operated by scrollbars, by arrow keys or by dragging using the mouse middle button.\n\
\n\
The plots are by default horizontally constrained within the x-range of valid data. These constraints can be overriden by manual adjustment of the horizontal limits beyond the valid data x-range using the Plot panel. The new limits remain in effect until horizontal autoscale is used or until subsequent manual setting of horizontal axis range takes place.\n\
\n\
If the plotted data are by their general nature non-negative, the lower limit of the vertical range of the plot is by default constrained to zero. This constraint can be turned off by manual adjustment of the vertical minimum to negative value using the Plot panel. Vertical autoscale or subsequent manual setting of vertical minimum to non-negative value turn the constraint on again.\n\
\n\
The settings (axis ranges etc.) of each plot are independent from other plots of the same or any other ser of fid file. The settings can be easily duplicated in whole or in part using the Store and Load functionality of the Plot panel as desired.\n\
\n\
The reference point [x',y'], coordinates of which are used in the Cursor box in the Display panel, can be set by a secondary mouse button click and removed by a primary mouse button click in the graph area.\n\
\n\
The cursor coordinates can be copied to the clipboard by primary mouse button click combined with Ctrl key. The coordinates can be appended to previously copied values by Ctrl + Shift + primary mouse button click.\n\
\n\
=== Offset correction ===\n\
\n\
Two offset correction methods intended for FID(-like) signals are implemented. The Remove offset checkbox in the Phase panel allows for a simple offset correction taking into account the assumed position of the FID start, which is used for the first order phase correction. If no first order phase correction is applied, the Remove offset functionality is equivalent to scaling the first chunk point by 0.5 in the Analyze panel. \n\
\n\
=== Processing parameters ===\n\
\n\
A set of parameters used for processing some particular data can be easily saved and reloaded using the Views panel. \n\
\n\
Exported text data files contain the set of processing parameters in their header, thus when the raw data are opened, they can be processed again using the same parameters just by loading the corresponding exported text data file from the Views panel either directly or using the Load other... button.\
";
