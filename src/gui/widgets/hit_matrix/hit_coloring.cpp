/*  $Id: hit_coloring.cpp 21333 2010-04-29 22:40:23Z voronov $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/hit_matrix/hit_coloring.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE


SHitColoringParams::SHitColoringParams()
:   m_MinValue(0),
    m_MaxValue(0),
    m_EnableMinGrad(false),
    m_EnableMaxGrad(false),
    m_MinGrad(0),
    m_MaxGrad(0),
    m_MinColor(0.5f, 0.0f, 1.0f),
    m_MaxColor(1.0f, 0.5f, 0.0f),
    m_LogScale(false),
    m_Steps(4)
{
    SetPrecision( 0.001 );
}


void SHitColoringParams::SetValueRange(double min, double max)
{
    _ASSERT(min <= max);
    m_MinValue = min;
    m_MaxValue = max;
}

static double Blur( double value, double precision, bool up )
{ 
    if( precision > 0.1 || precision < 1.0e-12 ){
        return value;
    }

    double delta = pow( 10, ceil( log10( value ) ) ) * precision;

    return value + (up ? delta : -delta)/2;
}

double SHitColoringParams::GetMin( bool precise ) const
{
    double grad = m_EnableMinGrad ? m_MinGrad : m_MinValue;
    if( !precise ){
        grad = Blur( grad, m_Precision, false );
    }

    return grad;
}

double SHitColoringParams::GetMax( bool precise ) const
{
    double grad = m_EnableMaxGrad ? m_MaxGrad : m_MaxValue;
    if( !precise ){
        grad = Blur( grad, m_Precision, true );
    }

    return grad;
}

double SHitColoringParams::GetColorNorm( double value, bool precise ) const
{
    double v_start = m_MinValue;
    if(m_EnableMinGrad) {
        v_start = min(v_start, m_MinGrad);
    }
    double v_end = m_MaxValue;
    if(m_EnableMaxGrad) {
        v_end = max(v_end, m_MaxGrad);
    }

    if( !precise ){
        v_start = Blur( v_start, m_Precision, false );
        v_end = Blur( v_end, m_Precision, true );
    }

   if(m_LogScale)  {
        double log_min = log10(v_start);
        if( ! finite(log_min))  {
            log_min = -300;
        }
        double log_max = log10(v_end);
        if( ! finite(log_max))  {
            log_max = -299;
        }
        double log_value = log10(value);

        double d_log = log_value - log_min;
        double norm = d_log / (log_max - log_min);
        return norm;
    } else {
        double range = v_end - v_start;
        double norm = (value - v_start) / range;
        return norm;
    }
}

void SHitColoringParams::SetPrecision( double prec )
{
    m_Precision = prec;

    if( m_Precision > 0.1 || m_Precision < 1.0e-12 ){
        sprintf( m_PreFormat, "%%g" );
        return;
    }

    int dig_num = (int)floor( -log10( m_Precision ) );

    sprintf( m_PreFormat, "%%.%ig", dig_num );
}

END_NCBI_SCOPE
