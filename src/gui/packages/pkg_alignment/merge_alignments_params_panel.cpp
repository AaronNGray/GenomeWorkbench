/*  $Id: merge_alignments_params_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <objtools/alnmgr/aln_user_options.hpp>

#include <gui/packages/pkg_alignment/merge_alignments_params_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// SMergeAlignmentsParams

SMergeAlignmentsParams::SMergeAlignmentsParams()
{
    Init();
}

void SMergeAlignmentsParams::Init()
{
    m_Direction = CAlnUserOptions::eBothDirections;
    m_MergeAlgo = CAlnUserOptions::eMergeAllSeqs;
    m_MergeFlags = 0;
    m_FillUnaligned = false;
}

void SMergeAlignmentsParams::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

static const char* kDirectionTag = "Direction";
static const char* kMergeAlgoTag = "MergeAlgo";
static const char* kMergeFlagsTag = "MergeFlags";
static const char* kFillUnalignedTag = "FillUnaligned";

void SMergeAlignmentsParams::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kDirectionTag, m_Direction);
        view.Set(kMergeAlgoTag, m_MergeAlgo);
        view.Set(kMergeFlagsTag, m_MergeFlags);
        view.Set(kFillUnalignedTag, m_FillUnaligned);
    }
}


void SMergeAlignmentsParams::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_Direction = view.GetInt(kDirectionTag, m_Direction);
        m_MergeAlgo = view.GetInt(kMergeAlgoTag, m_MergeAlgo);
        m_MergeFlags = view.GetInt(kMergeFlagsTag, m_MergeFlags);
        m_FillUnaligned = view.GetBool(kFillUnalignedTag, m_FillUnaligned);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// CMergeAlignmentsParamsPanel

IMPLEMENT_DYNAMIC_CLASS( CMergeAlignmentsParamsPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CMergeAlignmentsParamsPanel, wxPanel )
////@begin CNetBLASTLoadOptionPanel event table entries
////@end CNetBLASTLoadOptionPanel event table entries
END_EVENT_TABLE()



CMergeAlignmentsParamsPanel::CMergeAlignmentsParamsPanel()
{
    Init();
}


CMergeAlignmentsParamsPanel::CMergeAlignmentsParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CMergeAlignmentsParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMergeAlignmentsParamsPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMergeAlignmentsParamsPanel creation
    return true;
}


CMergeAlignmentsParamsPanel::~CMergeAlignmentsParamsPanel()
{
////@begin CMergeAlignmentsParamsPanel destruction
////@end CMergeAlignmentsParamsPanel destruction
}


void CMergeAlignmentsParamsPanel::Init()
{
////@begin CMergeAlignmentsParamsPanel member initialisation
    m_ObjectListCtrl = NULL;
    m_MergeQuerySeqOnly = NULL;
    m_AssumeNegStrand = NULL;
    m_TruncateOverlap = NULL;
    m_SortInput = NULL;
    m_FillUnaligned = NULL;
////@end CMergeAlignmentsParamsPanel member initialisation

    m_Params = NULL;
    m_InputListDirty = true;
}

void CMergeAlignmentsParamsPanel::CreateControls()
{
////@begin CMergeAlignmentsParamsPanel content construction
    CMergeAlignmentsParamsPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Alignments to Merge"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    m_ObjectListCtrl = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_LISTCTRL1, wxDefaultPosition, itemCAlgoToolManagerParamsPanel1->ConvertDialogToPixels(wxSize(100, 100)), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_ObjectListCtrl, 1, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer5->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer5, 0, wxGROW|wxALL, 5);

    m_MergeQuerySeqOnly = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX6, _("Merge query sequence only"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MergeQuerySeqOnly->SetValue(false);
    itemFlexGridSizer5->Add(m_MergeQuerySeqOnly, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AssumeNegStrand = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX7, _("Assume all on negative strand"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AssumeNegStrand->SetValue(false);
    itemFlexGridSizer5->Add(m_AssumeNegStrand, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TruncateOverlap = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX8, _("Truncate overlapping segments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TruncateOverlap->SetValue(false);
    itemFlexGridSizer5->Add(m_TruncateOverlap, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SortInput = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX9, _("Sort inputs by source alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SortInput->SetValue(false);
    itemFlexGridSizer5->Add(m_SortInput, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FillUnaligned = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX10, _("Fill unaligned regions"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FillUnaligned->SetValue(false);
    itemFlexGridSizer5->Add(m_FillUnaligned, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CMergeAlignmentsParamsPanel content construction
}

bool CMergeAlignmentsParamsPanel::ShowToolTips()
{
    return true;
}

wxBitmap CMergeAlignmentsParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMergeAlignmentsParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMergeAlignmentsParamsPanel bitmap retrieval
}

wxIcon CMergeAlignmentsParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMergeAlignmentsParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMergeAlignmentsParamsPanel icon retrieval
}


void CMergeAlignmentsParamsPanel::SetParams(SMergeAlignmentsParams* params,
                                            TConstScopedObjects* objects)
{
    m_Params = params;
    m_InputObjects = objects;
}


bool CMergeAlignmentsParamsPanel::TransferDataToWindow()
{
    if(m_InputListDirty)    {
        m_ObjectListCtrl->SetObjects(*m_InputObjects);
        m_ObjectListCtrl->SelectAll();
        m_InputListDirty = false;
    }

    m_MergeQuerySeqOnly->SetValue(m_Params->m_MergeAlgo == CAlnUserOptions::eQuerySeqMergeOnly);
    m_AssumeNegStrand->SetValue(m_Params->m_Direction == CAlnUserOptions::eReverse);
    m_TruncateOverlap->SetValue((m_Params->m_MergeFlags&CAlnUserOptions::fTruncateOverlaps) != 0);
    m_SortInput->SetValue((m_Params->m_MergeFlags&CAlnUserOptions::fSkipSortByScore) == 0);
    m_FillUnaligned->SetValue(m_Params->m_FillUnaligned);

    return wxPanel::TransferDataToWindow();
}


bool CMergeAlignmentsParamsPanel::TransferDataFromWindow()
{
    if(!wxPanel::TransferDataFromWindow())
        return false;

    m_ObjectListCtrl->GetSelection(m_Params->m_Alignments);

    m_Params->m_MergeAlgo = m_MergeQuerySeqOnly->GetValue() ?
        CAlnUserOptions::eQuerySeqMergeOnly : CAlnUserOptions::eMergeAllSeqs;
    m_Params->m_Direction = m_AssumeNegStrand->GetValue() ?
        CAlnUserOptions::eReverse : CAlnUserOptions::eBothDirections;

    if (m_TruncateOverlap->GetValue())
        m_Params->m_MergeFlags |= CAlnUserOptions::fTruncateOverlaps;
    else
        m_Params->m_MergeFlags &= ~CAlnUserOptions::fTruncateOverlaps;

    if (m_SortInput->GetValue())
        m_Params->m_MergeFlags &= ~CAlnUserOptions::fSkipSortByScore;
    else
        m_Params->m_MergeFlags |= CAlnUserOptions::fSkipSortByScore;

    m_Params->m_FillUnaligned = m_FillUnaligned->GetValue();

    return true;
}

void CMergeAlignmentsParamsPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kTableTag = "Table";

void CMergeAlignmentsParamsPanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryWriteView table_view =
            gui_reg.GetWriteView(reg_path);
        m_ObjectListCtrl->SaveTableSettings(table_view);
    }
}


void CMergeAlignmentsParamsPanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryReadView table_view = gui_reg.GetReadView(reg_path);
        m_ObjectListCtrl->LoadTableSettings(table_view);
    }
}

void CMergeAlignmentsParamsPanel::RestoreDefaults()
{
    if (!m_Params)
        return;
    m_Params->Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
