/*  $Id: prefix_deflinedlg.cpp 42223 2019-01-16 16:07:06Z asztalos $
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
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objtools/edit/autodef_with_tax.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>

#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>
#include <gui/packages/pkg_sequence_edit/prefix_deflinedlg.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/arrstr.h>
#include <wx/string.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

IMPLEMENT_DYNAMIC_CLASS( CPrefixDeflinedlg, wxDialog )


BEGIN_EVENT_TABLE( CPrefixDeflinedlg, wxDialog )

END_EVENT_TABLE()


CPrefixDeflinedlg::CPrefixDeflinedlg()
{
    Init();
}

CPrefixDeflinedlg::CPrefixDeflinedlg( wxWindow* parent, CSeq_entry_Handle seh,
                            unsigned int withcombo, wxWindowID id, 
                            const wxString& caption, const wxPoint& pos, 
                            const wxSize& size, long style )
: m_TopSeqEntry(seh), m_WithComboboxes(withcombo)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CPrefixDeflinedlg creator
 */

bool CPrefixDeflinedlg::Create( wxWindow* parent, wxWindowID id, 
        const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPrefixDeflinedlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPrefixDeflinedlg creation
    return true;
}


/*!
 * CPrefixDeflinedlg destructor
 */

CPrefixDeflinedlg::~CPrefixDeflinedlg()
{

}


/*!
 * Member initialisation
 */

void CPrefixDeflinedlg::Init()
{
////@begin CPrefixDeflinedlg member initialisation
    m_Firstmod = NULL;
    m_Secondmod = NULL;
    m_Thirdmod = NULL;
    m_PrefixTaxname = NULL;
    m_UseLabels = NULL;
////@end CPrefixDeflinedlg member initialisation
}


/*!
 * Control creation for CPrefixDeflinedlg
 */

void CPrefixDeflinedlg::CreateControls()
{    
////@begin CPrefixDeflinedlg content construction
    CPrefixDeflinedlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    if (m_WithComboboxes > 0){
        wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
        itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

        wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("1"), 
                                            wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        wxArrayString m_FirstmodStrings;
        m_Firstmod = new wxComboBox( itemDialog1, ID_COMBOBOX1, wxEmptyString, 
                wxDefaultPosition, wxDefaultSize, m_FirstmodStrings, wxCB_DROPDOWN );
        itemBoxSizer3->Add(m_Firstmod, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        
        if (m_WithComboboxes > 1){
            wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, 
                        wxID_STATIC, _("2"), wxDefaultPosition, wxDefaultSize, 0 );
            itemBoxSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

            m_Secondmod = new wxComboBox( itemDialog1, ID_COMBOBOX2, wxEmptyString, 
                    wxDefaultPosition, wxDefaultSize, m_FirstmodStrings, wxCB_DROPDOWN );
            itemBoxSizer3->Add(m_Secondmod, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

            if (m_WithComboboxes > 2){
                wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, 
                            wxID_STATIC, _("3"), wxDefaultPosition, wxDefaultSize, 0 );
                itemBoxSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

                m_Thirdmod = new wxComboBox( itemDialog1, ID_COMBOBOX3, wxEmptyString, 
                        wxDefaultPosition, wxDefaultSize, m_FirstmodStrings, wxCB_DROPDOWN );
                itemBoxSizer3->Add(m_Thirdmod, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
            }
        }
    }

    m_PrefixTaxname = new wxCheckBox( itemDialog1, ID_CHECKBOX15, _("Prefix with taxonomy name"), 
                wxDefaultPosition, wxDefaultSize, 0 );
    m_PrefixTaxname->SetValue(true);
    itemBoxSizer2->Add(m_PrefixTaxname, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_UseLabels = new wxCheckBox( itemDialog1, ID_CHECKBOX16, _("Use modifier labels"), 
            wxDefaultPosition, wxDefaultSize, 0 );
    m_UseLabels->SetValue(true);
    itemBoxSizer2->Add(m_UseLabels, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), 
                wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), 
                wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CPrefixDeflinedlg content construction
}

/*!
 * Should we show tooltips?
 */

bool CPrefixDeflinedlg::ShowToolTips()
{
    return true;
}

wxBitmap CPrefixDeflinedlg::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}


