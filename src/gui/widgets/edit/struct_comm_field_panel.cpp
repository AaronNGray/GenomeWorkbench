/*  $Id: struct_comm_field_panel.cpp 44072 2019-10-21 17:57:10Z asztalos $
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
 */


#include <ncbi_pch.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/auto_complete_text_ctrl.hpp>
#include <gui/widgets/wx/cont_text_completer.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>

#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const string kStrComm("Structured comment ");
/*!
 * CStructCommentFieldPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CStructCommentFieldPanel, CFieldNamePanel )


/*!
 * CStructCommentFieldPanel event table definition
 */

BEGIN_EVENT_TABLE( CStructCommentFieldPanel, CFieldNamePanel )

    EVT_CHOICE( ID_STRCMNT_CHOICE, CStructCommentFieldPanel::OnStrCmntChoiceSelected )
    EVT_TEXT( ID_STRCMNT_TXTCTRL, CStructCommentFieldPanel::OnFieldNameEnter )

END_EVENT_TABLE()


/*!
 * CStructCommentFieldPanel constructors
 */

CStructCommentFieldPanel::CStructCommentFieldPanel()
{
    Init();
}

CStructCommentFieldPanel::CStructCommentFieldPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CStructCommentFieldPanel creator
 */

bool CStructCommentFieldPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CStructCommentFieldPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CStructCommentFieldPanel creation
   return true;
}


/*!
 * CStructCommentFieldPanel destructor
 */

CStructCommentFieldPanel::~CStructCommentFieldPanel()
{
////@begin CStructCommentFieldPanel destruction
////@end CStructCommentFieldPanel destruction
}


/*!
 * Member initialisation
 */

void CStructCommentFieldPanel::Init()
{
////@begin CStructCommentFieldPanel member initialisation
    m_StructCommFieldType = NULL;
    m_FieldName = NULL;
////@end CStructCommentFieldPanel member initialisation
}


/*!
 * Control creation for CStructCommentFieldPanel
 */

void CStructCommentFieldPanel::CreateControls()
{    
////@begin CStructCommentFieldPanel content construction
    CStructCommentFieldPanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_StructCommFieldTypeStrings;
    m_StructCommFieldTypeStrings.Add(_("Field"));
    m_StructCommFieldTypeStrings.Add(_("Database Name"));
    m_StructCommFieldTypeStrings.Add(_("Field Name"));
    m_StructCommFieldType = new wxChoice( itemCFieldNamePanel1, ID_STRCMNT_CHOICE, wxDefaultPosition, wxDefaultSize, m_StructCommFieldTypeStrings, 0 );
    itemBoxSizer3->Add(m_StructCommFieldType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_StructCommFieldType->SetSelection(0);

    m_FieldName = new CAutoCompleteTextCtrl( itemCFieldNamePanel1, ID_STRCMNT_TXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(150,-1), 0 );
    itemBoxSizer3->Add(m_FieldName, 0, wxALIGN_CENTER_VERTICAL|wxRESERVE_SPACE_EVEN_IF_HIDDEN|wxALL, 5);

////@end CStructCommentFieldPanel content construction
}

void CStructCommentFieldPanel::ClearValues()
{
    m_StructCommFieldType->SetSelection(0);
    m_FieldName->SetValue(wxEmptyString);
}

string CStructCommentFieldPanel::GetFieldName(const bool subfield)
{
    string field = kEmptyStr;
    int val = m_StructCommFieldType->GetSelection();
    if (val > -1) {
        field = ToStdString(m_StructCommFieldType->GetString(val));
        field = kStrComm + field;
        if (val == 0) {
            string label = ToStdString(m_FieldName->GetValue());
            if (NStr::IsBlank(label)) {
                field += " undefined";
            } else {
                field += " " + label;
            }
        }
    }
    return field;
}

string CStructCommentFieldPanel::GetMacroFieldName(const string &target, const string& selected_field)
{
    string field = kEmptyStr;
    int val = m_StructCommFieldType->GetSelection();
    switch(val)
    {
    case 0 : field = macro::CMacroFunction_StructCommField::GetFuncName() + "(\"" + ToStdString(m_FieldName->GetValue()) + "\")"; break;
    case 1 : field = macro::CMacroFunction_StructCommDatabase::GetFuncName() + "()"; break;
    case 2 : field = macro::CMacroFunction_StructCommFieldname::GetFuncName() + "()"; break;
    default : break;
    }
    return field;   
}


bool CStructCommentFieldPanel::SetFieldName(const string& field)
{
    bool rval = false;
    string lcl_field = field.substr(kStrComm.length(), NPOS);

    int n = -1;
    if (NStr::EqualNocase(lcl_field, "field")) {
        n = 0;
    } else if (NStr::EqualNocase(lcl_field, "database name")) {
        n = 1;
    } else if (NStr::EqualNocase(lcl_field, "field name")) {
        n = 2;
    }

    if (n > -1) {
        m_StructCommFieldType->SetSelection(n);
        x_UpdatePanel();
        rval = true;
    }

    return rval;
}

void CStructCommentFieldPanel::OnStrCmntChoiceSelected( wxCommandEvent& event)
{
    x_UpdatePanel();
    event.Skip();
}

void CStructCommentFieldPanel::OnFieldNameEnter( wxCommandEvent& event)
{
    x_UpdateParent();
    event.Skip();
}

void CStructCommentFieldPanel::x_UpdatePanel()
{
    bool show = (m_StructCommFieldType->GetSelection() == 0) ? true : false;
    m_FieldName->Show(show);
    x_UpdateParent();
}
/*!
 * Should we show tooltips?
 */

bool CStructCommentFieldPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CStructCommentFieldPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CStructCommentFieldPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CStructCommentFieldPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CStructCommentFieldPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CStructCommentFieldPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CStructCommentFieldPanel icon retrieval
}


void CStructCommentFieldPanel::GetStructCommentFields(const CSeq_entry_Handle& seh, set<string> &fields, size_t max)
{
    size_t count = 0;    
    for ( CSeq_entry_CI entry_it(seh, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) {
        for ( CSeqdesc_CI desc_ci(*entry_it, CSeqdesc::e_User, 1); desc_ci; ++desc_ci) {
            if (CComment_rule::IsStructuredComment(desc_ci->GetUser())) {
                const CUser_object& user = desc_ci->GetUser();
                if (user.IsSetData()) {
                    ITERATE (CUser_object::TData, usr_it, user.GetData()) {
                        if ((*usr_it)->IsSetLabel() && (*usr_it)->GetLabel().IsStr()) {
                            string field = (*usr_it)->GetLabel().GetStr();
                            if (!NStr::EqualNocase(field, "StructuredCommentPrefix")
                                && !NStr::EqualNocase(field, "StructuredCommentSuffix")) {
                                fields.insert(field);
                            }
                        }
                    }
                }
            }
        }
        ++count;
        if (count > max)
            break;
    }
}


void CStructCommentFieldPanel::CompleteStructCommentFields(const set<string> &fields)
{
    if (fields.empty())
        return;

    for (const auto &str : fields)
    {
        m_Fields.insert( ToWxString(str) );
    }

    wxTextCompleter* textCompleter = new CContTextCompleter<set<wxString> >(m_Fields);
    m_FieldName->AutoComplete(textCompleter);
}

END_NCBI_SCOPE
