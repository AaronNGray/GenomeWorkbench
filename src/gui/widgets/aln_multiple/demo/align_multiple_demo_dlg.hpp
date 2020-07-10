#ifndef GUI_WIDGETS_ALN_MULTIPLE_DEMO___ALIGN_MULTIPLE_DEMO_DLG__HPP
#define GUI_WIDGETS_ALN_MULTIPLE_DEMO___ALIGN_MULTIPLE_DEMO_DLG__HPP

/*  $Id: align_multiple_demo_dlg.hpp 25617 2012-04-13 14:36:39Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */
#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_multiple/row_display_style.hpp>
#include <gui/widgets/aln_multiple/alnmulti_widget.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>

#include <gui/widgets/wx/ui_command.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>

#include <wx/dialog.h>

////@begin includes
#include "wx/statline.h"
////@end includes

class wxTextCtrl;

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////@begin forward declarations
class CAlnMultiWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ALIGN_MULTI_DEMO_DLG 10000
#define ID_WIDGET 10001
#define ID_ACC_TEXT 10004
#define ID_ACC 10005
#define ID_FILE_TEXT 10006
#define ID_FILE 10003
#define SYMBOL_CALIGNMULTIPLEDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CALIGNMULTIPLEDEMODLG_TITLE _("Align Mulitple Widget Demo")
#define SYMBOL_CALIGNMULTIPLEDEMODLG_IDNAME ID_ALIGN_MULTI_DEMO_DLG
#define SYMBOL_CALIGNMULTIPLEDEMODLG_SIZE wxDefaultSize
#define SYMBOL_CALIGNMULTIPLEDEMODLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

class CAlignMultipleDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CAlignMultipleDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    CAlignMultipleDemoDlg();
    CAlignMultipleDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNMULTIPLEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CALIGNMULTIPLEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CALIGNMULTIPLEDEMODLG_POSITION, const wxSize& size = SYMBOL_CALIGNMULTIPLEDEMODLG_SIZE, long style = SYMBOL_CALIGNMULTIPLEDEMODLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNMULTIPLEDEMODLG_IDNAME, const wxString& caption = SYMBOL_CALIGNMULTIPLEDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CALIGNMULTIPLEDEMODLG_POSITION, const wxSize& size = SYMBOL_CALIGNMULTIPLEDEMODLG_SIZE, long style = SYMBOL_CALIGNMULTIPLEDEMODLG_STYLE );

    ~CAlignMultipleDemoDlg();

    void Init();

    void CreateControls();

    void OnRebuild();

////@begin CAlignMultipleDemoDlg event handler declarations

    void OnCloseWindow( wxCloseEvent& event );

    void OnAccClick( wxCommandEvent& event );

    void OnFileClick( wxCommandEvent& event );

    void OnCloseClick( wxCommandEvent& event );

////@end CAlignMultipleDemoDlg event handler declarations

////@begin CAlignMultipleDemoDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAlignMultipleDemoDlg member function declarations

    static bool ShowToolTips();

////@begin CAlignMultipleDemoDlg member variables
    CAlnMultiWidget* m_AlnWidget;
    wxTextCtrl* m_AccInput;
    wxTextCtrl* m_FileInput;
    CAlnMultiDSBuilder m_Builder;
protected:
    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;
    CRef<IAlnMultiDataSource> m_DataSource;
    CRowStyleCatalog m_StyleCatalog;
////@end CAlignMultipleDemoDlg member variables
};


END_NCBI_SCOPE

#endif   // GUI_WIDGETS_ALN_MULTIPLE_DEMO___ALIGN_MULTIPLE_DEMO_DLG__HPP
