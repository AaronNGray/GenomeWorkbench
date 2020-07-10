#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_IMPL__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_IMPL__HPP

/* $Id: layout_track_impl.hpp 42438 2019-02-26 15:46:24Z shkeda $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbistr.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/gui_export.h>
#include <gui/opengl/gltypes.hpp>
#include <gui/widgets/gl/html_active_area.hpp>

#include <gui/widgets/seq_graphic/layout_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/utils/track_info.hpp>

#include <gui/opengl/i3dtexture.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CLayoutTrack - the abstract base class for various tracks.
/// interfaces and the basic common functionalities. The common functionalities
/// include:
///   - handling user interactions such as collapsing/expanding, closing,
///   - generating layout according to applied layout policy,
///   - managing the life cycle of layout objects and layouts,
///
/// The main goal for the default implementation of layout track is to minimize
/// the work in the derived classes such that the sub classes only need to load
/// the data, create the layout objects, and render the layout objects.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CLayoutTrack :
    public CSeqGlyph
{
public:
    typedef vector< CConstRef<CSeqGlyph> > TLayObjVector;
    typedef unsigned TTrackAttrFlags;
    typedef list< CRef<CLayoutTrack> >  TLayoutTracks;
    typedef map<string, string>         TKeyValuePairs;

    enum FTrackAttr {
        fMovable        = 1 << 0,
        fCollapsible    = 1 << 1, ///< can be collpased and expanded.
        fFrameVisible   = 1 << 2, ///< track frame will be rendered when triggered.
        fShowTitle      = 1 << 3,
        fShowAlways     = 1 << 4, ///< always be rendered even for an empty track.
        fFullTrack      = 1 << 5, ///< track occupying the full horizontal screen.
        fNavigable      = 1 << 6, ///< Track has navigation controls
        fShowIcons      = 1 << 7, ///< Track has control icons at the bottom
        fDefaultAttr = fMovable | fCollapsible | fFrameVisible |
                          fShowTitle | fFullTrack | fShowAlways
    };

    CLayoutTrack(CRenderingContext* r_cntx);

    virtual ~CLayoutTrack();

    virtual const CTrackTypeInfo&  GetTypeInfo() const = 0;

    /// set track profile and load settings.
    /// It is up to a track to interprate the content of the profile.
    /// For example, a profile can be the name of a preset
    /// settings style for a track.  A profile can also be
    /// a list of settings, such as "a_color:red,b_size:10",
    /// or the conbination of both.  Example of profile setting:
    /// 1) TrackProfile = Default, the Default is a preset style name
    /// 2) TrackProfile = a_color:red,b_size:10
    /// 3) TrackProfile = preset:Default,a_color:red,b_size:10
    /// Note: if both preset style and concrete settings present in
    /// the profile string, do use 'preset' key before the preset style
    /// name.
    void LoadProfile(const string& profile_str);
    void SaveProfile();
    void SetProfile(const string& preset_style);
    const string& GetProfile() const;

    /// set comments.
    virtual void SetComments(const string& /*comment_str*/) {};
    virtual void SetHighlights(const string& /*hl_str*/) {};

    /// get a more meaningful title.
    /// This allows a concrete track to compose a more meaningful
    /// title in a more complex way.
    virtual string GetFullTitle() const;

    /// get the track title.
    string GetTitle() const;

    const TTrackErrorStatus& GetErrorStatus() const;
    void SetErrorStatus(const TTrackErrorStatus& ErrorStatus);
    bool isInErrorStatus() const;

    // true if the track is based on a remote file derived  annotation
    bool isRmtBased() const;

    bool isRemotePath() const;

    /// @name track-specific methods
    /// @{
    bool IsOn() const;
    bool IsExpanded() const;
    bool IsMovable() const;
    bool IsCollapsible() const;
    /// set the flag only.
    void SetShow(bool shown);
    void SetExpanded(bool f);
    void SetShowFrame(bool show);
    void MoveTo(TModelUnit y);
    void SetTrackAttr(TTrackAttrFlags flags);
    void SetTitle(const string& label, const string& default_title = NcbiEmptyString);
    void SetIsBlast(bool isBlast);
    void SetHost(ILayoutTrackHost* host);
    void SetOrder(int order);
    int  GetOrder() const;
    void SetId(const string& id);
    const string& GetId() const;

    /// close this track.
    void CloseMe();
    void ShowMe();
    /// Collapse or expand the track.
    void Expand(bool expand);
    /// @}


    /// @name CSeqGlyph interface implementation
    /// @{
    virtual bool OnLeftDown(const TModelPoint& p);
    virtual bool OnLeftDblClick(const TModelPoint& p);
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    /// @}

    /// Initialize the HTML active area for a track.
    /// @return retrun the newly added HTML active area for this track
    ///         if further initializatin is required.
    virtual CHTMLActiveArea* InitHTMLActiveArea(TAreaVector* p_areas) const;

    void MouseOut();
    bool OnMotion(const TModelPoint& pt);

    bool GetShowTitle() const;
    void SetShowTitle(bool flag);
    void SetConfig(CRef<CSeqGraphicConfig> conf);
    int  GetIndent() const;
    void SetIndent(int indent);
    void SetMsg(const string& msg);

    void SetParentTrack(CLayoutTrack* parent);
    CLayoutTrack* GetParentTrack();
    const CLayoutTrack* GetParentTrack() const;

