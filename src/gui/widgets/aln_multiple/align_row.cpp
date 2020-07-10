/*  $Id: align_row.cpp 45038 2020-05-13 01:22:22Z evgeniev $
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
 * Authors:  Andrey Yazhuk, Anatoliy Kuznetsov, Victor Joukov, Andrei Shkeda
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>

#include <math.h>
#include <gui/types.hpp>
#include <gui/objutils/label.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/rasterizer.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/label.hpp>

#include <objmgr/graph_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seq/Seq_descr.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>

#include <gui/widgets/aln_multiple/align_row.hpp>
#include <gui/widgets/aln_multiple/alnmulti_renderer.hpp>
#include <gui/widgets/aln_multiple/alnvec_row_graph.hpp>
#include <gui/objutils/taxon_cache.hpp>

#include <gui/widgets/seq_graphic/config_utils.hpp>

#include <objmgr/util/sequence.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/widgets/gl/gl_widget_base.hpp>
#include <wx/richmsgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

// Copy of scope singleton from sviewer_data.cpp
typedef CRef<CScope> TScopeRef;

static CStaticTls<TScopeRef> s_ScopeTls;

// need to keep m_RenderingCtx.pane pointing to m_TrackPane
// as it's used in in track handlers
struct SRenderingContextGuard
{
    SRenderingContextGuard(const function<void()>& f) : m_f(f) {}
    ~SRenderingContextGuard() { m_f(); }
    function<void()> m_f;
};


template <class T>
void TlsValCleanup(T* oldVal, void* /*data*/)
{
    delete oldVal;
}



// Get the scope without any seq-entries loaded into it.
// Seq-entries can interfere with id resulution and mask information
// otherwise available for well-known ids, e.g. tax id.
// See MSA-75 for examples.
objects::CScope& s_GetPristineScope(const objects::CBioseq_Handle& bio_handle)
{
    TScopeRef* scope = s_ScopeTls.GetValue();
    if (!scope ) {
        TScopeRef*  s(new TScopeRef(
            new CScope(bio_handle.GetScope().GetObjectManager())
        ));
        (*s)->AddDefaults();
        s_ScopeTls.SetValue(s, TlsValCleanup<TScopeRef>, 0);
        scope = s_ScopeTls.GetValue();
    }
    return **scope;
}


////////////////////////////////////////////////////////////////////////////////
/// CAlnRowState

CAlnRowState::CAlnRowState()
: m_RowState(0)
{}

int CAlnRowState::GetRowState() const
{
    return m_RowState;
}

void CAlnRowState::SetRowState(int mask, bool b_set)
{
    if(b_set)   {
        m_RowState |= mask;
    } else {
        m_RowState &= ~mask;
    }
}


////////////////////////////////////////////////////////////////////////////////
/// CAlnVecRow
CAlnVecRow::CAlnVecRow(const IAlignRowHandle& handle, bool skip_label)
:   m_Handle(handle),
    m_ConsensusRow(NULL),
    m_Host(NULL),
    m_Height(0),
    m_BaseHeight(0),
    m_TaxId(0),
    m_fTaxLabelInit(false),
    m_bExpanded(false),
    m_fExtOrgLookup(true),
    m_Style(NULL),
    m_Cache(NULL),
    m_PixLeft(-1), m_PixRight(-1), m_ModelLeft(-1), m_ModelRight(-1)
{
    m_Text = m_Handle.GetSeq_id().GetSeqIdString();
    if (skip_label == false) {
        if (!CSeqUtils::IsVDBAccession(m_Text)) {
            m_Text.clear();
            CLabel::GetLabel(m_Handle.GetSeq_id(), &m_Text, CLabel::eDefault, &m_Handle.GetScope());
        }
    }
}


BEGIN_EVENT_MAP(CAlnVecRow, CEventHandler)
    ON_EVENT(CGraphNotification, CGraphNotification::eContentChanged,
               &CAlnVecRow::x_OnGraphContentChanged)
    ON_EVENT(CGraphNotification, CGraphNotification::eSizeChanged,
               &CAlnVecRow::x_OnGraphSizeChanged)
END_EVENT_MAP()


CAlnVecRow::~CAlnVecRow()
{
    m_Style = NULL; //do not delete
}


void CAlnVecRow::SetScoreCache(CScoreCache* cache)
{
    m_Cache = cache;
}

static void s_PrepareTrackPane(TModelUnit aln_from, TModelUnit aln_to, TModelUnit seq_from, TModelUnit seq_to, 
                               const IAlignRowHandle& row_handle,
                               const CGlPane& source_pane, 
                               CGlPane& target_pane)
{
    TModelUnit start = seq_from;
    TModelUnit stop = seq_to;
    if (start > stop) {
        swap(start, stop);
    }
    if (start != stop) { // if not empty
        if (aln_from > aln_to)
            swap(aln_from, aln_to);
        aln_from = floor(aln_from);
        aln_to = floor(aln_to );

        const TModelRect& source_rc_vis = source_pane.GetVisibleRect();
        int rc_right = floor(source_rc_vis.Right());
        int rc_left = floor(source_rc_vis.Left());
        if (row_handle.IsNegativeStrand()) {
            if (aln_to == rc_right) {
                TModelUnit offset = source_rc_vis.Right() - rc_right;
                start += (1. - offset);
            }

            if (aln_from == rc_left) {
                TModelUnit offset = source_rc_vis.Left() - rc_left;
                stop += (1. - offset);
            } else {
                stop += 1.;
            }
        } else {
            if (aln_from == rc_left) {
                TModelUnit offset = source_rc_vis.Left() - rc_left;
                start += offset;
            }
            if (aln_to == rc_right) {
                TModelUnit offset = source_rc_vis.Right() - rc_right;
                stop += offset;
            } else {
               stop += 1.;
            }
        }
    } else {
        stop += 1;
    }

    if (row_handle.IsNegativeStrand())
       swap(start, stop);
    TModelRect rc_vis = target_pane.GetVisibleRect();
    rc_vis.SetHorz(start, stop);
    target_pane.SetModelLimitsRect(rc_vis);
    target_pane.SetVisibleRect(rc_vis);
}



void CAlnVecRow::GetHTMLActiveAreas(TColumnType col_type, CGlPane& pane, TAreaVector& areas)
{
    string s_row = NStr::IntToString(m_Handle.GetRowNum());
    m_RenderCtx.m_Viewport.SetHorz(m_RenderCtx.m_Pane->GetViewport().Left(), m_RenderCtx.m_Pane->GetViewport().Right());
    const auto& rc = m_RenderCtx.m_Viewport;
    switch(col_type)    {
    case eDescr: 
        areas.emplace_back(CAlnMultiRenderer::eHtmlArea_Link, rc, s_row, m_Text, "row description");
        break;
    case eIcons:    {       // generate click-able button for expanding/collapsing row
        areas.emplace_back(CAlnMultiRenderer::eHtmlArea_Link,
                           x_GetButtonRect(eStrandMarker),
                           s_row, m_Handle.IsNegativeStrand() ? "-" : "+",
                           "Strand");
        if (!IsExpandable())
            break;
        areas.emplace_back(CAlnMultiRenderer::eHtmlArea_CheckBox,
                           x_GetButtonRect(eExpand),
                           s_row, "Click to expand/collapse", "toggle_expand");
        break;
    }
    case eSeqStart: {
        TSeqPos pos = m_Handle.IsNegativeStrand() ?
                      m_Handle.GetSeqStop() : m_Handle.GetSeqStart();
        string s_num = NStr::IntToString(pos);
        areas.emplace_back(CAlnMultiRenderer::eHtmlArea_Link, rc, s_row, s_num, "Seq Start");
        break;
    }
    case eSeqEnd: {
        TSeqPos pos = m_Handle.IsNegativeStrand() ?
                      m_Handle.GetSeqStart() : m_Handle.GetSeqStop();
        string s_num = NStr::IntToString(pos);
        areas.emplace_back(CAlnMultiRenderer::eHtmlArea_Link, rc, s_row, s_num, "Seq End");
        break;
    }
    case eTaxLabel: 
        areas.emplace_back(CAlnMultiRenderer::eHtmlArea_Link, rc, s_row, x_GetTaxLabel(), "Organism");
        break;
    case eAlignment:    {   // create area for alignment
        /*
        if (IsExpanded()) {
            // setup CGlPane for the graph
            CGlPane track_pane(CGlPane::eAlwaysUpdate);
            x_InitTrackPane(pane, track_pane);
            
            
            TModelRect rc_vis = track_pane.GetVisibleRect();
            // prepare context for data update
            TModelUnit seq_from = m_Handle.GetSeqPosFromAlnPos(rc_vis.Left(), IAlnExplorer::eRight);
            TModelUnit seq_to = m_Handle.GetSeqPosFromAlnPos(rc_vis.Right(), IAlnExplorer::eLeft);
            
            s_PrepareTrackPane(rc_vis.Left(), rc_vis.Right(), seq_from, seq_to, m_Handle.IsNegativeStrand(), pane, track_pane);
            
            TModelRect rc_model = track_pane.GetModelLimitsRect();
            rc_model.SetHorz(m_Handle.GetSeqStart(), m_Handle.GetSeqStop());    
            track_pane.SetModelLimitsRect(rc_model);

            //    track_pane.SetViewport(track_pane.GetViewport());
            m_RenderingCtx.PrepareContext(track_pane, true, m_Handle.IsNegativeStrand());
            TAreaVector track_areas;
            m_TrackPanel->GetHTMLActiveAreas(&track_areas);
            for (auto&& area : track_areas) {
                area.m_ID = s_row;
                areas.push_back(area);
            }
        }
        */
            
        /* It's not used by any clients, don't generate it
        const TModelRect& rc_vis = pane.GetVisibleRect();
        TSignedSeqPos vis_start = (TSignedSeqPos) floor(rc_vis.Left());
        TSignedSeqPos vis_stop = (TSignedSeqPos) ceil(rc_vis.Right());

        TSignedSeqPos start = max(m_Handle.GetSeqAlnStart(), vis_start);
        TSignedSeqPos stop = min(m_Handle.GetSeqAlnStop(), vis_stop);

        pane.OpenOrtho();
        pane.Close();

        TVPUnit vp_start = pane.ProjectX(start);
        TVPUnit vp_stop = pane.ProjectX(stop);
        TVPRect rc = pane.GetViewport();
        rc.SetHorz(vp_start, vp_stop);

        CHTMLActiveArea area(CAlnMultiRenderer::eHtmlArea_Link, rc,
                             s_row, m_Text, "whole alignment");
        areas.push_back(area);

        // generate HTML areas representing segments
        TSignedRange   range(start, stop);
        IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eSkipInserts;
        auto_ptr<IAlnSegmentIterator> p_it(m_Handle.CreateSegmentIterator(range, flags));

        for ( IAlnSegmentIterator& it(*p_it);  it;  ++it )  {
            const IAlnSegment& seg = *it;
            const TSignedSeqRange& R = seg.GetAlnRange();
            start = max(R.GetFrom(), vis_start);
            stop = min(R.GetToOpen(), vis_stop);
            vp_start = pane.ProjectX(start);
            vp_stop = pane.ProjectX(stop);
            rc.SetHorz(vp_start, vp_stop);

            CHTMLActiveArea sg_area(CAlnMultiRenderer::eHtmlArea_Link, rc, s_row,
                                 m_Text, "alignment section");
            areas.push_back(sg_area);
        }
        */
        }
        break;
    default:
        break;
    };
}


/// height of "white-space" between row borders and alignment band
static const int kAlignSpace = 1;

/// vertical offset from the border of the alignment band to the border of gap band
static const int kGapOffset = 4;

static const int kTextVertSpace = 2;
static const int kSeqVertSpace = kAlignSpace + 2;


void CAlnVecRow::SetDisplayStyle(const CRowDisplayStyle* style)
{
    m_Style = style;
    UpdateOnStyleChanged();
}


void CAlnVecRow::UpdateOnStyleChanged()
{
    m_Height = m_BaseHeight = x_GetBaseHeight();
    if (m_bExpanded && m_TrackPanel) {
        m_Height += m_TrackPanel->GetHeight();
    }
}

void CAlnVecRow::UpdateOnAnchorChanged()
{
    m_NeedUpdateLayout = true;
}


void CAlnVecRow::Update(const CGlPane& pane, bool layout_only)
{
    const TModelRect& rc_vis = pane.GetVisibleRect();
    bool updated = (m_ModelLeft != rc_vis.Left() || m_ModelRight != rc_vis.Right());
    m_ModelLeft = rc_vis.Left();
    m_ModelRight = rc_vis.Right();
    const auto& vp = pane.GetViewport();
    m_PixLeft = vp.Left();
    m_PixRight = vp.Right();

    m_NeedUpdateLayout = true;
    m_NeedUpdateSelection = true;
    if (m_bExpanded && m_TrackPanel) {
        m_NeedUpdateContent = updated;
    }
}

int CAlnVecRow::x_GetBaseHeight() const
{
    int h = 0;
    if (m_Style && (m_Style->GetShownElements() & CRowDisplayStyle::fShown_Alignment)) {
        TModelUnit h1 = kTextVertSpace * 2;
        TModelUnit h2 = kSeqVertSpace * 2;
        h1 += m_Style->GetTextFont().TextHeight();
        h2 += m_Style->GetSeqFont().TextHeight();
        h = (int) max(h1, h2);
    }
    return h;
}


string& CAlnVecRow::x_GetTaxLabel() const
{
    if (!m_fTaxLabelInit) {
        m_fTaxLabelInit = true;
        // skip it for SRA accessions as they don't have tax_id defined
        if (!CSeqUtils::IsVDBAccession(m_Text) && m_Handle.CanGetBioseqHandle()) {
            const CBioseq_Handle& bio_handle = m_Handle.GetBioseqHandle();
            if (m_fExtOrgLookup) {
                const CBioSource* biosrc = objects::sequence::GetBioSource(bio_handle);
                if(biosrc) {
                    biosrc->GetOrg().GetLabel(&m_TaxLabel);
                } else {
                    m_TaxId = sequence::GetTaxId(bio_handle);
                    if (!m_TaxId) {
                        // There can be conflicting seq-entries in the scope which
                        // prevent us to get the tax id. MSA-75
                        // Use a fresh CScope to get tax id for well-known ids
                        CConstRef<CSeq_id> seq_id = bio_handle.GetSeqId();
                        m_TaxId = s_GetPristineScope(bio_handle).GetTaxId(*seq_id);
                    }
                    if (m_TaxId) {
                        CTaxonCache& tc = CTaxonCache::GetInstance();
                        m_TaxLabel = tc.GetLabel(m_TaxId);
                    }
                }
            } else {
                // Try descr.title
                if (bio_handle.CanGetDescr() && bio_handle.GetDescr().CanGet()) {
                    list<CRef<CSeqdesc> > seqdesclist = bio_handle.GetDescr().Get();
                    for (auto seqdesc : seqdesclist) {
                        if (seqdesc->IsTitle()) {
                            m_TaxLabel = seqdesc->GetTitle();
                            break;
                        }
                    }
                }
            }
        }
    }
    return m_TaxLabel;
}


const CRowDisplayStyle*    CAlnVecRow::GetDisplayStyle()
{
    return m_Style;
}


void CAlnVecRow::SetHost(IAlignRowHost* host)
{
    m_Host = host;
}


int CAlnVecRow::GetRowNum()  const
{
    return m_Handle.GetRowNum();
}


int CAlnVecRow::GetHeightPixels()    const
{
    return m_Height;
}


