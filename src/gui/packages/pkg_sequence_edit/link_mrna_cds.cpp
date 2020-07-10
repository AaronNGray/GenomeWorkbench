/*  $Id: link_mrna_cds.cpp 42162 2019-01-07 20:09:12Z filippov $
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
 * 
 *   Place reciprocal cross-references between mRNA and CDS. The features are selected 
 *   according to some criteria.
 */

#include <ncbi_pch.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/seq_desktop/desktop_typed_items.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>
#include <gui/packages/pkg_sequence_edit/link_mrna_cds.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);	

void CmRNACDSLinker::LinkSelectedFeatures(TConstScopedObjects& objects, ICommandProccessor* cmdProcessor)
{
    if (objects.empty()) {
        wxMessageBox(wxT("No features are selected"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return;
    }

    CConstRef<CSeq_feat> mrna, cds;
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CObject* ptr = it->object.GetPointer();
        const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(ptr);
        if (seqfeat && seqfeat->IsSetData()) {
            if (seqfeat->GetData().IsCdregion()) {
                cds.Reset(seqfeat);
            } else if (seqfeat->GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA) {
                mrna.Reset(seqfeat);
            }
        }
    }

    if (!x_OkForLinking(mrna, cds)) {
        return;
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Link Selected CDS and mRNA Pair"));
    CScope& scope = m_TopSeqEntry.GetScope();

    bool cds_hasid = cds->IsSetId() && cds->GetId().IsLocal();
    bool mrna_hasid = mrna->IsSetId() && mrna->GetId().IsLocal();

    bool modified = false;
    // assign feature ids in case there is none:
    if (!cds_hasid || !mrna_hasid) {

        CRef<CSeq_feat> new_mrna, new_cds;
        CObject_id::TId top_id = CFixFeatureId::s_FindHighestFeatureId(m_TopSeqEntry);
        for (CFeat_CI feat_it(m_TopSeqEntry); feat_it; ++feat_it) {
            CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
            if (fh.IsSetId() && fh.GetId().IsLocal()) {
                continue;
            }
            
            CConstRef<CSeq_feat> orig_feat = feat_it->GetOriginalSeq_feat();
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->Assign(*orig_feat);
            top_id++;
            new_feat->SetId().SetLocal().SetId(top_id);
            
            if (orig_feat == mrna ) {
                new_mrna.Reset(new_feat.GetPointer());
            } else if (orig_feat == cds) {
                new_cds.Reset(new_feat.GetPointer());
            } else {
                CIRef<IEditCommand> chg_feat(new CCmdChangeSeq_feat(fh, *new_feat));
                cmd->AddCommand(*chg_feat);
                modified = true;
            }
        }

        if (new_mrna.NotEmpty() && new_cds.NotEmpty()) {
            s_CreateXRefLink(*new_mrna, *new_cds);
            s_CreateXRefLink(*new_cds, *new_mrna);

            CIRef<IEditCommand> chg_mrna(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*mrna), *new_mrna));
            cmd->AddCommand(*chg_mrna);
            CIRef<IEditCommand> chg_cds(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*cds), *new_cds));
            cmd->AddCommand(*chg_cds);
            modified = true;
        } 
    } else if (cds_hasid && mrna_hasid) { //both features have feat_id
        modified = SetReciprocalXrefs(*mrna, *cds, cmd);
    }

    if (modified) {
        cmdProcessor->Execute(cmd);
    }
}

bool CmRNACDSLinker::SetReciprocalXrefs(const CSeq_feat& mrna, const CSeq_feat& cds, CRef<CCmdComposite> cmd)
{
    bool modified = false;
    CScope& scope = m_TopSeqEntry.GetScope();
    if (!s_IsDirectXrefBetween(mrna, cds)) {
        CRef<CSeq_feat> new_mrna(new CSeq_feat);
        new_mrna->Assign(mrna);
        s_CreateXRefLink(*new_mrna, cds);
        CIRef<IEditCommand> chg_mrna(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(mrna), *new_mrna));
        cmd->AddCommand(*chg_mrna);
        modified = true;
    }

    if (!s_IsDirectXrefBetween(cds, mrna)) {
        CRef<CSeq_feat> new_cds(new CSeq_feat);
        new_cds->Assign(cds);
        s_CreateXRefLink(*new_cds, mrna);
        CIRef<IEditCommand> chg_cds(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(cds), *new_cds));
        cmd->AddCommand(*chg_cds);
        modified = true;
    }
    return modified;
}

