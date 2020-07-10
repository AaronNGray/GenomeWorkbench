/*  $Id: util_cmds.cpp 44877 2020-04-02 20:22:08Z asztalos $
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

#include <objects/taxon3/taxon3.hpp>
#include <misc/discrepancy/discrepancy.hpp>

#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <objtools/edit/cds_fix.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <objtools/validator/validatorp.hpp>
#include <objtools/validator/go_term_validation_and_cleanup.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/cmd_factory.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_del_bioseq_set.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/util_cmds.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// are there other coding regions that point to this protein

static bool s_AreThereOtherCDSForThisProduct(const CSeq_feat_Handle& fh)
{
    CConstRef<CSeq_feat> orig_feat = fh.GetOriginalSeq_feat();
    const CSeq_loc& product_loc = fh.GetProduct();

    CScope::TTSE_Handles tses;
    fh.GetScope().GetAllTSEs(tses, CScope::eAllTSEs);
    ITERATE(CScope::TTSE_Handles, handle, tses) {
        for (CBioseq_CI b_iter(*handle, CSeq_inst::eMol_na); b_iter; ++b_iter) {
            for (CFeat_CI feat_it(*b_iter, SAnnotSelector(CSeqFeatData::eSubtype_cdregion)); feat_it; ++feat_it) {
                if (feat_it->IsSetProduct()
                    && feat_it->GetProduct().Equals(product_loc)
                    && feat_it->GetOriginalSeq_feat().GetPointer() != orig_feat.GetPointer()) {
                    return true;
                }
            }
        }
    }
    return false;
}

static bool s_IsFarLocation(CSeq_feat_Handle fh)
{
    const CSeq_loc& loc = fh.GetLocation();
    for (CSeq_loc_CI loc_iter(fh.GetLocation(), CSeq_loc_CI::eEmpty_Skip);  loc_iter;  ++loc_iter)
    {
    CBioseq_Handle bsh;  
    try 
    {
        bsh = fh.GetScope().GetBioseqHandle(loc_iter.GetSeq_id());
    } catch (CObjMgrException&) {}
    if (!bsh)
        return true;
    CSeq_entry_Handle parent = bsh.GetSeq_entry_Handle();
    if (!parent)
        return true;
    if (validator::IsFarLocation(fh.GetProduct(), parent))
        return true;
    }
    return false;
}

CRef<CCmdComposite> GetDeleteFeatureCommand(const CSeq_feat_Handle& fh, bool remove_proteins)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Delete Feature"));

    const CSeqFeatData& data = fh.GetData();
    if (data.IsPub()) {
        CBioseq_Handle this_seq = fh.GetScope().GetBioseqHandle(fh.GetLocationId());
        CSeq_entry_Handle top_parent = this_seq.GetSeq_entry_Handle();
        CBioseq_set_Handle parent_set = top_parent.GetParentBioseq_set();
        while (parent_set) {
            top_parent = parent_set.GetParentEntry();
            parent_set = top_parent.GetParentBioseq_set();
        }
        CIRef<IEditCommand> cit_cmd = CCmdFactory::RemoveCitations(top_parent, data.GetPub().GetPub());
        if (cit_cmd) {
            cmd->AddCommand(*cit_cmd);
        }
    }

    CRef<CCmdDelSeq_feat> del_feat(new CCmdDelSeq_feat(fh));
    cmd->AddCommand(*del_feat);

    // do not delete the protein if
    // - the protein is not local
    // - there is at least another coding region pointing to this protein
    if (fh.IsSetProduct() && remove_proteins && !s_AreThereOtherCDSForThisProduct(fh)) {
        if (!s_IsFarLocation(fh) && fh.GetProductId()) {
            CBioseq_Handle product = fh.GetScope().GetBioseqHandle(*(fh.GetProductId().GetSeqId()));
            if (product && product.IsProtein()) {   
                CRef<CCmdDelBioseqInst> del_product(new CCmdDelBioseqInst(product));
                cmd->AddCommand(*del_product);
            }
        }
    }

    return cmd;
}

void GetProductToCDSMap(CScope &scope, map<CBioseq_Handle, set<CSeq_feat_Handle> > &product_to_cds)
{
    product_to_cds.clear();
    CScope::TTSE_Handles tses;
    scope.GetAllTSEs(tses, CScope::eAllTSEs);
    for (auto tse : tses)
    {
    for (CFeat_CI feat_it(tse, SAnnotSelector(CSeqFeatData::eSubtype_cdregion)); feat_it; ++feat_it)
    {
        if (feat_it->IsSetProduct())
        {
        CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
        CBioseq_Handle product = scope.GetBioseqHandle(*(fh.GetProductId().GetSeqId()));
        product_to_cds[product].insert(fh);
        }
    }
    }
}

CRef<CCmdComposite> GetDeleteFeatureCommand(const CSeq_feat_Handle& fh, bool remove_proteins, map<CBioseq_Handle, set<CSeq_feat_Handle> > &product_to_cds)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Delete Feature"));

    const CSeqFeatData& data = fh.GetData();
    if (data.IsPub()) {
        CBioseq_Handle this_seq = fh.GetScope().GetBioseqHandle(fh.GetLocationId());
        CSeq_entry_Handle top_parent = this_seq.GetSeq_entry_Handle();
        CBioseq_set_Handle parent_set = top_parent.GetParentBioseq_set();
        while (parent_set) {
            top_parent = parent_set.GetParentEntry();
            parent_set = top_parent.GetParentBioseq_set();
        }
        CIRef<IEditCommand> cit_cmd = CCmdFactory::RemoveCitations(top_parent, data.GetPub().GetPub());
        if (cit_cmd) {
            cmd->AddCommand(*cit_cmd);
        }
    }

    CRef<CCmdDelSeq_feat> del_feat(new CCmdDelSeq_feat(fh));
    cmd->AddCommand(*del_feat);

    // do not delete the protein if
    // - the protein is not local
    // - there is at least another coding region pointing to this protein
    if (fh.IsSetProduct() && remove_proteins) {
        if (!s_IsFarLocation(fh) && fh.GetProductId()) {
            CBioseq_Handle product = fh.GetScope().GetBioseqHandle(*(fh.GetProductId().GetSeqId()));
        product_to_cds[product].erase(fh);
            if (product && product.IsProtein() && product_to_cds[product].empty()) {   
                CRef<CCmdDelBioseqInst> del_product(new CCmdDelBioseqInst(product));
                cmd->AddCommand(*del_product);
            }
        }
    }
    
    return cmd;
}


CRef<CCmdComposite> GetDeleteSequenceCommand(CBioseq_Handle bsh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Delete Sequence"));

    CBioseq_set_Handle nuc_parent = bsh.GetParentBioseq_set();
    if (nuc_parent && nuc_parent.IsSetClass() && nuc_parent.GetClass() == CBioseq_set::eClass_nuc_prot                
        && bsh.IsNa()) {
        if (nuc_parent.GetTopLevelEntry() == nuc_parent.GetParentEntry()) {
            NCBI_THROW(CException, eUnknown, "Cannot delete last sequence");
        } else {
            CRef<CCmdDelBioseqSet> delSet(new CCmdDelBioseqSet(nuc_parent));
            cmd->AddCommand(*delSet);
        }
    } else if (bsh.GetTopLevelEntry() == bsh.GetSeq_entry_Handle()) {
        NCBI_THROW(CException, eUnknown, "Cannot delete last sequence");
    } else {
        CRef<CCmdDelBioseqInst> scmd (new CCmdDelBioseqInst(bsh));
        cmd->AddCommand(*scmd);
    }
    return cmd;
}

static bool s_IsNucLocation(CSeq_feat_Handle fh)
{
    const CSeq_loc& loc = fh.GetLocation();
    for (CSeq_loc_CI loc_iter(fh.GetLocation(), CSeq_loc_CI::eEmpty_Skip);  loc_iter;  ++loc_iter)
    {
    CBioseq_Handle bsh;  
    try 
    {
        bsh = fh.GetScope().GetBioseqHandle(loc_iter.GetSeq_id());
    } catch (CObjMgrException&) {}
    if (!bsh || !bsh.IsNa())
        return false;
    }
    return true;
}

CRef<CCmdComposite> GetDeleteAllFeaturesCommand(CSeq_entry_Handle seh, size_t& count, bool remove_proteins)
{
    //CLogPerformance perfLog("GetDeleteAllFeaturesCommand");
    map<CBioseq_Handle, set<CSeq_feat_Handle> > product_to_cds;
    GetProductToCDSMap(seh.GetScope(), product_to_cds);
    CRef<CCmdComposite> delete_features_cmd(new CCmdComposite("Remove All Features"));
    count = 0;
    // only delete features on nucleotide sequences, protein features are automatically removed with the coding region
    for (CFeat_CI it(seh); it; ++it)
    {
    CSeq_feat_Handle fh = it->GetSeq_feat_Handle();
    if (!s_IsNucLocation(fh))
        continue;

    CRef<CCmdComposite> cmd = GetDeleteFeatureCommand(fh, remove_proteins, product_to_cds);
    delete_features_cmd->AddCommand(*cmd);
    count++;
    }
    //perfLog.Post(CRequestStatus::e200_Ok);
    return delete_features_cmd;
}


static void s_GetSourceDescriptors(const CSeq_entry& se, vector<pair<const CSeqdesc*, const CSeq_entry*>>& src_descs)
{
    if (se.IsSetDescr()) {
        ITERATE(CBioseq::TDescr::Tdata, it, se.GetDescr().Get()) {
            if ((*it)->IsSource() && (*it)->GetSource().IsSetOrg()) {
                src_descs.emplace_back(*it, &se);
            }
        }
    }

    if (se.IsSet() && se.GetSet().IsSetSeq_set()) {
        ITERATE(CBioseq_set::TSeq_set, it, se.GetSet().GetSeq_set()) {
            s_GetSourceDescriptors(**it, src_descs);
        }
    }
}
 
static void s_FixSpecificHost(COrg_ref& org_ref)
{
    if (!org_ref.IsSetOrgMod()) return;

    for (auto&& it : org_ref.SetOrgname().SetMod()) {
        if (it->IsSetSubtype()
            && it->GetSubtype() == COrgMod::eSubtype_nat_host
            && it->IsSetSubname()) {
            string result = validator::FixSpecificHost(it->GetSubname());
            if (!result.empty() && !NStr::EqualCase(result, it->GetSubname())) {
                it->SetSubname(result);
            }
        }
    }
}


CRef<CCmdComposite> CleanupCommand(CSeq_entry_Handle orig_seh, bool extended, bool do_tax)
{
    CConstRef<CSeq_entry> entry = orig_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);
    
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    CScope scope2(*objmgr);
    scope2.AddDefaults(); 
    CSeq_entry_Handle new_seh = scope2.AddTopLevelSeqEntry(*copy);

    if (do_tax) {
        CRef<CCmdComposite> tax = TaxonomyLookupCommand(new_seh);
        if (tax) {
            tax->Execute();
        }
    }

    CCleanup cleanup;
    cleanup.SetScope(&scope2);
        
    // perform BasicCleanup and ExtendedCleanup
    try {
        CConstRef<CCleanupChange> changes;        
        if (extended) {
            // basic cleanup is intrinsically part of ExtendedCleanup
            changes = cleanup.ExtendedCleanup(new_seh, CCleanup::eClean_SyncGenCodes|CCleanup::eClean_KeepTopSet);
        } else {
            changes = cleanup.BasicCleanup(const_cast<CSeq_entry&>(*copy), CCleanup::eClean_SyncGenCodes);
        }
    } catch (CException& e) {
        LOG_POST(Error << "error in cleanup: " << e.GetMsg());
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Cleanup"));
    CRef<CCmdChangeSeqEntry> clean(new CCmdChangeSeqEntry(orig_seh, copy));
    cmd->AddCommand(*clean);

    return cmd;
}


CRef<CCmdComposite> TaxonomyLookupCommand(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Tax lookup"));

    validator::CTaxValidationAndCleanup tval;
    tval.Init(*(seh.GetCompleteSeq_entry()));

    vector<CRef<COrg_ref> > original_orgs = tval.GetTaxonomyLookupRequest();
    if (original_orgs.empty()) 
    {
        return cmd;
    }
    size_t chunk_size = 250;
    vector< CRef<COrg_ref> > edited_orgs;

    CTaxon3 taxon3;
    taxon3.Init();
    size_t i = 0;
    while (i < original_orgs.size())
    {
        size_t len = min(chunk_size, original_orgs.size() - i);
        vector< CRef<COrg_ref> >  tmp_original_orgs(original_orgs.begin() + i, original_orgs.begin() + i + len);
        vector< CRef<COrg_ref> >  tmp_edited_orgs;
        ITERATE(vector<CRef<COrg_ref> >, it, tmp_original_orgs)
        {
            CRef<COrg_ref> cpy(new COrg_ref());
            cpy->Assign(**it);
            tmp_edited_orgs.push_back(cpy);
        }
        CRef<CTaxon3_reply> tmp_lookup_reply = taxon3.SendOrgRefList(tmp_original_orgs);
        if (tmp_lookup_reply) {
            string error_message;
            tval.AdjustOrgRefsWithTaxLookupReply(*tmp_lookup_reply, tmp_edited_orgs, error_message);
            if (!NStr::IsBlank(error_message))
            {
                // post error message
                LOG_POST(Error << error_message);
                return CRef<CCmdComposite>(NULL);
            }
            edited_orgs.insert(edited_orgs.end(), tmp_edited_orgs.begin(), tmp_edited_orgs.end());
            i += len;
        } else if (chunk_size > 20) {
            // try a smaller chunk size and try again.
            chunk_size = chunk_size / 2;
        } else {
            // post error message
            LOG_POST(Error << "Unable to contact taxonomy server");
            return CRef<CCmdComposite>(NULL);
        }
    }

    vector< CRef<COrg_ref> > spec_host_rq = tval.GetSpecificHostLookupRequest(true);
    i = 0;
    while (i < spec_host_rq.size())
    {
        size_t len = min(chunk_size, spec_host_rq.size() - i);
        vector< CRef<COrg_ref> > tmp_spec_host_rq(spec_host_rq.begin() + i, spec_host_rq.begin() + i + len);
        CRef<CTaxon3_reply> tmp_spec_host_reply = taxon3.SendOrgRefList(tmp_spec_host_rq);
        string error_message = tval.IncrementalSpecificHostMapUpdate(tmp_spec_host_rq, *tmp_spec_host_reply);
        if (!NStr::IsBlank(error_message)) 
        {
            // post error message
            LOG_POST(Error << error_message);
            return CRef<CCmdComposite>(NULL);
        }
        i += len;
    }

    tval.AdjustOrgRefsForSpecificHosts(edited_orgs);

    // update descriptors
    size_t num_descs = tval.NumDescs();
    size_t num_updated_descs = 0;
    for (size_t n = 0; n < num_descs; n++) {
        if (!original_orgs[n]->Equals(*(edited_orgs[n]))) {
            CConstRef<CSeqdesc> original_desc = tval.GetDesc(n);
            CConstRef<CSeq_entry> orig_entry = tval.GetSeqContext(n);
            if (original_desc && orig_entry) {
                CRef<CSeqdesc> new_desc(new CSeqdesc());
                new_desc->Assign(*original_desc);
                new_desc->SetSource().SetOrg().Assign(*(edited_orgs[n]));
                CSeq_entry_Handle orig_seh = seh.GetScope().GetSeq_entryHandle(*orig_entry);
                CRef<CCmdChangeSeqdesc> subcmd(new CCmdChangeSeqdesc(orig_seh, *original_desc, *new_desc));
                cmd->AddCommand(*subcmd);
                num_updated_descs++;
            }
        }
    }

    // now update features
    size_t num_updated_feats = 0;
    for (size_t n = 0; n < tval.NumFeats(); n++) {
        if (!original_orgs[n + num_descs]->Equals(*edited_orgs[n + num_descs])) {
            CConstRef<CSeq_feat> feat = tval.GetFeat(n);
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(*feat);
            new_feat->SetData().SetBiosrc().SetOrg().Assign(*(edited_orgs[n + num_descs]));

            CSeq_feat_Handle fh = seh.GetScope().GetSeq_featHandle(*feat);
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_feat));
            cmd->AddCommand(*chgFeat);

            num_updated_feats++;
        }
    }

    return cmd;
}


CRef<CCmdComposite> SpecificHostCleanupCommand(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Tax lookup"));

    validator::CTaxValidationAndCleanup tval;
    tval.Init(*(seh.GetCompleteSeq_entry()));

    vector<CRef<COrg_ref> > original_orgs = tval.GetTaxonomyLookupRequest();
    if (original_orgs.empty()) 
    {
        return cmd;
    }
    const size_t chunk_size = 1000;
    vector< CRef<COrg_ref> > edited_orgs;

    CTaxon3 taxon3;
    taxon3.Init();
    size_t i = 0;
    while (i < original_orgs.size())
    {
        size_t len = min(chunk_size, original_orgs.size() - i);
        vector< CRef<COrg_ref> >  tmp_original_orgs(original_orgs.begin() + i, original_orgs.begin() + i + len);
        vector< CRef<COrg_ref> >  tmp_edited_orgs;
        ITERATE(vector<CRef<COrg_ref> >, it, tmp_original_orgs)
        {
            CRef<COrg_ref> cpy(new COrg_ref());
            cpy->Assign(**it);
            tmp_edited_orgs.push_back(cpy);
        }
#if 0
        CRef<CTaxon3_reply> tmp_lookup_reply = taxon3.SendOrgRefList(tmp_original_orgs);
        string error_message;
        tval.AdjustOrgRefsWithTaxLookupReply(*tmp_lookup_reply, tmp_edited_orgs, error_message);
        if (!NStr::IsBlank(error_message)) 
        {
            // post error message
            LOG_POST(Error << error_message);
            return CRef<CCmdComposite>(NULL);
        }      
#endif
        edited_orgs.insert(edited_orgs.end(), tmp_edited_orgs.begin(), tmp_edited_orgs.end());
        i += len;
    }

    vector< CRef<COrg_ref> > spec_host_rq = tval.GetSpecificHostLookupRequest(true);
    i = 0;
    while (i < spec_host_rq.size())
    {
        size_t len = min(chunk_size, spec_host_rq.size() - i);
        vector< CRef<COrg_ref> > tmp_spec_host_rq(spec_host_rq.begin() + i, spec_host_rq.begin() + i + len);
        CRef<CTaxon3_reply> tmp_spec_host_reply = taxon3.SendOrgRefList(tmp_spec_host_rq);
        string error_message = tval.IncrementalSpecificHostMapUpdate(tmp_spec_host_rq, *tmp_spec_host_reply);
        if (!NStr::IsBlank(error_message)) 
        {
            // post error message
            LOG_POST(Error << error_message);
            return CRef<CCmdComposite>(NULL);
        }
        i += len;
    }

    tval.AdjustOrgRefsForSpecificHosts(edited_orgs);

    // update descriptors
    size_t num_descs = tval.NumDescs();
    size_t num_updated_descs = 0;
    for (size_t n = 0; n < num_descs; n++) {
        if (!original_orgs[n]->Equals(*(edited_orgs[n]))) {
            CConstRef<CSeqdesc> original_desc = tval.GetDesc(n);
            CConstRef<CSeq_entry> orig_entry = tval.GetSeqContext(n);
            if (original_desc && orig_entry) {
                CRef<CSeqdesc> new_desc(new CSeqdesc());
                new_desc->Assign(*original_desc);
                new_desc->SetSource().SetOrg().Assign(*(edited_orgs[n]));
                CSeq_entry_Handle orig_seh = seh.GetScope().GetSeq_entryHandle(*orig_entry);
                CRef<CCmdChangeSeqdesc> subcmd(new CCmdChangeSeqdesc(orig_seh, *original_desc, *new_desc));
                cmd->AddCommand(*subcmd);
                num_updated_descs++;
            }
        }
    }

    // now update features
    size_t num_updated_feats = 0;
    for (size_t n = 0; n < tval.NumFeats(); n++) {
        if (!original_orgs[n + num_descs]->Equals(*edited_orgs[n + num_descs])) {
            CConstRef<CSeq_feat> feat = tval.GetFeat(n);
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(*feat);
            new_feat->SetData().SetBiosrc().SetOrg().Assign(*(edited_orgs[n + num_descs]));

            CSeq_feat_Handle fh = seh.GetScope().GetSeq_featHandle(*feat);
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_feat));
            cmd->AddCommand(*chgFeat);

            num_updated_feats++;
        }
    }

    return cmd;
}


CRef<CCmdComposite> GetChangeStrainForwardingCommand(CSeq_entry_Handle seh, bool disable)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Disable Strain Forwarding"));

    vector<pair<const CSeqdesc*, const CSeq_entry*>> src_descs;
    vector<CConstRef<CSeq_feat> > src_feats;

    CScope& scope = seh.GetScope();
    s_GetSourceDescriptors(*(seh.GetCompleteSeq_entry()), src_descs);
    for (auto&& desc_it : src_descs) {
        const CSeqdesc& old_desc = *desc_it.first;
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->Assign(old_desc);
        new_desc->SetSource().SetDisableStrainForwarding(disable);
        CRef<CCmdChangeSeqdesc> subcmd(new CCmdChangeSeqdesc(scope.GetSeq_entryHandle(*desc_it.second), old_desc, *new_desc));
        cmd->AddCommand(*subcmd);
    }

    CFeat_CI feat_it (seh, SAnnotSelector(CSeqFeatData::e_Biosrc));
    while (feat_it) {
        CConstRef<CSeq_feat> old_feat = feat_it->GetSeq_feat();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*old_feat);
        new_feat->SetData().SetBiosrc().SetDisableStrainForwarding(disable);
        CSeq_feat_Handle fh = seh.GetScope().GetSeq_featHandle(*old_feat);
        CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_feat));
        cmd->AddCommand(*chgFeat);
        ++feat_it;
    }

    return cmd;
}

CRef<CCmdComposite> GetDisableStrainForwardingCommand(CSeq_entry_Handle seh)
{
    return GetChangeStrainForwardingCommand(seh, true);
}

CRef<CCmdComposite> GetEnableStrainForwardingCommand(CSeq_entry_Handle seh)
{
    return GetChangeStrainForwardingCommand(seh, false);
}


bool s_IsSpeciesLevel(const COrg_ref& org)
{
    bool rval = false;
    CRef<CT3Request> rq(new CT3Request());
    rq->SetOrg().Assign(org);
    CTaxon3_request request;
    request.SetRequest().push_back(rq);
    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply = taxon3.SendRequest(request);
    if (reply) {
        CTaxon3_reply::TReply::const_iterator reply_it = reply->GetReply().begin();
        while (reply_it != reply->GetReply().end()) {
            if ((*reply_it)->IsData() 
                && (*reply_it)->GetData().GetOrg().IsSetTaxname()) {
                bool is_species_level = false, force_consult = false, has_nucleomorphs = false;
                (*reply_it)->GetData().GetTaxFlags (is_species_level, force_consult, has_nucleomorphs);
                if (is_species_level) {
                    rval = true;
                }
                break;
            }
            ++reply_it;
        }
    }
    return rval;
}


CRef<CCmdComposite> GetRmCultureNotesCommand(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Rm Culture Notes"));

    vector<pair<const CSeqdesc*, const CSeq_entry*>> src_descs;
    vector<bool> is_species_level;

    s_GetSourceDescriptors(*(seh.GetCompleteSeq_entry()), src_descs);

    CTaxon3_request request;
    for (auto&& desc_it : src_descs) {
        const CSeqdesc& desc = *desc_it.first;
        CRef<CT3Request> rq(new CT3Request());
        rq->SetOrg().Assign(desc.GetSource().GetOrg());
        request.SetRequest().push_back(rq);
        is_species_level.push_back(false);
    }

    CFeat_CI feat_it (seh, SAnnotSelector(CSeqFeatData::e_Biosrc));
    while (feat_it) {
        CRef<CT3Request> rq(new CT3Request());
        rq->SetOrg().Assign(feat_it->GetData().GetBiosrc().GetOrg());
        request.SetRequest().push_back(rq);
        is_species_level.push_back(false);
        ++feat_it;
    }

    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply = taxon3.SendRequest(request);
    if (reply) {
        CTaxon3_reply::TReply::const_iterator reply_it = reply->GetReply().begin();
        size_t pos = 0;
        while (reply_it != reply->GetReply().end()) {
            if ((*reply_it)->IsData() 
                && (*reply_it)->GetData().GetOrg().IsSetTaxname()) {
                bool species_level = false, force_consult = false, has_nucleomorphs = false;
                (*reply_it)->GetData().GetTaxFlags (species_level, force_consult, has_nucleomorphs);
                if (species_level) {
                    is_species_level[pos] = true;
                }
            }
            ++reply_it;
            ++pos;
        }
    }

    CScope& scope = seh.GetScope();
    size_t pos = 0;
    for (auto&& desc_it : src_descs) {
        const CSeqdesc& old_desc = *desc_it.first;
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->Assign(old_desc);
        new_desc->SetSource().RemoveCultureNotes(is_species_level[pos]);
        CRef<CCmdChangeSeqdesc> subcmd(new CCmdChangeSeqdesc(scope.GetSeq_entryHandle(*desc_it.second), old_desc, *new_desc));
        cmd->AddCommand(*subcmd);
        ++pos;
    }

    feat_it.Rewind();
    while (feat_it) {
        CConstRef<CSeq_feat> old_feat = feat_it->GetSeq_feat();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*old_feat);
        new_feat->SetData().SetBiosrc().RemoveCultureNotes(is_species_level[pos]);
        CSeq_feat_Handle fh = seh.GetScope().GetSeq_featHandle(*old_feat);
        CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_feat));
        cmd->AddCommand(*chgFeat);
        ++pos;
        ++feat_it;
    }

    return cmd;
}


CRef<CCmdComposite> GetSynchronizeProductMolInfoCommand(CScope& scope, const CSeq_feat& cds)
{
    if (!cds.IsSetData() || !cds.GetData().IsCdregion() || !cds.IsSetProduct()) {
        return CRef<CCmdComposite>(NULL);
    }

    CBioseq_Handle product = scope.GetBioseqHandle(cds.GetProduct());
    if (!product || (product && product.IsNa())) {
        return CRef<CCmdComposite>(NULL);
    }

    // change molinfo on bioseq
    CRef<CSeqdesc> new_molinfo_desc( new CSeqdesc );
    CMolInfo & molinfo = new_molinfo_desc->SetMolinfo();
    bool molinfo_changed = false;
    CSeqdesc_CI desc_ci( product, CSeqdesc::e_Molinfo);
    if (desc_ci) {
        molinfo.Assign(desc_ci->GetMolinfo());
    } else {
        molinfo.SetBiomol(CMolInfo::eBiomol_peptide);
        molinfo_changed = true;
    }

    molinfo_changed |= feature::AdjustProteinMolInfoToMatchCDS(molinfo, cds);

    if (molinfo_changed) {
        CRef<CCmdComposite> cmd(new CCmdComposite("Create molinfo"));
        if (desc_ci) {
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_molinfo_desc));
            cmd->AddCommand(*ecmd);
        } else {
            CRef<CCmdCreateDesc> ecmd(new CCmdCreateDesc(product.GetParentEntry(), *new_molinfo_desc));
            cmd->AddCommand(*ecmd);
        }
        return cmd;
    }
    return CRef<CCmdComposite>(NULL);
}


CRef<CCmdComposite> GetSynchronizeProteinPartialsCommand(CScope& scope, const CSeq_feat& cds)
{
    if (!cds.IsSetData() || !cds.GetData().IsCdregion() || !cds.IsSetProduct()) {
        return CRef<CCmdComposite>(NULL);
    }

    CBioseq_Handle product = scope.GetBioseqHandle(cds.GetProduct());
    if (!product) {
        return CRef<CCmdComposite>(NULL);
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Synchronize Partials"));
    bool any_cmd = false;

    // change molinfo on bioseq
    CRef<CCmdComposite> synch_molinfo = GetSynchronizeProductMolInfoCommand(scope, cds);
    if (synch_molinfo) {
        cmd->AddCommand(*synch_molinfo);
        any_cmd = true;
    }

    // change partials for protein feature
    CFeat_CI f(product, SAnnotSelector(CSeqFeatData::eSubtype_prot));
    if (f) {
        CRef<CSeq_feat> new_prot(new CSeq_feat());
        new_prot->Assign(*(f->GetSeq_feat()));
        if (edit::AdjustProteinFeaturePartialsToMatchCDS(*new_prot, cds)) {
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(f->GetSeq_feat_Handle(), *new_prot)));
            any_cmd = true;
        }
    }
    
    if (!any_cmd) {
        cmd.Reset(NULL);
    }
    return cmd;
}

void SetProteinFeature(CSeq_feat& prot, CBioseq_Handle product, const CSeq_feat& cds)
{
    bool partial5 = cds.GetLocation().IsPartialStart(eExtreme_Biological);
    bool partial3 = cds.GetLocation().IsPartialStop(eExtreme_Biological);

    prot.SetData().SetProt();
    CRef<CSeq_id> id(new CSeq_id());
    id->Assign(*(product.GetId().front().GetSeqId()));
    prot.SetLocation().SetInt().SetId(*id);
    prot.SetLocation().SetInt().SetFrom(0);
    prot.SetLocation().SetInt().SetTo(product.GetInst_Length() - 1);
    prot.SetLocation().SetPartialStart(partial5, eExtreme_Biological);
    prot.SetLocation().SetPartialStop(partial3, eExtreme_Biological);
    prot.SetPartial(partial5 || partial3);
}

void AdjustProteinFeature(CSeq_feat& prot, CBioseq_Handle product, CSeq_feat& cds, bool& cds_change)
{
    SetProteinFeature(prot, product, cds);

    if (cds.IsSetXref()) {
        NON_CONST_ITERATE (CSeq_feat::TXref, it, cds.SetXref()) {
            if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                prot.SetData().SetProt().Assign((*it)->GetData().GetProt());
                cds.SetXref().erase(it);
                cds_change = true;
                break;
            }
        }
        if (cds.GetXref().size() == 0) {
            cds.ResetXref();
            cds_change = true;
        }
    }
}


CRef<CCmdComposite> CreateOrAdjustProteinFeature(CBioseq_Handle product, CSeq_feat& cds, bool& cds_change)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Create or Adjust Protein Feature"));
    CRef<CSeq_feat> prot(new CSeq_feat());
    CFeat_CI existing(product, CSeqFeatData::eSubtype_prot);
    if (existing) {
        prot->Assign(*((*existing).GetSeq_feat()));
        AdjustProteinFeature(*prot, product, cds, cds_change);
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(*existing, *prot)));
    } else {
        AdjustProteinFeature(*prot, product, cds, cds_change);
        CSeq_entry_Handle psh = product.GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(psh, *prot)));
    }
    CCleanup cleanup;
    cleanup.BasicCleanup(*prot);

    return cmd;
}


CRef<CCmdComposite> TranslateCDSCommand(CScope& scope, CSeq_feat& cds, bool& cds_change, int& offset, bool create_general_only)
{
    CRef<CBioseq> new_product = CSeqTranslator::TranslateToProtein(cds, scope);
    if (!new_product) {
        return CRef<CCmdComposite>(NULL);
    }
    cds_change = false; // set to true if we need a command to change the coding region
    if (!new_product->IsSetId()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(cds.GetLocation());
        CSeq_entry_Handle seh = bsh.GetTopLevelEntry();
        CRef<CSeq_id> product_id;
        if (cds.IsSetProduct()) {
            product_id.Reset(new CSeq_id());
            product_id->Assign(*(cds.GetProduct().GetId()));
        } else {
            string id_label;
            product_id = edit::GetNewProtId(bsh, offset, id_label, create_general_only);
            cds.SetProduct().SetWhole().Assign(*product_id);
            cds_change = true;
        }
        
        new_product->SetId().push_back(product_id);
    }

    CRef<CSeq_entry> prot_entry (new CSeq_entry());
    prot_entry->SetSeq(*new_product);
    bool partial5 = cds.GetLocation().IsPartialStart(eExtreme_Biological);
    bool partial3 = cds.GetLocation().IsPartialStop(eExtreme_Biological);
    SetMolinfoForProtein (prot_entry, partial5, partial3);
    CRef<CSeq_feat> prot = AddEmptyProteinFeatureToProtein(prot_entry, partial5, partial3);
    if (cds.IsSetXref()) {
        NON_CONST_ITERATE (CSeq_feat::TXref, it, cds.SetXref()) {
            if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                prot->SetData().SetProt().Assign((*it)->GetData().GetProt());
                cds.SetXref().erase(it);
                cds_change = true;
                break;
            }
        }
        if (cds.GetXref().size() == 0) {
            cds.ResetXref();
            cds_change = true;
        }
    }
                    

    CRef<CCmdComposite> cmd(new CCmdComposite("Translate CDS"));

    CRef<CCmdAddSeqEntry> subcmd(new CCmdAddSeqEntry(prot_entry, 
                                            scope.GetBioseqHandle(cds.GetLocation()).GetParentEntry()));
    cmd->AddCommand(*subcmd);
    return cmd;
}


CRef<CCmdComposite> GetRetranslateCDSCommand(CScope& scope, CSeq_feat& cds, bool& cds_change, bool create_general_only)
{
    int offset = 1;
    return GetRetranslateCDSCommand(scope, cds, cds_change, offset, create_general_only);
}


CRef<CCmdComposite> GetRetranslateCDSCommand(CScope& scope, CSeq_feat& cds, bool& cds_change, int& offset, bool create_general_only)
{
    // feature must be cds
    if (!cds.IsSetData() && !cds.GetData().IsCdregion()) {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }
    cds_change = false;

    if (!cds.IsSetProduct()) {
        return TranslateCDSCommand(scope, cds, cds_change, offset, create_general_only);
    }

    // Use Cdregion.Product to get handle to protein bioseq 
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds.GetProduct());

    if (!prot_bsh) {
        return TranslateCDSCommand(scope, cds, cds_change, offset, create_general_only);
    }

    // Should be a protein!
    if (!prot_bsh.IsProtein())
    {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }


    // Make a copy of existing CSeq_inst
    CRef<CSeq_inst> new_inst(new CSeq_inst());
    new_inst->Assign(prot_bsh.GetInst());

    // Make edits to the CSeq_inst copy
    CRef<CBioseq> new_protein_bioseq;
    if (new_inst->IsSetSeq_data())
    {
        // Generate new protein sequence data and length
        new_protein_bioseq = CSeqTranslator::TranslateToProtein(cds, scope);
        if (new_protein_bioseq && new_protein_bioseq->IsSetInst())
            new_inst->Assign(new_protein_bioseq->GetInst());
    }
    if ( !new_protein_bioseq ) {
        string prot_seq;
        try
        {
            CSeqTranslator::Translate(cds, scope, prot_seq);
        }
        catch (CSeqMapException&) {}

        if (prot_seq.empty()) {
            CRef<CCmdComposite> empty(NULL);
            return empty;
        }
        else {
            new_protein_bioseq.Reset(new CBioseq);
            new_protein_bioseq->Assign(*(prot_bsh.GetCompleteBioseq()));
            if (NStr::EndsWith(prot_seq, "*")) {
                prot_seq = prot_seq.substr(0, prot_seq.length() - 1);
            }
            new_protein_bioseq->ResetInst();
            new_protein_bioseq->SetInst().SetRepr(CSeq_inst::eRepr_raw);
            new_protein_bioseq->SetInst().SetSeq_data().SetNcbieaa().Set(prot_seq);
            new_protein_bioseq->SetInst().SetLength(TSeqPos(prot_seq.length()));
            new_protein_bioseq->SetInst().SetMol(CSeq_inst::eMol_aa);
            new_inst->Assign(new_protein_bioseq->GetInst());
        }
    }
    CRef<CCmdComposite> cmd(new CCmdComposite("Retranslate CDS"));

    // Update protein sequence data and length
    CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(prot_bsh, 
                                 *new_inst));
    cmd->AddCommand(*chgInst);

    // change molinfo on bioseq
    CRef<CCmdComposite> synch_molinfo = GetSynchronizeProductMolInfoCommand(scope, cds);
    if (synch_molinfo) {
        cmd->AddCommand(*synch_molinfo);
    }

    // If protein feature exists, update it
    for (CFeat_CI prot_feat_ci(prot_bsh, CSeqFeatData::e_Prot); prot_feat_ci; ++prot_feat_ci ) 
    {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(prot_feat_ci->GetOriginalFeature());

        if (prot_feat_ci->GetFeatSubtype() != CSeqFeatData::eSubtype_prot)
        {
            if (new_feat->IsSetLocation() && new_feat->GetLocation().GetStop(eExtreme_Positional) >= new_inst->GetLength())
            {
                CRef<CSeq_loc> sub(new CSeq_loc);
                sub->SetInt().SetFrom(new_inst->GetLength());
                sub->SetInt().SetTo(new_feat->GetLocation().GetStop(eExtreme_Positional));
                CRef<CSeq_id> id(new CSeq_id);
                id->Assign(*prot_bsh.GetSeqId());
                sub->SetId(*id);
                CRef<CSeq_loc> new_loc = new_feat->SetLocation().Subtract(*sub, 0, nullptr, nullptr); // TODO delete feature if location is removed completely
                new_feat->SetLocation(*new_loc);
                
            }
        }
        else
        {
            if ( new_feat->CanGetLocation() &&
                 new_feat->GetLocation().IsInt() &&
                 new_feat->GetLocation().GetInt().CanGetTo() )
            {
                new_feat->SetLocation().SetInt().SetTo(new_protein_bioseq->GetLength() - 1);
            }
            edit::AdjustProteinFeaturePartialsToMatchCDS(*new_feat, cds);
        }


        CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*prot_feat_ci,
                                                           *new_feat));
        cmd->AddCommand(*chgFeat);
    }

    return cmd;
}


CRef<CCmdComposite> GetRetranslateCDSCommand(CScope& scope, const CSeq_feat& cds, bool create_general_only)
{
    int offset = 1;
    return GetRetranslateCDSCommand(scope, cds, offset, create_general_only);
}


CRef<CCmdComposite> GetRetranslateCDSCommand(CScope& scope, const CSeq_feat& cds, int& offset, bool create_general_only)
{
    // feature must be cds
    if (!cds.IsSetData() && !cds.GetData().IsCdregion()) {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }

    bool cds_change = false;
    CRef<CSeq_feat> new_cds(new CSeq_feat());
    new_cds->Assign(cds);
    CRef<CCmdComposite> cmd = GetRetranslateCDSCommand(scope, *new_cds, cds_change, offset, create_general_only);
    if (cds_change) {
        CSeq_feat_Handle fh;
        try {
            fh = scope.GetSeq_featHandle(cds);
        } catch (CException&) {
        }

        if (fh) {
            // if feature handle doesn't exist, feature can't be adjusted
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_cds));
            cmd->AddCommand(*chgFeat);
        }
    }
    return cmd;
}


/// SetTranslExcept
/// A function to set a code break at the 3' end of coding regions in a Seq-entry
/// to indicate that the stop codon is formed by the addition of a poly-A tail.
/// @param seh         The Seq-entry to search for coding regions to adjust
/// @param comment     The string to place in the note on cds if a code break is added
/// @param strict      Only add code break if last partial codon consists of "TA" or just "T".
///                    If strict is false, add code break if first NT of last partial codon
///                    is T or N.
/// @param extend      If true, extend coding region to cover partial stop codon
/// @param adjust_gene If true, adjust gene location to match coding region after adjustment
CRef<CCmdComposite> SetTranslExcept(CSeq_entry_Handle seh, const string& comment, bool strict, bool extend, bool adjust_gene)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Set Transl Except"));
    CScope& scope = seh.GetScope();
    CFeat_CI feat_ci (seh, SAnnotSelector(CSeqFeatData::e_Cdregion));
    bool any = false;

    while (feat_ci) {
        // TODO: first, get gene before location changes
        CConstRef<CSeq_feat> gene(NULL);
        if (adjust_gene) {
            gene = sequence::GetOverlappingGene(feat_ci->GetLocation(), scope);
        }
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_ci->GetOriginalFeature());
        if (edit::SetTranslExcept(*new_feat, comment, strict, extend, scope)) {
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*feat_ci,
                                   *new_feat));
            cmd->AddCommand(*chgFeat);
            // if extending feature, also extend gene
            if (adjust_gene && gene) {
                TSeqPos cds_stop = new_feat->GetLocation().GetStop(eExtreme_Biological);
                TSeqPos gene_stop = gene->GetLocation().GetStop(eExtreme_Biological);
                TSeqPos diff = 0;
                if (gene->GetLocation().GetStrand() == eNa_strand_minus) {
                    if (gene_stop > cds_stop) {
                        diff = gene_stop - cds_stop;
                    }
                } else {
                    if (gene_stop < cds_stop) {
                        diff = cds_stop - gene_stop;
                    }
                }
                if (diff > 0) {
                    CRef<CSeq_feat> new_gene(new CSeq_feat());
                    new_gene->Assign(*gene);
                    edit::ExtendStop(new_gene->SetLocation(), diff, scope);
                    CSeq_feat_Handle gh = scope.GetSeq_featHandle(*gene);
                    CIRef<IEditCommand> chgGene(new CCmdChangeSeq_feat(gh, *new_gene));
                    cmd->AddCommand(*chgGene);
                }
            }
            any = true;
        }
        ++feat_ci;
    }

    if (!any) {
        cmd.Reset(NULL);
    }
    return cmd;
}


CRef<CCmdComposite> GetReverseComplimentSequenceCommand(CBioseq_Handle bsh)
{
    if (!bsh)
        return CRef<CCmdComposite>(NULL);

    CRef<CCmdComposite> cmd(new CCmdComposite("Reverse Compliment Bioseq"));
    CScope &scope =  bsh.GetScope();

    // Change Inst
    CRef<CSeq_inst> inst(new CSeq_inst());
    inst->Assign(bsh.GetInst());
    ReverseComplement(*inst, &scope);
    CRef<CCmdChangeBioseqInst> flip(new CCmdChangeBioseqInst(bsh, *inst));
    cmd->AddCommand(*flip);

    // Change feature locations
    CFeat_CI fit(bsh);
    while (fit) {
        CConstRef<CSeq_feat> f = fit->GetSeq_feat();
        CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        edit::ReverseComplementFeature(*new_feat, scope);
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
        ++fit;
    }

    return cmd;
}

//////////////////////////////////////////////////////////////////
/// TrimStopsFromCompleteCDS
/// If the CDS is 3' complete and the protein sequence ends with a stop codon,
/// the function removes the stop codon, shortens the protein length and 
/// adjusts the features on the protein sequence so that they do not extend past the
/// "new" end of the sequence. The function does not modify either the CDS location or 
/// the nucleotide sequence. 
/// @param cds        Coding region feature which product will be examined
/// @param scope      The scope in which adjustments are to be made (if necessary)
///
/// @return           True if stop codon was found, removed and features on the protein sequence
///                   were adjusted, false otherwise
CRef<CCmdComposite> TrimStopsFromCompleteCDS(const CSeq_feat& cds, CScope& scope)
{
    CRef<CCmdComposite> empty(NULL);

    if (!cds.GetData().IsCdregion() || !cds.IsSetProduct()) {
        return empty;
    }
    // check if cds is 3' complete
    if (cds.GetLocation().IsPartialStop(eExtreme_Biological)) {
        return empty;
    }
    CBioseq_Handle product = scope.GetBioseqHandle(cds.GetProduct());
    if (!product || !product.IsProtein()) {
        return empty;
    }
    // check if protein sequence ends with a stop residue
    CSeqVector prot_vec = product.GetSeqVector(CBioseq_Handle::eCoding_Ncbi);
    prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
    string prot_seq;
    prot_vec.GetSeqData(0, prot_vec.size(), prot_seq);
    if (!NStr::EndsWith(prot_seq, '*')) {
        return empty;
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Remove trailing * from complete CDS"));

    CRef<CSeq_inst> new_inst(new CSeq_inst());
    new_inst->Assign(product.GetInst());

    // Make edits to the CSeq_inst copy - shorten length by one
    bool modified = false;
    if (new_inst->IsSetSeq_data()) {
        prot_seq.erase(prot_seq.end() - 1);
        if (product.GetInst_Seq_data().IsIupacaa()) {
            new_inst->SetSeq_data().SetIupacaa().Set(prot_seq);
            new_inst->SetLength((TSeqPos)prot_seq.size());
            modified = true;
        } else if (product.GetInst_Seq_data().IsNcbieaa()) {
            new_inst->SetSeq_data().SetNcbieaa().Set(prot_seq);
            new_inst->SetLength((TSeqPos)prot_seq.size());
            modified = true;
        }
    }
        
    if (modified) {
        CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(product, *new_inst));
        cmd->AddCommand(*chgInst);

        // If features exist, adjust them to the new sequence end
        SAnnotSelector sel(CSeqFeatData::e_Prot);
        CFeat_CI feat_ci(product);
        for ( ; feat_ci; ++feat_ci ) {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(feat_ci->GetOriginalFeature());
            if (new_feat->CanGetLocation() && 
                new_feat->GetLocation().IsInt() &&  // only single intervals are tested
                new_feat->GetLocation().GetInt().GetTo()) {
            
                TSeqPos feat_stop = new_feat->GetLocation().GetStop(eExtreme_Biological);
                if (feat_stop > new_inst->GetLength() -1 ) {
                    new_feat->SetLocation().SetInt().SetTo(new_inst->GetLength() - 1);
                    CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*feat_ci, *new_feat));
                    cmd->AddCommand(*chgFeat);
                }
            }
        }
        return cmd;
    } 
    return empty;
}

static const char* kDiscrepancyEdit = "Dialogs.Edit.Discrepancy";

static string s_GetProductRules()
{
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryReadView reg_view = reg.GetReadView(kDiscrepancyEdit);
    string fname = reg_view.GetString("ProductRules");
    CFile file(fname);
    return file.Exists() ? fname : kEmptyStr;
}

CRef<CCmdComposite> AutofixCommand(CSeq_entry_Handle orig_seh, const string& test_name, string* output, const string& suspect_rules)
{
    CConstRef<CSeq_entry> entry = orig_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    CScope scope2(*objmgr);
    scope2.AddDefaults();
    CSeq_entry_Handle new_seh = scope2.AddTopLevelSeqEntry(*copy);
    

    CRef<NDiscrepancy::CDiscrepancySet> discrSet = NDiscrepancy::CDiscrepancySet::New(scope2);
    discrSet->AddTest(test_name);
    if (suspect_rules.empty()) {
        discrSet->SetSuspectRules(s_GetProductRules());
    }
    else {
        discrSet->SetSuspectRules(suspect_rules);
    }
    discrSet->Parse(*copy.GetNCPointer());
    discrSet->Summarize();

    const map<string, CRef<NDiscrepancy::CDiscrepancyCase> >& tests = discrSet->GetTests();
    if (tests.size() != 1) {
        return CRef<CCmdComposite>();
    }

    NDiscrepancy::TReportObjectList tofix;
    NDiscrepancy::TReportItemList reports = tests.at(test_name)->GetReport();
    for (auto it : reports) {
        for (auto obj : it->GetDetails()) {
            if (obj->CanAutofix()) {
                tofix.push_back(CRef<NDiscrepancy::CReportObj>(&*obj));
            }
        }
    }
    if (tofix.empty()) {
        return CRef<CCmdComposite>();
    }

    map<string, size_t> result;
    discrSet->Autofix(tofix, result);

    if (output && result.size()) {
        for (auto J : result) {
            *output += NDiscrepancy::CDiscrepancySet::Format(J.first, (unsigned)J.second) + "\n";
        }
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Perform Autofix for " + test_name + " discrepancy test"));
    CRef<CCmdChangeSeqEntry> clean(new CCmdChangeSeqEntry(orig_seh, copy));
    cmd->AddCommand(*clean);

    return cmd;
}

//////////////////////////////////////////////////////////////////
/// GetEditLocationCommand
/// Changes the location properties of a feature according to a location policy
///
CRef<CCmdComposite> GetEditLocationCommand(const edit::CLocationEditPolicy& policy,
    bool retranslate, bool adjust_gene, const CSeq_feat& orig_feat, CScope& scope,
    int& offset, bool create_general_only)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Edit Location"));
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig_feat);

    bool any_change = policy.ApplyPolicyToFeature(*new_feat, scope);

    if (any_change) {
        // adjust gene feature
        if (adjust_gene) {
            CRef<CCmdChangeSeq_feat> adjust_gene_cmd = AdjustGene(orig_feat, *new_feat, scope);
            if (adjust_gene_cmd) {
                cmd->AddCommand(*adjust_gene_cmd);
            }
        }

        // set partial flag
        new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Biological) || new_feat->GetLocation().IsPartialStop(eExtreme_Biological));

        // command to change feature
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(orig_feat), *new_feat)));

        // retranslate or resynch if coding region
        if (new_feat->IsSetProduct() && new_feat->GetData().IsCdregion()) {
            if (retranslate) {
                CRef<CCmdComposite> retranslate_cmd = GetRetranslateCDSCommand(scope, *new_feat, offset, create_general_only);
                if (retranslate_cmd) {
                    cmd->AddCommand(*retranslate_cmd);
                }
            }
            else {
                CRef<CCmdComposite> synch = GetSynchronizeProteinPartialsCommand(scope, *new_feat);
                if (synch) {
                    cmd->AddCommand(*synch);
                }
            }
        }

    }
    else {
        cmd.Reset(NULL);
    }
    return cmd;
}

CRef<CCmdChangeSeq_feat> AdjustGene(const CSeq_feat& orig_feat, const CSeq_feat& new_feat, CScope& scope)
{
    CConstRef<CSeq_feat> old_gene = sequence::GetOverlappingGene(orig_feat.GetLocation(), scope);
    if (!old_gene) {
        return CRef<CCmdChangeSeq_feat>();
    }
    
    size_t feat_start = new_feat.GetLocation().GetStart(eExtreme_Biological);
    size_t feat_stop = new_feat.GetLocation().GetStop(eExtreme_Biological);
    CRef<CSeq_feat> new_gene(new CSeq_feat());
    new_gene->Assign(*old_gene);

    bool gene_change = false;
    // adjust ends of gene to match ends of feature
    CRef<CSeq_loc> new_loc = edit::SeqLocExtend(new_gene->GetLocation(), feat_start, &scope);
    if (new_loc) {
        new_gene->SetLocation().Assign(*new_loc);
        gene_change = true;
    }
    new_loc = edit::SeqLocExtend(new_gene->GetLocation(), feat_stop, &scope);
    if (new_loc) {
        new_gene->SetLocation().Assign(*new_loc);
        gene_change = true;
    }
    // adjust partial ends and the partial flag
    new_gene->SetLocation().SetPartialStart(new_feat.GetLocation().IsPartialStart(eExtreme_Biological), eExtreme_Biological);
    new_gene->SetLocation().SetPartialStop(new_feat.GetLocation().IsPartialStop(eExtreme_Biological), eExtreme_Biological);
    new_gene->SetPartial(new_gene->GetLocation().IsPartialStart(eExtreme_Biological) || new_gene->GetLocation().IsPartialStop(eExtreme_Biological));
    if (new_gene->GetLocation().IsPartialStart(eExtreme_Biological) != old_gene->GetLocation().IsPartialStart(eExtreme_Biological) ||
        new_gene->GetLocation().IsPartialStop(eExtreme_Biological) != old_gene->GetLocation().IsPartialStop(eExtreme_Biological)) {

        gene_change = true;
    }
        
    if (gene_change) {
        return Ref(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*old_gene), *new_gene));
    }
    return CRef<CCmdChangeSeq_feat>();
}


bool GetNormalizeGeneQualsCommand(CBioseq_Handle bsh, CRef<CCmdComposite> cmd)
{
    bool any_change = false;

    vector<CCleanup::TFeatGenePair> cds_gene_pairs = CCleanup::GetNormalizableGeneQualPairs(bsh);
    for (auto copy_pair : cds_gene_pairs) {
        CRef<CSeq_feat> new_cds(new CSeq_feat());
        new_cds->Assign(*(copy_pair.first.GetOriginalSeq_feat()));
        CRef<CSeq_feat> new_gene(new CSeq_feat());
        new_gene->Assign(*(copy_pair.second.GetOriginalSeq_feat()));
        if (CCleanup::NormalizeGeneQuals(*new_cds, *new_gene)) {
            CIRef<IEditCommand> chgGene(new CCmdChangeSeq_feat(copy_pair.second, *new_gene));
            cmd->AddCommand(*chgGene);
            CIRef<IEditCommand> chgCDS(new CCmdChangeSeq_feat(copy_pair.first, *new_cds));
            cmd->AddCommand(*chgCDS);
            any_change = true;
        }
    }
    return any_change;
}


CRef<CCmdComposite> GetNormalizeGeneQualsCommand(CSeq_entry_Handle seh)
{
    CBioseq_CI bi(seh, CSeq_inst::eMol_na);
    CRef<CCmdComposite> cmd(new CCmdComposite("Normalize Gene Quals"));

    bool any = false;
    while (bi) {
        any |= GetNormalizeGeneQualsCommand(*bi, cmd);
        ++bi;
    }
    if (!any) {
        cmd.Reset(NULL);
    }
    return cmd;
}


bool GetRemoveDuplicateGOTermsCommand(CBioseq_Handle bsh, CRef<CCmdComposite> cmd)
{
    bool any_change = false;

    CFeat_CI f(bsh);
    while (f) {
        CRef<CSeq_feat> cp(new CSeq_feat());
        cp->Assign(*(f->GetSeq_feat()));
        if (validator::RemoveDuplicateGoTerms(*cp)) {
            CSeq_feat_Handle fh = f->GetSeq_feat_Handle();
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *cp));
            cmd->AddCommand(*chgFeat);
            any_change = true;
        }
        ++f;
    }
    return any_change;
}


CRef<CCmdComposite> GetRemoveDuplicateGOTermsCommand(CSeq_entry_Handle seh)
{
    CBioseq_CI bi(seh, CSeq_inst::eMol_na);
    CRef<CCmdComposite> cmd(new CCmdComposite("Remove Duplicate GO Terms"));

    bool any = false;
    while (bi) {
        any |= GetRemoveDuplicateGOTermsCommand(*bi, cmd);
        ++bi;
    }
    if (!any) {
        cmd.Reset(NULL);
    }
    return cmd;
}


CRef<CCmdComposite> UpdatemRNAProduct(const CSeq_feat& protein, CScope& scope, string& message)
{
    if (!protein.IsSetData() ||
        (protein.IsSetData() && protein.GetData().GetSubtype() != CSeqFeatData::eSubtype_prot)) {
        return CRef<CCmdComposite>(nullptr);
    }
    CRef<CCmdComposite> cmd;
    const string& product = (protein.GetData().GetProt().IsSetName()) ? protein.GetData().GetProt().GetName().front() : kEmptyStr;
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(protein.GetLocation());
    const CSeq_feat* cds = sequence::GetCDSForProduct(*prot_bsh.GetCompleteBioseq(), &scope);
    if (cds) {
        CConstRef<CSeq_feat> mrna = sequence::GetmRNAforCDS(*cds, scope);
        if (mrna) {
            CRef<CSeq_feat> new_mrna(new CSeq_feat());
            new_mrna->Assign(*mrna);

            bool modified = false;
            if (mrna->GetData().IsRna() && mrna->GetData().GetRna().GetType() == CRNA_ref::eType_mRNA) {
                CRNA_ref& rna = new_mrna->SetData().SetRna();
                if (product.empty()) {
                    rna.ResetExt();
                    message += "reset the mRNA product name ";
                    modified = true;
                }
                else if (!rna.IsSetExt() || (rna.IsSetExt() && !NStr::Equal(rna.GetExt().GetName(), product))) {
                    rna.SetExt().SetName(product);
                    message = "applied " + product + " to mRNA product name ";
                    modified = true;
                }
            }

            if (modified) {
                CSeq_feat_Handle fh = scope.GetSeq_featHandle(*mrna);
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_mrna));
                cmd.Reset(new CCmdComposite("Update mRNA product to match CDS product name"));
                cmd->AddCommand(*chgFeat);
            }
        }
    }
    return cmd;
}


CRef<CCmdComposite> UpdatemRNAProduct(const string& prot_product, CConstRef<CObject> object, CScope& scope)
{
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(object.GetPointer());
    // at this point, only handle protein features
    // TO DO: extend it to other features as well
    if (!seq_feat) {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }

    CConstRef<CSeq_feat> orig_mrna;
    CSeqFeatData::ESubtype subtype = seq_feat->GetData().GetSubtype();

    switch (subtype) {
    case CSeqFeatData::eSubtype_prot: {
        // for proteins: find first the CDS and then find the original overlapping mRNA
        CBioseq_Handle prot_bsh = scope.GetBioseqHandle(seq_feat->GetLocation());
        const CSeq_feat* cds = sequence::GetCDSForProduct(prot_bsh);
        if (cds) {
            orig_mrna.Reset(sequence::GetmRNAforCDS(*cds, scope));
        }
        break;
    }
    default:
        break;
    }

    if (orig_mrna) {
        CRef<CCmdComposite> cmd(new CCmdComposite("Update mRNA product name to match CDS protein name"));
        CRef<CSeq_feat> new_mrna(new CSeq_feat());
        new_mrna->Assign(*orig_mrna);

        bool modified = false;
        if (orig_mrna->GetData().IsRna()) {
            string remainder;
            new_mrna->SetData().SetRna().SetRnaProductName(prot_product, remainder);
            modified = true;
        }

        if (modified) {
            CSeq_feat_Handle fh = scope.GetSeq_featHandle(*orig_mrna);
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_mrna));
            cmd->AddCommand(*chgFeat);
            return cmd;
        }
    }
    CRef<CCmdComposite> empty(NULL);
    return empty;
}

END_NCBI_SCOPE