void CAlnVecRow::RenderColumn(TColumnType col_type)
{
    string error;
    _ASSERT(m_RenderCtx.m_Pane);
    if (m_Style) try {
    //CGlAttrGuard AttrGuard(GL_POLYGON_MODE | GL_LINE_BIT);
        auto state = m_RenderCtx.m_State;
        m_RenderCtx.m_Viewport.SetHorz(m_RenderCtx.m_Pane->GetViewport().Left(), m_RenderCtx.m_Pane->GetViewport().Right());

        switch (col_type) {
        case eDescr:     x_RenderDescr();       break;
        case eIcons:     x_RenderIcons();       break;
        case eIconStrand:x_RenderIconStrand();  break;
        case eIconExpand:x_RenderIconExpand();  break;
        case eSeqStart:  x_RenderSeqStart();    break;
        case eStart:     x_RenderStartPos();    break;
        case eAlignment: x_RenderAlign();       break;
        case eEnd:       x_RenderEndPos();      break;
        case eSeqEnd:    x_RenderSeqEnd();      break;
        case eSeqLength: x_RenderSeqLength();   break;
        case eTaxLabel:  x_RenderTaxLabel();    break;
        default: 
            NCBI_ASSERT(0, "Unknown column type");
            break; // unknown column
        } // switch
    } catch(CException& e)  {
        error = e.GetMsg();
    } catch(std::exception& e)  {
        error = e.what();
    } 
    if( ! error.empty())    {
        ERR_POST("CAlnVecRow::RenderColumn() " << error);
    }
}

void CAlnVecRow::GetColumnText(TColumnType col_type, string& text, bool for_printer) const
{
    text.clear();
    switch (col_type) {
    case eDescr:     
        text = m_Text;
        if (for_printer) {   
            text += " (";
            text += (m_Handle.IsNegativeStrand()) ? "-" : "+";
            text += ")";
        }
        break;
    case eSeqStart:  {
        text = CTextUtils::FormatSeparatedNumber(m_Handle.IsNegativeStrand() ? (m_Handle.GetSeqStop() + 1) : (m_Handle.GetSeqStart() + 1));
        break;
    }
    case eStart:  {   
        int aln_pos = (int)floor(m_RenderCtx.m_Pane->GetVisibleRect().Left());
        x_GetAlnPosLabel(aln_pos, IAlnExplorer::eRight, text);
        break;
    }
    case eEnd: {      
        int aln_pos = -1 + (int) ceil(m_RenderCtx.m_Pane->GetVisibleRect().Right());
        x_GetAlnPosLabel(aln_pos, IAlnExplorer::eLeft, text);
        break;
    }
    case eSeqEnd:    
        text = CTextUtils::FormatSeparatedNumber(m_Handle.IsNegativeStrand() ? (m_Handle.GetSeqStart() + 1) : (m_Handle.GetSeqStop() + 1));
        break;
    case eSeqLength: {
        TSeqPos pos = 0;
        if (m_Handle.CanGetBioseqHandle()) {
            pos = m_Handle.GetBioseqHandle().GetBioseqLength();
        } else {
            pos = abs(m_Handle.GetSeqStop() - m_Handle.GetSeqStart()) + 1;
        }
        text = CTextUtils::FormatSeparatedNumber(pos);
        break;
    }
    case eTaxLabel: 
        text = x_GetTaxLabel();
        break;
    default: 
        break; // unknown column
    } // switch
}

void CAlnVecRow::OnMouseEvent(wxMouseEvent& event, TColumnType col_type, CGlPane& pane)
{
    event.Skip(true);
    wxEventType type = event.GetEventType();
    if (type == wxEVT_LEFT_DOWN && (col_type == IAlignRow::eIconExpand || col_type == IAlignRow::eIcons)) {
        wxPoint pos = event.GetPosition();
        TVPPoint pt = x_GetHost()->ARH_GetVPPosByWindowPos(pos);
        int shift = ((col_type == IAlignRow::eIconExpand) ? 0 : 1);
        auto row_top = x_GetHost()->ARH_GetRowTop(this);
        PrepareRendering(pane, row_top, shift);
        TVPRect rc_btn = x_GetButtonRect(eExpand, shift);
        if(rc_btn.PtInRect(pt)) {
            event.Skip(false);
            x_OnClickExpandBtn(pane);
            return;
        }
    } 
    if (m_bExpanded && m_TrackPanel && m_TrackPanel->IsTrackInitDone()) {
        wxPoint pos = event.GetPosition();
        TVPPoint pt = x_GetHost()->ARH_GetVPPosByWindowPos(pos);
        TModelPoint hit_point;
        CRef<CSeqGlyph> glyph = x_HitTest(pt, pane, hit_point);
        //m_LastHitGlyph.Reset(glyph.GetPointer());
        _ASSERT(m_TrackHandler);
        m_TrackHandler->ProcessEvent(event);
        bool skipped = event.GetSkipped() || dynamic_cast<CLayoutTrack*>(glyph.GetPointer()) == nullptr;
        if (skipped) {
            if (glyph && glyph->IsClickable()) {
                if (type == wxEVT_LEFT_DOWN) {
                    skipped = glyph->OnLeftDown(hit_point);
                } else if (type == wxEVT_LEFT_UP) {
                    skipped = glyph->OnLeftUp(hit_point);
                    const IObjectBasedGlyph* obj = dynamic_cast<const IObjectBasedGlyph*>(&*glyph);
                    if (obj) {
                        CSelectionVisitor* sel_visitor = m_Host->ARH_GetSelectionVisitor();
                        CConstRef<CObject> sel_obj = obj->GetObject(hit_point.m_X);
                        if (sel_visitor->IsObjectSelected(sel_obj)) {
                            sel_visitor->DeSelectObject(sel_obj);
                        } else {
                            if (!event.CmdDown()) {
                                if (sel_visitor->HasSelectedObjects())
                                    sel_visitor->Clear();
                            }
                            sel_visitor->SelectObject(sel_obj, true);
                        }
                        skipped = true;
                        m_NeedUpdateSelection = true;
                        m_Host->ARH_OnSelectionChanged();
                    }
                } else if (type == wxEVT_LEFT_DCLICK) {
                    skipped = glyph->OnLeftDblClick(hit_point);
                }
            }
        } 
        event.Skip(skipped);
    }
}


static const int kTooltipMaxLines = 5;
static const int kMaxSeqInTooltip = 20;


void CAlnVecRow::GetTooltip(const TVPPoint& vp_pt, TColumnType col_type,
                            CGlPane& pane, ITooltipFormatter& tooltip)
{
    switch(col_type) {
    case eDescr:
        x_GetDescrTooltip(vp_pt, pane, tooltip);
        break;
    case eIcons:
        x_GetIconsTooltip(vp_pt, pane, tooltip);
        break;
    case eSeqStart:
        tooltip.AddRow("First position in sequence coordinates");
        break;
    case eStart:
        tooltip.AddRow("First visible position in alignment coordinates");
        break;
    case eAlignment:
        x_GetAlignmentTooltip(vp_pt, pane, tooltip);
        break;
    case eEnd:
        tooltip.AddRow("Last visible position in alignment coordinates");
        break;
    case eSeqEnd:
        tooltip.AddRow("Last position in sequence");
        break;
    case eSeqLength:
        tooltip.AddRow("Sequence length");
        break;
    case eTaxLabel:
        tooltip.AddRow("Organism name");
        break;
    default: 
        break;
    }
}


void CAlnVecRow::x_GetDescrTooltip(const TVPPoint& /*vp_pt*/, CGlPane& /*pane*/, ITooltipFormatter& tooltip)
{
    // first, the text for our row
    tooltip.AddRow(m_Text);
    string type;

    if (m_Handle.CanGetBioseqHandle()) {
        CBioseq_Handle handle = m_Handle.GetBioseqHandle();
        // next, title of the bioseq
        tooltip.AddRow(sequence::CDeflineGenerator().GenerateDefline(handle));

        // org info
        try {
            const COrg_ref& ref = sequence::GetOrg_ref(handle);
            string str;
            ref.GetLabel(&str);
            tooltip.AddRow("Organism:", str);
        } catch (CException&) {
        }

        /// sequence length information
        if (handle.IsNa()) {
            type = " bases";
        } else {
            type = " residues";
        }
        tooltip.AddRow("Sequence length:", NStr::IntToString(handle.GetBioseqLength(), NStr::fWithCommas) + type);

    }
    tooltip.AddRow("Aligned length:", NStr::IntToString(m_Handle.GetSeqStop() - m_Handle.GetSeqStart() + 1, NStr::fWithCommas) + type);
}

void CAlnVecRow::x_GetIconsTooltip(const TVPPoint& vp_pt, CGlPane& pane, ITooltipFormatter& tooltip)
{
    _ASSERT(x_GetHost());
    string s_tip;

    EButtons btn = x_HitTestButton(pane, vp_pt);
    switch(btn) {
    case eStrandMarker: {
        s_tip = "Strand - ";
        s_tip += m_Handle.IsNegativeStrand() ? "Negative" : "Positive";
    }; break;
    case eExpand:   {
        if(IsExpandable())    {
            s_tip = "Graphs: Click to ";
            s_tip += m_bExpanded ? "collapse" : "expand";
        } else {
            s_tip = "No graphs available - nothing to expand";
        }
    }; break;
    //case eSetupGraphs:  s_tip = "Setup Graphs"; break;
    default: break;
    }
    tooltip.AddRow(s_tip);
}


#define TOOLTIP_RANGE_PIX 3

string  FormatRange(TSignedSeqPos from, TSignedSeqPos to)
{
    string s =  NStr::IntToString(from + 1, NStr::fWithCommas);
    s += "-";
    s += NStr::IntToString(to + 1, NStr::fWithCommas);
    return s;
}


void CAlnVecRow::x_ForEachAlnSeg(const IAlnExplorer::TSignedRange& range, 
                                    const CGlPane& src_pane, 
                                    CGlPane& target_pane,
                                    const function <bool(CGlPane& pane)>& f)
{
    TModelUnit offset_x = src_pane.GetOffsetX();
    TModelUnit scale_x = src_pane.GetScaleX();
    TVPRect viewport = target_pane.GetViewport();
    TVPUnit left = viewport.Left();
    TVPUnit right = viewport.Right();

    IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eSkipInserts;
    auto_ptr<IAlnSegmentIterator> align_it(m_Handle.CreateSegmentIterator(range, flags));
    using TRangeData = tuple < IAlnExplorer::TSignedRange, IAlnExplorer::TSignedRange, COpenRange<double> >;
    vector <TRangeData> render_list;
    int index = -1;
    for (; *align_it; ++(*align_it)) {
        const IAlnSegment& seg = **align_it;
        if (!seg.IsAligned())
            continue;
        auto& seq_range = seg.GetRange();
        if (seq_range.Empty())
            continue;
        auto& aln_range = seg.GetAlnRange();
        if (aln_range.Empty())
            continue;

        //TSignedSeqPos aln_from = m_Handle.GetAlnPosFromSeqPos(seq_range.GetFrom(), IAlnExplorer::eRight);
//        TSignedSeqPos aln_to = m_Handle.GetAlnPosFromSeqPos(seq_range.GetTo(), IAlnExplorer::eLeft);
//        IAlnExplorer::TSignedRange aln_range(aln_from, aln_to);
        double x1 = aln_range.GetFrom();
        double x2 = aln_range.GetTo();
        if (x1 > x2)
            swap(x1, x2);
        x1 = (x1 - offset_x) / scale_x;
        x2 = (x2 - offset_x + 1) / scale_x;
        x1 = max<TModelUnit>(x1 + left, left);
        x2 = min<TModelUnit>(x2 + left, right);
        /*
        // x1 - prev x2
        if (index >= 0 && floor(x1 - get<2>(render_list[index]).GetTo()) <= 1.0) {
            auto& data = render_list[index];
            get<0>(data).SetTo(seq_range.GetTo());
            if (m_Handle.IsNegativeStrand())
                get<1>(data).SetTo(aln_range.GetTo());
            else
                get<1>(data).SetFrom(aln_range.GetFrom());
            get<2>(data).SetTo(x2);
        } else 
            */
        {
            render_list.emplace_back(seq_range, aln_range, COpenRange<double>(x1, x2));
            ++index;
        }
    }
    for (auto& data : render_list) {
        auto& seq_range = get<0>(data);
        auto& aln_range = get<1>(data);
        auto& view_range = get<2>(data);
        viewport.SetHorz(view_range.GetFrom(), view_range.GetTo());
        if (abs(viewport.Width()) <= 1)
            continue;
        auto l = view_range.GetFrom(), r = view_range.GetTo();
        if (l > r) swap(l, r);
        if (m_PixRight <= l || m_PixLeft >= r)
            continue;
        auto len = (r - l) + 1;
        if (len < 2)
            continue;

        IRender& gl = GetGl();
        if (gl.IsPrinterFriendly()) {
            gl.BeginClippingRect(viewport.Left(), viewport.Top(), viewport.Width(), viewport.Height());
        }
        target_pane.SetViewport(viewport);
        s_PrepareTrackPane(aln_range.GetFrom(), aln_range.GetTo(),
                           seq_range.GetFrom(), seq_range.GetTo(),
                           m_Handle, src_pane, target_pane);
        bool error = f(target_pane);
        if (gl.IsPrinterFriendly()) {
            gl.EndClippingRect();
        }
        if (error)
            break;
    }
}

CRef<CSeqGlyph> CAlnVecRow::x_HitTest(const TVPPoint& vp_pt, CGlPane& pane, TModelPoint& hit_point)
{
    /*
    // need to keep m_RenderingCtx.pane pointing to m_TrackPane
    // as it's used in in track handlers
    struct SRenderingContextGuard
    {
        SRenderingContextGuard(const function<void()>& f) : m_f(f) {}
        ~SRenderingContextGuard() { m_f(); }
        function<void()> m_f;
    };
    */
    SRenderingContextGuard ctx_guard([&]() {
        m_RenderingCtx->SetRenderingFlags(0);
        m_RenderingCtx->PrepareContext(*m_TrackPane, true, m_Handle.IsNegativeStrand());
    });

    // First check if there is a hit for TrackPane 
    // which covers the whole track area
    // and return if mouse hits a track container.
    // In this case we're intersted in  track title bar and icon controls hits 
    // for all other objects we check the hits within each alignment segment
    CRef<CSeqGlyph> glyph;

    CGlPane track_pane(CGlPane::eAlwaysUpdate);
    if (!x_InitTrackPane(pane, track_pane))
        return glyph;

    const TModelRect& rc_vis = track_pane.GetVisibleRect();
    m_RenderingCtx->SetRenderingFlags(0);

    TSignedSeqRange range(rc_vis.Left(), rc_vis.Right());
    TModelUnit seq_from = m_Handle.GetSeqPosFromAlnPos(rc_vis.Left(), IAlnExplorer::eRight);
    TModelUnit seq_to = m_Handle.GetSeqPosFromAlnPos(rc_vis.Right(), IAlnExplorer::eLeft);
    s_PrepareTrackPane(rc_vis.Left(), rc_vis.Right(), seq_from, seq_to,
                       m_Handle, pane, track_pane);
    if (m_Style->GetSeqGraphicConfig()->GetCgiMode())
        m_RenderingCtx->SetRenderingFlags(CRenderingContext::fSkipTitleBar | CRenderingContext::fSkipControls);
    m_RenderingCtx->PrepareContext(track_pane, true, m_Handle.IsNegativeStrand());

    {
        CGlPaneGuard GUARD(track_pane, CGlPane::eOrtho);
        hit_point = track_pane.UnProject(vp_pt.X(), vp_pt.Y());
    }
    //m_TrackPanel->Update(true);
    CRef<CSeqGlyph> layout_track_hit = m_TrackPanel->HitTest(hit_point);
    if (dynamic_cast<CLayoutTrack*>(layout_track_hit.GetPointer()) != 0)
        m_LastHitTrack.Reset(dynamic_cast<CLayoutTrack*>(layout_track_hit.GetPointer()));
    //  a title/icons bar or any glyph if it's a master row
    if (m_Handle.IsAnchor() || dynamic_cast<CLayoutTrack*>(layout_track_hit.GetPointer()) != 0) 
        return layout_track_hit;
    {
        // get hit_point.m_X from the alignment row pane
        // to avoid rounding discrepancies
        // when the pos is uprojected from track_pane
        // hit_point.m_Y is unprojected from m_TrackPane
        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
        TSeqPos pos = (TSeqPos)pane.UnProjectX(vp_pt.X());
        hit_point.m_X = m_Handle.GetSeqPosFromAlnPos(pos);
    }

    if (hit_point.m_X < 0)
        return glyph;

    if (m_NeedUpdateSelection) {
        CSelectionVisitor* sel_visitor = m_Host->ARH_GetSelectionVisitor();
        sel_visitor->UpdateSelection(&*m_TrackPanel);
    }
    m_RenderingCtx->SetRenderingFlags(CRenderingContext::fSkipTitleBar | CRenderingContext::fSkipControls);
    
    x_ForEachAlnSeg(range, pane, track_pane, [&](CGlPane& track_pane) {
        m_RenderingCtx->PrepareContext(track_pane, true, m_Handle.IsNegativeStrand());
        if (!m_RenderingCtx->IsInVisibleRange(hit_point.m_X))
            return false;
        m_TrackPanel->Update(true);
        glyph = m_TrackPanel->CGlyphContainer::HitTest(hit_point);
        if (dynamic_cast<CLayoutTrack*>(glyph.GetPointer()) != 0)
            m_LastHitTrack.Reset(dynamic_cast<CLayoutTrack*>(glyph.GetPointer()));
        return glyph != 0;
    });
    return glyph;

}

