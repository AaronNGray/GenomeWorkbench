/*  $Id: add_deflinedlg.hpp 42821 2019-04-18 19:32:56Z joukovv $
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
 * Authors:  Andrea Asztalos
 * 
 *  Dialog that allows manual editing of the definition line
 */

#ifndef _ADD_DEFLINEDLG_H_
#define _ADD_DEFLINEDLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE

#define ID_CADDDEFLINEDLG 10262
#define ID_ADDDEFLINE_TEXTCTRL 10263
#define ID_ADDDEFLINE_OKCANCEL 10278
#define SYMBOL_CADDDEFLINEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADDDEFLINEDLG_TITLE _("Add Definition Line")
#define SYMBOL_CADDDEFLINEDLG_IDNAME ID_CADDDEFLINEDLG
#define SYMBOL_CADDDEFLINEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CADDDEFLINEDLG_POSITION wxDefaultPosition


class CAddDefLineDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CAddDefLineDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAddDefLineDlg();
    CAddDefLineDlg( wxWindow* parent, IWorkbench* wb,
                wxWindowID id = SYMBOL_CADDDEFLINEDLG_IDNAME, 
                const wxString& caption = SYMBOL_CADDDEFLINEDLG_TITLE, 
                const wxPoint& pos = SYMBOL_CADDDEFLINEDLG_POSITION, 
                const wxSize& size = SYMBOL_CADDDEFLINEDLG_SIZE, 
                long style = SYMBOL_CADDDEFLINEDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CADDDEFLINEDLG_IDNAME, 
                const wxString& caption = SYMBOL_CADDDEFLINEDLG_TITLE, 
                const wxPoint& pos = SYMBOL_CADDDEFLINEDLG_POSITION, 
                const wxSize& size = SYMBOL_CADDDEFLINEDLG_SIZE, 
                long style = SYMBOL_CADDDEFLINEDLG_STYLE );

    /// Destructor
    ~CAddDefLineDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAddDefLineDlg event handler declarations

////@end CAddDefLineDlg event handler declarations

////@begin CAddDefLineDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAddDefLineDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    
    //// Implementation of class CBulkCmdDlg pure virtual functions
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

////@begin CAddDefLineDlg member variables
    wxTextCtrl* m_Title;
    COkCancelPanel* m_OkCancel;
////@end CAddDefLineDlg member variables
};

int NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CountDefLineConflicts(CSeq_entry_Handle entry, const string& title);

bool NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT UpdateAllDefinitionLine(CSeq_entry_Handle entry, 
    CCmdComposite* composite, const string& title, edit::EExistingText existing_text);
void NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT UpdateOneDefLine(const CSeq_entry_CI& entry_ci, 
    CCmdComposite* composite, const string& title, edit::EExistingText existing_text, bool& modified);

void NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT RemoveTitlesSeqEntry(CSeq_entry_Handle seh, 
        const CSeq_entry& entry, CCmdComposite* composite);

END_NCBI_SCOPE

#endif
    // _ADD_DEFLINEDLG_H_