protected:
    /// Icon id used in layout track.
    /// A derived track can extend the icon ids to accomodate
    /// any track-specific icons.  The eIcon_MaxTrackIconID can be
    /// considered as the maximum icon id value defined in this
    /// base class.  In other words, it is safe to use any id
    /// value > eIcon_MaxTrackIconID in the derived class.
    enum EIconID {
        eIcon_Close = 0,            ///< close icon
        eIcon_Expand,               ///< expand/collapse icon
        eIcon_Content,              ///< icon id for setting content
        eIcon_Layout,               ///< icon id for setting layout style
        eIcon_Settings,             ///< icon id for track settings
        eIcon_Help,                 ///< icon id for track help
        eIcon_MaxTrackIconID = 100   ///< the maximual icon ID defined in this class.
    };

    typedef int    TIconID;     ///< use int as TIconID instead of EIconID.
    typedef string TIconAlias;  ///< the icon alias used as a key

    /// A help struct for storing information about a icon.
    struct SIconInfo {
        SIconInfo()
            : m_Id(-1)
            , m_Tooltip("")
            , m_Enabled(true)
            , m_Shown(true)
        {};
        SIconInfo(TIconID id, const string& tt, bool enabled,
            const TIconAlias& icon_1, const TIconAlias& icon_2 = "",
            const TIconAlias& icon1_lite = "")
            : m_Id(id)
            , m_Tooltip(tt)
            , m_Icon1(icon_1)
            , m_Icon2(icon_2)
            , m_Icon1_lite(icon1_lite)
            , m_Enabled(enabled)
            , m_Shown(true)
        {};

        TIconID     m_Id;       ///< must be unique within the context
        string      m_Tooltip;
        TIconAlias  m_Icon1;    ///< icon alias for state #1
        /// icon alias for state #2, optional.
        /// This is used only for those buttons that have two states and
        /// need different visual presentation for each state, e.g. expand
        /// and collapse button.
        TIconAlias  m_Icon2;
        /// lighter version of icon1 (less opaque)
        TIconAlias  m_Icon1_lite;
        bool        m_Enabled;
        bool        m_Shown;
    };

    typedef vector<SIconInfo> TIcons;


    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

    /// Collapse or expand the track.
    /// @param expand true to expand the track, false to collapse it.
    virtual void x_Expand(bool expand);

    /// Need to preserve title space and show the title.
    /// Many factors may determine if a title will be rendered.
    virtual bool x_ShowTitle() const;

    /// Get title bar height including margin.
    TModelUnit x_GetTBHeight() const;

    /// Method for getting global configuration from rendering context.
    CConstRef<CSeqGraphicConfig> x_GetGlobalConfig() const;
    void x_OnLayoutChanged();

    /// load the track settings.
    /// By default, it does nothing.
    /// @param preset_style a preset setting style name
    /// @param settings a list of track-specific setting in a form of
    ///        key-value pair.
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);
    /// save the track settings to a profile string.
    virtual void x_SaveSettings(const string& preset_style);

    /// The content rendering must be implemented in the derived layout tracks.
    virtual void x_RenderContent() const = 0;
    virtual bool x_Empty() const = 0;
    virtual void x_ClearContent() {};

    /// Get title bar rectange exclude top margin.
    /// The tracks may need to override this method if the title bar is
    /// not standard one which is on the top.
    virtual void x_GetTBRect(TModelRect& rc) const;

    /// Method for rendering the job loading or layout generating progress.
    virtual void x_RenderProgressBar() const;

    /// Method for rendering the track title bar.
    virtual void x_RenderTitleBar() const;

    /// Histogram configuration (linear, log, etc) is also on the title bar but
    /// multiple track types can have histograms, so we use virtual functinons
    virtual string x_GetHistMsg() const { return ""; }

    /// Render track icons.
    /// If a derived track need to render icons differetly,
    /// then it has to override this method.
    /// If lite_version is true, and the lite version icon is provided,
    /// the lite version icon will be rendered, instead.
    virtual void x_RenderIcon(TIconID id, bool highlighted,
        bool lite_version = false) const;

    /// Mouse left-click event handler on an icon.
    /// In this class, only clicks on close and expand icons are handled.
    /// Any other icons are very likely track-specific, so they need to
    /// be handled in the derived tracks.
    virtual void x_OnIconClicked(TIconID id);

    /// Get current status for a given button.
    /// @return 0 if in its first state or only one state, otherwise, 1.
    virtual int x_GetIconState(TIconID id) const;


    /// register track icons.
    void x_RegisterIcon(const SIconInfo& icon);

    /// deregister an icon if already registered.
    void x_DeregisterIcon(TIconID id);

    /// Enable/disable an existing icon.
    void x_EnableIcon(TIconID id, bool enabled);

    /// Show/hide an existing icon.
    void x_ShowIcon(TIconID id, bool shown);

    /// hit any icon?.
    /// @return the icon index if yes, otherwise -1
    int x_HitIconTest(const TModelPoint& p) const;

    /// hit the title bar?.
    bool x_HitTitleBar(const TModelPoint& p) const;

    void x_SetStatus(const string& msg, int progress);
    void x_SetStartStatus();
    void x_SetFinishStatus();

    /// register images for the common icons.
    /// The common icons include close button, expand/collapse button,
    /// content button, layout policy button, and setting button.
    static void RegisterCommonIconImages();

    /// Indicates whether the track is NCBI track
    bool x_IsNCBITrack() const;

