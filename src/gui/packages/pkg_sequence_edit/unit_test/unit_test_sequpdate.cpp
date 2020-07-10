/*  $Id: unit_test_sequpdate.cpp 43963 2019-09-27 15:45:09Z asztalos $
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
* Author:  Andrea Asztalos
* File Description:
*   Unit tests for sequence update
*/

#include <ncbi_pch.hpp>
#include <serial/objistr.hpp>
#include <serial/typeinfo.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>

#include <objects/seq/Bioseq.hpp>
#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiapp.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/pub/pub_macros.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_vector.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <objtools/readers/fasta.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objtools/edit/cds_fix.hpp>
// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq.hpp>
#include <gui/packages/pkg_sequence_edit/update_multi_seq_input.hpp>
#include <objtools/unit_test_util/unit_test_util.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);

extern const char* sc_MainSequence;
extern const char* sc_UpdSequence_Same;
extern const char* sc_UpdSequence_Replace;
extern const char* sc_UpdSequence_Patch;
extern const char* sc_UpdSequence_Extend5;
extern const char* sc_UpdSequence_Extend5_AlignWithGap;
extern const char* sc_UpdSequence_Extend3;

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
    catch (const CException& e)  {
        LOG_POST(Error << e.ReportAll());
        return CRef<CSeq_entry>();
    }
    return entry;
}

CRef<CSeq_entry> ReadFastaSeqsFromFile(const string& fname)
{
    CFastaReader::TFlags flags = CFastaReader::fNoSplit
                                | CFastaReader::fUniqueIDs
                                | CFastaReader::fHyphensIgnoreAndWarn
                                | CFastaReader::fAddMods;

    CNcbiIfstream fstream(fname.c_str(), ios_base::in);
    CFastaReader fasta_reader(fstream, flags);

    CRef<CSeq_entry> entry(new CSeq_entry);
    try {
        CMessageListenerLenient msg_listener;
        entry = fasta_reader.ReadSet(kMax_Int, &msg_listener);
    }
    catch (const CException& e) {
        LOG_POST(Error << e.ReportAll());
        return CRef<CSeq_entry>();
    }
    return entry;
}

CRef<CSeq_entry> ReadEntry(const char* seq)
{
    CRef<CSeq_entry> entry(new CSeq_entry);
    CNcbiIstrstream istr(seq);
    istr >> MSerial_AsnText >> *entry;
    entry->Parentize();
    
    return entry;
}

CRef<CScope> BuildScope(void)
{
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    return scope;
}

void ReadFromFile(const string& old_seq, const string& upd_seq, CSeq_entry_Handle& old_seh, CSeq_entry_Handle& upd_seh)
{
    CRef<CSeq_entry> old_entry = ReadEntryFromFile(old_seq);
    CRef<CSeq_entry> upd_entry = ReadEntryFromFile(upd_seq);
    CRef<CScope> scope = BuildScope();

    old_seh = scope->AddTopLevelSeqEntry(*old_entry);
    upd_seh = scope->AddTopLevelSeqEntry(*upd_entry);
    BOOST_REQUIRE(old_seh);
    BOOST_REQUIRE(upd_seh);
}

void SetupForUpdate(const CSeq_entry_Handle& old_seh, const CSeq_entry_Handle& upd_seh, CUpdateSeq_Input& sequpd_in, bool identical_res = false)
{
    CBioseq_Handle old_bsh, upd_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter) old_bsh = *b_iter;
    CBioseq_CI b_it(upd_seh, CSeq_inst::eMol_na);
    if (b_it) upd_bsh = *b_it;

    BOOST_REQUIRE(old_bsh);
    BOOST_REQUIRE(upd_bsh);

    if (sequpd_in.SetOldBioseqAndScope(old_bsh)
        && sequpd_in.SetUpdateBioseq(upd_bsh)) {
        try {
            sequpd_in.CalculateAlignmentForUnitTest();
        }
        catch (const CException& e) {
            LOG_POST(Error << "Blasting the two sequences has failed: " << e.GetMsg());
        }
    }

    BOOST_REQUIRE(sequpd_in.IsReadyForUpdate());
    BOOST_CHECK_EQUAL(sequpd_in.HaveIdenticalResidues(), identical_res);
}

void ReadBioSeqs(const char* old_seq, const char* upd_seq, CSeq_entry_Handle& old_seh, CSeq_entry_Handle& upd_seh)
{
    CRef<CSeq_entry> old_entry = ReadEntry(old_seq);
    CRef<CSeq_entry> upd_entry = ReadEntry(upd_seq);
    CRef<CScope> scope = BuildScope();

    old_seh = scope->AddTopLevelSeqEntry(*old_entry);
    upd_seh = scope->AddTopLevelSeqEntry(*upd_entry);
    BOOST_REQUIRE(old_seh);
    BOOST_REQUIRE(upd_seh);
}

template<class T>
string MakeAsn(const T& object)
{
    CNcbiOstrstream str;
    str << MSerial_AsnText << object;
    return CNcbiOstrstreamToString(str);
}



BOOST_AUTO_TEST_CASE(Test_SameSequence)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Same, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input, true);
}

