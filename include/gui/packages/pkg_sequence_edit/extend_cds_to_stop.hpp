/*  $Id: extend_cds_to_stop.hpp 37267 2016-12-20 20:34:36Z filippov $
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
#ifndef _EXTEND_CDS_TO_STOP_H_
#define _EXTEND_CDS_TO_STOP_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/edit_feat_loc_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>


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
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CEXTEND_CDS_TO_STOP 10268
#define SYMBOL_CEXTEND_CDS_TO_STOP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEXTEND_CDS_TO_STOP_TITLE _("Extend Proteins to Stop Codons")
#define SYMBOL_CEXTEND_CDS_TO_STOP_IDNAME ID_CEXTEND_CDS_TO_STOP
#define SYMBOL_CEXTEND_CDS_TO_STOP_SIZE wxSize(900, 500)
#define SYMBOL_CEXTEND_CDS_TO_STOP_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBulkEditFeatDlg class declaration
 */

class CExtendCDSToStop: public CBulkCmdDlg 
{    
    DECLARE_DYNAMIC_CLASS( CExtendCDSToStop )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CExtendCDSToStop();
    CExtendCDSToStop( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CEXTEND_CDS_TO_STOP_IDNAME, const wxString& caption = SYMBOL_CEXTEND_CDS_TO_STOP_TITLE, const wxPoint& pos = SYMBOL_CEXTEND_CDS_TO_STOP_POSITION, const wxSize& size = SYMBOL_CEXTEND_CDS_TO_STOP_SIZE, long style = SYMBOL_CEXTEND_CDS_TO_STOP_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEXTEND_CDS_TO_STOP_IDNAME, const wxString& caption = SYMBOL_CEXTEND_CDS_TO_STOP_TITLE, const wxPoint& pos = SYMBOL_CEXTEND_CDS_TO_STOP_POSITION, const wxSize& size = SYMBOL_CEXTEND_CDS_TO_STOP_SIZE, long style = SYMBOL_CEXTEND_CDS_TO_STOP_STYLE );

    /// Destructor
    ~CExtendCDSToStop();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

     void ProcessUpdateFeatEvent( wxCommandEvent& event );


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
    CRef<CCmdComposite> RetranslateCDSCommand(CScope& scope, CSeq_feat& cds);
    void ResetGenes(CRef<CCmdComposite> cmd, const objects::CMappedFeat& feat, CRef<CSeq_loc> add, CScope &scope);
    void ExtendmRNA(CRef<CCmdComposite> cmd, const objects::CMappedFeat& feat, CRef<CSeq_loc> add, CScope &scope);

    string m_ErrorMessage;
    COkCancelPanel* m_OkCancel;
    wxCheckBox* m_ExtendProtFeats;
    wxCheckBox* m_Retranslate;
    wxCheckBox* m_ExtendmRNA;
    wxCheckBox* m_ResetGenes;
};

END_NCBI_SCOPE

#endif

