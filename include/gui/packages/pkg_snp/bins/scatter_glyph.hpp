#ifndef GUI_PACKAGES_SNP_BINS_TRACK___SCATTER_GLYPH__HPP
#define GUI_PACKAGES_SNP_BINS_TRACK___SCATTER_GLYPH__HPP

/*  $Id: scatter_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 * Authors:  Vlad Lebedev, Liangshou Wu, Dmitry Rudnev
 *
 * File Description:
 *    CScatterPlotGlyph -- utility class for having
 *                      bin-based scatter plots in graphical layouts.
 *
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/density_map.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <math.h>
#include <gui/packages/pkg_snp/bins/bins_glyph.hpp>

BEGIN_NCBI_SCOPE

/// the only requerement to TDataType is that it has a method GetValue() that returns some kind of float-castable value
//!! template<class TDataType>
class NCBI_GUIWIDGETS_SNP_EXPORT CScatterPlotGlyph : public CSeqGlyph
{
public:
    typedef CBinsGlyph::TDensityMap TMap;
    typedef double TDataType;

    /// @name ctors
    /// @{
    CScatterPlotGlyph(const TMap& map, const objects::CSeq_loc& loc);

    ~CScatterPlotGlyph();
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool OnLeftDblClick(const TModelPoint& /*p*/);
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool IsClickable() const;
    ///@}

    // access the position of this object.
    virtual TSeqRange GetRange(void) const;

    const TMap& GetDensityMap(void) const;

    void SetTitle(const string& title);
    const string& GetTitle() const;

    void SetAnnotName(const string& name);
    const string& GetAnnotName() const;

    /// Get the actually density max/min values.
    TDataType GetMax() const;
    TDataType GetMin() const;
    /// Get the axis limits (max/min).
    TDataType GetAxisMax() const;
    TDataType GetAxisMin() const;

    // set vertical axis min-max values based on density map min-max values
    void ComputeAxisRange();

private:
    string x_GetColorTheme() const;
    string x_GetDescription() const;

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

protected:
    string                          m_Title;
    string                          m_AnnotName;
    CConstRef<objects::CSeq_loc>    m_Location;
    TMap                            m_Map;

    TDataType               m_AxisMax;
    TDataType               m_AxisMin;

};

///////////////////////////////////////////////////////////////////////////////
/// CScatterPlotGlyph inline method implementation.

inline
const CScatterPlotGlyph::TMap& CScatterPlotGlyph::GetDensityMap(void) const
{
    return m_Map;
}



inline
void CScatterPlotGlyph::SetTitle(const string& title)
{ m_Title = title; }

inline
const string& CScatterPlotGlyph::GetTitle() const
{ return m_Title; }

inline
void CScatterPlotGlyph::SetAnnotName(const string& name)
{ m_AnnotName = name; }

inline
const string& CScatterPlotGlyph::GetAnnotName() const
{ return m_AnnotName; }

inline
CScatterPlotGlyph::TDataType CScatterPlotGlyph::GetMax() const
{
    return m_AxisMax;
}

inline
CScatterPlotGlyph::TDataType CScatterPlotGlyph::GetMin() const
{
    return m_AxisMin;
}

inline
CScatterPlotGlyph::TDataType CScatterPlotGlyph::GetAxisMax() const
{ return m_AxisMax; }

inline
CScatterPlotGlyph::TDataType CScatterPlotGlyph::GetAxisMin() const
{ return m_AxisMin; }



END_NCBI_SCOPE

/* @} */

#endif  // GUI_PACKAGES_SNP_BINS_TRACK___SCATTER_GLYPH__HPP
