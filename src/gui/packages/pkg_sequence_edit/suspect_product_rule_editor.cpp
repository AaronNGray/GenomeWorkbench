/*  $Id: suspect_product_rule_editor.cpp 41973 2018-11-26 17:21:37Z kachalos $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <objects/macro/String_constraint.hpp>
#include <objects/macro/Suspect_rule_set.hpp>
#include <objects/macro/Replace_func.hpp>
#include <objects/macro/Simple_replace.hpp>
#include <objects/macro/Constraint_choice_set.hpp>
#include <objects/macro/Constraint_choice.hpp>
#include <objtools/edit/text_object_description.hpp>
#include <serial/objistr.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/utils.hpp>
#include <misc/discrepancy/discrepancy.hpp>
#include <gui/widgets/edit/word_substitute_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/filename.h>

#include <gui/packages/pkg_sequence_edit/suspect_product_rule_editor.hpp>



BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CSuspectProductRulesEditor, wxFrame )


/*!
 * CSuspectProductRulesEditor event table definition
 */


BEGIN_EVENT_TABLE( CSuspectProductRulesEditor, wxFrame )
EVT_MENU( ID_LOAD_RULE_LIST_MENU, CSuspectProductRulesEditor::OnFileLoad)
EVT_MENU( ID_ADD_RULE_LIST_MENU, CSuspectProductRulesEditor::OnFileAdd)
EVT_MENU( ID_SAVE_RULE_LIST_MENU, CSuspectProductRulesEditor::OnFileSave)
EVT_UPDATE_UI( ID_SAVE_RULE_LIST_MENU, CSuspectProductRulesEditor::CanFileSave )
EVT_MENU( ID_SAVE_AS_RULE_LIST_MENU, CSuspectProductRulesEditor::OnFileSaveAs)
EVT_UPDATE_UI( ID_SAVE_AS_RULE_LIST_MENU, CSuspectProductRulesEditor::CanFileSaveAs )
EVT_MENU( ID_QUIT_RULE_LIST_MENU, CSuspectProductRulesEditor::OnQuit)
EVT_UPDATE_UI( ID_SORT_FIND_RULE_LIST_MENU, CSuspectProductRulesEditor::CanSort )
EVT_MENU( ID_SORT_FIND_RULE_LIST_MENU, CSuspectProductRulesEditor::OnSortByFind )
EVT_UPDATE_UI( ID_SORT_CAT_RULE_LIST_MENU, CSuspectProductRulesEditor::CanSort )
EVT_MENU( ID_SORT_CAT_RULE_LIST_MENU, CSuspectProductRulesEditor::OnSortByCat )
EVT_BUTTON( ID_DISPLAY_TEXT_RULE_LIST_BUTTON, CSuspectProductRulesEditor::OnDisplay )
EVT_BUTTON( ID_SHOW_DIFFS_RULE_LIST_BUTTON, CSuspectProductRulesEditor::OnShowDiffs )
EVT_BUTTON( ID_APPLY_FIXES_RULE_LIST_BUTTON, CSuspectProductRulesEditor::OnApplyFixes )
EVT_BUTTON( ID_LIST_MATCHES_RULE_LIST_BUTTON, CSuspectProductRulesEditor::OnListMatches )
EVT_BUTTON( ID_DELETE_SELECTED_RULE_LIST_BUTTON, CSuspectProductRulesEditor::OnDeleteRules )
EVT_BUTTON( ID_ADD_BEFORE_RULE_LIST_BUTTON, CSuspectProductRulesEditor::OnAddRuleBefore )
EVT_BUTTON( ID_ADD_AFTER_RULE_LIST_BUTTON, CSuspectProductRulesEditor::OnAddRuleAfter )
END_EVENT_TABLE()


/*!
 * CSuspectProductRulesEditor constructors
 */

CSuspectProductRulesEditor::CSuspectProductRulesEditor()
{
    Init();
}


CSuspectProductRulesEditor::CSuspectProductRulesEditor( wxWindow* parent, IWorkbench* workbench,
               wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(workbench)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


bool CSuspectProductRulesEditor::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSuspectProductRulesEditor creation
//    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CSuspectProductRulesEditor creation

    return true;
}


/*!
 * CSuspectProductRulesEditor destructor
 */

CSuspectProductRulesEditor::~CSuspectProductRulesEditor()
{
////@begin CSuspectProductRulesEditor destruction
////@end CSuspectProductRulesEditor destruction
}


/*!
 * Member initialisation
 */

void CSuspectProductRulesEditor::Init()
{
    m_Panel = NULL;
    m_DeleteButton = NULL;
}


/*!
 * Control creation for CSuspectProductRulesEditor
 */

