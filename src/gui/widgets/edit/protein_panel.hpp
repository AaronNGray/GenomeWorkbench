/*  $Id: protein_panel.hpp 25347 2012-03-01 18:22:54Z katargir $
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
#ifndef _PROTEIN_PANEL_H_
#define _PROTEIN_PANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/panel.h>
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
#define SYMBOL_CPROTEINPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROTEINPANEL_TITLE _("Protein Panel")
#define SYMBOL_CPROTEINPANEL_IDNAME ID_CCDSPROTEINPANEL
#define SYMBOL_CPROTEINPANEL_SIZE wxSize(266, 184)
#define SYMBOL_CPROTEINPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CProteinPanel class declaration
 */

class CProteinPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CProteinPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProteinPanel();
    CProteinPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CPROTEINPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROTEINPANEL_POSITION, const wxSize& size = SYMBOL_CPROTEINPANEL_SIZE, long style = SYMBOL_CPROTEINPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROTEINPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROTEINPANEL_POSITION, const wxSize& size = SYMBOL_CPROTEINPANEL_SIZE, long style = SYMBOL_CPROTEINPANEL_STYLE );

    /// Destructor
    ~CProteinPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CProteinPanel event handler declarations

////@end CProteinPanel event handler declarations

////@begin CProteinPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CProteinPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProteinPanel member variables
    wxChoice* m_ProcessedCtrl;
    /// Control identifiers
    enum {
        ID_CCDSPROTEINPANEL = 10031,
        ID_WINDOW1 = 10034,
        ID_TEXTCTRL12 = 10033,
        ID_CHOICE15 = 10112
    };
////@end CProteinPanel member variables
private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif
    // _PROTEIN_PANEL_H_
