/*  $Id: set_global_refgene_status.hpp 38635 2017-06-05 18:51:04Z asztalos $
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
#ifndef _SET_GLOBAL_REFGENE_STATUS_H_
#define _SET_GLOBAL_REFGENE_STATUS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/dialog.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
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
#define ID_CSET_GLOBAL_REFGENE_STATUS wxID_ANY
#define SYMBOL_CSET_GLOBAL_REFGENE_STATUS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSET_GLOBAL_REFGENE_STATUS_TITLE _("Set Global RefGene Status")
#define SYMBOL_CSET_GLOBAL_REFGENE_STATUS_IDNAME ID_CSET_GLOBAL_REFGENE_STATUS
#define SYMBOL_CSET_GLOBAL_REFGENE_STATUS_SIZE wxDefaultSize
#define SYMBOL_CSET_GLOBAL_REFGENE_STATUS_POSITION wxDefaultPosition
////@end control identifiers


class CSetGlobalRefGeneStatus : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CSetGlobalRefGeneStatus )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSetGlobalRefGeneStatus();
    CSetGlobalRefGeneStatus( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_IDNAME, const wxString& caption = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_TITLE, const wxPoint& pos = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_POSITION, const wxSize& size = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_SIZE, long style = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_IDNAME, const wxString& caption = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_TITLE, const wxPoint& pos = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_POSITION, const wxSize& size = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_SIZE, long style = SYMBOL_CSET_GLOBAL_REFGENE_STATUS_STYLE );

    /// Destructor
    ~CSetGlobalRefGeneStatus();

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

    CRef<CCmdComposite> GetCommand();
private:
    void ApplyToCSeq_entry(const CSeq_entry& se, CCmdComposite* composite);
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxChoice *m_Status;
};



END_NCBI_SCOPE

#endif
    // _SET_GLOBAL_REFGENE_STATUS_H_