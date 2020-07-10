#ifndef PKG_ALIGNMENT___MUSCLE_TOOL_PANEL__HPP
#define PKG_ALIGNMENT___MUSCLE_TOOL_PANEL__HPP

/*  $Id: muscle_tool_panel.hpp 38317 2017-04-25 18:47:02Z evgeniev $
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

#include <corelib/ncbiobj.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/muscle_tool_params.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>

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
#define SYMBOL_CMUSCLETOOLPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMUSCLETOOLPANEL_TITLE _("MUSCLE Tool Panel")
#define SYMBOL_CMUSCLETOOLPANEL_IDNAME ID_CMUSCLEPANEL
#define SYMBOL_CMUSCLETOOLPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CMUSCLETOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

/*!
 * CMuscleToolPanel class declaration
 */

class CMuscleToolPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CMuscleToolPanel )
    DECLARE_EVENT_TABLE()

public:
    CMuscleToolPanel();
    CMuscleToolPanel( wxWindow* parent, wxWindowID id = SYMBOL_CMUSCLETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMUSCLETOOLPANEL_POSITION, const wxSize& size = SYMBOL_CMUSCLETOOLPANEL_SIZE, long style = SYMBOL_CMUSCLETOOLPANEL_STYLE, bool visible = true );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMUSCLETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMUSCLETOOLPANEL_POSITION, const wxSize& size = SYMBOL_CMUSCLETOOLPANEL_SIZE, long style = SYMBOL_CMUSCLETOOLPANEL_STYLE, bool visible = true );

    ~CMuscleToolPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

////@begin CMuscleToolPanel event handler declarations

    void OnMusclePathClick( wxCommandEvent& event );

////@end CMuscleToolPanel event handler declarations

////@begin CMuscleToolPanel member function declarations

    CMuscleToolParams& GetData() { return m_data; }
    const CMuscleToolParams& GetData() const { return m_data; }
    void SetData(const CMuscleToolParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CMuscleToolPanel member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    static bool ShowToolTips();

////@begin CMuscleToolPanel member variables
    CObjectListWidgetSel* m_LocationSel;
    CMuscleToolParams m_data;
    enum {
        ID_CMUSCLEPANEL = 10050,
        ID_PANEL = 10051,
        ID_CHOICE2 = 10054,
        ID_CHECKBOX12 = 10053,
        ID_TEXTCTRL11 = 10052,
        ID_TEXTCTRL12 = 10055,
        ID_BITMAPBUTTON1 = 10056
    };
////@end CMuscleToolPanel member variables

    TConstScopedObjects*    m_InputObjects;
};

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___MUSCLE_TOOL_PANEL__HPP
