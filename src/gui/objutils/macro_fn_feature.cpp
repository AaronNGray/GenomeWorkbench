/*  $Id: macro_fn_feature.cpp 44628 2020-02-10 18:26:27Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 */

#include <ncbi_pch.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/macro/Featur_locatio_strand_from.hpp>
#include <objects/macro/Feature_location_strand_to.hpp>
#include <objects/macro/Partial_5_set_constraint.hpp>
#include <objects/macro/Partial_3_set_constraint.hpp>
#include <objects/macro/Partial_5_clear_constraint.hpp>
#include <objects/macro/Partial_3_clear_constraint.hpp>
#include <objects/macro/Partial_both_set_constrain.hpp>
#include <objects/macro/Partial_both_clear_constra.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_vector.hpp>

#include <objtools/writers/write_util.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <objtools/edit/gene_utils.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/validator/utilities.hpp>
#include <objtools/cleanup/cleanup.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <gui/objutils/adjust_consensus_splicesite.hpp>

#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_fn_feature.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

// All DO functions should make changes on the "Edited" object of the BioDataIterator

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_TrimStopFromCompleteCDS
/// TrimStopsFromCompleteCDS()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_TrimStopFromCompleteCDS, "TrimStopsFromCompleteCDS");

void CMacroFunction_TrimStopFromCompleteCDS::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || !scope)
        return;

    // first, modify the feature: here, no modification is made to the CDS

    // then, modify the related objects (mRNA, protein, etc.):
    CRef<CCmdComposite> cmd = TrimStopsFromCompleteCDS(*edit_feat, *scope);
    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        CNcbiOstrstream log;
        string best_id;
        CBioseq_Handle product = scope->GetBioseqHandle(edit_feat->GetProduct());
        CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(product.GetCompleteBioseq()->GetFirstId())), *scope, best_id);
        log << "Removed trailing * from protein sequence " << best_id;
        x_LogFunction(log);
    }
}

bool CMacroFunction_TrimStopFromCompleteCDS::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SynchronizeCDSPartials
/// SynchronizeCDSPartials()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SynchronizeCDSPartials, "SynchronizeCDSPartials")

void CMacroFunction_SynchronizeCDSPartials::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || !scope)
        return;

    // first, modify the feature: 
    bool change = feature::AdjustFeaturePartialFlagForLocation(*edit_feat);
    if (change) {
        m_DataIter->SetModified();
    }

    // then, modify the related objects (mRNA, protein, etc.):
    CRef<CCmdComposite> synch_cmd = GetSynchronizeProteinPartialsCommand(*scope, *edit_feat);
    if (synch_cmd) {
        m_DataIter->RunCommand(synch_cmd, m_CmdComposite);
    }

    if (change || synch_cmd) {
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << " synchronized coding region partials";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SynchronizeCDSPartials::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AdjustConsensusSpliceSites
/// AdjustCDSConsensusSpliceSites()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_AdjustConsensusSpliceSites, "AdjustCDSConsensusSpliceSites");

void CMacroFunction_AdjustConsensusSpliceSites::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || !scope)
        return;

    CSeq_feat orig_feat;
    orig_feat.Assign(*edit_feat);

    try {
        CBioseq_Handle bsh = GetBioseqForSeqFeat(*edit_feat, *scope);
        if (bsh && CAdjustForConsensusSpliceSite::s_IsBioseqGood_Strict(bsh)) {
            CAdjustForConsensusSpliceSite worker(*scope);
            // first, modify the feature:
            bool changed = worker.AdjustCDS(*edit_feat);
            if (changed) {
                m_DataIter->SetModified();

                // then, modify the related objects (mRNA, protein, etc.):
                CRef<CCmdComposite> update_cmd = worker.AdjustmRNAandExonFeatures(*edit_feat, orig_feat);
                if (update_cmd) {
                    m_DataIter->RunCommand(update_cmd, m_CmdComposite);
                }
                CNcbiOstrstream log;
                log << "Adjusted location for splice consensus: " << m_DataIter->GetBestDescr() << " became ";
                // append new location to log
                CSeq_loc loc;
                loc.Assign(edit_feat->GetLocation());
                sequence::ChangeSeqLocId(&loc, true, scope);
                string label;
                loc.GetLabel(&label);
                log << label;
                x_LogFunction(log);
            }
        }
    } 
    catch (const CException& err)
    {
        NCBI_RETHROW2(err, CMacroDataException, CException::eUnknown, "AdjustConsensusSpliceSites failed", CConstRef<CSerialObject>(edit_feat));
    }
}

bool CMacroFunction_AdjustConsensusSpliceSites::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveInvalidECNumbers
/// RemoveInvalidECNumbers();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveInvalidECNumbers, "RemoveInvalidECNumbers");

void CMacroFunction_RemoveInvalidECNumbers::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* prot_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    if (!prot_feat || prot_feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_prot) {
        return;
    }

    CProt_ref& prot = prot_feat->SetData().SetProt();
    if (!prot.IsSetEc() || prot.GetEc().empty())
        return;

    typedef vector< CProt_ref::EECNumberStatus > TVecStatus;

    TVecStatus st_before;
    ITERATE(CProt_ref::TEc, ec, prot.GetEc()) {
        st_before.push_back(CProt_ref::GetECNumberStatus(*ec));
    }

    prot.AutoFixEC();

    unsigned int replaced = 0;
    TVecStatus::const_iterator it_bef = st_before.begin();
    CProt_ref::TEc::const_iterator ec = prot.GetEc().begin();
    for (; ec != prot.GetEc().end() && it_bef != st_before.end(); ++ec, ++it_bef) {
        CProt_ref::EECNumberStatus status = CProt_ref::GetECNumberStatus(*ec);
        if (status == CProt_ref::eEC_specific && (*it_bef) == CProt_ref::eEC_replaced) {
            replaced++;
        }
    }

    prot.RemoveBadEC();
    unsigned int removed = (unsigned int)(st_before.size() - prot.GetEc().size());

    if (replaced || removed) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": ";
        if (replaced) {
            log << " replaced " << replaced << " EC numbers";
        }
        if (removed) {
            log << " removed " << removed << " EC numbers";
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveInvalidECNumbers::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_UpdateReplacedECNumbers
/// UpdateReplacedECNumbers( del_improper_format, del_unrecognized, del_mult_repl)
///
// Changes in the function and parameter names require changes in the respective
// XRC file used in the macro editor
DEFINE_MACRO_FUNCNAME(CMacroFunction_UpdateReplacedECNumbers, "UpdateReplacedECNumbers")

void CMacroFunction_UpdateReplacedECNumbers::TheFunction()
{
    bool del_improper_format = m_Args[0]->GetBool(); // remove improperly formatted EC numbers
    bool del_unrecognized = m_Args[1]->GetBool(); // remove unrecognized EC numbers
    bool del_mult_replacement = m_Args[2]->GetBool(); // remove EC numbers replaced by more than one number

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* prot_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!prot_feat || !prot_feat->GetData().IsProt() || !scope) {
        return;
    }

    CProt_ref& prot = prot_feat->SetData().SetProt();
    if (!prot.IsSetEc() || prot.GetEc().empty())
        return;

    // for logging:
    string locus_tag("No locus tag");
    NMacroUtil::GetLocusTagFromProtRef(*prot_feat, *scope, locus_tag);
    CNcbiOstrstream log;

    CProt_ref::TEc::iterator ec = prot.SetEc().begin();
    while (ec != prot.SetEc().end()) {
        bool removed = false;
        if (CProt_ref::IsValidECNumberFormat(*ec)) {
            CProt_ref::EECNumberStatus status = CProt_ref::GetECNumberStatus(*ec);
            if (status == CProt_ref::eEC_replaced) {
                string repl_ec = CProt_ref::GetECNumberReplacement(*ec);
                if (!NStr::IsBlank(repl_ec)) {
                    if (NStr::FindNoCase(repl_ec, "\t") == NPOS) {
                        log << locus_tag << ": replaced " << *ec << " with " << repl_ec << "\n";
                        *ec = repl_ec;
                    }
                    else if (del_mult_replacement) {
                        log << locus_tag << ": removed " << *ec << "\n";
                        ec = prot.SetEc().erase(ec);
                        removed = true;
                    }
                }
            }
            else if (del_unrecognized && status != CProt_ref::eEC_specific && status != CProt_ref::eEC_ambiguous) {
                log << locus_tag << ": removed " << *ec << "\n";
                ec = prot.SetEc().erase(ec);
                removed = true;
            }
        }
        else {
            if (del_improper_format) {
                log << locus_tag << ": removed " << *ec << "\n";
                ec = prot.SetEc().erase(ec);
                removed = true;
            }
        }

        if (!removed) {
            ++ec;
        }
    }

    if (prot.GetEc().empty()) {
        prot.ResetEc();
    }

    m_DataIter->SetModified();
    x_LogFunction(log);
}

bool CMacroFunction_UpdateReplacedECNumbers::x_ValidArguments() const
{
    size_t arg_size = m_Args.size();
    if (arg_size != 3) {
        return false;
    }

    for (size_t n = 0; n < arg_size; ++n) {
        if (m_Args[n]->GetDataType() != CMQueryNodeValue::eBool)
            return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_UpdatemRNAProduct
/// UpdatemRNAProduct()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_UpdatemRNAProduct, "UpdatemRNAProduct");
void CMacroFunction_UpdatemRNAProduct::TheFunction()
{
    if (!m_Args.empty()) {
        // for backward compatibility:
        CMQueryNodeValue& bool_obj = m_Args[0].GetNCObject();
        bool_obj.Dereference();
        if (bool_obj.GetDataType() != CMQueryNodeValue::eBool) {
            return;
        }
        // don't update the mRNA product name if the argument is false
        if (!bool_obj.GetBool()) {
            return;
        }
    }

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* prot_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    // assuming that the iterated object is a protein feature
    if (!prot_feat || !scope ||
        prot_feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_prot ||
        !prot_feat->GetData().GetProt().IsSetName()) {
        return;
    }

    const CProt_ref& prot = prot_feat->GetData().GetProt();
    string product = prot.GetName().front();
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": ";

    CBioseq_Handle prot_bsh = scope->GetBioseqHandle(prot_feat->GetLocation());
    if (prot_bsh)  {
        const CSeq_feat* cds = sequence::GetCDSForProduct(*prot_bsh.GetCompleteBioseq(), scope.GetPointer());
        if (cds) {
            CConstRef<CSeq_feat> mrna = sequence::GetmRNAforCDS(*cds, *scope);
            if (mrna) {
                CRef<CSeq_feat> new_mrna(new CSeq_feat());
                new_mrna->Assign(*mrna);

                bool modified = false;
                if (mrna->GetData().IsRna() && mrna->GetData().GetRna().GetType() == CRNA_ref::eType_mRNA) {
                    CRNA_ref& rna = new_mrna->SetData().SetRna();
                    if (NStr::IsBlank(product)) {
                        rna.ResetExt();
                        log << "Reset the mRNA product name ";
                        modified = true;
                    }
                    else if (!rna.IsSetExt() || (rna.IsSetExt() && !NStr::Equal(rna.GetExt().GetName(), product))){
                        rna.SetExt().SetName(product);
                        log << "Applied " << product << " to mRNA product name ";
                        modified = true;
                    }
                }

                if (modified) {
                    CSeq_feat_Handle fh = scope->GetSeq_featHandle(*mrna);
                    CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_mrna));
                    CRef<CCmdComposite> cmd(new CCmdComposite("Update mRNA product to match CDS product name"));
                    cmd->AddCommand(*chgFeat);
                    m_DataIter->RunCommand(cmd, m_CmdComposite);
                    x_LogFunction(log);
                }
            }
        }
    }
}

bool CMacroFunction_UpdatemRNAProduct::x_ValidArguments() const
{
    if (m_Args.empty())
        return true;
    // for backward compatibility:
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eRef);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveFeature
/// RemoveFeature([delete_overlapping_gene]) - also works for removing biosource and pub features
/// Set the flag TRUE to delete the overlapping gene
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveFeature, "RemoveFeature")

