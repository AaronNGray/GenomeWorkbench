#ifndef GUI_WIDGETS_SEQ_GRAPHIC___COMPACT_LAYOUT_POLICY__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___COMPACT_LAYOUT_POLICY__HPP

/*  $Id: compact_layout_policy.hpp 34827 2016-02-17 00:59:08Z rudnev $
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
/// CCompactLayout is amed to generate more compact 2D layout policy than
/// layered layout for glyphs with non-uniform height. But, it is also
/// more expensive in terms of performance.  This class can
/// perform layout of a number of objects into a single panel of non-layered
/// and non-overlapped layout.  All the layout objects are spreaded out
/// vertically with one connecting to the other without overlapping.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CCompactLayout :
    public CObject,
    public ILayoutPolicy
{
public:
    CCompactLayout()
        : m_MinDist(0)
        , m_VertSpace(3)
    {}

    /// access the minimum distance between two objects.
    /// The distance is in bases.
    TSeqPos GetMinDist(void) const;
    void    SetMinDist(TSeqPos dist);
    void    SetVertSpace(int d);

    /// @name ILayoutPolicy interface implementation.
    /// @{
    virtual void BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const;
    /// @}

private:
    /// The minimum distance we permit two items to be within, horizontally.
    TSeqPos m_MinDist;
    int     m_VertSpace;
};

///////////////////////////////////////////////////////////////////////////////
/// CCompactLayout inline methods.

inline
TSeqPos CCompactLayout::GetMinDist(void) const
{
    return m_MinDist;
}

inline
void CCompactLayout::SetMinDist(TSeqPos dist)
{
    m_MinDist = dist;
}

inline
void CCompactLayout::SetVertSpace(int d)
{
    m_VertSpace = d;
}


END_NCBI_SCOPE

#endif	// GUI_WIDGETS_SEQ_GRAPHIC___COMPACT_LAYOUT_POLICY__HPP


