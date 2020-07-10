/*  $Id: taxtree_tool_panel.cpp 37336 2016-12-23 21:13:16Z katargir $
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

#include <corelib/ncbifile.hpp>

#include <gui/widgets/object_list/object_list_widget_sel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "taxtree_tool_panel.hpp"

#include "wx/sizer.h"
#include "wx/bitmap.h"
#include "wx/icon.h"

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CTaxTreeToolPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CTaxTreeToolPanel, CAlgoToolManagerParamsPanel )

////@begin CTaxTreeToolPanel event table entries
////@end CTaxTreeToolPanel event table entries

END_EVENT_TABLE()

CTaxTreeToolPanel::CTaxTreeToolPanel()
{
    Init();
}

CTaxTreeToolPanel::CTaxTreeToolPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
{
    Init();
    Create(parent, id, pos, size, style, visible);
}

bool CTaxTreeToolPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
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

CTaxTreeToolPanel::~CTaxTreeToolPanel()
{
////@begin CTaxTreeToolPanel destruction
////@end CTaxTreeToolPanel destruction
}

void CTaxTreeToolPanel::Init()
{
////@begin CTaxTreeToolPanel member initialisation
    m_SeqIds = NULL;
////@end CTaxTreeToolPanel member initialisation
}

void CTaxTreeToolPanel::CreateControls()
{    
////@begin CTaxTreeToolPanel content construction
    CTaxTreeToolPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    m_SeqIds = new CObjectListWidgetSel( itemCAlgoToolManagerParamsPanel1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_SeqIds, 1, wxGROW|wxALL, 0);

////@end CTaxTreeToolPanel content construction

    m_SeqIds->SetDoSelectAll(true);
    m_SeqIds->SetTitle(wxT("Select SeqIds"));
}

void CTaxTreeToolPanel::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_SeqIds->SetObjects(objects);
}

bool CTaxTreeToolPanel::ShowToolTips()
{
    return true;
}
wxBitmap CTaxTreeToolPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTaxTreeToolPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTaxTreeToolPanel bitmap retrieval
}
wxIcon CTaxTreeToolPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTaxTreeToolPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTaxTreeToolPanel icon retrieval
}

bool CTaxTreeToolPanel::TransferDataFromWindow()
{
    if (!CAlgoToolManagerParamsPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection = m_SeqIds->GetSelection();
    GetData().SetObjects() = selection;

    return true;
}

static const char* kLocationSel = ".LocationSel";

void CTaxTreeToolPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_SeqIds->SetRegistryPath(m_RegPath + kLocationSel);
}

void CTaxTreeToolPanel::SaveSettings() const
{
    m_SeqIds->SaveSettings();
}

void CTaxTreeToolPanel::LoadSettings()
{
    m_SeqIds->LoadSettings();
}

void CTaxTreeToolPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
