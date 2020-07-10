/*  $Id: taxon_text_completer.cpp 37422 2017-01-09 17:10:43Z bollin $
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
 */

#include <ncbi_pch.hpp>
#include <util/line_reader.hpp>

#include <gui/widgets/wx/taxon_text_completer.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <objects/seqfeat/Org_ref.hpp>

BEGIN_NCBI_SCOPE

static CTaxonTextCompleter::TContainer s_Taxons;

CTaxonTextCompleter::CTaxonTextCompleter()
{
    x_Init();
    m_CurPos = s_Taxons.end();
}

CTaxonTextCompleter::~CTaxonTextCompleter()
{
}

bool CTaxonTextCompleter::x_Init()
{
    if (!s_Taxons.empty())
        return true;

    vector<string> taxnames = objects::COrg_ref::GetTaxnameList();
    ITERATE(vector<string>, it, taxnames) {
        s_Taxons.insert(*it);
    }

    return true;
}

bool CTaxonTextCompleter::Start(const wxString& prefix)
{
    m_Prefix = ToStdString(prefix);
    if (m_Prefix.empty()) {
        m_CurPos = s_Taxons.begin();
        return !s_Taxons.empty();
    }

    size_t length = m_Prefix.length();
    m_CurPos = s_Taxons.lower_bound(m_Prefix);
    if (m_CurPos == s_Taxons.end() || m_CurPos->length() < length)
        return false;

    return true;
}

wxString CTaxonTextCompleter::GetNext()
{
    if (m_CurPos == s_Taxons.end())
        return wxEmptyString;

    size_t length = m_Prefix.length();
    if (m_CurPos->length() < length)
        return wxEmptyString;

    if (!AStrEquiv(m_CurPos->substr(0, length), m_Prefix, PNocase()))
        return wxEmptyString;

    return ToWxString(*m_CurPos++);
}

END_NCBI_SCOPE
