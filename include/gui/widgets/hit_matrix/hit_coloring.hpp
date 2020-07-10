#ifndef GUI_WIDGETS_HIT_MATRIX___HIT_COLORING__HPP
#define GUI_WIDGETS_HIT_MATRIX___HIT_COLORING__HPP

/*  $Id: hit_coloring.hpp 21333 2010-04-29 22:40:23Z voronov $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbifloat.h>

#include <gui/utils/rgba_color.hpp>


BEGIN_NCBI_SCOPE

struct NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT  SHitColoringParams : public CObject
{
    string  m_ScoreName;
    double  m_MinValue;
    double  m_MaxValue;

    bool    m_EnableMinGrad;
    bool    m_EnableMaxGrad;
    double  m_MinGrad;
    double  m_MaxGrad;
    double m_Precision;
    char m_PreFormat[32];
    CRgbaColor  m_MinColor;
    CRgbaColor  m_MaxColor;
    bool    m_LogScale;
    int     m_Steps; // number of colors in gradient

    SHitColoringParams();

    void SetValueRange( double min, double max );
    double GetColorNorm( double value, bool precise = true ) const;

    double GetMin( bool precise = true ) const;
    double GetMax( bool precise = true ) const;

    inline double GetPrecision() const { return m_Precision; }
    void SetPrecision( double prec );

    inline const char* GetPrecisionFormat() const { return m_PreFormat; }
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___HIT_COLORING__HPP