public: // for the sake of CSixFramesTransTrack
    /// register the image for an icon.
    /// The image is used for generating texture for rendering purpose.
    /// @param key the icon alias used for accessing the icon/icon texture.
    /// No two identical keys can be registered application-wise.
    /// @param img_file the image file with full path
    static void RegisterIconImage(
        const TIconAlias& key, const string& img_file);

protected:
    /// initialize OpenGL ttexture objects for icons.
    /// Will check if we indeed need to reinitialize OpenGL texture objects.
    /// We need to do this for the very first time, or when the texture objects
    /// become invalid somehow.
    /// @return false if fails to initialize icon textures
    static bool InitIconTextures();

private:
    /// Method for rendering the track frame including the icons.
    void x_RenderFrame() const;

    TModelRect x_GetIconRect(int idx) const;

    /// get icon index in the registed icons using its id.
    int x_GetIconIdxById(TIconID id) const;

    /// prohibited copy constructor and assignment operator.
    CLayoutTrack(const CLayoutTrack&);
    CLayoutTrack& operator=(const CLayoutTrack&);

protected:
    int     m_Order;            ///< track order number
    string  m_Id;               ///< track identifier
    bool    m_On;               ///< is this track shown
    bool    m_ShowFrame;        ///< need to draw frame, not title
    bool    m_Expanded;         ///< is this track in expanded state

    TIcons  m_Icons;            ///< registered icon info
    int     m_HighlightedIcon;  ///< the highlighted icon index

    TTrackAttrFlags     m_Attrs;    ///< various track attributes
    ILayoutTrackHost*   m_LTHost;   ///< Top level host owning the tracks.
    string  m_Title;            ///< track title
    bool    m_isBlast{false};   ///< true if the track is known to be generated and uploaded by BLAST
    string  m_DefaultTitle;     ///< Default track title
    string  m_Msg;              ///< special message need to show on title bar
    int     m_Progress;         ///< any job progress index (0 - 100)
    int     m_IndentLevel;      ///< track hierachy indentation level.
    TTrackErrorStatus m_ErrorStatus;

    CRef<CSeqGraphicConfig> m_gConfig;  ///< global configuration.
    string                  m_Profile = "Default";  ///< track setting profile

