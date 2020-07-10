/*  $Id: misc_fieldtype.cpp 41308 2018-07-03 19:19:16Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/edit/misc_fieldtype.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>


BEGIN_NCBI_SCOPE

vector<string> CMiscFieldType::GetStrings()
{
    vector<string> options;
    options.push_back(kDefinitionLineLabel);
    options.push_back(kCommentDescriptorLabel);
    options.push_back(kGenomeProjectID);
    options.push_back(kGenbankBlockKeyword);

    return options;
}

vector<string> CMiscFieldType::GetMacroFieldNames()
{
    vector<string> options;
    options.push_back("\"descr..title\"");
    options.push_back("\"descr..comment\"");
    options.push_back("\"obj.data.int\"");
    options.push_back("\"descr..genbank.keywords\"");
    return options;
}

END_NCBI_SCOPE

