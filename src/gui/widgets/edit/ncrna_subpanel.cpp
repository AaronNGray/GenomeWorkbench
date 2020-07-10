/*  $Id: ncrna_subpanel.cpp 41550 2018-08-20 15:22:02Z katargir $
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
////@begin includes
////@end includes

#include <objects/seqfeat/RNA_gen.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "ncrna_subpanel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CncRNASubPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CncRNASubPanel, wxPanel )


/*!
 * CncRNASubPanel event table definition
 */

BEGIN_EVENT_TABLE( CncRNASubPanel, wxPanel )

////@begin CncRNASubPanel event table entries
    EVT_CHOICE( ID_CHOICE5, CncRNASubPanel::OnChoice5Selected )

////@end CncRNASubPanel event table entries

END_EVENT_TABLE()


/*!
 * CncRNASubPanel constructors
 */

CncRNASubPanel::CncRNASubPanel()
{
    Init();
}

CncRNASubPanel::CncRNASubPanel( wxWindow* parent, CRef<CRNA_gen> gen, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_Gen(gen)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CncRNASubPanel creator
 */

bool CncRNASubPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CncRNASubPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CncRNASubPanel creation
    return true;
}


/*!
 * CncRNASubPanel destructor
 */

CncRNASubPanel::~CncRNASubPanel()
{
////@begin CncRNASubPanel destruction
////@end CncRNASubPanel destruction
}


/*!
 * Member initialisation
 */

void CncRNASubPanel::Init()
{
////@begin CncRNASubPanel member initialisation
    m_ClassCtrl = NULL;
    m_OtherClass = NULL;
    m_ProductCtrl = NULL;
////@end CncRNASubPanel member initialisation
}


/*!
 * Control creation for CncRNASubPanel
 */

void CncRNASubPanel::CreateControls()
{    
////@begin CncRNASubPanel content construction
    CncRNASubPanel* itemPanel1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel1->SetSizer(itemFlexGridSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Class"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 5);

    wxArrayString m_ClassCtrlStrings;
    m_ClassCtrl = new wxChoice( itemPanel1, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, m_ClassCtrlStrings, 0 );
    itemBoxSizer4->Add(m_ClassCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OtherClass = new wxTextCtrl( itemPanel1, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    m_OtherClass->Enable(false);
    itemBoxSizer4->Add(m_OtherClass, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Product"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProductCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(m_ProductCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CncRNASubPanel content construction
    m_ClassCtrl->AppendString(wxEmptyString);
    vector<string> class_vals = CRNA_gen::GetncRNAClassList();
    ITERATE(vector<string>, it, class_vals) {
        m_ClassCtrl->AppendString(ToWxString(*it));
    }
    Layout();
}


bool CncRNASubPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }

    if (m_Gen->IsSetClass()) {
        bool need_other = false;
        bool extra_val = false;
        if (!m_ClassCtrl->SetStringSelection(ToWxString(m_Gen->GetClass()))) {
            need_other = true;
            extra_val = true;
        }
        if (NStr::EqualNocase(m_Gen->GetClass(), "other")) {
            need_other = true;
        }
        if (need_other) {
            m_ClassCtrl->SetStringSelection (_("other"));
            if (extra_val) {
                m_OtherClass->SetValue(ToWxString(m_Gen->GetClass()));
            }
            m_OtherClass->Enable(true);
        } else {
            m_OtherClass->Enable(false);
        }
    } else {
        m_ClassCtrl->SetStringSelection (_(""));
        m_OtherClass->SetValue (wxEmptyString);
        m_OtherClass->Enable(false);
    }

    if (m_Gen->IsSetProduct()) {
        m_ProductCtrl->SetValue(ToWxString(m_Gen->GetProduct()));
    } else {
        m_ProductCtrl->SetValue(wxEmptyString);
    }
    return true;
}


bool CncRNASubPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }

    string ncrnaclass = ToStdString (m_ClassCtrl->GetStringSelection());
    if (NStr::Equal(ncrnaclass, "other")) {
        string add = ToStdString(m_OtherClass->GetValue());
        if (!NStr::IsBlank(add)) {
            ncrnaclass = add;
        }
    }
    if (NStr::IsBlank(ncrnaclass)) {
        m_Gen->ResetClass();
    } else {
        m_Gen->SetClass(ncrnaclass);
    }
    
    string product = ToStdString (m_ProductCtrl->GetValue());
    if (NStr::IsBlank(product)) {
        m_Gen->ResetProduct();
    } else {
        m_Gen->SetProduct(product);
    }
    return true;
}


/*!
 * Should we show tooltips?
 */

bool CncRNASubPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CncRNASubPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CncRNASubPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CncRNASubPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CncRNASubPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CncRNASubPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CncRNASubPanel icon retrieval
}



/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE5
 */

void CncRNASubPanel::OnChoice5Selected( wxCommandEvent& event )
{
    string ncrnaclass = ToStdString (m_ClassCtrl->GetStringSelection());
    if (NStr::Equal(ncrnaclass, "other")) {
        m_OtherClass->Enable(true);
    } else {
        m_OtherClass->Enable(false);
    }
}

END_NCBI_SCOPE
