/*  $Id: bulk_molinfo_edit_dlg.hpp 27162 2013-01-03 14:10:05Z bollin $
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
#ifndef _BULK_MOLINFO_EDIT_DLG_H_
#define _BULK_MOLINFO_EDIT_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/grid.h"
////@end includes
#include "wx/sizer.h"

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
#define ID_CBULKMOLINFOEDITDLG 10077
#define ID_GRID 10012
#define SYMBOL_CBULKMOLINFOEDITDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CBULKMOLINFOEDITDLG_TITLE _("Molecule Information")
#define SYMBOL_CBULKMOLINFOEDITDLG_IDNAME ID_CBULKMOLINFOEDITDLG
#define SYMBOL_CBULKMOLINFOEDITDLG_SIZE wxSize(400, 300)
#define SYMBOL_CBULKMOLINFOEDITDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBulkMolinfoEditDlg class declaration
 */

class CBulkMolinfoEditDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CBulkMolinfoEditDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkMolinfoEditDlg();
    CBulkMolinfoEditDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, CSourceRequirements::EWizardType wizard_type, wxWindowID id = SYMBOL_CBULKMOLINFOEDITDLG_IDNAME, const wxString& caption = SYMBOL_CBULKMOLINFOEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CBULKMOLINFOEDITDLG_POSITION, const wxSize& size = SYMBOL_CBULKMOLINFOEDITDLG_SIZE, long style = SYMBOL_CBULKMOLINFOEDITDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBULKMOLINFOEDITDLG_IDNAME, const wxString& caption = SYMBOL_CBULKMOLINFOEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CBULKMOLINFOEDITDLG_POSITION, const wxSize& size = SYMBOL_CBULKMOLINFOEDITDLG_SIZE, long style = SYMBOL_CBULKMOLINFOEDITDLG_STYLE );

    /// Destructor
    ~CBulkMolinfoEditDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBulkMolinfoEditDlg event handler declarations

////@end CBulkMolinfoEditDlg event handler declarations

////@begin CBulkMolinfoEditDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkMolinfoEditDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBulkMolinfoEditDlg member variables
////@end CBulkMolinfoEditDlg member variables
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxGrid* m_Grid;
    CSourceRequirements::EWizardType m_WizardType;
};

END_NCBI_SCOPE

#endif
    // _BULK_MOLINFO_EDIT_DLG_H_
