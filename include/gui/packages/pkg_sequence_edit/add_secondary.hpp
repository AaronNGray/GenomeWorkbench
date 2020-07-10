/*  $Id: add_secondary.hpp 38636 2017-06-05 19:12:02Z asztalos $
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
 * Authors: Igor Filippov
 */
#ifndef _ADD_SECONDARY_H_
#define _ADD_SECONDARY_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/dialog.h>
#include <wx/datectrl.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

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
class CGrowableListCtrl;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CADDSECONDARY 10000
#define ID_CADDSECONDARY_TEXTCTRL 10001
#define ID_CADDSECONDARY_TEXTCTRL1 10002
#define ID_CADDSECONDARY_TEXTCTRL2 10003
#define ID_CADDSECONDARY_TEXTCTRL3 10004
#define ID_CADDSECONDARY_TEXTCTRL4 10005
#define ID_CADDSECONDARY_SCROLLEDWINDOW 10006
#define ID_CADDSECONDARY_CHECKBOX 10007
#define ID_CADDSECONDARY_CHECKBOX1 10008
#define ID_CADDSECONDARY_SCROLLEDWINDOW1 10009
#define ID_CADDSECONDARY_CHECKBOX2 10010
#define ID_CADDSECONDARY_CHECKBOX3 10011
#define ID_CADDSECONDARY_CHECKBOX4 10012
#define ID_CADDSECONDARY_CHECKBOX5 10013
#define ID_CADDSECONDARY_CHECKBOX6 10014
#define ID_CADDSECONDARY_CHECKBOX7 10015
#define ID_CADDSECONDARY_CHECKBOX8 10016
#define ID_CADDSECONDARY_CHECKBOX9 10017
#define ID_CADDSECONDARY_BUTTON 10018
#define ID_CADDSECONDARY_DATECTRL 10019
#define ID_CADDSECONDARY_CHECKBOX10 10020
#define ID_CADDSECONDARY_CHECKBOX11 10021
#define SYMBOL_CADDSECONDARY_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADDSECONDARY_TITLE _("Add Secondary")
#define SYMBOL_CADDSECONDARY_IDNAME ID_CADDSECONDARY
#define SYMBOL_CADDSECONDARY_SIZE wxSize(400, 300)
#define SYMBOL_CADDSECONDARY_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAddSecondary class declaration
 */

class CAddSecondary : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CAddSecondary )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAddSecondary();
    CAddSecondary( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CADDSECONDARY_IDNAME, const wxString& caption = SYMBOL_CADDSECONDARY_TITLE, const wxPoint& pos = SYMBOL_CADDSECONDARY_POSITION, const wxSize& size = SYMBOL_CADDSECONDARY_SIZE, long style = SYMBOL_CADDSECONDARY_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CADDSECONDARY_IDNAME, const wxString& caption = SYMBOL_CADDSECONDARY_TITLE, const wxPoint& pos = SYMBOL_CADDSECONDARY_POSITION, const wxSize& size = SYMBOL_CADDSECONDARY_SIZE, long style = SYMBOL_CADDSECONDARY_STYLE );

    /// Destructor
    ~CAddSecondary();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAddSecondary event handler declarations

////@end CAddSecondary event handler declarations

////@begin CAddSecondary member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAddSecondary member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
    bool TransferDataToWindow();
    void DeleteAllKeywords(wxCommandEvent& event );

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
////@begin CAddSecondary member variables
    wxTextCtrl* m_Division;
    wxTextCtrl* m_Origin;
    wxTextCtrl* m_OldDate;
    wxTextCtrl* m_Source;
    wxTextCtrl* m_Taxonomy;
    CGrowableListCtrl* m_SecondaryAccessions;
    wxCheckBox* m_CopyToReplaces;
    wxCheckBox* m_RetainReplaces;
    CGrowableListCtrl* m_Keywords;
    wxCheckBox* m_HTGS_DRAFT;
    wxCheckBox* m_HTGS_ACTIVEFIN;
    wxCheckBox* m_TPA_EXPERIMENTAL;
    wxCheckBox* m_TPA_REASSEMBLY;
    wxCheckBox* m_HTGS_FULLTOP;
    wxCheckBox* m_HTGS_CANCELLED;
    wxCheckBox* m_TPA_INFERENTIAL;
    wxCheckBox* m_BARCODE;
    wxCheckBox* m_UNORDERED;
    wxCheckBox* m_TPA_SPECIALIST_DB;
    wxDatePickerCtrl* m_Date;
////@end CAddSecondary member variables
};

END_NCBI_SCOPE

#endif
    // _ADD_SECONDARY_H_
