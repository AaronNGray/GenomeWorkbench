/*  $Id: create_needleman_wunsch_panel.cpp 42821 2019-04-18 19:32:56Z joukovv $
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

#include <corelib/ncbistd.hpp>
#include "create_needleman_wunsch_panel.hpp"

#include <gui/widgets/object_list/object_list_widget.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include "wx/bitmap.h"
#include "wx/icon.h"


BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CAlignNeedlemanWunschPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CAlignNeedlemanWunschPanel, CAlgoToolManagerParamsPanel )

////@begin CAlignNeedlemanWunschPanel event table entries
////@end CAlignNeedlemanWunschPanel event table entries

END_EVENT_TABLE()

CAlignNeedlemanWunschPanel::CAlignNeedlemanWunschPanel()
{
    Init();
}

CAlignNeedlemanWunschPanel::CAlignNeedlemanWunschPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CAlignNeedlemanWunschPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignNeedlemanWunschPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlignNeedlemanWunschPanel creation
    return true;
}

CAlignNeedlemanWunschPanel::~CAlignNeedlemanWunschPanel()
{
////@begin CAlignNeedlemanWunschPanel destruction
////@end CAlignNeedlemanWunschPanel destruction
}

void CAlignNeedlemanWunschPanel::Init()
{
////@begin CAlignNeedlemanWunschPanel member initialisation
    m_ObjectList = NULL;
////@end CAlignNeedlemanWunschPanel member initialisation
}

void CAlignNeedlemanWunschPanel::CreateControls()
{    
////@begin CAlignNeedlemanWunschPanel content construction
    CAlignNeedlemanWunschPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    m_ObjectList = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_LISTCTRL2, wxDefaultPosition, wxSize(100, 200), wxLC_REPORT );
    itemBoxSizer2->Add(m_ObjectList, 1, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Match Cost"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Mismatch Cost"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Gap Open Cost"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Gap Extension Cost"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Band Size (0 = no band)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Free ends, first sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice18Strings;
    itemChoice18Strings.Add(_("none"));
    itemChoice18Strings.Add(_("left"));
    itemChoice18Strings.Add(_("right"));
    itemChoice18Strings.Add(_("both"));
    wxChoice* itemChoice18 = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_CHOICE, wxDefaultPosition, wxDefaultSize, itemChoice18Strings, 0 );
    itemChoice18->SetStringSelection(_("none"));
    itemFlexGridSizer4->Add(itemChoice18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Free ends, second sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText19, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice20Strings;
    itemChoice20Strings.Add(_("none"));
    itemChoice20Strings.Add(_("left"));
    itemChoice20Strings.Add(_("right"));
    itemChoice20Strings.Add(_("both"));
    wxChoice* itemChoice20 = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, itemChoice20Strings, 0 );
    itemChoice20->SetStringSelection(_("none"));
    itemFlexGridSizer4->Add(itemChoice20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl6->SetValidator( wxGenericValidator(& GetData().m_MatchCost) );
    itemTextCtrl8->SetValidator( wxGenericValidator(& GetData().m_MismatchCost) );
    itemTextCtrl10->SetValidator( wxGenericValidator(& GetData().m_GapOpenCost) );
    itemTextCtrl12->SetValidator( wxGenericValidator(& GetData().m_GapExtendCost) );
    itemTextCtrl14->SetValidator( wxGenericValidator(& GetData().m_BandSize) );
    itemChoice18->SetValidator( wxGenericValidator(& GetData().m_FirstSeqFreeEnds) );
    itemChoice20->SetValidator( wxGenericValidator(& GetData().m_SecondSeqFreeEnds) );
////@end CAlignNeedlemanWunschPanel content construction
}

void CAlignNeedlemanWunschPanel::SetObjects(TConstScopedObjects* objects)
{
    m_ObjectList->SetObjects(*objects);

    if (objects->size() > 0)
        m_ObjectList->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    if (objects->size() > 1)
        m_ObjectList->SetItemState(1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

bool CAlignNeedlemanWunschPanel::ShowToolTips()
{
    return true;
}
wxBitmap CAlignNeedlemanWunschPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAlignNeedlemanWunschPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAlignNeedlemanWunschPanel bitmap retrieval
}
wxIcon CAlignNeedlemanWunschPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlignNeedlemanWunschPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlignNeedlemanWunschPanel icon retrieval
}


static const char* kLocationSel = "LocationSel";

void CAlignNeedlemanWunschPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CAlignNeedlemanWunschPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kLocationSel);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectList->SaveTableSettings(table_view);
    }
}


void CAlignNeedlemanWunschPanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kLocationSel);
        table_view = gui_reg.GetReadView(reg_path);
        m_ObjectList->LoadTableSettings(table_view);
    }
}


bool CAlignNeedlemanWunschPanel::TransferDataFromWindow()
{
    if (!CAlgoToolManagerParamsPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection;
    m_ObjectList->GetSelection(selection);

    if (selection.size() != 2) {
        wxMessageBox(wxT("Please select exactly 2 locations."), wxT("Error"),
             wxOK | wxICON_ERROR, this);

        m_ObjectList->SetFocus();
        return false;
    }

    GetData().SetObjects() = selection;

    return true;
}

void CAlignNeedlemanWunschPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}


END_NCBI_SCOPE
