/*  $Id: add_transl_except_dlg.hpp 29898 2014-03-07 19:06:46Z asztalos $
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
#ifndef _ADD_TRANSL_EXCEPT_DLG_H_
#define _ADD_TRANSL_EXCEPT_DLG_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxTextCtrl;
class wxCheckBox;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CADDTRANSLEXCEPTDLG 10075
#define ID_ATE_CDS_COMMENT_TXT 10334
#define ID_ATE_OVERHANG_BTN 10335
#define ID_ATE_EXTEND_BTN 10336
#define ID_ATE_ADJUST_GENE_BTN 10337
#define ID_ATE_OKCANCEL 10275
#define SYMBOL_CADDTRANSLEXCEPTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADDTRANSLEXCEPTDLG_TITLE _("Add Translation Exception")
#define SYMBOL_CADDTRANSLEXCEPTDLG_IDNAME ID_CADDTRANSLEXCEPTDLG
#define SYMBOL_CADDTRANSLEXCEPTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CADDTRANSLEXCEPTDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAddTranslExceptDlg class declaration
 */

class CAddTranslExceptDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CAddTranslExceptDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAddTranslExceptDlg();
    CAddTranslExceptDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CADDTRANSLEXCEPTDLG_IDNAME, const wxString& caption = SYMBOL_CADDTRANSLEXCEPTDLG_TITLE, const wxPoint& pos = SYMBOL_CADDTRANSLEXCEPTDLG_POSITION, const wxSize& size = SYMBOL_CADDTRANSLEXCEPTDLG_SIZE, long style = SYMBOL_CADDTRANSLEXCEPTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CADDTRANSLEXCEPTDLG_IDNAME, const wxString& caption = SYMBOL_CADDTRANSLEXCEPTDLG_TITLE, const wxPoint& pos = SYMBOL_CADDTRANSLEXCEPTDLG_POSITION, const wxSize& size = SYMBOL_CADDTRANSLEXCEPTDLG_SIZE, long style = SYMBOL_CADDTRANSLEXCEPTDLG_STYLE );

    /// Destructor
    ~CAddTranslExceptDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAddTranslExceptDlg event handler declarations

////@end CAddTranslExceptDlg event handler declarations

////@begin CAddTranslExceptDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAddTranslExceptDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAddTranslExceptDlg member variables
    wxTextCtrl* m_CDSCommentTxt;
    wxCheckBox* m_OverhangBtn;
    wxCheckBox* m_ExtendBtn;
    wxCheckBox* m_AdjustGeneBtn;
    COkCancelPanel* m_OkCancel;
////@end CAddTranslExceptDlg member variables

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
    string m_ErrorMessage;

};

END_NCBI_SCOPE

#endif
    // _ADD_TRANSL_EXCEPT_DLG_H_
