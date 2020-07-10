/*  $Id: srcmod_checkbox_panel.hpp 29061 2013-10-01 19:31:45Z bollin $
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
 * Authors:  Colleen Bollin
 */
#ifndef _SRCMOD_CHECKBOX_PANEL_H_
#define _SRCMOD_CHECKBOX_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/srcmod_edit_panel.hpp>

#include <wx/checkbox.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSRCMODCHECKBOXPANEL 10020
#define ID_CHECKBOX 10021
#define SYMBOL_CSRCMODCHECKBOXPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSRCMODCHECKBOXPANEL_TITLE _("SrcModCheckboxPanel")
#define SYMBOL_CSRCMODCHECKBOXPANEL_IDNAME ID_CSRCMODCHECKBOXPANEL
#define SYMBOL_CSRCMODCHECKBOXPANEL_SIZE wxDefaultSize
#define SYMBOL_CSRCMODCHECKBOXPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSrcModCheckboxPanel class declaration
 */

class CSrcModCheckboxPanel: public CSrcModEditPanel
{    
    DECLARE_DYNAMIC_CLASS( CSrcModCheckboxPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSrcModCheckboxPanel();
    CSrcModCheckboxPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSRCMODCHECKBOXPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODCHECKBOXPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODCHECKBOXPANEL_SIZE, long style = SYMBOL_CSRCMODCHECKBOXPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSRCMODCHECKBOXPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODCHECKBOXPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODCHECKBOXPANEL_SIZE, long style = SYMBOL_CSRCMODCHECKBOXPANEL_STYLE );

    /// Destructor
    ~CSrcModCheckboxPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSrcModCheckboxPanel event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX
    void OnCheckboxClick( wxCommandEvent& event );

////@end CSrcModCheckboxPanel event handler declarations

////@begin CSrcModCheckboxPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSrcModCheckboxPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSrcModCheckboxPanel member variables
    wxCheckBox* m_Checkbox;
////@end CSrcModCheckboxPanel member variables
    virtual string GetValue();
    virtual void SetValue(const string& val);
    virtual bool IsWellFormatted(const string& val);
};

END_NCBI_SCOPE

#endif
    // _SRCMOD_CHECKBOX_PANEL_H_
