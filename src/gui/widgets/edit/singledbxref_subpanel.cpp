/*  $Id: singledbxref_subpanel.cpp 27370 2013-02-04 18:09:51Z bollin $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <util/xregexp/regexp.hpp>

////@begin includes
////@end includes

#include "singledbxref_subpanel.hpp"
#include "dbxref_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSingleDbxref_SubPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleDbxref_SubPanel, wxPanel )


/*!
 * CSingleDbxref_SubPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleDbxref_SubPanel, wxPanel )

////@begin CSingleDbxref_SubPanel event table entries
    EVT_TEXT( ID_TEXTCTRL32, CSingleDbxref_SubPanel::OnTextctrl32TextUpdated )

////@end CSingleDbxref_SubPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleDbxref_SubPanel constructors
 */

CSingleDbxref_SubPanel::CSingleDbxref_SubPanel()
{
    Init();
}

CSingleDbxref_SubPanel::CSingleDbxref_SubPanel( wxWindow* parent, CDbtag& tag, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_Tag = new CDbtag();
    m_Tag->Assign(tag);
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleDbxref_SubPanel creator
 */

bool CSingleDbxref_SubPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleDbxref_SubPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleDbxref_SubPanel creation
    return true;
}


/*!
 * CSingleDbxref_SubPanel destructor
 */

CSingleDbxref_SubPanel::~CSingleDbxref_SubPanel()
{
////@begin CSingleDbxref_SubPanel destruction
////@end CSingleDbxref_SubPanel destruction
}


/*!
 * Member initialisation
 */

void CSingleDbxref_SubPanel::Init()
{
////@begin CSingleDbxref_SubPanel member initialisation
    m_DbCtrl = NULL;
    m_ObjectIdCtrl = NULL;
////@end CSingleDbxref_SubPanel member initialisation
}


/*!
 * Control creation for CSingleDbxref_SubPanel
 */

void CSingleDbxref_SubPanel::CreateControls()
{    
////@begin CSingleDbxref_SubPanel content construction
    CSingleDbxref_SubPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_DbCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL31, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_DbCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ObjectIdCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL32, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_ObjectIdCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleDbxref_SubPanel content construction
}


bool CSingleDbxref_SubPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    if (m_Tag->IsSetDb()) {
        m_DbCtrl->SetValue(ToWxString(m_Tag->GetDb()));
    }
    if (m_Tag->IsSetTag()) {
        if (m_Tag->GetTag().IsId()) {
            m_ObjectIdCtrl->SetValue(ToWxString(NStr::NumericToString(m_Tag->GetTag().GetId())));
        } else if (m_Tag->GetTag().IsStr()) {
            m_ObjectIdCtrl->SetValue(ToWxString(m_Tag->GetTag().GetStr()));
        } else {
            m_ObjectIdCtrl->SetValue(wxEmptyString);
        }            
    } else {
        m_ObjectIdCtrl->SetValue(wxEmptyString);
    }            

    return true;
}


static bool s_IsAllDigits(const string& str)
{
    static CRegexp all_digits_regex("^[0-9]+$");
    return all_digits_regex.IsMatch(str);
}


bool CSingleDbxref_SubPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    string db = ToStdString(m_DbCtrl->GetValue());
    string id = ToStdString(m_ObjectIdCtrl->GetValue());
    if (NStr::IsBlank(db)) {
        m_Tag->ResetDb();
    } else {
        m_Tag->SetDb(db);
    }

    if (NStr::IsBlank(id)) {
        m_Tag->ResetTag();
    } else if (s_IsAllDigits(id)) {
        m_Tag->SetTag().SetId(NStr::StringToInt(id));
    } else {
        m_Tag->SetTag().SetStr(id);
    }
    return true;
}


CRef<CDbtag> CSingleDbxref_SubPanel::GetDbtag()
{
    TransferDataFromWindow();
    if (m_Tag)
        return m_Tag;

    return CRef<CDbtag>();
}


/*!
 * Should we show tooltips?
 */

bool CSingleDbxref_SubPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleDbxref_SubPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleDbxref_SubPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleDbxref_SubPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleDbxref_SubPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleDbxref_SubPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleDbxref_SubPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL32
 */

void CSingleDbxref_SubPanel::OnTextctrl32TextUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetValue()))) {
        return;
    }
    wxWindow* parent = this->GetParent();

    CDbxrefPanel* dbxrefpanel = dynamic_cast<CDbxrefPanel*>(parent);

    while (parent && !dbxrefpanel) {
        parent = parent->GetParent();
        dbxrefpanel = dynamic_cast<CDbxrefPanel*>(parent);
    }

    if (!dbxrefpanel) {
        return;
    }
    dbxrefpanel->AddLastDbxref((wxWindow*)this);
}

END_NCBI_SCOPE
