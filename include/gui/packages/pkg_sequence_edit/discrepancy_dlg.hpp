/*  $Id: discrepancy_dlg.hpp 44322 2019-11-29 18:29:04Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Sema
 */

#ifndef _DISCREPANCY_DLG_H_
#define _DISCREPANCY_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <misc/discrepancy/discrepancy.hpp>
#include <gui/core/selection_client.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/framework/app_task.hpp>
#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <gui/framework/app_task_impl.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>
#include <wx/string.h>

#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/treelist.h>
#include <wx/dataview.h>

class wxCheckBox;

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

wxDEFINE_EVENT(WIDTH_EVENT, wxCommandEvent);

////@begin control identifiers
#define ID_CDISCR_DLG 10357
#define ID_CDISCR_CONF 10358
#define ID_CDISCR_LIST 10359
#define ID_DISCR_LEFT 10501
#define ID_DISCR_RIGHT 10502
#define ID_DISCR_PREV 10503
#define ID_DISCR_NEXT 10504
#define ID_DISCR_FIND_TEXT 10505
#define ID_DISCR_REFRESH 10506
#define ID_DISCR_CHECKALL 10507
#define ID_DISCR_UNCHECKALL 10508
#define ID_DISCR_AUTOFIX 10509
#define ID_DISCR_REPORT 10510
#define ID_DISCR_TESTNAME 10511
#define ID_DISCR_SETTINGS 10512
#define ID_DISCR_CLOSE 10513
#define ID_DISCR_OK 10514
#define ID_DISCR_TEXT 10515
#define ID_DISCR_LIST 10516
#define ID_DISCR_DETAILS 10517
#define ID_DISCR_CHECK 10518
#define ID_DISCR_UNCHECK 10519
#define ID_DISCR_TOGGLE 10520
#define ID_DISCR_ALL 10521
#define ID_DISCR_NONE 10522
#define ID_DISCR_DEFAULT 10523
#define ID_DISCR_EXPAND 10524
#define ID_DISCR_CONTRACT 10525
#define ID_DISCR_SEQUESTER 10526
#define ID_DISCR_CHECKFIXABLE 10527
#define ID_DISCR_SPR 10528
#define ID_DISCR_SPR_NAME 10529
#define ID_DISCR_REOPEN 10530
#define ID_CDISCR_PANEL 10531
#define SYMBOL_DISCR_PANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_DISCR_STYLE wxMINIMIZE_BOX|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL|wxCLIP_CHILDREN
#define SYMBOL_DISCR_TITLE _("Discrepancy Test List")
////@end control identifiers

class CDiscrepancyDlg;
class CCustomTree;
class CCustomTreeItem;
class CDiscrepancyView;
class CDiscrepancyConf;
class CDiscrepancyPanel;
class CGuiRegistry;
class ICommandProccessor;

struct CDiscrepancyPanelHost
{
    virtual void UpdateButtons() {}
};


class CDiscrepancyDlg : public wxFrame, public CEventHandler, public CDiscrepancyPanelHost
{
    DECLARE_DYNAMIC_CLASS(wxFrame)
    DECLARE_EVENT_TABLE()
    DECLARE_EVENT_MAP();

public:
    enum EReportType {
        eNormal,
        eOncaller,
        eSubmitter,
        eMega,
        eCdsTrnaOverlap
    };

    enum EChangeStatus {
        eChecked = 1 << 0,
        eUnchecked = 1 << 1,
        eSequester = 1 << 2
    };

    CDiscrepancyDlg() {}
    CDiscrepancyDlg(EReportType type, wxWindow* parent, IWorkbench* workbench, wxWindowID id = ID_CDISCR_DLG, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_STYLE);
    ~CDiscrepancyDlg();
    static CDiscrepancyDlg* OpenDiscrepancyDlg(EReportType type, CScope& scope, IWorkbench* workbench, const wxString& workDir = wxEmptyString);
    string RegisterRoot(bool separate = true);
    wxString GetSettingsPath();
    void WriteSettings(CGuiRegistry& gui_reg);
    EReportType GetType() const { return m_Type; };

    void RunTests();
    //void OnJobNotification(CEvent* evt);
    void OnDataChanged(CEvent* evt);

