/*  $Id: correct_rna_strand_dlg.hpp 30500 2014-06-03 16:08:32Z filippov $
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
#ifndef _CORRECT_RNA_STRAND_DLG_H_
#define _CORRECT_RNA_STRAND_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/scrolwin.h>

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
#define ID_CCORRECTRNASTRANDDLG 10299
#define ID_RNA_STRAND_SCROLLEDWINDOW 10300
#define ID_REFRESH_STRAND_BTN 10301
#define ID_REVERSE_FEATURES_BTN 10302
#define ID_USE_SMART_FOR_STRAND_BTN 10303
#define ID_AUTOCORRECT_STRAND_BTN 10304
#define ID_CANCEL_RNA_STRAND_BTN 10305
#define SYMBOL_CCORRECTRNASTRANDDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCORRECTRNASTRANDDLG_TITLE _("Correct RNA Strandedness")
#define SYMBOL_CCORRECTRNASTRANDDLG_IDNAME ID_CCORRECTRNASTRANDDLG
#define SYMBOL_CCORRECTRNASTRANDDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCORRECTRNASTRANDDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCorrectRNAStrandDlg class declaration
 */

class CCorrectRNAStrandDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CCorrectRNAStrandDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCorrectRNAStrandDlg();
    CCorrectRNAStrandDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CCORRECTRNASTRANDDLG_IDNAME, const wxString& caption = SYMBOL_CCORRECTRNASTRANDDLG_TITLE, const wxPoint& pos = SYMBOL_CCORRECTRNASTRANDDLG_POSITION, const wxSize& size = SYMBOL_CCORRECTRNASTRANDDLG_SIZE, long style = SYMBOL_CCORRECTRNASTRANDDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCORRECTRNASTRANDDLG_IDNAME, const wxString& caption = SYMBOL_CCORRECTRNASTRANDDLG_TITLE, const wxPoint& pos = SYMBOL_CCORRECTRNASTRANDDLG_POSITION, const wxSize& size = SYMBOL_CCORRECTRNASTRANDDLG_SIZE, long style = SYMBOL_CCORRECTRNASTRANDDLG_STYLE );

    /// Destructor
    ~CCorrectRNAStrandDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCorrectRNAStrandDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REFRESH_STRAND_BTN
    void OnRefreshStrandBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_AUTOCORRECT_STRAND_BTN
    void OnAutocorrectStrandBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CANCEL_RNA_STRAND_BTN
    void OnCancelRnaStrandBtnClick( wxCommandEvent& event );

////@end CCorrectRNAStrandDlg event handler declarations

////@begin CCorrectRNAStrandDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCorrectRNAStrandDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCorrectRNAStrandDlg member variables
    wxScrolledWindow* m_SequenceListWindow;
    wxBoxSizer* m_SequenceListSizer;
    wxCheckBox* m_ReverseFeaturesBtn;
    wxCheckBox* m_UseSmart;
////@end CCorrectRNAStrandDlg member variables

    // for CBulkCmdDlg
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

protected:
    string m_ErrorString;
    int m_NumRows;
    int m_TotalHeight;
    int m_ScrollRate;
    vector<CBioseq_Handle> m_SeqList;
    vector<wxCheckBox *>m_PickList;
    map<string,CBioseq_Handle> m_SeqMap;
    vector<string> m_AccessionList;

    void x_PopulateAccessionList();
    void x_AddRow(const string& text, const string& status, bool selected);

    void x_GetStatusSMART();
    void x_ProcessRNAStrandLine(const CTempString& line, size_t pos, size_t max);
    void x_GetStatusLocal();

    typedef enum {
        RNAstrand_PLUS = 0,
        RNAstrand_MINUS,
        RNAstrand_MIXED,
        RNAstrand_NO_HITS,
        RNAstrand_UNEXPECTED,
        RNAstrand_PARSE_ERROR,
        RNAstrand_IN_PROGRESS
    } ERNAstrand_return_val;

    ERNAstrand_return_val x_StrandFromString(const string& status);
    string x_StringFromStrand(ERNAstrand_return_val rval);
    void x_ReverseFeatures(CBioseq_Handle bsh, CRef<CCmdComposite> cmd);

    vector<ERNAstrand_return_val> m_StatusList;
};

END_NCBI_SCOPE

#endif
    // _CORRECT_RNA_STRAND_DLG_H_
