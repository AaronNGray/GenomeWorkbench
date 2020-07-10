/*  $Id: structuredcomment_panel.cpp 43325 2019-06-13 11:27:12Z bollin $
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
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/valid/Comment_set.hpp>
#include <objects/valid/Field_set.hpp>
#include <serial/objistr.hpp>
#include <util/util_misc.hpp>
#include <objtools/cleanup/cleanup.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/hyperlink.hpp>
#include <gui/widgets/edit/macro_edit_action_panel.hpp>
#include "structuredcomment_panel.hpp"

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/toplevel.h>
#include <wx/scrolwin.h>
#include <wx/checkbox.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CStructuredCommentPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CStructuredCommentPanel, wxPanel )


/*!
 * CStructuredCommentPanel event table definition
 */

BEGIN_EVENT_TABLE( CStructuredCommentPanel, wxPanel )

////@begin CStructuredCommentPanel event table entries
    EVT_CHOICE( ID_CHOICE11, CStructuredCommentPanel::OnChoice11Selected )

    EVT_HYPERLINK( ID_HYPERLINKCTRL, CStructuredCommentPanel::OnHyperlinkctrlHyperlinkClicked )

////@end CStructuredCommentPanel event table entries

    EVT_HYPERLINK(wxID_ANY, CStructuredCommentPanel::OnDelete)

END_EVENT_TABLE()


/*!
 * CStructuredCommentPanel constructors
 */

CStructuredCommentPanel::CStructuredCommentPanel()
{
    Init();
}

CStructuredCommentPanel::CStructuredCommentPanel( wxWindow* parent, CRef<CUser_object> user, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_User = new objects::CUser_object();
    if (user) {
        m_User->Assign(*user);
    }
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CStructuredCommentPanel creator
 */

bool CStructuredCommentPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CStructuredCommentPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CStructuredCommentPanel creation
    return true;
}


/*!
 * CStructuredCommentPanel destructor
 */

CStructuredCommentPanel::~CStructuredCommentPanel()
{
////@begin CStructuredCommentPanel destruction
////@end CStructuredCommentPanel destruction
}


/*!
 * Member initialisation
 */

void CStructuredCommentPanel::Init()
{
////@begin CStructuredCommentPanel member initialisation
    m_StandardCommentSizer = NULL;
    m_StandardCommentTypeLabel = NULL;
    m_StandardCommentCtrl = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CStructuredCommentPanel member initialisation
}


void CStructuredCommentPanel::s_RemovePoundSigns (string& str)
{
    while (NStr::StartsWith (str, "#")) {
        str = str.substr(1);
    }
    while (NStr::EndsWith (str, "#")) {
        str = str.substr(0, str.length() - 1);
    }
}


void CStructuredCommentPanel::s_AddPoundSigns (string& str)
{
    if (NStr::StartsWith(str, "##")) {
        // done
    } else if (NStr::StartsWith (str, "#")) {
        // add one more
        str = "#" + str;
    } else {
        // add two
        str = "##" + str;
    }

    if (NStr::EndsWith(str, "##")) {
        // done
    } else if (NStr::EndsWith (str, "#")) {
        // add one more
      str = str + "#";
    } else {
        // add two
        str = str + "##";
    }
}


/*!
 * Control creation for CStructuredCommentPanel
 */

