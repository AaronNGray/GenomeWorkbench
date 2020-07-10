/*  $Id: generic_panel.hpp 43609 2019-08-08 16:12:53Z filippov $
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
 * Authors:  Colleen Bollin, Andrea Asztalos
 */

#ifndef _GENERIC_PANEL_H_
#define _GENERIC_PANEL_H_

#include <corelib/ncbistd.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/choice.h>
#include <wx/textctrl.h>

class wxHyperlinkCtrl;

BEGIN_NCBI_SCOPE

class CGenericPanel;

class IRefreshCntrl
{
public:
    virtual ~IRefreshCntrl() {}
    virtual string GetRefreshedText() = 0;
    virtual void RefreshText(CGenericPanel* text) = 0;
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CGenTextCtrl : public wxTextCtrl
{
public:
    CGenTextCtrl() : wxTextCtrl(), m_scroll_pos(0) {}
    CGenTextCtrl(wxWindow *parent, wxWindowID id, const wxString &value = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxTextCtrlNameStr) : wxTextCtrl(parent, id, value, pos, size, style, validator, name), m_scroll_pos(0), m_safe_to_store(true) {}
    virtual ~CGenTextCtrl() {}
    void StorePosition();
    void RestorePosition();
    void OnKillFocus(wxFocusEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnSetChildFocus(wxChildFocusEvent& event);
    void OnActivate(wxActivateEvent& event);
    void OnScroll(wxScrollWinEvent& event);
    void OnScrollWinTop(wxScrollWinEvent& event);
    void OnIdle(wxIdleEvent&);
    void ShowPosition(long pos);
protected:
    wxDECLARE_NO_COPY_CLASS(CGenTextCtrl);
    DECLARE_EVENT_TABLE()
private:
    long m_scroll_pos;
    bool m_safe_to_store;
};

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGENERICPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CGENERICPANEL_IDNAME ID_CGENERICPANEL
#define SYMBOL_CGENERICPANEL_SIZE wxDefaultSize
#define SYMBOL_CGENERICPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGenericPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CGenericPanel : public wxPanel
{    
    DECLARE_DYNAMIC_CLASS(CGenericPanel)
    DECLARE_EVENT_TABLE()
public:
    /// Constructors
    CGenericPanel();
    CGenericPanel(wxWindow* parent,
                IWorkbench* workbench = NULL, 
                wxWindowID id = SYMBOL_CGENERICPANEL_IDNAME,
                const wxPoint& pos = SYMBOL_CGENERICPANEL_POSITION,
                const wxSize& size = SYMBOL_CGENERICPANEL_SIZE,
                long style = SYMBOL_CGENERICPANEL_STYLE);

    CGenericPanel(wxWindow* parent,
        bool simple,
        IWorkbench* workbench = NULL,
        wxWindowID id = SYMBOL_CGENERICPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CGENERICPANEL_POSITION,
        const wxSize& size = SYMBOL_CGENERICPANEL_SIZE,
        long style = SYMBOL_CGENERICPANEL_STYLE);

    /// Creation
    bool Create( wxWindow* parent, 
                wxWindowID id = SYMBOL_CGENERICPANEL_IDNAME,
                const wxPoint& pos = SYMBOL_CGENERICPANEL_POSITION,
                const wxSize& size = SYMBOL_CGENERICPANEL_SIZE,
                long style = SYMBOL_CGENERICPANEL_STYLE);

    /// Destructor
    ~CGenericPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void OnCloseButton(wxCommandEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERIC_REPORT_FIND
    void OnGenericReportFindClick(wxCommandEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERIC_REPORT_FIND2
    void OnGenericReportFindClick2(wxCommandEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GEN_RPT_REFRESH
    void OnGenRptRefreshClick(wxCommandEvent& event);
    void OnGenRptRefreshClick2(wxCommandEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_GENERIC_REPORT_EXPORT
    void OnGenericReportExportClick(wxCommandEvent& event);    
    void SetHelpUrl(const wxString &url);

////@begin CGenericPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenericPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    int GetFindMode() { return m_FindModeCtrl->GetSelection();}
    void SetFindMode(int mode) { m_FindModeCtrl->SetSelection(mode);}
////@begin CGenericPanel member variables
    CGenTextCtrl* m_RTCtrl;
    wxTextCtrl* m_FindText;
    wxTextCtrl* m_FindText2;
    wxButton* m_RefreshBtn;
    wxButton* m_RefreshBtn2;
    wxChoice* m_FindModeCtrl;
    wxHyperlinkCtrl* m_HelpButton;
////@end CGenericPanel member variables

    void SetLineSpacing(int spacing);
    void SetText( const wxString& text );
    void SetRefresh( IRefreshCntrl * refresh );
    void RefreshData();

    void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }
    
    enum {
        ID_CGENERICPANEL = 10350,
        ID_RICHTEXTCTRL = 10363,
        ID_GENERIC_REPORT_FIND = 10414,
        ID_GENERIC_REPORT_FIND2 = 10415,
        ID_GENERIC_REPORT_CLOSE = 10416,
        ID_GENERIC_REPORT_CLOSE2 = 10417,
        ID_GENERIC_REPORT_EXPORT = 10418,
        ID_GENERIC_REPORT_TEXT_FIND = 10419,
        ID_GENERIC_REPORT_TEXT_FIND2 = 10420,
        ID_GEN_RPT_REFRESH = 10406,
        ID_GEN_RPT_REFRESH2 = 10407
    };
    
protected:
    IRefreshCntrl * m_Refresh;
    long m_PrevFindPos;
    string m_PrevFindText;
private:
    void SetFontAsTextPanel();
    IWorkbench* m_Workbench;
    bool m_move_to_top;
    wxString m_WorkDir;    
    bool m_busy;
    bool m_Simple;    
};


END_NCBI_SCOPE

#endif
    // _GENERIC_PANEL_H_
