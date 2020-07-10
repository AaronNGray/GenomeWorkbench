/*  $Id: bulk_feature_add_dlg_std.hpp 34380 2015-12-22 21:33:57Z filippov $
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
 * Authors:  Colleen Bollin, Igor Filippov
 */
#ifndef _BULK_FEATURE_ADD_DLG_STD_H_
#define _BULK_FEATURE_ADD_DLG_STD_H_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/virusnoncodingfeaturespanel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/framework/workbench.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/radiobox.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/notebook.h>

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
#define ID_CBULKFEATUREADDDLG 10029
#define SYMBOL_CBULKFEATUREADDDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CBULKFEATUREADDDLG_TITLE _("Add Features")
#define SYMBOL_CBULKFEATUREADDDLG_IDNAME ID_CBULKFEATUREADDDLG
#define SYMBOL_CBULKFEATUREADDDLG_SIZE wxSize(400, 300)
#define SYMBOL_CBULKFEATUREADDDLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_FEATURE_TABLE_TEXT 10030
#define ID_APPLY_BUTTON 10031

/*!
 * CBulkFeatureAddDlgStd class declaration
 */

class CBulkFeatureAddDlgStd: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CBulkFeatureAddDlgStd )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkFeatureAddDlgStd();
    CBulkFeatureAddDlgStd( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                           IWorkbench* workbench = NULL, const string &key = kEmptyStr,
                           wxWindowID id = SYMBOL_CBULKFEATUREADDDLG_IDNAME, const wxString& caption = SYMBOL_CBULKFEATUREADDDLG_TITLE, const wxPoint& pos = SYMBOL_CBULKFEATUREADDDLG_POSITION, const wxSize& size = SYMBOL_CBULKFEATUREADDDLG_SIZE, long style = SYMBOL_CBULKFEATUREADDDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBULKFEATUREADDDLG_IDNAME, const wxString& caption = SYMBOL_CBULKFEATUREADDDLG_TITLE, const wxPoint& pos = SYMBOL_CBULKFEATUREADDDLG_POSITION, const wxSize& size = SYMBOL_CBULKFEATUREADDDLG_SIZE, long style = SYMBOL_CBULKFEATUREADDDLG_STYLE );
    
    /// Destructor
    ~CBulkFeatureAddDlgStd();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBulkFeatureAddDlgStd event handler declarations

////@end CBulkFeatureAddDlgStd event handler declarations

////@begin CBulkFeatureAddDlgStd member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkFeatureAddDlgStd member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBulkFeatureAddDlgStd member variables
////@end CBulkFeatureAddDlgStd member variables

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    void ChooseFeatureType(objects::CSeqFeatData::ESubtype subtype);
    void OnApplyClick(wxCommandEvent& event );
private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxNotebook *m_Notebook;
    
    IWorkbench* m_Workbench;
    string m_DefaultKey;


    CRef<CCmdComposite> x_CreateImportFeatures(string key, string comment, bool skip_existing = false);
    CRef<CCmdComposite> x_CreateDloops();
    CRef<CCmdComposite> x_CreateControlRegions();
    size_t x_GetFeatureChoice ();
};

END_NCBI_SCOPE

#endif
    // _BULK_FEATURE_ADD_DLG_STD_H_
