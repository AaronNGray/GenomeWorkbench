#ifndef _SEQGRAPHIC_DEMO_DLG_H_
#define _SEQGRAPHIC_DEMO_DLG_H_

/*  $Id: seqgraphic_demo_dlg.hpp 19282 2009-05-14 18:26:32Z wuliangs $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *    
 */

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <wx/dialog.h>
#include <wx/checkbox.h>

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

class wxTextCtrl;

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////@begin forward declarations
class CSeqGraphicWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SEQ_GRAPHIC_DEMO_DLG 10000
#define ID_WIDGET 10001
#define ID_ACC_TEXT 10004
#define ID_ACC 10005
#define ID_FILE_TEXT 10006
#define ID_FILE 10003
#define ID_CHECKBOX1 10002
#define SYMBOL_CSEQGRAPHICDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX
#define SYMBOL_CSEQGRAPHICDEMODLG_TITLE _("CSeqGraphicDemoDlg")
#define SYMBOL_CSEQGRAPHICDEMODLG_IDNAME ID_SEQ_GRAPHIC_DEMO_DLG
#define SYMBOL_CSEQGRAPHICDEMODLG_SIZE wxSize(800, 600)
#define SYMBOL_CSEQGRAPHICDEMODLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqGraphicDemoDlg class declaration
 */

class CSeqGraphicDemoDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CSeqGraphicDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    CSeqGraphicDemoDlg();
    CSeqGraphicDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSEQGRAPHICDEMODLG_IDNAME, const wxString& caption = SYMBOL_CSEQGRAPHICDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CSEQGRAPHICDEMODLG_POSITION, const wxSize& size = SYMBOL_CSEQGRAPHICDEMODLG_SIZE, long style = SYMBOL_CSEQGRAPHICDEMODLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQGRAPHICDEMODLG_IDNAME, const wxString& caption = SYMBOL_CSEQGRAPHICDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CSEQGRAPHICDEMODLG_POSITION, const wxSize& size = SYMBOL_CSEQGRAPHICDEMODLG_SIZE, long style = SYMBOL_CSEQGRAPHICDEMODLG_STYLE );

    ~CSeqGraphicDemoDlg();

    void Init();

    void CreateControls();

////@begin CSeqGraphicDemoDlg event handler declarations

    void OnCloseWindow( wxCloseEvent& event );

    void OnAccClick( wxCommandEvent& event );

    void OnFileClick( wxCommandEvent& event );

    void OnFlipStrand( wxCommandEvent& event );

    void OnCloseClick( wxCommandEvent& event );

////@end CSeqGraphicDemoDlg event handler declarations

////@begin CSeqGraphicDemoDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CSeqGraphicDemoDlg member function declarations

    static bool ShowToolTips();

////@begin CSeqGraphicDemoDlg member variables
    CSeqGraphicWidget* m_SeqWidget;
    wxTextCtrl* m_AccInput;
    wxTextCtrl* m_FileInput;
    wxCheckBox* m_FlipStrand;
private:
    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;
////@end CSeqGraphicDemoDlg member variables
    CRef<objects::CSeq_id>     m_SeqId;
};

END_NCBI_SCOPE

#endif
    // _SEQGRAPHIC_DEMO_DLG_H_
