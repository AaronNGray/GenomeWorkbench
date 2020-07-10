/*  $Id: submission_wizard.cpp 44057 2019-10-17 14:39:43Z asztalos $
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



#include <ncbi_pch.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <gui/objects/gbench_version.hpp>
#include <gui/objects/GBenchVersionInfo.hpp>
#include <objtools/edit/struc_comm_field.hpp>
#include <gui/widgets/edit/submitter_panel.hpp>
#include <gui/widgets/edit/general_panel.hpp>
#include <gui/widgets/edit/genomeinfo_panel.hpp>
#include <gui/packages/pkg_sequence_edit/orginfo_panel.hpp>
#include <gui/widgets/edit/seqsub_molinfo_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subannotation_panel.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <gui/widgets/edit/reference_panel.hpp>
#include <gui/packages/pkg_sequence_edit/sub_validate_disc_tab.hpp>

 // for importing/exporting templates
#include <util/format_guess.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <serial/objistr.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/MolInfo.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/label.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/core/cmd_change_projectitem.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/util_cmds.hpp>

#include <gui/core/document.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/core/project_service.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/submission_wizard.hpp>


#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/settings.h>
#include <wx/filedlg.h>
#include <wx/utils.h> 
#include <wx/filename.h>
#include <wx/msgdlg.h> 
#include <wx/cshelp.h>
#include <wx/utils.h> 
#include <wx/uri.h>
#include <wx/hyperlink.h>
BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CRaiseWindowTask : public CAppTask
{
public:
    CRaiseWindowTask(wxWindow *win) : m_Win(win) {}
protected:
    wxWindow* m_Win;
    virtual IAppTask::ETaskState x_Run()
    {
        m_Win->Raise();
        m_Win->SetFocus();
        return eCompleted;
    }
};


/*!
 * CSubmissionWizard type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CSubmissionWizard, wxFrame)


/*!
 * CSubmissionWizard event table definition
 */

BEGIN_EVENT_TABLE(CSubmissionWizard, wxFrame)
    EVT_BUTTON(ID_CSUBMISSIONCONTBTN, CSubmissionWizard::OnContinuebtnClick)
    EVT_BUTTON(ID_CSUBMISSIONBACKBTN, CSubmissionWizard::OnBackbtnClick)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_CSUBMISSIONNOTEBOOK, CSubmissionWizard::OnPageChanged)
    EVT_NOTEBOOK_PAGE_CHANGING(ID_CSUBMISSIONNOTEBOOK, CSubmissionWizard::OnPageChanging)
    EVT_BUTTON( ID_SUBMITTER_EXPORT, CSubmissionWizard::ExportTemplate )
    EVT_BUTTON( ID_SUBMITTER_IMPORT, CSubmissionWizard::ImportTemplate )
    EVT_HYPERLINK(wxID_HELP, CSubmissionWizard::OnHelp)
END_EVENT_TABLE()

BEGIN_EVENT_MAP( CSubmissionWizard, CEventHandler )
    ON_EVENT(CProjectViewEvent, CViewEvent::eProjectChanged, &CSubmissionWizard::OnDataChanged)
END_EVENT_MAP()

/*!
 * CSubmissionWizard constructors
 */

CSubmissionWizard::CSubmissionWizard()
{
    Init();
}

CSubmissionWizard::CSubmissionWizard( wxWindow* parent, IWorkbench* wb, CRef<CGBProjectHandle> ph, CRef<CProjectItem> pi,
                                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(wb), m_ProjectHandle(ph), m_ProjectItem(pi)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    ConnectListener();
    NEditingStats::ReportSubWizardUsage("open");
}

void CSubmissionWizard::x_LoadProjectItemData()
{
    m_SeqSubmit.Reset();
    m_Seh.Reset();
    if (m_ProjectItem->IsSetItem() && m_ProjectItem->GetItem().IsSubmit())
        m_SeqSubmit.Reset(&m_ProjectItem->GetItem().GetSubmit());

    CScope* scope = m_ProjectHandle->GetScope();
    if (m_SeqSubmit)
    {
        if (m_SeqSubmit->IsSetData() && m_SeqSubmit->GetData().IsEntrys())
        {
            for (auto entry : m_SeqSubmit->GetData().GetEntrys())
            {
                if (entry)
                {
                    CSeq_entry_Handle seh = scope->GetSeq_entryHandle(*entry);
                    if (seh)
                    {
                        m_Seh = seh.GetTopLevelEntry();
                        break;
                    }
                }
            }
        }            
    }
    else if (m_ProjectItem->IsSetItem() && m_ProjectItem->GetItem().IsEntry())
    {
        CSeq_entry_Handle seh = scope->GetSeq_entryHandle(m_ProjectItem->GetItem().GetEntry(), CScope::eMissing_Null);
        if (seh)
            m_Seh = seh.GetTopLevelEntry();
    }
}

void CSubmissionWizard::x_SetupCmdProcessor()
{
    CGBDocument* doc = dynamic_cast<CGBDocument*>(m_ProjectHandle.GetPointer());
    if (doc) 
    {
        m_CmdProcessor = &doc->GetUndoManager();
        m_WorkDir = doc->GetWorkDir();
    }   
}


/*!
 * CSubmissionWizard creator
 */

bool CSubmissionWizard::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmissionWizard creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY | wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmissionWizard creation
    return true;
}


/*!
 * CSubmissionWizard destructor
 */

CSubmissionWizard::~CSubmissionWizard()
{
////@begin CSubmissionWizard destruction
////@end CSubmissionWizard destruction
    DisconnectListener();
    m_Instance = NULL;
}


/*!
 * Member initialisation
 */

void CSubmissionWizard::Init()
{
    m_Notebook = NULL;
    m_Backbtn = NULL;
    m_ContinueBtn = NULL;
    m_SubmitterPanel = NULL;
    m_GeneralPanel = NULL;
    m_GenomePanel = NULL;
    m_OrgPanel = NULL;
    m_MoleculePanel = NULL;
    m_AnnotationPanel = NULL;
    m_ReferencePanel = NULL;
    m_ValidatePanel = NULL;
    m_BottomSizer = NULL;
    m_IsBusy = false;
    m_WorkDir = wxGetHomeDir();
    x_SetupCmdProcessor();
    x_LoadProjectItemData();
    x_InitializeSubmitBlock();
    x_InitializeDescriptorList();
}

/*!
 * Control creation for CSubmissionWizard
 */

