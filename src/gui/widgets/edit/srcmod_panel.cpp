/*  $Id: srcmod_panel.cpp 43640 2019-08-13 13:55:26Z asztalos $
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
 * Authors:  Vasuki Palanigobu
 */


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes
#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/srcmod_panel.hpp>
#include "srcmod_list_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CSrcModPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSrcModPanel, wxPanel )


/*
 * CSrcModPanel event table definition
 */

BEGIN_EVENT_TABLE( CSrcModPanel, wxPanel )

////@begin CSrcModPanel event table entries
    EVT_COMBOBOX( ID_MODIFIER_NAME, CSrcModPanel::OnModifierNameSelected )
    
////@end CSrcModPanel event table entries
END_EVENT_TABLE()


/*
 * CSrcModPanel constructors
 */

CSrcModPanel::CSrcModPanel()
{
    Init();
}

CSrcModPanel::CSrcModPanel( wxWindow* parent, SModData mod_data, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    m_ModData = mod_data;
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * SrcModPanel creator
 */

bool CSrcModPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSrcModPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSrcModPanel creation
    return true;
}


/*
 * CSrcModPanel destructor
 */

CSrcModPanel::~CSrcModPanel()
{
////@begin CSrcModPanel destruction
////@end CSrcModPanel destruction
}


/*
 * Member initialisation
 */

void CSrcModPanel::Init()
{
////@begin CSrcModPanel member initialisation
    m_Sizer = NULL;
    m_SrcModName = NULL;
    m_Selected = kEmptyStr;
////@end CSrcModPanel member initialisation
    m_Editor = NULL;
}


/*
 * Control creation for SrcModPanel
 */

