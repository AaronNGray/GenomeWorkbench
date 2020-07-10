/*  $Id: cleanup_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_sequence/cleanup_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/radiobox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images


/*!
 * CCleanupPanel type definition
 */

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CCleanupPanel, CAlgoToolManagerParamsPanel )


/*!
 * CCleanupPanel event table definition
 */

BEGIN_EVENT_TABLE( CCleanupPanel, CAlgoToolManagerParamsPanel )

////@begin CCleanupPanel event table entries
////@end CCleanupPanel event table entries

END_EVENT_TABLE()


/*!
 * CCleanupPanel constructors
 */

CCleanupPanel::CCleanupPanel() : m_InputObjects()
{
    Init();
}

CCleanupPanel::CCleanupPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_InputObjects()
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CCleanupPanel creator
 */

bool CCleanupPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCleanupPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCleanupPanel creation
    return true;
}


/*!
 * CCleanupPanel destructor
 */

CCleanupPanel::~CCleanupPanel()
{
////@begin CCleanupPanel destruction
////@end CCleanupPanel destruction
}


/*!
 * Member initialisation
 */

void CCleanupPanel::Init()
{
////@begin CCleanupPanel member initialisation
    m_ObjectList = NULL;
////@end CCleanupPanel member initialisation
}


/*!
 * Control creation for CCleanupPanel
 */

void CCleanupPanel::CreateControls()
{
////@begin CCleanupPanel content construction
    CCleanupPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Objects for Cleanup"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    m_ObjectList = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_LISTCTRL1, wxDefaultPosition, itemCAlgoToolManagerParamsPanel1->ConvertDialogToPixels(wxSize(300, 150)), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_ObjectList, 1, wxGROW|wxALL, 5);

    wxArrayString itemRadioBox5Strings;
    itemRadioBox5Strings.Add(_("&Basic"));
    itemRadioBox5Strings.Add(_("&Extended"));
    wxRadioBox* itemRadioBox5 = new wxRadioBox( itemCAlgoToolManagerParamsPanel1, ID_RADIOBOX1, _("Cleanup Mode"), wxDefaultPosition, wxDefaultSize, itemRadioBox5Strings, 1, wxRA_SPECIFY_ROWS );
    itemRadioBox5->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox5, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    itemRadioBox5->SetValidator( wxGenericValidator(& GetData().m_CleanupMode) );
////@end CCleanupPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CCleanupPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCleanupPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCleanupPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCleanupPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCleanupPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCleanupPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCleanupPanel icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CCleanupPanel::TransferDataToWindow()
{
    if (m_InputObjects) {
        m_ObjectList->SetObjects(*m_InputObjects);
        m_ObjectList->SelectAll();
    }

    return wxPanel::TransferDataToWindow();
}

/*!
 * Transfer data from the window
 */

bool CCleanupPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_ObjectList->GetSelection(GetData().m_Objects);
    if (GetData().m_Objects.empty()) {
        wxMessageBox(wxT("Please select objects for cleanup."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_ObjectList->SetFocus();
        return false;
    }

    return true;
}

static const char *kTableTag = "Table";

void CCleanupPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectList->SaveTableSettings(table_view);
    }
}

void CCleanupPanel::LoadSettings()
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

void CCleanupPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
