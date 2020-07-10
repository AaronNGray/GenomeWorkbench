/*  $Id: layout_track_impl.cpp 45024 2020-05-09 02:03:16Z evgeniev $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/layout_track_impl.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/track_container_track.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/utils.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE

static const int kIndent = 14;      /// in pixel
static const int kInvalidIcon = -1;

CLayoutTrack::TTexMap  CLayoutTrack::sm_IconMap;
CFastMutex CLayoutTrack::sm_Mutex;

CLayoutTrack::CLayoutTrack(CRenderingContext* r_cntx)
    : m_Order(-1)
    , m_On(true)
    , m_ShowFrame(false)
    , m_Expanded(true)
    , m_HighlightedIcon(kInvalidIcon)
    , m_Attrs(fDefaultAttr)
    , m_LTHost(NULL)
    , m_Title(kEmptyStr)
    , m_Msg(kEmptyStr)
    , m_Progress(100)
    , m_IndentLevel(0)   // 0 means first level track
    , m_ParentTrack(NULL)
{
    SetRenderingContext(r_cntx);
    x_RegisterIcon(SIconInfo(eIcon_Close, "Close", true, "track_close"));
    x_RegisterIcon(SIconInfo(eIcon_Expand, "Collapse/Expand", true,
        "track_expand", "track_collapse"));
    // x_Validate("Constructor");
}


CLayoutTrack::~CLayoutTrack()
{
    // x_Validate("Destructor");
}



void CLayoutTrack::LoadProfile(const string& profile_str)
{
    // cerr << "<<<< profile_str: " << profile_str << endl;
    // x_Validate("LoadProfile in");
    SetProfile(profile_str);
    TKeyValuePairs settings;
    CSGConfigUtils::ParseProfileString(profile_str, settings);
    string profile = kEmptyStr;
    if (settings.empty()) {
        profile = profile_str;
    } else {
        TKeyValuePairs::iterator iter = settings.find("profile");
        if (iter != settings.end()) {
            profile = iter->second;
            settings.erase(iter);
        }
//         iter = settings.find("comments");
//         if (iter != settings.end()) {
//             SetComments(iter->second);
//             settings.erase(iter);
//         }
    }
    x_LoadSettings(profile, settings);
    // cerr << ">>>>" << endl;
    // x_Validate("LoadProfile out");
}


void CLayoutTrack::SaveProfile()
{
    // x_Validate("SaveProfile in");
    TKeyValuePairs settings;
    CSGConfigUtils::ParseProfileString(GetProfile(), settings);
    string preset_style = kEmptyStr;
    if (settings.empty()) {
        preset_style = GetProfile();
    } else {
        TKeyValuePairs::iterator iter = settings.find("profile");
        if (iter != settings.end()) {
            preset_style = iter->second;
        }
    }
    x_SaveSettings(preset_style);
    // x_Validate("SaveProfile out");
}


void CLayoutTrack::SetTrackAttr(TTrackAttrFlags flags)
{
    // x_Validate("SetTrackAttr in");
    m_Attrs = flags;
    if ( ! (m_Attrs & fCollapsible) ) {
        x_DeregisterIcon(eIcon_Close);
        x_DeregisterIcon(eIcon_Expand);
    }
    // x_Validate("SetTrackAttr out");
}

void CLayoutTrack::CloseMe()
{
    // x_Validate("CloseMe in");
    SetShow(false);
    ITrackContainer* parent = dynamic_cast<ITrackContainer*>(GetParentTrack());
    if (parent) {
        parent->CloseTrack(GetOrder());
    }
    // x_Validate("CloseMe out");
}


void CLayoutTrack::ShowMe()
{
    // x_Validate("ShowMe in");
    SetShow(true);
    x_Expand(true);
    // x_Validate("ShowMe out");
}


bool CLayoutTrack::OnLeftDown(const TModelPoint& p)
{
    // x_Validate("OnLeftDown inm");
    bool consumed = false;
    // do we have title bar at all
    if (m_Attrs & fFrameVisible  &&  m_ShowFrame) {
        IRender& gl = GetGl();

        TModelPoint pp(p);
        x_World2Local(pp);
        int idx = x_HitIconTest(pp);
        if (idx >= 0  &&  m_Icons[idx].m_Enabled) {
            x_OnIconClicked(m_Icons[idx].m_Id);
            consumed = true;
        } else if (m_gConfig->GetShowTrackMinusPlusSign()) {
            TModelRect rcm;
            x_GetTBRect(rcm);
            CRef<CGlTextureFont> com_font = x_GetGlobalConfig()->GetCommentFont();
            TModelUnit half_size = gl.TextWidth(com_font, "+");
            if (m_IndentLevel > 1) {
                half_size *= 1.5;
            }
            half_size = m_Context->ScreenToSeq(half_size);

            TModelUnit center_x = rcm.Left() + half_size;
            m_Context->AdjustToOrientation(center_x);
            rcm.SetLeft(center_x - half_size);
            rcm.SetRight(center_x + half_size);
            if (PointInRect(pp, rcm)) {
                x_Expand(!m_Expanded);
                consumed = true;
            }
        }
    }
    // x_Validate("OnLeftDown out");
    return consumed;
}


bool CLayoutTrack::OnLeftDblClick(const TModelPoint& p)
{
    // x_Validate("OnLeftDblClick in");
    bool consumed = false;
    // do we have title bar at all
    if (m_Attrs & fFrameVisible  &&  m_ShowFrame) {
        TModelPoint pp(p);
        x_World2Local(pp);
        if (x_HitTitleBar(pp)  &&  m_Attrs & fCollapsible) {
            x_Expand(!m_Expanded);
            consumed = true;
        }
    }
    // x_Validate("OnLeftDblClick out");
    return consumed;
}

/*
bool CLayoutTrack::// x_Validate(const string& sCodeLocation) const
{
    if(this == NULL) {
        LOG_POST(Error << sCodeLocation << ": CLayoutTrack is NULL");
        return true;
    }
    if((int)m_On > 1 || (int)m_ShowFrame > 1 || (int)m_Expanded > 1) {
        LOG_POST(Error << sCodeLocation << ": Broken CLayoutTrack: " << (void*)this);
        return false;
    }
    LOG_POST(Error << sCodeLocation << ": Valid CLayoutTrack: " << (void*)this << 
             ", id: " << m_Id << ", title: " << m_Title << ", def-title: " << m_DefaultTitle << ", msg: " << m_Msg <<
             ", m_On: " << (int)m_On << ", ShowFrame: " << (int)m_ShowFrame << ", Expanded: " << (int)m_Expanded << 
             ", validating its parent: " << (void*)m_ParentTrack.GetPointer());
    if(m_ParentTrack.NotNull())
        m_ParentTrack->// x_Validate("Parent of " + sCodeLocation);
    return true; 
}
*/
bool CLayoutTrack::isRmtBased() const
{
    // x_Validate("isRmtBased");
    const CLayoutTrack* pLayoutTrack(GetParentTrack());
    const CTrackContainer* pContainer(dynamic_cast<const CTrackContainer*>(pLayoutTrack));
    if(pContainer == NULL) {
        return false;
    }
    const CTrackContainer::TTrackProxies& proxies(pContainer->GetSubtrackProxies());

     ITERATE(CTrackContainer::TTrackProxies, iter, proxies) {
         if((*iter)->GetTrack() == this) {
             CTempTrackProxy::TAnnots annots((*iter)->GetAnnots());
             for(auto annot: annots) {
                 if(CSeqUtils::isRmtAnnotName(annot)) {
                     return true;
                 }
             }
             if (!(*iter)->GetRemotePath().empty())
                return true;
         }
     }
     return false;
}

