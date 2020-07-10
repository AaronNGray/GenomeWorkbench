#ifndef GUI_GRAPH___IGRAPH__HPP
#define GUI_GRAPH___IGRAPH__HPP

/*  $Id: igraph.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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
#include <corelib/ncbistd.hpp>

#include <gui/graph/igraph_utils.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/utils/rgba_color.hpp>

#include <vector>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
/// IGraphDataSource

class NCBI_GUIGRAPH_EXPORT IGraphDataSource
{
public:
    virtual ~IGraphDataSource();

};

struct SGraphElem;

/////////////////////////////////////////////////////////////////////////////
/// IGraph

// pointers or refs ?
// const versions of functions ?
// throw exceptions for incompatible data sources ?
class NCBI_GUIGRAPH_EXPORT IGraph
{
public:
    typedef vector<SGraphElem*> TElemVector;

    virtual ~IGraph();

    virtual bool    SetDataSource(IGraphDataSource* pDS) = 0;
    virtual IGraphDataSource*   GetDataSource() = 0;

    virtual const TModelRect&    GetLimits()    const = 0;

    virtual void    Render(CGlPane* pPane, TElemVector* elems = NULL) = 0;
};


/////////////////////////////////////////////////////////////////////////////
/// SGraphElem
struct NCBI_GUIGRAPH_EXPORT SGraphElem
{
    IGraph* m_Graph;

    SGraphElem() : m_Graph(0)   {}
};


/////////////////////////////////////////////////////////////////////////////
/// CGraphBase

class NCBI_GUIGRAPH_EXPORT CGraphBase : public IGraph
{
public:
    CGraphBase();
    virtual ~CGraphBase();

    // IGraph implementation
    virtual bool    SetDataSource(IGraphDataSource* pDS);
    virtual IGraphDataSource*   GetDataSource();

    virtual const TModelRect&    GetLimits()    const;

    virtual void    SetColor(CRgbaColor Color)    {   m_Color = Color;    }
    virtual void    SetTextColor(CRgbaColor Color)    { m_TextColor = Color;  }
    virtual const CRgbaColor&     GetColor()  const   {   return m_Color; }
    virtual const CRgbaColor&     GetTextColor()  const   {   return m_TextColor; }

    virtual void    Render(CGlPane* pPane, TElemVector* elems = NULL) = 0;

protected:
    virtual void    CalculateLimits() = 0;

protected:
    TModelRect  m_Limits;
    IGraphDataSource* m_pDataSource;

    CRgbaColor    m_Color;
    CRgbaColor    m_TextColor;
};

/////////////////////////////////////////////////////////////////////////////
/// class CGraphDotMarker
class NCBI_GUIGRAPH_EXPORT CGraphDotMarker
{
public:
    enum EMarkerType {
        eNone,
        eRect,
        eDiamond,
        eTriangle,
        eCross
    };

    static void RenderMarker(TModelUnit cX, TModelUnit cY, TModelUnit MarkerW,
                            TModelUnit MarkerH, EMarkerType Type);
};

END_NCBI_SCOPE

#endif
