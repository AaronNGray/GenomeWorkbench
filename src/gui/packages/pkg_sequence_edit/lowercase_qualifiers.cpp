/*  $Id: lowercase_qualifiers.cpp 42449 2019-02-27 20:41:38Z asztalos $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <objmgr/bioseq_handle.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <gui/packages/pkg_sequence_edit/lowercase_qualifiers.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CLowerQuals type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLowerQuals, CBulkCmdDlg )


/*!
 * CLowerQuals event table definition
 */

BEGIN_EVENT_TABLE( CLowerQuals, CBulkCmdDlg )

////@begin CLowerQuals event table entries
    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CLowerQuals::ProcessUpdateFeatEvent )
////@end CLowerQuals event table entries
END_EVENT_TABLE()


/*!
 * CLowerQuals constructors
 */

CLowerQuals::CLowerQuals()
{
    Init();
}

CLowerQuals::CLowerQuals( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CLowerQuals creator
 */

bool CLowerQuals::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLowerQuals creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLowerQuals creation
    return true;
}


/*!
 * CLowerQuals destructor
 */

CLowerQuals::~CLowerQuals()
{
////@begin CLowerQuals destruction
////@end CLowerQuals destruction
}


/*!
 * Member initialisation
 */

void CLowerQuals::Init()
{
////@begin CLowerQuals member initialisation
    m_FieldType = NULL;
    m_OkCancel = NULL;
////@end CLowerQuals member initialisation
}


/*!
 * Control creation for CLowerQuals
 */

void CLowerQuals::CreateControls()
{    
////@begin CLowerQuals content construction
    CLowerQuals* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    GetTopLevelSeqEntryAndProcessor();

    vector<CFieldNamePanel::EFieldType> field_types;
    field_types.push_back(CFieldNamePanel::eFieldType_Taxname);
    field_types.push_back(CFieldNamePanel::eFieldType_Source);
    field_types.push_back(CFieldNamePanel::eFieldType_Misc);
    field_types.push_back(CFieldNamePanel::eFieldType_Feature);
    field_types.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
    field_types.push_back(CFieldNamePanel::eFieldType_RNA);
    field_types.push_back(CFieldNamePanel::eFieldType_MolInfo);
    field_types.push_back(CFieldNamePanel::eFieldType_Pub);
    field_types.push_back(CFieldNamePanel::eFieldType_DBLink);
    field_types.push_back(CFieldNamePanel::eFieldType_SeqId);
    m_FieldType = new CFieldChoicePanel( itemCBulkCmdDlg1, field_types, true, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_FieldType, 1, wxGROW|wxALL|wxFIXED_MINSIZE, 0);
    m_FieldType->SetFieldType(CFieldNamePanel::eFieldType_Source);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 0);

    wxRadioButton* itemRadioButton1 = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Descriptors and Features"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    itemRadioButton1->SetValue(true);
    itemBoxSizer6->Add(itemRadioButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RadioDesc = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Descriptors only"));
    m_RadioDesc->SetValue(false);
    itemBoxSizer6->Add(m_RadioDesc, 0, wxALIGN_CENTER_VERTICAL |wxALL, 5);

    m_RadioFeat = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Features only"));
    m_RadioFeat->SetValue(false);
    itemBoxSizer6->Add(m_RadioFeat, 0, wxALIGN_CENTER_VERTICAL |wxALL, 5);
 
    m_CapChangeOptions = new CCapChangePanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), wxNO_BORDER); // wxSIMPLE_BORDER
    itemBoxSizer2->Add(m_CapChangeOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_CapChangeOptions-> m_CapNochange->SetValue(false);
    m_CapChangeOptions->m_CapTolower->SetValue(true);

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CLowerQuals content construction
}

/*!
 * Should we show tooltips?
 */

bool CLowerQuals::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLowerQuals::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLowerQuals bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLowerQuals bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CLowerQuals::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLowerQuals icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLowerQuals icon retrieval
}


CRef<CCmdComposite> CLowerQuals::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);
    if (!m_CapChangeOptions || !m_FieldType) 
        return cmd;
    
    CRef<edit::CStringConstraint> string_constraint(NULL);
    string constraint_field = m_FieldType->GetFieldName(false);
    ECapChange cap_change = m_CapChangeOptions->GetCapitalizationRequest();

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(constraint_field));

    CSeq_entry_Handle seh = m_TopSeqEntry;
    CSeq_entry_Handle null_seh;
    vector<CRef<edit::CApplyObject> > objs = col->GetApplyObjects(seh, constraint_field, string_constraint);
    if (objs.empty()) 
    {
        wxMessageBox(wxT("No features found!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return cmd;
    }
  
    cmd.Reset(new CCmdComposite("Lowercase Qualifiers"));
    NON_CONST_ITERATE(vector<CRef<edit::CApplyObject> >, src_it, objs) 
    {
        if (!(*src_it)->PreExists()) 
        {
            continue;
        }     
        string val = col->GetVal((*src_it)->GetObject());
        string new_val = val;
        const CSeqdesc * desc = dynamic_cast<const CSeqdesc *>((*src_it)->GetOriginalObject());
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat *>((*src_it)->GetOriginalObject());
        bool is_src_org = false;
        if (feat && feat->IsSetData() && feat->GetData().IsBiosrc() && feat->GetData().GetBiosrc().IsSetOrg())
            is_src_org = true;
        if (desc && desc->IsSource() && desc->GetSource().IsSetOrg())
            is_src_org = true;
        if (feat && m_RadioDesc->GetValue())
            continue;
        if (desc && m_RadioFeat->GetValue())
            continue;

        if (is_src_org)
            FixCapitalizationInString(null_seh, new_val, cap_change); 
        else
            FixCapitalizationInString(seh, new_val, cap_change); 

        if (val != new_val)
        {
            col->SetVal((*src_it)->SetObject(), new_val, edit::eExistingText_replace_old);
            CRef<CCmdComposite> ecmd = GetCommandFromApplyObject(**src_it);
            if (ecmd) 
                cmd->AddCommand(*ecmd);
        }
    }

    return cmd;
}


string CLowerQuals::GetErrorMessage()
{
    return m_ErrorMessage;
}

void CLowerQuals::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this->GetSizer());
}

END_NCBI_SCOPE
