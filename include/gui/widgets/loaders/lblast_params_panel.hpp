#ifndef PKG_SEQUENCE___LBLAST_PARAMS_PANEL__HPP
#define PKG_SEQUENCE___LBLAST_PARAMS_PANEL__HPP

/*  $Id: lblast_params_panel.hpp 33831 2015-09-18 15:20:46Z katargir $
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
#include <wx/valgen.h>
#include <wx/timer.h>

#include <gui/widgets/loaders/lblast_load_params.hpp>

class wxTextCtrl;
class wxRadioBox;
class wxCheckBox;

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CLBLASTPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CLBLASTPARAMSPANEL_TITLE _("LBLAST Params Panel")
#define SYMBOL_CLBLASTPARAMSPANEL_IDNAME ID_CLBLASTPARAMSPANEL
#define SYMBOL_CLBLASTPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CLBLASTPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CLBLASTParamsPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CLBLASTParamsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CLBLASTParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLBLASTParamsPanel();
    CLBLASTParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CLBLASTPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLBLASTPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CLBLASTPARAMSPANEL_SIZE, long style = SYMBOL_CLBLASTPARAMSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLBLASTPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLBLASTPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CLBLASTPARAMSPANEL_SIZE, long style = SYMBOL_CLBLASTPARAMSPANEL_STYLE );

    /// Destructor
    ~CLBLASTParamsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

////@begin CLBLASTParamsPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_RADIOBOX1
    void OnSeqTypeSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL2
    void OnDBTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
    void OnSelectDir( wxCommandEvent& event );

////@end CLBLASTParamsPanel event handler declarations

    void OnTimer(wxTimerEvent& event);

////@begin CLBLASTParamsPanel member function declarations

    /// Data access
    CLBLASTLoadParams& GetData() { return m_data; }
    const CLBLASTLoadParams& GetData() const { return m_data; }
    void SetData(const CLBLASTLoadParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLBLASTParamsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    static bool ValidateLocalDatabase(const wxString& db, bool proteins);
    static bool SelectDatabase(wxWindow* parent, wxString& db, bool proteins);

////@begin CLBLASTParamsPanel member variables
    wxRadioBox* m_SeqTypeCtrl;
    wxTextCtrl* m_DBCtrl;
    wxBoxSizer* m_SeqCountSizer;
    wxCheckBox* m_CreateProjectItemsCtrl;
    /// The data edited by this window
    CLBLASTLoadParams m_data;
    /// Control identifiers
    enum {
        ID_CLBLASTPARAMSPANEL = 10011,
        ID_RADIOBOX1 = 10018,
        ID_TEXTCTRL2 = 10015,
        ID_BITMAPBUTTON = 10020,
        ID_CHECKBOX7 = 10019
    };
////@end CLBLASTParamsPanel member variables

private:
    void    x_UpdateSeqCount();
    wxTimer m_Timer;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___LBLAST_PARAMS_PANEL__HPP`
