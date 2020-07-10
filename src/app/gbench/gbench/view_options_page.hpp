#ifndef GUI_APP_GBENCH_NEW___VIEW_OPTIONS_PAGE__HPP
#define GUI_APP_GBENCH_NEW___VIEW_OPTIONS_PAGE__HPP

/*  $Id: view_options_page.hpp 39352 2017-09-14 15:38:35Z katargir $
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

/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
#include "wx/valgen.h"
////@end includes

#include <wx/panel.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxCheckBox;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CVIEWOPTIONSPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CVIEWOPTIONSPAGE_TITLE _("General")
#define SYMBOL_CVIEWOPTIONSPAGE_IDNAME ID_CVIEWOPTIONSPAGE
#define SYMBOL_CVIEWOPTIONSPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CVIEWOPTIONSPAGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CViewOptionsPage class declaration
 */

class CViewOptionsPage: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CViewOptionsPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CViewOptionsPage();
    CViewOptionsPage( wxWindow* parent, wxWindowID id = SYMBOL_CVIEWOPTIONSPAGE_IDNAME, const wxPoint& pos = SYMBOL_CVIEWOPTIONSPAGE_POSITION, const wxSize& size = SYMBOL_CVIEWOPTIONSPAGE_SIZE, long style = SYMBOL_CVIEWOPTIONSPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CVIEWOPTIONSPAGE_IDNAME, const wxPoint& pos = SYMBOL_CVIEWOPTIONSPAGE_POSITION, const wxSize& size = SYMBOL_CVIEWOPTIONSPAGE_SIZE, long style = SYMBOL_CVIEWOPTIONSPAGE_STYLE );

    /// Destructor
    ~CViewOptionsPage();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CViewOptionsPage event handler declarations

////@end CViewOptionsPage event handler declarations

////@begin CViewOptionsPage member function declarations

    wxString GetAutoSaveInterval() const { return m_AutoSaveInterval ; }
    void SetAutoSaveInterval(wxString value) { m_AutoSaveInterval = value ; }

    bool GetReportUsage() const { return m_ReportUsage ; }
    void SetReportUsage(bool value) { m_ReportUsage = value ; }

    int GetViewTitle() const { return m_ViewTitle ; }
    void SetViewTitle(int value) { m_ViewTitle = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CViewOptionsPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CViewOptionsPage member variables
    wxCheckBox* m_EmuilateExtCheckBox;
private:
    wxString m_AutoSaveInterval;
    bool m_DisableUpdates;
    bool m_HideDisabledProjectItems;
    bool m_LogStack;
    bool m_ReportUsage;
    int m_ViewTitle;
    /// Control identifiers
    enum {
        ID_CVIEWOPTIONSPAGE = 10034,
        ID_TEXTCTRL3 = 10023,
        ID_RADIOBOX = 10035,
        ID_DISABLE_UPDATES_CHECKBOX = 10001,
        ID_HIDE_DISABLED_CHECKBOX = 10000,
        ID_CHECKBOX2 = 10037,
        ID_REPORT_USAGE_CHECKBOX = 10002,
        ID_EMULATE_EXTERNAL_CHECKBOX = 10003
    };
////@end CViewOptionsPage member variables

    virtual bool TransferDataFromWindow();
};

END_NCBI_SCOPE

#endif  // GUI_APP_GBENCH_NEW___VIEW_OPTIONS_PAGE__HPP