bool CmRNACDSLinker::ReplaceExistingXrefs(const CSeq_feat& mrna, const CSeq_feat& cds, CRef<CCmdComposite> cmd)
{
    if (!mrna.IsSetId() || !(cds.IsSetId() && cds.GetId().IsLocal())) {
        return false;
    }

    CScope& scope = m_TopSeqEntry.GetScope();

    CRef<CSeq_feat> new_mrna(new CSeq_feat);
    new_mrna->Assign(mrna);

    bool replaced = x_ReplaceExistingFeatIdXrefs(*new_mrna, cds);
    if (!replaced) {
        s_CreateXRefLink(*new_mrna, cds);
    }
    CIRef<IEditCommand> chg_mrna(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(mrna), *new_mrna));
    cmd->AddCommand(*chg_mrna);

    CRef<CSeq_feat> new_cds(new CSeq_feat);
    new_cds->Assign(cds);
    replaced = false;
    replaced = x_ReplaceExistingFeatIdXrefs(*new_cds, mrna);
    if (!replaced) {
        s_CreateXRefLink(*new_cds, mrna);
    }
    CIRef<IEditCommand> chg_cds(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(cds), *new_cds));
    cmd->AddCommand(*chg_cds);

    return true;
}

bool CmRNACDSLinker::x_ReplaceExistingFeatIdXrefs(CSeq_feat& from_feat, const CSeq_feat& to_feat)
{
    if (!from_feat.IsSetXref()) {
        return false;
    }

    EDIT_EACH_XREF_ON_SEQFEAT(xref_it, from_feat) {
        CRef<CSeqFeatXref> xref = *xref_it;
        if (xref->IsSetId() && xref->GetId().IsLocal()) {
            xref->SetId().SetLocal().Assign(to_feat.GetId().GetLocal());
            return true;
        }
    }
    return false;
}


bool CmRNACDSLinker::AssignFeatureIds(CRef<CMacroCmdComposite> cmd)
{
    bool assigned = false;
    CObject_id::TId top_id = CFixFeatureId::s_FindHighestFeatureId(m_TopSeqEntry);
    for (CFeat_CI feat_it(m_TopSeqEntry); feat_it; ++feat_it) {
        CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
        if (fh.IsSetId() && fh.GetId().IsLocal()) {
            continue;
        }

        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(*feat_it->GetOriginalSeq_feat());
        top_id++;
        new_feat->SetId().SetLocal().SetId(top_id);
        CIRef<IEditCommand> chg_feat(new CCmdChangeSeq_feat(fh, *new_feat));
        chg_feat->Execute();
        cmd->AddCommand(*chg_feat);
        assigned = true;
    }
    return assigned;
}

void CmRNACDSLinker::LinkByOverlap(ICommandProccessor* cmdProcessor)
{
    CRef<CMacroCmdComposite> link_cmd(new CMacroCmdComposite("Link CDS-mRNA pair by overlap"));
    bool assigned = AssignFeatureIds(link_cmd);

    CRef<CCmdComposite> cmd(new CCmdComposite("Actual linking of cds and mRNA pairs by overlap"));
    CScope& scope = m_TopSeqEntry.GetScope();

    set<CConstRef<CSeq_feat> > linked_mRNAs;
    for (CFeat_CI cds_it(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::e_Cdregion)); cds_it; ++cds_it) {
        
        // find single, unused mRNA
        CConstRef<CSeq_feat> mrna = sequence::GetBestOverlappingFeat
            (cds_it->GetLocation(),
            CSeqFeatData::eSubtype_mRNA,
            sequence::eOverlap_CheckIntRev,
            scope);

        if (!mrna) {
            continue;
        }
        // check if the mRNA was already linked to a different CDS
        if (linked_mRNAs.find(mrna) != linked_mRNAs.end()) {
            continue;
        }

        CConstRef<CSeq_feat> cds = cds_it->GetOriginalSeq_feat();
        if (SetReciprocalXrefs(*mrna, *cds, cmd)) {
            linked_mRNAs.insert(mrna);
        }
    }

    if (!linked_mRNAs.empty()) {
        cmd->Execute();
        link_cmd->AddCommand(*cmd);
    }
    if (assigned || !linked_mRNAs.empty()) {
        cmdProcessor->Execute(link_cmd);
    }
}



