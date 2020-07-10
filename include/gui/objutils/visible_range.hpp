#ifndef GUI_UTILS___VISIBLE_RANGE__HPP
#define GUI_UTILS___VISIBLE_RANGE__HPP

/*  $Id: visible_range.hpp 40878 2018-04-25 19:54:29Z katargir $
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
 * Authors: Yury Voronov, Vladimir Tereshkov, Andrey Yazhuk, Mike Dicuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CVisibleRange
{
public:
    enum EBasicPolicy {
        eBasic_Track,
        eBasic_TrackCenter,
        eBasic_TrackExpand,
        eBasic_TrackContract,
        eBasic_Slave,
        eBasic_Ignore
    };

    typedef list< CConstRef<objects::CSeq_loc> > TLocations;

    CVisibleRange( objects::CScope& scope );
    ~CVisibleRange();

    void SetVisibleRangePolicy(EBasicPolicy policy) { m_Policy = policy; }

    void AddLocation( const objects::CSeq_loc& loc );
    const TLocations& GetLocations() const;

    /// See if we can match the supplied range
    bool Match( const objects::CSeq_id& id ) const;

    /// Alter the supplied range according to the rules provided in the
    /// policy and the supplied new location
    bool Clamp( const objects::CBioseq_Handle& handle, TSeqRange& range ) const;

    /// Alter the supplied location according to the rules provided in the
    /// policy and the supplied new location
    bool Clamp( objects::CSeq_loc& loc ) const;

public:
    static bool IsAutoBroadcast() { return sm_AutoBroadcast; }
    static void SetAutoBroadcast( bool flag ){ sm_AutoBroadcast = flag; }

    static EBasicPolicy GetDefaultVisibleRangePolicy() { return sm_DefaultPolicy; }
    static void SetDefaultVisibleRangePolicy(EBasicPolicy policy) { sm_DefaultPolicy = policy; }

private:
    /// enum controlling policy for response to a change in visible range
    enum EPolicy {
        eDefault = 0,

        /// position tracking options
        ePositionMask         = 0xff,
        ePosition_Ignore      = 1,
        ePosition_Track       = 2,
        ePosition_TrackCenter = 3,
        ePosition_Slave       = 4,

        /// location scale tracking options
        eScaleMask            = 0xff << 8,
        eScale_Ignore         = 1 << 8,
        eScale_Expand         = 2 << 8,
        eScale_Contract       = 3 << 8,
        eScale_TrackRatio     = 4 << 8,
        eScale_TrackScale     = 5 << 8,
        eScale_Slave          = 6 << 8,

        /// combination options
        eVisible_Ignore = ePosition_Ignore | eScale_Ignore,
        eVisible_Slave  = ePosition_Slave | eScale_Slave,
        eVisible_Track  = ePosition_Track,
        eVisible_TrackCenter  = ePosition_TrackCenter,
        eVisible_TrackExpand  = ePosition_Track | eScale_Expand,
        eVisible_TrackContract  = ePosition_Track | eScale_Contract
    };

    /// internal clamping function to clamp a total range based on a location
    bool x_Clamp(const objects::CSeq_loc& loc,
                 const objects::CBioseq_Handle& handle,
                 TSeqRange& range) const;

    bool x_Clamp(objects::CSeq_loc& loc,
                 const TSeqRange& range) const;

    int x_GetPolicy() const;

    TLocations m_Locs;
    CRef<objects::CScope> m_Scope;
    EBasicPolicy m_Policy;

private:
    static bool sm_AutoBroadcast;
    static EBasicPolicy sm_DefaultPolicy;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___VISIBLE_RANGE__HPP

