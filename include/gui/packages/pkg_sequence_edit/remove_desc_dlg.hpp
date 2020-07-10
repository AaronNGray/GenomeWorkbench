/*  $Id: remove_desc_dlg.hpp 45081 2020-05-26 20:33:57Z asztalos $
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
#ifndef _REMOVE_DESC_DLG_H_
#define _REMOVE_DESC_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>

class wxListCtrl;
class wxTextCtrl;
class wxRadioButton;
class wxCheckBox;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CREMOVE_DESC_DLG 11000
#define SYMBOL_CREMOVE_DESC_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CREMOVE_DESC_DLG_TITLE _("Remove Descriptors")
#define SYMBOL_CREMOVE_DESC_DLG_IDNAME ID_CREMOVE_DESC_DLG
#define SYMBOL_CREMOVE_DESC_DLG_SIZE wxDefaultSize
#define SYMBOL_CREMOVE_DESC_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_LISTCTRL_DESC_DLG 11001
#define ID_RBTN_RMVDESC1 11002
#define ID_RBTN_RMVDESC2 11003
#define ID_TXT_RMVDESC 11004
#define ID_CHKBOX_RMVDESC 11005

/*!
 * CRemoveDescDlg class declaration
 */

class CRemoveDescDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CRemoveDescDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemoveDescDlg();
    CRemoveDescDlg( wxWindow* parent, 
        objects::CSeq_entry_Handle seh, 
        wxWindowID id = SYMBOL_CREMOVE_DESC_DLG_IDNAME, 
        const wxString& caption = SYMBOL_CREMOVE_DESC_DLG_TITLE, 
        const wxPoint& pos = SYMBOL_CREMOVE_DESC_DLG_POSITION, 
        const wxSize& size = SYMBOL_CREMOVE_DESC_DLG_SIZE, 
        long style = SYMBOL_CREMOVE_DESC_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CREMOVE_DESC_DLG_IDNAME, 
        const wxString& caption = SYMBOL_CREMOVE_DESC_DLG_TITLE, 
        const wxPoint& pos = SYMBOL_CREMOVE_DESC_DLG_POSITION, 
        const wxSize& size = SYMBOL_CREMOVE_DESC_DLG_SIZE, 
        long style = SYMBOL_CREMOVE_DESC_DLG_STYLE );

    /// Destructor
    ~CRemoveDescDlg();

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

    CRef<CCmdComposite> GetCommand();

private:
    void x_ApplyToSeq_entry(objects::CSeq_entry_Handle tse,
        const objects::CSeq_entry& se, CCmdComposite* composite, objects::CSeqdesc::E_Choice choice, const string& name);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    map<string, objects::CSeqdesc::E_Choice> m_Types;
    wxListCtrl* m_ListCtrl;
    wxTextCtrl* m_TextCtrl;
    wxRadioButton* m_RadioButton;
    wxCheckBox* m_CheckBox;
};

END_NCBI_SCOPE

#endif
    // _REMOVE_DESC_DLG_H_
