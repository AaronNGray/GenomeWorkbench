/*  $Id: suppress_genes.hpp 37242 2016-12-19 18:51:11Z filippov $
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


#ifndef _SUPPRESS_GENES_H_
#define _SUPPRESS_GENES_H_

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

#define ID_SUPPRESS_GENES 14000
#define SYMBOL_SUPPRESS_GENES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SUPPRESS_GENES_TITLE _("Suppress Genes")
#define SYMBOL_SUPPRESS_GENES_IDNAME ID_SUPPRESS_GENES
#define SYMBOL_SUPPRESS_GENES_SIZE wxDefaultSize
#define SYMBOL_SUPPRESS_GENES_POSITION wxDefaultPosition
////@end control identifiers
#define ID_SUPPRESS_GENES_LISTCTRL 14001
#define ID_SUPPRESS_GENES_OKCANCEL 14005

class CSuppressGenes : public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CSuppressGenes )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSuppressGenes();
    CSuppressGenes( wxWindow* parent, IWorkbench *wb, wxWindowID id = SYMBOL_SUPPRESS_GENES_IDNAME, const wxString& caption = SYMBOL_SUPPRESS_GENES_TITLE, const wxPoint& pos = SYMBOL_SUPPRESS_GENES_POSITION, const wxSize& size = SYMBOL_SUPPRESS_GENES_SIZE, long style = SYMBOL_SUPPRESS_GENES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SUPPRESS_GENES_IDNAME, const wxString& caption = SYMBOL_SUPPRESS_GENES_TITLE, const wxPoint& pos = SYMBOL_SUPPRESS_GENES_POSITION, const wxSize& size = SYMBOL_SUPPRESS_GENES_SIZE, long style = SYMBOL_SUPPRESS_GENES_STYLE );

    /// Destructor
    ~CSuppressGenes();

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
   
private:
    bool CreateSuppressingGeneXrefs(CSeq_feat& f);
    map<pair<int,int>,string> m_Types;
    vector<string> m_Descriptions;
    vector<bool> m_SubtypeAnyFlag;
    wxListCtrl* m_ListCtrl;
    CStringConstraintPanel* m_StringConstraintPanel;
};

END_NCBI_SCOPE

#endif  // _SUPPRESS_GENES_H_
