/*  $Id: test_cmd_change_seqfeat.cpp 45018 2020-05-07 20:25:09Z asztalos $
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
*
* File Description:
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/bioseq_handle.hpp>

//#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_promote_cds.hpp>
/*
#include <gui/core/undo_manager.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
*/
#include <cstdio>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>

//#include <objtools/unit_test_util/unit_test_util.hpp>

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

BOOST_AUTO_TEST_CASE(Test_RemoveCDS)
{
    CNcbiIfstream ifstr("matpep_added.asn");
    CRef <CSeq_entry> entry(new CSeq_entry());
    ifstr >> MSerial_AsnText >> *entry;
    ifstr.close();

    BOOST_REQUIRE(entry);
    CRef <CScope> scope(new CScope(*CObjectManager::GetInstance()));
    BOOST_REQUIRE(scope);
    scope->AddDefaults();
   
    CSeq_entry_Handle orig_seh = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(orig_seh);

    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->SetData().SetCdregion();
    new_feat->SetLocation().SetInt().SetFrom(0);
    new_feat->SetLocation().SetInt().SetTo(1700);
    new_feat->SetLocation().SetInt().SetStrand(eNa_strand_plus);
    CRef<CSeq_id> id(new CSeq_id);
    id->SetGenbank().SetAccession("KP662589");
    new_feat->SetLocation().SetId(id.GetObject());
    // it does not have a product id, but it's not important in this case

    CBioseq_CI b_iter(orig_seh);
    CBioseq_Handle bsh = *b_iter;
    CIRef<IEditCommand> add_cds(new CCmdCreateFeatBioseq(bsh, new_feat.GetObject()));
    add_cds->Execute();

    CConstRef<CSeq_entry> updated_entry = orig_seh.GetCompleteSeq_entry();
    //cout << "CDS added:\n" << MSerial_AsnText << updated_entry.GetObject() << endl;

    CFeat_CI feat_it(orig_seh, SAnnotSelector(CSeqFeatData::eSubtype_cdregion));
    CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
    CRef<CCmdPromoteCDS> promote_cmd(new CCmdPromoteCDS(fh));
    promote_cmd->Execute();
    
    updated_entry = orig_seh.GetCompleteSeq_entry();
    //cout << "CDS promoted:\n" << MSerial_AsnText << updated_entry.GetObject() << endl;
    
    /* This calls bool DemoteCDSToNucSeq(objects::CSeq_feat_Handle& orig_feat) from objtools/edit/cds_fix.cpp
    * I suspect that line
    * orig_feat = new_annot.TakeFeat(feh); causes the null pointer exception later on
    */
    promote_cmd->Unexecute();

    updated_entry = orig_seh.GetCompleteSeq_entry();
    //cout << "CDS demoted:\n" << MSerial_AsnText << updated_entry.GetObject() << endl;

    // this command fails with null pointer exception
    add_cds->Unexecute();

    updated_entry = orig_seh.GetCompleteSeq_entry();
    //cout << "CDS removed:\n" << MSerial_AsnText << updated_entry.GetObject() << endl;
    
}


/*
BOOST_AUTO_TEST_CASE(RunTests)
{
    CNcbiIfstream ifstr("testfile_cmdchange_seqfeat.asn");
    CRef <CSeq_entry> entry(new CSeq_entry());
    ifstr >> MSerial_AsnText >> *entry;
    ifstr.close();
    
    BOOST_REQUIRE(entry);
    CRef <CScope> scope(new CScope(*CObjectManager::GetInstance()));
    BOOST_REQUIRE(scope);
    CSeq_entry_Handle tse = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(tse);

    CBioseq_CI b_iter(tse);
    CRef<CSeq_loc> loc;
    if (b_iter) {
        loc = b_iter->GetRangeSeq_loc(0, 0);
    }
    
    cout << "Location:\n " << MSerial_AsnText << loc.GetObject() << endl;
    getchar();

    CFeat_CI feat_it(tse.GetScope(), *loc, SAnnotSelector(CSeqFeatData::eSubtype_any));
    BOOST_CHECK(feat_it.GetSize() == 1);

    CRef<CSeq_feat> new_feat(new CSeq_feat);
    new_feat->Assign(feat_it->GetOriginalFeature());
    new_feat->SetComment("New feature");
    const auto& seq_ids = b_iter->GetCompleteBioseq()->GetId();
    for (auto& it : seq_ids) {
        if (it->IsGenbank()) {
            CRef<CSeq_id> new_id(new CSeq_id);
            new_id->Assign(*it);
            new_feat->SetLocation().SetId(*new_id);
        }
    }
    
    cout <<  "New feature:\n " << MSerial_AsnText << *new_feat << endl;
    getchar();

    CRef< CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
    cmd->Execute();

    cout << "After changing the old feature with the new one:\n";

    CFeat_CI feat_it_after(tse.GetScope(), *loc, SAnnotSelector(CSeqFeatData::eSubtype_any));
    if (feat_it_after) {
        cout << "Feature found (with scope): " << MSerial_AsnText << feat_it_after->GetOriginalFeature() << endl;
    }
    else {
        cout << "No feature was found - with scope\n";
    }
    getchar();

    CFeat_CI feat_it_general(tse);
    if (feat_it_general) {
        cout << "Feature found (general):\n " << MSerial_AsnText << feat_it_general->GetOriginalFeature() << endl;
    }
    else {
        cout << "No feature was found - with general\n";
    }
    getchar();

}
*/

