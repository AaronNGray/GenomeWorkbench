/*  $Id: edit_pub_dlg_std.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
 * Authors:  Colleen Bollin, Igor Filippov
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <objects/biblio/Affil.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>



#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/widgets/edit/work_dir.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/edit_pub_dlg_std.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>

#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/display.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);


IMPLEMENT_DYNAMIC_CLASS( CEditPubDlg, CBulkCmdDlg )


/*!
 * CEditPubDlg event table definition
 */

BEGIN_EVENT_TABLE( CEditPubDlg, CBulkCmdDlg )

////@begin CEditPubDlg event table entries
////@end CEditPubDlg event table entries

END_EVENT_TABLE()


/*!
 * CEditPubDlg constructors
 */

CEditPubDlg::CEditPubDlg()
{
    Init();
}

CEditPubDlg::CEditPubDlg( wxWindow* parent, IWorkbench* wb,  wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    SetRegistryPath("Dialogs.Edit.EditPublications");
    LoadSettings();
}


/*!
 * CEditPubDlg creator
 */

bool CEditPubDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditPubDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditPubDlg creation
    return true;
}


/*!
 * CEditPubDlg destructor
 */

CEditPubDlg::~CEditPubDlg()
{
////@begin CEditPubDlg destruction
    SaveSettings();
////@end CEditPubDlg destruction
}


static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CEditPubDlg::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CEditPubDlg::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);
}


void CEditPubDlg::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        SetSize(wxSize(width,height));

    int pos_x = view.GetInt(kFramePosX, -1);
    int pos_y = view.GetInt(kFramePosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       SetPosition(wxPoint(pos_x,pos_y));
   }
}


/*!
 * Member initialisation
 */

void CEditPubDlg::Init()
{
////@begin CEditPubDlg member initialisation
    m_Notebook = NULL;
    m_Fieldhandler = NULL;
    m_StrConstraintPanel = NULL;
    m_PubStatusConstraint = NULL;
    m_OkCancel = NULL;
////@end CEditPubDlg member initialisation
}


/*!
 * Control creation for CEditPubDlg
 */

void CEditPubDlg::CreateControls()
{    
////@begin CEditPubDlg content construction
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemCBulkCmdDlg1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemCBulkCmdDlg1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxNotebook(itemCBulkCmdDlg1, wxID_ANY,wxDefaultPosition,wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 5);
    
    CReplaceSectionPanel *panel1 = new CReplaceSectionPanel(m_Notebook, this);    
    m_Notebook->AddPage(panel1,_("Replace section"));

    CReplaceEntirePubPanel *panel2 = new CReplaceEntirePubPanel(m_Notebook, this);    
    m_Notebook->AddPage(panel2,_("Replace entire pub"));

    CReplaceSingleFieldPanel *panel3 = new CReplaceSingleFieldPanel(m_Notebook, this);    
    m_Notebook->AddPage(panel3,_("Replace single field"));


    CRef<CAuth_list> authors;
    CConstRef<CObject> sel_pub = GetSelectedPub(); 
    if (sel_pub) 
    {
        CConstRef<CAuth_list> auth_list = GetAuthList(sel_pub);
        if (auth_list) 
        {
            authors.Reset(new CAuth_list());
            authors->Assign(*auth_list);
        }
    }

    CAuthorReplacePanel *panel4 = new CAuthorReplacePanel(m_Notebook, this, authors);    
    m_Notebook->AddPage(panel4,_("Global author replace"));

    wxStaticBox* constraintDetails = new wxStaticBox(itemCBulkCmdDlg1, wxID_ANY, _("Constraint"));
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(constraintDetails, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer9->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_Fieldhandler = new CFieldHandlerNamePanel(itemStaticBoxSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0);
    m_Fieldhandler->SetFieldNames(CPubField::GetFieldNames());
    itemBoxSizer10->Add(m_Fieldhandler, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_StrConstraintPanel = new CStringConstraintPanel(itemStaticBoxSizer9->GetStaticBox(), true, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0);
    itemBoxSizer10->Add(m_StrConstraintPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer9->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxStaticText* itemStaticText14 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), wxID_STATIC, _("Publication Status"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_PubStatusConstraintStrings;
    m_PubStatusConstraintStrings.Add(_("Any"));
    m_PubStatusConstraintStrings.Add(_("Published"));
    m_PubStatusConstraintStrings.Add(_("Unpublished"));
    m_PubStatusConstraintStrings.Add(_("In Press"));
    m_PubStatusConstraintStrings.Add(_("Submitter Block"));
    m_PubStatusConstraint = new wxChoice( itemStaticBoxSizer9->GetStaticBox(), ID_PUB_STATUS_CONSTRAINT, wxDefaultPosition, wxDefaultSize, m_PubStatusConstraintStrings, 0 );
    m_PubStatusConstraint->SetStringSelection(_("Any"));
    itemBoxSizer13->Add(m_PubStatusConstraint, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OkCancel = new COkCancelPanel(itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0);
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);    


////@end CEditPubDlg content construction

    if (!GetTopLevelSeqEntryAndProcessor() && !m_TopSeqEntry)
        return;

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv) return;

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
    if (!doc) return;

    SetWorkDir(this, doc->GetWorkDir());
}


/*!
 * Should we show tooltips?
 */

bool CEditPubDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditPubDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditPubDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditPubDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditPubDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditPubDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditPubDlg icon retrieval
}


string CEditPubDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}

