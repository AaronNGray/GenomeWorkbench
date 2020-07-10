/*  $Id: generic_report_dlg.cpp 41125 2018-05-29 19:08:26Z filippov $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE

/*!
 * CGenericReportDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGenericReportDlg, wxFrame )


/*!
 * CGenericReportDlg event table definition
 */

BEGIN_EVENT_TABLE( CGenericReportDlg, wxFrame )
    EVT_CLOSE ( CGenericReportDlg::OnCloseWindow )
END_EVENT_TABLE()


/*!
 * CGenericReportDlg constructors
 */

CGenericReportDlg::CGenericReportDlg()
: m_Workbench(0)
{
    Init();
}

CGenericReportDlg::CGenericReportDlg( wxWindow* parent, IWorkbench* workbench, 
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(workbench)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    SetRegistryPath("GenericReportDialog");
    LoadSettings();
}


/*!
 * CGenericReportDlg creator
 */

bool CGenericReportDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGenericReportDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );
	wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
	if (attr.colBg != wxNullColour)
	    SetOwnBackgroundColour(attr.colBg);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CGenericReportDlg creation
    return true;
}


/*!
 * CGenericReportDlg destructor
 */

CGenericReportDlg::~CGenericReportDlg()
{
    SaveSettings();
}

static const char* kCaseSensitive = "Search mode";

void CGenericReportDlg::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CGenericReportDlg::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        
    if (m_GenericPanel)
        view.Set(kCaseSensitive, m_GenericPanel->GetFindMode());
    
}


void CGenericReportDlg::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
    if (m_GenericPanel)
        m_GenericPanel->SetFindMode(view.GetInt(kCaseSensitive, 1));
}

/*!
 * Member initialisation
 */

void CGenericReportDlg::Init()
{
    m_GenericPanel = NULL;
}


/*!
 * Control creation for CGenericReportDlg
 */

void CGenericReportDlg::CreateControls()
{    
    CGenericReportDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_GenericPanel = new CGenericPanel(itemDialog1, m_Workbench, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_GenericPanel, 1, wxGROW|wxALL, 5);
}


void CGenericReportDlg::SetWorkDir(const wxString& workDir)
{
    if (m_GenericPanel)
        m_GenericPanel->SetWorkDir(workDir);
}


/*!
 * Should we show tooltips?
 */

bool CGenericReportDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGenericReportDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGenericReportDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGenericReportDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGenericReportDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGenericReportDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGenericReportDlg icon retrieval
}


void CGenericReportDlg::SetLineSpacing(int spacing)
{
    m_GenericPanel->SetLineSpacing(spacing);
}


void CGenericReportDlg::SetText( const wxString& text )
{
    m_GenericPanel->SetText(text);
}

void CGenericReportDlg::OnCloseWindow(wxCloseEvent& event)
{
    Destroy();
}

void CGenericReportDlg::SetRefresh( IRefreshCntrl * refresh )
{
    m_GenericPanel->SetRefresh(refresh);
}

void CGenericReportDlg::RefreshData()
{
    m_GenericPanel->RefreshData();
}


////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS( CGenericReportDlgModal, wxDialog )


/*!
 * CGenericReportDlgModal event table definition
 */

BEGIN_EVENT_TABLE( CGenericReportDlgModal, wxDialog )
    EVT_CLOSE ( CGenericReportDlgModal::OnCloseWindow )
END_EVENT_TABLE()


/*!
 * CGenericReportDlgModal constructors
 */

CGenericReportDlgModal::CGenericReportDlgModal()
: m_Workbench(0)
{
    Init();
}

CGenericReportDlgModal::CGenericReportDlgModal( wxWindow* parent, IWorkbench* workbench, 
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(workbench)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CGenericReportDlgModal creator
 */

bool CGenericReportDlgModal::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGenericReportDlgModal creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );
	wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
	if (attr.colBg != wxNullColour)
	    SetOwnBackgroundColour(attr.colBg);
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CGenericReportDlgModal creation
    return true;
}


/*!
 * CGenericReportDlgModal destructor
 */

CGenericReportDlgModal::~CGenericReportDlgModal()
{
}


/*!
 * Member initialisation
 */

void CGenericReportDlgModal::Init()
{
    m_GenericPanel = NULL;
}


/*!
 * Control creation for CGenericReportDlgModal
 */

void CGenericReportDlgModal::CreateControls()
{    
    CGenericReportDlgModal* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_GenericPanel = new CGenericPanel(itemDialog1, m_Workbench, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_GenericPanel, 1, wxGROW|wxALL, 5);
}

/*!
 * Should we show tooltips?
 */

bool CGenericReportDlgModal::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGenericReportDlgModal::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGenericReportDlgModal bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGenericReportDlgModal bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGenericReportDlgModal::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGenericReportDlgModal icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGenericReportDlgModal icon retrieval
}



void CGenericReportDlgModal::SetText( const wxString& text )
{
    m_GenericPanel->SetText(text);
}

void CGenericReportDlgModal::OnCloseWindow(wxCloseEvent& event)
{
    Destroy();
}

void CGenericReportDlgModal::SetRefresh( IRefreshCntrl * refresh )
{
    m_GenericPanel->SetRefresh(refresh);
}

void CGenericReportDlgModal::RefreshData()
{
    m_GenericPanel->RefreshData();
}
    
END_NCBI_SCOPE

