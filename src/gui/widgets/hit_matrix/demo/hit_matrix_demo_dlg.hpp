#ifndef _HIT_MATRIX_DEMO_DLG_H_
#define _HIT_MATRIX_DEMO_DLG_H_

/*  $Id: hit_matrix_demo_dlg.hpp 25591 2012-04-12 13:46:20Z katargir $
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

#include <gui/widgets/hit_matrix/hit_matrix_widget.hpp>
#include <gui/widgets/hit_matrix/dense_ds.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_ds_builder.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>

#include <wx/dialog.h>

////@begin includes
#include "wx/statline.h"
////@end includes

class wxStaticLine;
class wxTextCtrl;


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////@begin forward declarations
class CHitMatrixWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_HIT_MATRIX_DEMO_DLG 10000
#define ID_WIDGET 10001
#define ID_ACC_TEXT 10004
#define ID_ACC 10005
#define ID_FILE_TEXT 10006
#define ID_FILE 10003
#define SYMBOL_CHITMATRIXDEMODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CHITMATRIXDEMODLG_TITLE _("CHitMatrixDemoDlg")
#define SYMBOL_CHITMATRIXDEMODLG_IDNAME ID_HIT_MATRIX_DEMO_DLG
#define SYMBOL_CHITMATRIXDEMODLG_SIZE wxSize(600, 600)
#define SYMBOL_CHITMATRIXDEMODLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */
#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


class CHitMatrixDemoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CHitMatrixDemoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CHitMatrixDemoDlg();
    CHitMatrixDemoDlg( wxWindow* parent, wxWindowID id = SYMBOL_CHITMATRIXDEMODLG_IDNAME, const wxString& caption = SYMBOL_CHITMATRIXDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CHITMATRIXDEMODLG_POSITION, const wxSize& size = SYMBOL_CHITMATRIXDEMODLG_SIZE, long style = SYMBOL_CHITMATRIXDEMODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CHITMATRIXDEMODLG_IDNAME, const wxString& caption = SYMBOL_CHITMATRIXDEMODLG_TITLE, const wxPoint& pos = SYMBOL_CHITMATRIXDEMODLG_POSITION, const wxSize& size = SYMBOL_CHITMATRIXDEMODLG_SIZE, long style = SYMBOL_CHITMATRIXDEMODLG_STYLE );

    /// Destructor
    ~CHitMatrixDemoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CHitMatrixDemoDlg event handler declarations

    void OnCloseWindow( wxCloseEvent& event );

    void OnAccClick( wxCommandEvent& event );

    void OnFileClick( wxCommandEvent& event );

    void OnCloseClick( wxCommandEvent& event );

////@end CHitMatrixDemoDlg event handler declarations

////@begin CHitMatrixDemoDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CHitMatrixDemoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CHitMatrixDemoDlg member variables
    CHitMatrixWidget* m_MatrixWidget;
    wxTextCtrl* m_AccInput;
    wxTextCtrl* m_FileInput;
    CHitMatrixDSBuilder m_Builder;
private:
    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;
    CIRef<IHitMatrixDataSource> m_DataSource;
////@end CHitMatrixDemoDlg member variables
};

END_NCBI_SCOPE

#endif
    // _HIT_MATRIX_DEMO_DLG_H_
