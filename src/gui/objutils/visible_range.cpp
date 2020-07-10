/*  $Id: visible_range.cpp 40878 2018-04-25 19:54:29Z katargir $
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
 * Authors:  Vladimir Tereshkov, Andrey Yazhuk, Mike Dicuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objmgr/util/sequence.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/objutils/visible_range.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

bool CVisibleRange::sm_AutoBroadcast = true;
CVisibleRange::EBasicPolicy CVisibleRange::sm_DefaultPolicy = eBasic_Track;

CVisibleRange::CVisibleRange(objects::CScope& scope)
    : m_Scope(&scope)
    , m_Policy(sm_DefaultPolicy)
{
}

CVisibleRange::~CVisibleRange()
{
}

void CVisibleRange::AddLocation(const objects::CSeq_loc& loc)
{
    m_Locs.push_back(CConstRef<CSeq_loc>(&loc));
}

const CVisibleRange::TLocations& CVisibleRange::GetLocations() const
{
    return m_Locs;
}

bool CVisibleRange::Match(const CSeq_id& other_id) const
{
    try {
        CRef<CScope> scope = m_Scope;

        ITERATE (TLocations, iter, m_Locs) {
            const CSeq_id& id = sequence::GetId(**iter, scope);
            if (CSeqUtils::Match(id, other_id, scope)) {
                return true;
            }
        }
    }
    catch (CException& e) {
        LOG_POST(Error << "CVisibleRange::Match(): error: " << e.GetMsg());
    }
    return false;
}

/// Alter the supplied range according to the rules provided in the
/// policy and the supplied new location
bool CVisibleRange::Clamp(const CBioseq_Handle& handle,
                               TSeqRange& range) const
{
    try {
        CRef<CScope> scope = m_Scope;
        ITERATE (TLocations, iter, m_Locs) {
            const CSeq_id& id = sequence::GetId(**iter, scope);
            if (CSeqUtils::Match(id, *handle.GetSeqId(), scope)) {
                return x_Clamp(**iter, handle, range);
            }
        }
    }
    catch (CException& e) {
        LOG_POST(Error << "CVisibleRange::Clamp(): error: " << e.GetMsg());
    }
    return false;
}

int CVisibleRange::x_GetPolicy() const
{
    switch (m_Policy) {
    case eBasic_TrackCenter :
        return eVisible_TrackCenter;
    case eBasic_TrackExpand :
        return eVisible_TrackExpand;
    case eBasic_TrackContract :
        return eVisible_TrackContract;
    case eBasic_Slave :
        return eVisible_Slave;
    case eBasic_Ignore :
        return eVisible_Ignore;
    default :
        return eVisible_Track;
    }
}

bool CVisibleRange::x_Clamp(const objects::CSeq_loc& loc,
                                 const objects::CBioseq_Handle& handle,
                                 TSeqRange& range) const
{
    int mode = x_GetPolicy();
    if (mode == eVisible_Ignore)
        return false;

    TSeqRange this_range(range);
    if (this_range.IsWhole()) {
        this_range.SetTo(handle.GetBioseqLength());
    }
    TSeqRange other_range = loc.GetTotalRange();
    if (other_range.IsWhole()) {
        other_range.SetTo(handle.GetBioseqLength());
    }

    if (mode == eDefault) {
        mode = GetDefaultVisibleRangePolicy();
    }

    if (mode == eVisible_Slave) {
        this_range = other_range;
    } else {
        if ( (mode & eScaleMask) == eScale_Expand) {
            if (other_range.GetLength() > this_range.GetLength()) {
                this_range = other_range;
            }
        } else if ((mode & eScaleMask) == eScale_Contract) {
            if (other_range.GetLength() < this_range.GetLength()) {
                this_range = other_range;
            }
        } else if ((mode & eScaleMask) == eScale_TrackScale) {
            TSeqPos other_diff = other_range.GetTo() - other_range.GetFrom();
            TSeqPos this_diff = this_range.GetTo() - this_range.GetFrom();
            TSignedSeqPos diff = (other_diff - this_diff) / 2;
            this_range.SetFrom(this_range.GetFrom() + diff);
            this_range.SetTo  (this_range.GetTo() + diff);
        }

        if ((mode & ePositionMask) == ePosition_Track  ||
            (mode & ePositionMask) == ePosition_TrackCenter) {
            if (this_range.GetLength() == other_range.GetLength()) {
                this_range = other_range;
            } else {
                TSeqRange r(this_range);
                r += other_range;
                if (this_range.GetLength() > other_range.GetLength()  && this_range != r) {
                    if (other_range.GetFrom() < this_range.GetFrom()) {
                        size_t diff = this_range.GetFrom() - other_range.GetFrom();
                        this_range.SetFrom((unsigned)(this_range.GetFrom() - diff));
                        this_range.SetTo  ((unsigned)(this_range.GetTo() - diff));
                    } else if (other_range.GetTo() > this_range.GetTo()) {
                        size_t diff = other_range.GetTo() - this_range.GetTo();
                        this_range.SetFrom((unsigned)(this_range.GetFrom() + diff));
                        this_range.SetTo  ((unsigned)(this_range.GetTo() + diff));
                    }
                } else if (this_range.GetLength() < other_range.GetLength()  &&  other_range != r) {
                    if (this_range.GetFrom() < other_range.GetFrom()) {
                        size_t diff = other_range.GetFrom() - this_range.GetFrom();
                        this_range.SetFrom((unsigned)(this_range.GetFrom() + diff));
                        this_range.SetTo  ((unsigned)(this_range.GetTo() + diff));
                    } else if (this_range.GetTo() > other_range.GetTo()) {
                        size_t diff = this_range.GetTo() - other_range.GetTo();
                        this_range.SetFrom((unsigned)(this_range.GetFrom() - diff));
                        this_range.SetTo  ((unsigned)(this_range.GetTo() - diff));
                    }
                }
            }
            if ((mode & ePositionMask) == ePosition_TrackCenter) {
                TSeqPos this_center =
                    (this_range.GetTo() + this_range.GetFrom()) / 2;
                TSeqPos other_center =
                    (other_range.GetTo() + other_range.GetFrom()) / 2;
                TSignedSeqPos diff     = other_center - this_center;
                TSignedSeqPos new_from = this_range.GetFrom() + diff;
                TSeqPos       new_to   = this_range.GetTo() + diff;
                if (new_from < 0) {
                    new_to += -new_from;
                    new_from = 0;
                } else if (new_to > handle.GetBioseqLength()) {
                    new_from -= new_to - handle.GetBioseqLength();
                    new_to    = handle.GetBioseqLength();
                }

                this_range.SetFrom(new_from);
                this_range.SetTo(new_to);
            }
        }
    }

    bool ret_val = (this_range != range);
    range = this_range;
    return ret_val;
}

/// Alter the supplied location according to the rules provided in the
/// policy and the supplied new location
bool CVisibleRange::Clamp(objects::CSeq_loc& loc) const
{
    try {
        CRef<CScope> scope = m_Scope;

        CBioseq_Handle bsh = scope->GetBioseqHandle(loc);
        TSeqRange total_range = loc.GetTotalRange();
        if (total_range.IsWhole()) {
            total_range.SetTo(bsh.GetBioseqLength());
        }
        CSeq_id_Handle idh = sequence::GetIdHandle(loc, scope);

        bool clamped = false;
        ITERATE (TLocations, iter, m_Locs) {
            const CSeq_id& id = sequence::GetId(**iter, scope);
            if (CSeqUtils::Match(id, *idh.GetSeqId(), scope)) {
                CBioseq_Handle bsh2 = scope->GetBioseqHandle(id);
                if (x_Clamp(**iter, bsh2, total_range)) {
                    clamped = true;
                    break;
                }
            }

            if (clamped) {
                break;
            }
        }

        if (clamped) {
            /// now, all we need to do is clamp the current
            /// seq-loc to this specific range
            return x_Clamp(loc, total_range);
        }
    }
    catch (CException& e) {
        LOG_POST(Error << "CVisibleRange::Clamp(): error: " << e.GetMsg());
    }
    return false;
}


bool CVisibleRange::x_Clamp(CSeq_loc& loc, const TSeqRange& range) const
{
    switch (loc.Which()) {
    case CSeq_loc::e_Whole:
        {{
            CRef<CSeq_id> id(&loc.SetWhole());
            loc.SetInt().SetFrom(range.GetFrom());
            loc.SetInt().SetTo  (range.GetTo());
            loc.SetId(*id);
        }}
        return true;

    case CSeq_loc::e_Int:
        loc.SetInt().SetFrom(range.GetFrom());
        loc.SetInt().SetTo  (range.GetTo());
        return true;

    case CSeq_loc::e_Packed_int:
        {{
            CSeq_loc::TPacked_int::Tdata::iterator iter = loc.SetPacked_int().Set().begin();
            CSeq_loc::TPacked_int::Tdata::iterator end  = loc.SetPacked_int().Set().end();
            for ( ;  iter != end;  ++iter) {
                CSeq_interval& ival = **iter;
                if (ival.GetTo() < range.GetFrom()  ||
                    ival.GetFrom() > range.GetTo()) {
                    /// spike the whole interval
                    loc.SetPacked_int().Set().erase(iter++);
                    /// ival is now dead!
                } else {
                    ival.SetFrom(max(ival.GetFrom(), range.GetFrom()));
                    ival.SetTo  (min(ival.GetTo(),   range.GetTo()));
                }
            }
        }}
        return true;

    case CSeq_loc::e_Mix:
        {{
            CSeq_loc::TMix::Tdata::iterator iter = loc.SetMix().Set().begin();
            CSeq_loc::TMix::Tdata::iterator end  = loc.SetMix().Set().end();
            for ( ;  iter != end;  ++iter) {
                CSeq_loc& this_loc = **iter;
                TSeqRange this_range = this_loc.GetTotalRange();
                if (this_range.GetTo() < range.GetFrom()  ||
                    this_range.GetFrom() > range.GetTo()) {
                    /// spike the whole sublocation
                    loc.SetMix().Set().erase(iter++);
                    /// this_loc is now dead!
                } else {
                    x_Clamp(this_loc, range);
                }
            }
        }}
        return true;

    default:
        LOG_POST(Error << "CVisibleRange::x_Clamp(): unhandled loc type");
        return false;
    }
}

END_NCBI_SCOPE
