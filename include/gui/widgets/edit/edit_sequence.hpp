/*  $Id: edit_sequence.hpp 44613 2020-02-04 19:08:11Z filippov $
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
#ifndef _EDIT_SEQUENCE_H_
#define _EDIT_SEQUENCE_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/widgets/edit/paint_sequence.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/textctrl.h>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/timer.h>

#include <unordered_map>
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
#define ID_CEDIT_SEQUENCE wxID_ANY
#define SYMBOL_CEDIT_SEQUENCE_STYLE wxMINIMIZE_BOX | wxMAXIMIZE_BOX |wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDIT_SEQUENCE_TITLE _("Edit Sequence")
#define SYMBOL_CEDIT_SEQUENCE_IDNAME ID_CEDIT_SEQUENCE
#define SYMBOL_CEDIT_SEQUENCE_SIZE wxSize(704, 504)
#define SYMBOL_CEDIT_SEQUENCE_POSITION wxDefaultPosition
////@end control identifiers
#define ID_CLOSE_EDSEQ_BUTTON 11401
#define ID_ACCEPT_EDSEQ_BUTTON 11402
#define ID_GOTO_EDSEQ_BUTTON   11403
#define ID_SHOW_TRANSLATE_EDSEQ_MENU 11405
#define ID_SHOW_TRANSLATE1_EDSEQ_MENU 11406
#define ID_SHOW_TRANSLATE2_EDSEQ_MENU 11407
#define ID_SHOW_TRANSLATE_ALL_EDSEQ_MENU 11408
#define ID_SHOW_ONTHEFLY_EDSEQ_MENU 11409
#define ID_SHOW_MISMATCH_EDSEQ_MENU 11410
#define ID_SHOW_REVCOMP_EDSEQ_MENU 11411
#define ID_SHOW_FEATURES_EDSEQ_MENU 11412
#define ID_EDIT_COPY_EDSEQ_MENU 11413
#define ID_EDIT_CUT_EDSEQ_MENU 11414
#define ID_EDIT_PASTE_EDSEQ_MENU 11415
#define ID_SHOW_REVTRANSLATE_EDSEQ_MENU 11416
#define ID_SHOW_REVTRANSLATE1_EDSEQ_MENU 11417
#define ID_SHOW_REVTRANSLATE2_EDSEQ_MENU 11418
#define ID_SHOW_REVTRANSLATE_ALL_EDSEQ_MENU 11419
#define ID_CANCEL_EDSEQ_BUTTON 11420
#define ID_SHOW_TRANSLATE_EVERYTHING_EDSEQ_MENU 11421
#define ID_SHOW_TRANSLATE_NONE_EDSEQ_MENU 11422
#define ID_EDIT_FIND_EDSEQ_MENU 11423
#define ID_RETRANSLATE_EDSEQ_BUTTON 11424
#define ID_RANGE_EDSEQ_BUTTON 11425
#define READONLY_TIMER_ID 11426

class NCBI_GUIWIDGETS_EDIT_EXPORT CEditSequence : public wxFrame, public CEventHandler
{    
    DECLARE_DYNAMIC_CLASS( CEditSequence )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditSequence();

    CEditSequence( wxWindow* parent, const CSeq_loc *loc, CScope &scope, ICommandProccessor* processor, int start = 0, wxWindowID id = SYMBOL_CEDIT_SEQUENCE_IDNAME, const wxString& caption = SYMBOL_CEDIT_SEQUENCE_TITLE, const wxPoint& pos = SYMBOL_CEDIT_SEQUENCE_POSITION, const wxSize& size = SYMBOL_CEDIT_SEQUENCE_SIZE, long style = SYMBOL_CEDIT_SEQUENCE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDIT_SEQUENCE_IDNAME, const wxString& caption = SYMBOL_CEDIT_SEQUENCE_TITLE, const wxPoint& pos = SYMBOL_CEDIT_SEQUENCE_POSITION, const wxSize& size = SYMBOL_CEDIT_SEQUENCE_SIZE, long style = SYMBOL_CEDIT_SEQUENCE_STYLE );

    /// Destructor
    virtual ~CEditSequence();

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

    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;

    void OnClose( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event );
    void OnAccept( wxCommandEvent& event );
    void OnGoTo( wxCommandEvent& event );
    void OnRange( wxCommandEvent& event );
    void OnFind( wxCommandEvent& event );
    void OnTranslate( wxCommandEvent& event );
    void OnTranslate1( wxCommandEvent& event );
    void OnTranslate2( wxCommandEvent& event );
    void OnTranslateAll( wxCommandEvent& event );
    void OnRevTranslate( wxCommandEvent& event );
    void OnRevTranslate1( wxCommandEvent& event );
    void OnRevTranslate2( wxCommandEvent& event );
    void OnRevTranslateAll( wxCommandEvent& event );
    void OnTranslateEverything( wxCommandEvent& event );
    void OnTranslateNone( wxCommandEvent& event );
    void OnOnTheFly( wxCommandEvent& event );
    void OnMismatch( wxCommandEvent& event );
    void OnComplement( wxCommandEvent& event );
    void OnFeatures( wxCommandEvent& event );
    void OnCopy( wxCommandEvent& event );
    void OnCut( wxCommandEvent& event );
    void OnPaste( wxCommandEvent& event );
    void GetSequence();
    void ReportPos(int pos);
    void ReportRange(int pos1, int pos2);
    void IsClipboard(wxUpdateUIEvent &event);
    void IsSelection(wxUpdateUIEvent &event);
    void IsSelectionAndClean(wxUpdateUIEvent &event);
    void IsNucleotide(wxUpdateUIEvent &event);
    void CreateFeature ( wxCommandEvent& event);
    void CreateRegulatory( wxCommandEvent& event);
    void EnableCommit(bool enable);
    void OnRetranslate ( wxCommandEvent& event);
    void ShowReadOnlyWarning();
    void OnReadOnlyTimer(wxTimerEvent& event);

private:
    void GetFeatures(const CSeq_loc &loc, CScope &scope);
    void GetProteinFeatures(CBioseq_Handle prot_bsh, int i, const CSeq_id* seq_id, const CSeq_feat &old_cds, TSeqPos seq_start);
    void AdjustFeatureLocations(CRef<CCmdComposite> composite);
    void AdjustProtFeatureLocations(CRef<CCmdComposite> composite, const CSeq_feat &old_cds, const CSeq_feat &cds, int i, const CSeq_id* seq_id, TSeqPos seq_start);
    string GetSeqTitle();
    bool x_ExecuteCommand(IEditCommand* command);

    IUndoManager* m_UndoManager;
    CPaintSequence *m_Panel;
    CSeq_entry_Handle m_seh, m_edited_seh;
    vector<vector<pair<TSeqPos,TSeqPos> > > m_FeatRanges;
    vector<pair<string,CSeqFeatData::ESubtype> > m_FeatTypes;
    unordered_map<int, vector<vector<pair<TSeqPos,TSeqPos> > > > m_ProtFeatRanges;
    vector<int> m_FeatFrames;
    vector<bool> m_Feat5Partial;
    vector< CRef<CGenetic_code> >  m_GeneticCode;
    bool m_modified;
    wxStaticText *m_Pos;
    wxStaticText *m_RangeLabel;
    wxStaticText *m_Range;
    wxTextCtrl* m_GoTo_TextCtrl;
    wxTextCtrl* m_RangeFrom;
    wxTextCtrl* m_RangeTo;
    wxMenuItem *m_translate_item, *m_translate_item1, *m_translate_item2, *m_translate_item_all;
    wxMenuItem *m_revtranslate_item, *m_revtranslate_item1, *m_revtranslate_item2, *m_revtranslate_item_all;
    wxMenuItem *m_translate_item_everything, *m_translate_item_none;
    wxButton* m_CommitButton;
    wxButton* m_RetranslateButton;
    wxStaticText* m_commit_warning;
    wxStaticText* m_readonly_warning;
    wxTimer m_readonly_timer;
    vector<string> m_RealProt;
    vector<CBioseq_Handle::EVectorStrand> m_FeatStrand;
    bool m_is_nucleotide;
    unsigned int m_NumCmds;
    const CSeq_loc *m_loc;
    CScope *m_scope;
    int m_start;
    string m_RegPath;
};



////@begin control identifiers
#define ID_CFINDEDITSEQUENCE 29000
#define ID_TEXTCTRL_FIND_EDIT_SEQ 29001
#define ID_NUC_FIND_EDIT_SEQ 29002
#define ID_REVCOMP_FIND_EDIT_SEQ 29003
#define ID_TRANSFRAME_FIND_EDIT_SEQ 29004
#define ID_FRAMECHOICE_FIND_EDIT_SEQ 29005
#define ID_PREV_FIND_EDIT_SEQ 29006
#define ID_NEXT_FIND_EDIT_SEQ 29007
#define ID_DISMISS_FIND_EDIT_SEQ 29008
#define SYMBOL_CFINDEDITSEQUENCE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFINDEDITSEQUENCE_TITLE _("find")
#define SYMBOL_CFINDEDITSEQUENCE_IDNAME ID_CFINDEDITSEQUENCE
#define SYMBOL_CFINDEDITSEQUENCE_SIZE wxSize(400, 300)
#define SYMBOL_CFINDEDITSEQUENCE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFindEditSequence class declaration
 */

