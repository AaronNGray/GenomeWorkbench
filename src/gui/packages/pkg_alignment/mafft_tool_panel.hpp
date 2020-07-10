#ifndef PKG_ALIGNMENT___MAFFT_TOOL_PANEL__HPP
#define PKG_ALIGNMENT___MAFFT_TOOL_PANEL__HPP

/*  $Id: mafft_tool_panel.hpp 38358 2017-04-27 21:49:22Z evgeniev $
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
#include <gui/packages/pkg_alignment/mafft_tool_params.hpp>
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
#define SYMBOL_CMAFFTTOOLPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMAFFTTOOLPANEL_TITLE _("MAFFT Tool Panel")
#define SYMBOL_CMAFFTTOOLPANEL_IDNAME ID_CMAFFTPANEL
#define SYMBOL_CMAFFTTOOLPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CMAFFTTOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
*
* @{
*/

/*!
 * CMafftToolPanel class declaration
 */

class CMafftToolPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CMafftToolPanel )
    DECLARE_EVENT_TABLE()

public:
    CMafftToolPanel();
    CMafftToolPanel(wxWindow* parent, wxWindowID id = SYMBOL_CMAFFTTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMAFFTTOOLPANEL_POSITION, const wxSize& size = SYMBOL_CMAFFTTOOLPANEL_SIZE, long style = SYMBOL_CMAFFTTOOLPANEL_STYLE, bool visible = true);

    bool Create(wxWindow* parent, wxWindowID id = SYMBOL_CMAFFTTOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMAFFTTOOLPANEL_POSITION, const wxSize& size = SYMBOL_CMAFFTTOOLPANEL_SIZE, long style = SYMBOL_CMAFFTTOOLPANEL_STYLE, bool visible = true);

    ~CMafftToolPanel();

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

////@begin CMafftToolPanel event handler declarations

    void OnMafftPathClick( wxCommandEvent& event );

////@end CMafftToolPanel event handler declarations

////@begin CMafftToolPanel member function declarations

    CMafftToolParams& GetData() { return m_data; }
    const CMafftToolParams& GetData() const { return m_data; }
    void SetData(const CMafftToolParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CMafftToolPanel member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    static bool ShowToolTips();

////@begin CMafftToolPanel member variables
    CObjectListWidgetSel* m_LocationSel;
    CMafftToolParams m_data;
    enum {
        ID_CMAFFTPANEL = 10050,
        ID_PANEL = 10051,
        ID_CHECKBOX1 = 10001,
        ID_CHECKBOX = 10000,
        ID_TEXTCTRL11 = 10052,
        ID_TEXTCTRL12 = 10055,
        ID_BITMAPBUTTON1 = 10056
    };
////@end CMafftToolPanel member variables
};

/** @}*/

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___MAFFT_TOOL_PANEL__HPP