void CSuspectProductRulesEditor::CreateControls()
{    
    wxMenuBar *menubar = new wxMenuBar();
    wxMenu *file_menu = new wxMenu();
    menubar->Append(file_menu, wxT("&File"));

    wxMenuItem *load_item = new wxMenuItem(file_menu, ID_LOAD_RULE_LIST_MENU, _("Load Rule List"));
    file_menu->Append(load_item);

    wxMenuItem *add_item = new wxMenuItem(file_menu, ID_ADD_RULE_LIST_MENU, _("Add Rules from File to List"));
    file_menu->Append(add_item);

    wxMenuItem *save_item = new wxMenuItem(file_menu, ID_SAVE_RULE_LIST_MENU, _("Save Rule List"));
    file_menu->Append(save_item);

    wxMenuItem *save_as_item = new wxMenuItem(file_menu, ID_SAVE_AS_RULE_LIST_MENU, _("Save As"));
    file_menu->Append(save_as_item);

    wxMenuItem *separator = new wxMenuItem(file_menu);
    file_menu->Append(separator);

    wxMenuItem *quit_item = new wxMenuItem(file_menu, ID_QUIT_RULE_LIST_MENU, _("Quit"));
    file_menu->Append(quit_item);

    wxMenu *sort_menu = new wxMenu();
    menubar->Append(sort_menu, wxT("&Sort"));

    wxMenuItem *by_find_item = new wxMenuItem(sort_menu, ID_SORT_FIND_RULE_LIST_MENU, _("By Find"));
    sort_menu->Append(by_find_item);

    wxMenuItem *by_cat_item = new wxMenuItem(sort_menu, ID_SORT_CAT_RULE_LIST_MENU, _("By Category, then Find"));
    sort_menu->Append(by_cat_item);

    SetMenuBar(menubar);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel *Dialog1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(Dialog1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    Dialog1->SetSizer(itemBoxSizer2);

    wxArrayInt order;
    wxArrayString items;

    wxBoxSizer *BoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(BoxSizer, 1, wxGROW|wxALL, 5);    
    m_Panel = new CRearrangeCtrl(Dialog1, wxID_ANY, wxDefaultPosition, wxSize(-1, 350), order, items);
    BoxSizer->Add(m_Panel, 1, wxGROW|wxALL, 5);
    m_Panel->GetList()->SetCheckListListener(this);
    m_Panel->GetList()->SetDoubleClickConsumer(this);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton *button7 = new wxButton( Dialog1, ID_ADD_BEFORE_RULE_LIST_BUTTON, _("Add Before"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(button7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton *button8 = new wxButton( Dialog1, ID_ADD_AFTER_RULE_LIST_BUTTON, _("Add After"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(button8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DeleteButton = new wxButton( Dialog1, ID_DELETE_SELECTED_RULE_LIST_BUTTON, _("Delete Selected"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(m_DeleteButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DeleteButton->Disable();
   
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton *button1 = new wxButton( Dialog1, ID_LIST_MATCHES_RULE_LIST_BUTTON, _("List current matches"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(button1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton *button2 = new wxButton( Dialog1, ID_APPLY_FIXES_RULE_LIST_BUTTON, _("Apply fixes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(button2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton *button5 = new wxButton( Dialog1, ID_SHOW_DIFFS_RULE_LIST_BUTTON, _("Show diffs with other file"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(button5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton *button6 = new wxButton( Dialog1, ID_DISPLAY_TEXT_RULE_LIST_BUTTON, _("Display Rule Text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(button6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CSuspectProductRulesEditor::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSuspectProductRulesEditor::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CSuspectProductRulesEditor::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

void CSuspectProductRulesEditor::OnFileLoad( wxCommandEvent& event )
{
    wxFileDialog file(this, wxT("Load Rule List"), m_DefaultDir, wxEmptyString, CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK)
        return;

    m_File = file.GetPath();
    if (m_File.IsEmpty())
        return;
    wxFileName filename(m_File);
    m_DefaultDir = filename.GetPath();

    CRef<CSuspect_rule_set> rule_set(new CSuspect_rule_set);
    unique_ptr<CObjectIStream> obj_str(CObjectIStream::Open(eSerial_AsnText, m_File.ToStdString()));
    obj_str->Read(ObjectInfo(*rule_set));
    obj_str->Close();
    
    if (!rule_set->IsSet())
        return;

    m_Rules.clear();
    wxArrayInt order;
    wxArrayString items;
    int i = 0;
    for (const auto &rule : rule_set->Get())
    {
        items.Add(wxString(rule->SummarizeRule()));
        order.Add(~i);
        m_Rules.push_back(rule);
        i++;
    }
    CRearrangeList *list = m_Panel->GetList();
    list->Set(order, items);
    m_DeleteButton->Disable();
}

void CSuspectProductRulesEditor::OnFileAdd( wxCommandEvent& event )
{
    wxFileDialog file(this, wxT("Add Rule List"), m_DefaultDir, wxEmptyString, CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK)
        return;

    wxString filepath = file.GetPath();
    if (filepath.IsEmpty())
        return;
    wxFileName filename(filepath);
    m_DefaultDir = filename.GetPath();

    CRef<CSuspect_rule_set> rule_set(new CSuspect_rule_set);
    unique_ptr<CObjectIStream> obj_str(CObjectIStream::Open(eSerial_AsnText, filepath.ToStdString()));
    obj_str->Read(ObjectInfo(*rule_set));
    obj_str->Close();
    
    if (!rule_set->IsSet())
        return;

    CRearrangeList *list = m_Panel->GetList();

    wxArrayInt order = list->GetCurrentOrder();
    wxArrayString items;
    for (auto i : order)
    {
        if ( i < 0 )
            i = -i - 1;
        const auto rule = m_Rules[i];
        items.Add(rule->SummarizeRule());
    }
    int i = order.size();
    for (const auto rule : rule_set->Get())
    {
        items.Add(wxString(rule->SummarizeRule()));
        order.Add(~i);
        m_Rules.push_back(rule);
        i++;
    }
    list->Set(order, items);
}

void CSuspectProductRulesEditor::CanFileSave(wxUpdateUIEvent &event)
{
    event.Enable(!m_File.IsEmpty() && !m_Rules.empty());
}

void CSuspectProductRulesEditor::OnFileSave( wxCommandEvent& event )
{
    if (m_File.IsEmpty() || m_Rules.empty())
        return;

    CRef<CSuspect_rule_set> rule_set(new CSuspect_rule_set);
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    for (auto i : order)
    {
        if ( i < 0 )
            i = -i - 1;
        const auto rule = m_Rules[i];
        rule_set->Set().push_back(rule);
    }
    CNcbiOfstream os(m_File.fn_str(), ios::out);
    os << MSerial_AsnText << *rule_set;
}

void CSuspectProductRulesEditor::CanFileSaveAs(wxUpdateUIEvent &event)
{
    event.Enable(!m_Rules.empty());
}

void CSuspectProductRulesEditor::OnFileSaveAs( wxCommandEvent& event )
{
    if (m_Rules.empty())
        return;

    wxFileDialog file(this, wxT("Save As"), m_DefaultDir, wxEmptyString, CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (file.ShowModal() != wxID_OK)
        return;

    m_File = file.GetPath();
    if (m_File.IsEmpty())
        return;
    wxFileName filename(m_File);
    m_DefaultDir = filename.GetPath();

    CRef<CSuspect_rule_set> rule_set(new CSuspect_rule_set);
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    for (auto i : order)
    {
        if ( i < 0 )
            i = -i - 1;
        const auto rule = m_Rules[i];
        rule_set->Set().push_back(rule);
    }
    CNcbiOfstream os(m_File.fn_str(), ios::out);
    os << MSerial_AsnText << *rule_set;
}

void CSuspectProductRulesEditor::OnQuit( wxCommandEvent& event )
{
    Close();
}

void CSuspectProductRulesEditor::CanSort(wxUpdateUIEvent &event)
{
    event.Enable(!m_Rules.empty());
}

static string s_GetFind( CRef<CSuspect_rule> rule)
{
    string str;
    if (rule->IsSetFind())
    {
        if (rule->GetFind().IsString_constraint() && rule->GetFind().GetString_constraint().IsSetMatch_text())
        {
            str = rule->GetFind().GetString_constraint().GetMatch_text();
        }
        if (rule->GetFind().IsPrefix_and_numbers())
        {
            str = rule->GetFind().GetPrefix_and_numbers();
        }
        if (rule->GetFind().IsHas_term())
        {
            str = rule->GetFind().GetHas_term();
        }
    }

    return NStr::ToLower(str);
}

class CCompareByFind
{
public:
    CCompareByFind(const vector< CRef<CSuspect_rule> > &rules) : m_Rules(rules) {}
    bool operator()(const int &a, const int& b)
        {
            int i = a;
            int j = b;
            if ( i < 0 )
                i = -i - 1;
            const auto rule1 = m_Rules[i];
            if ( j < 0 )
                j = -j - 1;
            const auto rule2 = m_Rules[j];
            string str1 = s_GetFind(rule1);
            string str2 = s_GetFind(rule2);
            return str1 < str2;
        }
private:
    const vector< CRef<CSuspect_rule> > &m_Rules;
};

void CSuspectProductRulesEditor::OnSortByFind( wxCommandEvent& event )
{
    if (m_Rules.empty())
        return;
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    stable_sort(order.begin(), order.end(), CCompareByFind(m_Rules));
    wxArrayString items;
    for (const auto &rule : m_Rules)
    {
        items.Add(wxString(rule->SummarizeRule()));
    }
    list->Set(order, items);
}

class CCompareByCat
{
public:
    CCompareByCat(const vector< CRef<CSuspect_rule> > &rules) : m_Rules(rules) {}
    bool operator()(const int &a, const int& b)
        {
            int i = a;
            int j = b;
            if ( i < 0 )
                i = -i - 1;
            const auto rule1 = m_Rules[i];
            if ( j < 0 )
                j = -j - 1;
            const auto rule2 = m_Rules[j];
            pair<string, string> pair1 = make_pair(rule1->GetRuleTypeName(), s_GetFind(rule1));
            pair<string, string> pair2 = make_pair(rule2->GetRuleTypeName(), s_GetFind(rule2));
            return pair1 < pair2;
        }
private:
    const vector< CRef<CSuspect_rule> > &m_Rules;
};

void CSuspectProductRulesEditor::OnSortByCat( wxCommandEvent& event )
{
    if (m_Rules.empty())
        return;
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    stable_sort(order.begin(), order.end(), CCompareByCat(m_Rules));
    wxArrayString items;
    for (const auto &rule : m_Rules)
    {
        items.Add(wxString(rule->SummarizeRule()));
    }
    list->Set(order, items);
}

void CSuspectProductRulesEditor::OnDisplay( wxCommandEvent& event )
{
    if (m_Rules.empty())
        return;
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    wxString msg;
    for (auto i : order)
    {
        if ( i < 0 )
            i = -i - 1;
        const auto rule = m_Rules[i];
        string summary = rule->SummarizeRule();
        msg << summary << "\n";
    }
    if (msg.IsEmpty())
        return;
    CGenericReportDlg* report = new CGenericReportDlg(this);
    report->SetTitle(wxT("Suspect Rule Descriptions"));
    report->SetText(msg);
    report->Show(true);
}


void CSuspectProductRulesEditor::OnShowDiffs( wxCommandEvent& event )
{
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    set<string> current;
    for (auto i : order)
    {
        if ( i < 0 )
            i = -i - 1;
        const auto rule = m_Rules[i];
        current.insert(rule->SummarizeRule());
    }

    wxFileDialog file(this, wxT("Open Rule List"), m_DefaultDir, wxEmptyString, CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK)
        return;

    wxString path = file.GetPath();
    if (path.IsEmpty())
        return;
    wxFileName filename(path);
    m_DefaultDir = filename.GetPath();

    CRef<CSuspect_rule_set> rule_set(new CSuspect_rule_set);
    unique_ptr<CObjectIStream> obj_str(CObjectIStream::Open(eSerial_AsnText, path.ToStdString()));
    obj_str->Read(ObjectInfo(*rule_set));
    obj_str->Close();
    
    if (!rule_set->IsSet())
        return;

    set<string> remote;
    for (const auto &rule : rule_set->Get())
    {
        remote.insert(rule->SummarizeRule());
    }
    vector<string> current_minus_remote(current.size());
    vector<string> remote_minus_current(remote.size());
    set_difference(current.begin(), current.end(), remote.begin(), remote.end(), current_minus_remote.begin());
    set_difference(remote.begin(), remote.end(), current.begin(), current.end(), remote_minus_current.begin());

    wxString msg1;
    for (const auto &s : current_minus_remote)
    {
        msg1 << s << "\n";
    }
    wxString msg2;
    for (const auto &s : remote_minus_current)
    {
        msg2 << s << "\n";
    }

    CGenericReportDlg* report1 = new CGenericReportDlg(this);
    report1->SetTitle(_("Found in current list but not ") + path);
    report1->SetText(msg1);
    report1->Show(true);

    CGenericReportDlg* report2 = new CGenericReportDlg(this);
    report2->SetTitle(_("Found in ") + path + _(" but not in current list"));
    report2->SetText(msg2);
    report2->Show(true);

}

bool CSuspectProductRulesEditor::MatchConstraint(CRef<CSuspect_rule> rule, const string &str)
{
    if (!rule->IsSetFeat_constraint())
        return true;
    if (!rule->GetFeat_constraint().IsSet())
        return true;
    if (rule->GetFeat_constraint().Get().empty())
        return true;
    for (auto c : rule->GetFeat_constraint().Get())
    {
        if (!c->IsString())
            continue;
        if (!c->GetString().Match(str))
            return false;
    }
    return true;
}

void CSuspectProductRulesEditor::OnApplyFixes( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager();

    if (!m_TopSeqEntry || !m_CmdProcessor)
        return;
    wxBusyCursor wait;
    CRef<CCmdComposite> composite( new CCmdComposite("Fix Product Names") );
    bool any_modified = false;
    for (CFeat_CI feat_ci(m_TopSeqEntry); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);
        bool modified = false; 
        if (new_feat->IsSetData() && new_feat->GetData().IsProt() && new_feat->GetData().GetProt().IsSetName())
        {
            EDIT_EACH_NAME_ON_PROTREF(name, new_feat->SetData().SetProt())
            {
                string new_name = *name;
                for (auto rule : m_Rules)
                {
                    if (MatchConstraint(rule, new_name))
                        rule->ApplyToString(new_name);
                }

                modified |= new_name != *name;
                *name = new_name;
            }
        }
	
        if (new_feat->IsSetData() && new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt() && new_feat->GetData().GetRna().GetExt().IsName() &&
	    new_feat->GetData().GetRna().IsSetType() && new_feat->GetData().GetRna().GetType() == CRNA_ref::eType_mRNA)
        {
            string new_name = new_feat->GetData().GetRna().GetExt().GetName();
            for (auto rule : m_Rules)
            {
                if (MatchConstraint(rule, new_name))
                    rule->ApplyToString(new_name);
            }
            modified |= new_name != new_feat->GetData().GetRna().GetExt().GetName();
            new_feat->SetData().SetRna().SetExt().SetName() = new_name;
        }
	/*
        if (new_feat->IsSetData() && new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt() && new_feat->GetData().GetRna().GetExt().IsGen() && new_feat->GetData().GetRna().GetExt().GetGen().IsSetProduct())
        {
            string new_name = new_feat->GetData().GetRna().GetExt().GetGen().GetProduct();
            for (auto rule : m_Rules)
            {
                if (MatchConstraint(rule, new_name))
                    rule->ApplyToString(new_name);
            }
            modified |= new_name != new_feat->GetData().GetRna().GetExt().GetGen().GetProduct();
            new_feat->SetData().SetRna().SetExt().SetGen().SetProduct() = new_name;
        }
	*/
        if (modified)
        {
            composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            any_modified = true;
        }
    }

    if (any_modified)
        m_CmdProcessor->Execute(composite);
}

void CSuspectProductRulesEditor::OnListMatches( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager();

    if (!m_TopSeqEntry)
        return;
    CScope &scope = m_TopSeqEntry.GetScope();
    wxBusyCursor wait;
    map<string, vector<string> > affected;
    for (CFeat_CI feat_ci(m_TopSeqEntry); feat_ci; ++feat_ci)
    {
        const CSeq_feat& feat = feat_ci->GetOriginalFeature();
        if (feat.IsSetData() && feat.GetData().IsProt() && feat.GetData().GetProt().IsSetName())
        {
            FOR_EACH_NAME_ON_PROTREF(name, feat.GetData().GetProt())
            {
                for (auto rule : m_Rules)
                {
                    if (rule->StringMatchesSuspectProductRule(*name) && MatchConstraint(rule, *name))
                    {
                        affected[rule->SummarizeRule()].push_back(edit::GetTextObjectDescription(feat, scope));
                    }
                }
            }
        }       
	
        if (feat.IsSetData() && feat.GetData().IsRna() && feat.GetData().GetRna().IsSetExt() && feat.GetData().GetRna().GetExt().IsName() &&
	    feat.GetData().GetRna().IsSetType() && feat.GetData().GetRna().GetType() == CRNA_ref::eType_mRNA)
        {
            const string &name = feat.GetData().GetRna().GetExt().GetName();
            for (auto rule : m_Rules)
            {
                if (rule->StringMatchesSuspectProductRule(name) && MatchConstraint(rule, name))
                {
                    affected[rule->SummarizeRule()].push_back(edit::GetTextObjectDescription(feat, scope));
                }
            }
        }
	/*
        if (feat.IsSetData() && feat.GetData().IsRna() && feat.GetData().GetRna().IsSetExt() && feat.GetData().GetRna().GetExt().IsGen() && feat.GetData().GetRna().GetExt().GetGen().IsSetProduct())
        {
            const string &name = feat.GetData().GetRna().GetExt().GetGen().GetProduct();
            for (auto rule : m_Rules)
            {
                if (rule->StringMatchesSuspectProductRule(name) && MatchConstraint(rule, name))
                {
                    affected[rule->SummarizeRule()].push_back(NDiscrepancy::CReportObj::GetTextObjectDescription(feat, scope));
                }   
            }             
        }
	*/
    }

    wxString msg;
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    for (auto i : order)
    {
        if ( i < 0 )
            i = -i - 1;
        const auto rule = m_Rules[i];
        const auto it = affected.find(rule->SummarizeRule());
        if (it != affected.end() && !it->second.empty())
        {
            msg << it->first << ":" << it->second.size() << "\n";
            for (const auto &s : it->second)
            {
                msg << "\t" << s << "\n";
            }
        }
    }
    
    CGenericReportDlg* report = new CGenericReportDlg(this);
    report->SetTitle(wxT("Suspect Rule Matches"));
    report->SetText(msg);
    report->Show(true);
}

void CSuspectProductRulesEditor::x_SetUpTSEandUnDoManager()
{
    m_TopSeqEntry.Reset();
    m_CmdProcessor = NULL;
    if (!m_Workbench)
        return;

    CSelectionService* service = m_Workbench->GetServiceByType<CSelectionService>();
    if (!service)
        return;

    TConstScopedObjects objects;
    service->GetActiveObjects(objects); 
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }

    if (objects.empty()) 
        return;

    NON_CONST_ITERATE (TConstScopedObjects, it, objects) 
    {
        if (!m_TopSeqEntry) 
        {
            CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);
            if (seh) 
            {
                m_TopSeqEntry = seh;
                break;
            }
        }
    }

    if (!m_TopSeqEntry) 
        return;
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) 
        return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
    if (doc)
        m_CmdProcessor = &doc->GetUndoManager(); 
}

void CSuspectProductRulesEditor::OnDeleteRules( wxCommandEvent& event )
{
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    set<int> to_delete;
    vector<size_t> rule_to_order(order.size());
    for (size_t i = 0; i < order.size(); i++)
    {
        int j = order[i];
        if ( j >= 0 )
        {
            to_delete.insert(j);
        }
        else
        {
            j = -j - 1;
        }
        rule_to_order[j] = i;
    }
    if (to_delete.empty())
        return;

    vector< CRef<CSuspect_rule> > rules;
    for (size_t i = 0; i < m_Rules.size(); i++)
    {
        if (to_delete.find(i) == to_delete.end())
        {
            size_t j = rule_to_order[i];
            int k = rules.size();
            order[j] = ~k;
            rules.push_back(m_Rules[i]);
        }
    }
    swap(rules, m_Rules);

    wxArrayInt new_order;
    for (size_t i = 0; i < order.size(); i++)
    {
        if (order[i] < 0)
            new_order.Add(order[i]);
    }
    swap(order, new_order);

    wxArrayString items;
    for (const auto &rule : m_Rules)
    {
        items.Add(wxString(rule->SummarizeRule()));
    }
    list->Set(order, items);
    m_DeleteButton->Disable();
}

void CSuspectProductRulesEditor::UpdateCheckedState(size_t checked)
{
    if (checked > 0)
        m_DeleteButton->Enable();
    else
        m_DeleteButton->Disable();
}

void CSuspectProductRulesEditor::OnAddRuleBefore( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager();
    CAddSuspectProductRule * dlg = new CAddSuspectProductRule(this, true, m_TopSeqEntry);
    dlg->Show(true);
}

void CSuspectProductRulesEditor::OnAddRuleAfter( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager();
    CAddSuspectProductRule * dlg = new CAddSuspectProductRule(this, false, m_TopSeqEntry);
    dlg->Show(true);
}

void CSuspectProductRulesEditor::AddRule(CRef<CSuspect_rule> rule, bool before)
{
    m_Rules.push_back(rule);
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    int sel = list->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        sel = 0;
        if (!before && !order.empty())
            sel = order.size() - 1;
    }
    wxArrayInt new_order;
    for (size_t i = 0; i < sel; i++)
        new_order.Add(order[i]);
    if (before)
    {
        new_order.Add(~(m_Rules.size() - 1));
        if (!order.empty())
            new_order.Add(order[sel]);
    }
    else
    {
        if (!order.empty())
            new_order.Add(order[sel]);
        new_order.Add(~(m_Rules.size() - 1));
    }
    for (size_t i = sel + 1; i < order.size(); i++)
        new_order.Add(order[i]);

    wxArrayString items;
    for (const auto &rule : m_Rules)
    {
        items.Add(wxString(rule->SummarizeRule()));
    }
    list->Set(new_order, items);
    list->EnsureVisible(sel);
}

void CSuspectProductRulesEditor::ReplaceRule(CRef<CSuspect_rule> rule, int n)
{
    vector< CRef<CSuspect_rule> > new_rules;
    for (size_t i = 0; i < n; i++)
        new_rules.push_back(m_Rules[i]);
    new_rules.push_back(rule);
    for (size_t i = n + 1; i < m_Rules.size(); i++)
        new_rules.push_back(m_Rules[i]);
    swap(m_Rules, new_rules);

    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();

    wxArrayString items;
    for (const auto &rule : m_Rules)
    {
        items.Add(wxString(rule->SummarizeRule()));
    }
    list->Set(order, items);
    list-> EnsureVisible(n);
}

void CSuspectProductRulesEditor::DoubleClick(int n)
{
    x_SetUpTSEandUnDoManager();
    CRearrangeList *list = m_Panel->GetList();
    wxArrayInt order = list->GetCurrentOrder();
    int i = order[n];
    if ( i < 0 )
        i = -i - 1;
    CRef<CSuspect_rule> rule = m_Rules[i];
    CAddSuspectProductRule * dlg = new CAddSuspectProductRule(this, false, m_TopSeqEntry);
    dlg->SetRule(rule, i);
    dlg->Show(true);
}


IMPLEMENT_DYNAMIC_CLASS( CAddSuspectProductRule, wxFrame )

BEGIN_EVENT_TABLE( CAddSuspectProductRule, wxFrame )

////@begin CAddSuspectProductRule event table entries
    EVT_BUTTON( wxID_OK, CAddSuspectProductRule::OnAccept )
    EVT_BUTTON( wxID_CANCEL, CAddSuspectProductRule::OnCancel )
////@end CAddSuspectProductRule event table entries

END_EVENT_TABLE()

CAddSuspectProductRule::CAddSuspectProductRule()
{
    Init();
}

CAddSuspectProductRule::CAddSuspectProductRule( wxWindow* parent, bool before, objects::CSeq_entry_Handle seh, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_before(before), m_replace(false), m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CAddSuspectProductRule::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddSuspectProductRule creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CAddSuspectProductRule creation
    return true;
}

CAddSuspectProductRule::~CAddSuspectProductRule()
{
////@begin CAddSuspectProductRule destruction
////@end CAddSuspectProductRule destruction
}

void CAddSuspectProductRule::Init()
{
////@begin CAddSuspectProductRule member initialisation
////@end CAddSuspectProductRule member initialisation
}

void CAddSuspectProductRule::CreateControls()
{    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemDialog1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemDialog1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    CEnumeratedTypeValues::TValues type_values = objects::ENUM_METHOD_NAME(EFix_type)()->GetValues();
    wxArrayString type_str;
    for (CEnumeratedTypeValues::TValues::const_iterator i = type_values.begin(); i != type_values.end(); ++i)
    {
        type_str.Add(wxString(i->first));
    }
    m_Type = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, type_str, 0,wxDefaultValidator);
    itemBoxSizer2->Add(m_Type, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    m_Type->SetSelection(0);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Where Product Matches"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_Matches = new wxChoicebook(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCHB_TOP);
    itemStaticBoxSizer4->Add(m_Matches, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    
    CMacroStringConstraintPanel* panel1_1 = new CMacroStringConstraintPanel(m_Matches);
    m_Matches->AddPage(panel1_1,_("String constraint"));

    CRef<CSearch_func> search_func;

    search_func.Reset(new CSearch_func);
    search_func->SetContains_plural();
    CMacroBlankConstraintPanel* panel1_2 = new CMacroBlankConstraintPanel(m_Matches, search_func);
    m_Matches->AddPage(panel1_2,_("Contains Plural"));

    search_func.Reset(new CSearch_func);
    search_func->SetN_or_more_brackets_or_parentheses();
    CMacroIntConstraintPanel* panel1_3 = new CMacroIntConstraintPanel(m_Matches, search_func, &search_func->SetN_or_more_brackets_or_parentheses());
    m_Matches->AddPage(panel1_3,_("N or more brackets or parenthesis"));

    search_func.Reset(new CSearch_func);
    search_func->SetThree_numbers();
    CMacroBlankConstraintPanel* panel1_4 = new CMacroBlankConstraintPanel(m_Matches, search_func);
    m_Matches->AddPage(panel1_4,_("Three numbers"));

    search_func.Reset(new CSearch_func);
    search_func->SetUnderscore();
    CMacroBlankConstraintPanel* panel1_5 = new CMacroBlankConstraintPanel(m_Matches, search_func);
    m_Matches->AddPage(panel1_5,_("Contains underscore"));

    search_func.Reset(new CSearch_func);
    search_func->SetPrefix_and_numbers();
    CMacroStrConstraintPanel* panel1_6 = new CMacroStrConstraintPanel(m_Matches, search_func, &search_func->SetPrefix_and_numbers());
    m_Matches->AddPage(panel1_6,_("Is prefix and numbers"));

    search_func.Reset(new CSearch_func);
    search_func->SetAll_caps();
    CMacroBlankConstraintPanel* panel1_7 = new CMacroBlankConstraintPanel(m_Matches, search_func);
    m_Matches->AddPage(panel1_7,_("Is all caps"));

    search_func.Reset(new CSearch_func);
    search_func->SetUnbalanced_paren();
    CMacroBlankConstraintPanel* panel1_8 = new CMacroBlankConstraintPanel(m_Matches, search_func);
    m_Matches->AddPage(panel1_8,_("Contains unbalanced parenthesis"));

    search_func.Reset(new CSearch_func);
    search_func->SetToo_long();
    CMacroIntConstraintPanel* panel1_9 = new CMacroIntConstraintPanel(m_Matches, search_func, &search_func->SetToo_long());
    m_Matches->AddPage(panel1_9,_("Is too long"));

    search_func.Reset(new CSearch_func);
    search_func->SetHas_term();
    CMacroStrConstraintPanel* panel1_10 = new CMacroStrConstraintPanel(m_Matches, search_func, &search_func->SetHas_term());
    m_Matches->AddPage(panel1_10,_("Contains special term"));

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemDialog1, wxID_ANY, _("But Product does not Match"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_DoesNotMatch = new wxChoicebook(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCHB_TOP);
    itemStaticBoxSizer5->Add(m_DoesNotMatch, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    
    CMacroStringConstraintPanel* panel2_1 = new CMacroStringConstraintPanel(m_DoesNotMatch);
    m_DoesNotMatch->AddPage(panel2_1,_("String constraint"));

    search_func.Reset(new CSearch_func);
    search_func->SetContains_plural();
    CMacroBlankConstraintPanel* panel2_2 = new CMacroBlankConstraintPanel(m_DoesNotMatch, search_func);
    m_DoesNotMatch->AddPage(panel2_2,_("Contains Plural"));
    
    search_func.Reset(new CSearch_func);
    search_func->SetN_or_more_brackets_or_parentheses();
    CMacroIntConstraintPanel* panel2_3 = new CMacroIntConstraintPanel(m_DoesNotMatch, search_func, &search_func->SetN_or_more_brackets_or_parentheses());
    m_DoesNotMatch->AddPage(panel2_3,_("N or more brackets or parenthesis"));

    search_func.Reset(new CSearch_func);
    search_func->SetThree_numbers();
    CMacroBlankConstraintPanel* panel2_4 = new CMacroBlankConstraintPanel(m_DoesNotMatch, search_func);
    m_DoesNotMatch->AddPage(panel2_4,_("Three numbers"));

    search_func.Reset(new CSearch_func);
    search_func->SetUnderscore();
    CMacroBlankConstraintPanel* panel2_5 = new CMacroBlankConstraintPanel(m_DoesNotMatch, search_func);
    m_DoesNotMatch->AddPage(panel2_5,_("Contains underscore"));

    search_func.Reset(new CSearch_func);
    search_func->SetPrefix_and_numbers();
    CMacroStrConstraintPanel* panel2_6 = new CMacroStrConstraintPanel(m_DoesNotMatch, search_func, &search_func->SetPrefix_and_numbers());
    m_DoesNotMatch->AddPage(panel2_6,_("Is prefix and numbers"));

    search_func.Reset(new CSearch_func);
    search_func->SetAll_caps();
    CMacroBlankConstraintPanel* panel2_7 = new CMacroBlankConstraintPanel(m_DoesNotMatch, search_func);
    m_DoesNotMatch->AddPage(panel2_7,_("Is all caps"));

    search_func.Reset(new CSearch_func);
    search_func->SetUnbalanced_paren();
    CMacroBlankConstraintPanel* panel2_8 = new CMacroBlankConstraintPanel(m_DoesNotMatch, search_func);
    m_DoesNotMatch->AddPage(panel2_8,_("Contains unbalanced parenthesis"));

    search_func.Reset(new CSearch_func);
    search_func->SetToo_long();
    CMacroIntConstraintPanel* panel2_9 = new CMacroIntConstraintPanel(m_DoesNotMatch, search_func, &search_func->SetToo_long());
    m_DoesNotMatch->AddPage(panel2_9,_("Is too long"));

    search_func.Reset(new CSearch_func);
    search_func->SetHas_term();
    CMacroStrConstraintPanel* panel2_10 = new CMacroStrConstraintPanel(m_DoesNotMatch, search_func, &search_func->SetHas_term());
    m_DoesNotMatch->AddPage(panel2_10,_("Contains special term"));

    wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Replacement Action"));
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_Action = new wxChoicebook(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCHB_TOP);
    itemStaticBoxSizer6->Add(m_Action, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    
    CReplacementActionNone* panel3_1 = new CReplacementActionNone(m_Action);
    m_Action->AddPage(panel3_1,_("None"));

    CReplacementActionSimple* panel3_2 = new CReplacementActionSimple(m_Action);
    m_Action->AddPage(panel3_2,_("Simple"));

    CReplacementActionHaem* panel3_3 = new CReplacementActionHaem(m_Action);
    m_Action->AddPage(panel3_3,_("Haem"));

    CReplacementActionHypothetical* panel3_4 = new CReplacementActionHypothetical(m_Action);
    m_Action->AddPage(panel3_4,_("Hypothetical"));

    m_OriginalToNote = new wxCheckBox( itemDialog1, wxID_ANY, _("Move original to note"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OriginalToNote->SetValue(false);
    itemStaticBoxSizer6->Add(m_OriginalToNote, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_Fatal = new wxCheckBox( itemDialog1, wxID_ANY, _("Fatal"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Fatal->SetValue(false);
    itemBoxSizer2->Add(m_Fatal, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticText* itemStaticText0 = new wxStaticText( itemDialog1, wxID_STATIC, _("Constraint List"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText0, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);  

    m_CompoundConstraintPanel = new CSuspectCompoundConstraintPanel( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_CompoundConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);  

    wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);  

    m_Description = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_Description, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);   

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

}

bool CAddSuspectProductRule::ShowToolTips()
{
    return true;
}
wxBitmap CAddSuspectProductRule::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddSuspectProductRule bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddSuspectProductRule bitmap retrieval
}
wxIcon CAddSuspectProductRule::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddSuspectProductRule icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddSuspectProductRule icon retrieval
}

void CAddSuspectProductRule::OnCancel( wxCommandEvent& event )
{
    Close();
}

void CAddSuspectProductRule::OnAccept( wxCommandEvent& event )
{    
    if (!m_rule)
        m_rule.Reset(new CSuspect_rule);
    m_rule->SetRule_type(static_cast<EFix_type>(m_Type->GetSelection()));
    wxWindow* win = m_Matches->GetCurrentPage();
    CSearch_func_provider *match = dynamic_cast<CSearch_func_provider*>(win);
    CRef<CSearch_func> find = match->GetSearchFunc();
    if (!find)
        return;
    m_rule->SetFind(*find);
    win = m_DoesNotMatch->GetCurrentPage();
    CSearch_func_provider *doesnotmatch = dynamic_cast<CSearch_func_provider*>(win);
    CRef<CSearch_func> except = doesnotmatch->GetSearchFunc();
    if (except)
        m_rule->SetExcept(*except);
    else
        m_rule->ResetExcept();
    win = m_Action->GetCurrentPage();
    CReplace_rule_provider *replace_provider = dynamic_cast<CReplace_rule_provider*>(win);
    CRef<CReplace_rule> replace = replace_provider->GetReplace();
    if (replace)
    {
        m_rule->SetReplace(*replace);
        m_rule->SetReplace().SetMove_to_note(m_OriginalToNote->GetValue());
    }
    else
        m_rule->ResetReplace();
    m_rule->SetFatal(m_Fatal->GetValue());

    m_rule->ResetFeat_constraint();
    if (m_CompoundConstraintPanel->IsSetConstraints())
    {
        CRef<objects::CConstraint_choice_set> constraints =  m_CompoundConstraintPanel->GetConstraints();
        for (auto c : constraints->Set())
        {
            m_rule->SetFeat_constraint().Set().push_back(c);
        }
    }

    if (!m_Description->GetValue().IsEmpty())
        m_rule->SetDescription(NStr::TruncateSpaces(m_Description->GetValue().ToStdString()));
    else
        m_rule->ResetDescription();

    wxWindow* parent = GetParent();
    CSuspectProductRulesEditor* editor = dynamic_cast<CSuspectProductRulesEditor*>(parent);
    if (m_replace)
        editor->ReplaceRule(m_rule, m_pos);
    else
        editor->AddRule(m_rule, m_before);
    Close();
}

void CAddSuspectProductRule::SetRule(CRef<CSuspect_rule> rule, int pos)
{
    if (!rule)
        return;
    m_replace = true;
    m_before = false;
    m_rule = rule;
    m_pos = pos;

    if (rule->IsSetRule_type())
        m_Type->SetSelection(rule->GetRule_type());
    if (rule->IsSetFind())
    {
        m_Matches->ChangeSelection(rule->GetFind().Which() - 1);
        wxWindow* win = m_Matches->GetCurrentPage();
        CSearch_func_provider *match = dynamic_cast<CSearch_func_provider*>(win);
        match->SetSearchFunc(rule->GetFind());
    }
    if (rule->IsSetExcept())
    {
        m_DoesNotMatch->ChangeSelection(rule->GetExcept().Which() - 1);
        wxWindow* win = m_DoesNotMatch->GetCurrentPage();
        CSearch_func_provider *except = dynamic_cast<CSearch_func_provider*>(win);
        except->SetSearchFunc(rule->GetExcept());
    }
    m_Action->ChangeSelection(0);
    if (rule->IsSetReplace())
    {
        if (rule->GetReplace().IsSetReplace_func())
        {
            if (rule->GetReplace().GetReplace_func().IsHaem_replace())
            {
                m_Action->ChangeSelection(2);
            }
            if (rule->GetReplace().GetReplace_func().IsSimple_replace())
            {
                if (rule->GetReplace().GetReplace_func().GetSimple_replace().IsSetReplace() && rule->GetReplace().GetReplace_func().GetSimple_replace().GetReplace() == "hypothetical protein" &&
                    rule->GetReplace().GetReplace_func().GetSimple_replace().IsSetWhole_string() && rule->GetReplace().GetReplace_func().GetSimple_replace().GetWhole_string() &&
                    rule->GetReplace().GetReplace_func().GetSimple_replace().IsSetWeasel_to_putative() && !rule->GetReplace().GetReplace_func().GetSimple_replace().GetWeasel_to_putative())
                    m_Action->ChangeSelection(3);
                else
                    m_Action->ChangeSelection(1);
            }
            wxWindow *win = m_Action->GetCurrentPage();
            CReplace_rule_provider *replace_provider = dynamic_cast<CReplace_rule_provider*>(win);
            replace_provider->SetReplace(rule->GetReplace());
        }

        m_OriginalToNote->SetValue(rule->GetReplace().IsSetMove_to_note() && rule->GetReplace().GetMove_to_note());
    }
    m_Fatal->SetValue(rule->IsSetFatal() && rule->GetFatal());

    if (m_rule->IsSetFeat_constraint())
        m_CompoundConstraintPanel->SetConstraints(m_rule->GetFeat_constraint());

    if (rule->IsSetDescription())
        m_Description->SetValue(wxString(rule->GetDescription()));
}

IMPLEMENT_DYNAMIC_CLASS( CMacroStringConstraintPanel, wxPanel )

BEGIN_EVENT_TABLE( CMacroStringConstraintPanel, wxPanel )
EVT_BUTTON(SYMBOL_CMACROSTRINGCONSTRAINTPANEL_CLEAR_ID, CMacroStringConstraintPanel::OnClear)
EVT_BUTTON(SYMBOL_CMACROSTRINGCONSTRAINTPANEL_WORD_SUBST_ID, CMacroStringConstraintPanel::OnWordSubstitution)
END_EVENT_TABLE()

/*!
 * CMacroStringConstraintPanel constructors
 */
CMacroStringConstraintPanel::CMacroStringConstraintPanel()
{
    Init();
}

CMacroStringConstraintPanel::CMacroStringConstraintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CMacroStringConstraintPanel creator
 */
bool CMacroStringConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroStringConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMacroStringConstraintPanel creation
    ShowChoiceOrText();
    
    return true;
}


/*!
 * CMacroStringConstraintPanel destructor
 */
CMacroStringConstraintPanel::~CMacroStringConstraintPanel()
{
////@begin CMacroStringConstraintPanel destruction
////@end CMacroStringConstraintPanel destruction
}


/*!
 * Member initialisation
 */
void CMacroStringConstraintPanel::Init()
{
////@begin CMacroStringConstraintPanel member initialisation
    m_MatchType = NULL;
    m_MatchText = NULL;
    m_IgnoreCase = NULL;
    m_IgnoreSpace = NULL;
    m_WholeWord = NULL;
    m_IgnorePunct = NULL;
    m_IgnoreSyn = NULL;
    m_AnyLetters = NULL;
    m_AllUpper = NULL;
    m_AllLower = NULL;
    m_AllPunct = NULL;
    m_MatchChoice = NULL;
    m_TextSizer = NULL;
    m_ClearButton = NULL;
    m_WordSubst = NULL;
    m_is_choice = false;
////@end CMacroStringConstraintPanel member initialisation
}


/*!
 * Control creation for CMacroStringConstraintPanel
 */
void CMacroStringConstraintPanel::CreateControls()
{    
////@begin CMacroStringConstraintPanel content construction
    //CMacroStringConstraintPanel* itemPanel1 = this;
    wxPanel* parentPanel = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_TOP|wxALL, 2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxArrayString m_MatchTypeStrings;
    m_MatchTypeStrings.Add(_("Contains"));
    m_MatchTypeStrings.Add(_("Does not contain"));
    m_MatchTypeStrings.Add(_("Equals"));
    m_MatchTypeStrings.Add(_("Does not equal"));
    m_MatchTypeStrings.Add(_("Starts with"));
    m_MatchTypeStrings.Add(_("Ends with"));
    m_MatchTypeStrings.Add(_("Is one of"));
    m_MatchTypeStrings.Add(_("Is not one of"));
    m_MatchTypeStrings.Add(_("Does not start with"));
    m_MatchTypeStrings.Add(_("Does not end with"));
    
    m_MatchType = new wxChoice( parentPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MatchTypeStrings, 0 );
    m_MatchType->SetStringSelection(_("Contains"));
    itemBoxSizer3->Add(m_MatchType, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 2);
    m_MatchType->Bind(wxEVT_CHOICE, &CMacroStringConstraintPanel::OnMatchTypeSelected, this);
    
    m_TextSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(m_TextSizer, 0, wxALIGN_TOP, 0);

    m_MatchText = new wxTextCtrl( parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE );
    m_TextSizer->Add(m_MatchText, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 2);   
    wxArrayString choice_strings;
    m_MatchChoice = new wxComboBox(parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), choice_strings, 0 );
    m_MatchChoice->Hide();


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_IgnoreCase = new wxCheckBox( parentPanel, wxID_ANY, _("Ignore Case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreCase->SetValue(true);
    itemBoxSizer6->Add(m_IgnoreCase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_IgnoreSpace = new wxCheckBox( parentPanel, wxID_ANY, _("Ignore Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSpace->SetValue(false);
    itemBoxSizer6->Add(m_IgnoreSpace, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_IgnorePunct = new wxCheckBox( parentPanel, wxID_ANY, _("Ignore Punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnorePunct->SetValue(false);
    itemBoxSizer6->Add(m_IgnorePunct, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);    

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_WholeWord = new wxCheckBox( parentPanel, wxID_ANY, _("Whole Word"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WholeWord->SetValue(false);
    itemBoxSizer7->Add(m_WholeWord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);   

    m_IgnoreSyn = new wxCheckBox( parentPanel, wxID_ANY, _("Ignore 'putative' synonyms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSyn->SetValue(false);
    itemBoxSizer7->Add(m_IgnoreSyn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_AnyLetters = new wxRadioButton( parentPanel, wxID_ANY, _("Any letters"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AnyLetters->SetValue(true);
    itemBoxSizer8->Add(m_AnyLetters, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_AllUpper = new wxRadioButton( parentPanel, wxID_ANY, _("All letters are upper case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllUpper->SetValue(false);
    itemBoxSizer8->Add(m_AllUpper, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_AllLower = new wxRadioButton( parentPanel, wxID_ANY, _("All letters are lower case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllLower->SetValue(false);
    itemBoxSizer8->Add(m_AllLower, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_AllPunct = new wxRadioButton( parentPanel, wxID_ANY, _("All characters are punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllPunct->SetValue(false);
    itemBoxSizer8->Add(m_AllPunct, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_ClearButton = new wxButton( parentPanel, SYMBOL_CMACROSTRINGCONSTRAINTPANEL_CLEAR_ID, _("Clear Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_ClearButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_WordSubst = new wxButton( parentPanel, SYMBOL_CMACROSTRINGCONSTRAINTPANEL_WORD_SUBST_ID, _("Word Substitutions"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_WordSubst, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    ////@end CMacroStringConstraintPanel content construction  
}

void CMacroStringConstraintPanel::OnClear( wxCommandEvent& event )
{
    ClearValues();  
}

void CMacroStringConstraintPanel::ClearValues()
{
    if (m_is_choice)
    {
        m_MatchChoice->Hide();
        m_TextSizer->Replace(m_MatchChoice, m_MatchText);
        m_MatchText->Show();
        m_is_choice = false;
        Layout();
        Fit();
    }
    m_MatchType->SetStringSelection(_("Contains"));
    m_MatchText->SetValue(wxEmptyString);
    m_IgnoreCase->SetValue(true);
    m_IgnoreSpace->SetValue(false);
    m_WholeWord->SetValue(false);
    m_IgnorePunct->SetValue(false);
    m_IgnoreSyn->SetValue(false);
    m_AnyLetters->SetValue(true);
    m_AllUpper->SetValue(false);
    m_AllLower->SetValue(false);
    m_AllPunct->SetValue(false);
    m_word_subst.Reset();
}

void CMacroStringConstraintPanel::SetChoices(const vector<string> &choices)
{
    wxArrayString choice_strings;
    ITERATE(vector<string>, it, choices) 
    {
        choice_strings.Add(ToWxString(*it));
    }
    m_MatchChoice->Set(choice_strings);
    ShowChoiceOrText();
}

void CMacroStringConstraintPanel::ShowChoiceOrText()
{
   if (!m_MatchChoice->IsListEmpty() && (m_MatchType->GetSelection() == eMatchType_Equals || m_MatchType->GetSelection() == eMatchType_DoesNotEqual) )
    {
        if (!m_is_choice)
        {
            m_MatchText->Hide();
            m_TextSizer->Replace(m_MatchText, m_MatchChoice);
            m_MatchChoice->Show();
        }
        m_is_choice = true;
    }
    else
    {
        if (m_is_choice)
        {
            m_MatchChoice->Hide();
            m_TextSizer->Replace(m_MatchChoice, m_MatchText);
            m_MatchText->Show();
        }
        m_is_choice = false;
    }
    Layout();
    Fit();

}

void CMacroStringConstraintPanel::OnMatchTypeSelected( wxCommandEvent& event )
{
    ShowChoiceOrText();
    event.Skip();
}

/*!
 * Should we show tooltips?
 */
bool CMacroStringConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */
wxBitmap CMacroStringConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroStringConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroStringConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */
wxIcon CMacroStringConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroStringConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroStringConstraintPanel icon retrieval
}

CRef<CString_constraint> CMacroStringConstraintPanel::GetStringConstraint()
{
    string match_text; 
    if (!m_is_choice)
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchText->GetValue()),"\r"," "),"\n"," ");
    else 
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchChoice->GetValue()),"\r"," "),"\n"," ");

    CRef<CString_constraint> c(new CString_constraint);
    c->SetMatch_text(NStr::TruncateSpaces(match_text));
    c->SetCase_sensitive(!m_IgnoreCase->GetValue());
    c->SetIgnore_space(m_IgnoreSpace->GetValue());
    c->SetIgnore_punct(m_IgnorePunct->GetValue());
    c->SetWhole_word(m_WholeWord->GetValue());
    c->SetIs_all_caps(m_AllUpper->GetValue());
    c->SetIs_all_lower(m_AllLower->GetValue());
    c->SetIs_all_punct(m_AllPunct->GetValue());
    c->SetIgnore_weasel(m_IgnoreSyn->GetValue());

    if (m_word_subst && m_word_subst->IsSet() && !m_word_subst->Get().empty())
    {
        c->SetIgnore_words(*m_word_subst);
    }

    switch(m_MatchType->GetSelection()) {
        case eMatchType_Contains:
            c->SetMatch_location(eString_location_contains);
            break;
        case eMatchType_DoesNotContain:
            c->SetMatch_location(eString_location_contains);
            c->SetNot_present(true);
            break;
        case eMatchType_Equals:
            c->SetMatch_location(eString_location_equals);
            break;
        case eMatchType_DoesNotEqual:
            c->SetMatch_location(eString_location_equals);
            c->SetNot_present(true);
            break;
        case eMatchType_StartsWith:
            c->SetMatch_location(eString_location_starts);
            break;
        case eMatchType_EndsWith:
            c->SetMatch_location(eString_location_ends);
            break;
        case eMatchType_IsOneOf:
            c->SetMatch_location(eString_location_inlist);
            break;
        case eMatchType_IsNotOneOf:
            c->SetMatch_location(eString_location_inlist);
            c->SetNot_present(true);
            break;
        case eMatchType_DoesNotStartWith:
            c->SetMatch_location(eString_location_starts);
            c->SetNot_present(true);
            break;
        case eMatchType_DoesNotEndWith:
            c->SetMatch_location(eString_location_ends);
            c->SetNot_present(true);
            break;
    default: c.Reset(); break;
    }
  
    return c;
}

CRef<CSearch_func> CMacroStringConstraintPanel::GetSearchFunc()
{
    CRef<CSearch_func> search_func(new CSearch_func);
    CRef<CString_constraint> str_constr = GetStringConstraint();
    if (str_constr && !str_constr->Empty())
        search_func->SetString_constraint(*str_constr);
    else
        search_func.Reset();
    return search_func;
}

void CMacroStringConstraintPanel::SetSearchFunc(const CSearch_func &search_func)
{
    if (!search_func.IsString_constraint())
        return;
    m_is_choice = false;
    const CString_constraint &c = search_func.GetString_constraint();
    if (c.IsSetMatch_text())
        m_MatchText->SetValue(wxString(c.GetMatch_text()));

    m_IgnoreCase->SetValue(!(c.IsSetCase_sensitive() && c.GetCase_sensitive()));
    m_IgnoreSpace->SetValue(c.IsSetIgnore_space() && c.GetIgnore_space());
    m_IgnorePunct->SetValue(c.IsSetIgnore_punct() && c.GetIgnore_punct());
    m_WholeWord->SetValue(c.IsSetWhole_word() && c.GetWhole_word());
    m_AllUpper->SetValue(c.IsSetIs_all_caps() && c.GetIs_all_caps());
    m_AllLower->SetValue(c.IsSetIs_all_lower() && c.GetIs_all_lower());
    m_AllPunct->SetValue(c.IsSetIs_all_punct() && c.GetIs_all_punct());
    m_IgnoreSyn->SetValue(c.IsSetIgnore_weasel() && c.GetIgnore_weasel());
    
    if (c.IsSetIgnore_words())
    {
        m_word_subst.Reset(new CWord_substitution_set);
        m_word_subst->Assign(c.GetIgnore_words());
    }

    if (c.IsSetMatch_location())
    {
        if (c.IsSetNot_present() && c.GetNot_present())
        {
            switch(c.GetMatch_location())
            {
            case eString_location_contains: m_MatchType->SetSelection(eMatchType_DoesNotContain); break;
            case eString_location_equals: m_MatchType->SetSelection(eMatchType_DoesNotEqual); break;
            case eString_location_starts: m_MatchType->SetSelection(eMatchType_DoesNotStartWith); break;
            case eString_location_ends: m_MatchType->SetSelection(eMatchType_DoesNotEndWith); break;
            case eString_location_inlist: m_MatchType->SetSelection(eMatchType_IsNotOneOf); break;
            }
        }
        else
        {
            switch(c.GetMatch_location())
            {
            case eString_location_contains: m_MatchType->SetSelection(eMatchType_Contains); break;
            case eString_location_equals: m_MatchType->SetSelection(eMatchType_Equals); break;
            case eString_location_starts: m_MatchType->SetSelection(eMatchType_StartsWith); break;
            case eString_location_ends: m_MatchType->SetSelection(eMatchType_EndsWith); break;
            case eString_location_inlist: m_MatchType->SetSelection(eMatchType_IsOneOf); break;
            }
        }
    }
}

void CMacroStringConstraintPanel::OnWordSubstitution( wxCommandEvent& event )
{
    CWordSubstitutionDlg * dlg = new CWordSubstitutionDlg(this, m_word_subst); 
    dlg->Show(true);
}

void CMacroStringConstraintPanel::AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst)
{
    m_word_subst = word_subst;
}



IMPLEMENT_DYNAMIC_CLASS( CReplacementActionNone, wxPanel )

BEGIN_EVENT_TABLE( CReplacementActionNone, wxPanel )
END_EVENT_TABLE()

CReplacementActionNone::CReplacementActionNone()
{
    Init();
}

CReplacementActionNone::CReplacementActionNone( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CReplacementActionNone::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

CReplacementActionNone::~CReplacementActionNone()
{
}

void CReplacementActionNone::Init()
{
}

void CReplacementActionNone::CreateControls()
{    
    wxPanel* parentPanel = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer1);
}

bool CReplacementActionNone::ShowToolTips()
{
    return true;
}

wxBitmap CReplacementActionNone::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CReplacementActionNone::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CReplace_rule> CReplacementActionNone::GetReplace()
{
    CRef<CReplace_rule> replace;
    return replace;
}

IMPLEMENT_DYNAMIC_CLASS( CReplacementActionSimple, wxPanel )

BEGIN_EVENT_TABLE( CReplacementActionSimple, wxPanel )
END_EVENT_TABLE()

CReplacementActionSimple::CReplacementActionSimple()
{
    Init();
}

CReplacementActionSimple::CReplacementActionSimple( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CReplacementActionSimple::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

CReplacementActionSimple::~CReplacementActionSimple()
{
}

void CReplacementActionSimple::Init()
{
}

void CReplacementActionSimple::CreateControls()
{    
    wxPanel* parentPanel = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer1);

    m_Text = new wxTextCtrl( parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer1->Add(m_Text, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);  
 
    m_Replace = new wxCheckBox( parentPanel, wxID_ANY, _("Replace entire string"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Replace->SetValue(false);
    itemBoxSizer1->Add(m_Replace, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    m_Retain = new wxCheckBox( parentPanel, wxID_ANY, _("Retain and normalize 'putative' synonym"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Retain->SetValue(false);
    itemBoxSizer1->Add(m_Retain, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
}

bool CReplacementActionSimple::ShowToolTips()
{
    return true;
}

wxBitmap CReplacementActionSimple::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CReplacementActionSimple::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CReplace_rule> CReplacementActionSimple::GetReplace()
{
    CRef<CReplace_rule> replace(new CReplace_rule);
    replace->SetReplace_func().SetSimple_replace().SetReplace(NStr::TruncateSpaces(m_Text->GetValue().ToStdString()));
    replace->SetReplace_func().SetSimple_replace().SetWhole_string(m_Replace->GetValue());
    replace->SetReplace_func().SetSimple_replace().SetWeasel_to_putative(m_Retain->GetValue());
    return replace;
}

void CReplacementActionSimple::SetReplace(const CReplace_rule &replace)
{
    if (replace.GetReplace_func().IsSimple_replace())
    {
        if (replace.GetReplace_func().GetSimple_replace().IsSetReplace())
            m_Text->SetValue(wxString(replace.GetReplace_func().GetSimple_replace().GetReplace()));
        m_Replace->SetValue(replace.GetReplace_func().GetSimple_replace().IsSetWhole_string() && replace.GetReplace_func().GetSimple_replace().GetWhole_string());
        m_Retain->SetValue(replace.GetReplace_func().GetSimple_replace().IsSetWeasel_to_putative() && replace.GetReplace_func().GetSimple_replace().GetWeasel_to_putative());
    }
}

IMPLEMENT_DYNAMIC_CLASS( CReplacementActionHaem, wxPanel )

BEGIN_EVENT_TABLE( CReplacementActionHaem, wxPanel )
END_EVENT_TABLE()

CReplacementActionHaem::CReplacementActionHaem()
{
    Init();
}

CReplacementActionHaem::CReplacementActionHaem( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CReplacementActionHaem::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

CReplacementActionHaem::~CReplacementActionHaem()
{
}

void CReplacementActionHaem::Init()
{
}

void CReplacementActionHaem::CreateControls()
{    
    wxPanel* parentPanel = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer1);

    m_Text = new wxTextCtrl( parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    itemBoxSizer1->Add(m_Text, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);     
}

bool CReplacementActionHaem::ShowToolTips()
{
    return true;
}

wxBitmap CReplacementActionHaem::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CReplacementActionHaem::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CReplace_rule> CReplacementActionHaem::GetReplace()
{
    CRef<CReplace_rule> replace(new CReplace_rule);
    replace->SetReplace_func().SetHaem_replace(NStr::TruncateSpaces(m_Text->GetValue().ToStdString()));
    return replace;
}

void CReplacementActionHaem::SetReplace(const CReplace_rule &replace)
{
    if (replace.GetReplace_func().IsHaem_replace())
        m_Text->SetValue(wxString(replace.GetReplace_func().GetHaem_replace()));
}

IMPLEMENT_DYNAMIC_CLASS( CReplacementActionHypothetical, wxPanel )

BEGIN_EVENT_TABLE( CReplacementActionHypothetical, wxPanel )
END_EVENT_TABLE()

CReplacementActionHypothetical::CReplacementActionHypothetical()
{
    Init();
}

CReplacementActionHypothetical::CReplacementActionHypothetical( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CReplacementActionHypothetical::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

CReplacementActionHypothetical::~CReplacementActionHypothetical()
{
}

void CReplacementActionHypothetical::Init()
{
}

void CReplacementActionHypothetical::CreateControls()
{    
    wxPanel* parentPanel = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer1);
}

bool CReplacementActionHypothetical::ShowToolTips()
{
    return true;
}

wxBitmap CReplacementActionHypothetical::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CReplacementActionHypothetical::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CReplace_rule> CReplacementActionHypothetical::GetReplace()
{
    CRef<CReplace_rule> replace(new CReplace_rule);
    replace->SetReplace_func().SetSimple_replace().SetReplace("hypothetical protein");
    replace->SetReplace_func().SetSimple_replace().SetWhole_string(true);
    replace->SetReplace_func().SetSimple_replace().SetWeasel_to_putative(false);
    return replace;
}

END_NCBI_SCOPE
