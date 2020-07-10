#ifndef _SEQ_TEXT_DEMO_DLG_H_
#define _SEQ_TEXT_DEMO_DLG_H_

/*  $Id: seq_text_demo_dlg.hpp 25551 2012-04-06 15:55:27Z katargir $
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
 * Authors:  Colleen Bollin, based on a file by Andrey Yazhuk
 *
 * File Description:
 *
 */


#include <corelib/ncbistd.hpp>


#include <gui/widgets/seq_text/seq_text_widget.hpp>
#include <gui/widgets/seq_text/seq_text_ds.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>

////@begin includes
#include <wx/dialog.h>
////@end includes

class wxStaticLine;
class wxTextCtrl;


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////@begin forward declarations
class CSeqTextWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SEQ_TEXT_DEMO_DLG 10000
#define ID_WIDGET 10001
#define ID_ACC_TEXT 10004
#define ID_ACC 10005
#define ID_FILE_TEXT 10006
#define ID_FILE 10003
#define SYMBOL_CSEQTEXTDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CSEQTEXTDEMODLG_TITLE _("CSeqTextDemoDlg")
#define SYMBOL_CSEQTEXTDEMODLG_IDNAME ID_SEQ_TEXT_DEMO_DLG
#define SYMBOL_CSEQTEXTDEMODLG_SIZE wxSize(600, 600)
#define SYMBOL_CSEQTEXTDEMODLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */
#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


class CSeqTextDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CSeqTextDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqTextDemoDlg();
    CSeqTextDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTEXTDEMODLG_IDNAME, const wxString& caption = SYMBOL_CSEQTEXTDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTEXTDEMODLG_POSITION, const wxSize& size = SYMBOL_CSEQTEXTDEMODLG_SIZE, long style = SYMBOL_CSEQTEXTDEMODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTEXTDEMODLG_IDNAME, const wxString& caption = SYMBOL_CSEQTEXTDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTEXTDEMODLG_POSITION, const wxSize& size = SYMBOL_CSEQTEXTDEMODLG_SIZE, long style = SYMBOL_CSEQTEXTDEMODLG_STYLE );

    /// Destructor
    ~CSeqTextDemoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CHitMatrixDemoDlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_HIT_MATRIX_DEMO_DLG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ACC
    void OnAccClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FILE
    void OnFileClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
    void OnCloseClick( wxCommandEvent& event );

////@end CHitMatrixDemoDlg event handler declarations

////@begin CHitMatrixDemoDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CHitMatrixDemoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSeqTextDemoDlg member variables
    CSeqTextWidget* m_SeqTextWidget;
    wxTextCtrl* m_AccInput;
    wxTextCtrl* m_FileInput;
private:
    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;
    CRef<CSeqTextDataSource> m_DataSource;

////@end CSeqTextDemoDlg member variables
};

END_NCBI_SCOPE

#endif
    // _SEQ_TEXT_DEMO_DLG_H_
