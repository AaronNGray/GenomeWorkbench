/*  $Id: cmd_feat_id_xrefs_from_qualifiers.cpp 41199 2018-06-11 17:59:58Z filippov $
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

#include <objmgr/util/sequence.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>

#include "cmd_feat_id_xrefs_from_qualifiers.hpp"
#include "cmd_assign_feat_local_ids.hpp"

#include <gui/packages/pkg_sequence_edit/modify_feature_id.hpp>

#include <gui/widgets/wx/message_box.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kLabel = "Link CDS-mRNA pair by qualifier";

CIRef<IEditCommand> CCmdFeatIdXrefsFromQualifiers::Create(const objects::CSeq_entry_Handle& seh, const vector<string>& quals)
{
    CRef<CCmdComposite> cmd(new CCmdComposite(kLabel));
    cmd->AddCommand(*CIRef<IEditCommand>(new CCmdAssignFeatLocalIds(seh)));
    cmd->AddCommand(*CIRef<IEditCommand>(new CCmdFeatIdXrefsFromQualifiers(seh, quals)));
    return CIRef<IEditCommand>(cmd);
}

string CCmdFeatIdXrefsFromQualifiers::GetLabel()
{
    return kLabel;
}

static bool s_IsDirectXrefBetween(const CSeq_feat& from_feat, const CSeq_feat& to_feat)
{
    if (!from_feat.IsSetXref() || !to_feat.IsSetId()) {
        return false;
    }
    
    const CFeat_id& feat_id = to_feat.GetId();
    FOR_EACH_SEQFEATXREF_ON_SEQFEAT(it, from_feat) {
        if ((*it)->IsSetId()
            && (*it)->GetId().IsLocal()
            && feat_id.Equals((*it)->GetId()))
            return true;
    }

    return false;
}

static void s_CreateXRefLink(CSeq_feat& from_feat, const CSeq_feat& to_feat)
{
    CRef<CSeqFeatXref> xref(new CSeqFeatXref);
    xref->SetId().Assign(to_feat.GetId());
    from_feat.SetXref().push_back(xref);
}

static void s_AddCommand(CRef<CCmdComposite>& composite, IEditCommand& command)
{
    if (!composite)
        composite.Reset(new CCmdComposite(kLabel));
    composite->AddCommand(command);
}

CIRef<IEditCommand> CCmdFeatIdXrefsFromQualifiers::x_CreateActionCmd()
{
    CScope& scope = m_seh.GetScope();
    string error_msg;
    CRef<CCmdComposite> cmd;

    for (CFeat_CI feat_it(m_seh, SAnnotSelector(CSeqFeatData::eSubtype_mRNA)); feat_it; ++feat_it) {
        CConstRef<CSeq_feat> mrna = feat_it->GetOriginalSeq_feat();
        FOR_EACH_GBQUAL_ON_FEATURE(qual_it, *mrna) {
            const CGb_qual& qual = **qual_it;
            if (find(m_Quals.begin(), m_Quals.end(), qual.GetQual()) != m_Quals.end() && qual.IsSetVal()) {
                CRef<CSeq_id> seq_id(new CSeq_id);
                try {
                    seq_id->Set(qual.GetVal(), CSeq_id::fParse_Default);
                }
                catch (const CException&) {
                    if (!error_msg.empty()) {
                        error_msg += "\n";
                    }
                    error_msg += "'" + qual.GetVal() + "' is not a valid seq ID";
                    continue;
                }

                CBioseq_Handle proth = scope.GetBioseqHandle(*seq_id);
                if (!proth || !proth.IsAa()) {
                    if (!error_msg.empty()) {
                        error_msg += "\n";
                    }
                    error_msg += "Sequence with '" + qual.GetVal() + "' seq ID is not a protein";
                    continue;
                }
                const CSeq_feat* cds = sequence::GetCDSForProduct(proth);
                if (cds) {
                    CRef<CSeq_feat> new_mrna(new CSeq_feat);
                    new_mrna->Assign(*mrna);

                    // update the mRNA product name if it's empty
                    string mrna_product = new_mrna->GetData().GetRna().GetRnaProductName();
                    if (mrna_product.empty()) {
                        string protein_name;
                        CFeat_CI prot_it(proth, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                        if (prot_it) {
                            CConstRef<CSeq_feat> prot_feat = prot_it->GetOriginalSeq_feat();
                            const CProt_ref& prot_ref = prot_feat->GetData().GetProt();
                            if (prot_ref.IsSetName() && !prot_ref.GetName().empty()) {
                                protein_name = prot_ref.GetName().front();
                            }
                        }

                        if (!protein_name.empty()) {
                            // command to change the mrna
                            string remainder;
                            new_mrna->SetData().SetRna().SetRnaProductName(protein_name, remainder);
                        }
                    }


                    if (!s_IsDirectXrefBetween(*mrna, *cds)) {
                        s_CreateXRefLink(*new_mrna, *cds);
                        CIRef<IEditCommand> chg_mrna(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*mrna), *new_mrna));
                        s_AddCommand(cmd, *chg_mrna);
                    }

                    if (!s_IsDirectXrefBetween(*cds, *mrna)) {
                        CRef<CSeq_feat> new_cds(new CSeq_feat);
                        new_cds->Assign(*cds);
                        s_CreateXRefLink(*new_cds, *mrna);
                        CIRef<IEditCommand> chg_cds(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*cds), *new_cds));
                        s_AddCommand(cmd, *chg_cds);
                    }
                   
                  
                }
            }
        }
    }

    if (!error_msg.empty()) {
        NcbiMessageBox(error_msg);
        return CIRef<IEditCommand>();
    }

    return CIRef<IEditCommand>(cmd);
}

END_NCBI_SCOPE
