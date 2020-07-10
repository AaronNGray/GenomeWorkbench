/*  $Id: macro_panel_loader.cpp 44731 2020-03-03 15:53:48Z asztalos $
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
* Authors:  Vladislav Evgeniev
*/

#include <ncbi_pch.hpp>
#include <exception>
#include <wx/xrc/xmlres.h>
#include <wx/valgen.h>
#include <wx/window.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/listbox.h>
#include <wx/checklst.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/wupdlock.h>
#include <wx/tglbtn.h>
#include <wx/stattext.h>
#include <wx/filedlg.h>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_entry.hpp>

#include "structuredcomment_panel.hpp"
#include <gui/widgets/edit/rawseqtodeltabyn_panel.hpp>
#include <gui/widgets/edit/macro_applysrc_table.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <gui/widgets/edit/macro_authors_panel.hpp>
#include <gui/widgets/edit/macro_panel_loader.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(NMacroArgs);


BEGIN_EVENT_TABLE(CMacroPanelLoader, wxEvtHandler)
END_EVENT_TABLE()

bool CMacroPanelLoader::m_Initialized = false;

void CMacroPanelLoader::Initialize()
{
    wxXmlResource::Get()->InitAllHandlers();
    wxString path = CSysPath::ResolvePath(wxT("<std>/etc/macro_xrc"));
#ifdef NCBI_OS_MSWIN
    path.Replace("/", "\\");
#endif
    wxXmlResource::Get()->LoadAllFiles(path);

    wxImage::AddHandler(new wxPNGHandler);
    m_Initialized = true;
}

CMacroPanelLoader::CMacroPanelLoader(wxWindow *parent, const string &panel_name, const TArgumentsVector &arguments) 
{
    if (!m_Initialized)
        CMacroPanelLoader::Initialize();

    m_Panel = wxXmlResource::Get()->LoadPanel(parent, panel_name);
    if (nullptr == m_Panel) 
        throw runtime_error("Panel " + panel_name + " not found!");


    for (const auto &argument : arguments) {
        wxControl *control = XRCCTRL(*m_Panel, wxString(argument.m_Name), wxControl);
        if (control == nullptr) {
            throw runtime_error("Control " + argument.m_Name + " not found!");
        }

        if (m_Arguments.count(argument.m_Name) > 0) {
            throw runtime_error("Argument " + argument.m_Name + " already exists!");
        }
        m_Controls[argument.m_Name] = control;
        // to obtain the initial configuration of the controls from the panel
        m_Arguments.Add(argument, control->IsEnabled(), control->IsShown());
        m_Arguments[argument.m_Name].SetEnabled().Attach(this);
        m_Arguments[argument.m_Name].SetShown().Attach(this);
        BindEvents(*control);
    }
}

wxPanel* CMacroPanelLoader::LoadCustomPanel(ECustomPanelType type)
{
    _ASSERT(m_Panel);
    wxPanel* custom_panel = nullptr;
    string panel_name;
    EMacroFieldType field_type = EMacroFieldType::eNotSet;
    switch (type) {
    case ECustomPanelType::eAddAuthorsPanel:
        custom_panel = new CMacroAuthorNamesPanel(m_Panel);
        panel_name.assign("addauthors_panel");
        break;
    case ECustomPanelType::eAddStructCommPanel:
        custom_panel = new CStructuredCommentPanel(m_Panel, CRef<objects::CUser_object>(nullptr));
        panel_name.assign("addstructcomment_panel");
        break;
    case ECustomPanelType::eAddAssmGapsByNPanel: {
        bool m_AdjustCDS = false;
        bool m_AddLinkage = true;
        custom_panel = new CRawSeqToDeltaByNPanel(m_Panel, m_AdjustCDS, m_AddLinkage);
        panel_name.assign("addassemblygapsbyns_panel");
        break;
    }
    case ECustomPanelType::eApplySrcQualTable:
        field_type = EMacroFieldType::eBiosourceAll;
        break;
    case ECustomPanelType::eApplyStrCommTable:
        field_type = EMacroFieldType::eStructComment;
        break;
    case ECustomPanelType::eApplyPubQualTable:
        field_type = EMacroFieldType::ePubdesc;
        break;
    case ECustomPanelType::eApplyDBLinkTable:
        field_type = EMacroFieldType::eDBLink;
        break;
    case ECustomPanelType::eApplyMolinfoQualTable:
        field_type = EMacroFieldType::eMolinfo;
        break;
    case ECustomPanelType::eApplyMiscQualTable:
        field_type = EMacroFieldType::eMiscDescriptors;
        break;
    case ECustomPanelType::eApplyFeatQualTable:
        field_type = EMacroFieldType::eFeatQualifiers;
        break;
    case ECustomPanelType::eApplyCDSGeneProtTable:
        field_type = EMacroFieldType::eCdsGeneProt;
        break;
    case ECustomPanelType::eApplyRNAQualTable:
        field_type = EMacroFieldType::eRNA;
        break;
    }

    if (field_type != EMacroFieldType::eNotSet) {
        custom_panel = new CMacroApplySrcTablePanel(m_Panel, field_type);
        panel_name.assign("srcqualtable_panel");
    }

    if (wxXmlResource::Get()->AttachUnknownControl(wxString(panel_name), custom_panel)) {
        m_Panel->Layout();
        m_Panel->Fit();
        m_Panel->Refresh();
    }
    return custom_panel;
}