void CMacroFunction_RemoveFeature::TheFunction()
{
    if (!m_DataIter->IsFeature()) {
        return;
    }

    m_DataIter->SetToDelete(true);

    bool delete_gene = false;

    if (m_Args.size() == 1 && m_Args[0]->GetBool()) {
        // delete overlapping gene
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
        CRef<CScope> scope = m_DataIter->GetScopedObject().scope;

        CConstRef<CSeq_feat> gene = sequence::GetOverlappingGene(edit_feat->GetLocation(), *scope);
        if (gene) {
            CRef<CCmdDelSeq_feat> del_cmd(new CCmdDelSeq_feat(scope->GetSeq_featHandle(*gene)));
            CRef<CCmdComposite> cmd(new CCmdComposite("Delete overlapping gene"));
            cmd->AddCommand(*del_cmd);
            m_DataIter->RunCommand(cmd, m_CmdComposite);
            delete_gene = true;
        }
    }

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": feature removed";
    if (delete_gene) {
        log << " and the overlapping gene";
    }
    x_LogFunction(log);
}

bool CMacroFunction_RemoveFeature::x_ValidArguments() const
{
    return m_Args.empty() || (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eBool);
}

///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_CopyNameToCDSNote
/// CopyNameToCDSNote();
/// Copies the first protein name to the cds comment
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_CopyNameToCDSNote, "CopyNameToCDSNote")
void CMacroFunction_CopyNameToCDSNote::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* prot_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    // assuming that the iterated object is a protein feature
    if (!prot_feat || !scope ||
        !prot_feat->GetData().IsProt() ||
        !prot_feat->GetData().GetProt().IsSetName()) {
        return;
    }

    const CProt_ref& prot = prot_feat->GetData().GetProt();
    string product = prot.GetName().front();
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": ";

    CBioseq_Handle prot_bsh = scope->GetBioseqHandle(prot_feat->GetLocation());
    if (prot_bsh) {
        const CSeq_feat* cds = sequence::GetCDSForProduct(*prot_bsh.GetCompleteBioseq(), scope.GetPointer());
        if (cds) {
            CRef<CSeq_feat> new_cds(new CSeq_feat());
            new_cds->Assign(*cds);

            bool modified = false;
            string orig_comment = (new_cds->IsSetComment()) ? new_cds->GetComment() : kEmptyStr;
            if (edit::AddValueToString(orig_comment, product, edit::eExistingText_append_semi)) {
                new_cds->SetComment(orig_comment);
                log << "Copied protein name to CDS note";
                modified = true;
            }

            if (modified) {
                CSeq_feat_Handle fh = scope->GetSeq_featHandle(*cds);
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_cds));
                CRef<CCmdComposite> cmd(new CCmdComposite("Copy protein name to CDS note"));
                cmd->AddCommand(*chgFeat);
                m_DataIter->RunCommand(cmd, m_CmdComposite);
                x_LogFunction(log);
            }
        }
    }
}

bool CMacroFunction_CopyNameToCDSNote::x_ValidArguments() const
{
    return (m_Args.empty());
}


/// class CMacroFunction_RemoveAllFeatures
/// RemoveAllFeatures();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveAllFeatures, "RemoveAllFeatures")
void CMacroFunction_RemoveAllFeatures::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!scope) return;

    size_t count = 0;
    bool remove_proteins = true;
    CRef<CCmdComposite> delete_features_cmd;
    if (entry) {
        delete_features_cmd = GetDeleteAllFeaturesCommand(m_DataIter->GetSEH(), count, remove_proteins);
    }
    else if (bseq && bseq->IsNa()) { // deprecated part
        delete_features_cmd.Reset(new CCmdComposite("Remove All Features"));
        // only delete features on nucleotide sequences, protein features are automatically removed with the coding region
        CBioseq_Handle bsh = scope->GetBioseqHandle(*bseq);
        if (!bsh)
            return;

        if (m_DataIter->IsBegin()) {
            m_ProductToCds.clear();
            GetProductToCDSMap(*scope, m_ProductToCds);
        }

        CFeat_CI it(bsh);
        while (it) {
            CRef<CCmdComposite> cmd = GetDeleteFeatureCommand(it->GetSeq_feat_Handle(), remove_proteins, m_ProductToCds);
            delete_features_cmd->AddCommand(*cmd);
            count++;
            ++it;
        }
    }

    if (count > 0) {
        m_DataIter->RunCommand(delete_features_cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << "Removed " << count << " features";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveAllFeatures::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveDuplFeatures
/// RemoveDuplicateFeatures(feat_type, check_partials, case_sensitive, remove_proteins)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveDuplFeatures, "RemoveDuplicateFeatures")

void CMacroFunction_RemoveDuplFeatures::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || !scope)
        return;

    CSeqFeatData::ESubtype type = NMacroUtil::GetFeatSubtype(m_Args[0]->GetString());
    bool check_partials = m_Args[1]->GetBool();
    bool case_sensitive = m_Args[2]->GetBool();
    bool remove_proteins = m_Args[3]->GetBool();

    CBioseq_Handle bsh = scope->GetBioseqHandle(*bseq);
    SAnnotSelector sel(type);
    set<CSeq_feat_Handle> duplicates;

    for (CFeat_CI feat_it1(bsh, sel); feat_it1; ++feat_it1) {
        for (CFeat_CI feat_it2(*scope, feat_it1->GetLocation(), sel); feat_it2; ++feat_it2) {
            if (feat_it1->GetSeq_feat_Handle() < feat_it2->GetSeq_feat_Handle()) {
                validator::EDuplicateFeatureType dup_type = validator::IsDuplicate(feat_it1->GetSeq_feat_Handle(), feat_it2->GetSeq_feat_Handle(), check_partials, case_sensitive);
                if (dup_type == validator::eDuplicate_Duplicate || dup_type == validator::eDuplicate_DuplicateDifferentTable) {
                    duplicates.insert(feat_it2->GetSeq_feat_Handle());
                }
            }
        }
    }

    if (duplicates.empty())
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("Delete duplicate features"));
    ITERATE(set<CSeq_feat_Handle>, fh, duplicates) {
        cmd->AddCommand(*GetDeleteFeatureCommand(*fh, remove_proteins));
    }

    m_DataIter->RunCommand(cmd, m_CmdComposite);
    CNcbiOstrstream log;
    log << duplicates.size() << ";removal of duplicate features";
    x_LogFunction(log);
}

bool CMacroFunction_RemoveDuplFeatures::x_ValidArguments() const
{
    if (m_Args.size() != 4 || m_Args[0]->GetDataType() != CMQueryNodeValue::eString) {
        return false;
    }

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool)
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_FixProteinFormat 
/// FixProteinFormat() - removes organism names from protein names
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixProteinFormat, "FixProteinFormat")

void CMacroFunction_FixProteinFormat::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    // the object should be a protein feature
    CSeq_feat* prot_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!prot_feat || !prot_feat->GetData().IsProt() || !scope)
        return;


    CRef<CMacroBioData_FeatIterBase> feat_iter = 
        Ref(dynamic_cast<CMacroBioData_FeatIterBase*>(m_DataIter.GetPointer()));
    if (m_DataIter->IsBegin() && feat_iter) {
        // obtain and cache the taxnames to be used later in fixing the protein names
        CSeq_entry_Handle seh;
        try {
            CBioseq_Handle protein_bsh = scope->GetBioseqHandle(prot_feat->GetLocation());
            seh = protein_bsh.GetTSE_Handle().GetTopLevelEntry();
        }
        catch (const CException&) {
            return;
        }

        if (seh) {
            FindOrgNames(seh, feat_iter->SetTaxnames());
        }
    }

    vector<string> orig_name, new_name;
    CProt_ref& prot_ref = prot_feat->SetData().SetProt();
    for (auto& it : prot_ref.SetName()) {
        string prot_name = s_FixProteinNameFormat(it, feat_iter->GetTaxnames());
        if (!prot_name.empty() && !NStr::EqualCase(it, prot_name)) {
            orig_name.push_back(it);
            it = prot_name;
            new_name.push_back(it);
        }
    }
    
    if (!new_name.empty()) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        for (size_t n = 0; n < new_name.size(); ++n) {
            log << "On " << m_DataIter->GetBestDescr();
            log << ": changed protein name from " << orig_name[n];
            log << " to " << new_name[n];
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_FixProteinFormat::x_ValidArguments() const
{
    return (m_Args.empty());
}


string CMacroFunction_FixProteinFormat::s_FixProteinNameFormat(const string& protein, const vector<string>& taxnames)
{
    if (NStr::IsBlank(protein))
        return kEmptyStr;
    if (taxnames.empty())
        return protein;

    string new_prot(protein);
    vector<string> pattern;
    pattern.push_back("()");
    pattern.push_back("( )");
    pattern.push_back("[]");
    pattern.push_back("[ ]");
    ITERATE(vector<string>, name, taxnames) {
        SIZE_TYPE pos = NStr::FindNoCase(new_prot, *name);
        if (pos != NPOS) {
            new_prot = protein.substr(0, pos) + protein.substr(pos + name->length());
            NStr::ReplaceInPlace(new_prot, "  ", "");
            
            // delete any brackets around the taxname:
            ITERATE(vector<string>, pat_it, pattern) {
                if (NStr::FindNoCase(new_prot, *pat_it) != NPOS) {
                    NStr::ReplaceInPlace(new_prot, *pat_it, "");
                }
            }
            NStr::ReplaceInPlace(new_prot, "  ", " ");
        }
    }
    NStr::TruncateSpacesInPlace(new_prot);

    return new_prot;
}

bool CMacroFunction_EditFeatLocation::x_CheckInitFeature()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    m_Origfeat = ConstRef(dynamic_cast<const CSeq_feat*>(obj.GetPointer()));
    m_Scope = m_DataIter->GetScopedObject().scope;
    if (!m_Origfeat || !m_Scope) {
        return false;
    }
    
    CObjectInfo oi = m_DataIter->GetEditedObject();
    m_Seqfeat = Ref(CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr()));
    if (!m_Seqfeat || !m_Seqfeat->IsSetLocation()) {
        return false;
    }

    return true;
}

void CMacroFunction_EditFeatLocation::x_RetranslateCDSAdjustGene(const string& descr)
{
    _ASSERT(m_Modified);
    m_Seqfeat->SetPartial(m_Seqfeat->GetLocation().IsPartialStart(eExtreme_Biological)
            || m_Seqfeat->GetLocation().IsPartialStop(eExtreme_Biological));

    m_DataIter->SetModified();
    string new_loc;
    m_Seqfeat->GetLocation().GetLabel(&new_loc);
    CNcbiOstrstream log;
    log << descr << " for " << m_DataIter->GetBestDescr() << " to " << new_loc;

    x_RetranslateCDS(log);

    if (m_AdjustGene && !m_Seqfeat->GetData().IsGene()) {
        CRef<CCmdChangeSeq_feat> adjust_gene_cmd = AdjustGene(*m_Origfeat, *m_Seqfeat, *m_Scope);
        if (adjust_gene_cmd) {
            CRef<CCmdComposite> cmd(new CCmdComposite("Adjust gene"));
            cmd->AddCommand(*adjust_gene_cmd);
            m_DataIter->RunCommand(cmd, m_CmdComposite);
            log << " and adjusted gene location.";
        }
    }
    x_LogFunction(log);
}

void CMacroFunction_EditFeatLocation::x_RetranslateCDS(CNcbiOstrstream& log)
{
    if (m_Seqfeat->GetData().IsCdregion()) {
        if (m_RetranslateCDS && !sequence::IsPseudo(*m_Seqfeat, *m_Scope)) {
            CRef<CMacroBioData_FeatIterBase> feat_iter =
                Ref(dynamic_cast<CMacroBioData_FeatIterBase*>(m_DataIter.GetPointer()));
            if (m_DataIter->IsBegin() && feat_iter) {
                feat_iter->SetCreateGeneralIdFlag();
            }
            
            bool cds_change = false;
            // this function might make changes in the cds, but changes are not included in the command
            CRef<CCmdComposite> retransl_cmd = GetRetranslateCDSCommand(*m_Scope, *m_Seqfeat, cds_change, feat_iter->GetCreateGeneralIdFlag());
            if (retransl_cmd) {
                m_DataIter->RunCommand(retransl_cmd, m_CmdComposite);
                log << " and retranslated the coding region ";
            }
        }
        else {
            // this function/command is not changing the feature
            CRef<CCmdComposite> synch_cmd = GetSynchronizeProteinPartialsCommand(*m_Scope, *m_Seqfeat);
            if (synch_cmd) {
                m_DataIter->RunCommand(synch_cmd, m_CmdComposite);
                log << " and synchronized CDS partials";
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetBothPartials
/// SetBothEndsPartial("all"|"at-end", extend(boolean) [,retranslate_cds][,adjust_gene])
/// The first parameter is for feature selection
/// use "at-end" to only apply to features whose both ends are at the end of sequence
/// use extend = true, to extend to ends of sequence if partials are set
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetBothPartials, "SetBothEndsPartial")

void CMacroFunction_SetBothPartials::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = s_SetBothEndsPartial(*m_Seqfeat, *m_Scope, m_Args[0]->GetString(), m_Args[1]->GetBool());
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.size() > 2) ? m_Args[2]->GetBool() : false;
        m_AdjustGene = (m_Args.size() == 4) ? m_Args[3]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Set both end partials");
    }
}

