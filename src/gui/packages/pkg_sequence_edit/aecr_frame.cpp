/*  $Id: aecr_frame.cpp 45101 2020-05-29 20:53:24Z asztalos $
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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/display.h>
#include <wx/listctrl.h>

#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/misc_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/gene_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/protein_field_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/pub_field.hpp>
#include <gui/packages/pkg_sequence_edit/dblink_field.hpp>
#include <gui/packages/pkg_sequence_edit/generalid_panel.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/packages/pkg_sequence_edit/dbxref_name_panel.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/aecr_frame.hpp>


BEGIN_NCBI_SCOPE


template <template <typename T2> class T1>
class CQualPanel: public wxPanel, public IQualPanel
{    
    wxDECLARE_NO_COPY_CLASS(CQualPanel);
public:
    CQualPanel();
    CQualPanel( wxWindow* parent, bool composed = false, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CQualPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void OnPageChanged(wxBookCtrlEvent& event);
    virtual void UnbindNotebookPageChanged();
    void SetShowLegalOnly(bool show);
protected:
    bool m_composed;
};

template<typename T>
class CSingleFieldPanel : public wxPanel, public ISpecificQual
{
    wxDECLARE_NO_COPY_CLASS(CSingleFieldPanel);
public:
    CSingleFieldPanel();
    CSingleFieldPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CSingleFieldPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();   
    void SetShowLegalOnly(bool show) {m_Field->SetShowLegalOnly(show);}
    void SetFieldNames(const vector<string>& field_names) {m_Field->SetFieldNames(field_names);}
    void AddAdditionalItems(bool composed);

    // inherited from ISpecificQual
    virtual bool SetFieldName(const string& field);
    virtual vector<string> GetChoices(bool& allow_other) const {return m_Field->GetChoices(allow_other);}
    virtual string GetField1(bool subfield = true) const {return m_Field->GetFieldName(subfield);}
    virtual CFieldNamePanel* GetFieldNamePanel1(void) {return m_Field;}

private:
    T* m_Field;
};

template<typename T>
class CDualFieldPanel : public wxPanel, public ISpecificQual
{
    wxDECLARE_NO_COPY_CLASS(CDualFieldPanel);
public:
    CDualFieldPanel();
    CDualFieldPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CDualFieldPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips(); 
    void SetFieldNames(const vector<string>& field_names) {m_Field1->SetFieldNames(field_names); m_Field2->SetFieldNames(field_names);}
    void AddAdditionalItems(bool composed) {}

    // inherited from ISpecificQual
    virtual bool SetFieldName(const string& field);
    virtual bool SetFieldName2(const string& field);
    virtual vector<string> GetChoices(bool& allow_other) const {return m_Field1->GetChoices(allow_other);}
    virtual string GetField1(bool subfield = true) const {return m_Field1->GetFieldName(subfield);}
    virtual string GetField2(bool subfield = true) const {return m_Field2->GetFieldName(subfield);}
    virtual CFieldNamePanel* GetFieldNamePanel1(void) {return m_Field1;}
    virtual CFieldNamePanel* GetFieldNamePanel2(void) {return m_Field2;}

private:
    T* m_Field1;
    T* m_Field2;
};

template<>
class CSingleFieldPanel<CCDSGeneProtFieldNamePanel> : public wxPanel, public ISpecificQual
{
    wxDECLARE_NO_COPY_CLASS(CSingleFieldPanel);
public:
    CSingleFieldPanel();
    CSingleFieldPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CSingleFieldPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from ISpecificQual
    virtual bool SetFieldName(const string& field);
    virtual vector<string> GetChoices(bool& allow_other) const {return m_Field->GetChoices(allow_other);}
    virtual string GetField1(bool subfield = true) const {return m_Field->GetFieldName(subfield);}
    virtual CFieldNamePanel* GetFieldNamePanel1(void) {return m_Field;}
    virtual void UpdateMRNAbutton(void);
    virtual bool GetUpdateMRNAProductFlag(void) const;
    virtual void UpdateRetranslateCds(void);
    virtual bool GetRetranslateCds(void) const;

private:
    CCDSGeneProtFieldNamePanel* m_Field;
    wxCheckBox* m_UpdatemRNAProduct;
    wxCheckBox* m_RetranslateCds;
};

template<>
class CDualFieldPanel<CCDSGeneProtFieldNamePanel> : public wxPanel, public ISpecificQual
{
    wxDECLARE_NO_COPY_CLASS(CDualFieldPanel);
public:
    CDualFieldPanel();
    CDualFieldPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CDualFieldPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from ISpecificQual
    virtual bool SetFieldName(const string& field);
    virtual bool SetFieldName2(const string& field);
    virtual vector<string> GetChoices(bool& allow_other) const {return m_Field1->GetChoices(allow_other);}
    virtual string GetField1(bool subfield = true) const {return m_Field1->GetFieldName(subfield);}
    virtual string GetField2(bool subfield = true) const {return m_Field2->GetFieldName(subfield);}
    virtual CFieldNamePanel* GetFieldNamePanel1(void) {return m_Field1;}
    virtual CFieldNamePanel* GetFieldNamePanel2(void) {return m_Field2;}
    virtual void UpdateMRNAbutton(void);
    virtual bool GetUpdateMRNAProductFlag(void) const;
    virtual void UpdateRetranslateCds(void);
    virtual bool GetRetranslateCds(void) const;

private:
    CCDSGeneProtFieldNamePanel* m_Field1;
    CCDSGeneProtFieldNamePanel* m_Field2;
    wxCheckBox* m_UpdatemRNAProduct;
    wxCheckBox* m_RetranslateCds;
};

template<>
class CDualFieldPanel<CRNAFieldNamePanel> : public wxPanel, public ISpecificQual
{
    wxDECLARE_NO_COPY_CLASS(CDualFieldPanel);
public:
    CDualFieldPanel();
    CDualFieldPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    virtual ~CDualFieldPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

    // inherited from ISpecificQual
    virtual bool SetFieldName(const string& field);
    virtual bool SetFieldName2(const string& field);
    virtual vector<string> GetChoices(bool& allow_other) const {return m_Field->GetChoices(allow_other);}
    virtual string GetField1(bool subfield = true) const {return m_Field->GetFieldName(subfield);}
    virtual string GetField2(bool subfield = true) const {return m_Field->GetFieldName2(subfield);}
    virtual CFieldNamePanel* GetFieldNamePanel1(void) {return m_Field;}
    virtual CFieldNamePanel* GetFieldNamePanel2(void) {return m_Field;}

private:
    CDualRNAFieldNamePanel* m_Field;
};


IMPLEMENT_DYNAMIC_CLASS( CAECRFrame, CBulkCmdDlg )
wxDEFINE_EVENT(wxEVT_COMMAND_UPDATE_CONSTRAINT_FIELD_NEW, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_UPDATE_AECR_QUAL_CHOICES, wxCommandEvent);


BEGIN_EVENT_TABLE( CAECRFrame, CBulkCmdDlg )

////@begin CAECRFrame event table entries  
  EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_UPDATE_CONSTRAINT_FIELD_NEW, CAECRFrame::UpdateConstraintFieldType) 
  EVT_CHAR_HOOK(CAECRFrame::OnKeyDown)
////@end CAECRFrame event table entries

END_EVENT_TABLE()

CAECRFrame::CAECRFrame()
{
    Init();
}

CAECRFrame::CAECRFrame( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, wb, id, caption, pos, size, style);
    SetRegistryPath("Dialogs.Edit.AECR");
    LoadSettings();
}

bool CAECRFrame::Create( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAECRFrame creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, wb, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CAECRFrame creation
    return true;
}

CAECRFrame::~CAECRFrame()
{
////@begin CAECRFrame destruction
    SaveSettings();
////@end CAECRFrame destruction
}

static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CAECRFrame::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CAECRFrame::SaveSettings() const
{
    if (m_RegPath.empty() || !RunningInsideNCBI())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);
}


void CAECRFrame::LoadSettings()
{
    if (m_RegPath.empty() || !RunningInsideNCBI())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        SetSize(wxSize(width,height));

    int pos_x = view.GetInt(kFramePosX, -1);
    int pos_y = view.GetInt(kFramePosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       SetPosition(wxPoint(pos_x,pos_y));
   }
}


void CAECRFrame::Init()
{
////@begin CAECRFrame member initialisation
    m_Listbook = NULL;
    m_Constraint = NULL;
    m_ClearWhenChanging = NULL;
    m_QualPage = wxNOT_FOUND;
    m_ActionPanel = NULL;
////@end CAECRFrame member initialisation
}

void CAECRFrame::CreateControls()
{    
    GetTopLevelSeqEntryAndProcessor();
    vector<const CFeatListItem *> featlist = GetSortedFeatList(m_TopSeqEntry, 100); 

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* parentPanel = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(parentPanel, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    parentPanel->SetSizer(itemBoxSizer2);

    m_Listbook = new wxListbook( parentPanel, ID_AECR_FRAME_LISTBOOK, wxDefaultPosition, wxDefaultSize, wxBK_LEFT );

    itemBoxSizer2->Add(m_Listbook, 0, wxALIGN_LEFT|wxALL, 5);

    CQualApplyPanel* page1 = new CQualApplyPanel(m_Listbook);
    m_Listbook->AddPage(page1, wxT("Apply"), false);

    CQualEditPanel* page2 = new CQualEditPanel(m_Listbook);
    m_Listbook->AddPage(page2, wxT("Edit"), false);

    CQualConvertPanel* page3 = new CQualConvertPanel(m_Listbook);
    m_Listbook->AddPage(page3, wxT("Convert"), false);

    CQualCopyPanel* page4 = new CQualCopyPanel(m_Listbook);
    m_Listbook->AddPage(page4, wxT("Copy"), false);

    CQualSwapPanel* page5 = new CQualSwapPanel(m_Listbook);
    m_Listbook->AddPage(page5, wxT("Swap"), false);

    CQualParsePanel* page6 = new CQualParsePanel(m_Listbook);
    m_Listbook->AddPage(page6, wxT("Parse"), false);

    CQualRemovePanel* page7 = new CQualRemovePanel(m_Listbook);
    m_Listbook->AddPage(page7, wxT("Remove"), false);

    CQualRemoveOutsidePanel* page8 = new CQualRemoveOutsidePanel(m_Listbook);
    m_Listbook->AddPage(page8, wxT("Remove Outside"), false);   

    m_Listbook->Bind(wxEVT_LISTBOOK_PAGE_CHANGING, &CAECRFrame::BeforePageChange, this);
    m_Listbook->Bind(wxEVT_LISTBOOK_PAGE_CHANGED, &CAECRFrame::AfterPageChange, this);

    if (RunningInsideNCBI())
    {
        m_Constraint = new CAdvancedConstraintPanel( parentPanel, m_TopSeqEntry, &featlist, wxID_ANY, wxDefaultPosition, wxDefaultSize );
        itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    if (RunningInsideNCBI())
    {
        m_ClearWhenChanging = new wxCheckBox( parentPanel, wxID_ANY, _("Clear when changing actions"), wxDefaultPosition, wxDefaultSize, 0 );
        m_ClearWhenChanging->SetValue(false);
        itemBoxSizer3->Add(m_ClearWhenChanging, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    m_OkCancel = new COkCancelPanel( parentPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_OkCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
   
    UpdateChildrenFeaturePanels(this, &featlist);
    set<string> fields;
    CStructCommentFieldPanel::GetStructCommentFields(m_TopSeqEntry,fields, 100); 
    UpdateStructuredCommentFields(this, fields);
    m_Listbook->GetListView()->SetId(ID_AECR_FRAME_LISTVIEW);
}

bool CAECRFrame::ShowToolTips()
{
    return true;
}
wxBitmap CAECRFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAECRFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAECRFrame bitmap retrieval
}
wxIcon CAECRFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAECRFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAECRFrame icon retrieval
}


void CAECRFrame::UpdateConstraintFieldType( wxCommandEvent& event )
{
    IQualPanel* panel = dynamic_cast<IQualPanel*>(event.GetEventObject() ); 
    if (panel)
    {
        int page =  panel->GetSelection();
        string field = panel->GetField1(false);
        if (m_Constraint)
        {
            m_Constraint->SetSelection(page);
            //m_Constraint->SetFieldName(field); // TODO
        }
    }   
}

void CAECRFrame::UpdateChildrenFeaturePanels( wxWindow* win, vector<const CFeatListItem *> *featlist )
{
    wxWindowList &slist = win->GetChildren();
    for (wxWindowList::iterator iter = slist.begin(); iter != slist.end(); ++iter) 
    {
        wxWindow* child = *iter;
        CFeatureTypePanel* panel = dynamic_cast<CFeatureTypePanel*>(child);
        if (panel) 
        {            
            panel->ListPresentFeaturesFirst(m_TopSeqEntry, featlist);
        }      
        UpdateChildrenFeaturePanels(child, featlist);
    }     
}

void CAECRFrame::UpdateStructuredCommentFields( wxWindow* win, const set<string> &fields )
{
    wxWindowList &slist = win->GetChildren();
    for (wxWindowList::iterator iter = slist.begin(); iter != slist.end(); ++iter) 
    {
        wxWindow* child = *iter;
        CStructFieldValuePanel* fieldvalue_panel = dynamic_cast<CStructFieldValuePanel*>(child);
        CStructCommentFieldPanel* strcomm_fields_panel = dynamic_cast<CStructCommentFieldPanel*>(child);
       
        if (fieldvalue_panel) 
        {
            fieldvalue_panel->ListStructCommentFields(fields);
        } 
        if (strcomm_fields_panel) 
        {
            strcomm_fields_panel->CompleteStructCommentFields(fields);
        }        
       
        UpdateStructuredCommentFields(child, fields);        
    }
}

void CAECRFrame::BeforePageChange(wxBookCtrlEvent& event)
{
    if (m_Listbook)
    {
        wxWindow *win = m_Listbook->GetCurrentPage();
        if (win)
        {
            m_ActionPanel = dynamic_cast<IActionPanel*>(win);
        }
    }
    if (m_ActionPanel)
    {
        m_QualPage = m_ActionPanel->GetSelection();
        m_CurrentField1 = m_ActionPanel->GetField1(false);
        string field2 = m_ActionPanel->GetField2(false);
        if (!field2.empty())
            m_CurrentField2 = field2;
    }
    event.Skip();
}

void CAECRFrame::AfterPageChange(wxBookCtrlEvent& event)
{
    if (m_Listbook)
    {
        wxWindow *win = m_Listbook->GetCurrentPage();
        if (win)
        {      
            m_ActionPanel = dynamic_cast<IActionPanel*>(win);
        }
    }
    if (m_ActionPanel)
    {
        m_ActionPanel->SetSelection(m_QualPage, m_CurrentField1, m_CurrentField2);
        m_ActionPanel->UpdateQualChoices();
        if (m_ClearWhenChanging && m_ClearWhenChanging->GetValue())
        {
            m_ActionPanel->ClearValues();
            if (m_Constraint)
                m_Constraint->ClearValues();
        }
    }
    event.Skip();
}

void CAECRFrame::SetInitParameters(EActionType action_type, EFieldType field_type, const string &field, const wxString &title)
{
    if (m_Listbook)
    {
        m_Listbook->SetSelection(action_type);
        wxWindow *win = m_Listbook->GetCurrentPage();
        if (win)
        {    
            m_ActionPanel = dynamic_cast<IActionPanel*>(win);            
        }   
    }
    if (m_ActionPanel)
        m_ActionPanel->SetSelection(field_type, field);  
	
	if (!RunningInsideNCBI() && m_Listbook)
    {        
        if (!title.IsEmpty())
            SetTitle(title);

        RemoveNotebook();
    }
}

void CAECRFrame::UnbindNotebookPageChanged()
{
    m_Listbook->Unbind(wxEVT_LISTBOOK_PAGE_CHANGING, &CAECRFrame::BeforePageChange, this);
    m_Listbook->Unbind(wxEVT_LISTBOOK_PAGE_CHANGED, &CAECRFrame::AfterPageChange, this);
}

void CAECRFrame::RemoveNotebook()
{
    m_ActionPanel->RemoveNotebook();

    wxWindow *win = m_Listbook->GetCurrentPage();
    wxWindow* parentPanel = m_Listbook->GetParent();
    UnbindNotebookPageChanged();
    m_Listbook->RemovePage(m_Listbook->GetSelection());
    win->Reparent(parentPanel);
    parentPanel->GetSizer()->Prepend(win, 0, wxGROW|wxALL, 5);
    m_Listbook->Destroy();           
    m_Listbook = NULL;

    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
    Refresh();
}

void CAECRFrame::UpdateEditor()
{
    if (m_Listbook)
    {
        wxWindow *win = m_Listbook->GetCurrentPage();
        if (win)
        {      
            m_ActionPanel = dynamic_cast<IActionPanel*>(win);
        }
    }
    if (m_ActionPanel)
    {
        m_ActionPanel->UpdateQualChoices();
        string field = m_ActionPanel->GetField1(false);
        //if (m_Constraint) // TODO causes endless recursion
        //    m_Constraint->SetFieldName(field);
    }       
}

void CAECRFrame::OnKeyDown(wxKeyEvent& event) // https://wiki.wxwidgets.org/Catching_key_events_globally
{
    int uc = event.GetKeyCode();
    if (uc == WXK_RETURN) //  || uc == WXK_NUMPAD_ENTER
    {
        if (GetTopLevelSeqEntryAndProcessor())
        {
            CRef<CCmdComposite> cmd = GetCommand();
            if (cmd) 
            {
                ExecuteCmd(cmd);
                if (!m_OkCancel->GetLeaveUp()) 
                {
                    Destroy();
                }
            } 
            else 
            {
                string error = GetErrorMessage();
                if (!NStr::IsBlank(error)) 
                {
                    wxMessageBox(ToWxString(error), wxT("Error"), wxOK | wxICON_ERROR, NULL);
                }
            }
        } 
        else 
        {
            wxMessageBox(ToWxString("No data was selected"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        }
    }
    event.Skip();
}


CRef<CCmdComposite> CAECRFrame::GetCommand()
{
    CRef<CCmdComposite> cmd;

    if (m_Listbook)
    {
        int sel = m_Listbook->GetSelection();
        if (sel == wxNOT_FOUND)
            return cmd;

        wxWindow *win = m_Listbook->GetCurrentPage();
        m_ActionPanel = dynamic_cast<IActionPanel*>(win);
    }

    if (!m_ActionPanel)
        return cmd;
   
    GetTopLevelSeqEntryAndProcessor();
    CIRef<IEditingAction> action1 = m_ActionPanel->CreateAction1(m_TopSeqEntry, m_SeqSubmit);
    if (!action1)
        return cmd;

    CIRef<IEditingAction> action2 = m_ActionPanel->CreateAction2(m_TopSeqEntry, m_SeqSubmit);
    if (!action2 && m_ActionPanel->IsAction2Required())
        return cmd;

    if (m_Constraint)
    {
        CRef<CEditingActionConstraint> constraint = m_Constraint->GetConstraint(m_ActionPanel->GetField1(), 
                                                                                CFieldNamePanel::GetFieldTypeFromName(m_ActionPanel->GetFieldType()), 
                                                                                GetSubtype(m_ActionPanel->GetFieldNamePanel1()));
    
        action1->SetConstraint(constraint);
    }
    
    int num_conflicts = m_ActionPanel->CheckForExistingText(action1, action2);
    edit::EExistingText existing_text = x_GetExistingTextHandling (num_conflicts, true); 
    if (existing_text == edit::eExistingText_cancel) 
        return cmd;

    m_ActionPanel->SetupAction(action1, action2, existing_text);
    action1->SetUpdateMRNAProduct(m_ActionPanel->GetUpdateMRNAProductFlag());
    action1->SetRetranslateCds(m_ActionPanel->GetRetranslateCds());
    cmd = action1->GetActionCommand();

    return cmd;
}

int IQualPanel::GetSelection(void) const
{
    if (m_Notebook)
        return m_Notebook->GetSelection();
    return 0;
}

void IQualPanel::SetSelection(int page, const string &field1, const string &field2)
{
    if (page != wxNOT_FOUND && m_Notebook)
    {
        m_Notebook->SetSelection(page);
        m_FieldType = m_Notebook->GetPageText(page).ToStdString();
        wxWindow* win = m_Notebook->GetCurrentPage();
        m_SpecificQual = dynamic_cast<ISpecificQual*>(win);              
    }
    if (m_SpecificQual)
    {
        m_SpecificQual->SetFieldName(field1);
        m_SpecificQual->SetFieldName2(field2);
    }    
}
void IQualPanel::RemoveNotebook()
{
    if (m_Notebook)
    {
        wxWindow* parentPanel = m_Notebook->GetParent();
        wxWindow* win = m_Notebook->GetCurrentPage();
        UnbindNotebookPageChanged();
        m_Notebook->RemovePage(m_Notebook->GetSelection());
        win->Reparent(parentPanel);
        parentPanel->GetSizer()->Prepend(win, 0, wxGROW|wxALL, 5);
        win->Show();
        m_Notebook->Destroy();     
        m_Notebook = NULL;
    }
}

vector<string> IQualPanel::GetChoices(bool& allow_other) const
{
    vector<string> choices;
    if (m_Notebook) 
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        m_SpecificQual = dynamic_cast<ISpecificQual*>(win);
    }
    if (m_SpecificQual)
    {
        choices = m_SpecificQual->GetChoices(allow_other);
    }
    
    return choices;
}

string IQualPanel::GetField1(bool subfield) const
{
    string name;
    if (m_Notebook) 
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        m_SpecificQual = dynamic_cast<ISpecificQual*>(win);
    }
    if (m_SpecificQual)
    {
        name = m_SpecificQual->GetField1(subfield);
    }
    
    return name;
}

string IQualPanel::GetField2(bool subfield) const
{
    string name;
    if (m_Notebook) 
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        m_SpecificQual = dynamic_cast<ISpecificQual*>(win);
    }
    if (m_SpecificQual)
    {
        name = m_SpecificQual->GetField2(subfield);
    }
    
    return name;
}

string IQualPanel::GetFieldType(void) const
{    
    if (m_Notebook)
    {
        int sel = m_Notebook->GetSelection();
        if (sel != wxNOT_FOUND)
        {
            m_FieldType = m_Notebook->GetPageText(sel).ToStdString();
        }
    }
    return m_FieldType;
}

CFieldNamePanel* IQualPanel::GetFieldNamePanel1(void)
{
    CFieldNamePanel* name_panel(NULL);
    if (m_Notebook) 
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        m_SpecificQual = dynamic_cast<ISpecificQual*>(win);
    }
    if (m_SpecificQual)
    {
        name_panel = m_SpecificQual->GetFieldNamePanel1();
    }
    
    return name_panel;
}

CFieldNamePanel* IQualPanel::GetFieldNamePanel2(void)
{
    CFieldNamePanel* name_panel(NULL);
    if (m_Notebook) 
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        m_SpecificQual = dynamic_cast<ISpecificQual*>(win);
    }
    if (m_SpecificQual)
    {
        name_panel = m_SpecificQual->GetFieldNamePanel2();
    }
    
    return name_panel;
}

ISpecificQual* IQualPanel::GetSpecificQualPanel()
{
    if (m_Notebook)
    {
        wxWindow *win = m_Notebook->GetCurrentPage();
        m_SpecificQual = dynamic_cast<ISpecificQual*>(win);
    }
    return m_SpecificQual;
}

void IActionPanel::UpdateQualChoices()
{
    ISpecificQual* qual_panel = m_Panel->GetSpecificQualPanel();
    if (qual_panel)
    {
        qual_panel->UpdateMRNAbutton();
        qual_panel->UpdateRetranslateCds();
    }
}

CAECRFrame *IActionPanel::GetBaseFrame(wxWindow *win)
{
    while (win)
    {
        CAECRFrame *base = dynamic_cast<CAECRFrame*>(win);
        if (base)
            return base;
        win = win->GetParent();
    }   
    return NULL;
}

string IActionPanel::GetField1(bool subfield) const
{
    return m_Panel->GetField1(subfield);
}

string IActionPanel::GetField2(bool subfield) const
{
    return m_Panel->GetField2(subfield);
}

string IActionPanel::GetFieldType() const
{
    return m_Panel->GetFieldType();
}

CFieldNamePanel* IActionPanel::GetFieldNamePanel1()
{
    return m_Panel->GetFieldNamePanel1();
}

CFieldNamePanel* IActionPanel::GetFieldNamePanel2()
{
    return m_Panel->GetFieldNamePanel2();
}

string IActionPanel::GetAutopopulateValue(wxWindow *parent_win)
{
    string val;
    string field1 = m_Panel->GetField1();
    if (!NStr::IsBlank(field1)) 
    {
        CFieldNamePanel::EFieldType field_type = CFieldNamePanel::GetFieldTypeFromName(m_Panel->GetFieldType());
        int subtype1 = GetSubtype(m_Panel->GetFieldNamePanel1());    
        CAECRFrame *parent = GetBaseFrame(parent_win);
        if (parent)
        {
            CSeq_entry_Handle seh = parent->GetTopSeqEntryHandle();
	    CConstRef<objects::CSeq_submit> submit = parent->GetSeqSubmit();
            if (seh)
            {
                CIRef<IEditingAction> action1 = CreateAction(seh, field1, field_type, subtype1, submit);
                if (action1)
                {
                    CAdvancedConstraintPanel* constraint_panel = parent->GetConstraintPanel();
                    if (constraint_panel)
                    {
                        CRef<CEditingActionConstraint> constraint = constraint_panel->GetConstraint(field1, field_type, subtype1); 
                        action1->SetConstraint(constraint);
                    }
                    action1->ResetChangedValues();
                    action1->SetMaxRecords(100); 
                    action1->NOOP();
                    if (!action1->GetChangedValues().empty())
                    {
                        val = action1->GetChangedValues().front();
                    }
                }
            }
        }       
    }
    return val;
}

bool IActionPanel::GetUpdateMRNAProductFlag() const
{
    bool value(false);
    ISpecificQual* qual_panel = m_Panel->GetSpecificQualPanel();
    if (qual_panel)
    {
        value = qual_panel->GetUpdateMRNAProductFlag();
    }
    return value;
}

bool IActionPanel::GetRetranslateCds() const
{
    bool value(false);
    ISpecificQual* qual_panel = m_Panel->GetSpecificQualPanel();
    if (qual_panel)
    {
        value = qual_panel->GetRetranslateCds();
    }
    return value;
}

CIRef<IEditingAction> IActionPanel::CreateAction2_imp(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
{
    CIRef<IEditingAction> cmd;
    string field2 = GetField2();
    if (NStr::IsBlank(field2)) 
        return cmd;
    CFieldNamePanel::EFieldType field_type = CFieldNamePanel::GetFieldTypeFromName(GetFieldType());
    int subtype2 = GetSubtype(GetFieldNamePanel2()); 
    cmd = CreateAction(seh, field2, field_type, subtype2, submit);

    return cmd;
}

CIRef<IEditingAction> IActionPanel::CreateAction1(CSeq_entry_Handle seh, CConstRef<objects::CSeq_submit> submit)
{
    CIRef<IEditingAction> cmd;
    string field1 = GetField1();
    if (NStr::IsBlank(field1)) 
        return cmd;

    CFieldNamePanel::EFieldType field_type = CFieldNamePanel::GetFieldTypeFromName(GetFieldType());

    int subtype1 = GetSubtype(GetFieldNamePanel1());    
    cmd = CreateAction(seh, field1, field_type, subtype1, submit);
    return cmd;
}


template <template <typename T2> class T1>
CQualPanel<T1>::CQualPanel()
{
    Init();
}

template <template <typename T2> class T1>
CQualPanel<T1>::CQualPanel( wxWindow* parent, bool composed, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    :    m_composed(composed)
{
    Init();
    Create(parent, id, pos, size, style);
}

template <template <typename T2> class T1>
bool CQualPanel<T1>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

template <template <typename T2> class T1>
CQualPanel<T1>::~CQualPanel()
{
}

template <template <typename T2> class T1>
void CQualPanel<T1>::Init()
{
    m_Notebook = NULL;
    m_SpecificQual = NULL;
}

template <template <typename T2> class T1>
void CQualPanel<T1>::CreateControls()
{    
    CQualPanel<T1>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxNotebook(itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 0, wxGROW|wxALL, 5);
    m_Notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &CQualPanel<T1>::OnPageChanged, this);
    
    T1<CSourceFieldNamePanel> *panel1 = new T1<CSourceFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel1,_("Taxname"));
    panel1->SetFieldName("taxname");

    T1<CSourceFieldNamePanel> *panel2 = new T1<CSourceFieldNamePanel>(m_Notebook); 
    m_Notebook->AddPage(panel2,_("Source"));
    panel2->AddAdditionalItems(m_composed);

    T1<CFeatureFieldNamePanel> *panel3 =  new T1<CFeatureFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel3,_("Feature"));   

    T1<CCDSGeneProtFieldNamePanel> *panel4 = new T1<CCDSGeneProtFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel4,_("CDS-Gene-Prot-mRNA"));
    panel4->SetFieldName("protein name");

    T1<CRNAFieldNamePanel> *panel5 = new T1<CRNAFieldNamePanel>(m_Notebook);
    m_Notebook->AddPage(panel5,_("RNA"));
    panel5->SetFieldName("product");

    T1<CFieldHandlerNamePanel> *panel6 =  new T1<CFieldHandlerNamePanel>(m_Notebook);
    panel6->SetFieldNames(CMolInfoField::GetFieldNames());
    m_Notebook->AddPage(panel6,_("MolInfo"));
    panel6->SetFieldName("molecule");

    T1<CFieldHandlerNamePanel> *panel7 =  new T1<CFieldHandlerNamePanel>(m_Notebook);
    panel7->SetFieldNames(CPubField::GetFieldNames());
    m_Notebook->AddPage(panel7,_("Pub"));
    panel7->SetFieldName("title");

    T1<CStructCommentFieldPanel> *panel8 =  new T1<CStructCommentFieldPanel>(m_Notebook);
    m_Notebook->AddPage(panel8,_("Structured Comment"));

    T1<CFieldHandlerNamePanel> *panel9 =  new T1<CFieldHandlerNamePanel>(m_Notebook);
    panel9->SetFieldNames(CDBLinkField::GetFieldNames());
    m_Notebook->AddPage(panel9,_("DBLink"));
    panel9->SetFieldName("title");

    T1<CFieldHandlerNamePanel> *panel10 =  new T1<CFieldHandlerNamePanel>(m_Notebook);
    panel10->SetFieldNames(CMiscFieldPanel::GetStrings());
    m_Notebook->AddPage(panel10,_("Misc"));
    panel10->SetFieldName(kDefinitionLineLabel);    

    wxCommandEvent fieldTypeEvent2(wxEVT_COMMAND_UPDATE_CONSTRAINT_FIELD_NEW, GetId());
    fieldTypeEvent2.SetEventObject(this);
    GetEventHandler()->ProcessEvent(fieldTypeEvent2);
}

template <template <typename T2> class T1>
void CQualPanel<T1>::OnPageChanged(wxBookCtrlEvent& event)
{
    wxCommandEvent fieldTypeEvent2(wxEVT_COMMAND_UPDATE_CONSTRAINT_FIELD_NEW);
    fieldTypeEvent2.SetEventObject(this);
    GetEventHandler()->ProcessEvent(fieldTypeEvent2);

    wxCommandEvent fieldTypeEvent3(wxEVT_COMMAND_UPDATE_AECR_QUAL_CHOICES);
    fieldTypeEvent3.SetEventObject(this);
    GetEventHandler()->ProcessEvent(fieldTypeEvent3);

    event.Skip();
}

template <template <typename T2> class T1>
void CQualPanel<T1>::UnbindNotebookPageChanged()
{
    m_Notebook->Unbind(wxEVT_NOTEBOOK_PAGE_CHANGED, &CQualPanel<T1>::OnPageChanged, this);
}

template <template <typename T2> class T1>
void CQualPanel<T1>::SetShowLegalOnly(bool show)
{
    for (size_t i = 0; i < m_Notebook->GetPageCount(); i++)
    {
        if (m_Notebook->GetPageText(i) == _("Feature"))
        {
            wxWindow *win = m_Notebook->GetPage(i);
            ISpecificQual* qual_win = dynamic_cast<ISpecificQual*>(win);
            qual_win->SetShowLegalOnly(show);
            break;
        }
    }
}

template <template <typename T2> class T1>
bool CQualPanel<T1>::ShowToolTips()
{
    return true;
}

template <template <typename T2> class T1>
wxBitmap CQualPanel<T1>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

template <template <typename T2> class T1>
wxIcon CQualPanel<T1>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


// Apply Panel
IMPLEMENT_DYNAMIC_CLASS( CQualApplyPanel, wxPanel )

BEGIN_EVENT_TABLE( CQualApplyPanel, wxPanel )
    EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_UPDATE_AECR_QUAL_CHOICES, CQualApplyPanel::OnUpdateQualChoices)
END_EVENT_TABLE()

CQualApplyPanel::CQualApplyPanel()
{
    Init();
}

CQualApplyPanel::CQualApplyPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualApplyPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualApplyPanel::~CQualApplyPanel()
{
}

void CQualApplyPanel::Init()
{
    m_Panel = NULL;
    m_ApplyChoiceBox = NULL;
    m_AutopopulateOpt = NULL;
}

void CQualApplyPanel::CreateControls()
{    
    CQualApplyPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CSingleFieldPanel>* panel = new CQualPanel<CSingleFieldPanel>(itemPanel1);
    panel->SetShowLegalOnly(true);
    m_Panel = panel;
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);

    m_AutopopulateOpt = new wxCheckBox( itemPanel1, ID_AECR_AUTOPOPULATE_APPLY_CHECKBOX, _("Autopopulate"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AutopopulateOpt->SetValue(true);
    itemBoxSizer2->Add(m_AutopopulateOpt, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_AutopopulateOpt->Bind(wxEVT_CHECKBOX, &CQualApplyPanel::OnAutopopulateOptClick, this);

    wxArrayString choice_strings;
    m_allow_other = true;
    vector<string> choices = m_Panel->GetChoices(m_allow_other); 
                
    ITERATE(vector<string>, it, choices) {
        choice_strings.Add(ToWxString(*it));
    }

    m_ApplyChoiceBox = new wxComboBox(itemPanel1, ID_AECR_AUTOPOPULATE_APPLY_COMBOBOX, wxEmptyString, wxDefaultPosition, wxSize(250, -1), choice_strings, 0 );
    itemBoxSizer2->Add(m_ApplyChoiceBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ApplyChoiceBox->Bind(wxEVT_TEXT, &CQualApplyPanel::OnTextEnter, this);

    x_Autopopulate();
}

void CQualApplyPanel::OnUpdateQualChoices(wxCommandEvent& event)
{
    UpdateQualChoices();
}

void CQualApplyPanel::UpdateQualChoices()
{
    if (!m_Panel || !m_ApplyChoiceBox)
        return;

    wxArrayString choice_strings;
    m_allow_other = true;
    vector<string> choices = m_Panel->GetChoices(m_allow_other); 
                
    ITERATE(vector<string>, it, choices) 
    {
        choice_strings.Add(ToWxString(*it));
    }
    wxString s = m_ApplyChoiceBox->GetValue();
    m_ApplyChoiceBox->Set(choice_strings);
    m_ApplyChoiceBox->ChangeValue(s);   
    x_Autopopulate();
    IActionPanel::UpdateQualChoices();
}

void CQualApplyPanel::OnTextEnter(wxCommandEvent& event)
{
    if (m_allow_other)
    {
        event.Skip();
        return;
    }
    wxString s = m_ApplyChoiceBox->GetValue();
    if ( m_ApplyChoiceBox->FindString(s)==wxNOT_FOUND)
    {
        m_ApplyChoiceBox->SetSelection(wxNOT_FOUND);
        m_ApplyChoiceBox->ChangeValue(wxEmptyString);
    }
}

void CQualApplyPanel::x_Autopopulate()
{
    if (!m_ApplyChoiceBox || !m_Panel || !m_AutopopulateOpt->IsChecked())
        return;

    wxString s = m_ApplyChoiceBox->GetValue();
    int old_sel =  m_ApplyChoiceBox->FindString(s);

    string val = GetAutopopulateValue(GetParent());
    int sel = m_ApplyChoiceBox->FindString(val);
    if (val.empty() && old_sel != wxNOT_FOUND)
    {
        return;
    }
    m_ApplyChoiceBox->SetSelection(sel);    
    m_ApplyChoiceBox->ChangeValue(wxString(val));   
    
}

void CQualApplyPanel::OnAutopopulateOptClick( wxCommandEvent& event )
{
    if (m_AutopopulateOpt->IsChecked()) 
    {
        x_Autopopulate();
    }    
    event.Skip();
}

void CQualApplyPanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text)
{
    string val = m_ApplyChoiceBox->GetValue().ToStdString();
    action1->SetExistingText(existing_text);
    action1->Apply(val);
}

void CQualApplyPanel::ClearValues()
{
    m_ApplyChoiceBox->SetSelection(wxNOT_FOUND);
    m_ApplyChoiceBox->ChangeValue(wxEmptyString);  
    CFieldNamePanel* panel = GetFieldNamePanel1();
    panel->ClearValues();
    x_Autopopulate();
}

bool CQualApplyPanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualApplyPanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualApplyPanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

// Edit Panel
IMPLEMENT_DYNAMIC_CLASS( CQualEditPanel, wxPanel )

BEGIN_EVENT_TABLE( CQualEditPanel, wxPanel )
    EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_UPDATE_AECR_QUAL_CHOICES, CQualEditPanel::OnUpdateQualChoices)
END_EVENT_TABLE()

CQualEditPanel::CQualEditPanel()
{
    Init();
}

CQualEditPanel::CQualEditPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualEditPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualEditPanel::~CQualEditPanel()
{
}

void CQualEditPanel::Init()
{
    m_AutopopulateOpt = NULL;
    m_EditOptions = NULL;
}

void CQualEditPanel::CreateControls()
{    
    CQualEditPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CSingleFieldPanel>* panel = new CQualPanel<CSingleFieldPanel>(itemPanel1);
    panel->SetShowLegalOnly(true);
    m_Panel = panel;
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);

    m_AutopopulateOpt = new wxCheckBox( itemPanel1, ID_AECR_AUTOPOPULATE_EDIT_CHECKBOX, _("Autopopulate"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AutopopulateOpt->SetValue(true);
    itemBoxSizer2->Add(m_AutopopulateOpt, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_AutopopulateOpt->Bind(wxEVT_CHECKBOX, &CQualEditPanel::OnAutopopulateOptClick, this);

    m_EditOptions = new CEditTextOptionsPanel(itemPanel1);
    itemBoxSizer2->Add(m_EditOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    x_Autopopulate();
}

void CQualEditPanel::x_Autopopulate()
{
    if (!m_EditOptions || !m_Panel || !m_AutopopulateOpt->IsChecked())
        return;

    string val = GetAutopopulateValue(GetParent());
    m_EditOptions->SetFind(val);    
}

void CQualEditPanel::OnAutopopulateOptClick( wxCommandEvent& event )
{
    if (m_AutopopulateOpt->IsChecked()) 
    {
        x_Autopopulate();
    }    
    event.Skip();
}

void CQualEditPanel::UpdateQualChoices()
{
    if (!m_Panel || !m_EditOptions)
        return;
    x_Autopopulate();
    IActionPanel::UpdateQualChoices();
}

void CQualEditPanel::OnUpdateQualChoices(wxCommandEvent& event)
{
    UpdateQualChoices();
}

void CQualEditPanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text) 
{
    action1->Edit(m_EditOptions->GetFind(), m_EditOptions->GetReplace(), m_EditOptions->GetSearchLoc(), m_EditOptions->GetIgnoreCase(), m_EditOptions->GetIsRegex());
}

void CQualEditPanel::ClearValues()
{
    m_EditOptions->ClearValues();
    CFieldNamePanel* panel = GetFieldNamePanel1();
    panel->ClearValues();
    x_Autopopulate();
}

bool CQualEditPanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualEditPanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualEditPanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

// Convert Panel
IMPLEMENT_DYNAMIC_CLASS( CQualConvertPanel, wxPanel )

BEGIN_EVENT_TABLE( CQualConvertPanel, wxPanel )
END_EVENT_TABLE()

CQualConvertPanel::CQualConvertPanel()
{
    Init();
}

CQualConvertPanel::CQualConvertPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualConvertPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualConvertPanel::~CQualConvertPanel()
{
}

void CQualConvertPanel::Init()
{
}

void CQualConvertPanel::CreateControls()
{    
    CQualConvertPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CDualFieldPanel>* panel = new CQualPanel<CDualFieldPanel>(itemPanel1);
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);
    m_Panel = panel;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_LeaveOriginal = new wxCheckBox(itemPanel1, ID_AECR_LEAVE_ORIGINAL_CONVERT_CHECKBOX, wxT("Leave on original"), wxDefaultPosition, wxDefaultSize, 0);
    m_LeaveOriginal->SetValue(false);
    itemBoxSizer3->Add(m_LeaveOriginal, 0, wxALL, 5);
    
    m_StripNameFromText = new wxCheckBox(itemPanel1, ID_AECR_STRIP_NAME_CONVERT_CHECKBOX, wxT("Strip name from text"), wxDefaultPosition, wxDefaultSize, 0);
    m_StripNameFromText->SetValue(false);
    itemBoxSizer3->Add(m_StripNameFromText, 0, wxALL, 5);

    m_CapChangeOptions = new CCapChangePanel(itemPanel1);
    itemBoxSizer2->Add(m_CapChangeOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CQualConvertPanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text)
{
    action2->SetExistingText(existing_text);
    ECapChange cap_change = m_CapChangeOptions->GetCapitalizationRequest();

    string strip_name;
    if (m_StripNameFromText->IsChecked()) 
    {
        strip_name = m_Panel->GetField2();
    }
    action1->ConvertFrom(cap_change, m_LeaveOriginal->IsChecked(), strip_name, action2);
}

void CQualConvertPanel::ClearValues()
{
    m_LeaveOriginal->SetValue(false);
    m_StripNameFromText->SetValue(false);
    m_CapChangeOptions->ClearValues();
    CFieldNamePanel* panel1 = GetFieldNamePanel1();
    panel1->ClearValues();
    CFieldNamePanel* panel2 = GetFieldNamePanel2();
    panel2->ClearValues();
}

bool CQualConvertPanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualConvertPanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualConvertPanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


// Parse Panel
IMPLEMENT_DYNAMIC_CLASS( CQualParsePanel, wxPanel )

BEGIN_EVENT_TABLE( CQualParsePanel, wxPanel )
END_EVENT_TABLE()

CQualParsePanel::CQualParsePanel()
{
    Init();
}

CQualParsePanel::CQualParsePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualParsePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualParsePanel::~CQualParsePanel()
{
}

void CQualParsePanel::Init()
{
}

void CQualParsePanel::CreateControls()
{    
    CQualParsePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CDualFieldPanel>* panel = new CQualPanel<CDualFieldPanel>(itemPanel1);
    m_Panel = panel;
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);

    m_ParseOptions = new CParseTextOptionsDlg(itemPanel1);
    itemBoxSizer2->Add(m_ParseOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CQualParsePanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text)
{
    action2->SetExistingText(existing_text);
    action1->ParseFrom(*m_ParseOptions->GetParseOptions(), action2);
}

void CQualParsePanel::ClearValues()
{   
    m_ParseOptions->ClearValues();
    CFieldNamePanel* panel1 = GetFieldNamePanel1();
    panel1->ClearValues();
    CFieldNamePanel* panel2 = GetFieldNamePanel2();
    panel2->ClearValues();
}

bool CQualParsePanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualParsePanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualParsePanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


// Remove Outside Panel
IMPLEMENT_DYNAMIC_CLASS( CQualRemoveOutsidePanel, wxPanel )

BEGIN_EVENT_TABLE( CQualRemoveOutsidePanel, wxPanel )
END_EVENT_TABLE()

CQualRemoveOutsidePanel::CQualRemoveOutsidePanel()
{
    Init();
}

CQualRemoveOutsidePanel::CQualRemoveOutsidePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualRemoveOutsidePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualRemoveOutsidePanel::~CQualRemoveOutsidePanel()
{
}

void CQualRemoveOutsidePanel::Init()
{
}

void CQualRemoveOutsidePanel::CreateControls()
{    
    CQualRemoveOutsidePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CSingleFieldPanel>* panel = new CQualPanel<CSingleFieldPanel>(itemPanel1);
    m_Panel = panel;
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);

    m_RemoveTextOutsideOptions = new CRemoveTextOutsideStringPanel(itemPanel1);
    itemBoxSizer2->Add(m_RemoveTextOutsideOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CQualRemoveOutsidePanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text)
{
    action1->RemoveOutside(m_RemoveTextOutsideOptions->GetRemoveTextOptions());
}

void CQualRemoveOutsidePanel::ClearValues()
{   
    m_RemoveTextOutsideOptions->ClearValues();
    CFieldNamePanel* panel1 = GetFieldNamePanel1();
    panel1->ClearValues();   
}

bool CQualRemoveOutsidePanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualRemoveOutsidePanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualRemoveOutsidePanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

// Remove Panel
IMPLEMENT_DYNAMIC_CLASS( CQualRemovePanel, wxPanel )

BEGIN_EVENT_TABLE( CQualRemovePanel, wxPanel )
END_EVENT_TABLE()

CQualRemovePanel::CQualRemovePanel()
{
    Init();
}

CQualRemovePanel::CQualRemovePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualRemovePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualRemovePanel::~CQualRemovePanel()
{
}

void CQualRemovePanel::Init()
{
}

void CQualRemovePanel::CreateControls()
{    
    CQualRemovePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CSingleFieldPanel>* panel = new CQualPanel<CSingleFieldPanel>(itemPanel1, true);
    m_Panel = panel;
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);
}

void CQualRemovePanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text)
{
    action1->Remove();
}

void CQualRemovePanel::ClearValues()
{   
    CFieldNamePanel* panel1 = GetFieldNamePanel1();
    panel1->ClearValues();   
}

bool CQualRemovePanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualRemovePanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualRemovePanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

// Copy Panel
IMPLEMENT_DYNAMIC_CLASS( CQualCopyPanel, wxPanel )

BEGIN_EVENT_TABLE( CQualCopyPanel, wxPanel )
END_EVENT_TABLE()

CQualCopyPanel::CQualCopyPanel()
{
    Init();
}

CQualCopyPanel::CQualCopyPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualCopyPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualCopyPanel::~CQualCopyPanel()
{
}

void CQualCopyPanel::Init()
{
}

void CQualCopyPanel::CreateControls()
{    
    CQualCopyPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CDualFieldPanel>* panel = new CQualPanel<CDualFieldPanel>(itemPanel1);
    m_Panel = panel;
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);
}

void CQualCopyPanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text)
{
    action2->SetExistingText(existing_text);
    action1->CopyFrom(action2);
}

void CQualCopyPanel::ClearValues()
{   
    CFieldNamePanel* panel1 = GetFieldNamePanel1();
    panel1->ClearValues();   
    CFieldNamePanel* panel2 = GetFieldNamePanel2();
    panel2->ClearValues();   
}


bool CQualCopyPanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualCopyPanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualCopyPanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

// Swap Panel
IMPLEMENT_DYNAMIC_CLASS( CQualSwapPanel, wxPanel )

BEGIN_EVENT_TABLE( CQualSwapPanel, wxPanel )
END_EVENT_TABLE()

CQualSwapPanel::CQualSwapPanel()
{
    Init();
}

CQualSwapPanel::CQualSwapPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualSwapPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualSwapPanel::~CQualSwapPanel()
{
}

void CQualSwapPanel::Init()
{
}

void CQualSwapPanel::CreateControls()
{    
    CQualSwapPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CQualPanel<CDualFieldPanel>* panel = new CQualPanel<CDualFieldPanel>(itemPanel1);
    m_Panel = panel;
    itemBoxSizer2->Add(panel, 0, wxGROW, 0);
}

void CQualSwapPanel::SetupAction(CIRef<IEditingAction> action1, CIRef<IEditingAction> action2, edit::EExistingText existing_text)
{
    action1->SwapFrom(action2);
}

void CQualSwapPanel::ClearValues()
{   
    CFieldNamePanel* panel1 = GetFieldNamePanel1();
    panel1->ClearValues();   
    CFieldNamePanel* panel2 = GetFieldNamePanel2();
    panel2->ClearValues();   
}


bool CQualSwapPanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualSwapPanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualSwapPanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

// Single Source Panel
template<typename T>
CSingleFieldPanel<T>::CSingleFieldPanel()
{
    Init();
}

template<typename T>
CSingleFieldPanel<T>::CSingleFieldPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

template<typename T>
bool CSingleFieldPanel<T>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

template<typename T>
CSingleFieldPanel<T>::~CSingleFieldPanel()
{
}

template<typename T>
void CSingleFieldPanel<T>::Init()
{
}

template<typename T>
void CSingleFieldPanel<T>::CreateControls()
{    
    CSingleFieldPanel<T>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Field = new T(itemPanel1);
    itemBoxSizer3->Add(m_Field, 0, wxALIGN_CENTER_HORIZONTAL, 0);
}

template<typename T>
bool CSingleFieldPanel<T>::SetFieldName(const string &field)
{
    if (!field.empty())
        return m_Field->SetFieldName(field);
    return false;
}

template<typename T>
void CSingleFieldPanel<T>::AddAdditionalItems(bool composed)
{
    if (composed)
        m_Field->AddAdditionalItems();
}

template<typename T>
bool CSingleFieldPanel<T>::ShowToolTips()
{
    return true;
}

template<typename T>
wxBitmap CSingleFieldPanel<T>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

template<typename T>
wxIcon CSingleFieldPanel<T>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


// Dual Source Panel

template<typename T>
CDualFieldPanel<T>::CDualFieldPanel()
{
    Init();
}

template<typename T>
CDualFieldPanel<T>::CDualFieldPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

template<typename T>
bool CDualFieldPanel<T>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

template<typename T>
CDualFieldPanel<T>::~CDualFieldPanel()
{
}

template<typename T>
void CDualFieldPanel<T>::Init()
{
}

template<typename T>
void CDualFieldPanel<T>::CreateControls()
{    
    CDualFieldPanel<T>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);
    
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText1 = new wxStaticText( itemPanel1, wxID_STATIC, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Field1 = new T(itemPanel1);
    itemBoxSizer3->Add(m_Field1, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Field2 = new T(itemPanel1);
    itemBoxSizer4->Add(m_Field2, 0, wxALIGN_CENTER_HORIZONTAL, 0);
}

template<typename T>
bool CDualFieldPanel<T>::SetFieldName(const string &field)
{
    if (!field.empty())
    {
        return m_Field1->SetFieldName(field);
    }
    return false;
}

template<typename T>
bool CDualFieldPanel<T>::SetFieldName2(const string &field)
{
    if (!field.empty())
    {
        return m_Field2->SetFieldName(field);
    }
    return false;
}

template<typename T>
bool CDualFieldPanel<T>::ShowToolTips()
{
    return true;
}

template<typename T>
wxBitmap CDualFieldPanel<T>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

template<typename T>
wxIcon CDualFieldPanel<T>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::CSingleFieldPanel()
{
    Init();
}

CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::CSingleFieldPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::~CSingleFieldPanel()
{
}

void CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::Init()
{
}

void CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::CreateControls()
{    
    CSingleFieldPanel<CCDSGeneProtFieldNamePanel>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Field = new CCDSGeneProtFieldNamePanel(itemPanel1);
    itemBoxSizer3->Add(m_Field, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    m_UpdatemRNAProduct = new wxCheckBox(itemPanel1, wxID_ANY, wxT("Make mRNA product match CDS protein name"), wxDefaultPosition, wxDefaultSize, 0);
    m_UpdatemRNAProduct->SetValue(true);
    itemBoxSizer3->Add(m_UpdatemRNAProduct, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);  

    m_RetranslateCds = new wxCheckBox(itemPanel1, wxID_ANY, wxT("Retranslate adjusted coding regions"), wxDefaultPosition, wxDefaultSize, 0);
    m_RetranslateCds->SetValue(false);
    itemBoxSizer3->Add(m_RetranslateCds, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);    
}

void CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::UpdateMRNAbutton()
{
    m_UpdatemRNAProduct->Disable();
    string field  = m_Field->GetFieldName(true);
    if (NStr::EqualNocase(field, "protein name"))
    {
        m_UpdatemRNAProduct->Enable();
    }       
}

void CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::UpdateRetranslateCds()
{
    m_RetranslateCds->Disable();
    string field  = m_Field->GetFieldName(true);
    if (NStr::EqualNocase(field, "codon-start"))
    {
        m_RetranslateCds->Enable();
    }       
}

bool CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::GetUpdateMRNAProductFlag() const
{
    return m_UpdatemRNAProduct->IsEnabled() && m_UpdatemRNAProduct->GetValue();
}

bool CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::GetRetranslateCds() const
{
    return m_RetranslateCds->IsEnabled() && m_RetranslateCds->GetValue();
}

bool CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::SetFieldName(const string &field)
{
    if (!field.empty())
        return m_Field->SetFieldName(field);
    return false;
}

bool CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::ShowToolTips()
{
    return true;
}

wxBitmap CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CSingleFieldPanel<CCDSGeneProtFieldNamePanel>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


CDualFieldPanel<CCDSGeneProtFieldNamePanel>::CDualFieldPanel()
{
    Init();
}

CDualFieldPanel<CCDSGeneProtFieldNamePanel>::CDualFieldPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CDualFieldPanel<CCDSGeneProtFieldNamePanel>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CDualFieldPanel<CCDSGeneProtFieldNamePanel>::~CDualFieldPanel()
{
}

void CDualFieldPanel<CCDSGeneProtFieldNamePanel>::Init()
{
}

void CDualFieldPanel<CCDSGeneProtFieldNamePanel>::CreateControls()
{    
    CDualFieldPanel<CCDSGeneProtFieldNamePanel>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText1 = new wxStaticText( itemPanel1, wxID_STATIC, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Field1 = new CCDSGeneProtFieldNamePanel(itemPanel1);
    itemBoxSizer3->Add(m_Field1, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Field2 = new CCDSGeneProtFieldNamePanel(itemPanel1);
    itemBoxSizer4->Add(m_Field2, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    m_UpdatemRNAProduct = new wxCheckBox(itemPanel1, wxID_ANY, wxT("Make mRNA product match CDS protein name"), wxDefaultPosition, wxDefaultSize, 0);
    m_UpdatemRNAProduct->SetValue(true);
    itemBoxSizer1->Add(m_UpdatemRNAProduct, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);    

    m_RetranslateCds = new wxCheckBox(itemPanel1, wxID_ANY, wxT("Retranslate adjusted coding regions"), wxDefaultPosition, wxDefaultSize, 0);
    m_RetranslateCds->SetValue(false);
    itemBoxSizer1->Add(m_RetranslateCds, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);    
}

void CDualFieldPanel<CCDSGeneProtFieldNamePanel>::UpdateMRNAbutton()
{
    m_UpdatemRNAProduct->Disable();
    string field1  = m_Field1->GetFieldName(true);
    string field2  = m_Field2->GetFieldName(true);
    if (NStr::EqualNocase(field1, "protein name") || NStr::EqualNocase(field2, "protein name"))
    {
        m_UpdatemRNAProduct->Enable();
    }       
}

bool CDualFieldPanel<CCDSGeneProtFieldNamePanel>::GetUpdateMRNAProductFlag() const
{
    return m_UpdatemRNAProduct->IsEnabled() && m_UpdatemRNAProduct->GetValue();
}

void CDualFieldPanel<CCDSGeneProtFieldNamePanel>::UpdateRetranslateCds()
{
    m_RetranslateCds->Disable();
    string field  = m_Field2->GetFieldName(true);
    if (NStr::EqualNocase(field, "codon-start"))
    {
        m_RetranslateCds->Enable();
    }       
}

bool CDualFieldPanel<CCDSGeneProtFieldNamePanel>::GetRetranslateCds() const
{
    return m_RetranslateCds->IsEnabled() && m_RetranslateCds->GetValue();
}

bool CDualFieldPanel<CCDSGeneProtFieldNamePanel>::SetFieldName(const string &field)
{
    if (!field.empty())
    {
        return m_Field1->SetFieldName(field);
    }
    return false;
}

bool CDualFieldPanel<CCDSGeneProtFieldNamePanel>::SetFieldName2(const string &field)
{
    if (!field.empty())
    {
        return m_Field2->SetFieldName(field);
    }
    return false;
}

bool CDualFieldPanel<CCDSGeneProtFieldNamePanel>::ShowToolTips()
{
    return true;
}

wxBitmap CDualFieldPanel<CCDSGeneProtFieldNamePanel>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CDualFieldPanel<CCDSGeneProtFieldNamePanel>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


CDualFieldPanel<CRNAFieldNamePanel>::CDualFieldPanel()
{
    Init();
}

CDualFieldPanel<CRNAFieldNamePanel>::CDualFieldPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CDualFieldPanel<CRNAFieldNamePanel>::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CDualFieldPanel<CRNAFieldNamePanel>::~CDualFieldPanel()
{
}

void CDualFieldPanel<CRNAFieldNamePanel>::Init()
{
}

void CDualFieldPanel<CRNAFieldNamePanel>::CreateControls()
{    
    CDualFieldPanel<CRNAFieldNamePanel>* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    m_Field = new CDualRNAFieldNamePanel(itemPanel1);
    itemBoxSizer1->Add(m_Field, 0, wxALIGN_CENTER_HORIZONTAL, 0);    
}

bool CDualFieldPanel<CRNAFieldNamePanel>::SetFieldName(const string &field)
{
    if (!field.empty())
    {
        return m_Field->SetFieldName(field);
    }
    return false;
}

bool CDualFieldPanel<CRNAFieldNamePanel>::SetFieldName2(const string &field)
{
    if (!field.empty())
    {
        return m_Field->SetFieldName2(field);
    }
    return false;
}

bool CDualFieldPanel<CRNAFieldNamePanel>::ShowToolTips()
{
    return true;
}

wxBitmap CDualFieldPanel<CRNAFieldNamePanel>::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CDualFieldPanel<CRNAFieldNamePanel>::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


END_NCBI_SCOPE
