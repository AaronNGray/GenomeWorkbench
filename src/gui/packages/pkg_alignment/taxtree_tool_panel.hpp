#ifndef PKG_ALIGNMENT___TAXTREE_TOOL_PANEL__HPP
#define PKG_ALIGNMENT___TAXTREE_TOOL_PANEL__HPP

/*  $Id: taxtree_tool_panel.hpp 32655 2015-04-07 18:11:22Z evgeniev $
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
#include <gui/packages/pkg_alignment/taxtree_tool_params.hpp>
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
#define SYMBOL_CTAXTREETOOLPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CTAXTREETOOLPANEL_TITLE _("Common TaxTree Tool Panel")
#define SYMBOL_CTAXTREETOOLPANEL_IDNAME ID_CTAXTREETOOLPANEL
#define SYMBOL_CTAXTREETOOLPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTAXTREETOOLPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

/*!
 * CTaxTreeToolPanel class declaration
 */

class CTaxTreeToolPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CTaxTreeToolPanel )
    DECLARE_EVENT_TABLE()

public:
    CTaxTreeToolPanel();
    CTaxTreeToolPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTAXTREETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CTAXTREETOOLPANEL_POSITION, const wxSize& size = SYMBOL_CTAXTREETOOLPANEL_SIZE, long style = SYMBOL_CTAXTREETOOLPANEL_STYLE, bool visibale = true );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTAXTREETOOLPANEL_IDNAME, const wxPoint& pos = SYMBOL_CTAXTREETOOLPANEL_POSITION, const wxSize& size = SYMBOL_CTAXTREETOOLPANEL_SIZE, long style = SYMBOL_CTAXTREETOOLPANEL_STYLE, bool visibale = true );

    ~CTaxTreeToolPanel();

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

////@begin CTaxTreeToolPanel event handler declarations

////@end CTaxTreeToolPanel event handler declarations

////@begin CTaxTreeToolPanel member function declarations

    CTaxTreeToolParams& GetData() { return m_data; }
    const CTaxTreeToolParams& GetData() const { return m_data; }
    void SetData(const CTaxTreeToolParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CTaxTreeToolPanel member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    static bool ShowToolTips();

////@begin CTaxTreeToolPanel member variables
    CObjectListWidgetSel* m_SeqIds;
    CTaxTreeToolParams m_data;
    enum {
        ID_CTAXTREETOOLPANEL = 10057,
        ID_PANEL3 = 10058
    };
////@end CTaxTreeToolPanel member variables

    TConstScopedObjects*    m_InputObjects;
};

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___TAXTREE_TOOL_PANEL__HPP