void CMacroPanelLoader::SetControlValues(const string &ctrl_name, const vector<string> &values)
{
    wxControl &ctrl = FindControl(ctrl_name);
    wxString class_name = ctrl.GetClassInfo()->GetClassName();
    if (class_name == "wxListBox") {
        Initialize((wxListBox&)ctrl, values);
    }
    else if (class_name == "wxCheckListBox") {
        Initialize((wxCheckListBox&)ctrl, values);
    }
    else if (class_name == "wxRadioBox") {
        Initialize((wxRadioBox&)ctrl, values);
    }
    else if (class_name == "wxTextCtrl") {
        Initialize((wxTextCtrl&)ctrl, values);
    }
    else if (class_name == "wxComboBox") {
        Initialize((wxComboBox&)ctrl, values);
    }
    else if (class_name == "wxChoice") {
        Initialize((wxChoice&)ctrl, values);
    }
    else if (class_name == "wxStaticText") {
        Initialize((wxStaticText&)ctrl, values);
    }
}

void CMacroPanelLoader::SetSelection(const string &ctrl_name, int n)
{
    wxControl &ctrl = FindControl(ctrl_name);
    wxString class_name = ctrl.GetClassInfo()->GetClassName();
    if (class_name == "wxChoice") {
        wxChoice& choice_ctrl = dynamic_cast<wxChoice&>(ctrl);
        choice_ctrl.SetSelection(n);
    }
    else if (class_name == "wxComboBox") {
        wxComboBox& combobox = dynamic_cast<wxComboBox&>(ctrl);
        combobox.SetSelection(n);
    } 
    else if (class_name == "wxListBox") {
        wxListBox& listbox = dynamic_cast<wxListBox&>(ctrl);
        listbox.SetSelection(n);
    }
}

void CMacroPanelLoader::OnComboBoxSelectionChanged(wxCommandEvent& event)
{
    wxComboBox *combo_box = dynamic_cast<wxComboBox*>(event.GetEventObject());
    if (nullptr == combo_box)
        return;

    m_Arguments[combo_box->GetName().ToStdString()].SetValue() = combo_box->GetValue().ToStdString();
    event.Skip();
}

void CMacroPanelLoader::OnChoiceSelectionChanged(wxCommandEvent& event)
{
    wxChoice *choice = dynamic_cast<wxChoice*>(event.GetEventObject());
    if (nullptr == choice)
        return;

    int val = choice->GetSelection();
    const string selected = choice->GetString(val).ToStdString();
        
    const string choice_name = choice->GetName().ToStdString();
    m_Arguments[choice_name].SetValue() = selected;

    if (choice_name == kRNAType) {
        const string rna_name = macro::CMacroFunction_ApplyRNA::sm_Arguments[0].m_Name;
        if (m_Arguments.count(rna_name) > 0) {
            vector<string> fieldnames = CMacroEditorContext::GetInstance().GetRNAProductnames(selected);
            SetControlValues(rna_name, fieldnames);
        }
    }
    else if (choice_name == kField) {
        if (m_Arguments.count(kExistingText) > 0) {
            wxControl& ctrl = FindControl(kExistingText);
            wxRadioBox& radiobox = dynamic_cast<wxRadioBox&>(ctrl);
            bool enable = false;
            if (NStr::EqualNocase(selected, "Field")) {
                enable = true;
            }
            radiobox.Enable(4, enable);
        }
    }
    event.Skip();
}

