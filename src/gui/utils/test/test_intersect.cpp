/*  $Id: test_intersect.cpp 34670 2016-02-01 21:45:16Z katargir $
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
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/utils/intersect.hpp>


USING_SCOPE(ncbi);

/////////////////////////////////////////////////////////////////////////////
//  CTestIntersectApp::


class CTestIntersectApp : public CNcbiApplication
{
public:
    CTestIntersectApp();

private:
    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


CTestIntersectApp::CTestIntersectApp()
{
}


/////////////////////////////////////////////////////////////////////////////
//  Init test for all different types of arguments


void CTestIntersectApp::Init(void)
{
    // Create command - line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Math library test app");

    arg_desc->AddDefaultKey("iters", "Iterations",
                            "Number of iterations for timing tests",
                            CArgDescriptions::eInteger,
#ifdef _DEBUG
                            "5"
#else
                            "500"
#endif
                           );

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());

}



/////////////////////////////////////////////////////////////////////////////


int CTestIntersectApp::Run(void)
{
    // Get arguments
    //CArgs args = GetArgs();
    //size_t iters = args["iters"].AsInteger();

    CVect3<float> p0(0.0f, 0.0f, 0.0f);
    CVect3<float> p1(1.0f, 0.0f, 0.0f);
    CVect3<float> p2(0.0f, 1.0f, 0.0f);

    CVect3<float> orig(0.25f, 0.25f, 1.0f);
    CVect3<float> dir (0.0f, 0.0f, -2.0f);

    CVect3<float> pt;

    if (math::IntersectRayTriangle(orig, dir, p0, p1, p2, pt) == math::eIntersects) {
        cout << "intersection found!" << endl;
    } else {
        cout << "no intersection" << endl;
    }

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CTestIntersectApp::Exit(void)
{
    SetDiagStream(0);
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestIntersectApp().AppMain(argc, argv);
}
