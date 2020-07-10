#ifndef GUI_WIDGETS___LOADERS___PSL_PARAMS_PANEL__HPP
#define GUI_WIDGETS___LOADERS___PSL_PARAMS_PANEL__HPP

/*  $Id: psl_params_panel.hpp 44439 2019-12-18 19:51:52Z katargir $
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

#include <gui/widgets/loaders/psl_load_params.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAssemblySelPanel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPSLPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPSLPARAMSPANEL_TITLE _("CPslParamsPanel")
#define SYMBOL_CPSLPARAMSPANEL_IDNAME ID_CPSLPARAMSPANEL
#define SYMBOL_CPSLPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPSLPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CAssemblySelPanel;

/*!
 * CPslParamsPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CPslParamsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CPslParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPslParamsPanel();
    CPslParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CPSLPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPSLPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CPSLPARAMSPANEL_SIZE, long style = SYMBOL_CPSLPARAMSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPSLPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPSLPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CPSLPARAMSPANEL_SIZE, long style = SYMBOL_CPSLPARAMSPANEL_STYLE );

    /// Destructor
    ~CPslParamsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

////@begin CPslParamsPanel event handler declarations

////@end CPslParamsPanel event handler declarations

////@begin CPslParamsPanel member function declarations

    /// Data access
    CPslLoadParams& GetData() { return m_data; }
    const CPslLoadParams& GetData() const { return m_data; }
    void SetData(const CPslLoadParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPslParamsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CPslParamsPanel member variables
    CAssemblySelPanel* m_AssemblyPanel;
    /// The data edited by this window
    CPslLoadParams m_data;
    /// Control identifiers
    enum {
        ID_CPSLPARAMSPANEL = 10055,
        ID_PANEL8 = 10051
    };
////@end CPslParamsPanel member variables
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___PSL_PARAMS_PANEL__HPP
