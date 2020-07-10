#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK__HPP

/* $Id: layout_track.hpp 44952 2020-04-25 01:20:25Z rudnev $
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
 * Author:  Liangshou Wu, Victor Joukov
 *
 */

 /**
 * File Description:
 */

#include <gui/gui_export.h>
#include <util/range.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/annot_info.hpp>
#include <gui/widgets/seq_graphic/layout_conf.hpp>
#include <objects/seqset/Seq_entry.hpp>

class wxMenu;
class wxEvtHandler;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// FeaturePanel uses the following extension point to obtain
/// registered seqgraphic track factories (instances of CLayoutTrack)

#define EXT_POINT__SEQGRAPHIC_LAYOUT_TRACK_FACTORY "seqgraphic_layout_track_factory"

class CRenderingContext;
class ISGDataSourceContext;
class CLayoutTrack;
class CTempTrackProxy;

///////////////////////////////////////////////////////////////////////////////
/// CTrackTypeInfo - holds description of a layout track type.
class  NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT  CTrackTypeInfo
{
public:
    CTrackTypeInfo(const string& id, const string& description)
        : m_Id(id)
        , m_Descr(description)
    {}

    virtual ~CTrackTypeInfo()  {};

    const string& GetId() const { return m_Id; }
    const string& GetDescr() const { return m_Descr; }

protected:
    string  m_Id;
    string  m_Descr;
};


