/*  $Id: convert_feat_dlg.cpp 44316 2019-11-27 16:08:06Z filippov $
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

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objects/macro/Feat_qual_legal.hpp>
#include <gui/packages/pkg_sequence_edit/convert_feat_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objtools/edit/cds_fix.hpp>

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
 * CConvertFeatDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CConvertFeatDlg, CBulkCmdDlg )


/*!
 * CConvertFeatDlg event table definition
 */

BEGIN_EVENT_TABLE( CConvertFeatDlg, CBulkCmdDlg )

////@begin CConvertFeatDlg event table entries
    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CConvertFeatDlg::ProcessUpdateFeatEvent )
////@end CConvertFeatDlg event table entries

END_EVENT_TABLE()


/*!
 * CConvertFeatDlg constructors
 */

CConvertFeatDlg::CConvertFeatDlg()
{
    Init();
}

CConvertFeatDlg::CConvertFeatDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb), m_Converter(NULL)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CConvertFeatDlg creator
 */

bool CConvertFeatDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConvertFeatDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConvertFeatDlg creation
    return true;
}


/*!
 * CConvertFeatDlg destructor
 */

CConvertFeatDlg::~CConvertFeatDlg()
{
////@begin CConvertFeatDlg destruction
////@end CConvertFeatDlg destruction
}


/*!
 * Member initialisation
 */

void CConvertFeatDlg::Init()
{
////@begin CConvertFeatDlg member initialisation
    m_FeatureChoiceSizer = NULL;
    m_FeatureTypeFrom = NULL;
    m_FeatureTypeTo = NULL;
    m_FunctionDescriptionTxt = NULL;
    m_OptionsSizer = NULL;
    m_LeaveOriginal = NULL;
    m_AddUnverified = NULL;
    m_Constraint = NULL;
    m_OkCancel = NULL;
////@end CConvertFeatDlg member initialisation
}


/*!
 * Control creation for CConvertFeatDlg
 */