private:
    CLayoutTrack*  m_ParentTrack = nullptr;  ///< parent track (might differ from parent)
    typedef map< TIconAlias, CIRef<I3DTexture> >  TTexMap;
    static TTexMap  sm_IconMap;     ///< only intialized once app-wise
    static CFastMutex sm_Mutex;     ///< mutex guarding the icon texture map

    // check that the track pointer is not corrupted
//    bool // x_Validate(const string& sCodeLocation) const;
};

///////////////////////////////////////////////////////////////////////////////
/// IDroppable
/// The interface for tracks that support drag&drop
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IDroppable
{
public:
    virtual ~IDroppable() {}
    virtual bool CanDrop(CRef<CLayoutTrack>& track) = 0;
    virtual bool Drop(CRef<CLayoutTrack>& track) = 0;
};



///////////////////////////////////////////////////////////////////////////////
/// CLayoutTrack inline method implmentation
///
inline
void CLayoutTrack::SetProfile(const string& preset_style)
{
    // x_Validate("SetProfile");
    m_Profile = preset_style;
}

inline
const string& CLayoutTrack::GetProfile() const
{
    // x_Validate("GetProfile");
    return m_Profile;
}

inline /* virtual */
string CLayoutTrack::GetFullTitle() const
{
    // x_Validate("GetFullTitle");
    return m_Title.empty() ? m_DefaultTitle : m_Title;
}

inline
string CLayoutTrack::GetTitle() const
{
    // x_Validate("GetTitle");
    return m_Title.empty() ? m_DefaultTitle : m_Title;
}

inline
bool CLayoutTrack::IsOn() const
{
    // x_Validate("IsOn");
    return m_On;
}

inline
bool CLayoutTrack::IsExpanded() const
{
    // x_Validate("IsExpanded");
    return m_Expanded;
}

inline
bool CLayoutTrack::IsMovable() const
{
    // x_Validate("IsMovable");
    return m_Attrs & fMovable;
}

inline
bool CLayoutTrack::IsCollapsible() const
{
    // x_Validate("IsCollapsible");
    return m_Attrs & fCollapsible;
}

inline
void CLayoutTrack::SetShow(bool shown)
{
    // x_Validate("SetShow");
    m_On = shown;
}

inline
void CLayoutTrack::SetExpanded(bool f)
{
    // x_Validate("SetExpanded");
    m_Expanded = f;
}

inline
void CLayoutTrack::SetShowFrame(bool show)
{
    // x_Validate("SetShowFrame");
    m_ShowFrame = show;
}

inline
void CLayoutTrack::SetHost(ILayoutTrackHost* host)
{
    // x_Validate("SetHost");
    m_LTHost = host;
}

inline
int CLayoutTrack::GetOrder() const
{
    // x_Validate("GetOrder");
    return m_Order;
}

inline
void CLayoutTrack::SetOrder(int order)
{
    // x_Validate("SetOrder");
    m_Order = order;
}

