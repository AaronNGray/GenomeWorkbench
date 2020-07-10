/*  $Id: sequencingtechnologypanel.cpp 41241 2018-06-20 19:25:04Z bollin $
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
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/valid/Field_set.hpp>

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/listctrl.h>
#include <wx/statbox.h>


#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/toplevel.h>
#include <wx/scrolwin.h>
#include <wx/hyperlink.h>

#include <gui/widgets/edit/edit_object_seq_desc.hpp>
#include "structuredcomment_panel.hpp"
#include <gui/widgets/edit/sequencingtechnologypanel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSequencingTechnologyPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSequencingTechnologyPanel, wxPanel )


/*!
 * CSequencingTechnologyPanel event table definition
 */

BEGIN_EVENT_TABLE( CSequencingTechnologyPanel, wxPanel )

////@begin CSequencingTechnologyPanel event table entries
    EVT_CHECKBOX( ID_CHECKBOX12, CSequencingTechnologyPanel::OnOtherMethodClick )

    EVT_HYPERLINK( ID_BUTTON2, CSequencingTechnologyPanel::OnButton2HyperlinkClicked )

    EVT_HYPERLINK( ID_BUTTON, CSequencingTechnologyPanel::OnClearClicked )

////@end CSequencingTechnologyPanel event table entries

END_EVENT_TABLE()


/*!
 * CSequencingTechnologyPanel constructors
 */

CSequencingTechnologyPanel::CSequencingTechnologyPanel()
{
    Init();
}

CSequencingTechnologyPanel::CSequencingTechnologyPanel( wxWindow* parent, CRef<objects::CUser_object> user, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_User = new objects::CUser_object();
    if (user) {
        m_User->Assign(*user);
    }
    Create(parent, id, pos, size, style);
    SetRegistryPath("Dialogs.Edit.SequencingTechnology");
}


/*!
 * CSequencingTechnologyPanel creator
 */

bool CSequencingTechnologyPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSequencingTechnologyPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSequencingTechnologyPanel creation
    return true;
}


/*!
 * CSequencingTechnologyPanel destructor
 */

CSequencingTechnologyPanel::~CSequencingTechnologyPanel()
{
    SaveSettings();
}

static const char* kReplaceAll = "Replace All";

void CSequencingTechnologyPanel::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CSequencingTechnologyPanel::SaveSettings() const
{
    if (m_RegPath.empty())
        return;
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
    bool replace_all = false;
    wxSizer *sizer = NULL;
    wxWindow *win = GetParent();
    if (win)
        sizer = win->GetSizer();

    if (sizer)
    {
        wxSizerItemList& itemList = sizer->GetChildren();
        for (size_t i = 0; i < itemList.size(); i++)
        {
            wxCheckBox* c_btn = dynamic_cast<wxCheckBox*>(itemList[i]->GetWindow());
            if (c_btn && c_btn->GetLabel() == _("Replace All")) 
            {
                replace_all = c_btn->GetValue();
                break;
            }
        }
    }
    view.Set(kReplaceAll, replace_all);
}


bool CSequencingTechnologyPanel::GetReplaceAll()
{
    if (m_RegPath.empty())
        return false;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    bool replace_all = view.GetBool(kReplaceAll, false);
    return replace_all;
}

/*!
 * Member initialisation
 */

void CSequencingTechnologyPanel::Init()
{
////@begin CSequencingTechnologyPanel member initialisation
    m_MethodsSizer = NULL;
    m_OtherMethod = NULL;
    m_OtherMethodText = NULL;
    m_AssemblyProgramSizer = NULL;
    m_SingleAssemblyProgram = NULL;
    m_AddPrograms = NULL;
    m_AssemblyName = NULL;
    m_Coverage = NULL;
////@end CSequencingTechnologyPanel member initialisation
}


/*!
 * Control creation for CSequencingTechnologyPanel
 */