void CConvertFeatDlg::CreateControls()
{    
////@begin CConvertFeatDlg content construction
    CConvertFeatDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_FeatureChoiceSizer = new wxFlexGridSizer(0, 3, 0, 0);
    itemBoxSizer2->Add(m_FeatureChoiceSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FeatureChoiceSizer->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FeatureChoiceSizer->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Conversion Function"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FeatureChoiceSizer->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FeatureTypeFrom = new CFeatureTypePanel( itemCBulkCmdDlg1, ID_CF_FEATURETYPEFROM, wxDefaultPosition, wxSize(100, 100), 0 );
    m_FeatureChoiceSizer->Add(m_FeatureTypeFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FeatureTypeTo = new CFeatureTypePanel( itemCBulkCmdDlg1, ID_CF_FEATURETYPETO, wxDefaultPosition, wxSize(100, 100), 0 );
    m_FeatureChoiceSizer->Add(m_FeatureTypeTo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    GetTopLevelSeqEntryAndProcessor();
    m_FeatureTypeFrom->SetListAllFeats(true);
    m_FeatureTypeFrom->ListPresentFeaturesFirst(m_TopSeqEntry);
    m_FeatureTypeTo->ListPresentFeaturesFirst(m_TopSeqEntry); 

    m_FunctionDescriptionTxt = new wxTextCtrl( itemCBulkCmdDlg1, ID_CF_FUNCTION_DESC, wxEmptyString, wxDefaultPosition, wxSize(180, -1), wxTE_MULTILINE|wxTE_READONLY );
    m_FeatureChoiceSizer->Add(m_FunctionDescriptionTxt, 0, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxALL, 5);

    m_OptionsSizer = new wxFlexGridSizer(0, 3, 0, 0);
    itemBoxSizer2->Add(m_OptionsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_LeaveOriginal = new wxCheckBox( itemCBulkCmdDlg1, ID_CF_LEAVE_ORIGINAL, _("Leave original feature"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveOriginal->SetValue(false);
    itemBoxSizer3->Add(m_LeaveOriginal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AddUnverified = new wxCheckBox( itemCBulkCmdDlg1, ID_CF_ADD_UNVERIFIED, _("Add unverified"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AddUnverified->SetValue(false);
    itemBoxSizer3->Add(m_AddUnverified, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
   
    m_Constraint = new CStringConstraintPanel(itemCBulkCmdDlg1, false, ID_CF_CONSTRAINT, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_CENTER_HORIZONTAL|wxFIXED_MINSIZE|wxALL, 0);

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, ID_CF_OKC, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CConvertFeatDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CConvertFeatDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CConvertFeatDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConvertFeatDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConvertFeatDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CConvertFeatDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConvertFeatDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConvertFeatDlg icon retrieval
}


CRef<CCmdComposite> CConvertFeatDlg::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);

    if (!m_Converter || NStr::Equal(m_Converter->GetDescription(), kConversionNotSupported)) {
        m_ErrorMessage = "You must select a supported conversion function.";
        return cmd;
    }

    x_SetOptions();

    string feat_type = m_FeatureTypeFrom->GetFieldName();
    if (NStr::EqualNocase(feat_type, "any")) {
        feat_type = "";
    }
    string field_name = "";
    if (NStr::IsBlank(feat_type)) {
        field_name = kPartialStart;
    } else {
        field_name = feat_type + " " + kPartialStart;
    }

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name));


    CRef<edit::CStringConstraint> string_constraint = m_Constraint->GetStringConstraint();
    set<string> qual_list = GetQualList();

    vector<CConstRef<CObject> > objs;

    TConstScopedObjects sel_objs = GetSelectedObjects();
    bool ignore_selected = true;
    if (!sel_objs.empty()) {
        bool any_feat = false;
        vector<CConstRef<CObject> > sel_no_match;
        NON_CONST_ITERATE (TConstScopedObjects, it, sel_objs) {
            const CSeq_feat * f = dynamic_cast<const CSeq_feat *>((*it).object.GetPointer());
            if (f) {
                any_feat = true;
                if (m_Converter->CanConvertFrom(f->GetData().GetSubtype())) {
                    if (DoesObjectAllQualsMatchFieldConstraint (*((*it).object), qual_list, string_constraint, (*it).scope)) {               
                        objs.push_back(it->object);
                    } else {
                        sel_no_match.push_back(it->object);
                    }
                }
            }
        }
        if (objs.empty() && any_feat) {
            if (!sel_no_match.empty()) {
                int answer = wxMessageBox(wxT("Selected features do not match constraint - apply to selected feature anyway?"),
                                          wxT("Error"), wxYES_NO | wxICON_QUESTION, this);
                if (answer == wxYES) {
                    objs = sel_no_match;
                }
            } else {
                int answer = wxMessageBox(wxT("No selected features are of the correct type. Search all features?"),
                                          wxT("Error"), wxYES_NO | wxICON_QUESTION, this);
                if (answer == wxNO) {
                    m_ErrorMessage = "No features found!";
                    return cmd;
                }
            }
        }
    }

    CRef<CScope> scope(&m_TopSeqEntry.GetScope());

    if (objs.empty()) {
        CBioseq_CI bi (m_TopSeqEntry, objects::CSeq_inst::eMol_na);
        while (bi) {
            vector<CConstRef<CObject> > these_objs = col->GetObjects(*bi);
            ITERATE (vector<CConstRef<CObject> >, it, these_objs) {
                if (DoesObjectAllQualsMatchFieldConstraint (**it, qual_list, string_constraint, scope)) {
                    objs.push_back (*it);
                }
            }
            ++bi;
        }
    }

    if (objs.size() == 0) {
        m_ErrorMessage = "No features found!";
        return cmd;
    }

    bool any_change = false;
    set<CSeq_entry_Handle> already_done;
    cmd.Reset(new CCmdComposite("Convert Features"));
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    m_Converter->SetCreateGeneralOnly(create_general_only);
    map<CBioseq_Handle, int> bioseq_to_offset;
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CBioseq_Handle bsh;
        if (f->IsSetLocation())
        {
            bsh = scope->GetBioseqHandle(f->GetLocation());
            if (bioseq_to_offset.find(bsh) == bioseq_to_offset.end())
                bioseq_to_offset[bsh] = 1;
            m_Converter->SetOffset(&bioseq_to_offset[bsh]);
        }
        CRef<CCmdComposite> subcmd = m_Converter->Convert(*f, m_LeaveOriginal->GetValue(), *scope);
        if (subcmd) {
            cmd->AddCommand(*subcmd);
            if (bsh && m_AddUnverified->GetValue())
                x_AddUnverified(bsh, cmd, already_done);
            any_change = true;            
        }
    }
    if (!any_change) {
        m_ErrorMessage = "All conversions failed.";
        cmd.Reset(NULL);
    }
    return cmd;
}

void CConvertFeatDlg::x_AddUnverified(CBioseq_Handle bsh, CRef<CCmdComposite> cmd,  set<CSeq_entry_Handle> &already_done)
{
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    if (seh  && already_done.find(seh) == already_done.end())
    {
        already_done.insert(seh);
        CRef<CUser_object> user(new CUser_object());
        user->SetObjectType(CUser_object::eObjectType_Unverified);
        user->AddUnverifiedFeature();
        CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
        new_desc->SetUser(*user);
        cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *new_desc)) );
    }    
}

set<string> CConvertFeatDlg::GetQualList()
{
    set<string> res;
    CEnumeratedTypeValues::TValues qual_names  = objects::ENUM_METHOD_NAME(EFeat_qual_legal)()->GetValues();
    for (CEnumeratedTypeValues::TValues::const_iterator i = qual_names.begin(); i != qual_names.end(); ++i)
    {
        string qual_name = i->first;
        NStr::ReplaceInPlace(qual_name,"-","_");
        if (NStr::EqualNocase(qual_name, "gene")) {
            qual_name = "locus";
        }
        
        res.insert(qual_name);
    }
    res.insert("product");
    res.insert("note");
    return res;
}

bool CConvertFeatDlg::DoesObjectAllQualsMatchFieldConstraint (const CObject& object, const set<string>& qual_list, CRef<edit::CStringConstraint> string_constraint, CRef<CScope> scope)
{
    if (!string_constraint) 
        return true;

    bool negation = string_constraint->GetNegation();
    bool res = negation;
    string feat_type = m_FeatureTypeFrom->GetFieldName();
    for (set<string>::const_iterator qi = qual_list.begin(); qi != qual_list.end(); ++qi)
    {
        string field_name = feat_type + " " + *qi;
        CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name)); 
        if (!col) {
            return false;
        }

        vector<string> val_list; 
        vector<CConstRef<CObject> > objs = col->GetRelatedObjects (object, scope);
        ITERATE(vector<CConstRef<CObject> >, it, objs) 
        {
            vector<string> add = col->GetVals(**it);
            val_list.insert(val_list.end(), add.begin(), add.end());
        }
        if (!val_list.empty())
        {
            if (negation)
                res = res && string_constraint->DoesListMatch(val_list);
            else
                res = res || string_constraint->DoesListMatch(val_list);
        }
    }
    return res;
}

string CConvertFeatDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}


static CSeqFeatData::ESubtype s_GetFeatureSubtypeFromString(const string& key)
{
    CSeqFeatData::ESubtype subtype;
    if (NStr::EqualNocase(key, "gene")) {
        subtype = CSeqFeatData::eSubtype_gene;
    } else if (NStr::EqualNocase(key, "site")) {
        subtype = CSeqFeatData::eSubtype_site;
    } else if (NStr::EqualNocase(key, "region")) {
        subtype = CSeqFeatData::eSubtype_region;
    } else if (NStr::EqualNocase(key, "sno_RNA")) {
        subtype = CSeqFeatData::eSubtype_snoRNA;
    } else if (NStr::EqualNocase(key, "preprotein")) {
        subtype = CSeqFeatData::eSubtype_preprotein;
    } else if (NStr::EqualNocase(key, "Mature Peptide AA")) {
        subtype = CSeqFeatData::eSubtype_mat_peptide_aa;
    } else if (NStr::EqualNocase(key, "Signal Peptide AA")) {
        subtype = CSeqFeatData::eSubtype_sig_peptide_aa;
    } else if (NStr::EqualNocase(key, "Transit Peptide AA")) {
        subtype = CSeqFeatData::eSubtype_transit_peptide_aa;
    } else {
        subtype = CSeqFeatData::SubtypeNameToValue(key);
    }
    return subtype;
}


void CConvertFeatDlg::UpdateEditor()
{
    string feature_from = m_FeatureTypeFrom->GetFieldName();
    string feature_to = m_FeatureTypeTo->GetFieldName();
  
    if (NStr::IsBlank(feature_from) || NStr::IsBlank(feature_to)) {
    } else {
        CSeqFeatData::ESubtype from_subtype = s_GetFeatureSubtypeFromString(feature_from);
        CSeqFeatData::ESubtype to_subtype = s_GetFeatureSubtypeFromString(feature_to);

        m_Converter = CConvertFeatureBaseFactory::Create(from_subtype, to_subtype);
        m_FunctionDescriptionTxt->SetValue(m_Converter->GetDescription());
    }
    x_UpdateOptions();
}

void CConvertFeatDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this->GetSizer());
}

void CConvertFeatDlg::x_UpdateOptions()
{
    while (m_OptionsSizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_OptionsSizer->GetItem(pos)->DeleteWindows();
        m_OptionsSizer->Remove(pos);
    }

    if (m_Converter) {
        int col = 0;
        int num_cols = m_OptionsSizer->GetCols();
        ITERATE(CConvertFeatureBase::TOptions, it, m_Converter->GetOptions()) {
            if ((*it)->IsBool()) {
                wxCheckBox* opt = new wxCheckBox( this, wxID_ANY, ToWxString((*it)->GetLabel()), wxDefaultPosition, wxDefaultSize, 0 );
                m_OptionsSizer->Add(opt, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
                opt->SetValue((*it)->GetBool()->GetDefaultVal());
                col++;
            } else if ((*it)->IsString()) {
                int remaining = num_cols - col;
                if (remaining < 2) {
                    for (int i = 0; i < remaining; i++) {
                        // add spacers
                        m_OptionsSizer->Add(5, 15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
                    }
                }

                wxStaticText* label = new wxStaticText( this, wxID_STATIC, ToWxString((*it)->GetLabel()), wxDefaultPosition, wxDefaultSize, 0 );
                m_OptionsSizer->Add(label, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
                const vector<string>& suggested_values = (*it)->GetString()->GetSuggestedValues();
                if (suggested_values.size() == 0) {
                    wxTextCtrl* val = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
                    m_OptionsSizer->Add(val, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
                    val->SetValue((*it)->GetString()->GetDefaultVal());
                } else if ((*it)->GetString()->GetOnlySuggested()) {
                    wxArrayString choice_strings;
                    ITERATE(vector<string>, sit, suggested_values) {
                        choice_strings.Add(ToWxString(*sit));
                    }
                    wxChoice* val = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
                    m_OptionsSizer->Add(val, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
                    val->SetStringSelection((*it)->GetString()->GetDefaultVal());
                } else {
                    wxArrayString choice_strings;
                    ITERATE(vector<string>, sit, suggested_values) {
                        choice_strings.Add(ToWxString(*sit));
                    }
                    wxComboBox* val = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
                    m_OptionsSizer->Add(val, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
                    val->SetStringSelection((*it)->GetString()->GetDefaultVal());
                }
                col += 2;
            }
            col = col % num_cols;
        }
    }
    m_OptionsSizer->Layout();
    x_FixLayout();
}


void CConvertFeatDlg::x_FixLayout()
{
    Layout();
    Fit();
    Refresh();
}


void CConvertFeatDlg::x_SetOptions()
{
    if (!m_Converter) {
        return;
    }
    size_t option_pos = 0;
    CConvertFeatureBase::TOptions& options = m_Converter->SetOptions();
    for (size_t pos = 0; pos < m_OptionsSizer->GetItemCount() && option_pos < options.size(); pos++) {
        wxWindow* w = m_OptionsSizer->GetItem(pos)->GetWindow();
        wxCheckBox* checkbox = dynamic_cast<wxCheckBox*>(w);
        if (checkbox) {
            if (options[option_pos]->IsBool()) {
                options[option_pos]->SetBool()->SetVal(checkbox->GetValue());
            }
            option_pos++;
        } else {
            wxTextCtrl* val = dynamic_cast<wxTextCtrl*>(w);
            if (val) {
                if (options[option_pos]->IsString()) {
                    options[option_pos]->SetString()->SetVal(ToStdString(val->GetValue()));
                }
                option_pos++;
            } else {
                wxComboBox* combo = dynamic_cast<wxComboBox*>(w);
                if (combo) {
                    if (options[option_pos]->IsString()) {
                        options[option_pos]->SetString()->SetVal(ToStdString(combo->GetStringSelection()));
                    }
                    option_pos++;
                } else {
                    wxChoice* choice = dynamic_cast<wxChoice*>(w);
                    if (choice) {
                        if (options[option_pos]->IsString()) {
                            options[option_pos]->SetString()->SetVal(ToStdString(choice->GetStringSelection()));
                        }
                        option_pos++;
                    }
                }
            }
        }

    }
}


END_NCBI_SCOPE