BOOST_AUTO_TEST_CASE(Test_NoChange)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Replace, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 985);
    //NcbiCout << "Passed Test_NoChange" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_DeleteAlignedFeatures)
{
    // remove existing features, do not import new ones
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // delete features within the aligned region
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveAligned;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_prot);
        BOOST_CHECK_EQUAL(type_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 2);
    //NcbiCout << "Passed Test_NoChange_DeleteAlignedFeatures" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_DeleteNotAlignedFeatures)
{
    // remove existing features, do not import new ones
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // delete features outside the aligned region
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNotAligned;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA);
        BOOST_CHECK_EQUAL(type_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 2);
    //NcbiCout << "Passed Test_NoChange_DeleteNotAlignedFeatures" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_DeleteAllFeatures)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // delete all existing features from the old sequence
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveAll;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        count++;
    }
    BOOST_CHECK_EQUAL(count, 0);
    //NcbiCout << "Passed Test_NoChange_DeleteAllFeatures" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportNewFeatures1)  // include duplicates
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import new (any kind of) features and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_any;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();
    
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
                    || (type == CSeqFeatData::eSubtype_gene)
                    || (type == CSeqFeatData::eSubtype_cdregion)
                    || (type == CSeqFeatData::eSubtype_mRNA);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;
        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 983) || (start == 568 && stop == 718);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_cdregion):
            feat_ok = (start == 249 && stop == 562) || (start == 587 && stop == 686);
            break;
        case (CSeqFeatData::eSubtype_prot):
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 25);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 7);
    //NcbiCout << "Passed Test_NoChange_ImportNewFeatures1" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportNewFeatures2)
{
    string old_fname("test_data/Pbseq.asn");
    string upd_fname("test_data/NC_014053.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    
    // import tRNAs and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_tRNA;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 14547);

    const CSeq_id* lcl_id = new_bsh.GetCompleteBioseq()->GetLocalId();
    BOOST_CHECK(lcl_id);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        BOOST_CHECK_EQUAL(type, CSeqFeatData::eSubtype_tRNA);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        
        const CSeq_id* feat_id = feat->GetLocation().GetId();
        BOOST_REQUIRE(feat_id);
        BOOST_CHECK(feat_id->Match(*lcl_id));

        const CRNA_ref::C_Ext& ext = feat->GetData().GetRna().GetExt();
        const CTrna_ext& trna_ext = ext.GetTRNA();
        // anticodon is set in all these tRNAs
        const TSeqPos start_ac = trna_ext.GetAnticodon().GetStart(eExtreme_Biological);
        const TSeqPos stop_ac = trna_ext.GetAnticodon().GetStop(eExtreme_Biological);
        
        const CSeq_id* anticodon_id = trna_ext.GetAnticodon().GetId();
        BOOST_REQUIRE(anticodon_id);
        BOOST_CHECK(anticodon_id->Match(*lcl_id));

        bool feat_ok(false);
        switch (count) {
        case (0):
            feat_ok = (start == 0 && stop == 68) && (start_ac == 32 && stop_ac == 34);
            break;
        case (1):
            feat_ok = (start == 69 && stop == 133) && (start_ac == 99 && stop_ac == 101);
            break;
        case (2):
            feat_ok = (start == 199 && stop == 131) && (start_ac == 169 && stop_ac == 167);
            break;
        case (3):
            feat_ok = (start == 1448 && stop == 1385) && (start_ac == 1418 && stop_ac == 1416);
            break;
        case (7):
            feat_ok = (start == 5911 && stop == 5977) && (start_ac == 5942 && stop_ac == 5944);
            break;
        case (11):
            feat_ok = (start == 8112 && stop == 8049) && (start_ac == 8082 && stop_ac == 8080);
            break;
        case (15):
            feat_ok = (start == 14087 && stop == 14024) && (start_ac == 14058 && stop_ac == 14056);
        default:
            feat_ok = true;
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 16);
    //NcbiCout << "Passed Test_NoChange_ImportNewFeatures2" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportNewFeatures_IncludeDupl)
{
    string old_fname("test_data/Pbseq_withtRNA.asn");
    string upd_fname("test_data/NC_014053.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import tRNAs, including duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_tRNA;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 14547);

    const CSeq_id* lcl_id = new_bsh.GetCompleteBioseq()->GetLocalId();
    BOOST_CHECK(lcl_id);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        BOOST_CHECK_EQUAL(type, CSeqFeatData::eSubtype_tRNA);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);

        const CSeq_id* feat_id = feat->GetLocation().GetId();
        BOOST_REQUIRE(feat_id);
        BOOST_CHECK(feat_id->Match(*lcl_id));

        const CRNA_ref::C_Ext& ext = feat->GetData().GetRna().GetExt();
        const CTrna_ext& trna_ext = ext.GetTRNA();
        // anticodon is set in all these tRNAs
        const TSeqPos start_ac = trna_ext.GetAnticodon().GetStart(eExtreme_Biological);
        const TSeqPos stop_ac = trna_ext.GetAnticodon().GetStop(eExtreme_Biological);

        const CSeq_id* anticodon_id = trna_ext.GetAnticodon().GetId();
        BOOST_REQUIRE(anticodon_id);
        BOOST_CHECK(anticodon_id->Match(*lcl_id));

        bool feat_ok(false);
        switch (count) {
        case (0) :
            feat_ok = (start == 0 && stop == 68) && (start_ac == 32 && stop_ac == 34);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (1) :
            feat_ok = (start == 69 && stop == 133) && (start_ac == 99 && stop_ac == 101);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (2) :
            feat_ok = (start == 6313 && stop == 6250) && (start_ac == 6283 && stop_ac == 6281);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (3) :
            feat_ok = (start == 14087 && stop == 14024) && (start_ac == 14058 && stop_ac == 14056);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (4) :
            feat_ok = (start == 0 && stop == 68) && (start_ac == 32 && stop_ac == 34);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (5) :
            feat_ok = (start == 69 && stop == 133) && (start_ac == 99 && stop_ac == 101);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (12) :
            feat_ok = (start == 5977 && stop == 6042) && (start_ac == 6006 && stop_ac == 6008);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (19) :
            feat_ok = (start == 14087 && stop == 14024) && (start_ac == 14058 && stop_ac == 14056);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        default:
            feat_ok = true;
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 20);
    //NcbiCout << "Passed Test_NoChange_ImportNewFeatures_IncludeDupl" << NcbiEndl;

}


BOOST_AUTO_TEST_CASE(Test_NoChange_ImportNewFeatures_ExceptDupl)
{
    string old_fname("test_data/Pbseq_withtRNA.asn");
    string upd_fname("test_data/NC_014053.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import tRNAs, except duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_tRNA;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllExceptDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 14547);

    const CSeq_id* lcl_id = new_bsh.GetCompleteBioseq()->GetLocalId();
    BOOST_CHECK(lcl_id);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        BOOST_CHECK_EQUAL(type, CSeqFeatData::eSubtype_tRNA);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);

        const CSeq_id* feat_id = feat->GetLocation().GetId();
        BOOST_REQUIRE(feat_id);
        BOOST_CHECK(feat_id->Match(*lcl_id));

        const CRNA_ref::C_Ext& ext = feat->GetData().GetRna().GetExt();
        const CTrna_ext& trna_ext = ext.GetTRNA();
        // anticodon is set in all these tRNAs
        const TSeqPos start_ac = trna_ext.GetAnticodon().GetStart(eExtreme_Biological);
        const TSeqPos stop_ac = trna_ext.GetAnticodon().GetStop(eExtreme_Biological);

        const CSeq_id* anticodon_id = trna_ext.GetAnticodon().GetId();
        BOOST_REQUIRE(anticodon_id);
        BOOST_CHECK(anticodon_id->Match(*lcl_id));

        bool feat_ok(false);
        switch (count) {
        case (0) :
            feat_ok = (start == 0 && stop == 68) && (start_ac == 32 && stop_ac == 34);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (1) :
            feat_ok = (start == 69 && stop == 133) && (start_ac == 99 && stop_ac == 101);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (2) :
            feat_ok = (start == 6313 && stop == 6250) && (start_ac == 6283 && stop_ac == 6281);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (3) :
            feat_ok = (start == 14087 && stop == 14024) && (start_ac == 14058 && stop_ac == 14056);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (4) :
            feat_ok = (start == 199 && stop == 131) && (start_ac == 169 && stop_ac == 167);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (12) :
            feat_ok = (start == 8112 && stop == 8049) && (start_ac == 8082 && stop_ac == 8080);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (15) :
            feat_ok = (start == 12670 && stop == 12602) && (start_ac == 12640 && stop_ac == 12638);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        default:
            feat_ok = true;
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 16);
    //NcbiCout << "Passed Test_NoChange_ImportNewFeatures_ExceptDupl" << NcbiEndl;
}


BOOST_AUTO_TEST_CASE(Test_NoChange_ImportNewFeatures_ReplaceDupl)
{
    string old_fname("test_data/Pbseq_withtRNA.asn");
    string upd_fname("test_data/NC_014053.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import tRNAs, replace duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_tRNA;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllReplaceDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 14547);

    const CSeq_id* lcl_id = new_bsh.GetCompleteBioseq()->GetLocalId();
    BOOST_CHECK(lcl_id);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        BOOST_CHECK_EQUAL(type, CSeqFeatData::eSubtype_tRNA);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);

        const CSeq_id* feat_id = feat->GetLocation().GetId();
        BOOST_REQUIRE(feat_id);
        BOOST_CHECK(feat_id->Match(*lcl_id));

        const CRNA_ref::C_Ext& ext = feat->GetData().GetRna().GetExt();
        const CTrna_ext& trna_ext = ext.GetTRNA();
        // anticodon is set in all these tRNAs
        const TSeqPos start_ac = trna_ext.GetAnticodon().GetStart(eExtreme_Biological);
        const TSeqPos stop_ac = trna_ext.GetAnticodon().GetStop(eExtreme_Biological);

        const CSeq_id* anticodon_id = trna_ext.GetAnticodon().GetId();
        BOOST_REQUIRE(anticodon_id);
        BOOST_CHECK(anticodon_id->Match(*lcl_id));

        bool feat_ok(false);
        switch (count) {
        case (0) :
            feat_ok = (start == 0 && stop == 68) && (start_ac == 32 && stop_ac == 34);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (1) :
            feat_ok = (start == 69 && stop == 133) && (start_ac == 99 && stop_ac == 101);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (2) :
            feat_ok = (start == 199 && stop == 131) && (start_ac == 169 && stop_ac == 167);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (10) :
            feat_ok = (start == 6313 && stop == 6250) && (start_ac == 6283 && stop_ac == 6281);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (15) :
            feat_ok = (start == 14087 && stop == 14024) && (start_ac == 14058 && stop_ac == 14056);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        default:
            feat_ok = true;
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 16);
    //NcbiCout << "Passed Test_NoChange_ImportNewFeatures_ReplaceDupl" << NcbiEndl;
}


BOOST_AUTO_TEST_CASE(Test_NoChange_ImportNewFeatures_MergeDupl1)
{
    string old_fname("test_data/Pbseq_withtRNA.asn");
    string upd_fname("test_data/NC_014053.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import tRNAs, merge duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_tRNA;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllMergeDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 14547);

    const CSeq_id* lcl_id = new_bsh.GetCompleteBioseq()->GetLocalId();
    BOOST_CHECK(lcl_id);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        BOOST_CHECK_EQUAL(type, CSeqFeatData::eSubtype_tRNA);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);

        const CSeq_id* feat_id = feat->GetLocation().GetId();
        BOOST_REQUIRE(feat_id);
        BOOST_CHECK(feat_id->Match(*lcl_id));

        const CRNA_ref::C_Ext& ext = feat->GetData().GetRna().GetExt();
        const CTrna_ext& trna_ext = ext.GetTRNA();
        // anticodon is set in all these tRNAs
        const TSeqPos start_ac = trna_ext.GetAnticodon().GetStart(eExtreme_Biological);
        const TSeqPos stop_ac = trna_ext.GetAnticodon().GetStop(eExtreme_Biological);

        const CSeq_id* anticodon_id = trna_ext.GetAnticodon().GetId();
        BOOST_REQUIRE(anticodon_id);
        BOOST_CHECK(anticodon_id->Match(*lcl_id));

        bool feat_ok(false);
        switch (count) {
        case (0) :
            feat_ok = (start == 0 && stop == 68) && (start_ac == 32 && stop_ac == 34);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (1) :
            feat_ok = (start == 69 && stop == 133) && (start_ac == 99 && stop_ac == 101);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (2) :
            feat_ok = (start == 199 && stop == 131) && (start_ac == 169 && stop_ac == 167);
            BOOST_CHECK(!feat->IsSetComment());
            break;
        case (10) :
            feat_ok = (start == 6313 && stop == 6250) && (start_ac == 6283 && stop_ac == 6281);
            BOOST_CHECK(feat->IsSetComment());
            break;
        case (15) :
            feat_ok = (start == 14087 && stop == 14024) && (start_ac == 14058 && stop_ac == 14056);
            BOOST_CHECK(feat->IsSetComment());
            BOOST_CHECK(feat->IsSetPartial());
            break;
        default:
            feat_ok = true;
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 16);
    //NcbiCout << "Passed Test_NoChange_ImportNewFeatures_MergeDupl1" << NcbiEndl;
}


BOOST_AUTO_TEST_CASE(Test_NoChange_ImportNewFeatures_MergeDupl2)
{
    string old_fname("test_data/Pbseq_withGenes.asn");
    string upd_fname("test_data/NC_014053_genes.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import genes, merge duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_gene;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllMergeDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/Pbseq_withGenes_updated.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportNewFeatures_MergeDupl2" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_NewId1)
{
    string old_fname("test_data/Pbseq_withGenes.asn");
    string upd_fname("test_data/NC_014053.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import all coding regions and assign new IDs to the products
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/Pbseq_importCDS_newId.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportCDSFeatures_NewId1" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_KeepId1)
{
    string old_fname("test_data/Pbseq_withGenes.asn");
    string upd_fname("test_data/NC_014053.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import coding regions and keep the original protein IDs
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;
    params.m_KeepProteinId = true;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/Pbseq_importCDS_keepId.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportCDSFeatures_KeepId1" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_NewId2)
{
    string old_fname("test_data/duplicateCDS.asn");
    string upd_fname("test_data/NC_024274_cds.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import coding regions and do not keep original protein IDs
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/duplicateCDS_newId_updated.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportCDSFeatures_NewId2" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_KeepId2)
{
    string old_fname("test_data/duplicateCDS.asn");
    string upd_fname("test_data/NC_024274_cds.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import coding regions and keep protein IDs
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;
    params.m_KeepProteinId = true;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/duplicateCDS_keepId_updated.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportCDSFeatures_KeepId2" << NcbiEndl;
}


BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_Merge)
{
    string old_fname("test_data/duplicateCDS.asn");
    string upd_fname("test_data/NC_024274_cds.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import coding regions, merge duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllMergeDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/duplicateCDS_merged.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportCDSFeatures_Merge" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_ExceptDuplicates)
{
    string old_fname("test_data/duplicateCDS.asn");
    string upd_fname("test_data/NC_024274_cds.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import coding regiosn except duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllExceptDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/duplicateCDS_exceptdupl.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportCDSFeatures_ExceptDuplicates" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_ReplaceDuplicates)
{
    string old_fname("test_data/duplicateCDS.asn");
    string upd_fname("test_data/NC_024274_cds.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import coding regions, replace duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllReplaceDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/duplicateCDS_replacedupl.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
    //NcbiCout << "Passed Test_NoChange_ImportCDSFeatures_ReplaceDuplicates" << NcbiEndl;
}


// When old protein ids are the same as imported protein ids
/* currently fails, the old proteins are 'shadowing' the imported ones
// Resolution: annotations should be placed in different scopes
BOOST_AUTO_TEST_CASE(Test_NoChange_ImportCDSFeatures_NewId2)  // GB-5732
{
    string old_fname("test_data/proteinacc.asn");
    string upd_fname("test_data/NC_024274_cds.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // import all features, merge duplicates and do not remove any existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_cdregion;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CRef<CSeq_entry> output = ReadEntryFromFile("test_data/proteinacc_CDS_newId_updated.asn");

    CConstRef<CSeq_entry> updated_entry = old_seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
}
*/

BOOST_AUTO_TEST_CASE(Test_NoChange_Delete_ImportAllFeatures)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // remove all existing features, and import all features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveAll;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_any;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;
        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            // if the first residue of the patch is different, start = 569 ( in Sequin it is 568)
            feat_ok = (start == 568 && stop == 718);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 587 && stop == 686);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 25);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 3);
    //NcbiCout << "Passed Test_NoChange_Delete_ImportAllFeatures" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportFeats_ShortSeq)
{
    string old_fname("test_data/shortSeq.asn");
    string upd_fname("test_data/update_shortSeq.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_any;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    short count = 0;
    // none of features from the update sequence get imported
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;
        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 127);
            break;
        case (CSeqFeatData::eSubtype_misc_feature):
            feat_ok = (start == 0 && stop == 29);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 29 && stop == 127);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 49);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
    //NcbiCout << "Passed Test_NoChange_ImportFeats_ShortSeq" << NcbiEndl;
}


BOOST_AUTO_TEST_CASE(Test_ReplaceSequence)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    // replace the sequence, and do not import any new features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();

    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), upd_bsh.GetBioseqLength());
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    // new sequence should be equal to the update sequence
    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);
    string updseq;
    CSeqVector upd_svec = upd_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    upd_svec.GetSeqData(0, upd_bsh.GetBioseqLength(), updseq);
    BOOST_CHECK_EQUAL(updseq, newseq);

    // check whether the existing features on the old sequence have been properly adjusted
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 131);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 0 && stop == 31);
            break;
        case (CSeqFeatData::eSubtype_cdregion):
            feat_ok = (start == 0 && stop == 0);
            break;
        case (CSeqFeatData::eSubtype_prot):
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
    //NcbiCout << "Passed Test_ReplaceSequence" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_WithoutFeats)
{
    string old_fname("test_data/OneSeqWithoutFeature.asn");
    string upd_fname("test_data/update_OneSeq.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 535);
    vector<string> check{
        "Seq-id ::= local str \"Seq1\"\n",
        "Seq-id ::= general {\n"
        "  db \"DFG\",\n"
        "  tag str \"3456/Seq1\"\n"
        "}\n",
        "Seq-id ::= general {\n"
        "  db \"test\",\n"
        "  tag id 2356\n"
        "}\n",
        "Seq-id ::= genbank {\n"
        "  accession \"AA123456\"\n"
        "}\n"
    };

    auto i = 0;
    ITERATE(CBioseq::TId, it, new_bsh.GetCompleteBioseq()->GetId()) {
        BOOST_CHECK_EQUAL(MakeAsn(**it), check[i++]);
    }
}


BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_FeatAtSeqEnd)
{
    string old_fname("test_data/KR002586.asn");
    string upd_fname("test_data/KR002586_update.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    // replace the sequence, and the original CDS should be at the end of sequence
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1080);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
                       || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 0 && stop == 1079);
            break;
        case (CSeqFeatData::eSubtype_prot):
            feat_ok = (start == 0 && stop == 394);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 2);
    //NcbiCout << "Passed Test_ReplaceSequence_FeatAtSeqEnd" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSeqWithinSet_FeatsAtSeqEnd1_AddCitSub)
{
    string old_fname("test_data/seq_withinSet.asn");
    string upd_fname("test_data/update_seqWithinSet1.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CSeq_entry_Handle oldSeh;
    for (CBioseq_CI b_iter(old_seh); b_iter; ++b_iter) {
        if (b_iter->GetBioseqLength() == 1430) {
            oldSeh = b_iter->GetSeq_entry_Handle();
            break;
        }
    }

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(oldSeh, upd_seh, sequpd_input);

    // replace the middle sequence
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;
    params.m_AddCitSub = true;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI bseq_it(old_seh, CSeq_inst::eMol_na);
    if (bseq_it && ++bseq_it) {
        new_bsh = *bseq_it;
    }

    CSeq_entry_Handle newSeh;
    CBioseq_set_Handle newSetSeh = new_bsh.GetParentBioseq_set();
    if (newSetSeh &&
        newSetSeh.IsSetClass() &&
        newSetSeh.GetClass() == CBioseq_set::eClass_nuc_prot) {
        newSeh = newSetSeh.GetParentEntry();
    }
    else {
        newSeh = new_bsh.GetSeq_entry_Handle();
    }

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1387);
    
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *newSeh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_rRNA)
                    || (type == CSeqFeatData::eSubtype_tRNA)
                    || (type == CSeqFeatData::eSubtype_mRNA)
                    || (type == CSeqFeatData::eSubtype_misc_feature)
                    || (type == CSeqFeatData::eSubtype_cdregion)
                    || (type == CSeqFeatData::eSubtype_prot)
                    || (type == CSeqFeatData::eSubtype_3UTR)
                    || (type == CSeqFeatData::eSubtype_5UTR)
                    || (type == CSeqFeatData::eSubtype_preRNA);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_rRNA) :
            feat_ok = ((start == 0 && stop == 1378) && feat->IsSetPartial()) ||
                        ((start == 0 && stop == 1386) && !feat->IsSetPartial()); // imported feature
            break;
        case (CSeqFeatData::eSubtype_tRNA):
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= mix {\n"
                        "  int {\n"
                        "    from 55,\n"
                        "    to 171,\n"
                        "    strand minus,\n"
                        "    id local str \"Seq23\"\n"
                        "  },\n"
                        "  int {\n"
                        "    from 0,\n"
                        "    to 15,\n"
                        "    strand minus,\n"
                        "    id local str \"Seq23\"\n"
                        "  }\n"
                        "}\n");
            feat_ok = true;
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = !feat->IsSetPartial() && ((start == 0 && stop == 94) 
                                            || (start == 271 && stop == 1148)
                                            || (start == 647 && stop == 1378));
            break;
        case (CSeqFeatData::eSubtype_mRNA):
            feat_ok = feat->IsSetPartial() 
                      && feat->GetLocation().IsPartialStop(eExtreme_Biological) 
                      && (start == 745 && stop == 171);
            break;
        case (CSeqFeatData::eSubtype_cdregion):
            feat_ok = !feat->IsSetPartial() && (start == 0 && stop == 94);
            break;
        case (CSeqFeatData::eSubtype_prot):
            feat_ok = !feat->IsSetPartial() && (start == 0 && stop == 31);
            break;
        case (CSeqFeatData::eSubtype_3UTR):
            feat_ok = feat->IsSetPartial()
                      && feat->GetLocation().IsPartialStart(eExtreme_Biological)
                      && (start == 1378 && stop == 1358);
            break;
        case (CSeqFeatData::eSubtype_5UTR):
            feat_ok = feat->IsSetPartial()
                      && feat->GetLocation().IsPartialStop(eExtreme_Biological)
                      && (start == 1368 && stop == 1374);
            break;
        case (CSeqFeatData::eSubtype_preRNA):
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= mix {\n"
                        "  int {\n"
                        "    from 20,\n"
                        "    to 35,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  },\n"
                        "  int {\n"
                        "    from 1308,\n"
                        "    to 1328,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  }\n"
                        "}\n");
            feat_ok = true;
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 12);

    // check added cit-sub that should appear on the sequence and not on the set
    CRef<CDate> today(new CDate);
    today->SetToTime(CurrentTime(), CDate::ePrecision_day);
    FOR_EACH_SEQDESC_ON_BIOSEQ(desc_it, *new_bsh.GetCompleteBioseq()) {
        if ((*desc_it)->IsPub()) {
            FOR_EACH_PUB_ON_PUBDESC(pub_it, (*desc_it)->GetPub()) {
                if ((*pub_it)->IsSub()) {
                    const CCit_sub& sub = (*pub_it)->GetSub();
                    BOOST_CHECK(sub.IsSetDate());
                    BOOST_CHECK_EQUAL(MakeAsn(sub.GetDate()), MakeAsn(*today));
                    if (sub.IsSetDescr()) {
                        BOOST_CHECK_EQUAL(sub.GetDescr(), string("Sequence update by submitter"));
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSeqWithinSet_FeatsAtSeqEnd2)
{
    string old_fname("test_data/seq_withinSet.asn");
    string upd_fname("test_data/update_seqWithinSet2.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CSeq_entry_Handle oldSeh;
    for (CBioseq_CI b_iter(old_seh); b_iter; ++b_iter) {
        if (b_iter->GetBioseqLength() == 1430) {
            oldSeh = b_iter->GetSeq_entry_Handle();
            break;
        }
    }

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(oldSeh, upd_seh, sequpd_input);

    // replace the middle sequence
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI bseq_it(old_seh, CSeq_inst::eMol_na);
    if (bseq_it && ++bseq_it) {
        new_bsh = *bseq_it;
    }

    CSeq_entry_Handle newSeh;
    CBioseq_set_Handle newSetSeh = new_bsh.GetParentBioseq_set();
    if (newSetSeh &&
        newSetSeh.IsSetClass() &&
        newSetSeh.GetClass() == CBioseq_set::eClass_nuc_prot) {
        newSeh = newSetSeh.GetParentEntry();
    }
    else {
        newSeh = new_bsh.GetSeq_entry_Handle();
    }

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1563);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *newSeh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_rRNA)
            || (type == CSeqFeatData::eSubtype_tRNA)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_misc_feature)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_3UTR)
            || (type == CSeqFeatData::eSubtype_5UTR)
            || (type == CSeqFeatData::eSubtype_region)
            || (type == CSeqFeatData::eSubtype_preRNA);

        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_rRNA) :
            feat_ok = ((start == 0 && stop == 1562) && feat->IsSetPartial()) || // imported feature
                        ((start == 133 && stop == 1562) && feat->IsSetPartial());
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = !feat->IsSetPartial() && ((start == 133 && stop == 232)
                                            || (start == 432 && stop == 1332)
                                            || (start == 832 && stop == 1562));
            break;
        case (CSeqFeatData::eSubtype_region):
            feat_ok = !feat->IsSetPartial() && start == 0 && stop == 132; // imported feature
            break;
        case (CSeqFeatData::eSubtype_tRNA) :
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                            "Seq-loc ::= mix {\n"
                            "  int {\n"
                            "    from 192,\n"
                            "    to 332,\n"
                            "    strand minus,\n"
                            "    id local str \"Seq23\"\n"
                            "  },\n"
                            "  int {\n"
                            "    from 133,\n"
                            "    to 152,\n"
                            "    strand minus,\n"
                            "    id local str \"Seq23\"\n"
                            "  }\n"
                            "}\n");
            feat_ok = true;  // ok
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = feat->IsSetPartial()
                      && feat->GetLocation().IsPartialStop(eExtreme_Biological)
                      && (start == 932 && stop == 332);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = !feat->IsSetPartial() && (start == 133 && stop == 232);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = !feat->IsSetPartial() && (start == 0 && stop == 31);
            break;
        case (CSeqFeatData::eSubtype_3UTR) :
            feat_ok = feat->IsSetPartial()
                      && feat->GetLocation().IsPartialStart(eExtreme_Biological)
                      && (start == 1562 && stop == 1542);
            break;
        case (CSeqFeatData::eSubtype_5UTR) :
            feat_ok = feat->IsSetPartial()
                      && feat->GetLocation().IsPartialStop(eExtreme_Biological)
                      && (start == 1552 && stop == 1558);
            break;
        case (CSeqFeatData::eSubtype_preRNA) :
                        BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= mix {\n"
                        "  int {\n"
                        "    from 157,\n"
                        "    to 172,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  },\n"
                        "  int {\n"
                        "    from 1492,\n"
                        "    to 1512,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  }\n"
                        "}\n");
            feat_ok = true;
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 13);
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_UpdateProteins_KeepProteinId)
{
    string old_fname("test_data/retranslateCDS_1.asn");
    string upd_fname("test_data/replace_retranslateCDS_1.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    // do not remove old features and do not import new features
    // retranslate the coding regions and update the proteins
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace, true);
    params.m_UpdateProteins |= SUpdateSeqParams::eProtUpdate;
    params.m_KeepProteinId = true;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 718);

    CBioseq_Handle psh;
    CBioseq_CI p_iter(old_seh, CSeq_inst::eMol_aa);
    if (p_iter) psh = *p_iter;

    BOOST_REQUIRE(psh);
    BOOST_CHECK_EQUAL(psh.GetBioseqLength(), 239);

    const CBioseq& protein = *psh.GetCompleteBioseq();
    string id_label;
    protein.GetFirstId()->GetLabel(&id_label);
    BOOST_CHECK_EQUAL(id_label, "lcl|12HNVN567_prot_72");
    
    const CBioseq::TDescr& descrs = protein.GetDescr();
    BOOST_CHECK(descrs.Get().size() == 3);

    string newpr;
    CSeqVector new_svec = psh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, psh.GetBioseqLength(), newpr);

    string prseq("CDVKVXXXKFQWDRTMXKCQLSTMHTWNXASGINSIAAKWQSCRRRDNDQIXKSHRQCQN\
HNSATXXFCRNQLYQTIRXXKNKXDSRTRTSILYNRRNXRKYKESILXXXXNKMXXSFXTGSWKTKRALXXXXYNLSTTL\
RRRSRNXNTSFXLXRGIFLLQYNTTVXXYCLHKQYKRGRVXWHYHTSMPDKTNYKHVAKSRKSNVCSSHQ\
WKHXVXIKYYRNTIDKRWWYXXLXXXYLR");
    BOOST_CHECK_EQUAL(newpr, prseq);

    CFeat_CI prot_feat(psh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
    BOOST_REQUIRE(prot_feat);
    
    bool feat_ok = (prot_feat->GetLocation().GetStart(eExtreme_Positional) == 0) &&
                    (prot_feat->GetLocation().GetStop(eExtreme_Positional) == 238);
    BOOST_CHECK(feat_ok);
    //NcbiCout << "Passed Test_ReplaceSequence_UpdateProteins_KeepProteinId" << NcbiEndl;
}


BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_UpdateProteins_DoNot_KeepProteinId)
{
    string old_fname("test_data/retranslateCDS_1.asn");
    string upd_fname("test_data/replace_retranslateCDS_1.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    // do not remove old features and do not import new features
    // retranslate the coding regions and update the proteins
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace, true);
    params.m_UpdateProteins |= SUpdateSeqParams::eProtUpdate;
    params.m_KeepProteinId = false;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 718);

    CBioseq_Handle psh;
    CBioseq_CI p_iter(old_seh, CSeq_inst::eMol_aa);
    if (p_iter) psh = *p_iter;

    BOOST_REQUIRE(psh);
    BOOST_CHECK_EQUAL(psh.GetBioseqLength(), 239);

    // the existing protein ID should never be removed
    const CBioseq& protein = *psh.GetCompleteBioseq();
    string id_label;
    protein.GetFirstId()->GetLabel(&id_label);
    BOOST_CHECK_EQUAL(id_label, "lcl|12HNVN567_prot_72");
    const CBioseq::TDescr& descrs = protein.GetDescr();
    BOOST_CHECK(descrs.Get().size() == 3);

    string newpr;
    CSeqVector new_svec = psh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, psh.GetBioseqLength(), newpr);

    string prseq("CDVKVXXXKFQWDRTMXKCQLSTMHTWNXASGINSIAAKWQSCRRRDNDQIXKSHRQCQN\
HNSATXXFCRNQLYQTIRXXKNKXDSRTRTSILYNRRNXRKYKESILXXXXNKMXXSFXTGSWKTKRALXXXXYNLSTTL\
RRRSRNXNTSFXLXRGIFLLQYNTTVXXYCLHKQYKRGRVXWHYHTSMPDKTNYKHVAKSRKSNVCSSHQ\
WKHXVXIKYYRNTIDKRWWYXXLXXXYLR");
    BOOST_CHECK_EQUAL(newpr, prseq);

    CFeat_CI prot_feat(psh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
    BOOST_REQUIRE(prot_feat);

    bool feat_ok = (prot_feat->GetLocation().GetStart(eExtreme_Positional) == 0) &&
                    (prot_feat->GetLocation().GetStop(eExtreme_Positional) == 238);
    BOOST_CHECK(feat_ok);
}

BOOST_AUTO_TEST_CASE(Test_PatchSeqWithinSet_FeatsAtSeqEnd)
{
    string old_fname("test_data/seq_withinSet.asn");
    string upd_fname("test_data/update_seqWithinSet1.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CSeq_entry_Handle oldSeh;
    for (CBioseq_CI b_iter(old_seh); b_iter; ++b_iter) {
        if (b_iter->GetBioseqLength() == 1430) {
            oldSeh = b_iter->GetSeq_entry_Handle();
            break;
        }
    }

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(oldSeh, upd_seh, sequpd_input);

    // patch the middle sequence, do not import new features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdatePatch);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI bseq_it(old_seh, CSeq_inst::eMol_na);
    if (bseq_it && ++bseq_it) {
        new_bsh = *bseq_it;
    }

    CSeq_entry_Handle newSeh;
    CBioseq_set_Handle newSetSeh = new_bsh.GetParentBioseq_set();
    if (newSetSeh &&
        newSetSeh.IsSetClass() &&
        newSetSeh.GetClass() == CBioseq_set::eClass_nuc_prot) {
        newSeh = newSetSeh.GetParentEntry();
    }
    else {
        newSeh = new_bsh.GetSeq_entry_Handle();
    }

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1360);
    
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *newSeh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_rRNA)
            || (type == CSeqFeatData::eSubtype_tRNA)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_misc_feature)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_3UTR)
            || (type == CSeqFeatData::eSubtype_5UTR)
            || (type == CSeqFeatData::eSubtype_preRNA);

        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_rRNA) :
            feat_ok = ((start == 0 && stop == 1359) && feat->IsSetPartial());
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = !feat->IsSetPartial() && ((start == 0 && stop == 98)
                                            || (start == 275 && stop == 1152)
                                            || (start == 651 && stop == 1359));
            break;
        case (CSeqFeatData::eSubtype_tRNA) :
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= mix {\n"
                        "  int {\n"
                        "    from 59,\n"
                        "    to 175,\n"
                        "    strand minus,\n"
                        "    id local str \"Seq23\"\n"
                        "  },\n"
                        "  int {\n"
                        "    from 0,\n"
                        "    to 19,\n"
                        "    strand minus,\n"
                        "    id local str \"Seq23\"\n"
                        "  }\n"
                        "}\n");
            feat_ok = true;
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = feat->IsSetPartial()
                    && feat->GetLocation().IsPartialStop(eExtreme_Biological)
                    && (start == 749 && stop == 175);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = !feat->IsSetPartial() && (start == 0 && stop == 98);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = !feat->IsSetPartial() && (start == 0 && stop == 31);
            break;
        case (CSeqFeatData::eSubtype_3UTR) :
            feat_ok = feat->IsSetPartial()
                    && feat->GetLocation().IsPartialStart(eExtreme_Biological)
                    && (start == 1359 && stop == 1359);
            break;
        case (CSeqFeatData::eSubtype_5UTR) :
            feat_ok = feat->IsSetPartial()
                    && feat->GetLocation().IsPartialStop(eExtreme_Biological)
                    && (start == 1359 && stop == 1359);
            break;
        case (CSeqFeatData::eSubtype_preRNA) :
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= mix {\n"
                        "  int {\n"
                        "    from 24,\n"
                        "    to 39,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  },\n"
                        "  int {\n"
                        "    from 1312,\n"
                        "    to 1332,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  }\n"
                        "}\n");
            feat_ok = true;
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 11);
}

