#ifndef __GUI_WIDGETS_ALNMULTI___ALIGN_ROW__HPP
#define __GUI_WIDGETS_ALNMULTI___ALIGN_ROW__HPP

/*  $Id: align_row.hpp 44984 2020-05-01 16:46:19Z shkeda $
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
 * Authors:  Andrey Yazhuk, Anatoliy Kuznetsov, Andrei Shkeda, Victor Joukov
 *
 * File Description:
 *
 */

#include <objtools/alnmgr/aln_explorer.hpp>

#include <gui/utils/attr_range_coll.hpp>
#include <gui/widgets/aln_score/aln_scoring.hpp>

#include <gui/widgets/aln_multiple/ialign_row.hpp>
#include <gui/widgets/aln_multiple/alnvec_row_handle.hpp>
#include <gui/widgets/aln_multiple/align_row_graph_cont.hpp>

#include <gui/widgets/seq_graphic/feature_panel.hpp>
#include <gui/widgets/seq_graphic/layout_track.hpp>
#include <gui/widgets/seq_graphic/layout_track_handler.hpp>

BEGIN_NCBI_SCOPE


class CScoreCache;
class IAlnVecRowGraph;
class IAlnRowGraphProperties;

////////////////////////////////////////////////////////////////////////////////
/// CAlnRowState - implements state management for Align Row
/// 

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnRowState : public IAlignRow
{
public:
    CAlnRowState();

    virtual int GetRowState() const;
    virtual void SetRowState(int mask, bool b_set);

private:
    int m_RowState;
};

