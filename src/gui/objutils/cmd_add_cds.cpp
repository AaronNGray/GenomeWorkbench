/*  $Id: cmd_add_cds.cpp 42385 2019-02-13 16:34:44Z asztalos $
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
 * Authors:  Colleen Bollin, based on a file by Roman Katargin
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
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdAddCDS::Execute()
{
    CSeq_entry_EditHandle eh = m_seh.GetEditHandle();
    if (!eh.IsSet()) {
        eh.ConvertSeqToSet();
        eh.SetSet().SetClass(CBioseq_set::eClass_nuc_prot);
        CBioseq_CI bi(eh, CSeq_inst::eMol_na);
        CBioseq_EditHandle nuc_h = bi->GetEditHandle();

        // move all source and pub descriptors from Bioseq to Nuc-prot set
        CSeq_descr::Tdata& seq_descr = nuc_h.SetDescr();
        CSeq_descr::Tdata::iterator desc_it = seq_descr.begin();
        while (desc_it != seq_descr.end()) {
            if ((*desc_it)->IsSource() || (*desc_it)->IsPub()) {
                CRef<CSeqdesc> cpy(new CSeqdesc());
                cpy->Assign(**desc_it);
                eh.AddSeqdesc(*cpy);
                desc_it = seq_descr.erase(desc_it);
            } else {
                ++desc_it;
            }
        }
    }
              

    CSeq_annot_Handle ftable;

    CSeq_annot_CI annot_ci(m_seh, CSeq_annot_CI::eSearch_entry);
    for (; annot_ci; ++annot_ci) {
        if ((*annot_ci).IsFtable()) {
            ftable = *annot_ci;
            break;
        }
    }

    if (!ftable)
        for (CBioseq_CI bi(m_seh, CSeq_inst::eMol_na); bi; ++bi)
        {
            for (CSeq_annot_CI annot_ci(bi->GetParentEntry(), CSeq_annot_CI::eSearch_entry); annot_ci; ++annot_ci) 
            {
                if ((*annot_ci).IsFtable()) 
                {
                    ftable = *annot_ci;
                    break;
                }
            }
            if (ftable)
                break;
        }

    if (!ftable) {
        CRef<CSeq_annot> new_annot(new CSeq_annot());
        ftable = m_FTableCreated = eh.AttachAnnot(*new_annot);
    }
   

    // add protein sequence that is translation
    CBioseq_Handle prot_h;
    if (m_CDS->IsSetProduct())
    {
        prot_h = eh.GetBioseqHandle(*(m_CDS->GetProduct().GetId()));
    }

    CRef<CBioseq> protein = CSeqTranslator::TranslateToProtein (*m_CDS, eh.GetScope());
    CRef<CSeq_id> prot_id(new CSeq_id());
    if (m_CDS->IsSetProduct() && !prot_h) {
        prot_id->Assign(*(m_CDS->GetProduct().GetId()));
    } else {
        // make up some new protein ID
        CBioseq_Handle nuc_h = m_seh.GetScope().GetBioseqHandle(m_CDS->GetLocation());
        int offset = 1;
        string id_label;
        prot_id->Assign(*objects::edit::GetNewProtId(nuc_h, offset, id_label, m_create_general_only));
        m_CDS->SetProduct().SetWhole().Assign(*prot_id);
    }
    protein->SetId().push_back(prot_id);    
    m_ProtHandle = eh.AttachBioseq(*protein, -1);

    CSeq_annot_EditHandle aeh(ftable);
    m_feh_CDS = aeh.AddFeat(*m_CDS);

    // create molinfo descriptor
    CRef<CSeqdesc> pdesc(new CSeqdesc());
    pdesc->SetMolinfo().SetBiomol(CMolInfo::eBiomol_peptide);
    pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_complete);

    // set location for protein feature
  
    if (m_CDS->GetLocation().IsPartialStart(eExtreme_Biological)) {
        if (m_Prot)
        {
            m_Prot->SetLocation().SetPartialStart(true, eExtreme_Biological);
            m_Prot->SetPartial(true);
        }
        pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_no_left);
    }
    if (m_CDS->GetLocation().IsPartialStop(eExtreme_Biological)) {
        if (m_Prot)
        {
            m_Prot->SetLocation().SetPartialStop(true, eExtreme_Biological);
            m_Prot->SetPartial(true);
        }
        if (pdesc->GetMolinfo().GetCompleteness() == CMolInfo::eCompleteness_complete) {
            pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_no_right);
        } else {
            pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_no_ends);
        }
    }

    if (m_Prot)
    {
        m_Prot->SetLocation().SetInt().SetId(*prot_id);
        m_Prot->SetLocation().SetInt().SetFrom(0);
        m_Prot->SetLocation().SetInt().SetTo(protein->GetLength() - 1);
        
        CRef<CSeq_annot> new_prot_annot(new CSeq_annot());
        CSeq_annot_Handle prot_ftable = m_ProtHandle.AttachAnnot(*new_prot_annot);
        CSeq_annot_EditHandle paeh(prot_ftable);
        paeh.AddFeat(*m_Prot);
    }

    // add molinfo descriptor
    m_ProtHandle.AddSeqdesc(*pdesc);

}

void CCmdAddCDS::Unexecute()
{
    m_ProtHandle.Remove();
    CSeq_entry_EditHandle eh = m_seh.GetEditHandle();
    if (eh.IsSet() && eh.GetSet().GetCompleteBioseq_set()->GetSeq_set().size() == 1) {
        eh.ConvertSetToSeq();    
    }      

    m_feh_CDS.Remove();

    if (m_FTableCreated) {
        m_FTableCreated.Remove();
    }
}

string CCmdAddCDS::GetLabel()
{
    return "Convert nuc to nuc-prot set";
}

void CCmdCreateCDS::Execute()
{
    CSeq_entry_EditHandle eh = m_seh.GetEditHandle();

    string prot = s_GetProductSequence(*m_CDS, eh.GetScope());
    if (!prot.empty()) {
        if (!eh.IsSet()) {
            eh.ConvertSeqToSet();
            eh.SetSet().SetClass(CBioseq_set::eClass_nuc_prot);
            CBioseq_CI bi(eh, CSeq_inst::eMol_na);
            CBioseq_EditHandle nuc_h = bi->GetEditHandle();

            // move all source and pub descriptors from Bioseq to Nuc-prot set
            CSeq_descr::Tdata& seq_descr = nuc_h.SetDescr();
            CSeq_descr::Tdata::iterator desc_it = seq_descr.begin();
            while (desc_it != seq_descr.end()) {
                if ((*desc_it)->IsSource() || (*desc_it)->IsPub()) {
                    CRef<CSeqdesc> cpy(new CSeqdesc());
                    cpy->Assign(**desc_it);
                    eh.AddSeqdesc(*cpy);
                    desc_it = seq_descr.erase(desc_it);
                }
                else {
                    ++desc_it;
                }
            }
        }
    }


    CSeq_annot_Handle ftable;
    CSeq_annot_CI annot_ci(m_seh, CSeq_annot_CI::eSearch_entry);
    for (; annot_ci; ++annot_ci) {
        if ((*annot_ci).IsFtable()) {
            ftable = *annot_ci;
            break;
        }
    }

    /*
    if (!ftable)
        for (CBioseq_CI bi(m_seh, CSeq_inst::eMol_na); bi; ++bi)
        {
            for (CSeq_annot_CI annot_ci(bi->GetParentEntry(), CSeq_annot_CI::eSearch_entry); annot_ci; ++annot_ci)
            {
                if ((*annot_ci).IsFtable())
                {
                    ftable = *annot_ci;
                    break;
                }
            }
            if (ftable)
                break;
        }
    */
    if (!ftable) {
        CRef<CSeq_annot> new_annot(new CSeq_annot());
        ftable = m_FTableCreated = eh.AttachAnnot(*new_annot);
    }


    if (!prot.empty()) {
        CRef<CBioseq> protein(new CBioseq);
        protein->SetInst().ResetExt();
        protein->SetInst().SetRepr(CSeq_inst::eRepr_raw);
        if (NStr::Find(prot, "-") == NPOS) {
            protein->SetInst().SetSeq_data().SetIupacaa().Set(prot);
        }
        else {
            protein->SetInst().SetSeq_data().SetNcbieaa().Set(prot);
        }
        protein->SetInst().SetLength(TSeqPos(prot.length()));
        protein->SetInst().SetMol(CSeq_inst::eMol_aa);

        m_CDS->SetProduct().SetWhole().Assign(*m_prot_id.front());
        for (auto new_prot_id : m_prot_id)
        {
            protein->SetId().push_back(new_prot_id);
        }

        m_ProtHandle = eh.AttachBioseq(*protein, -1);
        x_AddMolinfoDescriptorToProtein();
        x_AddProteinFeatures(protein->GetLength());
    }
    
    CSeq_annot_EditHandle aeh(ftable);
    m_feh_CDS = aeh.AddFeat(*m_CDS);
}

