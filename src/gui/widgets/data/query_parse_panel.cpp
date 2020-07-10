/*  $Id: query_parse_panel.cpp 44935 2020-04-21 20:16:21Z asztalos $
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
 *  and reliability of the software and m_Data, the NLM and the U.S.
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

#include <ncbi_pch.hpp>

#include <gui/widgets/data/query_parse_panel.hpp>
#include <gui/widgets/data/query_panel_event.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/richtextctrl.hpp>

#include <gui/objutils/query_data_source.hpp>
#include <gui/objutils/query_func_promote.hpp>
#include <gui/objutils/macro_parse.hpp>
#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_ex.hpp>

#include <gui/widgets/wx/async_call.hpp>

#include <util/qparse/query_parse.hpp>
#include <util/qparse/parse_utils.hpp>

#include <util/checksum.hpp>

#include <wx/app.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/button.h>
#include <wx/animate.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/toolbar.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/timer.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/choice.h>
#include <wx/settings.h>


#include <algorithm>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


#define ID_SEARCHCTRL       13001
#define ID_SELECT_ALL       13005
#define ID_QUERY_ANIMATIONCTRL 13007
#define ID_TOOLBAR          13010
#define ID_TOOLBAR2          13011
#define ID_CASE_SENSITIVE   13012
#define ID_STRING_SEARCH    13013
#define ID_MRU1             13014
#define ID_STRING_MATCH_TYPE1 14014

static const char* kRecentQueries = "Recent Queries";
static const char* kQueryPanel = ".QueryPanelGrid";

const int kStringMatchTypesCount = 4;
const char *kStringMatchTypes[kStringMatchTypesCount] = {
    "Exact Match",
    "Wildcards",
    "Regular Expression",
    "Phonetic"
};

// Allow display within toolbar to vary from display in menu (may be shorter
// to save space)
const char *kStringMatchTypesToolbar[kStringMatchTypesCount] = {
    "Exact Match",
    "Wildcards",
    "Reg Exp",
    "Phonetic"
};

// enummerated comparison type for each match type
CStringMatching::EStringMatching kStringMatchEnumTypes[kStringMatchTypesCount] = {
    CStringMatching::ePlainSearch,
    CStringMatching::eWildcardMatch,
    CStringMatching::eRegex,
    CStringMatching::eMetaphone
};

///////////////////////////////////////////////////////////////////////////////
//
// Background job for executing query on data source
//
///////////////////////////////////////////////////////////////////////////////
inline void CQueryParsePanel::CQueryJob::Execute(ICanceled& canceled)
{
    CStopWatch timer;
    timer.Start();

    if (!m_QExec.IsNull()) {
        m_QExec->EvalStart();
    
        while (!m_QExec->EvalComplete()) {
            if (canceled.IsCanceled())
                return;

            if (m_QueryTree != NULL) {
                if (m_QExec->EvalNext(*m_QueryTree))
                    ++m_NumSelected;
            }
            else {                
                if (m_QExec->EvalNext(*m_Macro))
                    ++m_NumSelected;
            }
        }
        m_NumQueried = m_QExec->GetQueriedCount();
    }
    else if (m_DS != NULL) {

        m_DS->ExecuteStringQuery(m_QueryString, m_NumSelected, m_NumQueried, 
            m_StringCompare, m_CaseSensitive ? NStr::eCase : NStr::eNocase);
    }

    m_QueryTime = timer.Elapsed();
}

CQueryParsePanel::CQueryJob::~CQueryJob() 
{
    delete m_QueryTree;
    delete m_Macro;
}

int CQueryParsePanel::CQueryJob::GetQueriedCount() const 
{ 
    return m_NumQueried;
}

int CQueryParsePanel::CQueryJob::GetExceptionCount() const 
{     
    if (!m_QExec.IsNull())
        return m_QExec->GetExceptionCount();
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// CQueryParsePanel
// 
///////////////////////////////////////////////////////////////////////////////

bool CQueryParsePanel::m_IconsInitialized = false;
const int CQueryParsePanel::m_MaxRecentQueries;   
const int CQueryParsePanel::m_MaxDefaultQueries;

BEGIN_EVENT_TABLE( CQueryParsePanel, wxPanel )
    EVT_CONTEXT_MENU( CQueryParsePanel::OnContextMenu )
    EVT_TEXT(ID_SEARCHCTRL, CQueryParsePanel::OnSearchctrlTextEnter)
    EVT_TEXT_PASTE(ID_SEARCHCTRL, CQueryParsePanel::OnSearchPaste)
    EVT_TEXT_ENTER(ID_SEARCHCTRL, CQueryParsePanel::OnSearchctrlSearchButtonClick)    
    EVT_TOOL(eCmdSelQuery, CQueryParsePanel::OnQuerySel)
    EVT_TOOL(eCmdNext, CQueryParsePanel::OnButtonNext)
    EVT_TOOL(eCmdPrevious, CQueryParsePanel::OnButtonPrevious)   
    EVT_TOOL(eCmdStartQuery, CQueryParsePanel::OnSearchctrlSearchButtonClick)
    EVT_TOOL(eCmdStopQuery, CQueryParsePanel::OnSearchctrlSearchStop)
    EVT_TOOL(eCmdHelp, CQueryParsePanel::OnSearchHelp)
    EVT_TOOL(eCmdFilter, CQueryParsePanel::OnToggleHideUnselected)
    EVT_TOOL(eCmdStringMatchType, CQueryParsePanel::OnStringMatchTypeBtn)
    
    EVT_CHECKBOX(ID_SELECT_ALL, CQueryParsePanel::OnToggleSelectAll)    
    EVT_COMMAND_RANGE(ID_MRU1, ID_MRU1 + 
                      CQueryParsePanel::m_MaxRecentQueries + 
                      CQueryParsePanel::m_MaxDefaultQueries - 1, 
                      wxEVT_COMMAND_MENU_SELECTED, 
                      CQueryParsePanel::OnRecentQuery)
    EVT_COMMAND_RANGE(ID_STRING_MATCH_TYPE1, ID_STRING_MATCH_TYPE1 + 4,
                      wxEVT_COMMAND_MENU_SELECTED, 
                      CQueryParsePanel::OnStringMatchTypeSelected)                      
    EVT_IDLE(CQueryParsePanel::OnIdle)
END_EVENT_TABLE()


CQueryParsePanel::CQueryParsePanel(IQueryWidget* w, IQueryDataSource* ds)
: m_pSearch(NULL)
, m_Animation(NULL)
, m_ToolBar(NULL)
, m_pSelectAll(NULL)
, m_SearchType(NULL)
, m_QueryWidget(w)
, m_CaseSensitive(false)
, m_PrevCaseSensitive(false)
, m_StringCompare(CStringMatching::ePlainSearch)
, m_PrevStringCompare(CStringMatching::ePlainSearch)
{
    SetDataSource(ds);
    Init();
}

CQueryParsePanel::~CQueryParsePanel()
{
    #ifndef __WXMAC__
        PopEventHandler( true );
    #endif

    if (m_Future.IsRunning()) {
        m_Future.cancel();
        GUI_AsyncJoinFuture(m_Future, "Canceling Search...");
    }
}

void CQueryParsePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size )
{
#ifdef __WXOSX_COCOA__ // GB-8581
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
#endif

    wxPanel::Create(parent, id, pos, size);
    CreateControls();

    #ifndef __WXMAC__
        PushEventHandler( new CCommandToFocusHandler( this ) );
    #endif
}

void CQueryParsePanel::Init()
{
    m_pSearch          = NULL;
    m_Animation        = NULL;
    m_ToolBar          = NULL; 
    m_ToolBar          = NULL;    
    m_pSelectAll       = NULL;


    if( !m_IconsInitialized ){
        wxFileArtProvider* provider = GetDefaultFileArtProvider();

        provider->RegisterFileAlias(wxT("menu::dm_start"), wxT("playhs.png"));
        provider->RegisterFileAlias(wxT("menu::dm_stop"),  wxT("stophs.png"));
        provider->RegisterFileAlias(wxT("menu::back"), wxT("back.png"));
        provider->RegisterFileAlias(wxT("menu::forward"), wxT("forward.png"));
        provider->RegisterFileAlias(wxT("menu::help"), wxT("help.png"));
        provider->RegisterFileAlias(wxT("menu::query_sel"), wxT("dropdown_sel.png"));
        provider->RegisterFileAlias(wxT("menu::dm_filter"), wxT("filter.png"));
        
        m_IconsInitialized = true;
    }
}

void CQueryParsePanel::CreateControls()
{
    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);

#ifndef __WXMAC__
    long styles = wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_HORZ_LAYOUT;
#else
    // Mac toolbars put text under the icon which looks really bad here so just doen't display icon text
    long styles = wxTB_FLAT | wxTB_HORIZONTAL;
#endif
    m_ToolBar = new wxToolBar(this, ID_TOOLBAR,
                              wxDefaultPosition, wxSize(-1, -1), styles);

    // label    
    wxStaticText* tool_text = new wxStaticText(m_ToolBar, wxID_ANY,
                                                wxT("Search:"));
    m_ToolBar->AddControl(tool_text);

    // Must supply wxTE_RICH otherwise the context menu (cut, copy, past...) for the search box will not work.
    // Without wxTE_RICH, context menus from the parent widgets (this one, and CDockPanel) will both
    // take priority over the search controls context menu meaning it never shows up.  This is probably a wx
    // bug coming from the fact wx uses the built-in context menu when wxTE_RICH is not used, and a wx-created
    // context menu when it is. (and search control is derived from text control, so wxTE_RICH is recognized)

    // Update - The other reason to use TE_RICH is so that colored text (for fieldnames ) will work.
    // unfortunately TE_RICH on cocoa forces creation of a multi-line control which is unusuable :-(.  
    // So I've patched wxWidgets so that when you do not provide wxTE_MULTILINE with wxTE_RICH it will
    // create a single-line rich text control.  Updates are in src/osx/cocoa/textctrl.mm (wxNSTextViewControl ctor)
    // and include/wx/osx/cocoa/private/textimpl.h (ctor declaration). 
/*  Actual wxWidgets patch.  This is for version 3.0.1
diff -ur -x'*-*' -x.DS_Store ./wxWidgets-3.0.1_orig/include/wx/osx/cocoa/private/textimpl.h ./wxWidgets-3.0.1/include/wx/osx/cocoa/private/textimpl.h
--- ./wxWidgets-3.0.1_orig/include/wx/osx/cocoa/private/textimpl.h      2014-06-14 17:48:48.000000000 -0400
+++ ./wxWidgets-3.0.1/include/wx/osx/cocoa/private/textimpl.h   2014-08-14 14:16:53.000000000 -0400
@@ -62,7 +62,7 @@
 class wxNSTextViewControl : public wxWidgetCocoaImpl, public wxTextWidgetImpl
 {
 public:
-    wxNSTextViewControl( wxTextCtrl *wxPeer, WXWidget w );
+    wxNSTextViewControl( wxTextCtrl *wxPeer, WXWidget w, long style );
     virtual ~wxNSTextViewControl();

     virtual wxString GetStringValue() const ;
diff -ur -x'*-*' -x.DS_Store ./wxWidgets-3.0.1_orig/src/osx/cocoa/textctrl.mm ./wxWidgets-3.0.1/src/osx/cocoa/textctrl.mm
--- ./wxWidgets-3.0.1_orig/src/osx/cocoa/textctrl.mm    2014-06-14 17:48:48.000000000 -0400
+++ ./wxWidgets-3.0.1/src/osx/cocoa/textctrl.mm 2014-08-14 14:36:58.000000000 -0400
@@ -545,25 +545,50 @@

 // wxNSTextViewControl

-wxNSTextViewControl::wxNSTextViewControl( wxTextCtrl *wxPeer, WXWidget w )
+wxNSTextViewControl::wxNSTextViewControl( wxTextCtrl *wxPeer, WXWidget w, long style )
     : wxWidgetCocoaImpl(wxPeer, w),
       wxTextWidgetImpl(wxPeer)
 {
     wxNSTextScrollView* sv = (wxNSTextScrollView*) w;
     m_scrollView = sv;

-    [m_scrollView setHasVerticalScroller:YES];
+    bool single_line = !(style & wxTE_MULTILINE);
+    [m_scrollView setHasVerticalScroller:NO];
+
     [m_scrollView setHasHorizontalScroller:NO];
     // TODO Remove if no regression, this was causing automatic resizes of multi-line textfields when the tlw changed
     // [m_scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
+    if (single_line) {
+        // Removes vertical scroll bar from single-line text
+        [m_scrollView setAutoresizingMask:NSViewNotSizable];
+    }
+    else {
+        [m_scrollView setHasVerticalScroller:YES];
+        [m_scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
+    }
     NSSize contentSize = [m_scrollView contentSize];

     wxNSTextView* tv = [[wxNSTextView alloc] initWithFrame: NSMakeRect(0, 0,
             contentSize.width, contentSize.height)];
     m_textView = tv;
-    [tv setVerticallyResizable:YES];
-    [tv setHorizontallyResizable:NO];
-    [tv setAutoresizingMask:NSViewWidthSizable];
+
+    const CGFloat LargeNumberForText = 1.0e7;
+
+    if (single_line) {
+        NSTextContainer *textContainer = [tv textContainer];
+        [textContainer setContainerSize:NSMakeSize(LargeNumberForText, LargeNumberForText)];
+        [textContainer setWidthTracksTextView:NO];
+        [textContainer setHeightTracksTextView:NO];
+        [tv setMaxSize:NSMakeSize(LargeNumberForText, LargeNumberForText)];
+        [tv setHorizontallyResizable:YES];
+        [tv setVerticallyResizable:NO];
+        [tv setAutoresizingMask:NSViewNotSizable];
+    }
+    else {
+        [tv setHorizontallyResizable:NO];
+        [tv setVerticallyResizable:YES];
+        [tv setAutoresizingMask:NSViewWidthSizable];
+    }

     [m_scrollView setDocumentView: tv];
*/

    
    int search_width = 180;

    // Search box on mac is a little too short (vertically) so set size here:
