/*  $Id: ini2reg.cpp 39425 2017-09-21 20:50:30Z katargir $
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
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>
#include <gui/objutils/registry.hpp>

BEGIN_NCBI_SCOPE


class CTestGuiRegistryApp : public CNcbiApplication
{
private:
    void Init();
    int Run();
};


void CTestGuiRegistryApp::Init()
{
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions());

    arg_desc->AddDefaultKey("ini", "IniFile",
                            "Ini File to convert",
                            CArgDescriptions::eInputFile,
                            "-");

    arg_desc->AddDefaultKey("reg", "RegistryFile",
                            "Registry File to create",
                            CArgDescriptions::eOutputFile,
                            "-");

    SetupArgDescriptions(arg_desc.release());
}


int CTestGuiRegistryApp::Run()
{
    const CArgs& args = GetArgs();
    CNcbiIstream& istr = args["ini"].AsInputFile();
    CNcbiOstream& ostr = args["reg"].AsOutputFile();

    CGuiRegistry reg;
    reg.FromConfigFile(istr);
    reg.Write(ostr);

    return 0;
}


END_NCBI_SCOPE
USING_SCOPE(ncbi);


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return CTestGuiRegistryApp().AppMain(argc, argv);
}
