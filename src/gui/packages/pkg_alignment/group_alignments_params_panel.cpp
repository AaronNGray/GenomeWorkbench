/*  $Id: group_alignments_params_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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

// Generated by DialogBlocks (unregistered), 17/07/2008 12:36:52

#include <ncbi_pch.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes


#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/packages/pkg_alignment/group_alignments_params_panel.hpp>

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CGroupAlignmentsParamsPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CGroupAlignmentsParamsPanel, CAlgoToolManagerParamsPanel )

////@begin CGroupAlignmentsParamsPanel event table entries
////@end CGroupAlignmentsParamsPanel event table entries

END_EVENT_TABLE()


SGroupAlignmentsParams::SGroupAlignmentsParams()
{
    Init();
}

void SGroupAlignmentsParams::Init()
{
    m_GroupBySeqId = false;
    m_GroupByStrand = false;
    m_GroupByTaxId = false;
    m_GroupByLikeTaxId = true;
    m_GroupByRefSeq = true;
    m_GroupByPredictedRefSeq = false;
    m_GroupByEST = true;
    m_GroupByGenBank = true;
    m_GroupByWGS = false;
    m_GroupByHTGS = false;
    m_GroupByPatent = false;
}

void SGroupAlignmentsParams::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

static const char* kGroupByEST = "GroupByEST";
static const char* kGroupByHTGS = "GroupByHTGS";
static const char* kGroupByLikeTaxId = "GroupByLikeTaxId";
static const char* kGroupByPatent = "GroupByPatent";
static const char* kGroupByPredictedRefSeq = "GroupByPredictedRefSeq";
static const char* kGroupByRefSeq = "GroupByRefSeq";
static const char* kGroupBySeqId = "GroupBySeqId";
static const char* kGroupByStrand = "GroupByStrand";
static const char* kGroupByTaxId = "GroupByTaxId";
static const char* kGroupByWGS = "GroupByWGS";

void SGroupAlignmentsParams::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kGroupByEST, m_GroupByEST);
        view.Set(kGroupByHTGS, m_GroupByHTGS);
        view.Set(kGroupByLikeTaxId, m_GroupByLikeTaxId);
        view.Set(kGroupByPatent, m_GroupByPatent);
        view.Set(kGroupByPredictedRefSeq, m_GroupByPredictedRefSeq);
        view.Set(kGroupByRefSeq, m_GroupByRefSeq);
        view.Set(kGroupBySeqId, m_GroupBySeqId);
        view.Set(kGroupByStrand, m_GroupByStrand);
        view.Set(kGroupByTaxId, m_GroupByTaxId);
        view.Set(kGroupByWGS, m_GroupByWGS);
    }
}


void SGroupAlignmentsParams::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_GroupByEST = view.GetBool(kGroupByEST, m_GroupByEST);
        m_GroupByHTGS = view.GetBool(kGroupByHTGS, m_GroupByHTGS);
        m_GroupByHTGS = view.GetBool(kGroupByHTGS, m_GroupByHTGS);
        m_GroupByLikeTaxId = view.GetBool(kGroupByLikeTaxId, m_GroupByLikeTaxId);
        m_GroupByPatent = view.GetBool(kGroupByPatent, m_GroupByPatent);
        m_GroupByPredictedRefSeq = view.GetBool(kGroupByPredictedRefSeq, m_GroupByPredictedRefSeq);
        m_GroupByRefSeq = view.GetBool(kGroupByRefSeq, m_GroupByRefSeq);
        m_GroupBySeqId = view.GetBool(kGroupBySeqId, m_GroupBySeqId);
        m_GroupByTaxId = view.GetBool(kGroupByTaxId, m_GroupByTaxId);
        m_GroupByWGS = view.GetBool(kGroupByWGS, m_GroupByWGS);
    }
}







CGroupAlignmentsParamsPanel::CGroupAlignmentsParamsPanel()
{
    Init();
}

CGroupAlignmentsParamsPanel::CGroupAlignmentsParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CGroupAlignmentsParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGroupAlignmentsParamsPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGroupAlignmentsParamsPanel creation
    return true;
}

CGroupAlignmentsParamsPanel::~CGroupAlignmentsParamsPanel()
{
////@begin CGroupAlignmentsParamsPanel destruction
////@end CGroupAlignmentsParamsPanel destruction
}

void CGroupAlignmentsParamsPanel::Init()
{
////@begin CGroupAlignmentsParamsPanel member initialisation
    m_ObjectList = NULL;
    m_GroupBySeqId = NULL;
    m_GroupByStrand = NULL;
    m_GroupByTaxId = NULL;
    m_GroupByLikeTaxId = NULL;
    m_GroupByRefSeq = NULL;
    m_GroupByEST = NULL;
    m_GroupByPredictedRefSeq = NULL;
    m_GroupByGenBank = NULL;
    m_GroupByWGS = NULL;
    m_GroupByHTGS = NULL;
    m_GroupByPatent = NULL;
////@end CGroupAlignmentsParamsPanel member initialisation
    m_Params = NULL;
    m_InputObjects = NULL;
    m_InputListDirty = true;
}

void CGroupAlignmentsParamsPanel::CreateControls()
{
////@begin CGroupAlignmentsParamsPanel content construction
    CGroupAlignmentsParamsPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Choose a set of alignments"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    m_ObjectList = new CObjectListWidget( itemStaticBoxSizer3->GetStaticBox(), ID_LISTCTRL, wxDefaultPosition, wxSize(100, 200), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_ObjectList, 1, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_GroupBySeqId = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_SEQID, _("Group by sequence identifier"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupBySeqId->SetValue(false);
    itemFlexGridSizer5->Add(m_GroupBySeqId, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByStrand = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_STRAND, _("Group by sequence strand"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByStrand->SetValue(false);
    itemFlexGridSizer5->Add(m_GroupByStrand, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByTaxId = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_TAXID, _("Group by taxonomic ID"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByTaxId->SetValue(false);
    itemFlexGridSizer5->Add(m_GroupByTaxId, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByLikeTaxId = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_SIMILAR_TAXID, _("Separate 'like' taxonomic IDs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByLikeTaxId->SetValue(true);
    itemFlexGridSizer5->Add(m_GroupByLikeTaxId, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByRefSeq = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_REFSEQ, _("Group RefSeq alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByRefSeq->SetValue(true);
    itemFlexGridSizer5->Add(m_GroupByRefSeq, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByEST = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_EST, _("Group EST alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByEST->SetValue(true);
    itemFlexGridSizer5->Add(m_GroupByEST, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByPredictedRefSeq = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_REFSEQ_PREDICTED, _("Group Predicted RefSeq alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByPredictedRefSeq->SetValue(false);
    itemFlexGridSizer5->Add(m_GroupByPredictedRefSeq, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByGenBank = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_GENBANK, _("Group GenBank/EMBL/DDBJ alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByGenBank->SetValue(true);
    itemFlexGridSizer5->Add(m_GroupByGenBank, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByWGS = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_WGS, _("Group Whole Genome Shotgun alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByWGS->SetValue(false);
    itemFlexGridSizer5->Add(m_GroupByWGS, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByHTGS = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_HTGS, _("Group HTGS sequence alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByHTGS->SetValue(false);
    itemFlexGridSizer5->Add(m_GroupByHTGS, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GroupByPatent = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX_PATENT, _("Group patent sequence alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GroupByPatent->SetValue(false);
    itemFlexGridSizer5->Add(m_GroupByPatent, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CGroupAlignmentsParamsPanel content construction
}

bool CGroupAlignmentsParamsPanel::ShowToolTips()
{
    return true;
}
wxBitmap CGroupAlignmentsParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGroupAlignmentsParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGroupAlignmentsParamsPanel bitmap retrieval
}
wxIcon CGroupAlignmentsParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGroupAlignmentsParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGroupAlignmentsParamsPanel icon retrieval
}


void CGroupAlignmentsParamsPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kTableTag = "Table";

void CGroupAlignmentsParamsPanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectList->SaveTableSettings(table_view);
    }
}


void CGroupAlignmentsParamsPanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryReadView table_view = gui_reg.GetReadView(reg_path);
        m_ObjectList->LoadTableSettings(table_view);
    }
}



void CGroupAlignmentsParamsPanel::SetParams(SGroupAlignmentsParams* params,
                                            TConstScopedObjects* objects)
{
    m_Params = params;
    m_InputObjects = objects;
}


bool CGroupAlignmentsParamsPanel::TransferDataToWindow()
{
    if (m_InputListDirty)    {
        m_ObjectList->SetObjects(*m_InputObjects);
        m_ObjectList->SelectAll();
        m_InputListDirty = false;
    }

    m_GroupBySeqId->SetValue(m_Params->m_GroupBySeqId);
    m_GroupByStrand->SetValue(m_Params->m_GroupByStrand);
    m_GroupByRefSeq->SetValue(m_Params->m_GroupByRefSeq);
    m_GroupByPredictedRefSeq->SetValue(m_Params->m_GroupByPredictedRefSeq);
    m_GroupByEST->SetValue(m_Params->m_GroupByEST);
    m_GroupByTaxId->SetValue(m_Params->m_GroupByTaxId);
    m_GroupByLikeTaxId->SetValue(m_Params->m_GroupByLikeTaxId);
    m_GroupByWGS->SetValue(m_Params->m_GroupByWGS);
    m_GroupByHTGS->SetValue(m_Params->m_GroupByHTGS);
    m_GroupByPatent->SetValue(m_Params->m_GroupByPatent);

    return wxPanel::TransferDataToWindow();
}


bool CGroupAlignmentsParamsPanel::TransferDataFromWindow()
{
    if(wxPanel::TransferDataFromWindow())   {
        m_ObjectList->GetSelection(m_Params->m_Alignments);

        m_Params->m_GroupBySeqId = m_GroupBySeqId->GetValue();
        m_Params->m_GroupByStrand = m_GroupByStrand->GetValue();
        m_Params->m_GroupByRefSeq = m_GroupByRefSeq->GetValue();
        m_Params->m_GroupByPredictedRefSeq = m_GroupByPredictedRefSeq->GetValue();
        m_Params->m_GroupByEST = m_GroupByEST->GetValue();
        m_Params->m_GroupByTaxId = m_GroupByTaxId->GetValue();
        m_Params->m_GroupByLikeTaxId = m_GroupByLikeTaxId->GetValue();
        m_Params->m_GroupByWGS = m_GroupByWGS->GetValue();
        m_Params->m_GroupByHTGS = m_GroupByHTGS->GetValue();
        m_Params->m_GroupByPatent = m_GroupByPatent->GetValue();
        return true;
    }
    return false;
}

void CGroupAlignmentsParamsPanel::RestoreDefaults()
{
    if (!m_Params)
        return;
    m_Params->Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