void CSubmissionWizard::CreateControls()
{    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemPanel1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemPanel1, 1, wxGROW | wxALL, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxNotebook(itemPanel1, ID_CSUBMISSIONNOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT /*wxNB_TOP */);
    itemBoxSizer2->Add(m_Notebook, 1, wxGROW | wxALL, 0);
    
    m_SubmitterPanel = new CSubmitterPanel(m_Notebook, m_SubmitBlock->SetContact());
    m_Notebook->AddPage(m_SubmitterPanel, "Submitter");

    m_GeneralPanel = new CGeneralPanel(m_Notebook, m_CmdProcessor, m_Seh);
    m_Notebook->AddPage(m_GeneralPanel, "General");

    m_GenomePanel = new CGenomeInfoPanel(m_Notebook, m_CmdProcessor, m_Seh);
    m_Notebook->AddPage(m_GenomePanel, "Genome info");

    m_OrgPanel = new COrganismInfoPanel(m_Notebook, m_Workbench, m_CmdProcessor, m_Seh, m_WorkDir);
    m_Notebook->AddPage(m_OrgPanel, "Organism info");

    m_MoleculePanel = new CSubMolinfoPanel(m_Notebook, m_CmdProcessor, m_Seh);
    m_Notebook->AddPage(m_MoleculePanel, "Molecule info");

    m_AnnotationPanel = new CSubAnnotationPanel(m_Notebook, m_CmdProcessor, m_WorkDir, m_Seh);
    m_Notebook->AddPage(m_AnnotationPanel, "Annotation");

    m_ReferencePanel = new CReferencePanel(m_Notebook, m_CmdProcessor, m_Seh);
    m_Notebook->AddPage(m_ReferencePanel, "Reference");

    m_ValidatePanel = new CSubValidateDiscPanel(m_Notebook, m_Workbench, m_WorkDir, m_ProjectHandle.GetPointer(), m_ProjectItem.GetPointer(), m_CmdProcessor);
    m_Notebook->AddPage(m_ValidatePanel, "Validation");

    wxStaticLine* itemStaticLine29 = new wxStaticLine(itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer2->Add(itemStaticLine29, 0, wxGROW | wxALL, 5);

    m_BottomSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_BottomSizer, 0, wxGROW | wxTOP | wxBOTTOM, 5);

    m_Backbtn = new wxButton(itemPanel1, ID_CSUBMISSIONBACKBTN, _("&Back"), wxDefaultPosition, wxDefaultSize, 0);
    m_BottomSizer->Add(m_Backbtn, 0, wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN  |wxALL, 5);

    m_BottomSizer->AddStretchSpacer();

    wxButton* itemButton4 = new wxButton( itemPanel1, ID_SUBMITTER_IMPORT, _("Import from template"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BottomSizer->Add(itemButton4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    m_BottomSizer->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemPanel1, ID_SUBMITTER_EXPORT, _("Export to template"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BottomSizer->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BottomSizer->AddStretchSpacer();

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( itemPanel1, wxID_HELP, _("Help"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    m_BottomSizer->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    m_ContinueBtn = new wxButton(itemPanel1, ID_CSUBMISSIONCONTBTN, _("&Continue"), wxDefaultPosition, wxDefaultSize, 0);
    m_BottomSizer->Add(m_ContinueBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    x_SetInitialState();

    TransferDataToWindow();   
}

void CSubmissionWizard::x_SetInitialState()
{
    m_BottomSizer->Show(m_Backbtn, false);
    wxColour bg_color = wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK);
    m_Notebook->SetBackgroundColour(bg_color);
    for (size_t i = 0; i < m_Notebook->GetPageCount(); ++i) {
        m_Notebook->GetPage(i)->SetBackgroundColour(bg_color);
    }
}


void CSubmissionWizard::x_SetTool()
{
    CGBenchVersionInfo version;
    GetGBenchVersionInfo(version);
    string version_str = "Genome Workbench ";
    version_str += NStr::IntToString(version.GetVer_major());
    version_str += ".";
    version_str += NStr::IntToString(version.GetVer_minor());
    version_str += ".";
    version_str += NStr::IntToString(version.GetVer_patch());
    version_str += " " + wxGetOsDescription().ToStdString();
    m_SubmitBlock->SetTool(version_str);
}

void CSubmissionWizard::x_SetSubmissionDate()
{
    auto& cit_sub = m_SubmitBlock->SetCit();
    if (!cit_sub.IsSetDate()) {
        CRef<CDate> today(new CDate);
        today->SetToTime(CurrentTime(), CDate::ePrecision_day);
        cit_sub.SetDate(*today);
    }
}


void CSubmissionWizard::x_InitializeSubmitBlock()
{
    m_SubmitBlock.Reset(new CSubmit_block);
    if (m_SeqSubmit && m_SeqSubmit->IsSetSub()) {
        m_SubmitBlock->Assign(m_SeqSubmit->GetSub());
    }
}


void CSubmissionWizard::x_UpdateSubmitBlock()
{
    m_SubmitterPanel->ApplySubmitBlock(*m_SubmitBlock);
    m_GeneralPanel->ApplySubmitBlock(*m_SubmitBlock);
    m_GenomePanel->ApplySubmitBlock(*m_SubmitBlock);
    m_OrgPanel->ApplySubmitBlock(*m_SubmitBlock);
    m_MoleculePanel->ApplySubmitBlock(*m_SubmitBlock);
    m_AnnotationPanel->ApplySubmitBlock(*m_SubmitBlock);
    m_ReferencePanel->ApplySubmitBlock(*m_SubmitBlock);
}


void CSubmissionWizard::x_UpdateDescriptors()
{
    bool found_pub = false;
    for (auto& it : m_Descriptors) {
        m_SubmitterPanel->ApplyDescriptor(*it);
        m_GeneralPanel->ApplyDescriptor(*it);
        m_GenomePanel->ApplyDescriptor(*it);
        m_OrgPanel->ApplyDescriptor(*it);
        m_MoleculePanel->ApplyDescriptor(*it);
        m_AnnotationPanel->ApplyDescriptor(*it);
        if (!found_pub & it->IsPub()) {
            m_ReferencePanel->ApplyDescriptor(*it);
            found_pub = true;
        }
    }
}

namespace {
    CRef<CPub> MakeEmptyRefPub()
    {
        CRef<CPub> citgen(new CPub());
        citgen->SetGen().SetCit("unpublished");
        return citgen;
    }

    bool s_IsCitSubPub(const CSeqdesc& desc)
    {
        if (desc.IsPub() && desc.GetPub().IsSetPub() && desc.GetPub().GetPub().IsSet() &&
            !desc.GetPub().GetPub().Get().empty() &&
            desc.GetPub().GetPub().Get().front()->IsSub()) {
            return true;
        }
        return false;
    }

    bool IsReferencePub(const CSeqdesc& seqdesc)
    {
        if (!seqdesc.IsPub()) {
            return false;
        }
        const CPubdesc& pubdesc = seqdesc.GetPub();
        if (pubdesc.IsSetPub() && pubdesc.GetPub().IsSet() &&
            !pubdesc.GetPub().Get().empty() &&
            (pubdesc.GetPub().Get().front()->IsGen() || pubdesc.GetPub().Get().front()->IsArticle())) {
            return true;
        }
        return false;
    }

    bool s_AddStringToField(const string& val, CUser_field& field) {
        if (field.GetData().IsStr()) {
            string old_val = field.GetData().GetStr();
            if (NStr::Equal(old_val, val)) {
                // nothing to do, already here
                return false;
            }
            else {
                field.SetData().SetStrs().push_back(old_val);
                field.SetData().SetStrs().push_back(val);
                return true;
            }
        }
        else if (field.GetData().IsStrs()) {
            for (auto s : field.GetData().GetStrs()) {
                if (NStr::Equal(s, val)) {
                    // already here
                    return false;
                }
            }
            field.SetData().SetStrs().push_back(val);
            return true;
        }
        else {
            return false;
        }
    }

    bool IsEmptyAuthor(const CAuthor& author) {
        if (author.IsSetAffil()) {
            return false;
        }
        else if (!author.IsSetName()) {
            return true;
        }
        else if (author.GetName().IsName()) {
            const CName_std& name_std = author.GetName().GetName();
            if (!name_std.IsSetLast()) {
                return true;
            }
            else if (NStr::Equal(name_std.GetLast(), "?")) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }

    bool IsEmptyRefPub(const CPub& pub)
    {
        if (!pub.IsGen()) {
            return false;
        }
        if (!pub.GetGen().IsSetAuthors()) {
            return true;
        }
        const CAuth_list& auth_list = pub.GetGen().GetAuthors();
        if (!auth_list.IsSetNames() || auth_list.GetNames().Which() == CAuth_list::TNames::e_not_set) {
            return true;
        }
        else if (!auth_list.GetNames().IsStd()) {
            if (auth_list.GetNames().GetStd().empty()) {
                return true;
            }
            for (auto it : auth_list.GetNames().GetStd()) {
                if (IsEmptyAuthor(*it)) {
                    // doesn't count
                }
                else {
                    return false;
                }
            }
        }
        return true;
    }

    bool s_IsEmptyStructuredComment(const CUser_object& user)
    {
        if (!user.IsSetData() || user.GetData().empty()) {
            return true;
        }
        for (auto it : user.GetData()) {
            if (it->IsSetLabel()
                && it->GetLabel().IsStr() 
                && (NStr::Equal(it->GetLabel().GetStr(), "StructuredCommentPrefix") || 
                    NStr::Equal(it->GetLabel().GetStr(), "StructuredCommentSuffix"))) {
                // doesn't count if it's a prefix or suffix
            }
            else {
                return false;
            }
        }
        return true;
    }


    bool s_IsEmptySource(const CBioSource& src)
    {
        if (src.IsSetSubtype() && !src.GetSubtype().empty()) {
            return false;
        }
        if (!src.IsSetOrg()) {
            return true;
        }
        const COrg_ref& org = src.GetOrg();
        if (org.IsSetTaxname() && !org.GetTaxname().empty()) {
            return false;
        }
        if (org.IsSetOrgname() && org.GetOrgname().IsSetMod() &&
            !org.GetOrgname().GetMod().empty()) {
            return false;
        }
        return true;
    }
}


void CSubmissionWizard::x_InitializeDescriptorList()
{
    m_Descriptors.clear();

    // add DBLink
    CRef<CSeqdesc> dblink(new CSeqdesc());
    dblink->SetUser().SetObjectType(CUser_object::eObjectType_DBLink);
    m_Descriptors.push_back(dblink);

    // add assembly-data
    m_AssemblyPrefix = "Genome-Assembly-Data";
    CRef<CSeqdesc> assembly(new CSeqdesc());
    CRef<CUser_object> user = edit::CStructuredCommentField::MakeUserObject(m_AssemblyPrefix);
    assembly->SetUser().Assign(*user);
    m_Descriptors.push_back(assembly);

    // add BioSource
    CRef<CSeqdesc> biosrc(new CSeqdesc());
    biosrc->SetSource();
    m_Descriptors.push_back(biosrc);

    // add reference pub
    CRef<CSeqdesc> pub(new CSeqdesc());
    pub->SetPub().SetPub().Set().push_back(MakeEmptyRefPub());
    m_Descriptors.push_back(pub);

    bool first_ref_pub = true;
    if (m_SeqSubmit && m_SeqSubmit->IsSetData() && m_SeqSubmit->GetData().IsEntrys()) {
        // search for descriptors in Seq-submit.data
        x_ExtractDescriptorsFromSeqEntry(*(m_SeqSubmit->GetData().GetEntrys().front()), first_ref_pub);
    } else if (m_Seh) {
        // search for descriptors in Seq-entry
        x_ExtractDescriptorsFromSeqEntry(*(m_Seh.GetCompleteSeq_entry()), first_ref_pub);
    }

}


/*!
 * Should we show tooltips?
 */

bool CSubmissionWizard::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSubmissionWizard::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmissionWizard bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmissionWizard bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSubmissionWizard::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmissionWizard icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmissionWizard icon retrieval
}

/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBMISSIONCONTBTN
 */

void CSubmissionWizard::OnContinuebtnClick(wxCommandEvent& event)
{
    GetSize(&m_Width, &m_Height);
    if (x_ValidateCurrentPage()) {
        ApplyCommand();
        x_StepForward();
        SetSize(wxSize(m_Width, m_Height));
    }
}

/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBMISSIONBACKBTN
 */

void CSubmissionWizard::OnBackbtnClick( wxCommandEvent& event )
{
    GetSize(&m_Width, &m_Height);
    ApplyCommand();
    x_StepBackward();
    SetSize(wxSize(m_Width, m_Height));
}

void CSubmissionWizard::OnPageChanging(wxBookCtrlEvent& event)
{
    ApplyCommand();
}

void CSubmissionWizard::OnPageChanged(wxBookCtrlEvent& event)
{
    wxWindow* wnd = m_Notebook->GetPage(event.GetSelection());
    UpdateOnPageChange(wnd);
}

static void restoreRecursivelyAllToolTips(wxWindow *win)
{
    if (!win)
    return;

    wxString tooltip = win->GetToolTipText();
    win->UnsetToolTip();
    if (!tooltip.IsEmpty())
        win->SetToolTip(tooltip);
    
    for (auto child : win->GetChildren())
    restoreRecursivelyAllToolTips(child);
}

void CSubmissionWizard::UpdateOnPageChange(wxWindow* current_page)
{
    if (!current_page) return;
    ISubmissionPage* current = dynamic_cast<ISubmissionPage*>(current_page);
    GetSize(&m_Width, &m_Height);
    if (current && m_Backbtn && m_ContinueBtn) {
        if (current->IsFirstPage()) {
            m_BottomSizer->Show(m_Backbtn, false);
        }
        else if (current->IsLastPage()) {
            m_ContinueBtn->SetLabel("Finish");
        }
        else {
            m_BottomSizer->Show(m_Backbtn, true);
            m_ContinueBtn->SetLabel("Continue");
        }
        SetSize(wxSize(m_Width, m_Height));
    }
#ifdef __WXMAC__
    restoreRecursivelyAllToolTips(current_page);
#endif
}

bool CSubmissionWizard::x_ValidateCurrentPage()
{
    return TransferDataFromWindow();
}

void CSubmissionWizard::x_StepForward()
{
    wxWindow* wnd = m_Notebook->GetCurrentPage();
    ISubmissionPage* current = dynamic_cast<ISubmissionPage*>(wnd);
    if (current) {
        if (current->IsLastPage()) {
            if (SaveFile())
            {
                NEditingStats::ReportSubWizardUsage("file_saved");
                Close();
            }
            return;
        }

        bool stepped = current->StepForward();
        if (!stepped) {
            int page = m_Notebook->GetSelection();
            if (page != wxNOT_FOUND) {
                m_Notebook->ChangeSelection(++page);
                stepped = true;
            }
        }
        if (stepped) {
            m_BottomSizer->Show(m_Backbtn, true);
            wnd = m_Notebook->GetCurrentPage();
            current = dynamic_cast<ISubmissionPage*>(wnd);
            if (current && current->IsLastPage()) {
                m_ContinueBtn->SetLabel("Finish");
            }
        }
    }
    else {
        LOG_POST(Error << "CSubmissionWizard::x_StepForward(): All pages of submission wizard should derive from ISubmissionPage");
    }
}

void CSubmissionWizard::x_StepBackward()
{
    wxWindow* wnd = m_Notebook->GetCurrentPage();
    ISubmissionPage* current = dynamic_cast<ISubmissionPage*>(wnd);
    if (current) {
        if (current->IsFirstPage()) {
            // this should never be true
            return;
        }
        
        bool stepped = current->StepBackward();
        if (!stepped) {
            int page = m_Notebook->GetSelection();
            if (page != wxNOT_FOUND) {
                m_Notebook->ChangeSelection(--page);
                stepped = true;
            }
            
        }
        if (stepped) {
            m_ContinueBtn->SetLabel("Continue");
            wnd = m_Notebook->GetCurrentPage();
            current = dynamic_cast<ISubmissionPage*>(wnd);
            if (current && current->IsFirstPage()) {
                m_BottomSizer->Show(m_Backbtn, false);
            }
        }
    } 
    else {
        LOG_POST(Error << "CSubmissionWizard::x_StepBackward(): All pages of submission wizard should derive from ISubmissionPage");
    }
}


void CSubmissionWizard::ApplySubmitBlock(const CSubmit_block& block)
{
    if (!m_SubmitBlock) {
        m_SubmitBlock.Reset(new CSubmit_block());
    }
    m_SubmitBlock->Assign(block);
}

void CSubmissionWizard::ExtractDescriptor(const CSeqdesc& desc, bool first_pub = false)
{    
    // Figure out if this descriptor should be added or should replace one we already have
    bool found = false;
    bool first_pub_in_list = true;
    CRef<CSeqdesc> add(NULL);
    for (auto& it : m_Descriptors) {
        if (it->Which() == desc.Which()) {
            if (it->IsUser()) {
                if (it->GetUser().IsSetType() && desc.GetUser().IsSetType() && it->GetUser().GetType().Equals(desc.GetUser().GetType())) {
                    found = true;
                    it->Assign(desc);
                    add = it;
                    break;
                }
            } else if (it->IsPub()) {
                if (first_pub_in_list && first_pub) {
                    found = true;
                    it->Assign(desc);
                    add = it;
                    break;
                } else if (it->GetPub().Equals(desc.GetPub())) {
                    // don't add if identical
                    found = true;
                    add = it;
                    break;
                }
                first_pub_in_list = false;
            } else {
                // TODO - are there any other descriptors of the same type that we could have multiples of?
                found = true;
                it->Assign(desc);
                add = it;
                break;
            }
        }
    }
    if (!found) {
        add.Reset(new CSeqdesc());
        add->Assign(desc);
        m_Descriptors.push_back(add);
    }   
}


unique_ptr<CObjectIStream> xCreateASNStream(CFormatGuess::EFormat format, unique_ptr<istream>& instream)
{
    // guess format
    ESerialDataFormat eSerialDataFormat = eSerial_None;
    { {
            if (format == CFormatGuess::eUnknown)
                format = CFormatGuess::Format(*instream);

            switch (format) {
            case CFormatGuess::eBinaryASN:
                eSerialDataFormat = eSerial_AsnBinary;
                break;
            case CFormatGuess::eUnknown:
            case CFormatGuess::eTextASN:
                eSerialDataFormat = eSerial_AsnText;
                break;
            case CFormatGuess::eXml:
                eSerialDataFormat = eSerial_Xml;
                break;
            default:
                wxMessageBox(_("Descriptor file seems to be in an unsupported format: ") + wxString(CFormatGuess::GetFormatName(format)));
                return unique_ptr<CObjectIStream>(nullptr);
                break;
            }

            //instream.seekg(0);
        }}

    unique_ptr<CObjectIStream> pObjIstrm(
        CObjectIStream::Open(eSerialDataFormat, *instream, eTakeOwnership));

    instream.release();

    return pObjIstrm;
}


// This is looking for the following:
// a source descriptor
// a DBLink User-object descriptor
// structured comment descriptors
// publications
void CSubmissionWizard::x_ExtractDescriptors(const CSeq_descr& descr, bool& first_ref_pub)
{
    if (!descr.IsSet()) {
        return;
    }
    for (auto it : descr.Get()) {
        if (it->IsSource()) {
            ExtractDescriptor(*it);
        }
        else if (it->IsUser()) {
            ExtractDescriptor(*it);
        }
        else if (it->IsPub()) {
            bool is_reference_pub = first_ref_pub && IsReferencePub(*it);
            ExtractDescriptor(*it, first_ref_pub);
            if (is_reference_pub) {
                first_ref_pub = false;
            }
        }
    }
}


void CSubmissionWizard::x_ExtractDescriptorsFromSeqEntry(const CSeq_entry& entry, bool& first_ref_pub)
{
    if (entry.IsSeq()) {
        if (entry.GetSeq().IsSetDescr()) {
            x_ExtractDescriptors(entry.GetSeq().GetDescr(), first_ref_pub);
        }
    } else if (entry.IsSet()) {
        const CBioseq_set& set = entry.GetSet();
        if (set.IsSetDescr()) {
            x_ExtractDescriptors(set.GetDescr(), first_ref_pub);
        }
        if (set.IsSetSeq_set()) {
            // Use first element of set only
            for (auto entry : set.GetSeq_set())  {
                x_ExtractDescriptorsFromSeqEntry(*entry, first_ref_pub);
                if (entry->IsSeq() && entry->GetSeq().IsNa())
                    break;
            }
        }
    }
}


void CSubmissionWizard::ImportTemplate( wxCommandEvent& event )
{
    wxFileDialog asn_open_file(this, wxT("Import from file"), m_WorkDir, wxEmptyString,
                               _("SBT files (*.sbt)|*.sbt|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (asn_open_file.ShowModal() == wxID_OK)
    {
        wxString path = asn_open_file.GetPath();
        if (!path.IsEmpty())
        {
            //m_WorkDir = wxFileName::DirName(path).GetPath();
            CNcbiIfstream istr(path.fn_str());
            unique_ptr<istream> instream(new CNcbiIfstream(path.fn_str()));
            unique_ptr<CObjectIStream> pObjIstrm = xCreateASNStream(CFormatGuess::eUnknown, instream);
            if (!pObjIstrm)
                return;

            // guess object type
            string sType = pObjIstrm->ReadFileHeader();

            // templates can come in many forms
            if (sType == CSubmit_block::GetTypeInfo()->GetName()) {
                CRef<CSubmit_block> block(new CSubmit_block());
                pObjIstrm->Read(ObjectInfo(*block), CObjectIStream::eNoFileHeader);
                ApplySubmitBlock(*block);
            } else if (sType == CSeq_submit::GetTypeInfo()->GetName()) {
                CRef<CSeq_submit> submit(new CSeq_submit());
                pObjIstrm->Read(ObjectInfo(*submit), CObjectIStream::eNoFileHeader);
                ApplySubmitBlock(submit->GetSub());
                if (submit->IsSetData() && submit->GetData().IsEntrys() && !submit->GetData().GetEntrys().empty()) {
                    bool first_ref_pub = true;
                    x_ExtractDescriptorsFromSeqEntry(*(submit->GetData().GetEntrys().front()), first_ref_pub);
                }
            } else if (sType == CSeq_entry::GetTypeInfo()->GetName()) {
                CRef<CSeq_entry> entry(new CSeq_entry());
                pObjIstrm->Read(ObjectInfo(*entry), CObjectIStream::eNoFileHeader);
                bool first_ref_pub = true;
                x_ExtractDescriptorsFromSeqEntry(*entry, first_ref_pub);
            }          

            // read in Seqdescs
            if (!pObjIstrm->EndOfData())
            {
                if (sType != CSeqdesc::GetTypeInfo()->GetName())
                    sType = pObjIstrm->ReadFileHeader();

                bool first_ref_pub = true;

                while (sType == CSeqdesc::GetTypeInfo()->GetName()) {
                    CRef<CSeqdesc> desc(new CSeqdesc);
                    pObjIstrm->Read(ObjectInfo(*desc), CObjectIStream::eNoFileHeader);

                    bool is_reference_pub = first_ref_pub && IsReferencePub(*desc);
                    ExtractDescriptor(*desc, desc->IsPub() ? first_ref_pub : false);
                    if (is_reference_pub) {
                        first_ref_pub = false;
                    }

                    if (pObjIstrm->EndOfData())
                        break;

                    try {
                        sType = pObjIstrm->ReadFileHeader();
                    }
                    catch (CEofException&) {
                        break;
                    }
                }
            }
        }
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("import template"));
    bool any_changes = false;

    // apply descriptors to record
    for (auto it : m_Descriptors) {
        any_changes |= x_ApplyDescriptorToRecord(*it, *cmd);
    }
    if (any_changes) {
        m_CmdProcessor->Execute(cmd);
    }

    ApplySubmitCommand();
}


void CSubmissionWizard::x_UpdateSeqEntryHandle()
{
    // m_SubmitterPanel->SetSeqEntryHandle(m_Seh); // no need
    m_GeneralPanel->SetSeqEntryHandle(m_Seh); 
    m_GenomePanel->SetSeqEntryHandle(m_Seh);
    m_ReferencePanel->SetSeqEntryHandle(m_Seh); 
    
    m_AnnotationPanel->SetSeqEntryHandle(m_Seh);
    m_OrgPanel->SetSeqEntryHandle(m_Seh);
    m_MoleculePanel->SetSeqEntryHandle(m_Seh);
    m_ValidatePanel->SetProjectItem(m_ProjectItem);
}


void CSubmissionWizard::ApplySubmitCommand()
{
    if (!m_SubmitBlock->IsSetContact() || 
        !m_SubmitBlock->GetContact().IsSetContact() ||
        !m_SubmitBlock->GetContact().GetContact().IsSetName() ||
        !m_SubmitBlock->GetContact().GetContact().GetName().IsName() ||
        !m_SubmitBlock->GetContact().GetContact().GetName().GetName().IsSetLast())
        return;
    if (!m_SubmitBlock->IsSetCit() ||
        !m_SubmitBlock->GetCit().IsSetAuthors() ||
        !m_SubmitBlock->GetCit().GetAuthors().IsSetNames() ||
        m_SubmitBlock->GetCit().GetAuthors().GetNames().Which() == CAuth_list::C_Names::e_not_set)
        return;

    if (!m_Seh)
        return;

    if (m_SeqSubmit && m_SeqSubmit->IsSetSub() && m_SeqSubmit->GetSub().Equals(m_SubmitBlock.GetNCObject()))
        return;

    x_SetTool();
    x_SetSubmissionDate();

    if (m_SeqSubmit && m_SeqSubmit->IsSetSub())
    {
        CRef<CChangeSubmitBlockCommand> cmd(new CChangeSubmitBlockCommand());
        CObject* actual = (CObject*) &m_SeqSubmit->GetSub();
        cmd->Add(actual, CConstRef<CObject>(m_SubmitBlock));
        m_CmdProcessor->Execute(cmd);
    }
    else
    {    
        CConstRef<CSeq_entry> entry = m_Seh.GetCompleteSeq_entry();
        CRef<CSeq_entry> copy(new CSeq_entry());
        copy->Assign(*entry);
    
        CRef<CSeq_submit> submission(new CSeq_submit);
        submission->SetSub(m_SubmitBlock.GetNCObject());
        submission->SetData().SetEntrys().push_back(copy);

        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_ProjectHandle.GetPointer());

        CRef<CProjectItem> new_item(new CProjectItem);
        new_item->SetDescr().assign(m_ProjectItem->GetDescr().begin(), m_ProjectItem->GetDescr().end());
        new_item->SetItem().SetSubmit(*submission);
        
        if (m_ProjectItem->IsSetLabel())
            new_item->SetLabel(m_ProjectItem->GetLabel());
        else
        {
            string label;
            CLabel::GetLabel(*copy, &label, CLabel::eDefault, nullptr);
            new_item->SetLabel(label);
        }

        CRef<CCmdChangeProjectItem> chg(new CCmdChangeProjectItem(*m_ProjectItem, doc, *new_item, m_Workbench));
        m_ProjectItem = new_item;
        m_CmdProcessor->Execute(chg);       
    }
}

void CSubmissionWizard::BeginBusy()
{
    if (!m_IsBusy)
    {
        wxBeginBusyCursor();
        m_IsBusy = true;
    }
}

bool CSubmissionWizard::x_ApplyDescriptorToRecord(const CSeqdesc& desc, CCmdComposite& cmd)
{
    bool rval = false;
    if (desc.IsSource()) {
        rval = x_ApplySourceDescriptorToRecord(desc.GetSource(), cmd); \
    } else if (desc.IsUser()) {
        if (desc.GetUser().GetObjectType() == CUser_object::eObjectType_DBLink) {
            rval = x_ApplyUserDescriptorToRecord(desc.GetUser(), false, cmd);
        } else {
            rval = x_ApplyUserDescriptorToRecord(desc.GetUser(), true, cmd);
        }
    } else if (desc.IsMolinfo()) {
        rval = x_ApplyMolinfoToRecords(desc, cmd);
    } else {
        bool already_has = false;
        if (m_Seh.IsSetDescr() && m_Seh.GetDescr().IsSet()) {
            for (auto di : m_Seh.GetDescr().Get()) {
                if (di->Equals(desc)) {
                    already_has = true;
                    break;
                }
            }
        }
        if (!already_has) {
            CRef<CSeqdesc> new_desc(new CSeqdesc());
            new_desc->Assign(desc);
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(m_Seh, *new_desc));
            cmd.AddCommand(*cmdAddDesc);
            rval = true;
        }
    }
    return rval;
}


bool CSubmissionWizard::x_ApplySourceDescriptorToRecord(const CBioSource& src, CCmdComposite& cmd)
{
    bool any_changes = false;

    for (CBioseq_CI bi(m_Seh, CSeq_inst::eMol_na); bi; ++bi) {
        CSeqdesc_CI di(*bi, CSeqdesc::e_Source);
        if (di) {
            // edit existing descriptor
            CRef<CSeqdesc> cpy(new CSeqdesc());
            cpy->Assign(*di);
            if (src.IsSetOrg()) {
                cpy->SetSource().SetOrg().Assign(src.GetOrg());
            }
            x_TransferImportedSubSources(src, cpy->SetSource());
            if (!di->GetSource().Equals(cpy->GetSource())) {
                CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *cpy));
                cmd.AddCommand(*chg);
                any_changes = true;
            }
        } else {
            // create new source descriptor on this sequence or on the nuc-prot that contains it
            CRef<CSeqdesc> new_desc(new CSeqdesc());
            new_desc->SetSource().Assign(src);
            CBioseq_set_Handle parent = bi->GetParentBioseq_set();
            if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(parent.GetParentEntry(), *new_desc));
                cmd.AddCommand(*cmdAddDesc);
            }
            else {
                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(bi->GetParentEntry(), *new_desc));
                cmd.AddCommand(*cmdAddDesc);
            }
            any_changes = true;
        }
    }
    return any_changes;
}


void CSubmissionWizard::x_TransferImportedSubSources(const CBioSource& src, CBioSource& dst)
{
    // only add or replace subsources, and only add subsources that are not chromosome or plasmid name
    if (!src.IsSetSubtype()) {
        return;
    }
    std::unordered_map <CSubSource::TSubtype, bool> found_first;
    for (auto sit : src.GetSubtype()) {
        if (sit->IsSetSubtype() && sit->IsSetName() &&
            sit->GetSubtype() != CSubSource::eSubtype_plasmid_name &&
            sit->GetSubtype() != CSubSource::eSubtype_chromosome) {
            auto findit = found_first.find(sit->GetSubtype());
            if (findit != found_first.end()) {
                // already found one of these and deleted all after the first, just add
                CRef<CSubSource> cpy(new CSubSource());
                cpy->Assign(*sit);
                dst.SetSubtype().push_back(cpy);
            } else {
                // replace first copy found and remove all other values
                bool found = false;
                auto dit = dst.SetSubtype().begin();
                while (dit != dst.SetSubtype().end()) {
                    if ((*dit)->IsSetSubtype() && (*dit)->GetSubtype() == sit->GetSubtype()) {
                        if (found) {
                            // already have one, remove the others
                            dit = dst.SetSubtype().erase(dit);
                        } else {
                            // replace value for this one
                            found = true;
                            (*dit)->SetName(sit->GetName());
                            dit++;
                        }
                    } else {
                        dit++;
                    }
                }
                if (!found) {
                    // add a new value
                    CRef<CSubSource> cpy(new CSubSource());
                    cpy->Assign(*sit);
                    dst.SetSubtype().push_back(cpy);
                }
                found_first[sit->GetSubtype()] = true;
            }
        } 
    }
}


bool CSubmissionWizard::x_ApplyOrReplaceDescriptorToRecord(const CSeqdesc& desc, CCmdComposite& cmd, CSeq_inst::TMol mol) 
{
    bool any_changes = false;
    for (CBioseq_CI bi(m_Seh, mol); bi; ++bi) {
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->Assign(desc);
        CSeqdesc_CI di(*bi, desc.Which());
        if (di) {
            // edit existing descriptor
            if (!di->Equals(desc)) {
                CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *new_desc));
                cmd.AddCommand(*chg);
                any_changes = true;
            }
        } else {
            // create new descriptor on this sequence
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(bi->GetParentEntry(), *new_desc));
            cmd.AddCommand(*cmdAddDesc);
            any_changes = true;
        }
    }
    return any_changes;
}