void CAlnVecRow::x_GetAlignmentTooltip(const TVPPoint& vp_pt, CGlPane& pane, ITooltipFormatter& tooltip)
{
    pane.OpenOrtho();
    pane.Close();
    double w_x = pane.UnProjectX(vp_pt.X());
    double pix_w = pane.UnProjectWidth(TOOLTIP_RANGE_PIX);
    TSeqPos pos = (TSeqPos)w_x;

    if (m_bExpanded && m_TrackPanel) {
        // m_PixLeft, m_PixRight are not initialized in sviewer context
        if (m_PixLeft <0 || m_PixRight < 0) {
            const auto& vp = pane.GetViewport();
            m_PixLeft = vp.Left();
            m_PixRight = vp.Right();
        }

        TModelPoint hit_point;
        CRef<CSeqGlyph> glyph = x_HitTest(vp_pt, pane, hit_point);
        //m_LastHitGlyph.Reset(glyph.GetPointer());
        if (glyph) {
            string s_tip;
            CIRef<ITooltipFormatter> track_tooltip = CHtmlTooltipFormatter::CreateTooltipFormatter();
            glyph->GetTooltip(hit_point, *track_tooltip, s_tip);
            if (!track_tooltip->IsEmpty()) {
                // Show only feat panel tooltip for title bars and menu icons
                // otherwise combine track panel and aligmnet tooltips
                if (dynamic_cast<CLayoutTrack*>(glyph.GetPointer()) != nullptr) {
                    m_LastHitTrack.Reset(dynamic_cast<CLayoutTrack*>(glyph.GetPointer()));
                    tooltip.Append(*track_tooltip);
                    return;
                }
                s_tip = m_Text;
                if (m_Handle.IsNegativeStrand())
                    s_tip += ", Negative Strand";
                tooltip.AddRow(s_tip);
                tooltip.AddRow("Alignment Pos:", NStr::IntToString(pos + 1, NStr::fWithCommas));
                tooltip.AddRow(NcbiEmptyString);
                tooltip.Append(*track_tooltip);
                return;
            }
        }
    } 
    // Append information on gaps and inserts
    TSeqPos start = (TSeqPos)floor(w_x - pix_w / 2);
    TSeqPos end = (TSeqPos)ceil(w_x + pix_w / 2);
    //TSeqPos start = (TSeqPos)floor(w_x);
    TSignedRange range(start, end);
    x_GetAlignmentTooltip_Helper(pos, range, tooltip);
}


string CAlnVecRow::GetAlignmentTooltip(TSeqPos pos, TSeqPos half_width)
{
    TSignedRange range(pos-half_width, pos+half_width);
    CIRef<ITooltipFormatter> formatter = CHtmlTooltipFormatter::CreateTooltipFormatter();
    x_GetAlignmentTooltip_Helper(pos, range, *formatter);
    return formatter->Render();
}

void CAlnVecRow::x_GetAlignmentTooltip_Helper(TSeqPos pos, TSignedRange& range, ITooltipFormatter& tooltip)
{
    x_GetAlignmentTooltip_General(pos, tooltip);
    {
        IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eInsertsOnly;
        //IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eAllSegments;
        auto_ptr<IAlnSegmentIterator> p_it(m_Handle.CreateSegmentIterator(range, flags));
        for (IAlnSegmentIterator& it = *p_it; it; ++it) {
            const IAlnSegment& seg = *it;
            if (seg.GetRange().Empty())
                continue;
            const TSignedSeqRange& aln_r = it->GetAlnRange();
            const TSignedSeqRange& seq_r = it->GetRange();
            x_GetAlignmentTooltip_Insert(aln_r, seq_r, tooltip);
            return;
        }
    }

    
    // obtain segments in the range [start, end]
    IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eAllSegments;
    auto_ptr<IAlnSegmentIterator> p_it(m_Handle.CreateSegmentIterator(range, flags));
    IAlnSegmentIterator& it = *p_it;

    if(it)   {
        // last positions on the last aligned segment
        TSignedSeqPos prev_to = -2, prev_aln_to = -2;
        const int kMaxRegions = 5; // max inserts or unaligned regions reported
        int n_regions = 0;

        for (int i = 0;  it  &&  i < kTooltipMaxLines;  i++, ++it  )    {
            const TSignedSeqRange& aln_r = it->GetAlnRange();
            if (m_Handle.GetSeqAlnRange().IntersectionWith(aln_r).Empty())
                break;
            IAlnSegment::TSegTypeFlags type = it->GetType();
            const TSignedSeqRange& seq_r = it->GetRange();

            bool aligned = (type & IAlnSegment::fAligned)  &&  ! aln_r.Empty();
            bool insert = (type & IAlnSegment::fAligned) && aln_r.Empty();
            bool gap = (type & IAlnSegment::fGap) && aln_r.GetLength() > 0;

            if(insert)  {
                if(n_regions < kMaxRegions) {
                    x_GetAlignmentTooltip_Insert(aln_r, seq_r, tooltip);
                }
                n_regions++;
            }   else if (gap)   {
                if(n_regions < kMaxRegions) {
                    auto seq_from = m_Handle.GetSeqPosFromAlnPos(pos, IAlnExplorer::eLeft);
                    auto seq_to = m_Handle.GetSeqPosFromAlnPos(pos, IAlnExplorer::eRight);
                    if (seq_from > seq_to)
                        swap(seq_from, seq_to);

                    auto aln_from = m_Handle.GetAlnPosFromSeqPos(seq_from, IAlnExplorer::eLeft) + 1;
                    auto aln_to = m_Handle.GetAlnPosFromSeqPos(seq_to, IAlnExplorer::eRight) - 1;
                    if (aln_from > aln_to)
                        swap(aln_from, aln_to);
                    
                    x_GetAlignmentTooltip_Gap(TSignedSeqRange(aln_from, aln_to), TSignedSeqRange(seq_from, seq_to), tooltip);
                    //x_GetAlignmentTooltip_Gap(aln_r, seq_r, tooltip);
                }
                n_regions++;
            } else if(aligned)  {
                if(aln_r.GetFrom() == prev_aln_to + 1  &&  seq_r.GetFrom() > prev_to + 1)  {
                    // there is an unaligned region
                    if(n_regions < kMaxRegions) {
                        x_GetAlignmentTooltip_Unaligned(prev_aln_to, aln_r.GetFrom(),
                                                        prev_to, seq_r.GetFrom(),
                                                        tooltip);
                    }
                    n_regions++;
                }

                prev_aln_to = aln_r.GetTo();
                prev_to = seq_r.GetTo();
            }
        }

        if(n_regions > kMaxRegions) 
            tooltip.AddRow("total: ", NStr::IntToString(n_regions));
    }
}


// Sequence title: descr / title
static string s_GetTitle(const CSeq_descr &descr)
{
    string res;
    for (auto seqdesc : descr.Get()) {
        if (seqdesc->IsTitle()) {
            res = seqdesc->GetTitle();
            break;
        }
    }
    return res;
}


// Source subtypes: descr / source / subtype / subtype *
static string s_GetSourceSubtype(const CSeq_descr &descr, CSubSource::ESubtype subtype)
{
    string res;
    for (auto seqdesc : descr.Get()) {
        if (seqdesc->IsSource()) {
            const CBioSource &bs = seqdesc->GetSource();
            if (bs.IsSetSubtype()) {
                for (auto subsrc : bs.GetSubtype()) {
                    if (subsrc->IsSetSubtype() && subsrc->GetSubtype() == subtype) {
                        return subsrc->GetName();
                    }
                }
            }
        }
    }
    return res;
}


// Host: descr / source / org / orgname / mod / subtype nat-host
static string s_GetOrgModNatHost(const CSeq_descr &descr)
{
    string res;
    for (auto seqdesc : descr.Get()) {
        if (seqdesc->IsSource()) {
            const CBioSource &bs = seqdesc->GetSource();
            if (bs.IsSetOrgname()) {
                const COrgName& on = bs.GetOrgname();
                if (on.CanGetMod()) {
                    for (auto orgmod : on.GetMod()) {
                        if (orgmod->IsSetSubtype() &&
                            orgmod->GetSubtype() == COrgMod::eSubtype_nat_host)
                        {
                            return orgmod->GetSubname();
                        }
                    }
                }
            }
        }
    }
    return res;
}


void CAlnVecRow::x_GetAlignmentTooltip_General(TSeqPos pos, ITooltipFormatter& tooltip)
{
    // append Name and Position
    string s_tip = m_Text;
    if (m_Handle.CanGetBioseqHandle()) {

        const CBioseq_Handle &bsh = m_Handle.GetBioseqHandle();

        if (bsh.IsAa()) {
            try {
                string title = sequence::GetProteinName(bsh);
                if (!title.empty()) {
                    s_tip += ", ";
                    s_tip += title;
                }
            } catch (const CException&) {

            }
        }

        s_tip += " (";
        s_tip += NStr::NumericToString(bsh.GetBioseqLength(), NStr::fWithCommas);
        if (bsh.IsAa())
            s_tip += " residues)";
        else
            s_tip += " bases)";

        if (m_Handle.IsNegativeStrand())
            s_tip += ", Negative Strand";

        tooltip.AddRow(s_tip);

        // Format taxonomy information
        x_GetTaxonomyInformation(bsh, tooltip);

    } else {
        if (m_Handle.IsNegativeStrand())
            s_tip += ", Negative Strand";
        tooltip.AddRow(s_tip);
    }


    // Insert here fields from MSA-404
    // Sequence title:   descr / title
    // Country:          descr / source / subtype / subtype country
    // Host:             descr / source / org / orgname / mod / subtype nat-host
    // Isolation source: descr / source / subtype / subtype isolation-source
    // Collection date:  descr / source / subtype / subtype collection-date
    if (m_Handle.CanGetBioseqHandle()) {
        const CBioseq_Handle &bsh = m_Handle.GetBioseqHandle();
        if (bsh.CanGetDescr()) {
            const CSeq_descr &descr = bsh.GetDescr();
            string s;
            s = s_GetTitle(descr);
            if (!s.empty()) tooltip.AddRow("Sequence title:", s);
            s = s_GetSourceSubtype(descr, CSubSource::eSubtype_country);
            if (!s.empty()) tooltip.AddRow("Country:", s);

 
            s = s_GetOrgModNatHost(descr);
            if (!s.empty()) tooltip.AddRow("Host:", s);

            s = s_GetSourceSubtype(descr, CSubSource::eSubtype_isolation_source);
            if (!s.empty()) tooltip.AddRow("Isolation source:", s);
            s = s_GetSourceSubtype(descr, CSubSource::eSubtype_collection_date);
            if (!s.empty()) tooltip.AddRow("Collection date:", s);
        }
    }

    tooltip.AddRow("Alignment Pos:", NStr::IntToString(pos + 1, NStr::fWithCommas));

    TSignedSeqPos seq_pos = m_Handle.GetSeqPosFromAlnPos(pos);
    auto base_width = m_Handle.UsesAATranslation() && m_Handle.IsNucProtAlignment() ? 3 : 1;
    seq_pos /= base_width;
    if (seq_pos >= 0)
        tooltip.AddRow("Sequence Pos:", NStr::IntToString(seq_pos + 1, NStr::fWithCommas));
}


void CAlnVecRow::x_GetTaxonomyInformation(const objects::CBioseq_Handle &bsh, ITooltipFormatter& tooltip) const
{
    string taxonomy;
        
    const CBioSource* biosrc = objects::sequence::GetBioSource(bsh);
    if (biosrc) {
        x_GetTaxonomyInfoFromBiosrc(*biosrc, taxonomy);
    }
    else if (0 != m_TaxId) {
        x_GetTaxonomyInfoFromService(taxonomy);
    }

    if (taxonomy.empty()) taxonomy = x_GetTaxLabel();
    if (taxonomy.empty())
        return;
    
    tooltip.AddRow("Organism:", taxonomy);
}


void CAlnVecRow::x_GetTaxonomyInfoFromService(std::string &taxonomy) const
{
    CTaxonCache& tc = CTaxonCache::GetInstance();
    taxonomy = tc.GetTaxname(m_TaxId);
    string temp = tc.GetCommon(m_TaxId);
    if (!temp.empty()) {
        taxonomy += " (";
        taxonomy += temp;
        taxonomy += ')';
    }
    temp = tc.GetBlastName(m_TaxId);
    if (!temp.empty()) {
        taxonomy += " [";
        taxonomy += temp;
        taxonomy += ']';
    }
}


void CAlnVecRow::x_GetTaxonomyInfoFromBiosrc(const objects::CBioSource &biosrc, std::string &taxonomy) const
{
    taxonomy = biosrc.GetTaxname();
    const COrg_ref &org = biosrc.GetOrg();
    if (org.CanGetCommon()) {
        taxonomy += " (";
        taxonomy += org.GetCommon();
        taxonomy += ')';
    }
}


void CAlnVecRow::x_GetAlignmentTooltip_Insert(const TSignedSeqRange& aln_r,
                                              const TSignedSeqRange& seq_r,
                                              ITooltipFormatter& tooltip)
{
    TSeqPos aln_left = aln_r.GetTo(); // a bit of hack

    string s_tip = "Insert - ";

    s_tip += NStr::IntToString(aln_left + 1, NStr::fWithCommas);
    s_tip += "][";
    s_tip += NStr::IntToString(aln_left + 2, NStr::fWithCommas);

    s_tip += ", on Sequence [";
    s_tip += FormatRange(seq_r.GetFrom(), seq_r.GetTo() + 1);
    s_tip += "] : ";

    s_tip += x_GetTooltipSequence(seq_r.GetFrom(), seq_r.GetTo());
    tooltip.AddRow(s_tip);
}


void CAlnVecRow::x_GetAlignmentTooltip_Gap(const TSignedSeqRange& aln_r,
                                           const TSignedSeqRange& seq_r,
                                           ITooltipFormatter& tooltip)
{
    //string s_tip = "Gap ";
    //s_tip += FormatRange(aln_r.GetFrom() + 1, aln_r.GetTo() + 1);
    //s_tip += ", on Sequence ";
    //s_tip += FormatRange(seq_r.GetFrom() + 1, seq_r.GetTo() + 1);
    //tooltip.AddRow(s_tip);

    int gap_len = aln_r.GetLength();
    if (seq_r.GetLength() > 0) {
        string s_tip = " Gap - ";
        s_tip += NStr::NumericToString(aln_r.GetLength());
        if (m_Handle.UsesAATranslation()) {
            s_tip += " residue";
        } else {
            s_tip += " base";
        }
        if (gap_len > 1)
            s_tip += "s";
        s_tip += " on Sequence ";
        s_tip += NStr::IntToString(seq_r.GetFrom() + 1, NStr::fWithCommas);
        s_tip += "][";
        s_tip += NStr::IntToString(seq_r.GetFrom() + 2, NStr::fWithCommas);
        tooltip.AddRow(s_tip);
    }

}


