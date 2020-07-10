#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_GLYPH__HPP

/*  $Id: alignment_glyph.hpp 43986 2019-10-03 20:09:35Z evgeniev $
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
 *   CAlignGlyph -- utility class to layout alignments and hold CAlnVec objects
 */


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>
#include <gui/widgets/seq_graphic/alignment_conf.hpp>
#include <gui/widgets/seq_graphic/alignment_score.hpp>
#include <gui/utils/rgba_color.hpp>

#include <gui/widgets/seq_graphic/alngraphic_ds.hpp>
#include <gui/objutils/label.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objtools/alnmgr/alnvec.hpp>
#include <objmgr/seq_align_handle.hpp>
#include <math.h>

/** @addtogroup GUI_OBJUTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE

enum ETearLineLimits
{
    // In adaptive display mode we limit the total number of rows displayed
    // to this number. This does not change the number of glyphs created, but
    // instead hides them if the layout algorithm (BuildLayout) puts them
    // in a row number higher than this
    eAdaptiveRowLimit = 256,
    eMaxRowLimit = 500
};


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlignGlyph :
    public CSeqGlyph,
    public IObjectBasedGlyph
{
public:
    typedef vector< CRef<CAlignGlyph> > TAlnList;
    typedef pair<CConstRef<CObject>, int> TAlnAnchorPair;


    enum EAlignSpacing {
        eOne2One,       // DNA to DNA or protein to protein
        eOne2Three,     // DNA to protein
        eThree2One      // Protein to DNA
    };

    // ctors
    CAlignGlyph(CConstRef<IAlnGraphicDataSource> aln_mgr,
                 const objects::CSeq_align_Handle& orig_align,
                 const objects::CSeq_align& mapped_align,
                 int orig_anchor);

    /// @name CSeqGlyph methods
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual TSeqRange   GetRange(void) const;
    virtual bool IsClickable() const;
    virtual bool LessBySeqPos(const CSeqGlyph& obj) const;
        /// @}

    /// @name IObjectBasedGlyph interface implement
    /// @{
    virtual const objects::CSeq_loc& GetLocation(void) const;
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;
    virtual string GetSignature() const;
    virtual const TIntervals& GetIntervals(void) const;
    /// @}

    const IAlnGraphicDataSource&      GetAlignMgr(void) const;
    const objects::CSeq_align&        GetAlignment(void) const;
    const objects::CSeq_align_Handle& GetOrigAlignment(void) const;
    TAlnAnchorPair                    GetAlnAnchorPair(void) const;
    int                               GetOrigAnchor(void) const;

    TModelUnit GetBarHeight() const;

    void SetConfig(const CAlignmentConfig* conf);
    void SetScore(const ISGAlnScore& score);

    void ApplyFading() const;
    void DrawPWAlignElem(const TModelRect& rcm, bool selected,
                         bool show_label, const CRgbaColor& c_fg,
                         const CRgbaColor& c_seq, const CRgbaColor& c_mis) const;

    void GetTitle(string* title, CLabel::ELabelType type) const;
    
    bool IsNegative() const;

    void SetShowScore();
    bool IsSimplified() const;
    bool IsLastRow() const;

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

private:
    enum ETail
    {
        e5_Prime,
        e3_Prime
    };

    typedef CRange< TModelUnit >  TModelRange;
    typedef vector< TModelRange > TMUnitRanges;

    void x_DrawPWAlign() const;
    void x_DrawMultiAlign() const;
    void x_DrawMultiAlignLabel(int row, TModelUnit y) const;
    void x_DrawAlignRowSequence(TModelUnit center_y, int row,
                                const CRgbaColor& c_fg, const CRgbaColor& c_seq,
                                const CRgbaColor& c_mis) const;

    void x_RenderNonAveragableScores(TModelUnit yy, TSignedSeqRange aln_range,
                                     TSeqRange seq_range, TModelUnit height, int row,
                                     bool override_color, const CRgbaColor& read_status_color) const;

    void x_RenderAveragableScores(TModelUnit yy, TSignedSeqRange aln_range,
                                  TSeqRange seq_range, TModelUnit height, int row,
                                  bool override_color, const CRgbaColor& read_status_color) const;

    bool x_IsPWAlign() const;

    // calculates intervals for the target row
    // for drawing, hairlines and navigation
    // if row == -1, pairwise alignemnt is assumed
    // and the intervals are calculated for non-anchor row
    void x_CalcIntervals(TIntervals& intervals, int target_row = -1) const;

    bool x_ShowLabel() const;
    bool x_ShowScore() const;
    bool x_ShowUnalignedTails(TModelUnit align_len) const;
        
    void x_GetOverrideColor(bool& override_color, 
                            CRgbaColor& read_status_color) const;
    struct SDrawContext {
        TSignedSeqRange aln_range;
        TSeqRange       seq_range;
        TSeqRange       whole_range;
        bool            reverse_strand = false;
        TSignedSeqPos   tail_5 = 0;
        TSignedSeqPos   tail_3 = 0;
        TSeqPos         base_width_anchor = 0;
        TSeqPos         base_width_curr = 0;
        TSeqPos         f = 0;
        TSeqPos         t = 0;
        TModelUnit      yy = 0;
        TModelUnit      half = 0;
        TModelUnit      bar_h = 0;
        TModelUnit      center_y = 0;
        bool            override_color = false;
        CRgbaColor      read_status_color;
    };

    void x_DrawGaps(const SDrawContext& ctx) const;
    void x_DrawInserts(const SDrawContext& ctx) const;
    void x_DrawIntronsAndInternalUnalignedRegions(const SDrawContext& ctx, TSignedSeqPos from, TSignedSeqPos to) const;
    void x_DrawInternalUnalignedRegions(const SDrawContext& ctx, TMUnitRanges &unaligned_regions) const;
    void x_DrawTailGlyphs(const SDrawContext& ctx, bool show_label, const CRgbaColor& c_fg) const;
    void x_DrawTailSequences(const SDrawContext& ctx, const CRgbaColor& c_fg) const;
    void x_DrawUnalignedBar(const ETail tail, const SDrawContext& ctx, const CRgbaColor& c_fg) const;
    void x_DrawUnalignedRowSequence(const ETail tail, const SDrawContext& ctx, const CRgbaColor& c_fg) const;
    void x_DrawNonConsensusSpliceSites(const SDrawContext& ctx) const;
    void x_DrawNoScoreAlign(const SDrawContext& ctx, const CRgbaColor& c_fg) const;
    void x_DrawStrandIndicators(const SDrawContext& ctx) const;
    void x_DrawSimplified(const SDrawContext& ctx, const CRgbaColor& c_fg) const;
    TSeqRange x_GetAlnTailRange(const ETail tail, const SDrawContext& ctx) const;
    TSeqRange x_ConvertAlnTaillRngToSeqRng(const ETail tail, const SDrawContext& ctx, const TSeqRange &aln_range) const;
    TModelUnit x_GetTailLabelWidth(const TSignedSeqPos tail) const;
    TModelUnit x_GetTailWidth(const TSignedSeqPos tail, const TSignedSeqPos visible_tail, const TSignedSeqPos base_width_curr) const;
    bool x_IsSRA() const;
protected:
    /// pointer to the actual alignment information.
	CConstRef<IAlnGraphicDataSource> m_AlnMgr;
    CConstRef<objects::CSeq_align>   m_SeqAlign;
    objects::CSeq_align_Handle       m_OrigSeqAlign;

    /// The anchor row number in original Seq-algin, not alnmgr.
    int                              m_OrigAnchor;
    CRef<objects::CSeq_loc>          m_Location;

    /// intervals.
    TIntervals                       m_Intervals;

    CConstRef<CAlignmentConfig>      m_Config;
    CConstIRef<ISGAlnScore>          m_Score;

    // Flag indicating if the alignment is simplifed and/or colored.
    // There are two cases we could simplify an alingment:
    //   1. alignment size is small on screen ( < 4 pixels )
    //   2. std-seg alignment with different segment length
    //      across rows
    bool                             m_ShowScore;

    /// Indicates whether there are internal unaligned regions
    mutable bool                     m_UnalignedRegions;

    mutable float                    m_FadeFactor; 
};


///////////////////////////////////////////////////////////////////////////////
/// Inline methods

inline
const IAlnGraphicDataSource& CAlignGlyph::GetAlignMgr(void) const
{
    return *m_AlnMgr;
}

inline
const objects::CSeq_align& CAlignGlyph::GetAlignment(void) const
{
    return *m_SeqAlign;
}

inline
CAlignGlyph::TAlnAnchorPair CAlignGlyph::GetAlnAnchorPair(void) const
{
    return TAlnAnchorPair(GetObject(0), m_OrigAnchor);
}

inline
int CAlignGlyph::GetOrigAnchor(void) const
{
    return m_OrigAnchor;
}

inline
const objects::CSeq_align_Handle& CAlignGlyph::GetOrigAlignment(void) const
{
    return m_OrigSeqAlign;
}

inline
TModelUnit CAlignGlyph::GetBarHeight() const
{
    TModelUnit bar_h = m_Config->m_BarHeight;
    if (m_Config->m_CompactMode == CAlignmentConfig::eCompact) {
        bar_h = floor(bar_h * 0.4);
    } else if (m_Config->m_CompactMode == CAlignmentConfig::eExtremeCompact) {
        bar_h = floor(bar_h * 0.2);
    }
    return bar_h;
}

inline
void CAlignGlyph::SetConfig(const CAlignmentConfig* conf)
{
    m_Config.Reset(conf);
}

inline
void CAlignGlyph::SetScore(const ISGAlnScore& score)
{
    m_Score.Reset(&score);
}

inline
bool CAlignGlyph::x_IsPWAlign() const
{
    return m_AlnMgr->GetNumRows() == 2;
}

inline
bool CAlignGlyph::IsNegative() const
{
    if (x_IsPWAlign()) {
        return m_AlnMgr->IsNegativeStrand(1) !=
            m_AlnMgr->IsNegativeStrand(0);
    }
    return false;
}

inline
void CAlignGlyph::SetShowScore()
{
    m_ShowScore = true;
}

inline
bool CAlignGlyph::IsSimplified() const
{
    return m_AlnMgr->IsSimplified();
}

inline
bool CAlignGlyph::IsLastRow() const
{
    return (GetTearline() > 0 && GetRowNum() == GetTearline());
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_GLYPH__HPP
