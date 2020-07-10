#ifndef PKG_SEQUENCE___CLEANUP_PANEL__HPP
#define PKG_SEQUENCE___CLEANUP_PANEL__HPP

/*  $Id: cleanup_panel.hpp 37331 2016-12-23 19:52:17Z katargir $
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
#include <gui/packages/pkg_sequence/cleanup_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCLEANUPPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCLEANUPPANEL_TITLE _("Cleanup Panel")
#define SYMBOL_CCLEANUPPANEL_IDNAME ID_CCLEANUPPANEL
#define SYMBOL_CCLEANUPPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCLEANUPPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

class CObjectListWidget;

/*!
 * CCleanupPanel class declaration
 */

class CCleanupPanel: public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CCleanupPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCleanupPanel();
    CCleanupPanel( wxWindow* parent, wxWindowID id = SYMBOL_CCLEANUPPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCLEANUPPANEL_POSITION, const wxSize& size = SYMBOL_CCLEANUPPANEL_SIZE, long style = SYMBOL_CCLEANUPPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCLEANUPPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCLEANUPPANEL_POSITION, const wxSize& size = SYMBOL_CCLEANUPPANEL_SIZE, long style = SYMBOL_CCLEANUPPANEL_STYLE );

    /// Destructor
    ~CCleanupPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    ////@begin CCleanupPanel event handler declarations

////@end CCleanupPanel event handler declarations

////@begin CCleanupPanel member function declarations

    /// Data access
    CCleanupParams& GetData() { return m_data; }
    const CCleanupParams& GetData() const { return m_data; }
    void SetData(const CCleanupParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCleanupPanel member function declarations

    void SetObjects(TConstScopedObjects* objects) { m_InputObjects = objects; }

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCleanupPanel member variables
    CObjectListWidget* m_ObjectList;
    /// The data edited by this window
    CCleanupParams m_data;
    /// Control identifiers
    enum {
        ID_CCLEANUPPANEL = 10000,
        ID_LISTCTRL1 = 10001,
        ID_RADIOBOX1 = 10002
    };
////@end CCleanupPanel member variables

    TConstScopedObjects*    m_InputObjects;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___CLEANUP_PANEL__HPP
