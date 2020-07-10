#ifndef GUI_WIDGETS_DATA___QUERY_PARSE_PANEL_HPP
#define GUI_WIDGETS_DATA___QUERY_PARSE_PANEL_HPP

/*  $Id: query_parse_panel.hpp 40264 2018-01-18 20:56:01Z katargir $
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
 * Authors:  Robert Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/utils/command.hpp>
#include <gui/objutils/query_data_source.hpp>
#include <gui/objutils/query_widget.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_query_exec.hpp>

#include <gui/utils/job_future.hpp>

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/stattext.h>

#include <map>


class wxButton;
class wxToolBar;
class wxCheckBox;
class wxAnimationCtrl;
class wxTextCtrl;


BEGIN_NCBI_SCOPE

class IServiceLocator;
class CQueryParseTree;
class CRichTextCtrl;

///////////////////////////////////////////////////////////////////////////////
/// CQueryParsePanel
///
/// This class represents a toolbar that allows the user to query against
/// a data source and then to visualize the query results as selected elements
/// within the attached view.  In order to support the query toolbar, the
/// developer must have a data source that is derived from IQueryDataSource
/// which supports execution of the parsed query against the data.  The 
/// developer must also provide a pointer to a widget (the view for the data
/// source) which must implement the interface IQueryWidget.
///
/// The queries themselves are run as background jobs since they could take
/// significant time in cases where network access is needed.  At job completion
/// a CQueryEvent event is sent to the IQueryWidget-derived widget.  
///
class NCBI_GUIWIDGETS_DATA_EXPORT CQueryParsePanel
:   public wxPanel
,   public CEventHandler
,   public IRegSettings
{
    DECLARE_EVENT_TABLE();

public:
    enum ECommands   {
        eCmdStartQuery=eBaseCmdLast + 2750,        
        eCmdStopQuery,
        eCmdSelQuery,
        eCmdNext,
        eCmdPrevious,
        eCmdCaseSensitive,
        eCmdHelp, 
        eCmdFilter,
        eCmdStringMatchType,

        eCmdLast
    };

    typedef vector<pair<string,string> >  TNamedQueries;

public:
    /// Ctor requires a data source derived from IQueryDataSource and a view
    /// of that data source derived from IQueryWidget.
    CQueryParsePanel(IQueryWidget* w,
                     IQueryDataSource* ds = NULL);
    virtual ~CQueryParsePanel();
    
    /// Create window
    virtual void Create(wxWindow* parent,
                        wxWindowID id = wxID_ANY,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize);

    /// Initialises member variables
    void    Init();

    /// Creates the controls and sizers
    void    CreateControls();

    /// Set or update data source
    void SetDataSource(IQueryDataSource* ds);

    /// Add given string to currently displayed query text
    void AddQueryText(const string& query_mod);

    // Perform actions to update toolbar ui after query completes
    void CompleteQuery(CMacroQueryExec* qexec, const string& status);

    /// Return true if select all checkbox is checked
    bool IsSelectAll();

    /// Hide the Select All checkbox
    void HideSelectAll(void);

    /// Set the set of default queries for the current data source
    void SetDefaultQueries(const TNamedQueries& q);
    /// Retrieve the current defualt queries for the data source
    TNamedQueries GetDefaultQueries() const { return m_DefaultQueries; }

    /// Get the text for the most recently executed query
    string GetLastQuery() const;

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name Command handlers
    /// @{
    void    OnSearchctrlTextEnter(wxCommandEvent& evt);
    void    OnSearchPaste(wxClipboardTextEvent& evt);
    void    OnQuerySel( wxCommandEvent& evt );
    void    OnButtonNext( wxCommandEvent& evt );
    void    OnButtonPrevious( wxCommandEvent& evt );
    void    OnSearchctrlSearchButtonClick( wxCommandEvent& evt );
    void    OnCtrlKillFocus(wxFocusEvent& evt);
    void    OnSearchctrlSearchStop( wxCommandEvent& evt );  
    void    OnSearchHelp(wxCommandEvent& evt);
    void    OnToggleSelectAll(wxCommandEvent & evt);
    void    OnToggleHideUnselected(wxCommandEvent & evt);
    void    OnContextMenu(wxContextMenuEvent&);
    void    OnRecentQuery(wxCommandEvent& evt);

    void    OnStringMatchTypeBtn(wxCommandEvent & evt);
    void    OnStringMatchTypeSelected( wxCommandEvent& evt );    
    /// @}

    void OnIdle(wxIdleEvent& event);

protected:

    /// Job class used to run query on separate thread
    class  CQueryJob : public CObject
    {
    public:
        CQueryJob(CMacroQueryExec*  qexec,
                  CQueryParseTree* qtree,
                  bool casesensitive,
                  CStringMatching::EStringMatching string_compare)
            : m_QExec(qexec)
            , m_QueryString("")
            , m_CaseSensitive(casesensitive)
            , m_StringCompare(string_compare)
            , m_DS(NULL)
            , m_QueryTree(qtree)
            , m_Macro(NULL)
            , m_NumSelected(0)
            , m_NumQueried(0)
            , m_QueryTime(0.0f) {}

        CQueryJob(CMacroQueryExec*  qexec,
            macro::CMacroRep* mrep,
            bool casesensitive,
            CStringMatching::EStringMatching string_compare)
            : m_QExec(qexec)
            , m_QueryString("")
            , m_CaseSensitive(casesensitive)
            , m_StringCompare(string_compare)
            , m_DS(NULL)
            , m_QueryTree(NULL)
            , m_Macro(mrep)
            , m_NumSelected(0)
            , m_NumQueried(0)
            , m_QueryTime(0.0f) {}

        CQueryJob(const string& query,
                  bool casesensitive,
                  CStringMatching::EStringMatching string_compare,
                  IQueryDataSource* ds)
            : m_QueryString(query)
            , m_CaseSensitive(casesensitive)
            , m_StringCompare(string_compare)
            , m_DS(ds)
            , m_QueryTree(NULL)
            , m_Macro(NULL)
            , m_NumSelected(0)
            , m_NumQueried(0)
            , m_QueryTime(0.0f) {}

        ~CQueryJob();

        /// Get stats on how many elements were checked and how many were 
        /// checked but failed for some reason (such as being unable to 
        /// compare types)
        int GetQueriedCount() const;
        int GetExceptionCount() const;
        int GetNumSelected() const { return m_NumSelected; }
        float GetQueryTime() { return m_QueryTime; }
        void DumpTimers();

        CMacroQueryExec* GetQueryExec()  { return m_QExec.GetPointerOrNull(); }

        void Execute(ICanceled& canceled);

    private:
        CQueryJob();
        CQueryJob& operator=(const CQueryJob&);

        /// Query execution object
        CRef<CMacroQueryExec> m_QExec;
        /// For simple string queries
        string          m_QueryString;
        bool            m_CaseSensitive;
        CStringMatching::EStringMatching m_StringCompare;

        IQueryDataSource* m_DS;
        /// Parsed query tree
        CQueryParseTree*    m_QueryTree;
        macro::CMacroRep* m_Macro;


        size_t m_NumSelected;
        size_t m_NumQueried;
        float m_QueryTime;
    };

    /// Return true if query parses correctly, putting field names in 'fields'
    bool x_IsValidQuery(string query, 
                        bool casesensitive,
                        vector<std::pair<size_t, size_t> >& fields);
    /// Submit query job for asynch execution
    void x_StartQueryJob(CRef<CMacroQueryExec>  qexec,
                         CQueryParseTree* qtree, 
                         macro::CMacroRep* macro_rep,
                         bool casesensitive,
                         const string& query);
    /// Parse query and determine how to execute
    void x_ExecuteQuery(int search_dir);
    /// If user presses cancel button, cancel running job
    void x_CancelQueryInProgress();
    /// Add a query to the list of recently used queries, if it is unique
    void x_AddRecentQuery(const std::string& q);


    /// Only need to load icons into art provider once per run
    static bool m_IconsInitialized;

    ///
    /// Widgets
    CRichTextCtrl* m_pSearch;
    wxAnimationCtrl* m_Animation;
    wxToolBar* m_ToolBar;  
    wxChoice* m_StringSearch;
    wxButton* m_StopQuery;  
    wxCheckBox* m_pSelectAll;
    wxStaticText* m_SearchType;

    /// positions of dictionary fields in current search string
    std::vector<std::pair<size_t, size_t> > m_SearchStringFields;

    /// current search string (as of previous update to m_pSearch)
    std::string m_SearchString;

    /// Set of recent queries
    vector<pair<string,string> > m_RecentQueries;
    /// Max number of recent queries to include in list
    const static int m_MaxRecentQueries = 5;
    /// Set of default queries
    vector<pair<string,string> > m_DefaultQueries;
    const static int m_MaxDefaultQueries = 10;

    /// Interface to widget which will display query results
    IQueryWidget* m_QueryWidget;
    /// Interface to data source that is being queried
    CIRef<IQueryDataSource> m_QueryDataSource;

    /// String of query last time query was executed
    string m_PrevQuery;
    /// Current value for case-sensitive
    bool m_CaseSensitive;
    /// Value of case-sensitive button last time query was executed
    bool m_PrevCaseSensitive;

    /// How to compare strings in query - wildcards, exact match, etc.
    CStringMatching::EStringMatching m_StringCompare;
    /// Value of string-matching selection last time query was executed
    CStringMatching::EStringMatching m_PrevStringCompare;

    /// Current tree-query job
    CIRef<CQueryJob>  m_Job;
    job_future<void>  m_Future;

    /// Registry path for query panel
    string m_RegPath;
    /// Registry section for specific current data source (derived from columns)
    string m_RegSection;
};




END_NCBI_SCOPE

#endif  // GUI_WIDGETS_DATA___QUERY_PARSE_PANEL_HPP
