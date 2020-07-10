#ifndef GUI_WIDGETS_EDIT___MRNA_SUBPANEL__HPP
#define GUI_WIDGETS_EDIT___MRNA_SUBPANEL__HPP

/*  $Id: mrna_subpanel.hpp 25482 2012-03-27 15:00:20Z kuznets $
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

/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
////@end includes

#include <wx/panel.h>

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
#define SYMBOL_CMRNASUBPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMRNASUBPANEL_TITLE wxT("mRNA SubPanel")
#define SYMBOL_CMRNASUBPANEL_IDNAME ID_CMRNASUBPANEL
#define SYMBOL_CMRNASUBPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CMRNASUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CmRNASubPanel class declaration
 */

class CmRNASubPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CmRNASubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CmRNASubPanel();
    CmRNASubPanel( wxWindow* parent, wxWindowID id = SYMBOL_CMRNASUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CMRNASUBPANEL_SIZE, long style = SYMBOL_CMRNASUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMRNASUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CMRNASUBPANEL_SIZE, long style = SYMBOL_CMRNASUBPANEL_STYLE );

    /// Destructor
    ~CmRNASubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CmRNASubPanel event handler declarations

////@end CmRNASubPanel event handler declarations

////@begin CmRNASubPanel member function declarations

    wxString GetRnaName() const { return m_Name ; }
    void SetRnaName(wxString value) { m_Name = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CmRNASubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CmRNASubPanel member variables
private:
    wxString m_Name;
    /// Control identifiers
    enum {
        ID_CMRNASUBPANEL = 10022,
        ID_TEXTCTRL8 = 10023
    };
////@end CmRNASubPanel member variables
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___MRNA_SUBPANEL__HPP