    void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }
    bool NeedToReopen() const { return m_AutoReopen; }
    void UpdateButtons();

protected:
    bool Create(wxWindow* parent, wxWindowID id = ID_CDISCR_DLG, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_STYLE);
    void Init();
    void CreateControls();
    void SetProductRules(const string rules);
    const string GetProductRules();
    void OnExpand(wxCommandEvent& evt);
    void OnContract(wxCommandEvent& evt);
    void OnRefresh(wxCommandEvent& evt);
    void OnCheckAll(wxCommandEvent& evt);
    void OnUncheckAll(wxCommandEvent& evt);
    void OnCheckFixable(wxCommandEvent& evt);
    void OnAutofix(wxCommandEvent& evt);
    void OnSequester(wxCommandEvent& evt);
    void OnReport(wxCommandEvent& evt);
    void OnTestName(wxCommandEvent& evt);
    void OnSettings(wxCommandEvent& evt);
    void OnReopen(wxCommandEvent& event);
    void OnClose(wxCommandEvent& evt);
    bool Destroy();
    void SaveTreeState();
    void RestoreTreeState();
    void LoadSettings();

    objects::CGBProjectHandle* m_Project;
    EReportType m_Type;
    IWorkbench* m_Workbench;
    int m_RectX;
    int m_RectY;
    int m_RectW;
    int m_RectH;
    ICommandProccessor* m_CmdProccessor;
    CDiscrepancyPanel* m_Panel;
    wxButton* m_ExpandBtn;
    wxButton* m_ContractBtn;
    wxButton* m_TestNameBtn;
    wxButton* m_RefreshBtn;
    wxButton* m_UncheckAllBtn;
    wxButton* m_CheckFixableBtn;
    wxButton* m_AutofixBtn;
    wxButton* m_ReportBtn;
    wxButton* m_SettingsBtn;
    wxButton* m_SequesterBtn;
    wxButton* m_TrimBtn;
    wxButton* m_PrevBtn;
    wxButton* m_NextBtn;
    wxCheckBox* m_Reopen;
    wxTextCtrl* m_FindText;
    wxStaticText* m_ProductRulesLabel;
    bool m_AutoReopen;    
    wxString m_WorkDir;
    friend class CDiscrepancyConf;
    friend class CDiscrepancyPanel;
};

class CDiscRepItem;
class CDiscrepancyReportJob;
class IProjectView;

class CDiscrepancyPanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS(wxPanel)
    DECLARE_EVENT_TABLE()

public:
    CDiscrepancyPanel() {}
    CDiscrepancyPanel(CDiscrepancyDlg::EReportType type, wxWindow* parent, CDiscrepancyPanelHost* host, IWorkbench* workbench, wxWindowID id = ID_CDISCR_PANEL, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_PANEL_STYLE);
    ~CDiscrepancyPanel();
    string RegisterRoot(bool separate = true);
    wxString GetSettingsPath();
    void WriteSettings(CGuiRegistry& gui_reg);
    CDiscrepancyDlg::EReportType GetType() const { return m_Type; };

    void RunTests();
    

    bool NeedRefresh() const { return m_NeedRefresh; }
    unsigned char GetStatus();
    bool IsInvalidJob();
    void RefreshData();
    void SetWorkDir(const wxString &dir) { m_WorkDir = dir; }
    void SetProject(objects::CGBProjectHandle* project) { m_Project = project; }
    void SetCmdProccessor(ICommandProccessor* proc) { m_CmdProccessor = proc; }

    class CDiscrepancyHelper : public CObjectEx, public CEventHandler
    {
    public:
        CDiscrepancyHelper(CDiscrepancyPanel* panel)
            : m_Panel(panel), m_JobId(CAppJobDispatcher::eInvalidJobID)  { }
        virtual ~CDiscrepancyHelper() {
            if (m_JobId != CAppJobDispatcher::eInvalidJobID) {
                CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
                m_JobId = CAppJobDispatcher::eInvalidJobID;
            }
        }
        void RunTests(CRef<CDiscrepancyReportJob> job);
        void OnJobNotification(CEvent* evt);
        bool IsInvalidJob();
    private:
        DECLARE_EVENT_MAP();
        CDiscrepancyPanel* m_Panel;
        CAppJobDispatcher::TJobID m_JobId;
    };

