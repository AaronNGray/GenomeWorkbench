/*  $Id: test_cmd_change_seqentry.cpp 35699 2016-06-10 20:21:15Z asztalos $
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
#include <corelib/ncbiapp.hpp>
#include <serial/objistr.hpp>
#include <serial/typeinfo.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>


USING_NCBI_SCOPE;
USING_SCOPE(objects);


class CTestChangeEntryApp : public CNcbiApplication
{
public:

    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);

    void Add_RemoveFeature();

    CRef<CCmdComposite> SimpleChangeCommand(CSeq_entry_Handle orig_seh);
    
};

void CTestChangeEntryApp::Init(void)
{
    SetDiagPostFlag(eDPF_All);
    SetDiagPostLevel(eDiag_Info);

    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                "Testing CCmdChangeSeqEntry command");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


CRef<CCmdComposite> CTestChangeEntryApp::SimpleChangeCommand(CSeq_entry_Handle orig_seh)
{

    CConstRef<CSeq_entry> entry = orig_seh.GetCompleteSeq_entry();
    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);

    // add a descriptor outside of the scope
    CRef<CSeqdesc> desc_outscope(new CSeqdesc);
    desc_outscope->SetTitle("new_title_outside_scope");
    copy->SetDescr().Set().push_back(desc_outscope);

    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    CScope scope2(*objmgr);
    scope2.AddDefaults();
    CSeq_entry_Handle new_seh = scope2.AddTopLevelSeqEntry(*copy);

    // add a descriptor inside of the scope
    CRef<CSeqdesc> desc_inscope(new CSeqdesc);
    desc_inscope->SetTitle("new_title_in_scope");
    new_seh.GetEditHandle().AddSeqdesc(*desc_inscope);

    CRef<CCmdComposite> simple(new CCmdComposite("Simple command"));
    CRef<CCmdChangeSeqEntry> clean(new CCmdChangeSeqEntry(orig_seh, copy));
    simple->AddCommand(*clean);

    return simple;
}


void CTestChangeEntryApp::Add_RemoveFeature()
{
    CRef<CSeq_entry> entry(new CSeq_entry);
    const string fname("addfeature.asn");

    try {
        CNcbiIfstream istr(fname.c_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *entry;
    }
    catch (const CException& e)  {
        LOG_POST(Error << "Could not read seq-entry from file: " << fname);
        return;
    }

    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();
    CSeq_entry_Handle tse = scope->AddTopLevelSeqEntry(*entry);
    _ASSERT(tse);

    CBioseq_CI b_iter(tse, CSeq_inst::eMol_na);
    CBioseq_Handle bsh;
    if (b_iter) {
        bsh = *b_iter;
    }

    if (!bsh) {
        LOG_POST(Error << "Could not find nucleotide sequence in the seq-entry");
        return;
    }

    ///
    /// add one feature to the sequence
    ///
    CConstRef<CSeq_id> seqid(bsh.GetCompleteBioseq()->GetFirstId());
    CRef<CSeq_feat> feat(new CSeq_feat);
    feat->SetLocation().SetInt().SetId().Assign(*seqid);
    feat->SetLocation().SetInt().SetFrom(0);
    feat->SetLocation().SetInt().SetTo(30);
    feat->SetData().SetImp().SetKey("misc_feature");

    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CRef<CCmdCreateFeat> create_feat_cmd(new CCmdCreateFeat(seh, *feat));
    if (create_feat_cmd) {
        create_feat_cmd->Execute();
        cout << "After adding a feature\n";
        cout << MSerial_AsnText << *tse.GetCompleteSeq_entry();
    }

    ///
    /// form the change seq-entry command
    ///
    CRef<CCmdComposite> cleanup_cmd = SimpleChangeCommand(tse);
    if (cleanup_cmd) {
        cleanup_cmd->Execute();
        cout << "After executing the simple change command\n";
        cout << MSerial_AsnText << *tse.GetCompleteSeq_entry();
    }

    _ASSERT(tse);

    ///
    /// The original bsh is removed and becomes invalid and it needs to be reset if we want 
    /// to use it later
    ///
    _ASSERT(bsh.IsRemoved());
    _ASSERT(!bsh);

    // we can reset the bsh from the top seq-entry  - it will point to the new data

    //bsh = tse.GetSeq();  // reset the bsh from the top seq-entry
    
    // OR by calling the unexecute command - it will point to the original data
    
    cleanup_cmd->Unexecute();

    _ASSERT(bsh);
    _ASSERT(!bsh.IsRemoved());
    cout << "Current bioseq: " << bsh.GetCompleteBioseq().GetObject();


    ///
    /// add another feature to the sequence
    ///
    CRef<CSeq_feat> feat2(new CSeq_feat);
    feat2->SetLocation().SetInt().SetId().Assign(*seqid);
    feat2->SetLocation().SetInt().SetFrom(40);
    feat2->SetLocation().SetInt().SetTo(60);
    feat2->SetData().SetImp().SetKey("misc_feature");

    CSeq_entry_Handle seh2 = bsh.GetSeq_entry_Handle();
    CRef<CCmdCreateFeat> create_feat_cmd2(new CCmdCreateFeat(seh2, *feat2));
    if (create_feat_cmd2) {
        create_feat_cmd2->Execute();
        cout << "After adding the second feature\n";
        cout << MSerial_AsnText << *tse.GetCompleteSeq_entry();
    }
    

    ///
    /// Unexecute the commands
    ///
    cout << "Unexecute the commands:\n---------------------------------------------------\n";
    create_feat_cmd2->Unexecute();
    cout << "After removing the second feature\n";
    cout << MSerial_AsnText << *tse.GetCompleteSeq_entry();

    /*
    cleanup_cmd->Unexecute();
    cout << "After undoing the simple change command\n";
    cout << MSerial_AsnText << *tse.GetCompleteSeq_entry();

    ///
    /// again, the bsh is invalid and it needs to be reset for further use
    ///
    _ASSERT(!bsh);
    bsh = seh.GetSeq();
    cout << MSerial_AsnText << *bsh.GetCompleteBioseq();

    create_feat_cmd->Unexecute();
    cout << "After removing the first feature\n";
    cout << MSerial_AsnText << *tse.GetCompleteSeq_entry();
    */
}

int CTestChangeEntryApp::Run(void)
{
    Add_RemoveFeature();
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CTestChangeEntryApp::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    CTestChangeEntryApp testApp;
    return testApp.AppMain(argc, argv);
}
