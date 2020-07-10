/*  $Id: single_sequence_panel.cpp 43202 2019-05-28 18:05:59Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/edit/single_sequence_panel.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seq/MolInfo.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>


#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CSingleSequencePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleSequencePanel, wxPanel )


/*
 * CSingleSequencePanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleSequencePanel, wxPanel )

////@begin CSingleSequencePanel event table entries
    EVT_CHOICE(ID_SINGLESEQID, CSingleSequencePanel::OnChangeSeqId)
////@end CSingleSequencePanel event table entries

END_EVENT_TABLE()


/*
 * CSingleSequencePanel constructors
 */

CSingleSequencePanel::CSingleSequencePanel()
    : m_is_organelle(false)
{
    Init();
}

CSingleSequencePanel::CSingleSequencePanel( wxWindow* parent, objects::CSeq_entry_Handle seh, objects::CSubSource::ESubtype subtype, bool is_organelle, 
                                            const vector<objects::CBioSource::EGenome> &organelle_types,wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) 
    : m_Seh(seh), m_Subtype(subtype), m_is_organelle(is_organelle), m_organelle_types(organelle_types)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSingleSequencePanel creator
 */

bool CSingleSequencePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleSequencePanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleSequencePanel creation
    return true;
}


/*
 * CSingleSequencePanel destructor
 */

CSingleSequencePanel::~CSingleSequencePanel()
{
////@begin CSingleSequencePanel destruction
////@end CSingleSequencePanel destruction
}


/*
 * Member initialisation
 */

void CSingleSequencePanel::Init()
{
////@begin CSingleSequencePanel member initialisation
    m_SeqId = NULL;
    m_LengthLabel = NULL;
    m_NameCtrl = NULL;
    m_OrganelleCtrl = NULL;
    m_Complete = NULL;
    m_Circular = NULL;
////@end CSingleSequencePanel member initialisation
}


/*
 * Control creation for CSingleSequencePanel
 */

