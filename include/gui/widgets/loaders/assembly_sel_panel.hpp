#ifndef GUI_WIDGETS___LOADERS___ASSEMBLY_SEL_PANEL__HPP
#define GUI_WIDGETS___LOADERS___ASSEMBLY_SEL_PANEL__HPP

/*  $Id: assembly_sel_panel.hpp 44756 2020-03-05 18:44:36Z shkeda $
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

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/reg_settings.hpp>

#include "wx/panel.h"
#include <wx/string.h>

/*!
 * Includes
 */

////@begin includes
#include "map_assembly_params.hpp"
#include "wx/valgen.h"
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
#define SYMBOL_CASSEMBLYSELPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CASSEMBLYSELPANEL_TITLE _("Assembly Selection Panel")
#define SYMBOL_CASSEMBLYSELPANEL_IDNAME ID_CASSEMBLYSELPANEL
#define SYMBOL_CASSEMBLYSELPANEL_SIZE wxDefaultSize
#define SYMBOL_CASSEMBLYSELPANEL_POSITION wxDefaultPosition
////@end control identifiers

class wxTextCtrl;
class wxStaticText;
class wxCheckBox;
class wxTextCompleter;

BEGIN_NCBI_SCOPE

DECLARE_EXPORTED_EVENT_TYPE(NCBI_GUIWIDGETS_LOADERS_EXPORT, wxEVT_ASSEMBLY_CHANGED_EVENT, -1)

/*!
 * CAssemblySelPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CAssemblySelPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CAssemblySelPanel )
    DECLARE_EVENT_TABLE()

public:
    CAssemblySelPanel();
    CAssemblySelPanel( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYSELPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYSELPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYSELPANEL_SIZE, long style = SYMBOL_CASSEMBLYSELPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYSELPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYSELPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYSELPANEL_SIZE, long style = SYMBOL_CASSEMBLYSELPANEL_STYLE );

    ~CAssemblySelPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

////@begin CAssemblySelPanel event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX6
    void OnUseMappingClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnSelectAssemBtnClick( wxCommandEvent& event );

////@end CAssemblySelPanel event handler declarations

////@begin CAssemblySelPanel member function declarations

    /// Data access
    CMapAssemblyParams& GetData() { return m_data; }
    const CMapAssemblyParams& GetData() const { return m_data; }
    void SetData(const CMapAssemblyParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAssemblySelPanel member function declarations

    static bool ShowToolTips();

////@begin CAssemblySelPanel member variables
    wxCheckBox* m_UseMappingCtrl;
    wxStaticText* m_AssemblyName;
    wxStaticText* m_AssemblyAccession;
    wxStaticText* m_AssemblyDescription;
    /// The data edited by this window
    CMapAssemblyParams m_data;
    /// Control identifiers
    enum {
        ID_CASSEMBLYSELPANEL = 10116,
        ID_CHECKBOX6 = 10038,
        ID_BUTTON1 = 10118
    };
////@end CAssemblySelPanel member variables

protected:    
    void x_UpdateControls();
    void x_UpdateAssembly();
    void x_SetTextCompleter(wxTextCompleter* textCompleter);
};

END_NCBI_SCOPE

#endif  /// GUI_WIDGETS___LOADERS___ASSEMBLY_SEL_PANEL__HPP
