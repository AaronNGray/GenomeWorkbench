
#include <ncbi_pch.hpp>
#include "save_hit_matrix_pdf_dlg.hpp"
#include <gui/widgets/hit_matrix/hit_matrix_pane.hpp>
#include <gui/widgets/wx/pdf_grabber.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/print/pdf.hpp>

#include <corelib/ncbifile.hpp>

#include <wx/utils.h>
#include <wx/msgdlg.h>

#include <stdlib.h>
#include <string.h>

// ----------------------------------------------------------------------------
// CSaveHitMatrixPdfDlg
// ----------------------------------------------------------------------------

BEGIN_NCBI_SCOPE

IMPLEMENT_CLASS(CSaveHitMatrixPdfDlg, CwxSavePdfBaseDlg)

BEGIN_EVENT_TABLE(CSaveHitMatrixPdfDlg, CwxSavePdfBaseDlg)
END_EVENT_TABLE()

static const string kSavePdfBaseKey("SavePdfHitMatrixDialog");

CSaveHitMatrixPdfDlg::CSaveHitMatrixPdfDlg(CHitMatrixPane* pane, wxWindow *parent, const string& file_name)
    : CwxSavePdfBaseDlg(parent)
    , m_Pane(pane)
{
    LoadSettings(kSavePdfBaseKey);

    wxString path = ToWxString(m_Path);
    if (!file_name.empty()) {
        wxFileName file(path);
        file.SetName(ToWxString(file_name));
        path = file.GetFullPath();
    }
    m_FileNameBox->SetValue(path);

    m_IncludeToolTips->Hide();
}

CSaveHitMatrixPdfDlg::~CSaveHitMatrixPdfDlg()
{
    // For this pdf dialog, save the entire file path including filename
    // since that is also chosen by the user
    m_Path = m_FileNameBox->GetValue();
    SaveSettings(kSavePdfBaseKey);
}

void CSaveHitMatrixPdfDlg::x_Init(wxBoxSizer* mainsizer, int widget_width)
{

}

bool CSaveHitMatrixPdfDlg::TransferDataToWindow()
{

    return true;
}

bool CSaveHitMatrixPdfDlg::TransferDataFromWindow()
{
    return true;
}

void CSaveHitMatrixPdfDlg::x_SavePdf()
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

    CPdfGrabber grb_pdf(*m_Pane);
//    grb_pdf.SaveTooltips(m_IncludeToolTips->GetValue());

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
            wxString full_path = ToWxString(CDir::ConcatPath(m_Path, m_FileName + ".pdf"));
            wxLaunchDefaultApplication(full_path);
            m_OpenPdf = false;
        }
    }

    EndModal(wxID_CANCEL);
}


END_NCBI_SCOPE