wxIcon CPrefixDeflinedlg::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

void CPrefixDeflinedlg::PopulateModifierCombo()
{
    wxArrayString ModStrings;
    
    for (size_t n=0; n < m_Params->m_ModifierList.size(); n++)
        if (m_Params->m_ModifierList[n].IsRequested()){
            wxString label(m_Params->m_ModifierList[n].Label());
            ModStrings.Add(label);
        }

    m_Firstmod->Append(ModStrings);
    if (m_WithComboboxes > 1){
        m_Secondmod->Append(ModStrings);
        if (m_WithComboboxes > 2)
            m_Thirdmod->Append(ModStrings);
    }
    
    Layout();
    Fit();
    Refresh();
}


size_t CPrefixDeflinedlg::x_FindModifierBasedOnLabel(const string& label)
{
    size_t index;
    for (index = 0; index < m_Params->m_ModifierList.size(); index++)
        if (m_Params->m_ModifierList[index].IsRequested())
            if (NStr::Equal(label, m_Params->m_ModifierList[index].Label(), NStr::eNocase)){
                return index;
            }
    return index;
}


CRef<CCmdComposite> CPrefixDeflinedlg::GetCommand()
{
    CRef<CCmdComposite> composite(new CCmdComposite("Prefix Modifiers to Definition Line"));
    
    string modifier;
    CAutoDefSourceDescription::TAvailableModifierVector mod_vector;
    
    if (m_WithComboboxes >0 ){
        modifier = ToStdString(m_Firstmod->GetStringSelection());
        if (!NStr::IsBlank(modifier)){
            size_t selected = x_FindModifierBasedOnLabel(modifier);
            mod_vector.push_back(CAutoDefAvailableModifier(m_Params->m_ModifierList[selected]));
        }
        if (m_WithComboboxes > 1){
            modifier = ToStdString(m_Secondmod->GetStringSelection());
            if (!NStr::IsBlank(modifier)){
                size_t selected = x_FindModifierBasedOnLabel(modifier);
                mod_vector.push_back(CAutoDefAvailableModifier(m_Params->m_ModifierList[selected]));
            }
            
            if (m_WithComboboxes > 2){
                modifier = ToStdString(m_Thirdmod->GetStringSelection());
                if (!NStr::IsBlank(modifier)){
                    size_t selected = x_FindModifierBasedOnLabel(modifier);
                    mod_vector.push_back(CAutoDefAvailableModifier(m_Params->m_ModifierList[selected]));
                }
            }
        }
    }
    
    if (AddModVectorToDefline(m_TopSeqEntry, composite, mod_vector, m_UseLabels->GetValue(), m_PrefixTaxname->GetValue()))
        return composite;
    else{
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }
}

