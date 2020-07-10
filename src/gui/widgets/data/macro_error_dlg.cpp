/*  $Id: macro_error_dlg.cpp 44338 2019-12-03 15:36:51Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  
 */


#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/hyperlink.h>
#include <wx/stattext.h>

#include <gui/widgets/data/macro_error_dlg.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/widgets/feedback/feedback.hpp>

BEGIN_NCBI_SCOPE

using namespace macro;

/*!
 * CMacroErrorDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMacroErrorDlg, wxDialog )


/*!
 * CMacroErrorDlg event table definition
 */

BEGIN_EVENT_TABLE( CMacroErrorDlg, wxDialog )

////@begin CMacroErrorDlg event table entries
    EVT_HYPERLINK( ID_ERROR_DETALS_HYPERLINKCTRL, CMacroErrorDlg::OnErrorDetalsClicked )
    EVT_HYPERLINK( ID_DATA_DETAILS_HYPERLINKCTRL, CMacroErrorDlg::OnDataDetailsClicked )
    EVT_BUTTON( ID_FEEDBACK_BUTTON, CMacroErrorDlg::OnFeedbackButtonClick )
////@end CMacroErrorDlg event table entries

END_EVENT_TABLE()


/*!
 * CMacroErrorDlg constructors
 */

 CMacroErrorDlg::CMacroErrorDlg()
{
    Init();
}

CMacroErrorDlg::CMacroErrorDlg(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CMacroErrorDlg creator
 */

bool CMacroErrorDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroErrorDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMacroErrorDlg creation
    return true;
}


/*!
 * CMacroErrorDlg destructor
 */

CMacroErrorDlg::~CMacroErrorDlg()
{
////@begin CMacroErrorDlg destruction
////@end CMacroErrorDlg destruction
}


/*!
 * Member initialisation
 */

void CMacroErrorDlg::Init()
{
////@begin CMacroErrorDlg member initialisation
    m_VSizer = NULL;
    m_ErrorTextCtrl = NULL;
    m_ErrorDetailsLink = NULL;
    m_ErrorDetailsCtrl = NULL;
    m_DataDetailsLink = NULL;
    m_DataDetailsCtrl = NULL;
////@end CMacroErrorDlg member initialisation
}


/*!
 * Control creation for CMacroErrorDlg
 */

