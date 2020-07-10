/*  $Id: wiggle_export_page.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
////@begin includes
////@end includes

#include "wiggle_export_page.hpp"
#include <gui/packages/pkg_sequence/bam_utils.hpp>
#include <objects/seq/Seq_annot.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/textctrl.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/artprov.h>
#include <wx/valnum.h>

#include <gui/widgets/object_list/object_list_widget.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CWiggleExportPage, wxPanel )

BEGIN_EVENT_TABLE( CWiggleExportPage, wxPanel )

////@begin CWiggleExportPage event table entries
    EVT_BUTTON( ID_BITMAPBUTTON, CWiggleExportPage::OnSelectFileClick )

////@end CWiggleExportPage event table entries

END_EVENT_TABLE()

CWiggleExportPage::CWiggleExportPage()
{
    Init();
}

CWiggleExportPage::CWiggleExportPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CWiggleExportPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWiggleExportPage creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CWiggleExportPage creation
    return true;
}

CWiggleExportPage::~CWiggleExportPage()
{
////@begin CWiggleExportPage destruction
////@end CWiggleExportPage destruction
}

void CWiggleExportPage::Init()
{
////@begin CWiggleExportPage member initialisation
    m_ObjectSel = NULL;
    m_BinSizePanel = NULL;
    m_BinSizeLabel = NULL;
    m_BinSizeCtrl = NULL;
    m_FromCtrl = NULL;
    m_ToCtrl = NULL;
    m_FileNameCtrl = NULL;
////@end CWiggleExportPage member initialisation
}

void CWiggleExportPage::CreateControls()
{    
////@begin CWiggleExportPage content construction
    CWiggleExportPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_ObjectSel = new CObjectListWidget( itemPanel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ObjectSel, 1, wxGROW|wxALL, 0);

    m_BinSizePanel = new wxStaticBox(itemPanel1, wxID_ANY, _("Recalculate Bin Size"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(m_BinSizePanel, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxGROW|wxALL, 0);

    m_BinSizeLabel = new wxStaticText( itemPanel1, wxID_STATIC, _("Graph bin size (bases):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add(m_BinSizeLabel, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    m_BinSizeCtrl = new wxSpinCtrl( itemPanel1, ID_SPINCTRL, _T("1000"), wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(60, -1)).x, -1), wxSP_ARROW_KEYS, 1, 1000000, 1000 );
    itemStaticBoxSizer4->Add(m_BinSizeCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Sequence Range (blank for the whole sequence)"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxHORIZONTAL);
    itemStaticBoxSizer7Static->Show(false);
    itemBoxSizer2->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("From:"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemStaticText8->Show(false);
    itemStaticBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    m_FromCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_FromCtrl->Show(false);
    itemStaticBoxSizer7->Add(m_FromCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("To:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText10->Show(false);
    itemStaticBoxSizer7->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    m_ToCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_ToCtrl->Show(false);
    itemStaticBoxSizer7->Add(m_ToCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("File name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileNameCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_FileNameCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton15 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton15->SetHelpText(_("Select ASN.1 File"));
    if (CWiggleExportPage::ShowToolTips())
        itemBitmapButton15->SetToolTip(_("Select ASN.1 File"));
    itemBoxSizer12->Add(itemBitmapButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_BinSizeCtrl->SetValidator( wxGenericValidator(& GetData().m_BinSize) );
    m_FromCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_From) );
    m_ToCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_To) );
    m_FileNameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_FileName) );
////@end CWiggleExportPage content construction
    
    m_SaveFile.reset(new CSaveFileHelper(this, *m_FileNameCtrl));

}

bool CWiggleExportPage::ShowToolTips()
{
    return true;
}
wxBitmap CWiggleExportPage::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}
wxIcon CWiggleExportPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWiggleExportPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CWiggleExportPage icon retrieval
}

static const char* kObjectList = "ObjectList";

void CWiggleExportPage::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CWiggleExportPage::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kObjectList);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectSel->SaveTableSettings(table_view);
    }
}

void CWiggleExportPage::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kObjectList);
        table_view = gui_reg.GetReadView(reg_path);
        m_ObjectSel->LoadTableSettings(table_view);
    }
}

void CWiggleExportPage::OnSelectFileClick( wxCommandEvent& event )
{
    _ASSERT(m_SaveFile);
    m_SaveFile->ShowSaveDialog(CFileExtensions::kWIG);
}

bool CWiggleExportPage::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    m_ObjectSel->SetObjects(m_data.m_Objects);
    m_ObjectSel->SelectAll();

    // Hide BinSize control if there are no BAM graph in the selected objects
    bool is_bam = false;
    for (auto&& obj : m_data.m_Objects) {
        auto seq_annot = dynamic_cast<const CSeq_annot*>(obj.object.GetPointer());
        if (!seq_annot)
            continue;
        string bam_data;
        string bam_index;
        string assembly;
        if (CBamUtils::GetCoverageGraphInfo(*seq_annot, bam_data, bam_index, assembly)) {
            is_bam = true;
            break;
        }
    }
    if (!is_bam) {
        m_BinSizeLabel->Hide();
        m_BinSizeCtrl->Hide();
        m_BinSizePanel->Hide();
    }
    return true;
}

bool CWiggleExportPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects objects;
    m_ObjectSel->GetSelection(objects);

    if (objects.empty()) {
        wxMessageBox(wxT("Please, select at least one object to export"), wxT("Error"),
            wxOK | wxICON_ERROR, this);
        FindWindow(ID_PANEL2)->SetFocus();
        return false;
    }

    GetData().m_Objects = objects;

    wxString path(GetData().GetFileName());
    _ASSERT(m_SaveFile);
    return m_SaveFile->Validate(path);
}

END_NCBI_SCOPE