void CAlnVecRow::x_GetAlignmentTooltip_Unaligned(TSignedSeqPos aln_prev_to,
                                                 TSignedSeqPos aln_from,
                                                 TSignedSeqPos prev_to,
                                                 TSignedSeqPos from,
                                                 ITooltipFormatter& tooltip)
{
    string s_tip = "Unaligned region - ";

    s_tip += NStr::IntToString(aln_prev_to + 1, NStr::fWithCommas);
    s_tip += "][";
    s_tip += NStr::IntToString(aln_from + 1, NStr::fWithCommas);

    s_tip += ", on Sequence [";
    s_tip += FormatRange(prev_to + 1, from - 1);
    s_tip += "] : ";

    s_tip += x_GetTooltipSequence(prev_to + 1, from - 1);
    tooltip.AddRow(s_tip);
}


string CAlnVecRow::x_GetTooltipSequence(TSeqPos from, TSeqPos to)
{
    string seq;
    TSeqRange seq_text_r(from, to);

    if((int) seq_text_r.GetLength() > kMaxSeqInTooltip)    { // to long to be displayed
        // get kMaxSeqInTooltip / 2 bases on the left and on the right
        TSeqPos left = seq_text_r.GetFrom();
        TSeqRange part_r(left, left + kMaxSeqInTooltip / 2);

        m_Handle.GetSeqString(seq, part_r);

        TSeqPos to_open = to + 1;
        part_r.Set(to_open - kMaxSeqInTooltip / 2, to_open -1);
        string right_seq;
        m_Handle.GetSeqString(right_seq, part_r);

        seq += "..." + right_seq;
    } else {
        m_Handle.GetSeqString(seq, seq_text_r);
    }
    return seq;
}


void CAlnVecRow::x_OnClickExpandBtn(CGlPane& pane)
{
    wxBusyCursor wait;

    Expand(! IsExpanded(), pane);
}


inline  void    glLined(TModelUnit x1, TModelUnit y1, TModelUnit x2, TModelUnit y2)
{
    IRender& gl = GetGl();
    gl.Vertex2d(x1, y1);
    gl.Vertex2d(x2, y2);
}


const static TModelUnit kScoreMinScale = 0.5;


/// Renders graphical part of the row
void CAlnVecRow::x_RenderAlign()
{
    _ASSERT(m_Style);

    IRender& gl = GetGl();
    CGlPane& pane = *m_RenderCtx.m_Pane;
    pane.EnableOffset();
    
    const TModelRect& rc_lim = (*m_RenderCtx.m_Pane).GetModelLimitsRect();
    const TModelRect& rc_model = (*m_RenderCtx.m_Pane).GetVisibleRect();
    m_ModelLeft = rc_model.Left();
    m_ModelRight = rc_model.Right();
    {
        const auto& vp = (*m_RenderCtx.m_Pane).GetViewport();
        m_PixLeft = vp.Left();
        m_PixRight = vp.Right();
    }
    // now render segments actually present
    int left = (int) floor(rc_model.Left());
    int right = int(ceil(rc_model.Right())) - 1;

    TSignedRange range(left, right);
    int shown = m_Style->GetShownElements();
    if (shown & CRowDisplayStyle::fShown_Alignment) {
        TModelUnit offset_y = m_RenderCtx.m_Viewport.Top();
        TModelUnit y2 = rc_lim.Top() + kAlignSpace - offset_y;
        TModelUnit y1 = y2 + m_BaseHeight - 1 - (kAlignSpace  * 2);
        TModelUnit scale_x = 1.;
        {
            CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
            scale_x = pane.GetScaleX();
        }

        IScoringMethod* method = m_Cache ? m_Cache->GetScoringMethod() : nullptr;
        bool averageable = method && method->IsAverageable();

        /// Render Alignment segments & Gaps
        bool has_scores = m_Cache && m_Cache->HasScores();
        bool can_render_scores = averageable || scale_x < kScoreMinScale; // resolution permits rendering
        gl.LineWidth(1.0f);
        gl.ColorC(m_Style->GetColor(CWidgetDisplayStyle::eAlignSegs));
        // render aligned segments
        IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eAllSegments;
        auto_ptr<IAlnSegmentIterator> p_it(m_Handle.CreateSegmentIterator(range, flags));
        IAlnSegmentIterator& it = *p_it;

        if (has_scores  &&  can_render_scores) {
            // render parts of segments accordingly to scores
            x_RenderAlignScores(it, (int)y1, (int)y2, averageable);
        } else {
            // scores cannot be rendered
            CWidgetDisplayStyle::EColorType type =
                has_scores ? CWidgetDisplayStyle::eAlignSegsNoScores
                : CWidgetDisplayStyle::eAlignSegs;
            const CRgbaColor& color = m_Style->GetColor(type);
            x_RenderAlignSegments(pane, it, y1, y2, color);
        }

        {
            CGlPaneGuard GUARD(pane, CGlPane::eOrtho);

            GLint orig_vp[4];
            if (gl.IsPrinterFriendly()) {
                gl.GetViewport(orig_vp);
                
                const TVPRect *clip = pane.GetClipRect();
                if (clip) {
                    gl.Viewport(orig_vp[0], clip->Bottom(), orig_vp[2], clip->Height() + 1);
                }
                TVPRect &vp = m_RenderCtx.m_Viewport;
                gl.BeginClippingRect(vp.Left(), vp.Top(), vp.Width(), vp.Height());
            }

            /// Render Inserts
            x_RenderInserts(pane, left, right, y1, y2);
            x_RenderUnaligned(pane, left, right, y1, y2);
            /// Render Sequence
            if (x_CanRenderSeq(pane)) {
                int start = (left < floor(rc_model.Left())) ? left + 1 : left;
                int stop = (int)ceil(rc_model.Right());
                if (stop + 1 > rc_model.Right()) {
                    stop--;
                }
                x_RenderSequence(pane, y1, y2, start, stop);
            }

            if (gl.IsPrinterFriendly()) {
                gl.Viewport(orig_vp[0], orig_vp[1], orig_vp[2], orig_vp[3]);
                gl.EndClippingRect();
            }
        }
    }

    if (shown & CRowDisplayStyle::fShown_Track) {
        /// Render Graph
        if (IsExpanded())
            x_RenderTracks(*m_RenderCtx.m_Pane, range);
    }

    if (!m_Style->IsWebRendering()) {   
        // table cell frame
        CGlPaneGuard GUARD(pane, CGlPane::ePixels);
        gl.LineWidth(1.0f);
        gl.ColorC(m_Style->GetColor(CWidgetDisplayStyle::eFrame));
        const auto& rc_vp = m_RenderCtx.m_Viewport;
        gl.Begin(GL_LINES);
        gl.Vertex2f((float)rc_vp.Right(), (float)rc_vp.Top() + 1);
        gl.Vertex2f((float)rc_vp.Right(), (float)rc_vp.Bottom() - 1);
        gl.End();
    }

}


inline double CompressLength(double len)
{
    if(len <= 10.0) {
        return len;
    } else {
        double log = log10(len) * 10;
        return log;
    }
}


void CAlnVecRow::x_RenderInserts(CGlPane& pane, int from, int to,
                                 TModelUnit top_y, TModelUnit bottom_y)
{
    IRender& gl = GetGl();

    TSignedRange range(from, to);
    TModelUnit offset_x = pane.GetOffsetX();

    CRgbaColor cl_insert(0.0f, 0.0f, 1.0f);
    gl.ColorC(cl_insert);
    gl.Begin(GL_LINES);

    IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eInsertsOnly;
    auto_ptr<IAlnSegmentIterator> p_it_2(m_Handle.CreateSegmentIterator(range, flags));

    const TModelUnit kMinLen = pane.GetScaleX() * 3; // 3 pixels in model units
    int row_start = m_Handle.GetSeqAlnStart();
    int row_stop = m_Handle.GetSeqAlnStop();

    for(  IAlnSegmentIterator& it_2 = *p_it_2;  it_2;  ++it_2 )  {
        TModelUnit ins_pos = it_2->GetAlnRange().GetFrom() - offset_x;
        if (ins_pos < row_start || ins_pos > row_stop)
            continue;
        TModelUnit len = it_2->GetRange().GetLength();
        TModelUnit log_len = CompressLength(len);
        log_len = max(log_len, kMinLen);
        TModelUnit x1 = ins_pos - log_len / 2;
        TModelUnit x2 = x1 + log_len;

        // assuming glBegin(GL_LINES)
        glLined(ins_pos, top_y - 1, ins_pos, bottom_y - 1);
        glLined(x1, top_y - 1, x2, top_y - 1);
        glLined(x1, bottom_y, x2, bottom_y);
    }
    gl.End();
    if (1==0) {
        IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eSkipInserts;
        auto_ptr<IAlnSegmentIterator> p_it(m_Handle.CreateSegmentIterator(range, flags));
        int row_start = m_Handle.GetSeqAlnStart();
        int row_stop = m_Handle.GetSeqAlnStop();

        const TModelUnit kMinLen = pane.GetScaleX() * 3; // 3 pixels in model units

        CRgbaColor blue("black");

        vector<TSeqRange> gaps;

        for (IAlnSegmentIterator& it_seg = *p_it; it_seg; ++it_seg) {
            // get the intersection of the score and align intervals
            const IAlnSegment& seg = *it_seg;
            if (seg.IsAligned())
                continue;
            TSignedSeqRange r = seg.GetAlnRange();
            if (r.Empty())
                continue;
            bool empty_seg = (r.GetFrom() < row_start || r.GetTo() > row_stop);
            if (empty_seg)
                continue;
            if (!seg.GetRange().GetToOpen() == 0)
                continue;
            if (gaps.empty())
                gaps.emplace_back(r.GetFrom(), r.GetTo());
            else {
                if (r.GetFrom() - gaps.back().GetTo() == 1) {
                    gaps.back().SetTo(r.GetTo());
                }
                else {
                    gaps.emplace_back(r.GetFrom(), r.GetTo());
                }
            }
        }
        if (!gaps.empty()) {
            gl.Enable(GL_BLEND);
            gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            gl.Begin(GL_LINES);
            for (const auto& r : gaps) {
                double pix_len = r.GetLength();
                pix_len /= pane.GetScaleX();
                if (pix_len < 1.5) {
                    auto pos = r.GetFrom() - offset_x;
                    float alpha = 0.25 + r.GetLength() * 0.75;
                    blue.SetAlpha(alpha);
                    gl.ColorC(blue);
                    gl.Vertex2d(pos, bottom_y);
                    gl.Vertex2d(pos, top_y - 2);
                }
            }
            gl.End();
            gl.Disable(GL_BLEND);
        }

    }
}


void CAlnVecRow::x_RenderUnaligned(CGlPane& pane, int from, int to,
                                 TModelUnit top_y, TModelUnit bottom_y)
{
    IRender& gl = GetGl();

    TSignedRange range(from, to);
    TModelUnit offset_x = pane.GetOffsetX();

    CRgbaColor cl_unaligned(0.0f, 0.5f, 0.0f);
    gl.ColorC(cl_unaligned);
    gl.Begin(GL_LINES);

    IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eSkipGaps;
    auto_ptr<IAlnSegmentIterator> p_it_2(m_Handle.CreateSegmentIterator(range, flags));

    const TModelUnit kMinLen = pane.GetScaleX() * 3; // 3 pixels in model units

    TSignedSeqPos prev_pos = -2, prev_aln_pos = -2;
    for(  IAlnSegmentIterator& it_2 = *p_it_2;  it_2;  ++it_2 )  {
        const TSignedRange& aln_r = it_2->GetAlnRange();
        const TSignedRange& r = it_2->GetRange();

        if(aln_r.GetFrom() == prev_aln_pos + 1  &&  r.GetFrom() > prev_pos + 1)  {
            TModelUnit ins_pos = aln_r.GetFrom() - offset_x;
            TModelUnit len = it_2->GetRange().GetLength();
            TModelUnit log_len = CompressLength(len);
            log_len = max(log_len, kMinLen);
            TModelUnit x1 = ins_pos - log_len / 2;
            TModelUnit x2 = x1 + log_len;

            // assuming gl.Begin(GL_LINES)
            glLined(ins_pos, top_y, ins_pos, bottom_y);
            glLined(x1, top_y, x2, top_y);
            glLined(x1, bottom_y, x2, bottom_y);
        }

        prev_aln_pos = aln_r.GetTo();
        prev_pos = r.GetTo();
    }
    gl.End();
}


struct FWeightedSum
{
    void    operator()(float& dst, const float& src, float fraction, bool long_seg)
    {
        dst += src * fraction;
    }
};


void CAlnVecRow::x_RenderAlignSegments(CGlPane& pane, IAlnSegmentIterator& it,
                                       TModelUnit top_y, TModelUnit bottom_y,
                                       const CRgbaColor& color)
{
    IRender& gl = GetGl();

    const auto& rc_vp = m_RenderCtx.m_Viewport;
    //CGlPane& pane = *m_RenderCtx.m_Pane;
    const TModelRect& rc_vis = pane.GetVisibleRect();  
    
    /// Render Segments
    CGlPaneGuard GUARD(pane, CGlPane::ePixels);

    /// Rasterize segments - raster represents density of pixel coverage by segments
    typedef CRasterizer<float>    TR;
    TR rasterizer(rc_vp.Left(), rc_vp.Right(), rc_vis.Left(), rc_vis.Right());

    FWeightedSum f_sum;
    for( ;  it;  ++it )  {
        const IAlnSegment& seg = *it;
        if(seg.IsAligned())  {
            const TSignedSeqRange& r = seg.GetAlnRange();
            rasterizer.AddInterval(r.GetFrom(), r.GetToOpen(), 1.0, f_sum); /* NCBI_FAKE_WARNING: WorkShop */
        }
    }

    TVPUnit vp_top_y = rc_vp.Top();
    TVPUnit vp_bottom_y = vp_top_y - (m_BaseHeight - 1) + kAlignSpace;

    const TR::TRaster& raster = rasterizer.GetRaster();

    //CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    int off_x = rc_vp.Left();
    static const float eps = 0.001f;

    TModelRange model_range(m_ModelLeft, m_ModelRight);
    TModelRange row_range(m_Handle.GetSeqAlnStart(), m_Handle.GetSeqAlnStop());

    row_range.IntersectWith(model_range);

    if (!row_range.Empty()) {
        // render a single line for all possible Gaps
        TVPUnit y_c = floor((float(vp_top_y + vp_bottom_y) / 2) + 0.5);

        TModelUnit row_start = rasterizer.Project(row_range.GetFrom());
        TModelUnit row_stop = rasterizer.Project(row_range.GetTo());

        gl.Begin(GL_LINES);
        gl.ColorC(color);
        glLined(row_start + off_x, y_c, row_stop + off_x, y_c);
        gl.End();
    }

    CRgbaColor cl(color);
    for( size_t i = 0;  i < raster.size(); )    {
        size_t start = i;
        float v = raster[i];
        while(i < raster.size()  &&  raster[i] == v) {
            i++;
        }

        if(v > eps)   {    /// otherwise consider it to be transparent
            /// use density as a measure of transparency
            // v is distributed in [0, 1], however Aplha channel needs to be in
            // [0.25, 1.0] range
            double alpha = 0.25 + v * 0.75;
            cl.SetAlpha((float) alpha);
            gl.ColorC(cl);
            gl.Rectd(start + off_x, vp_top_y, i + off_x, vp_bottom_y);
        }
    }
}