void CMacroPanelLoader::OnTextCtrlTextChanged(wxCommandEvent& event)
{
    wxTextCtrl *txt_ctrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if (nullptr == txt_ctrl)
        return;

    m_Arguments[txt_ctrl->GetName().ToStdString()].SetValue() = txt_ctrl->GetValue().ToStdString();
    event.Skip();
}

static EMacroFieldType s_GetSelectedMolinfoType(const string& name)
{
    EMacroFieldType type = EMacroFieldType::eNotSet;
    if (name == "molecule") {
        type = EMacroFieldType::eMolinfoMolecule;
    }
    else if (name == "technique") {
        type = EMacroFieldType::eMolinfoTech;
    }
    else if (name == "completedness") {
        type = EMacroFieldType::eMolinfoCompleteness;
    }
    else if (name == "class") {
        type = EMacroFieldType::eMolinfoClass;
    }
    else if (name == "topology") {
        type = EMacroFieldType::eMolinfoTopology;
    }
    else if (name == "strand") {
        type = EMacroFieldType::eMolinfoStrand;
    }

    return type;
}

void CMacroPanelLoader::OnListBoxSelectionChanged(wxCommandEvent& event)
{
    wxListBox *lst_box = dynamic_cast<wxListBox*>(event.GetEventObject());
    if (nullptr == lst_box)
        return;

    int selected_item = lst_box->GetSelection();
    if (selected_item == wxNOT_FOUND)
        return;

    const string& lst_box_name = lst_box->GetName().ToStdString();
    const string selected = lst_box->GetString(selected_item).ToStdString();
    m_Arguments[lst_box_name].SetValue() = selected;

    if (lst_box_name == kField) {
        if (m_Arguments.count(kNewValue) > 0) {
            EMacroFieldType molinfo_type = s_GetSelectedMolinfoType(selected);
            if (molinfo_type != EMacroFieldType::eNotSet) {
                vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(molinfo_type);
                SetControlValues(kNewValue, fieldnames);
            }
        }

        bool enable = true;
        if (selected == "status" || selected == "date" || selected == "year" || selected == "month" || selected == "day") {
            enable = false;
        }
        if (m_Arguments.count(kExistingText) > 0) {
            wxControl& ctrl = FindControl(kExistingText);
            wxRadioBox& radiobox = dynamic_cast<wxRadioBox&>(ctrl);
            for (size_t index = 1; index < 4; ++index) {
                radiobox.Enable(index, enable);
            }
        }
    }
    else if (lst_box_name == macro::CMacroFunction_ApplyOtherFeature::sm_Arguments[4].m_Name) {
        vector<string> quals = CMacroEditorContext::GetInstance().GetLegalQualifiers(selected);
        size_t arg_size = macro::CMacroFunction_ApplyOtherFeature::sm_Arguments.size();
        for (size_t index = 5; index < arg_size; index += 2) {
            SetControlValues(macro::CMacroFunction_ApplyOtherFeature::sm_Arguments[index].m_Name, quals);
        }
    }
    else if (lst_box_name == "from_feature" || lst_box_name == "to_feature") {
        string conversion_text = CMacroEditorContext::GetInstance().GetConversionDescr(m_Arguments["from_feature"].GetValue(), m_Arguments["to_feature"].GetValue());
        SetControlValues("conversion_descr", { conversion_text });
        
        if (lst_box_name == "to_feature") {
            const CTempString st_text = "selection_text";
            const CTempString choices = "choices";
            if (m_Arguments.count(st_text) > 0 && m_Arguments.count(choices) > 0) {
                vector<string> options;
                string label;
                if (selected == "bond") {
                    label = "Bond type";
                    options = CMacroEditorContext::GetInstance().GetBondTypes();
                }
                else if (selected == "site") {
                    label = "Site type";
                    options = CMacroEditorContext::GetInstance().GetSiteTypes();
                }
                else if (selected == "ncRNA") {
                    label = "ncRNA class";
                    options = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
                }
                else {
                    label = "ncRNA class";
                }
                SetControlValues(choices, options);
                SetSelection(choices, 0);
                wxControl& ctrl = FindControl(st_text);
                wxStaticText& text = dynamic_cast<wxStaticText&>(ctrl);
                text.SetLabel(label);
            }
        }
    }

    event.Skip();
}