void CmRNACDSLinker::LinkByProduct(ICommandProccessor* cmdProcessor)
{
    CRef<CMacroCmdComposite> link_cmd(new CMacroCmdComposite("Link CDS-mRNA pair by product"));
    bool assigned = AssignFeatureIds(link_cmd);

    CRef<CCmdComposite> cmd(new CCmdComposite("Actual linking of cds and mRNA pairs by product"));
    CScope& scope = m_TopSeqEntry.GetScope();

    map<CSeq_id_Handle, CConstRef<CSeq_feat>> mrna_products;
    map<CSeq_id_Handle, CConstRef<CSeq_feat>> cds_products;

    for (CFeat_CI mrna_it(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::eSubtype_mRNA)); mrna_it; ++mrna_it) {
        if (mrna_it->IsSetProduct()) {
            CSeq_id_Handle idh = mrna_it->GetProductId();
            mrna_products.emplace(idh, mrna_it->GetOriginalSeq_feat());
        }
    }

    for (CFeat_CI cds_it(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::eSubtype_cdregion)); cds_it; ++cds_it) {
        if (cds_it->IsSetProduct()) {
            CSeq_id_Handle idh = cds_it->GetProductId();
            cds_products.emplace(idh, cds_it->GetOriginalSeq_feat());
        }
    }

    if (mrna_products.empty() || cds_products.empty()) {
        return;
    }

    set<CConstRef<CSeq_feat>> linked_feats;
    for (auto& mrna_it : mrna_products) {
        if (linked_feats.find(mrna_it.second) != linked_feats.end()) {
            continue;
        }

        CBioseq_Handle cdna = scope.GetBioseqHandle(mrna_it.first);
        if (!cdna) continue;

        CBioseq_set_Handle cdna_parenth = cdna.GetParentBioseq_set();
        if (cdna_parenth.IsSetClass() && cdna_parenth.GetClass() == CBioseq_set::eClass_nuc_prot) {
            for (CBioseq_CI prot_it(cdna_parenth, CSeq_inst::eMol_aa); prot_it; ++prot_it) {
                vector<CSeq_id_Handle> prot_ids = prot_it->GetId();
                for (auto&& id_it : prot_ids) {
                    auto found_cds_it = cds_products.find(id_it);
                    if (found_cds_it != cds_products.end()) {
                        if (linked_feats.find(found_cds_it->second) != linked_feats.end()) {
                            continue;
                        }
                        if (ReplaceExistingXrefs(*mrna_it.second, *found_cds_it->second, cmd)) {
                            linked_feats.insert(mrna_it.second);
                            linked_feats.insert(found_cds_it->second);
                        }
                    }
                }
            }
        }
    }

    if (!linked_feats.empty()) {
        cmd->Execute();
        link_cmd->AddCommand(*cmd);
    }
    if (assigned || !linked_feats.empty()) {
        cmdProcessor->Execute(link_cmd);
    }
}

static string s_GetFeatureLabel(const CSeq_feat_Handle& fh)
{
    const CSeq_feat& feat = *fh.GetOriginalSeq_feat();

    string label;
    // cds label will be the same as protein name
    if (feat.GetData().IsCdregion() && feat.IsSetProduct()) {
        label = CDesktopFeatItem::s_GetProteinName(fh);
    }
    // mrna label will be equal to the mRNA's product name
    else if (feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA) {
        label = feat.GetData().GetRna().GetRnaProductName();
    }
    return label;
}

