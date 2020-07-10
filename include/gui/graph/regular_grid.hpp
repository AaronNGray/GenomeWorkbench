#ifndef GUI_GRAPH___REGULAR_GRID__HPP
#define GUI_GRAPH___REGULAR_GRID__HPP

/*  $Id: regular_grid.hpp 23556 2011-04-21 12:24:42Z wuliangs $
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

#include <gui/utils/rgba_color.hpp>

#include <gui/graph/igraph.hpp>
#include <gui/opengl/glpane.hpp>
#include <math.h>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CRegularGridGen

class NCBI_GUIGRAPH_EXPORT CRegularGridGen
{
public:
    class const_iterator;

    CRegularGridGen();
    virtual ~CRegularGridGen();

    //void SetScaleType(EScaleType TypeX, EScaleType TypeY);
    void SetIntegerMode(bool IntegerX, bool IntegerY);

    /// set limitations on the CellSize in pixels.
    void SetCellLimits(int Min, int Max);

    /// plus/minus one base to align grid to ruler bars.
    void EnableOneBased(bool en_x, bool en_y);
    void EnableMinusOneBased(bool en_x, bool en_y);

    /// allow adjusting step externally.
    void SetStep(double step);   
    void SetAutoStepUpdate(bool f);

    /// iteration interface.
    void    GenerateGrid(CGlPane* pPane, bool bHorz);
    const_iterator begin() const;
    const_iterator end() const;

protected:
    static double   SelectBaseStep(double MinV, double MaxV );
    static void     RoundRangeToStep(double& Start, double& Finish, double Step );
    static double   SelectScreenStep(double BaseStep,  double MinStep, double MaxStep);
protected:
    static const int ms_StepK[];

    int m_MinCellSize;
    int m_MaxCellSize;

    //EScaleType  m_XScaleType;
    //EScaleType  m_YScaleType;

    bool    m_IntegerX;
    bool    m_IntegerY;
    bool    m_OneBasedX;
    bool    m_OneBasedY;
    bool    m_MinusOneBasedX;
    bool    m_MinusOneBasedY;

    /// Automatic step generation.
    /// Set false to allow updating the step externally so that
    /// the grid is consistent with other rendering such as ruler.
    bool    m_AutoStepUpdate;

    /// Generation context.
    bool    m_bHorz;
    double  m_Start, m_Finish;
    double  m_Step;

public:
    class NCBI_GUIGRAPH_EXPORT const_iterator
    {
    public:
        typedef double value_type;
        typedef const value_type& const_reference;

        const_iterator()  // end iterator
            : m_Start(0), m_Finish(0), m_Step(0), m_Iter(0), m_Value(1)    {}

        const_iterator(double Start, double Finish, double Step)
            : m_Start(Start), m_Finish(Finish), m_Step(Step),  m_Iter(0), m_Value(Start)    {}

        const_reference operator*()  const
        {
            return m_Value;
        }
        const_iterator& operator++()
        {
            Inc();
            return (*this);
        }
        const_iterator operator++(int)
        {
            const_iterator Tmp = *this;
            Inc();
            return Tmp;
        }
        bool operator==(const const_iterator& I) const
        {
            return (IsEnd() && I.IsEnd())  ||
                   (m_Value == I.m_Value  &&  m_Step == I.m_Step  &&  m_Start == I.m_Start
                    && m_Finish == I.m_Finish  &&  m_Iter == I.m_Iter);
        }
        bool operator!=(const const_iterator& I) const
        {
            return ! (*this==I);
        }
    protected:
        bool IsEnd()   const   {   return m_Value > m_Finish;  }
        void Inc()
        {
            m_Value = m_Start + (++m_Iter) * m_Step;

            double snr = fabs(m_Value/m_Step);

            if (snr < 0.001)    // clean up computation error for 0
                m_Value = 0.;

            if (m_Value > m_Finish)
                m_Iter = -1;
        }

    protected:
        double m_Start;
        double m_Finish;
        double m_Step;
        int    m_Iter;
        double m_Value;
    };
};

///////////////////////////////////////////////////////////////////////////////
/// CRegularGridRenderer

class NCBI_GUIGRAPH_EXPORT CRegularGridRenderer
{
public:
    CRegularGridRenderer();

    void    EnableIntegerCentering(bool b_en) { m_bCentering = b_en; }

    void    SetColor(const CRgbaColor& color)
    { m_HorzColor = m_VertColor = color; }

    void    SetHorzColor(const CRgbaColor& color)
    { m_HorzColor = color; }

    void    SetVertColor(const CRgbaColor& color)
    { m_VertColor = color; }

    void    SetShowHorzGrid(bool f)
    { m_ShowHorzGrid = f; }

    void    SetShowVertGrid(bool f)
    { m_ShowVertGrid = f; }

    void    Render(CGlPane* pAreaPane, CGlPane* pPane, CRegularGridGen* pGenerator) const;

protected:
    double          m_bCentering;
    CRgbaColor      m_HorzColor;
    CRgbaColor      m_VertColor;
    bool            m_ShowHorzGrid;
    bool            m_ShowVertGrid;
};


///////////////////////////////////////////////////////////////////////////////
/// CRegularGridGen inline methods
inline
void CRegularGridGen::SetIntegerMode(bool IntegerX, bool IntegerY)
{
    m_IntegerX = IntegerX;
    m_IntegerY = IntegerY;
}

inline
void CRegularGridGen::EnableOneBased(bool en_x, bool en_y)
{
    m_OneBasedX = en_x;
    m_OneBasedY = en_y;
}

inline
void CRegularGridGen::EnableMinusOneBased(bool en_x, bool en_y)
{
    m_MinusOneBasedX = en_x;
    m_MinusOneBasedY = en_y;
}

inline
void CRegularGridGen::SetCellLimits(int Min, int Max)
{
    // set limitations on the CellSize in pixels
    m_MinCellSize = Min;  m_MaxCellSize = Max;
}

inline
void CRegularGridGen::SetStep(double step)
{ m_Step = step; }

inline
void CRegularGridGen::SetAutoStepUpdate(bool f)
{ m_AutoStepUpdate = f; }

inline
CRegularGridGen::const_iterator CRegularGridGen::begin() const
{ return const_iterator(m_Start, m_Finish, m_Step); }

inline
CRegularGridGen::const_iterator CRegularGridGen::end() const
{ return const_iterator(); }


END_NCBI_SCOPE

#endif