void CStructuredCommentPanel::CreateControls()
{    
////@begin CStructuredCommentPanel content construction
    CStructuredCommentPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_StandardCommentSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_StandardCommentSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_StandardCommentTypeLabel = new wxStaticText( itemPanel1, wxID_STATIC, _("Standard Comment Type"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StandardCommentSizer->Add(m_StandardCommentTypeLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_StandardCommentCtrlStrings;
    m_StandardCommentCtrl = new wxChoice( itemPanel1, ID_CHOICE11, wxDefaultPosition, wxDefaultSize, m_StandardCommentCtrlStrings, 0 );
    m_StandardCommentSizer->Add(m_StandardCommentCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Field Name"), wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Value"), wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer6->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl9 = new wxHyperlinkCtrl( itemPanel1, ID_HYPERLINKCTRL, _("Add Field"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer6->Add(itemHyperlinkCtrl9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ScrolledWindow, 1, wxGROW|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxBoxSizer(wxVERTICAL);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->SetMinSize(wxSize(-1, 250));



////@end CStructuredCommentPanel content construction

    m_StructuredCommentRules = CComment_set::GetCommentRules();

    if (m_StructuredCommentRules) {
        // add wxchoice with available comment rules
        m_StandardCommentCtrlStrings.Add(wxEmptyString);
        ITERATE (CComment_set::Tdata, it, m_StructuredCommentRules->Get()) {
            const CComment_rule& rule = **it;
            string prefix = rule.GetPrefix();
            s_RemovePoundSigns(prefix);
            m_StandardCommentCtrlStrings.Add(ToWxString(prefix));
        }
                
        m_StandardCommentCtrl->Append(m_StandardCommentCtrlStrings);
    } else {
    }
}


void CStructuredCommentPanel::s_AddRow (string field_name, string field_value, bool static_fieldname, bool is_required)
{
    wxBoxSizer* row_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_Sizer->Add (row_sizer, 0, wxALIGN_NOT|wxALL);
    if (static_fieldname) {
        wxStaticText* fieldNameText = new wxStaticText( m_ScrolledWindow, 0, ToWxString(field_name), wxDefaultPosition, wxSize(200, -1), 0 );
        row_sizer->Add(fieldNameText, 0, wxALIGN_NOT|wxALL);
        m_FieldNameCtrls.push_back(fieldNameText);
    } else {
        wxTextCtrl* fieldNameText = new wxTextCtrl( m_ScrolledWindow, 0, ToWxString(field_name), wxDefaultPosition, wxSize(200, -1), 0 );
        row_sizer->Add(fieldNameText, 0, wxALIGN_NOT|wxALL);
        m_FieldNameCtrls.push_back(fieldNameText);
    }

    wxStaticText* star = new wxStaticText(m_ScrolledWindow, 0, is_required ? wxT("(*)") : wxT("   "), wxDefaultPosition, wxSize(20, -1), 0);
    row_sizer->Add(star, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

    wxTextCtrl* fieldValueText = new wxTextCtrl( m_ScrolledWindow, 0, ToWxString(field_value), wxDefaultPosition, wxSize(200, -1), 0 );
    row_sizer->Add(fieldValueText, 0, wxALIGN_NOT|wxALL);
    fieldValueText->Bind(wxEVT_TEXT, &CStructuredCommentPanel::OnTextEntered, this);
    m_FieldValueCtrls.push_back(fieldValueText);


    if (!static_fieldname) {
        wxHyperlinkCtrl* itemHyperLink = new CHyperlink(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT(""));
        itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
        row_sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
    }

    int row_width;
    int row_height;
    fieldValueText->GetClientSize(&row_width, &row_height);
    m_TotalHeight += row_height;
    m_ScrollRate = row_height;

}


const string kStructuredCommentPrefix = "StructuredCommentPrefix";
const string kStructuredCommentSuffix = "StructuredCommentSuffix";

bool CStructuredCommentPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    Freeze();

    m_Sizer->Clear(true);

    m_TotalHeight = 0;
    m_ScrollRate = 0;

    m_FieldNameCtrls.clear();
    m_FieldValueCtrls.clear();

    // first, find prefix and add it, save suffix for later
    string prefix = "";
    string suffix = "";
    vector<string> field_names;
    vector<string> field_values;
    vector<bool> displayed;

    ITERATE (CUser_object::TData, it, m_User->GetData()) {
        string field_name = "";
        const CObject_id& obj_id = (*it)->GetLabel();
        if ( obj_id.IsStr() ) {
            field_name = obj_id.GetStr();
        }
        
        if (NStr::EqualNocase (field_name, kStructuredCommentPrefix)) {
            if ((*it)->IsSetData() && (*it)->GetData().IsStr()) {
                prefix = (*it)->GetData().GetStr();
                s_RemovePoundSigns(prefix);
            }
        } else if (NStr::EqualNocase(field_name, kStructuredCommentSuffix)) {
            if ((*it)->IsSetData() && (*it)->GetData().IsStr()) {
                suffix = (*it)->GetData().GetStr();
                s_RemovePoundSigns(suffix);
            }
        } else {
            field_names.push_back(field_name);
            string value = "";
            if ((*it)->IsSetData()) {
                if ((*it)->GetData().IsStr()) {
                    value = (*it)->GetData().GetStr();
                } else if ((*it)->GetData().IsInt()) {
                    value = NStr::IntToString ((*it)->GetData().GetInt());
                }
            }
            field_values.push_back(value);
            displayed.push_back(false);
        }
    }

    // if this is a standard comment, set the control, and list the expected fields first
    bool use_default_layout = true;

    if (m_StructuredCommentRules) {
        try {
            const CComment_rule& rule = m_StructuredCommentRules->FindCommentRule("##" + prefix + "##");
            m_StandardCommentCtrl->SetStringSelection(ToWxString (prefix));
            ITERATE (CComment_rule::TFields::Tdata, field_it, rule.GetFields().Get()) {
                bool is_required = (*field_it)->IsSetRequired() && (*field_it)->GetRequired();
                // find existing value if we have one
                // note - display multiple values here if present
                bool found = false;
                for (size_t i = 0; i < field_names.size(); i++) {
                    if (NStr::EqualNocase(field_names[i], (*field_it)->GetField_name())) {
                        s_AddRow((*field_it)->GetField_name(), field_values[i], true, is_required);
                        displayed[i] = true;
                        found = true;
                    }
                }
                if (!found) {
                    s_AddRow((*field_it)->GetField_name(), "", true, is_required);
                }
            }

            use_default_layout = false;
        } catch (CException ) {
        }
    }

    if (use_default_layout) {
        s_AddRow(kStructuredCommentPrefix, prefix, true);
    }

    for (size_t i = 0; i < field_names.size(); i++) {
        if (!displayed[i]) {
            s_AddRow (field_names[i], field_values[i]);
        }
    }

    if (use_default_layout) {
        s_AddRow(kStructuredCommentSuffix, suffix, true);
    }

    m_ScrolledWindow->SetVirtualSize(400 + 10, m_TotalHeight);
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
    m_ScrolledWindow->FitInside();

    Layout();
    Thaw();
    return true;
}


bool CStructuredCommentPanel::x_GetFields (bool keep_blanks)
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_User->ResetData();

    string prefix = "";

    if (m_StandardCommentCtrl) {
        prefix = ToStdString (m_StandardCommentCtrl->GetStringSelection());
        if (!NStr::IsBlank(prefix)) {
            // structured comment prefix
            s_AddPoundSigns(prefix);
            m_User->AddField (kStructuredCommentPrefix, prefix);
        }
    }    

    for (size_t i = 0; i < m_FieldValueCtrls.size(); i++) {
        string field_name = "";
        string field_value = "";

        /* first is either label or text */
        wxTextCtrl* fieldNameText = dynamic_cast<wxTextCtrl*> (m_FieldNameCtrls[i]);
        if (fieldNameText) {
            field_name = ToStdString(fieldNameText->GetValue());
        } else {
            wxStaticText* staticFieldNameText = dynamic_cast<wxStaticText*> (m_FieldNameCtrls[i]);
            if (staticFieldNameText) {
                field_name = ToStdString (staticFieldNameText->GetLabel());
            }
        }

        /* second is value - could be wxText or ?*/
        field_value = ToStdString(m_FieldValueCtrls[i]->GetValue());

        if (keep_blanks || (!NStr::IsBlank(field_name) && !NStr::IsBlank(field_value))) {
            if (NStr::EqualNocase(field_name, kStructuredCommentPrefix)
                || NStr::EqualNocase(field_name, kStructuredCommentSuffix)) {
                if (NStr::IsBlank (prefix)) {
                    s_AddPoundSigns (field_value);
                    m_User->AddField (field_name, field_value);
                }
            } else {
                m_User->AddField (field_name, field_value);
            }
        }
    }  

    if (!NStr::IsBlank(prefix)) {
        NStr::ReplaceInPlace (prefix, "START##", "END##");
        m_User->AddField (kStructuredCommentSuffix, prefix);
    }
    return true;
}


bool CStructuredCommentPanel::TransferDataFromWindow()
{
    return x_GetFields (false);
}


CRef<objects::CUser_object> CStructuredCommentPanel::GetUser_object() const
{
    if (m_User)
        return m_User;

    return CRef<objects::CUser_object>();
}


/*!
 * Should we show tooltips?
 */

bool CStructuredCommentPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CStructuredCommentPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CStructuredCommentPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CStructuredCommentPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CStructuredCommentPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CStructuredCommentPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CStructuredCommentPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL
 */

void CStructuredCommentPanel::OnHyperlinkctrlHyperlinkClicked( wxHyperlinkEvent& event )
{
    x_GetFields(true);
    m_User->AddField("", "");
    TransferDataToWindow();
}


int CStructuredCommentPanel::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return -1;

    int row_num = 0;
    if (!NStr::IsBlank(ToStdString(m_StandardCommentCtrl->GetStringSelection()))) {
        // add one because prefix field will be first in user object but will not
        // be represented by a sizer
        row_num ++;
    }
    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it, ++row_num) {
        wxSizer* sizer = (**it).GetSizer();
        if (sizer && sizer->GetItem(wnd))
            return row_num;
    }

    return -1;
}


static bool s_ShouldPropagateEvent(wxWindow* w)
{
    while (w) {
        w = w->GetParent();
        if (dynamic_cast<CMacroEditingActionPanel*>(w)) {
            return true;
        }
    }
    return false;
}

void CStructuredCommentPanel::OnDelete (wxHyperlinkEvent& event)
{
    wxSizerItemList& itemList = m_Sizer->GetChildren();

    int row_num = x_FindRow((wxWindow*)event.GetEventObject(), itemList);
    if (row_num > -1) {
        x_GetFields(true);
        int row = 0;
        NON_CONST_ITERATE (CUser_object::TData, it, m_User->SetData()) {
            if (row == row_num) {
                it = m_User->SetData().erase(it);
                break;
            }
            row++;
        }
        TransferDataToWindow();
        if (s_ShouldPropagateEvent(this))
            event.Skip();
    }
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE11
 */

void CStructuredCommentPanel::OnChoice11Selected( wxCommandEvent& event )
{
    x_GetFields(false);
    TransferDataToWindow();
    if (s_ShouldPropagateEvent(this))
        event.Skip();
}

void CStructuredCommentPanel::OnTextEntered(wxCommandEvent& event)
{
    if (s_ShouldPropagateEvent(this)) {
        TransferDataFromWindow();
        event.Skip();
    }
}


void CStructuredCommentPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    m_User.Reset(new CUser_object);
    m_User->Assign(desc.GetUser());
    TransferDataToWindow();
}


void CStructuredCommentPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    CRef<CUser_object> user = GetUser_object();
    CCleanup::CleanupUserObject(*user); 
    desc.SetUser(*user);
}

bool CStructuredCommentPanel::IsImportEnabled()
{
    return RunningInsideNCBI();
}

bool CStructuredCommentPanel::IsExportEnabled()
{
    return RunningInsideNCBI();
}

CRef<CSerialObject> CStructuredCommentPanel::OnExport()
{
    TransferDataFromWindow();
    CRef<CSerialObject> so((CSerialObject*)(new objects::CUser_object));
    so->Assign(*GetUser_object());
    auto user = dynamic_cast<CUser_object*>(so.GetPointer());
    CCleanup::CleanupUserObject(*user); 
    if (!user->IsSetData() || user->GetData().empty()) {
        //create dummy field so that it can be exported
        user->AddField(kStructuredCommentPrefix, "");
    }
    return so;
}

void CStructuredCommentPanel::OnImport( CNcbiIfstream &istr)
{
    m_User.Reset(new CUser_object);
    istr >> MSerial_AsnText >> *m_User;
    TransferDataToWindow();
}

END_NCBI_SCOPE
