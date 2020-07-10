#ifndef GUI_WIDGETS_GL___HISTOGRAM_GRAPH__HPP
#define GUI_WIDGETS_GL___HISTOGRAM_GRAPH__HPP

/*  $Id: histogram_graph.hpp 24846 2011-12-09 20:52:39Z wuliangs $
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

#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/glcolortable.hpp>
#include <gui/widgets/gl/irenderable.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// IHistogramGraphDS interface
class NCBI_GUIWIDGETS_GL_EXPORT IHistogramGraphDS
{
public:
    virtual ~IHistogramGraphDS()    {};

    /// returns start of the series in model space
    virtual double  GetLimit() const = 0;

    virtual double  GetStart() const = 0;
    virtual double  GetStop() const = 0;

    /// discrete size in model space
    virtual double  GetStep() const = 0;

    /// number of samples in the series
    virtual size_t     GetCount() const = 0;

    /// value of the sample with the given index
    virtual double  GetValue(size_t index) = 0;

    virtual double  GetMaxValue() = 0;

    virtual string  GetLabel() const = 0;

    virtual void    Update(double start, double stop) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// CHistogramGraph

class NCBI_GUIWIDGETS_GL_EXPORT CHistogramGraph :
    public CObject,
    public CRenderableImpl
{
public:
    class NCBI_GUIWIDGETS_GL_EXPORT SProperties   {
    public:
        CRgbaColor  m_MinColor;
        CRgbaColor  m_MaxColor;
        int m_Margin;

        SProperties();
    };

    CHistogramGraph(bool b_horz);
    virtual ~CHistogramGraph();

    virtual void    SetDataSource(IHistogramGraphDS* ds);
    virtual IHistogramGraphDS* GetDataSource();
    virtual const IHistogramGraphDS* GetDataSource() const;

    /// @name IRenderable implemenation
    /// @{
    virtual void        Render(CGlPane& pane);

    virtual TVPPoint    PreferredSize();

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    /// @}


    const SProperties&  GetProperties() const;
    void    SetProperties(const SProperties& props);

protected:
    virtual void    x_RenderHistogram(CGlPane& pane);

protected:
    bool    m_Horz;
    CIRef<IHistogramGraphDS>  m_DataSource;
    SProperties         m_Properties;
    CRgbaGradColorTable m_ColorTable;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL___HISTOGRAM_GRAPH__HPP
