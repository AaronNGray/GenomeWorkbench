/*  $Id: create_gene_model_params_panel.cpp 42821 2019-04-18 19:32:56Z joukovv $
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

#include <gui/packages/pkg_alignment/create_gene_model_params_panel.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>

BEGIN_NCBI_SCOPE


SCreateGeneModelParams::SCreateGeneModelParams()
{
    Init();
}

void SCreateGeneModelParams::Init()
{
    m_CreateGene = true;
    m_CreateMrna = true;
    m_CreateCds = true;
    m_PropagateNcrnaFeats = true;
    m_GroupByGeneId = true;
    m_TranscribeMrna = false;
    m_TranslateCds = false;
    m_MergeResults = false;
    m_PropagateLocalIds = false;
}

void SCreateGeneModelParams::SetRegistryPath( const string& reg_path )
{
    m_RegPath = reg_path;
}

void SCreateGeneModelParams::LoadSettings()
{
}

void SCreateGeneModelParams::SaveSettings() const
{
}

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CCreateGeneModelParamsPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CCreateGeneModelParamsPanel, CAlgoToolManagerParamsPanel )

////@begin CCreateGeneModelParamsPanel event table entries
    EVT_CHECKBOX( ID_CHECK_GENE, CCreateGeneModelParamsPanel::OnCreateGeneClick )

    EVT_CHECKBOX( ID_PROPAGATE_NCRNA, CCreateGeneModelParamsPanel::OnCreateMrnaClick )

    EVT_CHECKBOX( ID_CHECK_MRNA, CCreateGeneModelParamsPanel::OnCreateMrnaClick )

    EVT_CHECKBOX( ID_CHECK_CDS, CCreateGeneModelParamsPanel::OnCreateCdsClick )

////@end CCreateGeneModelParamsPanel event table entries

END_EVENT_TABLE()

CCreateGeneModelParamsPanel::CCreateGeneModelParamsPanel()
    : m_Params(NULL)
    , m_InputObjects(NULL)
    , m_InputListDirty(true)
{
    Init();
}

CCreateGeneModelParamsPanel::CCreateGeneModelParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CCreateGeneModelParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCreateGeneModelParamsPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCreateGeneModelParamsPanel creation
    return true;
}

CCreateGeneModelParamsPanel::~CCreateGeneModelParamsPanel()
{
////@begin CCreateGeneModelParamsPanel destruction
////@end CCreateGeneModelParamsPanel destruction
}

void CCreateGeneModelParamsPanel::Init()
{
////@begin CCreateGeneModelParamsPanel member initialisation
    m_Params = NULL;
    m_ObjectList = NULL;
    m_CreateGene = NULL;
    m_PropagateNcrnaFeats = NULL;
    m_CreateMrna = NULL;
    m_TranscribeMrna = NULL;
    m_CreateCds = NULL;
    m_TranslateCds = NULL;
    m_GroupByGeneId = NULL;
    m_MergeToGenome = NULL;
    m_PropagateLocalIds = NULL;
////@end CCreateGeneModelParamsPanel member initialisation
}

void CCreateGeneModelParamsPanel::CreateControls()
{
////@begin CCreateGeneModelParamsPanel content construction
    CCreateGeneModelParamsPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Choose Alignments"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxGROW|wxALL, 5);

    m_ObjectList = new CObjectListWidget( itemStaticBoxSizer3->GetStaticBox(), ID_OBJLISTCTRL, wxDefaultPosition, wxSize(100, 200), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_ObjectList, 1, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 16);
    itemBoxSizer2->Add(itemFlexGridSizer5, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CreateGene = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECK_GENE, _("Create Gene Feature"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CreateGene->SetValue(true);
    itemFlexGridSizer5->Add(m_CreateGene, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PropagateNcrnaFeats = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_PROPAGATE_NCRNA, _("Propagate ncRNA Features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PropagateNcrnaFeats->SetValue(true);
    itemFlexGridSizer5->Add(m_PropagateNcrnaFeats, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CreateMrna = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECK_MRNA, _("Create mRNA Feature"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CreateMrna->SetValue(true);
    itemFlexGridSizer5->Add(m_CreateMrna, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TranscribeMrna = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECK_TRANSCRIPT, _("Copy Transcript from Genome"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TranscribeMrna->SetValue(false);
    itemFlexGridSizer5->Add(m_TranscribeMrna, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CreateCds = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECK_CDS, _("Create CDS Feature"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CreateCds->SetValue(true);
    itemFlexGridSizer5->Add(m_CreateCds, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TranslateCds = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECK_TRANSLATE, _("Translate CDS from Genome"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TranslateCds->SetValue(false);
    itemFlexGridSizer5->Add(m_TranslateCds, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByGeneId = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECK_GROUP, _("Group features by gene ID"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByGeneId->SetValue(true);
    itemFlexGridSizer5->Add(m_GroupByGeneId, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MergeToGenome = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECK_MERGE, _("Merge features to Genome"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MergeToGenome->SetValue(false);
    itemFlexGridSizer5->Add(m_MergeToGenome, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PropagateLocalIds = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_PROPAGATE_LOCAL_IDS, _("Generate Local IDs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PropagateLocalIds->SetValue(false);
    itemFlexGridSizer5->Add(m_PropagateLocalIds, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CCreateGeneModelParamsPanel content construction
}

wxBitmap CCreateGeneModelParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCreateGeneModelParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCreateGeneModelParamsPanel bitmap retrieval
}
wxIcon CCreateGeneModelParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCreateGeneModelParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCreateGeneModelParamsPanel icon retrieval
}


void CCreateGeneModelParamsPanel::SetParams(
    SCreateGeneModelParams* params, TConstScopedObjects* objects
){
    m_Params = params;
    m_InputObjects = objects;
}


bool CCreateGeneModelParamsPanel::TransferDataToWindow()
{
    if( m_InputListDirty ){
        m_ObjectList->SetObjects( *m_InputObjects );
        m_ObjectList->SelectAll();
        m_InputListDirty = false;
    }

    if( m_Params ){
        m_CreateGene->SetValue( m_Params->m_CreateGene );
        m_GroupByGeneId->SetValue( m_Params->m_GroupByGeneId );
        m_GroupByGeneId->Enable( m_CreateGene->IsChecked() );
        m_CreateMrna->SetValue( m_Params->m_CreateMrna );
        m_TranscribeMrna->SetValue (m_Params->m_TranscribeMrna );
        m_PropagateNcrnaFeats->SetValue(m_Params->m_PropagateNcrnaFeats);
        m_CreateCds->SetValue( m_Params->m_CreateCds );
        m_TranslateCds->SetValue( m_Params->m_TranslateCds );
        m_MergeToGenome->SetValue( m_Params->m_MergeResults );
        m_PropagateLocalIds->SetValue( m_Params->m_PropagateLocalIds );
    }

    return CAlgoToolManagerParamsPanel::TransferDataToWindow();
}


bool CCreateGeneModelParamsPanel::TransferDataFromWindow()
{
    if( CAlgoToolManagerParamsPanel::TransferDataFromWindow() ){
        if( m_Params ){
            m_ObjectList->GetSelection( m_Params->m_Alignments );

            m_Params->m_CreateGene = m_CreateGene->GetValue();
            m_Params->m_GroupByGeneId = m_GroupByGeneId->GetValue();
            m_Params->m_CreateMrna = m_CreateMrna->GetValue();
            m_Params->m_TranscribeMrna = m_TranscribeMrna->GetValue();
            m_Params->m_PropagateNcrnaFeats = m_PropagateNcrnaFeats->GetValue();
            m_Params->m_CreateCds = m_CreateCds->GetValue();
            m_Params->m_TranslateCds = m_TranslateCds->GetValue();
            m_Params->m_MergeResults = m_MergeToGenome->GetValue();
            m_Params->m_PropagateLocalIds = m_PropagateLocalIds->GetValue();
        }

        return true;
    }

    return false;
}

void CCreateGeneModelParamsPanel::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

static const char* kTableTag = "Table";

void CCreateGeneModelParamsPanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryReadView table_view = gui_reg.GetReadView(reg_path);
        m_ObjectList->LoadTableSettings(table_view);
    }
}


void CCreateGeneModelParamsPanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectList->SaveTableSettings(table_view);
    }
}

void CCreateGeneModelParamsPanel::OnCreateGeneClick( wxCommandEvent& event )
{
    m_GroupByGeneId->Enable( event.IsChecked() );
}

void CCreateGeneModelParamsPanel::OnCreateMrnaClick( wxCommandEvent& event )
{
    m_TranscribeMrna->Enable( event.IsChecked() );
}

void CCreateGeneModelParamsPanel::OnCreateCdsClick( wxCommandEvent& event )
{
    m_TranslateCds->Enable( event.IsChecked() );
}

void CCreateGeneModelParamsPanel::RestoreDefaults()
{
    if (!m_Params)
        return;
    m_Params->Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