bool CLayoutTrack::isRemotePath() const
{
    // x_Validate("isRemotePath");
    const CTrackContainer* pContainer(dynamic_cast<const CTrackContainer*>(GetParentTrack()));
    if(pContainer == nullptr)
        return false;

    const CTrackContainer::TTrackProxies& proxies(pContainer->GetSubtrackProxies());

    for (const auto& proxy : proxies) {
        if(proxy->GetTrack() != this)
            continue;

        return !(proxy->GetRemotePath().empty());
    }
    return false;
}

CHTMLActiveArea* CLayoutTrack::InitHTMLActiveArea(TAreaVector* p_areas) const
{
    // x_Validate("InitHTMLActiveArea");
    TModelUnit tb_height = x_GetTBHeight();

    CHTMLActiveArea area;
    CSeqGlyph::x_InitHTMLActiveArea(area);
    area.m_Flags =
        CHTMLActiveArea::fNoSelection |
        CHTMLActiveArea::fNoPin |
        CHTMLActiveArea::fNoHighlight |
        CHTMLActiveArea::fTrack;
    if (!(m_Attrs & fNavigable))
        area.m_Flags |= CHTMLActiveArea::fNoNavigation;
    area.m_Bounds.SetBottom(area.m_Bounds.Top() +
        (TVPUnit)tb_height + (tb_height > 0.0 ? 2 : 0));
    area.m_TrackErrorStatus = m_ErrorStatus;
    if(!x_IsNCBITrack() && !m_Title.empty()) {
        area.m_Descr = m_Title + (m_isBlast ? "" : " (Non-NCBI data)");
    }
    p_areas->push_back(area);
    return &p_areas->back();
}


