#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_PROXY__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_PROXY__HPP

/* $Id: layout_track_proxy.hpp 44576 2020-01-23 23:22:13Z rudnev $
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
 * Author:  Liangshou Wu, Dmitry Rudnev, Victor Joukov
 *
 */

 /**
 * File Description:
  */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/layout_track_impl.hpp>
#include <set>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CTempTrackProxy data structure for storing information about tracks
/// status and the layout of the tracks (both position and hierarchy).
/// The stored status includes track order and a flag indicating
/// weather a track is shown. For a given track type, there might be
/// more than one data track get created after loading the data. So,
/// the track status for those data source dependent temporary tracks
/// will also be stored.

class CTempTrackProxy : public CObjectEx
{
public:

    typedef list< CRef<CTempTrackProxy> >  TTrackProxies;
    typedef vector<string> TSubTracks;
    static const string kAnonTrackName;

    CTempTrackProxy() {}

    CTempTrackProxy(int order, const string& name, bool shown,
                    const string& key = NcbiEmptyString, const string& sub_key = NcbiEmptyString)
        : m_Order(order)
        , m_Name(name)
        , m_Shown(shown)
        , m_Key(key)
        , m_Subkey(sub_key)
    {}

    /// copy constructor.
    CTempTrackProxy(const CTempTrackProxy& param);

    /// assignment operator.
    CTempTrackProxy& operator=(const CTempTrackProxy& param);

    /// check if this is a temporary track.
    virtual bool IsTempTrack() const { return true; }
    virtual CRef<CTempTrackProxy> Clone() const;

    void SetOrder(int order);
    int  GetOrder() const;

    const string& GetName() const { return m_Name; }
    void SetName(const string& name) { m_Name = name; }

    const string& GetId() const;
    void SetId(const string& id);

    const string& GetDisplayName() const;
    void SetDisplayName(const string& name);

    void SetShown(bool shown);
    bool GetShown() const;

    const string& GetHelp() const;
    void SetHelp(const string& help);

    void SetExpanded(bool f);
    bool GetExpanded() const;

    /// Associated any track already?.
    void SetTrack(CLayoutTrack* track);
    bool TrackNotSet() const;
    const CLayoutTrack* GetTrack() const;
    CLayoutTrack* GetTrack();

    void SetTrackProfile(const string& profile);
    const string& GetTrackProfile() const;

    void SetSource(const string& src);
    const string& GetSource() const;

    void SetCategory(const string& cat);
    const string& GetCategory() const;

    void SetSubcategory(const string& scat);
    const string& GetSubcategory() const;

    void SetSettingGroup(const string& name);
    const string& GetSettingGroup() const;

    void SetRemoteData(const string& remote_data);
    const string& GetRemoteData() const;

    void SetRemoteId(const string& remote_id);
    const string& GetRemoteId() const;

    void SetSeqDataKey(const string& seq_data_key);
    const string& GetSeqDataKey() const;

    void SetUseGroupDefault(bool flag);
    bool GetUseGroupDefault() const;

    const string& GetFilter() const;
    void SetFilter(const string& filter);

    const string& GetSortBy() const;
    void SetSortBy(const string& sort_by);

    void SetRealized(bool f) { m_Realized = f; }
    bool IsRealized() const { return m_Realized; }

    void SetEmpty(bool f) { m_Empty = f; }
    bool IsEmpty() const { return m_Empty; }

    const TTrackProxies& GetChildren() const { return m_Children; }
    TTrackProxies& GetChildren() { return m_Children; }

    void SetVisitedRange(const TSeqRange& range);
    const TSeqRange& GetVisitedRange() const;

    /// add a new template track params.
    /// The order will be initialized based on maximal track order used
    /// @return the newly added track params.
    CRef<CTempTrackProxy> AddTempTrack(const string& name, const string& title);

    /// add a new template track params with a filter set
    CRef<CTempTrackProxy> AddTempTrack(
        const string& name, const string& filter, const string& title);

    /// Get track params for child temporary track.
    /// @return an existing track params or newly created if not available
    CConstRef<CTempTrackProxy> GetChildTempTrack(const string& name) const;

    /// @return an existing track params or newly created if not available
    CRef<CTempTrackProxy> GetChildTempTrack(const string& name);

