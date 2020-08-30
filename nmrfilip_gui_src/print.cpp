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

#include "print.h"

#include "doc.h"
#include "plotwin.h"
#ifdef __WXGTK__
#include "nfgps.h"
#endif


IMPLEMENT_DYNAMIC_CLASS(NFGPrintout, wxPrintout)

NFGPrintout::NFGPrintout(const wxString& title, NFGGraph* graph2set, PrintRequest* req2set, AcquParams* Aparams2set, ProcParams* Pparams2set, wxString path2set) : wxPrintout(title)
{
	Graph = graph2set;
	request = req2set;
	AParams = Aparams2set;
	PParams = Pparams2set;
	Path = path2set;
}

bool NFGPrintout::OnPrintPage(int page)
{
	wxDC *dc = GetDC();

	if (dc == NULL)	/// is also checked in NFGGraphRenderer::RenderGraph()
		return false;

	if (!dc->IsOk())	/// is also checked in NFGGraphRenderer::RenderGraph()
		return false;

	int w, h;
	GetPPIPrinter(&w, &h);
	
#ifdef __WXMSW__
	/// Unlike to wxW 2.8.11, wxW 3.1.2 sets screen PPI of a printout by wxGetDisplayPPI() based on mm size from ::GetDeviceCaps(dc, HORZSIZE), ::GetDeviceCaps(dc, VERTSIZE) calls and on geometry from :GetDeviceCaps(dc, HORZRES), ::GetDeviceCaps(dc, VERTRES)) calls. Under certain circumstances, this can unfortunately yield different PPI than the call to ::GetDeviceCaps(ScreenHDC(), LOGPIXELSY) used to calculate pixel size of a font (or wxScreenDC().GetPPI()).
//	int hs = ::GetDeviceCaps(ScreenHDC(), LOGPIXELSY);
	int hs = wxScreenDC().GetPPI().GetHeight();
#else
	int ws, hs;
	GetPPIScreen(&ws, &hs);
#endif
	
	int wp, hp, wd, hd;
	GetPageSizePixels(&wp, &hp);
	dc->GetSize(&wd, &hd);
	/// reduce the DPI accordingly if this is just a preview
	w = ((long long) w)*wd/wp;
	h = ((long long) h)*hd/hp;
	
	bool RetVal = NFGGraphRenderer::RenderGraph(Graph, dc, AParams, PParams, GetTitle(), Path, request->point_size, h, hs, true, 
								request->black_and_white, request->params_and_key_at_right, 
								request->print_params, request->print_title, request->print_key, wxFONTFAMILY_ROMAN);
		
	return RetVal;
}

bool NFGPrintout::OnBeginDocument(int startPage, int endPage)
{
	if (!wxPrintout::OnBeginDocument(startPage, endPage))
		return false;

	return true;
}

void NFGPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
	*minPage = 1;
	*maxPage = 1;
	*selPageFrom = 1;
	*selPageTo = 1;
}

bool NFGPrintout::HasPage(int pageNum)
{
	return (pageNum == 1);
}

#ifdef __WXGTK__
IMPLEMENT_CLASS(NFGPrinter, wxPrinterBase)

NFGPrinter::NFGPrinter(wxPrintDialogData *data) : wxPrinterBase(data)
{
}

NFGPrinter::~NFGPrinter()
{
}

bool NFGPrinter::Setup(wxWindow *parent)
{
	return false;
}

