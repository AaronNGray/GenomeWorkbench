#ifndef GUI_SEQ_TREE_DEMO___SEQ_TREE_DEMO_DLG_HPP
#define GUI_SEQ_TREE_DEMO___SEQ_TREE_DEMO_DLG_HPP

/*  $Id: seq_tree_demo_dlg.hpp 28407 2013-07-03 19:44:45Z katargir $
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
 * Authors:  
 */

#include <corelib/ncbistd.hpp>

#include <wx/dialog.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

BEGIN_NCBI_SCOPE

class CSeqTreeWidget;


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSEQTREEDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSEQTREEDEMODLG_TITLE _("SeqTreeDemoDlg")
#define SYMBOL_CSEQTREEDEMODLG_IDNAME ID_CSEQTREEDEMODLG
#define SYMBOL_CSEQTREEDEMODLG_SIZE wxSize(400, 300)
#define SYMBOL_CSEQTREEDEMODLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqTreeDemoDlg class declaration
 */

class CSeqTreeDemoDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CSeqTreeDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqTreeDemoDlg();
    CSeqTreeDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTREEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CSEQTREEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTREEDEMODLG_POSITION, const wxSize& size = SYMBOL_CSEQTREEDEMODLG_SIZE, long style = SYMBOL_CSEQTREEDEMODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTREEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CSEQTREEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTREEDEMODLG_POSITION, const wxSize& size = SYMBOL_CSEQTREEDEMODLG_SIZE, long style = SYMBOL_CSEQTREEDEMODLG_STYLE );

    /// Destructor
    ~CSeqTreeDemoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSeqTreeDemoDlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_CSEQTREEDEMODLG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end CSeqTreeDemoDlg event handler declarations

////@begin CSeqTreeDemoDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqTreeDemoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSeqTreeDemoDlg member variables
    CSeqTreeWidget* m_Widget;
    /// Control identifiers
    enum {
        ID_CSEQTREEDEMODLG = 10000,
        ID_FOREIGN = 10001
    };
////@end CSeqTreeDemoDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_SEQ_TREE_DEMO___SEQ_TREE_DEMO_DLG_HPP
