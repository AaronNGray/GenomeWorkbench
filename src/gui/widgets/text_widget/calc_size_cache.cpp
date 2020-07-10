/*  $Id: calc_size_cache.cpp 35499 2016-05-13 19:46:33Z evgeniev $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/text_widget/calc_size_cache.hpp>

BEGIN_NCBI_SCOPE

CCalcSizeCache::CCalcSizeCache() : m_Cache(1000)
{
}

void CCalcSizeCache::Put(const string& text, const CTextStyle* style, wxSize size)
{
    SKey key;
    key.text = text;
    key.style = style;
    m_Cache.insert(TCache::value_type(key, size));
}

wxSize CCalcSizeCache::Get(const string& text, const CTextStyle* style) const
{
    SKey key;
    key.text = text;
    key.style = style;
    TCache::const_iterator iter = m_Cache.find(key);
    if (iter != m_Cache.end())
        return iter->second;

    return wxDefaultSize;
}

void CCalcSizeCache::Reset()
{
    m_Cache.clear();
}

END_NCBI_SCOPE
