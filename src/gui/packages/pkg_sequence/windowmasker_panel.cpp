/*  $Id: windowmasker_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin, Yury Voronov
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence/windowmasker_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>
#include <gui/widgets/loaders/winmask_files.hpp>

#include <objmgr/util/sequence.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbireg.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/radiobox.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/imaglist.h>
#include <wx/filename.h>
#include <wx/dir.h>

/*!
 * CWindowMaskerPanel type definition
 */

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CWindowMaskerPanel, CAlgoToolManagerParamsPanel )


/*!
 * CWindowMaskerPanel event table definition
 */

BEGIN_EVENT_TABLE( CWindowMaskerPanel, CAlgoToolManagerParamsPanel )

////@begin CWindowMaskerPanel event table entries
    EVT_BUTTON( ID_BUTTON5, CWindowMaskerPanel::OnWMDownload )

////@end CWindowMaskerPanel event table entries

END_EVENT_TABLE()


CWindowMaskerPanel::TMaskerPaths CWindowMaskerPanel::sm_MaskerPaths;

/*!
 * CWindowMaskerPanel constructors
 */

CWindowMaskerPanel::CWindowMaskerPanel()
    : m_AutoDelete(this)
{
    Init();
}

CWindowMaskerPanel::CWindowMaskerPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_AutoDelete(this)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CWindowMaskerPanel creator
 */

bool CWindowMaskerPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWindowMaskerPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CWindowMaskerPanel creation
    return true;
}


/*!
 * CWindowMaskerPanel destructor
 */

CWindowMaskerPanel::~CWindowMaskerPanel()
{
////@begin CWindowMaskerPanel destruction
////@end CWindowMaskerPanel destruction
}


/*!
 * Member initialisation
 */

void CWindowMaskerPanel::Init()
{
////@begin CWindowMaskerPanel member initialisation
    m_ObjectList = NULL;
    m_WMStaticBoxSizer = NULL;
    m_WMStatic = NULL;
    m_WMTaxIds = NULL;
    m_WMDownload = NULL;
////@end CWindowMaskerPanel member initialisation
}


/*!
 * Control creation for CWindowMaskerPanel
 */

