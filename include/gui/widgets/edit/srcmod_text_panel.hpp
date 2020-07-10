/*  $Id: srcmod_text_panel.hpp 29060 2013-10-01 19:21:17Z bollin $
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
#ifndef _SRCMOD_TEXT_PANEL_H_
#define _SRCMOD_TEXT_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/srcmod_edit_panel.hpp>
#include <wx/textctrl.h>

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
#define ID_CSRCMODTEXTPANEL 10018
#define ID_TEXTCTRL5 10019
#define SYMBOL_CSRCMODTEXTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSRCMODTEXTPANEL_TITLE _("SrcModTextPanel")
#define SYMBOL_CSRCMODTEXTPANEL_IDNAME ID_CSRCMODTEXTPANEL
#define SYMBOL_CSRCMODTEXTPANEL_SIZE wxDefaultSize
#define SYMBOL_CSRCMODTEXTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSrcModTextPanel class declaration
 */

class CSrcModTextPanel: public CSrcModEditPanel
{    
    DECLARE_DYNAMIC_CLASS( CSrcModTextPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSrcModTextPanel();
    CSrcModTextPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSRCMODTEXTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODTEXTPANEL_SIZE, long style = SYMBOL_CSRCMODTEXTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSRCMODTEXTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODTEXTPANEL_SIZE, long style = SYMBOL_CSRCMODTEXTPANEL_STYLE );

    /// Destructor
    ~CSrcModTextPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSrcModTextPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL5
    void OnTextctrl5TextUpdated( wxCommandEvent& event );

////@end CSrcModTextPanel event handler declarations

////@begin CSrcModTextPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSrcModTextPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSrcModTextPanel member variables
    wxTextCtrl* m_Text;
////@end CSrcModTextPanel member variables
    virtual string GetValue();
    virtual void SetValue(const string& val);
    virtual bool IsWellFormatted(const string& val);
};

END_NCBI_SCOPE

#endif
    // _SRCMOD_TEXT_PANEL_H_