const string kUnknownError = "Unknown error";

CRef<CCmdComposite> CEditPubDlg::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);
    m_ErrorMessage = kUnknownError;
    if (!m_TopSeqEntry) {
        return cmd;
    }

    string constraint_field = m_Fieldhandler->GetFieldName(true);
    CRef<edit::CStringConstraint> string_constraint = m_StrConstraintPanel->GetStringConstraint();
    CRef<CEditingActionConstraint> constraint1 = CreateEditingActionConstraint("status", CFieldNamePanel::eFieldType_Pub, -1, constraint_field, CFieldNamePanel::eFieldType_Pub, -1,
									       CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(string_constraint)));     
    
    string desired_status = GetStatusConstraint();
    CRef<CEditingActionConstraint> constraint2 = CreateEditingActionConstraint("", CFieldNamePanel::eFieldType_Pub, -1, "", CFieldNamePanel::eFieldType_Unknown, -1, 
									       CRef<CConstraintMatcher>(new CPubStatusConstraintMatcher(desired_status)));                   
    *constraint1 += constraint2;
    
    vector<CSeq_entry_Handle> descr_ctxt;
    vector<CConstRef<CObject>> objs = GetObjects(m_TopSeqEntry, "status", CFieldNamePanel::eFieldType_Pub, -1, m_SeqSubmit, constraint1, &descr_ctxt);


    if (objs.empty()) 
    {
        m_ErrorMessage = "No publications found!";
        return cmd;
    } 

    int sel = m_Notebook->GetSelection();
    bool any_change(false);
    CEditPubCmdPanel *win = dynamic_cast<CEditPubCmdPanel*>(m_Notebook->GetPage(sel));
    if (win) 
    {
        cmd = win->GetCommand(objs, descr_ctxt);
    } 

    if (!cmd && NStr::Equal(m_ErrorMessage, kUnknownError))
    {
        wxMessageBox(wxT("No effect!"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }

    return cmd;
}

CConstRef<CObject> CEditPubDlg::GetSelectedPub()
{
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) return CConstRef<CObject>(NULL);
    sel_srv->GetCurrentSelection(objects);
    if (objects.empty()) return CConstRef<CObject>(NULL);
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CObject* ptr = it->object.GetPointer();
        const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(ptr);
        const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(ptr);
        const CCit_sub* citsub = dynamic_cast<const CCit_sub*>(ptr);
        const CPubdesc* pubdesc = dynamic_cast<const CPubdesc*>(ptr);
        const CSubmit_block* block = dynamic_cast<const CSubmit_block*>(ptr);
        if (seqdesc && seqdesc->IsPub()) {
            return CConstRef<CObject>(seqdesc);
        } else if (seqfeat && seqfeat->IsSetData() && seqfeat->GetData().IsPub()) {
            return CConstRef<CObject>(seqfeat);
        } else if (citsub) {
            return CConstRef<CObject>(citsub);
        } else if (pubdesc) {
            return CConstRef<CObject>(pubdesc);
        } else if (block && block->IsSetCit()) {
            return CConstRef<CObject>(block);
        }
    }
    return CConstRef<CObject>(NULL);
}


void CEditPubDlg::AppendToErrorMsg(const string &msg)
{
    if (NStr::Equal(m_ErrorMessage, kUnknownError)) {
        m_ErrorMessage = msg + "\n";
    } else {
        m_ErrorMessage += msg + "\n";
    }
}


IMPLEMENT_DYNAMIC_CLASS( CEditPubSubpanel, wxPanel )
BEGIN_EVENT_TABLE( CEditPubSubpanel, wxPanel )
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS( CReplaceSectionPanel, CEditPubSubpanel )

