#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_WIDGET__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_WIDGET__HPP

/*  $Id: seqgraphic_widget.hpp 44907 2020-04-14 13:03:07Z asztalos $
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
 *   Graphic Sequence Widget
 *
 */

#include <corelib/ncbiobj.hpp>

#include <util/range_coll.hpp>

#include <gui/gui.hpp>
#include <gui/opengl/gltypes.hpp>
#include <gui/utils/command.hpp>
#include <gui/utils/menu_item.hpp>
#include <gui/print/print_options.hpp>
#include <gui/objutils/ui_data_source_notif.hpp>

#include <gui/widgets/seq/sequence_goto_dlg.hpp>
#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/wx/ibioseq_editor.hpp>

class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;

class CObjectIndex;
class CSeqGraphicConfig;
class CSeqGraphicPane;
class CSGSequenceDS;
class CLayout;
class CPrintOptions;
class CLayoutTrack;
class CwxMarkerInfoDlg;
class CSeqMarkerEvent;
class CSeqMarkerDlgEvent;
class CFeaturePanel;

class ICommandProccessor;

BEGIN_SCOPE(objects)
    class CSeq_loc;
END_SCOPE(objects)


/// Definitions for Sequence Graphical Widget commands
///
enum ESeqGraphicsCommands {
    eCmdZoomObject = eBaseCmdLast + 6000,
    eCmdPrevSplice,
    eCmdNextSplice,
    eCmdAdjustRuler,
    eCmdSequenceLabel,
    eCmdSetSeqStart,
    eCmdSetMarkerDlg,
    eCmdSetMarkerForSelDlg,
    eCmdRenameMarkerDlg,
    eCmdModifyMarkerDlg,
    eCmdMarkerDetailsDlg,
    eCmdZoomAtMarker,
    eCmdSetSeqStartMarker,
    eCmdGoToMarker,
    eCmdRemoveMarker,
    eCmdResetSeqStart,
    eCmdResetMarker,
    eCmdResetSeqMarker,
    eCmdSetMarker,
    eCmdSetSwitchPoint,
    eCmdHairlineOptions,
    eCmdAnnotConfig,
    eCmdConfigChanged,
    eCmdChangeViewMode,
    eCmdFlipStrand,
    eCmdStrandFlipped,

    /// track-related coommands
    eCmdLoadDefaultTracks,
    eCmdCollapseAllTracks,
    eCmdExpandAllTracks,
    eCmdShowAllTracks,
    eCmdHideAllTracks,
    eCmdCollapseAllSubtracks,
    eCmdExpandAllSubtracks,
    eCmdCloneTrack,
    eCmdRemoveTrack,
    eCmdAddTrack,
    eCmdMoveTrackToBottom,
    eCmdMoveTrackToTop,
    eCmdRenameTrack,
    eCmdTrackSettings,
    eCmdHideTrack,
    eCmdRemoveTrackFromOverlay,

    /// dbSNP-specific commands
    eCmdSnpWebLinkout,
    eCmdSnpTable,
    eCmdSnpFilter,
    eCmdCreateSnpFilteredTrack,
    eCmdStudyDisplayToggle // toggle how study graph is displayed
};

