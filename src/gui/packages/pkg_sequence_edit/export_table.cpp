/*  $Id: export_table.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objects/seq/Seq_descr.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objects/macro/Feat_qual_legal.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/dblink_field.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>
#include <gui/packages/pkg_sequence_edit/pub_field.hpp>
#include <gui/packages/pkg_sequence_edit/export_table.hpp>


#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_features.hpp>

#ifdef NCBI_OS_MSWIN
#include <wx/msw/ole/automtn.h>
#endif

#include <wx/filename.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CExportTableDlg, wxDialog )


BEGIN_EVENT_TABLE( CExportTableDlg, wxDialog )


END_EVENT_TABLE()

CExportTableDlg::CExportTableDlg()
{
    Init();
}

CExportTableDlg::CExportTableDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    FindBioSource(m_TopSeqEntry);
    Create(parent, id, caption, pos, size, style);
}


bool CExportTableDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


CExportTableDlg::~CExportTableDlg()
{
}


/*!
 * Member initialisation
 */

void CExportTableDlg::Init()
{
    m_ExcelCheck = NULL;
}

void CExportTableDlg::CreateControls()
{    
    CExportTableDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer1);

    m_SourceCheck = new wxCheckBox(itemDialog1, ID_EXPTABLE_CHKBOX1, _("Source Qualifiers"));
    itemBoxSizer1->Add(m_SourceCheck, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_SourceCheck->SetValue(true);

    wxArrayString source_quals;
    source_quals.Add(ToWxString("All"));
    for (set<string>::iterator qual = m_source_choices.begin(); qual != m_source_choices.end(); ++qual)
        source_quals.Add(ToWxString(*qual));

    m_SourceChoice = new wxListBox(itemDialog1, ID_EXPTABLE_LISTBOX, wxDefaultPosition, wxDefaultSize, source_quals, wxLB_MULTIPLE, wxDefaultValidator);
    m_SourceChoice->SetSelection(0);
    itemBoxSizer1->Add(m_SourceChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer2Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Other Qualifiers"));
    wxStaticBoxSizer* itemBoxSizer2 = new wxStaticBoxSizer(itemStaticBoxSizer2Static, wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_NoneRB = new wxRadioButton(itemDialog1, ID_EXPTABLE_RADBTN1, _("None"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_NoneRB->SetValue(true);
    itemBoxSizer2->Add(m_NoneRB, 0, wxALIGN_LEFT|wxALL, 5);

    m_FeatureRB = new wxRadioButton(itemDialog1, ID_EXPTABLE_RADBTN2, _("Feature Qualifiers"));
    itemBoxSizer2->Add(m_FeatureRB, 0, wxALIGN_LEFT|wxALL, 5);
   
    m_CdsGeneProtRB = new wxRadioButton(itemDialog1, ID_EXPTABLE_RADBTN3, _("CDS-Gene-Prot Qualifiers"));
    itemBoxSizer2->Add(m_CdsGeneProtRB, 0, wxALIGN_LEFT|wxALL, 5);

    m_RnaRB = new wxRadioButton(itemDialog1, ID_EXPTABLE_RADBTN4, _("RNA Qualifiers"));
    itemBoxSizer2->Add(m_RnaRB, 0, wxALIGN_LEFT|wxALL, 5);

    m_DeflineRB = new wxRadioButton(itemDialog1, ID_EXPTABLE_RADBTN5, _("Definition Line"));
    itemBoxSizer2->Add(m_DeflineRB, 0, wxALIGN_LEFT|wxALL, 5);

    m_PubRB = new wxRadioButton(itemDialog1, ID_EXPTABLE_RADBTN6, _("Publication Titles"));
    itemBoxSizer2->Add(m_PubRB, 0, wxALIGN_LEFT|wxALL, 5);

    m_DbLinkRB = new wxRadioButton(itemDialog1, ID_EXPTABLE_RADBTN7, _("DBLink"));
    itemBoxSizer2->Add(m_DbLinkRB, 0, wxALIGN_LEFT|wxALL, 5);

#ifdef NCBI_OS_MSWIN
    m_ExcelCheck = new wxCheckBox(itemDialog1, ID_EXPTABLE_CHKBOX2, _("Launch Excel"));
    itemBoxSizer1->Add(m_ExcelCheck, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* itemButton1 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton2 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}


/*!
 * Should we show tooltips?
 */

bool CExportTableDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CExportTableDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CExportTableDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

void CExportTableDlg::AddBioSource(const CBioSource& biosource)
{
    FOR_EACH_ORGMOD_ON_BIOSOURCE(orgmod,biosource)
    {
        try {
            string qual_name = objects::COrgMod::GetSubtypeName ((*orgmod)->GetSubtype());
            if (!NStr::IsBlank(qual_name) && !objects::COrgMod::IsDiscouraged((*orgmod)->GetSubtype(), true)) {
                // special case for nat-host
                if (NStr::EqualNocase(qual_name, kNatHost)) {
                    m_source_choices.insert(ToStdString(kHost));
                } else if (NStr::EqualNocase(qual_name, "note")) {
                    m_source_choices.insert(kOrgModNote);
                } else {
                    m_source_choices.insert(qual_name);
                    if ( (*orgmod)->GetSubtype() == COrgMod::eSubtype_bio_material ||
                         (*orgmod)->GetSubtype() == COrgMod::eSubtype_culture_collection ||
                         (*orgmod)->GetSubtype() == COrgMod::eSubtype_specimen_voucher) {
                        m_source_choices.insert(qual_name + "-coll");
                        m_source_choices.insert(qual_name + "-inst");
                        m_source_choices.insert(qual_name + "-specid");
                    }
                }
            }
            
        } catch(const CException&) {} catch (const exception&) {} 
    }

    FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsource,biosource)
    {
        try {
            string qual_name = objects::CSubSource::GetSubtypeName ((*subsource)->GetSubtype());
            if (!NStr::IsBlank(qual_name) && !objects::CSubSource::IsDiscouraged((*subsource)->GetSubtype())) {
                if (NStr::EqualNocase(qual_name, "note")) {
                    m_source_choices.insert(kSubSourceNote);
                } else {
                    m_source_choices.insert(qual_name);
                }
            }
            
        } catch(const CException&) {} catch (const exception&) {} 
    }

    if (biosource.IsSetTaxname() && !biosource.GetTaxname().empty())
        m_source_choices.insert("taxname");
}

void CExportTableDlg::GetDesc(const CSeq_entry& se)  
{

    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) {
        if ((*it)->IsSource()) {
            AddBioSource ((*it)->GetSource());
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            GetDesc (**it);
        }
    }
}


void  CExportTableDlg::FindBioSource(objects::CSeq_entry_Handle tse)
{
    GetDesc (*(tse.GetCompleteSeq_entry()));
    CFeat_CI feat (tse, SAnnotSelector(CSeqFeatData::e_Biosrc));
    while (feat) {
        AddBioSource (feat->GetData().GetBiosrc()); 
        ++feat;
    }
}

void CExportTableDlg::SaveTable(const wxString& workDir)
{
    m_SaveFileDir = workDir;
    list< map<string, string> > table;
    set<string> columns;
    if (m_CdsGeneProtRB->GetValue())
    {
        AddCdsGeneMrnaTable(table, columns);
    }
    if (m_DeflineRB->GetValue())
    {        
        AddDeflinesToTable(table, columns);
    }
    if (m_FeatureRB->GetValue())
    {       
        AddFeaturesToTable(table, columns);
    }
    if (m_PubRB->GetValue())
    {
        AddPubsToTable(table, columns);
    }
    if (m_DbLinkRB->GetValue())
    {
        AddDblinkToTable(table, columns);
    }
    if (m_RnaRB->GetValue())
    {
        AddRnaToTable(table, columns);
    }
    if (m_SourceCheck->GetValue())
    {
        AddSourceToTable(table, columns);
    }
    SaveTableFile (this, m_SaveFileDir, m_SaveFileName, table, columns);
    wxFileName fname;
    fname.Assign(m_SaveFileDir, m_SaveFileName);

    if (m_ExcelCheck && m_ExcelCheck->GetValue())
    {
        RunExcel(fname.GetFullPath());
    }
}

void CExportTableDlg::AddCdsGeneMrnaTable( list< map<string, string> > &table, set<string> &columns)
{
    vector<pair<string, CIRef<IEditingAction> > >  cds_gene_mrna_actions;
    IEditingActionFeat* cds_gene_mrna_feat_action(NULL);
        
    vector<string> cds_gene_mrna_fields;
    cds_gene_mrna_fields.push_back("protein name");
    cds_gene_mrna_fields.push_back("protein description");
    cds_gene_mrna_fields.push_back("CDS comment");
    cds_gene_mrna_fields.push_back("gene locus");
    cds_gene_mrna_fields.push_back("gene description");
    cds_gene_mrna_fields.push_back("gene comment");
    cds_gene_mrna_fields.push_back("gene allele");
    cds_gene_mrna_fields.push_back("gene maploc");
    cds_gene_mrna_fields.push_back("gene locus tag");
    cds_gene_mrna_fields.push_back("gene synonym");
    cds_gene_mrna_fields.push_back("mRNA product");
    cds_gene_mrna_fields.push_back("mRNA comment");
    cds_gene_mrna_fields.push_back("protein name");
    cds_gene_mrna_fields.push_back("protein description");
    cds_gene_mrna_fields.push_back("protein EC number");
    cds_gene_mrna_fields.push_back("protein activity");
    cds_gene_mrna_fields.push_back("protein comment");
    cds_gene_mrna_fields.push_back("codon-start");
        
    for (auto field : cds_gene_mrna_fields)
    {
        int subtype = -1;
        if (field == "protein name" || field == "protein description" || field == "protein EC number"
            || field == "protein activity" || field == "protein comment")
        {
            subtype = CSeqFeatData::eSubtype_prot;
        }
        else if (field == "CDS comment" || field == "CDS inference" || field == "codon-start")
        {
            subtype =  CSeqFeatData::eSubtype_cdregion;
        }
        else if ( field == "gene description" || field == "gene comment" 
                  || field == "gene inference" || field == "gene allele" || field == "gene maploc"
                  || field == "gene locus tag" || field == "gene synonym" || field == "gene old_locus_tag"
                  || field == "gene locus")
        {
            subtype =  CSeqFeatData::eSubtype_gene;
        }       
        else if (field == "mRNA product" || field == "mRNA comment")
        {
            subtype =  CSeqFeatData::eSubtype_mRNA;
        }
/*      else if (field == "mat_peptide name" || field == "mat_peptide description" || field == "mat_peptide EC number"
                 || field == "mat_peptide activity" || field == "mat_peptide comment")
        {
            subtype = CSeqFeatData::eSubtype_mat_peptide_aa;
        }
*/
        cds_gene_mrna_actions.push_back(make_pair(field, CreateAction(m_TopSeqEntry, field, CFieldNamePanel::eFieldType_CDSGeneProt, subtype)));
        if (cds_gene_mrna_actions.back().second && !cds_gene_mrna_feat_action)
            cds_gene_mrna_feat_action = dynamic_cast<IEditingActionFeat*>(cds_gene_mrna_actions.back().second.GetPointer());
    }
    

    set<CSeq_feat_Handle> seen;
    CScope &scope = m_TopSeqEntry.GetScope();
    for( CFeat_CI feat (m_TopSeqEntry); feat; ++feat)
    {
        CSeq_feat_Handle fh = feat->GetSeq_feat_Handle();   
        if (seen.find(fh) != seen.end())
            continue;
        seen.insert(fh);
        map<string, string> row;
        if (cds_gene_mrna_feat_action)
        {
            CSeq_feat_Handle cds, gene, mrna, prot;
            if (fh.GetFeatSubtype() == CSeqFeatData::eSubtype_cdregion)
            {
                cds = fh;
                gene = cds_gene_mrna_feat_action->x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_gene);
                mrna = cds_gene_mrna_feat_action->x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_mRNA);
            }
            if (fh.GetFeatSubtype() == CSeqFeatData::eSubtype_gene)
            {
                gene = fh;
                cds = cds_gene_mrna_feat_action->x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_cdregion);
                mrna = cds_gene_mrna_feat_action->x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_mRNA);
            }
            if (fh.GetFeatSubtype() == CSeqFeatData::eSubtype_mRNA)
            {
                mrna = fh;
                gene = cds_gene_mrna_feat_action->x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_gene);
                cds = cds_gene_mrna_feat_action->x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_cdregion);
            }
            if (cds && cds.IsSetProduct())
            {
                const CSeq_loc& prot_loc = cds.GetProduct();
                CBioseq_Handle prot_bsh = scope.GetBioseqHandle(prot_loc);
                if (prot_bsh) 
                {
                    CFeat_CI prot_feat_ci(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                    if( prot_feat_ci)
                    {
                        prot = prot_feat_ci->GetSeq_feat_Handle();
                    }
                }
            }
            seen.insert(cds);
            seen.insert(gene);
            seen.insert(mrna);
            seen.insert(prot);
            for (auto field_action : cds_gene_mrna_actions)
            {
                string field = field_action.first;
                string value;
                IEditingActionFeat* feat_action = dynamic_cast<IEditingActionFeat*>(field_action.second.GetPointer());
                if (feat_action)
                {
                    bool is_feat_set(false);
                    if (feat_action->GetActionFeatSubtype() == CSeqFeatData::eSubtype_cdregion && cds)
                    {
                        feat_action->SetFeat(cds);
                        is_feat_set = true;
                    }
                    if (feat_action->GetActionFeatSubtype() == CSeqFeatData::eSubtype_gene && gene)
                    {
                        feat_action->SetFeat(gene);
                        is_feat_set = true;
                    }
                    if (feat_action->GetActionFeatSubtype() == CSeqFeatData::eSubtype_mRNA && mrna)
                    {
                        feat_action->SetFeat(mrna);
                        is_feat_set = true;
                    }
                    if (feat_action->GetActionFeatSubtype() == CSeqFeatData::eSubtype_prot && prot)
                    {
                        feat_action->SetFeat(prot);
                        is_feat_set = true;
                    }
                    
                    if (is_feat_set)
                    {
                        const vector<string>& values = feat_action->GetValues();
                        if (!values.empty())
                            value = values.front();
                    }
                }
                if (!value.empty())
                {
                    row[field] = value;
                    columns.insert(field);
                }
            }      
        }

        if (!row.empty())
        {
            string seqid;
            for (CSeq_loc_CI subloc(fh.GetLocation(), objects::CSeq_loc_CI::eEmpty_Skip); subloc; ++subloc)
            {
                CBioseq_Handle bsh = scope.GetBioseqHandle(subloc.GetSeq_id());
                if (!bsh)
                    continue;
                sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId()->GetLabel(&seqid, CSeq_id::eContent); 
                if (!seqid.empty())
                    break;
            }
            row["Seq-ID"] = seqid;
            table.push_back(row);           
        }
    }   
}

