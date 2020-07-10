/*  $Id: macro_selectqual_dlg.cpp 44787 2020-03-13 19:03:59Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/dblink_fieldtype.hpp>
#include <gui/widgets/edit/misc_fieldtype.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/macro_treeitemdata.hpp>
#include <gui/widgets/edit/macro_selectqual_dlg.hpp>

#include <wx/button.h>
#include <wx/choice.h>

BEGIN_NCBI_SCOPE


/*!
 * CSelectQualdDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSelectQualdDlg, wxDialog )


/*!
 * CSelectQualdDlg event table definition
 */

BEGIN_EVENT_TABLE( CSelectQualdDlg, wxDialog )
    EVT_CHOICE(ID_SELECTMATCHFIELD, CSelectQualdDlg::OnFieldTypeSelected)
    EVT_BUTTON(wxID_OK, CSelectQualdDlg::OnSelect)
    EVT_BUTTON(wxID_CANCEL, CSelectQualdDlg::OnCancel)
END_EVENT_TABLE()


/*!
 * CSelectQualdDlg constructors
 */

CSelectQualdDlg::CSelectQualdDlg()
{
    Init();
}

CSelectQualdDlg::CSelectQualdDlg( wxWindow* parent, EMacroFieldType field_type, EMacroFieldType match_type, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_FieldType(field_type), m_MatchType(match_type)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CSelectQualdDlg creator
 */

bool CSelectQualdDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSelectQualdDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CSelectQualdDlg creation
    return true;
}


/*!
 * CSelectQualdDlg destructor
 */

CSelectQualdDlg::~CSelectQualdDlg()
{
////@begin CSelectQualdDlg destruction
////@end CSelectQualdDlg destruction
}


/*!
 * Member initialisation
 */

void CSelectQualdDlg::Init()
{
    m_FieldChoice = NULL;
    m_FieldPicker = NULL;
    m_Field = NULL;
}


/*!
 * Control creation for CSelectQualdDlg
 */

