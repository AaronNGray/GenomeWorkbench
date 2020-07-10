#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SWITCH_POINT_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SWITCH_POINT_GLYPH__HPP

/*  $Id: switch_point_glyph.hpp 19261 2009-05-11 12:37:30Z wuliangs $
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
 *   CSwitchPointGlyph -- utility class to layout switch points in the
 *                          contig assembly
 */


#include <corelib/ncbiobj.hpp>

#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/seq_map_switch.hpp>
#include <objmgr/seq_align_handle.hpp>

/** @addtogroup GUI_OBJUTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSwitchPointGlyph : public CSeqGlyph
{
public:
    /// @name ctors
    /// @{
    CSwitchPointGlyph(CRef<objects::CSeqMapSwitchPoint> map_points);
    /// @}

    /// Access the alignment as a CObject.
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;

    /// Access the alignments's remapped location.
    const objects::CSeq_loc&          GetLocation(void) const;

    //TSeqRange                         GetRange(void) const;

    const objects::CSeq_align&        GetAlignment(void) const;

    CRef<objects::CSeqMapSwitchPoint> GetSwitchPoints() const;

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

    ///// calculate our intervals.
    //virtual void x_CalcIntervals(void);

protected:
    /// pointer to the actual alignment information.
    CRef<objects::CSeqMapSwitchPoint> m_MapPoint;
    CRef<objects::CSeq_loc>     m_Location;

    /// intervals (like for features or alignments).
    vector<TSeqRange>  m_Intervals;
};



END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SWITCH_POINT_GLYPH__HPP
