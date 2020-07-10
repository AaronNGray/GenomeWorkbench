/*  $Id: tree_builder_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin, Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/packages/pkg_alignment/tree_builder_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>

////@begin includes
////@end includes


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CTreeBuilderPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CTreeBuilderPanel, CAlgoToolManagerParamsPanel )

////@begin CTreeBuilderPanel event table entries
////@end CTreeBuilderPanel event table entries

END_EVENT_TABLE()

static const string sc_JukesCantor("Jukes-Cantor (DNA)");
static const string sc_Kimura("Kimura (protein)");
static const string sc_Poisson("Poisson (protein)");

static const string sc_FastMe("Fast Minimum Evolution");
static const string sc_Nj("Neighbor Joining");

static const string sc_SeqId("Sequence ID");
static const string sc_TaxName("Taxonomic Name (if available)");
static const string sc_SeqTitle("Sequence Title (if available)");

CTreeBuilderPanel::CTreeBuilderPanel() : m_InputObjects()
{
    Init();
}

CTreeBuilderPanel::CTreeBuilderPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_InputObjects()
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CTreeBuilderPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTreeBuilderPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTreeBuilderPanel creation
    return true;
}

CTreeBuilderPanel::~CTreeBuilderPanel()
{
////@begin CTreeBuilderPanel destruction
////@end CTreeBuilderPanel destruction
}

void CTreeBuilderPanel::Init()
{
////@begin CTreeBuilderPanel member initialisation
    m_Seq1List = NULL;    
////@end CTreeBuilderPanel member initialisation
}

void CTreeBuilderPanel::CreateControls()
{
////@begin CTreeBuilderPanel content construction
    CTreeBuilderPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, wxT("Input Alignments"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    m_Seq1List = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_LISTCTRL3, wxDefaultPosition, itemCAlgoToolManagerParamsPanel1->ConvertDialogToPixels(wxSize(300, 75)), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_Seq1List, 1, wxGROW|wxALL, 0);

    
    wxFlexGridSizer* itemFlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer7->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer7, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, wxT("Distance Method"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    //wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL2, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    //itemFlexGridSizer7->Add(itemTextCtrl9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxArrayString distMethod;
    distMethod.Add(ToWxString(sc_Kimura));
    distMethod.Add(ToWxString(sc_Poisson));
    distMethod.Add(ToWxString(sc_JukesCantor));
    m_pDistance = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL2, wxDefaultPosition, wxDefaultSize, distMethod, 0 );
    m_pDistance->SetSelection(1);    
    itemFlexGridSizer7->Add(m_pDistance, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    

    wxStaticText* itemStaticText10 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, wxT("Tree Construct Method"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    //wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL3, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    //itemFlexGridSizer7->Add(itemTextCtrl11, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxArrayString constructMethod;
    constructMethod.Add(ToWxString(sc_Nj));
    constructMethod.Add(ToWxString(sc_FastMe));    
    m_pConstruct = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL3, wxDefaultPosition, wxDefaultSize, constructMethod, 0 );
    m_pConstruct->SetSelection(0);    
    itemFlexGridSizer7->Add(m_pConstruct, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    

    wxStaticText* itemStaticText12 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Labels for Leaf Nodes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    //wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL4, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    //itemFlexGridSizer7->Add(itemTextCtrl13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxArrayString labelsType;
    labelsType.Add(ToWxString(sc_SeqId));
    labelsType.Add(ToWxString(sc_TaxName));    
    labelsType.Add(ToWxString(sc_SeqTitle));    
    m_pLabels = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL4, wxDefaultPosition, wxDefaultSize, labelsType, 0 );
    m_pLabels->SetSelection(0);    
    itemFlexGridSizer7->Add(m_pLabels, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
////@end CTreeBuilderPanel content construction
}

bool CTreeBuilderPanel::ShowToolTips()
{
    return true;
}
wxBitmap CTreeBuilderPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTreeBuilderPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTreeBuilderPanel bitmap retrieval
}
wxIcon CTreeBuilderPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTreeBuilderPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTreeBuilderPanel icon retrieval
}
bool CTreeBuilderPanel::TransferDataToWindow()
{
    if (m_InputObjects) {
        m_Seq1List->SetObjects(*m_InputObjects);
        if (m_InputObjects->size() > 0)
            m_Seq1List->SelectAll();
    }

    return wxPanel::TransferDataToWindow();
}
bool CTreeBuilderPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection;

    m_Seq1List->GetSelection(selection);
    if (selection.empty()) {
        wxMessageBox(wxT("Please select first alignment."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_Seq1List->SetFocus();
        return false;
    }
    m_data.SetObject() = selection[0];
    m_data.SetDistanceMethod(m_pDistance->GetStringSelection());
    m_data.SetConstructMethod(m_pConstruct->GetStringSelection());
    m_data.SetLeafLabels(m_pLabels->GetStringSelection());

    selection.clear();
    
    return true;
}

static const char *kTable1Tag = "Table1";

void CTreeBuilderPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable1Tag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_Seq1List->SaveTableSettings(table_view);
    }
}

void CTreeBuilderPanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable1Tag);
        table_view = gui_reg.GetReadView(reg_path);
        m_Seq1List->LoadTableSettings(table_view);
    }
}

void CTreeBuilderPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
