#ifndef GUI_WIDGETS_WX___TEXT_REPORT_DLG_H
#define GUI_WIDGETS_WX___TEXT_REPORT_DLG_H

/*  $Id: text_report_dlg.hpp 44437 2019-12-18 19:08:05Z asztalos $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

/*!
 * Includes
 */

#include <wx/dialog.h>


////@begin includes
#include "wx/html/htmlwin.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxHtmlWindow;
class wxStaticBitmap;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTEXTREPORTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTEXTREPORTDLG_TITLE _("Text Report Dialog")
#define SYMBOL_CTEXTREPORTDLG_IDNAME ID_CTEXTREPORTDLG
#define SYMBOL_CTEXTREPORTDLG_SIZE wxSize(400, 200)
#define SYMBOL_CTEXTREPORTDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTextReportDlg class declaration
 */

class NCBI_GUIWIDGETS_WX_EXPORT CTextReportDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CTextReportDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTextReportDlg();
    CTextReportDlg( wxWindow* parent, wxWindowID id = SYMBOL_CTEXTREPORTDLG_IDNAME, const wxString& caption = SYMBOL_CTEXTREPORTDLG_TITLE, const wxPoint& pos = SYMBOL_CTEXTREPORTDLG_POSITION, const wxSize& size = SYMBOL_CTEXTREPORTDLG_SIZE, long style = SYMBOL_CTEXTREPORTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTEXTREPORTDLG_IDNAME, const wxString& caption = SYMBOL_CTEXTREPORTDLG_TITLE, const wxPoint& pos = SYMBOL_CTEXTREPORTDLG_POSITION, const wxSize& size = SYMBOL_CTEXTREPORTDLG_SIZE, long style = SYMBOL_CTEXTREPORTDLG_STYLE );

    /// Destructor
    ~CTextReportDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTextReportDlg event handler declarations

    /// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_TEXTCTRL1
    void OnTextctrl1LinkClicked( wxHtmlLinkEvent& event );

////@end CTextReportDlg event handler declarations

////@begin CTextReportDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTextReportDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTextReportDlg member variables
    wxStaticBitmap* m_Bitmap;
    wxHtmlWindow* m_TextCtrl;
    /// Control identifiers
    enum {
        ID_CTEXTREPORTDLG = 10000,
        ID_TEXTCTRL1 = 10001
    };
////@end CTextReportDlg member variables
    void SetText(wxString value) {
        m_TextCtrl->SetPage(value);
    }

    void SetIcon(EDialogIcon icon);
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___TEXT_REPORT_DLG_H
