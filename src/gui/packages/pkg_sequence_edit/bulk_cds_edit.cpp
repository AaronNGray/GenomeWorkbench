/*  $Id: bulk_cds_edit.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqtable/seq_table_exception.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/existingtextdlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cds_edit.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_features.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);



IMPLEMENT_DYNAMIC_CLASS( CBulkCDS, wxDialog )

BEGIN_EVENT_TABLE( CBulkCDS, wxDialog )

    EVT_BUTTON( wxID_OK, CBulkCDS::OnClickOk )

    EVT_BUTTON( wxID_CANCEL, CBulkCDS::OnClickCancel )


END_EVENT_TABLE()

CBulkCDS::CBulkCDS()
{
    Init();
}

CBulkCDS::CBulkCDS( wxWindow* parent, objects::CSeq_entry_Handle seh, IWorkbench* workbench, CConstRef<objects::CSeq_submit> seqSubmit,
                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    this->m_Workbench = workbench;
    this->m_SeqSubmit = seqSubmit;

    CFeat_CI fi(seh, CSeqFeatData::eSubtype_cdregion);           
    for ( ; fi ; ++fi ) 
    {
        m_FeatHandles.push_back(fi->GetSeq_feat_Handle());
    }
    Init();
    Create(parent, id, caption, pos, size, style);
}

CBulkCDS::CBulkCDS( wxWindow* parent, const vector<CSeq_feat_Handle> &feat_handles, IWorkbench* workbench,
                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_FeatHandles(feat_handles)
{
    this->m_Workbench = workbench;
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CBulkCDS::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    SetLayoutAdaptationMode(wxDIALOG_ADAPTATION_MODE_ENABLED);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}


CBulkCDS::~CBulkCDS()
{
}


/*!
 * Member initialisation
 */

void CBulkCDS::Init()
{
    m_Grid=NULL;
}


