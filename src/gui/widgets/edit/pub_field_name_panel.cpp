/*  $Id: pub_field_name_panel.cpp 44731 2020-03-03 15:53:48Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <objtools/edit/field_handler.hpp>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>

#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CFieldHandlerNamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFieldHandlerNamePanel, CFieldNamePanel )


/*!
 * CFieldHandlerNamePanel event table definition
 */

BEGIN_EVENT_TABLE( CFieldHandlerNamePanel, CFieldNamePanel )

////@begin CFieldHandlerNamePanel event table entries
    EVT_LISTBOX( ID_PUBFIELDNAME_FIELD, CFieldHandlerNamePanel::OnFieldSelected)

////@end CFieldHandlerNamePanel event table entries

END_EVENT_TABLE()


/*!
 * CFieldHandlerNamePanel constructors
 */

CFieldHandlerNamePanel::CFieldHandlerNamePanel()
{
    Init();
}

CFieldHandlerNamePanel::CFieldHandlerNamePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CPubFieldNamePanel creator
 */

bool CFieldHandlerNamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFieldHandlerNamePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFieldHandlerNamePanel creation
    return true;
}


/*!
 * CFieldHandlerNamePanel destructor
 */

CFieldHandlerNamePanel::~CFieldHandlerNamePanel()
{
////@begin CFieldHandlerNamePanel destruction
////@end CFieldHandlerNamePanel destruction
}


/*!
 * Member initialisation
 */

void CFieldHandlerNamePanel::Init()
{
////@begin CFieldHandlerNamePanel member initialisation
    m_Field = NULL;
////@end CFieldHandlerNamePanel member initialisation
}


/*!
 * Control creation for CPubFieldNamePanel
 */

void CFieldHandlerNamePanel::CreateControls()
{    
////@begin CFieldHandlerNamePanel content construction
    CFieldHandlerNamePanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_FieldStrings;
    m_Field = new wxListBox(itemCFieldNamePanel1, ID_PUBFIELDNAME_FIELD, wxDefaultPosition, wxSize(220,109), m_FieldStrings, wxLB_SINGLE);
    itemBoxSizer2->Add(m_Field, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CFieldHandlerNamePanel content construction
}

void CFieldHandlerNamePanel::ClearValues()
{
    m_Field->SetSelection(0);
    m_Field->SetFirstItem(0);
}

/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_PUBFIELDNAME_FIELD
 */

void CFieldHandlerNamePanel::OnFieldSelected( wxCommandEvent& event )
{
    x_UpdateParent();
    event.Skip();
}


/*!
 * Should we show tooltips?
 */

bool CFieldHandlerNamePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFieldHandlerNamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFieldHandlerNamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFieldHandlerNamePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFieldHandlerNamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFieldHandlerNamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFieldHandlerNamePanel icon retrieval
}


string CFieldHandlerNamePanel::GetFieldName(const bool subfield)
{
    string field = "";
    int val = m_Field->GetSelection();
    if (val > -1) {
        field = ToStdString(m_Field->GetString(val));
        CPubFieldType::EPubFieldType field_type = CPubFieldType::GetTypeForLabel(field);
        if (CPubFieldType::IsAffilField(field_type) && !subfield) {
            field = "affiliation " + field;
        }
    }
    return field;
}

static string s_GetMolinfoMember(const string& field)
{
    _ASSERT(field.front() == '\"');
    size_t end_dot = NStr::Find(field, ".", NStr::eNocase, NStr::eReverseSearch);
    if (end_dot != NPOS) {
        return "\"" + field.substr(end_dot + 1, NPOS);
    }
    return kEmptyStr;
}

