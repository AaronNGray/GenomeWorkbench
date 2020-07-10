#ifndef GUI_GRAPH___SCATTER_GRAPH__HPP
#define GUI_GRAPH___SCATTER_GRAPH__HPP

/*  $Id: scatter_graph.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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

#include <gui/graph/igraph.hpp>
#include <gui/graph/igraph_data.hpp>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IScatterDataSource

class IScatterDataSource : public IGraphDataSource
{
public:
    virtual INumericArray*  GetXArray() = 0;
    virtual INumericArray*  GetYArray() = 0;
    virtual IStringArray*   GetLabelsArray() = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// CScatterDataSource
// underlying types used for storing X and Y values can be parametrized
// Here we use vector as a container type, it can be parametrized as well if needed.
template <class TX = TModelUnit, class TY = TModelUnit>
    class CScatterDataSource
        : public CSeriesBase, public IScatterDataSource
{
public:
    CScatterDataSource(int Length) : CSeriesBase(Length)    {}

    // IScatterGraph interface
    virtual INumericArray*  GetXArray()  {   return x_GetNumericArray(0); }
    virtual INumericArray*  GetYArray()  {   return x_GetNumericArray(1); }
    virtual IStringArray*   GetLabelsArray()  {   return x_GetStringArray(2); }
protected:
    typedef CTypedArrayAdapter<IDataArray::eNumeric, TX> TXAdapter;
    typedef CTypedArrayAdapter<IDataArray::eNumeric, TY> TYAdapter;
    typedef CTypedArrayAdapter<IDataArray::eString> TStrAdapter;

public:
    typedef typename TXAdapter::TCont    TXCont;
    typedef typename TYAdapter::TCont    TYCont;
    typedef typename TStrAdapter::TCont  TStrCont;

    // Data Manipulation interface
    TXCont&  GetXContainer()    {   return dynamic_cast<TXAdapter*>(GetArray(0))->GetContainer();    }
    TYCont&  GetYContainer()    {   return dynamic_cast<TYAdapter*>(GetArray(1))->GetContainer();    }
    TStrCont&  GetLabelContainer()    {   return dynamic_cast<TStrAdapter*>(GetArray(2))->GetContainer();    }

    virtual void    CreateArrays()
    {
        CSeriesBase::CreateArrays();
        TXAdapter* pXAd = new TXAdapter(m_Length);
        AddArray(/*"X", */static_cast<IDataArray*>(pXAd));

        TYAdapter* pYAd = new TYAdapter(m_Length);
        AddArray(static_cast<IDataArray*>(pYAd));

        TStrAdapter* pStrAd = new TStrAdapter(m_Length);
        AddArray(static_cast<IDataArray*>(pStrAd));
    }
};

///////////////////////////////////////////////////////////////////////////////
/// class CScatterGraph

class NCBI_GUIGRAPH_EXPORT CScatterGraph : public CGraphBase
{
public:
    CScatterGraph();

    CGraphDotMarker::EMarkerType GetMarkerType()  const;
    void        SetMarkerType(CGraphDotMarker::EMarkerType Type);

    bool    IsDrawLines() const;
    void    SetDrawLines(bool bDraw);

    // IGraph implementation
    virtual bool    SetDataSource(IGraphDataSource* pDS);
    virtual const   TModelRect&    GetLimits()    const;
    virtual void    Render(CGlPane* pViewport, TElemVector* elems = NULL);

protected:
    IScatterDataSource*    GetScatterDataSource()
            { return dynamic_cast<IScatterDataSource*>(m_pDataSource); };
    virtual void    CalculateLimits();

protected:
    CGraphDotMarker::EMarkerType m_MarkerType;
    bool    m_bDrawLines;
};

END_NCBI_SCOPE

#endif