void CSequencingTechnologyPanel::CreateControls()
{    
////@begin CSequencingTechnologyPanel content construction
    CSequencingTechnologyPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString standardCommentCtrlStrings;
    CConstRef<objects::CComment_set> structuredCommentRules = CComment_set::GetCommentRules();
    if (structuredCommentRules) 
    {
        // add wxchoice with available comment rules
        standardCommentCtrlStrings.Add(wxEmptyString);
        ITERATE (CComment_set::Tdata, it, structuredCommentRules->Get()) 
        {
            const CComment_rule& rule = **it;
            string prefix = rule.GetPrefix();
            CStructuredCommentPanel::s_RemovePoundSigns(prefix);
            standardCommentCtrlStrings.Add(wxString(prefix));
        }
    }
                
    m_StandardCommentCtrl = new wxChoice( itemPanel1, ID_CHOICE11, wxDefaultPosition, wxDefaultSize, standardCommentCtrlStrings, 0 );
    itemBoxSizer2->Add(m_StandardCommentCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_StandardCommentCtrl->SetStringSelection(_("Assembly-Data-START"));

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("What methods were used to obtain these sequences?"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_MethodsSizer = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer3->Add(m_MethodsSizer, 0, wxALIGN_LEFT|wxALL, 0);

    wxCheckBox* itemCheckBox5 = new wxCheckBox( itemPanel1, ID_CHECKBOX5, _("Sanger dideoxy sequencing"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox5->SetValue(false);
    m_MethodsSizer->Add(itemCheckBox5, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox6 = new wxCheckBox( itemPanel1, ID_CHECKBOX6, _("454"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox6->SetValue(false);
    m_MethodsSizer->Add(itemCheckBox6, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemPanel1, ID_CHECKBOX7, _("Helicos"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    m_MethodsSizer->Add(itemCheckBox7, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox8 = new wxCheckBox( itemPanel1, ID_CHECKBOX8, _("Illumina"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox8->SetValue(false);
    m_MethodsSizer->Add(itemCheckBox8, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemPanel1, ID_CHECKBOX9, _("Ion Torrent"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    m_MethodsSizer->Add(itemCheckBox9, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( itemPanel1, ID_CHECKBOX10, _("Pacific Biosciences"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox10->SetValue(false);
    m_MethodsSizer->Add(itemCheckBox10, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox11 = new wxCheckBox( itemPanel1, ID_CHECKBOX11, _("SOLiD"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox11->SetValue(false);
    m_MethodsSizer->Add(itemCheckBox11, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_OtherMethod = new wxCheckBox( itemPanel1, ID_CHECKBOX12, _("Other"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OtherMethod->SetValue(false);
    itemBoxSizer12->Add(m_OtherMethod, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OtherMethodText = new wxTextCtrl( itemPanel1, ID_OTHERMETHODTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_OtherMethodText->Enable(false);
    itemBoxSizer12->Add(m_OtherMethodText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer15Static = new wxStaticBox(itemPanel1, wxID_ANY, _("What program(s) did you use to assemble your sequences?"));
    wxStaticBoxSizer* itemStaticBoxSizer15 = new wxStaticBoxSizer(itemStaticBoxSizer15Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer15, 0, wxGROW|wxALL, 5);

    m_AssemblyProgramSizer = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer15->Add(m_AssemblyProgramSizer, 0, wxGROW|wxALL, 0);

    m_SingleAssemblyProgram = new CSingleAssemblyProgramPanel( itemPanel1, ID_ASSEMBLYFOREIGN, wxDefaultPosition, wxSize(100, 100), 0 );
    m_AssemblyProgramSizer->Add(m_SingleAssemblyProgram, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 10);

    m_AddPrograms = new wxHyperlinkCtrl( itemPanel1, ID_BUTTON2, _("Add More Assembly Programs"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_AssemblyProgramSizer->Add(m_AddPrograms, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer19 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer19, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel1, wxID_STATIC, _("Assembly Name (optional):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AssemblyName = new wxTextCtrl( itemPanel1, ID_TEXTCTRL19, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer19->Add(m_AssemblyName, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText22 = new wxStaticText( itemPanel1, wxID_STATIC, _("Coverage (optional):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText22, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Coverage = new wxTextCtrl( itemPanel1, ID_TEXTCTRL20, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(m_Coverage, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer23, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
    
    wxHyperlinkCtrl* itemHyperlinkCtrl24 = new wxHyperlinkCtrl( itemPanel1, ID_BUTTON, _("Clear"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer23->Add(itemHyperlinkCtrl24, 0, wxALL, 5);


////@end CSequencingTechnologyPanel content construction
//    m_ListBook->GetListView()->Show(false);
    m_AssemblyProgram = m_SingleAssemblyProgram;
}


void CSequencingTechnologyPanel::x_ClearMethods()
{
      wxSizerItemList& itemList = m_MethodsSizer->GetChildren();
      size_t pos = 0;
      while (pos < itemList.size()) {                
          wxCheckBox* method_check = dynamic_cast<wxCheckBox*>(itemList[pos]->GetWindow());
          if (method_check) {
              method_check->SetValue(false);
          }
          pos++;
      }
      m_OtherMethod->SetValue(false);
      m_OtherMethodText->SetValue(wxEmptyString);
      m_OtherMethodText->Enable(false);
}


void CSequencingTechnologyPanel::x_ChangeToMultiplePrograms()
{
    if (!m_SingleAssemblyProgram) {
        return;
    }
    
    Freeze();
    string val = m_SingleAssemblyProgram->GetValue();
    while (m_AssemblyProgramSizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_AssemblyProgramSizer->GetItem(pos)->DeleteWindows();
        m_AssemblyProgramSizer->Remove(pos);
    }
    m_SingleAssemblyProgram = NULL;
    m_AssemblyProgram = NULL;

    m_AssemblyProgram = new CMultipleAssemblyProgramPanel( this, ID_ASSEMBLYFOREIGN, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    m_AssemblyProgramSizer->Add(m_AssemblyProgram, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_AssemblyProgram->SetValue(val);
    Layout();
    Thaw();
}


bool CSequencingTechnologyPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    x_ClearMethods();

    ITERATE (CUser_object::TData, it, m_User->GetData()) {
        string field_name = "";
        const CObject_id& obj_id = (*it)->GetLabel();
        if ( obj_id.IsStr() ) {
            field_name = obj_id.GetStr();
        }
        string value = "";
        if ((*it)->IsSetData()) {
            if ((*it)->GetData().IsStr()) {
                value = (*it)->GetData().GetStr();
            } else if ((*it)->GetData().IsInt()) {
                value = NStr::IntToString ((*it)->GetData().GetInt());
            }
        }

        if (NStr::EqualNocase (field_name, "Assembly Name")) {
            m_AssemblyName->SetValue(ToWxString (value));
        } else if (NStr::EqualNocase (field_name, "Coverage")) {
            m_Coverage->SetValue(ToWxString (value));
        } else if (NStr::EqualNocase (field_name, "Assembly Method")) {
            if (NStr::Find(value, ";") != string::npos) {
                x_ChangeToMultiplePrograms();
            }
            m_AssemblyProgram->SetValue(value);
        } else if (NStr::EqualNocase (field_name, "Sequencing Technology")) {
            vector<string> techs;
            NStr::Split (value, ";", techs);
            string other_methods = "";
            ITERATE (vector<string>, it, techs) {
                string tech = NStr::TruncateSpaces(*it);
                wxSizerItemList& itemList = m_MethodsSizer->GetChildren();
                bool found = false;
                size_t pos = 0;
                while (pos < itemList.size() && !found) {                
                    wxCheckBox* method_check = dynamic_cast<wxCheckBox*>(itemList[pos]->GetWindow());
                    if (method_check) {
                        if (NStr::EqualNocase(tech, ToStdString (method_check->GetLabel()))) {
                            method_check->SetValue(true);
                            found = true;
                        }
                    }
                    pos++;
                }
                if (!found) {
                    if (!NStr::IsBlank(other_methods)) {
                        other_methods += "; ";
                    }
                    other_methods += *it;
                }
            }
            if (!NStr::IsBlank(other_methods)) {
                m_OtherMethod->SetValue(true);
                m_OtherMethodText->SetValue(ToWxString (other_methods));
                m_OtherMethodText->Enable(true);
            }
        }
    }

    return true;
}


static const string kKnownFields[] = {
  "Assembly Method",
  "Assembly Name",
  "Coverage",
  "Sequencing Technology",
  "StructuredCommentPrefix",
  "StructuredCommentSuffix"
};

static const int kNumKnownFields = sizeof (kKnownFields) / sizeof (string);

static bool IsKnownField (const CObject_id& label)
{
    if (!label.IsStr()) {
        return false;
    }
    for (int i = 0; i < kNumKnownFields; i++) {
        if (NStr::EqualNocase(kKnownFields[i], label.GetStr())) {
            return true;
        }
    }
    return false;
}

    
bool CSequencingTechnologyPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    CUser_object::TData::iterator it = m_User->SetData().begin();
    while (it != m_User->SetData().end()) {
        if (IsKnownField((*it)->GetLabel())) {
            it = m_User->SetData().erase(it);
        } else {
            ++it;
        }
    }

    string prefix;
    if (m_StandardCommentCtrl) {
        prefix = ToStdString (m_StandardCommentCtrl->GetStringSelection());
        if (!NStr::IsBlank(prefix)) {
            // structured comment prefix
            CStructuredCommentPanel::s_AddPoundSigns(prefix);
            m_User->AddField ("StructuredCommentPrefix", prefix);
        }
    }    

    string program = m_AssemblyProgram->GetValue();
    if (!NStr::IsBlank(program)) {
        CUser_field& program = m_User->SetField("Assembly Method");
        program.SetData().SetStr(m_AssemblyProgram->GetValue());
    }
    // optional fields
    if (!m_AssemblyName->GetValue().IsEmpty()) {
        CUser_field& assembly_name = m_User->SetField("Assembly Name");
        assembly_name.SetData().SetStr(ToStdString(m_AssemblyName->GetValue()));
    }
    if (!m_Coverage->GetValue().IsEmpty()) {
        CUser_field& coverage = m_User->SetField("Coverage");
        coverage.SetData().SetStr(ToStdString(m_Coverage->GetValue()));
    }

    // Sequencing Technology field is last
    string method = "";
    wxSizerItemList& itemList = m_MethodsSizer->GetChildren();
    size_t pos = 0;
    while (pos < itemList.size()) {
        wxCheckBox* method_check = dynamic_cast<wxCheckBox*>(itemList[pos]->GetWindow());
        if (method_check) {
            if (method_check->GetValue()) {
                if (!NStr::IsBlank(method)) {
                    method += "; ";
                }
                method += ToStdString (method_check->GetLabel());
            }
        }
        pos++;
    }
    if (m_OtherMethod->GetValue()) {
        string other_method = ToStdString (m_OtherMethodText->GetValue());
        if (!NStr::IsBlank(other_method)) {
            if (!NStr::IsBlank(method)) {
                method += "; ";
            }
            method += other_method;
        }
    }
    CUser_field& seqtech = m_User->SetField ("Sequencing Technology");
    seqtech.SetData().SetStr(method);

    if (!NStr::IsBlank(prefix)) {
        NStr::ReplaceInPlace (prefix, "START##", "END##");
        m_User->AddField ("StructuredCommentSuffix", prefix);
    }

    return true;
}


CRef<objects::CUser_object> CSequencingTechnologyPanel::GetUser_object() const
{
    if (m_User)
        return m_User;

    return CRef<objects::CUser_object>();
}


/*!
 * Should we show tooltips?
 */

bool CSequencingTechnologyPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSequencingTechnologyPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSequencingTechnologyPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSequencingTechnologyPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSequencingTechnologyPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSequencingTechnologyPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSequencingTechnologyPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX12
 */

void CSequencingTechnologyPanel::OnOtherMethodClick( wxCommandEvent& event )
{
    m_OtherMethodText->Enable(m_OtherMethod->GetValue());
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_BUTTON
 */

void CSequencingTechnologyPanel::OnClearClicked( wxHyperlinkEvent& event )
{
    x_ClearMethods();

    m_AssemblyProgram->SetValue("");
    m_AssemblyName->SetValue(wxEmptyString);
    m_Coverage->SetValue (wxEmptyString);
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_BUTTON2
 */

void CSequencingTechnologyPanel::OnButton2HyperlinkClicked( wxHyperlinkEvent& event )
{
    x_ChangeToMultiplePrograms();
}


void CSequencingTechnologyPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    m_User.Reset(new CUser_object);
    m_User->Assign(desc.GetUser());
    TransferDataToWindow();
}


void CSequencingTechnologyPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    CRef<CUser_object> user = GetUser_object();
    desc.SetUser(*user);
}


END_NCBI_SCOPE

