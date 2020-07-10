/*  $Id: alignment_assistant.hpp 42300 2019-01-29 19:33:00Z filippov $
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
#ifndef _ALIGNMENT_ASSISTANT_H_
#define _ALIGNMENT_ASSISTANT_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_align_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/widgets/edit/gbqual_panel.hpp>
#include <gui/widgets/edit/paint_alignment.hpp>

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
#include <wx/combobox.h>
#include <wx/listbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CALIGNMENT_ASSISTANT wxID_ANY
#define SYMBOL_CALIGNMENT_ASSISTANT_STYLE wxMINIMIZE_BOX | wxMAXIMIZE_BOX |wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CALIGNMENT_ASSISTANT_TITLE _("Alignment Assistant")
#define SYMBOL_CALIGNMENT_ASSISTANT_IDNAME ID_CALIGNMENT_ASSISTANT
#define SYMBOL_CALIGNMENT_ASSISTANT_SIZE wxSize(775,595)
#define SYMBOL_CALIGNMENT_ASSISTANT_POSITION wxDefaultPosition
////@end control identifiers
#define ID_CLOSE_ALIGNMENT_ASSISTANT_BUTTON 11501
#define ID_GOTO_ALIGNMENT_ASSISTANT_BUTTON   11502
#define ID_GOTO_SEQ_ALIGNMENT_ASSISTANT_BUTTON   11503
#define ID_EDIT_RM_SEQ_ALIGNMENT_ASSISTANT_MENU 11504
#define ID_EDIT_REV_STRANDS_ALIGNMENT_ASSISTANT_MENU 11505
#define ID_EDIT_VALIDATE_ALIGNMENT_ASSISTANT_MENU 11506
#define ID_EDIT_PROPAGATE_ALIGNMENT_ASSISTANT_MENU 11507
#define ID_EXPORT_INTER_ALIGNMENT_ASSISTANT_MENU 11508
#define ID_EXPORT_CONT_ALIGNMENT_ASSISTANT_MENU 11509
#define ID_EXPORT_TEXT_ALIGNMENT_ASSISTANT_MENU 11510
#define ID_SHOW_SUBSTITUTE_ALIGNMENT_ASSISTANT_MENU 11511
#define ID_SHOW_FEATURES_ALIGNMENT_ASSISTANT_MENU 11512
#define ID_ADD_ALIGN_CDS_ALIGNMENT_ASSISTANT_MENU 11513
#define ID_ADD_ALIGN_RNA_ALIGNMENT_ASSISTANT_MENU 11514
#define ID_ADD_ALIGN_OTHER_ALIGNMENT_ASSISTANT_MENU 11515

class NCBI_GUIWIDGETS_EDIT_EXPORT CAlignmentAssistant : public wxFrame, public CEventHandler
{    
    DECLARE_DYNAMIC_CLASS( CAlignmentAssistant )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAlignmentAssistant();

    CAlignmentAssistant( wxWindow* parent, CSeq_entry_Handle seh, ICommandProccessor* processor, CConstRef<CSeq_align> align = CConstRef<CSeq_align>(NULL), wxWindowID id = SYMBOL_CALIGNMENT_ASSISTANT_IDNAME, const wxString& caption = SYMBOL_CALIGNMENT_ASSISTANT_TITLE, const wxPoint& pos = SYMBOL_CALIGNMENT_ASSISTANT_POSITION, const wxSize& size = SYMBOL_CALIGNMENT_ASSISTANT_SIZE, long style = SYMBOL_CALIGNMENT_ASSISTANT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNMENT_ASSISTANT_IDNAME, const wxString& caption = SYMBOL_CALIGNMENT_ASSISTANT_TITLE, const wxPoint& pos = SYMBOL_CALIGNMENT_ASSISTANT_POSITION, const wxSize& size = SYMBOL_CALIGNMENT_ASSISTANT_SIZE, long style = SYMBOL_CALIGNMENT_ASSISTANT_STYLE );

    /// Destructor
    virtual ~CAlignmentAssistant();

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

    void OnClose( wxCommandEvent& event );
    void OnGoTo( wxCommandEvent& event );
    void OnGoToSeq( wxCommandEvent& event );
    void OnFeatures( wxCommandEvent& event );
    void OnSubstitute( wxCommandEvent& event );
    void ReportPos(int pos, const string &label);
    void ReportRange(int pos1, int pos2, const string &label);
    void IsSelectionAndClean(wxUpdateUIEvent &event);
    void IsNucleotide(wxUpdateUIEvent &event);
    void CreateFeature ( wxCommandEvent& event);
    void OnTarget( wxCommandEvent& event);
    void OnExportInter( wxCommandEvent& event);
    void OnExportCont( wxCommandEvent& event);
    void OnValidate( wxCommandEvent& event);
    void OnPropagateFeatures(wxCommandEvent& /*event*/);
    void OnRemoveSeqFromAlign(wxCommandEvent& /*event*/);
    void OnReverseStrand(wxCommandEvent& /*event*/);
    void OnAddAlignCDS(wxCommandEvent& /*event*/);
    void OnAddAlignRNA(wxCommandEvent& /*event*/);
    void OnAddAlignOther(wxCommandEvent& /*event*/);
    void TranslateLocations(CRef<CSeq_loc> loc, vector<pair<CRef<CSeq_loc>, CBioseq_Handle> > &locations);
    void DeleteSelection();