bool NFGPrinter::Print(wxWindow *parent, wxPrintout *printout, bool prompt)
{
	sm_lastError = wxPRINTER_NO_ERROR;

	if (!printout) {
		sm_lastError = wxPRINTER_ERROR;
		return false;
	}
	
	if (m_printDialogData.GetMinPage() < 1)
		m_printDialogData.SetMinPage(1);
	
	if (m_printDialogData.GetMaxPage() < 1)
		m_printDialogData.SetMaxPage(1000);
	
	NFGPSDC *dc;
	if (prompt) {
		dc = wxDynamicCast(PrintDialog(parent), NFGPSDC);
		if (!dc)
			return false;
	} else {
		wxPrintData pData = m_printDialogData.GetPrintData();
		pData.SetPaperId(wxPAPER_A4);
		m_printDialogData.SetPrintData(pData);
		
		m_printDialogData.SetNoCopies(1);
		m_printDialogData.SetFromPage(m_printDialogData.GetMinPage());
		m_printDialogData.SetToPage(m_printDialogData.GetMaxPage());
		
		wxPrintPaperDatabase* ppd = new wxPrintPaperDatabase();
		ppd->CreateDatabase();
		wxSize paper_size = ppd->GetSize(pData.GetPaperId());
		paper_size.Scale(0.1, 0.1);
		delete ppd;
		
		dc = new NFGPSDC(paper_size, pData.GetOrientation() == wxLANDSCAPE, false, 4*72);
	}
	
	if (!dc) {
		sm_lastError = wxPRINTER_ERROR;
		return false;
	}

	if (!dc->IsOk()) {
		delete dc;
		sm_lastError = wxPRINTER_ERROR;
		return false;
	}
	
	printout->SetPPIScreen(72, 72);
	printout->SetPPIPrinter(300, 300);
	
	printout->SetDC(dc);
	
	int mw, mh;
	dc->GetSizeMM(&mw, &mh);
	printout->SetPageSizeMM(mw, mh);
	
	int w, h;
	dc->GetSize(&w, &h);
	printout->SetPageSizePixels(w, h);
	printout->SetPaperRectPixels(wxRect(0, 0, w, h));
	
	printout->OnPreparePrinting();
	printout->OnBeginPrinting();
	
	sm_lastError = wxPRINTER_NO_ERROR;
	
	for (int copy = 1; copy <= m_printDialogData.GetNoCopies(); copy++) {
		if (!printout->OnBeginDocument(m_printDialogData.GetFromPage(), m_printDialogData.GetToPage())) {
			sm_lastError = wxPRINTER_ERROR;
			break;
		}
	
		for (int page = m_printDialogData.GetFromPage(); (page <= m_printDialogData.GetToPage()) && printout->HasPage(page); page++) {
			dc->StartPage();
			printout->OnPrintPage(page);
			dc->EndPage();
		}
		
		printout->OnEndDocument();
	}
	
	printout->OnEndPrinting();
	
	wxProcess* PrintProc = new wxProcess(wxPROCESS_REDIRECT);
	
	if (wxExecute("lpr", wxEXEC_ASYNC, PrintProc) == 0) {
		sm_lastError = wxPRINTER_ERROR;
		delete PrintProc;
	} else {
		wxOutputStream* PrintStream = PrintProc->GetOutputStream();
		dc->WriteToStream(PrintStream);
		PrintProc->CloseOutput();
		PrintProc->Detach();
	}
	
	delete dc;
	
	return (sm_lastError == wxPRINTER_NO_ERROR);
}

wxDC* NFGPrinter::PrintDialog(wxWindow *parent)
{
	wxDC* dc = NULL;

	if (wxMessageBox("Current plot is going to be sent to default printer with A4 page layout.", "Print", wxOK | wxCANCEL, parent) == wxOK) {
		wxPrintData pData = m_printDialogData.GetPrintData();
		pData.SetPaperId(wxPAPER_A4);
		m_printDialogData.SetPrintData(pData);
	
		m_printDialogData.SetNoCopies(1);
		m_printDialogData.SetFromPage(m_printDialogData.GetMinPage());
		m_printDialogData.SetToPage(m_printDialogData.GetMaxPage());
	
		wxPrintPaperDatabase* ppd = new wxPrintPaperDatabase();
		ppd->CreateDatabase();
		wxSize paper_size = ppd->GetSize(pData.GetPaperId());
		paper_size.Scale(0.1, 0.1);
		delete ppd;
	
		dc = new NFGPSDC(paper_size, pData.GetOrientation() == wxLANDSCAPE, false, 4*72);
	
		if (dc)
			sm_lastError = wxPRINTER_NO_ERROR;
		else
			sm_lastError = wxPRINTER_ERROR;
	} else
        	sm_lastError = wxPRINTER_CANCELLED;

    return dc;
}
#endif
