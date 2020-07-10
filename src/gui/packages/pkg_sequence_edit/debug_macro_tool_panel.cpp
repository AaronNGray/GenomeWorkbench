/*  $Id: debug_macro_tool_panel.cpp 37345 2016-12-27 18:24:36Z katargir $
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

////@begin includes
////@end includes

#include "debug_macro_tool_panel.hpp"


#include <wx/sizer.h>
#include <wx/textctrl.h>
#include "wx/valgen.h"
#include "wx/valtext.h"

#include "wx/bitmap.h"
#include "wx/icon.h"


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CDebugMacroToolPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDebugMacroToolPanel, CAlgoToolManagerParamsPanel )


/*!
 * CDebugMacroToolPanel event table definition
 */

BEGIN_EVENT_TABLE( CDebugMacroToolPanel, CAlgoToolManagerParamsPanel )

////@begin CDebugMacroToolPanel event table entries
////@end CDebugMacroToolPanel event table entries

END_EVENT_TABLE()


/*!
 * CDebugMacroToolPanel constructors
 */

CDebugMacroToolPanel::CDebugMacroToolPanel()
{
    Init();
}

CDebugMacroToolPanel::CDebugMacroToolPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CDebugMacroTool creator
 */

bool CDebugMacroToolPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDebugMacroToolPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDebugMacroToolPanel creation
    return true;
}


/*!
 * CDebugMacroToolPanel destructor
 */

CDebugMacroToolPanel::~CDebugMacroToolPanel()
{
////@begin CDebugMacroToolPanel destruction
////@end CDebugMacroToolPanel destruction
}


/*!
 * Member initialisation
 */

void CDebugMacroToolPanel::Init()
{
////@begin CDebugMacroToolPanel member initialisation
    m_ObjectSel = NULL;
////@end CDebugMacroToolPanel member initialisation
}


/*!
 * Control creation for CDebugMacroTool
 */

void CDebugMacroToolPanel::CreateControls()
{    
////@begin CDebugMacroToolPanel content construction
    CDebugMacroToolPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    m_ObjectSel = new CObjectListWidgetSel( itemCAlgoToolManagerParamsPanel1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ObjectSel, 1, wxGROW|wxALL, 5);

    wxTextCtrl* itemTextCtrl4 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH2 | wxTE_MULTILINE );
    itemBoxSizer2->Add(itemTextCtrl4, 1, wxGROW|wxALL, 5);
    /*
    wxFont new_font = wxFont(10, wx.MODERN, wx.NORMAL, wx.NORMAL, False, u'Consolas')
    myTextCtrl.SetFont(new_font);
    */

    /*
    wxTextAttr style = itemTextCtrl4->GetDefaultStyle();
    style.SetFontFamily(wxFONTFAMILY_TELETYPE);
    itemTextCtrl4->SetDefaultStyle (style);
    */

    // Set validators
    itemTextCtrl4->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_Text) );
////@end CDebugMacroToolPanel content construction

    m_ObjectSel->SetTitle(wxT("Select Objects"));
}

void CDebugMacroToolPanel::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_ObjectSel->SetObjects(objects);
}

/*!
 * Should we show tooltips?
 */

bool CDebugMacroToolPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDebugMacroToolPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDebugMacroToolPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDebugMacroToolPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDebugMacroToolPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDebugMacroToolPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDebugMacroToolPanel icon retrieval
}

/*!
 * Transfer data from the window
 */

bool CDebugMacroToolPanel::TransferDataFromWindow()
{
    if (!CAlgoToolManagerParamsPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects selection = m_ObjectSel->GetSelection();
    GetData().SetObjects() = selection;

    return true;
}

static const char* kObjectSel = ".ObjectSel";

void CDebugMacroToolPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_ObjectSel->SetRegistryPath(m_RegPath + kObjectSel);
}

void CDebugMacroToolPanel::SaveSettings() const
{
    m_ObjectSel->SaveSettings();
}

void CDebugMacroToolPanel::LoadSettings()
{
    m_ObjectSel->LoadSettings();
}

void CDebugMacroToolPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