private:
    void GetFeatures();
    void GetAlignment(CConstRef<CSeq_align> align);
    bool x_ExecuteCommand(IEditCommand* command);
    void ReportUnresolvedBioseqs(void);
    void AdjustBioseq(CBioseq_Handle bsh, int from, int to, CRef<CCmdComposite> cmd);
    CRef<CSeq_align> AdjustAlign(int from, int to, const vector<int> &offset, CRef<CCmdComposite> cmd);
    void AdjustFeatureLocations(CBioseq_Handle bsh, int from, int to, CRef<CCmdComposite> cmd);
    CRef<CSeq_loc> AdjustLoc(const CSeq_loc& feat_loc, int from, int to, bool &modified, bool &removed);
    //void AdjustFrame(CCdregion &feat, int diff);
    void TrimQualityScores(CBioseq_Handle bsh, int from, int to, CRef<CCmdComposite> command);

    CSeq_entry_Handle m_TopSeqEntry;
    CSeq_align_Handle m_Alignment;
    ICommandProccessor* m_CmdProcessor;
    CPaintAlignment *m_Panel;

    vector<vector<vector<pair<TSeqPos,TSeqPos> > > > m_FeatRanges;
    vector<vector<pair<string,CSeqFeatData::ESubtype> > > m_FeatTypes;
    vector<vector<CBioseq_Handle::EVectorStrand> > m_FeatStrand;

    wxStaticText *m_Range;
    wxTextCtrl* m_GoTo_TextCtrl;
    wxTextCtrl* m_GoToSeq_TextCtrl;
    bool m_is_nucleotide;
    vector<string> m_Labels;
    wxMenu *m_target_submenu;
};



/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CALIGNCDSADDPANEL wxID_ANY
#define ID_ALIGNCDSADD_PROTEIN_NAME wxID_ANY
#define ID_ALIGNCDSADD_PROTEIN_DESC wxID_ANY
#define ID_ALIGNCDSADD_GENE_NAME wxID_ANY
#define ID_ALIGNCDSADD_GENE_DESC wxID_ANY
#define ID_ALIGNCDSADD_COMMENT wxID_ANY
#define ID_WINDOW wxID_ANY
#define SYMBOL_CALIGNCDSADDPANEL_STYLE  wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CALIGNCDSADDPANEL_TITLE _("Add CDS to Alignment")
#define SYMBOL_CALIGNCDSADDPANEL_IDNAME ID_CALIGNCDSADDPANEL
#define SYMBOL_CALIGNCDSADDPANEL_SIZE wxDefaultSize
#define SYMBOL_CALIGNCDSADDPANEL_POSITION wxDefaultPosition
////@end control identifiers

class CLocationPanel;

/*!
 * CAlignCDSAddPanel class declaration
 */

