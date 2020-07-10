/*  $Id: bulk_gene_edit.cpp 43676 2019-08-14 14:28:05Z asztalos $
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
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <objects/seqtable/seq_table_exception.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_select.hpp>
#include <gui/packages/pkg_sequence_edit/apply_edit_convert_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_gene_edit.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


IMPLEMENT_DYNAMIC_CLASS( CBulkGene, wxDialog )

BEGIN_EVENT_TABLE( CBulkGene, wxDialog )

    EVT_BUTTON( wxID_OK, CBulkGene::OnClickOk )

    EVT_BUTTON( wxID_CANCEL, CBulkGene::OnClickCancel )


END_EVENT_TABLE()

CBulkGene::CBulkGene()
{
    Init();
}

CBulkGene::CBulkGene( wxWindow* parent, objects::CSeq_entry_Handle seh,  IWorkbench* workbench, CConstRef<objects::CSeq_submit> seqSubmit,
                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    this->m_Workbench = workbench;
    this->m_SeqSubmit = seqSubmit;

    CFeat_CI fi(seh, CSeqFeatData::eSubtype_gene);           
    for ( ; fi ; ++fi ) 
    {
        m_FeatHandles.push_back(fi->GetSeq_feat_Handle());
    }
    Init();
    Create(parent, id, caption, pos, size, style);
}

CBulkGene::CBulkGene( wxWindow* parent, const vector<CSeq_feat_Handle> &feat_handles, IWorkbench* workbench, 
                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_FeatHandles(feat_handles)
{
    this->m_Workbench = workbench;
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CBulkGene::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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


CBulkGene::~CBulkGene()
{
}


/*!
 * Member initialisation
 */

void CBulkGene::Init()
{
    m_Grid=NULL;
}


