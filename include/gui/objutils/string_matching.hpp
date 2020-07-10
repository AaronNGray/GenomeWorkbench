#ifndef GUI_OBJUTILS___STRING_MATCHING__HPP
#define GUI_OBJUTILS___STRING_MATCHING__HPP

/*  $Id: string_matching.hpp 32190 2015-01-16 17:01:01Z evgeniev $
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
 * Provides a class that supports several text matching algorithms - plain text search, 
 * wildcard matching, regexp and Metaphone search.
 *
 */

#include <memory>
#include <vector>
#include <string>
#include <corelib/ncbistr.hpp>
#include <util/xregexp/regexp.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class CQueryExec;

////////////////////////////////////////////////////////////////////////////////
/// class CTextSearch
///
/// Matches string by using different algorithms - plain text search, 
/// wildcard matching, regexp and Metaphone search.
///  
class NCBI_GUIOBJUTILS_EXPORT CStringMatching
{
public:
    /// String matching algorithms
    enum EStringMatching {
        ePlainSearch,   ///< Plain search
        eWildcardMatch, ///< Wildcard pattern matching
        eRegex,         ///< Regular expression matching
        eMetaphone      ///< Metaphone mathcing
    };

public:
    /// Constructs text search object with specific pattern and algorithm
    /// @param[in] pattern
    ///     Reference to a pattern to search for.
    /// @param[in] string_matching
    ///     String matching algorithm to use, default is plain text search.
    /// @param[in] use_case
    ///     Whether to do a case sensitive comparison (default), or not. 
    CStringMatching(const CTempString &pattern, EStringMatching string_matching = ePlainSearch, NStr::ECase use_case = NStr::eCase);

    /// Matches a string to a pattern, using the specified string matching algorithm
    /// @param[in] str
    ///     Reference to the string to be searched.
    /// @return
    ///     Return true if "str" matches the pattern, and false otherwise. 
    bool MatchString(const CTempString &str);

protected:
    typedef vector<string> TStringVector;

protected:
    /// Default constructor - declared, but not defined
    CStringMatching();

    /// Copy constructor - declared but not defined
    CStringMatching(const CStringMatching &src);
 
    /// Assignment operator - declared but not defined
    CStringMatching& operator=(const CStringMatching &src);

private:
    /// The pattern to search for.
    CTempString             m_Pattern;
    /// String matching algorithm to use.
    EStringMatching         m_MatchingAlgorithm;
    /// Whether to do a case sensitive comparison or not. 
    NStr::ECase             m_UseCase;
    /// Metaphone pattern(s)
    auto_ptr<TStringVector> m_MetaphonePattern;
    /// Regular expression
    auto_ptr<CRegexp>       m_Regex;
};

END_NCBI_SCOPE

#endif // GUI_OBJUTILS___STRING_MATCHING__HPP