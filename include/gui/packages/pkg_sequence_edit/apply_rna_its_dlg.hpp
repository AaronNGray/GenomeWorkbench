/*  $Id: apply_rna_its_dlg.hpp 42821 2019-04-18 19:32:56Z joukovv $
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
#ifndef _APPLY_RNA_ITS_DLG_H_
#define _APPLY_RNA_ITS_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <objtools/alnmgr/aln_stats.hpp>
#include <objtools/alnmgr/alnmap.hpp>
#include <objtools/alnmgr/pairwise_aln.hpp>
#include <gui/objutils/reg_settings.hpp>

class wxCheckBox;
class wxChoice;
class wxTextCtrl;
class wxRadioBox;
class wxButton;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CAPPLYRNAITSDLG 10307
#define ID_CHECKBOX7 10308
#define ID_CHECKBOX8 10309
#define ID_CHECKBOX9 10310
#define ID_RADIOBOX_ITS_IGS 10311
#define ID_CHOICE_FIRST 10312
#define ID_TEXTCTRL_FFROM 10313
#define ID_TEXTCTRL_FTO 10314
#define ID_CHOICE_SECOND 10315
#define ID_TEXTCTRL_SFROM 10316
#define ID_TEXTCTRL_STO 10317
#define ID_CHOICE_THIRD 10318
#define ID_TEXTCTRL_TFROM 10319
#define ID_TEXTCTRL_TTO 10320
#define ID_CHOICE_FOURTH 10321
#define ID_TEXTCTRL_FOFROM 10313
#define ID_TEXTCTRL_FOTO 10323
#define ID_CHOICE_FIFTH 10324
#define ID_TEXTCTRL_FIFROM 10325
#define ID_TEXTCTRL_FITO 10326
#define ID_LEAVE_DIALOG_UP 10327
#define ID_APPLY 10328
#define ID_CLOSEBTN 10329
#define SYMBOL_CAPPLYRNAITSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CAPPLYRNAITSDLG_TITLE _("Apply rRNA_ITS")
#define SYMBOL_CAPPLYRNAITSDLG_IDNAME ID_CAPPLYRNAITSDLG
#define SYMBOL_CAPPLYRNAITSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CAPPLYRNAITSDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE
/*!
 * CApplyRNAITSDlg class declaration
 */

class CApplyRNAITSDlg: public CBulkCmdDlg, public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CApplyRNAITSDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CApplyRNAITSDlg();
    CApplyRNAITSDlg( wxWindow* parent, IWorkbench* wb, 
        wxWindowID id = SYMBOL_CAPPLYRNAITSDLG_IDNAME, 
        const wxString& caption = SYMBOL_CAPPLYRNAITSDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CAPPLYRNAITSDLG_POSITION, 
        const wxSize& size = SYMBOL_CAPPLYRNAITSDLG_SIZE, 
        long style = SYMBOL_CAPPLYRNAITSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAPPLYRNAITSDLG_IDNAME, 
        const wxString& caption = SYMBOL_CAPPLYRNAITSDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CAPPLYRNAITSDLG_POSITION, 
        const wxSize& size = SYMBOL_CAPPLYRNAITSDLG_SIZE, 
        long style = SYMBOL_CAPPLYRNAITSDLG_STYLE );

    /// Destructor
    ~CApplyRNAITSDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    
    typedef vector<CConstRef<CSeq_feat> > TVecConstSeqFeat;
    typedef vector<CRef<CSeq_feat> > TVecSeqFeat;
    typedef vector<CConstRef<CSeq_id> > TVecConstSeqId;

    enum ERNA_ITSType {
        eRNA_ITS_unknown,
        eRNA_ITS_18S,
        eRNA_ITS_small,
        eRNA_ITS_18S_small,
        eRNA_ITS_Its1,
        eRNA_ITS_58S,
        eRNA_ITS_Its2,
        eRNA_ITS_28S,
        eRNA_ITS_26S,
        eRNA_ITS_25S,
        eRNA_ITS_large,
        eRNA_ITS_28S_large,
        eRNA_ITS_26S_large,
        eRNA_ITS_25S_large
    };  
    
    enum ERNA_IGSType {
        eRNA_IGS_unknown,
        eRNA_IGS_16S,
        eRNA_IGS_16S_23S_Igs,
        eRNA_IGS_23S
    };
    
    enum EChoiceSet {
        eChoice_empty,       /* it is either eRNA_ITS_unknown, or eRNA_IGS_unknown */
        eChoice_not_empty    /* otherwise */
    };