void CSelectQualdDlg::CreateControls()
{    
////@begin CSelectQualdDlg content construction
    CSelectQualdDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    CFieldNamePanel::EFieldType type = s_ConvertFieldtype(m_MatchType);
    vector<CFieldNamePanel::EFieldType> field_types{ 
        CFieldNamePanel::eFieldType_Source, 
        CFieldNamePanel::eFieldType_Feature,
        CFieldNamePanel::eFieldType_CDSGeneProt,
        CFieldNamePanel::eFieldType_RNA,
        CFieldNamePanel::eFieldType_MolInfo,
        CFieldNamePanel::eFieldType_Pub,
        CFieldNamePanel::eFieldType_StructuredComment,
        CFieldNamePanel::eFieldType_DBLink,
        CFieldNamePanel::eFieldType_Misc
    };
    wxArrayString field_type_strings;
    int selected = -1;
    for (size_t index = 0; index < field_types.size(); ++index) {
        if (type == field_types[index])
            selected = index;
        field_type_strings.Add(ToWxString(CFieldNamePanel::GetFieldTypeName(field_types[index])));
    }
    m_FieldChoice = new wxChoice(this, ID_SELECTMATCHFIELD, wxDefaultPosition, wxDefaultSize, field_type_strings, 0);
    itemBoxSizer2->Add(m_FieldChoice, 0, wxALIGN_LEFT|wxALL, 5);
    m_FieldChoice->SetSelection(selected);

    m_FieldPicker = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_FieldPicker, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxButton* itemButton13 = new wxButton(itemDialog1, wxID_OK, _("Select"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* itemButton14 = new wxButton(itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    SetFieldType(type);
    x_SetTarget();

////@end CSelectQualdDlg content construction
}

CFieldNamePanel::EFieldType CSelectQualdDlg::s_ConvertFieldtype(EMacroFieldType macro_field_type)
{
    CFieldNamePanel::EFieldType type = CFieldNamePanel::eFieldType_Unknown;
    switch (macro_field_type) {
    case EMacroFieldType::eBiosourceAll:
        type = CFieldNamePanel::eFieldType_Source;
        break;
    case EMacroFieldType::eFeatQualifiers:
        type = CFieldNamePanel::eFieldType_Feature;
        break;
    case EMacroFieldType::eCdsGeneProt:
        type = CFieldNamePanel::eFieldType_CDSGeneProt;
        break;
    case EMacroFieldType::eRNA:
        type = CFieldNamePanel::eFieldType_RNA;
        break;
    case EMacroFieldType::eMolinfo:
        type = CFieldNamePanel::eFieldType_MolInfo;
        break;
    case EMacroFieldType::ePubdesc:
        type = CFieldNamePanel::eFieldType_Pub;
        break;
    case EMacroFieldType::eStructComment:
        type = CFieldNamePanel::eFieldType_StructuredComment;
        break;
    case EMacroFieldType::eDBLink:
        type = CFieldNamePanel::eFieldType_DBLink;
        break;
    case EMacroFieldType::eMiscDescriptors:
        type = CFieldNamePanel::eFieldType_Misc;
        break;
    default:
        break;
    }
    return type;
}

void CSelectQualdDlg::x_SetTarget()
{
    m_Target.first.clear();
    m_Target.second.clear();

    switch (m_FieldType) {
    case EMacroFieldType::eBiosourceAll:
        m_Target.first = macro::CMacroBioData::sm_BioSource;
        break;
    case EMacroFieldType::eFeatQualifiers:
        m_Target.first = macro::CMacroBioData::sm_SeqFeat;
        break;
    case EMacroFieldType::eCdsGeneProt:
        m_Target.first = macro::CMacroBioData::sm_Protein;
        break;
    case EMacroFieldType::eRNA:
        m_Target.first = macro::CMacroBioData::sm_RNA;
        break;
    case EMacroFieldType::eMolinfo:
        m_Target.first = macro::CMacroBioData::sm_MolInfo;
        break;
    case EMacroFieldType::ePubdesc:
        m_Target.first = macro::CMacroBioData::sm_Pubdesc;
        break;
    case EMacroFieldType::eStructComment:
        m_Target.first = macro::CMacroBioData::sm_StrComm;
        break;
    case EMacroFieldType::eDBLink:
        m_Target.first = macro::CMacroBioData::sm_SeqNa;
        break;
    case EMacroFieldType::eMiscDescriptors:
        m_Target.first = macro::CMacroBioData::sm_SeqNa;
        break;
    default:
        break;
    }

}

void CSelectQualdDlg::UpdateEditor(void)
{
    string val = ToStdString(m_FieldChoice->GetStringSelection());
    CFieldNamePanel::EFieldType field_type = CFieldNamePanel::GetFieldTypeFromName(val);
    if (field_type == CFieldNamePanel::eFieldType_RNA && m_FieldType == EMacroFieldType::eRNA) {
        CRNAFieldNamePanel* rna_panel = dynamic_cast<CRNAFieldNamePanel*>(m_Field);
        if (rna_panel) {
            string rna_type = rna_panel->GetRnaTypeOnly();
            m_Target.second = rna_type;
            m_Target.first = NMItemData::UpdateTargetForRnas(rna_type);
        }
    }
}

void CSelectQualdDlg::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void CSelectQualdDlg::OnSelect(wxCommandEvent& event)
{
    m_SelectedField.Reset();
    m_SelectedField.m_GUIName = m_Field->GetFieldName(false);
    m_SelectedField.m_MacroName = m_Field->GetMacroFieldName(m_Target.first, m_Target.second);
    
    string val = ToStdString(m_FieldChoice->GetStringSelection());
    CFieldNamePanel::EFieldType field_type = CFieldNamePanel::GetFieldTypeFromName(val);
    if (field_type != s_ConvertFieldtype(m_FieldType)) {
        m_SelectedField.m_UseMacroName = true;
    }
    if (!m_SelectedField.m_GUIName.empty())
        EndModal(wxID_OK);
}

void CSelectQualdDlg::OnFieldTypeSelected(wxCommandEvent& event)
{
    string val = ToStdString(m_FieldChoice->GetStringSelection());
    CFieldNamePanel::EFieldType field_type = CFieldNamePanel::GetFieldTypeFromName(val);
    SetFieldType(field_type);
}

void CSelectQualdDlg::SetFieldType(CFieldNamePanel::EFieldType field_type)
{
    Freeze();
    while (m_FieldPicker->GetItemCount() > 0) {
        size_t pos = 0;
        m_FieldPicker->GetItem(pos)->DeleteWindows();
        m_FieldPicker->Remove(pos);
    }

    m_Field = nullptr;
    switch (field_type) {
    case CFieldNamePanel::eFieldType_Source:
    {
        auto panel = new CSourceFieldNamePanel(this);
        panel->HideSourceType();
        m_Field = panel;
        m_MatchType = EMacroFieldType::eBiosourceAll;
        break;
    }
    case CFieldNamePanel::eFieldType_Feature:
    {
        auto panel = new CFeatureFieldNamePanel(this);
        objects::CSeq_entry_Handle seh;
        panel->ListPresentFeaturesFirst(seh);
        m_Field = panel;
        m_MatchType = EMacroFieldType::eFeatQualifiers;
        break;
    }
    case CFieldNamePanel::eFieldType_CDSGeneProt:
        m_Field = new CCDSGeneProtFieldNamePanel(this);
        m_Field->SetFieldName("protein name");
        m_MatchType = EMacroFieldType::eCdsGeneProt;
        break;
    case CFieldNamePanel::eFieldType_RNA:
        m_Field = new CRNAFieldNamePanel(this);
        m_Field->SetFieldName("product");
        m_MatchType = EMacroFieldType::eRNA;
        break;
    case CFieldNamePanel::eFieldType_MolInfo:
    {
        auto panel = new CFieldHandlerNamePanel(this);
        panel->SetFieldNames(CMolInfoFieldType::GetFieldNames());
        panel->SetMacroFieldNames(CMolInfoFieldType::GetMacroFieldNames());
        panel->SetMacroSelf(macro::CMacroBioData::sm_MolInfo);
        panel->SetFieldName("molecule");
        m_Field = panel;
        m_MatchType = EMacroFieldType::eMolinfo;
        break;
    }
    case CFieldNamePanel::eFieldType_Pub:
    {
        auto panel = new CFieldHandlerNamePanel(this);
        panel->SetFieldNames(CPubFieldType::GetFieldNames(true));
        panel->SetMacroFieldNames(CPubFieldType::GetMacroFieldNames());
        panel->SetMacroSelf(macro::CMacroBioData::sm_Pubdesc);
        panel->SetFieldName("title");
        m_Field = panel;
        m_MatchType = EMacroFieldType::ePubdesc;
        break;
    }
    case CFieldNamePanel::eFieldType_StructuredComment:
        m_Field = new CStructCommentFieldPanel(this);
        m_MatchType = EMacroFieldType::eStructComment;
        break;

    case CFieldNamePanel::eFieldType_DBLink:
    {
        auto panel = new CFieldHandlerNamePanel(this);
        panel->SetFieldNames(CDBLinkFieldType::GetFieldNames());
        panel->SetMacroFieldNames(CDBLinkFieldType::GetFieldNames());
        panel->SetMacroSelf("DBLink");
        m_Field = panel;
        m_MatchType = EMacroFieldType::eDBLink;
        break;
    }
    case CFieldNamePanel::eFieldType_Misc:
    {
        auto panel = new CFieldHandlerNamePanel(this);
        panel->SetFieldNames(CMiscFieldType::GetStrings());
        panel->SetMacroFieldNames(CMiscFieldType::GetMacroFieldNames());
        panel->SetMacroSelf("Misc");
        panel->SetFieldName(kDefinitionLineLabel);
        m_Field = panel;
        m_MatchType = EMacroFieldType::eMiscDescriptors;
        break;
    }
    default:
        m_MatchType = EMacroFieldType::eNotSet;
        break;
    }

    if (m_Field) {
        m_FieldPicker->Add(m_Field);
    }

    Layout();
    m_FieldPicker->SetMinSize(wxSize(387, 153));
    Thaw();
}

/*!
 * Should we show tooltips?
 */

bool CSelectQualdDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSelectQualdDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSelectQualdDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSelectQualdDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSelectQualdDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSelectQualdDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSelectQualdDlg icon retrieval
}

END_NCBI_SCOPE