bool CMacroFunction_SetBothPartials::s_SetBothEndsPartial(CSeq_feat& feat, CScope& scope, const string& descr, bool extend)
{
    if (!ENUM_METHOD_NAME(EPartial_both_set_constraint)()->IsValidName(descr)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized option to set both partials: " + descr);
    }

    EPartial_both_set_constraint apply_mode =
        (EPartial_both_set_constraint)ENUM_METHOD_NAME(EPartial_both_set_constraint)()->FindValue(descr);

    edit::CLocationEditPolicy::EPartialPolicy policy5 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    edit::CLocationEditPolicy::EPartialPolicy policy3 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    if (apply_mode == ePartial_both_set_constraint_all) {
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSet;
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSet;
    }
    else {
        CBioseq_Handle bsh = GetBioseqForSeqFeat(feat, scope);
        if (bsh
            && edit::CLocationEditPolicy::Is5AtEndOfSeq(feat.GetLocation(), bsh)
            && edit::CLocationEditPolicy::Is3AtEndOfSeq(feat.GetLocation(), bsh)) {
            // when both ends of location coincide with the sequence ends
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd;
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd;
        }
    }

    bool extend5(false), extend3(false);
    if (extend) {
        extend5 = true;
        extend3 = true;
    }

    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy(policy5, policy3, extend5, extend3));
    return policy->ApplyPolicyToFeature(feat, scope);
}


bool CMacroFunction_SetBothPartials::x_ValidArguments() const
{
    if (m_Args.size() < 2 || m_Args.size() > 4)
        return false;

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveBothPartials
/// RemoveBothEndsPartial("all"|"not-at-end" [,retranslate_cds][,adjust_gene])
/// First parameter specifies which features to select:
/// "all" - all features, "not-at-end" - features whose both ends are not at the sequence ends
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveBothPartials, "RemoveBothEndsPartial")

void CMacroFunction_RemoveBothPartials::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = s_RemoveBothPartials(*m_Seqfeat, *m_Scope, m_Args[0]->GetString());
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.size() > 1) ? m_Args[1]->GetBool() : false;
        m_AdjustGene = (m_Args.size() == 3) ? m_Args[2]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Removed both ends partial");
    }
}

bool CMacroFunction_RemoveBothPartials::s_RemoveBothPartials(CSeq_feat& feat, CScope& scope, const string& descr)
{
    if (!ENUM_METHOD_NAME(EPartial_both_clear_constraint)()->IsValidName(descr)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized option to clear partials: " + descr);
    }

    EPartial_both_clear_constraint apply_mode =
        (EPartial_both_clear_constraint)ENUM_METHOD_NAME(EPartial_both_clear_constraint)()->FindValue(descr);

    edit::CLocationEditPolicy::EPartialPolicy policy5 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    edit::CLocationEditPolicy::EPartialPolicy policy3 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    if (apply_mode == ePartial_both_clear_constraint_all) {
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClear;
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClear;
    }
    else {
        CBioseq_Handle bsh = GetBioseqForSeqFeat(feat, scope);
        if (bsh 
            && !edit::CLocationEditPolicy::Is5AtEndOfSeq(feat.GetLocation(), bsh)
            && !edit::CLocationEditPolicy::Is3AtEndOfSeq(feat.GetLocation(), bsh)) {
            // when both ends of location do not coincide with the sequence ends
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd;
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd;
        }
    }

    bool extend5 = false, extend3 = false;
    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy(policy5, policy3, extend5, extend3));
    return policy->ApplyPolicyToFeature(feat, scope);
}

bool CMacroFunction_RemoveBothPartials::x_ValidArguments() const
{
    if (m_Args.size() < 1 || m_Args.size() > 3)
        return false;

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Set5Partial
/// SetPartialStart("all"|"at-end"|"bad-start"|"frame-not-one", extend(boolean) [,retranslate_cds][,adjust_gene])
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Set5Partial, "SetPartialStart")

const vector<SArgMetaData> CMacroFunction_Set5Partial::sm_Arguments 
    { SArgMetaData("feature_field", CMQueryNodeValue::eString, false)
    , SArgMetaData("partial_option", CMQueryNodeValue::eString, false)
    , SArgMetaData("extend", CMQueryNodeValue::eBool, false)
    , SArgMetaData("retranslate", CMQueryNodeValue::eBool, true)
    , SArgMetaData("adjustgene", CMQueryNodeValue::eBool, true)
    };

void CMacroFunction_Set5Partial::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = s_Set5EndPartial(*m_Seqfeat, *m_Scope, m_Args[0]->GetString(), m_Args[1]->GetBool());
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.size() > 2) ? m_Args[2]->GetBool() : false;
        m_AdjustGene = (m_Args.size() == 4) ? m_Args[3]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Set 5' end partial");
    }
}

bool CMacroFunction_Set5Partial::s_Set5EndPartial(CSeq_feat& feat, CScope& scope, const string& descr, bool extend5)
{
    if (!ENUM_METHOD_NAME(EPartial_5_set_constraint)()->IsValidName(descr)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized option to set 5' partials: " + descr);
    }

    EPartial_5_set_constraint filter =
        (EPartial_5_set_constraint)ENUM_METHOD_NAME(EPartial_5_set_constraint)()->FindValue(descr);

    edit::CLocationEditPolicy::EPartialPolicy policy5 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    switch (filter) {
    case ePartial_5_set_constraint_all:
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSet;
        break;
    case ePartial_5_set_constraint_at_end:
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd;
        break;
    case ePartial_5_set_constraint_bad_start:
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd;
        break;
    case ePartial_5_set_constraint_frame_not_one:
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSetForFrame;
        break;
    }

    bool extend3 = false;
    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy(policy5, edit::CLocationEditPolicy::ePartialPolicy_eNoChange, extend5, extend3));
    return policy->ApplyPolicyToFeature(feat, scope);
}

bool CMacroFunction_Set5Partial::x_ValidArguments() const
{
    if (m_Args.size() < 2 || m_Args.size() > 4)
        return false;

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Set3Partial
/// SetPartialStop("all"|"at-end"|"bad-end", extend(boolean) [,retranslate_cds][,adjust_gene])
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Set3Partial, "SetPartialStop")

void CMacroFunction_Set3Partial::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = s_Set3EndPartial(*m_Seqfeat, *m_Scope, m_Args[0]->GetString(), m_Args[1]->GetBool());
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.size() > 2) ? m_Args[2]->GetBool() : false;
        m_AdjustGene = (m_Args.size() == 4) ? m_Args[3]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Set 3' end partial");
    }
}

bool CMacroFunction_Set3Partial::s_Set3EndPartial(CSeq_feat& feat, CScope& scope, const string& descr, bool extend3)
{
    if (!ENUM_METHOD_NAME(EPartial_3_set_constraint)()->IsValidName(descr)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized option to set 3' partials: " + descr);
    }

    EPartial_3_set_constraint filter =
        (EPartial_3_set_constraint)ENUM_METHOD_NAME(EPartial_3_set_constraint)()->FindValue(descr);

    edit::CLocationEditPolicy::EPartialPolicy policy3 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    switch (filter) {
    case ePartial_3_set_constraint_all:
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSet;
        break;
    case ePartial_3_set_constraint_at_end:
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd;
        break;
    case ePartial_3_set_constraint_bad_end:
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd;
        break;
    }

    bool extend5 = false;
    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy(edit::CLocationEditPolicy::ePartialPolicy_eNoChange, policy3, extend5, extend3));
    return policy->ApplyPolicyToFeature(feat, scope);
}

bool CMacroFunction_Set3Partial::x_ValidArguments() const
{
    if (m_Args.size() < 2 || m_Args.size() > 4)
        return false;

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Clear5Partial
/// ClearPartialStart("all"|"not-at-end"|"good-start" [,retranslate_cds][,adjust_gene])
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Clear5Partial, "ClearPartialStart")

const vector<SArgMetaData> CMacroFunction_Clear5Partial::sm_Arguments
{ SArgMetaData("feature_field", CMQueryNodeValue::eString, false)
, SArgMetaData("partial_option", CMQueryNodeValue::eString, false)
, SArgMetaData("retranslate", CMQueryNodeValue::eBool, true)
, SArgMetaData("adjustgene", CMQueryNodeValue::eBool, true)
};
void CMacroFunction_Clear5Partial::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = s_Clear5EndPartial(*m_Seqfeat, *m_Scope, m_Args[0]->GetString());
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.size() > 1) ? m_Args[1]->GetBool() : false;
        m_AdjustGene = (m_Args.size() == 3) ? m_Args[2]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Cleared 5' end partial");
    }
}

bool CMacroFunction_Clear5Partial::s_Clear5EndPartial(CSeq_feat& feat, CScope& scope, const string& descr)
{
    if (!ENUM_METHOD_NAME(EPartial_5_clear_constraint)()->IsValidName(descr)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized option to clear 5' partials: " + descr);
    }

    EPartial_5_clear_constraint filter =
        (EPartial_5_clear_constraint)ENUM_METHOD_NAME(EPartial_5_clear_constraint)()->FindValue(descr);

    edit::CLocationEditPolicy::EPartialPolicy policy5 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    switch (filter) {
    case ePartial_5_clear_constraint_all:
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClear;
        break;
    case ePartial_5_clear_constraint_not_at_end:
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd;
        break;
    case ePartial_5_clear_constraint_good_start:
        policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd;
        break;
    }

    bool extend5 = false, extend3 = false;
    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy(policy5, edit::CLocationEditPolicy::ePartialPolicy_eNoChange, extend5, extend3));
    return policy->ApplyPolicyToFeature(feat, scope);
}

bool CMacroFunction_Clear5Partial::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 3)
        return false;

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Clear3Partial
/// ClearPartialStop Clear3EndPartial("all"|"not-at-end"|"good-end" [,retranslate_cds][,adjust_gene])
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Clear3Partial, "ClearPartialStop")

void CMacroFunction_Clear3Partial::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = s_Clear3EndPartial(*m_Seqfeat, *m_Scope, m_Args[0]->GetString());
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.size() > 1) ? m_Args[1]->GetBool() : false;
        m_AdjustGene = (m_Args.size() == 3) ? m_Args[2]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Cleared 3' end partial");
    }
}

bool CMacroFunction_Clear3Partial::s_Clear3EndPartial(CSeq_feat& feat, CScope& scope, const string& descr)
{
    if (!ENUM_METHOD_NAME(EPartial_3_clear_constraint)()->IsValidName(descr)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized option to clear 3' partials: " + descr);
    }

    EPartial_3_clear_constraint filter =
        (EPartial_3_clear_constraint)ENUM_METHOD_NAME(EPartial_3_clear_constraint)()->FindValue(descr);

    edit::CLocationEditPolicy::EPartialPolicy policy3 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    switch (filter) {
    case ePartial_3_clear_constraint_all:
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClear;
        break;
    case ePartial_3_clear_constraint_not_at_end:
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd;
        break;
    case ePartial_3_clear_constraint_good_end:
        policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd;
        break;
    }

    bool extend5 = false, extend3 = false;
    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy(edit::CLocationEditPolicy::ePartialPolicy_eNoChange, policy3, extend5, extend3));
    return policy->ApplyPolicyToFeature(feat, scope);
}

bool CMacroFunction_Clear3Partial::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 3)
        return false;

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ConvertLocStrand
/// ConvertLocationStrand(from_strand, to_strand [,retranslate_cds][,adjust_gene])
/// from_strand can be one of {any, plus, minus, unknown, both}
/// to_strand can be one of {plus, minus, unknown, both, reverse}
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ConvertLocStrand, "ConvertLocationStrand")

