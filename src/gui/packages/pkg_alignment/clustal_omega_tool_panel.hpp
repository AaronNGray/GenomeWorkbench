#ifndef PKG_ALIGNMENT____CLUSTAL_OMEGA_TOOL_PANEL__HPP
#define PKG_ALIGNMENT____CLUSTAL_OMEGA_TOOL_PANEL__HPP
/*  $Id: clustal_omega_tool_panel.hpp 38391 2017-05-03 13:45:05Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

#include <corelib/ncbiobj.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/clustal_omega_tool_params.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCLUSTALOMEGATOOLPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCLUSTALOMEGATOOLPANEL_TITLE _("Clustal Omega Tool Panel")
#define SYMBOL_CCLUSTALOMEGATOOLPANEL_IDNAME ID_CCLUSTALOMEGAPANEL
#define SYMBOL_CCLUSTALOMEGATOOLPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCLUSTALOMEGATOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
*
* @{
*/


/*!
 * CClustalOmegaToolPanel class declaration
 */

class CClustalOmegaToolPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CClustalOmegaToolPanel )
    DECLARE_EVENT_TABLE()

public:
    CClustalOmegaToolPanel();
    CClustalOmegaToolPanel(wxWindow* parent, wxWindowID id = SYMBOL_CCLUSTALOMEGATOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCLUSTALOMEGATOOLPANEL_POSITION, const wxSize& size = SYMBOL_CCLUSTALOMEGATOOLPANEL_SIZE, long style = SYMBOL_CCLUSTALOMEGATOOLPANEL_STYLE, bool visible = true );

    bool Create(wxWindow* parent, wxWindowID id = SYMBOL_CCLUSTALOMEGATOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCLUSTALOMEGATOOLPANEL_POSITION, const wxSize& size = SYMBOL_CCLUSTALOMEGATOOLPANEL_SIZE, long style = SYMBOL_CCLUSTALOMEGATOOLPANEL_STYLE, bool visible = true );

    ~CClustalOmegaToolPanel();

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

////@begin CClustalOmegaToolPanel event handler declarations

    void OnClustalOmegaPathClick( wxCommandEvent& event );

////@end CClustalOmegaToolPanel event handler declarations

////@begin CClustalOmegaToolPanel member function declarations

    CClustalOmegaToolParams& GetData() { return m_data; }
    const CClustalOmegaToolParams& GetData() const { return m_data; }
    void SetData(const CClustalOmegaToolParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CClustalOmegaToolPanel member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    static bool ShowToolTips();

////@begin CClustalOmegaToolPanel member variables
    CObjectListWidgetSel* m_LocationSel;
    CClustalOmegaToolParams m_data;
    enum {
        ID_CCLUSTALOMEGAPANEL = 10050,
        ID_PANEL = 10051,
        ID_CHECKBOX12 = 10053,
        ID_TEXTCTRL11 = 10052,
        ID_TEXTCTRL12 = 10055,
        ID_BITMAPBUTTON1 = 10056
    };
////@end CClustalOmegaToolPanel member variables
};

/** @}*/

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT____CLUSTAL_OMEGA_TOOL_PANEL__HPP