void CBulkGene::CreateControls()
{    
    CBulkGene* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;

    if (m_FeatHandles.empty())  
    {
        wxMessageBox(wxT("No Gene records found"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No Gene records found" );
    }

    CRef<objects::CSeq_table> values_table = GetValuesTableFromSeqEntry();
    if (values_table->GetNum_rows() < 1)
    {
        wxMessageBox(wxT("No Gene records found"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No Gene records found" );
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
    m_GridPanel->SetColumnSizesAndChoices();
    
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
    

    
    CStringConstraintSelect *itemStringConstraintPanel = new CStringConstraintSelect( itemDialog1, m_GridPanel, itemChoiceStrings, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);
    
    m_AecrPanel = new CApplyEditconvertPanel( itemDialog1, m_GridPanel, itemChoiceStringsWritable, 0, true, wxID_ANY, wxDefaultPosition, wxSize(1046, 219));
    itemBoxSizer2->Add(m_AecrPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 1);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


CRef<objects::CSeq_table> CBulkGene::GetValuesTableFromSeqEntry() 
{
    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);

    CRef<objects::CSeqTable_column> expand_col(new objects::CSeqTable_column());
    expand_col->SetHeader().SetTitle("");
    expand_col->SetHeader().SetField_name("expand");
    expand_col->SetData().SetString();

    CRef<objects::CSeqTable_column> locus_col(new objects::CSeqTable_column());
    locus_col->SetHeader().SetTitle("locus");
    locus_col->SetHeader().SetField_name("locus");
    locus_col->SetData().SetString();

    CRef<objects::CSeqTable_column> locus_tag_col(new objects::CSeqTable_column());
    locus_tag_col->SetHeader().SetTitle("locus_tag");
    locus_tag_col->SetHeader().SetField_name("locus_tag");
    locus_tag_col->SetData().SetString();

    CRef<objects::CSeqTable_column> desc_col(new objects::CSeqTable_column());
    desc_col->SetHeader().SetTitle("description");
    desc_col->SetHeader().SetField_name("description");
    desc_col->SetData().SetString();

    CRef<objects::CSeqTable_column> loc_col(new objects::CSeqTable_column());
    loc_col->SetHeader().SetTitle("location");
    loc_col->SetHeader().SetField_name("location");
    loc_col->SetData().SetString();

    CRef<objects::CSeqTable_column> comment_col(new objects::CSeqTable_column());
    comment_col->SetHeader().SetTitle("comment");
    comment_col->SetHeader().SetField_name("comment");
    comment_col->SetData().SetString();

    CRef<objects::CSeqTable_column> allele_col(new objects::CSeqTable_column());
    allele_col->SetHeader().SetTitle("allele");
    allele_col->SetHeader().SetField_name("allele");
    allele_col->SetData().SetString();

    CRef<objects::CSeqTable_column> synonym_col(new objects::CSeqTable_column());
    synonym_col->SetHeader().SetTitle("synonym");
    synonym_col->SetHeader().SetField_name("synonym");
    synonym_col->SetData().SetString();

    // bogus column to include last, otherwise deletion of the previous column will not work
    CRef<objects::CSeqTable_column> bogus_col(new objects::CSeqTable_column());
    bogus_col->SetHeader().SetTitle("");
    bogus_col->SetHeader().SetField_name("");
    bogus_col->SetData().SetString();

    CRef<objects::CSeq_table> table(new objects::CSeq_table());  
    table->SetColumns().push_back(id_col);
    table->SetColumns().push_back(expand_col);
    table->SetColumns().push_back(locus_col);
    table->SetColumns().push_back(locus_tag_col);
    table->SetColumns().push_back(desc_col);
    table->SetColumns().push_back(loc_col);

    CRef<CFeatureSeqTableColumnBase> partial_start_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStart);
    CRef<CSeqTable_column> partial_start_col = AddStringColumnToTable(table, kPartialStart);
    partial_start_col->SetHeader().SetField_name(kPartialStart);

    CRef<CFeatureSeqTableColumnBase> partial_stop_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStop);
    CRef<CSeqTable_column> partial_stop_col = AddStringColumnToTable(table, kPartialStop);
    partial_stop_col->SetHeader().SetField_name(kPartialStop);

    table->SetColumns().push_back(comment_col);
    table->SetColumns().push_back(allele_col);
    table->SetColumns().push_back(synonym_col);
    table->SetColumns().push_back(bogus_col);

    size_t row = 0;

    for ( vector<CSeq_feat_Handle>::iterator fi = m_FeatHandles.begin(); fi != m_FeatHandles.end() ; ++fi )     
    {
        string locus,locus_tag,desc,loc,comment, allele, synonym;
        
        const CSeq_feat &feat = *fi->GetOriginalSeq_feat();
        const CGene_ref &gene = feat.GetData().GetGene();
        if (gene.IsSetLocus())
            locus = gene.GetLocus();
        if (gene.IsSetLocus_tag())
            locus_tag = gene.GetLocus_tag();
        if (gene.IsSetDesc())
            desc = gene.GetDesc();
        if (feat.IsSetLocation())
            feat.GetLocation().GetLabel(&loc);
        if (feat.IsSetComment())
            comment = feat.GetComment();
        if (gene.IsSetAllele())
            allele = gene.GetAllele();
        if (gene.IsSetSyn())
            synonym = NStr::Join(gene.GetSyn(), "|");
        CRef<objects::CSeq_id> id(new objects::CSeq_id());
        id->SetLocal().SetId(row);
        id_col->SetData().SetId().push_back(id);
        expand_col->SetData().SetString().push_back("");
        locus_col->SetData().SetString().push_back(locus);
        locus_tag_col->SetData().SetString().push_back(locus_tag);
        desc_col->SetData().SetString().push_back(desc);
        loc_col->SetData().SetString().push_back(loc);
        AddValueToColumn(partial_start_col, partial_start_feat_col->GetFromFeat(feat), row);
        AddValueToColumn(partial_stop_col, partial_stop_feat_col->GetFromFeat(feat), row);
        comment_col->SetData().SetString().push_back(comment);
        allele_col->SetData().SetString().push_back(allele);
        synonym_col->SetData().SetString().push_back(synonym);
        bogus_col->SetData().SetString().push_back("");
        row++;
    }
    
    table->SetNum_rows(row);
    return table;
}


CRef<CCmdComposite> CBulkGene::GetCommandFromValuesTable(CRef<CSeq_table> values_table) 
{
 CRef<CCmdComposite> cmd(new CCmdComposite("Bulk Gene Edit"));
 CRef<CFeatureSeqTableColumnBase> partial_start_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStart);
 CRef<CFeatureSeqTableColumnBase> partial_stop_feat_col = CFeatureSeqTableColumnBaseFactory::Create(kPartialStop);
 m_Modified = false;
 size_t row = 0;

 for ( vector<CSeq_feat_Handle>::iterator fi = m_FeatHandles.begin(); fi != m_FeatHandles.end() ; ++fi ) 
 {
     const CSeq_feat &feat =  *fi->GetOriginalSeq_feat();
     CRef<CSeq_feat> new_feat(new CSeq_feat());
     new_feat->Assign(feat);
     string locus,locus_tag,desc,loc,comment, allele, synonym;
     bool modified = false;
     const CGene_ref &gene = feat.GetData().GetGene();
     if (gene.IsSetLocus())
         locus = gene.GetLocus();
     if (gene.IsSetLocus_tag())
         locus_tag = gene.GetLocus_tag();
     if (gene.IsSetDesc())
         desc = gene.GetDesc();
     if (feat.IsSetLocation())
         feat.GetLocation().GetLabel(&loc);
     if (feat.IsSetComment())
         comment = feat.GetComment();        
     if (gene.IsSetAllele())
         allele = gene.GetAllele();
     if (gene.IsSetSyn())
         synonym = NStr::Join(gene.GetSyn(), "|");

     string new_locus;
     try
     {
         if (row < values_table->GetColumn("locus").GetData().GetString().size())
             new_locus = values_table->GetColumn("locus").GetData().GetString()[row];
     } catch(CSeqTableException&) {}
     
     if (new_locus != locus)
     {
         if (new_locus.empty())
             new_feat->SetData().SetGene().ResetLocus();
         else
             new_feat->SetData().SetGene().SetLocus(new_locus);
         modified = true;
     }

     string new_locus_tag;
     try
     {
         if (row < values_table->GetColumn("locus_tag").GetData().GetString().size())
             new_locus_tag = values_table->GetColumn("locus_tag").GetData().GetString()[row];
     } catch(CSeqTableException&) {}
     
     if (new_locus_tag != locus_tag)
     {
         if (new_locus_tag.empty())
             new_feat->SetData().SetGene().ResetLocus_tag();
         else
             new_feat->SetData().SetGene().SetLocus_tag(new_locus_tag);
         modified = true;
     }
             

     string new_desc;
     try
     {
         if (row <  values_table->GetColumn("description").GetData().GetString().size())
             new_desc = values_table->GetColumn("description").GetData().GetString()[row];
     } catch(CSeqTableException&) {}
             
     if (new_desc != desc)
     {
         if (new_desc.empty())
             new_feat->SetData().SetGene().ResetDesc();
         else
             new_feat->SetData().SetGene().SetDesc() = new_desc;
         modified = true;
     }

     string new_comment;
     try
     {
         if (row <  values_table->GetColumn("comment").GetData().GetString().size())
             new_comment = values_table->GetColumn("comment").GetData().GetString()[row];
     } catch(CSeqTableException&) {}
     
     if (new_comment != comment)
     {
         if (new_comment.empty())
             new_feat->ResetComment();
         else
             new_feat->SetComment() = new_comment;
         modified = true;
     }

     string new_allele;
     try
     {
         if (row <  values_table->GetColumn("allele").GetData().GetString().size())
             new_allele = values_table->GetColumn("allele").GetData().GetString()[row];
     } catch(CSeqTableException&) {}
     if (new_allele != allele)
     {
         new_feat->SetData().SetGene().ResetAllele();
         if (!new_allele.empty())
             new_feat->SetData().SetGene().SetAllele(new_allele);
         modified = true;
     }

     string new_synonym;
     try
     {
         if (row <  values_table->GetColumn("synonym").GetData().GetString().size())
             new_synonym = values_table->GetColumn("synonym").GetData().GetString()[row];
     } catch(CSeqTableException&) {}
     if (new_synonym != synonym)
     {
         new_feat->SetData().SetGene().ResetSyn();
         vector<string> new_values;
         NStr::Split(new_synonym, "|", new_values);
         for (const auto& val : new_values)
             if (!val.empty())
                 new_feat->SetData().SetGene().SetSyn().push_back(val);
         
         modified = true;
     }

     string partial_start_val = partial_start_feat_col->GetFromFeat(feat);
     string new_partial_start_val;
     try
     {
         if (row <  values_table->GetColumn(kPartialStart).GetData().GetString().size())
             new_partial_start_val = values_table->GetColumn(kPartialStart).GetData().GetString()[row];
     } catch(CSeqTableException&) {}
     
     if (new_partial_start_val != partial_start_val)
     {
         if (new_partial_start_val.empty())
             partial_start_feat_col->ClearInFeature(*new_feat);
         else
             partial_start_feat_col->AddToFeature(*new_feat, new_partial_start_val, edit::eExistingText_replace_old);
         modified = true;
     }
         
     string partial_stop_val = partial_stop_feat_col->GetFromFeat(feat);
     string new_partial_stop_val;
     try
     {
         if (row <  values_table->GetColumn(kPartialStop).GetData().GetString().size())
             new_partial_stop_val = values_table->GetColumn(kPartialStop).GetData().GetString()[row];
     } catch(CSeqTableException&) {}
     
     if (new_partial_stop_val != partial_stop_val)
     {
         if (new_partial_stop_val.empty())
             partial_stop_feat_col->ClearInFeature(*new_feat);
         else
             partial_stop_feat_col->AddToFeature(*new_feat, new_partial_stop_val, edit::eExistingText_replace_old);
         modified = true;
     }

     if (modified)
         cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(*fi,*new_feat)));
     m_Modified |= modified;
     row++;
 }            
 
 return cmd;  
}


CRef<CCmdComposite> CBulkGene::GetCommand()
{
    CRef<CSeq_table> values_table = m_GridPanel->GetValuesTable();
    CRef<CCmdComposite> cmd = GetCommandFromValuesTable(values_table);
    return cmd;
}

string CBulkGene::GetErrorMessage()
{
    return "Invalid operation in Bulk Gene Edit";
}


/*!
 * Should we show tooltips?
 */

bool CBulkGene::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkGene::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkGene bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkGene bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkGene::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkGene icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkGene icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CBulkGene::OnClickOk( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in CBulkGene.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in CBulkGene. 
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CBulkGene::OnClickCancel( wxCommandEvent& event )
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

const CObject* CBulkGene::RowToScopedObjects(int row, TConstScopedObjects &objects,  CBioseq_Handle &bsh)
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
