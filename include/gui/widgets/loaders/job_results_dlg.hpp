#ifndef GUI_WIDGETS___LOADERS___JOB_RESULTS_DLG__HPP
#define GUI_WIDGETS___LOADERS___JOB_RESULTS_DLG__HPP

/*  $Id: job_results_dlg.hpp 44093 2019-10-24 20:19:01Z filippov $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */


/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <wx/dialog.h>

////@begin includes
#include "wx/html/htmlwin.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CJOBRESULTSDLG 10049
#define ID_HTMLWINDOW1 10051
#define ID_JOB_RESULTS_DLG_EXPORT 10052
#define SYMBOL_CJOBRESULTSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CJOBRESULTSDLG_TITLE _("Job Results")
#define SYMBOL_CJOBRESULTSDLG_IDNAME ID_CJOBRESULTSDLG
#define SYMBOL_CJOBRESULTSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CJOBRESULTSDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CJobResultsDlg class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CJobResultsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CJobResultsDlg )
    DECLARE_EVENT_TABLE()

public:
    CJobResultsDlg();
    CJobResultsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CJOBRESULTSDLG_IDNAME, const wxString& caption = SYMBOL_CJOBRESULTSDLG_TITLE, const wxPoint& pos = SYMBOL_CJOBRESULTSDLG_POSITION, const wxSize& size = SYMBOL_CJOBRESULTSDLG_SIZE, long style = SYMBOL_CJOBRESULTSDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CJOBRESULTSDLG_IDNAME, const wxString& caption = SYMBOL_CJOBRESULTSDLG_TITLE, const wxPoint& pos = SYMBOL_CJOBRESULTSDLG_POSITION, const wxSize& size = SYMBOL_CJOBRESULTSDLG_SIZE, long style = SYMBOL_CJOBRESULTSDLG_STYLE );

    ~CJobResultsDlg();

    void Init();

    void CreateControls();

////@begin CJobResultsDlg event handler declarations
    void OnExportClick(wxCommandEvent& event);
////@end CJobResultsDlg event handler declarations

////@begin CJobResultsDlg member function declarations

    wxString GetHTML() const { return m_HTML ; }
    void SetHTML(wxString value) { m_HTML = value ; }

    void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CJobResultsDlg member function declarations

    static bool ShowToolTips();

////@begin CJobResultsDlg member variables
private:
    wxString m_HTML;
    wxString m_WorkDir;    
////@end CJobResultsDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___JOB_RESULTS_DLG__HPP
