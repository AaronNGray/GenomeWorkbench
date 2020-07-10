/*  $Id: select_assembly_dialog.cpp 39749 2017-11-01 14:23:15Z katargir $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/select_assembly_dialog.hpp>

#include <gui/widgets/loaders/assembly_list_panel.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/checkbox.h>
#include <wx/valgen.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CSelectAssemblyDialog, CDialog )

BEGIN_EVENT_TABLE( CSelectAssemblyDialog, CDialog )

////@begin CSelectAssemblyDialog event table entries
    EVT_INIT_DIALOG( CSelectAssemblyDialog::OnInitDialog )

////@end CSelectAssemblyDialog event table entries

END_EVENT_TABLE()

CSelectAssemblyDialog::CSelectAssemblyDialog()
{
    Init();
}

CSelectAssemblyDialog::CSelectAssemblyDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CSelectAssemblyDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSelectAssemblyDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSelectAssemblyDialog creation

    return true;
}

CSelectAssemblyDialog::~CSelectAssemblyDialog()
{
////@begin CSelectAssemblyDialog destruction
////@end CSelectAssemblyDialog destruction
}

void CSelectAssemblyDialog::Init()
{
////@begin CSelectAssemblyDialog member initialisation
    m_AssemblyPanel = NULL;
////@end CSelectAssemblyDialog member initialisation
}

void CSelectAssemblyDialog::CreateControls()
{    
////@begin CSelectAssemblyDialog content construction
    CSelectAssemblyDialog* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_AssemblyPanel = new CAssemblyListPanel( itemCDialog1, ID_SELECTASSEMBLYPANEL, wxDefaultPosition, wxSize(400, 300), wxNO_BORDER|wxFULL_REPAINT_ON_RESIZE|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_AssemblyPanel, 1, wxGROW, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine4, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

////@end CSelectAssemblyDialog content construction

    m_AssemblyPanel->SetMultiSelection(false);
}

bool CSelectAssemblyDialog::ShowToolTips()
{
    return true;
}
wxBitmap CSelectAssemblyDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSelectAssemblyDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSelectAssemblyDialog bitmap retrieval
}
wxIcon CSelectAssemblyDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSelectAssemblyDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSelectAssemblyDialog icon retrieval
}

void CSelectAssemblyDialog::OnInitDialog( wxInitDialogEvent& event )
{
    CDialog::OnInitDialog(event);
}

bool CSelectAssemblyDialog::TransferDataFromWindow()
{
    return m_AssemblyPanel->IsInputValid();
}

void CSelectAssemblyDialog::SetAssmSearchTerm(const string& str) 
{
    m_AssemblyPanel->SetSearchTerm(str);
}

string CSelectAssemblyDialog::GetAssmSearchTerm() const
{
    return m_AssemblyPanel->GetSearchTerm();
}

string CSelectAssemblyDialog::GetSelectedAssembly(string& name, 
                                                  string& description) const
{    
    vector<string> accs = m_AssemblyPanel->GetSelectedAssemblies();

    if (accs.size() >= 1) {
        m_AssemblyPanel->GetAssmNameAndDesc(accs[0], name, description);
        return accs[0];
    }

    return "";
}

//static const char* kSelOptionTag = "SelectedOption";
//static const char* kDefaultItem = "DefaultView.";

void CSelectAssemblyDialog::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_AssemblyPanel->SetRegistryPath(m_RegPath + ".AssemblyPanel");

}

void CSelectAssemblyDialog::x_SaveSettings(CRegistryWriteView view) const
{
    m_AssemblyPanel->SaveSettings();
}

void CSelectAssemblyDialog::x_LoadSettings(const CRegistryReadView& view)
{  
    m_AssemblyPanel->LoadSettings();
}

END_NCBI_SCOPE
