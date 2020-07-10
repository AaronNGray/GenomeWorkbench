/*  $Id: convert_cds_to_misc_feat.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objmgr/bioseq_handle.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/convert_cds_to_misc_feat.hpp>

#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/msgdlg.h>
#include <wx/statbox.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CConvertCdsToMiscFeat type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CConvertCdsToMiscFeat, CBulkCmdDlg )


/*!
 * CConvertCdsToMiscFeat event table definition
 */

BEGIN_EVENT_TABLE( CConvertCdsToMiscFeat, CBulkCmdDlg )

////@begin CConvertCdsToMiscFeat event table entries
    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CConvertCdsToMiscFeat::ProcessUpdateFeatEvent )
////@end CConvertCdsToMiscFeat event table entries
    EVT_RADIOBUTTON(ID_CDS_TO_MISCFEAT_RADIOBUTTON, CConvertCdsToMiscFeat::OnChoiceChanged)
    EVT_RADIOBUTTON(ID_CDS_TO_MISCFEAT_RADIOBUTTON1, CConvertCdsToMiscFeat::OnChoiceChanged)
    EVT_RADIOBUTTON(ID_CDS_TO_MISCFEAT_RADIO_ALL, CConvertCdsToMiscFeat::OnConditionChanged)
    EVT_RADIOBUTTON(ID_CDS_TO_MISCFEAT_RADIO_WHERE, CConvertCdsToMiscFeat::OnConditionChanged)
END_EVENT_TABLE()


/*!
 * CConvertCdsToMiscFeat constructors
 */

CConvertCdsToMiscFeat::CConvertCdsToMiscFeat()
{
    Init();
}

CConvertCdsToMiscFeat::CConvertCdsToMiscFeat( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb), m_Converter(NULL)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CConvertCdsToMiscFeat creator
 */

bool CConvertCdsToMiscFeat::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConvertCdsToMiscFeat creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConvertCdsToMiscFeat creation
    return true;
}


/*!
 * CConvertCdsToMiscFeat destructor
 */

CConvertCdsToMiscFeat::~CConvertCdsToMiscFeat()
{
////@begin CConvertCdsToMiscFeat destruction
////@end CConvertCdsToMiscFeat destruction
}


/*!
 * Member initialisation
 */

void CConvertCdsToMiscFeat::Init()
{
////@begin CConvertCdsToMiscFeat member initialisation
    m_OptionsSizer = NULL;
    m_LeaveOriginal = NULL;
    m_Constraint = NULL;
    m_OkCancel = NULL;
////@end CConvertCdsToMiscFeat member initialisation
}


/*!
 * Control creation for CConvertCdsToMiscFeat
 */

