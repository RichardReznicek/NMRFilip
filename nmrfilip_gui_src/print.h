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

#ifndef __print__
#define __print__

#include "wx_pch.h"

#include <wx/print.h>
#include <wx/paper.h>
#include <wx/progdlg.h>
#include <wx/generic/prntdlgg.h>
#include <wx/process.h>

#include "cd.h"
#include "print_cd.h"

#include "doc_cd.h"
#include "nmrdata_cd.h"
#include "plotgen_cd.h"


class NFGPrintout : public wxPrintout
{
	DECLARE_DYNAMIC_CLASS(NFGPrintout)
	
	private:
		NFGGraph* Graph;
		PrintRequest* request;
		AcquParams* AParams;
		ProcParams* PParams;
		wxString Path;
	
	public:
		NFGPrintout(const wxString& title = "NMRFilip GUI printout", NFGGraph* graph2set = NULL, PrintRequest* req2set = NULL, AcquParams* Aparams2set = NULL, ProcParams* Pparams2set = NULL, wxString path2set = wxEmptyString);
		bool OnPrintPage(int page);
		bool HasPage(int page);
		bool OnBeginDocument(int startPage, int endPage);
		void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
			
		void SetGraph(NFGGraph* graph2set) { Graph = graph2set; }
		NFGGraph* GetGraph() { return Graph; }
		
		void SetPrintRequest(PrintRequest* req2set) { request = req2set; }
		PrintRequest* GetPrintRequest() { return request; }

		void SetAcquParams(AcquParams* params2set) { AParams = params2set; }
		AcquParams* GetAcquParams() { return AParams; }

		void SetProcParams(ProcParams* params2set) { PParams = params2set; }
		ProcParams* GetProcParams() { return PParams; }

		void SetPath(wxString path2set) { Path = path2set; }
		wxString GetPath() { return Path; }
};


#ifdef __WXGTK__
class NFGPrinter : public wxPrinterBase
{
	public:
		NFGPrinter(wxPrintDialogData *data = (wxPrintDialogData *) NULL);
		virtual ~NFGPrinter();

		virtual bool Setup(wxWindow *parent);
		virtual bool Print(wxWindow *parent, wxPrintout *printout, bool prompt = true);
		virtual wxDC* PrintDialog(wxWindow *parent);

	private:
		DECLARE_CLASS(NFGPrinter)
		DECLARE_NO_COPY_CLASS(NFGPrinter)
};
#else
#define NFGPrinter	wxPrinter
#endif

#endif
