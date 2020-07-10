/*  $Id: edit_object_seq_desc.cpp 43602 2019-08-07 19:04:57Z filippov $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Colleen Bollin, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>


#include <gui/utils/command_processor.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <gui/objutils/seqdesc_title_edit.hpp>
#include <gui/objutils/seqdesc_title_change.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_del_desc.hpp>

#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_field.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <objtools/cleanup/cleanup.hpp>
#include <objtools/edit/text_object_description.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <wx/panel.h>
#include <wx/treebook.h>
#include <wx/msgdlg.h>
#include <wx/toplevel.h>

#include "descriptortext_panel.hpp"
#include <gui/widgets/edit/publicationtype_panel.hpp>
#include "molinfo_panel.hpp"
#include "structuredcomment_panel.hpp"
#include <gui/widgets/edit/src_desc_editor_panel.hpp>
#include <gui/objutils/utils.hpp>
#include <objtools/edit/apply_object.hpp>
#include <gui/widgets/edit/sequencingtechnologypanel.hpp>
#include <gui/widgets/edit/dblinkpanel.hpp>
#include <gui/widgets/edit/unverified_panel.hpp>
#include <gui/widgets/edit/authorized_access_panel.hpp>
#include <gui/widgets/edit/ref_gene_tracking_panel.hpp>
#include <gui/widgets/edit/assembly_tracking_panel.hpp>
#include <gui/widgets/edit/biosource_autocomplete.hpp>
#include <gui/widgets/edit/non_ascii_replacement_dlg.hpp>

#include "loc_gcode_panel.hpp"
#include "biosourcepanel.hpp"
#include "src_other_pnl.hpp"
#include <gui/widgets/edit/edit_object_seq_desc.hpp>

#ifdef _DEBUG
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#endif

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static bool s_IsTpa(const CUser_object& user)
{
    if (user.IsSetType() && user.GetType().IsStr()
        && NStr::EqualNocase(user.GetType().GetStr(), "TpaAssembly")) {
        return true;
    } else {
        return false;
    }
}


static bool s_IsTpa(const CSeqdesc& desc)
{
    if (desc.IsUser() && s_IsTpa(desc.GetUser())) {
        return true;
    } else {
        return false;
    }
}


wxWindow* CEditObjectSeq_desc::CreateWindow(wxWindow* parent)
{
    if (m_Window) {
        NCBI_THROW(CException, eUnknown, 
                   "CEditObjectSeq_desc::CreateWindow - Window already created!");
    }

    wxTopLevelWindow* gui_widget = dynamic_cast<wxTopLevelWindow*>(parent);
    IHelpUrlClient* help_client = nullptr;
    if (m_CreateMode)
        help_client = dynamic_cast<IHelpUrlClient*>(parent);

    const CSeqdesc& seq_desc = dynamic_cast<const CSeqdesc&>(*m_Object);

    m_EditedDescriptor.Reset((CSerialObject*)CSeqdesc::GetTypeInfo()->Create());
    m_EditedDescriptor->Assign(seq_desc);
    m_EditedBioseq.Reset();
    CConstRef<CBioseq> orig_bioseq(NULL);
    // need to get bioseq for descriptor if editing molinfo
    if (seq_desc.IsMolinfo() || s_IsTpa(seq_desc)) {
        orig_bioseq = GetBioseqForSeqdesc(m_Scope, seq_desc);
        if (!orig_bioseq) {
            if (m_SEH.IsSeq()) {
                orig_bioseq = m_SEH.GetSeq().GetCompleteBioseq();
            }
        }
        if (orig_bioseq) {
            CBioseq* bioseq = (CBioseq*)(orig_bioseq.GetNonNullPointer());
            m_Bioseq.Reset((CSerialObject*)bioseq);
            CRef<CBioseq> new_bioseq (new CBioseq());
            new_bioseq->Assign (*bioseq);
            m_EditedBioseq.Reset((CSerialObject*)new_bioseq.GetPointer());
        }
    }

    CSeqdesc& edited_seq_desc = dynamic_cast<CSeqdesc&>(*m_EditedDescriptor);

    string subtype;
    edited_seq_desc.GetLabel(&subtype, CSeqdesc::eType);
   
    CHolderPanel *holder = 0;
    wxBoxSizer* holder_sizer = 0;
    m_Placement = 0;
    CSeq_entry_Handle top = m_SEH.GetTopLevelEntry();
    if (m_CreateMode && !top.IsSeq()) {
        holder = new CHolderPanel( parent );
        holder_sizer = new wxBoxSizer(wxHORIZONTAL);
        holder->SetSizer(holder_sizer);
        parent = holder;
        m_Placement = new CDescPlacementPanel(parent);
        holder_sizer->Add(m_Placement, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_Placement->PopulateLocationChoices(top);
        m_Placement->SetNotify(this);
    }
    else
    {
        parent = new CSehHolderPanel( parent, m_SEH );
        holder_sizer = new wxBoxSizer(wxVERTICAL);
        parent->SetSizer(holder_sizer);
    }
    bool replace_all = false;
    auto desc_type = GetDescriptorType(edited_seq_desc);
    switch (desc_type.first) {
        case eComment:
        {
            m_Window = new CDescriptorTextPanel( parent );
            if (gui_widget)
                gui_widget->SetTitle(_("Comment"));
            replace_all = true;
        }
            break;
        case eTitle:
        {
            m_Window = new CDescriptorTextPanel( parent );
            if (gui_widget)
                gui_widget->SetTitle(_("Title"));
        }
            break;
        case eName:
        {
            m_Window = new CDescriptorTextPanel( parent );
            if (gui_widget)
                gui_widget->SetTitle(_("Name"));
        }
            break;
        case eRegion:
        {
            m_Window = new CDescriptorTextPanel( parent );
            if (gui_widget)
                gui_widget->SetTitle(_("Region"));
        }
            break;
        case ePub:
            {
                CRef<CPubdesc> pubdesc(&(edited_seq_desc.SetPub()));
                m_Window = new CPublicationTypePanel ( parent, pubdesc, m_CreateMode );
                if (gui_widget)
                    gui_widget->SetTitle(_("Publication"));
                if (help_client)
                    help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual13/"));
                replace_all = true;
            }
            break;
        case eMolinfo:
            {
            CRef<CMolInfo> molinfo(&(edited_seq_desc.SetMolinfo()));
            CRef<CBioseq> bioseq;
            if (m_EditedBioseq) {
                CBioseq&  edited_bioseq   = dynamic_cast<CBioseq&>(*m_EditedBioseq);
                bioseq = &(edited_bioseq);
            }
            m_Window = new CMolInfoPanel ( parent, molinfo, bioseq);
            if (gui_widget)
                gui_widget->SetTitle(_("MolInfo"));
            }
            break;
        case eTPA:
            {{
                CAssemblyTrackingPanel* p(new CAssemblyTrackingPanel(parent));
                p->SetUser_object(CRef<CUser_object> (&(edited_seq_desc.SetUser())));
                if (m_EditedBioseq) {
                    p->SetBioseq(orig_bioseq);
                }
                m_Window = p;
                if (gui_widget)
                    gui_widget->SetTitle(_("TPA"));
            }}
            break;
        case eStructuredComment:
        {
            if (desc_type.second == "##Assembly-Data-START##") 
            {
                CSequencingTechnologyPanel* seq_tech_panel = new CSequencingTechnologyPanel ( parent, CRef<CUser_object> (&(edited_seq_desc.SetUser())) );
                m_Window = seq_tech_panel;
                if (gui_widget)
                    gui_widget->SetTitle(_("Sequencing Technology"));
                replace_all = seq_tech_panel->GetReplaceAll();
            }
            else
            {
                m_Window = new CStructuredCommentPanel ( parent, CRef<CUser_object> (&(edited_seq_desc.SetUser())) );
                if (gui_widget)
                    gui_widget->SetTitle(_("Structured Comment"));
                replace_all = true;
            }
            if (help_client)
            {
                if (desc_type.second == "##Genome-Assembly-Data-START##")
                    help_client->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual12/#assembly-comment"));
            }
        }
            break;
        case eRefGene:
            {{
                CRefGeneTrackingPanel* p(new CRefGeneTrackingPanel(parent));
                p->SetUser_object(CRef<CUser_object> (&(edited_seq_desc.SetUser())));
                m_Window = p;
                if (gui_widget)
                    gui_widget->SetTitle(_("RefGene"));
            }}
            break;
        case eDBLink:
            {{
                CDBLinkPanel* p(new CDBLinkPanel ( parent, CRef<CUser_object> (&(edited_seq_desc.SetUser()))));
                p->TransferDataToWindow();
                m_Window = p;
                if (gui_widget)
                    gui_widget->SetTitle(_("DBLink"));
                replace_all = true;
            }}
            break;
        case eUnverified:
            {{
                CUnverifiedPanel* up(new CUnverifiedPanel(parent));
                up->SetUser_object(CRef<CUser_object> (&(edited_seq_desc.SetUser())));
                m_Window = up;
                if (gui_widget)
                    gui_widget->SetTitle(_("Unverified"));
            }}
            break;
        case eAuthorizedAccess:
            {{
                CAuthorizedAccessPanel* up(new CAuthorizedAccessPanel(parent));
                up->SetUser_object(CRef<CUser_object> (&(edited_seq_desc.SetUser())));
                m_Window = up;
                if (gui_widget)
                    gui_widget->SetTitle(_("Authorized access"));
            }}
            break;
        case eUnknown:
            m_Window = new wxPanel( parent );
            break;
        case eSource:
        {
            m_Window = new CSrcDescEditorPanel(parent);
            parent->SetMinSize(wxSize(810, 650));
            if (gui_widget)
                gui_widget->SetTitle(_("BioSource"));
        }
            break;
        default:
            m_Window = new wxPanel( parent );
            break;
    }

    IDescEditorPanel* edit = dynamic_cast<IDescEditorPanel *>(m_Window);
    if (edit) {
        edit->ChangeSeqdesc(edited_seq_desc);
        if (holder) {
            holder->SetEditor(edit);
        }
    }

    if (holder) {
        holder_sizer->Add(m_Window,  1, wxGROW|wxALL, 5);
        LocationChange(top, gui_widget); 
        if (!m_CreateMode && !top.IsSeq())
        {
            m_ReplaceAll = new wxCheckBox( parent, wxID_ANY, _("Replace All"), wxDefaultPosition, wxDefaultSize, 0 );
            m_ReplaceAll->SetValue(replace_all);
            holder_sizer->Add(m_ReplaceAll, 0, wxALIGN_BOTTOM|wxALL, 0);
        }
        return holder;
    } else {
        holder_sizer->Add(m_Window,  1, wxGROW|wxALL, 0);
        if (m_CreateMode)
        {
            LocationChange(top, gui_widget);
        }
        if (!m_CreateMode && !top.IsSeq())
        {
            m_ReplaceAll = new wxCheckBox( parent, wxID_ANY, _("Replace All"), wxDefaultPosition, wxDefaultSize, 0 );
            m_ReplaceAll->SetValue(replace_all);
            holder_sizer->Add(m_ReplaceAll, 0, wxALIGN_RIGHT|wxALL, 0);
        }
        return parent;
    }
}


namespace {
    using SeqdescEntryPair = pair<CConstRef<CSeqdesc>, CSeq_entry_Handle>;
    void ConsolidateObjectList(vector<SeqdescEntryPair>& obj_list)
    {
        if (obj_list.size() < 2) {
            return;
        }
        vector<SeqdescEntryPair>::iterator it = obj_list.begin();
        vector<SeqdescEntryPair>::iterator prev = it;
        ++it;
        while (prev != obj_list.end()) {
            it = prev;
            ++it;
            while (it != obj_list.end()) {
                if (it->first.GetPointer() == prev->first.GetPointer()) {
                    it = obj_list.erase(it);
                }
                else {
                    ++it;
                }
            }
            ++prev;
        }
    }

    vector<SeqdescEntryPair> GetAllSeqdescInSeqEntry(CSeq_entry_Handle seh, CSeqdesc::E_Choice choice)
    {
        vector<SeqdescEntryPair> objects;
        for (CBioseq_CI bi(seh); bi; ++bi) {
            for (CSeqdesc_CI desc_it(*bi, choice); desc_it; ++desc_it) {
                objects.emplace_back(ConstRef(&(*desc_it)), desc_it.GetSeq_entry_Handle());
            }
        }

        sort(objects.begin(), objects.end(), [](const SeqdescEntryPair& a, const SeqdescEntryPair& b) { return a.first < b.first; });
        ConsolidateObjectList(objects);
        return objects;
    }
}


CRef<CCmdComposite> AddDescToSeqEntry(const CSeqdesc& desc, CSeq_entry_Handle seh, bool propagate)
{
    bool any = false;
    CRef<CCmdComposite> cmd(new CCmdComposite("Create Descriptors"));
    if (seh.IsSet() && propagate) {
        for (CBioseq_CI b_iter(seh, CSeq_inst::eMol_na); b_iter; ++b_iter)
        {
            CSeq_entry_Handle s = b_iter->GetSeq_entry_Handle();
            CBioseq_set_Handle bssh = b_iter->GetParentBioseq_set();
            if (bssh && bssh.IsSetClass() && bssh.GetClass() ==  CBioseq_set::eClass_nuc_prot)
            {
                s =  bssh.GetParentEntry();
            }
            CRef<CCmdComposite> subcmd = AddDescToSeqEntry(desc, s, false);
            if (subcmd) {
                cmd->AddCommand(*subcmd);
                any = true;
            }
        }
    } else {
        CRef<CCmdCreateDesc> subcmd(new CCmdCreateDesc(seh, desc));
        cmd->AddCommand(*subcmd);
        any = true;
    }
    if (!any) {
        cmd.Reset(NULL);
    }
    return cmd;
}


CRef<CCmdComposite> CreateMolInfoDesc(CSeq_entry_Handle seh, CRef<CSerialObject> obj, CMolInfoPanel* molPanel) 
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Create MolInfo Descriptors"));
    bool any = false;
    int num_nuc = 0;
    int num_desc = 0;
    for (CBioseq_CI b_iter(seh, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CSeq_entry_Handle s = b_iter->GetSeq_entry_Handle();
        num_nuc++;
        CSeqdesc_CI di(*b_iter, CSeqdesc::e_Molinfo, 1);
        if (di) 
        {
            const CSeqdesc& seqdesc = *di;
            const CSeqdesc* edited_seqdesc = dynamic_cast<const CSeqdesc*>(obj.GetPointerOrNull());
            if (edited_seqdesc) 
            {
                CRef<CCmdChangeSeqdesc> chgd_cmd(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), seqdesc, *edited_seqdesc));
                cmd->AddCommand(*chgd_cmd);
                num_desc++;
            }

        } 
        else 
        {
            CRef<CCmdCreateDesc> subcmd(new CCmdCreateDesc(s, dynamic_cast<const CSeqdesc&>(obj.GetObject())));
            cmd->AddCommand(*subcmd);
        }

        if (molPanel)
        {
            CConstRef<CBioseq> bioseq = b_iter->GetCompleteBioseq();
            CRef<CBioseq> new_bioseq (new CBioseq());
            new_bioseq->Assign (*bioseq);
            molPanel->SetBioseqValues(new_bioseq);
            CIRef<IEditCommand> chgInst(new CCmdChangeBioseqInst(*b_iter, new_bioseq->GetInst()));
            cmd->AddCommand(*chgInst);
        }
        any = true;
    }

    for ( CSeq_entry_CI entry_it(seh, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) 
    {
        if (!entry_it->IsSet())
            continue;
        
        for (CSeqdesc_CI di(*entry_it, CSeqdesc::e_Molinfo, 1); di; ++di)
        {
            CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(*entry_it, *di));
            cmd->AddCommand(*cmdDelDesc);
        }
    }

    if (num_nuc > 1 && num_desc != 0) 
    {
        wxString prompt;
        prompt << "There are " << num_desc <<" molinfo descriptors on " << num_nuc << " bioseqs.\n They will be overwritten.\n Procced?";
        int choice = wxMessageBox(prompt, _("Warning"), wxYES_NO | wxICON_WARNING, NULL);
        if (choice != wxYES) 
        {
            any = false;
        }
    }

    if (!any) 
        cmd.Reset(NULL);
    
    return cmd;
}

static CRef<CCmdComposite> s_ReplaceAllMolinfo(
    CSeq_entry_Handle seh,
    const CSeqdesc& orig_desc, const CSeqdesc& edited_desc, const CBioseq& orig_bseq, const CBioseq& edited_bseq)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Replace All MolInfo Descriptors"));
    const auto& orig_inst = orig_bseq.GetInst();
    for (CBioseq_CI b_iter(seh); b_iter; ++b_iter) {
        CBioseq_Handle bsh = *b_iter;
        bool same_mol = (bsh.IsSetInst_Mol() && orig_inst.IsSetMol() && bsh.GetInst_Mol() == orig_inst.GetMol()) ||
                        (!bsh.IsSetInst_Mol() && !orig_inst.IsSetMol());

        bool same_strand = (bsh.IsSetInst_Strand() && orig_inst.IsSetStrand() && bsh.GetInst_Strand() == orig_inst.GetStrand()) ||
                            (!bsh.IsSetInst_Strand() && !orig_inst.IsSetStrand());

        bool same_topology = (bsh.IsSetInst_Topology() && orig_inst.IsSetTopology() && bsh.GetInst_Topology() == orig_inst.GetTopology()) ||
                            (!bsh.IsSetInst_Topology() && !orig_inst.IsSetTopology());

        bool same_inst = same_mol && same_strand && same_topology;
        if (!same_inst)
            continue;

        bool updated = false;
        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Molinfo, 1); desc_it; ++desc_it) {
            if (desc_it->Equals(orig_desc)) {
                CRef<CSeqdesc> replacement(new CSeqdesc);
                replacement->Assign(edited_desc);
                CRef<CCmdChangeSeqdesc> chgd_cmd(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *replacement));
                cmd->AddCommand(*chgd_cmd);
                updated = true;
            }
        }

        if (updated) {
            CConstRef<CBioseq> bioseq = b_iter->GetCompleteBioseq();
            CRef<CBioseq> new_bioseq(new CBioseq());
            new_bioseq->Assign(*bioseq);


            if (edited_bseq.IsSetInst() && edited_bseq.GetInst().IsSetTopology()) {
                new_bioseq->SetInst().SetTopology(edited_bseq.GetInst().GetTopology());
            } else {
                new_bioseq->SetInst().ResetTopology();
            }

            if (edited_bseq.IsSetInst() && edited_bseq.GetInst().IsSetStrand()) {
                new_bioseq->SetInst().SetStrand(edited_bseq.GetInst().GetStrand());
            } else {
                new_bioseq->SetInst().ResetStrand();
            }

            if (edited_bseq.IsSetInst() && edited_bseq.GetInst().IsSetMol()) {
                new_bioseq->SetInst().SetMol(edited_bseq.GetInst().GetMol());
            } else {
                new_bioseq->SetInst().ResetMol();
            }

            CIRef<IEditCommand> chgInst(new CCmdChangeBioseqInst(bsh, new_bioseq->GetInst()));
            cmd->AddCommand(*chgInst);
        }
    }
    return cmd;
}



IEditCommand* CEditObjectSeq_desc::GetEditCommand()
{
    if (!m_Window)
        return 0;

    TestForNonAsciiText(m_Window);
    m_Window->TransferDataFromWindow();

    CSeq_entry_Handle specific = m_SEH;

    if (m_CreateMode) {
        // if we are in creation mode, associated Bioseq might not have
        // been selected to start
        if (m_Placement != NULL) {
            CSeq_entry_Handle seh = m_Placement->GetLocationChoice();
            if (seh) {
                specific = seh;
            }
        }
        if (specific && specific.IsSeq()) {
            CConstRef<CBioseq> orig_bioseq = specific.GetSeq().GetCompleteBioseq();
            if (orig_bioseq) {
                CBioseq* bioseq = (CBioseq*)(orig_bioseq.GetNonNullPointer());
                m_Bioseq.Reset((CSerialObject*)bioseq);
                CRef<CBioseq> new_bioseq (new CBioseq());
                new_bioseq->Assign (*bioseq);
                m_EditedBioseq.Reset((CSerialObject*)new_bioseq.GetPointer());
            }
        }
    }

    if (!m_EditedDescriptor)
        return nullptr;
    
    CSeqdesc& edited_seq_desc = dynamic_cast<CSeqdesc&>(*m_EditedDescriptor);

    if ( s_IsTpa(edited_seq_desc) && !m_EditedBioseq) {
        return 0;
    }


    IDescEditorPanel* edit = dynamic_cast<IDescEditorPanel *>(m_Window);
    if (edit) {
        edit->UpdateSeqdesc(edited_seq_desc);
        if (edited_seq_desc.Which() == CSeqdesc::e_not_set) {
            return 0;
        }
    }

    CMolInfoPanel* molPanel = nullptr;
    if ((molPanel = dynamic_cast<CMolInfoPanel*>(m_Window)) != nullptr && m_EditedBioseq) {
        CBioseq&  edited_bioseq   = dynamic_cast<CBioseq&>(*m_EditedBioseq);
        CRef<CBioseq> bioseq(&(edited_bioseq));
        molPanel->SetBioseqValues(bioseq);
    } 

    if (edited_seq_desc.IsUser() && !edited_seq_desc.GetUser().IsSetData()) {
        return NULL;
    }

    const CSeqdescTitleEdit* seq_desc_edit = dynamic_cast<const CSeqdescTitleEdit*>(m_Object.GetPointer());
    if (seq_desc_edit) {
        CBioseq_Handle bh = seq_desc_edit->GetBioseq_Handle();
        m_EditAction.Reset(new CChangeSeqdescTitleCommand(bh, edited_seq_desc.GetTitle()));
    }
    else if (edited_seq_desc.IsMolinfo()) {
        if ( m_CreateMode)
        {
            CRef<CCmdComposite> cmd = CreateMolInfoDesc(m_SEH.GetTopLevelEntry(), m_EditedDescriptor, molPanel);
            m_EditAction.Reset(cmd);   
            return m_EditAction.GetPointer();
        }
        else if (ReplaceAll()) {
            const CSeqdesc* original = dynamic_cast<const CSeqdesc*>(m_Object.GetPointer());
            const CBioseq* orig_bseq = dynamic_cast<const CBioseq*>(m_Bioseq.GetPointerOrNull());
            const CBioseq* edited_bioseq = dynamic_cast<const CBioseq*>(m_EditedBioseq.GetPointerOrNull());
            if (original && orig_bseq && edited_bioseq) {
                CRef<CCmdComposite> cmd = s_ReplaceAllMolinfo(m_SEH.GetTopLevelEntry(), *original, edited_seq_desc, *orig_bseq, *edited_bioseq);
                m_EditAction.Reset(cmd);
                return m_EditAction.GetPointer();
            }
        } else {
            CRef<CCmdComposite> cmd(new CCmdComposite("Edit MolInfo"));
            const CSeqdesc* original = dynamic_cast<const CSeqdesc*>(m_Object.GetPointer());
            CSeq_entry_Handle orig_seh = edit::GetSeqEntryForSeqdesc(m_Scope, *original);
            CRef<CCmdChangeSeqdesc> chgd_cmd(new CCmdChangeSeqdesc(orig_seh, *original, edited_seq_desc));
            cmd->AddCommand(*chgd_cmd);

            if (m_Bioseq && specific.IsSeq()) 
            {
                CRef<CBioseq> bioseq(dynamic_cast<CBioseq*>(m_EditedBioseq.GetPointer()));
                CIRef<IEditCommand> chgInst(new CCmdChangeBioseqInst(specific.GetSeq(), bioseq->GetInst()));
                cmd->AddCommand(*chgInst);
            }
            m_EditAction.Reset(cmd);
        }
    }
    else {
        try {
            if (m_CreateMode) {
                bool propagate = false;
                if (specific.IsSet() && (!specific.GetSet().IsSetClass() || specific.GetSet().GetClass() != CBioseq_set::eClass_nuc_prot)) {
                    int choice = wxMessageBox(wxT("Do you wish to propagate the descriptor to the sequences in the set?"), wxT("Warning"),
                                              wxYES_NO , NULL);
                    if (choice == wxYES) {
                        propagate = true;
                    } 
                } 
                m_EditAction.Reset(AddDescToSeqEntry(dynamic_cast<const CSeqdesc&>(m_EditedDescriptor.GetObject()), specific, propagate));
            }
            else {
                m_EditAction.Reset();
                if (edited_seq_desc.IsPub() && ReplaceAll()) {
                    const CSeqdesc* original = dynamic_cast<const CSeqdesc*>(m_Object.GetPointer());
                    CRef<CCmdComposite> comp_cmd(new CCmdComposite("Replace All Similar Pubs"));

                    CSeq_entry_Handle orig_seh = edit::GetSeqEntryForSeqdesc(m_Scope, *original);
                    vector<SeqdescEntryPair> obj_list = GetAllSeqdescInSeqEntry(orig_seh.GetTopLevelEntry(), CSeqdesc::e_Pub);
                    for (auto& it : obj_list) {
                        const CSeqdesc& p = *it.first;
                        if (p.GetPub().GetPub().SameCitation(original->GetPub().GetPub())) {
                            CRef<CSeqdesc> replacement(new CSeqdesc);
                            replacement->Assign(edited_seq_desc);
                            CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(it.second, *(it.first), *replacement));
                            comp_cmd->AddCommand(*cmd);
                        }
                    }
                    m_EditAction.Reset(comp_cmd);
                }
                // for editing similar seqdesc:
                bool edit_all_desc = !edited_seq_desc.IsPub() && ReplaceAll();
                   
                if (edit_all_desc) {
                    const CSeqdesc* original = dynamic_cast<const CSeqdesc*>(m_Object.GetPointer());
                    if (original) {
                        CSeq_entry_Handle orig_seh = edit::GetSeqEntryForSeqdesc(m_Scope, *original);
                        vector<SeqdescEntryPair> obj_list =
                            GetAllSeqdescInSeqEntry(orig_seh.GetTopLevelEntry(), original->Which());

                        CRef<CCmdComposite> comp_cmd(new CCmdComposite("Replace All Identical Descriptors"));
                        for (auto& it : obj_list) {
                            const CSeqdesc* descriptor = dynamic_cast<const CSeqdesc*>(it.first.GetPointer());
                            if (descriptor && descriptor->Equals(*original)) {
                                CRef<CSeqdesc> replacement(new CSeqdesc);
                                replacement->Assign(edited_seq_desc);
                                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(it.second, *(it.first), *replacement));
                                comp_cmd->AddCommand(*cmd);
                            }
                        }
                        m_EditAction.Reset(comp_cmd);
                    }
                    
                }
                    
                if (m_EditAction.IsNull()) {
                    const CSeqdesc* original = dynamic_cast<const CSeqdesc*>(m_Object.GetPointer());
                    if (original) {
                        CSeqdesc_CI desc_it(specific);
                        while (&*desc_it != original) {
                            ++desc_it;
                        }
                        CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *original, edited_seq_desc));
                        m_EditAction.Reset(cmd);
#ifdef _DEBUG
                        CNcbiOstrstream ostr;
                        {{
                            auto_ptr<CObjectOStream> out(CObjectOStream::Open(eSerial_AsnText, ostr));
                            *out << edited_seq_desc;
                            *out << *original;
                        }}
                        _TRACE(string(CNcbiOstrstreamToString(ostr)));
#endif
                    }
                }
            }
        }
        catch (const CException& e) {
            LOG_POST(Error << "CEditObjectSeq_desc::GetEditAction(): " << e.GetMsg());
        }
    }
#ifdef _DEBUG
    CNcbiOstrstream ostr;
    {{
        auto_ptr<CObjectOStream> out(CObjectOStream::Open(eSerial_AsnText, ostr));
        *out << edited_seq_desc;
    }}
    _TRACE(string(CNcbiOstrstreamToString(ostr)));
#endif

    return m_EditAction.GetPointer();
}

void CEditObjectSeq_desc::MolInfoLocationChange(CSeq_entry_Handle seh, IDescEditorPanel* edit, CMolInfoPanel* molPanel)
{
    CConstRef<CBioseq> bioseq;
    const CSeqdesc* seqdesc(nullptr);
    for (CBioseq_CI b_iter(seh, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CSeqdesc_CI di(*b_iter, CSeqdesc::e_Molinfo, 1);
        if (di)
        {
            seqdesc = &(*di);
            bioseq = b_iter->GetCompleteBioseq();
            break;
        }
    }

    if (seqdesc)
    {
        edit->ChangeSeqdesc(*seqdesc);
        molPanel->ChangeBioseq(bioseq);
        m_Object.Reset(seqdesc);
        m_EditedDescriptor.Reset((CSerialObject*)CSeqdesc::GetTypeInfo()->Create());
        m_EditedDescriptor->Assign(*seqdesc);
        m_Bioseq.Reset((CSerialObject*)bioseq.GetPointer());
        CRef<CBioseq> new_bioseq (new CBioseq());
        new_bioseq->Assign (*bioseq);
        m_EditedBioseq.Reset((CSerialObject*)new_bioseq.GetPointer());           
    }
    
    if (m_Placement != NULL) 
    {
        m_Placement->Show(false);
    }
}

void CEditObjectSeq_desc::LocationChange(CSeq_entry_Handle seh, wxTopLevelWindow* gui_widget)
{
    IDescEditorPanel* edit = dynamic_cast<IDescEditorPanel *>(m_Window);
    CMolInfoPanel* molPanel = dynamic_cast<CMolInfoPanel*>(m_Window);
    if (edit) {
        // look for existing descriptors to edit instead
        CSeqdesc& edited_seq_desc = dynamic_cast<CSeqdesc&>(*m_EditedDescriptor);
        pair<EDescriptorType,string> desc_type = GetDescriptorType(edited_seq_desc);
        if (desc_type.first == eMolinfo)
        {
            MolInfoLocationChange(seh.GetTopLevelEntry(), edit, molPanel);
        }
        else if (desc_type.first != ePub)    {  
            for ( CSeq_entry_CI entry_it(seh, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) {
                CSeqdesc_CI di(*entry_it, GetDescChoiceForType(desc_type.first), 1);
                while (di && GetDescriptorType(*di) != desc_type) {
                    ++di;
                }
                if (di) {
                    // descriptor already exists
                    if (gui_widget)
                    {
                        wxYield();
                        gui_widget->Hide();
                    }
                    wxString text;
                    string label = desc_type.second;
                    NStr::TrimPrefixInPlace(label, "##");
                    NStr::TrimSuffixInPlace(label, "##");
                    NStr::TrimSuffixInPlace(label, "-START");
                    NStr::TrimSuffixInPlace(label, "Data");
                    NStr::TrimSuffixInPlace(label, "-");
                    text << label << " ";
                    label = edit::GetTextObjectDescription(edited_seq_desc, seh.GetScope());                   
                    if (label.empty())
                    {
                        edited_seq_desc.GetLabel(&label, CSeqdesc::eType);
                    }

                    if (label == "StructuredComment")
                    {
                        label = "comment";
                    }
                    text << label;

                    int choice = wxMessageBox(_("A ") + text + _(" already exists at or above the selected target. Would you like to edit the existing ") + text + _(" instead?"), 
                                              wxT("Warning"), wxYES_NO | wxICON_ERROR, gui_widget);
                    if (gui_widget)
                    {
                        gui_widget->Show();
                        gui_widget->Raise();
                        gui_widget->SetFocus();
                    }
                    if (choice == wxYES) {
                        edit->ChangeSeqdesc(*di);
                        m_CreateMode = false;
                        m_Object.Reset(&(*di));
                        m_EditedDescriptor.Reset((CSerialObject*)CSeqdesc::GetTypeInfo()->Create());
                        m_EditedDescriptor->Assign(*di);
                        m_EditedBioseq.Reset();
                        if (m_Placement != NULL) {
                            m_SEH = m_Placement->GetLocationChoice();
                            if (m_SEH.IsSeq()) {
                                CConstRef<CBioseq> orig_bioseq = m_SEH.GetSeq().GetCompleteBioseq();
                                if (orig_bioseq) {
                                    CBioseq* bioseq = (CBioseq*)(orig_bioseq.GetNonNullPointer());
                                    m_Bioseq.Reset((CSerialObject*)bioseq);
                                    CRef<CBioseq> new_bioseq (new CBioseq());
                                    new_bioseq->Assign (*bioseq);
                                    m_EditedBioseq.Reset((CSerialObject*)new_bioseq.GetPointer());
                                }
                            }
                            m_Placement->Show(false);
                        }
                    }        
                    break;
                }    
            }
        }
    }

    CAssemblyTrackingPanel* assemblyPanel = dynamic_cast<CAssemblyTrackingPanel*>(m_Window);
    if (seh.IsSeq()) {
        CConstRef<CBioseq> b = seh.GetSeq().GetCompleteBioseq();
        if (molPanel) {
            molPanel->ChangeBioseq(b);
        } else if ((assemblyPanel = dynamic_cast<CAssemblyTrackingPanel*>(m_Window)) != 0) {
            assemblyPanel->SetBioseq(b);
        }
    }
}


CSeqdesc::E_Choice CEditObjectSeq_desc::GetDescChoiceForType(EDescriptorType desc_type)
{
    CSeqdesc::E_Choice rval = CSeqdesc::e_not_set;
    switch (desc_type) {
        case eUnknown:
            rval = CSeqdesc::e_not_set;
            break;
        case eTPA:
        case eStructuredComment:
        case eRefGene:
        case eDBLink:
        case eUnverified:
        case eAuthorizedAccess:
            rval = CSeqdesc::e_User;
            break;
        case eComment:
            rval = CSeqdesc::e_Comment;
            break;
        case eTitle:
            rval = CSeqdesc::e_Title;
            break;
        case eName:
            rval = CSeqdesc::e_Name;
            break;
        case eRegion:
            rval = CSeqdesc::e_Region;
            break;
        case ePub:
            rval = CSeqdesc::e_Pub;
            break;
        case eMolinfo:
            rval = CSeqdesc::e_Molinfo;
            break;
        case eSource:
            rval = CSeqdesc::e_Source;
            break;
    }
    return rval;
}


pair<CEditObjectSeq_desc::EDescriptorType,string> CEditObjectSeq_desc::GetDescriptorType(const CSeqdesc& desc)
{
    EDescriptorType rval = eUnknown;
    string sval;
    switch (desc.Which()) {
        case CSeqdesc::e_Comment:
            rval = eComment;
            break;
        case CSeqdesc::e_Title:
            rval = eTitle;
            break;
        case CSeqdesc::e_Name:
            rval = eName;
            break;
        case CSeqdesc::e_Region:
            rval = eRegion;
            break;
        case CSeqdesc::e_Pub:
            rval = ePub;
            break;
        case CSeqdesc::e_Molinfo:
            rval = eMolinfo;
            break;
        case CSeqdesc::e_User:
            switch (desc.GetUser().GetObjectType()) {
                case CUser_object::eObjectType_OriginalId:
                    rval = eUnknown;
                    break;
                case CUser_object::eObjectType_DBLink:
                    rval = eDBLink;
                    break;
                case CUser_object::eObjectType_Unverified:
                    rval = eUnverified;
                    break;
                case CUser_object::eObjectType_StructuredComment:
                  {{
                    rval = eStructuredComment;
                    ITERATE (CUser_object::TData, it, desc.GetUser().GetData()) {
                        if ((*it)->IsSetLabel()
                            && (*it)->GetLabel().IsStr()
                            && NStr::Equal((*it)->GetLabel().GetStr(), "StructuredCommentPrefix")
                            && (*it)->IsSetData()
                            && (*it)->GetData().IsStr()) {
                            sval = (*it)->GetData().GetStr();
                            break;
                        }
                    }                                      
                  }}
                  break;
                default:
                    if (desc.GetUser().IsSetType() 
                        && desc.GetUser().GetType().IsStr()) {
                        string label = desc.GetUser().GetType().GetStr();

                        if (NStr::EqualNocase(label, "AuthorizedAccess")) {
                            rval = eAuthorizedAccess;
                        } else if (NStr::EqualNocase(label, "RefGeneTracking")) {
                            rval = eRefGene;                
                        } else if (s_IsTpa(desc.GetUser())) {
                            rval = eTPA;

                        }
                    }
                    break;
            }
            break;
        case CSeqdesc::e_Source:
            rval = eSource;
            break;
        default:
            rval = eUnknown;
            break;
    }
    return make_pair(rval,sval);
}

END_NCBI_SCOPE