BEGIN_EVENT_TABLE( CReplaceSectionPanel, CEditPubSubpanel )
END_EVENT_TABLE()

void CReplaceSectionPanel::CreateControls()
{    
    CReplaceSectionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    m_AuthorListSection = new wxCheckBox( this, wxID_ANY, wxT("Replace author list with selected author list"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_AuthorListSection, 0, wxALIGN_LEFT|wxRIGHT|wxTOP|wxBOTTOM, 5);
    m_TitleSection = new wxCheckBox( this, wxID_ANY, wxT("Replace title with selected title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_TitleSection, 0, wxALIGN_LEFT|wxRIGHT|wxTOP|wxBOTTOM, 5);
    m_AffiliationSection = new wxCheckBox( this, wxID_ANY, wxT("Replace affiliation with selected affiliation"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_AffiliationSection, 0, wxALIGN_LEFT|wxRIGHT|wxTOP|wxBOTTOM, 5);
}

CRef<CCmdComposite> CReplaceSectionPanel::GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Edit Publications"));

    CConstRef<CObject> sel_pub = m_dlg->GetSelectedPub();
    if (!sel_pub) 
    {
        m_dlg->AppendToErrorMsg("No publication is selected to use as a source!");
        return CRef<CCmdComposite> (NULL);
    }
    CConstRef<CAuth_list> auth_list = GetAuthList(sel_pub);
    string title = GetPubTitle(sel_pub);
    
    bool any_change = false;
    auto obj_it = objs.begin();
    auto seh_it = descr_ctxt.begin();
    _ASSERT(objs.size() == descr_ctxt.size());
    for ( ; obj_it != objs.end() && seh_it != descr_ctxt.end(); ++obj_it, ++seh_it) 
    {
        const CSeqdesc* d = dynamic_cast<const CSeqdesc* >(obj_it->GetPointer());
        if (d && d->IsPub()) {
            bool this_change = false;
            CRef<CSeqdesc> new_desc(new CSeqdesc());
            new_desc->Assign(*d);
            
            if (m_AuthorListSection->GetValue() && auth_list) {
                this_change = SetAuthorNames(new_desc->SetPub(), *auth_list);
            }
            if (m_AffiliationSection->GetValue() && auth_list && auth_list->IsSetAffil()) {
                CConstRef<CAuth_list> this_auth = GetAuthList(new_desc->GetPub());
                CRef<CAuth_list> new_auth(new CAuth_list());
                new_auth->Assign(*this_auth);
                new_auth->SetAffil().Assign(auth_list->GetAffil());
                this_change |= SetAuthList(new_desc->SetPub(), *new_auth);
            }
            if (m_TitleSection->GetValue()) {
                bool skip_cit_sub = true;
                this_change |= CPubField::SetPubTitle (new_desc->SetPub(), title, edit::eExistingText_replace_old, skip_cit_sub);
            }
            
            if (this_change) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(*seh_it, *d, *new_desc));
                cmd->AddCommand (*ecmd);
                any_change = true;
            }
        }
    }

    if (!any_change) {
        cmd.Reset(NULL);
        m_dlg->AppendToErrorMsg("No action.");
    }
     
    return cmd;
}



IMPLEMENT_DYNAMIC_CLASS( CReplaceEntirePubPanel, CEditPubSubpanel )

BEGIN_EVENT_TABLE( CReplaceEntirePubPanel, CEditPubSubpanel )
END_EVENT_TABLE()

void CReplaceEntirePubPanel::CreateControls()
{    
    CReplaceEntirePubPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* instructions = new wxStaticText( this, wxID_STATIC, wxT("Replace with selected publication or submitter block"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(instructions, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxTOP|wxBOTTOM, 5);
}

CRef<CCmdComposite> CReplaceEntirePubPanel::GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt)
{
    CConstRef<CObject> sel_pub = m_dlg->GetSelectedPub();
    if (!sel_pub) 
    {
        m_dlg->AppendToErrorMsg("No publication is selected to use as a source!");
        return CRef<CCmdComposite> (NULL);
    }

    const CSeqdesc* orig_desc = dynamic_cast<const CSeqdesc* >(sel_pub.GetPointer());
    const CSeq_feat* orig_feat = dynamic_cast<const CSeq_feat*>(sel_pub.GetPointer());
    const CCit_sub* citsub = dynamic_cast<const CCit_sub*>(sel_pub.GetPointer());
    const CPubdesc* pubdesc = dynamic_cast<const CPubdesc*>(sel_pub.GetPointer());

    bool any_change = false;
    bool check_status = NStr::EqualNocase(m_dlg->GetStatusConstraint(), "Any");

    CRef<CCmdComposite> cmd(new CCmdComposite("Edit Publications"));
    auto obj_it = objs.begin();
    auto seh_it = descr_ctxt.begin();
    _ASSERT(objs.size() == descr_ctxt.size());
    for (; obj_it != objs.end() && seh_it != descr_ctxt.end(); ++obj_it, ++seh_it)
    {
        if (obj_it->GetPointer() == sel_pub.GetPointer()) {
            continue;
        }
        // skip pubs with status submitter, only when "Replace entire publication" is selected
        if (check_status) {
            string status = CPubField::GetStatus(**obj_it);
            if (NStr::EqualNocase(status, "Submitter Block"))
            {
                continue;
            }
        }
        const CSeqdesc* d = dynamic_cast<const CSeqdesc* >(obj_it->GetPointer());
        if (d && d->IsPub() && pubdesc != &(d->GetPub())) {
            CRef<CSeqdesc> new_desc(new CSeqdesc());            
            bool this_change = false;
            if (orig_desc) {
                new_desc->Assign(*orig_desc);
                this_change = true;
            } else if (orig_feat) {
                new_desc->Assign(*d);
                new_desc->SetPub().Assign(orig_feat->GetData().GetPub());
                this_change = true;
            } else if (citsub) {
                new_desc->Assign(*d);
                new_desc->SetPub().ResetPub();
                CRef<CPub> new_pub(new CPub());
                new_pub->SetSub().Assign(*citsub);
                new_desc->SetPub().SetPub().Set().push_back(new_pub);
                this_change = true;
            } else if (pubdesc) {
                new_desc->Assign(*d);
                new_desc->SetPub().Assign(*pubdesc);
                this_change = true;
            }
            if (this_change) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(*seh_it, *d, *new_desc));
                cmd->AddCommand (*ecmd);
                any_change = true;
            }
        }
    }

    if (!any_change) 
    {
        cmd.Reset(NULL);
        m_dlg->AppendToErrorMsg("No target publications.");
    }
            
    return cmd;
}