void CSingleSequencePanel::CreateControls()
{    
////@begin CSingleSequencePanel content construction
    CSingleSequencePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxArrayString m_SeqIdStrings;
    m_SeqIdStrings.Add(wxT(""));
    // populate seq-id strings
    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        string id_str = bi->GetSeqId()->AsFastaString();
        m_SeqIdStrings.Add(ToWxString(id_str));
    }

    m_SeqId = new wxChoice( itemPanel1, ID_SINGLESEQID, wxDefaultPosition, wxSize(130, -1), m_SeqIdStrings, 0 );
    itemBoxSizer1->Add(m_SeqId, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_LengthLabel = new wxStaticText( itemPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTRE);
    itemBoxSizer1->Add(m_LengthLabel, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    if (m_is_organelle)
    {
        wxArrayString choices;
        for (auto o : m_organelle_types)
        {
            choices.Add(wxString(objects::CBioSource::ENUM_METHOD_NAME(EGenome)()->FindName(o, true)));
        }
        m_OrganelleCtrl = new wxChoice(itemPanel1, ID_ORGANELLE_CHOICE, wxDefaultPosition, wxDefaultSize, choices, 0);
#ifdef __WXOSX_COCOA__
        m_OrganelleCtrl->SetSelection(-1);
#endif
        itemBoxSizer1->Add(m_OrganelleCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    }
    else
    {
#ifdef __WXMSW__
        m_NameCtrl = new wxTextCtrl(itemPanel1, ID_SEQNAMETEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0);
#else
        m_NameCtrl = new wxTextCtrl(itemPanel1, ID_SEQNAMETEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(120, -1), 0);
#endif
        
        itemBoxSizer1->Add(m_NameCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    }

    itemBoxSizer1->Add(30, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Complete = new wxCheckBox( itemPanel1, ID_SEQCOMPLETECHK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Complete->SetValue(false);
    itemBoxSizer1->Add(m_Complete, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    itemBoxSizer1->Add(30, 5, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_Circular = new wxCheckBox( itemPanel1, ID_SEQCIRCULARCHK, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Circular->SetValue(false);
    itemBoxSizer1->Add(m_Circular, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    itemBoxSizer1->Add(13, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleSequencePanel content construction
    if (m_Subtype == objects::CSubSource::eSubtype_plasmid_name) {
        m_NameCtrl->SetToolTip(wxT("Plasmid names must begin with 'p'. However, use 'unnamed' if the name \
is not determined; use 'unnamed1' and 'unnamed2', etc(without spaces) \
if there are multiple plasmids whose names are not determined."));
    } else if (m_Subtype == objects::CSubSource::eSubtype_chromosome) {
        m_NameCtrl->SetToolTip(wxT("Use the community accepted values for the chromosome or \
linkage group name, eg 1 or IX or LG3"));
        m_Complete->SetToolTip(wxT("This sequence represents the chromosome, although it may still have \
gaps or be missing telomere or centromere sequence."));
    }
}


void CSingleSequencePanel::EnableTextCtrl(bool value)
{
    if (m_NameCtrl)
        m_NameCtrl->Enable(value);
}


/*
 * Should we show tooltips?
 */

bool CSingleSequencePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSingleSequencePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleSequencePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleSequencePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSingleSequencePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleSequencePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleSequencePanel icon retrieval
}

bool CSingleSequencePanel::NeedsRow(objects::CBioseq_Handle bh, objects::CSubSource::ESubtype subtype)
{
    objects::CSeqdesc_CI di(bh, objects::CSeqdesc::e_Source);
    if (di) {
        const objects::CBioSource& src = di->GetSource();
        if (subtype == objects::CSubSource::eSubtype_plasmid_name
            && src.IsSetGenome() && src.GetGenome() == objects::CBioSource::eGenome_plasmid) {
            return true;
        } else if (subtype == objects::CSubSource::eSubtype_chromosome
            && src.IsSetGenome() && src.GetGenome() == objects::CBioSource::eGenome_chromosome) {
            return true;
        }

        if (di->GetSource().IsSetSubtype()) {
            for (auto s : di->GetSource().GetSubtype()) {
                if (s->IsSetSubtype() && s->GetSubtype() == subtype &&
                    s->IsSetName() && !NStr::IsBlank(s->GetName())) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool CSingleSequencePanel::NeedsOrganelleRow(objects::CBioseq_Handle bh, const vector<objects::CBioSource::EGenome> &organelle_types)
{
    objects::CSeqdesc_CI di(bh, objects::CSeqdesc::e_Source);
    if (di && di->GetSource().IsSetGenome()) {
        auto it = find(organelle_types.begin(), organelle_types.end(), di->GetSource().GetGenome());
        if (it != organelle_types.end())
            return true;
    }
    return false;
}

bool s_HasGaps(objects::CBioseq_Handle bh)
{
    const objects::CBioseq& seq = *(bh.GetCompleteBioseq());
    if (!seq.IsSetInst() || !seq.GetInst().IsSetExt() || 
        !seq.GetInst().GetExt().IsDelta() || 
        !seq.GetInst().GetExt().GetDelta().IsSet()) {
        return false;
    }

    bool found_gap = false;
    for (auto it : seq.GetInst().GetExt().GetDelta().Get()) {
        if (it->IsLiteral()) {
            if (!it->GetLiteral().IsSetSeq_data()) {
                found_gap = true;
                break;
            } else if (it->GetLiteral().GetSeq_data().IsGap()) {
                found_gap = true;
                break;
            }
        }
    }

    return found_gap;
}


void CSingleSequencePanel::SetValue(objects::CBioseq_Handle bh)
{
    m_Circular->SetValue(bh.IsSetInst_Topology() && bh.GetInst_Topology() == objects::CSeq_inst::eTopology_circular);

    m_LengthLabel->SetLabel(ToWxString(NStr::NumericToString(bh.GetBioseqLength())));

    bool is_complete = false;
    objects::CSeqdesc_CI di(bh, objects::CSeqdesc::e_Source);
    if (di && di->GetSource().IsSetSubtype()) {
        const objects::CBioSource& src = di->GetSource();
        if (m_Subtype == objects::CSubSource::eSubtype_chromosome && src.IsSetGenome() && src.GetGenome() == objects::CBioSource::eGenome_chromosome) {
            is_complete = true;
        }
        if (m_NameCtrl) {
            bool found_name = false;
            for (auto s : src.GetSubtype()) {
                if (s->IsSetSubtype() && s->GetSubtype() == m_Subtype &&
                    s->IsSetName() && !NStr::IsBlank(s->GetName())) {
                    m_NameCtrl->SetValue(ToWxString(s->GetName()));
                    found_name = true;
                }
            }
            if (!found_name) {
                m_NameCtrl->SetValue(wxT(""));
            }
        }
    }
    if (m_is_organelle &&  di->GetSource().IsSetGenome())
    {
        auto it = find(m_organelle_types.begin(), m_organelle_types.end(), di->GetSource().GetGenome());
        if (it != m_organelle_types.end())
        {
            int sel = std::distance(m_organelle_types.begin(), it);
            m_OrganelleCtrl->SetSelection(sel);
        }
    }
    if (m_Subtype == objects::CSubSource::eSubtype_plasmid_name || m_is_organelle) {
        bool has_gaps = s_HasGaps(bh);
        m_Complete->Enable(!has_gaps);
        if (!has_gaps) {
            objects::CSeqdesc_CI mi(bh, objects::CSeqdesc::e_Molinfo);
            if (mi && mi->GetMolinfo().IsSetCompleteness() && mi->GetMolinfo().GetCompleteness() == objects::CMolInfo::eCompleteness_complete) {
                is_complete = true;
            }
        }
    }
    m_Complete->SetValue(is_complete);

    m_SeqId->SetStringSelection(ToWxString(bh.GetSeqId()->AsFastaString()));
}


void CSingleSequencePanel::ClearValue()
{
    m_Circular->SetValue(false);
    m_Complete->SetValue(false);
    m_LengthLabel->SetLabel(wxT(""));
    if (m_NameCtrl)
        m_NameCtrl->SetValue(wxT(""));
    if (m_OrganelleCtrl)
        m_OrganelleCtrl->SetSelection(wxNOT_FOUND);
    m_SeqId->SetSelection(0);
}


void CSingleSequencePanel::OnChangeSeqId(wxCommandEvent& event)
{
    string seqid = GetSeqId();
    bool found = false;
    if (!seqid.empty()) {
        CRef<objects::CSeq_id> id(new objects::CSeq_id(seqid));
        objects::CBioseq_Handle bh = m_Seh.GetScope().GetBioseqHandle(*id);
        if (bh) {
            SetValue(bh);
            found = true;
        }
    }
    if (!found) {
        ClearValue();
    }
}

string CSingleSequencePanel::GetModifierValue() 
{ 
    string val;
    if (m_NameCtrl)
        val =  ToStdString(m_NameCtrl->GetValue()); 
    return val;
}

string CSingleSequencePanel::GetOrganelleValue() 
{ 
    string val;
    if (m_OrganelleCtrl)
        val =  ToStdString(m_OrganelleCtrl->GetStringSelection()); 
    return val;
}

bool CSingleSequencePanel::AddToUpdateCommand(CCmdComposite& cmd)
{
    bool any_changes = false;
    string seqid = GetSeqId();
    if (!seqid.empty()) {
        CRef<objects::CSeq_id> id(new objects::CSeq_id(seqid));
        objects::CBioseq_Handle bh = m_Seh.GetScope().GetBioseqHandle(*id);

        // value
        string val_name = GetModifierValue();
        objects::CSeqdesc_CI oldsrc(bh, objects::CSeqdesc::e_Source);
        if (oldsrc) {
            bool change_val = false;
            // edit existing source descriptor
            CRef<objects::CSeqdesc> new_src(new objects::CSeqdesc());
            new_src->Assign(*oldsrc);
            objects::CBioSource& bsrc = new_src->SetSource();

            if (bsrc.IsSetSubtype()) {
                bool found = false;
                auto sit = bsrc.SetSubtype().begin();
                while (sit != bsrc.SetSubtype().end()) {
                    if ((*sit)->IsSetSubtype() && ((*sit)->GetSubtype() == m_Subtype || m_is_organelle)) {
                        found = true;
                        if (val_name.empty()) {
                            sit = bsrc.SetSubtype().erase(sit);
                            change_val = true;
                        }
                        else {
                            if (!(*sit)->IsSetName() || !NStr::Equal((*sit)->GetName(), val_name)) {
                                (*sit)->SetName(val_name);
                                change_val = true;
                            }
                            sit++;
                        }
                    }
                    else {
                        sit++;
                    }
                }
                if (!found && !val_name.empty()) {
                    bsrc.SetSubtype().push_back(CRef<objects::CSubSource>(new objects::CSubSource(m_Subtype, val_name)));
                    change_val = true;
                }
                if (bsrc.GetSubtype().empty()) {
                    bsrc.ResetSubtype();
                    change_val = true;
                }
            } else {
                if (!val_name.empty()) {
                    // only need to make change if there is a value name where there wasn't one before
                    bsrc.SetSubtype().push_back(CRef<objects::CSubSource>(new objects::CSubSource(m_Subtype, val_name)));
                    change_val = true;
                }
            }
            // also set location
            if (val_name.empty()) {
                if (m_Subtype == objects::CSubSource::eSubtype_plasmid_name && new_src->GetSource().IsSetGenome() &&
                    new_src->GetSource().GetGenome() == objects::CBioSource::eGenome_plasmid) {
                    new_src->SetSource().ResetGenome();
                    change_val = true;
                }
            } else {
                if (m_Subtype == objects::CSubSource::eSubtype_plasmid_name) {
                    if (!new_src->GetSource().IsSetGenome() || new_src->GetSource().GetGenome() != objects::CBioSource::eGenome_plasmid) {
                        new_src->SetSource().SetGenome(objects::CBioSource::eGenome_plasmid);
                        change_val = true;
                    }
                }
            } 
            if (m_is_organelle && m_OrganelleCtrl)
            {
                int sel = m_OrganelleCtrl->GetSelection();
                if (sel == wxNOT_FOUND)
                    new_src->SetSource().ResetGenome();
                else
                    new_src->SetSource().SetGenome(m_organelle_types[sel]);
            }
            if (!oldsrc->Equals(*new_src)) {
                CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(oldsrc.GetSeq_entry_Handle(), *oldsrc, *new_src));
                cmd.AddCommand(*chg);
                any_changes = true;
            }
        } else if (!val_name.empty()) {
            // create new source descriptor
            CRef<objects::CSeqdesc> new_src(new objects::CSeqdesc());
            new_src->SetSource().SetOrg();
            CRef<objects::CSubSource> subsrc(new objects::CSubSource(m_Subtype, val_name));
            new_src->SetSource().SetSubtype().push_back(subsrc);
            if (m_Subtype == objects::CSubSource::eSubtype_plastid_name) {
                new_src->SetSource().SetGenome(objects::CBioSource::eGenome_plasmid);
            }
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(bh.GetSeq_entry_Handle(), *new_src));
            cmd.AddCommand(*cmdAddDesc);
            any_changes = true;
        } else if (m_is_organelle && m_OrganelleCtrl && m_OrganelleCtrl->GetSelection() != wxNOT_FOUND) {
            // create new source descriptor
            int sel = m_OrganelleCtrl->GetSelection();
            CRef<objects::CSeqdesc> new_src(new objects::CSeqdesc());
            new_src->SetSource().SetGenome(m_organelle_types[sel]);
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(bh.GetSeq_entry_Handle(), *new_src));
            cmd.AddCommand(*cmdAddDesc);
            any_changes = true;
        }

        if (m_Subtype == objects::CSubSource::eSubtype_plasmid_name || m_is_organelle) {
            // completeness
            objects::CSeqdesc_CI oldmol(bh, objects::CSeqdesc::e_Molinfo);
            if (oldmol) {
                bool current_completeness = (oldmol->GetMolinfo().IsSetCompleteness() && oldmol->GetMolinfo().GetCompleteness() == objects::CMolInfo::eCompleteness_complete);
                if ((current_completeness && !IsComplete()) || (!current_completeness && IsComplete())) {
                    // edit existing molinfo descriptor
                    CRef<objects::CSeqdesc> new_mol(new objects::CSeqdesc());
                    new_mol->Assign(*oldmol);
                    if (IsComplete()) {
                        new_mol->SetMolinfo().SetCompleteness(objects::CMolInfo::eCompleteness_complete);
                    }
                    else {
                        new_mol->SetMolinfo().ResetCompleteness();
                    }
                    CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(oldmol.GetSeq_entry_Handle(), *oldmol, *new_mol));
                    cmd.AddCommand(*chg);
                    any_changes = true;
                }
            } else {
                if (IsComplete()) {
                    // create new molinfo descriptor
                    CRef<objects::CSeqdesc> new_mol(new objects::CSeqdesc());
                    new_mol->SetMolinfo().SetCompleteness(objects::CMolInfo::eCompleteness_complete);
                    CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(bh.GetSeq_entry_Handle(), *new_mol));
                    cmd.AddCommand(*cmdAddDesc);
                    any_changes = true;
                }
            }
        }


        // topology
        bool currently_circular = (bh.IsSetInst_Topology() && bh.GetInst_Topology() == objects::CSeq_inst::eTopology_circular);
        if ((currently_circular && !IsCircular()) || (!currently_circular && IsCircular())) {
            CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
            new_inst->Assign(bh.GetInst());
            if (IsCircular()) {
                new_inst->SetTopology(objects::CSeq_inst::eTopology_circular);
            }
            else {
                new_inst->ResetTopology();
            }

            CIRef<IEditCommand> chgInst(new CCmdChangeBioseqInst(bh, *new_inst));
            cmd.AddCommand(*chgInst);
            any_changes = true;
        }
    }
    return any_changes;
}


bool CSingleSequencePanel::RemoveNamedValue(CCmdComposite& cmd, objects::CSubSource::ESubtype subtype, objects::CBioseq_Handle bh)
{
    bool change_val = false;

    // take name off
    objects::CSeqdesc_CI oldsrc(bh, objects::CSeqdesc::e_Source);
    if (oldsrc && oldsrc->GetSource().IsSetSubtype()) {
        // edit existing source descriptor
        CRef<objects::CSeqdesc> new_src(new objects::CSeqdesc());
        new_src->Assign(*oldsrc);
        objects::CBioSource& bsrc = new_src->SetSource();

        auto sit = bsrc.SetSubtype().begin();
        while (sit != bsrc.SetSubtype().end()) {
            if ((*sit)->IsSetSubtype() && (*sit)->GetSubtype() == subtype) {
                sit = bsrc.SetSubtype().erase(sit);
                change_val = true;
            }
            else {
                sit++;
            }
        }
        if (bsrc.GetSubtype().empty()) {
            bsrc.ResetSubtype();
            change_val = true;
        }

        if (change_val) {
            CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(oldsrc.GetSeq_entry_Handle(), *oldsrc, *new_src));
            cmd.AddCommand(*chg);
        }

    }
    return change_val;
}

END_NCBI_SCOPE