///////////////////////////////////////////////////////////////////////////////
/// ILayoutTrackFactory
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT  ILayoutTrackFactory
{
public:
    /// the tracks holding a list of annotation name and track pair.
    /// for those tracks that don't have annotation concept or no need
    /// to separate annotations, the key can be just the same as
    /// the track's name.
    typedef map< string, CRef<CLayoutTrack> > TTrackMap;

    /// List of annotations with the corresponding titles
    typedef map<string, string> TAnnotNameTitleMap;

    typedef map<string, CRef<CAnnotMetaData> > TAnnotMetaDataList;

    /// extra parameter for initializing a track.
    /// the tracks with no concept of level and annotation
    /// can completely ignore the extra parameters.
    struct SExtraParams
    {
        typedef vector<string>  TAnnots;
        typedef vector<string>  TSubTracks;

        /// layout level that limits feature retrieving used by annotation
        /// selector. By default, m_Level is set to -1, resovle all.
        /// For m_Level >=0, features/data will be fetched up to or on that
        /// level exactly depending on the setting of m_Adative.
        int     m_Level;

        /// Adaptive/Exact selector
        bool    m_Adaptive;

        /// particular annotations the track will be looking at.
        TAnnots m_Annots;

        /// potential track-specific filter.
        /// Any number of fitlers is supported using various logic operators.
        /// The operators include AND, OR, NOT, LIKE, =, !=, (), >, <, <=, and >= .
        /// For example:
        ///       f1=f1_val1 and (f1=f1_val2 or f2>f2_val1) and not f3
        string  m_Filter;

        /// potential track-specific sort_by.
        /// Rules for specifying sort_by:
        ///     1. Only one sorting critero is supported at one time
        ///     2. Sort_by name and values should be in this format:
        ///        sort_by=sort_name[|sort_value1|sort_value2...]
        ///        Sort_name should come at the beginning of the string.
        ///        Sort_values are options, but if there is any, '|' is
        ///        used to separate them from sort_name and from each other.
        string  m_SortBy;


        /// A subkey for creating feature-specific layout track for a
        /// generic layout track type.  The example of usage is the layout
        /// track for a subtype of seq-feat.
        string  m_Subkey;

        /// The requested range we want to look at.
        /// This is optional since the visible range can also be got from
        /// the rendering context for most cases.  The range need to be
        /// specified for projecting features from another feature, such
        /// as alginments.
        TSeqRange m_Range;

        /// Flag indicating if track verification is required.
        /// By default, a data track will be verified to make sure
        /// it indeed exists during track configuration.
        bool      m_SkipGenuineCheck;

        /// when set to true, indicates that that the factory is used by seqconfig
        /// this means that some operations taking a long time and not needed by seqconfig may be skipped
        /// while creating the track
        /// do not use this anywhere but in seqconfig!
        /// a track created with this flag is not usable for loading the track data!
        bool      m_FastConfig;

        /// Flag indicating if checking coverage graph is necessary.
        /// By defult, it is true.
        bool      m_CoverageGraphCheck;

        /// Flag indicating if track data should be checked against declared
        /// track type.
        /// By default, it's true.
        bool      m_DataTypeCheck;

        /// List of subTracks
        TSubTracks m_SubTracks;

        /// map of known correspondencies between annots and full titles
        TAnnotNameTitleMap m_AnnotNameTitleMap;

        /// Already used feature track subkeys
        /// To be used by CAllOtherFeaturesTrack container
        set<string> m_UsedFeatureSubkeys;

        /// Track setting profile for additionial parameters
        string m_TrackProfile;

        /// Track remote path 
        string m_RemotePath;

        SExtraParams()
            : m_Level(-1)
            , m_Adaptive(true)
            , m_SkipGenuineCheck(false)
            , m_FastConfig(false)
            , m_CoverageGraphCheck(true)
            , m_DataTypeCheck(true)
        {}

        SExtraParams(int level, bool adaptive, const TAnnots* annots = NULL,
                     const string& sub_key = NcbiEmptyString,
                     const string& filter = NcbiEmptyString,
                     const string& sort_by = NcbiEmptyString,
                     const string& remote_path = NcbiEmptyString)        
            : m_Level(level)
            , m_Adaptive(adaptive)
            , m_Filter(filter)
            , m_SortBy(sort_by)
            , m_Subkey(sub_key)
            , m_SkipGenuineCheck(false)
            , m_FastConfig(false)
            , m_CoverageGraphCheck(true)
            , m_DataTypeCheck(true)
            , m_RemotePath(remote_path)
        {
            if (annots) m_Annots = *annots;
        }
    };

    /// Help function to find matched annotations.
    /// @param src_annots is a list of known existing annotations served
    ///        as source annotations. Annotation type and track type may
    ///        or may not be set. It works together with target_annots.
    /// @param target_annots is the original wanted annotation. The final
    ///        output annotation will be found within the intersection
    ///        set between src_annots and target_annots
    /// @param annot_type targeted annotation type. If empty, it is ignored.
    /// @param track_type targeted track type. If empty, it is ignored.
    /// @param out_annots output annotation list.
    /// The current implementation only consider named annotations. It
    /// may be extended to include any other annotations.
    static void GetMatchedAnnots(const TAnnotMetaDataList& src_annots,
        const vector<string>& target_annots,
        const string& annot_type, const string& track_type,
        TAnnotNameTitleMap& out_annots);

    /// create a new name based on annotation name and fitler string.
    static string MergeAnnotAndFilter(const string& annot,
                                      const string& filter);

    /// extract anntation name from a source string.
    static string ExtractAnnotName(const string& source);

    /// extract filter name from a source string.
    static string ExtractFilterName(const string& source);

    /// create layout tracks based on input object and extra parameters.
    /// For extra parameters, please refer SExtraParams for details,
    /// especially, about the assumption for level parameter.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const = 0;

    virtual void GetMatchedAnnots(
        const TAnnotMetaDataList& /*src_annots*/,
        const ILayoutTrackFactory::SExtraParams& /*params*/,
        TAnnotNameTitleMap& /*out_annots*/) const {}

    /// duplicate any track setting from a given track instance to a proxy.
    /// by default, it does nothing.
    virtual void CloneTrack(const CLayoutTrack* /*track*/,
        CTempTrackProxy* /*track_proxy*/) const {};

    virtual ~ILayoutTrackFactory() {};

    /// Have any concept of level.
    /// Feature retrieving level (adaptive or exact level) for
    /// annotation selector.
    virtual bool UnderstandLevel() const = 0;

    // does this track support export to ASN1?
    // track factories returning true must be able to accept a NULL as
    // CRenderingContext in a call to CreateTracks()
    // the track constructor must be able to accept a NULL as well
    // and the track should implement IAsn1Generator interface
    virtual bool SupportsExportToAsn1() const { return false; }

    /// Background track initialization.
    /// Since some of track initializations will be time consumming,
    /// but others are not, we need to know if there is a need to do
    /// background track initialization.
    virtual bool NeedBackgroundInit() const = 0;

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// ITrackConfigurable interface for tracks that are configurable.
/// This is currently for sviewer (cgi) purpose.
/// If a track is configurable, it should provide a list of settings
/// (either dropdown list or check box) exposed to users to modify.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT  ITrackConfigurable
{
public:
    typedef map<string, string>  TKeyValuePairs;

    virtual ~ITrackConfigurable() {}
    virtual CRef<objects::CTrackConfigSet>
        GetSettings(const string& profile,
                    const TKeyValuePairs& settings,
                    const CTempTrackProxy* track_proxy) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// ILegendProvider interface for tracks that are have legend
/// This is currently for sviewer (cgi) purpose.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT  ILegendProvider
{
public:
    virtual ~ILegendProvider() {}
    virtual void GetLegend(const CTempTrackProxy* track_proxy, objects::CTrackConfig::TLegend& legend) const = 0;
};



///////////////////////////////////////////////////////////////////////////////
/// IIconProvider interface for tracks providing track icons.
/// For those tracks with special track icons (icon image), the track factory
/// need to implement this interface.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IIconProvider
{
public:
    virtual ~IIconProvider() {}
    virtual void RegisterIconImages() const = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// IAsn1Generator interface for tracks capable of generating an ASN1 seq_entry for their data.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IAsn1Generator
{
public:
    virtual ~IAsn1Generator() {}

    // write track contents into this seq-entry
    virtual void GenerateAsn1(objects::CSeq_entry& seq_entry, TSeqRange range) = 0;

    // if true, this track should be handled by stage 2
    virtual bool BypassStage1() = 0;
    
    // estimate number of features in the specified range
    // returns true if range is acceptable for downloads
    // if false, sMsg will contain a message that can be shown to the user
    virtual bool CheckRange(TSeqRange range, string& sMsg) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// ILayoutTrackHost
/// An interface used for handling GUI-related events, such as layout change
/// event that requires refresh the screen, zoom event that requires zooming
/// to a certain range, and menu event that pops up a context-sensitive menu.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ILayoutTrackHost
{
public:
    virtual ~ILayoutTrackHost() {}

    /// provides mouse coords in OpenGL viewport coord system
    //virtual TVPPoint    LTH_GetVPPosByWindowPos(const wxPoint& pos) = 0;

    /// notifies the host about changes of the track.
    virtual void  LTH_OnLayoutChanged() = 0;

    /// notifies the host we need to zoom on to a range.
    virtual void  LTH_ZoomOnRange(const TSeqRange& range) = 0;

    /// show track-specific context menu.
    virtual void  LTH_PopupMenu(wxMenu* menu) = 0;

    virtual void  LTH_PushEventHandler(wxEvtHandler* handler) = 0;

    virtual void  LTH_PopEventHandler() = 0;

    virtual void  LTH_ConfigureTracksDlg(const string& category) {}

    virtual void ResetSearch() {}
};


///////////////////////////////////////////////////////////////////////////////
/// ITrackContainer
/// The interface for layout track that is also a container of tracks.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ITrackContainer
{
public:
    virtual ~ITrackContainer() {}

    /// Replace the track by the given track order.
    virtual void SetTrack(CLayoutTrack* track, int order, bool shown = true) = 0;

    /// Duplicate an existing child track.
    /// @param id the source track order
    /// @param track_title the track title for the dupliated track
    virtual void CloneTrack(int order, const string& track_tile) = 0;

    /// Rename a child track.
    /// @param id the source track order
    /// @param new_name the track
    virtual void RenameChildTrack(int order, const string& new_name) = 0;

    /// completely remove a track from the hierachical tree.
    /// @param id is the track order.
    /// @param immediately if true the track/proxy will be removed
    ///        from its parent immediately.  Otherwise, it will be
    ///        marked as 'empty' so that the track removal will
    ///        be done from gui registryand.
    virtual void RemoveTrack(int order, bool immediately = true) = 0;

    /// Close a track.
    /// Hide the given track, but track is not removed.
    virtual void CloseTrack(int order) = 0;

    /// move a track up by 1.
    virtual void MoveUp(int order) = 0;

    /// move a track down by 1.
    virtual void MoveDown(int order) = 0;

        /// move a track up by 1.
    virtual void MoveTrackToTop(int order) = 0;

    /// move a track down by 1.
    virtual void MoveTrackToBottom(int order) = 0;

    /// Has any real subtracks.
    /// This differs from Empty() method.  Empty() method only considers
    /// the visible tracks, but HasSubtracks() will consider the ones
    /// that are off.
    virtual bool HasSubtracks() const = 0;

    /// Will have no subtrack for ever.
    /// There will be no any potential subtracks?
    virtual bool NoSubtrackEver() const = 0;
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK__HPP