protected:
    bool Create(wxWindow* parent, wxWindowID id = ID_CDISCR_PANEL, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_PANEL_STYLE);
    void Init();
    void CreateControls();
    void UpdateTestList();
    void OnClick(wxCommandEvent& evt);
    void OnDblClk(wxCommandEvent& evt);
    void OnListClk(wxCommandEvent& evt);
    void OnListDblClk(wxCommandEvent& evt);
    void ExpandAll();
    void OnContract(wxCommandEvent& evt);
    void OnRefresh(wxCommandEvent& evt);
    void OnCheckAll(wxCommandEvent& evt);
    void OnUncheckAll(wxCommandEvent& evt);
    void OnCheckFixable(wxCommandEvent& evt);
    void CollectAutofix(CCustomTreeItem& item, NDiscrepancy::TReportObjectList& tofix);
    void OnAutofix(wxCommandEvent& evt);
    void Sequester(CCustomTreeItem& item, set<CBioseq_Handle>& out, CScope& scope, bool all);
    void OnSequester(wxCommandEvent& evt);
    void Report(CCustomTreeItem& item, string& out, bool all);
    void OnReport(wxCommandEvent& evt);
    void OnTestName();
    void OnFindText(wxCommandEvent& evt);
    void OnFindNext(wxCommandEvent& evt);
    void OnFindPrev(wxCommandEvent& evt);
    void UpdateButtons() { if (m_Host) m_Host->UpdateButtons(); }
    void OpenEditor(const CSerialObject* ref, CScope& scope);
    const CSerialObject* GetTopObject(const CSerialObject* obj);
    bool InvalidObject(const CSerialObject* obj, CScope& scope);
    CBioseq_Handle GetAccessionForObject(const CSerialObject* obj);
    void SaveTreeState();
    void RestoreTreeState();
    void recursiveCdsTrnaOverlap(CDiscRepItem* item, vector<CRef<NDiscrepancy::CReportItem>>& reps, CScope& scope);
    IProjectView* FindGraphicalSequenceView(CConstRef<CSeq_id> id);

    CDiscrepancyDlg::EReportType m_Type;
    IWorkbench* m_Workbench;
    CRef<NDiscrepancy::CDiscrepancySet> m_Tests;
    vector<string> m_TestList;
    vector<string> m_DefaultTests;
    vector<string> m_AddTests;
    vector<string> m_RemoveTests;
    map<string, unsigned char> m_TreeState;
    string m_ProductRules;
    ICommandProccessor* m_CmdProccessor;
    CDiscrepancyPanelHost* m_Host;
    CCustomTree* m_Left;
    wxListBox* m_Right;
    wxButton* m_PrevBtn;
    wxButton* m_NextBtn;
    wxTextCtrl* m_FindText;
    wxStaticText* m_Loading;
    NDiscrepancy::CDiscrepancyGroup m_TGrp;
    bool m_ShowTestName;
    bool m_NeedRefresh;
    objects::CGBProjectHandle*  m_Project;
    CRef<CSelectionClient> m_SelectionClient;
    wxString m_WorkDir;
    friend class CDiscrepancyDlg;
    CRef<CDiscrepancyHelper> m_Helper;
};


inline void CDiscrepancyDlg::SetProductRules(const string rules) { m_Panel->m_ProductRules = rules; }
inline const string CDiscrepancyDlg::GetProductRules() { return m_Panel->m_ProductRules; }


class COpenDiscrepancyTask : public CAppTask
{
public:
    COpenDiscrepancyTask(CDiscrepancyDlg::EReportType type, CGBProjectHandle& proj, CWorkbench& wb) : m_Type(type), m_Proj(proj), m_Workbench(wb) {}
protected:
    CDiscrepancyDlg::EReportType m_Type;
    CGBProjectHandle& m_Proj;
    CWorkbench& m_Workbench;
    virtual IAppTask::ETaskState x_Run();
};