BOOST_AUTO_TEST_CASE(Test_PatchSeqWithinSet_ImportFeatures)
{
    string old_fname("test_data/seq_withinSet.asn");
    string upd_fname("test_data/update_seqWithinSet_Feats.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CSeq_entry_Handle oldSeh;
    for (CBioseq_CI b_iter(old_seh); b_iter; ++b_iter) {
        if (b_iter->GetBioseqLength() == 1430) {
            oldSeh = b_iter->GetSeq_entry_Handle();
            break;
        }
    }

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(oldSeh, upd_seh, sequpd_input);

    // patch the middle sequence, remove all existing features and import new features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdatePatch);
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveAll;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI bseq_it(old_seh, CSeq_inst::eMol_na);
    if (bseq_it && ++bseq_it) {
        new_bsh = *bseq_it;
    }

    CSeq_entry_Handle newSeh;
    CBioseq_set_Handle newSetSeh = new_bsh.GetParentBioseq_set();
    if (newSetSeh &&
        newSetSeh.IsSetClass() &&
        newSetSeh.GetClass() == CBioseq_set::eClass_nuc_prot) {
        newSeh = newSetSeh.GetParentEntry();
    }
    else {
        newSeh = new_bsh.GetSeq_entry_Handle();
    }

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1360);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *newSeh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_rRNA)
            || (type == CSeqFeatData::eSubtype_tRNA)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_rRNA) :
            feat_ok = (start == 4 && stop == 1359);
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = (!feat->IsSetPartial() && (start == 803 && stop == 303)) 
                    || (feat->IsSetPartial() && (start == 1313 && stop == 1359))
                    || (!feat->IsSetPartial() && (start == 1359 && stop == 1343));
            break;
        case (CSeqFeatData::eSubtype_tRNA) :
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= packed-int {\n"
                        "  {\n"
                        "    from 4,\n"
                        "    to 29,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  },\n"
                        "  {\n"
                        "    from 33,\n"
                        "    to 83,\n"
                        "    strand plus,\n"
                        "    id local str \"Seq23\"\n"
                        "  }\n"
                        "}\n");
            feat_ok = true;
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 5);
    //NcbiCout << "Passed Test_PatchSeqWithinSet_ImportFeatures" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_ImportFeatures)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    
    // import all new features and do not remove existing features
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();

    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), upd_bsh.GetBioseqLength());

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 131);
            break;
        case (CSeqFeatData::eSubtype_mRNA):
            feat_ok = (start == 0 && stop == 31);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 0 && stop == 0) || (start == 19 && stop == 99);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 25);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 7);

}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_ProteinId_5_Partial_CitSub1)
{
    string old_fname("test_data/Old_Bourbon.asn");
    string upd_fname("test_data/Update_Bourbon.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    params.m_AddCitSub = true;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();

    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), upd_bsh.GetBioseqLength());

    CFeat_CI cds_it(new_bsh, SAnnotSelector(CSeqFeatData::e_Cdregion));
    BOOST_CHECK(cds_it);
    CConstRef<CSeq_feat> cds = cds_it->GetOriginalSeq_feat();

    // check cds location and 5' completeness
    const CSeq_loc& loc = cds->GetLocation();
    BOOST_CHECK(!loc.IsPartialStart(eExtreme_Biological));
    BOOST_CHECK(loc.IsPartialStop(eExtreme_Biological));

    BOOST_CHECK_EQUAL(loc.GetStart(eExtreme_Biological), (TSeqPos)33);
    BOOST_CHECK_EQUAL(loc.GetStop(eExtreme_Biological), (TSeqPos)1936);

    // check protein ids
    {{
        CBioseq_Handle protein = old_seh.GetScope().GetBioseqHandle(cds->GetProduct());
        const CBioseq::TId& prot_ids = protein.GetCompleteBioseq()->GetId();
        BOOST_REQUIRE(prot_ids.size() == 2);

        vector<string> check{
            "Seq-id ::= genbank {\n"
            "  accession \"AJP32538\",\n"
            "  version 1\n"
            "}\n",
            "Seq-id ::= gi 761229279\n" 
        };

        auto i = 0;
        ITERATE(CBioseq::TId, it, prot_ids) {
            BOOST_CHECK_EQUAL(MakeAsn(**it), check[i++]);
        }
        }}

    // check the newly added cit-sub with current date
    {{
        CRef<CDate> today(new CDate);
        today->SetToTime(CurrentTime(), CDate::ePrecision_day);
        vector<string> check{
            "Date ::= std {\n"
            "  year 2015,\n"
            "  month 1,\n"
            "  day 14\n"
            "}\n",
            MakeAsn(*today)
        };

        auto i = 0;
        for (CSeqdesc_CI desc_it(new_bsh, CSeqdesc::e_Pub); desc_it; ++desc_it) {
            const CPubdesc& pubdesc = desc_it->GetPub();
            FOR_EACH_PUB_ON_PUBDESC(pub_it, pubdesc) {
                if ((*pub_it)->IsSub()) {
                    const CCit_sub& sub = (*pub_it)->GetSub();
                    BOOST_CHECK(sub.IsSetDate());
                    BOOST_CHECK_EQUAL(MakeAsn(sub.GetDate()), check[i++]);
                    if (sub.IsSetDescr()) {
                        BOOST_CHECK_EQUAL(sub.GetDescr(), string("Sequence update by submitter"));
                    }
                }
            }
        }
    }}
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_CitSub2)
{
    string old_fname("test_data/Old_Bourbon_woSub.asn");
    string upd_fname("test_data/Update_Bourbon.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    params.m_AddCitSub = true;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();

    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), upd_bsh.GetBioseqLength());
    BOOST_CHECK_EQUAL(updater.GetCitSubMessage(), string("There is no earlier Cit-sub template"));
    //NcbiCout << "Passed Test_ReplaceSequence_CitSub2" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_WithoutAlign)
{
    string old_fname("test_data/replace_woalign.asn");
    string upd_fname("test_data/replace_woalign_update.asn");
    CRef<CSeq_entry> old_entry = ReadEntryFromFile(old_fname);
    CRef<CSeq_entry> upd_entry = ReadEntryFromFile(upd_fname);
    CRef<CScope> scope = BuildScope();

    CSeq_entry_Handle old_seh = scope->AddTopLevelSeqEntry(*old_entry);
    BOOST_REQUIRE(old_seh);

    CUpdateMultipleSeq_Input multiupdseq_in;
    bool ok = multiupdseq_in.SetOldEntryAndScope(old_seh);
    ok = ok && multiupdseq_in.SetUpdateEntry(upd_entry);
    ok = ok && multiupdseq_in.PrepareMultipleSequenceInputsForUnitTest();
    BOOST_REQUIRE(ok);

    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& updates = multiupdseq_in.GetNonIdenticalUpdates();
    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& identicals = multiupdseq_in.GetIdenticalUpdates();
    const CUpdateMultipleSeq_Input::TSeqIDHVector& noUpdates = multiupdseq_in.GetSeqsWithoutUpdates();

    BOOST_CHECK(updates.size() == 3);
    BOOST_CHECK(identicals.empty());
    BOOST_CHECK(noUpdates.empty());

    CRef<CCmdComposite> update_cmd(new CCmdComposite("Update multiple sequences"));
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    
    for (auto& it : updates) {
        CSequenceUpdater updater(it.second.GetObject(), params);        
        CRef<CCmdComposite> cmd = updater.Update(create_general_only);
        update_cmd->AddCommand(*cmd);
    }

    update_cmd->Execute();

    short index = 0;
    for (CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        BOOST_CHECK(b_iter->GetBioseqLength() == 709);
        CFeat_CI feat(*b_iter);
        BOOST_REQUIRE(feat);
        index++;
        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Positional);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Positional);
        switch (index) {
        case 1:
            BOOST_CHECK(start == 0 && stop == 230);
            break;
        case 2:
            BOOST_CHECK(start == 0 && stop == 708);
            break;
        case 3:
            BOOST_CHECK(start == 0 && stop == 911);
            break;
        }
        
    }
}

BOOST_AUTO_TEST_CASE(Test_PatchSequence)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdatePatch, true);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 966);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);
    
    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);

    string patchseq("TGCAGTCGAGCGGCAGCACGGGGAGCTTGCTCCCTGGTGGCGAGCGGCGGACGGGTGAGTAATGTAGGAA\
TCTGCCCGGTAGTGGGGGATAACGTGGGGAAACCCACGCTAATACCGCATACGTCCTACGGGAGAAAGCG\
GAGGATCTTCGGACTTCGCGCTATCGGATGAGCCTATGTCGGATTAGCTAGTTGGTAAGGTAACGGCTTA\
CCAAGGCGACGATCCGTAGCTGGTCTGAGAGGATGATCAGCCACACTGGGACTGAGACACGGCCCAGACT\
CCTACGGGAGGCAGCAGTGGGGAATATTGGACAATGGGCGAAAGCCTTGATCCAGCCATGCCGCGTGTGT\
GAAGAAGGCTTTCGGGTTGTAAAGCACTTTCAGCGAGGAAGAAAGCCTGGTGGTTAAAGCACCGGCTAAC\
TCCGTGCCAGCAGCCGCGGTAATACGGAGGGTGCGAGCGTTAATCGGAATTACTGGGCGTAAAGCGCGCG\
TAGGTGGCTTGGCACGCCGGTTGTGAAAGCCCCGGGCTCAACCTGGGAACGGCATCCGGAACGGCCAGGC\
TAGAGTGCAGGAGAGGAAGGTAGAATTCCCGGTGTAGCGGTGAAATGCGTAGAGATCGGGCCCCCCCCCC\
CCCCCCCCCCTGACACTGAGGTGCGAAAGCATGGGTAGCAAACAGGATTAGATACCCTGGTAGTCCACGC\
CGTAAACGATGTCGACTAGCCGTTGGGACCTTTAAGGACTTAGTGGCGCAGTTAACGCGATAAGTCGACC\
GCCTGGGGGAGTACGGCCGCAAGGTTAAAACTCAAATGAATTGACGGGGGCCCGCACAAGCGGTGGAGCA\
TGTGGTTTAATTCGATGCAACGCGAAGAACCTTACCTACCCTTGACATCCTGCGAATTTGGTAGAGATAC\
CTTAGTGCCTTCGGGAGCGCAGTGACAGGTGCTGCATGGCTGTCGTCAGCTCGCGC");
    BOOST_CHECK_EQUAL(patchseq, newseq);

    // check whether the existing features on the old sequence have been properly adjusted
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 964);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 249 && stop == 562);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
}

BOOST_AUTO_TEST_CASE(Test_PatchSequence_ImportFeatures)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdatePatch, true);
    params.m_ImportFeatures = true;
    // do not remove existing features

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 966);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 964) || (start == 568 && stop == 699);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 587 && stop == 667) || (start == 249 && stop == 562);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 25);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 7);
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_Import_ExceptDuplFeatures)
{
    // do not remove existing features
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Patch, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAllExceptDups;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();

    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), upd_bsh.GetBioseqLength());

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
        || (type == CSeqFeatData::eSubtype_gene)
        || (type == CSeqFeatData::eSubtype_mRNA)
        || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            // only the gene from the old sequence should be present on the updated sequence
            feat_ok = (start == 0 && stop == 131)
            && (NStr::EqualNocase(feat->GetData().GetGene().GetLocus(), "16S rDNA"));
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 0 && stop == 31);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 0 && stop == 0) || (start == 19 && stop == 99);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 25);
            break;
        default:
            break;
        }
    BOOST_CHECK_EQUAL(feat_ok, true);
    count++;
    }
    BOOST_CHECK_EQUAL(count, 6);
    //NcbiCout << "Passed Test_ReplaceSequence_Import_ExceptDuplFeatures" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_IgnoreAlign)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend5, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    const CBioseq_Handle& old_bsh = sequpd_input.GetOldBioseq();
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();

    string origseq;
    CSeqVector orig_svec = old_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    orig_svec.GetSeqData(0, old_bsh.GetBioseqLength(), origseq);
    string extendseq;
    CSeqVector extendvec = upd_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    extendvec.GetSeqData(0, upd_bsh.GetBioseqLength(), extendseq);

    bool ignore_alignment(true);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, ignore_alignment);
    params.m_ImportFeatures = false;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
   
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1080);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);
    BOOST_CHECK_EQUAL(newseq, extendseq + origseq);

    // check whether features are properly adjusted
    // every feature should be shifted towards right
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 95 && stop == 1078);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 294 && stop == 694);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 344 && stop == 657);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
}

BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_IgnoreAlign_ImportAllFeatures_Var1)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend5, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(true);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, ignore_alignment);
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1080);

    // existing features should be shifted and all features from update sequence should be imported
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 95 && stop == 1078) || (start == 54 && stop == 2);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 294 && stop == 694);
            break;
        case (CSeqFeatData::eSubtype_misc_feature):
            feat_ok = (start == 70 && stop == 89);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 344 && stop == 657) || (start == 54 && stop == 2);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 16);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 8);
}

BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_IgnoreAlign_ImportAllFeatures_Var2)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend5_AlignWithGap, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(true);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, ignore_alignment);
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1077);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 92 && stop == 1075) || (start == 2 && stop == 54);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 291 && stop == 691);
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = (start == 67 && stop == 86);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 341 && stop == 654) || (start == 2 && stop == 54);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 16);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 8);
}
 
BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_DoNotIgnoreAlign)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend5, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(false);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, ignore_alignment);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1043);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);

    string extend5seq("GGGGGGGGGGCCCCGGAAAAAAAAAGGGGGGGGGGGCGCACGTTTTTTCACACAGGGGTGCAGTCGAGCG\
GCAGCACGGGGAGCTTGCTCCCTGGTGGCGAGCGGCGGACGGGTGAGTAATGTAGGAATCTGCCCGGTAG\
TGGGGGATAACGTGGGGAAACCCACGCTAATACCGCATACGTCCTACGGGAGAAAGCGGAGGATCTTCGG\
ACTTCGCGCTATCGGATGAGCCTATGTCGGATTAGCTAGTTGGTAAGGTAACGGCTTACCAAGGCGACGA\
TCCGTAGCTGGTCTGAGAGGATGATCAGCCACACTGGGACTGAGACACGGCCCAGACTCCTACGGGAGGC\
AGCAGTGGGGAATATTGGACAATGGGCGAAAGCCTTGATCCAGCCATGCCGCGTGTGTGAAGAAGGCTTT\
CGGGTTGTAAAGCACTTTCAGCGAGGAAGAAAGCCTGGTGGTTAAAGCACCGGCTAACTCCGTGCCAGCA\
GCCGCGGTAATACGGAGGGTGCGAGCGTTAATCGGAATTACTGGGCGTAAAGCGCGCGTAGGTGGCTTGG\
CACGCCGGTTGTGAAAGCCCCGGGCTCAACCTGGGAACGGCATCCGGAACGGCCAGGCTAGAGTGCAGGA\
GAGGAAGGTAGAATTCCCGGTGTAGCGGTGAAATGCGTAGAGATCGGGAGGAATACCAGTGGCGAAGGCG\
GCCTTCTGGCCTGACACTGACACTGAGGTGCGAAAGCGTGGGTAGCAAACAGGATTAGATACCCTGGTAG\
TCCACGCCGTAAACGATGTCGACTAGCCGTTGGGACCTTTAAGGACTTAGTGGCGCAGTTAACGCGATAA\
GTCGACCGCCTGGGGGAGTACGGCCGCAAGGTTAAAACTCAAATGAATTGACGGGGGCCCGCACAAGCGG\
TGGAGCATGTGGTTTAATTCGATGCAACGCGAAGAACCTTACCTACCCTTGACATCCTGCGAATTTGGTA\
GAGATACCTTAGTGCCTTCGGGAGCGCAGTGACAGGTGCTGCATGGCTGTCGTCAGCTCGCGC");
    BOOST_CHECK_EQUAL(extend5seq, newseq);

    // check whether the existing features on the old sequence have been properly adjusted
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 58 && stop == 1041);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 257 && stop == 657);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 307 && stop == 620);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
}

BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_DonotIgnoreAlign_ImportAllFeatures_Var1)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend5, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(false);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, ignore_alignment);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);

    // existing features should be shifted and all features from update sequence should be imported
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 58 && stop == 1041) || (start == 54 && stop == 2);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 257 && stop == 657);
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = (start == 70 && stop == 89);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 307 && stop == 620) || (start == 54 && stop == 2);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 16);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 8);
}

BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_DonotIgnoreAlign_ImportAllFeatures_Var2)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend5_AlignWithGap, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(false);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, ignore_alignment);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1040);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    // existing features should be shifted and all features from update sequence should be imported
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 58 && stop == 1038) || (start == 2 && stop == 54);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 254 && stop == 654);
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = (start == 67 && stop == 86);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 304 && stop == 617) || (start == 2 && stop == 54);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 16);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 8);
}

BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_DonotIgnoreAlign_ImportAllFeatures_Var3)
{
    string old_fname("test_data/seq_for_5extend.asn");
    string upd_fname("test_data/update_5extend.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(false);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, ignore_alignment);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1043);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
                    || (type == CSeqFeatData::eSubtype_gene)
                    || (type == CSeqFeatData::eSubtype_mRNA)
                    || (type == CSeqFeatData::eSubtype_cdregion)
                    || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 39 && stop == 64) || (start == 58 && stop == 1041);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 60 && stop == 77) 
                    || (start == 1037 && stop == 60)
                    || (start == 82 && stop == 107);
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = (start == 70 && stop == 89);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 307 && stop == 620) || (start == 54 && stop == 2);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 17);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 10);
    //NcbiCout << "Passed Test_Extend5Sequence_DonotIgnoreAlign_ImportAllFeatures_Var3" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_Extend3Sequence_IgnoreAlign)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend3, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    const CBioseq_Handle& old_bsh = sequpd_input.GetOldBioseq();
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();

    string origseq;
    CSeqVector orig_svec = old_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    orig_svec.GetSeqData(0, old_bsh.GetBioseqLength(), origseq);
    string extendseq;
    CSeqVector extendvec = upd_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    extendvec.GetSeqData(0, upd_bsh.GetBioseqLength(), extendseq);

    bool ignore_alignment(true);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend3, ignore_alignment);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1180);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);
    BOOST_CHECK_EQUAL(newseq, origseq + extendseq);

    // check whether features are properly placed on the updated sequence
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 983);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 249 && stop == 562);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
}

BOOST_AUTO_TEST_CASE(Test_Extend3Sequence_IgnoreAlign_ImportAllFeatures)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend3, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(true);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend3, ignore_alignment);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1180);



    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 983) || (start == 989 && stop == 1074);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = (start == 1104 && stop == 1124);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 249 && stop == 562) || (start == 1029 && stop == 1104);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 24);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 8);
}

BOOST_AUTO_TEST_CASE(Test_Extend3Sequence_DoNotIgnoreAlign)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend3, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(false);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend3, ignore_alignment);
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1095);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);
    
    string extend3seq("TGCAGTCGAGCGGCAGCACGGGGAGCTTGCTCCCTGGTGGCGAGCGGCGGACGGGTGAGTAATGTAGGAA\
TCTGCCCGGTAGTGGGGGATAACGTGGGGAAACCCACGCTAATACCGCATACGTCCTACGGGAGAAAGCG\
GAGGATCTTCGGACTTCGCGCTATCGGATGAGCCTATGTCGGATTAGCTAGTTGGTAAGGTAACGGCTTA\
CCAAGGCGACGATCCGTAGCTGGTCTGAGAGGATGATCAGCCACACTGGGACTGAGACACGGCCCAGACT\
CCTACGGGAGGCAGCAGTGGGGAATATTGGACAATGGGCGAAAGCCTTGATCCAGCCATGCCGCGTGTGT\
GAAGAAGGCTTTCGGGTTGTAAAGCACTTTCAGCGAGGAAGAAAGCCTGGTGGTTAAAGCACCGGCTAAC\
TCCGTGCCAGCAGCCGCGGTAATACGGAGGGTGCGAGCGTTAATCGGAATTACTGGGCGTAAAGCGCGCG\
TAGGTGGCTTGGCACGCCGGTTGTGAAAGCCCCGGGCTCAACCTGGGAACGGCATCCGGAACGGCCAGGC\
TAGAGTGCAGGAGAGGAAGGTAGAATTCCCGGTGTAGCGGTGAAATGCGTAGAGATCGGGAGGAATACCA\
GTGGCGAAGGCGGCCTTCTGGCCTGACACTGACACTGAGGTGCGAAAGCGTGGGTAGCAAACAGGATTAG\
ATACCCTGGTAGTCCACGCCGTAAACGATGTCGACTAGCCGTTGGGACCTTTAAGGACTTAGTGGCGCAG\
TTAACGCGATAAGTCGACCGCCTGGGGGAGTACGGCCGCAAGGTTAAAACTCAAATGAATTGACGGGGGC\
CCGCACAAGCGGTGGAGCATGTGGTTTAATTCGATGCAACGCGAAGAACCTTACCTACCCTTGACATCCT\
GCGAATTTGGTAGAGATACCTTAGTGCCTTCGGGAGCGCAGTGACAGGTGCTGCATGGCTGTCGTCAGCT\
CGCGCGGGAGAAAGGGGTTTTTTTTTTTATATTATACCCCACCCCTCTCTCCCGGGGGGAGATTAGCCAC\
AGGGGTTTTTTTTTTTATATTATACCCCCCGGGGGGAGATTAGCC");
    BOOST_CHECK_EQUAL(extend3seq, newseq);

    // add a feature that is around the alignment
    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 983);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 249 && stop == 562);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
    //NcbiCout << "Passed Test_Extend3Sequence_DoNotIgnoreAlign" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_Extend3Sequence_DoNotIgnoreAlign_ImportAllFeatures_Var1)
{
    CSeq_entry_Handle old_seh, upd_seh;
    ReadBioSeqs(sc_MainSequence, sc_UpdSequence_Extend3, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(false);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend3, ignore_alignment);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
   
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1095);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 983) || (start == 904 && stop == 989);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_misc_feature):
            feat_ok = (start == 1019 && stop == 1039);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 249 && stop == 562) || (start == 944 && stop == 1019);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69) || (start == 0 && stop == 24);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 8);
}

BOOST_AUTO_TEST_CASE(Test_Extend3Sequence_DonotIgnoreAlign_ImportAllFeatures_Var2)
{
    string old_fname("test_data/seq_for_3extend.asn");
    string upd_fname("test_data/update_3extend.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    bool ignore_alignment(false);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend3, ignore_alignment);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportOption = SUpdateSeqParams::eFeatUpdateAll;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1095);
    BOOST_CHECK_EQUAL(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna, true);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion)
            || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 983) || (start == 904 && stop == 944);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 979 && stop == 2)
                    || (start == 2 && stop == 19)
                    || (start == 849 && stop == 909);
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            feat_ok = (start == 959 && stop == 999);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 899 && stop == 953) || (start == 1019 && stop == 1055);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 11) || (start == 0 && stop == 16);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 10);
}

BOOST_AUTO_TEST_CASE(Test_PatchSequence_UpdateProteins01)
{
    string old_fname("test_data/retranslateCDS.asn");
    string upd_fname("test_data/patch_retranslateCDS.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    // do not remove old features and do not import new features
    // retranslate the coding regions and update the proteins
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdatePatch, true);
    params.m_UpdateProteins |= SUpdateSeqParams::eProtUpdate;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
   
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 3728);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot) || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_cdregion) : 
        {
            vector<TSeqRange> sublocs;
            for (CSeq_loc_CI loc_iter(feat->GetLocation()); loc_iter; ++loc_iter) {
                sublocs.push_back(loc_iter.GetRange());
            }

            vector<TSeqRange> explocs;
            explocs.push_back(CRange<TSeqPos>(64, 73));
            explocs.push_back(CRange<TSeqPos>(529, 668));
            explocs.push_back(CRange<TSeqPos>(805, 983));
            explocs.push_back(CRange<TSeqPos>(1699, 1854));
            explocs.push_back(CRange<TSeqPos>(2898, 3044));
            explocs.push_back(CRange<TSeqPos>(3665, 3727));  // in Sequin, the end was adjusted to 3725
            feat_ok = (sublocs == explocs);

            string new_prot;
            CBioseq_Handle prot_bsh = old_seh.GetScope().GetBioseqHandle(feat->GetProduct());
            CSeqVector new_svec = prot_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
            new_svec.GetSeqData(0, prot_bsh.GetBioseqLength(), new_prot);

            string protein("MGQVFLLLPVLLVSCFLSQGAAMENQRLFNIAVNRVQHLHLMAQKMFNDFEVTLLPDERRQLNKIFLLDF\
CNSDSIVSPPHIYIVLDRHVFKEREFKDRXSTSTRLRRVQSXSCSTSLTVXLNPGSTLARPXPSPTASXS\
ETPTRSLRSSATSKWASTCSSRGARMAYXAWMTMTLSSCPPTGTTTRTWGATETSGGTTSCWPASRRTCT\
RSRPTXPSPSAGSHWRPTALC");
            BOOST_CHECK_EQUAL(new_prot, protein);
            break;
        }
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 230);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 2);
    //NcbiCout << "Passed Test_PatchSequence_UpdateProteins01" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_ReplaceSequence_NoAlign)
{
    string old_fname("test_data/OldSeq_CDS_mRNA.asn");
    string upd_fname("test_data/UpdSeq_NoAlign.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);

    // replace old sequence, do not remove old features
    CSequenceUpdater updater(sequpd_input, params);
    BOOST_CHECK(updater.IsOldSequenceOK());
    BOOST_CHECK(updater.IsUpdateSequenceRaw());
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 1055);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 0 && stop == 983);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 199 && stop == 599);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 249 && stop == 562);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
}

BOOST_AUTO_TEST_CASE(Test_Extend5Sequence_NoAlign)
{
    string old_fname("test_data/OldSeq_CDS_mRNA.asn");
    string upd_fname("test_data/UpdSeq_NoAlign.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend5, true);

    // replace old sequence, do not remove old features
    CSequenceUpdater updater(sequpd_input, params);
    BOOST_CHECK(updater.IsOldSequenceOK());
    BOOST_CHECK(updater.IsUpdateSequenceRaw());
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 2040);

    short count = 0;
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_prot)
            || (type == CSeqFeatData::eSubtype_gene)
            || (type == CSeqFeatData::eSubtype_mRNA)
            || (type == CSeqFeatData::eSubtype_cdregion);
        BOOST_CHECK_EQUAL(type_ok, true);

        const TSeqPos start = feat->GetLocation().GetStart(eExtreme_Biological);
        const TSeqPos stop = feat->GetLocation().GetStop(eExtreme_Biological);
        bool feat_ok = false;

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            feat_ok = (start == 1055 && stop == 2038);
            break;
        case (CSeqFeatData::eSubtype_mRNA) :
            feat_ok = (start == 1254 && stop == 1654);
            break;
        case (CSeqFeatData::eSubtype_cdregion) :
            feat_ok = (start == 1304 && stop == 1617);
            break;
        case (CSeqFeatData::eSubtype_prot) :
            feat_ok = (start == 0 && stop == 69);
            break;
        default:
            break;
        }
        BOOST_CHECK_EQUAL(feat_ok, true);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 4);
} 

BOOST_AUTO_TEST_CASE(Test_Extend3Sequence_NoAlign) // GB-5422
{
    string old_fname("test_data/GB_5422.asn");
    string upd_fname("test_data/UpdSeq_NoAlign.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateExtend3, true);

    // replace old sequence, do not remove old features
    CSequenceUpdater updater(sequpd_input, params);
    BOOST_CHECK(updater.IsOldSequenceOK());
    BOOST_CHECK(updater.IsUpdateSequenceRaw());
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), 2141);

    CFeat_CI feat_it(new_bsh);
    BOOST_CHECK(feat_it);
    CConstRef<CSeq_feat> rna = feat_it->GetOriginalSeq_feat();
    BOOST_CHECK(rna);
    BOOST_CHECK(rna->GetData().GetSubtype() == CSeqFeatData::eSubtype_rRNA);

    BOOST_CHECK_EQUAL(rna->GetLocation().GetStart(eExtreme_Biological), (TSeqPos)0);
    BOOST_CHECK_EQUAL(rna->GetLocation().GetStop(eExtreme_Biological), (TSeqPos)1085);
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportFeatures_MixedAlign1)
{
    string old_fname("test_data/negstrand2.asn");
    string upd_fname("test_data/negstrand2_upd.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_any;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_gene)
                    || (type == CSeqFeatData::eSubtype_tRNA)
                    || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= int {\n"
                        "  from 125,\n"
                        "  to 709,\n"
                        "  strand minus,\n"
                        "  id local str \"AE10_1\",\n"
                        "  fuzz-to lim gt\n"
                        "}\n");
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
                BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= int {\n"
                        "  from 18,\n"
                        "  to 124,\n"
                        "  strand minus,\n"
                        "  id local str \"AE10_1\"\n"
                        "}\n");
            break;
        case (CSeqFeatData::eSubtype_tRNA):
                BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                        "Seq-loc ::= int {\n"
                        "  from 125,\n"
                        "  to 174,\n"
                        "  strand minus,\n"
                        "  id local str \"AE10_1\",\n"
                        "  fuzz-to lim gt\n"
                        "}\n");
            break;
        default:
            break;
        }
    }
}

BOOST_AUTO_TEST_CASE(Test_NoChange_ImportFeatures_MixedAlign2)
{
    string old_fname("test_data/negstrand3.asn");
    string upd_fname("test_data/negstrand3_upd.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_any;

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
   
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_mRNA)
                    || (type == CSeqFeatData::eSubtype_cdregion)
                    || (type == CSeqFeatData::eSubtype_prot);
        BOOST_CHECK_EQUAL(type_ok, true);

        switch (type) {
        case (CSeqFeatData::eSubtype_mRNA) :
        case (CSeqFeatData::eSubtype_cdregion):
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
            "Seq-loc ::= mix {\n"
            "  int {\n"
            "    from 0,\n"
            "    to 22,\n"
            "    strand plus,\n"
            "    id local str \"Nay14\",\n"
            "    fuzz-from lim lt\n"
            "  },\n"
            "  int {\n"
            "    from 124,\n"
            "    to 186,\n"
            "    strand plus,\n"
            "    id local str \"Nay14\"\n"
            "  },\n"
            "  int {\n"
            "    from 421,\n"
            "    to 559,\n"
            "    strand plus,\n"
            "    id local str \"Nay14\"\n"
            "  },\n"
            "  int {\n"
            "    from 615,\n"
            "    to 650,\n"
            "    strand plus,\n"
            "    id local str \"Nay14\",\n"
            "    fuzz-to lim gt\n"
            "  }\n"
            "}\n");
            break;
        default:
            break;
        }
    }
}