void CExportTableDlg::AddRnaToTable( list< map<string, string> > &table, set<string> &columns)
{
    vector<pair<string, CIRef<IEditingAction> > >  actions;
    IEditingActionFeat* rna_feat_action(NULL);
    vector<string> fields;

    fields.push_back("product");
    fields.push_back("comment");
    fields.push_back("ncRNA class"); // ncRNA
    fields.push_back("gene locus");
    fields.push_back("gene description");
    fields.push_back("gene maploc");
    fields.push_back("gene locus tag");
    fields.push_back("gene synonym");
    fields.push_back("gene comment");
    fields.push_back("codons recognized"); // tRNA
    fields.push_back("anticodon");         // tRNA
    fields.push_back("tag_peptide"); // tmRNA
        

    
    for (auto field : fields)
    {
        int subtype = -1;
        if ( field == "gene description" || field == "gene comment"  || field == "gene maploc"
             || field == "gene locus tag" || field == "gene synonym" || field == "gene locus")
        {
            subtype = CSeqFeatData::eSubtype_gene;
        }       
        else if (field == "ncRNA class")
        {
            subtype = CSeqFeatData::eSubtype_ncRNA;
        }
        else if (field == "codons recognized" || field == "anticodon")
        {
            subtype = CSeqFeatData::eSubtype_tRNA;
        }
        else if (field == "tag_peptide")
        {
            subtype = CSeqFeatData::eSubtype_tmRNA;
        }

       actions.push_back(make_pair(field, CreateAction(m_TopSeqEntry, field, CFieldNamePanel::eFieldType_RNA, subtype)));
        if (actions.back().second && !rna_feat_action)
            rna_feat_action = dynamic_cast<IEditingActionFeat*>(actions.back().second.GetPointer());
    }
    
    CScope &scope = m_TopSeqEntry.GetScope();
    for( CFeat_CI feat (m_TopSeqEntry, CSeqFeatData::e_Rna); feat; ++feat)
    {
        CSeq_feat_Handle rna = feat->GetSeq_feat_Handle();   
        map<string, string> row;
        if (rna_feat_action)
        {
            CSeq_feat_Handle  gene = rna_feat_action->x_FindGeneForFeature(rna.GetLocation(), scope, CSeqFeatData::eSubtype_gene);
          
            for (auto field_action : actions)
            {
                string field = field_action.first;
                string value;
                IEditingActionFeat* feat_action = dynamic_cast<IEditingActionFeat*>(field_action.second.GetPointer());
                if (feat_action)
                {
                    bool is_feat_set(false);
                    if (feat_action->GetActionFeatSubtype() == CSeqFeatData::eSubtype_gene && gene)
                    {
                        feat_action->SetFeat(gene);
                        is_feat_set = true;
                    }
                    else if (feat_action->GetActionFeatSubtype() == rna.GetFeatSubtype())
                    {
                        feat_action->SetFeat(rna);
                        is_feat_set = true;
                    }
                    else if (feat_action->GetActionFeatSubtype() == CSeqFeatData::eSubtype_any)
                    {
                        feat_action->SetFeat(rna);
                        is_feat_set = true;
                    }

                    if (is_feat_set)
                    {
                        const vector<string>& values = feat_action->GetValues();
                        if (!values.empty())
                            value = values.front();
                    }
                }
                if (!value.empty())
                {
                    row[field] = value;
                    columns.insert(field);
                }
            }      
        }

        if (!row.empty())
        {
            string seqid;
            for (CSeq_loc_CI subloc(rna.GetLocation(), objects::CSeq_loc_CI::eEmpty_Skip); subloc; ++subloc)
            {
                CBioseq_Handle bsh = scope.GetBioseqHandle(subloc.GetSeq_id());
                if (!bsh)
                    continue;
                sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId()->GetLabel(&seqid, CSeq_id::eContent); 
                if (!seqid.empty())
                    break;
            }
            row["Seq-ID"] = seqid;
            table.push_back(row);           
        }
    }   
}


