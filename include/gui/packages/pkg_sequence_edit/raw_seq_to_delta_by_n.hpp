/*  $Id: raw_seq_to_delta_by_n.hpp 43609 2019-08-08 16:12:53Z filippov $
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
#ifndef _RAW_SEQ_TO_DELTA_BY_N_H_
#define _RAW_SEQ_TO_DELTA_BY_N_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/rawseqtodeltabyn_panel.hpp>
#include <gui/widgets/data/report_dialog.hpp>

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_RAW_SEQ_TO_DELTA_BY_N wxID_ANY
#define SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_TITLE _("Add Assembly Gaps To Sequence")
#define SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_IDNAME ID_RAW_SEQ_TO_DELTA_BY_N
#define SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_SIZE wxSize(400,100)
#define SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_POSITION wxDefaultPosition
////@end control identifiers

class CRawSeqToDeltaByN : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CRawSeqToDeltaByN )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRawSeqToDeltaByN();
    CRawSeqToDeltaByN( wxWindow* parent, 
                      wxWindowID id = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_IDNAME, 
                      const wxString& caption = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_TITLE, 
                      const wxPoint& pos = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_POSITION, 
                      const wxSize& size = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_SIZE, 
                      long style = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
         wxWindowID id = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_IDNAME, 
         const wxString& caption = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_TITLE, 
         const wxPoint& pos = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_POSITION, 
         const wxSize& size = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_SIZE, 
         long style = SYMBOL_CRAW_SEQ_TO_DELTA_BY_N_STYLE );

    /// Destructor
    ~CRawSeqToDeltaByN();

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

    void apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title);
   
    static void DeltaSeqToRaw(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor);
   
private:
    CRawSeqToDeltaByNPanel* m_Panel;
};


END_NCBI_SCOPE

#endif
    // _RAW_SEQ_TO_DELTA_BY_N_H_
