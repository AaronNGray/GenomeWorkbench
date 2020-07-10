/*  $Id: test_getalignments.cpp 34698 2016-02-01 22:25:58Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *    Test application for plugins
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbireg.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/mate_pair.hpp>
#include <gui/objutils/alignment.hpp>
#include <gui/objutils/pw_alignment.hpp>
#include <gui/objutils/label.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objects/seqalign/Seq_align.hpp>

#include <serial/objostr.hpp>
#include <serial/serial.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);


class CTestGetAlignments : public CNcbiApplication
{
public:

    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};



void CTestGetAlignments::Init(void)
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Test: CSeqUtils::GetAlignments()");

    arg_desc->AddKey("acc", "Accession",
                     "Accession to test",
                     CArgDescriptions::eString);

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());

}


int CTestGetAlignments::Run(void)
{
    // Get arguments
    CArgs args = GetArgs();

    string id_str = args["acc"].AsString();
    CSeq_id id;
    try {
        id.Set(id_str);
    }
    catch (CSeqIdException&) {
        LOG_POST(Fatal << "can't interpret id = " << id_str);
    }

    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*obj_mgr);

    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    CBioseq_Handle handle = scope->GetBioseqHandle(id);
    if ( !handle ) {
        LOG_POST(Fatal << "failed to retrieve sequence for: " << id_str);
    }


    SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    CLayoutEngine::TObjects objs;
    CSeqUtils::GetAlignments(handle,
                             TSeqRange(0, handle.GetBioseqLength()),
                             sel,
                             objs);

    cout << "got " << objs.size() << " alignments:" << endl;
    auto_ptr<CObjectOStream> os(CObjectOStream::Open(eSerial_AsnText, cout));
    ITERATE (CLayoutEngine::TObjects, iter, objs) {
        const CLayoutObject& obj = **iter;
        if (dynamic_cast<const CLayoutPWAlign*>(&obj)) {
            const CLayoutPWAlign& pwalign =
                dynamic_cast<const CLayoutPWAlign&>(obj);
            const CSeq_align& align = pwalign.GetAlignment();

            string label;
            CLabel::GetLabel(align, &label, CLabel::eDefault, scope);
            cout << string(72, '-') << endl;
            cout << "pairwise alignment: " << label << endl;
            *os << align;

        } else if (dynamic_cast<const CLayoutMatePair*>(&obj)) {
            const CLayoutMatePair& mpalign =
                dynamic_cast<const CLayoutMatePair&>(obj);
            const CLayoutMatePair::TAlignList& aligns =
                mpalign.GetSeqAligns();

            cout << string(72, '-') << endl;
            cout << "mate pair alignment: " << aligns.size()
                << " alignments" << endl;
            cout << "mate pair library: " << mpalign.GetLibraryId() << endl;
            cout << "mate pair error code: " << mpalign.GetError() << endl;
            ITERATE (CLayoutMatePair::TAlignList, iter, aligns) {
                *os << (*iter)->GetAlignment();
            }

        } else if (dynamic_cast<const CLayoutAlign*>(&obj)) {
            const CLayoutAlign& aobj =
                dynamic_cast<const CLayoutAlign&>(obj);
            const CSeq_align& align = aobj.GetAlignment();

            string label;
            CLabel::GetLabel(align, &label, CLabel::eDefault, scope);
            cout << string(72, '-') << endl;
            cout << "pairwise alignment: " << label << endl;
            *os << align;

        } else {
            LOG_POST(Error << "unhandled alignment object");
        }
    }


    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CTestGetAlignments::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestGetAlignments().AppMain(argc, argv);
}