void CMacroPanelLoader::OnCheckListBoxSelectionChanged(wxCommandEvent& event)
{
    wxCheckListBox* chklist_box = dynamic_cast<wxCheckListBox*>(event.GetEventObject());
    if (!chklist_box) return;

    m_Arguments[chklist_box->GetName().ToStdString()].SetValue() = x_GetCheckListBoxSelection(*chklist_box);
    event.Skip();
}

void CMacroPanelLoader::OnRadioBoxClicked(wxCommandEvent& event)
{
    wxRadioBox *radio_box = dynamic_cast<wxRadioBox*>(event.GetEventObject());
    if (nullptr == radio_box)
        return;

    m_Arguments[radio_box->GetName().ToStdString()].SetValue() = x_GetRadioboxSelection(*radio_box);
    event.Skip();
}

void CMacroPanelLoader::OnRadiobuttonClicked(wxCommandEvent& event)
{
    wxRadioButton *radio_btn = dynamic_cast<wxRadioButton*>(event.GetEventObject());
    if (nullptr == radio_btn)
        return;

    m_Arguments[radio_btn->GetName().ToStdString()].SetValue() = x_GetRadiobuttonSelection(*radio_btn);
    const string& rdbname = radio_btn->GetName().ToStdString();

    vector<string> fieldnames;
    if (m_Arguments.count(kField) > 0) {
        bool show = true;
        if (NStr::EqualNocase(rdbname, kBsrcOrigin)) {
            SetControlValues(kField, vector<string>());
            show = false;
            if (m_Arguments.count(kNewValue) > 0) {
                fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceOrigin);
                SetControlValues(kNewValue, fieldnames);
            }
        }
        else if (NStr::EqualNocase(rdbname, kBsrcLoc)) {
            SetControlValues(kField, vector<string>());
            show = false;
            if (m_Arguments.count(kNewValue) > 0) {
                fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceLocation);
                SetControlValues(kNewValue, fieldnames);
            }
        }
        else if (NStr::EqualNocase(rdbname, kBsrcTax)) {
            fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceTax);
            SetControlValues(kField, fieldnames);
            if (m_Arguments.count(kNewValue) > 0) {
                SetControlValues(kNewValue, vector<string>());
            }
        }
        else if (NStr::EqualNocase(rdbname, kBsrcText)) {
            auto type = m_Arguments.count(kTextLeft) > 0 ? EMacroFieldType::eBiosourceText : EMacroFieldType::eBiosourceTextAll;
            fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(type);
            SetControlValues(kField, fieldnames);
            if (m_Arguments.count(kNewValue) > 0) {
                SetControlValues(kNewValue, vector<string>());
            }
        }

        wxControl& ctrl = FindControl(kField);
        if (wxListBox* lbox = dynamic_cast<wxListBox*>(&ctrl)) {
            lbox->Show(show);
        }
    } 
    // for converting source qualifiers
    else if (NStr::EqualNocase(rdbname, kBsrcTextFrom)) {
        fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
        SetControlValues(kFromField, fieldnames);
    }
    else if (NStr::EqualNocase(rdbname, kBsrcTaxFrom)) {
        fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceTax);
        SetControlValues(kFromField, fieldnames);
    } else if (NStr::EqualNocase(rdbname, kBsrcTextTo)) {
        fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
        SetControlValues(kToField, fieldnames);
    } else if (NStr::EqualNocase(rdbname, kBsrcTaxTo)) {
        fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceTax);
        SetControlValues(kToField, fieldnames);
    }

    event.Skip();
}