void CMacroErrorDlg::CreateControls()
{    
////@begin CMacroErrorDlg content construction
    CMacroErrorDlg* itemDialog1 = this;

    m_VSizer = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(m_VSizer);

    m_ErrorTextCtrl = new wxStaticText( itemDialog1, wxID_ERROR_STATIC, _("Static text"), wxDefaultPosition, wxSize(400, -1), 0 );
    m_VSizer->Add(m_ErrorTextCtrl, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    m_VSizer->Add(itemStaticLine4, 0, wxGROW|wxALL, 5);

    m_ErrorDetailsLink = new wxHyperlinkCtrl( itemDialog1, ID_ERROR_DETALS_HYPERLINKCTRL, wxGetTranslation(wxString() + (wxChar) 0x02C5 + wxT(" Error Details")), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_ErrorDetailsLink->Enable(false);
    m_VSizer->Add(m_ErrorDetailsLink, 0, wxALIGN_LEFT|wxALL, 5);

    m_ErrorDetailsCtrl = new wxTextCtrl( itemDialog1, ID_ERROR_DETAILS_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(-1, 100), wxTE_MULTILINE|wxTE_READONLY );
    m_ErrorDetailsCtrl->Show(false);
    m_VSizer->Add(m_ErrorDetailsCtrl, 0, wxGROW|wxALL|wxFIXED_MINSIZE, 5);

    wxStaticLine* itemStaticLine7 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    m_VSizer->Add(itemStaticLine7, 0, wxGROW|wxALL, 5);

    m_DataDetailsLink = new wxHyperlinkCtrl( itemDialog1, ID_DATA_DETAILS_HYPERLINKCTRL, wxGetTranslation(wxString() + (wxChar) 0x02C5 + wxT(" Data Details")), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_DataDetailsLink->Enable(false);
    m_VSizer->Add(m_DataDetailsLink, 0, wxALIGN_LEFT|wxALL, 5);

    m_DataDetailsCtrl = new wxTextCtrl( itemDialog1, ID_DATA_DETAILS_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(-1, 100), wxTE_MULTILINE|wxTE_READONLY );
    m_DataDetailsCtrl->Show(false);
    m_VSizer->Add(m_DataDetailsCtrl, 0, wxGROW|wxALL|wxFIXED_MINSIZE, 5);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    m_VSizer->Add(itemStaticLine10, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    m_VSizer->Add(itemBoxSizer11, 0, wxGROW|wxALL, 5);

    itemBoxSizer11->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, ID_FEEDBACK_BUTTON, _("Send Feedback..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton14->SetDefault();
    itemBoxSizer11->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CMacroErrorDlg content construction
}

void CMacroErrorDlg::SetException(const string &message, const CException &error)
{
    string msg(message);
    msg += '\n';
    msg += x_MergeExceptionMessages(error);
    SetMessage(msg);

    const macro::CMacroDataException *pDataException = x_FindDataException(error);

    SetReport(error.ReportAll());
    if (pDataException) {
        SetData(pDataException->GetDataAsString());
    }
}

void CMacroErrorDlg::SetMessage(const string &message)
{
    _ASSERT(m_ErrorTextCtrl);
    m_ErrorTextCtrl->SetLabelText(message.c_str());
    m_ErrorTextCtrl->GetContainingSizer()->Fit(m_ErrorTextCtrl);
    m_VSizer->SetSizeHints(this);
}

void CMacroErrorDlg::SetReport(const string &report)
{
    _ASSERT(m_ErrorDetailsCtrl);
    m_ErrorDetailsCtrl->Clear();
    m_ErrorDetailsCtrl->AppendText(report.c_str());
    m_ErrorDetailsLink->Enable();
    m_VSizer->SetSizeHints(this);
}

void CMacroErrorDlg::SetData(const string &data)
{
    _ASSERT(m_DataDetailsCtrl);
    m_DataDetailsCtrl->Clear();
    m_DataDetailsCtrl->AppendText(data.c_str());
    m_DataDetailsLink->Enable();
    m_VSizer->SetSizeHints(this);
}

/*!
 * Should we show tooltips?
 */

bool CMacroErrorDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMacroErrorDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroErrorDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroErrorDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMacroErrorDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroErrorDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroErrorDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_ERROR_DETALS_HYPERLINKCTRL
 */

void CMacroErrorDlg::OnErrorDetalsClicked( wxHyperlinkEvent& event )
{
    if (m_ErrorDetailsCtrl->IsShown()) {
        m_ErrorDetailsLink->SetLabel(wxT("˅ Error Details"));
        m_VSizer->Show(m_ErrorDetailsCtrl, false);
    }
    else {
        m_ErrorDetailsLink->SetLabel(wxT("˄ Error Details"));
        m_VSizer->Show(m_ErrorDetailsCtrl);
    }
    //m_ErrorDetailsCtrl->GetContainingSizer()->Fit(m_ErrorDetailsCtrl);
    m_VSizer->SetSizeHints(this);
    m_VSizer->Layout();
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_DATA_DETAILS_HYPERLINKCTRL
 */

void CMacroErrorDlg::OnDataDetailsClicked( wxHyperlinkEvent& event )
{
    if (m_DataDetailsCtrl->IsShown()) {
        m_DataDetailsLink->SetLabel(wxT("˅ Data Details"));
        m_VSizer->Show(m_DataDetailsCtrl, false);
    }
    else {
        m_DataDetailsLink->SetLabel(wxT("˄ Data Details"));
        m_VSizer->Show(m_DataDetailsCtrl);
    }
    //m_DataDetailsCtrl->GetContainingSizer()->Fit(m_DataDetailsCtrl);
    m_VSizer->SetSizeHints(this);
    m_VSizer->Layout();
}

string CMacroErrorDlg::x_MergeExceptionMessages(const CException &error) const
{
    string result;
    const CException *current = &error;
    while (nullptr != current) {
        result += current->GetMsg();
        current = current->GetPredecessor();
        if (current)
            result += '\n';
    }
    return result;
}

const CMacroDataException* CMacroErrorDlg::x_FindDataException(const CException &error) const
{
    const CMacroDataException *pDataException = nullptr;
    const CException *current = &error;
    while (nullptr != current) {
        pDataException = dynamic_cast<const CMacroDataException *>(current);
        if (pDataException)
            break;
        current = current->GetPredecessor();
    }
    return pDataException;
}

/*!
* wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FEEDBACK_BUTTON
*/

void CMacroErrorDlg::OnFeedbackButtonClick(wxCommandEvent& event)
{
    string description;
    description += m_ErrorTextCtrl->GetLabelText().mb_str(wxConvUTF8);
    description += "\n\n";
    description += "Error Details:\n";
    description += m_ErrorDetailsCtrl->GetValue().mb_str(wxConvUTF8);

    ShowFeedbackDialog(false, description.c_str());
}

END_NCBI_SCOPE

