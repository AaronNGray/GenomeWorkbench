/*  $Id: test_cmd_propagate_feat.cpp 42666 2019-04-01 14:27:25Z filippov $
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
 * File Description:
 */

#include <ncbi_pch.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
 // This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/objutils/cmd_propagate_feat.hpp>


USING_NCBI_SCOPE;
USING_SCOPE(objects);

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

CRef<CSeq_entry> ReadEntryFromFile(const string& fname)
{
    CRef<CSeq_entry> entry(new CSeq_entry);

    try {
        CNcbiIfstream istr(fname.c_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *entry;
    }
    catch (const CException& e) {
        LOG_POST(Error << e.ReportAll());
        return CRef<CSeq_entry>();
    }
    return entry;
}

CRef<CSeq_annot> ReadAnnotFromFile(const string& fname)
{
    CRef<CSeq_annot> annot(new CSeq_annot);

    try {
        CNcbiIfstream istr(fname.c_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *annot;
    }
    catch (const CException& e) {
        LOG_POST(Error << e.ReportAll());
        return CRef<CSeq_annot>();
    }
    return annot;
}

BOOST_AUTO_TEST_CASE(Test_GB_3971)
{
    string entry_file = "PropagateFeatData/GB_3971.asn";
    CRef<CSeq_entry> entry = ReadEntryFromFile(entry_file);
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    CSeq_entry_Handle orig_seh = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(orig_seh);

    CBioseq_CI b_iter(orig_seh, CSeq_inst::eMol_na);
    CBioseq_Handle src_bsh = *b_iter;

    vector<CConstRef<CSeq_feat>> propagatedFeats;
    for (CFeat_CI ci(src_bsh); ci; ++ci) {
        propagatedFeats.push_back(ci->GetSeq_feat());
    }

    bool stopCdsAtStopCodon = false;
    bool CdsCleanupPartials = true;
    bool MergeAbuttingIntervals = false;
    bool ExtendOverGaps = true;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(orig_seh);
    CRef<CCmdComposite> composite(new CCmdComposite("Propagate features"));

    CAlign_CI align_iter(orig_seh);
    CMessageListener_Basic listener;
    for (CBioseq_CI bseq_it(orig_seh, CSeq_inst::eMol_na); bseq_it; ++bseq_it) {
        CBioseq_Handle target_bsh = *bseq_it;
        if (!target_bsh || target_bsh == src_bsh) {
            continue;
        }
        PropagateToTarget(
            composite,
            propagatedFeats,
            src_bsh,
            target_bsh,
            *align_iter,
            stopCdsAtStopCodon,
            CdsCleanupPartials,
            MergeAbuttingIntervals,
            ExtendOverGaps,
            create_general_only,
            &listener);
    }

    composite->Execute();

    CConstRef<CSeq_entry> updated_entry = orig_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> output = ReadEntryFromFile("PropagateFeatData/GB_3971_propagated.asn");
    BOOST_CHECK(output->Equals(*updated_entry));
}

BOOST_AUTO_TEST_CASE(Test_GB_3971_negative)
{
    string entry_file = "PropagateFeatData/GB_3971_negative.asn";
    CRef<CSeq_entry> entry = ReadEntryFromFile(entry_file);
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    CSeq_entry_Handle orig_seh = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(orig_seh);

    CBioseq_CI b_iter(orig_seh, CSeq_inst::eMol_na);
    CBioseq_Handle src_bsh = *b_iter;

    vector<CConstRef<CSeq_feat>> propagatedFeats;
    for (CFeat_CI ci(src_bsh); ci; ++ci) {
        propagatedFeats.push_back(ci->GetSeq_feat());
    }

    bool stopCdsAtStopCodon = false;
    bool CdsCleanupPartials = true;
    bool MergeAbuttingIntervals = false;
    bool ExtendOverGaps = true;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(orig_seh);
    CRef<CCmdComposite> composite(new CCmdComposite("Propagate features"));

    CAlign_CI align_iter(orig_seh);
    CMessageListener_Basic listener;
    for (CBioseq_CI bseq_it(orig_seh, CSeq_inst::eMol_na); bseq_it; ++bseq_it) {
        CBioseq_Handle target_bsh = *bseq_it;
        if (!target_bsh || target_bsh == src_bsh) {
            continue;
        }
        PropagateToTarget(
            composite,
            propagatedFeats,
            src_bsh,
            target_bsh,
            *align_iter,
            stopCdsAtStopCodon,
            CdsCleanupPartials,
            MergeAbuttingIntervals,
            ExtendOverGaps,
            create_general_only,
            &listener);
    }

    composite->Execute();

    CConstRef<CSeq_entry> updated_entry = orig_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> output = ReadEntryFromFile("PropagateFeatData/GB_3971_negative_propagated.asn");
    BOOST_CHECK(output->Equals(*updated_entry));
}

static CObject_id::TId s_FindHighestFeatId(const CSeq_entry_Handle seh)
{
    CObject_id::TId id = 0;
    for (CFeat_CI feat_it(seh); feat_it; ++feat_it) {
        if (feat_it->IsSetId()) {
            const CFeat_id& feat_id = feat_it->GetId();
            if (feat_id.IsLocal() && feat_id.GetLocal().IsId() && feat_id.GetLocal().GetId() > id) {
                id = feat_id.GetLocal().GetId();
            }
        }
    }
    return id;
}

BOOST_AUTO_TEST_CASE(Test_GB_7881)
{
    string entry_file = "PropagateFeatData/GB_7881.asn";
    CRef<CSeq_entry> entry = ReadEntryFromFile(entry_file);
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    CSeq_entry_Handle orig_seh = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(orig_seh);

    CBioseq_CI b_iter(orig_seh, CSeq_inst::eMol_na);
    CBioseq_Handle src_bsh = *b_iter;

    vector<CConstRef<CSeq_feat>> propagatedFeats;
    for (CFeat_CI ci(src_bsh); ci; ++ci) {
        propagatedFeats.push_back(ci->GetSeq_feat());
    }

    bool stopCdsAtStopCodon = false;
    bool CdsCleanupPartials = true;
    bool MergeAbuttingIntervals = false;
    bool ExtendOverGaps = true;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(orig_seh);
    CRef<CCmdComposite> composite(new CCmdComposite("Propagate features"));

    CRef<CSeq_annot> annot = ReadAnnotFromFile("PropagateFeatData/GB_7881_align.asn");
    BOOST_CHECK(annot);
    const CSeq_align& align = *(annot->GetData().GetAlign().front());

    CObject_id::TId maxFeatId = s_FindHighestFeatId(src_bsh.GetTopLevelEntry());

    CMessageListener_Basic listener;
    for (CBioseq_CI bseq_it(orig_seh, CSeq_inst::eMol_na); bseq_it; ++bseq_it) {
        CBioseq_Handle target_bsh = *bseq_it;
        if (!target_bsh || target_bsh == src_bsh) {
            continue;
        }
        PropagateToTarget(
            composite,
            propagatedFeats,
            src_bsh,
            target_bsh,
            align,
            stopCdsAtStopCodon,
            CdsCleanupPartials,
            MergeAbuttingIntervals,
            ExtendOverGaps,
            create_general_only,
            &listener,
            &maxFeatId);
    }

    composite->Execute();

    CConstRef<CSeq_entry> updated_entry = orig_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> output = ReadEntryFromFile("PropagateFeatData/GB_7881_propagated.asn");
    BOOST_CHECK(output->Equals(*updated_entry));
}