void CLayoutTrack::MouseOut()
{
    // do we have title bar at all
    if (m_Attrs & fFrameVisible  &&  m_ShowFrame) {
        m_ShowFrame = false;
        m_HighlightedIcon = kInvalidIcon;
    }
}


bool CLayoutTrack::OnMotion(const TModelPoint& pt)
{
    // x_Validate("OnMotion in");
    TModelPoint p(pt);
    x_World2Local(p);
    int pre_highlighted = m_HighlightedIcon;
    bool was_shown = m_ShowFrame;
    m_ShowFrame = false;
    m_HighlightedIcon = kInvalidIcon;
    if (m_Attrs & fFrameVisible  &&  x_HitTitleBar(p)) {
        m_HighlightedIcon = x_HitIconTest(p);
        m_ShowFrame = true;
    }
    // x_Validate("OnMotion out");
    return m_HighlightedIcon != pre_highlighted  ||
        was_shown != m_ShowFrame;
}


bool CLayoutTrack::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    // x_Validate("NeedTooltip");
    GetTooltip(p, tt, t_title);
    return !tt.IsEmpty();
}


void CLayoutTrack::GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& /*t_title*/) const
{
    if (m_HighlightedIcon != kInvalidIcon) {
        tt.AddRow(m_Icons[m_HighlightedIcon].m_Tooltip);
    }
    // x_Validate("GetTooltip");
}


void CLayoutTrack::x_Draw() const
{
    // x_Validate("x_Draw in");
    if (m_On  &&  x_Visible_V()) {
        IRender& gl = GetGl();

        gl.PushMatrix();
        gl.Translatef(0.0f, GetTop(), 0.0f);
        if (m_Expanded) {
            bool skip_content = m_Context->IsSkipContent() && dynamic_cast<const ITrackContainer*>(this) == 0;
            if (!skip_content)
                x_RenderContent();
        }
        if (x_ShowTitle() && !m_Context->IsSkipTitleBar()) {
            x_RenderTitleBar();
        }
        gl.PopMatrix();
    }
    // x_Validate("x_Draw out");
}


void CLayoutTrack::x_UpdateBoundingBox()
{
    // x_Validate("x_UpdateBoundingBox in");
    SetHeight(0.0);
    SetWidth(0.0);
    if (!m_On) {
        return;
    }
    SetHeight(GetHeight() + x_GetTBHeight());

    if (m_Attrs & fFullTrack) {
        const TModelRange& vr = m_Context->GetVisibleRange();
        SetLeft(vr.GetFrom());
        SetWidth(vr.GetLength() - 1.0);
    }
    // x_Validate("x_UpdateBoundingBox out");
}


void CLayoutTrack::x_Expand(bool expand)
{
    // x_Validate("x_Expand in");
    if (m_Attrs & fCollapsible) {
        m_Expanded = expand;
        if (m_Expanded) {
            Update(false);
        }
        x_OnLayoutChanged();
    }
    // x_Validate("x_Expand out");
}