void CWindowMaskerPanel::CreateControls()
{
////@begin CWindowMaskerPanel content construction
    CWindowMaskerPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    m_ObjectList = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_COBJECTLISTWIDGET, wxDefaultPosition, wxSize(300, 150), wxLC_REPORT );
    itemBoxSizer2->Add(m_ObjectList, 1, wxGROW|wxALL, 5);

    m_WMStaticBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_WMStaticBoxSizer, 0, wxGROW|wxALL, 5);

    m_WMStatic = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Mask using parameters for"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WMStaticBoxSizer->Add(m_WMStatic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_WMTaxIdsStrings;
    m_WMTaxIds = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, m_WMTaxIdsStrings, 0 );
    m_WMStaticBoxSizer->Add(m_WMTaxIds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WMStaticBoxSizer->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WMDownload = new wxButton( itemCAlgoToolManagerParamsPanel1, ID_BUTTON5, _("Configure..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_WMStaticBoxSizer->Add(m_WMDownload, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Objects to Create"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemComboBox11Strings;
    itemComboBox11Strings.Add(_("Features representing masked regions"));
    itemComboBox11Strings.Add(_("A sequence location representing masked regions"));
    itemComboBox11Strings.Add(_("\"Both features and sequence location"));
    wxComboBox* itemComboBox11 = new wxComboBox( itemCAlgoToolManagerParamsPanel1, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox11Strings, wxCB_READONLY );
    itemBoxSizer9->Add(itemComboBox11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox12 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_LOCALRUN, _("Run tool as standalone"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox12->SetValue(true);
    itemBoxSizer2->Add(itemCheckBox12, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    itemComboBox11->SetValidator( wxGenericValidator(& GetData().m_OutputType) );
    itemCheckBox12->SetValidator( wxGenericValidator(& GetData().m_Standalone) );
////@end CWindowMaskerPanel content construction  

    wxStaticText* loadingText = new wxStaticText(itemCAlgoToolManagerParamsPanel1, ID_LOADING_TEXT, wxT("Windomasker tax ID: none. Loading..."), wxDefaultPosition, wxDefaultSize, 0);
    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    loadingText->SetFont(bold);
    loadingText->SetForegroundColour(*wxBLACK);
    m_WMStaticBoxSizer->Add(loadingText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_WMStaticBoxSizer->Hide(loadingText);

    m_WMStaticBoxSizer->Add(1, m_WMDownload->GetSize().GetHeight(), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    CIndProgressBar* progress = new CIndProgressBar(itemCAlgoToolManagerParamsPanel1, ID_LOADING_PROGRESS, wxDefaultPosition, 100);
    m_WMStaticBoxSizer->Add(progress, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_WMStaticBoxSizer->Hide(progress);
}


/*!
 * Should we show tooltips?
 */

bool CWindowMaskerPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWindowMaskerPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWindowMaskerPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CWindowMaskerPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWindowMaskerPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWindowMaskerPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CWindowMaskerPanel icon retrieval
}

void CWindowMaskerPanel::OnWMDownload(wxCommandEvent& event)
{
    CWinMaskerFileStorage::GetInstance().ShowOptionsDlg();
    x_InitTaxons();
}

static const char *kTableTag = "Table";

void CWindowMaskerPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectList->SaveTableSettings(table_view);
    }
}

void CWindowMaskerPanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);
        table_view = gui_reg.GetReadView(reg_path);
        m_ObjectList->LoadTableSettings(table_view);
    }
}


bool CWindowMaskerPanel::TransferDataToWindow()
{
    if (m_InputObjects) {
        m_ObjectList->SetObjects(*m_InputObjects);
        m_ObjectList->SelectAll();
    }

    x_InitTaxons();

    return wxPanel::TransferDataToWindow();
}

void CWindowMaskerPanel::x_InitTaxons()
{
    if (CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, true, GetData().GetTaxId()))
        x_ShowWM(true);
    else
        x_ShowWM(false);
}

void CWindowMaskerPanel::TaxonsLoaded(bool local)
{
    if (!local) return;
    CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, true, GetData().GetTaxId());
    x_ShowWM(true);
}

void CWindowMaskerPanel::x_ShowWM(bool show)
{
    m_WMStaticBoxSizer->Show(FindWindow(ID_LOADING_TEXT), !show);
    m_WMStaticBoxSizer->Show(FindWindow(ID_LOADING_PROGRESS), !show);

    m_WMStaticBoxSizer->Show(m_WMStatic, show);
    m_WMStaticBoxSizer->Show(m_WMTaxIds, show);
    m_WMStaticBoxSizer->Show(m_WMDownload, show);
    GetSizer()->Layout();
}


/*!
 * Transfer data from the window
 */

bool CWindowMaskerPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection;
    m_ObjectList->GetSelection(selection);
    if (selection.empty()) {
        wxMessageBox(
            wxT("Please, select sequences to mask."), wxT("Error"),
            wxOK | wxICON_ERROR, this
            );
        m_ObjectList->SetFocus();
        return false;
    }
    GetData().SetObjects() = selection;
    
    int taxId = 0, index = m_WMTaxIds->GetSelection();
    if (wxNOT_FOUND != index)
        taxId = (int)(size_t)m_WMTaxIds->GetClientData(index);

    if (taxId <= 0) {
        wxMessageBox(
            wxT("Please, select organism."), wxT("Error"),
            wxOK | wxICON_ERROR, this
            );
        m_WMTaxIds->SetFocus();
        return false;
    }

    GetData().SetTaxId(taxId);

    return true;
}

void CWindowMaskerPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
