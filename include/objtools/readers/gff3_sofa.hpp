/*  $Id: gff3_sofa.hpp 587889 2019-06-12 15:25:38Z ludwigf $
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
 * Author: Frank Ludwig
 *
 * File Description:
 *   BED file reader
 *
 */

#ifndef OBJTOOLS_READERS___GFF3_SOFA__HPP
#define OBJTOOLS_READERS___GFF3_SOFA__HPP

#include <corelib/ncbistd.hpp>
#include <util/static_map.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects) // namespace ncbi::objects::

//  ----------------------------------------------------------------------------
class CompareNoCase 
//  ----------------------------------------------------------------------------
{
public:
    bool operator()(string x, const string& y) const {
        string::const_iterator pX = x.begin();
        string::const_iterator pY = y.begin();
        while (pX != x.end()  &&  pY != y.end()  &&  
                tolower(*pX) ==  tolower(*pY)) {
            ++pX;
            ++pY;
        }
        if (pX == x.end()) {
            return (pY != y.end());
        }
        if (pY == y.end()) {
            return false;
        }
        return (tolower(*pX) < tolower(*pY)); 
    }
};

typedef map< string, CFeatListItem, CompareNoCase > TLookupSofaToGenbank;
typedef TLookupSofaToGenbank::const_iterator TLookupSofaToGenbankCit;
typedef map< string, string, CompareNoCase > TAliasToTerm;
typedef TAliasToTerm::const_iterator TAliasToTermCit;

//  ----------------------------------------------------------------------------
class NCBI_DEPRECATED NCBI_XOBJREAD_EXPORT CGff3SofaTypes
//  ----------------------------------------------------------------------------
{
    friend CGff3SofaTypes& SofaTypes();

public:
    CGff3SofaTypes();
    ~CGff3SofaTypes();

    CSeqFeatData::ESubtype MapSofaTermToGenbankType(
        const string&);

    CFeatListItem MapSofaTermToFeatListItem(
        const string&);

    string MapSofaAliasToSofaTerm(
        const string&);

    bool IsStringSofaAlias(
        const string&);

protected:
    static CSafeStatic<TLookupSofaToGenbank> m_Lookup;
    static CSafeStatic<TAliasToTerm> m_Aliases;
};

//  ----------------------------------------------------------------------------
CGff3SofaTypes& SofaTypes();
//  ----------------------------------------------------------------------------

END_SCOPE(objects)
END_NCBI_SCOPE


#endif // OBJTOOLS_READERS___GFF3_SOFA__HPP