string CFieldHandlerNamePanel::GetMacroFieldName(const string& target, const string& selected_field)
{
    string field;
    int val = m_Field->GetSelection();
    if (val > -1)
    {
        field = m_MacroField[val];
    }

    bool is_inst_field = NStr::StartsWith(field, "\"inst.");
    if (m_self == macro::CMacroBioData::sm_MolInfo)
    {
        if (target == macro::CMacroBioData::sm_MolInfo) {
            if (is_inst_field) {
                return CTempString(macro::CMacroFunction_GetSequence::sm_SeqForDescr) + "(" + field + ")";
            }
            else {
                return s_GetMolinfoMember(field);
            }
        }

        if (target == macro::CMacroBioData::sm_BioSource) {
            if (is_inst_field) {
                return CTempString(macro::CMacroFunction_GetSequence::sm_SeqForDescr) + "(" + field + ")";
            }
            else {
                return CTempString(macro::CMacroFunction_GetSeqdesc::sm_MolinfoForBsrc) + "(" + s_GetMolinfoMember(field) + ")";
            }
        }

        if (target == macro::CMacroBioData::sm_Seqdesc) {
            if (is_inst_field) {
                return CTempString(macro::CMacroFunction_GetSequence::sm_SeqForDescr) + "(" + field + ")";
            }
        }

        if (target == macro::CMacroBioData::sm_Seq ||
            target == macro::CMacroBioData::sm_SeqNa ||
            target == macro::CMacroBioData::sm_SeqAa) {
            return field;
        }

        if (CFieldNamePanel::IsFeature(target))
        {
            if (is_inst_field) {
                return CTempString(macro::CMacroFunction_GetSequence::sm_SeqForFeat) + "(" + field + ")";
            }
            else {
                return CTempString(macro::CMacroFunction_GetSeqdesc::sm_MolinfoForFeat) + "(" + s_GetMolinfoMember(field) + ")";
            }
        }
        return kEmptyStr;
    }

    if (m_self == macro::CMacroBioData::sm_DBLink) {
        if (target == m_self) {
            return "data_obj";
        }
        else {
            return macro::CMacroFunction_GetDBLink::GetFuncName() + "(\"" + field + "\")";
        }
    }

    if (m_self == "Misc") {
        if (target == macro::CMacroBioData::sm_Seqdesc) {
            size_t pos = field.find("..");
            field = field.substr(pos + 2, NPOS);
            field = "\"" + field;
        }
        else if (CFieldNamePanel::IsFeature(target)) {
            return CTempString(macro::CMacroFunction_GetSequence::sm_SeqForFeat) + "(" + field + ")";
        }
        else {
            return CTempString(macro::CMacroFunction_GetSequence::sm_SeqForDescr) + "(" + field + ")";
        }
    }
    return field;
}

string CFieldHandlerNamePanel::GetMacroFieldLabel(const string& target, const string& selected_field)
{
    string field;
    int val = m_Field->GetSelection();
    if (val > -1) 
    {
        field = ToStdString(m_Field->GetString(val));
    }
    if (m_self == "Misc")
    {
        if (field == kGenomeProjectID)
            return "obj.label.str = \"ProjectID\"";
    }
    if (m_self == "DBLink" && target == m_self)
    {
        return "obj.label.str = \"" + field + "\"";
    }
    return kEmptyStr;
}

void CFieldHandlerNamePanel::SetMacroFieldNames(const vector<string>& field_names)
{
    m_MacroField = field_names;    
}

bool CFieldHandlerNamePanel::SetFieldName(const string& field)
{
    bool rval = false;
    for (size_t i = 0; i < m_Field->GetStrings().size(); i++) {
        string misc_field = ToStdString(m_Field->GetString(i));
        CPubFieldType::EPubFieldType field_type = CPubFieldType::GetTypeForLabel(misc_field);
        if (objects::edit::CFieldHandler::QualifierNamesAreEquivalent(misc_field, field) ||
            misc_field == field ||
            (CPubFieldType::IsAffilField(field_type) && "affiliation " + misc_field == field)) {
            m_Field->SetSelection(i);
            m_Field->SetFirstItem(i);
            rval = true;
            break;
        }
    }
    x_UpdateParent();
    return rval;
}


void CFieldHandlerNamePanel::SetFieldNames(const vector<string>& field_names)
{
    wxArrayString field_strings;
    ITERATE(vector<string>, it, field_names) {
        field_strings.push_back(ToWxString(*it));
    }
    m_Field->Clear();
    m_Field->Append(field_strings);
    m_Field->SetSelection(0);
    m_Field->SetFirstItem(0);
}


vector<string> CFieldHandlerNamePanel::GetChoices(bool& allow_other)
{
    vector<string> choices;

    string field_name = GetFieldName();

    CMolInfoFieldType::EMolInfoFieldType field_type = CMolInfoFieldType::GetFieldType(field_name);
    if (field_type != CMolInfoFieldType::e_Unknown) {
        choices = CMolInfoFieldType::GetChoicesForField(field_type, allow_other);
    } else {
        CPubFieldType::EPubFieldType pub_field_type = CPubFieldType::GetTypeForLabel(field_name);
        if (pub_field_type != CPubFieldType::ePubFieldType_Unknown) {
            choices = CPubFieldType::GetChoicesForField(pub_field_type, allow_other);
        }
    }


    return choices;
}

vector<string> CFieldHandlerNamePanel::GetMacroValues()
{
    vector<string> values;
    
    string field_name = GetFieldName();
    
    CMolInfoFieldType::EMolInfoFieldType field_type = CMolInfoFieldType::GetFieldType(field_name);
    if (field_type != CMolInfoFieldType::e_Unknown) {
        values = CMolInfoFieldType::GetValuesForField(field_type);
    } else {
        CPubFieldType::EPubFieldType pub_field_type = CPubFieldType::GetTypeForLabel(field_name);
        if (pub_field_type != CPubFieldType::ePubFieldType_Unknown) {
            values = CPubFieldType::GetValuesForField(pub_field_type);
        }
    }

    return values;
}

END_NCBI_SCOPE

