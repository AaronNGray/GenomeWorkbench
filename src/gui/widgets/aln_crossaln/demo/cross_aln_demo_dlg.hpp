#ifndef _CROSS_ALN_DEMO_DLG_H_
#define _CROSS_ALN_DEMO_DLG_H_


/*  $Id: cross_aln_demo_dlg.hpp 25624 2012-04-13 17:21:45Z katargir $
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

#include <gui/widgets/aln_crossaln/cross_aln_widget.hpp>
#include <gui/widgets/hit_matrix/dense_ds.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_ds_builder.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>


/*!
 * Includes
 */

#include <wx/dialog.h>

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
class CCrossAlnWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CROSS_ALN_DEMO_DLG 10000
#define ID_WIDGET 10001
#define ID_ACC_TEXT 10004
#define ID_ACC 10005
#define ID_FILE_TEXT 10006
#define ID_FILE 10003
#define SYMBOL_CCROSSALNDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CCROSSALNDEMODLG_TITLE _("CCrossAlnDemoDlg")
#define SYMBOL_CCROSSALNDEMODLG_IDNAME ID_CROSS_ALN_DEMO_DLG
#define SYMBOL_CCROSSALNDEMODLG_SIZE wxSize(600, 600)
#define SYMBOL_CCROSSALNDEMODLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCrossAlnDemoDlg class declaration
 */

class CCrossAlnDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CCrossAlnDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    CCrossAlnDemoDlg();
    CCrossAlnDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CCROSSALNDEMODLG_IDNAME, const wxString& caption = SYMBOL_CCROSSALNDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CCROSSALNDEMODLG_POSITION, const wxSize& size = SYMBOL_CCROSSALNDEMODLG_SIZE, long style = SYMBOL_CCROSSALNDEMODLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCROSSALNDEMODLG_IDNAME, const wxString& caption = SYMBOL_CCROSSALNDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CCROSSALNDEMODLG_POSITION, const wxSize& size = SYMBOL_CCROSSALNDEMODLG_SIZE, long style = SYMBOL_CCROSSALNDEMODLG_STYLE );

    ~CCrossAlnDemoDlg();

    void Init();

    void CreateControls();

////@begin CCrossAlnDemoDlg event handler declarations

    void OnCloseWindow( wxCloseEvent& event );

    void OnAccClick( wxCommandEvent& event );

    void OnFileClick( wxCommandEvent& event );

    void OnCloseClick( wxCommandEvent& event );

////@end CCrossAlnDemoDlg event handler declarations

////@begin CCrossAlnDemoDlg member function declarations

    CHitMatrixDSBuilder GetBuilder() const { return m_Builder ; }
    void SetBuilder(CHitMatrixDSBuilder value) { m_Builder = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CCrossAlnDemoDlg member function declarations

    static bool ShowToolTips();

////@begin CCrossAlnDemoDlg member variables
    CCrossAlnWidget* m_CrossAlnWidget;
    wxTextCtrl* m_AccInput;
    wxTextCtrl* m_FileInput;
    CHitMatrixDSBuilder m_Builder;
private:
    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;
    CIRef<ICrossAlnDataSource> m_DataSource;
////@end CCrossAlnDemoDlg member variables
};

END_NCBI_SCOPE

#endif
    // _CROSS_ALN_DEMO_DLG_H_
