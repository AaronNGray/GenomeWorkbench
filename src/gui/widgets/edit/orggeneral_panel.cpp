/*  $Id: orggeneral_panel.cpp 44057 2019-10-17 14:39:43Z asztalos $
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
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objtools/edit/source_edit.hpp>

#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/orggeneral_panel.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * COrgGeneralPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COrgGeneralPanel, wxPanel )


/*
 * COrgGeneralPanel event table definition
 */

BEGIN_EVENT_TABLE( COrgGeneralPanel, wxPanel )

////@begin COrgGeneralPanel event table entries
////@end COrgGeneralPanel event table entries

END_EVENT_TABLE()


/*
 * COrgGeneralPanel constructors
 */

COrgGeneralPanel::COrgGeneralPanel()
{
    Init();
}

COrgGeneralPanel::COrgGeneralPanel( wxWindow* parent, 
    ICommandProccessor* cmdproc,
    CSeq_entry_Handle seh,
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * COrgGeneralPanel creator
 */

bool COrgGeneralPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COrgGeneralPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end COrgGeneralPanel creation
    return true;
}


/*
 * COrgGeneralPanel destructor
 */

COrgGeneralPanel::~COrgGeneralPanel()
{
////@begin COrgGeneralPanel destruction
////@end COrgGeneralPanel destruction
}


/*
 * Member initialisation
 */

void COrgGeneralPanel::Init()
{
////@begin COrgGeneralPanel member initialisation
    m_Taxname = NULL;
    m_Strain = NULL;
    m_Isolate = NULL;
    m_Cultivar = NULL;
    m_Breed = NULL;
////@end COrgGeneralPanel member initialisation
}


/*
 * Control creation for COrgGeneralPanel
 */