#ifdef __WXMAC__
    int search_height = tool_text->GetFont().GetPointSize()+8;
#else
    int search_height = -1;
#endif

    m_pSearch = new CRichTextCtrl(m_ToolBar,
                                  ID_SEARCHCTRL, wxEmptyString, 
                                  wxDefaultPosition, wxSize(search_width, search_height),  
                                  wxTE_PROCESS_ENTER | wxTE_RICH );
    
    m_pSearch->SetToolTip(wxT("Enter search text as:\n\
1. A string to match against all fields\n\
2. A query based on field names, e.g. :\n\
   dist > 0.2 and seq-id = \"NP_110296\""));

    m_ToolBar->AddControl(m_pSearch);

    wxBitmap bmp_selquery = wxArtProvider::GetBitmap(wxT("menu::query_sel"));
    m_ToolBar->AddTool(eCmdSelQuery, wxEmptyString, bmp_selquery, wxNullBitmap, wxITEM_NORMAL, wxT("Select Query")); 
    m_ToolBar->EnableTool(eCmdSelQuery, true);

    // add empty space before the next control (seems to need to be a separator
    // to do that)
    m_ToolBar->AddSeparator();

    m_SearchType = new wxStaticText(m_ToolBar, wxID_ANY,
        wxT("Exact Match"));
    m_ToolBar->AddControl(m_SearchType);
    m_ToolBar->AddTool(eCmdStringMatchType, wxEmptyString, bmp_selquery, wxNullBitmap, wxITEM_NORMAL, wxT("Match Type"));
    m_ToolBar->EnableTool(eCmdStringMatchType, true);

    m_ToolBar->AddSeparator();

    // buttons
    wxBitmap bmp_start = wxArtProvider::GetBitmap(wxT("menu::dm_start"));
    m_ToolBar->AddTool(eCmdStartQuery, wxT(""), bmp_start, wxNullBitmap, wxITEM_NORMAL, wxT("Start Search")); 
    m_ToolBar->EnableTool(eCmdStartQuery, false);

    m_Animation = new wxAnimationCtrl(m_ToolBar, ID_QUERY_ANIMATIONCTRL, wxAnimation(),
                                      wxDefaultPosition, wxSize(22, 22));
    m_Animation->SetToolTip(wxT("Light moves while query is running"));

    wxString path = CSysPath::ResolvePath(wxT("<res>\\status_anim.gif"));
    m_Animation->LoadFile(path);
    m_ToolBar->AddControl(m_Animation); 

    wxBitmap bmp_stop = wxArtProvider::GetBitmap(wxT("menu::dm_stop"));
    m_ToolBar->AddTool(eCmdStopQuery, wxT("Stop"), bmp_stop, wxNullBitmap, wxITEM_NORMAL, wxT("Stop Search"));
    m_ToolBar->EnableTool(eCmdStopQuery, false);
       
    m_ToolBar->AddSeparator();

    wxBitmap bmp_filter = wxArtProvider::GetBitmap(wxT("menu::dm_filter"));  
    m_ToolBar->AddTool(eCmdFilter, wxT("Filter"), bmp_filter, wxNullBitmap, wxITEM_CHECK, wxT("Only show query results"));
    m_ToolBar->AddSeparator();

    m_pSelectAll = new wxCheckBox(m_ToolBar, ID_SELECT_ALL, _("All"), 
        wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    m_pSelectAll->SetValue(true);
    m_pSelectAll->SetToolTip(wxT("Check to show all matching nodes as selected\n\
Uncheck to show one matching node at a time\n"));
    m_ToolBar->AddControl(m_pSelectAll);

    wxBitmap bmp_prev = wxArtProvider::GetBitmap(wxT("menu::back"));
    m_ToolBar->AddTool(eCmdPrevious, wxT(""), bmp_prev, wxT("Previous selection"));
    // Initially disabled because 'All' is true.  Can only use previous
    // and next when 'All' is false (so you are only selecting one at a
    // time)
 
    wxBitmap bmp_next = wxArtProvider::GetBitmap(wxT("menu::forward"));
    m_ToolBar->AddTool(eCmdNext, wxT(""), bmp_next, wxT("Next selection"));

    if (m_QueryWidget != NULL &&
        m_QueryWidget->GetSearchHelpAddr() != string("")) {            
            m_ToolBar->AddSeparator();
            wxBitmap bmp_help = wxArtProvider::GetBitmap(wxT("menu::help"));
            m_ToolBar->AddTool(eCmdHelp, wxT("Help"), bmp_help, wxT("Online query help"));
    }
    
    m_ToolBar->Realize();
    panel_sizer->Add(m_ToolBar, 1, wxALL|wxGROW|wxALIGN_LEFT, 0);

    SetSizerAndFit(panel_sizer);

    m_pSearch->Connect(ID_SEARCHCTRL, wxEVT_KILL_FOCUS, wxFocusEventHandler(CQueryParsePanel::OnCtrlKillFocus), NULL, this);
    m_pSelectAll->Connect(ID_SELECT_ALL, wxEVT_KILL_FOCUS, wxFocusEventHandler(CQueryParsePanel::OnCtrlKillFocus), NULL, this);
}

