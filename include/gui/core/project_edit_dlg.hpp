#ifndef GUI_CORE___PROJECT_EDIT_DLG__HPP
#define GUI_CORE___PROJECT_EDIT_DLG__HPP

/*  $Id: project_edit_dlg.hpp 28647 2013-08-09 20:16:42Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dialog.hpp>

////@begin includes
#include "wx/valgen.h"
////@end includes


////@begin control identifiers
#define SYMBOL_CPROJECTEDITDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CPROJECTEDITDLG_TITLE _("Dialog")
#define SYMBOL_CPROJECTEDITDLG_IDNAME ID_CPROJECTEDITDLG
#define SYMBOL_CPROJECTEDITDLG_SIZE wxDefaultSize
#define SYMBOL_CPROJECTEDITDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CProjectEditDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CProjectEditDlg )
    DECLARE_EVENT_TABLE()

public:
    CProjectEditDlg();
    CProjectEditDlg( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTEDITDLG_IDNAME, const wxString& caption = SYMBOL_CPROJECTEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CPROJECTEDITDLG_POSITION, const wxSize& size = SYMBOL_CPROJECTEDITDLG_SIZE, long style = SYMBOL_CPROJECTEDITDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTEDITDLG_IDNAME, const wxString& caption = SYMBOL_CPROJECTEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CPROJECTEDITDLG_POSITION, const wxSize& size = SYMBOL_CPROJECTEDITDLG_SIZE, long style = SYMBOL_CPROJECTEDITDLG_STYLE );

    ~CProjectEditDlg();

    void Init();

    void CreateControls();

////@begin CProjectEditDlg event handler declarations

    void OnOkClick( wxCommandEvent& event );

////@end CProjectEditDlg event handler declarations

////@begin CProjectEditDlg member function declarations

    wxString GetDescr() const { return m_Descr ; }
    void SetDescr(wxString value) { m_Descr = value ; }

    wxString GetFilename() const { return m_Filename ; }
    void SetFilename(wxString value) { m_Filename = value ; }

    wxString GetCreated() const { return m_Created ; }
    void SetCreated(wxString value) { m_Created = value ; }

    wxString GetModified() const { return m_Modified ; }
    void SetModified(wxString value) { m_Modified = value ; }

    wxString GetProjectName() const { return m_Name ; }
    void SetProjectName(wxString value) { m_Name = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CProjectEditDlg member function declarations

    void SetReadOnly();

    static bool ShowToolTips();


////@begin CProjectEditDlg member variables
protected:
    wxString m_Descr;
    wxString m_Filename;
    wxString m_Created;
    wxString m_Modified;
    wxString m_Name;
    enum {
        ID_CPROJECTEDITDLG = 10027,
        ID_NAME = 10000,
        ID_DESCR = 10003,
        ID_FILENAME = 10001,
        ID_CREATED = 10002,
        ID_MODIFIED = 10004
    };
////@end CProjectEditDlg member variables
};

END_NCBI_SCOPE

#endif // GUI_CORE___PROJECT_EDIT_DLG__HPP