    /// compare operator used by sorting algorithms.
    bool operator<(const CTempTrackProxy& item) const;

    void SetComments(const string& comment_str);
    const string& GetComments() const;

    void SetHighlights(const string& hl_str);
    const string& GetHighlights() const;

    void SetHighlightsColor(const string& hl_str);
    const string& GetHighlightsColor() const;

    void SetShowTitle(bool flag);
    bool GetShowTitle() const;

    const TTrackErrorStatus& GetErrorStatus() const;
    void SetErrorStatus(const TTrackErrorStatus& ErrorStatus);

    void SetIsPrivate(bool flag);
    bool GetIsPrivate() const;

    void SetStoredScale(const string& stored_scale);
    const string& GetStoredScale() const;

    void SetUId(const string& uid);
    const string& GetUId() const;

    void SetBatch(const string& batch);
    const string& GetBatch() const;

    void SetExtraInfo(const string& sExtraInfo);
    const string& GetExtraInfo() const;

    void SetRemotePath(const string& remote_path);
    const string& GetRemotePath() const;

    const TSubTracks& GetSubTracks() const;
    TSubTracks& SetSubTracks();

    enum EAction
    {
        eNone,
        eMakeVisible,
        eHide
    };

    void    SetAction(EAction action) { m_Action = action; }
    EAction GetAction() const { return m_Action; }

    void SetTimeStamp(time_t timeStamp) { m_TimeStamp = timeStamp; }
    time_t GetTimeStamp() const { return m_TimeStamp; }

    const string& GetKey() const { return m_Key; }
    void SetKey(const string& key) { m_Key = key; }

    const string& GetSubkey() const { return m_Subkey; }
    void SetSubkey(const string& sub_key) { m_Subkey = sub_key; }

    typedef vector<string> TAnnots;
    const TAnnots& GetAnnots() const { return m_Annots; }
    void SetAnnots(const TAnnots& annots) { m_Annots = annots; }

    bool IsNA() const;
    /// check  for an extended NA (named annotation#number, as used in e.g. SNP2)
    bool IsExtendedNA() const;

    void SetNAdata(CAnnotMetaData& na);
    const CAnnotMetaData* GetNAdata() const { return m_NAdata.GetPointerOrNull(); }

    void SetBigDataIndex(const string& big_data_index);
    const string& GetBigDataIndex() const;

    void SetHubId(const string& hub_id);
    const string& GetHubId() const;

    void UpdateNA_ChildVisibility(CTempTrackProxy& child) const;

    /// if isMergeWithExisting is true, then load should skip all registry tracks that have keys as in the already
    /// existing proxies
    static void LoadFromRegistry(TTrackProxies& proxies, const string& path, bool isMergeWithExisting = false);
    static void SaveToRegistry(const TTrackProxies& proxies, const string& path);

    /// convert a string with a list of subtracks to a list of subtracks
    static void ParseSubtracks(const string& subtracks_str, TSubTracks& subtracks);

private:
    /// track order for vertical ordering in its parent scope.
    /// Initially, the track order is -1 for a newly created track params
    /// before associating it with a real track.
    int     m_Order = -1;

    /// track name, may serve as track title.
    string  m_Name;

    /// unique track identifier.
    string  m_Id;

    /// track display name.
    /// If empty, use track name
    string  m_DispName;

    /// track profile (setting style or list of settings).
    string  m_Profile;

    /// data source.
    /// For most of cases, data source is the same as track name.
    /// If that is the case, m_Source may be empty.  m_Source will be
    /// used when a track need data source info which is different
    /// from m_Name.
    string  m_Source;

    /// track category.
    string  m_Category;

    /// track sub-category.
    string  m_Subcategory;

    /// setting group name.
    /// A group name defining a set of similar tracks
    /// that have the same properties and potentially can
    /// share the same set of settings
    string  m_SettingGroup;

    /// flag indicating if group default setting applies.
    /// default: false
    bool    m_UseGroupDefault = false;

    /// track-specific filter
    string  m_Filter;

    /// track-specific sortby
    string  m_SortBy;