IMPLEMENT_DYNAMIC_CLASS( CReplaceSingleFieldPanel, CEditPubSubpanel )

BEGIN_EVENT_TABLE( CReplaceSingleFieldPanel, CEditPubSubpanel )
END_EVENT_TABLE()

void CReplaceSingleFieldPanel::CreateControls()
{    
    CReplaceSingleFieldPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_PubField = new CFieldHandlerNamePanel(this);
    m_PubField->SetFieldNames(CPubField::GetFieldNames());
    itemBoxSizer2->Add(m_PubField, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxTOP|wxBOTTOM, 5);
    m_FieldText = new wxTextCtrl(this, wxID_ANY);
    itemBoxSizer2->Add(m_FieldText, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxTOP|wxBOTTOM, 5);
}

CRef<CCmdComposite> CReplaceSingleFieldPanel::GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt)
{
    string field_name = m_PubField->GetFieldName(true);
    if (NStr::IsBlank(field_name)) 
    {
        m_dlg->AppendToErrorMsg("You must select a field to apply.");
        return CRef<CCmdComposite>(NULL);
    }

    string val = ToStdString(m_FieldText->GetValue());
    string constraint_field = m_dlg->GetFieldName(true);
    CRef<edit::CStringConstraint> string_constraint = m_dlg->GetStringConstraint();   

    string desired_status = m_dlg->GetStatusConstraint();

    CConstRef<objects::CSeq_submit> submit = m_dlg->GetSeqSubmit();

    CIRef<IEditingAction> action = CreateAction(m_dlg->GetTopSeqEntry(), field_name, CFieldNamePanel::eFieldType_Pub, -1, submit);
    CRef<CEditingActionConstraint> constraint1 = CreateEditingActionConstraint(field_name, CFieldNamePanel::eFieldType_Pub, -1, constraint_field, CFieldNamePanel::eFieldType_Pub, -1, 
									      CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(string_constraint)));  
    CRef<CEditingActionConstraint> constraint2 = CreateEditingActionConstraint("", CFieldNamePanel::eFieldType_Pub, -1, "", CFieldNamePanel::eFieldType_Unknown, -1, 
									       CRef<CConstraintMatcher>(new CPubStatusConstraintMatcher(desired_status)));                   
    *constraint1 += constraint2;
    action->SetConstraint(constraint1);
    action->SetExistingText(edit::eExistingText_replace_old);
    action->Apply(val);
    CRef<CCmdComposite> cmd = action->GetActionCommand();
    if (!cmd) 
    {
	    m_dlg->AppendToErrorMsg("No action.");
    }
    
    return cmd;
}


