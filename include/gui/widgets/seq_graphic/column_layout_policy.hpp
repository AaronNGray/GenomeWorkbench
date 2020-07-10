#ifndef GUI_WIDGETS_SEQ_GRAPHIC___COLUMN_LAYOUT_POLICY__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___COLUMN_LAYOUT_POLICY__HPP

/*  $Id: column_layout_policy.hpp 34827 2016-02-17 00:59:08Z rudnev $
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
/// CColumnLayout is for creating layout by sorting glyphs into 'columns'.
/// The 'column' here is more or less in shape of column, to exactly one
/// glyph per row in one 'column'.  The glyph in each column is sorted by pos.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CColumnLayout :
    public CObject,
    public ILayoutPolicy
{
public:
    CColumnLayout()
        : m_MinDist(0)
        , m_VertSpace(3)
        , m_LimitRowPerGroup(true)
    {}

    /// access the minimum distance between two objects.
    /// The distance is in bases.
    TSeqPos GetMinDist(void) const;
    void    SetMinDist(TSeqPos dist);
    void    SetVertSpace(int d);
    void    SetLimitRowPerGroup(bool f);

    /// @name ILayoutPolicy interface implementation.
    /// @{
    virtual void BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const;
    /// @}

private:
    /// Set the objects' position (Y), and update the bounding
    /// box of the group (of all objects).
    void x_SetObjectPos(TLayout& layout, SBoundingBox& bound) const;

private:
    /// The minimum distance we permit two items to be within, horizontally.
    TSeqPos m_MinDist;
    int     m_VertSpace;
    ///
    bool    m_LimitRowPerGroup;
};

///////////////////////////////////////////////////////////////////////////////
/// CColumnLayout inline methods.

inline
TSeqPos CColumnLayout::GetMinDist(void) const
{
    return m_MinDist;
}

inline
void CColumnLayout::SetMinDist(TSeqPos dist)
{
    m_MinDist = dist;
}

inline
void CColumnLayout::SetVertSpace(int d)
{
    m_VertSpace = d;
}

inline
void CColumnLayout::SetLimitRowPerGroup(bool f)
{
    m_LimitRowPerGroup = f;
}


END_NCBI_SCOPE

#endif	// GUI_WIDGETS_SEQ_GRAPHIC___COLUMN_LAYOUT_POLICY__HPP


