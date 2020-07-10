#ifndef GUI_PKG_SEQ_EDIT___SUBMISSION_WIZARD__HPP
#define GUI_PKG_SEQ_EDIT___SUBMISSION_WIZARD__HPP
/*  $Id: submission_wizard.hpp 43610 2019-08-08 16:25:07Z filippov $
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

#include <corelib/ncbistd.hpp>
#include <gui/utils/event_handler.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/event.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>

#include <objects/seqfeat/BioSource.hpp>

#include <wx/frame.h>
#include <wx/bookctrl.h>

class wxNotebook;
class wxButton;
class wxBoxSizer;

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_submit;
    class CSubmit_block;
END_SCOPE(objects);

class CSubmitterPanel;
class CGeneralPanel;
class CGenomeInfoPanel;
class COrganismInfoPanel;
class CSubMolinfoPanel;
class CSubAnnotationPanel;
class CReferencePanel;
class CSubValidateDiscPanel;

////@begin control identifiers
#define SYMBOL_CSUBMISSIONWIZARD_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CSUBMISSIONWIZARD_TITLE _("Submission Wizard")
#define SYMBOL_CSUBMISSIONWIZARD_IDNAME ID_CSUBMISSIONWIZARD
#define SYMBOL_CSUBMISSIONWIZARD_SIZE wxSize(650, 548)
#define SYMBOL_CSUBMISSIONWIZARD_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubmissionWizard class declaration
 */

class CSubmissionWizard: public wxFrame, public CEventHandler, public CSubmissionPageChangeListener
{    
    DECLARE_DYNAMIC_CLASS( CSubmissionWizard )
    DECLARE_EVENT_TABLE()

    DECLARE_EVENT_MAP();

public:
    /// Constructors
    CSubmissionWizard();
    CSubmissionWizard( wxWindow* parent,
                       IWorkbench* wb, CRef<CGBProjectHandle> ph, CRef<CProjectItem> pi,                      
                       wxWindowID id = SYMBOL_CSUBMISSIONWIZARD_IDNAME,
                       const wxString& caption = SYMBOL_CSUBMISSIONWIZARD_TITLE,
                       const wxPoint& pos = SYMBOL_CSUBMISSIONWIZARD_POSITION,
                       const wxSize& size = SYMBOL_CSUBMISSIONWIZARD_SIZE,
                       long style = SYMBOL_CSUBMISSIONWIZARD_STYLE );

    static CSubmissionWizard* GetInstance( wxWindow* parent,
                                           IWorkbench* wb, CRef<CGBProjectHandle> ph, CRef<CProjectItem> pi,                             
                                           wxWindowID id = SYMBOL_CSUBMISSIONWIZARD_IDNAME,
                                           const wxString& caption = SYMBOL_CSUBMISSIONWIZARD_TITLE,
                                           const wxPoint& pos = SYMBOL_CSUBMISSIONWIZARD_POSITION,
                                           const wxSize& size = SYMBOL_CSUBMISSIONWIZARD_SIZE,
                                           long style = SYMBOL_CSUBMISSIONWIZARD_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBMISSIONWIZARD_IDNAME,
        const wxString& caption = SYMBOL_CSUBMISSIONWIZARD_TITLE,
        const wxPoint& pos = SYMBOL_CSUBMISSIONWIZARD_POSITION,
        const wxSize& size = SYMBOL_CSUBMISSIONWIZARD_SIZE,
        long style = SYMBOL_CSUBMISSIONWIZARD_STYLE );

    /// Destructor
    ~CSubmissionWizard();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    virtual bool TransferDataToWindow();

////@begin CSubmissionWizard event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBMISSIONBACKBTN
    void OnBackbtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBMISSIONCONTBTN
    void OnContinuebtnClick( wxCommandEvent& event );

    void OnPageChanged(wxBookCtrlEvent& event);
    void OnPageChanging(wxBookCtrlEvent& event);

    ////@end CSubmissionWizard event handler declarations

////@begin CSubmissionWizard member function declarations

