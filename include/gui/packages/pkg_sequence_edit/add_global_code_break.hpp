/*  $Id: add_global_code_break.hpp 37325 2016-12-23 17:50:04Z filippov $
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
 * Authors:  Igor Filippov
 */
#ifndef _ADD_GLOBAL_CODE_BREAK_H_
#define _ADD_GLOBAL_CODE_BREAK_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/edit_feat_loc_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>


/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/textctrl.h>
/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CADD_GLOBAL_CODE_BREAK 10268
#define SYMBOL_CADD_GLOBAL_CODE_BREAK_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADD_GLOBAL_CODE_BREAK_TITLE _("Add Code Break")
#define SYMBOL_CADD_GLOBAL_CODE_BREAK_IDNAME ID_CADD_GLOBAL_CODE_BREAK
#define SYMBOL_CADD_GLOBAL_CODE_BREAK_SIZE wxSize(900, 500)
#define SYMBOL_CADD_GLOBAL_CODE_BREAK_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAddGlobalCodeBreak class declaration
 */

class CAddGlobalCodeBreak: public CBulkCmdDlg 
{    
    DECLARE_DYNAMIC_CLASS( CAddGlobalCodeBreak )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAddGlobalCodeBreak();
    CAddGlobalCodeBreak( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CADD_GLOBAL_CODE_BREAK_IDNAME, const wxString& caption = SYMBOL_CADD_GLOBAL_CODE_BREAK_TITLE, const wxPoint& pos = SYMBOL_CADD_GLOBAL_CODE_BREAK_POSITION, const wxSize& size = SYMBOL_CADD_GLOBAL_CODE_BREAK_SIZE, long style = SYMBOL_CADD_GLOBAL_CODE_BREAK_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CADD_GLOBAL_CODE_BREAK_IDNAME, const wxString& caption = SYMBOL_CADD_GLOBAL_CODE_BREAK_TITLE, const wxPoint& pos = SYMBOL_CADD_GLOBAL_CODE_BREAK_POSITION, const wxSize& size = SYMBOL_CADD_GLOBAL_CODE_BREAK_SIZE, long style = SYMBOL_CADD_GLOBAL_CODE_BREAK_STYLE );

    /// Destructor
    ~CAddGlobalCodeBreak();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
    CRef<CCmdComposite> RetranslateCDSCommand(CScope& scope, CSeq_feat& cds);
  
    string m_ErrorMessage;
    COkCancelPanel* m_OkCancel;
    wxTextCtrl* m_Triplet;
    wxChoice* m_AminoAcidCtrl;

    /// Control identifiers
    enum {
        ID_CSINGLECDSEXCEPTIONPANEL = 10104,
        ID_TEXTCTRL35 = 10105,
        ID_CHOICE14 = 10106
    };
    vector<string> m_AminoAcidList;
};

END_NCBI_SCOPE

#endif