bool CSubmissionWizard::x_ApplyMolinfoToRecords(const CSeqdesc& desc, CCmdComposite& cmd)
{
    if (!desc.IsMolinfo()) {
        return false;
    }
    const CMolInfo& molinfo = desc.GetMolinfo();
    bool any_changes = false;
    if (molinfo.IsSetBiomol() && molinfo.GetBiomol() == CMolInfo::eBiomol_peptide) {
        // apply to proteins
        any_changes = x_ApplyOrReplaceDescriptorToRecord(desc, cmd, CSeq_inst::eMol_aa);
    } else {
        // apply to nucleotides
        any_changes = x_ApplyOrReplaceDescriptorToRecord(desc, cmd, CSeq_inst::eMol_na);
    }
    return any_changes;
}


bool CSubmissionWizard::x_ApplyUserDescriptorToRecord(const CUser_object& user, bool add_to_set, CCmdComposite& cmd)
{
    bool any_changes = false;
    bool found_any = false;
    if (!user.IsSetType()) {
        // reject
        return false;
    }

    for (CBioseq_CI bi(m_Seh, CSeq_inst::eMol_na); bi; ++bi) {
        bool found = false;
        for (CSeqdesc_CI di(*bi, CSeqdesc::e_User); di; ++di) {
            const CUser_object& this_user = di->GetUser();
            if (this_user.IsSetType() && this_user.GetType().Equals(user.GetType())) {
                if (user.GetObjectType() == CUser_object::eObjectType_DBLink) {
                    // merge
                    CRef<CSeqdesc> cpy(new CSeqdesc());
                    cpy->SetUser().Assign(this_user);
                    bool merge = x_MergeDBLink(user, cpy->SetUser());
                    if (merge) {
                        CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *cpy));
                        cmd.AddCommand(*chg);
                        any_changes = true;
                    }
                    found = true;
                } else if (user.GetObjectType() == CUser_object::eObjectType_StructuredComment) {
                    string template_prefix = CComment_rule::GetStructuredCommentPrefix(user);
                    string this_prefix = CComment_rule::GetStructuredCommentPrefix(this_user);
                    if (NStr::Equal(template_prefix, this_prefix)) {
                        found = true;
                    }
                } else {
                    // don't overwrite
                    found = true;
                }
                break;
            }
        }
        if (!found && !add_to_set) {
            // create new descriptor on this sequence or on the nuc-prot that contains it
            CRef<CSeqdesc> new_desc(new CSeqdesc());
            new_desc->SetUser().Assign(user);
            CBioseq_set_Handle parent = bi->GetParentBioseq_set();
            if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(parent.GetParentEntry(), *new_desc));
                cmd.AddCommand(*cmdAddDesc);
            } else {
                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(bi->GetParentEntry(), *new_desc));
                cmd.AddCommand(*cmdAddDesc);
            }
            any_changes = true;
        } else if (found) {
            found_any = true;
        }
    }
    if (!found_any && add_to_set && !s_IsEmptyStructuredComment(user)) {
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->SetUser().Assign(user);
        CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(m_Seh, *new_desc));
        cmd.AddCommand(*cmdAddDesc);
        any_changes = true;
    }
    return any_changes;
}

