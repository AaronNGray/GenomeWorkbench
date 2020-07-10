/*  $Id: phylo_tree_demo_dlg.hpp 25623 2012-04-13 17:17:09Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */
#ifndef _PHYLO_TREE_DEMO_DLG_H_
#define _PHYLO_TREE_DEMO_DLG_H_


/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

#include <wx/dialog.h>
#include <wx/bitmap.h>

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

BEGIN_NCBI_SCOPE

////@begin forward declarations
class CPhyloTreeWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CHITMATRIXDEMODLG 10000
#define ID_PANEL1 10001
#define SYMBOL_CPHYLOTREEDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CPHYLOTREEDEMODLG_TITLE _("CPhyloTreeDemoDlg")
#define SYMBOL_CPHYLOTREEDEMODLG_IDNAME ID_CHITMATRIXDEMODLG
#define SYMBOL_CPHYLOTREEDEMODLG_SIZE wxSize(600, 500)
#define SYMBOL_CPHYLOTREEDEMODLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPhyloTreeDemoDlg class declaration
 */

class CPhyloTreeDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CPhyloTreeDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPhyloTreeDemoDlg();
    CPhyloTreeDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CPHYLOTREEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CPHYLOTREEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CPHYLOTREEDEMODLG_POSITION, const wxSize& size = SYMBOL_CPHYLOTREEDEMODLG_SIZE, long style = SYMBOL_CPHYLOTREEDEMODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPHYLOTREEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CPHYLOTREEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CPHYLOTREEDEMODLG_POSITION, const wxSize& size = SYMBOL_CPHYLOTREEDEMODLG_SIZE, long style = SYMBOL_CPHYLOTREEDEMODLG_STYLE );

    /// Destructor
    ~CPhyloTreeDemoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CPhyloTreeDemoDlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_CHITMATRIXDEMODLG
    void OnCloseWindow( wxCloseEvent& event );

////@end CPhyloTreeDemoDlg event handler declarations

////@begin CPhyloTreeDemoDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPhyloTreeDemoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CPhyloTreeDemoDlg member variables
    CPhyloTreeWidget* m_Widget;
////@end CPhyloTreeDemoDlg member variables
};

END_NCBI_SCOPE

#endif
    // _PHYLO_TREE_DEMO_DLG_H_
