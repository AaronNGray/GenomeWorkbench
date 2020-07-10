#ifndef GUI_WIDGETS___LOADERS___WIG_PARAMS_PANEL__HPP
#define GUI_WIDGETS___LOADERS___WIG_PARAMS_PANEL__HPP

/*  $Id: wig_params_panel.hpp 28586 2013-08-02 19:32:29Z katargir $
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
#include <gui/gui_export.h>

/*!
 * Includes
 */

#include <wx/panel.h>

#include <gui/widgets/loaders/wig_load_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWIGPARAMSPANEL 10052
#define ID_SPINCTRL1 10054
#define ID_PANEL10 10016
#define SYMBOL_CWIGPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CWIGPARAMSPANEL_TITLE _("CWigParamsPanel")
#define SYMBOL_CWIGPARAMSPANEL_IDNAME ID_CWIGPARAMSPANEL
#define SYMBOL_CWIGPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CWIGPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CAssemblySelPanel;

/*!
 * CWigParamsPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CWigParamsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CWigParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWigParamsPanel();
    CWigParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CWIGPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CWIGPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CWIGPARAMSPANEL_SIZE, long style = SYMBOL_CWIGPARAMSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIGPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CWIGPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CWIGPARAMSPANEL_SIZE, long style = SYMBOL_CWIGPARAMSPANEL_STYLE );

    /// Destructor
    ~CWigParamsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

////@begin CWigParamsPanel event handler declarations

////@end CWigParamsPanel event handler declarations

////@begin CWigParamsPanel member function declarations

    /// Data access
    CWigLoadParams& GetData() { return m_data; }
    const CWigLoadParams& GetData() const { return m_data; }
    void SetData(const CWigLoadParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWigParamsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWigParamsPanel member variables
    CAssemblySelPanel* m_AssemblyPanel;
    /// The data edited by this window
    CWigLoadParams m_data;
////@end CWigParamsPanel member variables
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___WIG_PARAMS_PANEL__HPP