void CExportTableDlg::AddSourceToTable( list< map<string, string> > &table, set<string> &columns)
{
    auto table_it = table.begin();
    vector<string> fields;
    wxArrayInt selections;
    int num = m_SourceChoice->GetSelections (selections);
    for (size_t j = 0; j < num; j++)
    {
        int  i = selections[j];
        string sel = m_SourceChoice->GetString(i).ToStdString();
        if (sel == "All")
        {
            fields.clear();
            for (set<string>::iterator s = m_source_choices.begin(); s != m_source_choices.end(); ++s)
                fields.push_back(*s);
            break;
        }
        else
            fields.push_back(sel);
    }
    vector<pair<string, CIRef<IEditingAction> > > actions;
    for (auto &field : fields)
    {
        actions.push_back(make_pair(field, CreateAction(m_TopSeqEntry, field, CFieldNamePanel::eFieldType_Source, -1)));
    }
   for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CBioseq_Handle bsh = *b_iter; 
        map<string, string> row;
        for (CSeqdesc_CI desc_ci( bsh, CSeqdesc::e_Source); desc_ci; ++desc_ci) 
        {
            const CSeqdesc* desc = &*desc_ci;
            for (auto field_action : actions)
            {
                string field = field_action.first;
                string value;
                IEditingActionBiosource* action = dynamic_cast<IEditingActionBiosource*>(field_action.second.GetPointer());
                if (action)
                {
                    action->SetDesc(desc);
                    const vector<string>& values = action->GetValues();
                    if (!values.empty())
                        value = values.front();
                }
                if (!value.empty())
                {
                    row[field] = value;
                    columns.insert(field);
                }
            }
        }

        for (CFeat_CI feat_ci(bsh, CSeqFeatData::e_Biosrc); feat_ci; ++feat_ci) 
        {
            CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();   
             for (auto field_action : actions)
             {
                 string field = field_action.first;
                 string value;
                 IEditingActionBiosource* action = dynamic_cast<IEditingActionBiosource*>(field_action.second.GetPointer());
                 if (action)
                 {
                     action->SetFeat(fh);
                     const vector<string>& values = action->GetValues();
                     if (!values.empty())
                         value = values.front();
                 }             
                 if (!value.empty())
                 {
                     row[field] = value;
                     columns.insert(field);
                 }
             }                 
        }


        string seqid;
        sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId()->GetLabel(&seqid, CSeq_id::eContent);
        bool found(false);
        while (table_it != table.end() && (*table_it)["Seq-ID"] == seqid)
        {
            found = true;
            if (!row.empty())
                table_it->insert(row.begin(), row.end());
            ++table_it;
        }
        if (!found && !row.empty())
        {          
            row["Seq-ID"] = seqid;
            table_it = table.insert(table_it, row);       
            ++table_it;
        }
    }
}


