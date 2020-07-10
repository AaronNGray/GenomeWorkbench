/*  $Id: update_multi_seq_dlg.hpp 42167 2019-01-08 17:17:20Z filippov $
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
 */
#ifndef _UPDATE_MULTI_SEQ_DLG_H_
#define _UPDATE_MULTI_SEQ_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/utils/job_future.hpp>

#include <gui/packages/pkg_sequence_edit/update_seq_params.hpp>
#include <gui/widgets/wx/dialog.hpp>

class wxListBox;
class wxButton;
class wxNotebook;

BEGIN_NCBI_SCOPE

class CUpdateSeqPanel;
class CUpdateMultipleSeq_Input;
class CCmdComposite;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CUPDATEMULTISEQ_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CUPDATEMULTISEQ_DLG_TITLE _("Multiple Sequence Update")
#define SYMBOL_CUPDATEMULTISEQ_DLG_IDNAME ID_CUPDATEMULTISEQ_DLG
#define SYMBOL_CUPDATEMULTISEQ_DLG_SIZE wxSize(400, 300)
#define SYMBOL_CUPDATEMULTISEQ_DLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CUpdateMultiSeq_Dlg class declaration
 */

class CUpdateMultiSeq_Dlg : public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CUpdateMultiSeq_Dlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CUpdateMultiSeq_Dlg();
    CUpdateMultiSeq_Dlg( wxWindow* parent,
        CUpdateMultipleSeq_Input& updmultiseq_in,
        CRef<CCmdComposite> cmd,
        wxWindowID id = SYMBOL_CUPDATEMULTISEQ_DLG_IDNAME, 
        const wxString& caption = SYMBOL_CUPDATEMULTISEQ_DLG_TITLE, 
        const wxPoint& pos = SYMBOL_CUPDATEMULTISEQ_DLG_POSITION, 
        const wxSize& size = SYMBOL_CUPDATEMULTISEQ_DLG_SIZE, 
        long style = SYMBOL_CUPDATEMULTISEQ_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CUPDATEMULTISEQ_DLG_IDNAME, 
        const wxString& caption = SYMBOL_CUPDATEMULTISEQ_DLG_TITLE, 
        const wxPoint& pos = SYMBOL_CUPDATEMULTISEQ_DLG_POSITION, 
        const wxSize& size = SYMBOL_CUPDATEMULTISEQ_DLG_SIZE, 
        long style = SYMBOL_CUPDATEMULTISEQ_DLG_STYLE );

    /// Destructor
    ~CUpdateMultiSeq_Dlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CUpdateMultiSeq_Dlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_CUPDATEMULTISEQ_DLG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_UPDMULTSEQ_PNL1_LISTBOX1
    void OnNonIdentSeqSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_UPDMULTSEQ_PNL1_LISTBOX2
    void OnIdentSeqSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDMULTSEQ_PNL1_BTN
    void OnRemoveIdenticalsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDMULTSEQ_PNL2_BTN1
    void OnLoadMapClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDMULTSEQ_PNL2_BTN2
    void OnMapSelectionClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDMULTSEQ_OKTHIS
    void OnUpdateThisSequenceClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDMULTSEQ_SKIP
    void OnSkipThisSequenceClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDMULTSEQ_OKALL
    void OnUpdateAllSequenceClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_UPDMULTSEQ_STOP
    void OnStopUpdatingClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

    void OnRemoveIdenticalUpdate(wxUpdateUIEvent& event);

    void OnMapSelectionUpdate(wxUpdateUIEvent& event);

////@end CUpdateMultiSeq_Dlg event handler declarations

    void OnIdle(wxIdleEvent& event);

////@begin CUpdateMultiSeq_Dlg member function declarations

    static void s_ReportIdenticalPairs(const CUpdateMultipleSeq_Input& input);

    void ReportStats() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CUpdateMultiSeq_Dlg member function declarations

    virtual void x_LoadSettings(const CRegistryReadView& view);
    virtual void x_SaveSettings(CRegistryWriteView view) const;

    /// Should we show tooltips?
    static bool ShowToolTips();

    static string s_GetSeqIDLabel(const objects::CSeq_id_Handle& idh);

    const string& GetRevCompReport() const { return m_RevCompReport; }
    bool AnyUpdated() const { return (m_Updated > 0); }