bool CSubmissionWizard::x_MergeDBLink(const CUser_object& template_user, CUser_object& existing_user)
{
    if (!template_user.IsSetData()) {
        return false;
    }
    bool any_change = false;
    for (auto t_it : template_user.GetData()) {
        if (t_it->IsSetLabel() && t_it->IsSetData() && (t_it->GetData().IsStr() || t_it->GetData().IsStrs())) {
            bool found = false;
            for (auto& e_it : existing_user.SetData()) {
                if (e_it->IsSetLabel() && e_it->GetLabel().Equals(t_it->GetLabel())) {
                    if (t_it->GetData().IsStr()) {
                        any_change |= s_AddStringToField(t_it->GetData().GetStr(), *e_it);
                    } else {
                        for (auto s : t_it->GetData().GetStrs()) {
                            any_change |= s_AddStringToField(s, *e_it);
                        }
                    }
                }
            }
        }
    }
    return any_change;
}

void CSubmissionWizard::ExportTemplate( wxCommandEvent& event )
{
    ApplyCommand();

    // make a few changes before exporting
    // if authors empty, need to put in dummy name or cannot save ASN.1
    if (!m_SubmitBlock->IsSetCit() || !m_SubmitBlock->GetCit().IsSetAuthors() ||
        !m_SubmitBlock->GetCit().GetAuthors().IsSetNames() ||
        (m_SubmitBlock->GetCit().GetAuthors().GetNames().IsStd() &&
         !m_SubmitBlock->GetCit().GetAuthors().GetNames().GetStd().front()->GetName().GetName().IsSetLast())) {
        if (m_SubmitBlock->IsSetContact() && m_SubmitBlock->GetContact().IsSetContact()) {
            CRef<CAuthor> copy(new CAuthor());
            copy->Assign(m_SubmitBlock->GetContact().GetContact());
            m_SubmitBlock->SetCit().SetAuthors().SetNames().SetStd().push_back(copy);
        } else {
            m_SubmitBlock->SetCit().SetAuthors().SetNames().SetStr().push_back("?");
        }
    }


    wxFileDialog asn_save_file(this, wxT("Export to file"), m_WorkDir, wxEmptyString,
                               _("SBT files (*.sbt)|*.sbt|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (asn_save_file.ShowModal() == wxID_OK)
    {
        wxString path = asn_save_file.GetPath();
        if (!path.IsEmpty())
        {
            //m_WorkDir = wxFileName::DirName(path).GetPath();
            ios::openmode mode = ios::out;
            CNcbiOfstream os(path.fn_str(), mode);
            os << MSerial_AsnText;

            os << m_SubmitBlock;
            for (auto it : m_Descriptors) {
                if (it->IsUser()) {
                    // only write out if it has fields
                    // and is not an empty structured comment
                    const CUser_object& user = it->GetUser();
                    if (user.IsSetData() && !user.GetData().empty() &&
                        !s_IsEmptyStructuredComment(user)) {
                        os << it;
                    }
                }
                else if (it->IsSource()) {
                    // only write out if non-empty
                    if (!s_IsEmptySource(it->GetSource())) {
                        os << it;
                    }
                } else if (it->IsPub()) {
                    // only write out if non-empty
                    const CPubdesc& pub = it->GetPub();
                    if (pub.IsSetPub() && pub.GetPub().IsSet() && !pub.GetPub().Get().empty() &&
                        !IsEmptyRefPub(*(pub.GetPub().Get().front()))) {
                        os << it;
                    }
                } else {
                    os << it;
                }
            }
        }
    }

}

CSubmissionWizard *CSubmissionWizard::m_Instance = NULL;

CSubmissionWizard* CSubmissionWizard::GetInstance( wxWindow* parent, IWorkbench* wb, 
    CRef<CGBProjectHandle> ph, CRef<CProjectItem> pi, wxWindowID id, 
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    if (!m_Instance)
    {
        m_Instance = new CSubmissionWizard(parent, wb, ph, pi, id, caption, pos, size, style);
        m_Instance->Show(true);           
    }
    else
    {
        m_Instance->Refresh();
    }
    m_Instance->RaiseWindow();
    return m_Instance;
}


void CSubmissionWizard::ConnectListener()
{
    CGBDocument* doc = dynamic_cast<CGBDocument*>(m_ProjectHandle.GetPointer());
    if (!doc)
        return;
    doc->AddListener(this);
}


void CSubmissionWizard::DisconnectListener()
{
    CGBDocument* doc = dynamic_cast<CGBDocument*>(m_ProjectHandle.GetPointer());
    if (!doc)
        return;
    doc->RemoveListener(this);
}

void CSubmissionWizard::OnDataChanged(CEvent *evt) 
{
    if (!evt)
        return;
    CProjectViewEvent *prj_evt = dynamic_cast<CProjectViewEvent*>(evt);
    if (!prj_evt)
        return;
    switch (prj_evt->GetSubtype()) 
    {
    case CProjectViewEvent::eData:  
        x_LoadProjectItemData();
        x_InitializeSubmitBlock();
        x_InitializeDescriptorList();     
        TransferDataToWindow();
        Refresh();
        //RaiseWindow();
        break;
    case CProjectViewEvent::eUnloadProject:
        Close();
        break;
    default:
        break;
    } 
}

bool CSubmissionWizard::TransferDataToWindow()
{
    x_UpdateSeqEntryHandle();
    x_UpdateSubmitBlock();
    x_UpdateDescriptors();
    for (size_t i = 0; i < m_Notebook->GetPageCount(); ++i) {
        m_Notebook->GetPage(i)->TransferDataToWindow();
    }
    
    return true;
}

void CSubmissionWizard::EndBusy()
{
    if (m_IsBusy)
    {
        wxEndBusyCursor();
        m_IsBusy = false;
    }
}

void CSubmissionWizard::ApplyCommand()
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    page->ApplyCommand();
}

void CSubmissionWizard::OnHelp( wxHyperlinkEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    
    wxString anchor = page->GetAnchor(); 
    if (anchor.IsEmpty())
        return;
    
    wxURI help_url (wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual6/#") + anchor);
    wxString help_encoded_url = help_url.BuildURI();
    wxLaunchDefaultBrowser(help_encoded_url, 0); // wxBROWSER_NEW_WINDOW
}


bool CSubmissionWizard::SaveFile()
{
    if (!m_Seh)
    {
        wxMessageBox (_("Unable to find submission data"));
        return false;
    }

    CConstRef<CSerialObject> so;
    if (m_SeqSubmit)
    {
        so = m_SeqSubmit;
    }
    else
    {
        int answer = wxMessageBox (_("You need to add contact information before submitting to GenBank - save incomplete file?"), 
                                   _("Save Incomplete File"), wxYES_NO |  wxICON_QUESTION);
        if (answer == wxYES)
        {
            so = m_Seh.GetCompleteSeq_entry();
        }
    }
    if (!so)
        return false;
    if (ReportMissingFields())
        return false;

    try {
        CRef<CCmdComposite> cleanup_cmd = CleanupCommand(m_Seh, true, false);
        if (cleanup_cmd)
            m_CmdProcessor->Execute(cleanup_cmd);
    }
    catch (const CException& e) {
        LOG_POST(Error << "Cleanup command failed: " << e.GetMsg());
    }

    wxFileDialog asn_save_file(this, wxT("Save Submission File"), m_WorkDir, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (asn_save_file.ShowModal() == wxID_OK)
    {
        wxString path = asn_save_file.GetPath();
        if( !path.IsEmpty())
        {
            ios::openmode mode = ios::out;
            CNcbiOfstream os(path.fn_str(), mode);
            os << MSerial_AsnText;
            os << *so;
            return true;
        }
    }
    return false;   
}

bool CSubmissionWizard::ReportMissingFields()
{
    string text;
    for (size_t i = 0; i < m_Notebook->GetPageCount(); ++i) 
    {
        wxWindow* win = m_Notebook->GetPage(i);
        ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
        page->ReportMissingFields(text);
    }
    if (text.empty())
        return false;
    wxString msg;
    msg << "The following fields are missing.\n";
    msg << "This file should not be submitted to GenBank without this information.\n";    
    msg << text;
    CGenericReportDlg* report = new CGenericReportDlg(NULL);  
    report->SetTitle(wxT("Missing Fields")); 
    report->SetText(msg);
    report->Show(true);
    return true;
}

void CSubmissionWizard::RaiseWindow()
{
    CAppTaskService* taskService = m_Workbench->GetServiceByType<CAppTaskService>();
    CRaiseWindowTask* task = new CRaiseWindowTask(this);
    taskService->AddTask(*task);
}

END_NCBI_SCOPE