    /// settings for children tracks.
    /// Each created track may correspond to one data source,
    /// or data type (feature type/subtype). For example, the gene model
    /// may have gene model track created based on NCBI gene model,
    /// ENSEMBL gene model, other RNAs and Exons.
    /// optional, only for group-type track
    TTrackProxies     m_Children;

    /// the delegated layout track object.
    CRef<CLayoutTrack>  m_Track;

    /// track on/off state.
    bool            m_Shown = true;

    /// track help string
    string          m_Help;

    /// track expanded/collapsed.
    bool            m_Expanded = true;

    /// Has this been connected to any real track before.
    /// A tempory track proxy may be created for a potential track
    /// that may not exist. This flag is for indicating if we
    /// need to save the proxy.
    bool            m_Realized = true;

    /// The track proxy is empty.
    /// It can be removed and will be removed.
    bool            m_Empty = false;

    /// The sequence range visited so far.
    TSeqRange       m_VisitedRange;

    string          m_Comments;
    string          m_Highlights;
    /// Color to use for highlighting
    string          m_HighlightsColor;

    bool            m_ShowTitle = true;
    TTrackErrorStatus m_ErrorStatus;
    bool            m_isPrivate = false;
    string          m_StoredScale;

    string m_UId; // User-defined track id
    TSubTracks m_SubTracks;

    string          m_Key;      ///< may be used for creating a track
    string          m_Subkey;   ///< may be used for creating DataSource
    TAnnots         m_Annots;   ///< can be 'Unnamed', 'Named', or annot names

    EAction         m_Action = eNone;
    time_t          m_TimeStamp = -1;
    CRef<CAnnotMetaData> m_NAdata;
    string          m_Batch;    ///< AlignDb batch

    string          m_RemotePath; ///< Track remote path

    string          m_sExtraInfo; ///< extra information potentially needed to create the track
    string          m_RemoteData;
    string          m_RemoteId;
    string          m_SeqDataKey;
    string          m_BigDataIndex; ///< The location of a remote data file containing the index
    string          m_HubId;
};


///////////////////////////////////////////////////////////////////////////////
/// CTrackProxy data structure contains information about the position of
/// a track in its parent scope, and information about intializing the layout
/// track and its associated data source.

class CTrackProxy : public CTempTrackProxy
{
public:
    /// help functor for less comparison.
    struct STPSorterByOrder
    {
        static bool s_CompareCRefs (const CRef<CTempTrackProxy>& t1,
                                    const CRef<CTempTrackProxy>& t2)
        {
            return t1->GetOrder() < t2->GetOrder();
        }

        bool operator() (const CRef<CTempTrackProxy>& t1,
            const CRef<CTempTrackProxy>& t2)
        {
            return s_CompareCRefs(t1, t2);
        }

        bool operator() (const CRef<CTrackProxy>& t1,
            const CRef<CTrackProxy>& t2)
        {
            return t1->GetOrder() < t2->GetOrder();
        }
    };

    CTrackProxy() {}

    CTrackProxy(int order, const string& name, bool shown,
        const string& key, const string& sub_key)
        : CTempTrackProxy(order, name, shown, key, sub_key), m_ContShown(false)
    {}

    CTrackProxy(const CTrackProxy& param);

    CTrackProxy& operator=(const CTrackProxy& param);

    virtual bool IsTempTrack() const { return false; }
    virtual CRef<CTempTrackProxy> Clone() const;

    /// @name accessors.
    /// @{
    const string& GetOriginalKey() const { return m_OriginalKey; }
    const string& GetDB() const { return m_DB; }
    const string& GetDataKey() const { return m_DataKey; }
    bool          GetContShown() const { return m_ContShown; }
     
    /// @}

    /// @name mutators.
    /// @{
    void SetOriginalKey(const string& key) { m_OriginalKey = key; }
    void SetDB(const string& db) { m_DB = db; }
    void SetDataKey(const string& key) { m_DataKey = key; }
    void SetContShown(bool contShown) { m_ContShown = contShown; }
    /// @}

private:
    string          m_OriginalKey; // if the key was set in URL parameters, it's duplicated here
    string          m_DB;       ///< data source DB name
    string          m_DataKey;  ///< user-data net storage key (SViewer only)
    bool            m_ContShown = false;
};


