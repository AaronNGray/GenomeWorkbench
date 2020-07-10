/*  $Id: rna_name_completer.cpp 40482 2018-02-22 21:21:14Z bollin $
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
 * Authors:  Colleen Bollin
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/rna_name_completer.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

static CRNANameCompleter::TContainer s_RNANames;

CRNANameCompleter::CRNANameCompleter()
{
    x_Init();
    m_CurPos = s_RNANames.end();
}

CRNANameCompleter::~CRNANameCompleter()
{
}

bool CRNANameCompleter::x_Init()
{
    if (!s_RNANames.empty())
        return true;

    s_RNANames.insert("12S ribosomal RNA");
    s_RNANames.insert("16S ribosomal RNA");
    s_RNANames.insert("18S ribosomal RNA");
    s_RNANames.insert("23S ribosomal RNA");
    s_RNANames.insert("26S ribosomal RNA");
    s_RNANames.insert("28S ribosomal RNA");
    s_RNANames.insert("5.8S ribosomal RNA");
    s_RNANames.insert("5S ribosomal RNA");
    s_RNANames.insert("large subunit ribosomal RNA");
    s_RNANames.insert("small subunit ribosomal RNA");

    return true;
}

bool CRNANameCompleter::Start(const wxString& prefix)
{
    m_Prefix = ToStdString(prefix);
    if (m_Prefix.empty()) {
        m_CurPos = s_RNANames.begin();
        return !s_RNANames.empty();
    }

    size_t length = m_Prefix.length();
    m_CurPos = s_RNANames.lower_bound(m_Prefix);
    if (m_CurPos == s_RNANames.end() || m_CurPos->length() < length)
        return false;

    return true;
}

wxString CRNANameCompleter::GetNext()
{
    if (m_CurPos == s_RNANames.end())
        return wxEmptyString;

    size_t length = m_Prefix.length();
    if (m_CurPos->length() < length)
        return wxEmptyString;

    if (!AStrEquiv(m_CurPos->substr(0, length), m_Prefix, PNocase()))
        return wxEmptyString;

    return ToWxString(*m_CurPos++);
}

END_NCBI_SCOPE