void CmRNACDSLinker::LinkByLabel(ICommandProccessor* cmdProcessor)
{
    CRef<CMacroCmdComposite> link_cmd(new CMacroCmdComposite("Link CDS-mRNA pair by label"));
    bool assigned = AssignFeatureIds(link_cmd);

    CRef<CCmdComposite> cmd(new CCmdComposite("Actual linking of cds and mRNA pairs by label"));
    CScope& scope = m_TopSeqEntry.GetScope();

    // loop through CDS features, finding mRNA partner by label
    set<CConstRef<CSeq_feat> > linked_mRNAs;
    for (CFeat_CI cds_it(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::e_Cdregion)); cds_it; ++cds_it) {
        CConstRef<CSeq_feat> cds_feat = cds_it->GetOriginalSeq_feat();
        string cds_label = s_GetFeatureLabel(*cds_it);

        for (CFeat_CI mrna_it(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::eSubtype_mRNA)); mrna_it; ++mrna_it) {
            CConstRef<CSeq_feat> mrna = mrna_it->GetOriginalSeq_feat();
            string mrna_label = s_GetFeatureLabel(*mrna_it);
            
            if (NStr::EqualNocase(cds_label, mrna_label)) {
                // check if the mRNA was already linked to a different CDS
                if (linked_mRNAs.find(mrna) != linked_mRNAs.end()) {
                    continue;
                }

                if (SetReciprocalXrefs(*mrna, *cds_feat, cmd)) {
                    linked_mRNAs.insert(mrna);
                }
            }
        }
    }

    if (!linked_mRNAs.empty()) {
        cmd->Execute();
        link_cmd->AddCommand(*cmd);
    }
    if (assigned || !linked_mRNAs.empty()) {
        cmdProcessor->Execute(link_cmd);
    }
}

static bool s_DoesFeatureHasXRef(const CSeq_feat& feat)
{
    FOR_EACH_SEQFEATXREF_ON_SEQFEAT(xref_it, feat) {
        if ((*xref_it)->IsSetId() && (*xref_it)->GetId().IsLocal()) {
            return true;
        }
    }
    return false;
}

