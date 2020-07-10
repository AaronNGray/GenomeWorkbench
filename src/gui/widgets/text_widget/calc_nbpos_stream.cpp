/*  $Id: calc_nbpos_stream.cpp 19853 2009-08-13 18:27:23Z katargir $
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

#include <gui/widgets/text_widget/calc_nbpos_stream.hpp>

BEGIN_NCBI_SCOPE

CCalcNBPosOStream::CCalcNBPosOStream() :
    m_Row(0), m_CurrentLineLength(0),
    m_NB(false), m_NBStartChar(0), m_NBEndChar(0), m_NBRow(0)
{
}

void CCalcNBPosOStream::NewLine()
{
    ++m_Row;
    m_CurrentLineLength = 0;

    if (m_NB) {
        m_NB = false;
        m_NBEndChar = m_CurrentLineLength;
    }
}

void CCalcNBPosOStream::NBtoggle()
{
    if (m_NB) {
        m_NB = false;
        m_NBEndChar = m_CurrentLineLength;
    }
    else {
        m_NB = true;
        m_NBRow = m_Row;
        m_NBStartChar = m_CurrentLineLength;
    }
}

CStyledTextOStream& CCalcNBPosOStream::operator<< (const string& text)
{
    m_CurrentLineLength += text.length();
    return *this;
}

void CCalcNBPosOStream::Indent (size_t steps)
{
    m_CurrentLineLength += steps;
}

int CCalcNBPosOStream::CurrentRowLength() const
{
    return m_CurrentLineLength;
}

void CCalcNBPosOStream::GetNBPos(int& start, int& end, int& row) const
{
    if (m_NBStartChar == m_NBEndChar) {
        start = m_CurrentLineLength;
        end   = m_CurrentLineLength;
        row   = m_Row;
    }
    else {
        start = m_NBStartChar;
        end   = m_NBEndChar;
        row   = m_NBRow;
    }
}

END_NCBI_SCOPE
