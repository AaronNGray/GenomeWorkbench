/*  $Id: test_convert_raw_to_delta.cpp 43537 2019-07-25 14:57:43Z filippov $
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
* Author:  Igor Filippov
*
* File Description:
*   DOI lookup unit test.
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>
#include <cstdio>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>


USING_NCBI_SCOPE;
USING_SCOPE(objects);

NCBITEST_AUTO_INIT()
{
}


NCBITEST_INIT_CMDLINE(arg_descrs)
{   
}

NCBITEST_AUTO_FINI()
{
}

CRef<objects::CSeq_entry> BuildGoodSeq(void)
{
    CRef<objects::CSeq_entry> entry(new objects::CSeq_entry());
    entry->SetSeq().SetInst().SetMol(objects::CSeq_inst::eMol_dna);
    entry->SetSeq().SetInst().SetRepr(objects::CSeq_inst::eRepr_raw);
    string seq;
    seq = string(20, 'A') + string (20, 'N') + string(20, 'A');
    entry->SetSeq().SetInst().SetSeq_data().SetIupacna().Set(seq);
    entry->SetSeq().SetInst().SetLength(seq.length());

    CRef<objects::CSeq_id> id(new objects::CSeq_id());
    id->SetLocal().SetStr ("good");
    entry->SetSeq().SetId().push_back(id);

    return entry;
}

CRef<objects::CSeq_feat> MakeMrnaFeature(size_t left_end, size_t right_end)
{
    CRef<objects::CSeq_feat> feat(new objects::CSeq_feat());
    CRef<objects::CSeq_id> id(new objects::CSeq_id());
    id->SetLocal().SetStr ("good");
    feat->SetLocation().SetInt().SetId().Assign(*id);
    feat->SetLocation().SetInt().SetFrom(left_end);
    feat->SetLocation().SetInt().SetTo(right_end);
    //feat->SetData().SetImp().SetKey("misc_feature");
    feat->SetData().SetRna().SetType(objects::CRNA_ref::eType_mRNA);
    return feat;
}

CRef<objects::CSeq_annot> AddFeat (CRef<objects::CSeq_feat> feat, CRef<objects::CSeq_entry> entry)
{
    CRef<objects::CSeq_annot> annot;

    if (entry->IsSeq()) {
        if (!entry->GetSeq().IsSetAnnot() 
            || !entry->GetSeq().GetAnnot().front()->IsFtable()) {
            CRef<objects::CSeq_annot> new_annot(new objects::CSeq_annot());
            entry->SetSeq().SetAnnot().push_back(new_annot);
            annot = new_annot;
        } else {
            annot = entry->SetSeq().SetAnnot().front();
        }
    } else if (entry->IsSet()) {
        if (!entry->GetSet().IsSetAnnot() 
            || !entry->GetSet().GetAnnot().front()->IsFtable()) {
            CRef<objects::CSeq_annot> new_annot(new objects::CSeq_annot());
            entry->SetSet().SetAnnot().push_back(new_annot);
            annot = new_annot;
        } else {
            annot = entry->SetSet().SetAnnot().front();
        }
    }
    annot->SetData().SetFtable().push_back(feat);
    return annot;   

}

BOOST_AUTO_TEST_CASE(testFeatureBeforeGap)
{
    CRef<CSeq_entry> entry = BuildGoodSeq();
    CRef<CSeq_feat> feat = MakeMrnaFeature(10,15);
    AddFeat(feat, entry);
    
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));;
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);  
    bool remove_alignments(false);
    int count = 0;
    CRef<CCmdComposite> cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(seh, 10, 50, 100, 100, true, false, remove_alignments, count);
    cmd->Execute();
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetFrom(), 10);
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetTo(), 15);
}

BOOST_AUTO_TEST_CASE(testFeatureAfterGap)
{
    CRef<CSeq_entry> entry = BuildGoodSeq();
    CRef<CSeq_feat> feat = MakeMrnaFeature(45,55);
    AddFeat(feat, entry);
    
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));;
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);  
    bool remove_alignments(false);
    int count = 0;
    CRef<CCmdComposite> cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(seh, 10, 50, 100, 100, true, false, remove_alignments, count);
    cmd->Execute();

    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetFrom(), 125);
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetTo(), 135);
}

BOOST_AUTO_TEST_CASE(testFeatureInsideGap)
{
    CRef<CSeq_entry> entry = BuildGoodSeq();
    CRef<CSeq_feat> feat = MakeMrnaFeature(25,35);
    AddFeat(feat, entry);
    
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));;
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);  
    bool remove_alignments(false);
    int count = 0;
    CRef<CCmdComposite> cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(seh, 10, 50, 100, 100, true, false, remove_alignments, count);
    cmd->Execute();
    BOOST_CHECK(!seh.GetCompleteSeq_entry()->GetSeq().IsSetAnnot());
}

BOOST_AUTO_TEST_CASE(testFeatureOverlapGap)
{
    CRef<CSeq_entry> entry = BuildGoodSeq();
    CRef<CSeq_feat> feat = MakeMrnaFeature(10,50);
    AddFeat(feat, entry);
    
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));;
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);  
    bool remove_alignments(false);
    int count = 0;
    CRef<CCmdComposite> cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(seh, 10, 50, 100, 100, true, false, remove_alignments, count);
    cmd->Execute();
    //cout << MSerial_AsnText << *seh.GetCompleteSeq_entry();
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetFrom(), 10);
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetTo(), 19);
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().back()->GetLocation().GetInt().GetFrom(), 120);
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().back()->GetLocation().GetInt().GetTo(), 130);
}

BOOST_AUTO_TEST_CASE(testFeatureBeforeGapOverlap)
{
    CRef<CSeq_entry> entry = BuildGoodSeq();
    CRef<CSeq_feat> feat = MakeMrnaFeature(10,25);
    AddFeat(feat, entry);
    
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));;
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);  
    bool remove_alignments(false);
    int count = 0;
    CRef<CCmdComposite> cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(seh, 10, 50, 100, 100, true, false, remove_alignments, count);
    cmd->Execute();
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetFrom(), 10);
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetTo(), 19);
}

BOOST_AUTO_TEST_CASE(testFeatureAfterGapOverlap)
{
    CRef<CSeq_entry> entry = BuildGoodSeq();
    CRef<CSeq_feat> feat = MakeMrnaFeature(35,55);
    AddFeat(feat, entry);
    
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));;
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);  
    bool remove_alignments(false);
    int count = 0;
    CRef<CCmdComposite> cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(seh, 10, 50, 100, 100, true, false, remove_alignments, count);
    cmd->Execute();

    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetFrom(), 120);
    BOOST_CHECK_EQUAL(seh.GetCompleteSeq_entry()->GetSeq().GetAnnot().front()->GetData().GetFtable().front()->GetLocation().GetInt().GetTo(), 135);
}