void CQueryParsePanel::SetDataSource(IQueryDataSource* ds)
{
    m_QueryDataSource.Reset(ds);

    string prev_section = m_RegSection;

    if (ds != NULL) {
        CChecksum cs(CChecksum::eMD5);
        size_t i;
        for (i = 0; i < m_QueryDataSource->GetColsCount(); ++i) {
            cs.AddLine(m_QueryDataSource->GetColumnLabel(i));
        }

        m_RegSection = GetMD5Digest(cs);

        if (m_RegSection != prev_section)
            LoadSettings();
    }
    else {
        m_RegSection = "";
    }

    // Update selection so that if data source changed, we are not pointing to 
    // selected elements from previous source. Don't do this during widget 
    // initialization (when m_pSelectAll is NULL still)
    if (m_QueryWidget != NULL && m_pSelectAll != NULL)
        m_QueryWidget->SetSelectAll(IsSelectAll());

    // pevious query results may no longer be valid, so make sure we re-run
    // the query if the start button is pressed again.
    m_PrevQuery = "";
}

void CQueryParsePanel::SetDefaultQueries(const TNamedQueries& q)
{
    m_DefaultQueries = q;

    if (m_DefaultQueries.size() > (unsigned int)m_MaxDefaultQueries) {
        m_DefaultQueries.erase(m_DefaultQueries.begin() + m_MaxDefaultQueries, 
                               m_DefaultQueries.end());
    }
}

