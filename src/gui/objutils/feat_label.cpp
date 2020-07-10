/*  $Id: feat_label.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *   CLayoutFeatLabel -- utility class to layout feature labels
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/feat_label.hpp>



BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CLayoutFeatLabel::CLayoutFeatLabel(const CMappedFeat& feat)
    : CLayoutFeat(feat)
{
}


void CLayoutFeatLabel::SetRange(TSeqPos from, TSeqPos to)
{
    m_Location.Reset(new CSeq_loc());
    m_Location->SetInt().SetFrom(from);
    m_Location->SetInt().SetTo  (to);
}


CConstRef<CObject> CLayoutFeatLabel::GetObject(TSeqPos) const
{
    return CConstRef<CObject>(&m_Feature.GetOriginalFeature());
}


void CLayoutFeatLabel::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    objs.push_back( CConstRef<CObject>(&m_Feature.GetOriginalFeature()) );
}


const objects::CSeq_loc& CLayoutFeatLabel::GetLocation(void) const
{
    return *m_Location;
}


TSeqRange CLayoutFeatLabel::GetRange(void) const
{
    return m_Location->GetTotalRange();
}


CLayoutFeatLabel::EType CLayoutFeatLabel::GetType() const
{
    return eFeatLabel;
}


END_NCBI_SCOPE
