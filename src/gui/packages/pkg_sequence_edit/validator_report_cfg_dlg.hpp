/*  $Id: validator_report_cfg_dlg.hpp 41057 2018-05-16 16:03:20Z katargir $
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
#ifndef _VALIDATOR_REPORT_CFG_DLG_H_
#define _VALIDATOR_REPORT_CFG_DLG_H_

#include <corelib/ncbistd.hpp>

#include <objects/submit/Seq_submit.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/framework/pkg_wb_connect.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/checkbox.h>

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

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CVALIDATORREPORTCFGDLG 10384
#define ID_REPORT_BTN 10385
#define ID_VALID_REPORT_DISMISS 10386
#define ID_VALID_REPORT_SEQUESTER_BTN 10387
#define ID_VALID_REPORT_SEGREGATE_BTN 10388
#define SYMBOL_CVALIDATORREPORTCFGDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CVALIDATORREPORTCFGDLG_TITLE _("Validator Report")
#define SYMBOL_CVALIDATORREPORTCFGDLG_IDNAME ID_CVALIDATORREPORTCFGDLG
#define SYMBOL_CVALIDATORREPORTCFGDLG_SIZE wxSize(400, 300)
#define SYMBOL_CVALIDATORREPORTCFGDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CValidatorReportCfgDlg class declaration
 */

class CValidatorReportCfgDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CValidatorReportCfgDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CValidatorReportCfgDlg();
    CValidatorReportCfgDlg( wxWindow* parent, 
                            CObjectFor<objects::CValidError::TErrs>* errs, objects::CScope* scope,  objects::CSeq_entry_Handle top_seq_entry, CConstRef<objects::CSeq_submit> seq_submit, IWorkbench* workbench,
        wxWindowID id = SYMBOL_CVALIDATORREPORTCFGDLG_IDNAME, const wxString& caption = SYMBOL_CVALIDATORREPORTCFGDLG_TITLE, const wxPoint& pos = SYMBOL_CVALIDATORREPORTCFGDLG_POSITION, const wxSize& size = SYMBOL_CVALIDATORREPORTCFGDLG_SIZE, long style = SYMBOL_CVALIDATORREPORTCFGDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CVALIDATORREPORTCFGDLG_IDNAME, const wxString& caption = SYMBOL_CVALIDATORREPORTCFGDLG_TITLE, const wxPoint& pos = SYMBOL_CVALIDATORREPORTCFGDLG_POSITION, const wxSize& size = SYMBOL_CVALIDATORREPORTCFGDLG_SIZE, long style = SYMBOL_CVALIDATORREPORTCFGDLG_STYLE );

    /// Destructor
    ~CValidatorReportCfgDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CValidatorReportCfgDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REPORT_BTN
    void OnReportBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_VALID_REPORT_DISMISS
    void OnValidReportDismissClick( wxCommandEvent& event );

////@end CValidatorReportCfgDlg event handler declarations

////@begin CValidatorReportCfgDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CValidatorReportCfgDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CValidatorReportCfgDlg member variables
////@end CValidatorReportCfgDlg member variables

    void OnSequester( wxCommandEvent& event );
    void OnSegregate( wxCommandEvent& event );

    void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }

protected:
    void GetSelected(set<objects::CBioseq_Handle> &selected);

    CObjectFor<objects::CValidError::TErrs>* m_Errs;
    objects::CScope* m_Scope;
    vector<wxCheckBox *> m_Opts;
    wxCheckBox *m_Verbose;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    IWorkbench*     m_Workbench;
    wxString m_WorkDir;
};

END_NCBI_SCOPE

#endif
    // _VALIDATOR_REPORT_CFG_DLG_H_
