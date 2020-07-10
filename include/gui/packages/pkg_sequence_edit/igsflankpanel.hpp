/*  $Id: igsflankpanel.hpp 26516 2012-10-01 13:12:25Z bollin $
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
#ifndef _IGSFLANKPANEL_H_
#define _IGSFLANKPANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>

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
#define ID_CIGSFLANKPANEL 10036
#define ID_RADIOBUTTON 10038
#define ID_CHOICE 10037
#define ID_RADIOBUTTON1 10039
#define ID_TEXTCTRL3 10040
#define ID_TEXTCTRL4 10041
#define ID_RADIOBOX1 10042
#define SYMBOL_CIGSFLANKPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CIGSFLANKPANEL_TITLE _("IGSFlankPanel")
#define SYMBOL_CIGSFLANKPANEL_IDNAME ID_CIGSFLANKPANEL
#define SYMBOL_CIGSFLANKPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CIGSFLANKPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CIGSFlankPanel class declaration
 */

class CIGSFlankPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CIGSFlankPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CIGSFlankPanel();
    CIGSFlankPanel( wxWindow* parent, wxWindowID id = SYMBOL_CIGSFLANKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CIGSFLANKPANEL_POSITION, const wxSize& size = SYMBOL_CIGSFLANKPANEL_SIZE, long style = SYMBOL_CIGSFLANKPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CIGSFLANKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CIGSFLANKPANEL_POSITION, const wxSize& size = SYMBOL_CIGSFLANKPANEL_SIZE, long style = SYMBOL_CIGSFLANKPANEL_STYLE );

    /// Destructor
    ~CIGSFlankPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CIGSFlankPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
    void OntrnabuttonSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON1
    void OnProteinSelected( wxCommandEvent& event );

////@end CIGSFlankPanel event handler declarations

////@begin CIGSFlankPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CIGSFlankPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CIGSFlankPanel member variables
    wxRadioButton* m_IstRNA;
    wxChoice* m_tRNA;
    wxRadioButton* m_IsProtein;
    wxTextCtrl* m_ProteinName;
    wxTextCtrl* m_GeneSymbol;
    wxRadioBox* m_IncludeFeat;
////@end CIGSFlankPanel member variables

    string GetIGSLabel();
    string GetFlankLabel();
    bool   MayContainFeat() { return m_IncludeFeat->GetSelection() == 1; };

};

END_NCBI_SCOPE

#endif
    // _IGSFLANKPANEL_H_
