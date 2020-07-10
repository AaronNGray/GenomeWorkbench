/*  $Id: submit_remarks_panel.cpp 39708 2017-10-27 14:03:13Z filippov $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>
#include <objects/biblio/ArticleId.hpp>
#include <objects/biblio/ArticleIdSet.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes
////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/msgdlg.h> 

#include "submit_remarks_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CRemarksAndSerialNumberPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubmitRemarksPanel, wxPanel )


/*
 * CSubmitRemarksPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubmitRemarksPanel, wxPanel )

////@begin CSubmitRemarksPanel event table entries

////@end CSubmitRemarksPanel event table entries

END_EVENT_TABLE()


/*
 * CSubmitRemarksPanel constructors
 */

CSubmitRemarksPanel::CSubmitRemarksPanel()
{
    Init();
}

CSubmitRemarksPanel::CSubmitRemarksPanel( wxWindow* parent, CSerialObject& object,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CSubmit_block*>(&object);
    Create(parent, id, pos, size, style);
}


/*
 * CSubmitRemarksPanel creator
 */

bool CSubmitRemarksPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmitRemarksPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmitRemarksPanel creation
    return true;
}


/*
 * CSubmitRemarksPanel destructor
 */

CSubmitRemarksPanel::~CSubmitRemarksPanel()
{
////@begin CSubmitRemarksPanel destruction
////@end CSubmitRemarksPanel destruction
}


/*
 * Member initialisation
 */

void CSubmitRemarksPanel::Init()
{
////@begin CSubmitRemarksPanel member initialisation
    m_Comment = NULL;
////@end CSubmitRemarksPanel member initialisation
}


/*
 * Control creation for CSubmitRemarksPanel
 */

void CSubmitRemarksPanel::CreateControls()
{
////@begin CSubmitRemarksPanel content construction
    CSubmitRemarksPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Remark"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemPanel1, ID_REMARK, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CSubmitRemarksPanel content construction
}


bool CSubmitRemarksPanel::TransferDataToWindow()
{
    string comment = kEmptyStr;
    if (m_Object && m_Object->IsSetComment()) 
    {
        comment = m_Object->GetComment();
    }
       
    m_Comment->SetValue(ToWxString(comment));
    return true;
}


bool CSubmitRemarksPanel::TransferDataFromWindow()
{
    if (m_Object) {
        string comment = ToStdString(m_Comment->GetValue());
        if (NStr::IsBlank(comment)) {
            m_Object->ResetComment();
        } else {
            m_Object->SetComment(comment);
        }
        return true;
    } else {
        return false;
    }
}


/*
 * Should we show tooltips?
 */

bool CSubmitRemarksPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubmitRemarksPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmitRemarksPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmitRemarksPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubmitRemarksPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmitRemarksPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmitRemarksPanel icon retrieval
}


END_NCBI_SCOPE