void CBulkCDS::CreateControls()
{    
    CBulkCDS* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;

    if (m_FeatHandles.empty()) 
    {
        wxMessageBox(wxT("No CDS records found"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No CDS records found" );
    }

    CRef<objects::CSeq_table> values_table = GetValuesTableFromSeqEntry();
    if (values_table->GetNum_rows() < 1)
    {
        wxMessageBox(wxT("No CDS records found"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No CDS records found" );
    }
    CRef<CSeq_table> choices = GetChoices(values_table);
    int glyph_col =   GetCollapsible();
    m_GridPanel = new CSeqTableGridPanel(this, values_table, choices, glyph_col);
    itemBoxSizer3->Add(m_GridPanel, 0, wxALIGN_TOP|wxALL, 5);
    m_Grid = m_GridPanel->GetGrid();
    CSeqTableGrid *gridAdapter = new CSeqTableGrid(values_table);
    m_Grid->SetTable(gridAdapter, true);
    m_Grid->AutoSizeColumns();
    int l_height = m_Grid->GetColLabelSize();
    m_Grid->SetColLabelSize( 2 * l_height );
    
    int pos = 0;
    ITERATE (CSeq_table::TColumns, it, values_table->GetColumns()) 
    {
        if (pos > 0) 
        {
            if ((*it)->IsSetHeader() && (*it)->GetHeader().IsSetTitle() )
            {
                string title = (*it)->GetHeader().GetTitle();
                if (!title.empty())
                {
                    itemChoiceStrings.Add(wxString(title));
                    if (!IsReadOnlyColumn(title)) 
                        itemChoiceStringsWritable.Add(wxString(title));
                }
                if (IsReadOnlyColumn(title)) 
                    m_GridPanel->MakeColumnReadOnly(pos - 1, true);
            } 
        }
        pos++;
    }
    
    if (glyph_col >= 0 && glyph_col+1 < m_Grid->GetNumberCols())
    {
        m_GridPanel->InitColumnCollapse(glyph_col+1);  
    }
  

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintSelect( itemDialog1, m_GridPanel, itemChoiceStrings, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AecrPanel = new CApplyEditconvertPanel( itemDialog1, m_GridPanel, itemChoiceStringsWritable, 0, true, wxID_ANY, wxDefaultPosition, wxSize(1128, 219));
    itemBoxSizer5->Add(m_AecrPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 1);

    m_CopyProtNameToComment = new wxCheckBox( itemDialog1, wxID_ANY, _("Save old protein name in CDS comment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_CopyProtNameToComment, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_CopyProtNameToComment->SetValue(false);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

CRef<objects::CSeq_table> CBulkCDS::GetValuesTableFromSeqEntry() 
{

    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);

    CRef<objects::CSeqTable_column> expand_col(new objects::CSeqTable_column());
    expand_col->SetHeader().SetTitle("");
    expand_col->SetHeader().SetField_name("expand");
    expand_col->SetData().SetString();

    CRef<objects::CSeqTable_column> name_col(new objects::CSeqTable_column());
    name_col->SetHeader().SetTitle("protein name");
    name_col->SetHeader().SetField_name("protein_name");
    name_col->SetData().SetString();

    CRef<objects::CSeqTable_column> desc_col(new objects::CSeqTable_column());
    desc_col->SetHeader().SetTitle("protein description");
    desc_col->SetHeader().SetField_name("protein_description");
    desc_col->SetData().SetString();

    CRef<objects::CSeqTable_column> loc_col(new objects::CSeqTable_column());
    loc_col->SetHeader().SetTitle("location");
    loc_col->SetHeader().SetField_name("location");
    loc_col->SetData().SetString();

    CRef<objects::CSeqTable_column> comment_col(new objects::CSeqTable_column());
    comment_col->SetHeader().SetTitle("comment");
    comment_col->SetHeader().SetField_name("comment");
    comment_col->SetData().SetString();

    CRef<objects::CSeqTable_column> ec_number_col(new objects::CSeqTable_column());
    ec_number_col->SetHeader().SetTitle("ec number");
    ec_number_col->SetHeader().SetField_name("ec_number");
    ec_number_col->SetData().SetString();

    CRef<objects::CSeqTable_column> activity_col(new objects::CSeqTable_column());
    activity_col->SetHeader().SetTitle("activity");
    activity_col->SetHeader().SetField_name("activity");
    activity_col->SetData().SetString();

    // bogus column to include last, otherwise deletion of the previous column will not work
    CRef<objects::CSeqTable_column> bogus_col(new objects::CSeqTable_column());
    bogus_col->SetHeader().SetTitle("");
    bogus_col->SetHeader().SetField_name("");
    bogus_col->SetData().SetString();

    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    table->SetColumns().push_back(id_col);
    table->SetColumns().push_back(expand_col);
    table->SetColumns().push_back(name_col);
    table->SetColumns().push_back(desc_col);
    table->SetColumns().push_back(loc_col);
  
    CRef<CFeatureSeqTableColumnBase> partial_start_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStart);
    CRef<CSeqTable_column> partial_start_col = AddStringColumnToTable(table, kPartialStart);
    partial_start_col->SetHeader().SetField_name(kPartialStart);

    CRef<CFeatureSeqTableColumnBase> partial_stop_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStop);
    CRef<CSeqTable_column> partial_stop_col = AddStringColumnToTable(table, kPartialStop);
    partial_stop_col->SetHeader().SetField_name(kPartialStop);

    table->SetColumns().push_back(comment_col);
    table->SetColumns().push_back(ec_number_col);
    table->SetColumns().push_back(activity_col);
    table->SetColumns().push_back(bogus_col);

    size_t row = 0;

    CSeq_entry_Handle seh;
    CIRef<IEditingAction> ec_action = CreateAction(seh, "ec_number", CFieldNamePanel::eFieldType_Feature, CSeqFeatData::eSubtype_cdregion);
    IEditingActionFeat* ec_feat_action = dynamic_cast<IEditingActionFeat*>(ec_action.GetPointer());
    CIRef<IEditingAction> act_action = CreateAction(seh, "activity", CFieldNamePanel::eFieldType_Feature, CSeqFeatData::eSubtype_cdregion);
    IEditingActionFeat* act_feat_action = dynamic_cast<IEditingActionFeat*>(act_action.GetPointer());

    for ( vector<CSeq_feat_Handle>::iterator fi = m_FeatHandles.begin(); fi != m_FeatHandles.end() ; ++fi ) 
    {
        // Use Cdregion.Product to get handle to protein bioseq 
        if (!fi->IsSetProduct()) continue;
            CBioseq_Handle prot_bsh = fi->GetScope().GetBioseqHandle(fi->GetProduct());
            // Should be a protein!
            if (!prot_bsh || !prot_bsh.IsProtein())   continue;

            CRef<objects::CSeq_id> id(new objects::CSeq_id());
            id->SetLocal().SetId(row);
            //id->Assign (*(prot_bsh.GetSeqId()));
            string name,desc,loc,comment;

            SAnnotSelector sel(CSeqFeatData::e_Prot);
            CFeat_CI prot_feat_ci(prot_bsh, sel);
            if (prot_feat_ci)
            {
                const CSeq_feat &feat = prot_feat_ci->GetOriginalFeature();
                const CProt_ref &prot = feat.GetData().GetProt();
                if (prot.IsSetName())
                    name = prot.GetName().front();
                if (prot.IsSetDesc())
                    desc = prot.GetDesc();
                // if ( feat.IsSetLocation() )
                //   feat.GetLocation().GetLabel(&loc);
              
            }

            if ( fi->GetOriginalSeq_feat()->IsSetLocation() )
                fi->GetOriginalSeq_feat()->GetLocation().GetLabel(&loc);

            if (fi->GetOriginalSeq_feat()->IsSetComment())
                comment = fi->GetOriginalSeq_feat()->GetComment();

            seh = fi->GetScope().GetBioseqHandle(fi->GetLocation()).GetSeq_entry_Handle();
            ec_feat_action->SetTopSeqEntry(seh);
            ec_feat_action->SetFeat(*fi);
            const vector<string>& ec_values = ec_feat_action->GetValues();
            string ec_number = NStr::Join(ec_values, "|");

            act_feat_action->SetTopSeqEntry(seh);
            act_feat_action->SetFeat(*fi);
            const vector<string>& act_values = act_feat_action->GetValues();
            string activity = NStr::Join(act_values, "|");

            id_col->SetData().SetId().push_back(id);
            expand_col->SetData().SetString().push_back("");
            name_col->SetData().SetString().push_back(name);
            desc_col->SetData().SetString().push_back(desc);
            loc_col->SetData().SetString().push_back(loc);
            AddValueToColumn(partial_start_col, partial_start_feat_col->GetFromFeat(*fi->GetOriginalSeq_feat()), row);
            AddValueToColumn(partial_stop_col, partial_stop_feat_col->GetFromFeat(*fi->GetOriginalSeq_feat()), row);
            comment_col->SetData().SetString().push_back(comment);
            ec_number_col->SetData().SetString().push_back(ec_number);
            activity_col->SetData().SetString().push_back(activity);
            bogus_col->SetData().SetString().push_back("");
            row++;
    }
    
   
    table->SetNum_rows(row); 

    return table;
}


CRef<CCmdComposite> CBulkCDS::GetCommandFromValuesTable(CRef<CSeq_table> values_table) 
{
 CRef<CCmdComposite> cmd(new CCmdComposite("Bulk CDS Edit"));

 CSeq_entry_Handle seh;
 CIRef<IEditingAction> ec_action = CreateAction(seh, "ec_number", CFieldNamePanel::eFieldType_Feature, CSeqFeatData::eSubtype_cdregion);
 IEditingActionFeat* ec_feat_action = dynamic_cast<IEditingActionFeat*>(ec_action.GetPointer());
 CIRef<IEditingAction> act_action = CreateAction(seh, "activity", CFieldNamePanel::eFieldType_Feature, CSeqFeatData::eSubtype_cdregion);
 IEditingActionFeat* act_feat_action = dynamic_cast<IEditingActionFeat*>(act_action.GetPointer());

 CRef<CFeatureSeqTableColumnBase> partial_start_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStart);
 CRef<CFeatureSeqTableColumnBase> partial_stop_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStop);
 m_Modified = false;
 size_t row = 0;
 objects::edit::EExistingText existing_text = objects::edit::eExistingText_cancel;
 bool not_asked = true;
 for ( vector<CSeq_feat_Handle>::iterator fi = m_FeatHandles.begin(); fi != m_FeatHandles.end() ; ++fi ) 
     {
         if (!fi->IsSetProduct()) continue;
         CBioseq_Handle prot_bsh = fi->GetScope().GetBioseqHandle(fi->GetProduct());
         // Should be a protein!
         if (!prot_bsh || !prot_bsh.IsProtein())   continue;
            
         CRef<CSeq_feat> new_cds(new CSeq_feat());
         new_cds->Assign(*fi->GetOriginalSeq_feat());

         string name,desc,loc;
         string save_old_name;

         SAnnotSelector sel(CSeqFeatData::e_Prot);
         CFeat_CI prot_feat_ci(prot_bsh, sel);
         if (prot_feat_ci)
         {
             const CSeq_feat &feat = prot_feat_ci->GetOriginalFeature();
             const CProt_ref &prot = feat.GetData().GetProt();
             CRef<CSeq_feat> new_feat(new CSeq_feat());
             new_feat->Assign(feat);
             bool modified = false;

             if (prot.IsSetName())
                 name = prot.GetName().front();
             if (prot.IsSetDesc())
                 desc = prot.GetDesc();
             //if ( feat.IsSetLocation() )
             //    feat.GetLocation().GetLabel(&loc);
             
             string new_name;
             try
             {
                 if (row < values_table->GetColumn("protein_name").GetData().GetString().size())
                     new_name = values_table->GetColumn("protein_name").GetData().GetString()[row];
             } catch(CSeqTableException& ) {}
             
             if (new_name != name)
             {
                 if (new_name.empty())
                     new_feat->SetData().SetProt().ResetName();
                 else
                 {
                     if (new_feat->SetData().SetProt().SetName().empty())
                         new_feat->SetData().SetProt().SetName().push_back(new_name);
                     else
                         new_feat->SetData().SetProt().SetName().front() = new_name;
                 }
                 GetUpdateMRNAProductNameCmd(new_cds, fi->GetScope(), new_name, cmd);
                 modified = true;

                 if (m_CopyProtNameToComment->GetValue())
                     save_old_name = name;
             }
             

             string new_desc;
             try
             {
                 if (row <  values_table->GetColumn("protein_description").GetData().GetString().size())
                     new_desc = values_table->GetColumn("protein_description").GetData().GetString()[row];
             } catch(CSeqTableException&) {}

             if (new_desc != desc)
             {
                 if (new_desc.empty())
                     new_feat->SetData().SetProt().ResetDesc();
                 else
                     new_feat->SetData().SetProt().SetDesc() = new_desc;
                 modified = true;
             }

             seh = fi->GetScope().GetBioseqHandle(fi->GetLocation()).GetSeq_entry_Handle();
             ec_feat_action->SetTopSeqEntry(seh);
             ec_feat_action->SetFeat(*fi);
             const vector<string>& ec_values = ec_feat_action->GetValues();
             string old_ec_number = NStr::Join(ec_values, "|");
             string new_ec_number;
             try
             {
                 if (row <  values_table->GetColumn("ec_number").GetData().GetString().size())
                     new_ec_number = values_table->GetColumn("ec_number").GetData().GetString()[row];
             } catch(CSeqTableException&) {}
             if (new_ec_number != old_ec_number)
             {
                 if (new_cds->GetProtXref() != NULL)
                 {
                     new_cds->SetProtXref().ResetEc();
                 }
                 new_cds->RemoveQualifier("EC_number");
                 new_feat->RemoveQualifier("EC_number");
                 new_feat->SetData().SetProt().ResetEc();
                 vector<string> new_values;
                 NStr::Split(new_ec_number, "|", new_values);
                 for (const auto& val : new_values)
                 {
                     if (!val.empty())
                         {
                             new_feat->SetData().SetProt().SetEc().push_back(val);
                         }
                 }
                 modified = true;
             }

             act_feat_action->SetTopSeqEntry(seh);
             act_feat_action->SetFeat(*fi);
             const vector<string>& act_values = act_feat_action->GetValues();
             string old_activity = NStr::Join(act_values, "|");
             string new_activity;
             try
             {
                 if (row <  values_table->GetColumn("activity").GetData().GetString().size())
                     new_activity = values_table->GetColumn("activity").GetData().GetString()[row];
             } catch(CSeqTableException&) {}
             if (new_activity != old_activity)
             {
                 new_cds->RemoveQualifier("activity");
                 new_feat->SetData().SetProt().ResetActivity();
                 vector<string> new_values;
                 NStr::Split(new_activity, "|", new_values);
                 for (const auto& val : new_values)
                 {
                     if (!val.empty())
                         {
                             new_feat->SetData().SetProt().SetActivity().push_back(val);
                         }
                 }
                 modified = true;
             }


             if (modified)
                 cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(*prot_feat_ci,*new_feat)));
             m_Modified |= modified;
         }

         if ( fi->GetOriginalSeq_feat()->IsSetLocation() )
             fi->GetOriginalSeq_feat()->GetLocation().GetLabel(&loc);

         bool changed_feat = false;

         string comment;
         if (new_cds->IsSetComment())
             comment = new_cds->GetComment();

         string new_comment;
         try
         {
             if (row <  values_table->GetColumn("comment").GetData().GetString().size())
                 new_comment = values_table->GetColumn("comment").GetData().GetString()[row];
         } catch(CSeqTableException&) {}
         
         if (!save_old_name.empty())
         {
             if (new_comment.empty())
                 new_comment = save_old_name;
             else
             {
                 if (not_asked)
                 {
                     string msg = "Do you want to overwrite existing comment values?";
                     int answer = wxMessageBox(ToWxString(msg), wxT("Existing Text"), wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
                     if (answer == wxCANCEL) 
                     {
                         existing_text = objects::edit::eExistingText_cancel;
                     } 
                     else if (answer == wxYES) 
                     {
                         existing_text = objects::edit::eExistingText_replace_old;
                     } 
                     else 
                     {
                         CExistingTextDlg dlg(this);
                         dlg.AllowAdditionalQual(false); 
                         if (dlg.ShowModal() == wxID_OK) {
                             existing_text = dlg.GetExistingTextHandler();
                         } else {
                             existing_text = objects::edit::eExistingText_cancel;
                         }
                     }
                     not_asked = false;
                 }
                 AddValueToString(new_comment, save_old_name, existing_text);  
                 //new_comment += "; " + save_old_name;
             }
         }

         if (new_comment != comment)
         {
             if (new_comment.empty())
                 new_cds->ResetComment();
             else
                 new_cds->SetComment() = new_comment;
             changed_feat = true;
         }        


         string partial_start_val = partial_start_feat_col->GetFromFeat(*fi->GetOriginalSeq_feat());
         string new_partial_start_val;
         try
         {
             if (row <  values_table->GetColumn(kPartialStart).GetData().GetString().size())
                 new_partial_start_val = values_table->GetColumn(kPartialStart).GetData().GetString()[row];
         } catch(CSeqTableException&) {}

         if (new_partial_start_val != partial_start_val)
         {
             if (new_partial_start_val.empty())
                 partial_start_feat_col->ClearInFeature(*new_cds);
             else
                 partial_start_feat_col->AddToFeature(*new_cds, new_partial_start_val, edit::eExistingText_replace_old);
             changed_feat = true;
         }
         
         string partial_stop_val = partial_stop_feat_col->GetFromFeat(*fi->GetOriginalSeq_feat());
         string new_partial_stop_val;
         try
         {
             if (row <  values_table->GetColumn(kPartialStop).GetData().GetString().size())
                 new_partial_stop_val = values_table->GetColumn(kPartialStop).GetData().GetString()[row];
         } catch(CSeqTableException&) {}
         
         if (new_partial_stop_val != partial_stop_val)
         {
             if (new_partial_stop_val.empty())
                 partial_stop_feat_col->ClearInFeature(*new_cds);
             else
                 partial_stop_feat_col->AddToFeature(*new_cds, new_partial_stop_val, edit::eExistingText_replace_old);
             changed_feat = true;
         }

         if (changed_feat)
         {
             cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(*fi,*new_cds)));
         }
         m_Modified |= changed_feat;

         row++;
     }            
 
 return cmd;  
}

