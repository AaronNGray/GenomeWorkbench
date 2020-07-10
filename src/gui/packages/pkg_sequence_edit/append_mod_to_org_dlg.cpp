/*  $Id: append_mod_to_org_dlg.cpp 45101 2020-05-29 20:53:24Z asztalos $
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

////@begin includes
////@end includes

#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/append_mod_to_org_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/append_mod_to_org.hpp>

#include <wx/button.h>
#include <wx/statbox.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CAppendModToOrgDlg, wxDialog )


/*!
 * CAppendModToOrgDlg event table definition
 */

BEGIN_EVENT_TABLE( CAppendModToOrgDlg, wxDialog )

////@begin CAppendModToOrgDlg event table entries
////@end CAppendModToOrgDlg event table entries
    EVT_CHOICE(ID_COMBOBOX_MOD, CAppendModToOrgDlg::OnChoiceChanged)
END_EVENT_TABLE()


/*!
 * CAppendModToOrgDlg constructors
 */

CAppendModToOrgDlg::CAppendModToOrgDlg()
{
    Init();
}

CAppendModToOrgDlg::CAppendModToOrgDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CAppendModToOrgDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAppendModToOrgDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAppendModToOrgDlg creation

    SetSize(wxSize(300, 300));
    return true;
}


/*!
 * CAppendModToOrgDlg destructor
 */

CAppendModToOrgDlg::~CAppendModToOrgDlg()
{
////@begin CAppendModToOrgDlg destruction
////@end CAppendModToOrgDlg destruction
}


/*!
 * Member initialisation
 */

void CAppendModToOrgDlg::Init()
{
    m_Types.clear();
    m_Types["Strain"] = Tchoice_item(true,COrgMod::eSubtype_strain,"");
    m_Types["Isolate"] = Tchoice_item(true,COrgMod::eSubtype_isolate,"");
    m_Types["Authority"] = Tchoice_item(true,COrgMod::eSubtype_authority,"");
    m_Types["Bio-material"] = Tchoice_item(true,COrgMod::eSubtype_bio_material,"");
    m_Types["Biovar"] = Tchoice_item(true,COrgMod::eSubtype_biovar,"bv.");
    m_Types["Clone"] = Tchoice_item(false,CSubSource::eSubtype_clone,"");
    m_Types["Collection-date"] = Tchoice_item(false,CSubSource::eSubtype_collection_date,"");
    m_Types["Country"] = Tchoice_item(false,CSubSource::eSubtype_country,"");
    m_Types["Culture-collection"] = Tchoice_item(true,COrgMod::eSubtype_culture_collection,"");
    m_Types["Forma"] = Tchoice_item(true,COrgMod::eSubtype_forma,"f.");
    m_Types["Forma-specialis"] = Tchoice_item(true,COrgMod::eSubtype_forma_specialis,"f. sp.");
    m_Types["Genotype"] = Tchoice_item(false,CSubSource::eSubtype_genotype,"genotype");
    m_Types["Haplotype"] = Tchoice_item(false,CSubSource::eSubtype_haplotype,"");
    m_Types["Host"] = Tchoice_item(true,COrgMod::eSubtype_nat_host,"");
    m_Types["Pathovar"] = Tchoice_item(true,COrgMod::eSubtype_pathovar,"pv.");
    m_Types["Serotype"] = Tchoice_item(true,COrgMod::eSubtype_serotype,"");
    m_Types["Serovar"] = Tchoice_item(true,COrgMod::eSubtype_serovar,"serovar");
    m_Types["Specimen voucher"] = Tchoice_item(true,COrgMod::eSubtype_specimen_voucher,"");
    m_Types["Sub-species"] = Tchoice_item(true,COrgMod::eSubtype_sub_species,"subsp.");
    m_Types["Subtype"] = Tchoice_item(true,COrgMod::eSubtype_subtype,"subtype");
    m_Types["Type"] = Tchoice_item(true,COrgMod::eSubtype_type,"type");
    m_Types["Variety"] = Tchoice_item(true,COrgMod::eSubtype_variety,"var.");

    m_Choice = NULL;
    m_CheckBox_sp = NULL;
    m_CheckBox_cf = NULL;
    m_CheckBox_aff = NULL;
    m_CheckBox_nr = NULL;
    m_CheckBox_no_taxid = NULL;
    m_CheckBox_abbr = NULL;
    m_constraint = NULL;

}


/*!
 * Control creation for CAppendModToOrgDlg
 */

