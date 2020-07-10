/*  $Id: sequence_goto_dlg.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/frame.h>

////@begin includes
////@end includes

#include <gui/widgets/seq/sequence_goto_dlg.hpp>
#include <gui/widgets/wx/regexp_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/utils.hpp>
////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CSequenceGotoEvent type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CSequenceGotoEvent, wxCommandEvent)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_SEQ_GOTO)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_SEQ_GOTO_CLOSE)

/*!
 * CSequenceGotoDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSequenceGotoDlg, wxDialog )

/*!
 * CSequenceGotoDlg event table definition
 */

BEGIN_EVENT_TABLE( CSequenceGotoDlg, wxDialog )

////@begin CSequenceGotoDlg event table entries
    EVT_CLOSE( CSequenceGotoDlg::OnCloseWindow )

    EVT_TEXT_ENTER( ID_TEXTCTRL1, CSequenceGotoDlg::OnTextctrl1Enter )

    EVT_BUTTON( wxID_OK, CSequenceGotoDlg::OnOkClick )

    EVT_BUTTON( wxID_CANCEL, CSequenceGotoDlg::OnCancelClick )

////@end CSequenceGotoDlg event table entries

END_EVENT_TABLE()


/*!
 * CSequenceGotoDlg constructors
 */

CSequenceGotoDlg::CSequenceGotoDlg() : m_Data(0)
{
    Init();
}