void CExportTableDlg::RemoveEmptyColsAndRows(CRef<CSeq_table> values_table)
{
    size_t num_rows = values_table->GetNum_rows();
    CSeq_table::TColumns::iterator col = values_table->SetColumns().begin();
    while (col != values_table->SetColumns().end())
    {
        bool do_remove = false;
        if ((*col)->IsSetData())
        {
            if ((*col)->GetData().IsString())
            {
                bool empty_col = true;
                for (size_t row = 0; row < (*col)->GetData().GetString().size() && empty_col; row++)
                {
                    empty_col = empty_col && (*col)->GetData().GetString()[row].empty();
                }
                if (empty_col)
                {
                    do_remove = true;
                }
            }
        }
        if (do_remove)
        {
            col = values_table->SetColumns().erase(col);
        } 
        else
        {
            ++col;
        }
    }

    vector<size_t> erase_rows;
    for (size_t row = 0; row < num_rows; row++)
    {
        bool empty_row = true;
        for (CSeq_table::TColumns::iterator col = values_table->SetColumns().begin(); col != values_table->SetColumns().end(); ++col)
            if ((*col)->IsSetData() && (*col)->GetData().IsString() && row < (*col)->GetData().GetString().size())
                empty_row = empty_row && (*col)->GetData().GetString()[row].empty();
        if (empty_row)
        {
            erase_rows.push_back(row);
        }

    }
    for (int i = erase_rows.size()-1; i >= 0; i--)
    {        
        for (CSeq_table::TColumns::iterator col = values_table->SetColumns().begin(); col != values_table->SetColumns().end(); ++col)
            if ((*col)->IsSetData() && (*col)->GetData().IsString() && erase_rows[i] < (*col)->GetData().GetString().size())
            {
                (*col)->SetData().SetString().erase((*col)->SetData().SetString().begin()+erase_rows[i]);
            }       
            else if ((*col)->IsSetData() && (*col)->GetData().IsId() && erase_rows[i] < (*col)->GetData().GetId().size())
            {
                (*col)->SetData().SetId().erase((*col)->SetData().SetId().begin()+erase_rows[i]);
            }
    }
    values_table->SetNum_rows(num_rows-erase_rows.size());
}


