#ifndef GUI_WIDGETS___LOADERS___BED_PARAMS_PANEL__HPP
#define GUI_WIDGETS___LOADERS___BED_PARAMS_PANEL__HPP

/*  $Id: bed_params_panel.hpp 28583 2013-08-02 18:59:27Z katargir $
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

#include <gui/widgets/loaders/bed_load_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CBEDPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBEDPARAMSPANEL_TITLE _("CBedParamsPanel")
#define SYMBOL_CBEDPARAMSPANEL_IDNAME ID_CBEDPARAMSPANEL
#define SYMBOL_CBEDPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBEDPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CAssemblySelPanel;

/*!
 * CBedParamsPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CBedParamsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CBedParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBedParamsPanel();
    CBedParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CBEDPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBEDPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CBEDPARAMSPANEL_SIZE, long style = SYMBOL_CBEDPARAMSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBEDPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBEDPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CBEDPARAMSPANEL_SIZE, long style = SYMBOL_CBEDPARAMSPANEL_STYLE );

    /// Destructor
    ~CBedParamsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

////@begin CBedParamsPanel event handler declarations

////@end CBedParamsPanel event handler declarations

////@begin CBedParamsPanel member function declarations

    /// Data access
    CBedLoadParams& GetData() { return m_data; }
    const CBedLoadParams& GetData() const { return m_data; }
    void SetData(const CBedLoadParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBedParamsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBedParamsPanel member variables
    CAssemblySelPanel* m_AssemblyPanel;
    /// The data edited by this window
    CBedLoadParams m_data;
    /// Control identifiers
    enum {
        ID_CBEDPARAMSPANEL = 10055,
        ID_SPINCTRL2 = 10057,
        ID_PANEL8 = 10051
    };
////@end CBedParamsPanel member variables
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___BED_PARAMS_PANEL__HPP
