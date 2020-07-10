/*  $Id: unit_test_cmd_change_desc.cpp 42025 2018-12-04 21:29:15Z asztalos $
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
*   Unit test for change descriptor command
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <objtools/unit_test_util/unit_test_util.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

BOOST_AUTO_TEST_CASE(Test_ChangeDescriptor_Command)
{
    CRef<CSeq_entry> entry = unit_test_util::BuildGoodNucProtSet();
    BOOST_REQUIRE(entry);

    // add create_date descriptor 
    CRef<CSeqdesc> crdate_desc(new CSeqdesc());
    crdate_desc->SetCreate_date().SetStr("2018-09-12");
    if (entry->IsSet()) {
        entry->SetSet().SetDescr().Set().push_back(crdate_desc);
    }

    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(seh);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle nuc_bsh = *b_iter;
    BOOST_REQUIRE(nuc_bsh);

    // change biosource
    {
        CSeqdesc_CI desc_it(nuc_bsh, CSeqdesc::e_Source);
        const CSeqdesc& orig_desc = *desc_it;
        CSeq_entry_Handle parent_seh = desc_it.GetSeq_entry_Handle();

        const CBioSource& orig_bsrc = desc_it->GetSource();

        CRef<CBioSource> edit_bsrc(new CBioSource);
        edit_bsrc->Assign(orig_bsrc);
        edit_bsrc->SetOrg().SetCommon("common name");

        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->SetSource(*edit_bsrc);

        CCmdChangeSeqdesc chg_bsrc(parent_seh, orig_desc, *new_desc);
        chg_bsrc.Execute();

        auto it = parent_seh.GetDescr().Get().begin();
        BOOST_REQUIRE((*it)->IsSource());
        BOOST_CHECK((*it)->GetSource().IsSetCommon());

        chg_bsrc.Unexecute();

        it = parent_seh.GetDescr().Get().begin();
        BOOST_REQUIRE((*it)->IsSource());
        BOOST_CHECK(!(*it)->GetSource().IsSetCommon());
    }

    // change publication
    {
        CSeqdesc_CI desc_it(nuc_bsh, CSeqdesc::e_Pub);
        const CSeqdesc& orig_desc = *desc_it;
        CSeq_entry_Handle parent_seh = desc_it.GetSeq_entry_Handle();

        const CPubdesc& pubdesc = orig_desc.GetPub();
        BOOST_REQUIRE(pubdesc.GetPub().Get().size() == 1);
        const CPub& pub = *pubdesc.GetPub().Get().front();
        BOOST_REQUIRE(pub.IsPmid());

        CRef<CPubdesc> new_pubdesc(new CPubdesc);
        new_pubdesc->Assign(pubdesc);
        new_pubdesc->SetPub().Set().front()->SetPmid().Set(123);

        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->SetPub(*new_pubdesc);

        CCmdChangeSeqdesc chg_pub(parent_seh, orig_desc, *new_desc);
        chg_pub.Execute();

        auto it = parent_seh.GetDescr().Get().begin();
        advance(it, 1);
        BOOST_REQUIRE((*it)->IsPub());
        BOOST_CHECK((*it)->GetPub().GetPub().Get().front()->GetPmid().Get() == 123);

        chg_pub.Unexecute();

        it = parent_seh.GetDescr().Get().begin();
        advance(it, 1);
        BOOST_REQUIRE((*it)->IsPub());
        BOOST_CHECK((*it)->GetPub().GetPub().Get().front()->GetPmid().Get() == 1);
    }

    // change create_date
    {
        CSeqdesc_CI desc_it(nuc_bsh, CSeqdesc::e_Create_date);

        const CSeqdesc& orig_desc = *desc_it;
        CSeq_entry_Handle parent_seh = desc_it.GetSeq_entry_Handle();

        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->Assign(orig_desc);
        new_desc->SetCreate_date().SetStr("2019-01-01");

        CCmdChangeSeqdesc chg_date(parent_seh, orig_desc, *new_desc);
        chg_date.Execute();

        auto it = parent_seh.GetDescr().Get().end();
        --it;
        BOOST_REQUIRE((*it)->IsCreate_date());
        BOOST_CHECK((*it)->GetCreate_date().GetStr() == "2019-01-01");

        chg_date.Unexecute();

        it = parent_seh.GetDescr().Get().end();
        --it;
        BOOST_REQUIRE((*it)->IsCreate_date());
        BOOST_CHECK((*it)->GetCreate_date().GetStr() == "2018-09-12");
    }

    // change molinfo
    {
        CSeqdesc_CI desc_it(nuc_bsh, CSeqdesc::e_Molinfo);

        const CSeqdesc& orig_desc = *desc_it;
        CSeq_entry_Handle parent_seh = desc_it.GetSeq_entry_Handle();

        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->Assign(orig_desc);
        new_desc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_complete);

        CCmdChangeSeqdesc chg_molinfo(parent_seh, orig_desc, *new_desc);
        chg_molinfo.Execute();

        auto it = parent_seh.GetDescr().Get().begin();
        BOOST_REQUIRE((*it)->IsMolinfo());
        BOOST_CHECK((*it)->GetMolinfo().IsSetCompleteness());

        chg_molinfo.Unexecute();

        it = parent_seh.GetDescr().Get().begin();
        BOOST_REQUIRE((*it)->IsMolinfo());
        BOOST_CHECK(!(*it)->GetMolinfo().IsSetCompleteness());
    }
}

BOOST_AUTO_TEST_CASE(Test_ChangeDescriptor_CommandInaLoop)
{
    CRef<CSeq_entry> entry = unit_test_util::BuildGoodNucProtSet();
    BOOST_REQUIRE(entry);

    // add create_date descriptor 
    CRef<CSeqdesc> crdate_desc(new CSeqdesc());
    crdate_desc->SetCreate_date().SetStr("2018-09-12");
    if (entry->IsSet()) {
        entry->SetSet().SetDescr().Set().push_back(crdate_desc);
    }

    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(seh);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle nuc_bsh = *b_iter;
    BOOST_REQUIRE(nuc_bsh);

    CRef<CCmdComposite> cmd(new CCmdComposite("Change descriptors"));
    for (CSeqdesc_CI desc_it(nuc_bsh); desc_it; ++desc_it) {
        const CSeqdesc& orig_desc = *desc_it;
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->Assign(orig_desc);
        if (orig_desc.IsSource()) {
            new_desc->SetSource().SetOrg().SetCommon("common name");
        }
        else if (orig_desc.IsPub() && orig_desc.GetPub().GetPub().Get().front()->IsPmid()) {
            new_desc->SetPub().SetPub().Set().front()->SetPmid().Set(123);
        }
        else if (orig_desc.IsCreate_date()) {
            new_desc->SetCreate_date().SetStr("2019-01-01");
        }
        else if (orig_desc.IsMolinfo()) {
            new_desc->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_complete);
        }
        CRef<CCmdChangeSeqdesc> cmd_chg(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), orig_desc, *new_desc));
        cmd->AddCommand(*cmd_chg);
    }

    if (cmd) {
        cmd->Execute();
    }

    for (CSeqdesc_CI desc_it(nuc_bsh); desc_it; ++desc_it) {
        if (desc_it->IsSource()) {
            BOOST_CHECK(desc_it->GetSource().IsSetCommon());
        }
        else if (desc_it->IsPub() && desc_it->GetPub().GetPub().Get().front()->IsPmid()) {
            BOOST_CHECK(desc_it->GetPub().GetPub().Get().front()->GetPmid().Get() == 123);
        }
        else if (desc_it->IsCreate_date()) {
            BOOST_CHECK(desc_it->GetCreate_date().GetStr() == "2019-01-01");
        }
        else if (desc_it->IsMolinfo()) {
            BOOST_CHECK(desc_it->GetMolinfo().IsSetCompleteness());
        }
    }
}