void CExportTableDlg::AddDeflinesToTable(list< map<string, string> > &table, set<string> &columns)
{
    string field("Definition Line");

    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CBioseq_Handle bsh = *b_iter; 
        
        map<string, string> row;
        CSeqdesc_CI title(bsh, CSeqdesc::e_Title, 1);
        string value;
        if (  title ) 
        {
            value = title->GetTitle();
        }
        
        row[field] = value;
        if (!value.empty())
        {
            columns.insert(field);
            string seqid;
            sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId()->GetLabel(&seqid, CSeq_id::eContent);
            row["Seq-ID"] = seqid;
            table.push_back(row);           
        }  
    }
}

void CExportTableDlg::AddFeaturesToTable(list< map<string, string> > &table, set<string> &columns)
{
    CScope &scope = m_TopSeqEntry.GetScope();
    for( CFeat_CI feat (m_TopSeqEntry); feat; ++feat)
    {
        map<string, string> row;
        string feat_name = CSeqFeatData::SubtypeValueToName(feat->GetOriginalFeature().GetData().GetSubtype());
        FOR_EACH_GBQUAL_ON_SEQFEAT(qual,feat->GetOriginalFeature())
        {
            if ((*qual)->IsSetQual() && (*qual)->IsSetVal() && !(*qual)->GetQual().empty() && !(*qual)->GetVal().empty())
            {
                string field  = feat_name + " " + (*qual)->GetQual();
                string value = (*qual)->GetVal();
                row[field] = value;
                columns.insert(field);
            }
        }
        if (!row.empty())
        {
            string seqid;
            for (CSeq_loc_CI subloc(feat->GetLocation(), objects::CSeq_loc_CI::eEmpty_Skip); subloc; ++subloc)
            {
                CBioseq_Handle bsh = scope.GetBioseqHandle(subloc.GetSeq_id());
                if (!bsh)
                    continue;
                sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId()->GetLabel(&seqid, CSeq_id::eContent); 
                if (!seqid.empty())
                    break;
            }
            row["Seq-ID"] = seqid;
            table.push_back(row);
        }
    }
}

