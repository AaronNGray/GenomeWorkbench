/*  $Id: csv_exporter.cpp 36710 2016-10-26 16:51:44Z evgeniev $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/csv_exporter.hpp>

BEGIN_NCBI_SCOPE

void CCSVExporter::Field(const string& value)
{
    if (!m_NewRow)
        m_OS << m_Delimiter;

    if (value.find(m_Delimiter) != string::npos) {
        m_OS << m_Quote;

        string tmp = value;
        for (;!tmp.empty();) {
            size_t pos = tmp.find(m_Quote);
            if (pos == string::npos) {
                m_OS << tmp;
                break;
            }
            else {
                m_OS << tmp.substr(0, pos + 1) << m_Quote;
                tmp = tmp.substr(pos + 1);
            }
        }

        m_OS << m_Quote;
    }
    else {
        if (!m_QuoteAll)
            m_OS << value;
        else
            m_OS << m_Quote << value << m_Quote;
    }

    m_NewRow = false;
}

void CCSVExporter::Field(const wxString& value)
{
    Field(string(value.ToUTF8()));
}

void CCSVExporter::Field(long value)
{
    if (!m_NewRow)
        m_OS << m_Delimiter;

    m_OS << value;
    m_NewRow = false;
}

void CCSVExporter::Field(double value)
{
    if (!m_NewRow)
        m_OS << m_Delimiter;

    m_OS << value;
    m_NewRow = false;
}

void CCSVExporter::NewRow()
{
    m_OS << NcbiEndl;
    m_NewRow = true;
}

END_NCBI_SCOPE