void CBulkCDS::GetUpdateMRNAProductNameCmd(CRef<CSeq_feat> cds, CScope& scope, const string& prot_name, CRef<CCmdComposite> composite)
{
    CIRef<IEditCommand> cmd;
    CConstRef<CSeq_feat> orig_mrna;

    if (cds) 
    {
        orig_mrna.Reset(sequence::GetmRNAforCDS(*cds, scope));           
    }
    
    if (orig_mrna) 
    {
        CRef<CSeq_feat> new_mrna(new CSeq_feat());
        new_mrna->Assign(*orig_mrna);
        string remainder;
        new_mrna->SetData().SetRna().SetRnaProductName(prot_name, remainder);
        CSeq_feat_Handle mrna_fh = scope.GetSeq_featHandle(*orig_mrna);
        cmd.Reset(new CCmdChangeSeq_feat(mrna_fh, *new_mrna));
    }

    if (cmd)
        composite->AddCommand(*cmd);
}

CRef<CCmdComposite> CBulkCDS::GetCommand()
{
    CRef<CSeq_table> values_table = m_GridPanel->GetValuesTable();
    CRef<CCmdComposite> cmd = GetCommandFromValuesTable(values_table);
    return cmd;
}

string CBulkCDS::GetErrorMessage()
{
    return "Invalid operation in Bulk CDS Edit";
}