void CAppendModToOrgDlg::CreateControls()
{    
    CAppendModToOrgDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxArrayString choice_strings;
    choice_strings.Add(_("Strain")); // Indexers requested this two choices to be on top (and repeat again in the alphabetical order)
    choice_strings.Add(_("Isolate"));
    for (map<string,Tchoice_item>::iterator e = m_Types.begin(); e != m_Types.end(); e++)
        choice_strings.Add(ToWxString(e->first));

    m_Choice = new wxChoice( itemDialog1, ID_COMBOBOX_MOD, wxDefaultPosition, wxDefaultSize, choice_strings );
    itemBoxSizer2->Add(m_Choice, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_Choice->SetSelection(0);

    m_CheckBox_sp = new wxCheckBox( itemDialog1, ID_CHECKBOX_SP, _("Only append to sp. organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox_sp->SetValue(false);
    itemBoxSizer2->Add(m_CheckBox_sp, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox_cf = new wxCheckBox( itemDialog1, ID_CHECKBOX_CF, _("Only append to cf. organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox_cf->SetValue(false);
    itemBoxSizer2->Add(m_CheckBox_cf, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox_aff = new wxCheckBox( itemDialog1, ID_CHECKBOX_AFF, _("Only append to aff. organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox_aff->SetValue(false);
    itemBoxSizer2->Add(m_CheckBox_aff, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox_nr = new wxCheckBox( itemDialog1, ID_CHECKBOX_NR, _("Only append to nr. organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox_nr->SetValue(false);
    itemBoxSizer2->Add(m_CheckBox_nr, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox_no_taxid = new wxCheckBox( itemDialog1, ID_CHECKBOX_NO_TAXID, _("Only to organisms with no taxonomy id"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox_no_taxid->SetValue(false);
    itemBoxSizer2->Add(m_CheckBox_no_taxid, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckBox_abbr = new wxCheckBox( itemDialog1, ID_CHECKBOX_ABBR, _("Use abbreviations"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckBox_abbr->SetValue(true);
    m_CheckBox_abbr->Disable();
    itemBoxSizer2->Add(m_CheckBox_abbr, 0, wxALIGN_LEFT|wxALL, 5);

    m_constraint = new CTripleConstraintPanel_CSourceFieldNamePanel_Wrapper(itemDialog1); 
    itemBoxSizer2->Add(m_constraint, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CAppendModToOrgDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAppendModToOrgDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CAppendModToOrgDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CAppendModToOrgDlg::GetCommand()
{
    CRef<CCmdComposite> cmd;

    if (m_Choice && m_Choice->GetSelection() != wxNOT_FOUND)
    {
        string selection = ToStdString(m_Choice->GetString(m_Choice->GetSelection()));
        map<string,Tchoice_item>::iterator e = m_Types.find(selection);
        if (e != m_Types.end())
        {
            bool isOrgMod = e->second.is_org;
            string abbr = e->second.abbr;
            int subtype = e->second.subtype;
            bool use_abbr = m_CheckBox_abbr->GetValue();
            bool only_sp = m_CheckBox_sp->GetValue();
            bool only_cf = m_CheckBox_cf->GetValue();
            bool only_aff = m_CheckBox_aff->GetValue();
            bool only_nr = m_CheckBox_nr->GetValue();
            bool no_taxid = m_CheckBox_no_taxid->GetValue();
            pair<string, SFieldTypeAndMatcher > item = m_constraint->GetItem(CFieldNamePanel::eFieldType_Source);
            CRef<CEditingActionConstraint> constraint = GetConstraint(item, subtype);
            CAppendModToOrg worker(isOrgMod, subtype, abbr, use_abbr, only_sp, only_cf, only_aff, only_nr, no_taxid, false, constraint);
            cmd = worker.GetCommand(m_TopSeqEntry);
        }
    }
    return cmd;
}

CRef<CEditingActionConstraint> CAppendModToOrgDlg::GetConstraint(pair<string, SFieldTypeAndMatcher > &item, int orgmod_subtype)
{
    CRef<CEditingActionConstraint> constraint(new CEditingActionConstraint());
    if (item.second.enabled)
    {
        const string field;// = COrgMod::ENUM_METHOD_NAME(ESubtype)()->FindName(orgmod_subtype, true);
        CFieldNamePanel::EFieldType field_type = CFieldNamePanel::eFieldType_Source; 
        int subtype = -1;

        CRef<CEditingActionConstraint> add = CreateEditingActionConstraint(field,
                                                                           field_type,
                                                                           subtype,
                                                                           item.second.field,
                                                                           item.second.field_type,
                                                                           item.second.subtype,
                                                                           item.second.matcher);
        *constraint += add;

        if (item.second.enabled2)
        {
            CRef<CEditingActionConstraint> add2 = CreateEditingActionConstraint(field,
                                                                                field_type,
                                                                                subtype,
                                                                                item.second.field2,
                                                                                item.second.field_type2,
                                                                                item.second.subtype2,
                                                                                item.second.matcher2);
            *constraint += add2;
        }
    }
    return constraint;
}

void CAppendModToOrgDlg::OnChoiceChanged(wxCommandEvent& event)
{
    if (m_Choice && m_Choice->GetSelection() != wxNOT_FOUND)
    {
        string selection = ToStdString(m_Choice->GetString(m_Choice->GetSelection()));
        map<string,Tchoice_item>::iterator e = m_Types.find(selection);
        if (e != m_Types.end() && !NStr::IsBlank(e->second.abbr)) {
            m_CheckBox_abbr->Enable();
            m_CheckBox_abbr->SetValue(true);
            if (selection == "Genotype")
                m_CheckBox_abbr->SetValue(false);
        }
        else
        {
            m_CheckBox_abbr->Disable();
            m_CheckBox_abbr->SetValue(true);
        }
    }
}

END_NCBI_SCOPE
