/*  $Id: remove_text_inside_string_dlg.cpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Igor Filippov, based on work of Colleen Bollin
 */


#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence_edit/remove_text_inside_string_dlg.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/misc_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>

#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE

//USING_SCOPE(ncbi::objects);

/*!
 * CRemoveTextInsideStrDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRemoveTextInsideStrDlg, CBulkCmdDlg )


/*!
 * CRemoveTextInsideStrDlg event table definition
 */

BEGIN_EVENT_TABLE( CRemoveTextInsideStrDlg, CBulkCmdDlg )

////@begin CRemoveTextInsideStrDlg event table entries
////@end CRemoveTextInsideStrDlg event table entries
    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CRemoveTextInsideStrDlg::ProcessUpdateFeatEvent )

END_EVENT_TABLE()


/*!
 * CRemoveTextInsideStrDlg constructors
 */

CRemoveTextInsideStrDlg::CRemoveTextInsideStrDlg()
{
    Init();
}

CRemoveTextInsideStrDlg::CRemoveTextInsideStrDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CRemoveTextInsideStrDlg creator
 */

bool CRemoveTextInsideStrDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRemoveTextInsideStrDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRemoveTextInsideStrDlg creation
    return true;
}


/*!
 * CRemoveTextInsideStrDlg destructor
 */

CRemoveTextInsideStrDlg::~CRemoveTextInsideStrDlg()
{
////@begin CRemoveTextInsideStrDlg destruction
////@end CRemoveTextInsideStrDlg destruction
}


/*!
 * Member initialisation
 */

void CRemoveTextInsideStrDlg::Init()
{
////@begin CRemoveTextInsideStrDlg member initialisation
    m_ParseOptions = NULL;
    m_FromFieldSizer = NULL;
    m_OkCancel = NULL;
////@end CRemoveTextInsideStrDlg member initialisation
    m_FieldFrom = NULL;
}


/*!
 * Control creation for CRemoveTextInsideStrDlg
 */

