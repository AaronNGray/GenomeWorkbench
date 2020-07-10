/*  $Id: set_class_panel.cpp 31595 2014-10-27 14:51:04Z bollin $
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

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/set_class_panel.hpp>

#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSetClassPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSetClassPanel, wxPanel )


/*!
 * CSetClassPanel event table definition
 */

BEGIN_EVENT_TABLE( CSetClassPanel, wxPanel )

////@begin CSetClassPanel event table entries
    EVT_CHOICE( ID_CHOICE19, CSetClassPanel::OnChoice19Selected )

////@end CSetClassPanel event table entries

END_EVENT_TABLE()


/*!
 * CSetClassPanel constructors
 */

CSetClassPanel::CSetClassPanel()
{
    Init();
}

CSetClassPanel::CSetClassPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSetClassPanel creator
 */

bool CSetClassPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSetClassPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSetClassPanel creation
    return true;
}


/*!
 * CSetClassPanel destructor
 */

CSetClassPanel::~CSetClassPanel()
{
////@begin CSetClassPanel destruction
////@end CSetClassPanel destruction
}


/*!
 * Member initialisation
 */

void CSetClassPanel::Init()
{
////@begin CSetClassPanel member initialisation
    m_SetClass = NULL;
////@end CSetClassPanel member initialisation
}


/*!
 * Control creation for CSetClassPanel
 */

void CSetClassPanel::CreateControls()
{    
////@begin CSetClassPanel content construction
    CSetClassPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_SetClassStrings;
    m_SetClass = new wxChoice( itemPanel1, ID_CHOICE19, wxDefaultPosition, wxDefaultSize, m_SetClassStrings, 0 );
    itemBoxSizer2->Add(m_SetClass, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CSetClassPanel content construction

    x_SetDefaultTypes();
}


/*!
 * Should we show tooltips?
 */

bool CSetClassPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSetClassPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSetClassPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSetClassPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSetClassPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSetClassPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSetClassPanel icon retrieval
}


string CSetClassPanel::GetClassName(CBioseq_set::TClass class_type)
{
    string rval = "";

    switch (class_type) {
        case CBioseq_set::eClass_conset:
            rval = "Conset";
            break;
        case CBioseq_set::eClass_eco_set:
            rval = "Eco-set";
            break;
        case CBioseq_set::eClass_equiv:
            rval = "Equiv";
            break;
        case CBioseq_set::eClass_genbank:
            rval = "Genbank";
            break;
        case CBioseq_set::eClass_gen_prod_set:
            rval = "Gen-prod-set";
            break;
        case CBioseq_set::eClass_gi:
            rval = "Gi-set";
            break;
        case CBioseq_set::eClass_gibb:
            rval = "Gibb-set";
            break;
        case CBioseq_set::eClass_mut_set:
            rval = "Mut-set";
            break;
        case CBioseq_set::eClass_named_annot:
            rval = "Named-annot";
            break;
        case CBioseq_set::eClass_named_annot_prod:
            rval = "Named-annot-prod";
            break;
        case CBioseq_set::eClass_not_set:
            rval = "";
            break;
        case CBioseq_set::eClass_nuc_prot:
            rval = "Nuc-prot";
            break;
        case CBioseq_set::eClass_other:
            rval = "Other";
            break;
        case CBioseq_set::eClass_paired_end_reads:
            rval = "Paired-end-reads";
            break;
        case CBioseq_set::eClass_parts:
            rval = "Parts";
            break;
        case CBioseq_set::eClass_pdb_entry:
            rval = "Pdb-entry";
            break;
        case CBioseq_set::eClass_phy_set:
            rval = "Phy-set";
            break;
        case CBioseq_set::eClass_pir:
            rval = "Pir";
            break;
        case CBioseq_set::eClass_pop_set:
            rval = "Pop-set";
            break;
        case CBioseq_set::eClass_pub_set:
            rval = "Pub-set";
            break;
        case CBioseq_set::eClass_read_set:
            rval = "Read-set";
            break;
        case CBioseq_set::eClass_segset:
            rval = "Segset";
            break;
        case CBioseq_set::eClass_small_genome_set:
            rval = "Small-genome-set";
            break;
        case CBioseq_set::eClass_swissprot:
            rval = "Swissprot";
            break;
        case CBioseq_set::eClass_wgs_set:
            rval = "WGS-set";
            break;
    }
    return rval;
}


CBioseq_set::TClass GetClassType(const string& class_name)
{
    if (NStr::EqualNocase(class_name, "other")) {
        return CBioseq_set::eClass_other;
    }
    for (unsigned int class_type = CBioseq_set::eClass_not_set;
         class_type <= CBioseq_set::eClass_small_genome_set; class_type++) {
        if (NStr::EqualNocase(class_name, CSetClassPanel::GetClassName((CBioseq_set::TClass)class_type))) {
            return (CBioseq_set::TClass)class_type;
        }
    }
    return CBioseq_set::eClass_not_set;
}
    

void CSetClassPanel::SetClass(CBioseq_set::EClass class_type)
{
    x_SetDefaultTypes();

    string class_name = GetClassName(class_type);
    if (!m_SetClass->SetStringSelection(ToWxString(class_name))) {
        m_SetClass->AppendString(ToWxString(class_name));
        m_SetClass->SetStringSelection(ToWxString(class_name));
    }
}


objects::CBioseq_set::EClass CSetClassPanel::GetClass()
{
    string val = ToStdString(m_SetClass->GetStringSelection());

    return GetClassType(val);
}


void CSetClassPanel::x_SetDefaultTypes()
{
    m_SetClass->Clear();
    m_SetClass->AppendString(ToWxString(GetClassName(CBioseq_set::eClass_genbank)));
    m_SetClass->AppendString(ToWxString(GetClassName(CBioseq_set::eClass_mut_set)));
    m_SetClass->AppendString(ToWxString(GetClassName(CBioseq_set::eClass_pop_set)));
    m_SetClass->AppendString(ToWxString(GetClassName(CBioseq_set::eClass_phy_set)));
    m_SetClass->AppendString(ToWxString(GetClassName(CBioseq_set::eClass_eco_set)));
    m_SetClass->AppendString(ToWxString(GetClassName(CBioseq_set::eClass_wgs_set)));
    m_SetClass->AppendString(ToWxString(GetClassName(CBioseq_set::eClass_small_genome_set)));
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE19
 */

void CSetClassPanel::OnChoice19Selected( wxCommandEvent& event )
{
    wxWindow* parent = this->GetParent();
    ISetClassNotify* listener = dynamic_cast<ISetClassNotify*>(parent);
    while (parent && !listener) 
    {
        parent = parent->GetParent();
        listener = dynamic_cast<ISetClassNotify*>(parent);
    }

    if (listener) {
        listener->ChangeNotify();
    }
}


END_NCBI_SCOPE