string CQueryParsePanel::GetLastQuery() const
{
    if (m_RecentQueries.size() > 0)
        return m_RecentQueries[0].second;
    return "";
}


void CQueryParsePanel::SetRegistryPath(const string& reg_path) 
{ 
    m_RegPath = reg_path + kQueryPanel;
}

void CQueryParsePanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath + "." + m_RegSection);

        vector<string> queries;
        view.GetStringVec(kRecentQueries, queries);

        for (size_t i=0; i<queries.size(); ++i) {
            if (m_RecentQueries.size() < (unsigned int)m_MaxRecentQueries)      
                m_RecentQueries.push_back(pair<string,string>(string(), queries[i]));
        }
    }
}

void CQueryParsePanel::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath + "." + m_RegSection);

    vector<string> queries;

    vector<pair<string,string> >::const_iterator iter;
    for (iter = m_RecentQueries.begin(); iter != m_RecentQueries.end(); ++iter) {
        if (queries.size() < (unsigned int)m_MaxRecentQueries)
            queries.push_back((*iter).second);      
    }

    if (queries.size() > 0)
        view.Set(kRecentQueries, queries);
}


void CQueryParsePanel::AddQueryText(const string& query_mod)
{
    wxString str = ToWxString(query_mod);
    
    if (str != "") {
        string new_query = m_pSearch->GetUtf8();
        NStr::TruncateSpacesInPlace(new_query);

        if (new_query != "")  {

            // If there is already query text, add an "AND" before
            // the new subclause UNLESS the last term in the query is
            // already AND, OR, NOT or XOR            
            vector<string> arr;
            NStr::Split(new_query, " ", arr);

            if (arr.size() > 0) {
                string last_token = arr[arr.size()-1];
                if ((NStr::CompareNocase(last_token, "and") != 0) &&
                    (NStr::CompareNocase(last_token, "or") != 0) &&
                    (NStr::CompareNocase(last_token, "xor") != 0) &&
                    (NStr::CompareNocase(last_token, "not") != 0)) {
                        new_query += " AND ";
                }
               
                else {
                }
            }
        }

        // We trimmed the existing query above.  Now add one space to keep things
        // from running together.
        new_query += " " + str;

        m_pSearch->SetValue(new_query);
        // This should automatically call OnSearchctrlTextEnter.
    }
}

void CQueryParsePanel::OnContextMenu( wxContextMenuEvent& evt )
{
}

void CQueryParsePanel::OnSearchPaste(wxClipboardTextEvent& evt)
{
    wxTextDataObject data;
    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(wxDF_UNICODETEXT)) {
            wxTheClipboard->GetData(data);
            wxTextDataObject* new_data = new wxTextDataObject(); 
            string str = ToStdString(data.GetText());
            new_data->SetText(ToWxString(str));
            wxTheClipboard->SetData(new_data);
        }
        wxTheClipboard->Close();
    }

    evt.Skip();
}

void CQueryParsePanel::OnRecentQuery(wxCommandEvent& evt)
{
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    unsigned int query_idx = evt.GetId() - ID_MRU1;
    string query;

    if (query_idx < m_DefaultQueries.size())
        query = m_DefaultQueries[query_idx].second;
    else 
        query = m_RecentQueries[query_idx - m_DefaultQueries.size()].second;

    m_pSearch->SetValue(ToWxString(query));

    wxCommandEvent evt_dummy;    
    OnSearchctrlTextEnter(evt_dummy);
    x_ExecuteQuery(0);
}

void CQueryParsePanel::OnSearchctrlSearchButtonClick( wxCommandEvent& evt )
{
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    x_ExecuteQuery(0);
}

void CQueryParsePanel::OnCtrlKillFocus(wxFocusEvent& evt)
{
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    evt.Skip();
}

void CQueryParsePanel::OnSearchctrlSearchStop( wxCommandEvent& evt )
{
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    x_CancelQueryInProgress();
}

void CQueryParsePanel::OnSearchHelp(wxCommandEvent& evt)
{
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    string help_addr = m_QueryWidget->GetSearchHelpAddr();

    ::wxLaunchDefaultBrowser(ToWxString(help_addr));
}

