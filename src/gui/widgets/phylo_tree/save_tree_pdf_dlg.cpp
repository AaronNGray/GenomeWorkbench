
#include <ncbi_pch.hpp>
#include "save_tree_pdf_dlg.hpp"
#include <gui/widgets/phylo_tree/phylo_tree_pane.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <gui/widgets/phylo_tree/phylo_pdf_grabber.hpp>
//#include <gui/widgets/wx/ind_progress_bar.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/print/pdf.hpp>

#include <corelib/ncbifile.hpp>

#include <wx/utils.h>
#include <wx/msgdlg.h>
//#include "wx/slider.h"

//#if wxUSE_STATLINE
//    #include "wx/statline.h"
//#endif

#include <stdlib.h>
#include <string.h>

// ----------------------------------------------------------------------------
// CSaveTreePdfDlg
// ----------------------------------------------------------------------------

BEGIN_NCBI_SCOPE

IMPLEMENT_CLASS(CSaveTreePdfDlg, CwxSavePdfBaseDlg)

BEGIN_EVENT_TABLE(CSaveTreePdfDlg, CwxSavePdfBaseDlg)
    //EVT_BUTTON(ePrintIDSetup, CSaveTreePdfDlg::OnPrinter)
    //EVT_SCROLL(CSaveTreePdfDlg::OnScroll)
END_EVENT_TABLE()

static const string kSavePdfBaseKey("SavePdfTreeDialog");

CSaveTreePdfDlg::CSaveTreePdfDlg(CPhyloTreePane* pane, wxWindow *parent)
    : CwxSavePdfBaseDlg(parent)
    , m_Pane(pane)
{
    LoadSettings(kSavePdfBaseKey);

    wxString path = ToWxString(m_Path);
    m_FileNameBox->SetValue(path);
}

CSaveTreePdfDlg::~CSaveTreePdfDlg()
{
    // For this pdf dialog, save the entire file path including filename
    // since that is also chosen by the user
    m_Path = m_FileNameBox->GetValue();
    SaveSettings(kSavePdfBaseKey);
}

void CSaveTreePdfDlg::x_Init(wxBoxSizer* mainsizer, int widget_width)
{

}

bool CSaveTreePdfDlg::TransferDataToWindow()
{

    return true;
}

bool CSaveTreePdfDlg::TransferDataFromWindow()
{
    return true;
}

void CSaveTreePdfDlg::x_SavePdf() 
{
    SetCursor(*wxHOURGLASS_CURSOR);
    // add a progress bar to indicate that data are being updated
    m_Info->SetLabel(_("Saving PDF File...  Please Wait!"));
    m_InfoSizer->Layout();
    Layout();
    m_Panel->Enable(false);
    m_OpenButton->Enable(false);
    m_SaveButton->Enable(false);
    this->Update();

    CPhyloPdfGrabber grb_pdf(*m_Pane);
    grb_pdf.SaveTooltips(m_IncludeToolTips->GetValue());

    grb_pdf.SetOutputInfo(m_Path, m_FileName, "pdf");
    IImageGrabber::eCaptureResult res =
        grb_pdf.GrabImages(0/*dummy value, not used currently*/);    

    // Set the cursor to its value prior to the save
    SetCursor(*wxSTANDARD_CURSOR);
    if (res == IImageGrabber::eOpenGLError) {
        m_Info->SetLabel(_(""));
        wxMessageBox(wxT("Graphics system does not support in-memory image save feature"), 
            wxT("Error Saving"), wxOK);
    } else if (res == IImageGrabber::eFileError) {
        m_Info->SetLabel(_(""));
        m_Panel->Enable(true);
        m_OpenButton->Enable(true);
        m_SaveButton->Enable(true);
        wxMessageBox(wxT("File error occured during save. Make sure file and directory names are valid."), 
            wxT("Error Saving"), wxOK);
    } else {
        m_Info->SetLabel(_("PDF file saved."));
        if (m_OpenPdf) {
            // May vary from actual name if user did not end file name with ".pdf"
            wxString full_path = wxString::FromUTF8(CDir::ConcatPath(m_Path, m_FileName + ".pdf").c_str());
            wxLaunchDefaultApplication(full_path);
            m_OpenPdf = false;
        }
    }

    EndModal(wxID_CANCEL);
}


END_NCBI_SCOPE
