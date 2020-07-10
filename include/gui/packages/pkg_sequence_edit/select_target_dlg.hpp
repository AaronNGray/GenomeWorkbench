/*  $Id: select_target_dlg.hpp 42741 2019-04-08 19:39:07Z filippov@NCBI.NLM.NIH.GOV $
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
#ifndef _SELECT_TARGET_DLG_H_
#define _SELECT_TARGET_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/widgets/data/report_dialog.hpp>

#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

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
#define ID_CSELECTTARGETDLG 10409
#define ID_TEXTCTRL13 10410
#define ID_SEL_TARGET_ACCEPT 10411
#define ID_SEL_TARGET_CLEAR 10412
#define ID_SEL_TARGET_CANCEL 10413
#define SYMBOL_CSELECTTARGETDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSELECTTARGETDLG_TITLE _("Select Target")
#define SYMBOL_CSELECTTARGETDLG_IDNAME ID_CSELECTTARGETDLG
#define SYMBOL_CSELECTTARGETDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSELECTTARGETDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSelectTargetDlg class declaration
 */

class CSelectTargetDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CSelectTargetDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSelectTargetDlg();
    CSelectTargetDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CSELECTTARGETDLG_IDNAME, const wxString& caption = SYMBOL_CSELECTTARGETDLG_TITLE, const wxPoint& pos = SYMBOL_CSELECTTARGETDLG_POSITION, const wxSize& size = SYMBOL_CSELECTTARGETDLG_SIZE, long style = SYMBOL_CSELECTTARGETDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSELECTTARGETDLG_IDNAME, const wxString& caption = SYMBOL_CSELECTTARGETDLG_TITLE, const wxPoint& pos = SYMBOL_CSELECTTARGETDLG_POSITION, const wxSize& size = SYMBOL_CSELECTTARGETDLG_SIZE, long style = SYMBOL_CSELECTTARGETDLG_STYLE );

    /// Destructor
    ~CSelectTargetDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSelectTargetDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_ACCEPT
    void OnSelTargetAcceptClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_CLEAR
    void OnSelTargetClearClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_CANCEL
    void OnSelTargetCancelClick( wxCommandEvent& event );

////@end CSelectTargetDlg event handler declarations

////@begin CSelectTargetDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSelectTargetDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSelectTargetDlg member variables
    wxTextCtrl* m_Text;
////@end CSelectTargetDlg member variables

protected:
    CBioseq_Handle FindBioseq(CSeq_entry_Handle seh, const string &acc_text);

    IWorkbench*     m_Workbench;
private:
    size_t m_previous_count;
    string m_previous_acc;    
};

END_NCBI_SCOPE

#endif
    // _SELECT_TARGET_DLG_H_