    virtual void UpdateOnPageChange(wxWindow* current_page);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmissionWizard member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void ApplySubmitBlock(const objects::CSubmit_block& block);
    void ExtractDescriptor(const objects::CSeqdesc& desc, bool first_pub);
    void ImportTemplate( wxCommandEvent& event );
    void ExportTemplate( wxCommandEvent& event );
    void OnHelp( wxHyperlinkEvent& event );
    void ApplyCommand();
    virtual void ApplySubmitCommand();
    void RaiseWindow();
    void EndBusy();
    void BeginBusy();

    objects::CSeq_entry_Handle GetSeh() { return m_Seh; }

private:
    void x_SetInitialState();
    bool x_ValidateCurrentPage();
    void x_StepForward();
    void x_StepBackward();
    void x_InitializeSubmitBlock();
    void x_InitializeDescriptorList();
    void x_UpdateSubmitBlock();
    void x_SetTool();
    void x_SetSubmissionDate();
    void x_UpdateDescriptors();
    void x_ExtractDescriptorsFromSeqEntry(const objects::CSeq_entry& entry, bool& first_ref_pub);
    void x_ExtractDescriptors(const objects::CSeq_descr& descr, bool& first_ref_pub);

    // for applying descriptors from template to record
    bool x_ApplyDescriptorToRecord(const objects::CSeqdesc& desc, CCmdComposite& cmd);
    bool x_ApplySourceDescriptorToRecord(const objects::CBioSource& src, CCmdComposite& cmd);
    void x_TransferImportedSubSources(const objects::CBioSource& src, objects::CBioSource& dst);
    bool x_ApplyMolinfoToRecords(const objects::CSeqdesc& desc, CCmdComposite& cmd);
    bool x_ApplyOrReplaceDescriptorToRecord(const objects::CSeqdesc& desc, CCmdComposite& cmd, objects::CSeq_inst::TMol mol);
    bool x_ApplyUserDescriptorToRecord(const objects::CUser_object& user, bool add_to_set, CCmdComposite& cmd);
    bool x_MergeDBLink(const objects::CUser_object& template_user, objects::CUser_object& existing_user);
    void x_LoadProjectItemData();
    void x_UpdateSeqEntryHandle();
    void x_SetupCmdProcessor();

    void ConnectListener(void);
    void DisconnectListener(void);
    void OnDataChanged(CEvent *evt);
    bool SaveFile();
    bool ReportMissingFields();

////@begin CSubmissionWizard member variables
    CRef<CGBProjectHandle> m_ProjectHandle;
    CRef<CProjectItem> m_ProjectItem;

    wxNotebook* m_Notebook;
    CSubmitterPanel* m_SubmitterPanel;
    CGeneralPanel* m_GeneralPanel;
    CGenomeInfoPanel* m_GenomePanel;
    COrganismInfoPanel* m_OrgPanel;
    CSubMolinfoPanel* m_MoleculePanel;
    CSubAnnotationPanel* m_AnnotationPanel;
    CReferencePanel* m_ReferencePanel;
    CSubValidateDiscPanel* m_ValidatePanel;
    wxButton* m_Backbtn;
    wxButton* m_ContinueBtn;
    int m_Width{ 0 }, m_Height{ 0 };
    wxBoxSizer* m_BottomSizer;
////@end CSubmissionWizard member variables
    objects::CSeq_entry_Handle m_Seh;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    CRef<objects::CSubmit_block> m_SubmitBlock;

    vector<CRef<objects::CSeqdesc> > m_Descriptors;
    string m_AssemblyPrefix;

    // for importing/exporting templates
    wxString m_WorkDir;
    static CSubmissionWizard *m_Instance;
    IWorkbench* m_Workbench;
    ICommandProccessor* m_CmdProcessor;
    bool m_IsBusy;
public:
    /// Control identifiers
    enum {
        ID_CSUBMISSIONWIZARD = 10590,
        ID_CSUBMISSIONNOTEBOOK,
        ID_CSUBMISSIONBACKBTN,
        ID_CSUBMISSIONCONTBTN,
        ID_SUBMITTER_EXPORT,
        ID_SUBMITTER_IMPORT
    };
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SUBMISSION_WIZARD__HPP