private:
    void x_CreatePairwiseAlignments();
    void x_LaunchJobs(const CUpdateMultipleSeq_Input::TIDToUpdInputMap& updates);
    void x_GenerateAlignments(const CUpdateMultipleSeq_Input::TOldToUpdateMap& update_pairs);
    void x_CreateAlignmentForIdenticalUpdates();

    void x_FillNonIdenticalList();
    void x_FillIdenticalList();
    void x_FillNoUpdateList();
    void x_FillUnmatchedList();

    void x_SetUp();
    string x_GetCurrentSelection();

    void x_UpdateAlignmentPanel(const string& seq_name);
    // skip the selected row in sel_list and update sel_list
    // if sel_list is empty, update other_list
    void x_SkipSelected(wxListBox& sel_list, unsigned int selected, wxListBox* other_list);
    CRef<CCmdComposite> x_UpdateSelected(const string& seq_name, bool create_general_only);

    void x_UpdateNotebookAfterMapping(void);

    void x_CloseDialog();

    typedef map<string, objects::CSeq_id_Handle> TNameToIDHandleMap;

    CUpdateSeqPanel* m_UpdSeqPanel;
    CUpdateMultipleSeq_Input* m_UpdMultiSeqInput;
    TNameToIDHandleMap m_SeqList;

    TNameToIDHandleMap m_UnmatchedOldSeqs; // list of old sequences that could not be matched
    TNameToIDHandleMap m_UnmatchedUpdSeqs; // list of update sequences that could not be mapped to an old sequence

    CRef<CCmdComposite> m_UpdateCmd;

    typedef tuple<objects::CSeq_id_Handle, CConstRef<objects::CSeq_align> >  TAlignResult;
    list<job_future<TAlignResult> > m_Futures;

    set<objects::CSeq_id_Handle> m_Running;

    CStopWatch m_StopWatch;

    enum {
        ID_CUPDATEMULTISEQ_DLG = 10900,
        ID_NOTEBOOK = 10901,
        ID_PANEL = 10902,
        ID_UPDMULTSEQ_PNL1_LISTBOX1 = 10903,
        ID_UPDMULTSEQ_PNL1_LISTBOX2 = 10904,
        ID_UPDMULTSEQ_PNL1_BTN = 10905,
        ID_PANEL1 = 10906,
        ID_UPDMULTSEQ_PNL2_LISTBOX1 = 10907,
        ID_UPDMULTSEQ_PNL2_LISTBOX2 = 10908,
        ID_UPDMULTSEQ_PNL2_BTN1 = 10909,
        ID_UPDMULTSEQ_PNL2_BTN2 = 10910,
        ID_FOREIGN = 10911,
        ID_UPDMULTSEQ_OKTHIS = 10912,
        ID_UPDMULTSEQ_SKIP = 10913,
        ID_UPDMULTSEQ_OKALL = 10914,
        ID_UPDMULTSEQ_STOP = 10915
    };

    wxNotebook* m_Notebook;
    wxListBox* m_NonIdenticalList;  // list of old sequences with nonidentical updates
    wxListBox* m_IdenticalList;     // list of old sequences with identical updates
    wxListBox* m_NoUpdateList;      // list of old sequences without updates
    wxListBox* m_UnmatchedList;     // list of update sequences that were not matched to any sequence in the seq-entry
    
    wxButton* m_IdenticalUpdBtn;
    wxButton* m_LoadMap;
    wxButton* m_MapSelection;

    unsigned int m_Updated;
    unsigned int m_Skipped;
    string m_RevCompReport;
};

END_NCBI_SCOPE

#endif
    // _UPDATE_MULTI_SEQ_DLG_H_
