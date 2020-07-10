/*
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
*   Government have not placed any restriction on its use or reproduction.
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
* Author:  Vladislav Evgeniev
*
* File Description: CGuiEutilsClient implementatoin
*
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <connect/ncbi_conn_test.hpp>
#include <gui/objutils/gui_eutils_client.hpp>

using namespace std;

BEGIN_NCBI_SCOPE

struct SEutilsParams
{
    string  tool;
    string  email;
    string  api_key;
    SEutilsParams();
};

SEutilsParams::SEutilsParams()
{
    const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
    tool = reg.GetString("eutils", "tool", "ncbi-guitools");
    email = reg.GetString("eutils", "email", "cpp-gui-core@ncbi.nlm.nih.gov");
    if (!CConnTest::IsNcbiInhouseClient()) {
        api_key = reg.GetEncryptedString("eutils", "api_key", 0, "NCBI");
    }
}

CGuiEutilsClient::CGuiEutilsClient()
{
    static SEutilsParams params;
    AddParameter("tool", params.tool);
    AddParameter("email", params.email);
    if (!params.api_key.empty()) {
        AddParameter("api_key", params.api_key);
    }
}

END_NCBI_SCOPE