const vector<SArgMetaData> CMacroFunction_ConvertLocStrand::sm_Arguments 
{ SArgMetaData("feature_field", CMQueryNodeValue::eString, false)
, SArgMetaData("from_strand", CMQueryNodeValue::eString, false)
, SArgMetaData("to_strand", CMQueryNodeValue::eString, false)
, SArgMetaData("retranslate", CMQueryNodeValue::eBool, true)
, SArgMetaData("adjustgene", CMQueryNodeValue::eBool, true)
};
void CMacroFunction_ConvertLocStrand::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_RetranslateCDS = (m_Args.size() > 2) ? m_Args[2]->GetBool() : false;
    m_AdjustGene = (m_Args.size() == 4) ? m_Args[3]->GetBool() : false;

    m_Modified = s_ConvertLocationStrand(*m_Seqfeat, *m_Scope, m_Args[0]->GetString(), m_Args[1]->GetString());
    if (m_Modified) {
        m_Seqfeat->SetPartial(m_Seqfeat->GetLocation().IsPartialStart(eExtreme_Biological)
                            || m_Seqfeat->GetLocation().IsPartialStop(eExtreme_Biological));

        m_DataIter->SetModified();
        string new_loc;
        m_Seqfeat->GetLocation().GetLabel(&new_loc);
        CNcbiOstrstream log;
        log << "Converted location strand for " << m_DataIter->GetBestDescr() << " to " << new_loc;

        
        x_RetranslateCDS(log);

        if (m_AdjustGene && !m_Seqfeat->GetData().IsGene()) {
            CConstRef<CSeq_feat> overlap_gene = sequence::GetGeneForFeature(*m_Origfeat, *m_Scope);
            if (overlap_gene) {
                CRef<CSeq_feat> new_gene(new CSeq_feat);
                new_gene->Assign(*overlap_gene);
                bool change_gene = s_ConvertLocationStrand(*new_gene, *m_Scope, m_Args[0]->GetString(), m_Args[1]->GetString());
                if (change_gene) {
                    CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(*overlap_gene);
                    CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_gene));
                    if (chgFeat) {
                        CRef<CCmdComposite> cmd(new CCmdComposite("Adjust gene"));
                        cmd->AddCommand(*chgFeat);
                        m_DataIter->RunCommand(cmd, m_CmdComposite);
                        log << " and adjusted gene location.";
                    }
                }
            }
        }
        x_LogFunction(log);
    }
}

ENa_strand CMacroFunction_ConvertLocStrand::s_GetStrandFromString(const string& str)
{
    ENa_strand strand = eNa_strand_unknown;
    if (NStr::EqualNocase(str, "minus")) {
        strand = eNa_strand_minus;
    }
    else if (NStr::EqualNocase(str, "plus")) {
        strand = eNa_strand_plus;
    }
    else if (NStr::EqualNocase(str, "both")) {
        strand = eNa_strand_both;
    }
    return strand;
}

bool CMacroFunction_ConvertLocStrand::s_ConvertLocationStrand(CSeq_feat& feat, CScope& scope, const string& str_from, const string& str_to)
{
    bool modified = false;
    if (!feat.IsSetLocation()) {
        return modified;
    }

    string from = str_from;
    NStr::ToLower(from);
    string to = str_to;
    NStr::ToLower(to);
    if (!ENUM_METHOD_NAME(EFeature_location_strand_from)()->IsValidName(from)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized source strand: " + from);
    }
    if (!ENUM_METHOD_NAME(EFeature_location_strand_to)()->IsValidName(to)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized target strand: " + to);
    }

    ENa_strand strand_from = s_GetStrandFromString(from);
    ENa_strand strand_to = s_GetStrandFromString(to);
    bool start_any = NStr::EqualNocase(from, "any");
    bool reverse = NStr::EqualNocase(to, "reverse");
    ENa_strand current_strand = eNa_strand_plus;


    if (feat.GetLocation().IsSetStrand())
        current_strand = feat.GetLocation().GetStrand();

    if (start_any || strand_from == current_strand) {
        // count for cases where the strand will be reversed
        if (!reverse) {
            if ((current_strand == eNa_strand_plus || current_strand == eNa_strand_unknown)
                && strand_to == eNa_strand_minus) {
                reverse = true;
            }
            else if (current_strand == eNa_strand_minus
                && (strand_to == eNa_strand_plus || strand_to == eNa_strand_unknown)) {
                reverse = true;
            }
        }

        if (reverse) {
            CRef<CSeq_loc> rev_loc(sequence::SeqLocRevCmpl(feat.GetLocation(), &scope));
            feat.SetLocation().Assign(*rev_loc);
            modified = true;
        }
        else if (strand_to != current_strand) {
            feat.SetLocation().SetStrand(strand_to);
            modified = true;
        }
    }

    return modified;
}

bool CMacroFunction_ConvertLocStrand::x_ValidArguments() const
{
    if (m_Args.size() < 2 || m_Args.size() > 4)
        return false;

    for (size_t i = 2; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString
        && m_Args[1]->GetDataType() == CMQueryNodeValue::eString;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ConvertLocType
/// ConvertLocationType("join"|"order"|"single-interval" [,retranslate_cds][,adjust_gene])
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ConvertLocType, "ConvertLocationType")
const vector<SArgMetaData> CMacroFunction_ConvertLocType::sm_Arguments
{ SArgMetaData("feature_field", CMQueryNodeValue::eString, false)
, SArgMetaData("loc_type", CMQueryNodeValue::eString, false)
, SArgMetaData("retranslate", CMQueryNodeValue::eBool, true)
, SArgMetaData("adjustgene", CMQueryNodeValue::eBool, true)
};
void CMacroFunction_ConvertLocType::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_RetranslateCDS = (m_Args.size() > 1) ? m_Args[1]->GetBool() : false;
    m_AdjustGene = (m_Args.size() == 3) ? m_Args[2]->GetBool() : false;

    m_Modified = s_ConvertLocationType(*m_Seqfeat, *m_Scope, m_Args[0]->GetString());
    if (m_Modified) {
        m_Seqfeat->SetPartial(m_Seqfeat->GetLocation().IsPartialStart(eExtreme_Biological)
            || m_Seqfeat->GetLocation().IsPartialStop(eExtreme_Biological));

        m_DataIter->SetModified();
        string new_loc;
        m_Seqfeat->GetLocation().GetLabel(&new_loc);
        CNcbiOstrstream log;
        log << "Converted location type for " << m_DataIter->GetBestDescr() << " to " << new_loc;


        x_RetranslateCDS(log);

        if (m_AdjustGene && !m_Seqfeat->GetData().IsGene()) {
            CConstRef<CSeq_feat> overlap_gene = sequence::GetGeneForFeature(*m_Origfeat, *m_Scope);
            if (overlap_gene) {
                CRef<CSeq_feat> new_gene(new CSeq_feat);
                new_gene->Assign(*overlap_gene);
                bool change_gene = s_ConvertLocationType(*new_gene, *m_Scope, m_Args[0]->GetString());
                if (change_gene) {
                    CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(*overlap_gene);
                    CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_gene));
                    if (chgFeat) {
                        CRef<CCmdComposite> cmd(new CCmdComposite("Adjust gene"));
                        cmd->AddCommand(*chgFeat);
                        m_DataIter->RunCommand(cmd, m_CmdComposite);
                        log << " and adjusted gene location.";
                    }
                }
            }
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_ConvertLocType::s_ConvertLocationType(CSeq_feat& feat, CScope& scope, const string& descr)
{
    edit::CLocationEditPolicy::EMergePolicy merge_type = edit::CLocationEditPolicy::eMergePolicy_NoChange;

    if (NStr::EqualNocase(descr, "join")) {
        merge_type = edit::CLocationEditPolicy::eMergePolicy_Join;
    }
    else if (NStr::EqualNocase(descr, "order")) {
        merge_type = edit::CLocationEditPolicy::eMergePolicy_Order;
    }
    else if (NStr::EqualNocase(descr, "single-interval")) {
        merge_type = edit::CLocationEditPolicy::eMergePolicy_SingleInterval;
    }
    else {
        NCBI_THROW(CException, eUnknown, "Unrecognized location merge policy: " + descr);
    }

    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy());
    policy->SetMergePolicy(merge_type);
    return policy->ApplyPolicyToFeature(feat, scope);
}

bool CMacroFunction_ConvertLocType::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 3)
        return false;

    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return m_Args[0]->GetDataType() == CMQueryNodeValue::eString;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Extend5Feature - extends the feature ends to the respective end of the sequence 
/// ExtendFeatToSeqStart([retranslate_cds][,adjust_gene]), 
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Extend5Feature, "ExtendFeatToSeqStart")
const vector<SArgMetaData> CMacroFunction_Extend5Feature::sm_Arguments
{ SArgMetaData("feature_field", CMQueryNodeValue::eString, false)
, SArgMetaData("retranslate", CMQueryNodeValue::eBool, true)
, SArgMetaData("adjustgene", CMQueryNodeValue::eBool, true)
};
void CMacroFunction_Extend5Feature::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = edit::CLocationEditPolicy::Extend5(*m_Seqfeat, *m_Scope);
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.empty()) ? false : m_Args[0]->GetBool();
        m_AdjustGene = (m_Args.size() == 2) ? m_Args[1]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Extended feature to 5' end");
    }
}