/// represents a pixel with a score, if several intervals cover a pixel, the interval
/// with the maximum coverage will "own" the pixel (i.e. the pixel will be colored
/// according to the score of this interval)
/// This is used for non-averageable scores.
struct SScPix   {
    float   m_Score;    /// score assigned to the pixel
    float   m_MaxCoverage; /// fraction of the pixel corresponding to the m_Score
    bool    m_Gap; /// true if this is a gap interval

    bool operator==(const SScPix& other) const
    {
        return m_Score == other.m_Score  &&  m_MaxCoverage == other.m_MaxCoverage
               &&  m_Gap == other.m_Gap;
    }
};


/// a pixel with the Max coverage (fraction) overrides all attributes
static void FSetPixMaxCoverage(SScPix& dst, const SScPix& src, float fraction, bool long_seg)
{
    if(fraction > dst.m_MaxCoverage)   {
        dst.m_Score = src.m_Score;
        dst.m_Gap = src.m_Gap;
        dst.m_MaxCoverage = fraction;
    }
}


void CAlnVecRow::x_RenderAlignScores(IAlnSegmentIterator& it_seg, int /*top_y*/, int /*bottom_y*/, bool averageable)
{
    IRender& gl = GetGl();

    typedef CScoreCache::TScore TScore;
    const auto& rc_vp = m_RenderCtx.m_Viewport;
    CGlPane& pane = *m_RenderCtx.m_Pane;

    CGlPaneGuard GUARD(pane, CGlPane::ePixels);

    const TModelRect& rc_vis = pane.GetVisibleRect();

    //gl.Translated(0.0, rc_vp.Top(), 0.0); // to make the display list "scrollable" vertically

    //if horz projection has changed - invalidate display list
    if(m_PixLeft != rc_vp.Left()  ||  m_PixRight != rc_vp.Right()
        ||  m_ModelLeft != rc_vis.Left()  ||  m_ModelRight != rc_vis.Right())    {
        m_PixLeft = rc_vp.Left();
        m_PixRight = rc_vp.Right();
        m_ModelLeft = rc_vis.Left();
        m_ModelRight = rc_vis.Right();
    }

    //CGlAttrGuard guard(GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    TVPUnit vp_top_y = rc_vp.Top();
    TVPUnit vp_bottom_y = vp_top_y - (m_BaseHeight - 1) + kAlignSpace;

    // obtain Score Collection and setup iterators
    int range_from = (int) (floor(rc_vis.Left()));
    const CScoreCache::TScoreColl& ScoreColl = m_Cache->GetScores(m_Handle.GetRowNum());
    CScoreCache::TScoreColl::const_iterator it = ScoreColl.begin();
    const CScoreCache::TScoreColl::const_iterator it_end = ScoreColl.end();
    it = ScoreColl.find(it, it_end, range_from);

    if(averageable) {
        x_RenderAveragableScores(pane, it_seg, it, it_end, vp_top_y, vp_bottom_y);
    } else {
        x_RenderNonAveragableScores(pane, it_seg, it, it_end, vp_top_y, vp_bottom_y);
    }
}


/// represents a pixel with two scores, one corresponds to a gap interval, another -
/// to an aligned segment.
struct SScPix2   {
    float m_IntScore;
    float m_IntLen;
    float m_GapScore;
    float m_GapLen;
    float m_MinScore;
    bool  m_Init;

    bool    operator==(const SScPix2& other) const
    {
        return m_IntScore == other.m_IntScore  &&  m_IntLen == other.m_IntLen &&
                m_GapScore == other.m_GapScore  &&  m_GapLen == other.m_GapLen &&
                m_MinScore == other.m_MinScore;
    }
};

class FSetPix2
{
public:
    FSetPix2(bool sequence_level) :
        m_SequenceLevel(sequence_level)
    {
    }

    /// accumulate weighted scores
    void operator()(SScPix2& dst, const SScPix2& src, float fraction, bool long_seg)
    {
        if (src.m_IntScore == IColumnScoringMethod::kAmbiguousResidueScore
            || dst.m_IntScore == IColumnScoringMethod::kAmbiguousResidueScore) { 
            dst.m_IntScore = src.m_IntScore;
            dst.m_IntLen = src.m_IntLen;
        } else {
            dst.m_IntScore += src.m_IntScore * fraction;
            dst.m_IntLen += src.m_IntLen * fraction;
        }
        dst.m_GapScore += src.m_GapScore * fraction;
        dst.m_GapLen += src.m_GapLen * fraction;
        
        if( ! long_seg)  {
            // only for small segments
            static const float eps = 0.00001f;
            if( ! dst.m_Init)   {
                if(src.m_IntLen > eps && src.m_IntScore != IColumnScoringMethod::kAmbiguousResidueScore)  {
                    dst.m_MinScore = src.m_IntScore;
                    dst.m_Init = true;
                } else if(src.m_GapLen > eps)  {
                    dst.m_MinScore = src.m_GapScore;
                    dst.m_Init = true;
                }
            } else {
                if(src.m_IntLen > eps && src.m_IntScore != IColumnScoringMethod::kAmbiguousResidueScore)  {
                    dst.m_MinScore = min(dst.m_MinScore, src.m_IntScore);
                }
                if(src.m_GapLen > eps)  {
                    dst.m_MinScore = min(dst.m_MinScore, src.m_GapScore);
                }
            }
        }
    }
protected:
    bool m_SequenceLevel = false;
};




void CAlnVecRow::x_RenderAveragableScores(CGlPane& pane, IAlnSegmentIterator& it_seg,
                                             CScoreCache::TScoreColl::const_iterator it,
                                             const CScoreCache::TScoreColl::const_iterator it_end,
                                             int vp_top_y, int vp_bottom_y)
{
    IRender& gl = GetGl();

    const TVPRect& rc_vp = pane.GetViewport();
    const TModelRect& rc_vis = pane.GetVisibleRect();

    int row_start = m_Handle.GetSeqAlnStart();
    int row_stop = m_Handle.GetSeqAlnStop();

    static const float eps = 0.001f;

    typedef CRasterizer<SScPix2>    TR;
    TR rasterizer(rc_vp.Left(), rc_vp.Right(), rc_vis.Left(), rc_vis.Right());
    CRange<float> value_limits(numeric_limits<float>::max(), numeric_limits<float>::min());
    FSetPix2 f_sum(pane.GetScaleX() <= 1.f/8.f);

    while(it_seg  &&  it != it_end  &&  it->GetFrom() <= rc_vis.Right()) {
        // get the intersection of the score and align intervals
        const IAlnSegment& seg = *it_seg;
        const TSignedSeqRange& r = seg.GetAlnRange();
        int from = std::max<int>(r.GetFrom(), it->GetFrom());
        int to = std::min<int>(r.GetTo(), it->GetTo());

        /// apply attributes to the intersection
        SScPix2 pix = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false };
        bool empty_seg = (from < row_start  ||  to > row_stop);
        if(seg.IsAligned())  {
            pix.m_IntLen = 1.0;
            pix.m_IntScore = it->GetAttr();
            if (pix.m_IntScore != IColumnScoringMethod::kAmbiguousResidueScore) {
                if (pix.m_IntScore < value_limits.GetFrom())
                    value_limits.SetFrom(pix.m_IntScore);
                else if (pix.m_IntScore > value_limits.GetTo())
                    value_limits.SetTo(pix.m_IntScore);
            }
            rasterizer.AddInterval(from, to + 1, pix, f_sum);
        } else if (!empty_seg && ((to - from) + 1) > 0) {
            pix.m_GapLen = 1.;
            pix.m_GapScore = 0;//it->GetAttr();
            pix.m_IntLen = 0;
            rasterizer.AddInterval(from, to + 1, pix, f_sum);
        } 

        /// advance iterators
        if(to == r.GetTo()) 
            ++it_seg;
        if(to == (int) it->GetTo())  
            ++it;
    }

    // Rendering
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    const TR::TRaster& raster = rasterizer.GetRaster();

    int shift_x = rc_vp.Left();
    float value_scale = value_limits.GetTo() - value_limits.GetFrom();
    if (value_scale <= 0)
        value_scale = 1;
    TVPUnit y_c = floor((float(vp_top_y + vp_bottom_y ) / 2) + 0.5);

    for( size_t i = 0;  i < raster.size(); )    {
        auto x1 = i + shift_x;
        const SScPix2& sc_pix = raster[i];
        while (i < raster.size()  &&  raster[i] == sc_pix) {
            ++i;
        }
        auto x2 = i + shift_x;
        if (sc_pix.m_IntLen > eps) {
            double sc = sc_pix.m_IntScore / sc_pix.m_IntLen;
            CRgbaColor color;
            if (sc == IColumnScoringMethod::kAmbiguousResidueScore) {
                static const CRgbaColor kAmbiguousResidueColor("darkgrey");
                color = kAmbiguousResidueColor;
            } else {
                color = m_Cache->GetColorForScore(sc, IScoringMethod::fBackground);
                //double alpha = 0.25 + sc_pix.m_IntLen * 0.75;
                //gl.Color4f(color.GetRed(), color.GetGreen(), color.GetBlue(), alpha);
                if (sc_pix.m_Init) {
                    sc = min(1.0, max<double>(0., ((sc - value_limits.GetFrom()) / value_scale)));
                    CRgbaColor color2 = m_Cache->GetColorForScore(sc_pix.m_MinScore, IScoringMethod::fBackground);
                    float alpha = 1.0f - sc * 0.75f;
                    color = CRgbaColor::Interpolate(color2, color, alpha);
                }
            } 
            gl.ColorC(color);
            gl.Rectd(x1, vp_top_y, x2, vp_bottom_y);
        } else if (sc_pix.m_GapLen > eps) {
            //float sc = sc_pix.m_GapScore / sc_pix.m_GapLen;
            const CRgbaColor& color =
                m_Cache->GetColorForScore(1., IScoringMethod::fBackground);
            //double alpha = 0.25 + sc_pix.m_GapLen * 0.75;
            //gl.Color4f(color.GetRed(), color.GetGreen(), color.GetBlue(), alpha);
            gl.ColorC(color);
            gl.Rectd(x1, y_c - 1, x2, y_c + 1);
        } else {
            // use  previously set color to paint the remaining area
            // Fix for MSA-381. This was an attempt to fix one pixel gap,
            // which instead removed flanking gaps altogether. I'm keeping
            // it here in case pixel gap problem arises.
            //gl.Rectd(x1, vp_top_y, x2, vp_bottom_y + 1);
        }
    }
}


void CAlnVecRow::x_RenderNonAveragableScores(CGlPane& pane, IAlnSegmentIterator& it_seg,
                                             CScoreCache::TScoreColl::const_iterator it,
                                             const CScoreCache::TScoreColl::const_iterator it_end,
                                             int vp_top_y, int vp_bottom_y)
{
    IRender& gl = GetGl();

    const TVPRect& rc_vp = pane.GetViewport();
    const TModelRect& rc_vis = pane.GetVisibleRect();

    //LOG_POST("CAlnVecRow::x_RenderNonAveragableScores()  "  << rc_vis.Left() << " " << rc_vis.Right());
    int row_start = m_Handle.GetSeqAlnStart();
    int row_stop = m_Handle.GetSeqAlnStop();

    typedef CRasterizer<SScPix>    TR;
    TR rasterizer(rc_vp.Left(), rc_vp.Right(), rc_vis.Left(), rc_vis.Right());
    SScPix pix = { 0.0f, 0.0f, false };

    while(it_seg  &&  it != it_end  &&  it->GetFrom() <= rc_vis.Right()) {
        // get the intersection of the score and align intervals
        const IAlnSegment& seg = *it_seg;
        const TSignedSeqRange& r = seg.GetAlnRange();
        int from = std::max<int>(r.GetFrom(), it->GetFrom());
        int to = std::min<int>(r.GetTo(), it->GetTo());

        /// apply attributes to the intersection
        bool empty_seg = (from < row_start  ||  to > row_stop);
        if(! empty_seg) {
            pix.m_Score = it->GetAttr();
            pix.m_Gap = ! seg.IsAligned();

            rasterizer.AddInterval(from, to + 1, pix, FSetPixMaxCoverage);
        }

        /// advance iterators
        if(to == r.GetTo()) {
            ++it_seg;
        }
        if(to == (int) it->GetTo())   {
            ++it;
        }
    }

    // Rendering
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    const TR::TRaster& raster = rasterizer.GetRaster();

    TVPUnit y_c = floor((float(vp_top_y + vp_bottom_y ) / 2) + 0.5);
    static float eps = 0.0001f;

    int shift_x = rc_vp.Left();
    for( size_t i = 0;  i < raster.size(); )    {
        size_t start = i;
        const SScPix& sc_pix = raster[i];
        while(++i < raster.size()  &&  raster[i] == sc_pix) ;

        if(sc_pix.m_MaxCoverage > eps)    {
            const CRgbaColor& color =
                m_Cache->GetColorForScore(sc_pix.m_Score, IScoringMethod::fBackground);
            gl.ColorC(color);

            if (sc_pix.m_Gap) {
                gl.Rectd(start + shift_x, y_c - 1 , i + shift_x, y_c + 1);
            } else {
                gl.Rectd(start + shift_x, vp_top_y, i + shift_x, vp_bottom_y + 1);
            }
            
            //LOG_POST("Start " << start << ", end " << i << ", color " << color.ToString()
            //         << ",  c_pix.m_MaxCoverage " << sc_pix.m_MaxCoverage
            //         << ", sc_pix.m_Score " << sc_pix.m_Score);
        }
    }
}

#define SEQ_OFF_VERT 0
#define SEQ_OFF_HORZ 0


/// Checks if sequence can be drawn.

bool CAlnVecRow::x_CanRenderSeq(CGlPane& pane)
{
    if(m_Style)    {
        double min_w = 1.0 / m_Style->GetSeqFont().GetMetric(CGlTextureFont::eMetric_MaxCharWidth);
        double scale_x = pane.GetScaleX();
        return min_w >= scale_x;
    } else return false;
}


bool IfDash(char ch) {   return ch == '-';  }


