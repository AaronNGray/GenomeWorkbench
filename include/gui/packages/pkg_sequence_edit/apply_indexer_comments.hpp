/*  $Id: apply_indexer_comments.hpp 38632 2017-06-05 17:16:13Z asztalos $
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
#ifndef _APPLY_INDEXER_COMMENTS_H_
#define _APPLY_INDEXER_COMMENTS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>

#include <wx/dialog.h>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <gui/widgets/wx/wx_utils.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
using namespace objects;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CAPPLY_INDEXER_COMMENTS 11000
#define SYMBOL_CAPPLY_INDEXER_COMMENTS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CAPPLY_INDEXER_COMMENTS_TITLE _("Apply Indexer Comments")
#define SYMBOL_CAPPLY_INDEXER_COMMENTS_IDNAME ID_CAPPLY_INDEXER_COMMENTS
#define SYMBOL_CAPPLY_INDEXER_COMMENTS_SIZE wxDefaultSize
#define SYMBOL_CAPPLY_INDEXER_COMMENTS_POSITION wxDefaultPosition
////@end control identifiers

class CApplyIndexerComments: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CApplyIndexerComments )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CApplyIndexerComments();
    CApplyIndexerComments( wxWindow* parent, IWorkbench *wb, wxWindowID id = SYMBOL_CAPPLY_INDEXER_COMMENTS_IDNAME, const wxString& caption = SYMBOL_CAPPLY_INDEXER_COMMENTS_TITLE, const wxPoint& pos = SYMBOL_CAPPLY_INDEXER_COMMENTS_POSITION, const wxSize& size = SYMBOL_CAPPLY_INDEXER_COMMENTS_SIZE, long style = SYMBOL_CAPPLY_INDEXER_COMMENTS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAPPLY_INDEXER_COMMENTS_IDNAME, const wxString& caption = SYMBOL_CAPPLY_INDEXER_COMMENTS_TITLE, const wxPoint& pos = SYMBOL_CAPPLY_INDEXER_COMMENTS_POSITION, const wxSize& size = SYMBOL_CAPPLY_INDEXER_COMMENTS_SIZE, long style = SYMBOL_CAPPLY_INDEXER_COMMENTS_STYLE );

    /// Destructor
    ~CApplyIndexerComments();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage(){ return m_ErrorMessage;}
    void OnSelect(wxCommandEvent& event);
private:  
    wxChoice* m_Choice;
    wxTextCtrl* m_TextCtrl;
    CStringConstraintPanel *m_StringConstraintPanel;
    wxChoice* m_ChoiceType;
    void ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CCmdComposite* composite);
    CRef<objects::CSeqdesc> CreateNewComment();
    CRef<CUser_object> GetUserObjectComment(const string &str);
    string GetTextComment(const string &str);
};

END_NCBI_SCOPE

#endif
    // _APPLY_INDEXER_COMMENTS_H_