IMPLEMENT_DYNAMIC_CLASS( CAuthorReplacePanel, CEditPubSubpanel )

BEGIN_EVENT_TABLE( CAuthorReplacePanel, CEditPubSubpanel )
END_EVENT_TABLE()

void CAuthorReplacePanel::CreateControls()
{    
    CAuthorReplacePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    if (!m_Authors) 
    {
        m_Authors.Reset(new CAuth_list());
    }
    m_AuthorsPanel = new CAuthorNamesPanel(this, *m_Authors, false);
    itemBoxSizer2->Add(m_AuthorsPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxTOP|wxBOTTOM, 5);
    m_AuthorsPanel->TransferDataToWindow();    
}

CRef<CCmdComposite> CAuthorReplacePanel::GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt)
{
    if (!m_Authors) 
    {
        m_Authors.Reset(new CAuth_list());
    }
    m_AuthorsPanel->PopulateAuthors(*m_Authors);
    if (!m_Authors->IsSetNames() || m_Authors->GetNameCount() == 0) 
    {
        m_dlg->AppendToErrorMsg("You must supply an author list.");
        return CRef<CCmdComposite>(NULL);
    }


   CAuth_list& edited_auth_list = *m_Authors;
    if (!edited_auth_list.IsSetNames())
    {
        m_dlg->AppendToErrorMsg("No names set.");
        return CRef<CCmdComposite>(NULL);
    }
    CAuth_list::TNames& edited_names = edited_auth_list.SetNames();

    bool any_modified = false;
    CRef<CCmdComposite> comp_cmd(new CCmdComposite("Replace All Authors"));
    auto obj_it = objs.begin();
    auto seh_it = descr_ctxt.begin();
    _ASSERT(objs.size() == descr_ctxt.size());
    for (; obj_it != objs.end() && seh_it != descr_ctxt.end(); ++obj_it, ++seh_it)
    {
        if (const CSeqdesc* desc = dynamic_cast<const CSeqdesc* >(obj_it->GetPointer())) {
            if (desc->IsPub()) {
                CRef<CSeqdesc> replacement(new CSeqdesc());
                replacement->Assign(*desc);
                bool modified = false;
                NON_CONST_ITERATE(CPub_equiv::Tdata, pub, replacement->SetPub().SetPub().Set())
                {
                    switch ((*pub)->Which())
                    {
                    case CPub::e_Gen:
                        (*pub)->SetGen().SetAuthors().SetNames(edited_names);
                        break;
                    case CPub::e_Article:
                        (*pub)->SetArticle().SetAuthors().SetNames(edited_names);
                        break;
                    case CPub::e_Book:
                        (*pub)->SetBook().SetAuthors().SetNames(edited_names);
                        break;
                    case CPub::e_Proc:
                        (*pub)->SetProc().SetBook().SetAuthors().SetNames(edited_names);
                        break;
                    case CPub::e_Sub:
                        (*pub)->SetSub().SetAuthors().SetNames(edited_names);
                        break;
                    case CPub::e_Patent:
                        (*pub)->SetPatent().SetAuthors().SetNames(edited_names);
                        break;
                    case CPub::e_Man:
                        (*pub)->SetMan().SetCit().SetAuthors().SetNames(edited_names);
                        break;
                    default:
                        (*pub)->SetGen().SetAuthors().SetNames(edited_names);
                        break;
                    }
                    modified = true;
                }
                if (modified)
                {
                    CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(*seh_it, *desc, *replacement));
                    comp_cmd->AddCommand(*ecmd);
                    any_modified = true;
                }
            }
        }
        else if (const CSubmit_block* block = dynamic_cast<const CSubmit_block*>(obj_it->GetPointer())) {
            CRef<CSubmit_block> new_submit_block(new CSubmit_block);
            new_submit_block->Assign(*block);

            CAuth_list& authors = new_submit_block->SetCit().SetAuthors();
            authors.SetNames(edited_names);

            CChangeSubmitBlockCommand* cmd = new CChangeSubmitBlockCommand();
            CObject* actual = (CObject*)(block);
            cmd->Add(actual, CConstRef<CObject>(new_submit_block));
            comp_cmd->AddCommand(*cmd);
            any_modified = true;
        }

    }
    
    if (!any_modified)
    {
        comp_cmd.Reset();
        m_dlg->AppendToErrorMsg("No action.");
    }
    return comp_cmd;   
}

END_NCBI_SCOPE