////////////////////////////////////////////////////////////////////////////////
/// class CAlnVecRow - implementation of IAlignRow for CAlnVec-based alignments.
/// CAlnVecRow uses CAlnVecRowHandle as proxy for accesing data in CAlnVec.

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnVecRow :
    public CObjectEx,
    public CEventHandler,
    public ILayoutTrackHost,
    public ITrackHandlerHost,       ///< for layout track interaction
    public IGenericHandlerHost,
    public CAlnRowState
{
public:
    typedef IAlnExplorer::TSignedRange    TSignedRange;

public:
    CAlnVecRow(const IAlignRowHandle& handle, bool skip_label = false);
    ~CAlnVecRow();

    void SetConsensus(CConsensusRow* crow) { m_ConsensusRow = crow; }

    /// @name IAlignRow implementation
    /// @{
    virtual void    SetHost(IAlignRowHost* host);

    virtual void    SetDisplayStyle(const CRowDisplayStyle* style);
    virtual const CRowDisplayStyle*    GetDisplayStyle();

    virtual int     GetRowNum() const;
    virtual int     GetHeightPixels() const;

    virtual void    RenderColumn(TColumnType  col_type);
    virtual void    GetColumnText(TColumnType col_type, string& text, bool for_printer) const;
    virtual void    OnMouseEvent(wxMouseEvent& event, TColumnType  col_type, CGlPane& pane);

    /// returns tooltip text by point in the viewport coordinates
    virtual void    GetTooltip(const TVPPoint& vp_pt, TColumnType col_type, CGlPane& pane, ITooltipFormatter& tooltip);

    virtual void    GetHTMLActiveAreas(TColumnType col_type, CGlPane& pane, TAreaVector& areas);

    virtual void    UpdateOnStyleChanged();
    virtual void    UpdateOnAnchorChanged();
    virtual void    Update(const CGlPane& pane, bool layout_only = false);

    virtual void    PrepareRendering(CGlPane& pane, TVPUnit row_top, int state);
    /// @}

    virtual void    SetScoreCache(CScoreCache* cache);

    virtual bool    IsExpandable() const;
    virtual bool    IsExpanded() const;
    virtual void    Expand(bool b_exp, CGlPane& pane);
    virtual void    UseExternalOrganismLookup(bool f);
    
    // MSA methods
    // ad-hoc method for correct synchronous layout of graphs
    virtual string  GetAlignmentTooltip(TSeqPos pos, TSeqPos half_width);
    //

    virtual const IAlnRowGraphProperties*     GetProperties() const;
    virtual void    SetProperties(IAlnRowGraphProperties* props);

    virtual TSeqPos GetColumnAsSeqPos(TColumnType type, const TModelRect& rc_vis);
    virtual string  GetColumnAsString(TColumnType type);

    virtual string& GetAlnSeqString(string &buffer, const IAlnExplorer::TSignedRange &aln_rng) const;
    virtual string& GetStringAtPos(string& buffer, TSignedSeqPos aln_pos) const;

    virtual void GetRowStatistics(ITooltipFormatter& tooltip) const;

    virtual bool UsesAATranslation() const;
    virtual bool IsNucProtAlignment() const;

    const IAlignRowHandle& GetRowHandle() const;
    
    // ILayoutTrackHost implementation
    virtual void LTH_OnLayoutChanged();
    virtual void LTH_ZoomOnRange(const TSeqRange &range) {}
    virtual void LTH_PopupMenu(wxMenu *menu);
    virtual void LTH_PushEventHandler(wxEvtHandler *handler);
    virtual void LTH_PopEventHandler();
    virtual void LTH_ConfigureTracksDlg(const string& category);

    /// @name class ITrackHandlerHost implementation
    /// @{
    virtual CRef<CLayoutTrack> THH_GetLayoutTrack();
    virtual TModelPoint THH_GetModelByWindow(const wxPoint& pt);
    /// @}
    
    /// @name class IGenericHandlerHost implementation
    virtual void GHH_Redraw();
    virtual void GHH_SetCursor(const wxCursor &cursor);
    virtual void GHH_CaptureMouse();
    virtual void GHH_ReleaseMouse();
    /// @}


    void SaveTrackConfig();

    void LoadDefaultTracks();
    void ConfigureTracks(bool reset = false);
    void ConfigureTracksDialog(const string& category = NcbiEmptyString);

    size_t GetTrackCount(CGlPane& pane);

protected:
    DECLARE_EVENT_MAP();

    enum EButtons   {
        eInvalid = -1,
        eStrandMarker,
        eExpand,
        eSetupGraphs
    };
    typedef CScoreCache::TScoreColl::const_iterator TScoreConstIterator;

    virtual int x_GetBaseHeight() const;
    string& x_GetTaxLabel() const;

    virtual void    x_RenderDescr();
    virtual void    x_RenderTaxLabel();
    virtual void    x_RenderAlign();
    virtual void    x_RenderIcons();
    virtual void    x_RenderIconStrand();
    virtual void    x_RenderIconExpand();
    virtual void    x_RenderEmptyBox();

    virtual void    x_RenderInserts(CGlPane& pane, int from, int to,
                                    TModelUnit top_y, TModelUnit bottom_y);
    virtual void    x_RenderUnaligned(CGlPane& pane, int from, int to,
                                    TModelUnit top_y, TModelUnit bottom_y);
    virtual void    x_RenderAlignSegments(CGlPane& pane, IAlnSegmentIterator& it_seg,
                                          TModelUnit top_y, TModelUnit bottom_y,
                                          const CRgbaColor& color);
    virtual void    x_RenderAlignScores(IAlnSegmentIterator& it_seg,
                                        int top_y, int bottom_y, bool averageable);
    virtual void    x_RenderAveragableScores(CGlPane& pane,
                                                IAlnSegmentIterator& it_seg,
                                                TScoreConstIterator it,
                                                const TScoreConstIterator it_end,
                                                int vp_top_y, int vp_bottom_y);

    virtual void    x_RenderNonAveragableScores(CGlPane& pane,
                                                IAlnSegmentIterator& it_seg,
                                                TScoreConstIterator it,
                                                const TScoreConstIterator it_end,
                                                int vp_top_y, int vp_bottom_y);
    virtual bool    x_CanRenderSeq(CGlPane& pane);
    virtual void    x_RenderSequence(CGlPane& pane, TModelUnit top_y, TModelUnit bottom_y,
                                     int start, int stop);
    virtual void    x_RenderGraph(const TSignedRange& range);
    virtual void    x_RenderStartPos();
    virtual void    x_RenderEndPos();
    virtual void    x_RenderSeqStart();
    virtual void    x_RenderSeqEnd();
    virtual void    x_RenderSeqLength();
    virtual void    x_RenderAlnPos(TSeqPos aln_pos, IAlnExplorer::ESearchDirection dir);
    virtual void    x_RenderTextCell(const string& text, IGlFont::TAlign align);
    virtual TVPRect x_GetButtonRect(EButtons btn, int shift=1);
    virtual EButtons    x_HitTestButton(CGlPane& pane, const TVPPoint& pt);
    virtual void    x_OnClickExpandBtn(CGlPane& pane);

    virtual void x_GetDescrTooltip    (const TVPPoint& pt, CGlPane& pane, ITooltipFormatter& tooltip);
    virtual void x_GetIconsTooltip    (const TVPPoint& pt, CGlPane& pane, ITooltipFormatter& tooltip);
    virtual void x_GetAlignmentTooltip(const TVPPoint& pt, CGlPane& pane, ITooltipFormatter& tooltip);

    void x_GetAlnPosLabel(TSeqPos aln_pos, IAlnExplorer::ESearchDirection dir, string& label) const;

    void x_GetAlignmentTooltip_Helper(TSeqPos pos, TSignedRange& range, ITooltipFormatter& tooltip);
    void x_GetAlignmentTooltip_General(TSeqPos pos, ITooltipFormatter& tooltip);
    void x_GetTaxonomyInformation(const objects::CBioseq_Handle &bsh, ITooltipFormatter& tooltip) const;
    void x_GetTaxonomyInfoFromService(std::string &taxonomy) const;
    void x_GetTaxonomyInfoFromBiosrc(const objects::CBioSource &biosrc, std::string &taxonomy) const;
    //void x_GetAlignmentTooltip_General(const TSeqPos pos, ITooltipFormatter& tooltip);
    void x_GetAlignmentTooltip_Insert(const TSignedSeqRange& aln_r,
                                      const TSignedSeqRange& seq_r,
                                      ITooltipFormatter& tooltip);
    void x_GetAlignmentTooltip_Gap(const TSignedSeqRange& aln_r,
                                   const TSignedSeqRange& seq_r,
                                   ITooltipFormatter& tooltip);
    void x_GetAlignmentTooltip_Unaligned(TSignedSeqPos aln_prev_to,
                                         TSignedSeqPos aln_from,
                                         TSignedSeqPos prev_to,
                                         TSignedSeqPos from,
                                         ITooltipFormatter& tooltip);
    string x_GetTooltipSequence(TSeqPos from, TSeqPos to);

    IAlignRowHost*  x_GetHost()     {   return m_Host; }

    void    x_OnGraphContentChanged(CEvent* evt);
    void    x_OnGraphSizeChanged(CEvent* evt);

    virtual void    x_UpdateRowHeight();

    virtual void  x_CreateTrackPanel(CGlPane& pane);
    virtual void x_ResetTrackPanel();
    bool x_InitTrackPane(const CGlPane& row_pane, CGlPane& track_pane);
    void x_RenderTracks(CGlPane& pane, const TSignedRange& range);
    CRef<CSeqGlyph> x_HitTest(const TVPPoint& vp_pt, CGlPane& pane, TModelPoint& hit_point);
    void x_ForEachAlnSeg(const IAlnExplorer::TSignedRange& range,
                         const CGlPane& src_pane,
                         CGlPane& target_pane,
                         const function <bool(CGlPane& pane)>& f);
protected:
    const IAlignRowHandle&  m_Handle;

    CConsensusRow* m_ConsensusRow;

    IAlignRowHost*  m_Host;

    int     m_Height;
    int     m_BaseHeight;
    string  m_Text;
    mutable int     m_TaxId;
    mutable bool    m_fTaxLabelInit;
    mutable string  m_TaxLabel;

    bool    m_bExpanded;
    bool    m_fExtOrgLookup;

    const CRowDisplayStyle*   m_Style; /// row display style

    CScoreCache*    m_Cache;

    unique_ptr<CRenderingContext> m_RenderingCtx;
    unique_ptr<CRenderingContext> m_SequenceRenderingCtx;
    unique_ptr<CGlPane> m_TrackPane;
    CRef<CFeaturePanel> m_TrackPanel;
    bool m_NeedUpdateContent = false;
    bool m_NeedUpdateLayout = false;
    bool m_NeedUpdateSelection = false;
    AutoPtr<CLayoutTrackHandler> m_TrackHandler;
    //CWeakRef<CSeqGlyph> m_LastHitGlyph;
    CWeakRef<CLayoutTrack> m_LastHitTrack;

    /// Graphics caching
    TVPUnit         m_PixLeft, m_PixRight;
    TModelUnit      m_ModelLeft, m_ModelRight;
};


inline
const IAlignRowHandle& CAlnVecRow::GetRowHandle() const
{
    return m_Handle;
}


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALIGN_ROW__HPP