void CExportTableDlg::AddPubsToTable(list< map<string, string> > &table, set<string> &columns)
{
    vector<pair<string, CIRef<IEditingAction> > > actions;
    vector<string> pubs = CPubField::GetFieldNames();
    for (auto &field : pubs)
    {
        actions.push_back(make_pair("publication " + field, CreateAction(m_TopSeqEntry, field, CFieldNamePanel::eFieldType_Pub, -1)));
    }
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CBioseq_Handle bsh = *b_iter; 
        for (CSeqdesc_CI desc_ci( bsh, CSeqdesc::e_Pub); desc_ci; ++desc_ci)
        {
            map<string, string> row;
            const CSeqdesc* desc = &*desc_ci;
            for (auto field_action : actions)
            {
                string field = field_action.first;
                string value;
                IEditingActionDesc* action = dynamic_cast<IEditingActionDesc*>(field_action.second.GetPointer());
                if (action)
                {
                    action->SetDesc(desc);
                    const vector<string>& values = action->GetValues();
                    if (!values.empty())
                        value = values.front();
                }
                if (!value.empty())
                {
                    row[field] = value;
                    columns.insert(field);
                }
            }
        
            if (!row.empty())
            {
                string seqid;
                sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId()->GetLabel(&seqid, CSeq_id::eContent);
                row["Seq-ID"] = seqid;
                table.push_back(row);       
            }
        }
    }
}