bool AddOrgToDefline(CSeq_entry_Handle seh, CCmdComposite* composite)
{
    bool modified = false;
    CSeq_entry_CI entry_it(seh, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry);
    for ( ; entry_it ; ++entry_it){
        if (entry_it->IsSeq() && entry_it->GetSeq().IsNa()){
            bool found_title = false, found_org = false;
            string title, orgname;
                    
            CSeqdesc_CI title_ci(*entry_it, CSeqdesc::e_Title, 1);
            if (title_ci && !NStr::IsBlank(title_ci->GetTitle())){
                title = title_ci->GetTitle();
                found_title = true;
            }
            // find organism name at any level
            for (CSeqdesc_CI org_ci(*entry_it, CSeqdesc::e_Source); org_ci && !found_org; ++org_ci)
                if (org_ci->GetSource().IsSetTaxname() && !NStr::IsBlank(org_ci->GetSource().GetTaxname())){
                    orgname = org_ci->GetSource().GetTaxname();
                    found_org = true;
                }
                
            if (found_org){
                if (!found_title){ // get the next level title:
                    for (CSeqdesc_CI uptitle_ci(*entry_it, CSeqdesc::e_Title); uptitle_ci && !found_title ; ++uptitle_ci)
                        if (!NStr::IsBlank(uptitle_ci->GetTitle())){
                            title = uptitle_ci->GetTitle();
                            found_title = true;
                        }
                }
                if (found_title){ // the title has been found
                    string new_title = orgname + " " + title;
                    CRef<CSeqdesc> new_desc(new CSeqdesc());
                    new_desc->SetTitle(new_title);
                    if (title_ci){ // there is a title descriptor on the record
                        CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(title_ci.GetSeq_entry_Handle(), *title_ci, *new_desc));
                        composite->AddCommand(*cmd);
                        modified = true;
                    }else{ // make a title descriptor
                        CRef<CCmdCreateDesc> cmd(new CCmdCreateDesc(*entry_it, *new_desc));
                        composite->AddCommand(*cmd);
                        modified = true;
                    }
                }
            }
        }
        else if (entry_it->IsSet() && entry_it->GetSet().IsSetClass() && CBioseq_set::NeedsDocsumTitle(entry_it->GetSet().GetClass())) {
            bool found_title = false;
            string title;
                        
            CSeqdesc_CI title_ci(*entry_it, CSeqdesc::e_Title, 1);
            if (title_ci && !NStr::IsBlank(title_ci->GetTitle())){
                found_title = true;
                title = title_ci->GetTitle();
            }
            
            if (found_title && title_ci){ // it does not add a new set title
                string new_title = CAutoDefWithTaxonomy::GetDocsumOrgDescription(*entry_it) + " " + title;
                CRef<CSeqdesc> new_desc(new CSeqdesc());
                new_desc->SetTitle(new_title);
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(title_ci.GetSeq_entry_Handle(), *title_ci, *new_desc));
                composite->AddCommand(*cmd);
                modified = true;
            }
        } 
    }
    return modified;
}

bool FindModifierOnBioseq(const objects::CBioseq_CI& bioseq_ci, const CAutoDefAvailableModifier& mod, string& modifier, bool show_label)
{
    // find modifier name at any level on this bioseq
    if (!bioseq_ci)
        return false;
    
    bool found_mod = false;
    if (mod.IsOrgMod()){
        for (CSeqdesc_CI mod_ci(*bioseq_ci, CSeqdesc::e_Source); mod_ci && !found_mod; ++mod_ci){
            if (mod_ci->GetSource().IsSetOrgMod()){
                COrgName_Base::TMod::const_iterator it = mod_ci->GetSource().GetOrgname().GetMod().begin();
                while (it != mod_ci->GetSource().GetOrgname().GetMod().end() && ! found_mod){
                    if ((**it).IsSetSubtype() && (**it).GetSubtype() == mod.GetOrgModType()){
                        if ((**it).IsSetSubname() && !NStr::IsBlank((**it).GetSubname())){
                            modifier = (**it).GetSubname() + " " + modifier;
                            found_mod = true;
                        }
                    }
                    ++it;
                }
            }
        }
    } else{ // subsource type
        for (CSeqdesc_CI mod_ci(*bioseq_ci, CSeqdesc::e_Source); mod_ci && !found_mod; ++mod_ci){
            if (mod_ci->GetSource().IsSetSubtype()){
                CBioSource_Base::TSubtype::const_iterator it = mod_ci->GetSource().GetSubtype().begin();
                while (it != mod_ci->GetSource().GetSubtype().end() && ! found_mod){
                    if ((**it).IsSetSubtype() && (**it).GetSubtype() == mod.GetSubSourceType()){
                        if ((**it).IsSetName() && !NStr::IsBlank((**it).GetName())){
                            modifier = (**it).GetName() + " " + modifier;
                            found_mod = true;
                        }
                    }
                    ++it;
                }
            }
        }
    }
    
    if (found_mod && show_label)
        modifier = mod.Label() + " " + modifier;
        
    return found_mod;
}

