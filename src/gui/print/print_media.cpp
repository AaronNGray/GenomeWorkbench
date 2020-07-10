/*  $Id: print_media.cpp 28446 2013-07-12 15:35:17Z falkrb $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CMedia - dimensions for physical print media
 *
 */

#include <ncbi_pch.hpp>
#include <gui/print/print_media.hpp>


BEGIN_NCBI_SCOPE



CMedia::CMedia(const string& name, float width, float height, CUnit::TUnit unit)
    : m_Name(name),
    m_Width(width),
    m_Height(height),
    m_Unit(unit)
{
    x_SetDimensions();
}


CMedia::~CMedia()
{
}


void CMedia::x_SetDimensions(void)
{
    switch (m_Unit) {
    case CUnit::eInch:
        m_UnitWidth = InchesToPdfUnits(m_Width);
        m_UnitHeight = InchesToPdfUnits(m_Height);
        break;

    case CUnit::eMillimeter:
        m_UnitWidth = MmToPdfUnits(m_Width);
        m_UnitHeight = MmToPdfUnits(m_Height);
        break;

    case CUnit::ePdfUnit:
        m_UnitWidth = m_Width;
        m_UnitHeight = m_Height;
        break;

    default:
        NCBI_THROW(CException, eUnknown, "CMedia::x_SetDimensions: unknown unit");
    }
}


const string& CMedia::GetName(void) const
{
    return m_Name;
}


CUnit::TPdfUnit CMedia::GetWidth(void) const
{
    return m_UnitWidth;
}


CUnit::TPdfUnit CMedia::GetHeight(void) const
{
    return m_UnitHeight;
}


float CMedia::GetNativeWidth(void) const
{
    return m_Width;
}


float CMedia::GetNativeHeight(void) const
{
    return m_Height;
}


CUnit::TUnit CMedia::GetUnit(void) const
{
    return m_Unit;
}

bool CMedia::Inside(const CVect2<float>& pt) const
{
    if (pt.X() >= 0 && pt.Y() >= 0 &&
        pt.X() <= m_UnitWidth && pt.Y() <= m_UnitHeight)
            return true;

    return false;
}


const CMedia& CMedia::GetMedia(const string& media)
{
    if (media == "Letter") {
        return Letter;
    }
    if (media == "US Letter") {
        return Letter;
    }
    if (media == "Legal") {
        return Legal;
    }
    if (media == "Ledger") {
        return Ledger;
    }

    if (media == "A0") {
        return A0;
    }
    if (media == "A1") {
        return A1;
    }
    if (media == "A2") {
        return A2;
    }
    if (media == "A3") {
        return A3;
    }
    if (media == "A4") {
        return A4;
    }
    if (media == "A5") {
        return A5;
    }
    if (media == "A6") {
        return A6;
    }

    if (media == "B0") {
        return B0;
    }
    if (media == "B1") {
        return B1;
    }
    if (media == "B2") {
        return B2;
    }
    if (media == "B3") {
        return B3;
    }
    if (media == "B4") {
        return B4;
    }
    if (media == "B5") {
        return B5;
    }
    if (media == "B6") {
        return B6;
    }

    NCBI_THROW(CException, eUnknown, "CMedia::GetMedia: unknown media " + media);
}



END_NCBI_SCOPE