bool CLayoutTrack::x_ShowTitle() const
{
    // x_Validate("x_ShowTitle");
//cerr << "CLayoutTrack::x_ShowTitle(): " << ((x_GetGlobalConfig()->GetShowComments() ||
  //           x_GetGlobalConfig()->ReserveCmmtSpace())  &&
    //         m_Attrs & fShowTitle &&
      //  ( (m_Attrs & fShowAlways  &&  !m_gConfig->AutoHideTrack())  ||
        //  !x_Empty())) << endl;

    return (x_GetGlobalConfig()->GetShowComments() ||
             x_GetGlobalConfig()->ReserveCmmtSpace())  &&
             m_Attrs & fShowTitle &&
        ( (m_Attrs & fShowAlways  &&  !m_gConfig->AutoHideTrack())  ||
          !x_Empty());
}


TModelUnit CLayoutTrack::x_GetTBHeight() const
{
    // x_Validate("x_GetTBHeight in");
    IRender& gl = GetGl();

    if (m_On  &&  x_ShowTitle()) {
        CConstRef<CSeqGraphicConfig> conf = x_GetGlobalConfig();
        CRef<CGlTextureFont> com_font = conf->GetCommentFont();
        return gl.TextHeight(com_font) +
            conf->GetCommentBottomMargin() + conf->GetCommentTopMargin();
    }
    // x_Validate("x_GetTBHeight out");
    return 0.0;
}


void CLayoutTrack::x_OnLayoutChanged()
{
    // x_Validate("x_OnLayoutChanged in");
    CSeqGlyph::x_OnLayoutChanged();
    if (m_LTHost  &&  x_GetGlobalConfig()->GetBackgroundLoading()) {
        m_LTHost->LTH_OnLayoutChanged();
    }
    // x_Validate("x_OnLayoutChanged out");
}


void CLayoutTrack::x_LoadSettings(const string& /*preset_style*/,
                                  const TKeyValuePairs& /*settings*/)
{
    // x_Validate("x_LoadSettings");
}


void CLayoutTrack::x_SaveSettings(const string& /*preset_style*/)
{
    // x_Validate("x_SaveSettings");
}


void CLayoutTrack::x_GetTBRect(TModelRect& rect) const
{
    // x_Validate("x_GetTBRect in");
    bool compact = m_gConfig->GetSizeLevel() == "Compact";

    TModelUnit indent = compact ? 0 : m_IndentLevel * m_Context->ScreenToSeq(kIndent);
    TModelUnit h = x_GetTBHeight();
    rect.Init(GetLeft() + indent, h, GetRight(),
        m_gConfig->GetCommentTopMargin());
    // x_Validate("x_GetTBRect out");
}


void CLayoutTrack::x_RenderProgressBar() const
{
    // x_Validate("x_RenderProgressBar in");
    IRender& gl = GetGl();

    TModelRect rect;
    x_GetTBRect(rect);
    TModelUnit len = m_Progress * 0.01 * rect.Width() * 0.5;
    TModelUnit off_x = rect.Left() + len;
    m_Context->AdjustToOrientation(off_x);
    rect.SetLeft(-len);
    rect.SetRight(len);
    rect.Offset(off_x, 0.0);
    gl.Color4f(0.6f, 0.6f, 0.9f, 0.4f);
    m_Context->DrawQuad(rect);
    // x_Validate("x_RenderProgressBar out");
}


