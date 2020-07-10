/*  $Id: remove_gene_xrefs.hpp 33202 2015-06-16 18:57:02Z filippov $
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


#ifndef _REMOVE_GENE_XREFS_H_
#define _REMOVE_GENE_XREFS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>

#include <wx/listctrl.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

#define ID_REMOVE_GENE_XREFS 14000
#define SYMBOL_REMOVE_GENE_XREFS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_REMOVE_GENE_XREFS_TITLE _("Remove Gene Xrefs")
#define SYMBOL_REMOVE_GENE_XREFS_IDNAME ID_REMOVE_GENE_XREFS
#define SYMBOL_REMOVE_GENE_XREFS_SIZE wxDefaultSize
#define SYMBOL_REMOVE_GENE_XREFS_POSITION wxDefaultPosition
////@end control identifiers
#define ID_REMOVE_GENE_XREFS_LISTCTRL 14001
#define ID_SUPPRESSING_CHECKBOX 14002
#define ID_NONSUPPRESSING_CHECKBOX 14003
#define ID_UNNECESSARY_CHECKBOX 14004
#define ID_REMOVE_GENE_XREFS_OKCANCEL 14005

class CRemoveGeneXrefs : public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CRemoveGeneXrefs )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemoveGeneXrefs();
    CRemoveGeneXrefs( wxWindow* parent, IWorkbench *wb, wxWindowID id = SYMBOL_REMOVE_GENE_XREFS_IDNAME, const wxString& caption = SYMBOL_REMOVE_GENE_XREFS_TITLE, const wxPoint& pos = SYMBOL_REMOVE_GENE_XREFS_POSITION, const wxSize& size = SYMBOL_REMOVE_GENE_XREFS_SIZE, long style = SYMBOL_REMOVE_GENE_XREFS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_REMOVE_GENE_XREFS_IDNAME, const wxString& caption = SYMBOL_REMOVE_GENE_XREFS_TITLE, const wxPoint& pos = SYMBOL_REMOVE_GENE_XREFS_POSITION, const wxSize& size = SYMBOL_REMOVE_GENE_XREFS_SIZE, long style = SYMBOL_REMOVE_GENE_XREFS_STYLE );

    /// Destructor
    ~CRemoveGeneXrefs();

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

    static CRef<CCmdComposite> RemoveAll(CSeq_entry_Handle seh);
    static CRef<CCmdComposite> RemoveUnnecessary(CSeq_entry_Handle seh);
    static CRef<CCmdComposite> RemoveNonsuppressing(CSeq_entry_Handle seh);
    static CRef<CCmdComposite> RemoveOrphanLocus(CSeq_entry_Handle seh);
    static CRef<CCmdComposite> RemoveOrphanLocus_tag(CSeq_entry_Handle seh);
private:
    bool RemoveSuppressingGeneXrefs(CSeq_feat& f);
    map<pair<int,int>,string> m_Types;
    vector<string> m_Descriptions;
    vector<bool> m_SubtypeAnyFlag;
    wxListCtrl* m_ListCtrl;
    wxCheckBox* m_Suppressing;
    wxCheckBox* m_Nonsuppressing;
    wxCheckBox* m_Unnecessary;
    CStringConstraintPanel* m_StringConstraintPanel;
};

END_NCBI_SCOPE

#endif  // _REMOVE_GENE_XREFS_H_