void CQueryParsePanel::OnSearchctrlTextEnter(wxCommandEvent& evt)
{
    // Enter processing is not working on Mac in the current wxWidgets
    // release, so after each update, strip the string of carriage 
    // returns and, if any were found, call the function to submit the query.
#ifdef __WXMAC__
    wxString no_return; 
    bool found_return = false;
    for (size_t i=0; i<evt.GetString().size(); ++i) {
        if (evt.GetString()[i] != wxT('\n'))
            no_return += evt.GetString()[i];
        else 
            found_return = true;
    }
    if (found_return) {
        long cursor = m_pSearch->GetInsertionPoint();
        // cursor is 0-based index so we subtract 1. Should never be <=0 but just in case:
        cursor = std::max(long(0), cursor-1);

        m_pSearch->SetValue(no_return);
        m_pSearch->SetInsertionPoint(std::min(cursor, m_pSearch->GetLastPosition()));
        wxCommandEvent dummy_evt;
        OnSearchctrlSearchButtonClick(dummy_evt);
        return;
    }
#endif

    // Check if query is currently valid and give user some feedback if 
    // it is (or isn't)
    string str = m_pSearch->GetUtf8();
    vector<std::pair<size_t, size_t> > fields;

    bool valid = x_IsValidQuery(str, m_CaseSensitive, fields);

    wxFont f = m_pSearch->GetFont();

    // Remove any formatting from previous calls to this function:
    wxTextAttr attr(wxColour(wxString(wxT("black"))));
    attr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR|wxTEXT_ATTR_FONT_WEIGHT);
    attr.SetFont(f);
    m_pSearch->SetStyle(0, str.length(), attr);

    // Find out how many leading blanks there are.  Since those are truncated
    // before query parsing, we need to add them back in when we highlight
    // data fields.
    string truncated_str = NStr::TruncateSpaces(str, NStr::eTrunc_Begin);
    int tdelta = str.length() - truncated_str.length();

    // If query is valid, enable the search button:
    if (valid)
        m_ToolBar->EnableTool(eCmdStartQuery, true);
    else
        m_ToolBar->EnableTool(eCmdStartQuery, false);

    // If query is valid,  highlight any substrings that were found during query 
    // parsing to be fields from the data dictionary.  An invalid query may
    // also return valid fields so highlight found-fields for invalid
    // queries too.
    if (valid || fields.size() > 0) {       
        wxTextAttr attr(wxColour(wxString(wxT("blue"))));
        attr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR|wxTEXT_ATTR_FONT_WEIGHT);
        attr.SetFont(f);

        for (size_t i=0; i<fields.size(); ++i) {
            m_pSearch->SetStyle((long)(fields[i].first + tdelta),
                                (long)(fields[i].second + tdelta), 
                                attr);
        }

        // Remember 'good' string and fields so that we can continue
        // to highlight fields in parts of the query that have not been
        // modified when the query doesn't parse.
        m_SearchString = str;
        m_SearchStringFields = fields;
    }
    // If the query is not valid and no fields were returned, highlight only
    // those fields which appear in the part of the query string prior to
    // any part of the string that has not been altered (this highlights
    // fields found from the most recent valid query).
    else {        
        // Keep highlighted dictionary fields that are in the text
        // prior to any changes (which presumably made the text invalid)
        if (m_SearchStringFields.size() > 0) {
            wxTextAttr attr(wxColour(wxString(wxT("blue"))));
            attr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR|wxTEXT_ATTR_FONT_WEIGHT);
            attr.SetFont(f);

            string::size_type idx;
            for (idx = 0; idx<str.length() && idx<m_SearchString.length(); ++idx) {
                if (str[idx] != m_SearchString[idx])
                    break;
            }

            for (size_t i=0; i<m_SearchStringFields.size(); ++i) {
                if (m_SearchStringFields[i].second < idx) {
                    m_pSearch->SetStyle((long)(m_SearchStringFields[i].first + tdelta),
                                        (long)(m_SearchStringFields[i].second + tdelta),
                                        attr);
                }
            }
        }
    }

    m_pSearch->Refresh();
}

void CQueryParsePanel::OnQuerySel( wxCommandEvent& evt )
{
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    wxMenu* m = new wxMenu();
    unsigned int i;
    unsigned int max_id = ID_MRU1;

    if (m_DefaultQueries.size() > 0) {
        wxMenu* default_menu = new wxMenu();

        for (i=0; i<m_DefaultQueries.size(); ++i) {
            default_menu->Append(max_id++, ToWxString(m_DefaultQueries[i].first));
        }

        m->AppendSubMenu(default_menu, wxT("Saved Queries"));
    }

    for (i=0; i<m_RecentQueries.size(); ++i) {
        m->Append(max_id++, ToWxString(m_RecentQueries[i].second));
    }

    m_ToolBar->PopupMenu(m);

    delete m;
}

void CQueryParsePanel::OnButtonNext( wxCommandEvent& evt )
{
    m_QueryWidget->IterateSelection(1);
    // Fix here for some reason can mess up checkbox labels on windows, so don't use it.
    //m_QueryWidget->DlgOverlayFix(this->GetParent());
}

void CQueryParsePanel::OnButtonPrevious( wxCommandEvent& evt )
{    
    m_QueryWidget->IterateSelection(-1);  
    // Fix here for some reason can mess up checkbox labels on windows, so don't use it.
    //m_QueryWidget->DlgOverlayFix(this->GetParent());
}

bool CQueryParsePanel::IsSelectAll() 
{ 
    return m_pSelectAll->IsChecked();
}

void CQueryParsePanel::HideSelectAll()
{
    m_pSelectAll->SetValue(false);
    m_pSelectAll->Show(false);
}

void CQueryParsePanel::OnToggleSelectAll(wxCommandEvent & evt)
{
    // Enable or disable previous/next selected element based on whether
    // select all is checked.  In 'select all' mode, previous and next
    // do not do anything.
    bool select_all = m_pSelectAll->IsChecked();

    m_ToolBar->EnableTool(eCmdNext, true);
    m_ToolBar->EnableTool(eCmdPrevious, true);

    m_QueryWidget->SetSelectAll(select_all);
}

void CQueryParsePanel::OnToggleHideUnselected(wxCommandEvent & evt)
{  
    // Quick check to see if the query has already been run - if not,
    // clicking 'filter' will run the query too.
    string query = m_pSearch->GetUtf8();
    bool  casesensitive = m_CaseSensitive;

    // Re-run the query if options that can change query results have changed
    if (query!=m_PrevQuery || 
        casesensitive!=m_PrevCaseSensitive || 
        m_StringCompare != m_PrevStringCompare) {  
        x_ExecuteQuery(0);
    }

    m_QueryWidget->SetHideUnselected(m_ToolBar->GetToolState(eCmdFilter));
    
}