void CLayoutTrack::x_RenderTitleBar() const
{
    // x_Validate("x_RenderTitleBar in");
    IRender& gl = GetGl();

    CConstRef<CSeqGraphicConfig> config = x_GetGlobalConfig();

    // render track frame
    TModelRect rcm;
    x_GetTBRect(rcm);
    if (m_Attrs & fFrameVisible) {
        TModelUnit left = rcm.Left();
        TModelUnit right = rcm.Right();
        m_Context->AdjustToOrientation(left);
        m_Context->AdjustToOrientation(right);
        TModelRect rcm_adj(left, rcm.Bottom(), right, rcm.Top());
        if (m_ShowFrame && !gl.IsPrinterFriendly()) {
            CRgbaColor dark_c = CRgbaColor(0.1f, 0.15f, 0.5f, 0.8f);
            CRgbaColor light_c = CRgbaColor(0.77f, 0.85f, 1.0f, 0.8f);

            // draw the box surrounding the track content if not empty
            if ( !x_Empty() ) {
                TModelUnit offset = m_Context->ScreenToSeq(1.0);
                TModelRect content_rc(GetLeft() + offset,
                    GetHeight(), GetRight() - offset, rcm.Bottom());
                gl.ColorC(dark_c);
                m_Context->DrawRect(content_rc);
            }
            // draw the shaded title bar

            m_Context->DrawShadedQuad(rcm_adj, dark_c, light_c, false);

            // draw the icons on right side of the title bar
            int icon_num = m_Icons.size();
            for (int idx = 0;  idx < icon_num;  ++idx) {
                x_RenderIcon(m_Icons[idx].m_Id, m_HighlightedIcon == idx);
            }
        } else if (config->GetShowCommentBg()) {

            if (gl.IsPrinterFriendly()) {
                const CGlPane* pane = m_Context->GetGlPane();
                _ASSERT(pane);
                TVPRect viewport = pane->GetViewport();
                gl.BeginClippingRect(viewport.Left(), viewport.Top(), viewport.Width(), viewport.Height());
            }
                        
            if (x_IsNCBITrack()) {
                m_Context->DrawShadedQuad(rcm_adj, config->GetTitleBackground(), CRgbaColor(1.0f, 1.0f, 1.0f, 0.0));
            }
            else {
                m_Context->DrawShadedQuad(rcm_adj, config->GetNonNCBITitleBackground(), CRgbaColor(1.0f, 1.0f, 1.0f, 0.0));
            }

            if (gl.IsPrinterFriendly()) {
                gl.EndClippingRect();
            }
        }
    }

    // render title
    if (config->GetShowComments()) {
        CRef<CGlTextureFont> com_font = config->GetCommentFont();
        TModelUnit fs =  gl.TextHeight(com_font);

        TModelUnit xM = rcm.Left();
        string title = "";
        if (m_gConfig->GetShowTrackMinusPlusSign()) {
            bool compact = m_gConfig->GetSizeLevel() == "Compact";
            if (compact || m_IndentLevel < 2) {
                title = m_Expanded ? " - " : " + ";
            } else {
                title = m_Expanded ? " -- " : " ++ ";
            }
        }
        title += GetFullTitle();
        if (m_gConfig->GetShowMessage()) {
            title += m_Msg;
        }
        title += x_GetHistMsg();

        if (m_Progress > 0  &&  m_Progress < 100) {
            title += ": " + NStr::Int8ToString(m_Progress) + "%";
            x_RenderProgressBar();
        }

        if (m_ShowFrame && !gl.IsPrinterFriendly()) {
            gl.Color3f(1.0f, 1.0f, 1.0f);
        } else {
            gl.ColorC(config->GetFGCommentColor());
        }

        TModelUnit repeat_dist = config->GetCommentRepeatDist();
        TModelUnit title_len = gl.TextWidth(com_font, title.c_str()) + 20.0;
        repeat_dist = max(repeat_dist,  title_len);
        repeat_dist = m_Context->ScreenToSeq(repeat_dist);
        int repeat_num = 1;
        if (config->GetRepeatComment()) {
            repeat_num = (int)ceil(fabs(rcm.Width() / repeat_dist));
        } else {
            // truncate the title if it is too long
            TModelUnit max_title_w = max(50.0, m_Context->SeqToScreen(fabs(rcm.Width())) - 100.0);
            title = com_font->Truncate(title, max_title_w);
        }

        TModelUnit yM = rcm.Top() + fs + 1.0;

        for (int i = 0; i < repeat_num; ++i) {
            TModelUnit x = xM;
            m_Context->AdjustToOrientation(x);
            m_Context->TextOut(com_font, title.c_str(), x, yM, false, false);
            xM += repeat_dist;
        }
    }
    // x_Validate("x_RenderTitleBar out");
}


