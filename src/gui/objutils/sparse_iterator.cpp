/*  $Id: sparse_iterator.cpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <ncbi_pch.hpp>

#include <gui/objutils/sparse_iterator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


////////////////////////////////////////////////////////////////////////////////
/// CSparseIterator

inline void CSparseIterator::x_InitSegment()
{
    if( ! (bool)*this)   {
        m_Segment.Init(-1, -1, -1, -1, IAlnSegment::fInvalid); // invalid
    } else {
        int dir_flag = m_It_1->IsDirect() ? 0 : IAlnSegment::fReversed;

        if(m_It_1 == m_It_2)  {   // aligned segment
            if(m_Clip  &&  (m_It_1 == m_Clip->m_First_It  ||  m_It_1 == m_Clip->m_Last_It_1))  {
                // we need to clip the current segment
                TAlignRange r = *m_It_1;
                CRange<TSignedSeqPos> clip(m_Clip->m_From, m_Clip->m_ToOpen - 1);
                r.IntersectWith(clip);

                m_Segment.Init(r.GetFirstFrom(), r.GetFirstTo(),
                           r.GetSecondFrom(), r.GetSecondTo(), IAlnSegment::fAligned | dir_flag);
            } else {
                const TAlignRange& r = *m_It_1;
                m_Segment.Init(r.GetFirstFrom(), r.GetFirstTo(),
                               r.GetSecondFrom(), r.GetSecondTo(), IAlnSegment::fAligned | dir_flag);
            }
        } else {  // gap
            TSignedSeqPos from = m_It_2->GetSecondToOpen();
            TSignedSeqPos to = m_It_1->GetSecondFrom() - 1;

            if(m_Clip  &&  (m_It_1 == m_Clip->m_First_It  ||  m_It_1 == m_Clip->m_Last_It_1))  {
                TSignedRange r(m_It_2->GetFirstToOpen(), m_It_1->GetFirstFrom() - 1);
                TSignedRange clip(m_Clip->m_From, m_Clip->m_ToOpen - 1);
                r.IntersectWith(clip);

                m_Segment.Init(r.GetFrom(), r.GetTo(), from, to, IAlnSegment::fGap);
            } else {
                m_Segment.Init(m_It_2->GetFirstToOpen(), m_It_1->GetFirstFrom() - 1,
                               from, to, IAlnSegment::fGap);
            }
        }
    }
}

// assuming clipping range
void CSparseIterator::x_InitIterator()
{
    _ASSERT(m_Coll);

    bool first_gap = false, last_gap = false;
    if(m_Clip)   {
        // adjsut starting position
        TSignedSeqPos from = m_Clip->m_From;

        pair<TAlignColl::const_iterator, bool> res = m_Coll->find_2(from);

        // set start position
        if(res.second)  {
            _ASSERT(res.first->FirstContains(from)); // pos inside a segment
            m_Clip->m_First_It = res.first;
        } else {    // pos is in the gap
            m_Clip->m_First_It = res.first;
            if(res.first != m_Coll->begin())    {
                first_gap = true;
            }
        }

        // set end condition
        TSignedSeqPos to = m_Clip->m_ToOpen - 1;
        res = m_Coll->find_2(to);

        if(res.second)  {
            _ASSERT(res.first->FirstContains(to)); // pos inside a segment
            m_Clip->m_Last_It_1 = m_Clip->m_Last_It_2 = res.first;
        } else {
            // pos is in the gap
            if(res.first == m_Coll->end())   {
                m_Clip->m_Last_It_1 = m_Clip->m_Last_It_2 = res.first - 1;
            } else {
                m_Clip->m_Last_It_1 = res.first;
                m_Clip->m_Last_It_2 = res.first - 1;
                last_gap = true;
            }
        }
        // initialize iterators
        m_It_2 = m_It_1 = m_Clip->m_First_It;
    } else { // no clip
        m_It_1 = m_It_2 = m_Coll->begin();
    }

    // adjsutiterators
    switch(m_Flag)  {
    case eAllSegments:
        if(first_gap)   {
            --m_It_2;
        }
        break;
    case eSkipGaps:
        if(last_gap)    {
            --m_Clip->m_Last_It_1;
        }
        break;
    case eInsertsOnly:
        if(first_gap)   {
            --m_It_2;
        } else {
            ++m_It_1;
        }
        if( ! x_IsInsert()) {
            ++(*this);
        }
        break;
    case eSkipInserts:
        if(first_gap  &&  x_IsInsert()) {
            ++(*this);
        }
    }

    x_InitSegment();
}


CSparseIterator::CSparseIterator()
:   m_Coll(NULL),
    m_Clip(NULL)
{
    x_InitSegment();
}


CSparseIterator::CSparseIterator(const TAlignColl& coll, EFlags flag)
:   m_Coll(&coll),
    m_Flag(flag),
    m_Clip(NULL)
{
    x_InitIterator();
}


CSparseIterator::CSparseIterator(const TAlignColl& coll, EFlags flag,
                                 const IAlnExplorer::TSignedRange& range)
:   m_Coll(&coll),
    m_Flag(flag),
    m_Clip(NULL)
{
    if(m_Coll)  {
        m_Clip = new SClip;
        m_Clip->m_From = range.GetFrom();
        m_Clip->m_ToOpen = range.GetToOpen();
    }

    x_InitIterator();
}


CSparseIterator::CSparseIterator(const CSparseIterator& orig)
:   m_Coll(orig.m_Coll),
    m_Flag(orig.m_Flag),
    m_It_1(orig.m_It_1),
    m_It_2(orig.m_It_2)
{
    if(orig.m_Clip)  {
        m_Clip = new SClip(*orig.m_Clip);
    }
    x_InitSegment();
}


CSparseIterator::~CSparseIterator()
{
    delete m_Clip;
}


IAlnSegmentIterator*    CSparseIterator::Clone() const
{
    return new CSparseIterator(*this);
}


CSparseIterator::operator bool() const
{
    if(m_Coll  &&  m_It_1 >= m_Coll->begin())  {
        if(m_Clip)  {
            return  (m_It_1 <= m_Clip->m_Last_It_1)  &&   (m_It_2 <= m_Clip->m_Last_It_2);
        }
    }
    return false;
}


IAlnSegmentIterator& CSparseIterator::operator++()
{
    _ASSERT(m_Coll);

    switch(m_Flag)  {
    case eAllSegments:
        if(m_It_1 == m_It_2)   {  // aligned segment - move to gap
            ++m_It_1;
        } else {    // gap - move to the next segment
            ++m_It_2;
        }
        break;
    case eSkipGaps:
        ++m_It_1;
        ++m_It_2;
        break;
    case eInsertsOnly:
        do {
            ++m_It_1;
            ++m_It_2;
        } while((bool)*this  &&  m_It_1->GetFirstFrom() != m_It_2->GetFirstToOpen());
        break;
    case eSkipInserts:
        if(m_It_1 == m_It_2)   {  // aligned segment - move to gap
            ++m_It_1;
            if(m_It_1->GetFirstFrom() == m_It_2->GetFirstTo())   { // insert - skip
                ++m_It_2;
            }
        } else {    // gap - move to the next segment
            ++m_It_2;
        }
        break;
    default:
        _ASSERT(false); // not implemented
    }

    x_InitSegment();
    return *this;
}


bool CSparseIterator::operator==(const IAlnSegmentIterator& it) const
{
    if(typeid(*this) == typeid(it)) {
        const CSparseIterator* sparse_it =
            dynamic_cast<const CSparseIterator*>(&it);
        return x_Equals(*sparse_it);
    }
    return false;
}


bool CSparseIterator::operator!=(const IAlnSegmentIterator& it) const
{
    if(typeid(*this) == typeid(it)) {
        const CSparseIterator* sparse_it =
            dynamic_cast<const CSparseIterator*>(&it);
        return ! x_Equals(*sparse_it);
    }
    return true;
}


const CSparseIterator::value_type& CSparseIterator::operator*() const
{
    _ASSERT(*this);
    return m_Segment;
}


const CSparseIterator::value_type* CSparseIterator::operator->() const
{
    _ASSERT(*this);
    return &m_Segment;
}


END_NCBI_SCOPE
