#ifndef GUI_WIDGETS_ALNMULTI___ALNMULTI_WIDGET__HPP
#define GUI_WIDGETS_ALNMULTI___ALNMULTI_WIDGET__HPP

/*  $Id: alnmulti_widget.hpp 44954 2020-04-27 17:57:36Z evgeniev $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/aln_multiple/alnmulti_model.hpp>
#include <gui/widgets/aln_multiple/alnmulti_pane.hpp>

#include <gui/utils/command.hpp>
#include <gui/utils/menu_item.hpp>

#include <gui/print/print_options.hpp>

#include <objects/seqloc/Seq_loc.hpp>

#include <util/range_coll.hpp>

class wxMenu;
class wxFileArtProvider;

BEGIN_NCBI_SCOPE

/// Definitions for Multiple Alignment Widget commands
///
enum EAlnMultiCommands {
    eCmdSetSelMaster = eBaseCmdLast + 1, // make selected sequence Master
    eCmdUnsetMaster,

    eCmdMarkSelected,
    eCmdUnMarkSelected,
    eCmdUnMarkAll,

    eCmdHideSelected,
    eCmdUnhideSelected,
    eCmdShowOnlySelected,
    eCmdShowAll,

    eCmdMoveSelectedUp,

    eCmdRebuildAlignment
};


class CUICommandRegistry;
class IAlnMultiDataSource;
class CGuiRegistry;

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

////////////////////////////////////////////////////////////////////////////////
/// class CAlnMultiWidget
///
/// CAlnMultiWidget is a reusable component for visual presntations of multiple
/// alignments. CAlnMultiWidget works as Mediator to organize interaction of its
/// internal components and to support communication with external components.
/// Data Source is an external component that serves as a source of alignment
/// data. CAlnMultiWidget builds a visual Model object around provided Data
/// Source and uses CAlnMultiPane component to represent this model.
/// CAlnMultiWidget provides API for external components allowing for
/// executing commands and retrieving state information such as selections.

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiWidget
    : public CGlWidgetBase,
      public IAlnMultiPaneParent,  // Widget-Pane relashionship
      public CDataChangeNotifier::IListener,
      public CScoreCache::IListener
{
    DECLARE_EVENT_TABLE()
public:
    typedef CAlnMultiModel::TNumrow     TNumrow;
    typedef int TLine;
    typedef list<objects::CSeq_loc*>        TPSeqLocList;
    typedef CRangeCollection<TSeqPos>       TRangeColl;
    typedef vector<CConstRef<objects::CSeq_id> >     TRSeqIdVector;

    static  void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    CAlnMultiWidget(wxWindow* parent,
                    wxWindowID id = wxID_ANY,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0,
                    const wxString& name = wxT("panel"));
    virtual ~CAlnMultiWidget();

    /// @name CGlWidgetBase virtual functions
    /// @{
    virtual CGlPane&    GetPort();
    virtual const CGlPane&    GetPort() const;
    virtual void    x_SetPortLimits(void) {};
    virtual CGlWidgetPane* x_GetPane();

    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    virtual void    SetDataSource(IAlnMultiDataSource* p_ds, objects::CScope* scope);

    /// performs initialization, creates Model and Controls. Call Create()
    /// right after constructing CAlnMultiWidget; assumes ownership of the
    /// model. If model == NULL, creates a default model
    virtual void    CreateWidget(CAlnMultiModel* model=NULL);

    virtual CAlnMultiModel* GetModel();
    virtual const CAlnMultiModel* GetModel() const;

    virtual void    SetStyleCatalog(const CRowStyleCatalog* catalog);
    virtual const CWidgetDisplayStyle*      GetDisplayStyle() const;

    /// makes specified rows visible/hidden, other rows aren't affected
    virtual void    SetVisible(const vector<TNumrow>& rows, bool b_visible,
                               bool b_invert_others = false);

    virtual void    Select(const vector<TNumrow>& rows, bool b_select,
                               bool b_invert_others = false);

    /// @name IAlnMultiRenderContext implementation
    /// @{
    virtual bool    IsRendererFocused();
    /// @}

    virtual IAlnMultiDataSource*  GetDataSource();

    /// @name IAlnMultiPaneParent implementation
    /// @{
    virtual const IAlnMultiDataSource*  GetDataSource() const;
    virtual TNumrow     GetRowNumByLine(TLine line) const;
    virtual int         GetLineByRowNum(TNumrow Row) const;

    virtual void    OnChildResize();
    virtual void    SetScaleX(TModelUnit scale_x, const TModelPoint& point);
    virtual void    ZoomRect(const TModelRect& rc);
    virtual void    ZoomPoint(const TModelPoint& point, TModelUnit factor);
    virtual void    Scroll(TModelUnit d_x, TModelUnit d_y);
    virtual void    OnRowChanged(IAlignRow* p_row);

    virtual void    OnColumnsChanged();
    /// @}

    /// @name CUIDataSource::IListener implementation
    /// @{
    virtual void    OnDSChanged(CDataChangeNotifier::CUpdate& update);
    /// @}

    /// @name CAlnMultiModel::IListener implementation
    /// @{
    virtual void    OnScoringProgress(float progress, const string& msg);
    virtual void    OnScoringFinished();
    /// @}

    void    OnContextMenu(wxContextMenuEvent& event);

    /// @name Command Handlers
    /// @{
    void    OnSavePdf(wxCommandEvent & evt);
    void    OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& evt);
    void    OnSaveSvg(wxCommandEvent & evt);
    void    OnEnableSaveSvgCmdUpdate(wxUpdateUIEvent& evt);

    void    OnZoomSequence(wxCommandEvent& event);
    void    OnZoomSelection(wxCommandEvent& event);
    void    OnUpdateZoomSelection(wxUpdateUIEvent& event);

    void    OnMakeSelectedRowMaster(wxCommandEvent& event);
    void    OnUpdateMakeSelectedRowMaster(wxUpdateUIEvent& event);
    void    OnUnsetMaster(wxCommandEvent& event);
    void    OnUpdateUnsetMaster(wxUpdateUIEvent& event);
    void    OnMakeConsensusRowMaster(wxCommandEvent& event);
    void    OnUpdateMakeConsensusRowMaster(wxUpdateUIEvent& event);

    void    OnResetSelection(wxCommandEvent& event);

    void    OnMarkSelected(wxCommandEvent& event); /// mark selected regions of selected rows
    void    OnUnMarkSelected(wxCommandEvent& event); /// unmark selected regions of selected rows
    void    OnUpdateMarkSelected(wxUpdateUIEvent& event);
    void    OnUnMarkAll(wxCommandEvent& event); // remove all marks

    void    OnHideSelected(wxCommandEvent& event); /// hide selected rows
    void    OnUnhideSelected(wxCommandEvent& event); /// hide selected rows
    void    OnUpdateUnhideSelected(wxUpdateUIEvent& event);
    
    void    OnHasSelected(wxUpdateUIEvent& event);
    void    OnShowOnlySelected(wxCommandEvent& event); /// hide rows that are not selected
    void    OnShowAll(wxCommandEvent& event); /// show all hidden rows

    void    OnMoveSelectedUp(wxCommandEvent& event);

    void    OnSettings(wxCommandEvent& event);
    void    OnUpdateSettings(wxUpdateUIEvent& event);
    /// @}

    /// @name Command handler for Scoring methods management
    /// @{
    void OnShowMethodDlg(wxCommandEvent& event);
    void OnUpdateShowMethodDlg(wxUpdateUIEvent& event);

    void OnDisableScoring(wxCommandEvent& event);
    void OnUpdateDisableScoring(wxUpdateUIEvent& event);

    void OnMethodProperties(wxCommandEvent& event);
    void OnUpdateMethodProperties(wxUpdateUIEvent& event);

    void OnScoringMethod(wxCommandEvent& event);
    void OnUpdateScoringMethod(wxUpdateUIEvent& event);

    void OnSetAsDefaultMethod(wxCommandEvent& event);
    /// @}

    /// @name Selection handling
    /// @{
    void    GetSelectedIds(TRSeqIdVector& ids) const;
    void    SetSelectedIds(const TRSeqIdVector& ids, objects::CScope& scope);
    void    GetSelectedRows(vector<TNumrow>& rows);

    const   TRangeColl& GetSelection(void) const;
    void    SetSelection(const TRangeColl& coll);
    void    GetMarks(TPSeqLocList& ls_locs) const;
    void    GetObjectSelection(TConstObjects &objs) const;
    bool    HasSelectedObjects() const;
    /// @}

    virtual void    UpdateSortOrder();
    virtual void    UpdateOnScoringChanged();

    void    ZoomToSelection();

    void    Print(const string& filename);
    void    UpdateView();

    bool    SaveVectorImage(CPrintOptions::EOutputFormat format, string const &path, string const &file_name, bool show_header, bool simplified);

protected:
    void    x_CreatePane(void);

    /// factory method creating and initializing a default model
    virtual CAlnMultiModel* x_CreateModel();

    //virtual void    x_CreateControls(void); ///factory method

    /// @name Update handlers
    /// Functions in this group are called by the widget when something changes.
    /// Each functions performs minimal set of operations needed to keep widget
    /// up-to-date.
    /// @{
    virtual void    x_UpdateOnDataChanged(bool b_reset = true);

    virtual void    x_UpdateOnRowHChanged(void);
    virtual void    x_UpdateOnSwitchMaster(TNumrow NewMasterRow);
    virtual void    x_UpdateOnZoom(void);
    virtual void    x_UpdateOnStylesChanged(void);
    virtual void    x_OnResize(void);
    /// @}

    //virtual void    x_SetScrollbars(void);
    virtual void    x_AdjustVisible(bool b_reset);
    virtual void    x_RedrawControls(void);

    virtual void    x_UpdateStyleColumns();

    void    x_ZoomToRange(TSeqPos from, TSeqPos to);

    void    x_MakeVisibleHorz(TSeqPos pos);
    void    x_MakeVisibleHorz(TSeqPos from, TSeqPos to);

    virtual wxMenu*  x_CreatePopupMenu();

    virtual string  x_GetCurrentMethodName() const;
    virtual void    x_SetScoringMethod(const string& method_name, bool save);
    virtual void x_CreateScoringMethodsMenu(wxMenu& color_menu);

    virtual string* x_GetDefMethodName();
    virtual string  x_GetScoringMRUListName();

    void x_GetMatchingHiddenRows(vector<TNumrow>& rows);
    
    void x_OnSaveVectorImage(CPrintOptions::EOutputFormat format);
protected:
    typedef map<int, string>   TCmdToName;

    IAlnMultiDataSource*    m_DataSource; /// source of alignment data

    CAlnMultiModel* m_Model; /// representation of alignment data

    CAlnMultiPane*  m_AlignPane; /// widget rendering data and handling events

    TCmdToName  m_CmdToName;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_ALNMULTI___ALNMULTI_WIDGET__HPP