// Testing sequence update using alignment with mixed strands
BOOST_AUTO_TEST_CASE(Test_Replace_MixedAlign)
{
    string old_fname("test_data/negstrand.asn");
    string upd_fname("test_data/negstrand_upd.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
   
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), upd_bsh.GetBioseqLength());

    // new sequence should be equal to the update sequence
    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);
    string updseq;
    CSeqVector upd_svec = upd_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    upd_svec.GetSeqData(0, upd_bsh.GetBioseqLength(), updseq);
    BOOST_CHECK_EQUAL(updseq, newseq);

    // check whether the existing features on the old sequence have been properly adjusted
    VISIT_ALL_SEQFEATS_WITHIN_SEQENTRY(feat, *old_seh.GetCompleteSeq_entry()) {
        CSeqFeatData::ESubtype type = feat->GetData().GetSubtype();
        bool type_ok = (type == CSeqFeatData::eSubtype_gene)
                    || (type == CSeqFeatData::eSubtype_misc_feature);
        BOOST_CHECK_EQUAL(type_ok, true);

        switch (type) {
        case (CSeqFeatData::eSubtype_gene) :
            BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                            "Seq-loc ::= int {\n"
                            "  from 47,\n"
                            "  to 53,\n"
                            "  strand minus,\n"
                            "  id local str \"AE10_1_1\",\n"
                            "  fuzz-to lim gt\n"
                            "}\n");
            break;
        case (CSeqFeatData::eSubtype_misc_feature) :
            if (NStr::EqualNocase(feat->GetComment(), "before the alignment (1..>11)")) {
                BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                            "Seq-loc ::= int {\n"
                            "  from 55,\n"
                            "  to 55,\n"
                            "  strand minus,\n"
                            "  id local str \"AE10_1_1\",\n"
                            "  fuzz-from lim lt\n"
                            "}\n");
            }
            else if (NStr::EqualNocase(feat->GetComment(), "partially inside the alignment  (<5..23)")) {
                BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                            "Seq-loc ::= int {\n"
                            "  from 44,\n"
                            "  to 55,\n"
                            "  strand minus,\n"
                            "  id local str \"AE10_1_1\",\n"
                            "  fuzz-to lim gt\n"
                            "}\n");
            }
            else if (NStr::EqualNocase(feat->GetComment(), "negative strand (<6..>50)")) {
                BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                            "Seq-loc ::= int {\n"
                            "  from 17,\n"
                            "  to 55,\n"
                            "  strand plus,\n"
                            "  id local str \"AE10_1_1\",\n"
                            "  fuzz-from lim lt,\n"
                            "  fuzz-to lim gt\n"
                            "}\n");
            }
            else if (NStr::EqualNocase(feat->GetComment(), "negative strand (1..>7)")) {
                BOOST_CHECK_EQUAL(MakeAsn(feat->GetLocation()),
                            "Seq-loc ::= int {\n"
                            "  from 55,\n"
                            "  to 55,\n"
                            "  strand plus,\n"
                            "  id local str \"AE10_1_1\",\n"
                            "  fuzz-from lim lt\n"
                            "}\n");
            }
            break;
        default:
            break;
        }
    }
}

BOOST_AUTO_TEST_CASE(Test_Replace_DeltaSequence)
{
    string old_fname("test_data/deltaseq.asn");
    string upd_fname("test_data/update_rawseq.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);

    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter)  new_bsh = *b_iter;

    BOOST_REQUIRE(new_bsh);
    const CBioseq_Handle& upd_bsh = sequpd_input.GetUpdateBioseq();
    BOOST_CHECK_EQUAL(new_bsh.GetBioseqLength(), upd_bsh.GetBioseqLength());

    // new sequence should be equal to the update sequence
    string newseq;
    CSeqVector new_svec = new_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, new_bsh.GetBioseqLength(), newseq);
    string updseq;
    CSeqVector upd_svec = upd_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    upd_svec.GetSeqData(0, upd_bsh.GetBioseqLength(), updseq);
    BOOST_CHECK_EQUAL(updseq, newseq);

    BOOST_CHECK(new_bsh.GetInst_Repr() == CSeq_inst::eRepr_raw);
    BOOST_CHECK(!new_bsh.GetInst().IsSetExt());
    BOOST_CHECK(new_bsh.GetInst_Mol() == CSeq_inst::eMol_rna);
    BOOST_CHECK(new_bsh.GetInst_Topology() == CSeq_inst::eTopology_tandem);
    BOOST_CHECK(new_bsh.GetInst_Strand() == CSeq_inst::eStrand_ds);
    BOOST_CHECK(new_bsh.GetInst().GetSeq_data().IsNcbi4na());
    //NcbiCout << "Passed Test_Replace_DeltaSequence" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_Replace_DeltaSeqFarPointers)
{
    string old_fname("test_data/delta_farpointers.asn");

    CRef<CSeq_entry> old_entry = ReadEntryFromFile(old_fname);
    CRef<CScope> scope = BuildScope();
    CSeq_entry_Handle old_seh = scope->AddTopLevelSeqEntry(*old_entry);

    CBioseq_Handle old_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter) old_bsh = *b_iter;
    CSeq_id id("NG_001019.5");
    
    CRef<CObjectManager> object_manager = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*object_manager, NULL, CObjectManager::eDefault);
    scope->AddDataLoader(CGBDataLoader::GetLoaderNameFromArgs());
    CBioseq_Handle upd_bsh = scope->GetBioseqHandle(id);
    BOOST_REQUIRE(upd_bsh);
    CSeq_entry_Handle upd_seh = upd_bsh.GetTopLevelEntry();


    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateNoChange);
    params.m_FeatRemoveOption = SUpdateSeqParams::eFeatRemoveNone;
    params.m_ImportFeatures = true;
    params.m_FeatImportType = CSeqFeatData::eSubtype_gene;
    CSequenceUpdater updater(sequpd_input, params);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());
    CRef<CCmdComposite> cmd = updater.Update(create_general_only);
    
    BOOST_REQUIRE(cmd);
    cmd->Execute();

    CBioseq_Handle new_bsh;
    CBioseq_CI bseq_it(old_seh, CSeq_inst::eMol_na);
    if (bseq_it)  new_bsh = *bseq_it;

    short count_genes = 0;
    for (CFeat_CI feat_it(new_bsh); feat_it; ++feat_it) {
        count_genes++;
    }

    BOOST_CHECK_EQUAL(count_genes, 185);
    //NcbiCout << "Passed Test_Replace_DeltaSeqFarPointers" << NcbiEndl;
}

BOOST_AUTO_TEST_CASE(Test_ReplaceProteinSeqs1)
{
    string old_fname("test_data/protein_seqs1.asn");
    string upd_fname("test_data/protein_seqs1_update.fa");

    CRef<CSeq_entry> old_entry = ReadEntryFromFile(old_fname);
    CRef<CSeq_entry> upd_entry = ReadFastaSeqsFromFile(upd_fname);
    CRef<CScope> scope = BuildScope();

    CSeq_entry_Handle old_seh = scope->AddTopLevelSeqEntry(*old_entry);
    BOOST_REQUIRE(old_seh);


    CUpdateMultipleSeq_Input multiupdseq_in;
    bool ok = multiupdseq_in.SetOldEntryAndScope(old_seh);
    ok = ok && multiupdseq_in.SetUpdateEntry(upd_entry);
    ok = ok && multiupdseq_in.PrepareMultipleSequenceInputsForUnitTest();
    BOOST_REQUIRE(ok);

    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& updates = multiupdseq_in.GetNonIdenticalUpdates();
    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& identicals = multiupdseq_in.GetIdenticalUpdates();
    const CUpdateMultipleSeq_Input::TSeqIDHVector& noUpdates = multiupdseq_in.GetSeqsWithoutUpdates();

    BOOST_CHECK(updates.size() == 4);
    BOOST_CHECK(identicals.empty());
    BOOST_CHECK(noUpdates.size() == 1);

    CRef<CCmdComposite> update_cmd(new CCmdComposite("Update multiple sequences"));
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());

    for (auto& it : updates) {
        CSequenceUpdater updater(it.second.GetObject(), params);
        CRef<CCmdComposite> cmd = updater.Update(create_general_only);
        update_cmd->AddCommand(*cmd);
    }

    update_cmd->Execute();

    for (CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_aa); b_iter; ++b_iter) {
        string prot_seq;
        CSeqVector new_svec = b_iter->GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
        new_svec.GetSeqData(0, b_iter->GetBioseqLength(), prot_seq);
        BOOST_CHECK(prot_seq.front() == 'M');
    }
}

BOOST_AUTO_TEST_CASE(Test_ReplaceProteinSeqs2)
{
    string old_fname("test_data/mat_peptide.asn");
    string upd_fname("test_data/mat_pepshort_upd.fa");

    CRef<CSeq_entry> old_entry = ReadEntryFromFile(old_fname);
    CRef<CSeq_entry> upd_entry = ReadFastaSeqsFromFile(upd_fname);
    CRef<CScope> scope = BuildScope();

    CSeq_entry_Handle old_seh = scope->AddTopLevelSeqEntry(*old_entry);
    BOOST_REQUIRE(old_seh);


    CUpdateMultipleSeq_Input multiupdseq_in;
    bool ok = multiupdseq_in.SetOldEntryAndScope(old_seh);
    ok = ok && multiupdseq_in.SetUpdateEntry(upd_entry);
    ok = ok && multiupdseq_in.PrepareMultipleSequenceInputsForUnitTest();
    BOOST_REQUIRE(ok);

    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& updates = multiupdseq_in.GetNonIdenticalUpdates();
    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& identicals = multiupdseq_in.GetIdenticalUpdates();
    const CUpdateMultipleSeq_Input::TSeqIDHVector& noUpdates = multiupdseq_in.GetSeqsWithoutUpdates();

    BOOST_CHECK(updates.size() == 1);
    BOOST_CHECK(identicals.empty());
    BOOST_CHECK(noUpdates.empty());

    CRef<CCmdComposite> update_cmd(new CCmdComposite("Update multiple sequences"));
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(old_seh.GetTopLevelEntry());

    for (auto& it : updates) {
        CSequenceUpdater updater(it.second.GetObject(), params);
        CRef<CCmdComposite> cmd = updater.Update(create_general_only);
        update_cmd->AddCommand(*cmd);
    }

    update_cmd->Execute();

    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_aa);
    BOOST_CHECK(b_iter);
    string prot_seq;
    CSeqVector new_svec = b_iter->GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    new_svec.GetSeqData(0, b_iter->GetBioseqLength(), prot_seq);
    BOOST_CHECK(prot_seq.front() == 'M');

    BOOST_CHECK_EQUAL(b_iter->GetBioseqLength(), 562);
    for (CFeat_CI it(*b_iter, SAnnotSelector(CSeqFeatData::e_Prot)); it; ++it) {
        const CProt_ref& prot_ref = it->GetOriginalFeature().GetData().GetProt();
        if (prot_ref.IsSetProcessed()) {
            if (prot_ref.GetProcessed() == CProt_ref::eProcessed_signal_peptide) {
                BOOST_CHECK_EQUAL(MakeAsn(it->GetLocation()),
                    "Seq-loc ::= int {\n"
                    "  from 0,\n"
                    "  to 15,\n"
                    "  id local str \"Seq4_prot_6\"\n"
                    "}\n");
            }
            else {
                if (prot_ref.GetName().front() == "HA1") {
                    BOOST_CHECK_EQUAL(MakeAsn(it->GetLocation()),
                        "Seq-loc ::= int {\n"
                        "  from 16,\n"
                        "  to 343,\n"
                        "  id local str \"Seq4_prot_6\"\n"
                        "}\n");
                }
                else {
                    BOOST_CHECK_EQUAL(MakeAsn(it->GetLocation()),
                        "Seq-loc ::= int {\n"
                        "  from 344,\n"
                        "  to 561,\n"
                        "  id local str \"Seq4_prot_6\"\n"
                        "}\n");
                }
            }
        }
        else {
            BOOST_CHECK_EQUAL(MakeAsn(it->GetLocation()),
                "Seq-loc ::= int {\n"
                "  from 0,\n"
                "  to 561,\n"
                "  id local str \"Seq4_prot_6\"\n"
                "}\n");
        }
    }

}

BOOST_AUTO_TEST_CASE(Test_s_ExtendOneEndOfSequence)
{
    CRef<CSeq_entry> entry = unit_test_util::BuildGoodSeq();
    entry->SetSeq().SetInst().SetMol(CSeq_inst::eMol_rna);

    CScope scope(*CObjectManager::GetInstance());
    CSeq_entry_Handle seh = scope.AddTopLevelSeqEntry(*entry);
    
    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    BOOST_REQUIRE(b_iter);

    {{
        string extend("aaallaagggggzzzztttttccccec");
        CRef<CSeq_inst> inst_ext5 = CSequenceUpdater::s_ExtendOneEndOfSequence(*b_iter, extend, SUpdateSeqParams::eSeqUpdateExtend5);
        CRef<CSeq_inst> inst_ext3 = CSequenceUpdater::s_ExtendOneEndOfSequence(*b_iter, extend, SUpdateSeqParams::eSeqUpdateExtend3);

        BOOST_CHECK(inst_ext5->GetLength() == inst_ext3->GetLength());
        BOOST_CHECK(inst_ext5->GetLength() == 80);
        BOOST_CHECK(inst_ext5->GetMol() == CSeq_inst::eMol_rna);

        string extended5;
        CSeqConvert::Convert(inst_ext5->GetSeq_data().GetNcbi2na().Get(),
            CSeqUtil::e_Ncbi2na, 0, 80, extended5, CSeqUtil::e_Iupacna);
        BOOST_CHECK_EQUAL(extended5, string("AAAAAGGGGGTTTTTCCCCCAATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA"));

        string extended3;
        CSeqConvert::Convert(inst_ext3->GetSeq_data().GetNcbi2na().Get(),
            CSeqUtil::e_Ncbi2na, 0, 80, extended3, CSeqUtil::e_Iupacna);
        BOOST_CHECK_EQUAL(extended3, string("AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAAAAAAAGGGGGTTTTTCCCCC"));
    }}

    {{

        string extend("PPPPP");
        CRef<CSeq_inst> inst_ext5 = CSequenceUpdater::s_ExtendOneEndOfSequence(*b_iter, extend, SUpdateSeqParams::eSeqUpdateExtend5);
        CRef<CSeq_inst> inst_ext3 = CSequenceUpdater::s_ExtendOneEndOfSequence(*b_iter, extend, SUpdateSeqParams::eSeqUpdateExtend3);

        BOOST_CHECK(!inst_ext5);
        BOOST_CHECK(!inst_ext3);
    }}
}

