/* 
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
 * Authors:  Vasuki Gobu
 */



#include <ncbi_pch.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/general/Dbtag.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include "src_other_pnl.hpp"


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSourceOtherPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSourceOtherPanel, wxPanel )


/*
 * CSourceOtherPanel event table definition
 */

BEGIN_EVENT_TABLE( CSourceOtherPanel, wxPanel )

////@begin CSourceOtherPanel event table entries
////@end CSourceOtherPanel event table entries

END_EVENT_TABLE()


/*
 * CSourceOtherPanel constructors
 */

CSourceOtherPanel::CSourceOtherPanel()
{
    Init();
}

CSourceOtherPanel::CSourceOtherPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSourceOtherPanel creator
 */

bool CSourceOtherPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSourceOtherPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSourceOtherPanel creation
    return true;
}


/*
 * CSourceOtherPanel destructor
 */

CSourceOtherPanel::~CSourceOtherPanel()
{
////@begin CSourceOtherPanel destruction
////@end CSourceOtherPanel destruction
}


/*
 * Member initialisation
 */

void CSourceOtherPanel::Init()
{
////@begin CSourceOtherPanel member initialisation
    m_CommonNameCtrl = NULL;
    m_LineageCtrl = NULL;
    m_DivisionCtrl = NULL;
////@end CSourceOtherPanel member initialisation
    m_OrgRef.Reset(new COrg_ref());
}


/*
 * Control creation for CSourceOtherPanel
 */

void CSourceOtherPanel::CreateControls()
{    
////@begin CSourceOtherPanel content construction
    CSourceOtherPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Common Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CommonNameCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_CommonNameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Taxonomic Lineage"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LineageCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(m_LineageCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Division"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DivisionCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_DivisionCtrl, 0, wxALIGN_LEFT|wxGROW|wxALL, 5);

////@end CSourceOtherPanel content construction

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 1, wxALIGN_LEFT|wxALL, 0);
    
    m_Dbxrefs = new CDbxrefPanel(this, *m_OrgRef);
    itemBoxSizer4->Add(m_Dbxrefs, 0, wxEXPAND|wxALL, 5);
}


/*
 * Should we show tooltips?
 */

bool CSourceOtherPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSourceOtherPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSourceOtherPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSourceOtherPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSourceOtherPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSourceOtherPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSourceOtherPanel icon retrieval
}


void CSourceOtherPanel::TransferFromOrgRef(const COrg_ref& org)
{
    if (org.IsSetCommon()) {
        m_CommonNameCtrl->SetValue(ToWxString(org.GetCommon()));
    } else {
        m_CommonNameCtrl->SetValue(wxEmptyString);
    }
    if (org.IsSetLineage()) {
        m_LineageCtrl->SetValue(ToWxString(org.GetLineage()));
    } else {
        m_LineageCtrl->SetValue(wxEmptyString);
    }
    if (org.IsSetDivision()) {
        m_DivisionCtrl->SetValue(ToWxString(org.GetDivision()));
    } else {
        m_DivisionCtrl->SetValue(wxEmptyString);
    }

    m_OrgRef->ResetDb();
    if (org.IsSetDb()) {
        ITERATE(COrg_ref::TDb, it, org.GetDb()) {
            CRef<CDbtag> db(new CDbtag());
            db->Assign(**it);
            m_OrgRef->SetDb().push_back(db);
        }
    }
    m_Dbxrefs->TransferDataToWindow();
}


void CSourceOtherPanel::TransferToOrgRef(COrg_ref& org)
{
    TransferDataFromWindow();
    string common = ToStdString(m_CommonNameCtrl->GetValue());
    if (NStr::IsBlank(common)) {
        org.ResetCommon();
    } else {
        org.SetCommon(common);
    }

    string lineage = ToStdString(m_LineageCtrl->GetValue());
    if (NStr::IsBlank(lineage)) {
        org.SetOrgname().ResetLineage();
    } else {
        org.SetOrgname().SetLineage(lineage);
    }

    string div = ToStdString(m_DivisionCtrl->GetValue());
    if (NStr::IsBlank(div)) {
        org.SetOrgname().ResetDiv();
    } else {
        org.SetOrgname().SetDiv(div);
    }
   
    m_Dbxrefs->PopulateDbxrefs(org);
}

void CSourceOtherPanel::OnChangedTaxname(void)
{
    m_CommonNameCtrl->Clear();
    m_LineageCtrl->Clear();
    m_Dbxrefs->DeleteTaxonRef();
    m_DivisionCtrl->Clear();
}

END_NCBI_SCOPE