bool AddModToDefline(CSeq_entry_Handle entry, CCmdComposite* composite,
                    const CAutoDefAvailableModifier& mod, bool show_label)
{
    bool modified = false;
    CBioseq_CI b_iter(entry, CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter){
        bool found_title = false;
        string title, mod_name;
        
        CSeqdesc_CI title_ci(*b_iter, CSeqdesc::e_Title, 1);
        if (title_ci && !NStr::IsBlank(title_ci->GetTitle())){
            title = title_ci->GetTitle();
            found_title = true;
        }
        
        if (FindModifierOnBioseq(b_iter, mod, mod_name, show_label)){
            if (!found_title){ // get the next level title:
                for (CSeqdesc_CI uptitle_ci(*b_iter, CSeqdesc::e_Title); uptitle_ci && !found_title ; ++uptitle_ci)
                    if (!NStr::IsBlank(uptitle_ci->GetTitle())){
                        title = uptitle_ci->GetTitle();
                        found_title = true;
                    }
            }		
            if (found_title){ // the title has been found
                string new_title = mod_name + title;
                CRef<CSeqdesc> new_desc(new CSeqdesc());
                new_desc->SetTitle(new_title);
                if (title_ci){ // there is a title descriptor on the record
                    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(title_ci.GetSeq_entry_Handle(), *title_ci, *new_desc));
                    composite->AddCommand(*cmd);
                    modified = true;
                }else{ // make a title descriptor
                    CRef<CCmdCreateDesc> cmd(new CCmdCreateDesc(b_iter->GetSeq_entry_Handle(), *new_desc));
                    composite->AddCommand(*cmd);
                    modified = true;
                }
            }
        }
    }
    
    return modified;
}

bool AddModVectorToDefline(CSeq_entry_Handle entry, CCmdComposite* composite, 
                    const CAutoDefSourceDescription::TAvailableModifierVector &mod_vector, 
                    bool show_label, bool show_org)
{
    /* prefixes the list of modifiers maybe with the optional organism name to all nt sequences only*/
    bool modified = false;
    CBioseq_CI b_iter(entry, CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter){
        bool found_title = false, found_prefix = false;
        string title, prefix;
        CSeqdesc_CI title_ci(*b_iter, CSeqdesc::e_Title, 1);
        if (title_ci && !NStr::IsBlank(title_ci->GetTitle())){
            title = title_ci->GetTitle();
            found_title = true;
        }
        
        CAutoDefSourceDescription::TAvailableModifierVector::const_reverse_iterator mod_it= mod_vector.rbegin();
        for ( ; mod_it != mod_vector.rend() ; ++mod_it){ // find modifier name at any level
            if (FindModifierOnBioseq(b_iter, *mod_it, prefix, show_label))
                found_prefix = true;
        }
        
        if (show_org){ 
            bool found_org = false;
            for (CSeqdesc_CI org_ci(*b_iter, CSeqdesc::e_Source); org_ci && !found_org; ++org_ci)
                if (org_ci->GetSource().IsSetTaxname() && !NStr::IsBlank(org_ci->GetSource().GetTaxname())){
                    prefix = org_ci->GetSource().GetTaxname() + " " + prefix;
                    found_prefix = found_org = true;
                }
        }
        
        if (found_prefix){
            if (!found_title){ 
                for (CSeqdesc_CI uptitle_ci(*b_iter, CSeqdesc::e_Title); uptitle_ci && !found_title ; ++uptitle_ci)
                    if (!NStr::IsBlank(uptitle_ci->GetTitle())){
                        title = uptitle_ci->GetTitle();
                        found_title = true;
                    }
            }
            if (found_title){
                string new_title = prefix + title;
                CRef<CSeqdesc> new_desc(new CSeqdesc());
                new_desc->SetTitle(new_title);
                if (title_ci){
                    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(title_ci.GetSeq_entry_Handle(), *title_ci, *new_desc));
                    composite->AddCommand(*cmd);
                    modified = true;
                }else{ 
                    CRef<CCmdCreateDesc> cmd(new CCmdCreateDesc(b_iter->GetSeq_entry_Handle(), *new_desc));
                    composite->AddCommand(*cmd);
                    modified = true;
                }
            }
                    
        }
    }
    return modified;
}

END_NCBI_SCOPE