void CExportTableDlg::AddDblinkToTable(list< map<string, string> > &table, set<string> &columns)
{
    vector<string> dbs = CDBLinkField::GetFieldNames();
    vector<pair<string, CIRef<IEditingAction> > > actions;
    for (auto &field : dbs)
    {
        actions.push_back(make_pair("DBLink " + field, CreateAction(m_TopSeqEntry, field, CFieldNamePanel::eFieldType_DBLink, -1)));
    }
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CBioseq_Handle bsh = *b_iter; 
        for (CSeqdesc_CI desc_ci( bsh, CSeqdesc::e_User); desc_ci; ++desc_ci)
        {
            if (desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
                && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), "DBLink") )
            {
                map<string, string> row;
                const CSeqdesc* desc = &*desc_ci;
                for (auto field_action : actions)
                {
                    string field = field_action.first;
                    string value;
                    IEditingActionDesc* action = dynamic_cast<IEditingActionDesc*>(field_action.second.GetPointer());
                    if (action)
                    {
                        action->SetDesc(desc);
                        const vector<string>& values = action->GetValues();
                        if (!values.empty())
                            value = values.front();
                    }
                    if (!value.empty())
                    {
                        row[field] = value;
                        columns.insert(field);
                    }
                }
        
                if (!row.empty())
                {
                    string seqid;
                    sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId()->GetLabel(&seqid, CSeq_id::eContent);
                    row["Seq-ID"] = seqid;
                    table.push_back(row);       
                }
            }
        }
    }
}

