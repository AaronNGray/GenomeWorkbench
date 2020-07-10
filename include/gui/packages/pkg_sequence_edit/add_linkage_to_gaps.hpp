/*  $Id: add_linkage_to_gaps.hpp 43364 2019-06-20 14:46:17Z asztalos $
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
#ifndef _ADD_LINKAGE_TO_GAPS_H_
#define _ADD_LINKAGE_TO_GAPS_H_

#include <corelib/ncbistd.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_align_handle.hpp>

#include <wx/dialog.h>
#include <wx/sizer.h>

class wxChoice;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ADD_LINKAGE_TO_GAPS wxID_ANY
#define SYMBOL_CADD_LINKAGE_TO_GAPS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADD_LINKAGE_TO_GAPS_TITLE _("Add Linkage To Gaps")
#define SYMBOL_CADD_LINKAGE_TO_GAPS_IDNAME ID_ADD_LINKAGE_TO_GAPS
#define SYMBOL_CADD_LINKAGE_TO_GAPS_SIZE wxSize(400,100)
#define SYMBOL_CADD_LINKAGE_TO_GAPS_POSITION wxDefaultPosition
////@end control identifiers
#define ID_ADD_LINKAGE_TO_GAPS_GAP_TYPE 11508
#define ID_ADD_LINKAGE_TO_GAPS_LINKAGE 11509

class CAddLinkageToGaps : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CAddLinkageToGaps )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAddLinkageToGaps();
    CAddLinkageToGaps( wxWindow* parent, 
                      wxWindowID id = SYMBOL_CADD_LINKAGE_TO_GAPS_IDNAME, 
                      const wxString& caption = SYMBOL_CADD_LINKAGE_TO_GAPS_TITLE, 
                      const wxPoint& pos = SYMBOL_CADD_LINKAGE_TO_GAPS_POSITION, 
                      const wxSize& size = SYMBOL_CADD_LINKAGE_TO_GAPS_SIZE, 
                      long style = SYMBOL_CADD_LINKAGE_TO_GAPS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
		 wxWindowID id = SYMBOL_CADD_LINKAGE_TO_GAPS_IDNAME, 
		 const wxString& caption = SYMBOL_CADD_LINKAGE_TO_GAPS_TITLE, 
		 const wxPoint& pos = SYMBOL_CADD_LINKAGE_TO_GAPS_POSITION, 
		 const wxSize& size = SYMBOL_CADD_LINKAGE_TO_GAPS_SIZE, 
		 long style = SYMBOL_CADD_LINKAGE_TO_GAPS_STYLE );

    /// Destructor
    ~CAddLinkageToGaps();

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

    CRef<CCmdComposite> GetCommand(objects::CSeq_entry_Handle tse);
    void OnGapType(wxCommandEvent& event);
    void OnLinkage(wxCommandEvent& event);

    
private:

    wxChoice *m_GapType;
    wxChoice *m_Linkage;
    wxChoice *m_LinkageEvidence;
};


END_NCBI_SCOPE

#endif
    // _ADD_LINKAGE_TO_GAPS_H_