///////////////////////////////////////////////////////////////////////////////
/// CTempTrackProxy class implementation

inline
void CTempTrackProxy::SetOrder(int order)
{
    m_Order = order;
    if (m_Track) m_Track->SetOrder(order);
}

inline
int  CTempTrackProxy::GetOrder() const
{
    if (m_Track &&  m_Track->GetOrder() != m_Order) {
        LOG_POST(Error << m_Track->GetFullTitle() << ": inconsistent ids!");
    }
    return m_Track ? m_Track->GetOrder() : m_Order;
}

inline
void CTempTrackProxy::SetId(const string& id)
{
    m_Id = id;
    if (m_Track) m_Track->SetId(id);
}

inline
const string& CTempTrackProxy::GetId() const
{ return m_Id; }

inline
const string& CTempTrackProxy::GetDisplayName() const
{ return m_DispName; }

inline
void CTempTrackProxy::SetDisplayName(const string& name)
{ m_DispName = name; }

inline
void CTempTrackProxy::SetShown(bool shown)
{ m_Shown = shown; }

inline
bool CTempTrackProxy::GetShown() const
{ return m_Track ? m_Track->IsOn() : m_Shown; }

inline
const string& CTempTrackProxy::GetHelp() const
{
    return m_Help;
}

inline
void CTempTrackProxy::SetHelp(const string& help)
{
    m_Help = help;
}

inline
void CTempTrackProxy::SetExpanded(bool f)
{ m_Expanded = f; }

inline
bool CTempTrackProxy::GetExpanded() const
{ return m_Track ? m_Track->IsExpanded() : m_Expanded; }

inline
void CTempTrackProxy::SetTrack(CLayoutTrack* track)
{
    m_Track.Reset(track);
    if (track) {
        m_Track->SetOrder(m_Order);
        m_Track->SetId(m_Id);
        m_Track->SetShow(m_Shown);
        m_Track->SetExpanded(m_Expanded);
        m_Track->SetProfile(m_Profile);
        m_Track->SetErrorStatus(m_ErrorStatus);
        m_Realized = true;
    }
}

inline
const CLayoutTrack* CTempTrackProxy::GetTrack() const
{ return m_Track.GetPointer(); }

inline
CLayoutTrack* CTempTrackProxy::GetTrack()
{ return m_Track; }

inline
bool CTempTrackProxy::TrackNotSet() const
{ return m_Track.IsNull(); }

inline
void CTempTrackProxy::SetTrackProfile(const string& profile)
{ m_Profile = profile; }

inline
const string& CTempTrackProxy::GetTrackProfile() const
{ return m_Track ? m_Track->GetProfile() : m_Profile; }

inline
bool CTempTrackProxy::operator<(const CTempTrackProxy& item) const
{ return m_Order < item.GetOrder(); }

inline
void CTempTrackProxy::SetSource(const string& src)
{ m_Source = src; }

inline
const string& CTempTrackProxy::GetSource() const
{ return m_Source; }

inline
void CTempTrackProxy::SetCategory(const string& cat)
{ m_Category = cat; }

inline
const string& CTempTrackProxy::GetCategory() const
{ return m_Category; }

inline
void CTempTrackProxy::SetSubcategory(const string& scat)
{ m_Subcategory = scat; }

inline
const string& CTempTrackProxy::GetSubcategory() const
{ return m_Subcategory; }

inline
void CTempTrackProxy::SetSettingGroup(const string& name)
{ m_SettingGroup = name; }

inline
const string& CTempTrackProxy::GetSettingGroup() const
{ return m_SettingGroup; }

inline
void CTempTrackProxy::SetUseGroupDefault(bool flag)
{ m_UseGroupDefault = flag; }

inline
bool CTempTrackProxy::GetUseGroupDefault() const
{ return m_UseGroupDefault; }

inline
const string& CTempTrackProxy::GetFilter() const
{ return m_Filter; }

inline
void CTempTrackProxy::SetFilter(const string& filter)
{ m_Filter = filter; }

inline
const string& CTempTrackProxy::GetSortBy() const
{ return m_SortBy; }

inline
void CTempTrackProxy::SetSortBy(const string& sort_by)
{ m_SortBy = sort_by; }

