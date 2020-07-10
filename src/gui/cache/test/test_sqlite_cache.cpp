/*  $Id: test_sqlite_cache.cpp 39424 2017-09-21 20:49:19Z katargir $
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
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/feat_ci.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <serial/serial.hpp>
#include <serial/objostr.hpp>


USING_SCOPE(ncbi);
USING_SCOPE(objects);



class CTestCacheApp : public CNcbiApplication
{
public:

    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


void CTestCacheApp::Init(void)
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "SQLITE3 cache test app");

    arg_desc->AddKey("id", "SeqId",
                     "Accession to retrieve",
                     CArgDescriptions::eString);

    arg_desc->AddDefaultKey("o", "OutputFile",
                            "File for results",
                            CArgDescriptions::eOutputFile,
                            "-");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


int CTestCacheApp::Run(void)
{
    // Get arguments
    const CArgs& args = GetArgs();
    CNcbiOstream& ostr = args["o"].AsOutputFile();

    auto_ptr<CObjectOStream> os(CObjectOStream::Open(eSerial_AsnBinary, ostr));

    CStopWatch sw;
    sw.Start();
    {{
         CSeq_id id(args["id"].AsString());

         CRef<CObjectManager> om(CObjectManager::GetInstance());
         CGBDataLoader::RegisterInObjectManager(*om);

         CScope scope(*om);
         scope.AddDefaults();

         CBioseq_Handle bsh = scope.GetBioseqHandle(id);
         CFeat_CI feat_iter(bsh);
         for ( ;  feat_iter;  ++feat_iter) {
             *os << feat_iter->GetOriginalFeature();
         }
     }}
    LOG_POST(Error << "done, " << sw.Elapsed() << " second");

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CTestCacheApp::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestCacheApp().AppMain(argc, argv, 0, eDS_Default,
                                   _T_XCSTRING("test_sqlite_cache.ini"));
}
