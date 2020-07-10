/*  $Id: subprepfieldeditor.hpp 27600 2013-03-11 20:24:24Z filippov $
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
#ifndef _SUBPREPFIELDEDITOR_H_
#define _SUBPREPFIELDEDITOR_H_

#include <corelib/ncbistd.hpp>

#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSubPrep_panel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSUBPREPFIELDEDITOR 10093
#define ID_MIX 10094
#define SYMBOL_CSUBPREPFIELDEDITOR_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBPREPFIELDEDITOR_TITLE _("SubPrepFieldEditor")
#define SYMBOL_CSUBPREPFIELDEDITOR_IDNAME ID_CSUBPREPFIELDEDITOR
#define SYMBOL_CSUBPREPFIELDEDITOR_SIZE wxSize(400, 300)
#define SYMBOL_CSUBPREPFIELDEDITOR_POSITION wxDefaultPosition
////@end control identifiers
#define ID_GLOBALTEXT 10096
#define ID_GLOBALCHOICE 10097
#define ID_CLEARCTRL 10098
#define ID_EDITCTRL 10099
#define ID_REGISTER_URL 10100

/*!
 * CSubPrepFieldEditor class declaration
 */

class CSubPrepFieldEditor: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSubPrepFieldEditor )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubPrepFieldEditor();
    CSubPrepFieldEditor( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                         ICommandProccessor* proccessor,
                         IWorkbench* workbench,
                         wxWindowID id = SYMBOL_CSUBPREPFIELDEDITOR_IDNAME, const wxPoint& pos = SYMBOL_CSUBPREPFIELDEDITOR_POSITION, const wxSize& size = SYMBOL_CSUBPREPFIELDEDITOR_SIZE, long style = SYMBOL_CSUBPREPFIELDEDITOR_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSUBPREPFIELDEDITOR_IDNAME, const wxPoint& pos = SYMBOL_CSUBPREPFIELDEDITOR_POSITION, const wxSize& size = SYMBOL_CSUBPREPFIELDEDITOR_SIZE, long style = SYMBOL_CSUBPREPFIELDEDITOR_STYLE );

    /// Destructor
    ~CSubPrepFieldEditor();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSubPrepFieldEditor event handler declarations

////@end CSubPrepFieldEditor event handler declarations
    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_GLOBALCHOICE
    void OnGlobalchoiceSelected( wxCommandEvent& event );
    void OnLeaveTextColumnWindow(wxMouseEvent &event);
    void OnClearValues( wxHyperlinkEvent& event );
    void OnEditctrlHyperlinkClicked( wxHyperlinkEvent& event );
    void OnRegisterHyperlinkClicked( wxHyperlinkEvent& event );

////@begin CSubPrepFieldEditor member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubPrepFieldEditor member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSubPrepFieldEditor member variables
    wxStaticText* m_Label;
    wxStaticText* m_Star;
    wxBoxSizer* m_ValueSizer;
    wxTextCtrl* m_Mix;
    wxBoxSizer* m_EditSizer;
////@end CSubPrepFieldEditor member variables

    void SetConverter (string label, string column_name, CTableFieldCommandConverter * converter,
                       bool must_choose = false, bool required = false,
                       string default_val = "",
                       string url = "",
                       bool allow_mix = true);
    bool UpdateValues(string default_val = "");
    const string GetColumnName() { return m_ColumnName; };

private:
    wxChoice* m_GlobalValue;
    wxTextCtrl* m_GlobalText;

    objects::CSeq_entry_Handle m_Seh;
    ICommandProccessor* m_CmdProcessor;
    IWorkbench* m_Workbench;
    CTableFieldCommandConverter * m_Converter;
    string m_ColumnName;
    string m_Url;
    bool m_Required;
    bool m_MustChoose;
    bool m_AllowMix;

    void x_ShowStar(bool show);
    bool x_SetEditorType(bool mixed, bool has_choices);
    void x_SetEditorControls(bool mixed, string message);
    CSubPrep_panel* x_GetParent();
    void x_SetOneValue(string new_val);

};

END_NCBI_SCOPE

#endif
    // _SUBPREPFIELDEDITOR_H_