void CQueryParsePanel::OnStringMatchTypeBtn( wxCommandEvent& evt )
{
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    wxMenu* m = new wxMenu();
    unsigned int max_id = ID_STRING_MATCH_TYPE1;

    m->SetTitle(wxT("String Matching Options"));

    for (int i=0; i<kStringMatchTypesCount; ++i) {
        m->Append(max_id++, ToWxString(kStringMatchTypes[i]));
    }
    m->AppendSeparator();
    m->AppendCheckItem(max_id, ToWxString("Case Sensitive"));
    m->Check(max_id, m_CaseSensitive);

    m_ToolBar->PopupMenu(m);

    delete m;
}
void CQueryParsePanel::OnStringMatchTypeSelected(wxCommandEvent & evt)
{  
    m_QueryWidget->DlgOverlayFix(this->GetParent());
    unsigned int query_idx = evt.GetId() - ID_STRING_MATCH_TYPE1;
    string match_type = kStringMatchTypes[0];

    // If selected menu item is one of the phonetic matching choices
    if (query_idx < kStringMatchTypesCount) {
        match_type = kStringMatchTypesToolbar[query_idx];
        m_SearchType->SetLabel(ToWxString(match_type));
        m_StringCompare = kStringMatchEnumTypes[query_idx];
    }
    // If selected menu item is to turn case-sensitive on/off
    else if (query_idx == kStringMatchTypesCount) {      
         m_CaseSensitive = evt.IsChecked();
    }

    m_ToolBar->Realize();
}

///////////////////////////////////////////////////////////////////////////////
/// Class for extracting positions of data fields from the query.  A data field
/// is a query string token that matches a name in the dictionary.
///
class CQueryHighlightFunction
{
public: 
    CQueryHighlightFunction() {}

    std::vector<std::pair<size_t, size_t> > 
        GetFieldPositions() const { return m_FieldPositions; }
          
    ETreeTraverseCode 
    operator()(const CTreeNode<CQueryParseNode>& tr, int delta) 
    {
        if (delta < 0)
            return eTreeTraverse;

        const CQueryParseNode& qnode = tr.GetValue();

        const IQueryParseUserObject* uo = qnode.GetUserObject();
        const CQueryNodeValue* v = dynamic_cast<const CQueryNodeValue*>(uo);

        if (v != NULL && v->IsDataField()) {
            std::pair<size_t, size_t> field_position;

            // The length in SSrcLoc is left as 0
            field_position.first = qnode.GetLoc().pos;
            field_position.second = qnode.GetLoc().pos + qnode.GetStrValue().length();

            m_FieldPositions.push_back(field_position);
        }

        return eTreeTraverse;
    }


protected:
    std::vector<std::pair<size_t, size_t> > m_FieldPositions;
};


bool CQueryParsePanel::x_IsValidQuery(string query, 
                                      bool casesensitive,  
                                      vector<std::pair<size_t, size_t> >& fields)
{
    bool valid = (query.length() > 0);

    if (valid) {
        CQueryParseTree qtree;
        bool is_macro = false;
        if (query.length() > 5 &&
            !NStr::CompareNocase(query.substr(0, 5), "MACRO")) {
            is_macro = true;
            macro::CMacroRep* macro_rep = NULL;

            try {
                CRef<CMacroQueryExec> qexec(m_QueryDataSource->GetQueryExec(casesensitive, m_StringCompare));

                macro::CMacroParser p;
                qexec->GetFunctionNames(p);
                p.SetSource(query.c_str());
                p.Parse(true, qexec);
                macro_rep = p.DetachMacroRep();

                if (macro_rep->GetWhereClause() != NULL &&
                    macro_rep->GetDoTree() != NULL) {

                    CRef<objects::CScope> sc = m_QueryDataSource->GetScope();

                    CQueryExecPreProcessFunc  pre_process_exec(sc.GetPointerOrNull(),
                        *qexec);
                    TreeDepthFirstTraverse(*macro_rep->GetWhereClause()->GetQueryTree(),
                        pre_process_exec);
                    TreeDepthFirstTraverse(*macro_rep->GetDoTree()->GetQueryTree(),
                        pre_process_exec);

                    CQueryHighlightFunction field_finder;
                    field_finder = TreeDepthFirstTraverse(*macro_rep->GetWhereClause()->GetQueryTree(),
                        field_finder);

                    fields = field_finder.GetFieldPositions();

                    field_finder = TreeDepthFirstTraverse(*macro_rep->GetDoTree()->GetQueryTree(),
                        field_finder);

                    fields = field_finder.GetFieldPositions();
                }

                delete macro_rep;
            }
            catch (const CException& e) {
                LOG_POST(Info << "Error pre-processing macro: " << e.GetMsg());
                valid = false;
                delete macro_rep;
            }
        }
        else {
            try {
                // parse the query.  This will throw exceptions for syntax errors
                // like 'dist > 0.2 AND' but since it does not look at the tree's 
                // dictionary, strings/phrases that can't be promoted to a reasonable
                // value still pass, becuase they *could* be data fields, e.g.:
                // 'dist > 0.7 AND my dog likes to bark' will parse without error.   
                CRef<CMacroQueryExec> qexec(m_QueryDataSource->GetQueryExec(casesensitive, m_StringCompare));
                macro::CMacroParser p;
                qexec->GetFunctionNames(p);

                qtree.Parse(query.c_str(),
                    casesensitive ? CQueryParseTree::eCaseSensitiveUpper :
                                    CQueryParseTree::eCaseInsensitive,
                    CQueryParseTree::eSyntaxCheck,
                    false,
                    p.GetWhereFunctions());

                // If parsed query only has one top-level token then it is
                // a simple string which is always a valid query
                bool simple_string = false;
                if (qtree.GetQueryTree()->CountNodes() == 0 &&
                    qtree.GetQueryTree()->GetValue().IsValue() &&
                    qtree.GetQueryTree()->GetValue().GetStrValue() != "") {
                    simple_string = true;
                }

                if (!simple_string) {
                    CRef<CMacroQueryExec> qexec(m_QueryDataSource->GetQueryExec(casesensitive, m_StringCompare));

                    // Pre-process the query.  This step determines, to the extent
                    // possible, the types to be used in comaparison and whether a valid
                    // number of arguments are present. Where possible, static types
                    // (as opposed to identifers for field-values) are converted into
                    // their required types for comparisons.
                    CRef<objects::CScope> sc = m_QueryDataSource->GetScope();

                    CQueryExecPreProcessFunc  pre_process_exec(sc.GetPointerOrNull(),
                                                               *qexec);
                    TreeDepthFirstTraverse(*qtree.GetQueryTree(),
                                           pre_process_exec);

                    CQueryHighlightFunction field_finder;
                    field_finder = TreeDepthFirstTraverse(*qtree.GetQueryTree(),
                        field_finder);

                    fields = field_finder.GetFieldPositions();
                }
            }
            catch (const CException& e) {
                LOG_POST(Info << "Query Parse Error: " << e.GetMsg());
                valid = false;
            }
        }
    }

    // A simple string query.  Return true unless string is empty:
    return (valid);
}