class CDiscrepancyConf : public wxDialog
{
    DECLARE_DYNAMIC_CLASS(CDiscrepancyConf)
    DECLARE_EVENT_TABLE()
public:
    /// Constructors
    CDiscrepancyConf();
    CDiscrepancyConf(CDiscrepancyDlg::EReportType type, vector<string>* t, vector<string>* dt, vector<string>* at, vector<string>* rt, CDiscrepancyDlg* parent, wxWindowID id = ID_CDISCR_CONF, const wxString& caption = SYMBOL_DISCR_TITLE, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_STYLE);

    bool Create(wxWindow* parent, wxWindowID id = ID_CDISCR_CONF, const wxString& caption = SYMBOL_DISCR_TITLE, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_STYLE);
    void Init();
    void CreateControls();
    void OnTreeListCheck(wxTreeListEvent& event);
    void OnDetails(wxCommandEvent& event);
    void OnSelectAll(wxCommandEvent& event);
    void OnSelectNone(wxCommandEvent& event);
    void OnSelectDefault(wxCommandEvent& event);
    void OnCheck(wxCommandEvent& event);
    void OnUncheck(wxCommandEvent& event);
    void OnToggle(wxCommandEvent& event);
    void OnProductRules(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnFindText(wxCommandEvent& event);
    void OnFindNext(wxCommandEvent& event);
    void OnFindPrev(wxCommandEvent& event);
    void UpdateList();
    void UpdateButtons();

protected:
    CDiscrepancyDlg::EReportType m_Type;
    bool m_Details;
    string m_OldProductRules;
    string m_NewProductRules;
    wxTreeListCtrl* m_List;
    wxButton* m_SelAll;
    wxButton* m_SelNon;
    wxButton* m_SelDef;
    wxButton* m_Check;
    wxButton* m_Unheck;
    wxButton* m_Toggle;
    wxButton* m_Show;
    wxButton* m_PrevBtn;
    wxButton* m_NextBtn;
    wxTextCtrl* m_FindText;
    wxStaticText* m_ProductRulesLabel;
    vector<string>* m_Tests;
    vector<string>* m_DefaultTests;
    vector<string>* m_AddTests;
    vector<string>* m_RemoveTests;
    map<string, bool> m_MapTests;
    map<string, bool> m_MapDefaultTests;
    CDiscrepancyDlg* m_DiscrepancyDlg;
};


class CDiscrepancyList : public wxFrame
{
    DECLARE_DYNAMIC_CLASS(CDiscrepancyList)
    DECLARE_EVENT_TABLE()
public:
    /// Constructors
    CDiscrepancyList();
    CDiscrepancyList(wxWindow* parent, wxWindowID id = ID_CDISCR_LIST, const wxString& caption = SYMBOL_DISCR_TITLE, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_STYLE);
    bool Create(wxWindow* parent, wxWindowID id = ID_CDISCR_LIST, const wxString& caption = SYMBOL_DISCR_TITLE, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = SYMBOL_DISCR_STYLE);
    void CreateControls();
    void OnClose(wxCommandEvent& event);
    void OnDetails(wxCommandEvent& event);
    void UpdateList();

protected:
    bool m_Details;
    wxButton* m_Show;
    wxTextCtrl* m_Text;
};


/////////////////////////////////////////////////////////////////////////////////////////////
class CDiscrepancyReportJob : public CJobCancelable
{
public:
    CDiscrepancyReportJob(const CGBProjectHandle* proj, NDiscrepancy::CDiscrepancySet* tests) : m_Proj(proj), m_Tests(tests) {}
    ~CDiscrepancyReportJob() {}
    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress() { return CConstIRef<IAppJobProgress>(0); }
    virtual CRef<CObject>               GetResult() { return CRef<CObject>(&*m_Tests); }
    virtual CConstIRef<IAppJobError>    GetError() { return CConstIRef<IAppJobError>(0); }
    virtual string GetDescr(void) const { return "Discrepancy Test"; }
    /// @}
protected:
    const CGBProjectHandle* m_Proj;
    CRef<NDiscrepancy::CDiscrepancySet> m_Tests;
};


END_NCBI_SCOPE

#endif
    // _DISCREPANCY_DLG_H_
