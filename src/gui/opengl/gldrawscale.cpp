/*  $Id: gldrawscale.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
* Author: Philip Johnson
*
* File Description: 'gldrawscale' -- contains class to nicely draw a scale
* on screen (think sequence coordinates).
*
* ---------------------------------------------------------------------------
*/

#include <ncbi_pch.hpp>
#include <gui/opengl/gldrawscale.hpp>
#include <gui/opengl/glfont.hpp>
#include <corelib/ncbistre.hpp>
#include <algorithm>
#include <math.h>

BEGIN_NCBI_SCOPE

const char* CGlDrawScale::sm_SISymbols = "kMGTPEZY";

//-----------------------------------------------------------------------------
//PRE : font to use for drawing scale; whether or not to use abbreviations
//when appropriate (and, if so, what sort to use)
//POST: viewscale initialized
CGlDrawScale::CGlDrawScale(IGlFont &font, EAbbrevType useAbbrev)
    : m_Font(font),
      m_UseAbbrev(useAbbrev),
      m_CoordConverter(NULL)
{
    char tmp[2];
    tmp[1] = '\0';

    m_MaxDigitWidth = 0;
    for( char d = '0'; d <= '9'; ++d) {
        tmp[0] = d;
        m_MaxDigitWidth = max(m_MaxDigitWidth, m_Font.TextWidth(tmp));
    }

    m_MaxSIPrefixWidth = 0;
    for(unsigned int i = 0; sm_SISymbols[i] != '\0'; ++i) {
        tmp[0] = sm_SISymbols[i];
        m_MaxSIPrefixWidth = max(m_MaxSIPrefixWidth, m_Font.TextWidth(tmp));
    }
}

//-----------------------------------------------------------------------------
//PRE : unsigned integer (x)
//POST: 10^n
unsigned int CGlDrawScale::x_Exp10(unsigned int x) const
{
    unsigned int n;

    n = 1;
    for(unsigned int i = 0; i < x; ++i) {
        n *= 10;
    }

    return n;
}


//-----------------------------------------------------------------------------
//PRE : an integer
//POST: if greater than 2 digits, integer rounded up to first number with
//only 2 significant digits; if exactly 2 digits, integer rounded up to
//first number with 1 significant digit; if between 5 and 10, rounded up to
//10; if less than 5, rounded up to 5
void CGlDrawScale::x_Normalize(int &num) const
{
    int sign = 1;
    if (num < 0) { //remove negative
        sign = -1;
        num *= -1;
    }

    int digitsI = (num == 0) ? 1 : (int) floor(log10((float)num) + 1);
    if (digitsI >= 3) {
        num = (num / x_Exp10(digitsI-2) + 1) * x_Exp10(digitsI-2);
    } else if (digitsI == 2) {
        num = (num / x_Exp10(digitsI-1) + 1) * x_Exp10(digitsI-1);
    } else if (num > 5) {
        num = 10;
    } else {
        num = 5;
    }

    num *= sign; //put back negative (if had in originally)
}

//-----------------------------------------------------------------------------
//PRE : a number
//POST: a string representing that number
string CGlDrawScale::x_GenerateLabel(int num, EAbbrevType type) const
{
    CNcbiOstrstream oss;

    switch(type) {
    case eNoAbbrev:
        oss << num;
        break;

    case eCommas:
        {
            int count = 0;
            while (abs(num) >= 10) {
                oss << num % 10;
                if (++count % 3 == 0) {
                    oss << ',';
                }
                num /= 10;
            }
            oss << num % 10;
            string tmp = CNcbiOstrstreamToString(oss);
            reverse(tmp.begin(), tmp.end());
            return tmp;
        }

    case eUseScientificNotation:
        oss.precision(3);
        oss.flags(oss.scientific);
        oss << (float) num;
        break;

    case eUseSISymbols:
        {{
            int prefixI = -1;
            float decimal = (float)num;
            while (abs(num) >= 1000) {
                ++prefixI;
                decimal = (float) num / 1000;
                num /= 1000;
            }
            oss << decimal;
            if (prefixI >= 0) {
                oss << sm_SISymbols[prefixI] << '\0';
            }
        }}
        break;
    }

    return CNcbiOstrstreamToString(oss);
}

