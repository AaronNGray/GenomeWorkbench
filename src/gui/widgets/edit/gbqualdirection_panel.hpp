/*  $Id: gbqualdirection_panel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
#ifndef _GBQUALDIRECTION_PANEL_H_
#define _GBQUALDIRECTION_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/formattedqual_panel.hpp>

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
#define SYMBOL_CGBQUALDIRECTIONPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CGBQUALDIRECTIONPANEL_TITLE _("GBQualDirectionPanel")
#define SYMBOL_CGBQUALDIRECTIONPANEL_IDNAME ID_CGBQUALDIRECTIONPANEL
#define SYMBOL_CGBQUALDIRECTIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGBQUALDIRECTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGBQualDirectionPanel class declaration
 */

class CGBQualDirectionPanel: public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CGBQualDirectionPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGBQualDirectionPanel();
    CGBQualDirectionPanel( wxWindow* parent, wxWindowID id = SYMBOL_CGBQUALDIRECTIONPANEL_IDNAME, const wxString& caption = SYMBOL_CGBQUALDIRECTIONPANEL_TITLE, const wxPoint& pos = SYMBOL_CGBQUALDIRECTIONPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALDIRECTIONPANEL_SIZE, long style = SYMBOL_CGBQUALDIRECTIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGBQUALDIRECTIONPANEL_IDNAME, const wxString& caption = SYMBOL_CGBQUALDIRECTIONPANEL_TITLE, const wxPoint& pos = SYMBOL_CGBQUALDIRECTIONPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALDIRECTIONPANEL_SIZE, long style = SYMBOL_CGBQUALDIRECTIONPANEL_STYLE );

    /// Destructor
    ~CGBQualDirectionPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue(string val);
    virtual string GetValue();

////@begin CGBQualDirectionPanel event handler declarations

////@end CGBQualDirectionPanel event handler declarations

////@begin CGBQualDirectionPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGBQualDirectionPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGBQualDirectionPanel member variables
    wxChoice* m_DirectionCtrl;
    /// Control identifiers
    enum {
        ID_CGBQUALDIRECTIONPANEL = 10088,
        ID_CHOICE4 = 10089
    };
////@end CGBQualDirectionPanel member variables
};

END_NCBI_SCOPE

#endif
    // _GBQUALDIRECTION_PANEL_H_
