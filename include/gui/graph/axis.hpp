#ifndef _AXIS_HPP
#define _AXIS_HPP

/*  $Id: axis.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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

#include <corelib/ncbistl.hpp>

#include <gui/opengl/glbitmapfont.hpp>

#include <gui/graph/igraph.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/graph/regular_grid.hpp>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CAxisRenderer
// This class render an Axis with ticks and labels. CRegularGridGen provides
// CAxisRenderer with positions whery ticks and labels should be drawn.
class NCBI_GUIGRAPH_EXPORT CAxisRenderer
{
public:
    enum EPosition {
        ePosMin,
        ePosMax,
        ePosFixedValue
    };

    enum ETextAlign {
        eLeftText,
        eRightText,
        eCenterText
    };

    enum ETickStyle {
        eNoTick,
        eMinSide,
        eMaxSide,
        eBothSides
    };

public:
    CAxisRenderer(bool bHorz = true);
    virtual ~CAxisRenderer();

    void    SetHorizontal(bool bHorz = true);
    void    SetScaleType(EScaleType Type);

    // Axis position
    void    SetPositionType(EPosition Type);
    void    SetFixedPosition(TModelUnit Value);

    // Labels drawing attributes
    void    SetTextAlign(ETextAlign Type);
    void    SetFormattingTemplate(const string& sTempl); // printf style
    //void    SetFont(CGlBitmapFont::EFont Type);

    // Tick drawing attributes
    void    SetTickStyle(ETickStyle Style);
    void    SetTickSize(int SizePix);

    // Colors
    void    SetColor(CRgbaColor Color);
    void    SetTextColor(CRgbaColor Color);

    virtual void    Render(CGlPane* pAreaPane, CGlPane* pGraphPane, CRegularGridGen* pGen);

protected:
    virtual void    x_GenerateLabels(CGlPane* pAreaPane, CGlPane* pGraphPane,
                                     CRegularGridGen* pGen);
    virtual void    x_LayoutLabels(int Start, int Finish);

    virtual void    x_DrawHorzLabels(CGlPane* pAreaPane, CGlPane* pGraphPane,
                                     CRegularGridGen* pGen, int dTickMin, int dTickMax);

    // these functions could be overridden in order to customize generation of text labels
    virtual string      x_FormatLabel(TModelUnit Value);
    virtual TModelUnit  x_GetLabelValue(TModelUnit Value);

protected:
    struct SLabelDescr
    {
        TModelUnit  m_Value;
        string  m_Str;
        int m_Pos;
        int m_Width;
        bool m_bVisible;

        SLabelDescr() : m_Value(0), m_Pos(0), m_Width(0), m_bVisible(true)   {};
    };
protected:
    static const int kDefTickSize;

    bool m_bHorz;
    EScaleType m_Type;

    EPosition   m_PosType;
    TModelUnit  m_FixedPosValue;

    ETextAlign  m_TextAlignType;
    string      m_sFormatTempl;

    ETickStyle  m_TickStyle;
    int         m_TickSize;

    CRgbaColor  m_Color;
    CRgbaColor  m_TextColor;

    CGlBitmapFont m_Font;

    vector<SLabelDescr> m_vLabels;
};

////////////////////////////////////////////////////////////////////////////////
/// class CAxisArea

class NCBI_GUIGRAPH_EXPORT CAxisArea
{
public:
    CAxisArea();
    virtual ~CAxisArea();

    virtual void    SetupAxes();
    void    SetScaleType(EScaleType TypeX, EScaleType TypeY);

    void    Render(CGlPane* pAreaPane, CGlPane* pGraphPane, CRegularGridGen* pGen);

protected:
    CRgbaColor    m_BoundsColor;

    CAxisRenderer m_XAxis;
    CAxisRenderer m_YAxis;
};

END_NCBI_SCOPE

#endif
