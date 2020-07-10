/*  $Id: cmd_add_seqentry.cpp 43474 2019-07-11 19:20:45Z filippov $
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

#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>

#include <objects/seq/MolInfo.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objtools/readers/source_mod_parser.hpp>
#include <objtools/edit/cds_fix.hpp>

#include <gui/objutils/cmd_add_seqentry.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CCmdAddSeqEntry::CCmdAddSeqEntry(CSeq_entry* seqentry, CSeq_entry_Handle seh)
    : m_SEH(seh), m_Add(seqentry)
{
}

void CCmdAddSeqEntry::Execute()  
{
    CSeq_entry_EditHandle eh = m_SEH.GetEditHandle();
    if (!eh.IsSet() && m_Add->IsSeq() && m_Add->GetSeq().IsAa()) {
        CBioseq_set_Handle nuc_parent = eh.GetParentBioseq_set();
        if (nuc_parent && nuc_parent.IsSetClass() && nuc_parent.GetClass() == objects::CBioseq_set::eClass_nuc_prot) {
            m_SEH = nuc_parent.GetParentEntry();
            eh = m_SEH.GetEditHandle();
            
        }
    }
    if (!eh.IsSet()) {
        eh.ConvertSeqToSet();
        if (m_Add->IsSeq() && m_Add->GetSeq().IsAa()) {
            // when add protein sequence, convert sequence to nuc-prot set
            eh.SetSet().SetClass(CBioseq_set::eClass_nuc_prot);
            m_SEH = CSeq_entry_Handle(eh);
            CConstRef<CBioseq_set> set = eh.GetSet().GetCompleteBioseq_set();
            if (set && set->IsSetSeq_set()) {
                CConstRef<CSeq_entry> nuc = set->GetSeq_set().front();
                CSeq_entry_EditHandle neh = m_SEH.GetScope().GetSeq_entryEditHandle(*nuc);
                CBioseq_set::TDescr::Tdata::const_iterator it = nuc->GetDescr().Get().begin();
                // move all descriptors from nucleotide sequence to the set
                // except molinfo, title, OriginalID and AutodefOptions user objects
                while (it != nuc->GetDescr().Get().end()) {
                    if (!(*it)->IsMolinfo() 
                        && !(*it)->IsTitle()
                        && !((*it)->IsUser() && ( (*it)->GetUser().GetObjectType() == CUser_object::eObjectType_AutodefOptions
                                                || (*it)->GetUser().GetObjectType() == CUser_object::eObjectType_OriginalId))) {
                        CRef<CSeqdesc> copy(new CSeqdesc());
                        copy->Assign(**it);
                        eh.AddSeqdesc(*copy);
                        neh.RemoveSeqdesc(**it);
                        if (nuc->IsSetDescr()) {
                            it = nuc->GetDescr().Get().begin();
                        } else {
                            break;
                        }
                    } else {
                        ++it;
                    }
                }
            }
        }
    }
    

    CSeq_entry_EditHandle added = eh.AttachEntry(*m_Add);
    m_index = eh.GetSet().GetSeq_entry_Index(added);
}

void CCmdAddSeqEntry::Unexecute() 
{
    int pos = 0;
    CConstRef<CSeq_entry> entry = m_SEH.GetCompleteSeq_entry();
    if (entry->IsSet()) {
        const CBioseq_set& set = entry->GetSet();
        CBioseq_set::TSeq_set::const_iterator it = set.GetSeq_set().begin();
        while (it != set.GetSeq_set().end() && pos < m_index) {
            ++pos;
            ++it;
        }
        
        if (it != set.GetSeq_set().end()) {
            CSeq_entry_EditHandle eh = m_SEH.GetScope().GetSeq_entryEditHandle(**it);
            eh.Remove();
            if (set.GetSeq_set().size() == 1) {
                eh = m_SEH.GetEditHandle();
                eh.ConvertSetToSeq();
            }
        }
    }
}

string CCmdAddSeqEntry::GetLabel()
{
    return "Add SeqEntry";
}

bool SetMolinfoCompleteness (CMolInfo& mi, bool partial5, bool partial3)
{
    bool changed = false;
    CMolInfo::ECompleteness new_val;
    if ( partial5  &&  partial3 ) {
        new_val = CMolInfo::eCompleteness_no_ends;
    } else if ( partial5 ) {
        new_val = CMolInfo::eCompleteness_no_left;
    } else if ( partial3 ) {
        new_val = CMolInfo::eCompleteness_no_right;
    } else {
        new_val = CMolInfo::eCompleteness_complete;
    }
    if (!mi.IsSetCompleteness() || mi.GetCompleteness() != new_val) {
        mi.SetCompleteness(new_val);
        changed = true;
    }
    return changed;
}


void SetMolinfoForProtein (CRef<objects::CSeq_entry> protein, bool partial5, bool partial3)
{
    CRef<objects::CSeqdesc> pdesc(NULL);
    NON_CONST_ITERATE(objects::CSeq_descr::Tdata, it, protein->SetDescr().Set()) {
        if ((*it)->IsMolinfo()) {
            pdesc = *it;
            break;
        }
    }
    if (!pdesc) {
        pdesc = new objects::CSeqdesc();
        protein->SetDescr().Set().push_back(pdesc);
    }
    pdesc->SetMolinfo().SetBiomol(objects::CMolInfo::eBiomol_peptide);
    SetMolinfoCompleteness(pdesc->SetMolinfo(), partial5, partial3);
    
}


CRef<objects::CSeq_feat> AddEmptyProteinFeatureToProtein (CRef<objects::CSeq_entry> protein, bool partial5, bool partial3)
{
    CRef<objects::CSeq_annot> ftable(NULL);
    NON_CONST_ITERATE(objects::CSeq_entry::TAnnot, annot_it, protein->SetSeq().SetAnnot()) {
        if ((*annot_it)->IsFtable()) {
            ftable = *annot_it;
            break;
        }
    }
    if (!ftable) {
        ftable = new objects::CSeq_annot();
        protein->SetSeq().SetAnnot().push_back(ftable);
    }
    
    CRef<objects::CSeq_feat> prot_feat(NULL);
    NON_CONST_ITERATE(objects::CSeq_annot::TData::TFtable, feat_it, ftable->SetData().SetFtable()) {
        if ((*feat_it)->IsSetData() && (*feat_it)->GetData().IsProt() && !(*feat_it)->GetData().GetProt().IsSetProcessed()) {
            prot_feat = *feat_it;
            break;
        }
    }
    if (!prot_feat) {
        prot_feat = new objects::CSeq_feat();
        prot_feat->SetData().SetProt();
        ftable->SetData().SetFtable().push_back(prot_feat);
    }
    CRef<objects::CSeq_id> prot_id(new objects::CSeq_id());
    prot_id->Assign(*(protein->GetSeq().GetId().front()));
    prot_feat->SetLocation().SetInt().SetId(*prot_id);
    prot_feat->SetLocation().SetInt().SetFrom(0);
    prot_feat->SetLocation().SetInt().SetTo(protein->GetSeq().GetLength() - 1);
    prot_feat->SetLocation().SetPartialStart(partial5, objects::eExtreme_Biological);
    prot_feat->SetLocation().SetPartialStop(partial3, objects::eExtreme_Biological);
    if (partial5 || partial3) {
        prot_feat->SetPartial(true);
    } else {
        prot_feat->ResetPartial();
    }
    return prot_feat;
}


CRef<objects::CSeq_feat> AddProteinFeatureToProtein (CRef<objects::CSeq_entry> protein, bool partial5, bool partial3)
{
    objects::CSourceModParser smp(  
      objects::CSourceModParser::eHandleBadMod_Ignore );
    // later - fix protein title by removing attributes used?
    CConstRef<objects::CSeqdesc> title_desc
        = protein->GetSeq().GetClosestDescriptor(objects::CSeqdesc::e_Title);
    if (title_desc) {
        string& title(const_cast<string&>(title_desc->GetTitle()));
        title = smp.ParseTitle(title, CConstRef<objects::CSeq_id>(protein->GetSeq().GetFirstId()) );
        smp.ApplyAllMods(protein->SetSeq());
    }

    return AddEmptyProteinFeatureToProtein(protein, partial5, partial3);
}


CRef<objects::CSeq_entry> CreateTranslatedProteinSequence (CRef<objects::CSeq_feat> cds, objects::CBioseq_Handle nuc_h, bool create_general_only, int *offset)
{
    CRef<objects::CBioseq> protein = objects::CSeqTranslator::TranslateToProtein (*cds, nuc_h.GetScope());
    objects::CSeqTranslator::ChangeDeltaProteinToRawProtein(protein);
    CRef<objects::CSeq_id> prot_id(new objects::CSeq_id());
    if (cds->IsSetProduct()) {
        prot_id->Assign(*(cds->GetProduct().GetId()));
    } else {
        // make up some new protein ID
        int local_offset = 1;
        if (offset)
            local_offset = *offset;
        string id_label;
        prot_id->Assign(*objects::edit::GetNewProtId(nuc_h, local_offset, id_label, create_general_only));
        cds->SetProduct().SetWhole().Assign(*prot_id);
        ++local_offset;
        if (offset)
            *offset = local_offset;
    }
    protein->SetId().push_back(prot_id);
    CRef<objects::CSeq_entry> entry(new objects::CSeq_entry());
    entry->SetSeq().Assign(*protein);
    SetMolinfoForProtein (entry, 
                          cds->GetLocation().IsPartialStart(objects::eExtreme_Biological), 
                          cds->GetLocation().IsPartialStop(objects::eExtreme_Biological)); 
    return entry;
}

END_NCBI_SCOPE