string CCmdCreateCDS::s_GetProductSequence(const CSeq_feat& cds, CScope& scope)
{
    string prot;
    try {
        CSeqTranslator::Translate(cds, scope, prot);
    }
    catch (const CSeqVectorException&) {}

    if (NStr::EndsWith(prot, "*")) {
        prot = prot.substr(0, prot.length() - 1);
    }
    return prot;
}

void CCmdCreateCDS::x_AddMolinfoDescriptorToProtein()
{
    CRef<CSeqdesc> pdesc(new CSeqdesc());
    pdesc->SetMolinfo().SetBiomol(CMolInfo::eBiomol_peptide);
    pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_complete);

    // set location for protein feature

    if (m_CDS->GetLocation().IsPartialStart(eExtreme_Biological)) {
        if (m_Prot)
        {
            m_Prot->SetLocation().SetPartialStart(true, eExtreme_Biological);
            m_Prot->SetPartial(true);
        }
        pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_no_left);
    }
    if (m_CDS->GetLocation().IsPartialStop(eExtreme_Biological)) {
        if (m_Prot)
        {
            m_Prot->SetLocation().SetPartialStop(true, eExtreme_Biological);
            m_Prot->SetPartial(true);
        }
        if (pdesc->GetMolinfo().GetCompleteness() == CMolInfo::eCompleteness_complete) {
            pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_no_right);
        }
        else {
            pdesc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_no_ends);
        }
    }

    m_ProtHandle.AddSeqdesc(*pdesc);
}