BOOST_AUTO_TEST_CASE(Test_s_OffsetFeature)
{
    CRef<CSeq_id> seq_id(new CSeq_id(CSeq_id::e_Local, "good"));
    size_t left = 0;
    size_t right = 10;
    CRef<CSeq_feat> misc_feat = unit_test_util::MakeMiscFeature(seq_id, right, left);

    CRef<CSeq_loc> newloc(new CSeq_loc);
    newloc->SetInt().SetFrom(30);
    newloc->SetInt().SetTo(59);
    newloc->SetInt().SetId().Assign(*seq_id);
    misc_feat->SetLocation().Add(*newloc);

    string extend = CSequenceUpdater::s_GetValidExtension("aaalaapgggggzzzztttttccccpc");
    BOOST_REQUIRE(extend.size() == 20);
    {{
        CRef<CSeq_feat> newfeat = CSequenceUpdater::s_OffsetFeature(*misc_feat, extend.size());
        BOOST_CHECK_EQUAL(MakeAsn(newfeat->GetLocation()),
                            "Seq-loc ::= packed-int {\n"
                            "  {\n"
                            "    from 20,\n"
                            "    to 30,\n"
                            "    id local str \"good\"\n"
                            "  },\n"
                            "  {\n"
                            "    from 50,\n"
                            "    to 79,\n"
                            "    id local str \"good\"\n"
                            "  }\n"
                            "}\n");
    }}

    misc_feat->SetLocation().SetStrand(eNa_strand_minus);
    {{
        CRef<CSeq_feat> newfeat = CSequenceUpdater::s_OffsetFeature(*misc_feat, extend.size());
        BOOST_CHECK_EQUAL(MakeAsn(newfeat->GetLocation()),
                            "Seq-loc ::= packed-int {\n"
                            "  {\n"
                            "    from 20,\n"
                            "    to 30,\n"
                            "    strand minus,\n"
                            "    id local str \"good\"\n"
                            "  },\n"
                            "  {\n"
                            "    from 50,\n"
                            "    to 79,\n"
                            "    strand minus,\n"
                            "    id local str \"good\"\n"
                            "  }\n"
                            "}\n");
    }}

    CRef<CSeq_feat> trna(new CSeq_feat);
    trna->SetData().SetRna().SetType(CRNA_ref::eType_tRNA);
    trna->SetLocation().Assign(misc_feat->GetLocation());

    CRNA_ref::C_Ext& ext = trna->SetData().SetRna().SetExt();
    ext.SetTRNA().SetAnticodon().SetInt().SetFrom(40);
    ext.SetTRNA().SetAnticodon().SetInt().SetTo(42);
    ext.SetTRNA().SetAnticodon().SetInt().SetStrand(eNa_strand_minus);
    ext.SetTRNA().SetAnticodon().SetInt().SetId().Assign(*seq_id);
    {{
        CRef<CSeq_feat> newfeat = CSequenceUpdater::s_OffsetFeature(*trna, extend.size());
        BOOST_CHECK_EQUAL(MakeAsn(newfeat->GetLocation()),
                            "Seq-loc ::= packed-int {\n"
                            "  {\n"
                            "    from 20,\n"
                            "    to 30,\n"
                            "    strand minus,\n"
                            "    id local str \"good\"\n"
                            "  },\n"
                            "  {\n"
                            "    from 50,\n"
                            "    to 79,\n"
                            "    strand minus,\n"
                            "    id local str \"good\"\n"
                            "  }\n"
                            "}\n");
        const CRNA_ref::C_Ext& newExt = newfeat->GetData().GetRna().GetExt();
        BOOST_CHECK_EQUAL(MakeAsn(newExt.GetTRNA().GetAnticodon()),
                            "Seq-loc ::= int {\n"
                            "  from 60,\n"
                            "  to 62,\n"
                            "  strand minus,\n"
                            "  id local str \"good\"\n"
                            "}\n");
    }}
    
}

BOOST_AUTO_TEST_CASE(Test_FixCollidingIDs)
{
    string old_fname("test_data/collideID.asn");
    string upd_fname("test_data/collideID_update.asn");
    CRef<CScope> scope = BuildScope();

    CRef<CSeq_entry> old_entry = ReadEntryFromFile(old_fname);
    CSeq_entry_Handle old_seh = scope->AddTopLevelSeqEntry(*old_entry);
    BOOST_REQUIRE(old_seh);

    CRef<CSeq_entry> upd_entry = ReadEntryFromFile(upd_fname);
    upd_entry->Parentize();
    const CBioseq::TId& old_ids = old_entry->GetSeq().GetId();
    CUpdateSeq_Input::s_FixCollidingIDs_Entry(*upd_entry, old_ids);

    CSeq_entry_Handle upd_seh = scope->AddTopLevelSeqEntry(*upd_entry);
    BOOST_REQUIRE(upd_seh);
    BOOST_REQUIRE(CFeat_CI(upd_seh, CSeqFeatData::eSubtype_cdregion));
}

BOOST_AUTO_TEST_CASE(Test_Align2IdentSeq)
{
    string old_fname("test_data/GB_5422.asn");
    CRef<CScope> scope = BuildScope();

    CRef<CSeq_entry> old_entry = ReadEntryFromFile(old_fname);
    CSeq_entry_Handle old_seh = scope->AddTopLevelSeqEntry(*old_entry);
    BOOST_REQUIRE(old_seh);

    CBioseq_Handle old_bsh;
    CBioseq_CI b_iter(old_seh, CSeq_inst::eMol_na);
    if (b_iter) old_bsh = *b_iter;

    CRef<CSeq_entry> upd_entry = ReadEntryFromFile(old_fname);
    upd_entry->Parentize();
    const CBioseq::TId& old_ids = old_entry->GetSeq().GetId();
    CUpdateSeq_Input::s_FixCollidingIDs_Entry(*upd_entry, old_ids);

    CSeq_entry_Handle upd_seh = scope->AddTopLevelSeqEntry(*upd_entry);
    BOOST_REQUIRE(upd_seh);

    CBioseq_Handle upd_bsh;
    CBioseq_CI bupd_iter(upd_seh, CSeq_inst::eMol_na);
    if (bupd_iter) upd_bsh = *bupd_iter;

    CRef<CSeq_annot> annot = CUpdateSeq_Input::s_Align2IdenticalSeq(old_bsh, upd_bsh);
    BOOST_CHECK_EQUAL(MakeAsn(annot.GetObject()),
                    "Seq-annot ::= {\n"
                    "  desc {\n"
                    "    name \"Alignment generated between two identical sequences\"\n"
                    "  },\n"
                    "  data align {\n"
                    "    {\n"
                    "      type partial,\n"
                    "      dim 2,\n"
                    "      segs denseg {\n"
                    "        dim 2,\n"
                    "        numseg 1,\n"
                    "        ids {\n"
                    "          local str \"KT250608_update\",\n"
                    "          genbank {\n"
                    "            accession \"KT250608\"\n"
                    "          }\n"
                    "        },\n"
                    "        starts {\n"
                    "          0,\n"
                    "          0\n"
                    "        },\n"
                    "        lens {\n"
                    "          1085\n"
                    "        }\n"
                    "      }\n"
                    "    }\n"
                    "  }\n"
                    "}\n");
    
}