class CFindEditSequence : public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CFindEditSequence )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFindEditSequence();
    CFindEditSequence( wxWindow* parent, CPaintSequence *panel, bool is_nucleotide, wxWindowID id = SYMBOL_CFINDEDITSEQUENCE_IDNAME, const wxString& caption = SYMBOL_CFINDEDITSEQUENCE_TITLE, const wxPoint& pos = SYMBOL_CFINDEDITSEQUENCE_POSITION, const wxSize& size = SYMBOL_CFINDEDITSEQUENCE_SIZE, long style = SYMBOL_CFINDEDITSEQUENCE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFINDEDITSEQUENCE_IDNAME, const wxString& caption = SYMBOL_CFINDEDITSEQUENCE_TITLE, const wxPoint& pos = SYMBOL_CFINDEDITSEQUENCE_POSITION, const wxSize& size = SYMBOL_CFINDEDITSEQUENCE_SIZE, long style = SYMBOL_CFINDEDITSEQUENCE_STYLE );

    /// Destructor
    ~CFindEditSequence();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFindEditSequence event handler declarations

////@end CFindEditSequence event handler declarations

////@begin CFindEditSequence member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFindEditSequence member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    void OnPrev( wxCommandEvent& event );
    void OnNext( wxCommandEvent& event );
    void OnDismiss( wxCommandEvent& event );
    void OnClose(wxCloseEvent& event);
    void OnChange( wxCommandEvent& event );
private:
    void FindMatchPositions(const string &choice);
    void SetHighlights();
    string GetPattern();

////@begin CFindEditSequence member variables
    wxTextCtrl* m_Pattern;
    wxStaticText* m_ItemOfText;
    wxRadioButton* m_Nuc;
    wxCheckBox* m_RevComp;
    wxRadioButton* m_TransFrame;
    wxChoice* m_FrameChoice;
    wxButton* m_Prev;
    wxButton* m_Next;
////@end CFindEditSequence member variables
    int m_Num;
    int m_Total;
    set<int> m_Pos;
    set<int>::iterator m_CurrentPos;
    CPaintSequence *m_Panel;
    bool m_is_nucleotide;
};

END_NCBI_SCOPE

#endif
    // _EDIT_SEQUENCE_H_