void CmRNACDSLinker::LinkByLabelAndLocation(ICommandProccessor* cmdProcessor)
{
    // group cds and mRNAs based on label, and for each CDS/mRNA in a given group find the match with the best location
    CRef<CMacroCmdComposite> link_cmd(new CMacroCmdComposite("Link CDS-mRNA pair by label and location"));
    bool assigned = AssignFeatureIds(link_cmd);

    CRef<CCmdComposite> cmd(new CCmdComposite("Actual linking of cds and mRNA pairs by label and location"));
    CScope& scope = m_TopSeqEntry.GetScope();

    SAnnotSelector sel;
    sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_mRNA);
    sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_cdregion);
    set<CConstRef<CSeq_feat>> linked_feats;
    for (CFeat_CI feat_it(m_TopSeqEntry, sel); feat_it; ++feat_it) {
        CConstRef<CSeq_feat> feat = feat_it->GetOriginalSeq_feat();

        // skip if it already has an Xrefpp
        if (s_DoesFeatureHasXRef(*feat) || linked_feats.find(feat) != linked_feats.end()) {
            continue;
        }

        string feat_label = s_GetFeatureLabel(*feat_it);

        CFeat_CI pair_it = feat_it;
        ++pair_it;
        CConstRef<CSeq_feat> best_fit;
        TSeqPos best_diff = 0;
        for (; pair_it; ++pair_it) {
            CConstRef<CSeq_feat> feat_pair = pair_it->GetOriginalSeq_feat();
            if (feat_pair->GetData().GetSubtype() == feat->GetData().GetSubtype()) {
                continue;
            }

            // skip if it already has an Xref
            if (s_DoesFeatureHasXRef(*feat_pair) || linked_feats.find(feat_pair) != linked_feats.end()) {
                continue;
            }

            string pair_label = s_GetFeatureLabel(*pair_it);

            if (NStr::EqualNocase(feat_label, pair_label)) {
                CConstRef<CSeq_feat> mRNA, cds;
                if (feat->GetData().IsCdregion()) {
                    cds = feat;
                    mRNA = feat_pair;
                }
                else {
                    cds = feat_pair;
                    mRNA = feat;
                }

                sequence::ECompare located = sequence::Compare(mRNA->GetLocation(), cds->GetLocation(), &scope, sequence::fCompareOverlapping);
                if (located == sequence::eContains || located == sequence::eSame) {
                    if (best_fit.IsNull()) {
                        best_fit = feat_pair;
                        TSeqPos best_fit_length = best_fit->GetLocation().GetTotalRange().GetLength();
                        TSeqPos feat_length = feat->GetLocation().GetTotalRange().GetLength();
                        best_diff = (best_fit_length > feat_length) ? (best_fit_length - feat_length) : (feat_length - best_fit_length);
                    }
                    else {
                        TSeqPos candidate_length = feat_pair->GetLocation().GetTotalRange().GetLength();
                        TSeqPos feat_length = feat->GetLocation().GetTotalRange().GetLength();
                        TSeqPos current_diff = (candidate_length > feat_length) ? (candidate_length - feat_length) : (feat_length - candidate_length);
                        if (current_diff < best_diff) {
                            best_fit = feat_pair;
                            best_diff = current_diff;
                        }
                    }
                }
            }
        }

        if (best_fit) {
            SetReciprocalXrefs(*feat, *best_fit, cmd);
            linked_feats.insert(feat);
            linked_feats.insert(best_fit);
        }
    }

    if (!linked_feats.empty()) {
        cmd->Execute();
        link_cmd->AddCommand(*cmd);
    }
    if (assigned || !linked_feats.empty()) {
        cmdProcessor->Execute(link_cmd);
    }
}
   
void CmRNACDSLinker::CompleteHalfFormedXrefPairs(ICommandProccessor* cmdProcessor)
{
    // assumed that feature IDs are already assigned
    SAnnotSelector sel;
    sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_mRNA);
    sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_cdregion);
    CTSE_Handle tse = m_TopSeqEntry.GetTSE_Handle();

    CRef<CCmdComposite> link_cmd(new CCmdComposite("Complete CDS and mRNA XRef pair"));
    bool linked = false;
    for (CFeat_CI feat_it(m_TopSeqEntry, sel); feat_it; ++feat_it) {
        CConstRef<CSeq_feat> feat = feat_it->GetOriginalSeq_feat();
        if (!feat->IsSetXref() || !feat->IsSetId()) {
            continue;
        }

        ITERATE(CSeq_feat::TXref, xref_it, feat->GetXref()) {
            if ((*xref_it)->IsSetId() && (*xref_it)->GetId().IsLocal() && (*xref_it)->GetId().GetLocal().IsId()) {
                CSeq_feat_Handle pair_fh;  // potential pair feature

                if (feat->GetData().IsCdregion()) {
                    pair_fh = tse.GetFeatureWithId(CSeqFeatData::eSubtype_mRNA, (*xref_it)->GetId().GetLocal().GetId());
                }
                else if (feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA) {
                    pair_fh = tse.GetFeatureWithId(CSeqFeatData::eSubtype_cdregion, (*xref_it)->GetId().GetLocal().GetId());
                }

                if (!pair_fh) {
                    continue;
                }

                // check if the other feature has already have an Xref to this feature
                bool has_xref = false;
                if (pair_fh.IsSetXref()) {
                    ITERATE(CSeq_feat::TXref, it, pair_fh.GetXref()) {
                        if ((*it)->IsSetId() && (*it)->GetId().IsLocal() && (*it)->GetId().GetLocal().IsId()) {
                            const CFeat_id& pair_feat_id = (*it)->GetId();
                            if (pair_feat_id.Equals(feat->GetId())) {
                                has_xref = true;
                                break;
                            }
                        }
                    }
                }

                if (!has_xref) {
                    linked = SetReciprocalXrefs(*feat, *pair_fh.GetOriginalSeq_feat(), link_cmd);
                }
            }
        }
    }

    if (linked) {
        cmdProcessor->Execute(link_cmd);
    }
}

