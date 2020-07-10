/*  $Id: struct_fieldvalue_panel.cpp 42432 2019-02-22 18:44:43Z filippov $
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
 * Authors:  Andrea Asztalos
 * 
 *  Structured Comment panel used in Parse Text dialog
 */


#include <ncbi_pch.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>

#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CStructFieldValuePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CStructFieldValuePanel, CFieldNamePanel )


/*!
 * CStructFieldValuePanel event table definition
 */

BEGIN_EVENT_TABLE( CStructFieldValuePanel, CFieldNamePanel )

////@begin CStructFieldValuePanel event table entries
////@end CStructFieldValuePanel event table entries

END_EVENT_TABLE()


/*!
 * CStructFieldValuePanel constructors
 */

CStructFieldValuePanel::CStructFieldValuePanel()
{
    Init();
}

CStructFieldValuePanel::CStructFieldValuePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CStructFieldValuePanel creator
 */

bool CStructFieldValuePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CStructFieldValuePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CStructFieldValuePanel creation
    return true;
}


/*!
 * CStructFieldValuePanel destructor
 */

CStructFieldValuePanel::~CStructFieldValuePanel()
{
////@begin CStructFieldValuePanel destruction
////@end CStructFieldValuePanel destruction
}


/*!
 * Member initialisation
 */

void CStructFieldValuePanel::Init()
{
////@begin CStructFieldValuePanel member initialisation
    m_StructField = NULL;
////@end CStructFieldValuePanel member initialisation
}


/*!
 * Control creation for CStructFieldValuePanel
 */

void CStructFieldValuePanel::CreateControls()
{    
////@begin CStructFieldValuePanel content construction
    CStructFieldValuePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_StructFieldStrings;
    m_StructField = new wxChoice( itemPanel1, ID_STRCMNTFLD_CHOICE, wxDefaultPosition, wxDefaultSize, m_StructFieldStrings, 0 );
    itemBoxSizer2->Add(m_StructField, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CStructFieldValuePanel content construction
}

string CStructFieldValuePanel::GetFieldName(const bool subfield)
{
    string field = kEmptyStr;
    int val = m_StructField->GetSelection();
    if (val > -1) {
        field = ToStdString(m_StructField->GetString(val));
        field = kStructCommFieldValuePair + ' ' + field;
    }
    return field;
}

bool CStructFieldValuePanel::SetFieldName(const string& field)
{
    return true;
}

void CStructFieldValuePanel::ListStructCommentFields(const set<string> &field_names)
{
   
    if (!field_names.empty()) {
        size_t size = field_names.size();
        wxArrayString arraystr; 
        arraystr.Alloc(size);
        for(set<string>::const_iterator it = field_names.begin(); 
            it != field_names.end(); ++it ){
            arraystr.Add( ToWxString(*it) );
        }
        wxWindowUpdateLocker noUpdates(m_StructField);
        m_StructField->Append(arraystr);
        this->SetSizerAndFit(GetSizer());
    }
}
/*!
 * Should we show tooltips?
 */

bool CStructFieldValuePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CStructFieldValuePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CStructFieldValuePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CStructFieldValuePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CStructFieldValuePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CStructFieldValuePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CStructFieldValuePanel icon retrieval
}

END_NCBI_SCOPE
