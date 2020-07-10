/*  $Id: test_taxtree.cpp 34642 2016-02-01 20:57:17Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>
#include <gui/widgets/tax_tree/tax_tree.hpp>
#include <gui/widgets/tax_tree/tax_tree_ds.hpp>

#include <objects/entrez2/entrez2_client.hpp>
#include <objects/entrez2/Entrez2_eval_boolean.hpp>
#include <objects/entrez2/Entrez2_boolean_exp.hpp>
#include <objects/entrez2/Entrez2_boolean_element.hpp>
#include <objects/entrez2/Entrez2_boolean_reply.hpp>
#include <objects/entrez2/Entrez2_id_list.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <gui/widgets/wx/wx_app.hpp>
#include "taxtree_testdlg.hpp"

USING_SCOPE(ncbi);
USING_SCOPE(ncbi::objects);

class CTestTreeApp : public CwxNCBIApp
{
protected:
    // we are overriding this function to create our main frame window
    virtual bool    x_CreateGUI();
};



// Declare wxWidgets application class
// this will allow CNCBIwxApplication to instantiate our class
DECLARE_APP(CTestTreeApp)
IMPLEMENT_APP_NO_MAIN(CTestTreeApp)
IMPLEMENT_WX_THEME_SUPPORT


bool CTestTreeApp::x_CreateGUI()
{
    CTaxTreeTestDlg dlg(NULL);  
    dlg.ShowModal();
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//  main entry point

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // create standard NCBI wxWidgets application
    ncbi::CNCBIwxApplication ncbi_app("wxWidgets demo App");

    // Execute main application function
    return ncbi_app.AppMain(argc, argv);
}
