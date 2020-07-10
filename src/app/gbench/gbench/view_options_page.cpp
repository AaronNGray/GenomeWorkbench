/*  $Id: view_options_page.cpp 41347 2018-07-12 18:28:31Z evgeniev $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "view_options_page.hpp"

#include <gui/utils/extension_impl.hpp>
#include <gui/framework/options_dlg_extension.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/core/pt_utils.hpp>
#include <gui/core/project_tree_view.hpp>
#include <gui/core/project_tree_panel.hpp>
#include <gui/objutils/usage_report_job.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <connect/ncbi_conn_test.hpp>

#include "gbench_app.hpp"

#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/filefn.h>
#include <wx/file.h>

BEGIN_NCBI_SCOPE

wxDECLARE_APP(ncbi::CGBenchApp);

/*!
 * CViewOptionsPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CViewOptionsPage, wxPanel )


/*!
 * CViewOptionsPage event table definition
 */

BEGIN_EVENT_TABLE( CViewOptionsPage, wxPanel )

////@begin CViewOptionsPage event table entries
////@end CViewOptionsPage event table entries

END_EVENT_TABLE()


/*!
 * CViewOptionsPage constructors
 */

CViewOptionsPage::CViewOptionsPage()
{
    Init();
}

CViewOptionsPage::CViewOptionsPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CViewOptionsPage creator
 */

bool CViewOptionsPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CViewOptionsPage creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CViewOptionsPage creation
    return true;
}


/*!
 * CViewOptionsPage destructor
 */

CViewOptionsPage::~CViewOptionsPage()
{
////@begin CViewOptionsPage destruction
////@end CViewOptionsPage destruction
}


/*!
 * Member initialisation
 */

void CViewOptionsPage::Init()
{
    ////@begin CViewOptionsPage member initialisation
    m_DisableUpdates = false;
    m_HideDisabledProjectItems = false;
    m_LogStack = (CException::GetStackTraceLevel() == eDiag_Error);
    m_ReportUsage = true;
    m_ViewTitle = 0;
    m_EmuilateExtCheckBox = NULL;
    ////@end CViewOptionsPage member initialisation

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    m_ViewTitle = gui_reg.GetInt("GBENCH.Application.ViewOptions.ViewTitle", m_ViewTitle);
    int autoSaveInterval = gui_reg.GetInt("GBENCH.Application.ViewOptions.AutoSaveInterval", 60);
    m_AutoSaveInterval = ToWxString(NStr::NumericToString(autoSaveInterval / 60));
    m_HideDisabledProjectItems = gui_reg.GetBool("GBENCH.Application.ViewOptions.HideDisabledProjectItems", false);
    m_DisableUpdates = gui_reg.GetBool("GBENCH.Application.AutoUpdates.Disabled", false);
    m_ReportUsage = CUsageReportJob::IsEnabled();
}


static const wxChar* kEmulateExternalFname = wxT("<home>/emulate_external");

/*!
 * Control creation for CViewOptionsPage
 */