//-----------------------------------------------------------------------------
//PRE : left-most user coordinate, right-most user coordinate
//POST: maximum size of label for this range
unsigned int CGlDrawScale::x_CalcMaxLabelSize(int left, int right,
                                              EAbbrevType abbrev) const
{
    unsigned int size = 0;

    int digitsL = (left == 0) ? 1 : (int) floor(log10((float)abs(left)) + 1);
    int digitsR = (right == 0) ? 1 : (int) floor(log10((float)abs(right)) + 1);
    switch (abbrev) {
    case eNoAbbrev: //DDDDD..
        size = (unsigned int)(max(digitsR, digitsL) * m_MaxDigitWidth);
        break;
    case eCommas: //D,DDD,DDD...
        size =
            (unsigned int)((max(digitsL, digitsR) + max(digitsL, digitsR) / 3)
            * m_MaxDigitWidth);
        break;
    case eUseSISymbols: //DDD.DDDk
        size = (unsigned int)(m_MaxDigitWidth * 6 + m_Font.TextWidth(".") +
            m_MaxSIPrefixWidth);
        break;
    case eUseScientificNotation: //D.DDDe+DD
        size = (unsigned int)(m_MaxDigitWidth * 6 + m_Font.TextWidth(".e+"));
        break;
    }

    if (left < 0  ||  right < 0) {
        size += (unsigned int)(m_Font.TextWidth("-"));
    }
    return size;
}

//-----------------------------------------------------------------------------
//PRE : width in pixels; left-most user coordinate, right-most user
//coordinate
//POST: scale drawn in the following box: left, -0.5, right, [font-height]
void CGlDrawScale::Draw(int width, int left, int right) const
{
    if (width <= 0)
        throw runtime_error("CGlDrawScale::Draw given non-positive width.");

    double xScale, yScale;
    {
        GLint viewport[4];
        GLdouble mvMatrix[16], projMatrix[16];
        GLdouble wx1,wx2,wy1,wy2,wz; //world coordinates

        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_MODELVIEW_MATRIX, mvMatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

        int glL = m_CoordConverter ? m_CoordConverter->ToGl(left) : left;
        int glR = m_CoordConverter ? m_CoordConverter->ToGl(right) : right;

        gluUnProject(glL,0,0, mvMatrix, projMatrix, viewport, &wx1,&wy1,&wz);
        gluUnProject(glR,1,0, mvMatrix, projMatrix, viewport, &wx2,&wy2,&wz);
        xScale = (wx2-wx1)/(glR-glL);
        yScale = (wy2-wy1);
    }

    EAbbrevType abbrevType;
    if (m_UseAbbrev != eNoAbbrev && //use abbreviations
        abs(right-left) > width) { //big interval
        abbrevType = m_UseAbbrev;
    } else {
        abbrevType = eNoAbbrev;
    }

    unsigned int maxLabelSize = x_CalcMaxLabelSize(left, right, abbrevType);
    int labelInterval = (int) ceil(((float)(right-left) / width) *
                                   (maxLabelSize*1.5));
    x_Normalize(labelInterval);

    int minBound = min(left, right);
    int maxBound = max(left, right);


    int labelStart = left;
    if (labelStart % labelInterval != 0) {
        labelStart = (labelStart / labelInterval) * labelInterval;
        if (labelStart < minBound  ||  labelStart > maxBound) {
            labelStart += labelInterval;
        }
    }

    //labels
    for (int labelPos = labelStart;
         labelPos >= minBound  &&  labelPos <= maxBound;
         labelPos += labelInterval) {
        string label = x_GenerateLabel(labelPos, abbrevType);

        m_Font.TextOut((m_CoordConverter ? m_CoordConverter->ToGl(labelPos) :
                        labelPos) -
                       (m_Font.TextWidth(label.c_str()) * xScale / 2),
                       3.0f * yScale, label.c_str());
    }

    //tickmarks
    glBegin(GL_LINES);
    int tickInterval = max(1, abs(labelInterval) / 5);
    if (labelInterval < 0) {
        tickInterval *= -1;
    }
    int tickStart = left;
    if (tickStart % tickInterval != 0) {
        tickStart = (tickStart / tickInterval) * tickInterval;
        if (tickStart < minBound  ||  tickStart > maxBound) {
            tickStart += tickInterval;
        }
    }

    for (int labelPos = tickStart;
        labelPos >= minBound  &&  labelPos <= maxBound;
        labelPos += tickInterval) {
        int glPos = (m_CoordConverter ? m_CoordConverter->ToGl(labelPos) :
                                        labelPos);
        glVertex2d(glPos, (labelPos % labelInterval == 0) ? -yScale * 4.0
                                                          : -yScale * 2.0);
        glVertex2d(glPos, 0);
    }
    glEnd();
}

END_NCBI_SCOPE
