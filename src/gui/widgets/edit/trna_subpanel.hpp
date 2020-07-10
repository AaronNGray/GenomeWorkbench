/*  $Id: trna_subpanel.hpp 35570 2016-05-25 14:38:34Z asztalos $
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
#ifndef _TRNA_SUBPANEL_H_
#define _TRNA_SUBPANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/treebook.h"
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
class wxTreebook;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CLocationPanel;
class CGBQualPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTRNASUBPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTRNASUBPANEL_TITLE _("tRNA SubPanel")
#define SYMBOL_CTRNASUBPANEL_IDNAME ID_CTRNASUBPANEL
#define SYMBOL_CTRNASUBPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTRNASUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CtRNASubPanel class declaration
 */

class CtRNASubPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CtRNASubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CtRNASubPanel();
    CtRNASubPanel(wxWindow* parent, CRef<objects::CTrna_ext> trna, CConstRef<objects::CSeq_feat> feat, objects::CScope& scope, 
                   wxWindowID id = SYMBOL_CTRNASUBPANEL_IDNAME, const wxString& caption = SYMBOL_CTRNASUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CTRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CTRNASUBPANEL_SIZE, long style = SYMBOL_CTRNASUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTRNASUBPANEL_IDNAME, const wxString& caption = SYMBOL_CTRNASUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CTRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CTRNASUBPANEL_SIZE, long style = SYMBOL_CTRNASUBPANEL_STYLE );

    /// Destructor
    ~CtRNASubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void SetGBQualPanel(CGBQualPanel* gbqual_panel);

////@begin CtRNASubPanel event handler declarations

////@end CtRNASubPanel event handler declarations

////@begin CtRNASubPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CtRNASubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CtRNASubPanel member variables
    wxTreebook* m_tRNATree;
    /// Control identifiers
    enum {
        ID_CTRNASUBPANEL = 10043,
        ID_TREEBOOK = 10111
    };
////@end CtRNASubPanel member variables

private:
    CRef<objects::CTrna_ext> m_trna;
    CConstRef<objects::CSeq_feat> m_Feat;
    CRef<objects::CScope> m_Scope;
    CLocationPanel* m_AnticodonLocPanel;
    CGBQualPanel* m_GbQualPanel;

};

END_NCBI_SCOPE

#endif
    // _TRNA_SUBPANEL_H_
