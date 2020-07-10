#ifndef OBJECTS_GENERAL___CLEANUP_UTILS__HPP
#define OBJECTS_GENERAL___CLEANUP_UTILS__HPP

/* $Id: cleanup_utils.hpp 581496 2019-03-01 16:42:04Z bollin $
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
 * Author:  Mati Shomrat
 *
 * File Description:
 *   General utilities for data cleanup.
 *
 * ===========================================================================
 */
#include <corelib/ncbistd.hpp>
#include <algorithm>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/BioSource.hpp>

#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CCit_sub;
class CPubdesc;
class CAuthor;
class CAuth_list;

/// Cleaning functions may return true if they have changed something.
/// false if no change was needed.

/// convert double quotes to single quotes
inline 
bool ConvertDoubleQuotes(string& str)
{
    bool changed = false;
    if (!str.empty()) {
        NON_CONST_ITERATE(string, c_it, str) {
            if (*c_it == '\"') {
                *c_it = '\'';
                changed = true;
            }
        }
    }
    return changed;
}


bool CleanVisString( string &str );

bool CleanVisStringJunk( string &str, bool allow_ellipses = false );

/// remove white space between pairs of tildes.
/// "~ ~  ~ a~" -> "~~~ a~"
bool  RemoveSpacesBetweenTildes(string& str);

/// remove duplicate internal semicolons.
/// "a;;b" will become "a;b".
void TrimInternalSemicolons (string& str);

/// weird space compression from C Toolkit
bool Asn2gnbkCompressSpaces (string& val);

// special trimming function from C Toolkit
bool TrimSpacesSemicolonsAndCommas (string& val);

/// Change double to single quotes
bool CleanDoubleQuote(string& str);

/// remove all spaces from a string
bool RemoveSpaces(string& str);

