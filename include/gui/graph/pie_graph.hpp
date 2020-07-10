#ifndef GUI_GRAPH___PIE_GRAPH__HPP
#define GUI_GRAPH___PIE_GRAPH__HPP

/*  $Id: pie_graph.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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

#include <gui/graph/igraph.hpp>
#include <gui/graph/igraph_data.hpp>
#include <gui/graph/legend.hpp>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
/// IPieDataSource

class IPieDataSource : public IGraphDataSource
{
public:
    /*virtual string GetName() = 0;

    virtual int GetElemCount() const = 0;
    virtual double  GetValue(int iElem) const = 0;
    virtual CRgbaColor    GetColor(int iElem) const = 0;
    virtual string  GetLabel(int iElem) const = 0;*/

    virtual INumericArray*  GetValueArray() = 0;
    virtual IColorArray*    GetColorArray() = 0;
    virtual IStringArray*   GetLabelsArray() = 0;
};

/////////////////////////////////////////////////////////////////////////////
/// CPieGraph

class NCBI_GUIGRAPH_EXPORT CPieGraph
    : public CGraphBase, public ILegendDataSource
{
public:
    CPieGraph();

    // IGraph implementation
    virtual bool    SetDataSource(IGraphDataSource* pDS);
    virtual void    Render(CGlPane* pViewport, TElemVector* elems = NULL);

    // ILegendDataSource implementation
    virtual bool    ShowMarkers();
    virtual IStringArray*   GetLabelArray();
    virtual IColorArray*    GetColorArray();
    virtual INumericArray*  GetMarkerArray();

protected:
    IPieDataSource*    GetPieDataSource()
            { return dynamic_cast<IPieDataSource*>(m_pDataSource); };
    virtual void    CalculateLimits();
};

/////////////////////////////////////////////////////////////////////////////
/// CPieDataSource

class NCBI_GUIGRAPH_EXPORT CPieDataSource
    : public CSeriesBase, public IPieDataSource
{
public:
    CPieDataSource(int SectorsN, const string& Name = "");

    virtual INumericArray*  GetValueArray() {   return x_GetNumericArray(0); };
    virtual IColorArray*    GetColorArray() {   return x_GetColorArray(1); };
    virtual IStringArray*   GetLabelsArray()    {   return x_GetStringArray(2); };
protected:
    typedef CTypedArrayAdapter<IDataArray::eNumeric> TValueAdapter;
    typedef CTypedArrayAdapter<IDataArray::eColor>   TColorAdapter;
    typedef CTypedArrayAdapter<IDataArray::eString>  TStringAdapter;

public:
    typedef TValueAdapter::TCont    TValueCont;
    typedef TColorAdapter::TCont    TColorCont;
    typedef TStringAdapter::TCont   TStringCont;

    TValueCont& GetValueContainer()    {   return dynamic_cast<TValueAdapter*>(GetArray(0))->GetContainer();    }
    TColorCont& GetColorContainer()    {   return dynamic_cast<TColorAdapter*>(GetArray(1))->GetContainer();    }
    TStringCont&   GetStringContainer()    {   return dynamic_cast<TStringAdapter*>(GetArray(2))->GetContainer();    }

    virtual void    CreateArrays();
protected:
    string m_Name;
};

END_NCBI_SCOPE

#endif