////@begin CApplyRNAITSDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CApplyRNAITSDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    
    static const string& GetRNA_ITSName( ERNA_ITSType field_type );
    static const string& GetRNA_IGSName( ERNA_IGSType field_type );
    
    static ERNA_ITSType GetRNA_ITSTypeFromName ( const string& label );
    static ERNA_IGSType GetRNA_IGSTypeFromName ( const string& label );
    
    ERNA_ITSType GetNextRNA_ITSChoice( ERNA_ITSType field_type);
    ERNA_IGSType GetNextRNA_IGSChoice( ERNA_IGSType field_type);
    
    // event handler functions
    void OnFirstChoiceSelected( wxCommandEvent& event );
    void OnAnyOtherChoiceSelected (wxCommandEvent& event );
    void OnITSRadioBoxSelected( wxCommandEvent& event );
    void OnCtrlFromKillFocus( wxFocusEvent& event );
    void OnCtrlToKillFocus( wxFocusEvent& event );
    void OnCloseWindow ( wxCloseEvent& event );
    void OnGoClick ( wxCommandEvent& event );
    void OnCancelClick ( wxCommandEvent& event );
            
    /// CBulkCmdDlg pure virtual functions
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

    // IRegSettings interface implementation
    virtual void SetRegistryPath( const string& reg_path );
    virtual void LoadSettings();
    virtual void SaveSettings() const;

    void Apply_RNAITS_ToBioseq( const CBioseq_Handle& bsh, CRef<CCmdComposite> composite, bool& modified, bool create_general_only );
    void Apply_RNAITS_ToBioseqSet( CSeq_entry_Handle entry, CRef<CCmdComposite> composite, bool& modified );
    void Apply_RNAITS_ToTSE( CRef<CCmdComposite> composite, bool& modified );

    void AddFeatures (const CBioseq_Handle& bsh, TVecConstSeqFeat& featlist, CRef<CCmdComposite> composite, bool& modified );
    void Propagate_RNAITSList( const CBioseq_Handle& bsh, TVecConstSeqFeat& featlist, CRef<CCmdComposite> composite, bool& modified, bool create_general_only );
    
    TVecConstSeqFeat GetRNAITSList_SeqCoordFromDlg( const CBioseq_Handle& bsh );
    TVecConstSeqFeat GetRNAITSList_AlignCoordFromDlg( const CBioseq_Handle& bsh );
    TVecConstSeqFeat ObtainRNAITSList( const CBioseq_Handle& bsh );

////@begin CApplyRNAITSDlg member variables
    wxCheckBox* m_5partial;
    wxCheckBox* m_3partial;
    wxCheckBox* m_AlignCoords;
    wxRadioBox* m_ITSRadiobox;
    wxCheckBox* m_LeaveUp;
    wxButton* m_OK;
    wxButton* m_Cancel;
////@end CApplyRNAITSDlg member variables

private:
    void x_Set_ITSArray ( wxArrayString& Its_tags );
    void x_Set_IGSArray ( wxArrayString& Igs_tags );
    /// if required, sets the first and last features incomplete
    TVecConstSeqFeat x_UpdatePartials(TVecSeqFeat& featlist);
    void x_SetupAlnMgr( void );
    bool x_IsThereAlignment( void );

    IAlnExplorer::TNumrow x_GetRow(const CBioseq& bioseq);
    /// returns the number of dense-seg type alignment which the bioseq belongs to
    SIZE_TYPE x_BioseqInAlignments ( const CBioseq_Handle& bsh );
    /// obtains all dense-seg type alignments where this bioseq is present, in this seq-entry
    vector<CConstRef<CSeq_align> > x_GetAlignments( const CBioseq_Handle& bsh );
    /// obtains a vector of seqIds aligned with bsh through the given alignment - used for feature propagation
    TVecConstSeqId x_SeqIdsAlignedWith( const CBioseq_Handle& bsh, const CSeq_align& align );
    /// returns true if features were already propagated to the sequence 
    bool x_WasPropagatedTo(const CBioseq_Handle& bsh);

    void x_SaveAndDestroyFrame();

private:    
    struct SRNAITSChoice
    {
        wxChoice*       rnaType; 
        wxTextCtrl*     from;
        wxTextCtrl*     to;
        EChoiceSet      isEmpty;
    };
    vector<SRNAITSChoice>   m_ChoiceList;
    int                     m_Choice;
    string                  m_RegPath;
    CRef<CAlnMap>           m_Alnmap;
    // vector of propagated seqids
    TVecConstSeqId          m_PropSeqids; 
};

END_NCBI_SCOPE

#endif
        // _APPLY_RNA_ITS_DLG_H_