bool CmRNACDSLinker::x_OkForLinking(CConstRef<CSeq_feat> mrna, CConstRef<CSeq_feat> cds)
{
    string msg;
    if (!cds && !mrna) {
        msg.assign("No features are selected");
    } else if (cds && !mrna) {
        msg.assign("No mRNA is selected");
    } else if (!cds && mrna) {
        msg.assign("No CDS is selected");
    }

    if (!NStr::IsBlank(msg)) {
        wxMessageBox(ToWxString(msg), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return false;
    }

    return true;
}

void CmRNACDSLinker::s_CreateXRefLink(CSeq_feat& from_feat, const CSeq_feat& to_feat)
{
    CRef<CSeqFeatXref> xref(new CSeqFeatXref);
    xref->SetId().Assign(to_feat.GetId());
    from_feat.SetXref().push_back(xref);
}

bool CmRNACDSLinker::s_IsDirectXrefBetween(const CSeq_feat& from_feat, const CSeq_feat& to_feat)
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

vector<CConstRef<CObject> > CmRNACDSLinker::GetReferencedmRNA_CDS(TConstScopedObjects& objects)
{
    vector<CConstRef<CObject> > objs;
    if (objects.empty()) {
        wxMessageBox(ToWxString("No feature selected"), ToWxString("Message"), 
            wxOK | wxICON_INFORMATION);
        return objs;
    }

    size_t count_feat = 0;
    ITERATE (TConstScopedObjects, it, objects) {
        const CObject* sel_obj = it->object.GetPointer();
        const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(sel_obj);
        if (seqfeat) {
            count_feat++;
        }
    }

    if (count_feat > 1) {
        wxMessageBox(ToWxString("Please select only one feature"), ToWxString("Message"), 
            wxOK | wxICON_INFORMATION);
        return objs;
    }

    const CObject* sel_obj = objects[0].object.GetPointer();
    const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(sel_obj);
    if (!seqfeat) {
        wxMessageBox(ToWxString("No feature selected"), ToWxString("Message"), 
            wxOK | wxICON_INFORMATION);
        return objs;
    }
    
    bool is_cds = seqfeat->IsSetData() && seqfeat->GetData().IsCdregion();
    bool is_mrna = seqfeat->IsSetData() && (seqfeat->GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA);
    if (!is_cds && !is_mrna) {
        wxMessageBox(ToWxString("A CDS or an mRNA should be selected"), ToWxString("Message"), 
            wxOK | wxICON_INFORMATION);
        return objs;
    }

    CConstRef<CSeq_feat> ref_feat;
    if (seqfeat->IsSetXref()) {
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(seqfeat->GetLocation());
        CTSE_Handle tse = bsh.GetTSE_Handle();
        FOR_EACH_SEQFEATXREF_ON_SEQFEAT (it, *seqfeat) {
            if ((*it)->IsSetId() && (*it)->GetId().IsLocal() && (*it)->GetId().GetLocal().IsId()) {
                CSeqFeatData::ESubtype search_type = (is_cds) ? CSeqFeatData::eSubtype_mRNA : CSeqFeatData::eSubtype_cdregion;
                CSeq_feat_Handle fh = tse.GetFeatureWithId(search_type, (*it)->GetId().GetLocal().GetId());
                if (fh) {
                    ref_feat = fh.GetSeq_feat();
                }
            }
        }
    }

    if (!ref_feat) {
        wxMessageBox(ToWxString("Unable to find referenced feature"), ToWxString("Message"), 
            wxOK | wxICON_INFORMATION);
        return objs;
    }
    
    objs.push_back(CConstRef<CObject>(ref_feat.GetPointer()));
    objs.push_back(objects[0].object);
    return objs;
}

END_NCBI_SCOPE