void CConvertCdsToMiscFeat::CreateControls()
{    
////@begin CConvertCdsToMiscFeat content construction
    CConvertCdsToMiscFeat* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_RadioButton = new wxRadioButton( itemCBulkCmdDlg1, ID_CDS_TO_MISCFEAT_RADIOBUTTON, _("misc_feat"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton->SetValue(true);
    itemBoxSizer10->Add(m_RadioButton, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton9 = new wxRadioButton( itemCBulkCmdDlg1, ID_CDS_TO_MISCFEAT_RADIOBUTTON1, _("pseudogene"));
    itemRadioButton9->SetValue(false);
    itemBoxSizer10->Add(itemRadioButton9, 0, wxALIGN_LEFT|wxALL, 5);

    m_OptionsSizer = new wxFlexGridSizer(0, 3, 0, 0);
    itemBoxSizer2->Add(m_OptionsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    
    m_Converter.Reset(new CConvertCDSToMiscFeat()); 

    if (m_Converter) 
    {
        ITERATE(CConvertFeatureBase::TOptions, it, m_Converter->GetOptions()) 
        {
            if ((*it)->IsBool()) 
            {
                wxCheckBox* opt = new wxCheckBox( this, wxID_ANY, ToWxString((*it)->GetLabel()), wxDefaultPosition, wxDefaultSize, 0 );
                m_OptionsSizer->Add(opt, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
                opt->SetValue((*it)->GetBool()->GetDefaultVal());
            } 
        }
    }

    m_LeaveOriginal = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Keep original CDS"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveOriginal->SetValue(false);
    itemBoxSizer2->Add(m_LeaveOriginal, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AddUnverified = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Add Unverified"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AddUnverified->SetValue(false);
    itemBoxSizer2->Add(m_AddUnverified, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);



    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemCBulkCmdDlg1, wxID_ANY, _("Conditions"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxRadioButton* itemRadioButton10 = new wxRadioButton( itemCBulkCmdDlg1, ID_CDS_TO_MISCFEAT_RADIO_ALL, _("All"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    itemRadioButton10->SetValue(true);
    itemStaticBoxSizer7->Add(itemRadioButton10, 0, wxALIGN_LEFT|wxALL, 5);

    m_RadioWhere = new wxRadioButton( itemCBulkCmdDlg1, ID_CDS_TO_MISCFEAT_RADIO_WHERE, _("Where"));
    m_RadioWhere->SetValue(false);
    itemStaticBoxSizer7->Add(m_RadioWhere, 0, wxALIGN_LEFT|wxALL, 5);

    m_InternalStop = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("CDS has internal stop codon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_InternalStop->SetValue(false);
    itemStaticBoxSizer7->Add(m_InternalStop, 0, wxALIGN_LEFT|wxALL, 5);
    m_InternalStop->Disable();

    m_BadStart = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("CDS has bad start codon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BadStart->SetValue(false);
    itemStaticBoxSizer7->Add(m_BadStart, 0, wxALIGN_LEFT|wxALL, 5);
    m_BadStart->Disable();

    m_BadStop = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("CDS has bad stop codon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BadStop->SetValue(false);
    itemStaticBoxSizer7->Add(m_BadStop, 0, wxALIGN_LEFT|wxALL, 5);
    m_BadStop->Disable();

    m_IsPseudo = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("CDS is pseudo"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IsPseudo->SetValue(false);
    itemStaticBoxSizer7->Add(m_IsPseudo, 0, wxALIGN_LEFT|wxALL, 5);
    m_IsPseudo->Disable();

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_RadioOr = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Or"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_RadioOr->SetValue(true);
    itemBoxSizer8->Add(m_RadioOr, 0, wxALIGN_LEFT|wxALL, 5);
    m_RadioOr->Disable();

    m_RadioAnd = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("And"));
    m_RadioAnd->SetValue(false);
    itemBoxSizer8->Add(m_RadioAnd, 0, wxALIGN_LEFT|wxALL, 5);
    m_RadioAnd->Disable();

    wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemCBulkCmdDlg1, wxID_ANY, _("Comment Format"));
    wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_RadioSimilar = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("similar to X"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_RadioSimilar->SetValue(true);
    itemStaticBoxSizer8->Add(m_RadioSimilar, 0, wxALIGN_LEFT|wxALL, 5);

    m_RadioNonfunctional = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("nonfunctional X due to mutation"));
    m_RadioNonfunctional->SetValue(false);
    itemStaticBoxSizer8->Add(m_RadioNonfunctional, 0, wxALIGN_LEFT|wxALL, 5);

    GetTopLevelSeqEntryAndProcessor();
    m_Constraint = new CConstraintPanel( itemCBulkCmdDlg1, m_TopSeqEntry);
    itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Constraint->SetSelection(10);
    m_Constraint->GetStringConstraintPanel()->SetStringSelection(_("Is one of"));

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CConvertCdsToMiscFeat content construction
}

void CConvertCdsToMiscFeat::OnChoiceChanged( wxCommandEvent& event )
{
    if (m_RadioButton->GetValue())
        m_Converter.Reset(new CConvertCDSToMiscFeat());
    else
        m_Converter.Reset(new CConvertCDSToPseudoGene());
}

void CConvertCdsToMiscFeat::OnConditionChanged( wxCommandEvent& event )
{
    if (m_RadioWhere->GetValue())
    {
        m_InternalStop->Enable();
        m_BadStart->Enable();
        m_BadStop->Enable();
        m_IsPseudo->Enable();
        m_RadioOr->Enable();
        m_RadioAnd->Enable();
    }
    else
    {
        m_InternalStop->Disable();
        m_BadStart->Disable();
        m_BadStop->Disable();
        m_IsPseudo->Disable();
        m_RadioOr->Disable();
        m_RadioAnd->Disable();
    }
}

/*!
 * Should we show tooltips?
 */

bool CConvertCdsToMiscFeat::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CConvertCdsToMiscFeat::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConvertCdsToMiscFeat bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConvertCdsToMiscFeat bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CConvertCdsToMiscFeat::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConvertCdsToMiscFeat icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConvertCdsToMiscFeat icon retrieval
}


CRef<CCmdComposite> CConvertCdsToMiscFeat::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);

    if (!m_Converter || NStr::Equal(m_Converter->GetDescription(), kConversionNotSupported)) {
        m_ErrorMessage = "You must select a supported conversion function.";
        return cmd;
    }

    x_SetOptions();

    string field_name = "CDS " + kPartialStart;
    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name));


    CFieldNamePanel* constraint_field_panel = m_Constraint->GetFieldNamePanel();
    string constraint_field;
    string constraint_field_type = m_Constraint->GetFieldType();
    if (constraint_field_panel)
        constraint_field = constraint_field_panel->GetFieldName();
    
    if (NStr::IsBlank(constraint_field)) {
        constraint_field = constraint_field_type;
    } else {
        if (NStr::StartsWith(constraint_field_type, "RNA") && NStr::Find(constraint_field, "RNA") == NPOS ) {
            constraint_field = "RNA " + constraint_field;
        }
    }
    CRef<edit::CStringConstraint> string_constraint = m_Constraint->GetStringConstraint();

    vector<CConstRef<CObject> > objs;

    objs = col->GetObjects(m_TopSeqEntry, constraint_field, string_constraint);

    if (objs.empty()) 
    {
        m_ErrorMessage = "No features found!";
        return cmd;
    }
    
    bool any_change = false;
    set<CSeq_entry_Handle> already_done;

    cmd.Reset(new CCmdComposite("Convert CDS to misc_feat or pseudogene"));
    ITERATE(vector<CConstRef<CObject> >, it, objs) 
    {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());

        if (f->IsSetExcept_text() && NStr::Find(f->GetExcept_text(), "RNA editing") != string::npos) 
        {
            continue;
        }

        if ( m_RadioWhere->GetValue() )
        {
            bool is_pseudo =  f->IsSetPseudo() && f->GetPseudo();
            bool internal_stop, bad_start, bad_stop;
            DetectStartStopCodons(f, internal_stop, bad_start, bad_stop);
            bool go = false;
            if (m_RadioOr->GetValue())
            {
                go = ( m_InternalStop->GetValue() && internal_stop ) ||
                    (  m_BadStart->GetValue() && bad_start         ) ||
                    (  m_BadStop->GetValue()  && bad_stop          ) ||
                    (  m_IsPseudo->GetValue() && is_pseudo         );
            }
            else
            {
                go = ( !m_InternalStop->GetValue() || internal_stop ) &&
                    (  !m_BadStart->GetValue() || bad_start         ) &&
                    (  !m_BadStop->GetValue()  || bad_stop          ) &&
                    (  !m_IsPseudo->GetValue() || is_pseudo         );
            }
            if (!go || ( !m_InternalStop->GetValue() && !m_BadStart->GetValue() &&  !m_BadStop->GetValue() &&  !m_IsPseudo->GetValue() ))
                continue;
        }
               
        if (m_AddUnverified->GetValue())
        {
            CScope &scope = m_TopSeqEntry.GetScope();
            if (f && f->IsSetLocation())
            {
                CBioseq_Handle bsh = scope.GetBioseqHandle(f->GetLocation());
                if (bsh)
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
            }
        }
        CConvertCDSToMiscFeat *conv_miscfeat = dynamic_cast<CConvertCDSToMiscFeat *>(m_Converter.GetPointer());
        if (conv_miscfeat)
        {
            conv_miscfeat->SetSimilar(m_RadioSimilar->GetValue());
            conv_miscfeat->SetNonfunctional(m_RadioNonfunctional->GetValue());
            CRef<CConversionOption> opt = conv_miscfeat->FindRemoveTranscriptId();
            if (opt) {
                opt->SetBool()->SetVal(true);
            }
        }
        CRef<CCmdComposite> subcmd = m_Converter->Convert(*f, m_LeaveOriginal->GetValue(), m_TopSeqEntry.GetScope());
        if (subcmd) 
        {
            cmd->AddCommand(*subcmd);
            any_change = true;            
        }       
    }

    if (!any_change) 
    {
        m_ErrorMessage = "All conversions failed.";
        cmd.Reset(NULL);
    }
    return cmd;
}

void CConvertCdsToMiscFeat::DetectStartStopCodons(const CSeq_feat* f, bool &stop_codon_present, bool &bad_start_codon, bool &bad_stop_codon)
{
    stop_codon_present = false;
    bad_start_codon = false;
    bad_stop_codon = false;
    if (f->GetData().IsCdregion())
    {
        string prot_seq;
        bool altst;
        CSeqTranslator::Translate(*f, m_TopSeqEntry.GetScope(), prot_seq, true, false, &altst);
        size_t pos = NStr::Find(prot_seq,"*");
        if (pos != NPOS && pos < prot_seq.length()-1)
            stop_codon_present = true;
        bool partial5 = f->GetLocation().IsPartialStart(eExtreme_Biological);
        if (!partial5 && prot_seq[0] != 'M')
            bad_start_codon = true;
        bool partial3 = f->GetLocation().IsPartialStop(eExtreme_Biological);
        if (!partial3 && prot_seq[prot_seq.length()-1] != '*')
            bad_stop_codon = true;
    }
}


string CConvertCdsToMiscFeat::GetErrorMessage()
{
    return m_ErrorMessage;
}

void CConvertCdsToMiscFeat::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this->GetSizer());
}

void CConvertCdsToMiscFeat::x_SetOptions()
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


void CConvertCdsToMiscFeat::ApplyToAllAndDie(bool similar, bool nonfunctional, bool unverified)
{
    m_Converter.Reset(new CConvertCDSToMiscFeat());
    m_RadioWhere->SetValue(true);
    m_RadioOr->SetValue(true);
    m_InternalStop->SetValue(true);
    m_BadStart->SetValue(false);
    m_BadStop->SetValue(false);
    m_IsPseudo->SetValue(false);
    m_RadioSimilar->SetValue(similar);
    m_RadioNonfunctional->SetValue(nonfunctional);
    m_AddUnverified->SetValue(unverified);

    GetTopLevelSeqEntryAndProcessor();
    CRef<CCmdComposite> cmd = GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);            
    }
    Destroy();
}


END_NCBI_SCOPE
