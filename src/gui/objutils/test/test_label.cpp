/*  $Id: test_label.cpp 34690 2016-02-01 22:15:38Z katargir $
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
#include <gui/objutils/label.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_interval.hpp>


USING_SCOPE(ncbi);
USING_SCOPE(objects);



class CTestLabelApp : public CNcbiApplication
{
public:

    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


void CTestLabelApp::Init(void)
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "CLabel label interface test application");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


int CTestLabelApp::Run(void)
{
    // Get arguments
    CArgs args = GetArgs();

    CSeq_loc loc;
    loc.SetInt().SetFrom(0);
    loc.SetInt().SetTo(100);

    CRef<CSeq_id> id(new CSeq_id("gi|12345"));
    loc.SetId(*id);

    string type_label;
    string content_label;
    string both_label;
    CLabel::GetLabel(loc, &type_label,    CLabel::eType);
    CLabel::GetLabel(loc, &content_label, CLabel::eContent);
    CLabel::GetLabel(loc, &both_label,    CLabel::eUserTypeAndContent);

    cout << "type:    " << type_label << endl;
    cout << "content: " << content_label << endl;
    cout << "both:    " << both_label << endl;

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CTestLabelApp::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestLabelApp().AppMain(argc, argv);
}
