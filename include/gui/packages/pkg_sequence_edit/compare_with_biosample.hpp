/*  $Id: compare_with_biosample.hpp 43953 2019-09-25 18:13:27Z filippov $
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
 * Authors:  Igor Filippov
 */
#ifndef _COMPARE_WITH_BIOSAMPLE_H_
#define _COMPARE_WITH_BIOSAMPLE_H_

#include <corelib/ncbistd.hpp>
#include <connect/ncbi_http_session.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_select.hpp>
#include <gui/packages/pkg_sequence_edit/apply_edit_convert_panel.hpp>
#include <misc/biosample_util/biosample_util.hpp>

#include <wx/dialog.h>
BEGIN_NCBI_SCOPE


#define SYMBOL_COMPARE_WITH_BIOSAMPLE_STYLE wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX|wxCAPTION|wxCLIP_CHILDREN
#define SYMBOL_COMPARE_WITH_BIOSAMPLE_TITLE _("Compare with Biosample")
#define SYMBOL_COMPARE_WITH_BIOSAMPLE_IDNAME wxID_ANY
#define SYMBOL_COMPARE_WITH_BIOSAMPLE_SIZE wxDefaultSize
#define SYMBOL_COMPARE_WITH_BIOSAMPLE_POSITION wxDefaultPosition
#define ID_COPY_TO_SOURCE 12001
#define ID_COPY_TO_SAMPLE 12002
#define ID_SYNC           12003
#define ID_MAKE_REPORT    12005
#define ID_CREATE_BIOSAMPLE 12006
#define ID_UPDATE_BIOSOURCE 12007
#define ID_REFRESH_BIOSAMPLE 12008
#define ID_LOGOUT_BIOSAMPLE 12009
#define ID_ADD_TO_SOURCE 12010
#define ID_ADD_TO_SAMPLE 12011

class  CCompareWithBiosample : public wxFrame
{    
    DECLARE_DYNAMIC_CLASS(  CCompareWithBiosample )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCompareWithBiosample();
    CCompareWithBiosample( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, bool all_records,
              wxWindowID id = SYMBOL_COMPARE_WITH_BIOSAMPLE_IDNAME, const wxString& caption = SYMBOL_COMPARE_WITH_BIOSAMPLE_TITLE, const wxPoint& pos = SYMBOL_COMPARE_WITH_BIOSAMPLE_POSITION, const wxSize& size = SYMBOL_COMPARE_WITH_BIOSAMPLE_SIZE, long style = SYMBOL_COMPARE_WITH_BIOSAMPLE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COMPARE_WITH_BIOSAMPLE_IDNAME, const wxString& caption = SYMBOL_COMPARE_WITH_BIOSAMPLE_TITLE, const wxPoint& pos = SYMBOL_COMPARE_WITH_BIOSAMPLE_POSITION, const wxSize& size = SYMBOL_COMPARE_WITH_BIOSAMPLE_SIZE, long style = SYMBOL_COMPARE_WITH_BIOSAMPLE_STYLE );

    /// Destructor
    ~CCompareWithBiosample();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    CRef<objects::CSeq_table> GetValuesTableFromSeqEntry();
    CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table) { CRef<objects::CSeq_table> choices; return choices; } ;
    int GetCollapsible() {return 0;}

    string GetErrorMessage();
    bool IsReadOnlyColumn(string column_name) { if (column_name == "Field" || column_name == "Biosample Id" || column_name.empty() || column_name == "expand") return true; return false; };
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );
    
    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
    
    /// Should we show tooltips?
    static bool ShowToolTips();
    void OnClickOk( wxCommandEvent& event );
    void OnClickCancel( wxCommandEvent& event );
    void GetSelectedRows(std::set<int> &selected);
    void OnSyncButtonClick( wxCommandEvent& event );
    void OnCopyToSourceButtonClick( wxCommandEvent& event );
    void OnCopyToSampleButtonClick( wxCommandEvent& event );
    void OnAddToSource( wxCommandEvent& event );
    void OnAddToSample( wxCommandEvent& event );
    void MakeReport(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnLogout(wxCommandEvent& event);
    void CreateBiosampleUpdateWebService(wxCommandEvent& event);
private:
    void LoadData();
    void ReloadTable();
    void AddTablePanel(CRef<objects::CSeq_table> values_table);
    bool TestCookie();
    void SaveSettings() const;
    void LoadSettings();
    bool Authenticate();
    wxString ReportBiosampleUpdate(const string &str);

    wxBoxSizer* m_GridPanelSizer;
    wxGrid* m_Grid;
    CSeqTableGridPanel* m_GridPanel;
    CRef<objects::CSeqTable_column> m_id_col;
    CStringConstraintSelect* m_StringConstraintPanel;
    bool m_Modified;
    CApplyEditconvertPanel *m_AecrPanel;
    biosample_util::TBioSamples m_Cache;
    map<pair<string, string>, unordered_map<string, pair<set<string>, set<string> > > >  m_DiffVec;
    int m_source_col, m_sample_col, m_field_col, m_sample_id_col;
    bool m_modified_sample, m_modified_source;
    CSeq_entry_Handle m_TopSeqEntry;
    ICommandProccessor* m_CmdProcessor;
    map<string, CConstRef<CSeq_id> > m_ids;
    CHttpCookie m_cookie;
    bool m_all_records;
    wxButton *m_RefreshButton;
    class CRefreshTimer : public wxTimer
    {
        wxButton* m_button;
    public:
        CRefreshTimer() : wxTimer(), m_button(NULL) {}
	void SetButton(wxButton *button) {m_button = button;}
        void Notify() 
            {
		if (m_button)
		    m_button->Enable();
		if (IsRunning())
		    Stop();
            }
    };

    CRefreshTimer m_timer;
};

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CLOGINDIALOG 13000
#define ID_USERNAMETEXTCTRL 13001
#define ID_PASSWORDTEXTCTRL 13002
#define SYMBOL_CLOGINDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CLOGINDIALOG_TITLE _("Login to MyNCBI")
#define SYMBOL_CLOGINDIALOG_IDNAME ID_CLOGINDIALOG
#define SYMBOL_CLOGINDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CLOGINDIALOG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLoginDialog class declaration
 */

class CLoginDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CLoginDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLoginDialog();
    CLoginDialog( wxWindow* parent, wxWindowID id = SYMBOL_CLOGINDIALOG_IDNAME, const wxString& caption = SYMBOL_CLOGINDIALOG_TITLE, const wxPoint& pos = SYMBOL_CLOGINDIALOG_POSITION, const wxSize& size = SYMBOL_CLOGINDIALOG_SIZE, long style = SYMBOL_CLOGINDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLOGINDIALOG_IDNAME, const wxString& caption = SYMBOL_CLOGINDIALOG_TITLE, const wxPoint& pos = SYMBOL_CLOGINDIALOG_POSITION, const wxSize& size = SYMBOL_CLOGINDIALOG_SIZE, long style = SYMBOL_CLOGINDIALOG_STYLE );

    /// Destructor
    ~CLoginDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CLoginDialog event handler declarations

////@end CLoginDialog event handler declarations

////@begin CLoginDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLoginDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    string GetUsername() {return m_Username->GetValue().ToStdString();}
    string GetPassword() {return m_Password->GetValue().ToStdString();}
private:
////@begin CLoginDialog member variables
    wxTextCtrl* m_Username;
    wxTextCtrl* m_Password;
////@end CLoginDialog member variables
};

END_NCBI_SCOPE
#endif