class CAlignCDSAddPanel: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CAlignCDSAddPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAlignCDSAddPanel();
    CAlignCDSAddPanel( wxWindow* parent,  CRef<CSeq_loc> align_loc, CScope &scope,
                       wxWindowID id = SYMBOL_CALIGNCDSADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGNCDSADDPANEL_POSITION, const wxSize& size = SYMBOL_CALIGNCDSADDPANEL_SIZE, long style = SYMBOL_CALIGNCDSADDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNCDSADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGNCDSADDPANEL_POSITION, const wxSize& size = SYMBOL_CALIGNCDSADDPANEL_SIZE, long style = SYMBOL_CALIGNCDSADDPANEL_STYLE );

    /// Destructor
    ~CAlignCDSAddPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAlignCDSAddPanel event handler declarations

////@end CAlignCDSAddPanel event handler declarations

////@begin CAlignCDSAddPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAlignCDSAddPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
    void AddOneCommand(const objects::CBioseq_Handle& bsh, CRef<CSeq_loc> seq_loc, CRef<CCmdComposite> cmd);
private:
////@begin CAlignCDSAddPanel member variables
    wxTextCtrl* m_ProteinName;
    wxTextCtrl* m_ProteinDescription;
    wxTextCtrl* m_GeneSymbol;
    wxTextCtrl* m_GeneDescription;
    wxTextCtrl* m_Comment;
    CLocationPanel* m_Location;
////@end CAlignCDSAddPanel member variables
    CRef<CSeq_loc> m_Loc;
    CRef<CScope> m_scope;
    bool m_create_general_only;
};



/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CALIGN_RNA_ADD_PANEL wxID_ANY
#define ID_CALIGN_RNA_ADD_PANEL_CHOICE1 11601
#define ID_CALIGN_RNA_ADD_PANEL_COMBOBOX wxID_ANY
#define ID_CALIGN_RNA_ADD_PANEL_TEXTCTRL10 wxID_ANY
#define ID_CALIGN_RNA_ADD_PANEL_BUTTON11 11602
#define ID_CALIGN_RNA_ADD_PANEL_BUTTON12 11603
#define ID_CALIGN_RNA_ADD_PANEL_WINDOW1 wxID_ANY
#define SYMBOL_CALIGN_RNA_ADD_PANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CALIGN_RNA_ADD_PANEL_TITLE _("Add RNA to Alignment")
#define SYMBOL_CALIGN_RNA_ADD_PANEL_IDNAME ID_CALIGN_RNA_ADD_PANEL
#define SYMBOL_CALIGN_RNA_ADD_PANEL_SIZE wxDefaultSize
#define SYMBOL_CALIGN_RNA_ADD_PANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAlignRNAAddPanel class declaration
 */

