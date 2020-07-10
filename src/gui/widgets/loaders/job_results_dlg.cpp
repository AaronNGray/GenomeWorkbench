/*  $Id: job_results_dlg.cpp 44096 2019-10-25 15:20:48Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/filedlg.h>
#include <wx/textbuf.h>

////@begin includes
////@end includes
#include <html/html.hpp> 
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/loaders/job_results_dlg.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CJobResultsDlg, wxDialog )

BEGIN_EVENT_TABLE( CJobResultsDlg, wxDialog )

////@begin CJobResultsDlg event table entries
EVT_BUTTON(ID_JOB_RESULTS_DLG_EXPORT, CJobResultsDlg::OnExportClick)
////@end CJobResultsDlg event table entries

END_EVENT_TABLE()

CJobResultsDlg::CJobResultsDlg()
{
    Init();
}

CJobResultsDlg::CJobResultsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CJobResultsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CJobResultsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CJobResultsDlg creation
    return true;
}

CJobResultsDlg::~CJobResultsDlg()
{
////@begin CJobResultsDlg destruction
////@end CJobResultsDlg destruction
}

void CJobResultsDlg::Init()
{
////@begin CJobResultsDlg member initialisation
////@end CJobResultsDlg member initialisation
}

void CJobResultsDlg::CreateControls()
{    
////@begin CJobResultsDlg content construction
    CJobResultsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxHtmlWindow* itemHtmlWindow3 = new wxHtmlWindow( itemDialog1, ID_HTMLWINDOW1, wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(350, 200)), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(itemHtmlWindow3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxALL, 5);

    wxButton* itemButton11 = new wxButton(itemDialog1, ID_JOB_RESULTS_DLG_EXPORT, _("Export..."), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer5->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    itemBoxSizer5->AddStretchSpacer();

    wxButton* itemButton4 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CJobResultsDlg content construction

    wxHtmlWindow* htmlWnd = (wxHtmlWindow*)FindWindow(ID_HTMLWINDOW1);
    if (htmlWnd) htmlWnd->SetPage(m_HTML);
}

bool CJobResultsDlg::ShowToolTips()
{
    return true;
}
wxBitmap CJobResultsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CJobResultsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CJobResultsDlg bitmap retrieval
}
wxIcon CJobResultsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CJobResultsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CJobResultsDlg icon retrieval
}

void CJobResultsDlg::OnExportClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, wxT("Select a file"), m_WorkDir, wxT(""),
                     CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString path = dlg.GetPath();
    if (!path.IsEmpty()) {
        CNcbiOfstream os(path.fn_str(), ios::out);
        string text = m_HTML.ToStdString();
        NStr::ReplaceInPlace(text, "</h5>", wxString(wxTextBuffer::GetEOL()).ToStdString());
        NStr::ReplaceInPlace(text, "<br />", wxString(wxTextBuffer::GetEOL()).ToStdString());
        os << CHTMLHelper::StripHTML(text);
    }
}

END_NCBI_SCOPE
