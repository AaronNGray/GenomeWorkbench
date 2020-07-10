#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LAYERED_LAYOUT_POLICY__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LAYERED_LAYOUT_POLICY__HPP

/*  $Id: layered_layout_policy.hpp 34827 2016-02-17 00:59:08Z rudnev $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/seq_graphic/layout_policy.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CLayeredLayout is a standard 2D layout policy.
/// This class can perform layout of a number of objects into a single panel of
/// multiple rows.  All the layout objects are spreaded out vertically with one
/// connecting to the other without overlapping.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CLayeredLayout :
    public CObject,
    public ILayoutPolicy
{
public:
    CLayeredLayout()
        : m_MinDist(0)
        , m_VertSpace(3)
        , m_TopMargin(2)
        , m_MaxRow(-1)
        , m_Sorted(false)
        , m_LimitRowPerGroup(true)
    {}

    /// access the minimum distance between two objects.
    /// The distance is in bases.
    TSeqPos GetMinDist(void) const;
    int     GetVertSpace() const;
    int     GetTopMargin() const;
    int     GetMaxRow();

    void    SetMinDist(TSeqPos dist);
    void    SetVertSpace(int d);
    void    SetTopMargin(int m);
    void    SetMaxRow(int max);
    void    SetMaxHeight(int height);
    void    SetSorted(bool flag);
    void    SetLimitRowPerGroup(bool f);

    /// @name ILayoutPolicy interface implementation.
    /// @{
    virtual void BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const;
    /// @}

private:
    /// build the layered layout in favor of object size.
    /// the larger objects will be arranged first, hence appear
    /// on the top. This algorithm tends to be more stable in
    /// terms of object position during zooming in/out, but it
    /// is more expensive when the object size is large.
    void x_LayerBySize(CLayoutGroup& group, SBoundingBox& bound) const;

    /// build the layered layout in favor of object seq position.
    /// The objects will be sorted by their sequence start position.
    /// Then arrange accordingly. This algorithm much cheaper compared
    /// to the previous. It will be used for cases that the object
    /// number is large.
    void x_LayerByPos(CLayoutGroup& group, SBoundingBox& bound) const;

protected:
    /// Set the objects' position (Y), and update the bounding
    /// box of the group (of all objects).
    virtual void x_SetObjectPos(TLayout& layout, SBoundingBox& bound) const;

private:
    /// The minimum distance we permit two items to be within, horizontally.
    TSeqPos m_MinDist;
    int     m_VertSpace;
    int     m_TopMargin;

    /// Maximal number of rows allowed.
    /// Any value <=0 means no limit.
    int     m_MaxRow;
    bool    m_Sorted;    ///< layout object already in sorted order.
    bool    m_LimitRowPerGroup;
};

///////////////////////////////////////////////////////////////////////////////
/// CLayeredLayout inline methods.
///
inline
TSeqPos CLayeredLayout::GetMinDist(void) const
{
    return m_MinDist;
}

inline
int CLayeredLayout::GetVertSpace() const
{
    return m_VertSpace;
}

inline
int CLayeredLayout::GetTopMargin() const
{
    return m_TopMargin;
}

inline
int CLayeredLayout::GetMaxRow()
{
    return m_MaxRow;
}

inline
void CLayeredLayout::SetMinDist(TSeqPos dist)
{
    m_MinDist = dist;
}

inline
void CLayeredLayout::SetVertSpace(int d)
{
    m_VertSpace = d;
}

inline
void CLayeredLayout::SetTopMargin(int d)
{
    m_TopMargin = d;
}

inline
void CLayeredLayout::SetMaxRow(int l)
{
    m_MaxRow = l;
}

inline
void CLayeredLayout::SetSorted(bool flag)
{
    m_Sorted = flag;
}

inline
void CLayeredLayout::SetLimitRowPerGroup(bool f)
{
    m_LimitRowPerGroup = f;
}


END_NCBI_SCOPE

#endif	// GUI_WIDGETS_SEQ_GRAPHIC___LAYERED_LAYOUT_POLICY__HPP