void CQueryParsePanel::CompleteQuery(CMacroQueryExec* qexec, const string& status)
{
    // Let widget make any needed changes with query completed,
    // e.g. enable widget interaction, update selection set, etc.
    m_QueryWidget->QueryEnd(qexec);

    // Stop animation in query tool bar
    m_Animation->Stop();

    // Re-enable all input to window (and disable cancel button)
    this->SetCursor(*wxSTANDARD_CURSOR);
    m_ToolBar->EnableTool(eCmdStopQuery, false);
    m_ToolBar->EnableTool(eCmdStartQuery, true);
    m_ToolBar->EnableTool(eCmdNext, true);
    m_ToolBar->EnableTool(eCmdPrevious, true);
    m_ToolBar->EnableTool(eCmdStringMatchType, true);
    m_pSearch->Enable();
    m_pSelectAll->Enable();
    m_ToolBar->EnableTool(eCmdFilter, true);

    // Because we disable windows during the query we lose focus in the text 
    // box. This brings it back.  Now since the user can use other windows 
    // while search is running (if the search turns out to be long...), we 
    // only put focus back if it hasn't been assigned somewhere else.
    wxWindow* fwin = wxWindow::FindFocus();

    if (fwin == NULL)
        m_pSearch->SetFocus();

    if (status != "") {
        CQueryPanelEvent evt(status);
        Send(&evt, ePool_Parent);
    }

    // On mac (osx Cocoa) formatting can be lost after enter (when the wxTextCtrl
    // m_pSearch was disabled then re-enabled).  This re-computes the highlighting.
    // (note: formatting is not lost on other platforms when we do this, but no
    //  harm in re-setting it here)
    wxCommandEvent evt_dummy;
    OnSearchctrlTextEnter(evt_dummy);
}

void CQueryParsePanel::x_StartQueryJob(CRef<CMacroQueryExec> qexec,
                                       CQueryParseTree* qtree,
                                       macro::CMacroRep* macro_rep,
                                       bool casesensitive,
                                       const string& query)
{
    string result_status;

    // Pre-process the query.  This step determines, to the extent
    // possible, the types to be used in comparison and whether a valid
    // number of arguments are present. Where possible, static types
    // (as opposed to identifers for field-values) are converted into
    // their required types for comparisons.
    if (macro_rep) {
        try {
            CRef<CScope> scope = m_QueryDataSource->GetScope();
            CQueryExecPreProcessFunc  pre_process_exec(scope.GetPointerOrNull(), *qexec);

            CQueryParseTree* where_tree = macro_rep->GetWhereClause();
            if (where_tree) {
                TreeDepthFirstTraverse(*(where_tree->GetQueryTree()), pre_process_exec);
            }

            CQueryParseTree* do_tree = macro_rep->GetDoTree();
            if (do_tree) {
                TreeDepthFirstTraverse(*(do_tree->GetQueryTree()), pre_process_exec);
            }
        }
        catch (const CQueryExecException &e) {
            LOG_POST(Info << "Error pre-processing query: " << e.GetMsg());
        }
    }
    else if (qtree) {
        try {
            CRef<CScope> scope = m_QueryDataSource->GetScope();
            CQueryExecPreProcessFunc  pre_process_exec(scope.GetPointerOrNull(), *qexec);
            TreeDepthFirstTraverse(*(qtree->GetQueryTree()), pre_process_exec);

        }
        catch (CQueryExecException &e) {
            LOG_POST(Info << "Error pre-processing query: " << e.GetMsg());
        }
    }

    // Create a job to run in the background so that the user can still access
    // the cancel button or interact with other views while the job is
    // running
    try {
        if (m_Future.IsRunning()) {
            m_Future.cancel();
            GUI_AsyncJoinFuture(m_Future, "Canceling Search...");
        }
        m_Job.Reset();

        if (macro_rep) {
            m_Job.Reset(new CQueryJob(qexec, macro_rep, casesensitive, m_StringCompare));
        }
        else if (qtree) {
            m_Job.Reset(new CQueryJob(qexec, qtree, casesensitive, m_StringCompare));
        }
        else {
            m_Job.Reset(new CQueryJob(query, casesensitive, m_StringCompare, m_QueryDataSource));
        }

        m_Future = job_async([job = m_Job](ICanceled& canceled) mutable { job->Execute(canceled); }, "Querying Tree");
    }
    // Unable to start a job - try to do physics in foreground
    catch (CAppJobException&) {
        _TRACE("Query job start failed");
    }
}

