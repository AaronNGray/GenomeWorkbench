/*  $Id: table_names.cpp 28266 2013-06-05 16:04:59Z wuliangs $
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
 * Authors: Robert Smith, Liangshou Wu
 *  Associate names with the standard score matrixes.
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>
#include <corelib/ncbi_safe_static.hpp>
#include <corelib/ncbimtx.hpp>
#include <gui/utils/table_names.hpp>
#include <util/static_map.hpp>

BEGIN_NCBI_SCOPE

typedef SStaticPair<const char*, const SNCBIPackedScoreMatrix *> TNameTablePair;
static const TNameTablePair s_TableNames[] = {
    {"Blosum 45", &NCBISM_Blosum45},
    {"Blosum 62", &NCBISM_Blosum62},
    {"Blosum 80", &NCBISM_Blosum80},
    {"PAM 30", &NCBISM_Pam30},
    {"PAM 70", &NCBISM_Pam70},
    {"PAM250", &NCBISM_Pam250},
};

typedef CStaticArrayMap<const char*, const SNCBIPackedScoreMatrix *, PNocase_CStr> TNameTableMap;
DEFINE_STATIC_ARRAY_MAP(TNameTableMap, s_NameTableMap, s_TableNames);


string  CTableNames::TableToName(const SNCBIPackedScoreMatrix *mp)
{
    const TNameTableMap& tnmap = s_NameTableMap;
    ITERATE(TNameTableMap, iter, tnmap) {
        if (mp == iter->second) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


const SNCBIPackedScoreMatrix *  CTableNames::NameToTable(const string& name)
{
    const TNameTableMap& tnmap = s_NameTableMap;
    TNameTableMap::const_iterator iter = tnmap.find(name.c_str());
    if (iter == tnmap.end())
        return NULL;
    return iter->second;
}


vector<string>  CTableNames::AllTableNames()
{
    const TNameTableMap& tnmap = s_NameTableMap;
    vector<string> names;
    ITERATE(TNameTableMap, iter, tnmap) {
        names.push_back(iter->first);
    }
    return names;
}

END_NCBI_SCOPE