/*!
 * Should we show tooltips?
 */

bool CBulkCDS::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkCDS::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkCDS bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkCDS bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkCDS::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkCDS icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkCDS icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CBulkCDS::OnClickOk( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in CBulkCDS.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in CBulkCDS. 
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CBulkCDS::OnClickCancel( wxCommandEvent& event )
{    
    bool modified = m_GridPanel->GetModified() | m_AecrPanel->GetModified();
    if (modified)
    {
        wxMessageDialog dlg(this,_("Discard modifications?"), _("Attention"),wxOK|wxCANCEL|wxCENTRE);
        if (dlg.ShowModal() == wxID_OK)
        {
            event.Skip();
        }
    }
    else
        event.Skip();

}

const CObject* CBulkCDS::RowToScopedObjects(int row, TConstScopedObjects &objects, CBioseq_Handle &bsh)
{
    objects.clear();
    const CObject* obj = NULL;

    if (row < m_FeatHandles.size() )
    {
        if ( m_FeatHandles[row].GetOriginalSeq_feat()->IsSetLocation() )
        {
            bsh = m_FeatHandles[row].GetScope().GetBioseqHandle(m_FeatHandles[row].GetOriginalSeq_feat()->GetLocation());   // CBioseq_Handle GetBioseqHandle(const CSeq_loc& loc);
            if (bsh)
            {
                if (m_SeqSubmit)
                    objects.push_back(SConstScopedObject(m_SeqSubmit,&bsh.GetScope()));
                else
                    objects.push_back(SConstScopedObject(bsh.GetTopLevelEntry().GetCompleteSeq_entry(),&bsh.GetScope()));
                obj = bsh.GetBioseqCore().GetPointer();
            }
        }
    }
    return obj;
}

END_NCBI_SCOPE
