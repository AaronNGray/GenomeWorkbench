/*  $Id: assembly_term_completer.cpp 28562 2013-08-02 15:03:07Z katargir $
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

#include <gui/widgets/loaders/assembly_term_completer.hpp>
#include <gui/widgets/wx/taxon_text_completer.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CAssemblyTermCompleter::CAssemblyTermCompleter(const list<string>& history)
    : m_History(history), m_CurPos(m_History.end()),
    m_TaxonTextCompleter (new CTaxonTextCompleter()),
    m_HaveTaxons(false)
{
}

CAssemblyTermCompleter::~CAssemblyTermCompleter()
{
    delete m_TaxonTextCompleter;
}


bool CAssemblyTermCompleter::Start(const wxString& prefix)
{
    m_Prefix = ToStdString(prefix);
    m_HaveTaxons = m_TaxonTextCompleter->Start(prefix);
    m_CurPos = m_History.begin();

    if (m_Prefix.empty()) {
        return m_History.size() > 0 || m_HaveTaxons;
    }

    size_t length = m_Prefix.length();
    while (m_CurPos != m_History.end()) {
        if (m_CurPos->length() >= length &&
            AStrEquiv(m_CurPos->substr(0, length), m_Prefix, PNocase())) {
            return true;
        }
        ++m_CurPos;
    }

    return m_HaveTaxons;
}

wxString CAssemblyTermCompleter::GetNext()
{
    size_t length = m_Prefix.length();
    while (m_CurPos != m_History.end()) {
        if (m_CurPos->length() >= length &&
            AStrEquiv(m_CurPos->substr(0, length), m_Prefix, PNocase())) {
            return *m_CurPos++;
        }
        ++m_CurPos;
    }

    return m_HaveTaxons ? m_TaxonTextCompleter->GetNext() : wxString();
}

END_NCBI_SCOPE
