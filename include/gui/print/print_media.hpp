#ifndef GUI_UTILS__PRINT_MEDIA_HPP
#define GUI_UTILS__PRINT_MEDIA_HPP

/*  $Id: print_media.hpp 28446 2013-07-12 15:35:17Z falkrb $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CMedia - dimensions for physical print media
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/print/print_utils.hpp>
#include <gui/utils/vect2.hpp>


BEGIN_NCBI_SCOPE



class NCBI_GUIPRINT_EXPORT CMedia
{
public:
    /// Note that the standard allows a maximum width/height in pdf units
    /// (the MediaBox width/height written to the file) of 14400. UserUnit 
    /// option allows pdf 1.4+ files to render beyond that.
    CMedia(const string& name,
           float width,
           float height,
           CUnit::TUnit unit = CUnit::ePdfUnit);
    virtual ~CMedia();

    const string& GetName(void) const;
    CUnit::TPdfUnit GetWidth(void) const;
    CUnit::TPdfUnit GetHeight(void) const;
    float GetNativeWidth(void) const;
    float GetNativeHeight(void) const;
    CUnit::TUnit GetUnit(void) const;
    bool Inside(const CVect2<float>& pt) const;

    static const CMedia& GetMedia(const string& media);

private:
    void x_SetDimensions(void);

    string m_Name;
    float m_Width, m_Height;
    CUnit::TUnit m_Unit;

    //
    // the unit height/width is always in default user coordinates (1/72 inch)
    //
    CUnit::TPdfUnit m_UnitWidth, m_UnitHeight;
};


//
// US imperial paper formats
//
static const CMedia Letter("Letter", 8.5, 11, CUnit::eInch);
static const CMedia Legal("Legal", 8.5, 14, CUnit::eInch);
static const CMedia Ledger("Ledger", 11, 17, CUnit::eInch);

//
// ISO paper formats
//
static const CMedia A0("A0", 841, 1189, CUnit::eMillimeter);
static const CMedia A1("A1", 594, 841, CUnit::eMillimeter);
static const CMedia A2("A2", 420, 594, CUnit::eMillimeter);
static const CMedia A3("A3", 297, 420, CUnit::eMillimeter);
static const CMedia A4("A4", 210, 297, CUnit::eMillimeter);
static const CMedia A5("A5", 148, 210, CUnit::eMillimeter);
static const CMedia A6("A6", 105, 148, CUnit::eMillimeter);

static const CMedia B0("B0", 1000, 1414, CUnit::eMillimeter);
static const CMedia B1("B1", 707, 1000, CUnit::eMillimeter);
static const CMedia B2("B2", 500, 707, CUnit::eMillimeter);
static const CMedia B3("B3", 353, 500, CUnit::eMillimeter);
static const CMedia B4("B4", 250, 353, CUnit::eMillimeter);
static const CMedia B5("B5", 176, 250, CUnit::eMillimeter);
static const CMedia B6("B6", 125, 176, CUnit::eMillimeter);



END_NCBI_SCOPE


#endif // GUI_UTILS__PRINT_MEDIA_HPP