void COrgGeneralPanel::CreateControls()
{    
////@begin COrgGeneralPanel content construction
    COrgGeneralPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("Organism*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText2, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifdef __WXMSW__
    m_Taxname = new wxTextCtrl(itemPanel1, ID_ORGTAXNAME, wxEmptyString, wxDefaultPosition, wxSize(240, -1), 0);
#else
    m_Taxname = new wxTextCtrl(itemPanel1, ID_ORGTAXNAME, wxEmptyString, wxDefaultPosition, wxSize(270, -1), 0);
#endif
    itemFlexGridSizer1->Add(m_Taxname, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("strain**"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText3, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Strain = new wxTextCtrl( itemPanel1, ID_ORGSTRAIN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_Strain, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("isolate**"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Isolate = new wxTextCtrl( itemPanel1, ID_ORGISOLATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_Isolate, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("cultivar**"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Cultivar = new wxTextCtrl( itemPanel1, ID_ORGCULTIVAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(m_Cultivar, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText(itemPanel1, wxID_STATIC, _("breed**"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer1->Add(itemStaticText6, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_Breed = new wxTextCtrl(itemPanel1, ID_ORGBREED, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer1->Add(m_Breed, 0, wxGROW | wxALIGN_CENTER_VERTICAL | wxALL, 5);


    itemBoxSizer2->AddStretchSpacer();

    wxStaticText* itemStaticText7 = new wxStaticText(itemPanel1, wxID_STATIC, _("**At least one field is required"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(itemStaticText7, 0, wxALIGN_RIGHT | wxALL, 5);


////@end COrgGeneralPanel content construction
}


/*
 * Should we show tooltips?
 */

bool COrgGeneralPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap COrgGeneralPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin COrgGeneralPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end COrgGeneralPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon COrgGeneralPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin COrgGeneralPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end COrgGeneralPanel icon retrieval
}

void COrgGeneralPanel::ApplyBioSource(CBioSource& src)
{
    m_Source.Reset(&src);
    TransferDataToWindow();
}


void COrgGeneralPanel::x_Reset()
{
    m_Taxname->SetValue(wxT(""));
    m_Strain->SetValue(wxT(""));
    m_Isolate->SetValue(wxT(""));
    m_Cultivar->SetValue(wxT(""));
    m_Breed->SetValue(wxT(""));
}


bool COrgGeneralPanel::TransferDataToWindow()
{
    x_Reset();
    if (m_Source) {
        if (m_Source->IsSetTaxname()) {
            m_Taxname->SetValue(ToWxString(m_Source->GetTaxname()));
        }
        if (m_Source->IsSetOrgMod()) {
            for (auto& it : m_Source->GetOrg().GetOrgname().GetMod()) {
                if (it->IsSetSubtype() && it->IsSetSubname()) {
                    auto subtype = it->GetSubtype();
                    switch (subtype) {
                    case COrgMod::eSubtype_strain:
                        m_Strain->SetValue(ToWxString(it->GetSubname()));
                        break;
                    case COrgMod::eSubtype_isolate:
                        m_Isolate->SetValue(ToWxString(it->GetSubname()));
                        break;
                    case COrgMod::eSubtype_cultivar:
                        m_Cultivar->SetValue(ToWxString(it->GetSubname()));
                        break;
                    case COrgMod::eSubtype_breed:
                        m_Breed->SetValue(ToWxString(it->GetSubname()));
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    return true;
}

COrgName::TMod::iterator s_UpdateSubtype
(COrgName::TMod& orgmod_list, COrgName::TMod::iterator it, wxTextCtrl* ctrl, bool& changed)
{
    if (ctrl->IsEmpty()) {
        it = orgmod_list.erase(it);
        changed = true;
    } else {
        if (!(*it)->IsSetSubname() || !NStr::Equal((*it)->GetSubname(), ToStdString(ctrl->GetValue()))) {
            (*it)->SetSubname(ToStdString(ctrl->GetValue()));
            changed = true;
        }
        ++it;
    }
    return it;
}

bool COrgGeneralPanel::x_ApplyChangesToSource(CBioSource& src)
{
    bool any_change = false;

    COrg_ref& org = src.SetOrg();

    // get taxname
    const string taxname = ToStdString(m_Taxname->GetValue());
    if (taxname.empty() && org.IsSetTaxname()) {
        org.ResetTaxname();
        any_change = true;
    } else if (!org.IsSetTaxname() || !NStr::Equal(taxname, org.GetTaxname())) {
        org.SetTaxname(taxname);
        any_change = true;
    }

    bool found_strain = false;
    bool found_isolate = false;
    bool found_cultivar = false;
    bool found_breed = false;

    auto& orgmodlist = org.SetOrgname().SetMod();
    auto mod = orgmodlist.begin();
    while (mod != orgmodlist.end()) {
        if ((*mod)->IsSetSubtype() && (*mod)->IsSetSubname()) {
            auto subtype = (*mod)->GetSubtype();
            switch (subtype) {
            case COrgMod::eSubtype_strain:
                found_strain = true;
                mod = s_UpdateSubtype(orgmodlist, mod, m_Strain, any_change);
                break;
            case COrgMod::eSubtype_isolate:
                found_isolate = true;
                mod = s_UpdateSubtype(orgmodlist, mod, m_Isolate, any_change);
                break;
            case COrgMod::eSubtype_cultivar:
                found_cultivar = true;
                mod = s_UpdateSubtype(orgmodlist, mod, m_Cultivar, any_change);
                break;
            case COrgMod::eSubtype_breed:
                found_breed = true;
                mod = s_UpdateSubtype(orgmodlist, mod, m_Breed, any_change);
                break;
            default:
                ++mod;
            }
        }
        else {
            ++mod;
        }
    }

    // Create new orgmods for items that have values but weren't already present 
    if (!found_strain && !m_Strain->IsEmpty()) {
        orgmodlist.push_back(CRef<COrgMod>(new COrgMod(COrgMod::eSubtype_strain, ToStdString(m_Strain->GetValue()))));
        any_change = true;
    }
    if (!found_isolate && !m_Isolate->IsEmpty()) {
        orgmodlist.push_back(CRef<COrgMod>(new COrgMod(COrgMod::eSubtype_isolate, ToStdString(m_Isolate->GetValue()))));
        any_change = true;
    }
    if (!found_cultivar && !m_Cultivar->IsEmpty()) {
        orgmodlist.push_back(CRef<COrgMod>(new COrgMod(COrgMod::eSubtype_cultivar, ToStdString(m_Cultivar->GetValue()))));
        any_change = true;
    }
    if (!found_breed && !m_Breed->IsEmpty()) {
        orgmodlist.push_back(CRef<COrgMod>(new COrgMod(COrgMod::eSubtype_breed, ToStdString(m_Breed->GetValue()))));
        any_change = true;
    }

    // reset orgmods if none found
    if (any_change && orgmodlist.empty()) {
        org.SetOrgname().ResetMod();
    }
    return any_change;
}

bool COrgGeneralPanel::x_HasTaxnameChanged(const CBioSource& orig_src, const CBioSource& edited_src)
{
    if (orig_src.IsSetTaxname() && edited_src.IsSetTaxname() && !NStr::EqualCase(orig_src.GetTaxname(), edited_src.GetTaxname()))
        return true;

    if (!orig_src.IsSetTaxname() && edited_src.IsSetTaxname())
        return true;

    if (orig_src.IsSetTaxname() && !edited_src.IsSetTaxname())
        return true;
    
    return false;
}

bool COrgGeneralPanel::TransferDataFromWindow()
{
    x_ApplyChangesToSource(*m_Source);

    return true;
}


void COrgGeneralPanel::ApplyCommand()
{
    CRef<CSeqdesc> empty(new CSeqdesc());
    empty->SetSource();

    CRef<CSeqdesc> new_desc(new CSeqdesc());
    new_desc->SetSource();
    x_ApplyChangesToSource(new_desc->SetSource());

    if (new_desc->Equals(*empty))
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("update source"));
    bool any_changes = false;

    bool taxname_changed = false;
    for (CBioseq_CI bi(m_Seh, CSeq_inst::eMol_na); bi; ++bi) {
        CSeqdesc_CI di(*bi, CSeqdesc::e_Source);
        if (di) {
            // edit existing descriptor
            CRef<CSeqdesc> cpy(new CSeqdesc());
            cpy->Assign(*di);
            
            x_ApplyChangesToSource(cpy->SetSource());
            taxname_changed = x_HasTaxnameChanged(di->GetSource(), cpy->GetSource());
            if (taxname_changed) {
                edit::CleanupForTaxnameChange(cpy->SetSource());
                if (cpy->GetSource().IsSetLineage())
                    cpy->SetSource().SetOrg().SetOrgname().ResetLineage();
            }
            if (!di->Equals(*cpy)) {
                CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *cpy));
                cmd->AddCommand(*chg);
                any_changes = true;
            }
        } else {
            // create new source descriptor on this sequence or on the nuc-prot that contains it
            CRef<CSeqdesc> cpy(new CSeqdesc());
            cpy->Assign(*new_desc);
            CBioseq_set_Handle parent = bi->GetParentBioseq_set();
            auto entry = bi->GetParentEntry();
            if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) 
              entry = parent.GetParentEntry();
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(entry, *cpy));
            cmd->AddCommand(*cmdAddDesc);
            any_changes = true;
            taxname_changed = true;
        }
    }

    if (any_changes) {
        m_CmdProcessor->Execute(cmd);
    }

    if (taxname_changed) {
        try {
            CRef<CCmdComposite> taxlookup_cmd = TaxonomyLookupCommand(m_Seh);
            if (taxlookup_cmd)
                m_CmdProcessor->Execute(taxlookup_cmd);
        }
        catch (const CException& e) {
            LOG_POST(Error << e.GetMsg());
        }
    }
}

void COrgGeneralPanel::ReportMissingFields(string &text)
{
    if (m_Taxname->GetValue().IsEmpty())
        text += "Organism\n";

    if (m_Strain->GetValue().IsEmpty() && m_Isolate->GetValue().IsEmpty() && m_Cultivar->GetValue().IsEmpty() && m_Breed->GetValue().IsEmpty())
        text += "At least one field is required: strain, isolate, cultivar, breed\n";
}

END_NCBI_SCOPE