enum {
    ID_GLCHILDPANE = wxID_HIGHEST + 400,
    ID_VSCROPLLBAR,
    ID_HSCROPLLBAR
};

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSeqGraphicWidget
    : public CGlWidgetBase
    , public IBioseqEditorCB
    //, public CCommandTarget
    //, public CScrollbar::IListener
{
    DECLARE_EVENT_TABLE()

public:
    enum FZoomFlag {
        fAddMargins = 1 << 0,
        fSaveRange = 1 << 1
    };

    typedef int TZoomFlag;

    typedef CRangeCollection<TSeqPos> TRangeColl;

    static void RegisterCommands(CUICommandRegistry& cmd_reg,
        wxFileArtProvider& provider);

    CSeqGraphicWidget(wxWindow* parent,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxTAB_TRAVERSAL);

    //CSeqGraphicWidget(int x, int y, int w, int h, const char* label = NULL);
    virtual ~CSeqGraphicWidget();

    void InitBioseqEditor(ICommandProccessor& cmdProcessor);
    virtual CGlPane&    GetPort();
    virtual const CGlPane&    GetPort() const;

    void OnDataChanging();
    void OnDataChanged();

    /// access the data source
    void SetInputObject(SConstScopedObject& obj);
    void InitDataSource(SConstScopedObject& obj);
    CSGSequenceDS* GetDataSource(void) const;

	CFeaturePanel* GetFeaturePanel();

    void ConfigureTracksDlg();

    bool FindText(const string &text, bool match_case);
    void ResetSearch();

    void SetExternalGlyphs(const CSeqGlyph::TObjects& objs);

    void ZoomOnRange(TSeqRange range, TZoomFlag flag);

    void SetHorizontal(bool b_horz, bool b_flip);
    bool IsHorizontal() const;
    bool IsFlipped() const;
    //bool IsSequenceLabel();


    /// returns the visible sequence range
    TModelRange GetVisibleRange(void) const;
    TSeqRange GetVisibleSeqRange() const;

    /// indicates visible sequence range has changes
    bool VisibleRangeChanged() const;
    void ResetVisibleRangeChanged();

    /// set/clear selections
    void SelectObject(const CObject* obj, bool verified);
    void SelectSeqLoc(const objects::CSeq_loc* loc); // TO DO - clarify semantics
    void ResetSelection();
    void ResetRangeSelection();
    void ResetObjectSelection();

    /// retrieve selected objects from this widget
    void GetObjectSelection(TConstObjects& objs) const;

    /// retrieve set of intervals selected on the sequence
    const TRangeColl&   GetRangeSelection(void) const;
    void                SetRangeSelection(const TRangeColl& ranges);
    void                GetRangeOrObjectSelection(TRangeColl& ranges) const;

    /// Here is the config object you are going to use to configure yourself.
    /// make sure all your children who need it have references to it.
    CRef<CSeqGraphicConfig> GetConfig();
    /// The config object has changed. Do what is necessary.
    void UpdateConfig();


    // Popup
    //void    OnShowPopup(int area);
    //void    SetViewPopupMenuItems(CMenuItem * itm_view, CMenuItem * itm_default);
    //void    OnEditFeature(ncbi::CEvent* evt);

    void SetScaleX(TModelUnit scale, const TModelPoint& point);
    void UpdateHeight(TModelUnit height);

    /// @name Event Handling
    /// @{
    void OnSize(wxSizeEvent& event);
    void OnScrollEnd(wxScrollEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnSavePdf(wxCommandEvent & evt);
    void OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& evt);
    void OnSaveSvg(wxCommandEvent & evt);
    void OnEnableSaveSvgCmdUpdate(wxUpdateUIEvent& evt);

    typedef CViewEvent TEvent;

    void OnZoomObject(wxCommandEvent& event);
    void OnPrevSplice(wxCommandEvent& event);
    void OnNextSplice(wxCommandEvent& event);
    void OnLoadDefaultTracks(wxCommandEvent& event);
    void OnCollapseAllTracks(wxCommandEvent& event);
    void OnExpandAllTracks(wxCommandEvent& event);
	void OnShowAllTracks(wxCommandEvent& event);
    void OnHideAllTracks(wxCommandEvent& event);
	void OnCollapseAllSubtracks(wxCommandEvent& event);
	void OnExpandAllSubtracks(wxCommandEvent& event);
	void OnMoveTrackToTop(wxCommandEvent& event);
	void OnMoveTrackToBottom(wxCommandEvent& event);

    void OnCloneTrack(wxCommandEvent& event);
	void OnRemoveTrack(wxCommandEvent& event);
	void OnAddTrack(wxCommandEvent& event);
    void OnRenameTrack(wxCommandEvent& event);

    void OnZoomSelection(wxCommandEvent& event);
    void OnZoomSequence(wxCommandEvent& event);

    // update
    void OnUpdateZoomSelection(wxUpdateUIEvent& event);
    void OnUpdateZoomObject(wxUpdateUIEvent& event);

    void OnSetMarker(wxCommandEvent& event);
    void OnSetSeqMarkerDlg(wxCommandEvent& event);
    void OnSetSeqMarkerForSelDlg(wxCommandEvent& event);
    void OnUpdateSetSeqMarkerForSelDlg(wxUpdateUIEvent& event);
    void OnRenameMarkerDlg(wxCommandEvent& event);
    void OnModifyMarkerDlg(wxCommandEvent& event);
    void OnMarkerDetailsDlg(wxCommandEvent& event);
    void OnZoomAtMarker(wxCommandEvent& event);
    void OnGoToMarker(wxCommandEvent& event);
    void OnRemoveMarker(wxCommandEvent& event);
    void OnRemoveAllMarkersCmd(wxCommandEvent& event);
    void OnMarkerChanged();
    void OnResetSeqStart(wxCommandEvent& event);
    void OnSetSwitchPoint(wxCommandEvent& event);

    void OnSetSeqStart(wxCommandEvent& event);
    void OnSetSeqStartMarker(wxCommandEvent& event);

    void OnAdjustRuler(wxCommandEvent& event);
    void OnUpdateAdjustRuler(wxUpdateUIEvent& event);

    void OnHairlineOptions(wxCommandEvent& event);
    void OnSeqGotoCmd(wxCommandEvent& event);
	//void OnViewProperties(wxCommandEvent& event);


    // Back / Forward
    void OnUpdateGoBack(wxUpdateUIEvent& event);
    void OnUpdateGoForward(wxUpdateUIEvent& event);
    void OnGoBack(wxCommandEvent& event);
    void OnGoForward(wxCommandEvent& event);

    void OnVertical(wxCommandEvent& event);
    void OnUpdateVertical(wxUpdateUIEvent& event);

    void OnUpdateSetSwitchPoint(wxUpdateUIEvent& event);

    void OnFlipStrands(wxCommandEvent& event);
    void OnUpdateFlipStrands(wxUpdateUIEvent& event);

    //void OnEditUndo();
    //void OnEditRedo();
    void OnEditCut(wxCommandEvent& event);
    void OnEditCopy(wxCommandEvent& event);
    void OnEditPaste(wxCommandEvent& event);
    void OnEditSelection();
    void OnDeleteSelection();
    void OnDeleteSelectionAndOrphaned();

    void OnObjToRangeSelection();
    void OnRangeToObjSelection();

    void OnSeqGoto(CSequenceGotoEvent& event);
    void OnSeqGotoClose(CSequenceGotoEvent& event);
    void OnRemoveAllMarkers(CSeqMarkerDlgEvent& event);
    void OnRemoveMarker_Info(CSeqMarkerEvent& event);
    void OnMarkerInfoDlgClose(CSeqMarkerDlgEvent& event);

    /// @name event handler for sticky tooltip
    /// @{
    void OnSearchTip(wxCommandEvent& evt);
    void OnZoomTip(wxCommandEvent& evt);
    void OnInfoTip(wxCommandEvent& evt);
    void OnTipActivated(wxCommandEvent& evt);
    void OnTipDeactivated(wxCommandEvent& evt);
    void OnMoveTip(wxCommandEvent& evt);
    void OnTipAdded(wxCommandEvent& evt);
    void OnTipRemoved(wxCommandEvent& evt);
    /// @}

    /// Get markers as a string.
    /// The syntax for markers string is: pos|name|color[,pos|name|color].
    /// Name and color are optional.
    string GetMarkers() const;
    void SetMarkers(const string& markers);

    void SaveViewSettings();

    /// check if there is any visible range or marker change 
    bool IsDirty() const;
    void SetDirty(bool flag); 

    // IBioseqEditorCB
    virtual void GetItemSelection(TFlatItemList&) {}
    virtual void GetCurrentSelection(TEditObjectList& objs);    
    virtual objects::CBioseq_Handle GetCurrentBioseq();
    virtual wxString GetWorkDir() const {return m_WorkDir;}
    virtual bool MayCreateFeatures() const override { return false; }

    void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }
