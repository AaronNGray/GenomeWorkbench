/*  $Id: mixedstrands.hpp 38626 2017-06-05 13:53:11Z asztalos $
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
 * Authors:  Andrea ASztalos
 */
#ifndef _MIXEDSTRANDS_H_
#define _MIXEDSTRANDS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/utils/job_adapter.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/dialog.h>
#include <wx/collpane.h>

class wxChoice;
class wxCheckBox;
class wxStaticText;

BEGIN_NCBI_SCOPE

class CCmdComposite;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMIXEDSTRANDS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CMIXEDSTRANDS_TITLE _("Mixed Strands")
#define SYMBOL_CMIXEDSTRANDS_IDNAME ID_CMIXEDSTRANDS
#define SYMBOL_CMIXEDSTRANDS_SIZE wxSize(400, 300)
#define SYMBOL_CMIXEDSTRANDS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMixedStrands class declaration
 */

class CMixedStrands : public CReportEditingDialog, public CJobAdapter::IJobCallback
{    
    DECLARE_DYNAMIC_CLASS( CMixedStrands )
    DECLARE_EVENT_TABLE()

public:
    typedef vector<CConstRef<objects::CSeq_align>> TAlignVector;

    /// Constructors
    CMixedStrands();
    CMixedStrands(wxWindow* parent, objects::CSeq_entry_Handle seh,
        wxWindowID id = SYMBOL_CMIXEDSTRANDS_IDNAME, 
        const wxString& caption = SYMBOL_CMIXEDSTRANDS_TITLE, 
        const wxPoint& pos = SYMBOL_CMIXEDSTRANDS_POSITION, 
        const wxSize& size = SYMBOL_CMIXEDSTRANDS_SIZE, 
        long style = SYMBOL_CMIXEDSTRANDS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMIXEDSTRANDS_IDNAME, 
        const wxString& caption = SYMBOL_CMIXEDSTRANDS_TITLE, 
        const wxPoint& pos = SYMBOL_CMIXEDSTRANDS_POSITION, 
        const wxSize& size = SYMBOL_CMIXEDSTRANDS_SIZE, 
        long style = SYMBOL_CMIXEDSTRANDS_STYLE );

    /// Destructor
    ~CMixedStrands();

    /// @name CJobAdapter::IJobCallback interface implementation
    /// @{
    virtual void OnJobResult(CObject* result, CJobAdapter& adapter);
    virtual void OnJobFailed(const string&, CJobAdapter& adapter);
    /// @}

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMixedStrands event handler declarations

////@end CMixedStrands event handler declarations

////@begin CMixedStrands member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMixedStrands member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();

    void OnSequenceSelected(wxCommandEvent& event);
    void Report();

    /// Blasts two nucleotide sequences using the BLAST best hits algorithm
    static TAlignVector s_Blast2NASeq(const objects::CBioseq_Handle& sh, const objects::CBioseq_Handle& qh);
    static TAlignVector s_GetAlignVector(const blast::TSeqAlignVector& seqaligns);

private:
    void x_InitDialog();
    void x_AddSequenceIds();

////@begin CMixedStrands member variables
    wxStaticText* m_Description;
    wxChoice* m_Sequences;
    wxCheckBox* m_RevFeatChkbox;
#if defined(__WXMSW__)
    wxGenericCollapsiblePane* m_ColPane;
#else
    wxCollapsiblePane* m_ColPane;
#endif

    /// Control identifiers
    enum {
        ID_CMIXEDSTRANDS = 10000,
        ID_COLLAPSIBLEPANE = 10003,
        ID_CHOICE = 10001,
        ID_CHECKBOX1 = 10004
    };
////@end CMixedStrands member variables

    objects::CSeq_entry_Handle m_TopSeqEntry;
    vector<objects::CBioseq_Handle> m_BshList;
    CRef<CJobAdapter> m_JobAdapter;
    string m_Report;
};

END_NCBI_SCOPE

#endif
    // _MIXEDSTRANDS_H_
