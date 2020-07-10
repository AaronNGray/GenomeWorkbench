#ifndef GUI_UTILS___SIMPLE_LAYOUT_POLICY__HPP
#define GUI_UTILS___SIMPLE_LAYOUT_POLICY__HPP

/*  $Id: simple_layout_policy.hpp 34827 2016-02-17 00:59:08Z rudnev $
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
 */

#include <gui/widgets/seq_graphic/layout_policy.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSimpleLayout is the simpliest layout policy that simply stack
/// a set of glyphs one on top of the other vertically.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSimpleLayout :
    public CObject,
    public ILayoutPolicy
{
public:
    enum ESortingType {
        eSort_No,
        eSort_BySeqPos, ///< seq start position
        eSort_BySeqSize ///< sequence length
        //eSort_ByName
    };

    typedef ESortingType    TSortingMethod;

    CSimpleLayout()
        : m_VertSpace(3)
        , m_TopMargin(2)
        , m_SortingType(eSort_No) {}

    virtual void BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const;

    int GetVertSpace() const;
    int GetTopMargin() const;
    TSortingMethod GetSortingType() const;

    void SetVertSpace(int d);
    void SetTopMargin(int m);
    void SetSortingType(TSortingMethod meth);

private:
    int             m_VertSpace;
    int             m_TopMargin;
    TSortingMethod  m_SortingType;
};


///////////////////////////////////////////////////////////////////////////////
/// CInlineLayout is the anther simple layout policy that put a list of
/// glyphs in a single line horizontally sorted by glyph's start position.
/// Additionally, if a glyph has sided label, the policy will force to
/// hide its label when there is not enough space between glyphs to fit
/// the label.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CInlineLayout :
    public CObject,
    public ILayoutPolicy
{
public:
    CInlineLayout()
        : m_TopMargin(2)
        , m_AllowOverlap(true) {}

    CInlineLayout(bool allow_overlap)
        : m_TopMargin(2)
        , m_AllowOverlap(allow_overlap) {}

    virtual void BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const;
    void BuildLayout(TObjectList& objs, SBoundingBox& bound) const;

    int GetTopMargin() const;
    void SetTopMargin(int d);

    void SetAllowOverlap(bool flag);

private:
    void x_BuildLayout1Row(TObjectList& objects,
        SBoundingBox& bound, bool side_labeling) const;

    void x_BuildLayoutMultiRows(TObjectList& objects,
        SBoundingBox& bound,bool side_labeling) const;

private:
    int             m_TopMargin;
    bool            m_AllowOverlap;
};


///////////////////////////////////////////////////////////////////////////////
/// COverlayLayout is the layout policy that arranges glyphs over the top of each other
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT COverlayLayout :
    public CObject,
    public ILayoutPolicy
{
public:
    virtual void BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const;
private:
};



///////////////////////////////////////////////////////////////////////////////
/// CSimpleLayout inline method implementation
///
inline
int CSimpleLayout::GetVertSpace() const
{
    return m_VertSpace;
}

inline
int CSimpleLayout::GetTopMargin() const
{
    return m_TopMargin;
}

inline
CSimpleLayout::TSortingMethod CSimpleLayout::GetSortingType() const
{
    return m_SortingType;
}

inline
void CSimpleLayout::SetVertSpace(int d)
{
    m_VertSpace = d;
}

inline
void CSimpleLayout::SetTopMargin(int m)
{
    m_TopMargin = m;
}

inline
void CSimpleLayout::SetSortingType(TSortingMethod meth)
{
    m_SortingType = meth;
}

///////////////////////////////////////////////////////////////////////////////
/// CInlineLayout inline method implementation
///
inline
int CInlineLayout::GetTopMargin() const
{
    return m_TopMargin;
}

inline
void CInlineLayout::SetTopMargin(int m)
{
    m_TopMargin = m;
}

inline
void CInlineLayout::SetAllowOverlap(bool flag)
{
    m_AllowOverlap = flag;
}

END_NCBI_SCOPE

#endif  // GUI_UTILS___SIMPLE_LAYOUT_POLICY__HPP