void CMacroPanelLoader::OnButtonClicked(wxCommandEvent& event)
{
    wxButton *btn = dynamic_cast<wxButton*>(event.GetEventObject());
    if (nullptr == btn)
        return;

    const string& btnname = btn->GetName().ToStdString();

    if (btnname == kCopy) {
        // handle copy btn in the Edit panels
        SetControlValues(kRepl, { m_Arguments[kFind].GetValue() });
    }
    else if (btnname == kOpenFile) {
        wxFileDialog dlg(m_Panel, wxT("Select table file"), wxEmptyString, wxEmptyString, _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK) {
            auto mapped_path = CMacroEditorContext::GetInstance().GetAbsolutePath(dlg.GetPath());
            wxControl& ctrl = FindControl(kTableName);
            wxTextCtrl* textctrl = dynamic_cast<wxTextCtrl*>(&ctrl);
            if (textctrl) {
                textctrl->SetValue(mapped_path);
                textctrl->SetInsertionPoint(textctrl->GetLastPosition());
            }
        }
    } else {
        const string field = macro::CMacroFunction_ApplyOtherFeature::sm_Arguments[0].m_Name;
        string comment;
        if (m_Arguments.count(field) > 0) {
            comment = m_Arguments[field].GetValue();
            if (!comment.empty()) {
                comment += "; ";
            }
        }

        if (NStr::EqualNocase(btnname, macro::CMacroFunction_ApplyRNA::sm_Arguments[1].m_Name)) {
            comment += "contains 18S ribosomal RNA, internal transcribed spacer 1, 5.8S ribosomal RNA, internal transcribed spacer 2, and 28S ribosomal RNA";
        }
        else if (NStr::EqualNocase(btnname, macro::CMacroFunction_ApplyRNA::sm_Arguments[2].m_Name)) {
            comment += "contains 16S ribosomal RNA, 16S-23S ribosomal RNA intergenic spacer, and 23S ribosomal RNA";
        }
        SetControlValues(field, { comment });
    }

    event.Skip();
}

void CMacroPanelLoader::OnToggleButtonClicked(wxCommandEvent& event)
{
    wxToggleButton *btn = dynamic_cast<wxToggleButton*>(event.GetEventObject());
    if (nullptr == btn)
        return;

    bool state = btn->GetValue();
    const string& btn_name = btn->GetName().ToStdString();
    if (btn_name == macro::CMacroFunction_Autodef::sm_Arguments[4].m_Name) {
        string chklist_name = macro::CMacroFunction_Autodef::sm_Arguments[0].m_Name;
        if (m_Arguments.count(chklist_name) > 0) {
            wxControl &ctrl = FindControl(chklist_name);
            wxCheckListBox* chklistbox = dynamic_cast<wxCheckListBox*>(&ctrl);
            if (chklistbox) {
                for (size_t i = 0; i < chklistbox->GetCount(); ++i) {
                    chklistbox->Check(i, state);
                }
            }
        }
    }

    event.Skip();
}

void CMacroPanelLoader::OnCheckBoxClicked(wxCommandEvent& event)
{
    wxCheckBox *check_box = dynamic_cast<wxCheckBox*>(event.GetEventObject());
    if (nullptr == check_box)
        return;

    m_Arguments[check_box->GetName().ToStdString()].SetValue() = x_GetCheckboxSelection(*check_box);
    event.Skip();
}

void CMacroPanelLoader::Update(const bool& value)
{
    for (auto &arg : m_Arguments.GetArguments()) {
        wxControl &ctrl = *(m_Controls[arg->GetName()]);
        ctrl.Enable(arg->GetEnabled().get());
        ctrl.Show(arg->GetShown().get());
    }
    m_Panel->Refresh();
}