bool CMacroFunction_Extend5Feature::x_ValidArguments() const
{
    if (m_Args.size() > 2)
        return false;

    for (auto& it : m_Args) {
        if (it->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Extend3Feature - extends the feature ends to the respective end of the sequence 
/// ExtendFeatToSeqStop([retranslate_cds][,adjust_gene])
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Extend3Feature, "ExtendFeatToSeqStop")

void CMacroFunction_Extend3Feature::TheFunction()
{
    if (!x_CheckInitFeature())
        return;

    m_Modified = edit::CLocationEditPolicy::Extend3(*m_Seqfeat, *m_Scope);
    if (m_Modified) {
        m_RetranslateCDS = (m_Args.empty()) ? false : m_Args[0]->GetBool();
        m_AdjustGene = (m_Args.size() == 2) ? m_Args[1]->GetBool() : false;
        x_RetranslateCDSAdjustGene("Extended feature to 3' end");
    }
}

bool CMacroFunction_Extend3Feature::x_ValidArguments() const
{
    if (m_Args.size() > 2)
        return false;

    for (auto& it : m_Args) {
        if (it->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ApplyCDSFrame
/// ApplyCDSFrame("best"|"match")
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyCDSFrame, "ApplyCDSFrame")

void CMacroFunction_ApplyCDSFrame::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* cds = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!cds || !cds->GetData().IsCdregion() || !scope) {
        return;
    }
    
    ECdsFrame frame = s_GetFrameFromName(m_Args[0]->GetString());
    if (frame == eNotSet) {
        // warn the user about the existing options
        return; 
    }
    bool modified = s_SetCDSFrame(*cds, frame, *scope);
    
    if (modified) {
        m_DataIter->SetModified();

        //note: if product existed before, it will be retranslated
        if (cds->IsSetProduct()) {
            CRef<CMacroBioData_FeatIterBase> feat_iter =
                Ref(dynamic_cast<CMacroBioData_FeatIterBase*>(m_DataIter.GetPointer()));
            if (m_DataIter->IsBegin() && feat_iter) {
                feat_iter->SetCreateGeneralIdFlag();
            }

            bool cds_change = false;
            // this command will not make changes in the cds
            CRef<CCmdComposite> upd_cmd = GetRetranslateCDSCommand(*scope, *cds, feat_iter->GetCreateGeneralIdFlag());
            if (upd_cmd) {
                m_DataIter->RunCommand(upd_cmd, m_CmdComposite);
            }
        }
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": the " << m_Args[0]->GetString() << " frame was set";
        x_LogFunction(log);
    }
}

bool CMacroFunction_ApplyCDSFrame::s_SetCDSFrame(CSeq_feat& cds, CMacroFunction_ApplyCDSFrame::ECdsFrame frame_type, CScope& scope)
{
    if (!cds.IsSetData() || !cds.GetData().IsCdregion())
        return false;

    CCdregion::TFrame orig_frame = CCdregion::eFrame_not_set;
    if (cds.GetData().GetCdregion().IsSetFrame()) {
        orig_frame = cds.GetData().GetCdregion().GetFrame();
    }
    // retrieve the new frame
    CCdregion::TFrame new_frame = orig_frame;
    switch (frame_type) {
    case eNotSet:
        break;
    case eBest:
        new_frame = CSeqTranslator::FindBestFrame(cds, scope);
        break;
    case eMatch:
        new_frame = s_FindMatchingFrame(cds, scope);
        break;
    }

    bool modified = false;
    if (orig_frame != new_frame) {
        cds.SetData().SetCdregion().SetFrame(new_frame);
        modified = true;
    }
    return modified;
}

CCdregion::EFrame CMacroFunction_ApplyCDSFrame::s_FindMatchingFrame(const CSeq_feat& cds, CScope& scope)
{
    CCdregion::TFrame new_frame = CCdregion::eFrame_not_set;
    //return the frame that matches the protein sequence if it can find one
    if (!cds.IsSetData() || !cds.GetData().IsCdregion() || !cds.IsSetLocation() || !cds.IsSetProduct()) {
        return new_frame;
    }

    // get the protein sequence
    CBioseq_Handle product = scope.GetBioseqHandle(cds.GetProduct());
    if (!product || !product.IsProtein()) {
        return new_frame;
    }

    // obtaining the original protein sequence
    CSeqVector prot_vec = product.GetSeqVector(CBioseq_Handle::eCoding_Ncbi);
    prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
    string orig_prot_seq;
    prot_vec.GetSeqData(0, prot_vec.size(), orig_prot_seq);
    if (NStr::IsBlank(orig_prot_seq)) {
        return new_frame;
    }

    CRef<CSeq_feat> tmp_cds(new CSeq_feat);
    tmp_cds->Assign(cds);
    for (int enumI = CCdregion::eFrame_one; enumI < CCdregion::eFrame_three + 1; ++enumI) {
        CCdregion::EFrame fr = (CCdregion::EFrame) (enumI);
        tmp_cds->SetData().SetCdregion().SetFrame(fr);
    
        string new_prot_seq;
        CSeqTranslator::Translate(*tmp_cds, scope, new_prot_seq);
        if (NStr::EndsWith(new_prot_seq, '*'))
            new_prot_seq.erase(new_prot_seq.end() - 1);
        if (NStr::EqualNocase(new_prot_seq, orig_prot_seq)) {
            new_frame = fr;
            break;
        }
    }

    return new_frame;
}

CMacroFunction_ApplyCDSFrame::ECdsFrame CMacroFunction_ApplyCDSFrame::s_GetFrameFromName(const string& name)
{
    ECdsFrame frame = eNotSet;
    if (NStr::EqualNocase(name, "best")) {
        frame = eBest;
    } else if (NStr::EqualNocase(name, "match")) {
        frame = eMatch;
    }
    return frame;
}

bool CMacroFunction_ApplyCDSFrame::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ConvertFeature
/// ConvertFeature(feat_type);
/// ConvertFeature(feat_type, subtype) // subtype can be used for specifying type of bond, ncRNA class or type of site
/// ConvertFeature(feat_type, create_on_protein) // when the feat_type is region, create_on_protein is of bool type
/// ConvertCDSToFeature(feat_type, rem_gene, rem_mRNA, rem_transcript_id);
/// ConvertCDSToFeature(feat_type, subtype, rem_gene, rem_mRNA, rem_transcript_id);
/// ConvertCDSToFeature(feat_type, create_on_protein, rem_gene, rem_mRNA, rem_transcript_id);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ConvertFeature, "ConvertFeature")
DEFINE_MACRO_FUNCNAME(CMacroFunction_ConvertCDS, "ConvertCDSToFeature")

void CMacroFunction_ConvertFeature::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* from_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* const_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!from_feat || !const_feat || !scope) {
        return;
    }

    const string& to_feat = m_Args[0]->GetString();
    m_FromType = from_feat->GetData().GetSubtype();
    m_ToType = NMacroUtil::GetFeatSubtype(to_feat);

    CRef<CConvertFeatureBase> converter = CConvertFeatureBaseFactory::Create(m_FromType, m_ToType);
    // when conversion is not supported
    if (NStr::EqualNocase(converter->GetDescription(), kConversionNotSupported)) {
        CNcbiOstrstream log;
        log << kConversionNotSupported;
        x_LogFunction(log);
        return;
    }

    x_SetConvertOptions(converter);

    bool keep_orig(true);
    CRef<CCmdComposite> convert_cmd = converter->Convert(*from_feat, keep_orig, *scope);

    if (convert_cmd) {
        m_DataIter->RunCommand(convert_cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << "Converted " << m_DataIter->GetBestDescr() << " to " << to_feat;
        x_LogFunction(log);
    }
}

bool CMacroFunction_ConvertFeature::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args[0]->GetDataType() != CMQueryNodeValue::eString) {
        return false;
    }
    
    if (m_Args.size() == 2 && (m_Args[1]->GetDataType() != CMQueryNodeValue::eString && m_Args[1]->GetDataType() != CMQueryNodeValue::eBool)) {
        return false;
    }
    return true;
}

void CMacroFunction_ConvertFeature::x_SetConvertOptions(CRef<CConvertFeatureBase> converter)
{
    CConvertFeatureBase::TOptions& opts = converter->SetOptions();
    if (m_FromType == CSeqFeatData::eSubtype_cdregion) {
        size_t index = (m_Args[1]->GetDataType() != CMQueryNodeValue::eBool) ? 2 : 1;
        NON_CONST_ITERATE(CConvertFeatureBase::TOptions, it, opts) {
            const string& opt_label = (*it)->GetLabel();
            if (NStr::EqualNocase(opt_label, CConvertFeatureBase::s_RemovemRNA)) {
                (*it)->SetBool()->SetVal(m_Args[index]->GetBool());
                ++index;
            }
            else if (NStr::EqualNocase(opt_label, CConvertFeatureBase::s_RemoveGene)) {
                (*it)->SetBool()->SetVal(m_Args[index]->GetBool());
                ++index;
            }
            else if (NStr::EqualNocase(opt_label, CConvertFeatureBase::s_RemoveTranscriptID)) {
                (*it)->SetBool()->SetVal(m_Args[index]->GetBool());
                ++index;
            }
        }
    }

    if (m_ToType == CSeqFeatData::eSubtype_ncRNA) {
        NON_CONST_ITERATE(CConvertFeatureBase::TOptions, it, opts) {
            const string& opt_label = (*it)->GetLabel();
            if (NStr::EqualNocase(opt_label, CConvertFeatureBase::s_NcrnaClass)) {
                (*it)->SetString()->SetVal(m_Args[1]->GetString());
            }
        }
    } else if (m_ToType == CSeqFeatData::eSubtype_site) {
        NON_CONST_ITERATE(CConvertFeatureBase::TOptions, it, opts) {
            const string& opt_label = (*it)->GetLabel();
            if (NStr::EqualNocase(opt_label, CConvertFeatureBase::s_SiteType)) {
                (*it)->SetString()->SetVal(m_Args[1]->GetString());
            }
        }
    } else if (m_ToType == CSeqFeatData::eSubtype_bond) {
        NON_CONST_ITERATE(CConvertFeatureBase::TOptions, it, opts) {
            const string& opt_label = (*it)->GetLabel();
            if (NStr::EqualNocase(opt_label, CConvertFeatureBase::s_BondType)) {
                (*it)->SetString()->SetVal(m_Args[1]->GetString());
            }
        }
    } else if (m_ToType == CSeqFeatData::eSubtype_region
        && CSeqFeatData::GetTypeFromSubtype(m_FromType) != CSeqFeatData::e_Prot) {
        NON_CONST_ITERATE(CConvertFeatureBase::TOptions, it, opts) {
            const string& opt_label = (*it)->GetLabel();
            if (NStr::EqualNocase(opt_label, CConvertFeatureBase::s_PlaceOnProt)) {
                (*it)->SetBool()->SetVal(m_Args[1]->GetBool());
            }
        }
    }
}

bool CMacroFunction_ConvertCDS::x_ValidArguments() const
{
    if (m_Args.empty() || 
        m_Args.size() < 4 ||
        m_Args[0]->GetDataType() != CMQueryNodeValue::eString) {
        return false;
    }

    if (m_Args[1]->GetDataType() != CMQueryNodeValue::eString && m_Args[1]->GetDataType() != CMQueryNodeValue::eBool) {
        return false;
    }

    for (size_t index = m_Args.size() - 1; index < m_Args.size() - 3; --index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eBool) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AddGeneXref
/// AddGeneXref(); - add a gene Xref to the selected feature
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddGeneXref, "AddGeneXref")

void CMacroFunction_AddGeneXref::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || (edit_feat->IsSetData() && edit_feat->GetData().IsGene()) || !scope) {
        return;
    }
    
    CConstRef<CSeq_feat> gene = sequence::GetOverlappingGene(edit_feat->GetLocation(), *scope);
    if (!gene || !gene->IsSetData() || !gene->GetData().IsGene()) 
        return;

    edit_feat->SetGeneXref().Assign(gene->GetData().GetGene());
    m_DataIter->SetModified();

    CNcbiOstrstream log;
    log << "Added Gene xref to " << m_DataIter->GetBestDescr();
    x_LogFunction(log);
}

bool CMacroFunction_AddGeneXref::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveGeneXref
/// RemoveGeneXref(suppressing_type, necessary_type);
/// suppressing_type = {"any", "suppressing", "nonsuppressing"}
/// necessary_type = {"any", "necessary", "unnecessary"}
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveGeneXref, "RemoveGeneXref")