// renders a row of seq symbols in the range [Start, Stop]
void CAlnVecRow::x_RenderSequence(CGlPane& pane, TModelUnit top_y, TModelUnit bottom_y,
                                  int start, int stop)
{
    _ASSERT(m_Style);
    auto_ptr<IAlnSegmentIterator> p_it(
        m_Handle.CreateSegmentIterator(TSignedSeqRange(start, stop), IAlnSegmentIterator::eSkipGaps));
    if (!*p_it)
        return;

    IRender& gl = GetGl();
    auto base_width = m_Handle.GetBaseWidth();
    //auto base_width = m_Handle.UsesAATranslation() && m_Handle.IsNucProtAlignment() ? 3 : 1;
    const CGlTextureFont& font = m_Style->GetSeqFont();
    TModelUnit H = font.TextHeight();
    
    TModelUnit sc_x = pane.GetScaleX();
    TModelUnit off_y = ((abs(bottom_y) - abs(top_y) + 1) - H);
    off_y = floor(off_y/2. + 0.5);
    
    // Get half-character offset in world coords 
    // (use 'A' as stand in for any capital character)
    TModelUnit off_x = (font.TextWidth("A") * sc_x) * 0.5;

    //TModelUnit y = bottom_y + H;
    TModelUnit y = top_y - off_y;
    bool render_scores = false;
    if (m_Cache  &&  m_Cache->HasScores()) {
        IScoringMethod* method = m_Cache->GetScoringMethod();
        render_scores = method &&
            (method->GetSupportedColorTypes() & IScoringMethod::fForeground);
    }
    if (!render_scores)
        gl.ColorC(m_Style->GetColor(CWidgetDisplayStyle::eSequence));

    pane.EnableOffset(false);
    if (!m_SequenceRenderingCtx.get()) {
        m_SequenceRenderingCtx.reset(new CRenderingContext());
    }
    m_SequenceRenderingCtx->PrepareContext(pane, true, m_Handle.IsNegativeStrand());
    pane.EnableOffset(true);

    for (IAlnSegmentIterator& seg_it(*p_it); seg_it; ++seg_it) {
        const IAlnSegment& seg = *seg_it;
        if (!seg.IsAligned())
            continue;
        auto& aln_r = seg.GetAlnRange();
        start = aln_r.GetFrom();
        stop = aln_r.GetTo();
        if (start > stop)
            continue;
        auto& seq_r = seg.GetRange();
        auto sStart = seq_r.GetFrom();
        auto sStop = seq_r.GetTo();
        TSeqPos frame_shift_f = 0;
        TSeqPos frame_shift_t = 0;
        if (base_width == 3) {
            frame_shift_f = sStart % base_width;
            frame_shift_t = 2 - sStop % base_width;
        }
        string seq;
        m_Handle.GetAlnSeqString(seq, TSignedRange(start - frame_shift_f, stop + frame_shift_t));
        // replace gap symbols with spaces
        //replace_if(seq.begin(), seq.end(), IfDash, ' ');
        
        //TModelUnit x = (start * base_width) + 0.5 - pane.GetOffsetX();
        TModelUnit x = start - pane.GetOffsetX() + 0.5;
        if (base_width == 3) {
            x += 1;
            x -= frame_shift_f;
        }

        vector<CRgbaColor*> colors;
        if (render_scores) {
            /// need to draw colored sequence on colored background
            const CScoreCache::TScoreColl& ScoreColl = m_Cache->GetScores(m_Handle.GetRowNum());
            CScoreCache::TScoreColl::const_iterator it = ScoreColl.begin();
            const CScoreCache::TScoreColl::const_iterator it_end = ScoreColl.end();
            it = ScoreColl.find(it, it_end, start);

            int len = stop - start + 1;
            _ASSERT(len >= 0);
            if (len > 0) {
                colors.resize(len); // create vector of the required length

                for (; it != it_end; ++it) {
                    TSeqPos from = max(it->GetFrom(), (TSeqPos)start);
                    TSeqPos to = min(it->GetTo(), (TSeqPos)stop);
                    if (to >= from) {
                        IScoringMethod::TScore score = it->GetAttr();
                        CRgbaColor& cl = const_cast<CRgbaColor&>(
                            m_Cache->GetColorForScore(score, IScoringMethod::fForeground));
                        std::fill_n(colors.begin() + (from - start), to - from + 1, &cl);
                    }
                }
            }
        }

        // Check if there is a consensus sequence and, if so, get the sequence chars
        string consensus;
        bool has_consensus = false;
        if (m_ConsensusRow != NULL && m_ConsensusRow->IsValid() && !m_Style->GetShowIdenticalBases())  {
            m_ConsensusRow->GetAlnSeqString(consensus, TSignedRange(start - frame_shift_f, stop + frame_shift_t));

            if ((m_ConsensusRow->GetHandle().GetRowNum() != m_Handle.GetRowNum()) && 
                (consensus.length() == seq.length())) {
                has_consensus = true;
            }
        }

        bool has_colors = (colors.size() == seq.length());
        
        TModelUnit radius = (std::abs(top_y - bottom_y) / 2.0) + 0.5;
        string text_char(" ");
        TModelUnit center_y = (top_y + bottom_y) * 0.5 + 0.5;
        CRgbaColor default_color = m_Style->GetColor(CWidgetDisplayStyle::eSequence);

        double base_h = double(base_width)/2.;
        // Iterate over the characters drawing glyphs when consensus is matched
        // (except for the consensus sequence itself).
        TModelUnit posx;
        CRgbaColor current_color;

        for (size_t i = 0; i < seq.size(); ++i) {
            posx = x;
            current_color = has_colors ? *colors[i] : default_color;
            if (has_consensus && seq[i] != 'X' && consensus[i] == seq[i]) {
                bool render_dot = m_SequenceRenderingCtx->GetScale() <= 1./16.; // sequence level
                if (render_dot) {
                    auto dot_texture = m_Style->GetDotTexture();
                    render_dot = dot_texture.NotEmpty();
                    if (render_dot) {
                        CGlAttrGuard AttrGuard(GL_LINE_BIT);
                        gl.Enable(GL_TEXTURE_2D);
                        //                    gl.Enable(GL_BLEND);
                        //                    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        
                        double w = dot_texture->GetImage()->GetWidth();
                        dot_texture->MakeCurrent();
                        TModelRect rc_coord(0.0, 0., 1.0, 1.0);
                        w *=  0.5;          
                        auto r = min(0.5, m_SequenceRenderingCtx->ScreenToSeq(w ));
                        TModelRect rc_rbn(x - r,  bottom_y, x + r, top_y);     
                        m_SequenceRenderingCtx->DrawTexture(rc_rbn, rc_coord);
                        //                    gl.Disable(GL_BLEND);
                        gl.Disable(GL_TEXTURE_2D);
                    } 
                }

                if (!render_dot)  {
                    current_color.Lighten(0.65f);
                    gl.ColorC(current_color);
                    gl.LineWidth(2.0);
                    gl.Begin(GL_LINES);
                    gl.Vertex2d(x - sc_x, center_y);
                    gl.Vertex2d(x + sc_x, center_y);
                    gl.End();
                }
            }
            else {
                gl.ColorC(current_color);
                text_char[0] = seq[i];    
                gl.BeginText(&font, current_color);
                gl.WriteText(x - off_x, y, text_char.c_str());
                gl.EndText();
            }
            x += base_width;
        }
    }
}
static const int kTextOffsetY = 0;
static const int kTextOffsetX = 2;
static const int kPosOffsetX = 4;


void CAlnVecRow::x_RenderDescr()
{
    _ASSERT(m_Style);
    x_RenderTextCell(m_Text, IGlFont::eAlign_Left);

    IRender& gl = GetGl();
    if (gl.IsPrinterFriendly())  {   
        bool selected = (m_RenderCtx.m_State & fItemSelected) != 0;
        const auto& rc_vp = m_RenderCtx.m_Viewport;
        string s = "(";
        s += (m_Handle.IsNegativeStrand()) ? "-" : "+";
        s += ")";
        CGlPaneGuard GUARD(*m_RenderCtx.m_Pane, CGlPane::ePixels);
        TModelUnit x = rc_vp.Left() + kTextOffsetX * 2;
        TModelUnit y = rc_vp.Top() - m_BaseHeight;
        TModelUnit w = rc_vp.Width() - kTextOffsetX * 4;
        gl.BeginText(&(m_Style->GetTextFont()), m_Style->GetColor(selected ? CWidgetDisplayStyle::eSelectedText : CWidgetDisplayStyle::eText));
        gl.WriteText(x, y, w, m_BaseHeight, s.c_str(), IGlFont::eAlign_Right | IGlFont::eAlign_VCenter);
        gl.EndText();
    }
}

void CAlnVecRow::x_RenderTaxLabel()
{
    _ASSERT(m_Style);
    x_RenderTextCell(x_GetTaxLabel(), IGlFont::eAlign_Left);
}


bool CAlnVecRow::x_InitTrackPane(const CGlPane& row_pane, CGlPane& track_pane) {

    track_pane.EnableOffset(true);
    int base_height = 0;
    int shown = m_Style->GetShownElements();
    if (shown & CRowDisplayStyle::fShown_Alignment) {
        base_height = m_BaseHeight;
    }

    TVPRect rc_vp = row_pane.GetViewport();
    auto height = abs(rc_vp.Height()) - 1;
    int row_top = m_RenderCtx.m_Top;
    auto offset_top = (rc_vp.Top() - row_top);
    auto top = rc_vp.Top();
    if (offset_top >= 0) {
        top -= (offset_top + base_height);
        offset_top = 0;
    }
    TModelUnit model_height = 0;
    if (top < 0) { // Track pane is not visible
        top = 0;
    } else {
        _ASSERT(offset_top <= 0);
        model_height = m_TrackPanel->GetHeight() + offset_top; 
    }
//    auto bottom = top - model_height;
//    if (offset_top != 0) 
//        bottom -= base_height;
//    double offset_bottom = 0;
    TModelUnit bottom = max<int>(0, m_RenderCtx.m_Viewport.Bottom());
    //if (bottom < bottom_line) {
    //    offset_bottom = -bottom;
    //    bottom = bottom_line;
    // }
    _ASSERT(top >=0);
    _ASSERT(bottom >=0);
    rc_vp.SetVert(bottom, top);
    _ASSERT(rc_vp.Height() >= 0);
    track_pane.SetViewport(rc_vp);
    
    TModelRect rc_vis = row_pane.GetVisibleRect();
    _ASSERT(offset_top <= 0);
    double model_top = abs(offset_top);
    double model_bottom = model_top + rc_vp.Height();
//    double model_bottom = model_top + model_height;
    rc_vis.SetVert(model_bottom, model_top);
    track_pane.SetVisibleRect(rc_vis);

    TModelRect rc_limits = row_pane.GetModelLimitsRect();
    rc_limits.SetVert(m_TrackPanel->GetHeight(), 0);
    track_pane.SetModelLimitsRect(rc_limits);
    return model_height > 0;
}


void CAlnVecRow::x_RenderTracks(CGlPane&, const TSignedRange& range)
{
    _ASSERT(m_TrackPanel && IsExpanded());
    if (!IsExpanded())
        return;
    CGlPane& pane = *m_RenderCtx.m_Pane;
    CGlPane saved_pane(pane);
    if (!x_InitTrackPane(pane, *m_TrackPane))
        return;
    SRenderingContextGuard ctx_guard([&]() {
        m_RenderingCtx->SetRenderingFlags(0);
        m_RenderingCtx->PrepareContext(*m_TrackPane, true, m_Handle.IsNegativeStrand());
    });


    auto orig_height = m_TrackPanel->GetHeight();

    TVPRect rc_vp = m_RenderCtx.m_Viewport;
    //rc_vp.SetTop(rc_vp.Top() - m_BaseHeight);

    //rc_vp.SetTop(rc_vp.Top() - m_BaseHeight);
    TVPRect rc_clip_gr = rc_vp;
    const TVPRect* rc_clip = pane.GetClipRect();
    if (rc_clip) 
        rc_clip_gr.IntersectWith(*rc_clip);
    m_TrackPane->SetClipRect(&rc_clip_gr);

    const TModelRect& rc_vis = m_TrackPane->GetVisibleRect();
    // prepare context for data update
    TModelUnit seq_from = m_Handle.GetSeqPosFromAlnPos(rc_vis.Left(), IAlnExplorer::eLeft);
    TModelUnit seq_to = m_Handle.GetSeqPosFromAlnPos(rc_vis.Right(), IAlnExplorer::eRight);
    s_PrepareTrackPane(rc_vis.Left(), rc_vis.Right(), seq_from, seq_to,
                       m_Handle, pane, *m_TrackPane);

    TModelRect rc_model = m_TrackPane->GetModelLimitsRect();
    rc_model.SetHorz(m_Handle.GetSeqStart(), m_Handle.GetSeqStop());
    m_TrackPane->SetModelLimitsRect(rc_model);

    m_RenderingCtx->PrepareContext(*m_TrackPane, true, m_Handle.IsNegativeStrand());
    if (m_NeedUpdateContent) {
        m_TrackPanel->Update(false);
        m_NeedUpdateContent = false;
    } else if (m_NeedUpdateLayout) {
        m_TrackPanel->Update(true);
    }
    if (m_NeedUpdateSelection) {
        CSelectionVisitor* sel_visitor = m_Host->ARH_GetSelectionVisitor();
        sel_visitor->UpdateSelection(&*m_TrackPanel);
    }
    int max_height = 0;
    m_RenderingCtx->SetRenderingFlags(0);
    if (!m_Handle.IsAnchor()) {

        // First draw alignment segments areas 
        // skipping title bars and icon controls
        if (!GetGl().IsPrinterFriendly()) {
            m_RenderingCtx->SetRenderingFlags(CRenderingContext::fSkipTitleBar | CRenderingContext::fSkipControls | CRenderingContext::fSkipFrames);
        }
        else {
            // Don't render grid when exporting image
            m_RenderingCtx->SetRenderingFlags(CRenderingContext::fSkipTitleBar | CRenderingContext::fSkipControls | CRenderingContext::fSkipFrames | CRenderingContext::fSkipGrid);
        }
        CGlPane segments_pane(*m_TrackPane);
        x_ForEachAlnSeg(range, pane, segments_pane, [&](CGlPane& track_pane) {
            m_RenderingCtx->PrepareContext(track_pane, true, m_Handle.IsNegativeStrand());
            m_TrackPanel->Update(true);
            max_height = max<int>(max_height, m_TrackPanel->GetHeight());
            m_TrackPanel->Draw();
            return false;
        });
        m_RenderingCtx->PrepareContext(*m_TrackPane, true, m_Handle.IsNegativeStrand());
        // Draw title bars and icon controls 
        // skipping grid and features 
        m_RenderingCtx->SetRenderingFlags(CRenderingContext::fSkipContent | CRenderingContext::fSkipGrid);
    } 
    m_TrackPanel->Update(true);
    //m_TrackPanel->SetHeight(orig_height);

    //x_InitTrackPane(saved_pane, *m_TrackPane);
    //m_RenderingCtx->PrepareContext(*m_TrackPane, true, m_Handle.IsNegativeStrand());
    m_TrackPanel->Draw();
    //m_RenderingCtx->SetRenderingFlags(0);
//    cout << "TP height after:" << m_TrackPanel->GetHeight() << ", before:" << orig_height << ", Viewport H:" << m_TrackPane->GetViewport().Height() << endl;

    //m_TrackPanel->SetHeight(orig_height);
    //{
      //  auto vr = m_TrackPane->GetVisibleRect();
//        vr.SetVert(orig_height, 0);
  //      m_TrackPane->SetVisibleRect(vr);
//    }
    m_TrackHandler->Render(*m_TrackPane);
    m_TrackPane->SetClipRect(nullptr);
}

void CAlnVecRow::x_RenderGraph(const TSignedRange& range)
{
    if(IsExpanded()) 
        x_RenderTracks(*m_RenderCtx.m_Pane, range);
}


const static int kButtonOffset = 2;
const static int kButtonSize = 11;
const static int kButtonSignSize = 7;


// Returns buttons bounds in screen coordinates, "pane" parameter specifies
// CGlPane corresponding to "Icons" column
TVPRect CAlnVecRow::x_GetButtonRect(EButtons btn, int shift)
{
    const auto& rc_vp = m_RenderCtx.m_Viewport;
    // when shown flag is set not to show the alignments
    // m_BaseHeight can be less than kButtonSize
    // in this cases we still want to create a valid rectangle 
    // to provide information for this row in htmlareas 
    auto h = (m_BaseHeight > kButtonSize) ? (m_BaseHeight - kButtonSize) / 2 : 0;
        
    int btn_space = kButtonSize + kButtonOffset;
    int x = rc_vp.Left() + kButtonOffset + ((int)btn * btn_space * shift);
    int y_top = rc_vp.Top() - h;

    TVPRect rc(x, y_top - kButtonSize + 1, x + kButtonSize - 1, y_top);
    //TVPRect rc(x, y_top - (kButtonSize - 1), x + (kButtonSize - 1), y_top);
    return rc;
}


CAlnVecRow::EButtons
    CAlnVecRow::x_HitTestButton(CGlPane& pane, const TVPPoint& vp_pt)
{
    const TVPRect& rc_vp = pane.GetViewport();
    //int y_top = rc_vp.Top() - (m_BaseHeight - kButtonSize) / 2;

    //if(vp_pt.Y() <= y_top  &&  vp_pt.Y() >= y_top - kButtonSize + 1)    { // vertical hit
        int left = rc_vp.Left() + kButtonOffset;
        for( int i = 0; i < 3; i++ )    {
            if(vp_pt.X() >= left  &&  vp_pt.X() < left + kButtonSize) {
                return (EButtons) i;
            }
            left += kButtonSize + kButtonOffset;
        }
    //}
    return eInvalid;
}


