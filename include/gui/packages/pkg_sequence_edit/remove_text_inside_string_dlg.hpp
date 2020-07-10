/*  $Id: remove_text_inside_string_dlg.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Igor Filippov, based on work of Colleen Bollin
 */
#ifndef _REMOVE_TEXT_INSIDE_STR_DLG_H_
#define _REMOVE_TEXT_INSIDE_STR_DLG_H_

#include <corelib/ncbistd.hpp>

#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/button.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CParseTextOptionsDlg;
class wxBoxSizer;
class CCapChangePanel;
class CFieldConstraintPanel;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CREMTEXTINSSTRDLG 10161
#define ID_WINDOW2 10162
#define ID_WINDOW7 10387
#define ID_WINDOW3 10172
#define ID_WINDOW13 10277
#define SYMBOL_CREMTEXTINSSTRDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CREMTEXTINSSTRDLG_TITLE _("Remove Text Inside String")
#define SYMBOL_CREMTEXTINSSTRDLG_IDNAME ID_CREMTEXTINSSTRDLG
#define SYMBOL_CREMTEXTINSSTRDLG_SIZE wxSize(400, 300)
#define SYMBOL_CREMTEXTINSSTRDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRemoveTextInsideStrDlg class declaration
 */

class CRemoveTextInsideStrDlg: public CBulkCmdDlg , public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CRemoveTextInsideStrDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemoveTextInsideStrDlg();
    CRemoveTextInsideStrDlg( wxWindow* parent, IWorkbench* wb, 
        wxWindowID id = SYMBOL_CREMTEXTINSSTRDLG_IDNAME, 
        const wxString& caption = SYMBOL_CREMTEXTINSSTRDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CREMTEXTINSSTRDLG_POSITION, 
        const wxSize& size = SYMBOL_CREMTEXTINSSTRDLG_SIZE, 
        long style = SYMBOL_CREMTEXTINSSTRDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CREMTEXTINSSTRDLG_IDNAME, 
        const wxString& caption = SYMBOL_CREMTEXTINSSTRDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CREMTEXTINSSTRDLG_POSITION, 
        const wxSize& size = SYMBOL_CREMTEXTINSSTRDLG_SIZE, 
        long style = SYMBOL_CREMTEXTINSSTRDLG_STYLE );

    /// Destructor
    ~CRemoveTextInsideStrDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRemoveTextInsideStrDlg event handler declarations

////@end CRemoveTextInsideStrDlg event handler declarations
    void ProcessUpdateFeatEvent( wxCommandEvent& event );

////@begin CRemoveTextInsideStrDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRemoveTextInsideStrDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRemoveTextInsideStrDlg member variables
    CParseTextOptionsDlg* m_ParseOptions;
    wxBoxSizer* m_FromFieldSizer;
    wxBoxSizer* m_ToFieldSizer;
    CCapChangePanel* m_CapChangeOptions;
    CFieldConstraintPanel* m_ConstraintPanel;
    COkCancelPanel* m_OkCancel;
////@end CRemoveTextInsideStrDlg member variables
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    virtual void UpdateEditor();

    void SetFromFieldType(CFieldNamePanel::EFieldType field_type);

private:
    CFieldChoicePanel* m_FieldFrom;
    wxRadioButton *m_RemoveFirstOnly;
};

END_NCBI_SCOPE

#endif
    // _REMOVE_TEXT_INSIDE_STR_DLG_H_