BOOST_AUTO_TEST_CASE(Test_GoodAlignment_GB7819)
{
    string old_fname("test_data/goodalign.asn");
    string upd_fname("test_data/update_goodalign.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    auto align = sequpd_input.GetAlignment();
    BOOST_CHECK(align);

    BOOST_CHECK(align->GetAlignLength() == 228);
}

BOOST_AUTO_TEST_CASE(Test_GoodAlignment_GB8669)
{
    string old_fname("test_data/GB_8669.asn");
    string upd_fname("test_data/update_GB_8669.asn");
    CSeq_entry_Handle old_seh, upd_seh;
    ReadFromFile(old_fname, upd_fname, old_seh, upd_seh);

    CUpdateSeq_Input sequpd_input;
    SetupForUpdate(old_seh, upd_seh, sequpd_input);

    auto align = sequpd_input.GetAlignment();
    BOOST_CHECK(align);

    BOOST_CHECK(align->GetAlignLength() == 53333);
    NcbiCout << "Passed ALL sequence update tests" << NcbiEndl;
}

//////////////////////////////////////////////////////////////////////////////////
const char* sc_MainSequence = "\
Seq-entry ::= set { \
 class nuc-prot , \
 descr { \
    source{ \
     genome genomic, \
     org { \
       taxname \"Salinicola sp. SOil-1\" , \
        orgname { \
          gcode 1 } } } }, \
 seq-set { \
  seq { \
   id { \
      local str \"Untitled_Seq_#1\" , \
      genbank { \
          accession \"KM986631\" } } , \
   descr { \
      molinfo { \
          biomol rRNA } }, \
   inst { \
      repr raw , \
      mol rna , \
      length 985 , \
      seq-data iupacna \"\
TGCAGTCGAGCGGCAGCACGGGGAGCTTGCTCCCTGGTGGCGAGCGGCGGACGGGTGAGTAATGTAGGAA\
TCTGCCCGGTAGTGGGGGATAACGTGGGGAAACCCACGCTAATACCGCATACGTCCTACGGGAGAAAGCG\
GAGGATCTTCGGACTTCGCGCTATCGGATGAGCCTATGTCGGATTAGCTAGTTGGTAAGGTAACGGCTTA\
CCAAGGCGACGATCCGTAGCTGGTCTGAGAGGATGATCAGCCACACTGGGACTGAGACACGGCCCAGACT\
CCTACGGGAGGCAGCAGTGGGGAATATTGGACAATGGGCGAAAGCCTTGATCCAGCCATGCCGCGTGTGT\
GAAGAAGGCTTTCGGGTTGTAAAGCACTTTCAGCGAGGAAGAAAGCCTGGTGGTTAAAGCACCGGCTAAC\
TCCGTGCCAGCAGCCGCGGTAATACGGAGGGTGCGAGCGTTAATCGGAATTACTGGGCGTAAAGCGCGCG\
TAGGTGGCTTGGCACGCCGGTTGTGAAAGCCCCGGGCTCAACCTGGGAACGGCATCCGGAACGGCCAGGC\
TAGAGTGCAGGAGAGGAAGGTAGAATTCCCGGTGTAGCGGTGAAATGCGTAGAGATCGGGAGGAATACCA\
GTGGCGAAGGCGGCCTTCTGGCCTGACACTGACACTGAGGTGCGAAAGCGTGGGTAGCAAACAGGATTAG\
ATACCCTGGTAGTCCACGCCGTAAACGATGTCGACTAGCCGTTGGGACCTTTAAGGACTTAGTGGCGCAG\
TTAACGCGATAAGTCGACCGCCTGGGGGAGTACGGCCGCAAGGTTAAAACTCAAATGAATTGACGGGGGC\
CCGCACAAGCGGTGGAGCATGTGGTTTAATTCGATGCAACGCGAAGAACCTTACCTACCCTTGACATCCT\
GCGAATTTGGTAGAGATACCTTAGTGCCTTCGGGAGCGCAGTGACAGGTGCTGCATGGCTGTCGTCAGCT\
CGCGC\" } , \
    annot { \
     { \
     data \
       ftable{ \
         { \
           data \
             gene{ locus \"16S rDNA\" } , \
           comment \"partial sequence\", \
           location \
             int { \
                from 0 , \
                to 983 , \
                strand plus , \
                id local str \"Untitled_Seq_#1\" } } , \
        { \
          data \
            rna { \
                type mRNA , \
                ext \
                    name \"new mRNA\" } , \
          comment \"mRNA comment\" , \
          location \
            mix { \
             int{ \
                from 199 , \
                to 399 , \
                strand plus , \
                id \
                    genbank{ \
                        accession \"KM986631\" } } , \
             int{ \
                from 499 , \
                to 599 , \
                strand plus , \
                id \
                    genbank { \
                        accession \"KM986631\" } } } } } } } } , \
   seq { \
    id { local  str \"KM986631_1\" } , \
    descr { \
        molinfo { \
            biomol peptide , \
            completeness partial } } , \
    inst{ \
        repr raw , \
        mol aa , \
        length 70 , \
        seq-data \
        ncbieaa \"HTGTETRPRLLREAAVGNIGQWAKALIQPCRVCEEGFRVVKHFQRGRKPGGTPVVKAPGSTWERHPERPG\" } , \
    annot{ \
       { \
        data \
           ftable{ \
             { \
                data \
                 prot { \
                  name { \"new protein\" } } , \
                partial TRUE , \
                location \
                    int { \
                        from 0 , \
                        to 69 , \
                        id local str \"KM986631_1\" } } } } } } } , \
  annot { \
    { \
     data \
       ftable{ \
         { \
            data \
            cdregion { \
                frame three , \
                code { id 1 } } , \
            partial TRUE , \
            product \
                whole local str \"KM986631_1\" , \
            location \
                mix { \
                  int { \
                    from 249 , \
                    to 399 , \
                    strand plus , \
                    id \
                      genbank { \
                        accession \"KM986631\" } } , \
                   int { \
                     from 499 , \
                     to 562 , \
                     strand plus , \
                     id \
                        genbank { \
                            accession \"KM986631\" } } } } } } } } \
}";
 
const char* sc_UpdSequence_Same = "\
Seq-entry ::= seq { \
  id { \
    local str \"Seq_01\" } , \
  inst { \
    repr raw , \
    mol dna , \
    length 985 , \
    seq-data iupacna \"\
TGCAGTCGAGCGGCAGCACGGGGAGCTTGCTCCCTGGTGGCGAGCGGCGGACGGGTGAGTAATGTAGGAA\
TCTGCCCGGTAGTGGGGGATAACGTGGGGAAACCCACGCTAATACCGCATACGTCCTACGGGAGAAAGCG\
GAGGATCTTCGGACTTCGCGCTATCGGATGAGCCTATGTCGGATTAGCTAGTTGGTAAGGTAACGGCTTA\
CCAAGGCGACGATCCGTAGCTGGTCTGAGAGGATGATCAGCCACACTGGGACTGAGACACGGCCCAGACT\
CCTACGGGAGGCAGCAGTGGGGAATATTGGACAATGGGCGAAAGCCTTGATCCAGCCATGCCGCGTGTGT\
GAAGAAGGCTTTCGGGTTGTAAAGCACTTTCAGCGAGGAAGAAAGCCTGGTGGTTAAAGCACCGGCTAAC\
TCCGTGCCAGCAGCCGCGGTAATACGGAGGGTGCGAGCGTTAATCGGAATTACTGGGCGTAAAGCGCGCG\
TAGGTGGCTTGGCACGCCGGTTGTGAAAGCCCCGGGCTCAACCTGGGAACGGCATCCGGAACGGCCAGGC\
TAGAGTGCAGGAGAGGAAGGTAGAATTCCCGGTGTAGCGGTGAAATGCGTAGAGATCGGGAGGAATACCA\
GTGGCGAAGGCGGCCTTCTGGCCTGACACTGACACTGAGGTGCGAAAGCGTGGGTAGCAAACAGGATTAG\
ATACCCTGGTAGTCCACGCCGTAAACGATGTCGACTAGCCGTTGGGACCTTTAAGGACTTAGTGGCGCAG\
TTAACGCGATAAGTCGACCGCCTGGGGGAGTACGGCCGCAAGGTTAAAACTCAAATGAATTGACGGGGGC\
CCGCACAAGCGGTGGAGCATGTGGTTTAATTCGATGCAACGCGAAGAACCTTACCTACCCTTGACATCCT\
GCGAATTTGGTAGAGATACCTTAGTGCCTTCGGGAGCGCAGTGACAGGTGCTGCATGGCTGTCGTCAGCT\
CGCGC\" } \
}";

const char* sc_UpdSequence_Replace = "\
Seq-entry ::= seq { \
  id { \
    local str \"Seq_02\" } , \
  inst { \
    repr raw , \
    mol dna , \
    length 1360 , \
    seq-data iupacna \"\
GAGGCACGGGGAGCTTGCTCCCTGGTGGCGAGCGGCGGACGGGTGAGTAATGTAGGAA\
TCTGCCCGGTAGTGGGGGATAACGTGGGGAAACCCACGCTAATACCGCATACGTCCTACGGGAGAAAGCG\
GAGGATCTTCGGACTTCGCGCTATCGGATGAGCCTATGTCGGATTAGCTAGTTGGTAAGGTAACGGCTTA\
CCAAGGCGACGATCCGTAGCTGGTCTGAGAGGATGATCAGCCACACTGGGACTGAGACACGGCCCAGACT\
CCTACGGGAGGCAGCAGTGGGGAATATTGGACAATGGGCGAAAGCCTTGATCCAGCCATGCCGCGTGTGT\
GAAGAAGGCTTTCGGGTTGTAAAGCACTTTCAGCGAGGAAGAAAGCCTGGTGGTTAATACCCATCAGGAA\
GGACATCACTCGCAGAAGAAGCACCGGCTAACTCCGTGCCAGCAGCCGCGGTAATACGGAGGGTGCGAGC\
GTTAATCGGAATTACTGGGCGTAAAGCGCGCGTAGGTGGCTTGGCACGCCGGTTGTGAAAGCCCCGGGCT\
CAACCTGGGAACGGCATCCGGAACGGCCAGGCTAGAGTGCAGGAGAGGAAGGTAGAATTCCCGGTGTAGC\
GGTGAAATGCGTAGAGATCGGGAGGAATACCAGTGGCGAAGGCGGCCTTCTGGCCTGACACTGACACTGA\
GGTGCGAAAGCGTGGGTAGCAAACAGGATTAGATACCCTGGTAGTCCACGCCGTAAACGATGTCGACTAG\
CCGTTGGGACCTTTAAGGACTTAGTGGCGCAGTTAACGCGATAAGTCGACCGCCTGGGGGAGTACGGCCG\
CAAGGTTAAAACTCAAATGAATTGACGGGGGCCCGCACAAGCGGTGGAGCATGTGGTTTAATTCGATGCA\
ACGCGAAGAACCTTACCTACCCTTGACATCCTGCGAATTTGGTAGAGATACCTTAGTGCCTTCGGGAGCG\
CAGTGACAGGTGCTGCATGGCTGTCGTCAGCTCGTGTTGTGAAATGTTGGGTTAAGTCCCGTAACGAGCG\
CAACCCTTTGTCCTTATTTGCCAGCACGTAATGGTGGGAACTCTAAGGAGACTGCCGGTGACAAACCGGA\
GGAAGGTGGGGACGACGTCGGCCCTTACGGGTAGGGCTACACACGTGCTACAATGGCCGG\
TACAAAGGGTTGCGAGACCGCGAGGTGGAGCGAATCCCAGAAAGCCGGCCTCAGTCCGGATCGGAGTCTG\
CAACTCGACTCCGTGAAGTCGGAAAGTAATCGTGAATCAGAATGTCACGGTGAATACGTTCCCGG\
GCCTTGTACACACCGCCCGTCACACCATGGGAGTGGACTGCACCAGAAGTGGTTAGC\" } \
}";

const char* sc_UpdSequence_Patch = "\
Seq-entry ::= set { \
 class nuc-prot , \
 seq-set { \
    seq { \
      id { local str \"Seq_02\" } , \
    inst { \
      repr raw , \
      mol dna , \
      length 132 , \
      seq-data iupacna \"AGGAGAGGAAGGTAGAATTCCCGGTGTAGCGGTGAAATGCGTAGAGATCGGGCCCCCCCCCCCCCCCCCC\
CCTGACACTGAGGTGCGAAAGCATGGGTAGCAAACAGGATTAGATACCCTGGTAGTCCACGC\" } , \
    annot {  \
     { \
      data \
        ftable{ \
          { \
            data \
            gene{ locus \"new gene locus\" } , \
            location \
                int { \
                    from 0 , \
                    to 131 , \
                    strand plus , \
                    id local str \"seq_02\" } } } } } } , \
   seq { \
    id{ local str \"seq_02_1\" } , \
    descr { \
        title \"new protein name, partial\", \
        molinfo { \
            biomol peptide, \
            tech concept-trans, \
            completeness no-left } } , \
        inst { \
            repr raw , \
            mol aa, \
            length 26 , \
            seq-data ncbieaa \"PGVAVKCVEIGPPPPPPPDTEVRKHG\" } , \
        annot { \
            { \
            data \
             ftable{ \
               { \
                data \
                 prot { \
                    name { \"new protein name\" } , \
                    desc \"new protein descr\" } , \
                 partial TRUE , \
                 location \
                    int { \
                        from 0 , \
                        to 25 , \
                        id local str \"seq_02_1\" , \
                        fuzz-from lim lt } } } } } } } , \
annot{ \
  { \
   data \
    ftable{ \
      { \
     data \
        cdregion { \
            frame one, \
            code{ id 1 } } , \
        partial TRUE, \
        product whole local str \"seq_02_1\" , \
        location \
          int { \
            from 19 , \
            to 99 , \
            strand plus , \
            id local str \"seq_02\" , \
            fuzz-from lim lt } } } } } } \
}";

const char* sc_UpdSequence_Extend5 = "\
Seq-entry ::= set { \
 class nuc-prot , \
  seq-set { \
    seq { \
     id { local str \"Seq_extend5\" } , \
    inst { \
     repr raw , \
     mol na , \
     length 95 , \
     seq-data iupacna \"\
GGGGGGGGGGCCCCGGAAAAAAAAAGGGGGGGGGGGCGCACGTTTTTTCACACAGGGGTGCAGTCGAGCGGCAGCACGGGGAGCTTGCTCCCTGG\" } , \
    annot { \
     { \
     data \
       ftable{ \
        { \
        data \
         gene{ \
            locus \"import gene locusA\" }, \
        partial TRUE, \
        location \
            int { \
                from 2, \
                to 54, \
                strand minus, \
                id local str \"Seq_extend5\",  \
                fuzz-from lim lt, \
                fuzz-to lim gt } } , \
        { \
        data \
          imp { \
            key \"misc_feature\" } , \
        location \
         int{ \
          from 70, \
          to 89, \
          strand plus, \
          id local str \"Seq_extend5\" } , \
        qual{ \
          { \
            qual \"number\", \
            val \"1\" }, \
          { \
            qual \"product\", \
            val \"feat_product\" } } } } } } }, \
    seq { \
      id { local str \"Seq_extend5_1\" } , \
      descr{  \
        title \"import gene locusA gene product\", \
        molinfo { \
            biomol peptide, \
            tech concept-trans } } , \
      inst{ \
            repr raw, \
            mol aa, \
            length 17, \
            seq-data ncbieaa \"MCEKTCAPPPFFFPGPPP\" } , \
      annot { \
        { \
        data \
        ftable{ \
            { \
            data \
             prot{ \
                name{ \"GR_protein name\" } } , \
            location \
                int{ \
                    from 0, \
                    to 16, \
                    id local str \"Seq_extend5_1\" } } } } } } } , \
 annot { \
  { \
   data \
    ftable{ \
      { \
        data \
        cdregion{ \
            code{ id 1 } } , \
            product \
              whole local str \"Seq_extend5_1\", \
            location \
                int { \
                    from 2, \
                    to 54, \
                    strand minus, \
                    id local str \"Seq_extend5\" } } } } } } \
}";

const char* sc_UpdSequence_Extend5_AlignWithGap = "\
Seq-entry ::= set { \
  class nuc-prot , \
  seq-set { \
   seq { \
    id { local str \"Seq_extend5\" } , \
    inst { \
     repr raw , \
     mol na , \
     length 92 , \
     seq-data iupacna \"\
GGGGGGGGGGCCCCGGAAAAAAAAAGGGGGGGGGGGCGCACGTTTTTTCACACAGGGGTGCAGTCGGGCAGCACGGGGAGCTTGCTCCCTGG\" } , \
   annot { \
    { \
     data \
      ftable{ \
       { \
        data \
         gene{ \
          locus \"import gene locusA\" }, \
        partial TRUE, \
        location \
          int { \
            from 2, \
            to 54, \
            strand plus, \
            id local str \"Seq_extend5\",  \
          fuzz-from lim lt, \
          fuzz-to lim gt } } , \
      { \
        data \
         imp { \
            key \"misc_feature\" } , \
        location \
          int{ \
            from 67, \
            to 86, \
            strand plus, \
            id local str \"Seq_extend5\" } , \
        qual{ \
          { \
            qual \"number\", \
            val \"1\" }, \
          { \
            qual \"product\", \
            val \"feat_product\" } } } } } } }, \
    seq { \
      id { local str \"Seq_extend5_1\" } , \
      descr{  \
        title \"import gene locusA gene product\", \
        molinfo { \
            biomol peptide, \
            tech concept-trans } } , \
      inst{ \
        repr raw, \
        mol aa, \
        length 17, \
        seq-data ncbieaa \"-GGPGKKKGGGAHVFSH\" } , \
      annot { \
        { \
         data \
          ftable{ \
            { \
            data \
              prot{ \
                name{ \"GR_protein name\" } } , \
            location \
                int{ \
                  from 0, \
                  to 16, \
                  id local str \"Seq_extend5_1\" } } } } } } } , \
 annot { \
  { \
    data \
     ftable{ \
        { \
        data \
         cdregion{ \
            code{ id 1 } } , \
        product \
            whole local str \"Seq_extend5_1\", \
        location \
            int { \
                from 2, \
                to 54, \
                strand plus, \
                id local str \"Seq_extend5\" } } } } } } \
}";

const char* sc_UpdSequence_Extend3 = "\
Seq-entry ::= set { \
 class nuc-prot , \
  seq-set { \
   seq { \
    id { local str \"Seq_extend3\" } , \
   inst { \
    repr raw , \
    mol na , \
    length 195, \
    seq-data iupacna \"\
TTGACATCCTGCGAATTTGGTAGAGATACCTTAGTGCCTTCGGGAGCGCAGTGACAGGTGCTGCATGGCTGTCGT\
CAGCTCGCGCGGGAGAAAGGGGTTTTTTTTTTTATATTATACCCCACCCCTCTCTCCCGGGGGGAGATTAGCCAC\
AGGGGTTTTTTTTTTTATATTATACCCCCCGGGGGGAGATTAGCC\" } , \
    annot{ \
     { \
      data \
       ftable{ \
        { \
        data \
         gene { \
            locus \"import extend3 gene locus\" ,\
            allele \"dominant\" } ,\
        location \
          int{ \
             from 4, \
             to 89, \
             strand plus, \
             id local str \"Seq_extend3\" } }, \
        { \
        data \
         imp{ \
            key \"misc_feature\" }, \
        location \
            mix{ \
             int{ \
                from 119, \
                to 131, \
                strand plus, \
                id local str \"Seq_extend3\" }, \
            pnt{ \
                point 139, \
                strand plus, \
                id local str \"Seq_extend3\" } }, \
        qual{ \
         { \
            qual \"number\", \
            val \"1\" }, \
         { \
            qual \"product\", \
            val \"ext3 misc_feat product\" } } } } } } }, \
     seq { \
     id{ \
        local str \"Seq_extend3_1\" }, \
     descr{ \
        title \"new test protein ext3\", \
        molinfo{ \
         biomol peptide, \
         tech concept-trans } }, \
     inst{ \
        repr raw, \
        mol aa, \
        length 25, \
        seq-data ncbieaa \"AQ*QVLHGCRQLAREKGVFFFILYP\" }, \
      annot{ \
        { \
        data \
        ftable{ \
          { \
            data \
             prot{ \
                name{ \"new test protein ext3\" } }, \
            location \
                int{ \
                    from 0, \
                    to 24, \
                    id local str \"Seq_extend3_1\" } } } } } } }, \
annot { \
 { \
 data \
    ftable{ \
     { \
        data \
        cdregion{ \
            frame two, \
            code{ id 1 } }, \
        product whole local str \"Seq_extend3_1\", \
        location \
         int{ \
            from 44, \
            to 119, \
            strand plus, \
            id local str \"Seq_extend3\" } } } } } } \
}";

