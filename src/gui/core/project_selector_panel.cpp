/*  $Id: project_selector_panel.cpp 44243 2019-11-19 16:06:54Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/project_selector_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>
#include <gui/core/select_project_options.hpp>

#include <serial/iterator.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <corelib/ncbi_process.hpp>

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>

////@begin includes
////@end includes


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
SProjectSelectorParams::SProjectSelectorParams()
:   m_EnablePackaging(false),
    m_EnableDecideLater(true),
    m_EnableCreateSeparate(true),
    m_ProjectMode(eAddToExistingProject),
    m_SelectedProjectId(-1),
    m_Package(false),
    m_CreateFolder(false),
    m_FolderName("New Folder")
{
}


void SProjectSelectorParams::SelectProjectByObjects(TConstScopedObjects& objects,
                                                    CProjectService* srv)
{
    if (!objects.empty()) {
        CScope* scope = objects.front().scope;
        if (!scope)
            return;

        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
        if (doc)
            m_SelectedProjectId = doc->GetId();
    }
}


static const char* kModeTag = "Mode";
static const char* kCreateFolderTag = "CreateFolder";
static const char* kFolderNameTag = "FolderName";
static const char* kPIDTag = "PID";
static const char* kSelPrjIdTag = "Selected Project Id";

void SProjectSelectorParams::SaveSettings(CRegistryWriteView& view,
                                          const string& section) const
{
    view.Set(CGuiRegistryUtil::MakeKey(section, kModeTag), m_ProjectMode);
    view.Set(CGuiRegistryUtil::MakeKey(section, kCreateFolderTag), m_CreateFolder);
    view.Set(CGuiRegistryUtil::MakeKey(section, kFolderNameTag), m_FolderName);

    /// project IDs are valid only within a single application session
    /// so we use TPid to distinguish between the runs
    /// (this is not 100% guarantee but works for practical purposes)
    TPid pid = CCurrentProcess::GetPid();
    view.Set(CGuiRegistryUtil::MakeKey(section, kPIDTag), int(pid));
    view.Set(CGuiRegistryUtil::MakeKey(section, kSelPrjIdTag), int(m_SelectedProjectId));
}


void SProjectSelectorParams::LoadSettings(CRegistryReadView& view,
                                          const string& section)
{
    m_ProjectMode = (EMode) view.GetInt(CGuiRegistryUtil::MakeKey(section, kModeTag), m_ProjectMode);
    m_CreateFolder = view.GetBool(CGuiRegistryUtil::MakeKey(section, kCreateFolderTag), m_CreateFolder);
    m_FolderName = view.GetString(CGuiRegistryUtil::MakeKey(section, kFolderNameTag), m_FolderName);

    int pid = CCurrentProcess::GetPid();
    int saved_pid = view.GetInt(CGuiRegistryUtil::MakeKey(section, kPIDTag), 0);
    if(saved_pid == pid)    {
        m_SelectedProjectId =
            view.GetInt(CGuiRegistryUtil::MakeKey(section, kSelPrjIdTag), m_SelectedProjectId);
    }
}


void SProjectSelectorParams::ToLoadingOptions(CSelectProjectOptions& options)
{
    string folder_name = m_CreateFolder ? m_FolderName : kEmptyStr;

    switch(m_ProjectMode) {
    case SProjectSelectorParams::eCreateOneProject:
        options.Set_CreateNewProject(folder_name);
        break;

    case SProjectSelectorParams::eCreateSeparateProjects:
        options.Set_CreateSeparateProjects();
        break;

    case SProjectSelectorParams::eAddToExistingProject:
        options.Set_AddToExistingProject(m_SelectedProjectId, folder_name);
        break;

    default:
        _ASSERT(false);
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CProjectSelectorPanel

IMPLEMENT_DYNAMIC_CLASS( CProjectSelectorPanel, wxPanel )

BEGIN_EVENT_TABLE( CProjectSelectorPanel, wxPanel )

////@begin CProjectSelectorPanel event table entries
    EVT_INIT_DIALOG( CProjectSelectorPanel::OnInitDialog )

    EVT_RADIOBUTTON( ID_CREATE_NEW_BTN, CProjectSelectorPanel::OnCreateNewBtnSelected )

    EVT_RADIOBUTTON( ID_CREATE_DEP_BTN, CProjectSelectorPanel::OnCreateDepBtnSelected )

    EVT_RADIOBUTTON( ID_ADD_TO_EXIST_BTN, CProjectSelectorPanel::OnAddToExistBtnSelected )

    EVT_LISTBOX( ID_PROJECT_LIST, CProjectSelectorPanel::OnProjectListSelected )

    EVT_CHECKBOX( ID_CREATE_FOLDER_CHECK, CProjectSelectorPanel::OnCreateFolderCheckClick )

////@end CProjectSelectorPanel event table entries


END_EVENT_TABLE()

CProjectSelectorPanel::CProjectSelectorPanel()
{
    Init();
}

CProjectSelectorPanel::CProjectSelectorPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CProjectSelectorPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProjectSelectorPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProjectSelectorPanel creation
    return true;
}


CProjectSelectorPanel::~CProjectSelectorPanel()
{
////@begin CProjectSelectorPanel destruction
////@end CProjectSelectorPanel destruction
}


void CProjectSelectorPanel::Init()
{
////@begin CProjectSelectorPanel member initialisation
    m_CreateSeparateRadio = NULL;
    m_ExistingRadio = NULL;
    m_ProjectList = NULL;
    m_Separator = NULL;
    m_PackageCheck = NULL;
    m_FolderCheck = NULL;
    m_FolderInput = NULL;
////@end CProjectSelectorPanel member initialisation

    m_SimpleMode = false;

    m_ProjectService = NULL;
    m_SelProjectIndex = -1;
}


void CProjectSelectorPanel::SetProjectService(CProjectService* service)
{
    m_ProjectService  = service;

    x_FillProjectsList();
}


void CProjectSelectorPanel::SetParams(const SProjectSelectorParams& params)
{
    m_Params = params;
}


void CProjectSelectorPanel::GetParams(SProjectSelectorParams& params) const
{
    params = m_Params;
}


void CProjectSelectorPanel::CreateControls()
{
////@begin CProjectSelectorPanel content construction
    CProjectSelectorPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxRadioButton* itemRadioButton3 = new wxRadioButton( itemPanel1, ID_CREATE_NEW_BTN, _("Create a new Project"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    itemRadioButton3->SetValue(false);
    itemBoxSizer2->Add(itemRadioButton3, 0, wxALIGN_LEFT|wxALL, 5);

    m_CreateSeparateRadio = new wxRadioButton( itemPanel1, ID_CREATE_DEP_BTN, _("Create a separate Project for every item"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CreateSeparateRadio->SetValue(false);
    itemBoxSizer2->Add(m_CreateSeparateRadio, 0, wxALIGN_LEFT|wxALL, 5);

    m_ExistingRadio = new wxRadioButton( itemPanel1, ID_ADD_TO_EXIST_BTN, _("Add to an existing Project"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExistingRadio->SetValue(false);
    itemBoxSizer2->Add(m_ExistingRadio, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 1, wxGROW|wxALL, 0);

    itemBoxSizer6->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ProjectListStrings;
    m_ProjectList = new wxListBox( itemPanel1, ID_PROJECT_LIST, wxDefaultPosition, wxSize(200, 100), m_ProjectListStrings, wxLB_SINGLE );
    itemBoxSizer6->Add(m_ProjectList, 1, wxGROW|wxALL, 5);

    m_Separator = new wxStaticLine( itemPanel1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(m_Separator, 0, wxGROW|wxALL, 5);

    m_PackageCheck = new wxCheckBox( itemPanel1, ID_PACKAGE_CHECK, _("Package as a single item"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PackageCheck->SetValue(false);
    itemBoxSizer2->Add(m_PackageCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_FolderCheck = new wxCheckBox( itemPanel1, ID_CREATE_FOLDER_CHECK, _("Create a Folder for the items"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FolderCheck->SetValue(false);
    itemBoxSizer2->Add(m_FolderCheck, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxGROW|wxALL, 0);

    itemBoxSizer12->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FolderInput = new wxTextCtrl( itemPanel1, ID_FOLDER_INPUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_FolderInput, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemRadioButton3->SetValidator( wxGenericValidator(& m_CreateOption) );
    m_CreateSeparateRadio->SetValidator( wxGenericValidator(& m_SeparateOption) );
    m_ExistingRadio->SetValidator( wxGenericValidator(& m_ExistingOption) );
    m_PackageCheck->SetValidator( wxGenericValidator(& m_Params.m_Package) );
    m_FolderCheck->SetValidator( wxGenericValidator(& m_Params.m_CreateFolder) );
    m_FolderInput->SetValidator( wxGenericValidator(& m_FolderName) );
////@end CProjectSelectorPanel content construction
}


void CProjectSelectorPanel::SetSimpleMode()
{
    m_Separator->Hide();
    m_PackageCheck->Hide();
    m_FolderCheck->Hide();
    GetSizer()->Hide(GetSizer()->GetItemCount() - 1);
    //m_FolderInput->Hide();
    //m_LaterRadio->Hide();

    Layout();
}


void CProjectSelectorPanel::x_FillProjectsList()
{
    _ASSERT(m_ProjectService);

    m_ProjectIds.clear();
    m_ProjectList->Clear();

    CRef<CGBWorkspace> ws = m_ProjectService->GetGBWorkspace();
    if (!ws) return;

    CWorkspaceFolder& root_folder = ws->SetWorkspace();

    for (CTypeConstIterator<CGBProjectHandle> it(root_folder); it; ++it) {
        const CGBDocument* doc = dynamic_cast<const CGBDocument*>(&*it);
        if (doc && doc->IsLoaded()) {
		    m_ProjectIds.push_back(doc->GetId());
            const CProjectDescr& descr = doc->GetDescr();
            m_ProjectList->Append(ToWxString(descr.GetTitle()));
        }
    }
}


void CProjectSelectorPanel::OnCreateNewBtnSelected( wxCommandEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CProjectSelectorPanel::OnCreateDepBtnSelected( wxCommandEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CProjectSelectorPanel::OnAddToExistBtnSelected( wxCommandEvent& event )
{
    TransferDataFromWindow();
    //x_SelectProjectByParams();
    TransferDataToWindow();
}


void CProjectSelectorPanel::OnProjectListSelected( wxCommandEvent& event )
{
    m_ExistingRadio->SetValue(true);
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CProjectSelectorPanel::x_SelectProjectByParams()
{
    if(m_ExistingOption  &&  m_SelProjectIndex < 0  &&  ! m_ProjectIds.empty()) {
        m_SelProjectIndex = 0;
        for(  size_t i = 0;  i < m_ProjectIds.size();  i++  )   {
            if(m_ProjectIds[i] == m_Params.m_SelectedProjectId) {
                m_SelProjectIndex = (int)i;
                break;
            }
        }
    }
}


void CProjectSelectorPanel::OnCreateFolderCheckClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


bool CProjectSelectorPanel::TransferDataToWindow()
{
    typedef SProjectSelectorParams TParams;

    int prj_count = m_ProjectList->GetCount();

    // correct option if needed
    if( ! prj_count  &&  m_Params.m_ProjectMode == TParams::eAddToExistingProject)    {
        m_Params.m_ProjectMode = TParams::eCreateOneProject;
    }

    m_CreateOption = (m_Params.m_ProjectMode == SProjectSelectorParams::eCreateOneProject);
    m_SeparateOption = (m_Params.m_ProjectMode == SProjectSelectorParams::eCreateSeparateProjects);
    m_ExistingOption = (m_Params.m_ProjectMode == SProjectSelectorParams::eAddToExistingProject);

    // update Project List
    bool en_list = (prj_count > 0);
    m_ProjectList->Enable(en_list);

    x_SelectProjectByParams();

    int sel_index = (m_ExistingOption  &&  m_SelProjectIndex > -1) ?  m_SelProjectIndex : wxNOT_FOUND;
    m_ProjectList->SetSelection(sel_index);

    m_ExistingRadio->Enable( ! m_ProjectIds.empty());

    bool en_package = (m_SeparateOption  &&  m_Params.m_EnablePackaging);
    m_PackageCheck->Enable(en_package);

    m_CreateSeparateRadio->Enable(m_Params.m_EnableCreateSeparate);

    m_FolderInput->Enable(m_Params.m_CreateFolder);

    m_FolderName = ToWxString(m_Params.m_FolderName);
    return wxPanel::TransferDataToWindow();
}


bool CProjectSelectorPanel::TransferDataFromWindow()
{
    if(wxPanel::TransferDataFromWindow())   {
        if(m_ProjectList->IsEnabled())    {
            // save item selection, it can be removed
            m_SelProjectIndex = m_ProjectList->GetSelection();
            if(m_SelProjectIndex >= 0)    {
                m_Params.m_SelectedProjectId = m_ProjectIds[m_SelProjectIndex];
            }
        }
        if(m_CreateOption)  {
            m_Params.m_ProjectMode = SProjectSelectorParams::eCreateOneProject;
        } else if(m_SeparateOption)  {
            m_Params.m_ProjectMode = SProjectSelectorParams::eCreateSeparateProjects;
        } else {
            m_Params.m_ProjectMode = SProjectSelectorParams::eAddToExistingProject;
        }
        m_Params.m_FolderName = string(m_FolderName.ToUTF8());

        return true;
    }
    return false;
}


void CProjectSelectorPanel::OnInitDialog( wxInitDialogEvent& event )
{
    wxPanel::OnInitDialog(event);
}


bool CProjectSelectorPanel::ShowToolTips()
{
    return true;
}


wxBitmap CProjectSelectorPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectSelectorPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProjectSelectorPanel bitmap retrieval
}


wxIcon CProjectSelectorPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectSelectorPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProjectSelectorPanel icon retrieval
}

END_NCBI_SCOPE