void CMacroFunction_RemoveGeneXref::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || !edit_feat->IsSetXref() || !scope) {
        return;
    }

    EGene_xref_suppression_type suppr_type = eGene_xref_suppression_type_any;
    const string& suppr_str = m_Args[0]->GetString();
    if (NStr::EqualNocase(suppr_str, "suppressing")) {
        suppr_type = eGene_xref_suppression_type_suppressing;
    } else if (NStr::EqualNocase(suppr_str, "nonsuppressing")) {
        suppr_type = eGene_xref_suppression_type_non_suppressing;
    }

    EGene_xref_necessary_type necessary_type = eGene_xref_necessary_type_any;
    const string& necessary_str = m_Args[0]->GetString();
    if (NStr::EqualNocase(necessary_str, "necessary")) {
        necessary_type = eGene_xref_necessary_type_necessary;
    } else if (NStr::EqualNocase(necessary_str, "unnecessary")) {
        necessary_type = eGene_xref_necessary_type_unnecessary;
    }
    
    bool removed(false);
    CSeq_feat::TXref::iterator xref_it = edit_feat->SetXref().begin();
    while (xref_it != edit_feat->SetXref().end()) {
        if ((*xref_it)->IsSetData() && (*xref_it)->GetData().IsGene() &&
            s_GeneXrefMatchesSuppression((*xref_it)->GetData().GetGene(), suppr_type) &&
            s_GeneXrefMatchesNecessary((*xref_it)->GetData().GetGene(), *edit_feat, *scope, necessary_type)) {
                xref_it = edit_feat->SetXref().erase(xref_it);
                removed = true;
            }
            else {
                ++xref_it;
            }
    }
    
    if (removed) {
        if (edit_feat->IsSetXref() && edit_feat->GetXref().empty()) {
            edit_feat->ResetXref();
        }

        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << "Removed Gene xref from " << m_DataIter->GetBestDescr();
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveGeneXref::x_ValidArguments() const
{
    if (m_Args.size() != 2) {
        return false;
    }

    return (m_Args[0]->GetDataType() == CMQueryNodeValue::eString &&
            m_Args[1]->GetDataType() == CMQueryNodeValue::eString);
}

bool CMacroFunction_RemoveGeneXref::s_GeneXrefMatchesNecessary(const CGene_ref& gene, const CSeq_feat& feat, CScope& scope,
    EGene_xref_necessary_type necessary_type)
{
    bool rval(false);
    switch (necessary_type) {
    case eGene_xref_necessary_type_any:
        rval = true;
        break;
    case eGene_xref_necessary_type_necessary:
        rval = !CCleanup::IsGeneXrefUnnecessary(feat, scope, gene);
        break;
    case eGene_xref_necessary_type_unnecessary:
        rval = CCleanup::IsGeneXrefUnnecessary(feat, scope, gene);
        break;
    }
    return rval;
}

bool CMacroFunction_RemoveGeneXref::s_GeneXrefMatchesSuppression(const CGene_ref& gene, EGene_xref_suppression_type suppr_type)
{
    bool rval(false);
    switch (suppr_type) {
    case eGene_xref_suppression_type_any:
        rval = true;
        break;
    case eGene_xref_suppression_type_suppressing:
        rval = gene.IsSuppressed();
        break;
    case eGene_xref_suppression_type_non_suppressing:
        rval = !gene.IsSuppressed();
        break;
    }
    return rval;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_JoinShorttRNAs
/// JoinShorttRNAs() - joins short (< 50bp) tRNAs and extends the corresponding genes
///
static bool s_SortShorttRNAs(const CConstRef<CSeq_feat>& feat1, const CConstRef<CSeq_feat>& feat2)
{
    const string& product1 = feat1->GetData().GetRna().GetRnaProductName();
    const string& product2 = feat2->GetData().GetRna().GetRnaProductName();

    ENa_strand strand1 = feat1->GetLocation().GetStrand();
    ENa_strand strand2 = feat2->GetLocation().GetStrand();

    const TSeqPos start1 = feat1->GetLocation().GetStart(eExtreme_Positional);
    const TSeqPos start2 = feat2->GetLocation().GetStart(eExtreme_Positional);
    if (NStr::EqualNocase(product1, product2)) {
        if (strand1 == strand2) {
            if (strand1 == eNa_strand_minus) {
                return (start1 > start2);
            }
            else {
                return (start1 < start2);
            }
        }
        else {
            return strand1 < strand2;
        }
    }
    return product1 < product2;
}

static bool s_ShouldBeMerged(const CSeq_feat& feat1, const CSeq_feat& feat2)
{
    string product1 = feat1.GetData().GetRna().GetRnaProductName();
    string product2 = feat2.GetData().GetRna().GetRnaProductName();

    ENa_strand strand1 = feat1.GetLocation().GetStrand();
    ENa_strand strand2 = feat2.GetLocation().GetStrand();

    bool merged = NStr::EqualNocase(product1, product2) &&
                  ((strand1 == eNa_strand_minus && strand2 == eNa_strand_minus) 
                    || (strand1 != eNa_strand_minus && strand2 != eNa_strand_minus));
    return merged;
}

DEFINE_MACRO_FUNCNAME(CMacroFunction_JoinShorttRNAs, "JoinShorttRNAs")

void CMacroFunction_JoinShorttRNAs::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || !scope)
        return;

    vector<CConstRef<CSeq_feat> > tRNAFeats;
    CBioseq_Handle bsh = scope->GetBioseqHandle(*bseq);
    for (CFeat_CI feat_it(bsh, SAnnotSelector(CSeqFeatData::eSubtype_tRNA)); feat_it; ++feat_it) {
        const CSeq_feat& feature = *feat_it->GetOriginalSeq_feat();
        if (sequence::GetLength(feature.GetLocation(), scope) < 50) { // short tRNA
            tRNAFeats.push_back(feat_it->GetOriginalSeq_feat());
        }
    }
    
    sort(tRNAFeats.begin(), tRNAFeats.end(), s_SortShorttRNAs);

    CNcbiOstrstream log;
    CRef<CCmdComposite> cmd(new CCmdComposite("Join short tRNAs"));
    auto it = tRNAFeats.begin();
    auto modified = false;
    while (it != tRNAFeats.end()) {
        auto next = it;
        ++next;

        CRef<CSeq_loc> rnaLoc(new CSeq_loc);
        vector<string> comments;

        CConstRef<CSeq_feat> gene = sequence::GetGeneForFeature(**it, *scope);
        CRef<CSeq_loc> geneLoc(new CSeq_loc);
        
        while (next != tRNAFeats.end() && s_ShouldBeMerged(**it, **next)) {
            rnaLoc->Add((*next)->GetLocation());
            if ((*next)->IsSetComment()) {
                comments.push_back((*next)->GetComment());
            }
            
            if (gene) {
                CConstRef<CSeq_feat> geneNext = sequence::GetGeneForFeature(**next, *scope);
                if (geneNext) {
                    geneLoc->Add(geneNext->GetLocation());
                    CSeq_feat_Handle gNexth = scope->GetSeq_featHandle(*geneNext);
                    CRef<CCmdComposite> delGene = GetDeleteFeatureCommand(gNexth);
                    cmd->AddCommand(*delGene);
                    modified = true;
                } else {
                    geneLoc->Add((*next)->GetLocation());
                }
            }

            CSeq_feat_Handle featNexth = scope->GetSeq_featHandle(**next);
            CRef<CCmdComposite> delNext = GetDeleteFeatureCommand(featNexth);
            cmd->AddCommand(*delNext);
            modified = true;
            ++next;
        }

        if (rnaLoc->Which() != CSeq_loc::e_not_set || !comments.empty()) {
            CRef<CSeq_feat> newtRNA(new CSeq_feat);
            newtRNA->Assign(**it);

            rnaLoc->Add((*it)->GetLocation());
            rnaLoc = sequence::Seq_loc_Merge(*rnaLoc, CSeq_loc::fSortAndMerge_All, scope);
            newtRNA->SetLocation(*rnaLoc);

            if (!comments.empty()) {
                string origComment = (*it)->IsSetComment() ? (*it)->GetComment() : kEmptyStr;
                size_t index = 0;
                if (origComment.empty()) {
                    origComment = comments[0];
                    ++index;
                }
                for ( ; index < comments.size(); ++index) {
                    edit::AddValueToString(origComment, comments[index], edit::eExistingText_append_semi);
                }
                if (!origComment.empty()) {
                    newtRNA->SetComment(origComment);
                }
            }

            CSeq_feat_Handle fh = scope->GetSeq_featHandle(**it);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *newtRNA)));
            
            CSeq_loc_CI loc_ci(newtRNA->GetLocation());
            if (!IsOssEmpty(log)) {
                log << "\n";
            }
            log << "Joined " << loc_ci.GetSize() << " short tRNAs with product name " << newtRNA->GetData().GetRna().GetRnaProductName();
            modified = true;

            if (geneLoc->Which() != CSeq_loc::e_not_set) {
                geneLoc->Add(gene->GetLocation());
                geneLoc = sequence::Seq_loc_Merge(*geneLoc, CSeq_loc::fSortAndMerge_All|CSeq_loc::fMerge_SingleRange, scope);
                if (geneLoc) {
                    CRef<CSeq_feat> newGene(new CSeq_feat);
                    newGene->Assign(*gene);
                    newGene->SetLocation(*geneLoc);

                    CSeq_feat_Handle geneh = scope->GetSeq_featHandle(*gene);
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(geneh, *newGene)));
                    string label;
                    newGene->GetLocation().GetLabel(&label);
                    log << "\n" << "Corresponding gene has been extended to " << label << "\n";
                    modified = true;
                }
            }
        }
        it = next;
    }

    if (modified) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        x_LogFunction(log);
    }
}

bool CMacroFunction_JoinShorttRNAs::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ReplaceStopWithSelenocysteine
/// ReplaceSelenocysteineStops();
/// Replace all stops with selenocysteine (U) if the codon is TGA and the product name contains "seleno"
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_ReplaceStopWithSelenocysteine, "ReplaceSelenocysteineStops")

void CMacroFunction_ReplaceStopWithSelenocysteine::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CRef<CSeq_feat> cds_feat = Ref(CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr()));
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!cds_feat ||
        !cds_feat->GetData().IsCdregion() ||
        !cds_feat->IsSetProduct() ||
        !scope) {
        return;
    }

    // check if protein name contains "seleno"
    CBioseq_Handle protein = scope->GetBioseqHandle(cds_feat->GetProduct());
    CFeat_CI feat_it(protein, SAnnotSelector(CSeqFeatData::eSubtype_prot));
    if (!feat_it || !feat_it->GetData().GetProt().IsSetName()) {
        return;
    }

    const CProt_ref::TName& names = feat_it->GetData().GetProt().GetName();
    auto it = find_if(names.begin(), names.end(), [](const string& str) { return (NStr::Find(str, "seleno") != NPOS); });
    if (it == names.end()) {
        return;
    }

    // check if protein has stop codons
    CSeqVector prot_vec = protein.GetSeqVector(CBioseq_Handle::eCoding_Ncbi);
    prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
    string prot_seq;
    prot_vec.GetSeqData(0, prot_vec.size(), prot_seq);
    if (NStr::Find(prot_seq, "*") == NPOS) {
        return;
    }

    // protein id and label
    CConstRef<CSeq_id> prot_id = protein.GetAccessSeq_id_Handle().GetSeqId();
    string prot_label;
    prot_id->GetLabel(&prot_label);

    CBioseq_Handle bsh = scope->GetBioseqHandle(cds_feat->GetLocation());
    CSeqVector nuc_vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
    nuc_vec.SetIupacCoding();

    unsigned int replaced = 0;
    CNcbiOstrstream log;
    
    CRef<CSeq_loc_Mapper> prot2nuc(new CSeq_loc_Mapper(*cds_feat, CSeq_loc_Mapper::eProductToLocation, scope));
    for (size_t index = 0; index < prot_seq.size(); ++index) {
        if (prot_seq[index] == '*') {
            CRef<CSeq_loc> prot_loc = x_GetProteinLoc(*prot_id, (TSeqPos)index);
            CRef<CSeq_loc> nuc_loc = prot2nuc->Map(*prot_loc);
            if (!nuc_loc) {
                continue;
            }

            string codon;
            nuc_vec.GetSeqData(nuc_loc->GetStart(eExtreme_Biological), nuc_loc->GetStop(eExtreme_Biological) +1 , codon);

            if (codon == "TGA") {
                CRef<CCode_break> code_break(new CCode_break);
                code_break->SetLoc(*nuc_loc);
                code_break->SetAa().SetNcbieaa('U');
                cds_feat->SetData().SetCdregion().SetCode_break().push_back(code_break);
                replaced++;
            } 
            else {
                log << "Unable to add transl_except for stop codon at position " << index + 1;
                log << " in protein " << prot_label << " because codon is not TGA\n";
            }
        }
    }

    if (replaced) {
        // retranslate CDS
        if (!sequence::IsPseudo(*cds_feat, *scope)) {
            CRef<CMacroBioData_FeatIterBase> feat_iter =
                Ref(dynamic_cast<CMacroBioData_FeatIterBase*>(m_DataIter.GetPointer()));
            if (m_DataIter->IsBegin() && feat_iter) {
                feat_iter->SetCreateGeneralIdFlag();
            }

            bool transl_change = false;
            CRef<CCmdComposite> retransl_cmd = GetRetranslateCDSCommand(*scope, *cds_feat, transl_change, feat_iter->GetCreateGeneralIdFlag());
            if (retransl_cmd) {
                m_DataIter->RunCommand(retransl_cmd, m_CmdComposite);
            }
        }
    
        m_DataIter->SetModified();
        log << "Replaced " << replaced << " stops with selenocysteine.";
        x_LogFunction(log);
    }
}

bool CMacroFunction_ReplaceStopWithSelenocysteine::x_ValidArguments() const
{
    return (m_Args.empty());
}

CRef<CSeq_loc> CMacroFunction_ReplaceStopWithSelenocysteine::x_GetProteinLoc(const CSeq_id& prot_id, TSeqPos pos) const
{
    CRef<CSeq_id> new_id(new CSeq_id);
    SerialAssign(*new_id, prot_id);
    CRef<CSeq_point> prot_pnt(new CSeq_point(*new_id, pos));

    CRef<CSeq_loc> prot_loc(new CSeq_loc);
    prot_loc->SetPnt(*prot_pnt);
    return prot_loc;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RetranslateCDS
/// RetranslateCDS(obey_stop_codon);
///

// Changes in the function and parameter names require changes in the respective
// XRC file used in the macro editor
DEFINE_MACRO_FUNCNAME(CMacroFunction_RetranslateCDS, "RetranslateCDS")

void CMacroFunction_RetranslateCDS::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CRef<CSeq_feat> cds = Ref(CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr()));
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!cds ||
        !cds->GetData().IsCdregion() ||
        !scope ||
        sequence::IsPseudo(*cds, *scope)) { // check if feature is pseudo
        return;
    }

    bool obey_stop_codon = m_Args[0]->GetBool();
    bool cds_change = (obey_stop_codon && edit::TruncateCDSAtStop(*cds, *scope)) ? true : false;

    CRef<CMacroBioData_FeatIterBase> feat_iter =
        Ref(dynamic_cast<CMacroBioData_FeatIterBase*>(m_DataIter.GetPointer()));
    if (m_DataIter->IsBegin() && feat_iter) {
        feat_iter->SetCreateGeneralIdFlag();
    }

    bool transl_change = false;
    CRef<CCmdComposite> retransl_cmd = GetRetranslateCDSCommand(*scope, *cds, transl_change, feat_iter->GetCreateGeneralIdFlag());
    if (retransl_cmd) {
        CNcbiOstrstream log;
        m_DataIter->RunCommand(retransl_cmd, m_CmdComposite);
        if (cds_change || transl_change) {
            m_DataIter->SetModified();
        }
        log << "Retranslated " << m_DataIter->GetBestDescr();
        x_LogFunction(log);
    }
}