CSequenceGotoDlg::CSequenceGotoDlg( wxWindow* parent,
                                    CSequenceGotoData* data,
                                    wxEvtHandler* eventHandler,
                                    wxWindowID id, const wxString& caption,
                                    const wxPoint& pos, const wxSize& size, long style )
{
    m_Data = data;
    m_EventHandler = eventHandler ? eventHandler : parent;
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CSequenceGotoDlg creator
 */

bool CSequenceGotoDlg::Create( wxWindow* parent, wxWindowID id,
                               const wxString& caption, const wxPoint& pos,
                               const wxSize& size, long style )
{
////@begin CSequenceGotoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSequenceGotoDlg creation

    wxSize maxSize = this->GetSize();
    maxSize.SetWidth(-1);
    this->SetMaxSize(maxSize);

    return true;
}


/*!
 * CSequenceGotoDlg destructor
 */

CSequenceGotoDlg::~CSequenceGotoDlg()
{
////@begin CSequenceGotoDlg destruction
////@end CSequenceGotoDlg destruction
}


/*!
 * Member initialisation
 */

void CSequenceGotoDlg::Init()
{
////@begin CSequenceGotoDlg member initialisation
    m_InputBox = NULL;
////@end CSequenceGotoDlg member initialisation

    if (m_Data)
        m_SeqPos = m_Data->GetPositionString();
}


/*!
 * Control creation for CSequenceGotoDlg
 */

void CSequenceGotoDlg::CreateControls()
{
////@begin CSequenceGotoDlg content construction
    CSequenceGotoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Sequence Position/Range:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_InputBox = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_InputBox, 1, wxGROW|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("Go!"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton6->SetDefault();
    itemBoxSizer3->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Range examples: 1000;  1000-2000;  1000..2000;\n                        10,000:2,000,000;  100k:1m;\n                        1000 2000;  1000[tab]2000"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer7->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_InputBox->SetValidator( CRegexpValidator("^[ \t]*[1-9][0-9,]*[ \t]*[kKmM]?((([ \t]*([-:]|\\.\\.)[ \t]*)|([ \t]+))[1-9][0-9,]*[ \t]*[kKmM]?)?[ \t]*$","1000\n1000-2000\n1000..2000\n10,000:2,000,000\n100k:1m\n1000 2000\n1000    2000" , & m_SeqPos) );
////@end CSequenceGotoDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CSequenceGotoDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSequenceGotoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSequenceGotoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSequenceGotoDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSequenceGotoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSequenceGotoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSequenceGotoDlg icon retrieval
}


void CSequenceGotoDlg::SetFocusToInputBox()
{
    m_InputBox->SetFocus();
}


bool CSequenceGotoDlg::Show(bool show)
{
#ifdef __WXOSX_COCOA__
    wxFrame* f = NULL;
    wxWindow* pp = GetParent();
    while (f==NULL  &&  pp !=NULL) {
        f = dynamic_cast<wxFrame*>(pp);
        pp = pp->GetParent();
    }

    if (f != NULL) {
        if (show) {
            ncbi::AddChildWindowAbove(f, this);
        } else {
            ncbi::RemoveChildWindow(f, this);
        }
    }
#endif
    return wxWindow::Show(show);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */



void CSequenceGotoDlg::OnOkClick( wxCommandEvent& WXUNUSED(event) )
{
    if (Validate() && TransferDataFromWindow()) {
        CSequenceGotoEvent dlgEvent(wxEVT_COMMAND_SEQ_GOTO, GetId());
        dlgEvent.SetPositionString(m_SeqPos);
        if (m_Data)
            m_Data->SetPositionString(m_SeqPos);
        Send(dlgEvent);
    }
    m_InputBox->SetFocus();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CSequenceGotoDlg::OnCancelClick( wxCommandEvent& WXUNUSED(event) )
{
    CSequenceGotoEvent dlgEvent(wxEVT_COMMAND_SEQ_GOTO_CLOSE, GetId());
    Send(dlgEvent);
}

void CSequenceGotoDlg::Send(CSequenceGotoEvent& event)
{
    m_EventHandler->ProcessEvent(event);
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_CSEQUENCEGOTODLG
 */

void CSequenceGotoDlg::OnCloseWindow( wxCloseEvent& WXUNUSED(event) )
{
    CSequenceGotoEvent dlgEvent(wxEVT_COMMAND_SEQ_GOTO_CLOSE, GetId());
    Send(dlgEvent);
}


/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_TEXTCTRL1
 */

void CSequenceGotoDlg::OnTextctrl1Enter( wxCommandEvent& event )
{
    OnOkClick(event);
}


bool CSequenceGotoData::GetRange(long& from, long& to) const
{
    string pos_str = ToStdString(m_PositionStr);
    return CSeqUtils::StringToRange(pos_str, from, to);
}


//bool CSequenceGotoData::GetRange(long& from, long& to) const
//{
//    bool valid = false;
//    from = to = -1;
//    m_PositionStr.Trim(false); // trim white spaces on left side
//    m_PositionStr.Trim(true); // trim white spaces on right side
//    size_t len = m_PositionStr.length();
//
//    if (len == 0) return valid;
//
//    string pos_str;
//    bool space = false;
//    for (size_t i = 0; i < len; ++i) {
//        if (m_PositionStr[i
//        NStr::ReplaceInPlace(pos_str, ",", "");
//        NStr::ReplaceInPlace(pos_str, "k", "000");
//        NStr::ReplaceInPlace(pos_str, "K", "000");
//        NStr::ReplaceInPlace(pos_str, "m", "000000");
//        NStr::ReplaceInPlace(pos_str, "M", "000000");
//    }
//
//    typedef vector<string> TPositions;
//    TPositions pos;
//    NStr::Split(pos_str, "-:.", pos);
//    if (pos.size() < 2) {
//        // maybe space or tab is used as a separator
//        NStr::Split(pos_str, " \t", pos);
//    } else {
//        NStr::ReplaceInPlace(pos_str, " ", "");
//        NStr::ReplaceInPlace(pos_str, "\t", "");
//    }
//    try {
//        bool is_from = true;
//        ITERATE (TPositions, iter, pos) {
//            if (iter->empty()) continue;
//            if (is_from) {
//                to = from = NStr::StringToLong(*iter);
//                is_from = false;
//            } else {
//                to = NStr::StringToLong(*iter);
//            }            
//        }
//    } catch (CException&) {
//        LOG_POST(Warning
//            << "can't convert the input position string to integer: "
//            << pos_str) ;
//    }
//}
//

END_NCBI_SCOPE


