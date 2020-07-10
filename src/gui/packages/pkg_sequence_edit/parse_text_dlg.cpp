/*  $Id: parse_text_dlg.cpp 45101 2020-05-29 20:53:24Z asztalos $
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

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>

#include <gui/packages/pkg_sequence_edit/parse_text_dlg.hpp>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CParseTextDlg, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CParseTextDlg, CBulkCmdDlg )

////@begin CParseTextDlg event table entries
  EVT_UPDATE_FEATURE_LIST(wxID_ANY, CParseTextDlg::ProcessUpdateFeatEvent )
////@end CParseTextDlg event table entries

END_EVENT_TABLE()

CParseTextDlg::CParseTextDlg()
{
    Init();
}

CParseTextDlg::CParseTextDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, wb, id, caption, pos, size, style);
    SetRegistryPath("Dialogs.Edit.ParseText");
    LoadSettings();
}

bool CParseTextDlg::Create( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CParseTextDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, wb, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CParseTextDlg creation
    return true;
}

CParseTextDlg::~CParseTextDlg()
{
////@begin CParseTextDlg destruction
    SaveSettings();
////@end CParseTextDlg destruction
}

static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CParseTextDlg::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CParseTextDlg::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);
}


void CParseTextDlg::LoadSettings()
{
    if (m_RegPath.empty())
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


void CParseTextDlg::Init()
{
////@begin CParseTextDlg member initialisation
    m_Constraint = NULL;
    m_Field1 = NULL;
    m_Field2 = NULL;
////@end CParseTextDlg member initialisation
}

void CParseTextDlg::CreateControls()
{    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemCBulkCmdDlg1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemCBulkCmdDlg1, 1, wxGROW, 0);   

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_ParseOptions = new CParseTextOptionsDlg(itemCBulkCmdDlg1);
    itemBoxSizer2->Add(m_ParseOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText1 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Field1 = new CQualChoicePanel(itemCBulkCmdDlg1);
    itemBoxSizer4->Add(m_Field1, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText2 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Field2 = new CQualChoicePanel(itemCBulkCmdDlg1);
    itemBoxSizer5->Add(m_Field2, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    GetTopLevelSeqEntryAndProcessor();
    m_Constraint = new CConstraintPanel( itemCBulkCmdDlg1, m_TopSeqEntry);
    itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    COkCancelPanel* panelOkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(panelOkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}

bool CParseTextDlg::ShowToolTips()
{
    return true;
}
wxBitmap CParseTextDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CParseTextDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CParseTextDlg bitmap retrieval
}
wxIcon CParseTextDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CParseTextDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CParseTextDlg icon retrieval
}

void CParseTextDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this);
}

void CParseTextDlg::UpdateChildrenFeaturePanels( wxWindow* win )
{
    wxWindowList &slist = win->GetChildren();
    for (wxWindowList::iterator iter = slist.begin(); iter != slist.end(); ++iter) 
    {
        wxWindow* child = *iter;
        CFeatureTypePanel* panel = dynamic_cast<CFeatureTypePanel*>(child);
        if (panel) 
        {
            GetTopLevelSeqEntryAndProcessor();
            panel->ListPresentFeaturesFirst(m_TopSeqEntry);
        } 
        else 
        {
            UpdateChildrenFeaturePanels(child);
        }
    }     
}

void CParseTextDlg::SetFromFieldType(EFieldType field_type)
{
    m_Field1->SetSelection(field_type);
}

void CParseTextDlg::SetToFieldType(EFieldType field_type)
{
    m_Field2->SetSelection(field_type);
}

CRef<CCmdComposite> CParseTextDlg::GetCommand()
{
    CRef<CCmdComposite> cmd;

    GetTopLevelSeqEntryAndProcessor();

    CIRef<IEditingAction> action1 = m_Field1->CreateEditingAction(m_TopSeqEntry);
    if (!action1)
        return cmd;

    CIRef<IEditingAction> action2 = m_Field2->CreateEditingAction(m_TopSeqEntry);
    if (!action2)
        return cmd;

    CFieldNamePanel* field_name_panel = m_Constraint->GetFieldNamePanel();
    CRef<CEditingActionConstraint> constraint = CreateEditingActionConstraint(m_Field1->GetField(), 
                                                                              CFieldNamePanel::GetFieldTypeFromName(m_Field1->GetFieldType()), 
                                                                              GetSubtype(m_Field1->GetFieldNamePanel()), 
                                                                              field_name_panel ? field_name_panel->GetFieldName(true) : m_Constraint->GetFieldType(), 
                                                                              CFieldNamePanel::GetFieldTypeFromName(m_Constraint->GetFieldType()), 
                                                                              GetSubtype(field_name_panel),
                                                                              CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(m_Constraint->GetStringConstraint())));

    action1->SetConstraint(constraint);

    int num_conflicts = action1->CheckForExistingText(action2, IEditingAction::eActionType_ParseFrom, *m_ParseOptions->GetParseOptions());
    edit::EExistingText existing_text = x_GetExistingTextHandling (num_conflicts, true); 
    if (existing_text == edit::eExistingText_cancel) 
        return cmd;


    action2->SetExistingText(existing_text);
    action1->ParseFrom(*m_ParseOptions->GetParseOptions(), action2);

    cmd = action1->GetActionCommand();

    return cmd;
}

IMPLEMENT_DYNAMIC_CLASS( CQualChoicePanel, wxPanel )

BEGIN_EVENT_TABLE( CQualChoicePanel, wxPanel )
END_EVENT_TABLE()

CQualChoicePanel::CQualChoicePanel()
{
    Init();
}

CQualChoicePanel::CQualChoicePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CQualChoicePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
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

CQualChoicePanel::~CQualChoicePanel()
{
}

void CQualChoicePanel::Init()
{
}

void CQualChoicePanel::CreateControls()
{    
    CQualChoicePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxChoicebook(itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 0, wxGROW|wxALL, 5);
    
    CSourceFieldNamePanel *panel1 = new CSourceFieldNamePanel(m_Notebook);
    m_Notebook->AddPage(panel1,_("Taxname"));
    panel1->SetFieldName("taxname");

    CSourceFieldNamePanel *panel2 = new CSourceFieldNamePanel(m_Notebook); 
    m_Notebook->AddPage(panel2,_("Source"));

    CFeatureFieldNamePanel *panel3 =  new CFeatureFieldNamePanel(m_Notebook);
    m_Notebook->AddPage(panel3,_("Feature"));
    panel3->PopulateFeatureListbox();

    CCDSGeneProtFieldNamePanel *panel4 = new CCDSGeneProtFieldNamePanel(m_Notebook);
    m_Notebook->AddPage(panel4,_("CDS-Gene-Prot-mRNA"));
    panel4->SetFieldName("protein name");

    CRNAFieldNamePanel *panel5 = new CRNAFieldNamePanel(m_Notebook);
    m_Notebook->AddPage(panel5,_("RNA"));
    panel5->SetFieldName("product");

    CFieldHandlerNamePanel *panel6 =  new CFieldHandlerNamePanel(m_Notebook);
    panel6->SetFieldNames(CMolInfoField::GetFieldNames());
    m_Notebook->AddPage(panel6,_("MolInfo"));
    panel6->SetFieldName("molecule");

    CFieldHandlerNamePanel *panel7 =  new CFieldHandlerNamePanel(m_Notebook);
    panel7->SetFieldNames(CPubField::GetFieldNames());
    m_Notebook->AddPage(panel7,_("Pub"));
    panel7->SetFieldName("title");

    CStructCommentFieldPanel *panel8 =  new CStructCommentFieldPanel(m_Notebook);
    m_Notebook->AddPage(panel8,_("Structured Comment"));

    CFieldHandlerNamePanel *panel9 =  new CFieldHandlerNamePanel(m_Notebook);
    panel9->SetFieldNames(CDBLinkField::GetFieldNames());
    m_Notebook->AddPage(panel9,_("DBLink"));
    panel9->SetFieldName("title");

    CFieldHandlerNamePanel *panel10 =  new CFieldHandlerNamePanel(m_Notebook);
    panel10->SetFieldNames(CMiscFieldPanel::GetStrings());
    m_Notebook->AddPage(panel10,_("Misc"));
    panel10->SetFieldName(kDefinitionLineLabel);   

    wxPanel *panel11 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel11, wxString(kLocalId));

    wxPanel *panel12 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel12, wxString(kDefinitionLineLabel));

    wxPanel *panel13 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel13, wxString(kBankITComment));

    wxPanel *panel14 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel14, wxString(kTaxnameAfterBinomial));

    wxPanel *panel15 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel15, wxString("Dbxref"));

    wxPanel *panel16 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel16, wxString(kFileSeqId));

    CGeneralIDPanel *panel17 = new CGeneralIDPanel(m_Notebook);
    m_Notebook->AddPage(panel17, wxString(kGeneralId));

    wxPanel *panel18 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel18, wxString(kComment));

    wxPanel *panel19 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel19, wxString(kFlatFile));
}

void CQualChoicePanel::SetSelection(int page)
{
    if (page != wxNOT_FOUND)
    {
        m_Notebook->SetSelection(page);       
    }
}

string CQualChoicePanel::GetField(bool subfield) 
{
    string name;
    if (m_Notebook) 
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(win);
        if (panel)
        {
            name = panel->GetFieldName(subfield);;
        }
        else
        {
            name = GetFieldType();
        }
    }
    return name;
}

string CQualChoicePanel::GetFieldType(void)
{
    string field_type;
    int sel = m_Notebook->GetSelection();
    if (sel != wxNOT_FOUND)
    {
        field_type = m_Notebook->GetPageText(sel).ToStdString();
    }
    return field_type;
}

CFieldNamePanel* CQualChoicePanel::GetFieldNamePanel(void)
{
    CFieldNamePanel* name_panel(NULL);
    if (m_Notebook) 
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        name_panel = dynamic_cast<CFieldNamePanel*>(win);
    }
    return name_panel;
}


CIRef<IEditingAction> CQualChoicePanel::CreateEditingAction(CSeq_entry_Handle seh)
{
    CIRef<IEditingAction> cmd;
    string field = GetField();

    if (NStr::IsBlank(field)) 
        return cmd;

    CFieldNamePanel::EFieldType field_type = CFieldNamePanel::GetFieldTypeFromName(GetFieldType());

    int subtype = GetSubtype(GetFieldNamePanel());    
    cmd = CreateAction(seh, field, field_type, subtype);
    return cmd;
}

bool CQualChoicePanel::ShowToolTips()
{
    return true;
}

wxBitmap CQualChoicePanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CQualChoicePanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}


END_NCBI_SCOPE

// TODO
// GenBank Flatfile 1902


