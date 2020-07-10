/*  $Id: restore_rna_edit.hpp 41749 2018-09-19 20:25:42Z asztalos $
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

#ifndef _GUI_PACKAGES_RESTORERNAEDITING_HPP_
#define _GUI_PACKAGES_RESTORERNAEDITING_HPP_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/utils/command_processor.hpp>
BEGIN_NCBI_SCOPE

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

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CRESTORERNAEDITING wxID_ANY
#define ID_RESTORE_PROTEINNAME 21001
#define ID_RESTORE_RNA_EDIT_APPLY 21002
#define ID_CHECKBOX 21004
#define SYMBOL_CRESTORERNAEDITING_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRESTORERNAEDITING_TITLE _("Restore RNA editing of coding region")
#define SYMBOL_CRESTORERNAEDITING_IDNAME ID_CRESTORERNAEDITING
#define SYMBOL_CRESTORERNAEDITING_SIZE wxSize(400, 300)
#define SYMBOL_CRESTORERNAEDITING_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRestoreRNAediting class declaration
 */

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CRestoreRNAediting: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CRestoreRNAediting )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRestoreRNAediting();
    CRestoreRNAediting( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor,
			wxWindowID id = SYMBOL_CRESTORERNAEDITING_IDNAME, const wxString& caption = SYMBOL_CRESTORERNAEDITING_TITLE, const wxPoint& pos = SYMBOL_CRESTORERNAEDITING_POSITION, const wxSize& size = SYMBOL_CRESTORERNAEDITING_SIZE, long style = SYMBOL_CRESTORERNAEDITING_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRESTORERNAEDITING_IDNAME, const wxString& caption = SYMBOL_CRESTORERNAEDITING_TITLE, const wxPoint& pos = SYMBOL_CRESTORERNAEDITING_POSITION, const wxSize& size = SYMBOL_CRESTORERNAEDITING_SIZE, long style = SYMBOL_CRESTORERNAEDITING_STYLE );

    /// Destructor
    ~CRestoreRNAediting();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRestoreRNAediting event handler declarations
    void OnApply(wxCommandEvent& event);
////@end CRestoreRNAediting event handler declarations

////@begin CRestoreRNAediting member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRestoreRNAediting member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CRestoreRNAediting member variables
    wxTextCtrl* m_Name;
    wxCheckBox* m_LeaveUp;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    ICommandProccessor* m_CmdProcessor;
////@end CRestoreRNAediting member variables
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES_RESTORERNAEDITING_HPP_
