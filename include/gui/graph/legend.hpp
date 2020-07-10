#ifndef GUI_GRAPH___LEGEND__HPP
#define GUI_GRAPH___LEGEND__HPP

/*  $Id: legend.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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

#include <gui/opengl/glbitmapfont.hpp>

#include <gui/graph/igraph.hpp>
#include <gui/graph/igraph_data.hpp>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// ILegendDataSource

class ILegendDataSource  : public IGraphDataSource
{
public:
    virtual bool    ShowMarkers() = 0;
    virtual IStringArray*   GetLabelArray() = 0;
    virtual IColorArray*    GetColorArray() = 0;
    virtual INumericArray*  GetMarkerArray() = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// class CLegend

class NCBI_GUIGRAPH_EXPORT CLegend : public CGraphBase
{
public:
    CLegend();
    virtual ~CLegend();

    void    SetBackColor(const CRgbaColor& Color);
    const CRgbaColor& GetBackColor() const;

    void    SetBorderColor(const CRgbaColor& Color);
    const CRgbaColor& GetBorderColor();

    // IGraph implementation
    virtual bool    SetDataSource(IGraphDataSource* pDS);
    virtual const   TModelRect&    GetLimits()    const;
    virtual void    Render(CGlPane* pViewport, TElemVector* elems = NULL);

protected:
    ILegendDataSource*    GetLegendDataSource()
            { return dynamic_cast<ILegendDataSource*>(m_pDataSource); };
    virtual void    CalculateLimits();

    virtual void    x_RenderItems(CGlPane* pPane);
    virtual void    x_RenderItem(int X, int Y, const string& sLabel, const CRgbaColor& Color, int iMarker);
protected:
    CGlBitmapFont m_Font;

    CRgbaColor    m_BackColor;
    CRgbaColor    m_BorderColor;

    bool    m_bHorz; // horizontal layout
    int     m_Space;
    int     m_BoxW, m_BoxH;
};

///////////////////////////////////////////////////////////////////////////////
/// class CLegendDataSource

class NCBI_GUIGRAPH_EXPORT CLegendDataSource
    : public CSeriesBase, public ILegendDataSource
{
public:
    CLegendDataSource(int Length, bool bShowMarkers = true)
         : CSeriesBase(Length), m_bShowMarkers(bShowMarkers)    {}
    // Data Manipulation interface

    // ILegendDataSource implementation
    virtual bool    ShowMarkers() { return m_bShowMarkers;  }
    virtual IStringArray*  GetLabelArray()  {   return x_GetStringArray(0); }
    virtual IColorArray*   GetColorArray()  {   return x_GetColorArray(1); }
    virtual INumericArray* GetMarkerArray() {   return x_GetNumericArray(2); }

protected:
    typedef CTypedArrayAdapter<IDataArray::eString>  TStrAdapter;
    typedef CTypedArrayAdapter<IDataArray::eColor>   TColorAdapter;
    typedef CTypedArrayAdapter<IDataArray::eNumeric, int> TEnumAdapter;

    bool    m_bShowMarkers;
public:
    typedef TStrAdapter::TCont      TStrCont;
    typedef TColorAdapter::TCont    TColorCont;
    typedef TEnumAdapter::TCont     TEnumCont;

    TStrCont&    GetLabelContainer()    {   return dynamic_cast<TStrAdapter*>(GetArray(0))->GetContainer();    }
    TColorCont&  GetColorContainer()    {   return dynamic_cast<TColorAdapter*>(GetArray(1))->GetContainer();    }
    TEnumCont&   GetMarkerContainer()    {   return dynamic_cast<TEnumAdapter*>(GetArray(2))->GetContainer();    }

    virtual void    CreateArrays();
};

END_NCBI_SCOPE

#endif