bool CMacroFunction_RetranslateCDS::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eBool);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_MakeLocation
/// Generates different type of seq-loc objects
///

bool CMacroFunction_MakeLocation::x_SetCurrentBioseq()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || !scope)
        return false;

    m_Bsh = scope->GetBioseqHandle(*bseq);
    return (m_Bsh);
}

void CMacroFunction_MakeLocation::x_HandleNegativeCoordinates(size_t index)
{
    if (index < m_Args.size()) {
        _ASSERT(m_Args[index]->GetDataType() == CMQueryNodeValue::eInt);
        if (m_Args[index]->GetInt() < 1) {
            NCBI_THROW(CException, eUnknown, "Negative sequence coordinate(s) provided: " + NStr::Int8ToString(m_Args[index]->GetInt()));
        }
    }
}

ENa_strand CMacroFunction_MakeLocation::x_GetStrand(const string& str)
{
    if (!ENUM_METHOD_NAME(ENa_strand)()->IsValidName(str)) {
        NCBI_THROW(CException, eUnknown, "Unrecognized strand: " + str);
    }

    return (ENa_strand)ENUM_METHOD_NAME(ENa_strand)()->FindValue(str);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_MakeInterval
/// MakeInterval(from, to, partial5(bool), partial3(bool), "plus"|"minus")
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_MakeInterval, "MakeInterval")
const vector<SArgMetaData> CMacroFunction_MakeInterval::sm_Arguments
{ SArgMetaData("partial5", CMQueryNodeValue::eBool, false)
, SArgMetaData("partial3", CMQueryNodeValue::eBool, false)
, SArgMetaData("loc_strand", CMQueryNodeValue::eString, false)
, SArgMetaData("use_whole_seq", CMQueryNodeValue::eBool, false)
, SArgMetaData("use_coord", CMQueryNodeValue::eBool, false)
, SArgMetaData("from_coord", CMQueryNodeValue::eInt, false)
, SArgMetaData("to_coord", CMQueryNodeValue::eInt, false)
};
void CMacroFunction_MakeInterval::TheFunction()
{
    if (!x_SetCurrentBioseq())
        return;

    x_HandleNegativeCoordinates(0);
    x_HandleNegativeCoordinates(1);

    ENa_strand strand = x_GetStrand(m_Args.back()->GetString());
    TSeqPos from = (TSeqPos)m_Args[0]->GetInt() - 1;
    TSeqPos to = (TSeqPos)m_Args[1]->GetInt() - 1;
    if (from > to) {
        swap(from, to);
    }
    if (to > m_Bsh.GetBioseqLength() - 1) {
        to = m_Bsh.GetBioseqLength() - 1;
    }
    CRef<CSeq_loc> location = m_Bsh.GetRangeSeq_loc(from, to, strand);
    if (m_Args[2]->GetBool()) {
        location->SetPartialStart(true, eExtreme_Biological);
    }
    if (m_Args[3]->GetBool()) {
        location->SetPartialStop(true, eExtreme_Biological);
    }
    
    CObjectInfo objInfo(location, location->GetTypeInfo());
    ResolveIdentToObjects(objInfo, kEmptyStr, m_Result.GetNCObject());
}

bool CMacroFunction_MakeInterval::x_ValidArguments() const
{
    if (m_Args.size() != 5) 
        return false;

    bool good = false;
    good = m_Args[0]->GetDataType() == CMQueryNodeValue::eInt
        && m_Args[1]->GetDataType() == CMQueryNodeValue::eInt
        && m_Args[2]->GetDataType() == CMQueryNodeValue::eBool
        && m_Args[3]->GetDataType() == CMQueryNodeValue::eBool
        && m_Args[4]->GetDataType() == CMQueryNodeValue::eString;
    return good;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_MakeWholeSeqInterval
/// MakeWholeSeqInterval(partial5, partial3, "plus" | "minus")
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_MakeWholeSeqInterval, "MakeWholeSeqInterval")

void CMacroFunction_MakeWholeSeqInterval::TheFunction()
{
    if (!x_SetCurrentBioseq())
        return;

    ENa_strand strand = x_GetStrand(m_Args.back()->GetString());
    CRef<CSeq_loc> location = m_Bsh.GetRangeSeq_loc(0, 0, strand);
    if (m_Args[0]->GetBool()) {
        location->SetPartialStart(true, eExtreme_Biological);
    }
    if (m_Args[1]->GetBool()) {
        location->SetPartialStop(true, eExtreme_Biological);
    }

    CObjectInfo objInfo(location, location->GetTypeInfo());
    ResolveIdentToObjects(objInfo, kEmptyStr, m_Result.GetNCObject());
}

bool CMacroFunction_MakeWholeSeqInterval::x_ValidArguments() const
{
    if (m_Args.size() != 3)
        return false;

    bool good = false;
    good =  m_Args[0]->GetDataType() == CMQueryNodeValue::eBool
            && m_Args[1]->GetDataType() == CMQueryNodeValue::eBool
            && m_Args[2]->GetDataType() == CMQueryNodeValue::eString;
    return good;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_MakePoint
/// MakePoint(point(integer), partial5, partial3, strand)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_MakePoint, "MakePoint")

void CMacroFunction_MakePoint::TheFunction()
{
    if (!x_SetCurrentBioseq())
        return;

    x_HandleNegativeCoordinates(0);
    ENa_strand strand = x_GetStrand(m_Args.back()->GetString());
    TSeqPos point = (TSeqPos)m_Args[0]->GetInt() - 1;
    if (point > m_Bsh.GetBioseqLength() - 1) {
        point = m_Bsh.GetBioseqLength() - 1;
    }

    CRef<CSeq_loc> location(new CSeq_loc);
    location->SetPnt().SetPoint(point);
    location->SetPnt().SetId().Assign(*m_Bsh.GetCompleteBioseq()->GetFirstId());
    location->SetPnt().SetStrand(strand);
    if (m_Args[1]->GetBool()) {
        location->SetPartialStart(true, eExtreme_Biological);
    }
    if (m_Args[2]->GetBool()) {
        location->SetPartialStop(true, eExtreme_Biological);
    }

    CObjectInfo objInfo(location, location->GetTypeInfo());
    ResolveIdentToObjects(objInfo, kEmptyStr, m_Result.GetNCObject());
}

bool CMacroFunction_MakePoint::x_ValidArguments() const
{
    if (m_Args.size() != 4) 
        return false;

    bool good = false;
    good = m_Args[0]->GetDataType() == CMQueryNodeValue::eInt
        && m_Args[1]->GetDataType() == CMQueryNodeValue::eBool
        && m_Args[2]->GetDataType() == CMQueryNodeValue::eBool
        && m_Args[3]->GetDataType() == CMQueryNodeValue::eString;
    return good;
}


/// class CMacroFunction_ApplyFeature
/// Generates different types of apply feature commands

bool CMacroFunction_ApplyFeature::x_ShouldAddRedundant(size_t index)
{
    _ASSERT(!m_NewFeat.IsNull());
    bool add_redundant = m_Args[index]->GetBool();
    if (add_redundant) {
        return true;
    }

    _ASSERT(m_Bsh.GetCompleteBioseq());
    CFeat_CI feat_it(m_Bsh, m_NewFeat->GetData().GetSubtype());
    return !(feat_it);
}

bool CMacroFunction_ApplyFeature::x_SetLocation(size_t index)
{
    CMQueryNodeValue::TObs location_obj;
    x_GetObjectsFromRef(location_obj, index);
    if (location_obj.empty()) {
        return false;
    }

    if (NStr::EqualCase(location_obj.front().field.GetName(), "Seq-loc")) {
        m_Location.Reset(CTypeConverter<CSeq_loc>::SafeCast(location_obj.front().field.GetObjectPtr()));
    }

    if (!m_Location) {
        NCBI_THROW(CMacroExecException, eVarNotResolved, "Location run-time variable could not be resolved");
    }

    _ASSERT(!m_NewFeat.IsNull());
    m_NewFeat->SetLocation().Assign(*m_Location);
    if (m_NewFeat->GetLocation().IsPartialStart(objects::eExtreme_Biological)
        || m_NewFeat->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
        m_NewFeat->SetPartial(true);
    }
    return true;
}

void CMacroFunction_ApplyFeature::x_SetAdditionalFields(size_t index)
{
    _ASSERT(!m_NewFeat.IsNull());
    size_t arg_size = m_Args.size();
    while (index < arg_size) {
        const string field = m_Args[index]->GetString();
        if (field == "qual.qual") { // it will be a GB-qualifier
            const string qualifier = m_Args[++index]->GetString();
            index += 2;
            const string value = (index < arg_size) ? m_Args[index]->GetString() : kEmptyStr;
            CRef<CGb_qual> new_gbqual(new CGb_qual(qualifier, value));
            m_NewFeat->SetQual().push_back(new_gbqual);
            ++index;
        }
        else {
            CObjectInfo oi(m_NewFeat, m_NewFeat->GetTypeInfo());
            CMQueryNodeValue::TObs res_oi;
            SetFieldsByName(&res_oi, oi, field);

            ++index;
            if (index < arg_size) {
                CMQueryNodeValue& new_value = *m_Args[index];
                NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {
                    if (it->field.GetTypeFamily() == eTypeFamilyContainer) {
                        CObjectInfo new_oi(it->field.AddNewElement());
                        SetSimpleTypeValue(new_oi, new_value);
                    } else {
                        SetSimpleTypeValue(it->field, new_value);
                    }
                }
                ++index;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// ApplyCDS(protein_name, protein_description, frame("best"|1|2|3), location(object), add_redundant(bool), add_mRNA(bool) [,qualifier_name][,qualifier_value])
/// qualifier_name should be specified by its asn path, relative to the CDS Seq-feat
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyCDS, "ApplyCDS")
const vector<SArgMetaData> CMacroFunction_ApplyCDS::sm_Arguments
{ SArgMetaData("protein_name", CMQueryNodeValue::eString, false)
, SArgMetaData("protein_descr", CMQueryNodeValue::eString, false)
, SArgMetaData("codon_start", CMQueryNodeValue::eString, false)
, SArgMetaData("add_mrna", CMQueryNodeValue::eBool, false)
};
void CMacroFunction_ApplyCDS::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    m_Bsh = scope->GetBioseqHandle(*bseq);
    if (!bseq || !scope || bseq->IsAa())
        return;

    m_NewFeat = Ref(new CSeq_feat());
    m_NewFeat->SetData().SetCdregion();

    // add appropriate genetic code
    CRef<CGenetic_code> code = edit::GetGeneticCodeForBioseq(m_Bsh);
    if (code) {
        m_NewFeat->SetData().SetCdregion().SetCode(*code);
    }

    bool find_best_frame = false;
    size_t fr_index = 2;
    CCdregion::EFrame codon_start = CCdregion::eFrame_not_set;
    if (m_Args[fr_index]->GetDataType() == CMQueryNodeValue::eString) {
        if (NStr::EqualNocase(m_Args[fr_index]->GetString(), "best")) {
            find_best_frame = true;
        }
        else {
            NCBI_THROW(CException, eUnknown, "Wrong frame specified (try \"best\")");
        }
    }
    else {
        switch (m_Args[fr_index]->GetInt()) {
        case 1:
            codon_start = CCdregion::eFrame_one;
            break;
        case 2:
            codon_start = CCdregion::eFrame_two;
            break;
        case 3:
            codon_start = CCdregion::eFrame_three;
            break;
        default:
            NCBI_THROW(CException, eUnknown, "Wrong frame specified: " + NStr::Int8ToString(m_Args[fr_index]->GetInt()));
        }
    }

    if (!x_SetLocation(3) || !x_ShouldAddRedundant(4))
        return;

    x_SetAdditionalFields(6);

    bool add_mRNA = m_Args[5]->GetBool();
    const string& protein_name = m_Args[0]->GetString();
    const string& protein_descr = m_Args[1]->GetString();

    CSeq_entry_Handle seh = m_Bsh.GetSeq_entry_Handle();
    CRef<CCmdComposite> cmd(new CCmdComposite("Add CDS"));
    if (add_mRNA) {
        CRef<CSeq_feat> new_mrna(new CSeq_feat());
        new_mrna->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
        new_mrna->SetLocation(*m_Location);
        new_mrna->SetPartial(new_mrna->GetLocation().IsPartialStart(eExtreme_Biological) | new_mrna->GetLocation().IsPartialStop(eExtreme_Biological));
        new_mrna->SetData().SetRna().SetExt().SetName(protein_name);
        cmd->AddCommand(*(CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_mrna))));
    }

    if (m_Location->IsPnt()) {
        m_NewFeat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_not_set);
    }
    else {
        if (find_best_frame) {
            bool ambiguous = false;
            m_NewFeat->SetData().SetCdregion().SetFrame(CSeqTranslator::FindBestFrame(*m_NewFeat, m_Bsh.GetScope(), ambiguous));
        }
        else {
            m_NewFeat->SetData().SetCdregion().SetFrame(codon_start);
        }
        bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_Bsh.GetTopLevelEntry());
        CRef<CSeq_entry> protein_entry = CreateTranslatedProteinSequence(m_NewFeat, m_Bsh, create_general_only);
        CRef<CSeq_feat> prot = AddProteinFeatureToProtein(protein_entry,
                            m_NewFeat->GetLocation().IsPartialStart(eExtreme_Biological),
                            m_NewFeat->GetLocation().IsPartialStop(eExtreme_Biological));

        if (!protein_name.empty()) {
            prot->SetData().SetProt().SetName().push_back(protein_name);
        }
        if (!protein_descr.empty()) {
            prot->SetData().SetProt().SetDesc(protein_descr);
        }

        CCleanup cleanup;
        cleanup.BasicCleanup(*prot);
        cmd->AddCommand(*CRef<CCmdAddSeqEntry>(new CCmdAddSeqEntry(protein_entry, seh)));
    }

    CCleanup cleanup;
    cleanup.BasicCleanup(*m_NewFeat);
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *m_NewFeat)));

    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);

        CNcbiOstrstream log;
        string best_id;
        CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(m_Bsh.GetCompleteBioseq()->GetFirstId())), m_Bsh.GetScope(), best_id);
        log << "Added CDS feature to " << best_id;
        x_LogFunction(log);
    }
}

