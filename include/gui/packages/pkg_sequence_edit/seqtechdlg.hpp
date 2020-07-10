/*  $Id: seqtechdlg.hpp 27619 2013-03-13 18:24:23Z bollin $
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
#ifndef _SEQTECHDLG_H_
#define _SEQTECHDLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/general/User_object.hpp>
#include <gui/widgets/edit/sequencingtechnologypanel.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>

#include <wx/dialog.h>
#include <wx/statbox.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SEQTECHDLG 10014
#define ID_RAW_BTN 10112
#define ID_ASSEMBLED_BTN 10113
#define SYMBOL_CSEQTECHDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSEQTECHDLG_TITLE _("Sequencing Technology")
#define SYMBOL_CSEQTECHDLG_IDNAME ID_SEQTECHDLG
#define SYMBOL_CSEQTECHDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSEQTECHDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqTechDlg class declaration
 */

class CSeqTechDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CSeqTechDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqTechDlg();
    CSeqTechDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, CSourceRequirements::EWizardType wizard_type, wxWindowID id = SYMBOL_CSEQTECHDLG_IDNAME, const wxString& caption = SYMBOL_CSEQTECHDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTECHDLG_POSITION, const wxSize& size = SYMBOL_CSEQTECHDLG_SIZE, long style = SYMBOL_CSEQTECHDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTECHDLG_IDNAME, const wxString& caption = SYMBOL_CSEQTECHDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTECHDLG_POSITION, const wxSize& size = SYMBOL_CSEQTECHDLG_SIZE, long style = SYMBOL_CSEQTECHDLG_STYLE );

    /// Destructor
    ~CSeqTechDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSeqTechDlg event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RAW_BTN
    void OnRawBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ASSEMBLED_BTN
    void OnAssembledBtnSelected( wxCommandEvent& event );

////@end CSeqTechDlg event handler declarations

////@begin CSeqTechDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqTechDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSeqTechDlg member variables
    wxBoxSizer* m_PanelSizer;
    wxRadioButton* m_RawBtn;
    wxRadioButton* m_AssembledBtn;
////@end CSeqTechDlg member variables

     virtual CRef<CCmdComposite> GetCommand();
     virtual string GetErrorMessage();
     static string CheckSequencingTechnology(CRef<objects::CUser_object> user, CSourceRequirements::EWizardType wizard_type, string raw_or_assembled);

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    CRef<objects::CUser_object> m_EditedObj;
    CSequencingTechnologyPanel* m_Panel;
    CSourceRequirements::EWizardType m_WizardType;
    string x_GetRawOrAssembled();

    void x_GetSeqTechUserFromSeqEntry ();
    static bool x_IsSpecialTech(string tech);
};

END_NCBI_SCOPE

/*!
 * CSeqTechDlg class declaration
 */

class CSeqTechDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CSeqTechDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqTechDlg();
    CSeqTechDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTECHDLG_IDNAME, const wxString& caption = SYMBOL_CSEQTECHDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTECHDLG_POSITION, const wxSize& size = SYMBOL_CSEQTECHDLG_SIZE, long style = SYMBOL_CSEQTECHDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTECHDLG_IDNAME, const wxString& caption = SYMBOL_CSEQTECHDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQTECHDLG_POSITION, const wxSize& size = SYMBOL_CSEQTECHDLG_SIZE, long style = SYMBOL_CSEQTECHDLG_STYLE );

    /// Destructor
    ~CSeqTechDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSeqTechDlg event handler declarations
////@end CSeqTechDlg event handler declarations

////@begin CSeqTechDlg member function declarations
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqTechDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSeqTechDlg member variables
    wxBoxSizer* m_PanelSizer;
    wxRadioButton* m_RawBtn;
    wxRadioButton* m_AssembledBtn;
////@end CSeqTechDlg member variables
};

#endif
    // _SEQTECHDLG_H_
