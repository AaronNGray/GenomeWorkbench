/*  $Id: string_matching.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
 * Author:  Vladislav Evgeniev
 *
 * File Description: 
 * Provides a class that supports several text matching algorithms like plain text search, 
 * wildcard matching, regexp or Metaphone search.
 *
 */

#include <ncbi_pch.hpp>

#include <assert.h>
#include <util/dictionary_util.hpp>
#include <gui/objutils/string_matching.hpp>

using namespace std;

BEGIN_NCBI_SCOPE

CStringMatching::CStringMatching(const CTempString &pattern, CStringMatching::EStringMatching string_matching, NStr::ECase use_case) :
    m_Pattern(pattern),
    m_MatchingAlgorithm(string_matching),
    m_UseCase(use_case)
{
    switch (m_MatchingAlgorithm) {
        case ePlainSearch   :
        case eWildcardMatch :
            break;
        case eRegex         :
            m_Regex.reset(new CRegexp(m_Pattern, (NStr::eCase == m_UseCase) ? CRegexp::fCompile_default : CRegexp::fCompile_ignore_case));
            break;
        case eMetaphone     :
            {
                m_MetaphonePattern.reset(new TStringVector);
                NStr::Split(m_Pattern, CTempString(" "), *m_MetaphonePattern.get());
                string sTemp;
                NON_CONST_ITERATE (TStringVector, it, *m_MetaphonePattern.get()) {
                    CDictionaryUtil::GetMetaphone(*it, &sTemp);
                    *it = sTemp;
                    sTemp.clear();
                }
            }
            break;
    }
}


bool CStringMatching::MatchString(const CTempString &str)
{
    switch (m_MatchingAlgorithm) {
    case ePlainSearch   :
        return (NPOS == NStr::Find(str, m_Pattern, m_UseCase)) ? false : true;
    case eWildcardMatch :
        return NStr::MatchesMask(str, m_Pattern, m_UseCase);
    case eRegex         :
        assert(m_Regex.get());
        return m_Regex->IsMatch(str);
    case eMetaphone     :
        {
            assert(m_MetaphonePattern.get());
            TStringVector metaphoneStr;
            TStringVector tokenizedStr;
            NStr::Split(str, " ", tokenizedStr);
            string sTemp;
            NON_CONST_ITERATE (vector<string>, it, tokenizedStr) {
                if (it->empty())
                    continue;
                if (it->length() < 5)
                    continue;
                CDictionaryUtil::GetMetaphone(*it, &sTemp);
                if (sTemp.empty())
                    continue;
                metaphoneStr.push_back(sTemp);
            }
            bool found = false;
            ITERATE (TStringVector, itPattern, *m_MetaphonePattern.get()) {
                found = false;
                ITERATE (TStringVector, itStr, metaphoneStr) {
                    if (*itPattern == *itStr) {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    break;
            }
            return found;
        }
    }
    return false;
}

END_NCBI_SCOPE