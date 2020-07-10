/*  $Id: dblink_fieldtype.cpp 39649 2017-10-24 15:22:12Z asztalos $
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

#include <gui/widgets/edit/dblink_fieldtype.hpp>


BEGIN_NCBI_SCOPE

vector<string> CDBLinkFieldType::GetFieldNames()
{
    vector<string> options;

    for (int field_type = eDBLinkFieldType_Trace; field_type < eDBLinkFieldType_Unknown; field_type++) {
        string field_name = GetLabelForType((EDBLinkFieldType)field_type);
        NormalizeDBLinkFieldName(field_name);
        options.push_back(field_name);
    }

    return options;
}

CDBLinkFieldType::EDBLinkFieldType CDBLinkFieldType::GetTypeForLabel(string label)
{
    NormalizeDBLinkFieldName(label);
    for (int i = eDBLinkFieldType_Trace; i < eDBLinkFieldType_Unknown; i++) {
        string match = GetLabelForType((EDBLinkFieldType)i);
        if (NStr::EqualNocase(label, match)) {
            return (EDBLinkFieldType)i;
        }
    }
    return eDBLinkFieldType_Unknown;
}

string CDBLinkFieldType::GetLabelForType(CDBLinkFieldType::EDBLinkFieldType field_type)
{
    string rval = "";
    switch (field_type) {
    case eDBLinkFieldType_Trace:
        rval = "Trace Assembly Archive";
        break;
    case eDBLinkFieldType_BioSample:
        rval = "BioSample";
        break;
    case eDBLinkFieldType_ProbeDB:
        rval = "ProbeDB";
        break;
    case eDBLinkFieldType_SRA:
        rval = "Sequence Read Archive";
        break;
    case eDBLinkFieldType_BioProject:
        rval = "BioProject";
        break;
    case eDBLinkFieldType_Assembly:
        rval = "Assembly";
        break;
    case eDBLinkFieldType_Unknown:
        break;
    }
    return rval;
}

//bool CDBLinkFieldType::IsDBLink(const CUser_object& user);
void CDBLinkFieldType::NormalizeDBLinkFieldName(string& orig_label)
{
    if (NStr::StartsWith(orig_label, "DBLink ")) {
        orig_label = orig_label.substr(7);
    }
}

END_NCBI_SCOPE

