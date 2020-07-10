/*  $Id: layout.cpp 26082 2012-07-18 18:05:51Z katargir $
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

#include <ncbi_pch.hpp>
#include <gui/widgets/aln_crossaln/layout.hpp>
#include <corelib/ncbiutil.hpp>


BEGIN_NCBI_SCOPE


//
//
// CLayoutObject
//
//


CLayoutObject::CLayoutObject()
    : m_Selected(false)
    , m_bHorz(true)
    , m_bFlipStrand(false)
    , m_bInlineChildren(false)
    , m_Parent(NULL)
{
}


// virtual dtor for our layout objects
CLayoutObject::~CLayoutObject()
{
}


bool CLayoutObject::IsSelected() const
{
    return m_Selected;
}


void CLayoutObject::SetSelected(bool flag)
{
    m_Selected = flag;
}


TModelRect CLayoutObject::GetModelRect() const
{
    return m_Rect;
}


void CLayoutObject::SetModelRect(const TModelRect& rc)
{
    m_Rect = rc;
}


void CLayoutObject::SetHorizontal(bool horz)
{
    m_bHorz = horz;
}


bool CLayoutObject::IsHorizontal() const
{
    return m_bHorz;
}


void CLayoutObject::SetFlippedStrand(bool b_flip)
{
    m_bFlipStrand = b_flip;
}


bool CLayoutObject::IsFlippedStrand() const
{
    return m_bFlipStrand;
}


// Accessors for parent-child relationships in features
const CLayoutObject* CLayoutObject::GetParent(void) const
{
    return m_Parent;
}


const CLayoutObject::TObjects& CLayoutObject::GetChildren(void) const
{
    return m_Children;
}


void CLayoutObject::SetParent(CLayoutObject* feat)
{
    m_Parent = feat;
}


CLayoutObject::TObjects& CLayoutObject::SetChildren(void)
{
    return m_Children;
}


void CLayoutObject::SetChildrenLayout(EChildrenLayout type)
{
    m_bInlineChildren = type == eInline ? true : false;
}


CLayoutObject::EChildrenLayout CLayoutObject::GetChildrenLayout(void) const
{
    return m_bInlineChildren ? eInline : eIndividual;
}


void CLayoutObject::x_CalcIntervals(void) const
{
}


// Access the sub-intervals in this object
const vector<TSeqRange>& CLayoutObject::GetIntervals(void) const
{
    if (m_Intervals.size() == 0) {
        x_CalcIntervals();
    }
    return m_Intervals;
}

//
//
//
// CLayout
//
//
//


// clear opur internal list of objects
void CLayout::Clear(void)
{
    m_Layout.clear();
}


// access the entire layout
const CLayout::TLayout& CLayout::GetLayout(void) const
{
    return m_Layout;
}


CLayout::TLayout& CLayout::SetLayout(void)
{
    return m_Layout;
}


// access a row of the layout
const CLayout::TLayoutRow& CLayout::GetRow(size_t row) const
{
    _ASSERT(row < m_Layout.size());
    return m_Layout[row];
}


CLayout::TLayoutRow& CLayout::SetRow(size_t row)
{
    _ASSERT(row < m_Layout.size());
    return m_Layout[row];
}


// add a new row to the layout
CLayout::TLayoutRow& CLayout::AddRow(void)
{
    return AddRow(TLayoutRow());
}


// add a new row to the layout
CLayout::TLayoutRow& CLayout::AddRow(const TLayoutRow& row)
{
    m_Layout.push_back(row);
    return m_Layout.back();
}


void CLayout::Append(const CLayout& layout)
{
    ITERATE (TLayout, row_iter, layout.GetLayout()) {
        m_Layout.push_back(*row_iter);
    }
}


void CLayout::Insert(const CLayout& layout, size_t row)
{
    TLayout::iterator it = m_Layout.begin() + row;
    ITERATE (TLayout, row_iter, layout.GetLayout()) {
        m_Layout.insert(it, *row_iter);
        it++;
    }
}


bool CLayout::IsEmpty() const
{
    return m_Layout.size() == 0;
}



//
//
//
// C2DLayoutEngine
//
//
//

///
/// sorting functor for finding a range inside a collection of layout objects
///
struct SRangeSorter
{
    bool operator()(const CRef<CLayoutObject>& obj1,
                    const CRef<CLayoutObject>& obj2) const
    {
        //return (obj1->GetRange().GetToOpen() < obj2->GetRange().GetFrom());
        return (obj1->GetRange() < obj2->GetRange()  &&
                obj1->GetRange().GetTo() < obj2->GetRange().GetFrom());
    }
};

class CDummyLayoutObject : public CLayoutObject
{
public:
    CDummyLayoutObject()
    {
    }

    void SetRange(const TSeqRange& range)
    {
        m_Range = range;
    }

    /// access the position of this object.
    TSeqRange GetRange(void) const
    {
        return m_Range;
    }

    ///
    /// all other interfaces to this class throw an exception
    ///

    /// access the position of this object.
    const objects::CSeq_loc& GetLocation(void) const
    {
        throw runtime_error("GetLocation() not implemented");
    }

    /// compare this object to another, based on position
    bool LessByPos (const CLayoutObject&) const
    {
        throw runtime_error("LessByPos() not implemented");
    }

    /// compare this object to another, based on size
    bool LessBySize(const CLayoutObject&) const
    {
        throw runtime_error("LessBySize() not implemented");
    }

    /// access our core component - we wrap an object(s) of some sort.
    /// This returns the object at a given sequence position; this is useful if
    /// the layout object wraps more than one object
    CConstRef<CObject> GetObject(TSeqPos /*pos*/) const
    {
        throw runtime_error("GetObject() not implemented");
    }

    /// retrieve CObjects corresponding to this CLayoutObject
    void GetObjects(vector<CConstRef<CObject> >& /*objs*/) const
    {
        throw runtime_error("GetObjects() not implemented");
    }

    /// check if the wrapped object(s) is the one
    bool HasObject(CConstRef<CObject>) const
    {
        throw runtime_error("HasObject() not implemented");
    }

    /// retrieve the type of this object
    EType GetType() const
    {
        throw runtime_error("GetType() not implemented");
    }

