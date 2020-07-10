#ifndef GUI_CORE___DATA_MINING_PANEL_HPP
#define GUI_CORE___DATA_MINING_PANEL_HPP

/*  $Id: data_mining_panel.hpp 33488 2015-08-03 20:34:48Z evgeniev $
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
 * Authors:  Andrey Yazhuk, Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>
#include <gui/core/dm_search_tool.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/utils/command.hpp>
#include <misc/xmlwrapp/xmlwrapp.hpp>

#include <map>

//TODO - remove
#include <gui/core/loading_app_job.hpp>
// TODO

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/menu.h>
#include <wx/animate.h>
#include <wx/statusbr.h>
#include <wx/timer.h>

class wxBoxSizer;
class wxButton;
class wxHyperlinkEvent;
class wxToolBar;

BEGIN_NCBI_SCOPE

class CDataMiningService;
class CDataMiningView;
class ISelectionClient;
class IWorkbench;


///////////////////////////////////////////////////////////////////////////////
/// CDataMiningPanel
class NCBI_GUICORE_EXPORT CDataMiningPanel
:   public wxPanel,
    public IRegSettings,
    public IDMSearchFormController,
    public CEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    enum ECommands   {
        eCmdAddToNew=999,
        eCmdCreateFeature,
        eCmdAddToSel,
        eCmdShowPanel,
        eCmdRangeLimit,
        eCmdFilter,
        eCmdStart,
        eCmdStop,   
        eCmdCopy,
        
        eCmdCopy1,
        eCmdCopy2,
        eCmdCopy3,
        eCmdCopy4,
        eCmdCopy5,
        eCmdCopy6,
        eCmdCopy7,
        eCmdCopy8,
        eCmdCopy9,
        eCmdCopy10,

        eCmdNoMoreCommands
    };

    static const int scm_ModeCmd = 13000;

    enum EDisplayMode    {
        eInvalidDisplayMode = -1,
        eTable,
        eBriefText,
        eText
    };

public:
    CDataMiningPanel(CDataMiningView* view);
    virtual ~CDataMiningPanel();

    virtual void Create(wxWindow* parent,
                        wxWindowID id = wxID_ANY,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize);
    virtual bool Destroy();

    /// Initialises member variables
    void    Init();

    /// Creates the controls and sizers
    void    CreateControls();

    void    SetService(CDataMiningService* service);
    void    Update();
    void    GetSelection(TConstScopedObjects& buf) const;

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& reg_path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    virtual void AppendMenuItems( wxMenu& aMenu );

    /// @name Command handlers
    /// @{
    void    OnContextMenu(wxContextMenuEvent& event);
    void    OnTargetChanged(wxCommandEvent& event);
    void    OnButtonClicked(wxCommandEvent& event);
    void    OnSelectionChanged(wxListEvent& event);
    void    OnHyperlinkctrlClicked(wxHyperlinkEvent& event);
    void    OnUpdateNeeded(wxCommandEvent& event);
    void    OnColumnEndDrag(wxListEvent & event);   
    void    OnListItemActivate(wxListEvent & event);
    void    OnAddToNew(wxCommandEvent& event);
    void    OnCreateFeature(wxCommandEvent& event);
    void    OnMenuEvent(wxCommandEvent& event);
    void    OnSearchButton(wxCommandEvent& event);
    void    HandleTool(wxCommandEvent& event);  
    void    OnCustomCopy(wxCommandEvent& event);  

    void    OnUpdateCreateFeature(wxUpdateUIEvent& event);
    void    OnEnableCmdRclick(wxUpdateUIEvent& event); 
    /// @}

    /// @name Timer handlers
    /// @{
    void    OnAutorun(wxTimerEvent& event);
    /// @}

    /// @name IDMSearchFormController
    /// @{
    virtual CDataMiningService*  GetDataMiningService();
    virtual void    OnSearchFinished(CDMSearchResult& result);
    virtual void    OnSearchCanceled();
    virtual void    OnSearchFailed(const string& error);
    virtual void    OnSearchEnabled(bool benable);
    virtual void    OnSearchProgress(CDMSearchResult& result,
                                     const string& prg_text);
    /// @}
    
    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    SelectToolByName(const string& tool_name);  

    void            SetRange(objects::CSeq_loc::TRange & range);

protected:
    EDisplayMode    x_GetModeByCommand(TCmdID cmd);

    void    x_Init();
    void    x_InitCurrentWidget();
    void    x_SearchInProgress(bool bYes = true);
    void    x_UpdateWidgetAndTool();
    void    x_UpdateWidget();
    void    x_ExecuteSearch();
    void    x_CancelSearch();
    void    x_RestartSearch();
    void    x_AddToProject(TConstScopedObjects & obj, string label = "", string comment = "");
    void    x_CreateFeature();
    void    x_SyncTableSettings(bool bWrite) const;
    void    x_TargetChanged();

protected:
    typedef map<int, ISelectionClient*> TIndexToClient;

    static bool m_RegisteredIcons;

    CDataMiningView*    m_View;
    CDataMiningService* m_Service;

    IWorkbench* m_Workbench;
    bool        m_bInitialized;

    string  m_RegPath; // path to the setting in CGuiRegistry
    
    string  m_CurrToolName;

    string  m_MainValue;
    
    TConstScopedObjects m_SelObjects;

    wxChoice* m_TargetChoice;
        
    TIndexToClient m_IndexToClient; // m_TargetChoice item index -> ISelectionClient*
    CRef<objects::CScope> m_CurrentScope;

    EDisplayMode        m_DisplayMode;
    CObjectListWidget*  m_ListWidget;
    wxPanel*        m_ParentPanel;
    wxTextCtrl*     m_Text;
    wxBoxSizer*     m_FormSizer;
    wxToolBar*      m_ToolBar;
    CFixGenericListCtrl m_FixGenericListCtrl;

    CIRef<IDMSearchForm>  m_Form;
    wxButton*   m_StartBtn;
    wxButton*   m_StopBtn;
    wxAnimationCtrl*    m_Animation;
    wxStatusBar* m_Status;
    wxTimer m_Timer;
    bool mf_Reentry;

    // saved settings
    list<string>  m_Dict;

    // search range (zero length when whole)
    objects::CSeq_loc::TRange        m_Range;

    int m_ToolSelIndex;

    bool m_SearchInProgress;
    bool m_NeedUpdate;
};


///////////////////////////////////////////////////////////////////////////////
/// CGBankLoadingJob

// TODO - this is temporrary, this class does not belong here !!!

BEGIN_SCOPE(objects);
    class CEntrez2_docsum;
END_SCOPE(objects);


class  NCBI_GUICORE_EXPORT CGBankLoadingJob :
    public CDataLoadingAppJob
{
public:
    CGBankLoadingJob(TConstScopedObjects & objs,
                     string label ="",
                     string comment = "");

    virtual string GetDescr() const;
    const TConstScopedObjects & Getobjects() const { return m_Objs;}

protected:
    virtual void        x_CreateProjectItems();
    CRef<CSerialObject> x_Convert(const xml::node &doc_sum, const string& db_name);

protected:
    TConstScopedObjects     m_Objs; 
    string                  m_Label;
    string                  m_Comment;
};


END_NCBI_SCOPE

#endif  // GUI_CORE___DATA_MINING_PANEL_HPP