void CAlnVecRow::x_RenderIcons()
{
    _ASSERT(m_Style);

    IRender& gl = GetGl();
    // If ExactOrthoProjection is set, the expand (+/-) box 
    // occasionally rendered incorrectly depending on the window size 
    // Its height fluctuates...
    // The workaround is to disable ExactOrthoProjection
    m_RenderCtx.m_Pane->SetExactOrthoProjection(false);
    CGlPaneGuard GUARD(*m_RenderCtx.m_Pane, CGlPane::ePixels);
    m_RenderCtx.m_Pane->SetExactOrthoProjection(true);

    const auto& rc_vp = m_RenderCtx.m_Viewport;

    bool wid_focused = (m_RenderCtx.m_State & fWidgetFocused) != 0;
    bool selected = (m_RenderCtx.m_State & fItemSelected) != 0;
    if (selected) {         //draw background
        gl.ColorC(m_Style->GetColor(wid_focused ? CWidgetDisplayStyle::eFocusedBack
            : CWidgetDisplayStyle::eSelectedBack));
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gl.Rectd(rc_vp.Left(), rc_vp.Bottom(), rc_vp.Right(), rc_vp.Top());
    }

    x_RenderEmptyBox();

    // draw strand marker
    TVPRect rc_btn = x_GetButtonRect(eStrandMarker);
    int y_c = rc_btn.CenterPoint().Y();
    int half = rc_btn.Height() / 2;
    int x1 = rc_btn.Left();
    int x2 = rc_btn.Right();

    if(m_Handle.IsNegativeStrand()) {
        swap(x1, x2);
    }


    gl.Disable(GL_LINE_SMOOTH);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Color3d(0.0, 0.5, 0.0);
    gl.Begin(GL_TRIANGLES); // render triangle
        gl.Vertex2d(x1, y_c - half);
        gl.Vertex2d(x2, y_c);
        gl.Vertex2d(x1, y_c + half);
    gl.End();

    // draw "Expand" button
    if (IsExpandable()) {
        gl.LineWidth(1.0f);

        rc_btn = x_GetButtonRect(eExpand);

        gl.ColorC(m_Style->GetColor(selected ? CWidgetDisplayStyle::eSelectedText
            : CWidgetDisplayStyle::eText));

        gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        gl.Rectd(rc_btn.Left(), rc_btn.Bottom(), rc_btn.Right(), rc_btn.Top());
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        int offset = (kButtonSize - kButtonSignSize) / 2;
        TVPPoint pt_c = rc_btn.CenterPoint();
        // render "+" or "-"
        gl.Begin(GL_LINES);
        gl.Vertex2d(rc_btn.Left() + offset, pt_c.Y());
        gl.Vertex2d(rc_btn.Left() + offset + kButtonSignSize, pt_c.Y());
        if (!m_bExpanded) {
            gl.Vertex2d(pt_c.X(), rc_btn.Bottom() + offset);
            gl.Vertex2d(pt_c.X(), rc_btn.Bottom() + offset + kButtonSignSize);
        }
        gl.End();
    }
    // draw focus frame
    /*
    if (wid_focused && (m_RenderCtx.m_State & fItemFocused)) {
        gl.LineWidth(1.0f);

        gl.Begin(GL_LINE_LOOP);
        gl.Vertex2d(rc_vp.Left(), rc_vp.Bottom() + 1);
        gl.Vertex2d(rc_vp.Right() - 1, rc_vp.Bottom() + 1);
        gl.Vertex2d(rc_vp.Right() - 1, rc_vp.Top());
        gl.Vertex2d(rc_vp.Left(), rc_vp.Top());
        gl.End();
    }
    */

    /**
    if(m_GraphCont  &&  m_GraphCont->HasData())   {   // draw "Graph" button
        rc_btn = x_GetButtonRect(pane, eSetupGraphs);
        gl.Rectd(rc_btn.Left(), rc_btn.Bottom(), rc_btn.Right(), rc_btn.Top());

        m_Style->GetTextFont().TextOut(rc_btn.Left() + 1, rc_btn.Bottom() + 1,
                                        kButtonSize - 2, kButtonSize - 2, "...", IGlFont::eAlign_Center);
    }
    **/

//    pane.Close();
}


void CAlnVecRow::x_RenderIconStrand()
{
    _ASSERT(m_Style);

    IRender& gl = GetGl();
    CGlPaneGuard GUARD(*m_RenderCtx.m_Pane, CGlPane::ePixels);

    // draw strand marker
    TVPRect rc_btn = x_GetButtonRect(eExpand);
    int y_c = rc_btn.CenterPoint().Y();
    int half = rc_btn.Height() / 2;
    int x1 = rc_btn.Left();
    int x2 = rc_btn.Right();

    if(m_Handle.IsNegativeStrand()) {
        swap(x1, x2);
    }

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Color3d(0.0, 0.5, 0.0);
    gl.Begin(GL_TRIANGLES); // render triangle
        gl.Vertex2d(x1, y_c - half);
        gl.Vertex2d(x2, y_c);
        gl.Vertex2d(x1, y_c + half);
    gl.End();
}

void CAlnVecRow::x_RenderIconExpand()
{
    _ASSERT(m_Style);
    if (!IsExpandable())
        return;
    IRender& gl = GetGl();

    CGlPaneGuard GUARD(*m_RenderCtx.m_Pane, CGlPane::ePixels);

    // draw "Expand" button
    TVPRect rc_btn = x_GetButtonRect(eStrandMarker);

    gl.ColorC(m_Style->GetColor(CWidgetDisplayStyle::eText));
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    gl.LineWidth(1.0f);     
    gl.Disable(GL_LINE_SMOOTH);

    rc_btn = x_GetButtonRect(eExpand, 0);
    gl.Rectd(rc_btn.Left(), rc_btn.Bottom(), rc_btn.Right(), rc_btn.Top());
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    int offset = (kButtonSize - kButtonSignSize) / 2;
    TVPPoint pt_c = rc_btn.CenterPoint();

    // render "+" or "-"
    gl.Begin(GL_LINES);
    gl.Vertex2d(rc_btn.Left() + offset, pt_c.Y());
    gl.Vertex2d(rc_btn.Left() + offset + kButtonSignSize, pt_c.Y());
    if (!m_bExpanded) {
        gl.Vertex2d(pt_c.X(), rc_btn.Bottom() + offset);
        gl.Vertex2d(pt_c.X(), rc_btn.Bottom() + offset + kButtonSignSize);
    }
    gl.End();

    /**
    if(m_GraphCont  &&  m_GraphCont->HasData())   {   // draw "Graph" button
        rc_btn = x_GetButtonRect(pane, eSetupGraphs);
        gl.Rectd(rc_btn.Left(), rc_btn.Bottom(), rc_btn.Right(), rc_btn.Top());

        m_Style->GetTextFont().TextOut(rc_btn.Left() + 1, rc_btn.Bottom() + 1,
                                        kButtonSize - 2, kButtonSize - 2, "...", IGlFont::eAlign_Center);
    }
    **/
}


void CAlnVecRow::x_RenderStartPos()
{
    const TModelRect& rc_v = m_RenderCtx.m_Pane->GetVisibleRect();
    int aln_pos = (int) floor(rc_v.Left());

    x_RenderAlnPos(aln_pos, IAlnExplorer::eRight);
}


void CAlnVecRow::x_RenderEndPos()
{
    const TModelRect& rc_v = m_RenderCtx.m_Pane->GetVisibleRect();
    int aln_pos = -1 + (int) ceil(rc_v.Right());

    x_RenderAlnPos(aln_pos, IAlnExplorer::eLeft);
}


void CAlnVecRow::x_RenderSeqStart()
{
    TSeqPos pos = m_Handle.IsNegativeStrand() ?
                  m_Handle.GetSeqStop() : m_Handle.GetSeqStart();

    string s_num = CTextUtils::FormatSeparatedNumber(pos + 1);

    x_RenderTextCell(s_num, IGlFont::eAlign_HCenter);
}


void CAlnVecRow::x_RenderSeqEnd()
{
    TSeqPos pos = m_Handle.IsNegativeStrand() ?
                  m_Handle.GetSeqStart() : m_Handle.GetSeqStop();

    string s_num = CTextUtils::FormatSeparatedNumber(pos + 1);

    x_RenderTextCell(s_num, IGlFont::eAlign_HCenter);
}


void CAlnVecRow::x_RenderSeqLength()
{
    TSeqPos pos = 0;
    if (m_Handle.CanGetBioseqHandle()) {
        pos = m_Handle.GetBioseqHandle().GetBioseqLength();
    } else {
        pos = abs(m_Handle.GetSeqStop() - m_Handle.GetSeqStart()) + 1;
    }
    string s_num = CTextUtils::FormatSeparatedNumber(pos);
    x_RenderTextCell(s_num, IGlFont::eAlign_HCenter);
}


void CAlnVecRow::x_GetAlnPosLabel(TSeqPos aln_pos, IAlnExplorer::ESearchDirection dir, string& label) const
{
    const char *s_marker = "";

    int seq_pos = m_Handle.GetSeqPosFromAlnPos(aln_pos);
    if(seq_pos == -1)   {
        seq_pos = m_Handle.GetSeqPosFromAlnPos(aln_pos, dir);
        switch(dir) {
        case IAlnExplorer::eRight:  s_marker = ">>  "; break;
        case IAlnExplorer::eLeft:  s_marker = "<<  "; break;
        default: break;
        }
    }
    label.clear();
    label.append(s_marker);
    label.append(CTextUtils::FormatSeparatedNumber(seq_pos + 1));
}


// generates text label for given position in alignment
void CAlnVecRow::x_RenderAlnPos(TSeqPos aln_pos, IAlnExplorer::ESearchDirection dir)
{
    string label;
    x_GetAlnPosLabel(aln_pos, dir, label);    
    x_RenderTextCell(label, IGlFont::eAlign_HCenter);
}


void CAlnVecRow::x_RenderTextCell(const string& text, IGlFont::TAlign align)
{
    _ASSERT(m_Style);

    IRender& gl = GetGl();
    bool selected = (m_RenderCtx.m_State & fItemSelected) != 0;
    bool wid_focused = (m_RenderCtx.m_State & fWidgetFocused) != 0;
    const auto& rc_vp = m_RenderCtx.m_Viewport;
    
    CGlPaneGuard GUARD(*m_RenderCtx.m_Pane, CGlPane::ePixels);

    if (selected) {         //draw background
        gl.ColorC(m_Style->GetColor(wid_focused ? CWidgetDisplayStyle::eFocusedBack
            : CWidgetDisplayStyle::eSelectedBack));
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gl.Rectd(rc_vp.Left(), rc_vp.Bottom(), rc_vp.Right(), rc_vp.Top());
    }
    x_RenderEmptyBox();
    
    // draw text label
    gl.ColorC(m_Style->GetColor(selected ? CWidgetDisplayStyle::eSelectedText
                                         : CWidgetDisplayStyle::eText));
    TModelUnit x = rc_vp.Left() + kTextOffsetX * 2;
    TModelUnit y = rc_vp.Top() - m_BaseHeight;
    TModelUnit w = rc_vp.Width() - kTextOffsetX * 4;

    gl.BeginText(&(m_Style->GetTextFont()), m_Style->GetColor(selected ? CWidgetDisplayStyle::eSelectedText : CWidgetDisplayStyle::eText));
    gl.WriteText(x, y, w, m_BaseHeight - m_Style->GetTextFont().GetFontDescender(), text.c_str(), align | IGlFont::eAlign_VCenter);
    gl.EndText();
}


void CAlnVecRow::x_RenderEmptyBox()
{
    IRender& gl = GetGl();

    gl.ColorC(m_Style->GetColor(CWidgetDisplayStyle::eFrame));
    const auto& rc_vp = m_RenderCtx.m_Viewport;
    gl.LineWidth(1.0);
    //gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //gl.Rectf((float)rc_vp.Left() - 1, rc_vp.Bottom() - 1, rc_vp.Right(), rc_vp.Top());

    gl.Begin(GL_LINES);
    gl.Vertex2d(rc_vp.Right(), rc_vp.Top());
    gl.Vertex2d(rc_vp.Right(), rc_vp.Bottom());
    gl.Vertex2d(rc_vp.Left(), rc_vp.Bottom());
    gl.Vertex2d(rc_vp.Right(), rc_vp.Bottom());
    gl.End();

}


bool    CAlnVecRow::IsExpandable() const
{
    return m_Handle.CanGetBioseqHandle();
}


bool    CAlnVecRow::IsExpanded() const
{
    return m_bExpanded;
}

void CAlnVecRow::x_CreateTrackPanel(CGlPane& pane)
{

    auto gConfig = m_Style->GetSeqGraphicConfig();
    // Setup rendering context
    const CBioseq_Handle& bsh = m_Handle.GetBioseqHandle();
    CRef<CSGSequenceDS> sequence_ds(new CSGSequenceDS(bsh.GetScope(), *bsh.GetSeqId()));
    m_RenderingCtx.reset(new CRenderingContext);
    m_RenderingCtx->SetSeqDS(&*sequence_ds);
    m_RenderingCtx->SetOverviewCutoff(gConfig->GetOverviewCutoff());

    m_TrackPane.reset(new CGlPane(pane));
    TModelRect rc = m_TrackPane->GetVisibleRect();
    TModelUnit seq_from = m_Handle.GetSeqPosFromAlnPos(rc.Left(), IAlnExplorer::eLeft);
    TModelUnit seq_to = m_Handle.GetSeqPosFromAlnPos(rc.Right(), IAlnExplorer::eRight);
    
//    rc.SetHorz(seq_from, seq_to);
//    m_TrackPane->SetVisibleRect(rc);
    s_PrepareTrackPane(rc.Left(), rc.Right(), seq_from, seq_to, m_Handle, pane, *m_TrackPane);

    rc.SetHorz(m_Handle.GetSeqStart(), m_Handle.GetSeqStop());
    m_TrackPane->SetModelLimitsRect(rc);
    m_RenderingCtx->PrepareContext(*m_TrackPane, true, m_Handle.IsNegativeStrand());

    m_TrackHandler.reset(new CLayoutTrackHandler);
    m_TrackHandler->SetHost(this);
    //m_TrackHandler->SetDraggable(false); // turn of drag&drop
    m_TrackHandler->SetPane(m_TrackPane.get());

    m_TrackPanel.Reset(new CFeaturePanel(m_RenderingCtx.get(), !CSGDataSource::IsBackgroundJob()));
    m_TrackPanel->SetConfig(gConfig);
    m_TrackPanel->SetConfigKey(m_Style->GetTrackConfigKey());
    m_TrackPanel->SetProfile(gConfig->GetTrackProfile());
    if (!m_Style->GetTrackSettings().empty()) {
        m_TrackPanel->SetTrackConfig(m_Style->GetTrackSettings());
    }

    CRef<CSGDataSourceContext> m_ds_ctx(new CSGDataSourceContext);
    m_TrackPanel->SetDSContext(&*m_ds_ctx);

    SConstScopedObject input_obj(bsh.GetSeqId(), &bsh.GetScope());
    m_TrackPanel->SetInputObject(input_obj);
    m_TrackPanel->SetHost(this);
    {
        // set minumum height 
        TModelUnit icon_s = gConfig->GetIconSize();
        m_TrackPanel->SetHeight(icon_s * 1.5);
    }

    x_InitTrackPane(pane, *m_TrackPane);

    if (!CSGDataSource::IsBackgroundJob()) {
        m_TrackPanel->Update(false);
        m_NeedUpdateContent = false;
    }
}