void CQueryParsePanel::x_ExecuteQuery(int search_dir)
{
    string status_msg;

    // Execute the query.  If it is a simple string query, do it synchronously,
    // otherwise start a asynchronous job to complete the query.  If the query
    // is unchanged from the lasttime this funcion was called, do not re-execute
    // the query.
    if (m_ToolBar->GetToolEnabled(eCmdStartQuery)) {       
        m_Animation->Play();

        // Disable all input to window except the cancel button
        this->SetCursor(*wxHOURGLASS_CURSOR);
        this->Update();
        m_ToolBar->EnableTool(eCmdStopQuery, true);
        m_ToolBar->EnableTool(eCmdStartQuery, false);
        m_ToolBar->EnableTool(eCmdNext, false);
        m_ToolBar->EnableTool(eCmdPrevious, false);

        // Let host widget perform query-start actions (disable widget, set cursor...)
        m_QueryWidget->QueryStart();

        m_pSearch->Disable();        
        m_pSelectAll->Disable();
        m_ToolBar->EnableTool(eCmdFilter, false);
        m_ToolBar->EnableTool(eCmdStringMatchType, false);

        // Setting host widget cursor my also set it for toolbar but we don't want
        // that since the cancel button is still active.
        m_ToolBar->SetCursor(*wxSTANDARD_CURSOR);       
        m_ToolBar->SetCursor(*wxSTANDARD_CURSOR);       

        try {
            bool  casesensitive = m_CaseSensitive;

            // use the macro parser to get a list of function names that
            // may be found in the query:  
            CRef<CMacroQueryExec> qexec(m_QueryDataSource->GetQueryExec(casesensitive, m_StringCompare));
            macro::CMacroParser p;
            qexec->GetFunctionNames(p);
           
            string query = m_pSearch->GetUtf8();      

            m_PrevQuery = m_pSearch->GetUtf8();
            m_PrevCaseSensitive = casesensitive;
            m_PrevStringCompare = m_StringCompare;

            m_QueryDataSource->ClearQueryResults();

            CQueryParseTree* qtree = NULL;
            macro::CMacroRep* macro_rep = NULL;
            bool is_macro = false;

            try {
                // User May enter a macro or a query and they require different processing.
                // macros all start with 'macro' so check that first:
                if (query.length() > 5 && 
                    !NStr::CompareNocase(query.substr(0, 5), "MACRO")) {
                    is_macro = true;

                    try {
                        p.SetSource(query.c_str());
                        p.Parse(true, qexec);

                        macro_rep = p.DetachMacroRep();
                    }
                    catch (const CException& e) {
                        LOG_POST(Error << "macro exception: " << e.GetMsg());

                        status_msg = "Macro parsing failed: " + e.GetMsg();
                        CompleteQuery(qexec.GetPointer(), status_msg);
                        delete macro_rep;
                        return;
                    }
                }
                else {
                    // If query tree parsing is case-sensitive, keywords like
                    // AND and LIKE have to be uppercase, so we set that to
                    // parsing to be case insensitive.  The casesensitive we use 
                    // from the gui refers to string comparisons during execution and 
                    // does not effect parsing.             

                    qtree = new CQueryParseTree();
                    qtree->Parse(query.c_str(),
                        CQueryParseTree::eCaseInsensitive,
                        CQueryParseTree::eSyntaxCheck,
                        false,
                        p.GetWhereFunctions());

                    Flatten_ParseTree(*(qtree->GetQueryTree()));


                    CNcbiOstrstream  strstrm;
                    qtree->Print(strstrm);

                    // Logging merges lines (even if i call 
                    // UnsetDiagPostFlag(eDPF_PreMergeLines/eDPF_MergeLines); 
                    LOG_POST(Info << "Parsed Query: " << query);
                    vector<string> arr;
                    string s = (string)CNcbiOstrstreamToString(strstrm);
                    NStr::Split(s, "\n", arr);
                    for (size_t i = 0; i < arr.size(); ++i)
                        LOG_POST(Info << arr[i]);
                }
            }
            catch (CQueryParseException& e) {
                LOG_POST(Info << "Error parsing query: " << e.GetMsg());
                m_QueryDataSource->ClearQueryResults();
                delete qtree;
                qtree = NULL;

                // Return completion status to caller:                      
                status_msg = "Query parsing failed: " + e.GetMsg();
                CompleteQuery(qexec.GetPointer(), status_msg);
                return;
            }

            if (is_macro) {
                // Regular queries are handled asynchronously since they
                // may require network access, which could get tied up.
                x_StartQueryJob(qexec, nullptr, macro_rep, casesensitive, query);
            }
            else {
                // If parsed query only has one top-level token, set that as a simple
                // string query (when query tree is NULL, string query is run)
                if (qtree->GetQueryTree()->CountNodes() == 0 &&
                    qtree->GetQueryTree()->GetValue().IsValue() &&
                    qtree->GetQueryTree()->GetValue().GetStrValue() != "") {
                    // parsed query will remove double-quotes
                    query = qtree->GetQueryTree()->GetValue().GetStrValue();
                    delete qtree;
                    qtree = NULL;
                }

                // Regular queries are handled asynchronously since they
                // may require network access, which could get tied up.
                x_StartQueryJob(qexec, qtree, macro_rep, casesensitive, query);
            }

            x_AddRecentQuery(query);
        }
        // If this widget has been marked for deletion and has an exception,
        // return For a normal (query execution) exception without deletion,
        // continue normal processing.
        catch (CException&) {
        }
    }
}

void CQueryParsePanel::OnIdle(wxIdleEvent& event)
{
    if (m_Future.IsComplete()) {
        try {
            m_Future();
        } NCBI_CATCH("CQueryParsePanel search.");

        if (!m_Job)
            return;

        string result_status;

        // If the job is finished, collect results and send completion message
        // to widget
        if (!m_Future.IsCanceled()) {
            LOG_POST(Info << "Number evaluated: " <<
                m_Job->GetQueriedCount() <<
                "  Number of query execution errors: " <<
                m_Job->GetExceptionCount() <<
                string(" Query Time: ") << m_Job->GetQueryTime());

            result_status = string("Number evaluated: ") +
                NStr::IntToString(m_Job->GetQueriedCount()) +
                string(". Number Selected: ") +
                NStr::IntToString(m_Job->GetNumSelected()) +
                string(". Query Time: ") + NStr::DoubleToString(m_Job->GetQueryTime());
            if (m_Job->GetExceptionCount() > 0) {
                result_status += string(" Number with query errors: ") +
                    NStr::IntToString(m_Job->GetExceptionCount());
            }

            // Set the selection set to the returned entries 
            if (m_Job->GetQueryExec() != NULL)
                m_QueryDataSource->SetQueryResults(m_Job->GetQueryExec());
        }
        // Done with job, but we don't want the results:
        else {
            LOG_POST(Info << "Number evaluated (for cancelled search): " <<
                m_Job->GetQueriedCount() <<
                " Number with query execution errors: " <<
                m_Job->GetExceptionCount());

            result_status = string("Number evaluated (for cancelled search): ") +
                NStr::IntToString(m_Job->GetQueriedCount()) +
                string(". Number Selected: ") +
                NStr::IntToString(m_Job->GetNumSelected()) + string(".");
            if (m_Job->GetExceptionCount() > 0) {
                result_status += string(" Number with query errors: ") +
                    NStr::IntToString(m_Job->GetExceptionCount());
            }
        }

        CompleteQuery(m_Job->GetQueryExec(), result_status);

        // Delete the completed job object
        m_Job.Reset();

        m_Future.reset();
    }
    else
        event.Skip();
}

void CQueryParsePanel::x_CancelQueryInProgress()
{
    if (m_Future.IsRunning()) {
        m_Future.cancel();

        // by default, we do not run the same query twice (since the results
        // are already there) but if the job is cancelled, this allows the
        // query to be re-executed.
        m_PrevQuery = "";
    }
}

void CQueryParsePanel::x_AddRecentQuery(const std::string& q)
{
    if (q == "")
        return;

    vector<pair<string,string> >::iterator iter;
    for (iter = m_RecentQueries.begin(); iter != m_RecentQueries.end(); ++iter) {
        if ((*iter).second == q) {
            m_RecentQueries.erase(iter);
            break;
        }
    }

    m_RecentQueries.insert(m_RecentQueries.begin(), pair<string,string>(string(), q));

    if (m_RecentQueries.size() > (unsigned int)m_MaxRecentQueries)
        m_RecentQueries.erase(m_RecentQueries.begin() + m_RecentQueries.size()-1);
}

END_NCBI_SCOPE
