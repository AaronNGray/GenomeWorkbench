/*  $Id: switch_point_glyph.cpp 19261 2009-05-11 12:37:30Z wuliangs $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *   CSwitchPointGlyph -- utility class to layout switch points
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/switch_point_glyph.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CSwitchPointGlyph::CSwitchPointGlyph(CRef<CSeqMapSwitchPoint> map_points)
    : m_MapPoint(map_points)
{
    m_Location.Reset(new CSeq_loc());
    m_Location->SetInt().SetFrom( map_points->m_MasterRange.GetFrom() );
    m_Location->SetInt().SetTo  ( map_points->m_MasterRange.GetTo() );
}


CRef<CSeqMapSwitchPoint> CSwitchPointGlyph::GetSwitchPoints() const
{
    return m_MapPoint;
}


CConstRef<CObject> CSwitchPointGlyph::GetObject(TSeqPos) const
{
    //return CConstRef<CObject>(m_MapPoint);
    return CConstRef<CObject>(m_MapPoint->m_FirstAlign);
}


void CSwitchPointGlyph::GetObjects(vector<CConstRef<CObject> >& objs)  const
{
    //CConstRef<CObject> obj = CConstRef<CObject>(m_MapPoint->m_FirstAlign.GetPointer());

    if (m_MapPoint->m_FirstAlign) {
        objs.push_back( CConstRef<CObject>(m_MapPoint->m_FirstAlign) );
    }
}


bool CSwitchPointGlyph::HasObject(CConstRef<CObject> obj) const
{
    return m_MapPoint->m_FirstAlign.GetPointer() == obj.GetPointer();
}


const objects::CSeq_align& CSwitchPointGlyph::GetAlignment(void) const
{
    return *m_MapPoint->m_FirstAlign;
}


const objects::CSeq_loc& CSwitchPointGlyph::GetLocation(void) const
{
    return *m_Location;
}


//TSeqRange CSwitchPointGlyph::GetRange(void) const
//{
//    return m_Location->GetTotalRange();
//}

void CSwitchPointGlyph::x_Draw() const
{
    ///TODO: implement me
}


void CSwitchPointGlyph::x_UpdateBoundingBox()
{
    ///TODO: implement me
}


// Access the sub-intervals in this feature
//void CSwitchPointGlyph::x_CalcIntervals(void)
//{
//    m_Intervals.clear();
//    CSeq_loc_CI iter(GetLocation());
//    for ( ;  iter;  ++iter) {
//        m_Intervals.push_back(iter.GetRange());
//    }
//}


END_NCBI_SCOPE