inline
void CLayoutTrack::SetId(const string& id)
{
    // x_Validate("SetId");
    m_Id = id;
}

inline
const string& CLayoutTrack::GetId() const
{
    // x_Validate("GetId");
    return m_Id;
}

inline
const TTrackErrorStatus& CLayoutTrack::GetErrorStatus() const
{
    // x_Validate("GetErrorStatus");
    return m_ErrorStatus;
}


inline
void CLayoutTrack::SetErrorStatus(const TTrackErrorStatus& ErrorStatus)
{
    // x_Validate("SetErrorStatus");
    m_ErrorStatus = ErrorStatus;
}

inline
bool CLayoutTrack::isInErrorStatus() const
{
    // x_Validate("isInErrorStatus");
    return m_ErrorStatus.m_ErrorSeverity != STrackErrorStatus::eErrorSeverity_NoError;
}


inline
void CLayoutTrack::MoveTo(TModelUnit y)
{
    // x_Validate("MoveTo");
    SetTop(GetTop() + y);
}

inline
void CLayoutTrack::SetTitle(const string& label, const string& default_title)
{
    // x_Validate("SetTitle");
    m_Title = label;
    m_DefaultTitle = default_title;
}

inline
void CLayoutTrack::SetIsBlast(bool isBlast)
{
    // x_Validate("SetTitle");
    m_isBlast = isBlast;
}

inline
void CLayoutTrack::Expand(bool expand)
{
    // x_Validate("Expand");
    x_Expand(expand);
}

inline
CConstRef<CSeqGraphicConfig> CLayoutTrack::x_GetGlobalConfig() const
{
    // x_Validate("x_GetGlobalConfig");
    return m_gConfig;
}

inline
bool CLayoutTrack::GetShowTitle() const
{
    // x_Validate("GetShowTitle");
    return m_Attrs & fShowTitle;
}

inline
void CLayoutTrack::SetShowTitle(bool flag)
{
    // x_Validate("SetShowTitle");
    m_Attrs = (m_Attrs & ~fShowTitle) | (flag ? fShowTitle : 0x0);
}

inline
void CLayoutTrack::SetConfig(CRef<CSeqGraphicConfig> conf)
{ 
    // x_Validate("SetConfig");
    m_gConfig = conf; 
}

inline
int CLayoutTrack::GetIndent() const
{ 
    // x_Validate("GetIndent");
    return m_IndentLevel; 
}

inline
void CLayoutTrack::SetIndent(int indent)
{ 
    // x_Validate("SetIndent");
    m_IndentLevel = indent; 
}

inline
void CLayoutTrack::SetMsg(const string& msg)
{ 
    // x_Validate("SetMsg");
    m_Msg = msg; 
}

inline
void CLayoutTrack::SetParentTrack(CLayoutTrack* parent)
{ 
    // x_Validate("SetParentTrack");
    m_ParentTrack = parent; 
}

inline
CLayoutTrack* CLayoutTrack::GetParentTrack()
{ 
    // x_Validate("GetParentTrack"); 
    return m_ParentTrack; 
}

inline
const CLayoutTrack* CLayoutTrack::GetParentTrack() const
{ 
    // x_Validate("GetParentTrack const"); 
    return m_ParentTrack;
}

inline
void CLayoutTrack::x_SetStatus(const string& msg, int progress)
{
    // x_Validate("x_SetStatus");
    m_Msg = msg;
    m_Progress = progress;
}

inline
void CLayoutTrack::x_SetStartStatus()
{
    // x_Validate("");
    m_Msg = ", Loading.....";
    m_Progress = 0;
}

inline
void CLayoutTrack::x_SetFinishStatus()
{
    // x_Validate("x_SetStartStatus");
    m_Msg = "";
    m_Progress = 100;
}

inline 
bool CLayoutTrack::x_IsNCBITrack() const
{
    // x_Validate("x_IsNCBITrack");
    if (NStr::StartsWith(m_Id, 'R') || NStr::StartsWith(m_Id, 'U') || isRemotePath())
        return false;

    return true;
}

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_IMPL__HPP
