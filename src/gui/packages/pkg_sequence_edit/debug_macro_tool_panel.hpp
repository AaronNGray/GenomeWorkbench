#ifndef PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_PANEL__HPP
#define PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_PANEL__HPP

/*  $Id: debug_macro_tool_panel.hpp 37345 2016-12-27 18:24:36Z katargir $
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


#include <corelib/ncbiobj.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_sequence_edit/debug_macro_tool_params.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>

/*!
 * Includes
 */

/*!
 * Forward declarations
 */

////@begin forward declarations
class CObjectListWidgetSel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CDEBUGMACROTOOLPANEL_STYLE wxNO_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_CDEBUGMACROTOOLPANEL_TITLE _("Debug Macro Tool")
#define SYMBOL_CDEBUGMACROTOOLPANEL_IDNAME ID_CDEBUGMACROTOOL
#define SYMBOL_CDEBUGMACROTOOLPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CDEBUGMACROTOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CDebugMacroToolPanel class declaration
 */

class CDebugMacroToolPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CDebugMacroToolPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDebugMacroToolPanel();
    CDebugMacroToolPanel( wxWindow* parent, wxWindowID id = SYMBOL_CDEBUGMACROTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CDEBUGMACROTOOLPANEL_POSITION, const wxSize& size = SYMBOL_CDEBUGMACROTOOLPANEL_SIZE, long style = SYMBOL_CDEBUGMACROTOOLPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDEBUGMACROTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CDEBUGMACROTOOLPANEL_POSITION, const wxSize& size = SYMBOL_CDEBUGMACROTOOLPANEL_SIZE, long style = SYMBOL_CDEBUGMACROTOOLPANEL_STYLE );

    /// Destructor
    ~CDebugMacroToolPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

////@begin CDebugMacroToolPanel event handler declarations

////@end CDebugMacroToolPanel event handler declarations

////@begin CDebugMacroToolPanel member function declarations

    /// Data access
    CDebugMacroToolParams& GetData() { return m_data; }
    const CDebugMacroToolParams& GetData() const { return m_data; }
    void SetData(const CDebugMacroToolParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDebugMacroToolPanel member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDebugMacroToolPanel member variables
    CObjectListWidgetSel* m_ObjectSel;
    /// The data edited by this window
    CDebugMacroToolParams m_data;
    /// Control identifiers
    enum {
        ID_CDEBUGMACROTOOL = 10330,
        ID_PANEL = 10331,
        ID_TEXTCTRL9 = 10332
    };
////@end CDebugMacroToolPanel member variables
};

END_NCBI_SCOPE

#endif  // PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_PANEL__HPP