class CAlignRNAAddPanel: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CAlignRNAAddPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAlignRNAAddPanel();
    CAlignRNAAddPanel( wxWindow* parent, CRef<CSeq_loc> align_loc, CScope &scope,
                       wxWindowID id = SYMBOL_CALIGN_RNA_ADD_PANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGN_RNA_ADD_PANEL_POSITION, const wxSize& size = SYMBOL_CALIGN_RNA_ADD_PANEL_SIZE, long style = SYMBOL_CALIGN_RNA_ADD_PANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGN_RNA_ADD_PANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGN_RNA_ADD_PANEL_POSITION, const wxSize& size = SYMBOL_CALIGN_RNA_ADD_PANEL_SIZE, long style = SYMBOL_CALIGN_RNA_ADD_PANEL_STYLE );

    /// Destructor
    ~CAlignRNAAddPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAlignRNAAddPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE1
    void OnSelectRNAType( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
    void OnAdd18SToComment( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
    void OnAdd16StoComment( wxCommandEvent& event );

////@end CAlignRNAAddPanel event handler declarations

////@begin CAlignRNAAddPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAlignRNAAddPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();


    CRef<CCmdComposite> GetCommand();
    void AddOneCommand(const objects::CBioseq_Handle& bsh, CRef<CSeq_loc> seq_loc, CRef<CCmdComposite> cmd);
private:
////@begin CAlignRNAAddPanel member variables
    wxChoice* m_RNAType;
    wxComboBox* m_ncRNAClass;
    wxFlexGridSizer* m_FieldSizer;
    wxBoxSizer* m_RNANameSizer;
    wxTextCtrl* m_Comment;
    wxBoxSizer* m_ButtonsSizer;
////@end CAlignRNAAddPanel member variables
    wxChoice* m_tRNAname;
    wxTextCtrl* m_RNAname;
    wxComboBox* m_rRNAname;
    wxTextCtrl* m_Locus;
    wxTextCtrl* m_GeneDesc;
    CLocationPanel* m_Location;
    CRef<CSeq_loc> m_Loc;
    CRef<CScope> m_scope;
    void x_RemoveRNANames();
    bool x_AddrRNAName();
    bool x_AddGeneFields();
    bool x_RemoveGeneFields();
    objects::CRNA_ref::TType x_GetRnaType();
    string x_GetRnaName();
};


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CALIGN_OTHER_ADDPANEL wxID_ANY
#define ID_CALIGN_OTHER_ADDPANEL_FEATURE_TYPE_LISTBOX 11701
#define ID_CALIGN_OTHER_ADDPANEL_TEXTCTRL11 wxID_ANY
#define ID_CALIGN_OTHER_ADDPANEL_TEXTCTRL12 wxID_ANY
#define ID_CALIGN_OTHER_ADDPANEL_TEXTCTRL14 wxID_ANY
#define ID_CALIGN_OTHER_ADDPANEL_WINDOW1 wxID_ANY
#define SYMBOL_CALIGN_OTHER_ADDPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CALIGN_OTHER_ADDPANEL_TITLE _("Add Other Feature to Alignment")
#define SYMBOL_CALIGN_OTHER_ADDPANEL_IDNAME ID_CALIGN_OTHER_ADDPANEL
#define SYMBOL_CALIGN_OTHER_ADDPANEL_SIZE wxDefaultSize
#define SYMBOL_CALIGN_OTHER_ADDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAlignOtherAddPanel class declaration
 */

    class CAlignOtherAddPanel: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CAlignOtherAddPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAlignOtherAddPanel();
    CAlignOtherAddPanel( wxWindow* parent, CRef<CSeq_loc> align_loc, CSeq_entry_Handle seh, 
                         wxWindowID id = SYMBOL_CALIGN_OTHER_ADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGN_OTHER_ADDPANEL_POSITION, const wxSize& size = SYMBOL_CALIGN_OTHER_ADDPANEL_SIZE, long style = SYMBOL_CALIGN_OTHER_ADDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGN_OTHER_ADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGN_OTHER_ADDPANEL_POSITION, const wxSize& size = SYMBOL_CALIGN_OTHER_ADDPANEL_SIZE, long style = SYMBOL_CALIGN_OTHER_ADDPANEL_STYLE );

    /// Destructor
    ~CAlignOtherAddPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAlignOtherAddPanel event handler declarations

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_FEATURE_TYPE_LISTBOX
    void OnFeatureTypeListboxSelected( wxCommandEvent& event );

////@end CAlignOtherAddPanel event handler declarations

////@begin CAlignOtherAddPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAlignOtherAddPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
    void AddOneCommand(const objects::CBioseq_Handle& bsh, CRef<CSeq_loc> seq_loc, CRef<CCmdComposite> cmd);    

private:

    string m_DefaultKey;
    CGBQualPanel* m_GBQualPanel;
    CRef<objects::CSeq_feat> m_QualFeat;
////@begin CAlignOtherAddPanel member variables
    wxListBox* m_FeatureType;
    wxBoxSizer* m_GBQualSizer;
    wxTextCtrl* m_Locus;
    wxTextCtrl* m_GeneDesc;
    wxTextCtrl* m_Comment;
////@end CAlignOtherAddPanel member variables
    CLocationPanel* m_Location;
    CRef<CSeq_loc> m_Loc;
    CSeq_entry_Handle m_TopSeqEntry;
    CRef<CScope> m_scope;
    void x_ListFeatures();
    void x_ChangeFeatureType(const string& key);
    void x_AddQuals(objects::CSeq_feat& feat);
    void x_AddGeneQuals(objects::CSeq_feat& gene);
};

END_NCBI_SCOPE

#endif
    // _ALIGNMENT_ASSISTANT_H_