void CLayoutTrack::x_RenderIcon(TIconID id,
                                bool highlighted,
                                bool lite_version) const
{
    // x_Validate("x_RenderIcon in");
    IRender& gl = GetGl();

    int idx = x_GetIconIdxById(id);
    TModelRect rc_btn = x_GetIconRect(idx);
    double coord = m_gConfig->GetIconSize() / 16.0;
    TModelRect rc_coord(0.0, 1.0, coord, 1.0 - coord);
    if (highlighted) {
        //gl.Color3f(0.4f, 0.4f, 0.9f);
        //m_Context->DrawQuad(rc_btn);
        TModelUnit offset = m_Context->GetOffset();
        gl.Begin(GL_LINES);
        gl.Color3f(1.0f, 1.0f, 1.0f);
        gl.Vertex2d(rc_btn.Left() - offset, rc_btn.Top());
        gl.Vertex2d(rc_btn.Right() - offset, rc_btn.Top());
        gl.Vertex2d(rc_btn.Left() - offset, rc_btn.Top());
        gl.Vertex2d(rc_btn.Left() - offset, rc_btn.Bottom());
        gl.Color3f(0.1f, 0.1f, 0.1f);
        gl.Vertex2d(rc_btn.Left() - offset, rc_btn.Bottom());
        gl.Vertex2d(rc_btn.Right() - offset, rc_btn.Bottom());
        gl.Vertex2d(rc_btn.Right() - offset, rc_btn.Top());
        gl.Vertex2d(rc_btn.Right() - offset, rc_btn.Bottom());
        gl.End();
        gl.Color4f(0.6f, 0.6f, 1.0f, 0.6f);
        m_Context->DrawQuad(rc_btn);
    }

    CFastMutexGuard LOCK(sm_Mutex);
    CIRef<I3DTexture> tex;
    if (x_GetIconState(id) == 0) {
        if (lite_version  &&  !m_Icons[idx].m_Icon1_lite.empty()) {
            tex = sm_IconMap[m_Icons[idx].m_Icon1_lite];
        } else {
            tex = sm_IconMap[m_Icons[idx].m_Icon1];
        }
    } else {
        tex = sm_IconMap[m_Icons[idx].m_Icon2];
    }

    if (tex) {
        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.Enable(GL_TEXTURE_2D);
        tex->MakeCurrent();
        m_Context->DrawTexture(rc_btn, rc_coord);
        gl.Disable(GL_TEXTURE_2D);
    }
    // x_Validate("x_RenderIcon out");
}


void CLayoutTrack::x_OnIconClicked(TIconID id)
{
    // x_Validate("x_OnIconClicked in");
    switch (id) {
        case eIcon_Close:
            CloseMe();
            break;
        case eIcon_Expand:
            x_Expand(!m_Expanded);
            break;
        default:
            _ASSERT(false);
            // derived class must hanve handled other cases.
            break;
    }
    // x_Validate("x_OnIconClicked out");
}


int CLayoutTrack::x_GetIconState(TIconID id) const
{
    // x_Validate("x_GetIconState");
    if (id == eIcon_Expand  &&  !m_Expanded) {
        return 1;
    }
    return 0;
}


void CLayoutTrack::x_RegisterIcon(const SIconInfo& icon)
{
    TIcons::iterator iter = m_Icons.begin();
    while (iter != m_Icons.end()  &&  iter->m_Id != icon.m_Id) {
        ++iter;
    }
    if (iter == m_Icons.end()) {
        m_Icons.push_back(icon);
    }
    // x_Validate("x_RegisterIcon");
}


void CLayoutTrack::x_DeregisterIcon(TIconID id)
{
    TIcons::iterator iter = m_Icons.begin();
    while (iter != m_Icons.end()  &&  iter->m_Id != id) {
        ++iter;
    }
    if (iter != m_Icons.end()) {
        m_Icons.erase(iter);
    }
    // x_Validate("x_DeregisterIcon");
}


void CLayoutTrack::x_EnableIcon(TIconID id, bool enabled)
{
    TIcons::iterator iter = m_Icons.begin();
    while (iter != m_Icons.end()  &&  iter->m_Id != id) {
        ++iter;
    }
    if (iter != m_Icons.end()) {
        iter->m_Enabled = enabled;
    }
    // x_Validate("x_EnableIcon");
}


void CLayoutTrack::x_ShowIcon(TIconID id, bool shown)
{
    TIcons::iterator iter = m_Icons.begin();
    while (iter != m_Icons.end()  &&  iter->m_Id != id) {
        ++iter;
    }
    if (iter != m_Icons.end()) {
        iter->m_Shown = shown;
    }
    // x_Validate("x_ShowIcon");
}


