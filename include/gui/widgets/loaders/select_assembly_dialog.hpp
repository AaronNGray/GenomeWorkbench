#ifndef GUI_WIDGETS___LOADERS___SELECT_ASSEMBLY_DIALOG___HPP
#define GUI_WIDGETS___LOADERS___SELECT_ASSEMBLY_DIALOG___HPP

/*  $Id: select_assembly_dialog.hpp 39737 2017-10-31 17:03:07Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/dialog.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAssemblyListPanel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSELECTASSEMBLYDIALOG 10096
#define ID_SELECTASSEMBLYPANEL 10033
#define SYMBOL_CSELECTASSEMBLYDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxWANTS_CHARS|wxTAB_TRAVERSAL
#define SYMBOL_CSELECTASSEMBLYDIALOG_TITLE _("Select Assembly")
#define SYMBOL_CSELECTASSEMBLYDIALOG_IDNAME ID_CSELECTASSEMBLYDIALOG
#define SYMBOL_CSELECTASSEMBLYDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CSELECTASSEMBLYDIALOG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CAssemblyListPanel;

class NCBI_GUIWIDGETS_LOADERS_EXPORT CSelectAssemblyDialog: public  CDialog
{    
    DECLARE_DYNAMIC_CLASS( CSelectAssemblyDialog )
    DECLARE_EVENT_TABLE()

public:
    CSelectAssemblyDialog();
    CSelectAssemblyDialog( wxWindow* parent, wxWindowID id = SYMBOL_CSELECTASSEMBLYDIALOG_IDNAME, const wxString& caption = SYMBOL_CSELECTASSEMBLYDIALOG_TITLE, const wxPoint& pos = SYMBOL_CSELECTASSEMBLYDIALOG_POSITION, const wxSize& size = SYMBOL_CSELECTASSEMBLYDIALOG_SIZE, long style = SYMBOL_CSELECTASSEMBLYDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSELECTASSEMBLYDIALOG_IDNAME, const wxString& caption = SYMBOL_CSELECTASSEMBLYDIALOG_TITLE, const wxPoint& pos = SYMBOL_CSELECTASSEMBLYDIALOG_POSITION, const wxSize& size = SYMBOL_CSELECTASSEMBLYDIALOG_SIZE, long style = SYMBOL_CSELECTASSEMBLYDIALOG_STYLE );

    ~CSelectAssemblyDialog();

    void Init();

    void CreateControls();

    virtual bool TransferDataFromWindow();

////@begin CSelectAssemblyDialog event handler declarations

    /// wxEVT_INIT_DIALOG event handler for ID_CSELECTASSEMBLYDIALOG
    void OnInitDialog( wxInitDialogEvent& event );

////@end CSelectAssemblyDialog event handler declarations

////@begin CSelectAssemblyDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSelectAssemblyDialog member function declarations

    static bool ShowToolTips();

////@begin CSelectAssemblyDialog member variables
    CAssemblyListPanel* m_AssemblyPanel;
////@end CSelectAssemblyDialog member variables

public:

    /// Set assembly query search term
    void SetAssmSearchTerm(const string& str);

    /// Get assembly query search term
    string GetAssmSearchTerm() const;

    /// Get selected assemblies, if any
    string GetSelectedAssembly(string& name, string& description) const;

    void SetRegistryPath(const string& path);

protected:
    virtual void x_LoadSettings(const CRegistryReadView& view);
    virtual void x_SaveSettings(CRegistryWriteView view) const;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___SELECT_ASSEMBLY_DIALOG___HPP