inline
void CTempTrackProxy::SetVisitedRange(const TSeqRange& range)
{ m_VisitedRange = range; }

inline
const TSeqRange& CTempTrackProxy::GetVisitedRange() const
{ return m_VisitedRange; }

inline
void CTempTrackProxy::SetComments(const string& comment_str)
{ m_Comments = comment_str; }

inline
const string& CTempTrackProxy::GetComments() const
{ return m_Comments; }

inline
void CTempTrackProxy::SetHighlights(const string& hl_str)
{ m_Highlights = hl_str; }

inline
const string& CTempTrackProxy::GetHighlights() const
{ return m_Highlights; }

inline
void CTempTrackProxy::SetHighlightsColor(const string& hl_str)
{
    m_HighlightsColor = hl_str;
}

inline
const string& CTempTrackProxy::GetHighlightsColor() const
{
    return m_HighlightsColor;
}

inline
void CTempTrackProxy::SetShowTitle(bool flag)
{ m_ShowTitle = flag; }

inline
bool CTempTrackProxy::GetShowTitle() const
{ return m_ShowTitle; }

inline
void CTempTrackProxy::SetErrorStatus(const TTrackErrorStatus& ErrorStatus)
{
    m_ErrorStatus = ErrorStatus;
    if (m_Track) m_Track->SetErrorStatus(ErrorStatus);
}

inline
const TTrackErrorStatus& CTempTrackProxy::GetErrorStatus() const
{
    return m_ErrorStatus;
}

inline
void CTempTrackProxy::SetIsPrivate(bool flag)
{ m_isPrivate = flag; }

inline
bool CTempTrackProxy::GetIsPrivate() const
{ return m_isPrivate; }

inline
void CTempTrackProxy::SetRemoteData(const string& remote_data)
{ m_RemoteData = remote_data; }

inline
const string& CTempTrackProxy::GetRemoteData() const
{ return m_RemoteData; }

inline
void CTempTrackProxy::SetRemoteId(const string& remote_id)
{
    m_RemoteId = remote_id;
}

inline
const string& CTempTrackProxy::GetRemoteId() const
{
    return m_RemoteId;
}


inline
void CTempTrackProxy::SetSeqDataKey(const string& seq_data_key)
{ m_SeqDataKey = seq_data_key; }

inline
const string& CTempTrackProxy::GetSeqDataKey() const
{ return m_SeqDataKey; }

inline
void CTempTrackProxy::SetBatch(const string& batch)
{ m_Batch = batch; }

inline
const string& CTempTrackProxy::GetBatch() const
{ return m_Batch; }

inline
void CTempTrackProxy::SetExtraInfo(const string& sExtraInfo)
{ m_sExtraInfo = sExtraInfo; }

inline
const string& CTempTrackProxy::GetExtraInfo() const
{ return m_sExtraInfo; }

inline
void CTempTrackProxy::SetRemotePath(const string& remote_path)
{
    m_RemotePath = remote_path;
}

inline
const string& CTempTrackProxy::GetRemotePath() const
{
    return m_RemotePath;
}

inline
void CTempTrackProxy::SetStoredScale(const string& stored_scale)
{ m_StoredScale = stored_scale; }

inline
const string& CTempTrackProxy::GetStoredScale() const
{ return m_StoredScale; }

inline
void CTempTrackProxy::SetUId(const string& uid)
{
    m_UId = uid;
}

inline
const string& CTempTrackProxy::GetUId() const
{
    return m_UId;
}

inline
const CTempTrackProxy::TSubTracks& CTempTrackProxy::GetSubTracks() const
{
    return m_SubTracks;
}

inline
CTempTrackProxy::TSubTracks& CTempTrackProxy::SetSubTracks()
{
    return m_SubTracks;
}

inline
void CTempTrackProxy::SetBigDataIndex(const string& big_data_index)
{
    m_BigDataIndex = big_data_index;
}

inline
const string& CTempTrackProxy::GetBigDataIndex() const
{
    return m_BigDataIndex;
}

inline
void CTempTrackProxy::SetHubId(const string& hub_id)
{
    m_HubId = hub_id;
}

inline
const string& CTempTrackProxy::GetHubId() const
{
    return m_HubId;
}

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_PROXY__HPP
