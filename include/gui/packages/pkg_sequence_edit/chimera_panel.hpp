/*  $Id: chimera_panel.hpp 31553 2014-10-22 16:28:19Z katargir $
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
#ifndef _CHIMERA_PANEL_H_
#define _CHIMERA_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subprepfieldeditor.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/radiobut.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSubPrep_panel;
class CSubPrepFieldEditor;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCHIMERAPANEL 10095
#define ID_YES 10096
#define ID_NO 10097
#define SYMBOL_CCHIMERAPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCHIMERAPANEL_TITLE _("ChimeraPanel")
#define SYMBOL_CCHIMERAPANEL_IDNAME ID_CCHIMERAPANEL
#define SYMBOL_CCHIMERAPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCHIMERAPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CChimeraPanel class declaration
 */

class CChimeraPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CChimeraPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CChimeraPanel();
    CChimeraPanel( wxWindow* parent, 
                   objects::CSeq_entry_Handle seh, 
                   ICommandProccessor* proccessor,
                   IWorkbench* workbench,
                   wxWindowID id = SYMBOL_CCHIMERAPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCHIMERAPANEL_POSITION, const wxSize& size = SYMBOL_CCHIMERAPANEL_SIZE, long style = SYMBOL_CCHIMERAPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCHIMERAPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCHIMERAPANEL_POSITION, const wxSize& size = SYMBOL_CCHIMERAPANEL_SIZE, long style = SYMBOL_CCHIMERAPANEL_STYLE );

    /// Destructor
    ~CChimeraPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CChimeraPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_YES
    void OnYesSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NO
    void OnNoSelected( wxCommandEvent& event );

////@end CChimeraPanel event handler declarations

////@begin CChimeraPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CChimeraPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CChimeraPanel member variables
    wxRadioButton* m_Yes;
    wxRadioButton* m_No;
    wxBoxSizer* m_ProgVerSizer;
////@end CChimeraPanel member variables

    void UpdateChimera();

private:
    objects::CSeq_entry_Handle m_Seh;
    ICommandProccessor* m_CmdProcessor;
    IWorkbench* m_Workbench;

    CSubPrepFieldEditor* m_ProgramVersionField;

    CSubPrep_panel* x_GetParent();
    void x_ShowYes(CTableFieldCommandConverter* converter);
    void x_ShowNo();

};

END_NCBI_SCOPE

#endif
    // _CHIMERA_PANEL_H_
