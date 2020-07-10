#ifndef GUI_WIDGETS_SEQ_GRAPHIC___TRACK_CONTAINER_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___TRACK_CONTAINER_TRACK__HPP

/* $Id: track_container_track.hpp 44402 2019-12-12 18:12:05Z filippov $
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


#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/feature_panel_ds.hpp>
#include <gui/widgets/seq_graphic/track_config_manager.hpp>

class wxEvtHandler;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CTrackContainer - a track container in the form of layout track.
/// Compared to CGlyphContainer, this class assume its children must be one 
/// type of CLayoutTracks.  So it has knowledge about the child tracks
/// to turn them on or off. The purpose of CTrackContainer is that it forms a
/// group of tracks to make them behave like a single track: movable,
/// collapsible, and closible. The track container may be resposible of
/// creating/initializing the contained tracks. The reason of inhereting
/// from CDataTrack is because a container track may need to initialize 
/// its child tracks asynchronously. Potentially, a container track 
/// may delegate its child tracks to load data for performance purpose.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTrackContainer :
    public CDataTrack,
    public ITrackContainer

{
    friend class CTrackContainerFactory;
public:

    typedef CTempTrackProxy::TTrackProxies TTrackProxies;

    typedef ILayoutTrackFactory::TTrackMap TTrackMap;

    typedef ILayoutTrackFactory::TAnnotMetaDataList TAnnotMetaDataList;

    /// explicitly named annots used by a track.
    typedef list<string>    TUsedAnnots;
    typedef map< string, CIRef<ILayoutTrackFactory> >  TFactoryMap;
    typedef vector<string>  TNAAs;    

    CTrackContainer(CRenderingContext* r_cntx, CFeaturePanelDS* ds = NULL);
    virtual ~CTrackContainer();

    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;

    virtual void DeleteAllJobs();
    virtual bool AllJobsFinished() const;
    bool AllTracksCreated() const;

    /// Add a track proxy.
    /// The track proxy may or may not have a real track associated
    /// with it.
    void AddTrackProxy(CTempTrackProxy* proxy, bool upgateMsg = true);

    /// Add a new empty track proxy.
    /// track name is optional. If missing, a unique random track name will
    /// be used.
    CTrackProxy* AddNewTrack(const string& track_key,
        const CTrackProxy::TAnnots& annots,
        const string& disp_name, const string& name = "",
        const string& filter = "", const string& sub_key = "");

    void AddNewTrack(CTrackProxy* proxy);


    /// add sub tracks after tracks get created.
    /// The track proxy may already have tracks associated with it.
    /// If the proxy have more than one subtracks or tracks contains
    /// more than one tracks, those tracks will be put under a track
    /// container.
    void AddTracks(CTempTrackProxy* t_proxy, TTrackMap& tracks);

    /// @name ITrackContainer interface implementation.
    /// @{
    /// Replace the track by the given track order.
    /// Assumming there is an existing track proxy with the same order.
    /// It is also ok to have no corresponding track proxy if a track
    /// is unmovable, noncollapsible, and nonclosible.
    virtual void SetTrack(CLayoutTrack* track, int order, bool shown = true);

    /// Duplicate an existing track.
    /// @param order the source track order
    /// @param track_title the track title for the dupliated track
    virtual void CloneTrack(int order, const string& track_tile);

    /// Rename a child track.
    /// @param order the source track order
    /// @param new_name the track
    virtual void RenameChildTrack(int order, const string& new_name);

    /// completely remove a track from the hierachical tree.
    /// @param order is the track order.
    virtual void RemoveTrack(int order, bool immediately = true);

    /// Close a track.
    /// Hide the given track, but track is not removed.
    virtual void CloseTrack(int order);

    /// move a track up by 1.
    virtual void MoveUp(int order);

    /// move a track down by 1.
    virtual void MoveDown(int order);

    /// move a track up by 1.
    virtual void MoveTrackToTop(int order);

    /// move a track down by 1.
    virtual void MoveTrackToBottom(int order);

    void MoveTrack(int from, int to);

    /// Has any real subtracks.
    /// This differs from x_Empty() method.  x_Empty() method only considers
    /// the visible tracks, but HasSubtracks() will consider the ones
    /// that are off.
    virtual bool HasSubtracks() const;

    /// Will have no subtrack for ever.
    /// There will be no any potential subtracks?
    virtual bool NoSubtrackEver() const;
    /// @}

    /// Save track states to it proxy.
    void SaveTrackStates(int order);

    /// Remove all tracks.
    void ClearTracks();

    string GetUniqueTrackName(const string& name) const;

    /// Get a child track container track.
    /// The reutrned child track is also a track container track with
    /// the given track name.
    CTrackContainer* GetChildContainerTrack(const string& name);

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

	CFeaturePanelDS* GetDS() { return m_DS; }

    /// @name data track pure virtual methods.
    /// @{
    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;
    /// @}

    /// expand or collapse this track and its subtracks recursively.
    /// @param expand true if expand, false if collapse
    void ExpandAll(bool expand);
    /// turn on/off all children tracks (recursively).
    void ShowAll(bool shown, bool recursive = true);

    void SortChildTracksByOrder();

    /// remove the empty tracks recursively from bottom to top.
    void RemoveEmptyTracks();

    const TTrackProxies& GetSubtrackProxies() const;
    TTrackProxies& GetSubtrackProxies();
    void SetSubtrackProxies(const TTrackProxies& proxies);

    void SetConfigMgr(CTrackConfigManager* cfg_mgr);
    CTrackConfigManager* GetConfigMgr();
    const CTrackConfigManager* GetConfigMgr() const;

    void SetTrackInitDone(bool f);
    bool IsTrackInitDone() const;

    void SetAnnotLevel(int level);
    int GetAnnotLevel() const;

    void SetAdaptive(bool flag);
    bool GetAdaptive() const;

    void SetSkipGenuineCheck(bool flag);
    bool GetSkipGenuineCheck() const;

    /// @name content menu event handlers.
    /// @{
    void OnSelectAll();
    void OnDeselectAll();
    void OnRevertSelection();
    void OnMore();
    virtual void OnToggleTrack(int order);
    virtual void OnTrackSettings(int order);
    /// @}

    virtual bool IsOverlay() const;

    // naAccessions to be loaded by parent
    void ToggleTracks(vector<string>& naAccessions);

    void LoadNATracks(CTempTrackProxy* t_proxy, const vector<string>& accessions);
    void LoadNATracks(CDataTrack* child, const vector<string>& naAccessions);

    void OnMenuToggleTrack(int index);
    void OnMenuTrackSettings(int index);

    virtual void ReinitNASubtracks(TAnnotMetaDataList&, bool) {}
    void InitNATracks(const TAnnotMetaDataList& annots, bool makeContainersVisible);

    template <typename Tr>
    void Traverse(Tr&& tr, bool visible)
    {
        if (tr.ContainerBegin(nullptr, this, visible)) {
            x_Traverse(std::forward<Tr>(tr), visible);
            tr.ContainerEnd(nullptr, this, visible);
        }
    }

    void SyncSettings(TTrackProxies& dst_proxies, int level) const;

    virtual void ResetSearch() {m_LTHost->ResetSearch();}
protected:
    template <typename Tr>
    void x_Traverse(Tr&& tr, bool visible)
    {
        for (auto& p : m_TrackProxies) {
            CLayoutTrack* track = p->GetTrack();
            if (track) {
                bool v = visible && p->GetShown();
                CTrackContainer* cont = dynamic_cast<CTrackContainer*>(track);
                if (cont) {
                    if (tr.ContainerBegin(p, cont, v)) {
                        cont->x_Traverse(tr, v);
                        tr.ContainerEnd(p, cont, v);
                    }
                }
                else
                    tr.Track(p, track, v);
            }
            else {
                if (p->GetKey().empty()) { // ignore an empty or uninitalaized container
                    if (p->IsNA()) // not loaded NA track
                        tr.Track(p, nullptr, false);
                }
            }
        }
    }

    virtual bool x_Empty() const;

    CRef<CTempTrackProxy> x_GetMenuTrack(int index);

    /// @name data track pure virtual methods.
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

    /// override method in CLayoutTrack to handle speical cases.
    virtual void x_OnIconClicked(TIconID id);
    
    void x_CreateTracksMenu(wxMenu& menu, TIconID id);

    /// actions to take on all job finished.
    virtual void x_OnAllJobsFinished();

    /// reinitialize all subtracks if necessary.
    virtual void x_ReinitSubtracks();

    /// reinitialize a given subtrack
    void x_ReinitTrack(CTrackProxy* t_proxy);

    /// udpate message in track title bar.
    void x_UpdateMsg();

    CTrackContainer* x_ConvertToContainer(CTempTrackProxy* proxy);

    virtual bool x_ShowTitle() const;

private:
    /// prohibited copy constructor and assignment operator.
    CTrackContainer(const CTrackContainer&);
    CTrackContainer& operator=(const CTrackContainer&);

    void x_AddSubtrack(CTrackContainer* parent,
        CLayoutTrack* track, CTempTrackProxy* proxy);

    void x_ShowTrack(CLayoutTrack* track, bool on);

    bool x_HasToggleTracks() const;
    bool x_HasToBeShown() const;

protected:
    CRef<CFeaturePanelDS>       m_DS;

    /// all the tracks including on and off.
    /// All the tracks (on/off) are sorted by track order in increasing
    /// order. Similarly, all the visible tracks (hold in CGlyphContainer)
    /// are also sorted in increasing order by track order.
    TTrackProxies m_TrackProxies;

    void x_ReinitNATrack(CTempTrackProxy* t_proxy, TAnnotMetaDataList& annots, bool makeContainersVisible);

    /// Make all the tracks at different levels as top-level tracks.
    void x_MakeTopLevelTracks(CGlyphContainer* featurePanel, TTrackProxies& dst_proxies, int& order, const string& base_name, int indent);

private:
    /// wx-related event handler.
    wxEvtHandler*       m_Handler;

    CRef<CTrackConfigManager> m_ConfigMgr;

    /// Annotation selector depth.
    int     m_AnnotLevel;

    /// Adpative/Exact selector
    bool    m_Adaptive;

    /// flag indicating whether tracks initialization is finished.
    bool    m_TrackInitFinished;

    /// flag indicating if track verification is required.
    /// By default, a data track will be verified to make sure
    /// it indeed exists during track configuration according to
    /// a list of requested tracks.
    bool    m_SkipGenuineCheck;

    /// CLayoutTrack required track type info.
    static CTrackTypeInfo m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CTrackContainerFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTrackContainerFactory :
    public CObject,
    public ILayoutTrackFactory,
    public IExtension
{
public:
    CTrackContainerFactory(){}

    /// create a layout track based on the input objects and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual bool UnderstandLevel() const
    { return true; }

    virtual bool NeedBackgroundInit() const
    { return false; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CTrackContainer::m_TypeInfo; }

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};

///////////////////////////////////////////////////////////////////////////////
///  ITrackInitObserver: interface class work as an observer of a track that
///  is responsible of intializing subtracks.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ITrackInitObserver
{
public:
    virtual ~ITrackInitObserver() {}

    virtual void OnTrackInitFinished(const CLayoutTrack* sender) = 0;
};


///////////////////////////////////////////////////////////////////////////////
///   CTrackContainer inline methods
///
inline
const CTrackContainer::TTrackProxies& CTrackContainer::GetSubtrackProxies() const
{ return m_TrackProxies; }

inline
CTrackContainer::TTrackProxies& CTrackContainer::GetSubtrackProxies()
{ return m_TrackProxies; }


inline
void CTrackContainer::SetConfigMgr(CTrackConfigManager* cfg_mgr)
{ m_ConfigMgr.Reset(cfg_mgr); }

inline
CTrackConfigManager* CTrackContainer::GetConfigMgr()
{ return m_ConfigMgr; }

inline
const CTrackConfigManager* CTrackContainer::GetConfigMgr() const
{ return m_ConfigMgr; }

inline
void CTrackContainer::SetTrackInitDone(bool f)
{ m_TrackInitFinished = f; }

inline
bool CTrackContainer::IsTrackInitDone() const
{ return m_TrackInitFinished; }

inline
void CTrackContainer::SetAnnotLevel(int level)
{ m_AnnotLevel = level; }

inline
int CTrackContainer::GetAnnotLevel() const
{ return m_AnnotLevel; }

inline
void CTrackContainer::SetAdaptive(bool flag)
{ m_Adaptive = flag; }

inline
bool CTrackContainer::GetAdaptive() const
{ return m_Adaptive; }

inline
void CTrackContainer::SetSkipGenuineCheck(bool flag)
{ m_SkipGenuineCheck = flag; }

inline
bool CTrackContainer::GetSkipGenuineCheck() const
{ return m_SkipGenuineCheck; }

inline 
bool CTrackContainer::IsOverlay() const
{
    return false;
}

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___TRACK_CONTAINER_TRACK__HPP