void CExportTableDlg::SaveTableFile (wxWindow *parent, wxString& save_file_dir, wxString& save_file_name,  const list< map<string, string> > &table, const  set<string> &columns)

{
    if (columns.empty() || table.empty())
    {
        wxMessageBox(wxT("Empty table"), wxT("Error"),
                     wxOK | wxICON_ERROR, parent);
        return; 
    }
    wxFileDialog table_save_file(parent, wxT("Select a file"), save_file_dir, save_file_name,
                                _("TAB files(*.tab) | *.tab | All files(*.*) | *.*"),                             
                                 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (table_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = table_save_file.GetPath();
        wxString name = table_save_file.GetFilename();

        name.Trim(false);
        name.Trim(true);

        if (name.IsEmpty())
        {
            wxMessageBox(wxT("Please, select file name"), wxT("Error"),
                         wxOK | wxICON_ERROR, parent);
            return; 
        }
        ios::openmode mode = ios::out;

        CNcbiOfstream os(path.fn_str(), mode); 
        if (!os)
        {
            wxMessageBox(wxT("Cannot open file ")+name, wxT("Error"),
                         wxOK | wxICON_ERROR, parent);
            return; 
        }
        
        CCSVExporter exporter(os, '\t', '"');
        exporter.Field(string("Seq-ID"));
        for (auto col : columns)
            exporter.Field(col);
        exporter.NewRow();
        for (auto row : table)
        {
            string seqid = row["Seq-ID"];
            row.erase("Seq-ID");
            exporter.Field(seqid);
            for (auto col : columns)
            {
                exporter.Field(row[col]);
            }
            exporter.NewRow();
        }      

        save_file_dir = table_save_file.GetDirectory();
        save_file_name = table_save_file.GetFilename();
    }
}

void CExportTableDlg::RunExcel(wxString FileName)
{
#ifdef NCBI_OS_MSWIN
    wxAutomationObject ExcelObject;
    if (ExcelObject.GetInstance(wxT("Excel.Application")))
    {       
        ExcelObject.CallMethod("Workbooks.Open", FileName);
        ExcelObject.PutProperty("Visible", true);
    }
#endif
}

END_NCBI_SCOPE