void CRemoveTextInsideStrDlg::CreateControls()
{    
////@begin CRemoveTextInsideStrDlg content construction
    CRemoveTextInsideStrDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_ParseOptions = new CParseTextOptionsDlg( itemCBulkCmdDlg1, ID_WINDOW2, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER, true );
    itemBoxSizer2->Add(m_ParseOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_RemoveFirstOnly = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Remove first instance in each string"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RemoveFirstOnly->SetValue(true);
    itemBoxSizer3->Add(m_RemoveFirstOnly, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    wxRadioButton *remove_all = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Remove all instances"), wxDefaultPosition, wxDefaultSize, 0 );
    remove_all->SetValue(false);
    itemBoxSizer3->Add(remove_all, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_FromFieldSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(m_FromFieldSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FromFieldSizer->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, ID_WINDOW13, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

////@end CRemoveTextInsideStrDlg content construction

    vector<CFieldNamePanel::EFieldType> field_types;
    // number of items that should not appear in the "TO" field choice list.
    size_t not_TOList = 4;
    field_types.push_back(CFieldNamePanel::eFieldType_Taxname);
    field_types.push_back(CFieldNamePanel::eFieldType_Source);
    field_types.push_back(CFieldNamePanel::eFieldType_Feature);
    field_types.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
    field_types.push_back(CFieldNamePanel::eFieldType_RNA);
    field_types.push_back(CFieldNamePanel::eFieldType_Misc);
    field_types.push_back(CFieldNamePanel::eFieldType_TaxnameAfterBinomial);
    field_types.push_back(CFieldNamePanel::eFieldType_FileId);
    field_types.push_back(CFieldNamePanel::eFieldType_GeneralId);
    field_types.push_back(CFieldNamePanel::eFieldType_SeqId);
    m_FieldFrom = new CFieldChoicePanel( this, field_types, false, false, wxID_ANY, wxDefaultPosition, wxSize(400, 150), 0 ); // true, false
    m_FromFieldSizer->Add(m_FieldFrom, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}

void CRemoveTextInsideStrDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this->GetSizer());
}

/*!
 * Should we show tooltips?
 */

bool CRemoveTextInsideStrDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRemoveTextInsideStrDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRemoveTextInsideStrDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRemoveTextInsideStrDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRemoveTextInsideStrDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRemoveTextInsideStrDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRemoveTextInsideStrDlg icon retrieval
}


void CRemoveTextInsideStrDlg::SetFromFieldType(CFieldNamePanel::EFieldType field_type)
{ 
    m_FieldFrom->SetFieldType(field_type);

    CRef<edit::CStringConstraint> string_constraint = m_ConstraintPanel->GetStringConstraint();
    if (!string_constraint) {
        m_ConstraintPanel->SetFieldType(field_type);
    }
    Layout();
    Fit();
    Refresh();
} 


CRef<CCmdComposite> CRemoveTextInsideStrDlg::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);

    m_ErrorMessage = kEmptyStr;
    string field1 = m_FieldFrom->GetFieldName();
    if (NStr::IsBlank(field1)) {
        m_ErrorMessage = "Must choose source field";
        return cmd;
    }

    CRef<edit::CStringConstraint> string_constraint(NULL);
    CRef<CMiscSeqTableColumn> src_col(new CMiscSeqTableColumn(field1));
    vector<CRef<edit::CApplyObject> > src_objects = src_col->GetApplyObjects(m_TopSeqEntry, kEmptyStr, string_constraint);
    if (src_objects.size() == 0) {
        wxMessageBox(wxT("No source objects found!"), wxT("Error"),
                wxOK | wxICON_ERROR, NULL);
        return cmd;
    }

    CRef<CScope> scope(&m_TopSeqEntry.GetScope());

    // Construct command
    cmd = new CCmdComposite("Remove Text Inside String");
    CRef<edit::CParseTextOptions> parse_options(NULL);
    parse_options = m_ParseOptions->GetParseOptions();
    NON_CONST_ITERATE(vector<CRef<edit::CApplyObject> >, src_it, src_objects) {
        if (!(*src_it)->PreExists()) {
            continue;
        }
        string orig_val = src_col->GetVal((*src_it)->GetObject());
        string new_val = m_ParseOptions->GetSelectedText(orig_val);
        NStr::TruncateSpacesInPlace(new_val);      
        if (!NStr::IsBlank(new_val)) {           
            bool changed = false;
            parse_options->RemoveSelectedText(orig_val,m_RemoveFirstOnly->GetValue());
            NStr::TruncateSpacesInPlace(orig_val);
            if (NStr::IsBlank(orig_val)) {
                src_col->ClearVal((*src_it)->SetObject());
                changed = true;
            }
            else if (src_col->SetVal((*src_it)->SetObject(), orig_val, edit::eExistingText_replace_old)) {
                changed = true;
            }
                
            if (changed) {
                CRef<CCmdComposite> ecmd = GetCommandFromApplyObject(**src_it);
                if (ecmd) {
                    cmd->AddCommand(*ecmd);
                }
            }            
        }	
    }

    return cmd;
}


string CRemoveTextInsideStrDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}


static void s_AddwxSize(wxSize& orig, const wxSize& add, int borders)
{
    if (orig.GetWidth() < add.GetWidth() + borders) {
        orig.SetWidth(add.GetWidth() + borders);
    }
    orig.SetHeight(orig.GetHeight() + add.GetHeight() + borders);
}


void CRemoveTextInsideStrDlg::UpdateEditor()
{
    /* wxSize min_size(0,0);
    wxSize this_size(0,0);
    size_t borders = 10;
    s_AddwxSize(min_size, m_ParseOptions->GetSize(), borders);

    // get height of from and to, add only the taller    
    wxSize from_size(0,0);
    if (m_FieldFrom) {
        from_size = m_FieldFrom->GetSize();
        s_AddwxSize(min_size, from_size, borders);
    }

    if (m_OkCancel) {
        s_AddwxSize(min_size, m_OkCancel->GetSize(), borders + 10);
    }

    this->SetMinClientSize(min_size);*/
    Layout();
    Fit();
    Refresh();
}

END_NCBI_SCOPE
