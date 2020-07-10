/*  $Id: find_overlap_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/find_overlap_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>

////@begin includes
#include "wx/imaglist.h"
////@end includes


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CFindOverlapPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CFindOverlapPanel, CAlgoToolManagerParamsPanel )

////@begin CFindOverlapPanel event table entries
////@end CFindOverlapPanel event table entries

END_EVENT_TABLE()

CFindOverlapPanel::CFindOverlapPanel() : m_InputObjects()
{
    Init();
}

CFindOverlapPanel::CFindOverlapPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
: m_InputObjects()
{
    Init();
    Create(parent, id, pos, size, style, visible);
}

bool CFindOverlapPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
{
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    if (!visible) {
        Hide();
    }

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}

CFindOverlapPanel::~CFindOverlapPanel()
{
////@begin CFindOverlapPanel destruction
////@end CFindOverlapPanel destruction
}

void CFindOverlapPanel::Init()
{
////@begin CFindOverlapPanel member initialisation
    m_Seq1List = NULL;
    m_Seq2List = NULL;
////@end CFindOverlapPanel member initialisation
}

void CFindOverlapPanel::CreateControls()
{
////@begin CFindOverlapPanel content construction
    CFindOverlapPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Sequence 1"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    m_Seq1List = new CObjectListWidget( itemStaticBoxSizer3->GetStaticBox(), ID_LISTCTRL3, wxDefaultPosition, itemStaticBoxSizer3->GetStaticBox()->ConvertDialogToPixels(wxSize(300, 75)), wxLC_REPORT|wxLC_SINGLE_SEL );
    itemStaticBoxSizer3->Add(m_Seq1List, 1, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Sequence 2"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer5, 1, wxGROW|wxALL, 5);

    m_Seq2List = new CObjectListWidget( itemStaticBoxSizer5->GetStaticBox(), ID_LISTCTRL4, wxDefaultPosition, itemStaticBoxSizer5->GetStaticBox()->ConvertDialogToPixels(wxSize(300, 75)), wxLC_REPORT|wxLC_SINGLE_SEL );
    itemStaticBoxSizer5->Add(m_Seq2List, 1, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer7, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("BLAST parameters"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemTextCtrl9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText10 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Maximum \"slop\" (unaligned bases at ends)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemTextCtrl11, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText12 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Quality of overlaps"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxArrayString itemChoice13Strings;
    itemChoice13Strings.Add(_("Perfect (percentage of identity (ungap) >= 99.8%)"));
    itemChoice13Strings.Add(_("Green (percentage of identity (ungap) >= 99.6%)"));
    itemChoice13Strings.Add(_("Yellow (percentage of identity (ungap) >= 98.0%)"));
    itemChoice13Strings.Add(_("Red (percentage of identity (ungap) >= 95.0%)"));
    itemChoice13Strings.Add(_("Any"));
    wxChoice* itemChoice13 = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL5, wxDefaultPosition, wxDefaultSize, itemChoice13Strings, 0 );
    itemChoice13->SetStringSelection(_("Any"));
    itemFlexGridSizer7->Add(itemChoice13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    itemFlexGridSizer7->AddGrowableCol(1);

    // Set validators
    itemTextCtrl9->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_BlastParams) );
    itemTextCtrl11->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & GetData().m_MaxSlop) );
    itemChoice13->SetValidator( wxGenericValidator(& GetData().m_FilterQty) );
////@end CFindOverlapPanel content construction
}

bool CFindOverlapPanel::ShowToolTips()
{
    return true;
}
wxBitmap CFindOverlapPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFindOverlapPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFindOverlapPanel bitmap retrieval
}
wxIcon CFindOverlapPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFindOverlapPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFindOverlapPanel icon retrieval
}
bool CFindOverlapPanel::TransferDataToWindow()
{
    if (m_InputObjects) {
        m_Seq1List->SetObjects(*m_InputObjects);
        m_Seq2List->SetObjects(*m_InputObjects);

        if (m_InputObjects->size() > 0) {
            m_Seq1List->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            m_Seq2List->SetItemState((m_InputObjects->size() > 1 ? 1 : 0),
                                     wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    }

    return wxPanel::TransferDataToWindow();
}
bool CFindOverlapPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection;

    m_Seq1List->GetSelection(selection);
    if (selection.empty()) {
        wxMessageBox(wxT("Please select first sequence."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_Seq1List->SetFocus();
        return false;
    }
    m_data.SetSeq1() = selection[0];

    selection.clear();
    m_Seq2List->GetSelection(selection);
    if (selection.empty()) {
        wxMessageBox(wxT("Please select second sequence."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_Seq2List->SetFocus();
        return false;
    }
    m_data.SetSeq2() = selection[0];

    return true;
}

static const char *kTable1Tag = "Table1", *kTable2Tag = "Table2";

void CFindOverlapPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable1Tag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_Seq1List->SaveTableSettings(table_view);

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable2Tag);
        table_view = gui_reg.GetWriteView(reg_path);
        m_Seq2List->SaveTableSettings(table_view);
    }
}

void CFindOverlapPanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable1Tag);
        table_view = gui_reg.GetReadView(reg_path);
        m_Seq1List->LoadTableSettings(table_view);

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable2Tag);
        table_view = gui_reg.GetReadView(reg_path);
        m_Seq2List->LoadTableSettings(table_view);
    }
}

void CFindOverlapPanel::RestoreDefaults()
{
    GetData().Init();
    wxPanel::TransferDataToWindow();
}

END_NCBI_SCOPE