void CViewOptionsPage::CreateControls()
{    
////@begin CViewOptionsPage content construction
    CViewOptionsPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Auto Save interval, minutes (0 to disable):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemRadioBox6Strings;
    itemRadioBox6Strings.Add(_("&Content (Default)"));
    itemRadioBox6Strings.Add(_("Content &and View Type"));
    itemRadioBox6Strings.Add(_("&Detailed"));
    wxRadioBox* itemRadioBox6 = new wxRadioBox( itemPanel1, ID_RADIOBOX, _("View Title Options"), wxDefaultPosition, wxDefaultSize, itemRadioBox6Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox6->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox6, 0, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemPanel1, ID_DISABLE_UPDATES_CHECKBOX, _("Disable check for updates (not recommended)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox7, 0, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox8 = new wxCheckBox( itemPanel1, ID_HIDE_DISABLED_CHECKBOX, _("Hide disabled project items from the tree"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox8->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox8, 0, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemPanel1, ID_CHECKBOX2, _("Log call stack on error"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox9, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( itemPanel1, ID_REPORT_USAGE_CHECKBOX, _("Submit performance and usage data\n(helps to improve Genome Workbench)\n"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox10->SetValue(true);
    itemBoxSizer2->Add(itemCheckBox10, 0, wxGROW|wxALL, 5);

    m_EmuilateExtCheckBox = new wxCheckBox( itemPanel1, ID_EMULATE_EXTERNAL_CHECKBOX, _("Emulate non NCBI PC"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EmuilateExtCheckBox->SetValue(false);
    m_EmuilateExtCheckBox->Show(false);
    itemBoxSizer2->Add(m_EmuilateExtCheckBox, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    itemTextCtrl5->SetValidator( wxTextValidator(wxFILTER_DIGITS, & m_AutoSaveInterval) );
    itemRadioBox6->SetValidator( wxGenericValidator(& m_ViewTitle) );
    itemCheckBox7->SetValidator( wxGenericValidator(& m_DisableUpdates) );
    itemCheckBox8->SetValidator( wxGenericValidator(& m_HideDisabledProjectItems) );
    itemCheckBox9->SetValidator( wxGenericValidator(& m_LogStack) );
    itemCheckBox10->SetValidator( wxGenericValidator(& m_ReportUsage) );
////@end CViewOptionsPage content construction

    if (CConnTest::IsNcbiInhouseClient()) {
        m_EmuilateExtCheckBox->Show(true);
        wxString p = CSysPath::ResolvePathExisting(kEmulateExternalFname);
        m_EmuilateExtCheckBox->SetValue(!p.empty());
    }
}

bool CViewOptionsPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    gui_reg.Set("GBENCH.Application.ViewOptions.ViewTitle", m_ViewTitle);
    gui_reg.Set("GBENCH.Application.AutoUpdates.Disabled", m_DisableUpdates);
    CUsageReportJob::SetEnabled(m_ReportUsage);
    CUsageReportJob::SaveSettings();

    int autoSaveInterval = NStr::StringToNonNegativeInt(ToStdString(m_AutoSaveInterval));
    if (autoSaveInterval >= 0) {
        gui_reg.Set("GBENCH.Application.ViewOptions.AutoSaveInterval", autoSaveInterval*60);

        CGBenchApp& app = wxGetApp();
        if (autoSaveInterval == 0) {
            if (app.m_WsAutoSaver.get()) {
                app.m_WsAutoSaver->Stop();
                app.m_WsAutoSaver.reset();
                CWorkspaceAutoSaver::CleanUp();
            }
        } else {
            if (app.m_WsAutoSaver.get()) {
                if (autoSaveInterval*60 != app.m_WsAutoSaver->GetInterval())
                    app.m_WsAutoSaver->SetInterval((size_t)autoSaveInterval*60);
            } else {
                app.m_WsAutoSaver.reset(new CWorkspaceAutoSaver(*app.m_Workbench, (size_t)autoSaveInterval*60));
                app.m_WsAutoSaver->PostSave();
            }
        }
    }

    PT::sm_HideDisabledItems = m_HideDisabledProjectItems;

    IViewManagerService* view_srv = wxGetApp().m_Workbench->GetServiceByType<IViewManagerService>();
    if (view_srv) {
        IViewManagerService::TViews views;
        view_srv->GetViews(views);
        NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
            CProjectTreeView* projectTree = dynamic_cast<CProjectTreeView*>((*it).GetPointer());
            if (projectTree) {
                CProjectTreePanel* projectTreePanel = dynamic_cast<CProjectTreePanel*>(projectTree->GetWindow());
                if (projectTreePanel) projectTreePanel->UpdateDisabledItems();
                break;
            }
        }
    }

    CException::SetStackTraceLevel(m_LogStack ? eDiag_Error : eDiag_Critical);

    if (CConnTest::IsNcbiInhouseClient()) {
        bool showMsgBox = false;
        wxString f = CSysPath::ResolvePathExisting(kEmulateExternalFname);

        if (m_EmuilateExtCheckBox->GetValue()) {
            if (f.empty()) {
                showMsgBox = true;
                f = CSysPath::ResolvePath(kEmulateExternalFname);
                wxFile file;
                if (file.Create(f))
                    file.Close();
            }
        }
        else {
            if (!f.empty()) {
                wxRemoveFile(f);
                showMsgBox = true;
            }
        }

        if (showMsgBox) {
            wxMessageBox(wxT("Changes will take effect when you restart Genome Workbench."),
                wxT("Emulate non NCBI PC"), wxOK | wxICON_INFORMATION);
        }
    }

    return true;
}

/*!
 * Should we show tooltips?
 */

bool CViewOptionsPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CViewOptionsPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CViewOptionsPage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CViewOptionsPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CViewOptionsPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CViewOptionsPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CViewOptionsPage icon retrieval
}

class CViewOptionsDlgExtension :
        public CObject,
        public IExtension,
        public IOptionsDlgExtension
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const
        { return "options_dlg_page_view_options"; }
    virtual string  GetExtensionLabel() const
        { return ToStdString(SYMBOL_CVIEWOPTIONSPAGE_TITLE); }
    /// @}

    virtual wxWindow* CreateSettingsPage(wxWindow* parent)
        { return new CViewOptionsPage(parent); }

    virtual size_t GetPriority() const { return 1; }
    virtual string GetPageLabel() const { return ToStdString(SYMBOL_CVIEWOPTIONSPAGE_TITLE); }
};

static CExtensionDeclaration
    decl(EXT_POINT__OPTIONS_DLG_EXTENSION, new CViewOptionsDlgExtension());

END_NCBI_SCOPE
