#ifndef GUI_FRAMEWORK___TASK_VIEW__HPP
#define GUI_FRAMEWORK___TASK_VIEW__HPP

/*  $Id: task_view.hpp 31807 2014-11-17 19:32:54Z katargir $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/gui_export.h>

#include <gui/framework/view.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/widgets/wx/wm_client.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/command.hpp>
#include <gui/utils/extension.hpp>

#include <deque>

#include <wx/panel.h>
#include <wx/timer.h>

BEGIN_NCBI_SCOPE

class CwxTableListCtrl;


///////////////////////////////////////////////////////////////////////////////
/// CTaskViewModel - table model for Task View.

class CTaskViewModel : public CwxAbstractTableModel
{
public:
    typedef CAppTaskService::TTaskRef   TTaskRef;
    typedef CAppTaskService::TRecordRef    TRecordRef;
    typedef CAppTaskService::TRecRefVec TRecRefVec;

    enum EColumns {
        eNone = -1,
        eDescription,
        eState,
        eStatus,
        eTime,
        eMaxCols    /// this must be the last!
    };

public:
    CTaskViewModel(CAppTaskService* task_srv);

    virtual int GetNumRows() const;
    virtual int GetNumColumns() const;
    virtual wxVariant GetValueAt(int row, int col) const;
    virtual wxString GetColumnType(int col) const;
    virtual wxString GetColumnName(int col) const;

    void    ShowTasksByState(IAppTask::ETaskState state, bool show, bool up = true);
    bool    IsStateShown(IAppTask::ETaskState state);

    TTaskRef    GetTask(int row);

    void    UpdateTable();

protected:
    typedef vector<TTaskRef>    TTaskRefVec;

    void    x_AddTasks(TRecRefVec& tasks);

protected:
    string m_Names[eMaxCols];
    CTimeFormat m_TimeFormat;

    CAppTaskService*    m_TaskService;
    vector<bool>        m_Filter;
    vector<TRecordRef>  m_Records;
};


///////////////////////////////////////////////////////////////////////////////
/// CTaskTablePanel - panel that owns a Table and a Table Model, this is the
/// window for CTaskView.
class NCBI_GUIFRAMEWORK_EXPORT CTaskTablePanel :
    public wxPanel,
    public IRegSettings
{
    DECLARE_EVENT_TABLE()
public:
    typedef CAppTaskService::TTaskRef   TTaskRef;

    enum ECommands   {
        eCmdDetails = 20000, //TODO use Actions,
        eCmdCancel
    };

    CTaskTablePanel();
    virtual ~CTaskTablePanel();

    /// Initializes member variables
    virtual void    Init();

    /// Creates the controls and sizers
    virtual void    CreateControls();


    virtual void    Create(wxWindow* parent,
                           wxWindowID id = wxID_ANY,
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize);

    virtual void    CreateModel(CAppTaskService* task_srv);
    virtual void    DestroyModel();
    virtual void    UpdateTable();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    virtual void AppendMenuItems( wxMenu& aMenu );

    /// @name Event handers
    /// @{
    void    OnContextMenu(wxContextMenuEvent& event);
    void    OnItemActivated(wxListEvent& event);
    void    OnShowDetailsDialog(int vis_index);

    void    OnCancel(wxCommandEvent& event);
    void    OnDetails(wxCommandEvent& event);

    void    OnUpdateCancel(wxUpdateUIEvent& event);
    void    OnUpdateDetails(wxUpdateUIEvent& event);

    void    OnEventTimer(wxTimerEvent& event);
    /// @}

protected:
    void    x_GetSelectedTasks(vector<TTaskRef>& tasks);
    void    x_SetSelectedTasks(vector<TTaskRef>& tasks);

protected:
    CAppTaskService*    m_TaskService;
    string  m_RegPath;
    wxTimer m_EventTimer;

    CTaskViewModel*     m_Model;
    CwxTableListCtrl*   m_Table;
};

///////////////////////////////////////////////////////////////////////////////
/// CTaskView - IView, that uses CTaskTablePanel as its window.
/// Displays tasks executing in CAppTaskService.

class NCBI_GUIFRAMEWORK_EXPORT CTaskView :
    public CObjectEx,
    public IView,
    public IWMClient,
    public CEventHandler,
    public IRegSettings
{
    friend class CTaskViewFactory;
    DECLARE_EVENT_MAP();
public:
    CTaskView();
    virtual ~CTaskView();

    /// @name IView interface implementation
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow* GetWindow();
    virtual wxEvtHandler* GetCommandHandler() { return GetWindow()->GetEventHandler(); }
    virtual string  GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const;
    virtual string  GetIconAlias() const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor(const CRgbaColor& color);
    virtual const wxMenu*  GetMenu();
    virtual void    UpdateMenu(wxMenu& root_menu);
    virtual CFingerprint  GetFingerprint() const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    void    x_OnUpdate(CEvent*);

    static CViewTypeDescriptor m_TypeDescr;

    CAppTaskService*    m_TaskService;
    string m_RegPath;

    CTaskTablePanel*    m_Panel;
};

///////////////////////////////////////////////////////////////////////////////
/// CTaskViewFactory
class NCBI_GUIFRAMEWORK_EXPORT CTaskViewFactory :
    public CObject,
    public IExtension,
    public IViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IViewFactory interface implementation
    /// @{
    virtual void    RegisterIconAliases(wxFileArtProvider& provider);
    virtual const CViewTypeDescriptor&   GetViewTypeDescriptor() const;
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    /// @}
};


END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___TASK_VIEW__HPP