void CMacroPanelLoader::BindEvents(wxControl& ctrl)
{
    wxString class_name = ctrl.GetClassInfo()->GetClassName();
    if (class_name == "wxTextCtrl") {
        ctrl.Bind(wxEVT_TEXT, wxCommandEventHandler(CMacroPanelLoader::OnTextCtrlTextChanged), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxListBox") {
        ctrl.Bind(wxEVT_LISTBOX, wxCommandEventHandler(CMacroPanelLoader::OnListBoxSelectionChanged), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxCheckListBox") {
        ctrl.Bind(wxEVT_CHECKLISTBOX, wxCommandEventHandler(CMacroPanelLoader::OnCheckListBoxSelectionChanged), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxRadioBox") {
        ctrl.Bind(wxEVT_RADIOBOX, wxCommandEventHandler(CMacroPanelLoader::OnRadioBoxClicked), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxRadioButton") {
        ctrl.Bind(wxEVT_RADIOBUTTON, wxCommandEventHandler(CMacroPanelLoader::OnRadiobuttonClicked), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxButton") {
        ctrl.Bind(wxEVT_BUTTON, wxCommandEventHandler(CMacroPanelLoader::OnButtonClicked), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxToggleButton") {
        ctrl.Bind(wxEVT_TOGGLEBUTTON, wxCommandEventHandler(CMacroPanelLoader::OnToggleButtonClicked), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxBitmapButton") {
        ctrl.Bind(wxEVT_BUTTON, wxCommandEventHandler(CMacroPanelLoader::OnButtonClicked), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxCheckBox") {
        ctrl.Bind(wxEVT_CHECKBOX, wxCommandEventHandler(CMacroPanelLoader::OnCheckBoxClicked), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxComboBox") {
        ctrl.Bind(wxEVT_COMBOBOX, wxCommandEventHandler(CMacroPanelLoader::OnComboBoxSelectionChanged), this, XRCID(ctrl.GetName().c_str()));
    }
    else if (class_name == "wxChoice") {
        ctrl.Bind(wxEVT_CHOICE, wxCommandEventHandler(CMacroPanelLoader::OnChoiceSelectionChanged), this, XRCID(ctrl.GetName().c_str()));
    }
}

wxControl& CMacroPanelLoader::FindControl(const string &m_Name)
{
    if (m_Controls.end() == m_Controls.find(m_Name)) {
        throw runtime_error("Control " + m_Name + " not found!");
    }
    return *(m_Controls[m_Name]);
}

void CMacroPanelLoader::ToArrayString(const vector<string>& in, wxArrayString& out)
{
    size_t n = in.size();
    out.Alloc(n);
    for (size_t i = 0; i < n; i++){
        out.Add(wxString::FromUTF8(in[i].c_str()));
    }
}

void CMacroPanelLoader::Initialize(wxListBox &list_box, const vector<string> &values)
{
    wxWindowUpdateLocker locker(&list_box);
    wxArrayString wx_values;
    ToArrayString(values, wx_values);
    if (!list_box.IsEmpty()) {
        list_box.Clear();
    }
    list_box.Append(wx_values);
}

void CMacroPanelLoader::Initialize(wxRadioBox &radio_box, const vector<string> &values)
{
    if (radio_box.GetCount() != values.size())
        throw runtime_error("The number of values doesn't match the number of items for wxRadioBox " + radio_box.GetName().ToStdString());
    unsigned item = 0;
    for (const auto &value : values) {
        radio_box.SetItemHelpText(item++, wxString::FromUTF8(value.c_str()));
    }
}

void CMacroPanelLoader::Initialize(wxTextCtrl &txt_ctrl, const vector<string> &values)
{
    txt_ctrl.Clear();
    for (const auto &value : values) {
        txt_ctrl.AppendText(wxString::FromUTF8(value.c_str()));
    }
    txt_ctrl.ShowPosition(0);
}

void CMacroPanelLoader::Initialize(wxComboBox &combo_box, const vector<string> &values)
{
    combo_box.Clear();
    wxArrayString wx_values;
    ToArrayString(values, wx_values);
    combo_box.Set(wx_values);
}

void CMacroPanelLoader::Initialize(wxCheckListBox& chlist_box, const vector<string>& values)
{
    wxArrayString wx_values;
    ToArrayString(values, wx_values);
    chlist_box.Append(wx_values);
}

void CMacroPanelLoader::Initialize(wxChoice& choice, const vector<string>& values)
{
    choice.Clear();
    wxArrayString wx_values;
    ToArrayString(values, wx_values);
    choice.Append(wx_values);
}

void CMacroPanelLoader::Initialize(wxStaticText& st_text, const vector<string>& values)
{
    st_text.SetLabel(values.front());
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxTextCtrl &txt_ctrl)
{
    value = txt_ctrl.GetValue().ToStdString();
    return !(((string const&)value).empty());
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxListBox &lst_box)
{
    int selected_item = lst_box.GetSelection();
    if (wxNOT_FOUND == selected_item)
        return false;

    value = lst_box.GetString(selected_item).ToStdString();
    return !(((string const&)value).empty());
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxCheckListBox &chklist_box)
{
    value = x_GetCheckListBoxSelection(chklist_box);
    return !((string const&)value).empty();
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxRadioBox &radio_box)
{
    value = x_GetRadioboxSelection(radio_box);
    return !(((string const&)value).empty());
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxRadioButton &radio_btn)
{
    value = x_GetRadiobuttonSelection(radio_btn);
    return true;
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxCheckBox &chk_box)
{
    value = x_GetCheckboxSelection(chk_box);
    return true;
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxComboBox &combo_box)
{
    value = combo_box.GetValue().ToStdString();
    return !((string const&)value).empty();
}

bool CMacroPanelLoader::GetCtrlValue(TStdStringPublisher &value, const wxChoice &choice)
{
    int val = choice.GetSelection();
    if (wxNOT_FOUND == val)
        return false;

    value = choice.GetString(val).ToStdString();
    return !((string const&)value).empty();
}

string CMacroPanelLoader::x_GetRadioboxSelection(const wxRadioBox& radio_box)
{
    int selected_item = radio_box.GetSelection();
    if (wxNOT_FOUND == selected_item)
        return kEmptyStr;

    wxString wx_value = radio_box.GetItemHelpText(selected_item);
    if (wx_value.empty())
        wx_value = radio_box.GetString(selected_item);

    string value = wx_value.ToStdString();
    NStr::ReplaceInPlace(value, "&", "");
    NStr::ToLower(value);
    return value;
}


string CMacroPanelLoader::x_GetRadiobuttonSelection(const wxRadioButton& radio_btn)
{
    return (radio_btn.GetValue()) ? "true" : "false";
}

string CMacroPanelLoader::x_GetCheckboxSelection(const wxCheckBox& chk_box)
{
    return (chk_box.IsChecked()) ? "true" : "false";
}

string CMacroPanelLoader::x_GetCheckListBoxSelection(const wxCheckListBox& chklist_box)
{
    wxArrayInt checkedItems;
    chklist_box.GetCheckedItems(checkedItems);
    string test_names;
    for (auto& it : checkedItems) {
        if (!test_names.empty()) {
            test_names += ", ";
        }
        test_names += chklist_box.GetString(it);
    }
    return test_names;
}

bool CMacroPanelLoader::TransferDataFromPanel()
{
    for (auto &argument : m_Arguments.SetArguments()) {
        wxControl &ctrl = *(m_Controls[argument->GetName()]);
        wxString class_name = ctrl.GetClassInfo()->GetClassName();

        if (class_name == "wxTextCtrl") {
            GetCtrlValue(argument->SetValue(), (wxTextCtrl&)ctrl);
        }
        else if (class_name == "wxListBox") {
            GetCtrlValue(argument->SetValue(), (wxListBox&)ctrl);
        }
        else if (class_name == "wxCheckListBox") {
            GetCtrlValue(argument->SetValue(), (wxCheckListBox&)ctrl);
        }
        else if (class_name == "wxRadioBox") {
            GetCtrlValue(argument->SetValue(), (wxRadioBox&)ctrl);
        }
        else if (class_name == "wxRadioButton") {
            GetCtrlValue(argument->SetValue(), (wxRadioButton&)ctrl);
        }
        else if (class_name == "wxCheckBox") {
            GetCtrlValue(argument->SetValue(), (wxCheckBox&)ctrl);
        }
        else if (class_name == "wxComboBox") {
            GetCtrlValue(argument->SetValue(), (wxComboBox&)ctrl);
        }
        else if (class_name == "wxChoice") {
            GetCtrlValue(argument->SetValue(), (wxChoice&)ctrl);
        }
    }
    return true;
}

CMacroPanelLoader::~CMacroPanelLoader()
{
    if (m_Panel) {
        m_Panel->Destroy();
        m_Panel = nullptr;
    }
}

END_NCBI_SCOPE
