#ifndef PKG_ALIGNMENT___CLUSTALW_TOOL_PANEL__HPP
#define PKG_ALIGNMENT___CLUSTALW_TOOL_PANEL__HPP

/*  $Id: clustalw_tool_panel.hpp 38391 2017-05-03 13:45:05Z evgeniev $
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
#include <gui/packages/pkg_alignment/clustalw_tool_params.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCLUSTALWTOOLPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCLUSTALWTOOLPANEL_TITLE _("ClustalW Tool Panel")
#define SYMBOL_CCLUSTALWTOOLPANEL_IDNAME ID_CCLUSTALWPANEL
#define SYMBOL_CCLUSTALWTOOLPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCLUSTALWTOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

 /*!
 * CClustalwToolPanel class declaration
 */

class CClustalwToolPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CClustalwToolPanel )
    DECLARE_EVENT_TABLE()

public:
    CClustalwToolPanel();
    CClustalwToolPanel( wxWindow* parent, wxWindowID id = SYMBOL_CCLUSTALWTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCLUSTALWTOOLPANEL_POSITION, const wxSize& size = SYMBOL_CCLUSTALWTOOLPANEL_SIZE, long style = SYMBOL_CCLUSTALWTOOLPANEL_STYLE, bool visible = true );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCLUSTALWTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCLUSTALWTOOLPANEL_POSITION, const wxSize& size = SYMBOL_CCLUSTALWTOOLPANEL_SIZE, long style = SYMBOL_CCLUSTALWTOOLPANEL_STYLE, bool visible = true );

    ~CClustalwToolPanel();

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

////@begin CClustalwToolPanel event handler declarations

    void OnClustalWPathClick( wxCommandEvent& event );

////@end CClustalwToolPanel event handler declarations

////@begin CClustalwToolPanel member function declarations

    CClustalwToolParams& GetData() { return m_data; }
    const CClustalwToolParams& GetData() const { return m_data; }
    void SetData(const CClustalwToolParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CClustalwToolPanel member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    static bool ShowToolTips();

////@begin CClustalwToolPanel member variables
    CObjectListWidgetSel* m_LocationSel;
    CClustalwToolParams m_data;
    enum {
        ID_CCLUSTALWPANEL = 10050,
        ID_PANEL = 10051,
        ID_CHECKBOX12 = 10053,
        ID_TEXTCTRL11 = 10052,
        ID_TEXTCTRL12 = 10055,
        ID_BITMAPBUTTON1 = 10056
    };
////@end CClustalwToolPanel member variables
};

/** @}*/

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___CLUSTALW_TOOL_PANEL__HPP
