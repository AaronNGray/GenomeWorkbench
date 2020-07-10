#ifndef GUI_WIDGETS_TEXT_WIDGET___CALC_SIZE_CACHE__HPP
#define GUI_WIDGETS_TEXT_WIDGET___CALC_SIZE_CACHE__HPP

/*  $Id: calc_size_cache.hpp 39046 2017-07-21 19:37:22Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <unordered_map>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/gdicmn.h>

BEGIN_NCBI_SCOPE

class CTextStyle;

class CCalcSizeCache
{
public:
    CCalcSizeCache();

    struct SKey
    {
        string text;
        const CTextStyle* style;

        SKey() : style(0) {}

        bool operator==(const SKey& other) const
        {
            return text  == other.text && style == other.style;
        }
    };
    struct SKeyHash : public unary_function<SKey, size_t>
    {
        size_t operator()(const SKey& key) const
        {
            return hash<string>()(key.text) + hash<intptr_t>()((intptr_t)key.style);
        }
    };
    typedef std::unordered_map<SKey, wxSize, SKeyHash> TCache;

    void Put(const string& text, const CTextStyle* style, wxSize size);
    wxSize Get(const string& text, const CTextStyle* style) const;
    void Reset();

private:
    TCache m_Cache;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_TEXT_WIDGET___CALC_SIZE_CACHE__HPP
