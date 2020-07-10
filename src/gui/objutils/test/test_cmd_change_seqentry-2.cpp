/*  $Id: test_cmd_change_seqentry-2.cpp 42821 2019-04-18 19:32:56Z joukovv $
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
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/core/undo_manager.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>

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
    CNcbiIfstream ifstr("test_cmd_change_seqentry-2.asn");
    CRef <CSeq_entry> entry(new CSeq_entry());
    ifstr >> MSerial_AsnText >> *entry;
    ifstr.close();
    
    BOOST_REQUIRE(entry);
    CRef <CScope> scope(new CScope(*CObjectManager::GetInstance()));
    BOOST_REQUIRE(scope);
    CSeq_entry_Handle tse = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(tse);


    CFeat_CI mrna_ci(tse, SAnnotSelector(CSeqFeatData::eSubtype_mRNA));
    BOOST_REQUIRE(mrna_ci);

    CRef<CCmdComposite> cmd = GetDeleteFeatureCommand(*mrna_ci);

    BOOST_REQUIRE(cmd);
    CUndoManager mgr;
    mgr.Execute(cmd);

    CFlatFileConfig cfg;   
    cfg.SetNeverTranslateCDS();
    
    string str1;
    for (CFeat_CI feat_ci(tse, CSeqFeatData::eSubtype_regulatory); feat_ci; ++feat_ci)
    {
        str1 =  CFlatFileGenerator::GetSeqFeatText(*feat_ci, tse.GetScope(), cfg);
    }
    cout << "Before: " << str1 << endl;

    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);
    CRef<CCmdChangeSeqEntry> clean(new CCmdChangeSeqEntry(tse, copy));
    BOOST_REQUIRE(clean);
    mgr.Execute(clean);

    string str2;
    for (CFeat_CI feat_ci(tse, CSeqFeatData::eSubtype_regulatory); feat_ci; ++feat_ci)
    {
         str2 =  CFlatFileGenerator::GetSeqFeatText(*feat_ci, tse.GetScope(), cfg);
    }
    cout << "After: " << str2 << endl;
    BOOST_REQUIRE(str1 == str2);
}