int CLayoutTrack::x_HitIconTest(const TModelPoint& p) const
{
    // x_Validate("x_HitIconTest");
    int icon_num = m_Icons.size();
    for (int idx = 0;  idx < icon_num;  ++idx) {
        if (PointInRect(p, x_GetIconRect(idx))) {
            return idx;
        }
    }
    return kInvalidIcon;
}


bool CLayoutTrack::x_HitTitleBar(const TModelPoint& p) const
{
    // x_Validate("x_HitTitleBar in");
    // No need to check title if there is no title bar
    // however, feature panel has icons bar 
    // and hit test is suppposed to detect that

    if (!x_ShowTitle() && !(m_Attrs & fShowIcons))
        return false;
    if (m_Context->IsSkipTitleBar() || m_Context->IsSkipControls())
        return false;
    TModelRect rc;
    x_GetTBRect(rc);
    TModelUnit left = rc.Left();
    TModelUnit right = rc.Right();
    m_Context->AdjustToOrientation(left);
    m_Context->AdjustToOrientation(right);
    rc.SetLeft(left);
    rc.SetRight(right);
    // x_Validate("x_HitTitleBar out");
    return PointInRect(p, rc);
}


void CLayoutTrack::x_RenderFrame() const
{
    // x_Validate("x_RenderFrame");

}


TModelRect CLayoutTrack::x_GetIconRect(int idx) const
{
    // x_Validate("x_GetIconRect in");
    TModelRect rcm;
    x_GetTBRect(rcm);
    rcm.SetTop(rcm.Top() - m_gConfig->GetCommentTopMargin());
    int size_y = m_gConfig->GetIconSize() / 2;
    TModelUnit size_x = m_Context->ScreenToSeq(size_y);
    TModelPoint off_seq;
    off_seq.m_Y = floor(rcm.CenterPoint().Y() + 0.5);
    size_y = min<int>(size_y, rcm.Bottom() - off_seq.m_Y);
    off_seq.m_X = rcm.Right() - (idx * 3.0 + 2.0) * size_x;
    m_Context->AdjustToOrientation(off_seq.m_X);
    rcm.Init(-size_x, size_y, size_x, -size_y);
    m_Context->AdjustToOrientation(rcm);
    const CGlPane* pane = m_Context->GetGlPane();
    TVPPoint off_screen = pane->Project(off_seq.X(), off_seq.Y());
    off_seq = pane->UnProject(off_screen.X(), off_screen.Y());
    rcm.Offset(off_seq.X(), off_seq.Y());
    // x_Validate("x_GetIconRect out");
    return rcm;
}


int CLayoutTrack::x_GetIconIdxById(TIconID id) const
{
    TIcons::const_iterator iter = m_Icons.begin();
    int idx = 0;
    while (iter != m_Icons.end()  &&  iter->m_Id != id) {
        ++iter;
        ++idx;
    }
    // x_Validate("x_GetIconIdxById");
    if (iter != m_Icons.end()) {
        return idx;
    }
    return kInvalidIcon;
}


void CLayoutTrack::RegisterCommonIconImages()
{
    RegisterIconImage("track_close", "track_close.png");
    RegisterIconImage("track_expand", "track_collapse.png");
    RegisterIconImage("track_collapse", "track_expand.png");
    RegisterIconImage("track_content", "track_content.png");
    RegisterIconImage("track_layout", "track_layout.png");
    RegisterIconImage("track_settings", "track_settings.png");
    RegisterIconImage("track_help", "track_help.png");

    // lite versions
    RegisterIconImage("track_content_lite", "track_content_lite.png");
    RegisterIconImage("track_layout_lite", "track_layout_lite.png");
    RegisterIconImage("track_settings_lite", "track_settings_lite.png");
    RegisterIconImage("track_help_lite", "track_help_lite.png");
}