template<typename C>
bool CleanVisStringContainer(C& str_cont)
{
    bool changed = false;
    typename C::iterator it = str_cont.begin();
    while (it != str_cont.end()) {
        if (CleanVisString(*it)) {
            changed = true;
        }
        if (NStr::IsBlank(*it)) {
            it = str_cont.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }
    return changed;
}

template<typename C>
bool CleanVisStringContainerJunk(C& str_cont)
{
    bool changed = false;
    typename C::iterator it = str_cont.begin();
    while (it != str_cont.end()) {
        if (CleanVisStringJunk(*it)) {
            changed = true;
        }
        if (NStr::IsBlank(*it)) {
            it = str_cont.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }
    return changed;
}

struct SCaseInsensitiveStrComp {
    SCaseInsensitiveStrComp(const string& str) : m_Target(str) { }
    bool operator()(string& s) {
        return NStr::EqualNocase(s, m_Target);
    }
    string m_Target;
};

template<typename Cont>
bool RemoveDupsNoSort(Cont& l, bool case_insensitive = false)
{
    bool changed = false;
    typedef typename Cont::iterator iterator;
    iterator l_it = l.begin();
    while (l_it != l.end()) {
        
        iterator dup_it = case_insensitive ?
            find_if( l.begin(), l_it, SCaseInsensitiveStrComp(*l_it) ) :
            find( l.begin(), l_it, *l_it );
        if (dup_it != l_it) {
            l_it = l.erase(l_it);
            changed = true;
        } else {
            ++l_it;            
        }
    }
    return changed;
}


#define TRUNCATE_SPACES(o, x) \
    if ((o).IsSet##x()) { \
        size_t n = (o).Get##x().size(); \
        NStr::TruncateSpacesInPlace((o).Set##x()); \
        if (n != (o).Get##x().size() ) { \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset##x(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
    }

#define TRUNCATE_CHOICE_SPACES(o, x) \
    { \
        size_t n = (o).Get##x().size(); \
        NStr::TruncateSpacesInPlace((o).Set##x()); \
        if (n != (o).Get##x().size() ) { \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
    }

#define CONVERT_QUOTES(x) \
    if (IsSet##x()) { \
        if (ConvertDoubleQuotes(Set##x()) ) { \
            ChangeMade(CCleanupChange::eCleanDoubleQuotes); \
        } \
    }

#define COMPRESS_STRING_MEMBER(o, x) \
    if ((o).IsSet##x()) { \
        const size_t old_len = (o).Set##x().length(); \
        CompressSpaces((o).Set##x()); \
        if( old_len != (o).Set##x().length() ) { \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset##x(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
    }

#define CLEAN_STRING_MEMBER(o, x) \
    if ((o).IsSet##x()) { \
        if (CleanVisString((o).Set##x()) ) { \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset##x(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
    }

// The "do-while" forces the need for a semicolon
#define CLEAN_AND_COMPRESS_STRING_MEMBER(o, x) \
    do { COMPRESS_STRING_MEMBER(o, x); CLEAN_STRING_MEMBER(o, x); } while(false)

#define CLEAN_STRING_MEMBER_JUNK(o, x) \
if ((o).IsSet##x()) { \
    if (CleanVisStringJunk((o).Set##x()) ) { \
        ChangeMade(CCleanupChange::eTrimSpaces); \
    } \
    if (NStr::IsBlank((o).Get##x())) { \
        (o).Reset##x(); \
        ChangeMade(CCleanupChange::eTrimSpaces); \
    } \
}

// The "do-while" forces the need for a semicolon
#define CLEAN_AND_COMPRESS_STRING_MEMBER_JUNK(o, x) \
    do { COMPRESS_STRING_MEMBER(o, x); CLEAN_STRING_MEMBER_JUNK(o, x); } while(false)

#define EXTENDED_CLEAN_STRING_MEMBER(o, x) \
if ((o).IsSet##x()) { \
    CleanVisString((o).Set##x()); \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset##x(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
}

#define CLEAN_STRING_CHOICE(o, x) \
        if (CleanVisString((o).Set##x()) ) { \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        }

#define CLEAN_STRING_LIST(o, x) \
    if ((o).IsSet##x()) { \
        if (CleanVisStringContainer((o).Set##x()) ) { \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
        if ((o).Get##x().empty()) { \
            (o).Reset##x(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
    }

#define CLEAN_STRING_LIST_JUNK(o, x) \
if ((o).IsSet##x()) { \
    if (CleanVisStringContainerJunk((o).Set##x()) ) { \
        ChangeMade(CCleanupChange::eTrimSpaces); \
    } \
    if ((o).Get##x().empty()) { \
        (o).Reset##x(); \
        ChangeMade(CCleanupChange::eTrimSpaces); \
    } \
}

#define EXTENDED_CLEAN_STRING_LIST(o, x) \
    if ((o).IsSet##x()) { \
        CleanVisStringList((o).Set##x()); \
        if ((o).Get##x().empty()) { \
            (o).Reset##x(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
    }

/// clean a string member 'x' of an internal object 'o'
#define CLEAN_INTERNAL_STRING(o, x) \
    if (o.IsSet##x()) { \
        if ( CleanVisString(o.Set##x()) ) { \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
        if (NStr::IsBlank(o.Get##x())) { \
            o.Reset##x(); \
            ChangeMade(CCleanupChange::eTrimSpaces); \
        } \
    }

#define MERGEABLE_STRING_VALUE(o1, o2, x) \
    if ((o1).CanGet##x() && (o2).CanGet##x() \
        && !NStr::IsBlank ((o1).Get##x()) \
        && !NStr::IsBlank ((o2).Get##x()) \
        && !NStr::Equal ((o1).Get##x(), (o2).Get##x())) { \
        return false; \
    }    

#define MERGEABLE_STRING_LIST(o1, o2, x) \
    if ((o1).CanGet##x() && (o2).CanGet##x() \
        && (o1).Get##x().size() > 0 \
        && (o2).Get##x().size() > 0) { \
        list <string>::const_iterator it1 = (o1).Get##x().begin(); \
        list <string>::const_iterator it2 = (o2).Get##x().begin(); \
        while (it1 != (o1).Get##x().end() && it2 != (o2).Get##x().end()) { \
            if (!NStr::Equal ((*it1), (*it2))) { \
                return false; \
            } \
            ++it1; ++it2; \
        } \
        if (it1 != (o1).Get##x().end() || it2 != (o2).Get##x().end()) { \
            return false; \
        } \
    } 
    
#define MERGEABLE_INT_VALUE(o1, o2, x) \
    (!(o1).CanGet##x() || !(o2).CanGet##x() \
     || (o1).Get##x() == 0 \
     || (o2).Get##x() == 0 \
     || (o1).Get##x() == (o2).Get##x())

#define MATCH_STRING_VALUE(o1, o2, x) \
    ((!(o1).CanGet##x() && !(o2).CanGet##x()) \
     || ((o1).CanGet##x() && (o2).CanGet##x() \
         && NStr::Equal((o1).Get##x(), (o2).Get##x())))

#define MATCH_STRING_LIST(o1, o2, x) \
    if (((o1).CanGet##x() && (o1).Get##x().size() > 0 && !(o2).CanGet##x()) \
        || (!(o1).CanGet##x() && (o2).CanGet##x() && (o2).Get##x().size() > 0)) { \
        return false; \
    } else if ((o1).CanGet##x() && (o2).CanGet##x()) { \
        list <string>::const_iterator it1 = (o1).Get##x().begin(); \
        list <string>::const_iterator it2 = (o2).Get##x().begin(); \
        while (it1 != (o1).Get##x().end() && it2 != (o2).Get##x().end()) { \
            if (!NStr::Equal ((*it1), (*it2))) { \
                return false; \
            } \
            ++it1; ++it2; \
        } \
        if (it1 != (o1).Get##x().end() || it2 != (o2).Get##x().end()) { \
            return false; \
        } \
    } 
    
#define MATCH_INT_VALUE(o1, o2, x) \
    ((!(o1).CanGet##x() && !(o2).CanGet##x()) \
     || ((o1).CanGet##x() && (o2).CanGet##x() \
         && (o1).Get##x() == (o2).Get##x()) \
     || ((o1).CanGet##x() && (o1).Get##x() == 0 \
         && !(o2).CanGet##x()) \
     || ((o2).CanGet##x() && (o2).Get##x() == 0 \
         && !(o1).CanGet##x()))         

#define MATCH_BOOL_VALUE(o1, o2, x) \
    if (!(o1).IsSet##x()) { \
        if ((o2).IsSet##x()) { \
            return false; \
        } \
    } else if (!(o2).IsSet##x()) { \
        return false; \
    } else if (((o1).Get##x() && !(o2).Get##x()) \
               || (!(o2).Get##x() && (o2).Get##x())) { \
        return false; \
    }

// for converting strings to locations
CRef<CSeq_loc> ReadLocFromText(const string& text, const CSeq_id *id, CScope *scope);

// for finding the correct amino acid letter given an abbreviation
char ValidAminoAcid (const string& abbrev);

// for sorting and uniquing dbtags
bool s_DbtagCompare (const CRef<CDbtag>& dbt1, const CRef<CDbtag>& dbt2);
bool s_DbtagEqual (const CRef<CDbtag>& dbt1, const CRef<CDbtag>& dbt2);

// for sorting and uniquing org-ref syns
bool s_OrgrefSynCompare( const string & syn1, const string & syn2 );
bool s_OrgrefSynEqual  ( const string & syn1, const string & syn2 );

// Is this range sorted according to the given comparison?
// needed to see if sorts in cleanup do anything.
// Maybe should be at a higher level of the source tree?

template <class Iter, class Comp>
bool is_sorted(Iter first, Iter last,
               Comp comp)
{
    if (first == last)
        return true;
    
    Iter next = first;
    for (++next; next != last; first = next, ++next) {
        if (comp(*next, *first))
            return false;
    }
    
    return true;
}


template <class Iter>
bool is_sorted(Iter first, Iter last)
{
    if (first == last)
        return true;
    
    Iter next = first;
    for (++next; next != last; first = next, ++next) {
        if (*next < *first)
            return false;
    }
    
    return true;
}


END_SCOPE(objects)
END_NCBI_SCOPE

#endif  // OBJECTS_GENERAL___CLEANUP_UTILS__HPP