private:
    TSeqRange m_Range;
};


// perform 2D layout
// this arranges a set of layout objects into a non-overlapping series of rows
void C2DLayoutEngine::Layout(TObjects& objs, CLayout& layout, bool side_label)
{
    //typedef list< CRangeCollection<TSeqPos> > TRowRanges;
    //TObjects objs(objects);
    std::sort(objs.begin(), objs.end(), SLayoutBySize());

    SRangeSorter func;

    layout.Clear();

    CDummyLayoutObject range_holder;
    CRef<CLayoutObject> ref_obj(&range_holder);

    TObjects::const_reverse_iterator riter(objs.end());
    TObjects::const_reverse_iterator rend (objs.begin());
    for ( ;  riter != rend;  ++riter) {
        CRef<CLayoutObject> ref(*riter);
        bool need_extra_space = false;
//         const CLayoutFeat* feat =
//             dynamic_cast<const CLayoutFeat*>(ref.GetPointer());
//         if (feat  &&
//             feat->GetFeature().GetData().GetSubtype() ==
//             objects::CSeqFeatData::eSubtype_gene
//             &&  !feat->GetChildren().empty()) {
//             need_extra_space = true;
//         }

        TSeqRange intersect_range = ref->GetRange();
        {{
            TSeqPos from_off = m_MinDist * (need_extra_space ? 2 : 1);
            if (side_label) {
                from_off += m_SideSpace;
            }
            if (intersect_range.GetFrom() > from_off) {
                intersect_range.SetFrom(intersect_range.GetFrom() - from_off);
            } else {
                intersect_range.SetFrom(0);
            }
        }}
        {{
            TSeqPos expanded_to = intersect_range.GetTo() +
                m_MinDist * (need_extra_space ? 2 : 1);
            if (side_label) {
                expanded_to += m_SideSpace;
            }
            if (expanded_to > intersect_range.GetTo()) {
                /// guard against problems with whole ranges
                intersect_range.SetTo(expanded_to);
            }
        }}

        range_holder.SetRange(intersect_range);

        bool inserted = false;
        NON_CONST_ITERATE (CLayout::TLayout, iter, layout.SetLayout()) {
            bool intersects = false;

            CLayout::TLayoutRow::iterator row_iter =
                std::lower_bound(iter->begin(), iter->end(), ref_obj, func); /* NCBI_FAKE_WARNING: WorkShop */

            CRef<CLayoutObject> ref_row_iter;
            TSeqRange total_range = TSeqRange::GetWhole();
            if (row_iter != iter->end()) {
                ref_row_iter = *row_iter;
                total_range = (*row_iter)->GetRange();
                intersects =
                    intersect_range.IntersectingWith
                    ((*row_iter)->GetRange());
            }
            if ( !intersects ) {
                inserted = true;
                iter->insert(row_iter, ref);
                break;
            }
        }
        if ( !inserted ) {
            CLayout::TLayoutRow& row = layout.AddRow();
            row.push_back(ref);
        }
        /**/

        /**
        CRef<CLayoutObject> obj = *riter;
        TSeqRange range = obj->GetLocation().GetTotalRange();
        TSeqRange test_range(range);
        if (test_range.GetFrom() > m_MinDist) {
            test_range.SetFrom(test_range.GetFrom() - m_MinDist);
        } else {
            test_range.SetFrom(0);
        }
        test_range.SetTo(test_range.GetTo() + m_MinDist);

        bool placed = false;
        size_t row = 0;
        TRowRanges::iterator row_iter = ranges.begin();
        for ( ;  row_iter != ranges.end();  ++row_iter, ++row) {
            if ( !row_iter->IntersectingWith(test_range) ) {
                break;
            }
        }

        if (row_iter == ranges.end()) {
            // begin an entirely new row
            layout.AddRow();
            ranges.push_back(CRangeCollection<TSeqPos>());
            row_iter = ranges.end();
            --row_iter;
        }

        CLayout::TLayoutRow& lr = layout.SetRow(row);
        lr.push_back(obj);
        *row_iter += range;
        **/
    }



#if 0
    // clear our layout to start
    layout.Clear();

    // for each member in the incoming list...
    ITERATE (TObjects, iter, objects) {
        TSeqRange new_range;
        try {
            new_range = (*iter)->GetRange();
        }
        catch (CException&) {
            continue;
        }

        TSignedSeqPos start = 0;
        TSignedSeqPos end   = 0;
        if (new_range.GetFrom() < new_range.GetTo()) {
            start = new_range.GetFrom() - m_MinDist;
            end   = new_range.GetTo()   + m_MinDist;
        } else {
            start = new_range.GetTo()   - m_MinDist;
            end   = new_range.GetFrom() + m_MinDist;
        }

        // scan the current layout
        bool inserted = false;
        NON_CONST_ITERATE (CLayout::TLayout, row_iter, layout.SetLayout()) {
            // ... and try to insert at the end
            // since we're sorted by position, we only need to check the end
            const CLayoutObject& last_obj = *row_iter->back();
            const TSeqRange& last_range = last_obj.GetRange();
            if (last_range.GetFrom() < last_range.GetTo()) {
                if (TSignedSeqPos(last_range.GetTo()) <= start) {
                    row_iter->push_back(*iter);
                    inserted = true;
                    break;
                }
            } else {
                if (TSignedSeqPos(last_range.GetFrom()) <= start) {
                    row_iter->push_back(*iter);
                    inserted = true;
                    break;
                }
            }
        }

        // insert our interval info as we need
        if ( !inserted ) {
            // begin an entirely new row
            CLayout::TLayoutRow& row = layout.AddRow();
            row.push_back(*iter);
        }
    }

#endif

#if 0
    {
        int i;
        int j;

        _TRACE("\n" << features.size() << " items, spacing = " << spacing);
        for (i = 0;  i < features.size();  ++i) {
            _TRACE(i + 1 << ": "
                   << features[i]->GetTextId() << ": {"
                   << features[i]->GetExtent().GetFrom() << " - "
                   << features[i]->GetExtent().GetTo() << "}");
        }

        for (i = 0;  i < pack.size();  ++i) {
            _TRACE(i + 1 << ":");
            for (j = 0;  j < pack[i].size();  ++j) {

                dumpRecursive(*pack[i][j]);
            }
            cerr << endl;
        }
        cerr << endl;
    }
#endif
}


END_NCBI_SCOPE
