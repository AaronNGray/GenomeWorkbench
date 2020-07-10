/*  $Id: generic_report_dlg.hpp 41125 2018-05-29 19:08:26Z filippov $
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
 * Authors:  Colleen Bollin
 */

#ifndef _GENERIC_REPORT_DLG_H_
#define _GENERIC_REPORT_DLG_H_

#include <corelib/ncbistd.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>
#include <wx/string.h>
#include <gui/widgets/wx/gui_widget_dlg.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/widgets/edit/generic_panel.hpp>  


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CGENERICREPORTDLG 10357
#define SYMBOL_CGENERICREPORTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CGENERICREPORTDLG_TITLE _("Generic Report")
#define SYMBOL_CGENERICREPORTDLG_IDNAME ID_CGENERICREPORTDLG
#define SYMBOL_CGENERICREPORTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CGENERICREPORTDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGenericReportDlg class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CGenericReportDlg: public wxFrame
{    
    DECLARE_DYNAMIC_CLASS( CGenericReportDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGenericReportDlg();
    CGenericReportDlg( wxWindow* parent,  IWorkbench* workbench = NULL, 
                        wxWindowID id = SYMBOL_CGENERICREPORTDLG_IDNAME, 
                        const wxString& caption = SYMBOL_CGENERICREPORTDLG_TITLE, 
                        const wxPoint& pos = SYMBOL_CGENERICREPORTDLG_POSITION, 
                        const wxSize& size = SYMBOL_CGENERICREPORTDLG_SIZE, 
                        long style = SYMBOL_CGENERICREPORTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                wxWindowID id = SYMBOL_CGENERICREPORTDLG_IDNAME, 
                const wxString& caption = SYMBOL_CGENERICREPORTDLG_TITLE, 
                const wxPoint& pos = SYMBOL_CGENERICREPORTDLG_POSITION, 
                const wxSize& size = SYMBOL_CGENERICREPORTDLG_SIZE, 
                long style = SYMBOL_CGENERICREPORTDLG_STYLE );

    /// Destructor
    ~CGenericReportDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CGenericReportDlg event handler declarations

    void OnCloseWindow(wxCloseEvent& event);

////@end CGenericReportDlg event handler declarations
    
////@begin CGenericReportDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenericReportDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetLineSpacing(int spacing);
    void SetText( const wxString& text );
    void SetRefresh( IRefreshCntrl * refresh );
    void RefreshData();

    void SetWorkDir(const wxString& workDir);

    void SetRegistryPath(const string& reg_path);
    void LoadSettings();
    void SaveSettings() const;

private:
    CGenericPanel* m_GenericPanel;
    string m_RegPath;
    IWorkbench* m_Workbench;
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CGenericReportDlgModal: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CGenericReportDlgModal )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGenericReportDlgModal();
    CGenericReportDlgModal( wxWindow* parent,  IWorkbench* workbench = NULL, 
                        wxWindowID id = SYMBOL_CGENERICREPORTDLG_IDNAME, 
                        const wxString& caption = SYMBOL_CGENERICREPORTDLG_TITLE, 
                        const wxPoint& pos = SYMBOL_CGENERICREPORTDLG_POSITION, 
                        const wxSize& size = SYMBOL_CGENERICREPORTDLG_SIZE, 
                        long style = SYMBOL_CGENERICREPORTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                wxWindowID id = SYMBOL_CGENERICREPORTDLG_IDNAME, 
                const wxString& caption = SYMBOL_CGENERICREPORTDLG_TITLE, 
                const wxPoint& pos = SYMBOL_CGENERICREPORTDLG_POSITION, 
                const wxSize& size = SYMBOL_CGENERICREPORTDLG_SIZE, 
                long style = SYMBOL_CGENERICREPORTDLG_STYLE );

    /// Destructor
    ~CGenericReportDlgModal();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CGenericReportDlg event handler declarations

    void OnCloseWindow(wxCloseEvent& event);

////@end CGenericReportDlg event handler declarations
    
////@begin CGenericReportDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenericReportDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetText( const wxString& text );
    void SetRefresh( IRefreshCntrl * refresh );
    void RefreshData();

private:
    CGenericPanel* m_GenericPanel;

    IWorkbench* m_Workbench;
};


END_NCBI_SCOPE

#endif
    // _GENERIC_REPORT_DLG_H_