void CAlnVecRow::x_ResetTrackPanel()
{
    _ASSERT(m_TrackPanel);
    try {
        CSelectionVisitor* sel_visitor = m_Host->ARH_GetSelectionVisitor();
        sel_visitor->ResetObjectSelection(&*m_TrackPanel);
        m_TrackPanel->SaveTrackConfig();
        auto gConfig = m_Style->GetSeqGraphicConfig();
        if (gConfig)
            gConfig->GetHistParamsManager()->SaveSettings(gConfig->GetColorTheme(), gConfig->GetSizeLevel());
    } catch (exception& e) {
        ERR_POST(Error << e.what());
    }
    m_TrackHandler.reset(0);
    m_TrackPanel.Reset(0);
    m_TrackPane.reset();
    m_RenderingCtx.reset();
}


void CAlnVecRow::Expand(bool b_exp, CGlPane& pane)
{
    if(IsExpandable()  &&  IsExpanded() != b_exp)   {
        if (b_exp) {
            if (!m_TrackPanel) {
                x_CreateTrackPanel(pane);
                m_TrackPanel->ConfigureTracks();
            }
        } else {
            x_ResetTrackPanel();
        }
        m_bExpanded = b_exp;
        x_UpdateRowHeight();
        if (x_GetHost()) {
            x_GetHost()->ARH_OnRowChanged(this);
        }
    }
}

size_t CAlnVecRow::GetTrackCount(CGlPane& pane)
{
    if (!IsExpandable())
        return 0;
    if (!m_TrackPanel) {
        x_CreateTrackPanel(pane);
        m_TrackPanel->ConfigureTracks();
    }
    return m_TrackPanel->GetGroup().GetChildrenNum();
}

void CAlnVecRow::UseExternalOrganismLookup(bool f)
{
    m_fExtOrgLookup = f;
}


void CAlnVecRow::x_UpdateRowHeight()
{
    m_Height = m_BaseHeight;
    if (m_bExpanded && m_TrackPanel) {
        m_TrackPanel->Update(!m_NeedUpdateContent);
        m_NeedUpdateContent = false;
        m_Height += m_TrackPanel->GetHeight();
    }

}

const IAlnRowGraphProperties*     CAlnVecRow::GetProperties() const
{
    //return m_GraphCont ? m_GraphCont->GetProperties() : NULL;
    return nullptr;
}


void CAlnVecRow::SetProperties(IAlnRowGraphProperties* props)
{
/*
    if(m_GraphCont)   {
        m_GraphCont->SetProperties(props);

        m_Height = m_BaseHeight;
        if(m_GraphCont->IsCreated())  {
            m_Height += m_GraphCont->PreferredSize().Y();
        }
    }
*/
}


TSeqPos CAlnVecRow::GetColumnAsSeqPos(TColumnType type, const TModelRect& rc_vis)
{
    TSeqPos pos = (TSeqPos) -1;
    switch(type)    {
    case eStart:    {
        int aln_pos = (int) floor(rc_vis.Left());
        pos = m_Handle.GetSeqPosFromAlnPos(aln_pos, IAlnExplorer::eRight);
        break;
    }
    case eEnd:  {
        int aln_pos = -1 + (int) ceil(rc_vis.Right());
        pos = m_Handle.GetSeqPosFromAlnPos(aln_pos, IAlnExplorer::eLeft);
        break;
    }
    case eSeqStart:
        pos = m_Handle.IsNegativeStrand() ?
                m_Handle.GetSeqStop() : m_Handle.GetSeqStart();
        break;
    case eSeqEnd:
        pos = m_Handle.IsNegativeStrand() ?
                m_Handle.GetSeqStart() : m_Handle.GetSeqStop();
        break;
    case eSeqLength:
        if (m_Handle.CanGetBioseqHandle()) {
            pos = m_Handle.GetBioseqHandle().GetBioseqLength() - 1;
        } else {
            pos = abs(m_Handle.GetSeqStop() - m_Handle.GetSeqStart()) + 1;
        }
        break;
    default:
        break;
    }
    return pos;
}


string CAlnVecRow::GetColumnAsString(TColumnType type)
{
    switch(type)    {
    case eDescr:
        return m_Text;
    case eTaxLabel:
        return x_GetTaxLabel();
    default:
        return "";
    }
}

string& CAlnVecRow::GetAlnSeqString( string &buffer, const IAlnExplorer::TSignedRange &aln_rng ) const
{
    return m_Handle.GetAlnSeqString( buffer, aln_rng );
}


string& CAlnVecRow::GetStringAtPos(string& buffer, TSignedSeqPos aln_pos) const
{
    if (UsesAATranslation() && IsNucProtAlignment()) {
        auto pos = m_Handle.GetSeqPosFromAlnPos(aln_pos, IAlnExplorer::eRight);
        pos = (pos / 3) * 3;
        return m_Handle.GetSeqString(buffer, IAlnExplorer::TRange(pos, pos + 3));
    } else {
        return GetAlnSeqString(buffer, IAlnExplorer::TSignedRange(aln_pos, aln_pos));
    }

}



void CAlnVecRow::GetRowStatistics(ITooltipFormatter& tooltip) const
{
    TSignedSeqPos alstart = m_Handle.GetSeqAlnStart();
    TSignedSeqPos alstop = m_Handle.GetSeqAlnStop();

    TSignedRange alrange;
    if( alstart < alstop ){
        alrange.Set( alstart, alstop);
    } else {
        alrange.Set( alstop, alstart);
    }

    int aligned = 0;
    int gap = 0;
    int reversed = 0;
    int indel = 0;
    int unaligned = 0;

    IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eAllSegments;
    auto_ptr<IAlnSegmentIterator> p_it(m_Handle.CreateSegmentIterator(alrange, flags));

    TSignedSeqPos prev_seq_to = -2, prev_aln_to = -2;

    for( IAlnSegmentIterator& it(*p_it); it; ++it ){
        const IAlnSegment& seg = *it;
        auto& aln_r = seg.GetAlnRange();
        auto& seq_r = seg.GetRange();

        auto type = seg.GetType();

        if( type & IAlnSegment::fAligned ){
            if (!aln_r.Empty()) {
                aligned += seq_r.GetLength();
                if (aln_r.GetFrom() == prev_aln_to + 1 && seq_r.GetFrom() > prev_seq_to + 1) {
                    unaligned += seq_r.GetFrom() - prev_seq_to;
                }
            } else {
                indel += seq_r.GetLength();
            }
            prev_aln_to = aln_r.GetTo();
            prev_seq_to = seq_r.GetTo();
        }
        if( type & IAlnSegment::fGap ){
            gap += aln_r.GetLength();
        }
        if( type & IAlnSegment::fReversed ){
            reversed += aln_r.GetLength();
        }
        if( type & IAlnSegment::fIndel ){
            indel += aln_r.GetLength();
        }
        if( type & IAlnSegment::fUnaligned ){
            unaligned += aln_r.GetLength();
        }
    }

//    int start = m_Handle.GetSeqStart();
//    int stop = m_Handle.GetSeqStop();

    int start = m_Handle.GetSeqPosFromAlnPos(alstart, IAlnExplorer::eRight);
    int stop = m_Handle.GetSeqPosFromAlnPos(alstop, IAlnExplorer::eLeft);

    bool is_aa = UsesAATranslation();
    int base_width = (is_aa && m_Handle.IsNucProtAlignment()) ? 3 : 1;
    start /= base_width;
    stop /= base_width;

    string aln_str = "Sequence is aligned from ";
    aln_str += (is_aa) ? "residues " : "bases ";
    aln_str += NStr::IntToString(start + 1, NStr::fWithCommas);
    aln_str += " to ";
    aln_str += NStr::IntToString(stop + 1, NStr::fWithCommas);
    tooltip.AddRow(aln_str);

    string bases = is_aa ? "Residues" : "Bases";
    tooltip.AddRow(bases + " aligned:", NStr::IntToString(aligned / base_width, NStr::fWithCommas));
    tooltip.AddRow(bases + " through gaps:", NStr::IntToString(gap / base_width, NStr::fWithCommas));
    tooltip.AddRow(bases + " in indels:", NStr::IntToString(indel / base_width, NStr::fWithCommas));
    tooltip.AddRow(bases + " unaligned:", NStr::IntToString(unaligned / base_width, NStr::fWithCommas));
}

bool CAlnVecRow::UsesAATranslation() const
{
    return m_Handle.UsesAATranslation();
}

bool CAlnVecRow::IsNucProtAlignment() const
{
    return m_Handle.IsNucProtAlignment();
}

void CAlnVecRow::x_OnGraphContentChanged(CEvent* evt)
{
    //LOG_POST("CAlnVecRow::x_OnGraphContentChanged");
    x_UpdateRowHeight();
    if (m_Host) {
        m_Host->ARH_OnRowChanged(this);
    }
}


void CAlnVecRow::x_OnGraphSizeChanged(CEvent* evt)
{
    //LOG_POST("CAlnVecRow::x_OnGraphSizeChanged");
    x_UpdateRowHeight();
    if (m_Host) {
        m_Host->ARH_OnRowChanged(this);
    }
}

CRef<CLayoutTrack> CAlnVecRow::THH_GetLayoutTrack()
{
    CRef<CLayoutTrack> track;
/*
    CWeakRef<CSeqGlyph>::TRefType last_hit_glyph = m_LastHitGlyph.Lock();
    CRef<CLayoutTrack> track(dynamic_cast<CLayoutTrack*>(last_hit_glyph.GetPointer()));
    if (track)
        m_LastHitTrack.Reset(track.GetPointer());
    if (last_hit_glyph) {
        track.Reset(dynamic_cast<CLayoutTrack*>(last_hit_glyph.GetPointer()));
    } else {
        track.Reset(dynamic_cast<CLayoutTrack*>(m_TrackPanel.GetPointer()));
    }
    return track;
    */
    CWeakRef<CLayoutTrack>::TRefType last_hit_track = m_LastHitTrack.Lock();
    track.Reset(last_hit_track.GetPointer());
    return track;

}

TModelPoint CAlnVecRow::THH_GetModelByWindow(const wxPoint& pt)
{
    if (!m_Host)
        return TModelPoint(0, 0);
    if (!m_TrackPanel)
        return TModelPoint(0, 0);
    TVPPoint p = m_Host->ARH_GetVPPosByWindowPos(pt);
    return m_TrackPane->UnProject(p.X(), p.Y());
}

void CAlnVecRow::GHH_Redraw()
{
    IGenericHandlerHost* h = dynamic_cast<IGenericHandlerHost*>(m_Host);
    if (h)
        h->GHH_Redraw();
}

void CAlnVecRow::GHH_SetCursor(const wxCursor &cursor)
{
    IGenericHandlerHost* h = dynamic_cast<IGenericHandlerHost*>(m_Host);
    if (h)
        h->GHH_SetCursor(cursor);
}
void CAlnVecRow::GHH_CaptureMouse()
{
    IGenericHandlerHost* h = dynamic_cast<IGenericHandlerHost*>(m_Host);
    if (h)
        h->GHH_CaptureMouse();
}
void CAlnVecRow::GHH_ReleaseMouse()
{
    IGenericHandlerHost* h = dynamic_cast<IGenericHandlerHost*>(m_Host);
    if (h)
        h->GHH_ReleaseMouse();
}

void CAlnVecRow::LTH_OnLayoutChanged()
{
    if (m_bExpanded && m_TrackPanel) {
        _ASSERT(m_TrackPane);
        int max_height = 0;
        auto&& rc_vis = m_TrackPane->GetVisibleRect();
        auto f = m_ModelLeft;
        auto t = m_ModelRight;
        if (f > t) swap(f, t);
        TSignedRange range(f, t);
        CGlPane track_pane(*m_TrackPane);
        auto vr = m_TrackPane->GetVisibleRect();
        vr.SetHorz(f, t);
        track_pane.SetVisibleRect(vr);
        track_pane.EnableOffset(false);

        auto vp = m_TrackPane->GetViewport();
        vp.SetHorz(m_PixLeft, m_PixRight);
        track_pane.SetViewport(vp);

        if (m_NeedUpdateSelection) {
            CSelectionVisitor* sel_visitor = m_Host->ARH_GetSelectionVisitor();
            sel_visitor->UpdateSelection(&*m_TrackPanel);
        }

        x_ForEachAlnSeg(range, track_pane, track_pane, [&](CGlPane& track_pane) {
            m_RenderingCtx->PrepareContext(track_pane, true, m_Handle.IsNegativeStrand());
            m_TrackPanel->Update(true);
            max_height = max<int>(max_height, m_TrackPanel->GetHeight());
            return false;
        });
        if (max_height > 0) {
            m_RenderingCtx->PrepareContext(*m_TrackPane, true, m_Handle.IsNegativeStrand());
            m_TrackPanel->SetHeight(max_height);
            //auto vr = m_TrackPane->GetVisibleRect();
            //vr.SetVert(max_height, 0);
            //m_TrackPane->SetVisibleRect(vr);
        }
    }
    x_UpdateRowHeight();
    if (m_Host) {
        m_Host->ARH_OnRowChanged(this);
    }
    //if (m_TrackPanel && m_TrackPanel->AllJobsFinished())
//        m_TrackPanel->SaveTrackConfig();
}

void CAlnVecRow::LTH_PopupMenu(wxMenu* menu)
{
    CGlWidgetPane* widget = dynamic_cast<CGlWidgetPane*>(m_Host);
    if (widget) {
        widget->SetPopupMenuDisplayed(true);
        widget->PopupMenu(menu);
        widget->SetPopupMenuDisplayed(false);
    }
}


void CAlnVecRow::LTH_PushEventHandler(wxEvtHandler* handler)
{
    CGlWidgetPane* widget = dynamic_cast<CGlWidgetPane*>(m_Host);
    if (widget) {
        widget->PushEventHandler(handler);
    }
}


void CAlnVecRow::LTH_PopEventHandler()
{
    CGlWidgetPane* widget = dynamic_cast<CGlWidgetPane*>(m_Host);
    if (widget) {
        widget->PopEventHandler();
    }
}

void CAlnVecRow::LTH_ConfigureTracksDlg(const string& category)
{
    ConfigureTracksDialog(category);
}

void CAlnVecRow::LoadDefaultTracks()
{
    if (m_bExpanded && m_TrackPanel) {

        m_TrackHandler.reset(new CLayoutTrackHandler);
        m_TrackHandler->SetHost(this);
        //m_TrackHandler->SetDraggable(false); // turn of drag&drop
        m_TrackHandler->SetPane(m_TrackPane.get());

        m_TrackPanel->LoadDefaultTracks();
    }
}

void CAlnVecRow::ConfigureTracksDialog(const string& category)
{
    if (!(m_bExpanded && m_TrackPanel))
        return;
    m_TrackPanel->ShowConfigureTracksDlg(category);

}

void CAlnVecRow::ConfigureTracks(bool reset)
{
    if (!(m_bExpanded && m_TrackPanel))
        return;
    if (reset) {
        string default_profile = CSGConfigUtils::DefTrackProfile();
        m_TrackPanel->SetProfile(default_profile);
        CTrackConfigManager::TTrackProxies empty_settings;
        m_TrackPanel->SetTrackConfig(empty_settings);
        auto gConfig = m_Style->GetSeqGraphicConfig();
        gConfig->SetTrackProfile(default_profile);
        gConfig->SetDirty(true);
    }
    m_TrackPanel->ConfigureTracks();

}   

void CAlnVecRow::SaveTrackConfig()
{
    if (!(m_bExpanded && m_TrackPanel))
        return;
    m_TrackPanel->SaveTrackConfig();

}


void CAlnVecRow::PrepareRendering(CGlPane& pane, TVPUnit row_top, int state) 
{
    m_RenderCtx.m_Pane = &pane;
    m_RenderCtx.m_Viewport = pane.GetViewport();
    m_RenderCtx.m_Top = row_top;
    auto offset = (m_RenderCtx.m_Viewport.Top() - m_RenderCtx.m_Top);
    auto top = m_RenderCtx.m_Viewport.Top() - offset;
    auto bottom = top - (m_Height - 1);
    m_RenderCtx.m_Viewport.SetVert(bottom, top);
    m_RenderCtx.m_State = state;
}
END_NCBI_SCOPE
