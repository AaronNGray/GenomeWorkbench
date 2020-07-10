#ifndef GUI_UTILS___LAYOUT_OBJ__HPP
#define GUI_UTILS___LAYOUT_OBJ__HPP

/*  $Id: layout.hpp 26085 2012-07-18 18:52:44Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/opengl/gltypes.hpp>
#include <objects/seqloc/Seq_loc.hpp>


/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


///
/// class CLayoutObject defines an interface that wraps a rectilinear abstract
/// object.
///
/// CLayoutObject defines how an axis-aligned rectilinear object appears to a
/// layout algorithm.  The basic concepts supported include:
///  -- querying rectilnear position or interval as a total range
///  -- functors for sorting
///  -- Draw() for rendering the object, once it is laid out
///
class CLayoutObject : public CObject
{
public:
    typedef vector< CRef<CLayoutObject> > TObjects;
    typedef vector< CConstRef<CLayoutObject> > TConstObjects;

    /// type of layout objects currently supported
    enum EType {
        eAlign,
        eAlignPairwise,
        eAlignSmear,
        eComment,
        eFeat,
        eFeatLabel,
        eFeatPack,
        eFeatProtProduct,
        eFeatVariation,
        eGraph,
        eHistogram,
        eAssnStudy,
        eLabel,
        eMatepair,
        eSeqMap,
        eSequence,
        eSwitchPoints,
        eVisRange
    };

    // how to layout children
    enum EChildrenLayout {
        eInline,
        eIndividual
    };


    CLayoutObject();

    virtual ~CLayoutObject();

    /// access the position of this object.
    virtual const objects::CSeq_loc& GetLocation(void) const = 0;

    /// access the position of this object.
    virtual TSeqRange GetRange(void) const = 0;

    /// compare this object to another, based on position
    virtual bool LessByPos (const CLayoutObject& obj) const = 0;

    /// compare this object to another, based on size
    virtual bool LessBySize(const CLayoutObject& obj) const = 0;

    /// access our core component - we wrap an object(s) of some sort.
    /// This returns the object at a given sequence position; this is useful if
    /// the layout object wraps more than one object
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const = 0;

    /// retrieve CObjects corresponding to this CLayoutObject
    virtual void GetObjects(vector<CConstRef<CObject> >& objs) const = 0;

    /// check if the wrapped object(s) is the one
    virtual bool HasObject(CConstRef<CObject> obj) const = 0;

    /// retrieve the type of this object
    virtual EType GetType() const = 0;

    /// object is selected
    bool IsSelected() const;
    void SetSelected(bool flag);

    // access sub-intervals (if any)
    const vector<TSeqRange>& GetIntervals(void) const;

    // Accessors for parent-child relationships in features
    const CLayoutObject* GetParent  (void) const;
    void                 SetParent(CLayoutObject* feat);

    TObjects&        SetChildren(void);
    const TObjects&  GetChildren(void) const;

    void SetChildrenLayout(EChildrenLayout type);
    EChildrenLayout GetChildrenLayout(void) const;

    /// area rectangle of the object in model space
    TModelRect GetModelRect() const;
    void SetModelRect(const TModelRect& rect);

    // horizontal / vertical orientation
    void SetHorizontal(bool b_horz);
    bool IsHorizontal() const;

    // strand is flipped
    void SetFlippedStrand(bool b_flip);
    bool IsFlippedStrand() const;

protected:
    bool m_Selected;
    bool m_bHorz;
    bool m_bFlipStrand;
    bool m_bInlineChildren;

    // parent / child relationships for this feature
    CLayoutObject*  m_Parent;
    TObjects        m_Children;

    TModelRect m_Rect;

    // intervals (like for features or alignments)
    mutable vector<TSeqRange>  m_Intervals;

    // calculate our intervals
    virtual void x_CalcIntervals(void) const;

};


inline
bool operator< (const CLayoutObject& o1, const CLayoutObject& o2)
{
    return o1.LessByPos(o2);
}


///
/// basic sort by position function
///
struct SLayoutByPos
{
    bool operator() (const CRef<CLayoutObject>& ref_obj1,
                     const CRef<CLayoutObject>& ref_obj2) const
    {
        return (ref_obj1->LessByPos(*ref_obj2));
    }

    bool operator() (const CLayoutObject& obj1,
                     const CLayoutObject& obj2) const
    {
        return (obj1.LessByPos(obj2));
    }

};


///
/// basic sort by size function
///
struct SLayoutBySize
{
    bool operator() (const CRef<CLayoutObject>& ref_obj1,
                     const CRef<CLayoutObject>& ref_obj2) const
    {
        return (ref_obj1->LessBySize(*ref_obj2));
    }

    bool operator() (const CLayoutObject& obj1,
                     const CLayoutObject& obj2) const
    {
        return (obj1.LessBySize(obj2));
    }

};


///
/// class CLayout is a container of objects laid out in a series of rows or
/// tracks.  The meaning of each row is left to the user; it is possible, for
/// example, to create two layout objects and append the layout of one to
/// another.
///
class CLayout : public CObject
{
public:
    typedef vector< CRef<CLayoutObject> > TLayoutRow;
    typedef vector<TLayoutRow>            TLayout;

    /// clear opur internal list of objects
    void Clear(void);

    /// access the entire layout
    const TLayout&  GetLayout(void) const;
    TLayout&        SetLayout(void);

    /// access a row of the layout
    const TLayoutRow& GetRow(size_t row) const;
    TLayoutRow&       SetRow(size_t row);

    /// add a row to the layout
    TLayoutRow& AddRow(void);
    TLayoutRow& AddRow(const TLayoutRow& row);

    /// append an entire layout to this one
    void Append(const CLayout& layout);

    /// insert an entire layout to this one
    void Insert(const CLayout& layout, size_t row);

    // true is no rows in layout
    bool IsEmpty() const;

private:

    /// our layout.  This is a nested container - a set of rows.
    TLayout m_Layout;
};


///
/// class CLayoutEngine defines the abstract interface for a given alyout
/// algorithm.  This interface imposes little on the actual functioning of the
/// layout engine, beyond the fact that it takes a list of layout objects and
/// produced a layout.
///
class CLayoutEngine : public CObject
{
public:
    typedef vector< CRef<CLayoutObject> > TObjects;

    /// perform our layout functions
    virtual void Layout(TObjects& objects, CLayout& layout, bool side_label = false) = 0;
};


///
/// class C2DLayoutEngine defines the interface for a standard 2D layout engine.
/// This class can perform layout of a number of objects into a single panel f
/// multiple rows.
///
class C2DLayoutEngine : public CLayoutEngine
{
public:
    C2DLayoutEngine()
        : m_MinDist(0)
        , m_SideSpace(0)
    {}

    /// access the minimum distance, in bases, that we permit between objects on
    /// a given row
    TSeqPos GetMinDist(void) const;
    void    SetMinDist(TSeqPos dist);
    void    SetSideSpace(TSeqPos space);

    virtual void Layout(TObjects& objects, CLayout& layout, bool side_label = false);

private:
    /// the minimum distance we permit two items to be within
    TSeqPos m_MinDist;
    TSeqPos m_SideSpace;    ///< preserved left side space for label.
};


inline
TSeqPos C2DLayoutEngine::GetMinDist(void) const
{
    return m_MinDist;
}


inline
void C2DLayoutEngine::SetMinDist(TSeqPos dist)
{
    m_MinDist = dist;
}

inline
void C2DLayoutEngine::SetSideSpace(TSeqPos space)
{
    m_SideSpace = space;
}

END_NCBI_SCOPE

/* @} */

#endif  /// GUI_UTILS___LAYOUT_OBJ__HPP