void CSrcModPanel::CreateControls()
{    
////@begin CSrcModPanel content construction
    CSrcModPanel* itemPanel1 = this;

    m_Sizer = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(m_Sizer);

    wxArrayString m_SrcModNameStrings = s_GetModifierList();
    m_SrcModName = new wxComboBox( itemPanel1, ID_MODIFIER_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_SrcModNameStrings, wxCB_READONLY );
    m_SrcModName->Bind(wxEVT_LEFT_DOWN, &CSrcModPanel::OnLeftDown, this);
    m_SrcModName->Bind(wxEVT_LEFT_UP, &CSrcModPanel::OnLeftUp, this);
#ifdef __WXOSX_COCOA__
    m_SrcModName->SetSelection(-1);
#endif
    m_Sizer->Add(m_SrcModName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Sizer->Add(300, 10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSrcModPanel content construction
}


bool CSrcModPanel::DoNotListOrgMod(int st)
{
    if (st == COrgMod::eSubtype_old_lineage || st == COrgMod::eSubtype_old_name) {
        return false;
    }
    return COrgMod::IsDiscouraged(st, true);
}


bool CSrcModPanel::DoNotListSubSrc(int st)
{
    if (st >= CSubSource::eSubtype_fwd_primer_seq && st <= CSubSource::eSubtype_rev_primer_name) {
        return true;
    }
    return CSubSource::IsDiscouraged(st);
}


namespace {
    wxString s_AdjustModNameForDisplay(const string& orig)
    {
        string adjusted = orig;
        if (NStr::EqualNocase(adjusted, kNatHost)) {
            adjusted = kHost;
        }
        return ToWxString(adjusted);
    }

    wxString s_AdjustDisplayNameForModName(const wxString& display)
    {
        string adjusted = ToStdString(display);
        if (NStr::EqualNocase(adjusted, kHost)) {
            adjusted = ToWxString(kNatHost);
        }
        return adjusted;
    }

    bool s_DoNotListSubSource(int st)
    {
        if (st >= CSubSource::eSubtype_fwd_primer_seq && st <= CSubSource::eSubtype_rev_primer_name) {
            return true;
        }
        return CSubSource::IsDiscouraged(st);
    }


    bool s_DoNotListOrgMod(int st)
    {
        if (st == COrgMod::eSubtype_old_lineage || st == COrgMod::eSubtype_old_name) {
            return false;
        }
        return COrgMod::IsDiscouraged(st, true);
    }
}

wxArrayString CSrcModPanel::s_GetModifierList()
{
    wxArrayString choices;
    size_t i;
    for (i = 0; i < objects::CSubSource::eSubtype_other; i++) {
        try {
            string qual_name = objects::CSubSource::GetSubtypeName(i);
            if (!NStr::IsBlank(qual_name) && !DoNotListSubSrc(i)) {
                choices.push_back(s_AdjustModNameForDisplay(qual_name));
            }

        }
        catch (const exception&) {
        }
    }
    for (i = 0; i < objects::COrgMod::eSubtype_other; i++) {
        try {
            string qual_name = objects::COrgMod::GetSubtypeName(i);
            if (!NStr::IsBlank(qual_name) && !s_DoNotListOrgMod(i)) {
                // special case for nat-host
                choices.push_back(s_AdjustModNameForDisplay(qual_name));
            }

        }
        catch (const exception&) {
        }
    }
    choices.Sort();
    return choices;
}



void CSrcModPanel::x_SetEditor(CSrcModEditPanel::EEditorType editor_type)
{
    string val = ToStdString(m_ModData.value);
    if (m_Editor && m_Editor->GetEditorType() == editor_type && m_Editor->IsWellFormatted(val)) {
        if (editor_type == CSrcModEditPanel::eCheckbox) {
            m_Editor->SetValue("true");
            OnEditorChange();
        } else {
            m_Editor->SetValue(val);
        }
        return;
    }
    auto old_editor_type = editor_type;
    if (m_Editor)
        old_editor_type = m_Editor->GetEditorType();
    while (m_Sizer->GetItemCount() > 1) {
        size_t pos = 1;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }
    m_Editor = CSrcModEditFactory::Create(this, editor_type);
    if (!m_Editor || !m_Editor->IsWellFormatted(val)) {
        delete m_Editor;
        m_Editor = CSrcModEditFactory::Create(this, CSrcModEditPanel::eText);
        editor_type =  CSrcModEditPanel::eText;
    }
    if (m_Editor) {
        m_Sizer->Add(m_Editor, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        if (editor_type == CSrcModEditPanel::eCheckbox) {
            m_Editor->SetValue("true");
            OnEditorChange();
        } else if (old_editor_type != CSrcModEditPanel::eCheckbox) {            
            m_Editor->SetValue(val);
        }
    }
    Layout();
    
}


void CSrcModPanel::SetModifierData()
{
    string mod_name = ToStdString(m_ModData.name);
    m_SrcModName->SetValue(s_AdjustModNameForDisplay(mod_name));
    if (NStr::IsBlank(mod_name)) {
        return;
    }

    CSrcModEditPanel::EEditorType editor_type = CSrcModEditPanel::eText;
    bool is_subsource = objects::CSubSource::IsValidSubtypeName(mod_name, objects::CSubSource::eVocabulary_insdc);
    if (is_subsource) {
        CSubSource::TSubtype st = objects::CSubSource::GetSubtypeValue (mod_name, objects::CSubSource::eVocabulary_insdc);
        editor_type = CSrcModEditPanel::GetEditorTypeForSubSource(st);
    }
    else {
        bool is_orgmod = objects::COrgMod::IsValidSubtypeName(mod_name, objects::COrgMod::eVocabulary_insdc);
        if (is_orgmod) {
            COrgMod::TSubtype st = objects::COrgMod::GetSubtypeValue(mod_name, objects::COrgMod::eVocabulary_insdc);
            editor_type = CSrcModEditPanel::GetEditorTypeForOrgMod(st);
        }
    }
    x_SetEditor(editor_type);
}


const CSrcModPanel::SModData& CSrcModPanel::GetModifierData()
{
    wxString mod = m_SrcModName->GetString(m_SrcModName->GetCurrentSelection());
    m_ModData.name = s_AdjustDisplayNameForModName(mod);
    if (m_Editor) {  
        m_ModData.value = ToWxString(m_Editor->GetValue());
    }
    return m_ModData;
}


/*
 * Should we show tooltips?
 */

bool CSrcModPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSrcModPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSrcModPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSrcModPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSrcModPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSrcModPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSrcModPanel icon retrieval
}


void CSrcModPanel::OnEditorChange( )
{
    if (!m_Editor) {
        return;
    }
    string val = m_Editor->GetValue();
    if (NStr::IsBlank(val)) {
        return;
    }

    wxWindow* parent = this->GetParent();

    CSrcModListPanel* listpanel = dynamic_cast<CSrcModListPanel*>(parent);

    while (parent && !listpanel) {
        parent = parent->GetParent();
        listpanel = dynamic_cast<CSrcModListPanel*>(parent);
    }

    if (!listpanel) {
        return;
    }
    listpanel->AddLastSrcMod((wxWindow*)this);
}


/*
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_MODIFIER_NAME
 */

void CSrcModPanel::OnModifierNameSelected( wxCommandEvent& event )
{
    GetModifierData();
    SetModifierData();
}

void CSrcModPanel::OnLeftDown( wxMouseEvent& event)
{
    m_Selected = ToStdString(m_SrcModName->GetStringSelection());
    event.Skip();
}

void CSrcModPanel::OnLeftUp( wxMouseEvent& event)
{
    string current = ToStdString(m_SrcModName->GetString(m_SrcModName->GetCurrentSelection()));
    m_SrcModName->SetSelection(m_SrcModName->GetCurrentSelection());
    if ( ! NStr::Equal(current, m_Selected, NStr::eNocase)) {
        GetModifierData();
        SetModifierData();
    }
    event.Skip();
}


void CSrcModPanel::SetModifierData(const string& mod_name, const string& mod_val)
{
    m_ModData.name = ToWxString(mod_name);
    m_ModData.value = ToWxString(mod_val);
    SetModifierData();
}


END_NCBI_SCOPE