void CLayoutTrack::RegisterIconImage(const TIconAlias& key,
                                     const string& img_file)
{
    CFastMutexGuard LOCK(sm_Mutex);
    // make sure not identical keys
    _ASSERT(sm_IconMap.count(key) == 0);
    CIRef<I3DTexture> tex;
    wxString path = CSysPath::ResolvePath(wxT("<res>"), wxString::FromAscii(img_file.c_str()));
    try {
        // we are creating CGlTexture instance here, but not actual
        // OpenGL texture object. CGlTexture::Load() will take care
        // of this later on inside OpenGL rendering context.
        CNcbiIfstream istr(path.fn_str(), ios::in|ios::binary);
        tex.Reset(CGlResMgr::Instance().CreateTexture(istr, img_file));
        tex->SetFilterMag(GL_NEAREST);
        tex->SetFilterMin(GL_NEAREST);
        tex->SetWrapS(GL_CLAMP);
        tex->SetWrapT(GL_CLAMP);
        tex->SetTexEnv(GL_REPLACE);
    } catch (CException&) {
        tex.Reset(NULL);
        LOG_POST(Error << "CLayoutTrack::s_RegisterIconImage: " <<
                          "cannot read image from file "  << path.ToUTF8());
    }
    if (tex) {
        sm_IconMap[key] = tex;
    }
}


bool CLayoutTrack::InitIconTextures()
{
    CFastMutexGuard LOCK(sm_Mutex);

    bool need_reload = false;
    ITERATE (TTexMap, iter, sm_IconMap) {
        if ( !iter->second ) return false;

        if ( !iter->second->IsValid() ) {
            need_reload = true;
            break;
        }
    }
    if (need_reload) {
        NON_CONST_ITERATE (TTexMap, iter, sm_IconMap) {
            iter->second->Load();
        }
    }

    return true;
}


void ILayoutTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const vector<string>& target_annots,
    const string& annot_type,
    const string& track_type,
    TAnnotNameTitleMap& out_annots)
{
    if (target_annots.empty()  &&  annot_type.empty()  && track_type.empty()) {
        return;
    }

    string track_type_l = track_type;
    NStr::ToLower(track_type_l);
    if (target_annots.empty()) {
        // search annots based on annot_type and/or track_type
        ITERATE (TAnnotMetaDataList, iter, src_annots) {
            if (nullptr == iter->second)
                continue;
            bool matched = !annot_type.empty();
            if (matched  &&
                !NStr::EqualNocase(annot_type, iter->second->m_AnnotType)) {
                matched = false;
            }
            if (matched  &&  !track_type_l.empty()  &&
                iter->second->m_Subtypes.count(track_type_l) == 0) {
                    matched = false;
            }

            if (matched) {//  &&  (*iter)->m_Requested) {
                out_annots.insert(TAnnotNameTitleMap::value_type(
                    iter->second->m_Name, iter->second->m_Title));
            }
        }
    } else {
        set<string> annots;
        ITERATE (vector<string>, iter, target_annots) {
            if (CSeqUtils::IsNAA(*iter)) {
                annots.insert(*iter);
            }
        }
        ITERATE (set<string>, iter, annots) {
            TAnnotMetaDataList::const_iterator s_iter = src_annots.find(*iter);
            if (s_iter == src_annots.end()) continue;

            if (!annot_type.empty()  &&  !s_iter->second->m_AnnotType.empty()  &&
                !NStr::EqualNocase(annot_type, s_iter->second->m_AnnotType)) {
                    continue;
            }
            if (!track_type_l.empty() && !s_iter->second->m_Subtypes.empty() &&
                s_iter->second->m_Subtypes.count(track_type_l) == 0) {
                    continue;
            }
            out_annots.insert(TAnnotNameTitleMap::value_type(
                s_iter->second->m_Name, s_iter->second->m_Title));
        }
    }
}


static const string kFilterSeparator = ":FILTER:";

string ILayoutTrackFactory::MergeAnnotAndFilter(const string& annot,
                                                const string& filter)
{
    if (filter.empty()) {
        return annot;
    }

    return annot + kFilterSeparator + filter;
}


string ILayoutTrackFactory::ExtractAnnotName(const string& source)
{
    size_t pos = source.find(kFilterSeparator);
    if (pos != string::npos) {
        return source.substr(0, pos);
    }
    return source;
}


string ILayoutTrackFactory::ExtractFilterName(const string& source)
{
    size_t pos = source.find(kFilterSeparator);
    if (pos != string::npos) {
        return source.substr(pos + kFilterSeparator.length());
    }
    return kEmptyStr;
}


END_NCBI_SCOPE
