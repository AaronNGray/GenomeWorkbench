/*  $Id: unit_test_cmd_del_bioseq.cpp 42821 2019-04-18 19:32:56Z joukovv $
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
* Author:  Igor Filippov, based on vcf reader unit test by Frank Ludwig, NCBI.
*
* File Description:
*   delete bioseq command unit test.
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/core/undo_manager.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <objmgr/util/sequence.hpp>

#include <cstdio>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>

#include <objtools/unit_test_util/unit_test_util.hpp>

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

BOOST_AUTO_TEST_CASE(RunTests)
{
    CRef<objects::CSeq_entry> entry = objects::unit_test_util::BuildGoodNucProtSet();    

    /*CNcbiIfstream ifstr("../../../../../../KF591393_stopcodons.asn");
    CRef <CSeq_entry> entry(new CSeq_entry());
    ifstr >> MSerial_AsnText >> *entry;
    ifstr.close();
    */
    BOOST_REQUIRE(entry);
    CRef <CScope> scope(new CScope(*CObjectManager::GetInstance()));
    BOOST_REQUIRE(scope);
    CSeq_entry_Handle tse = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(tse);

    CBioseq_CI nuc_ci(tse,CSeq_inst::eMol_na);
    BOOST_REQUIRE(nuc_ci);
    CBioseq_Handle bsh = *nuc_ci;
    BOOST_REQUIRE(bsh);
//    cout << "Before: " << bsh << endl;
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert Feature"));
    for (CFeat_CI feat_ci(tse,SAnnotSelector(CSeqFeatData::eSubtype_cdregion)); feat_ci; ++feat_ci)
    {
        BOOST_REQUIRE(feat_ci);

        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        new_feat->SetData().SetGene();

        CConstRef<CSeq_feat> gene = sequence::GetOverlappingGene(feat_ci->GetLocation(), *scope);
        if (gene)
        {
            CRef<CSeq_feat> copy_gene(new CSeq_feat());
            copy_gene->Assign(*gene);
            new_feat->SetData().SetGene(copy_gene->SetData().SetGene());
            cmd->AddCommand(*GetDeleteFeatureCommand(scope->GetSeq_featHandle(*gene)));
        }
        new_feat->SetPseudo(true);
        new_feat->ResetProduct();
        CBioseq_Handle bsh2 = scope->GetBioseqHandle(new_feat->GetLocation());
        CSeq_entry_Handle seh = bsh2.GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
        cmd->AddCommand(*GetDeleteFeatureCommand(*feat_ci));
    }
    BOOST_REQUIRE(cmd);
    CUndoManager mgr;
    mgr.Execute(cmd);
    cout << MSerial_AsnText << *entry;
    mgr.Undo();
//mgr.Redo();
//cout << MSerial_AsnText << *entry;

    cout << "After: " << bsh << endl;
  
}
