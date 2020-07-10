/*  $Id: create_seq_hist_for_tpa.hpp 42019 2018-12-04 18:25:10Z katargir $
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


#ifndef _CCREATESEQHISTFORTPA_H_
#define _CCREATESEQHISTFORTPA_H_


/*!
 * Includes
 */

////@begin includes
#include <corelib/ncbistd.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
////@end includes

class wxTextCtrl;
class wxCheckBox;

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCREATESEQHISTFORTPA wxID_ANY
#define SYMBOL_CCREATESEQHISTFORTPA_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCREATESEQHISTFORTPA_TITLE _("Blast Options")
#define SYMBOL_CCREATESEQHISTFORTPA_IDNAME ID_CCREATESEQHISTFORTPA
#define SYMBOL_CCREATESEQHISTFORTPA_SIZE wxSize(400, 300)
#define SYMBOL_CCREATESEQHISTFORTPA_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCreateSeqHistForTpa class declaration
 */

class CCreateSeqHistForTpa : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CCreateSeqHistForTpa )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCreateSeqHistForTpa();
    CCreateSeqHistForTpa( wxWindow* parent, CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CCREATESEQHISTFORTPA_IDNAME, const wxString& caption = SYMBOL_CCREATESEQHISTFORTPA_TITLE, const wxPoint& pos = SYMBOL_CCREATESEQHISTFORTPA_POSITION, const wxSize& size = SYMBOL_CCREATESEQHISTFORTPA_SIZE, long style = SYMBOL_CCREATESEQHISTFORTPA_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCREATESEQHISTFORTPA_IDNAME, const wxString& caption = SYMBOL_CCREATESEQHISTFORTPA_TITLE, const wxPoint& pos = SYMBOL_CCREATESEQHISTFORTPA_POSITION, const wxSize& size = SYMBOL_CCREATESEQHISTFORTPA_SIZE, long style = SYMBOL_CCREATESEQHISTFORTPA_STYLE );

    /// Destructor
    ~CCreateSeqHistForTpa();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCreateSeqHistForTpa event handler declarations

////@end CCreateSeqHistForTpa event handler declarations

////@begin CCreateSeqHistForTpa member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCreateSeqHistForTpa member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
    static CRef<CCmdComposite> GetCreateSeqHistFromTpaCommand(CSeq_entry_Handle tse, string &msg, double word_size, double expect_value);
private:
    static void RunBlast(const CBioseq_Handle& sh, const CBioseq_Handle& qh, vector<CRef<objects::CSeq_align> > &align_vector, int from, int to, double word_size, double expect_value);
    static string ValidateAlignments(const CSeq_hist &hist, const CBioseq_Handle &bsh);

////@begin CCreateSeqHistForTpa member variables
    wxTextCtrl* m_WordSize;
    wxTextCtrl* m_ExpectValue;
    wxCheckBox* m_AllowRepeats;
////@end CCreateSeqHistForTpa member variables
    CSeq_entry_Handle m_TopSeqEntry;
};

class CRemoveSeqHistAssembly
{
public:
    static CRef<CCmdComposite> GetCommand(CSeq_entry_Handle tse);
};

END_NCBI_SCOPE
#endif
    // _CCREATESEQHISTFORTPA_H_