bool CMacroFunction_ApplyCDS::x_ValidArguments() const
{
    if (m_Args.size() < 6)
        return false;

    bool good = m_Args[0]->GetDataType() == CMQueryNodeValue::eString
                && m_Args[1]->GetDataType() == CMQueryNodeValue::eString
                && (m_Args[2]->GetDataType() == CMQueryNodeValue::eString || m_Args[2]->GetDataType() == CMQueryNodeValue::eInt)
                && m_Args[3]->GetDataType() == CMQueryNodeValue::eRef
                && m_Args[4]->GetDataType() == CMQueryNodeValue::eBool
                && m_Args[5]->GetDataType() == CMQueryNodeValue::eBool;

    if (m_Args.size() > 6) {
        for (size_t index = 6; index < m_Args.size(); ++index) {
            if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
                return false;
            }
        }
    }
    return good;
}

///////////////////////////////////////////////////////////////////////////////
/// ApplyRNA(type, rna_name, location(object), add_redundant(bool) [,qualifier_name][,qualifier_value])
/// qualifier_name should be specified by its asn path, relative to the Seq-feat
/// Type of RNA can be one of: preRNA, mRNA, tRNA, rRNA, ncRNA, tmRNA, miscRNA
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyRNA, "ApplyRNA")
const vector<SArgMetaData> CMacroFunction_ApplyRNA::sm_Arguments
{ SArgMetaData("rna_name", CMQueryNodeValue::eString, false)
, SArgMetaData("miscRNA_comment1", CMQueryNodeValue::eBool, true)
, SArgMetaData("miscRNA_comment2", CMQueryNodeValue::eBool, true)
, SArgMetaData("gene_symbol_static", CMQueryNodeValue::eString, true)
, SArgMetaData("gene_descr_static", CMQueryNodeValue::eString, true)
};
void CMacroFunction_ApplyRNA::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    m_Bsh = scope->GetBioseqHandle(*bseq);
    if (!bseq || !scope || bseq->IsAa())
        return;

    CRNA_ref::EType rna_type = NMacroUtil::GetRNAType(m_Args[0]->GetString());
    if (rna_type == CRNA_ref::eType_unknown) {
        NCBI_THROW(CException, eUnknown, "Wrong type of RNA specified: " + m_Args[0]->GetString());
    }

    CRef<CRNA_ref> rna_ref(new CRNA_ref());
    rna_ref->SetType(rna_type);
    const string& rna_name = m_Args[1]->GetString();
    string remainder;
    rna_ref->SetRnaProductName(rna_name, remainder);

    m_NewFeat = Ref(new CSeq_feat());
    m_NewFeat->SetData().SetRna().Assign(*rna_ref);

    if (!x_SetLocation(2) || !x_ShouldAddRedundant(3))
        return;

    x_SetAdditionalFields(4);
    CCleanup cleanup;
    cleanup.BasicCleanup(*m_NewFeat);
    CSeq_entry_Handle seh = m_Bsh.GetSeq_entry_Handle();
    CRef<CCmdComposite> cmd(new CCmdComposite("Add RNA"));
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *m_NewFeat)));

    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);

        CNcbiOstrstream log;
        string best_id;
        CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(m_Bsh.GetCompleteBioseq()->GetFirstId())), m_Bsh.GetScope(), best_id);
        log << "Added " << m_Args[0]->GetString() << " feature to " << best_id;
        x_LogFunction(log);
    }
}

bool CMacroFunction_ApplyRNA::x_ValidArguments() const
{
    if (m_Args.size() < 4)
        return false;

    bool good = m_Args[0]->GetDataType() == CMQueryNodeValue::eString
                && m_Args[1]->GetDataType() == CMQueryNodeValue::eString
                && m_Args[2]->GetDataType() == CMQueryNodeValue::eRef
                && m_Args[3]->GetDataType() == CMQueryNodeValue::eBool;

    if (m_Args.size() > 4) {
        for (size_t index = 4; index < m_Args.size(); ++index) {
            if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
                return false;
            }
        }
    }
    return good;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ApplyGene
/// ApplyGene(location [,qualifier_name][,qualifier_value])
/// Different from ApplyFeature() function as it always applies a gene to the sequence
/// Used mostly together with ApplyCDS/ApplyRNA/ApplyFeature functions to create an additional gene
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyGene, "ApplyGene")

void CMacroFunction_ApplyGene::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    m_Bsh = scope->GetBioseqHandle(*bseq);
    // don't create gene if the selected sequence is protein
    if (!bseq || !scope || bseq->IsAa())
        return;

    m_NewFeat = Ref(new CSeq_feat);
    if (!x_SetLocation(0))
        return;

    x_SetAdditionalFields(1);

    CSeq_entry_Handle seh = m_Bsh.GetSeq_entry_Handle();
    CRef<CCmdComposite> cmd(new CCmdComposite("Add Gene"));
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *m_NewFeat)));

    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        CNcbiOstrstream log;
        string best_id;
        CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(m_Bsh.GetCompleteBioseq()->GetFirstId())), m_Bsh.GetScope(), best_id);
        log << "Added Gene feature to " << best_id;
        x_LogFunction(log);
    }
}

bool CMacroFunction_ApplyGene::x_ValidArguments() const
{
    if (m_Args.empty())
        return false;

    bool good = m_Args[0]->GetDataType() == CMQueryNodeValue::eRef;

    if (m_Args.size() > 1) {
        for (size_t index = 1; index < m_Args.size(); ++index) {
            if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
                return false;
            }
        }
    }
    return good;
}

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_ApplyOtherFeature
/// ApplyFeature(type, name, location, add_redundant(bool) [,qualifier_name][,qualifier_value])
/// Example:
/// ApplyFeature("data.prot.name", "new protein", location, true, "data.prot.processed", "preprotein")
/// ApplyFeature("data.psec-str", "helix", location, false)
/// ApplyFeature("data.imp.key", "misc_feature", location, true);
/// ApplyFeature("data.gene.locus", "", location, true, "data.gene.desc", "gene description")
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyOtherFeature, "ApplyFeature")
const vector<SArgMetaData> CMacroFunction_ApplyOtherFeature::sm_Arguments
{ SArgMetaData("comment", CMQueryNodeValue::eString, false)
, SArgMetaData("gene_symbol", CMQueryNodeValue::eString, true)
, SArgMetaData("gene_descr", CMQueryNodeValue::eString, true)
, SArgMetaData("add_redundant", CMQueryNodeValue::eBool, false)
, SArgMetaData("feature_type", CMQueryNodeValue::eString, false)
, SArgMetaData("qualifier1", CMQueryNodeValue::eString, false)
, SArgMetaData("qual_value1", CMQueryNodeValue::eString, false)
, SArgMetaData("qualifier2", CMQueryNodeValue::eString, false)
, SArgMetaData("qual_value2", CMQueryNodeValue::eString, false)
, SArgMetaData("qualifier3", CMQueryNodeValue::eString, false)
, SArgMetaData("qual_value3", CMQueryNodeValue::eString, false)
, SArgMetaData("qualifier4", CMQueryNodeValue::eString, false)
, SArgMetaData("qual_value4", CMQueryNodeValue::eString, false)
};
void CMacroFunction_ApplyOtherFeature::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    m_Bsh = scope->GetBioseqHandle(*bseq);
    if (!bseq || !scope)
        return;

    m_NewFeat = Ref(new CSeq_feat);
    CObjectInfo oi(m_NewFeat, m_NewFeat->GetTypeInfo());
    CMQueryNodeValue::TObs res_oi;
    if (!SetFieldsByName(&res_oi, oi, m_Args[0]->GetString())) {
        NCBI_THROW(CException, eUnknown, "Feature " + m_Args[0]->GetString() + " could not be set");
    }

    CMQueryNodeValue& new_value = *m_Args[1];
    const string& new_str = m_Args[1]->GetString();
    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {

        CObjectInfo set_oi = it->field; 
        switch (set_oi.GetTypeFamily()) {
        case eTypeFamilyPrimitive:
            if (!SetSimpleTypeValue(it->field, new_value)) {
                NCBI_THROW(CException, eUnknown, "Value " + new_str + " could not be set");
            }
            break;
        case eTypeFamilyContainer: {
            CMQueryNodeValue::TObs objs;
            NMacroUtil::GetPrimitiveObjectInfos(objs, *it);

            if (objs.empty()) { // add new element when the container is empty
                CObjectInfo new_oi(set_oi.AddNewElement());
                if (!SetQualStringValue(new_oi, new_str)) {
                    NCBI_THROW(CException, eUnknown, "Value " + new_str + " could not be set");
                }
            }
            break;
        }
        default:
            NCBI_THROW(CException, eUnknown, "Value " + new_str + " could not be set");
        }
    }

    if (!x_SetLocation(2) || !x_ShouldAddRedundant(3))
        return;

    x_SetAdditionalFields(4);

    CSeq_entry_Handle seh = m_Bsh.GetSeq_entry_Handle();
    CRef<CCmdComposite> cmd(new CCmdComposite("Add Feature"));
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *m_NewFeat)));

    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);

        CNcbiOstrstream log;
        string best_id;
        CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(m_Bsh.GetCompleteBioseq()->GetFirstId())), m_Bsh.GetScope(), best_id);
        log << "Added feature to " << best_id;
        x_LogFunction(log);
    }
}

bool CMacroFunction_ApplyOtherFeature::x_ValidArguments() const
{
    if (m_Args.size() < 4) 
        return false;

    bool good = m_Args[0]->GetDataType() == CMQueryNodeValue::eString
                && m_Args[1]->GetDataType() == CMQueryNodeValue::eString
                && m_Args[2]->GetDataType() == CMQueryNodeValue::eRef
                && m_Args[3]->GetDataType() == CMQueryNodeValue::eBool;

    if (m_Args.size() > 4) {
        for (size_t index = 4; index < m_Args.size(); ++index) {
            if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
                return false;
            }
        }
    }
    return good;
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
