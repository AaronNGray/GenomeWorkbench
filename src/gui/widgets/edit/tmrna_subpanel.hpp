/*  $Id: tmrna_subpanel.hpp 25028 2012-01-12 17:26:13Z katargir $
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
#ifndef _TMRNASUBPANEL_H_
#define _TMRNASUBPANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>

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
#define SYMBOL_CTMRNASUBPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTMRNASUBPANEL_TITLE _("tmRNA SubPanel")
#define SYMBOL_CTMRNASUBPANEL_IDNAME ID_CTMRNASUBPANEL
#define SYMBOL_CTMRNASUBPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTMRNASUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CtmRNASubPanel class declaration
 */

class CtmRNASubPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CtmRNASubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CtmRNASubPanel();
    CtmRNASubPanel( wxWindow* parent, CRef<objects::CRNA_gen> gen, wxWindowID id = SYMBOL_CTMRNASUBPANEL_IDNAME, const wxString& caption = SYMBOL_CTMRNASUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CTMRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CTMRNASUBPANEL_SIZE, long style = SYMBOL_CTMRNASUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTMRNASUBPANEL_IDNAME, const wxString& caption = SYMBOL_CTMRNASUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CTMRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CTMRNASUBPANEL_SIZE, long style = SYMBOL_CTMRNASUBPANEL_STYLE );

    /// Destructor
    ~CtmRNASubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CtmRNASubPanel event handler declarations

////@end CtmRNASubPanel event handler declarations

////@begin CtmRNASubPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CtmRNASubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CtmRNASubPanel member variables
    wxTextCtrl* m_TagPeptideStartCtrl;
    wxTextCtrl* m_TagPeptideStopCtrl;
    wxTextCtrl* m_ProductCtrl;
    /// Control identifiers
    enum {
        ID_CTMRNASUBPANEL = 10054,
        ID_TEXTCTRL14 = 10055,
        ID_TEXTCTRL15 = 10056,
        ID_TEXTCTRL16 = 10057
    };
////@end CtmRNASubPanel member variables
private:
    CRef<objects::CRNA_gen> m_Gen;

};

END_NCBI_SCOPE

#endif
    // _TMRNASUBPANEL_H_
