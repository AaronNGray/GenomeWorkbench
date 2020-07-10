/*  $Id: parse_collectiondate_formats.cpp 42254 2019-01-22 15:22:29Z asztalos $
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
 * Authors:  Igor Filippov
 */

#include <ncbi_pch.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/packages/pkg_sequence_edit/parse_collectiondate_formats.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;


CRef<CCmdComposite> CParseCollectionDateFormats::GetCommand(CSeq_entry_Handle tse, bool monthFirst)
{
    m_MonthFirst = monthFirst;
    if (m_MonthFirst) {
        return x_GetCommand(tse, "Parse Collection Date Month First");
    }
    else {
        return x_GetCommand(tse, "Parse Collection Date Day First");
    }
}

bool CParseCollectionDateFormats::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;
    
    EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource) {
        if ((*subsource)->IsSetSubtype() && (*subsource)->IsSetName() && (*subsource)->GetSubtype() == CSubSource::eSubtype_collection_date)
        {
            bool month_ambiguous;
            string date = (*subsource)->GetName();
            string new_date = CSubSource::FixDateFormat(date, m_MonthFirst, month_ambiguous);
            if (!new_date.empty() && new_date != date)
            {
                (*subsource)->SetName(new_date);
                modified = true;
            }
        }
    }
    return modified;
}


END_NCBI_SCOPE
