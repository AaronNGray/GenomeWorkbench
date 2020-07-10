#ifndef PKG_SEQUENCE___MERGE_PANEL__HPP
#define PKG_SEQUENCE___MERGE_PANEL__HPP

/*  $Id: merge_panel.hpp 32655 2015-04-07 18:11:22Z evgeniev $
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

#include <corelib/ncbistd.hpp>

#include <gui/core/algo_tool_manager_base.hpp>

#include <wx/panel.h>

////@begin includes
#include "wx/valgen.h"
////@end includes


BEGIN_NCBI_SCOPE

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
#define SYMBOL_CMERGEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMERGEPANEL_TITLE _("Merge Panel")
#define SYMBOL_CMERGEPANEL_IDNAME ID_CMERGEPANEL
#define SYMBOL_CMERGEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CMERGEPANEL_POSITION wxDefaultPosition
////@end control identifiers


struct CMergeParams
{
    bool mf_StripAnnotDescs;
    bool mf_ConvertAccs;
    bool mf_ExetendedCleanup;

    CMergeParams() { Init(); }
    void Init() { mf_StripAnnotDescs = true; mf_ConvertAccs = true; mf_ExetendedCleanup = false; }
};

/*!
 * CMergePanel class declaration
 */

class CMergePanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CMergePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMergePanel();
    CMergePanel( wxWindow* parent, wxWindowID id = SYMBOL_CMERGEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMERGEPANEL_POSITION, const wxSize& size = SYMBOL_CMERGEPANEL_SIZE, long style = SYMBOL_CMERGEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMERGEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMERGEPANEL_POSITION, const wxSize& size = SYMBOL_CMERGEPANEL_SIZE, long style = SYMBOL_CMERGEPANEL_STYLE );

    /// Destructor
    ~CMergePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

////@begin CMergePanel event handler declarations

////@end CMergePanel event handler declarations

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath( const string& reg_path ) {}

    virtual void LoadSettings() {}
    virtual void SaveSettings() const {}
    /// @}

    void SetObjects( 
        map<string, TConstScopedObjects>& masters,
        map<string, TConstScopedObjects>& kids
    );

////@begin CMergePanel member function declarations

    /// Data access
    CMergeParams& GetData() { return m_data; }
    const CMergeParams& GetData() const { return m_data; }
    void SetData(const CMergeParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMergePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CMergePanel member variables
    CObjectListWidgetSel* m_MasterItems;
    CObjectListWidgetSel* m_ChildItems;
    /// The data edited by this window
    CMergeParams m_data;
    /// Control identifiers
    enum {
        ID_CMERGEPANEL = 10000,
        ID_MASTER_ITEMS = 10064,
        ID_CHILD_ITEMS = 10003,
        ID_STRIP_ANNOT_DESC = 10065,
        ID_CONVERT_ACC = 10066,
        ID_EXT_CLEANUP = 10067
    };
////@end CMergePanel member variables

protected:
    /// registry path to the settings
    string  m_RegPath;
};


END_NCBI_SCOPE

#endif // PKG_SEQUENCE___MERGE_PANEL__HPP