protected:
    /// Creates Pane, Scrollbars and other child widgets, called from Create()
    //virtual void    x_CreateControls();

    /// @name CGlWidgetBase overridables
    /// @{
    virtual void    x_CreatePane();
    virtual CGlWidgetPane*  x_GetPane();
    virtual void    x_Update();
    virtual void    x_UpdatePane();
    virtual void    x_SaveStates();
    virtual void    x_SetPortLimits();
    virtual void    x_UpdateOnVScroll();
    /// @}

    /// @name CScrollbar::IListener implementation
    /// @{gg
    //virtual void OnScroll(CScrollbar* scrollbar);
    //virtual void OnEndScroll(CScrollbar* scrollbar);
    /// @}

private:
    void x_SaveDefaultScoringMethod(string name);
    /// forbidden
    //CSeqGraphicWidget(const CSeqGraphicWidget&);
    //CSeqGraphicWidget& operator=(const CSeqGraphicWidget&);
    wxMenu*  x_CreatePopupMenu(int area);
    CLayoutTrack* x_GetCurrentHitTrack();
    void x_RemoveAllMarkers();
    void x_UpdateMarkerInfo();
    void x_SaveVectorImage(CPrintOptions::EOutputFormat format);

protected:
    CGlPane m_Port;

private:
    typedef map<int, string>   TCmdToName;

    //friend class CSeqGraphicPane;
    //CScrollbar*      m_ScrollX;
    //CScrollbar*      m_ScrollY;

    //CSeqGraphicPane* m_SeqGraphicPane;
    unique_ptr<CSeqGraphicPane>   m_SeqGraphicPane;

    //CMenuItem*       m_pViewPopupItems;
    //CMenuItem*       m_pViewDefaultPopupItems;

    TCmdToName       m_CmdToName;


    CSequenceGotoData m_SequenceGotoData;
    CSequenceGotoDlg* m_SequenceGotoDlg;
    CwxMarkerInfoDlg* m_MarkerInfoDlg;

    // flag indicating if any change on visible range or marker.
    bool    m_Dirty;
    
    CIRef<IBioseqEditor> m_BioseqEditor;
    wxString m_WorkDir;
    int m_context_x;
    int m_context_y;
};

///////////////////////////////////////////////////////////////////////////////
/// CSeqGraphicWidget inline methods
///
inline 
TModelRange CSeqGraphicWidget::GetVisibleRange() const
{
    const TModelRect& mrc = m_Port.GetVisibleRect();
    return TModelRange(mrc.Left(), mrc.Right());
}

inline 
TSeqRange CSeqGraphicWidget::GetVisibleSeqRange() const
{
    const TModelRect& mrc = m_Port.GetVisibleRect();
    return TSeqRange(TSeqPos(mrc.Left()), TSeqPos(mrc.Right()));
}



END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_WIDGET__HPP */