void CCmdCreateCDS::x_AddProteinFeatures(const TSeqPos& seq_length)
{
    if (m_Prot || !m_other_prot_feats.empty())
    {
        CRef<CSeq_annot> new_prot_annot(new CSeq_annot());
        CSeq_annot_Handle prot_ftable = m_ProtHandle.AttachAnnot(*new_prot_annot);
        CSeq_annot_EditHandle paeh(prot_ftable);

        if (m_Prot)
        {
            m_Prot->SetLocation().SetInt().SetId(*m_prot_id.front());
            m_Prot->SetLocation().SetInt().SetFrom(0);
            m_Prot->SetLocation().SetInt().SetTo(seq_length - 1);
            paeh.AddFeat(*m_Prot);
        }
        for (const auto f : m_other_prot_feats)
        {
            paeh.AddFeat(*f);
        }
    }
}



void CCmdCreateCDS::Unexecute()
{
    if (m_ProtHandle) {
        m_ProtHandle.Remove();
    }
    CSeq_entry_EditHandle eh = m_seh.GetEditHandle();
    if (eh.IsSet() && eh.GetSet().GetCompleteBioseq_set()->GetSeq_set().size() == 1) {
        eh.ConvertSetToSeq();
    }

    m_feh_CDS.Remove();

    if (m_FTableCreated) {
        m_FTableCreated.Remove();
    }
}

string CCmdCreateCDS::GetLabel()
{
    return "Create CDS";
}



END_NCBI_SCOPE
