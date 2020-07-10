/*  $Id: subprep_util.cpp 43676 2019-08-14 14:28:05Z asztalos $
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

#include <objects/seqset/Seq_entry.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/PCRReactionSet.hpp>
#include <objects/seqfeat/PCRReaction.hpp>
#include <objects/seqfeat/PCRPrimerSet.hpp>
#include <objects/seqfeat/PCRPrimer.hpp>
#include <objects/seqfeat/PCRPrimerSeq.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seq/IUPACaa.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_vector.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/biosource_autocomplete.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>

#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqloc/Seq_id.hpp>

#include <objmgr/util/sequence.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Spliced_seg.hpp>
#include <objects/seqalign/Spliced_exon.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <algo/align/prosplign/prosplign.hpp>
#include <objtools/readers/source_mod_parser.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <wx/msgdlg.h>
#include <objtools/edit/cds_fix.hpp>


BEGIN_NCBI_SCOPE


static TWizardName s_WizardNames[] = {
    TWizardName(CSourceRequirements::eWizardType_standard, "Standard"),
    TWizardName(CSourceRequirements::eWizardType_viruses, "Viruses"),
    TWizardName(CSourceRequirements::eWizardType_uncultured_samples, "Uncultured Samples"),
    TWizardName(CSourceRequirements::eWizardType_rrna_its_igs, "rRNA-ITS-IGS sequences"),
    TWizardName(CSourceRequirements::eWizardType_tsa, "TSA"),
    TWizardName(CSourceRequirements::eWizardType_igs, "Intergenic Spacer (IGS) sequences"),
    TWizardName(CSourceRequirements::eWizardType_microsatellite, "Microsatellite sequences"),
    TWizardName(CSourceRequirements::eWizardType_d_loop, "D-loops and control regions")
};
static int k_NumWizardNames = sizeof (s_WizardNames) / sizeof (TWizardName);


TWizardNameList GetWizardNameList()
{
    TWizardNameList list;
    list.clear();
    for (int i = 0; i < k_NumWizardNames; i++) {
        list.push_back(s_WizardNames[i]);
    }
    return list;
}


CSourceRequirements::EWizardType GetWizardTypeFromName(string wizard_name)
{
    for (int pos = 0; pos < k_NumWizardNames; pos++) {
        if (NStr::EqualNocase(wizard_name, s_WizardNames[pos].second)) {
            return s_WizardNames[pos].first;
        }
    }
    return CSourceRequirements::eWizardType_standard;
}


/*
// This looks like a copy-paste of RelaxedMatch from srcedit_util. Since we're making it available there, let's remove it here.
static bool RelaxedMatch (CRef<objects::CSeq_id> id1, CRef<objects::CSeq_id> id2)
{
    bool found = false;
    if (id1->IsLocal()) {
        string id1_label = "";
        id1->GetLabel(&id1_label, objects::CSeq_id::eContent);
        string id2_label = "";
        id2->GetLabel(&id2_label, objects::CSeq_id::eContent);
        id2->GetLabel(&id2_label, objects::CSeq_id::eContent);
        size_t pos = NStr::Find (id2_label, id1_label);
        if (pos == 0) {
            if (NStr::Equal(id2_label.substr(id1_label.length(), 1), ".")) {
                found = true;
            } 
        } else if (pos != string::npos && NStr::EndsWith(id2_label, id1_label)) {
            string delim = id2_label.substr(pos - 1, 1);
            if (NStr::Equal(delim, "|") || NStr::Equal(delim, "/") || NStr::Equal(delim, ":")) {
                found = true;
            }
        }
    }
    return found;
}
*/

static bool OneRowOk (CRef<objects::CSeq_id> id, CRef<objects::CSeqTable_column> id_col, objects::CSeq_entry_Handle seh) 
{
    if (!id || !id_col) {
        return false;
    }

    size_t row = 0;
    bool found = false;
    while (row < id_col->GetData().GetSize() && !found) {
        CRef<objects::CSeq_id> row_id = id_col->GetData().GetId()[row];
        objects::CSeq_id::E_SIC compare = id->Compare(*row_id);
        if (compare == objects::CSeq_id::e_YES) {
            found = true;
        } else if (compare == objects::CSeq_id::e_DIFF) {
            if (RelaxedMatch(id, row_id)) {
                found = true;
                id->Assign(*row_id);
            }
        }
        row++;
    }
    row = 0;    
    while (row < id_col->GetData().GetSize() && !found) {
        CRef<objects::CSeq_id> row_id = id_col->GetData().GetId()[row];
        objects::CBioseq_Handle bsh = seh.GetBioseqHandle (*row_id);
        if (bsh) {
            CConstRef<objects::CBioseq> b = bsh.GetCompleteBioseq();
            ITERATE (objects::CBioseq::TId, id_it, b->GetId()) {
                objects::CSeq_id::E_SIC compare = id->Compare(**id_it);
                if (compare == objects::CSeq_id::e_YES) {
                    found = true;
                    id->Assign(*row_id);
                    break;
                } else if (compare == objects::CSeq_id::e_DIFF) {
                    if (RelaxedMatch(id, *id_it)) {
                        found = true;
                        id->Assign(*row_id);
                        break;
                    }
                }
            }
        }
        row++;
    }

    return found;           
}


void FixTableAfterImport (CRef<objects::CSeq_table> input_table)
{
    if (!input_table || !input_table->IsSetColumns() || input_table->GetColumns().size() < 1) {
        return;
    }
   
    bool all_default = true;
    int  col_num = 1;
    ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
        string expected_title = "Column " + NStr::NumericToString(col_num);
        if (!(*cit)->IsSetHeader() || !(*cit)->GetHeader().IsSetTitle()
            || !NStr::Equal(expected_title, (*cit)->GetHeader().GetTitle())) {
            all_default = false;
            break;
        }
        col_num++;
    }                   

    bool all_match = true;
    // make titles
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
        if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
            // no data, skip
        } else {
            // find first value
            string first_val = "";
            if ((*cit)->GetData().IsString()) {
                first_val = (*cit)->GetData().GetString()[0];
            } else if ((*cit)->GetData().IsId()) {
                (*cit)->GetData().GetId()[0]->GetLabel(&first_val, objects::CSeq_id::eContent);
            }
            
            if (all_default || !(*cit)->IsSetHeader() || !(*cit)->GetHeader().IsSetTitle() || NStr::IsBlank((*cit)->GetHeader().GetTitle())) {                
                // set header if missing
                (*cit)->SetHeader().SetTitle(first_val);
            } else if (!NStr::Equal((*cit)->GetHeader().GetTitle(), first_val)) {
                all_match = false;
            }
        } 
    }

    // if all titles match data, delete first row
    if (all_match) {
        DeleteTableRow(input_table, 0);
    }   

    CRef<objects::CSeqTable_column> id_col = FindSeqIDColumn (*input_table);
    if (!id_col) {
        // if an ID column wasn't specified, go look for it
        NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
            if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
                // no data, skip
            } else if ((*cit)->GetData().IsString()) {
                string title = (*cit)->GetHeader().GetTitle();
                NStr::ReplaceInPlace(title, " ", "");
                NStr::ReplaceInPlace(title, "-", "");
                NStr::ReplaceInPlace(title, "_", "");
                if (NStr::EqualNocase(title, "SeqId") || NStr::EqualNocase(title, "SequenceID")) {
                    // title looks right
                    vector<CRef<objects::CSeq_id> > new_ids;
                    for (size_t j = 0; j < (*cit)->GetData().GetString().size(); j++) {
                        string val = (*cit)->GetData().GetString()[j];
                        try {
                            CRef<objects::CSeq_id> id(new objects::CSeq_id(val));
                            new_ids.push_back(id);
                        } catch (exception &) {
                            CRef<objects::CSeq_id> id(new objects::CSeq_id());
                            id->SetLocal().SetStr(val);
                            new_ids.push_back(id);
                        }
                    }
                    for (size_t j = 0; j < new_ids.size(); j++) {
                        (*cit)->SetData().SetId().push_back(new_ids[j]);
                    }
                    // found the ID column, stop looking now
                    break;
                }
            }
        }
    }

    // change numbers to strings
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
        if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
            // skip - no data
        } else if ((*cit)->GetData().IsInt()) {
            vector<string> values;
            for (size_t j = 0; j < (*cit)->GetData().GetInt().size(); j++) {
                values.push_back(NStr::NumericToString((*cit)->GetData().GetInt()[j]));
            }
            for (size_t j = 0; j < values.size(); j++) {
                (*cit)->SetData().SetString().push_back(values[j]);
            }
        } else if ((*cit)->GetData().IsReal()) {
            vector<string> values;
            for (size_t j = 0; j < (*cit)->GetData().GetInt().size(); j++) {
                values.push_back(NStr::NumericToString((*cit)->GetData().GetReal()[j]));
            }
            for (size_t j = 0; j < values.size(); j++) {
                (*cit)->SetData().SetString().push_back(values[j]);
            }
        }
    }


}


string FindBadRows (CRef<objects::CSeq_table> src, CRef<objects::CSeq_table> dst, objects::CSeq_entry_Handle seh)
{
    CRef<objects::CSeqTable_column> src_col = FindSeqIDColumn(*src);
    CRef<objects::CSeqTable_column> dst_col = FindSeqIDColumn(*dst);
    
    if (!src_col) {
        return "No sequence ID column selected!";
    }

    string error = "";    
    size_t row = 0;
    vector<CRef<objects::CSeq_id> > already_seen;
    while (row < src_col->GetData().GetSize()) {
        if (!OneRowOk(src_col->GetData().GetId()[row], dst_col, seh)) {
            string id_label = "";
            src_col->GetData().GetId()[row]->GetLabel(&id_label, objects::CSeq_id::eContent);
            error += "Unable to find " + id_label + " for row " + NStr::NumericToString(row + 1) + "; ";
        } else {
            int as_row = 1;
            ITERATE (vector<CRef<objects::CSeq_id> >, sit, already_seen) {
                if ((*sit)->Compare(*(src_col->GetData().GetId()[row])) == objects::CSeq_id::e_YES) {
                    error += "Row " + NStr::NumericToString(row + 1)
                          + " and row " + NStr::NumericToString(as_row) 
                          + " refer to the same sequence; ";
                }
                as_row++;
            }
            already_seen.push_back(src_col->GetData().GetId()[row]);
        }
        row++;
    }
    if (!NStr::IsBlank(error)) {
        error = error.substr(0, error.length() - 2);
    }
    return error;
}


CRef<objects::CSeqTable_column> FindSeqIDColumn(const objects::CSeq_table& table) 
{
    ITERATE (objects::CSeq_table::TColumns, cit, table.GetColumns()) {
        if ((*cit)->IsSetData() && (*cit)->GetData().IsId()) {
            return *cit;
        }
    }
    CRef<objects::CSeqTable_column> no_col(NULL);
    return no_col;
}


int FindRowForSeqId (CRef<objects::CSeqTable_column> id_col, CRef<objects::CSeq_id> id)
{
    if (!id_col || !id || !id_col->IsSetData() || !id_col->GetData().IsId()) {
        return -1;
    }
    size_t row = 0;
    while (row < id_col->GetData().GetSize()) {
        if (id->Compare(*(id_col->GetData().GetId()[row])) == objects::CSeq_id::e_YES) {
            return row;
        }
        row++;
    }
    return -1;
}


int CountColumnRowConflicts (CRef<objects::CSeqTable_column> dst_id, CRef<objects::CSeqTable_column> dst_col,
                                   CRef<objects::CSeqTable_column> src_id, CRef<objects::CSeqTable_column> src_col)
{
    int count = 0;
    for (size_t i = 0; i < src_id->GetData().GetSize() && i < src_col->GetData().GetSize(); i++) {
        int row = FindRowForSeqId(dst_id, src_id->GetData().GetId()[i]);
        if (row > -1) {
            if (dst_col->GetData().GetString().size() > row
                && !NStr::IsBlank(dst_col->GetData().GetString()[row])
                && !NStr::Equal(dst_col->GetData().GetString()[row], src_col->GetData().GetString()[i])) {
                count++;
            }
        }
    }
    return count;
}


int CountTableColumnConflicts (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src)
{
    CRef<objects::CSeqTable_column> src_id = FindSeqIDColumn(src);
    CRef<objects::CSeqTable_column> dst_id = FindSeqIDColumn(*dst);
    int conflicting_columns = 0;

    // for every column in source, if in dst, combine columns, otherwise add column to dst
    ITERATE (objects::CSeq_table::TColumns, cit, src.GetColumns()) {
        if ((*cit)->IsSetData() && !(*cit)->GetData().IsId()) {
            CRef<objects::CSeqTable_column> dst_col = FindSeqTableColumnByName (dst, (*cit)->GetHeader().GetTitle());
            if (dst_col) {
                if (CountColumnRowConflicts (dst_id, dst_col, src_id, *cit) > 0) {
                    conflicting_columns++;
                }
            }
        }
    }
    return conflicting_columns;
}


void DeleteTableRow (CRef<objects::CSeq_table> table, int row)
{
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, table->SetColumns()) {
        if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
            // no data, skip
        } else {
            int num_vals = (*cit)->GetData().GetSize();
            if (row >= num_vals) {
                continue;
            }
            if ((*cit)->GetData().IsString()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetString()[j] = (*cit)->GetData().GetString()[j + 1];
                }
                (*cit)->SetData().SetString().pop_back();
            } else if ((*cit)->GetData().IsId()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetId()[j]->Assign(*((*cit)->GetData().GetId()[j + 1]));
                }
                (*cit)->SetData().SetId().pop_back();
            } else if ((*cit)->GetData().IsLoc()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetLoc()[j]->Assign(*((*cit)->GetData().GetLoc()[j + 1]));
                }
                (*cit)->SetData().SetLoc().pop_back();
            } else if ((*cit)->GetData().IsInt()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetInt()[j] = (*cit)->GetData().GetInt()[j + 1];
                }
                (*cit)->SetData().SetInt().pop_back();
            } else if ((*cit)->GetData().IsReal()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetReal()[j] = (*cit)->GetData().GetReal()[j + 1];
                }
                (*cit)->SetData().SetReal().pop_back();
            } else if ((*cit)->GetData().IsBit()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetBit()[j] = (*cit)->GetData().GetBit()[j + 1];
                }
                (*cit)->SetData().SetBit().pop_back();
            } else if ((*cit)->GetData().IsBytes()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetBytes()[j] = (*cit)->GetData().GetBytes()[j + 1];
                }
                (*cit)->SetData().SetBytes().pop_back();
            }
        }
    }
}


static void CombineColumns (CRef<objects::CSeqTable_column> dst_id, CRef<objects::CSeqTable_column> dst_col,
                            CRef<objects::CSeqTable_column> src_id, CRef<objects::CSeqTable_column> src_col)
{
    size_t i = 0;

    for (i = 0; i < src_id->GetData().GetSize() && i < src_col->GetData().GetSize(); i++) {
        int row = FindRowForSeqId(dst_id, src_id->GetData().GetId()[i]);
        if (row > -1) {
            while (dst_col->SetData().SetString().size() < dst_id->GetData().GetSize()) {
                dst_col->SetData().SetString().push_back ("");
            }
            try {
                dst_col->SetData().SetString()[row] = src_col->GetData().GetString()[i];
            } catch (exception &) {
                // int x = 0; // compiler warning here, commented out unused "x"
            }
        }
    }
}


// return number of new columns
int CombineTables (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src)
{
    int num_new = 0;
    CRef<objects::CSeqTable_column> src_id = FindSeqIDColumn(src);
    CRef<objects::CSeqTable_column> dst_id = FindSeqIDColumn(*dst);

    // for every column in source, if in dst, combine columns, otherwise add column to dst
    ITERATE (objects::CSeq_table::TColumns, cit, src.GetColumns()) {
        if ((*cit)->IsSetData() && !(*cit)->GetData().IsId()) {
            CRef<objects::CSeqTable_column> dst_col = FindSeqTableColumnByName (dst, (*cit)->GetHeader().GetTitle());
            if (dst_col) {
                // combine columns
            } else {
                dst_col = new objects::CSeqTable_column();
                dst_col->SetHeader().Assign((*cit)->GetHeader());
                if (!dst_col->GetHeader().IsSetTitle() && dst_col->GetHeader().IsSetField_name()) {
                    dst_col->SetHeader().SetTitle(dst_col->GetHeader().GetField_name());
                }
                dst->SetColumns().push_back(dst_col);
                num_new++;
            }
            CombineColumns(dst_id, dst_col, src_id, *cit);
        }
    }
    return num_new;
}


bool SaveTableFile (wxWindow *parent, wxString& save_file_dir, wxString& save_file_name, CRef<objects::CSeq_table> values_table)
{
    wxFileDialog table_save_file(parent, wxT("Select a file"), save_file_dir, save_file_name,
      CFileExtensions::GetDialogFilter(CFileExtensions::kTable) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
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
            return false; 
        }
        ios::openmode mode = ios::out;

        CNcbiOfstream os(path.fn_str(), mode); 
        if (!os)
        {
            wxMessageBox(wxT("Cannot open file ")+name, wxT("Error"),
                         wxOK | wxICON_ERROR, parent);
            return false; 
        }
        
        CCSVExporter exporter(os, '\t', '"');
        ITERATE (objects::CSeq_table::TColumns, cit, values_table->GetColumns()) {
            exporter.Field((*cit)->GetHeader().GetTitle());
        }
        exporter.NewRow();
        for (int i = 0; i < values_table->GetNum_rows(); ++i) {
            ITERATE (objects::CSeq_table::TColumns, cit, values_table->GetColumns()) {
                if (i < (*cit)->GetData().GetSize()) {
                    if ((*cit)->GetData().IsId()) {
                        string label = "";
                        (*cit)->GetData().GetId()[i]->GetLabel(&label, objects::CSeq_id::eContent);
                        exporter.Field(label);
                    } else if ((*cit)->GetData().IsString()) {
                        exporter.Field((*cit)->GetData().GetString()[i]);
                    }
                } else {
                    string blank = "";
                    exporter.Field(blank);
                }
            }
            exporter.NewRow();
        }

        save_file_dir = table_save_file.GetDirectory();
        save_file_name = table_save_file.GetFilename();
        return true;
    } else {
        return false;
    }
}


bool AreAllColumnValuesTheSame(CRef<objects::CSeqTable_column> col, string default_val)
{
    if (!col) {
        return true;
    }
    size_t num_rows = col->GetData().GetSize();
    if (num_rows < 2) {
        return true;
    }
    if (col->GetData().IsString()) {
        string val = col->GetData().GetString()[0];
        if (NStr::IsBlank(val)) {
            val = default_val;
        }
        for (size_t row = 1; row < num_rows; row++) {
            string new_val = col->GetData().GetString()[row];
            if (NStr::IsBlank(new_val)) {
                new_val = default_val;
            }
            if (!NStr::Equal(val, new_val)) {
                return false;
            }
        }
        return true;
    } else if (col->GetData().IsInt()) {
        int val = col->GetData().GetInt()[0];
        for (size_t row = 1; row < num_rows; row++) {
            if (val != col->GetData().GetInt()[row]) {
                return false;
            }
        }
        return true;
    }
    // TODO: implement other data types
    return false;
}


bool AreAnyColumnValuesMissing(CRef<objects::CSeqTable_column> col)
{
    if (!col || !col->IsSetData()) {
        return true;
    }
    size_t num_rows = col->GetData().GetSize();
    if (num_rows < 1) {
        return true;
    }
    if (col->GetData().IsString()) {
        for (size_t row = 0; row < num_rows; row++) {
            string new_val = col->GetData().GetString()[row];
            if (NStr::IsBlank(new_val)) {
                return true;
            }
        }
    }
    // TODO: implement other data types
    return false;
}


bool DoesColumnHaveValue(CRef<objects::CSeqTable_column> col, string val)
{
    if (!col || !col->IsSetData()) {
        return true;
    }
    size_t num_rows = col->GetData().GetSize();
    if (num_rows < 1) {
        return true;
    }
    if (col->GetData().IsString()) {
        for (size_t row = 0; row < num_rows; row++) {
            string new_val = col->GetData().GetString()[row];
            if (NStr::Equal(new_val, val)) {
                return true;
            }
        }
    }
    // TODO: implement other data types
    return false;
}


bool AreAnyColumnValuesPresent(CRef<objects::CSeqTable_column> col, string default_val)
{
    if (!col || !col->IsSetData()) {
        return false;
    }
    size_t num_rows = col->GetData().GetSize();
    if (num_rows < 1) {
        return false;
    }
    if (col->GetData().IsString()) {
        for (size_t row = 0; row < num_rows; row++) {
            string new_val = col->GetData().GetString()[row];
            if (!NStr::EqualNocase(new_val, default_val) && !NStr::IsBlank(new_val)) {
                return true;
            }
        }
    }
    // TODO: implement other data types
    return false;
}


static void s_ExtendIntervalToEnd (objects::CSeq_interval& ival, objects::CBioseq_Handle bsh)
{
    if (ival.IsSetStrand() && ival.GetStrand() == objects::eNa_strand_minus) {
        if (ival.GetFrom() > 3) {
            ival.SetFrom(ival.GetFrom() - 3);
        } else {
            ival.SetFrom(0);
        }
    } else {
        size_t len = bsh.GetBioseqLength();
        if (ival.GetTo() < len - 4) {
            ival.SetTo(ival.GetTo() + 3);
        } else {
            ival.SetTo(len - 1);
        }
    }
}


bool AddProteinToSeqEntry(const objects::CSeq_entry* protein, objects::CSeq_entry_Handle seh, CRef<CCmdComposite> cmd, bool create_general_only)
{
    CProSplign prosplign(CProSplignScoring(), false, true, false, false);

    // bool rval = false;
    objects::CBioseq_Handle bsh_match;
    bool id_match = false;

    // only add protein if we can match it to a nucleotide sequence via the ID,
    // or if there is only one nucleotide sequence

    ITERATE (objects::CBioseq::TId, id_it, protein->GetSeq().GetId()) {
        bsh_match = seh.GetScope().GetBioseqHandle(**id_it);
        if (bsh_match) {
            id_match = true;
            break;
        }
    }
    if (!bsh_match) {
        // if there is only one nucleotide sequence, we will use that one
        int nuc_count = 0;
        for (objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na); b_iter ; ++b_iter ) {
            bsh_match = *b_iter;
            nuc_count++;
            if (nuc_count > 1) {
                break;
            }
        }

        if (nuc_count == 0) {
            wxMessageBox(wxT("You must import nucleotide sequences before importing protein sequences"), wxT("Error"),
                         wxOK | wxICON_ERROR, NULL);
            return false;
        } else if (nuc_count > 1) {
            wxMessageBox(wxT("If you have more than one nucleotide sequence, each protein sequence must use the ID of the nucleotide sequence where the coding region is found."), wxT("Error"),
                         wxOK | wxICON_ERROR, NULL);
            return false;
        }
    }
            
    CRef<objects::CSeq_id> seq_id(new objects::CSeq_id());
    seq_id->Assign(*(bsh_match.GetSeqId()));
    CRef<objects::CSeq_loc> match_loc(new objects::CSeq_loc(*seq_id, 0, bsh_match.GetBioseqLength() - 1));

    CRef<objects::CSeq_entry> protein_entry(new objects::CSeq_entry());
    protein_entry->Assign(*protein);
    if (id_match) {
        int offset = 1;
        string id_label;
        CRef<objects::CSeq_id> product_id = objects::edit::GetNewProtId(bsh_match, offset, id_label, create_general_only); 
        protein_entry->SetSeq().ResetId();
        protein_entry->SetSeq().SetId().push_back(product_id);
    }

    objects::CSeq_entry_Handle protein_h = seh.GetScope().AddTopLevelSeqEntry(*protein_entry);

    //time_t t1 = time(NULL);
    CRef<objects::CSeq_align> alignment = prosplign.FindAlignment(seh.GetScope(), *protein_entry->GetSeq().GetId().front(), *match_loc,
                                                     CProSplignOutputOptions(CProSplignOutputOptions::ePassThrough));
    //time_t t2 = time(NULL);
    //time_t elapsed = t2 - t1;
    CRef<objects::CSeq_loc> cds_loc(new objects::CSeq_loc());
    bool found_start_codon = false;
    bool found_stop_codon = false;
    if (alignment && alignment->IsSetSegs() && alignment->GetSegs().IsSpliced()) {
        CRef<objects::CSeq_id> seq_id (new objects::CSeq_id());
        seq_id->Assign(*match_loc->GetId());
        ITERATE (objects::CSpliced_seg::TExons, exon_it, alignment->GetSegs().GetSpliced().GetExons()) {
            CRef<objects::CSeq_loc> exon(new objects::CSeq_loc(*seq_id, 
                                                      (*exon_it)->GetGenomic_start(), 
                                                      (*exon_it)->GetGenomic_end()));                
            if ((*exon_it)->IsSetGenomic_strand()) {
                exon->SetStrand((*exon_it)->GetGenomic_strand());
            }
            cds_loc->SetMix().Set().push_back(exon);
        }
        ITERATE (objects::CSpliced_seg::TModifiers, mod_it,
                 alignment->GetSegs().GetSpliced().GetModifiers()) {
            if ((*mod_it)->IsStart_codon_found()) {
                found_start_codon = (*mod_it)->GetStart_codon_found();
            }
            if ((*mod_it)->IsStop_codon_found()) {
                found_stop_codon = (*mod_it)->GetStop_codon_found();
            }
        }
        
    }
    if (!cds_loc->IsMix()) {
        //no exons, no match
        string label = "";        
        protein->GetSeq().GetId().front()->GetLabel(&label, objects::CSeq_id::eContent);
        string error = "Unable to find coding region location for protein sequence " + label + ".  Import failed.";
        wxMessageBox(ToWxString(error), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return false;
    } else {
        if (cds_loc->GetMix().Get().size() == 1) {
            CRef<objects::CSeq_loc> exon = cds_loc->SetMix().Set().front();
            cds_loc->Assign(*exon);
        }
    }
    if (!found_start_codon) {
        cds_loc->SetPartialStart(true, objects::eExtreme_Biological);
    }
    if (found_stop_codon) {
        // extend to cover stop codon        
        if (cds_loc->IsMix()) {
            s_ExtendIntervalToEnd(cds_loc->SetMix().Set().back()->SetInt(), bsh_match);
        } else {
            s_ExtendIntervalToEnd(cds_loc->SetInt(), bsh_match);
        }        
    } else {
        cds_loc->SetPartialStop(true, objects::eExtreme_Biological);
    }

    // if we add the protein sequence, we'll do it in the new nuc-prot set
    seh.GetScope().RemoveTopLevelSeqEntry(protein_h);
    bool partial5 = cds_loc->IsPartialStart(objects::eExtreme_Biological);
    bool partial3 = cds_loc->IsPartialStop(objects::eExtreme_Biological);
    SetMolinfoForProtein(protein_entry, partial5, partial3);
    AddProteinFeatureToProtein(protein_entry, partial5, partial3);

    CRef<objects::CCmdAddSeqEntry> add_seqentry(new objects::CCmdAddSeqEntry(protein_entry, bsh_match.GetParentEntry()));
    cmd->AddCommand(*add_seqentry);
    CRef<objects::CSeq_feat> new_cds(new objects::CSeq_feat());
    new_cds->SetLocation(*cds_loc);
    if (partial5 || partial3) {
        new_cds->SetPartial(true);
    }
    new_cds->SetData().SetCdregion();
    CRef<objects::CSeq_id> product_id(new objects::CSeq_id());
    product_id->Assign(*(protein_entry->GetSeq().GetId().front()));
    new_cds->SetProduct().SetWhole(*product_id);
    CRef<CCmdCreateFeat> add_cds(new CCmdCreateFeat(seh, *new_cds));
    cmd->AddCommand(*add_cds);

    return true;
}


static void s_ReportMixError()
{
    wxMessageBox(wxT("Cannot import a mix of protein and nucleotide sequences unless proteins are already packaged in nuc-prot sets"), wxT("Error"),
             wxOK | wxICON_ERROR, NULL);
}


void ParseTitlesToNewSeqEntries(CRef<objects::CSeq_entry> entry)
{
    if (!entry) {
        return;
    }
    if (entry->IsSet()) {
        NON_CONST_ITERATE(objects::CBioseq_set::TSeq_set, it, entry->SetSet().SetSeq_set()) {
            ParseTitlesToNewSeqEntries(*it);
        }
    } else if (entry->IsSeq()) {
        if (!entry->GetSeq().IsAa()) {
            
            objects::CSourceModParser smp(  
              objects::CSourceModParser::eHandleBadMod_Ignore );
            // later - fix title by removing attributes used?
            CConstRef<objects::CSeqdesc> title_desc
                = entry->GetSeq().GetClosestDescriptor(objects::CSeqdesc::e_Title);
            if (title_desc) {
                string& title(const_cast<string&>(title_desc->GetTitle()));
                title = smp.ParseTitle(title, CConstRef<objects::CSeq_id>(entry->GetSeq().GetFirstId()) );
                smp.ApplyAllMods(entry->SetSeq());
            }
        }
    }
}


bool AddSeqEntryToSeqEntry(const objects::CSeq_entry* entry, objects::CSeq_entry_Handle seh, CRef<CCmdComposite> cmd, bool& has_nuc, bool& has_prot, bool create_general_only)
{
    if (!entry) {
        return false;
    }
    if (entry->IsSeq()) {
        if (entry->GetSeq().IsAa()) {
            if (has_nuc) {
                s_ReportMixError();
                return false;
            } else if (!AddProteinToSeqEntry (entry, seh, cmd, create_general_only)) {
                return false;
            }
            has_prot = true;
        } else {
            if (has_prot) {
                s_ReportMixError();
                return false;
            }                
            CRef<objects::CSeq_entry> new_entry(new objects::CSeq_entry());
            new_entry->Assign(*entry);
            ParseTitlesToNewSeqEntries(new_entry);
            CRef<objects::CCmdAddSeqEntry> subcmd(new objects::CCmdAddSeqEntry(new_entry, seh));
            cmd->AddCommand(*subcmd);
            has_nuc = true;
        }
    } else if (entry->IsSet()) {
        if (entry->GetSet().GetClass() == objects::CBioseq_set::eClass_nuc_prot) {
            if (has_prot) {
                s_ReportMixError();
                return false;
            }
            CRef<objects::CSeq_entry> new_entry(new objects::CSeq_entry());
            new_entry->Assign(*entry);
            ParseTitlesToNewSeqEntries(new_entry);
            CRef<objects::CCmdAddSeqEntry> subcmd(new objects::CCmdAddSeqEntry(new_entry, seh));
            cmd->AddCommand(*subcmd);
            has_nuc = true;
        } else {
            if (seh.IsSet() && seh.GetSet().IsEmptySeq_set() && entry->GetSet().IsSetClass()) {
                CRef<objects::CBioseq_set> new_set(new objects::CBioseq_set());
                new_set->Assign(*(seh.GetSet().GetCompleteBioseq_set()));
                new_set->SetClass(entry->GetSet().GetClass());
                CCmdChangeBioseqSet *set_cmd = new CCmdChangeBioseqSet(seh.GetSet(), *new_set);
                cmd->AddCommand(*set_cmd);
            }
            if (seh.IsSet() && seh.GetSet().IsEmptySeq_set() && entry->GetSet().IsSetDescr()) {
                ITERATE (objects::CBioseq_set::TDescr::Tdata, dit, entry->GetSet().GetDescr().Get()) {
                    CRef<objects::CSeqdesc> desc(new objects::CSeqdesc());
                    desc->Assign(**dit);
                    CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(seh, *desc));
                    cmd->AddCommand(*cmdAddDesc);
                }
            }  

            ITERATE (objects::CBioseq_set::TSeq_set, it, entry->GetSet().GetSeq_set()) {
                CRef<objects::CSeq_entry> new_entry(new objects::CSeq_entry());
                new_entry->Assign(**it);
                if (entry->GetSet().IsSetDescr()) {
                    ITERATE (objects::CBioseq_set::TDescr::Tdata, dit, entry->GetSet().GetDescr().Get()) {
                        CRef<objects::CSeqdesc> desc(new objects::CSeqdesc());
                        desc->Assign(**dit);
                        new_entry->SetDescr().Set().push_back(desc);
                    }
                }
                if (!AddSeqEntryToSeqEntry(new_entry, seh, cmd, has_nuc, has_prot, create_general_only)) {
                    return false;
                }
            }
        }
    }   
    return true;            
}


string CheckFeatureAnnotation (objects::CSeq_entry_Handle entry, bool& is_ok)
{
    vector<int> counts;
    counts.clear();
    bool all = true;
    bool any = false;
    int  num_with = 0;
    int  common_num = -1;
    bool same_num = true;
    is_ok = true;

    objects::CBioseq_CI b_iter(entry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        int this_count = 0;
        objects::CFeat_CI fit (*b_iter);
        while (fit) {
            this_count++;
            ++fit;
            any = true;
        }
        counts.push_back(this_count);
        if (this_count == 0) {
            all = false;
        } else {
            num_with ++;
        }
        if (common_num == -1) {
            common_num = this_count;
        } else if (common_num != this_count) {
            same_num = false;
        }
    }
    string summary = "";
    if (any) {
        if (all && same_num) {
            summary = "All " + NStr::NumericToString(num_with) + " sequences have " + NStr::NumericToString(common_num) + " features.";
        } else {
            vector<int>::iterator cit = counts.begin(); 
            while (cit != counts.end()) {
                int this_num = 1;
                vector<int>::iterator cit2 = cit;
                cit2++;
                while (cit2 != counts.end()) {
                    if (*cit == *cit2) {
                        this_num++;
                        cit2 = counts.erase(cit2);
                    } else {
                        cit2++;
                    }
                }
                summary += NStr::NumericToString(this_num) + " sequence";
                if (this_num == 1) {
                    summary += " has ";
                } else {
                    summary += "s have ";
                }
                summary += NStr::NumericToString(*cit) + " feature";
                if (*cit != 1) {
                    summary += "s";
                }
                summary += ".\n";
                cit++;
            }
        }
    } else {
        summary = "No features found.";
    }
    if (!all) {
        is_ok = false;
    }
    return summary;
}

vector<string> GetTrueFalseList()
{
    vector<string> tf_strings;
    tf_strings.push_back("true");
    tf_strings.push_back("");
    return tf_strings;
}

bool IsSynonymForTrue(const string& val)
{
    if (NStr::EqualNocase(val, "true") 
        || NStr::EqualNocase(val, "yes") 
        || NStr::EqualNocase(val, "T")) {
        return true;
    } else {
        return false;
    }
}


bool IsSynonymForFalse(const string& val)
{
    if (NStr::EqualNocase(val, "false") 
        || NStr::EqualNocase(val, "no") 
        || NStr::EqualNocase(val, "F")
        || NStr::IsBlank(val)) {
        return true;
    } else {
        return false;
    }
}


bool IsTrueFalseList(const vector<string>& choices)
{
    if (choices.size() != 2) {
        return false;
    }
    if (IsSynonymForTrue(choices[0]) && IsSynonymForFalse(choices[1])) {
        return true;
    } else if (IsSynonymForTrue(choices[1]) && IsSynonymForFalse(choices[0])) {
        return true;
    } else {
        return false;
    }
}


int AddFeatureToSeqTable (const objects::CSeq_feat& f, CRef<objects::CSeq_table> table)
{
    int row = 0;
    if (table->GetColumns().front()->IsSetData()) {
        row = table->GetColumns().front()->GetData().GetId().size();
    }

    CRef<objects::CSeq_id> id(new objects::CSeq_id());
    id->Assign(*f.GetLocation().GetId());
    table->SetColumns()[0]->SetData().SetId().push_back(id);
    AddValueToTable(table, kStartColLabel, f.GetLocation().GetStart(objects::eExtreme_Biological) + 1, row);
    AddValueToTable(table, kStopColLabel, f.GetLocation().GetStop(objects::eExtreme_Biological) + 1, row);
    if (f.IsSetId() && f.GetId().IsLocal() && f.GetId().GetLocal().IsId()) {
        AddValueToTable(table, kFeatureID, NStr::NumericToString(f.GetId().GetLocal().GetId()), row);
    }

    if (f.GetLocation().IsPartialStart(objects::eExtreme_Biological)) {
        AddValueToTable(table, kPartialStart, "true", row);
    }
    if (f.GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
        AddValueToTable(table, kPartialStop, "true", row);
    }
    if (f.IsSetComment()) {
        AddValueToTable(table, "Comment", f.GetComment(), row);
    }
    if (f.IsSetExcept() && f.IsSetExcept_text()) {
        AddValueToTable(table, "Exception", f.GetExcept_text(), row);
    }
    if (f.IsSetQual()) {
        ITERATE(objects::CSeq_feat::TQual, it, f.GetQual()) {
            if ((*it)->IsSetQual() && (*it)->IsSetVal()) {
                string qual = (*it)->GetQual();
                string val = (*it)->GetVal();
                // special case
                if (NStr::EqualNocase(qual, "satellite")) {
                    for (int i = 0; i < kNumSatelliteTypes; i++) {
                        if (NStr::StartsWith(val, kSatelliteTypes[i])) {
                            qual = kSatelliteTypes[i] + " name";
                            val = val.substr(kSatelliteTypes[i].length());
                            if (NStr::StartsWith(val, ":")) {
                                val = val.substr(1);
                            }
                            NStr::TruncateSpacesInPlace(val);
                            break;
                        }
                    }
                }
                AddValueToTable(table, qual, val, row);
            }
        }
    }    
    table->SetNum_rows(row + 1);
    return row;
}


static bool s_OkToAddFeature (const objects::CSeq_feat& f1, const objects::CSeq_feat& f2, const TFeatureSeqTableColumnList& reqs)
{
    bool ok = true;

    ITERATE(TFeatureSeqTableColumnList, it, reqs) {
        if (!(*it)->RelaxedMatch(f1, f2)) {
            ok = false;
            break;
        }
    }
    return ok;
}


CRef<objects::CSeq_table> GetFeaturesFromSeqEntry(objects::CSeq_entry_Handle entry, const objects::CSeq_feat& feat, const TFeatureSeqTableColumnList& reqs)
{
    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);
    AddStringColumnToTable(table, kFeatureID);
    AddIntColumnToTable(table, kStartColLabel);
    AddIntColumnToTable(table, kStopColLabel);
    objects::CFeat_CI fi(entry, objects::SAnnotSelector(feat.GetData().GetSubtype()));
    while (fi) {
        if (s_OkToAddFeature(feat, *(fi->GetSeq_feat()), reqs)) {
            AddFeatureToSeqTable (*(fi->GetSeq_feat()), table);
        }
        ++fi;
    }        

    return table;
}


void 
AddFeatureSeqTableRowToSeqEntry
(CRef<objects::CSeq_table> table,
 objects::CSeq_entry_Handle entry,
 objects::CSeqFeatData::ESubtype subtype,
 unsigned int row,
 TFeatureSeqTableColumnList & vecColEditFactories,
 CRef<CCmdComposite> cmd)
{
    CRef<objects::CSeq_feat> feat(new objects::CSeq_feat());

    CRef<objects::CSeqTable_column> start_col = FindSeqTableColumnByName (table, kStartColLabel);
    CRef<objects::CSeqTable_column> stop_col = FindSeqTableColumnByName (table, kStopColLabel);
    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName (table, kSequenceIdColLabel);
    CRef<objects::CSeqTable_column> featid_col = FindSeqTableColumnByName (table, kFeatureID);

    if (featid_col && featid_col->IsSetData() && featid_col->GetData().GetSize() > row) {
        string feature_id = featid_col->GetData().GetString()[row];
        if (!NStr::IsBlank(feature_id)) {
            try {
                int id = NStr::StringToInt(feature_id);
                CRef<objects::CFeat_id> feat_id(new objects::CFeat_id());
                
                objects::CSeq_feat_Handle orig_feat = entry.GetTopLevelEntry().GetTSE_Handle().GetFeatureWithId(objects::CSeqFeatData::e_not_set,
                                                          id);
                if (orig_feat) {
                    feat->Assign(*(orig_feat.GetOriginalSeq_feat()));
                    feat->ResetQual();
                } else {
                    feat->SetId().SetLocal().SetId(id);
                }
            } catch (exception &) {
            }
        }
    }

    feat->SetLocation().SetInt().SetId().Assign(*(id_col->GetData().GetId()[row]));
    int start = start_col->GetData().GetInt()[row];
    int stop = stop_col->GetData().GetInt()[row];
    if (stop < start) {
        feat->SetLocation().SetInt().SetFrom(stop - 1);
        feat->SetLocation().SetInt().SetTo(start - 1);
        feat->SetLocation().SetInt().SetStrand(objects::eNa_strand_minus);
    } else {
        feat->SetLocation().SetInt().SetFrom(start - 1);
        feat->SetLocation().SetInt().SetTo(stop - 1);
        feat->SetLocation().SetInt().SetStrand(objects::eNa_strand_plus);
    }

    for (unsigned int i = kSkipColumns; i < table->GetColumns().size(); i++) {
        CConstRef<objects::CSeqTable_column> col = table->GetColumns()[i];
        if (vecColEditFactories[i - kSkipColumns] != NULL
            && col->GetData().GetSize() > row) {
            if (col->GetData().IsString()) {
                vecColEditFactories[i - kSkipColumns]->AddToFeature(*feat, col->GetData().GetString()[row], edit::eExistingText_replace_old);
            } else if (col->GetData().IsInt()) {
                vecColEditFactories[i - kSkipColumns]->AddToFeature(*feat, NStr::NumericToString(col->GetData().GetInt()[row]), edit::eExistingText_replace_old);
            } else {
                wxMessageBox(wxT("Unrecognized table column type"), wxT("Error"),
                             wxOK | wxICON_ERROR, NULL);
            }
        }
    }

    objects::CSeqFeatData::E_Choice feature_type = objects::CSeqFeatData::GetTypeFromSubtype(subtype);
    switch (feature_type) {
        case objects::CSeqFeatData::e_Imp:
          {
              const objects::CFeatList& feats(*objects::CSeqFeatData::GetFeatList());
              string key = feats.GetStoragekey(subtype);
              feat->SetData().SetImp().SetKey(key);
          }
          break;
        case objects::CSeqFeatData::e_Cdregion:
            feat->SetData().SetCdregion();
            break;
        default:
            break;
    }

    bool change_existing = false;
    if (feat->IsSetId() && feat->GetId().IsLocal() && feat->GetId().GetLocal().IsId()) {
        objects::CSeq_feat_Handle orig_feat = entry.GetTopLevelEntry().GetTSE_Handle().GetFeatureWithId(objects::CSeqFeatData::e_not_set,
                                                          feat->GetId().GetLocal().GetId());
        if (orig_feat) {
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(orig_feat, *feat)));
            change_existing = true;
        } 
    }
    if (!change_existing) {
        objects::CBioseq_Handle bh = entry.GetScope().GetBioseqHandle(*(id_col->GetData().GetId()[row]));
        objects::CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *feat)));
    }
}


CRef<CCmdComposite> AddFeatureSeqTableToSeqEntry(CRef<objects::CSeq_table> table, objects::CSeq_entry_Handle entry, objects::CSeqFeatData::ESubtype subtype, const TFeatureSeqTableColumnList& reqs, const TFeatureSeqTableColumnList& opts)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Add Features"));

    TFeatureSeqTableColumnList vecColEditFactories;

    // the first kSkipColumns columns are not associated with column handlers
    for (size_t i = kSkipColumns; i < table->GetColumns().size(); i++) {
        vecColEditFactories.push_back( 
            GetColumnRuleForFeatureSeqTable(table->GetColumns()[i], reqs, opts));
    }

    for (size_t c = 0; c < table->GetColumns().front()->GetData().GetSize(); c++) {
        AddFeatureSeqTableRowToSeqEntry (table, entry, subtype, c, vecColEditFactories, cmd);
    }
    return cmd;
}


void MergeStringVectors (vector<string>& problems, vector<string> add)
{
    if (add.empty()) {
        return;
    } else if (problems.empty()) {
        problems.assign(add.begin(), add.end());
    } else {
        size_t pos = 0;
        while (pos < problems.size() && pos < add.size()) {
            if (!NStr::IsBlank(add[pos])) {
                if (!NStr::IsBlank(problems[pos])) {
                    problems[pos] += ", ";
                }
                problems[pos] += add[pos];
            }
            pos++;
        }
        while (pos < add.size()) {
            problems.push_back(add[pos]);
            pos++;
        }
    }
}


static void AddProblemsToColumn 
(CRef<objects::CSeqTable_column> val_col,
 CRef<CFeatureSeqTableColumnBase> rule_col,
 vector<string>& problems)
{
    vector<string> add_values;
    for (size_t j = 0; j < val_col->GetData().GetSize(); j++) {
        add_values.push_back(val_col->GetData().GetString()[j]);
    }
    vector<string> add_problems = rule_col->IsValid(add_values);
    MergeStringVectors(problems, add_problems);
}


void RemoveProblemsColumn (CRef<objects::CSeq_table> values_table)
{
    int num_table_columns = values_table->GetColumns().size();
    if (num_table_columns > 1) {
        CRef<objects::CSeqTable_column> last_col = values_table->GetColumns()[num_table_columns - 1];
        if (!last_col->IsSetHeader()
            || !last_col->GetHeader().IsSetTitle()
            || NStr::Equal(last_col->GetHeader().GetTitle(), kProblems)) {
            values_table->SetColumns().pop_back();
        }
    }   
}


CRef<CFeatureSeqTableColumnBase> GetColumnRuleForFeatureSeqTable(CRef<objects::CSeqTable_column> col, const TFeatureSeqTableColumnList& reqs, const TFeatureSeqTableColumnList& opts)
{
    ITERATE (TFeatureSeqTableColumnList, col_it, reqs) {
        if (MatchColumnName((*col_it)->GetLabel(), col->GetHeader().GetTitle())) {
            return *col_it;
        }
    }
    ITERATE (TFeatureSeqTableColumnList, col_it, opts) {
        if (MatchColumnName((*col_it)->GetLabel(), col->GetHeader().GetTitle())) {
            return *col_it;
        }
    }
    CRef<CFeatureSeqTableColumnBase> rule_col = CFeatureSeqTableColumnBaseFactory::Create(col->GetHeader().GetTitle());
    return rule_col;
}


void AddProblemsColumnToFeatureSeqTable(CRef<objects::CSeq_table> table, const TFeatureSeqTableColumnList& reqs, const TFeatureSeqTableColumnList& opts)
{
    vector<string> problems;

    // the first kSkipColumns columns are not associated with column handlers
    for (size_t i = kSkipColumns; i < table->GetColumns().size(); i++) {        
        if (table->GetColumns()[i]->IsSetData()
            && (table->GetColumns()[i]->GetData().IsString())) {
            CRef<CFeatureSeqTableColumnBase> col = GetColumnRuleForFeatureSeqTable(table->GetColumns()[i], reqs, opts);
            if (col) {
                AddProblemsToColumn(table->GetColumns()[i], col, problems);
            }
        }
    }

    RemoveProblemsColumn(table);
    CRef<objects::CSeqTable_column> problems_col(new objects::CSeqTable_column());
    problems_col->SetHeader().SetTitle(kProblems);
    problems_col->SetData().SetString();
    table->SetColumns().push_back(problems_col);
    ITERATE(vector<string>, it, problems) {
        problems_col->SetData().SetString().push_back(*it);
    }
}


bool HasLocalIntFeatureId (const objects::CSeq_feat& feat)
{
    if (feat.IsSetId() && feat.GetId().IsLocal() && feat.GetId().GetLocal().IsId()) {
        return true;
    } else {
        return false;
    }
}


CRef<CCmdComposite> RemoveMatchingFeaturesWithoutLocalIntFeatureIdsFromSeqEntry(objects::CSeq_entry_Handle entry, const objects::CSeq_feat &feat, const TFeatureSeqTableColumnList& reqs)
{
    CRef<CCmdComposite> cmd (new CCmdComposite("Remove Features"));

    objects::CFeat_CI fi(entry, objects::SAnnotSelector(feat.GetData().GetSubtype()));
    while (fi) {
        const objects::CSeq_feat& this_feat= *(fi->GetSeq_feat());
        if (!HasLocalIntFeatureId(this_feat) && s_OkToAddFeature(this_feat, feat, reqs)) {
            cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fi->GetSeq_feat_Handle())));
        }        
        ++fi;
    }
    return cmd;
}


wxString GetAsnSqnExtensions()
{
    vector<wxString> exts;
    exts.push_back(wxT("asn")); 
    exts.push_back(wxT("sqn")); 
    wxString filter;
    vector<wxString>::const_iterator it;
    for (it = exts.begin(); it != exts.end(); it++) {
        if (it != exts.begin())
            filter += wxT(";");
        
#ifdef NCBI_OS_MSWIN
        filter += wxT("*.") + *it;
#else
        if (*it == "*")   
            filter += *it;
        else
            filter += wxT("*.") + *it;
#endif
    }
    return filter;
}


bool IsWizardObject(const objects::CUser_object& user)
{
    if (!user.IsSetType() || !user.GetType().IsStr() || !NStr::EqualNocase(user.GetType().GetStr(), kWizardLabel)) {
        return false;
    } else {
        return true;
    }
}


CRef<objects::CUser_object> MakeWizardObject ()
{
    CRef<objects::CUser_object> user(new objects::CUser_object());
    user->SetType().SetStr(kWizardLabel);
    return user;
}


void SetWizardFieldInSeqEntryNoUndo(objects::CSeq_entry_Handle entry, string field_name, string value)
{
    bool found = false;

    objects::CSeq_entry_EditHandle eh = entry.GetEditHandle();
    
    for (objects::CSeqdesc_CI desc_ci( entry, objects::CSeqdesc::e_User);
         desc_ci;
         ++desc_ci) {
        const objects::CUser_object& u = desc_ci->GetUser();
        if (IsWizardObject (u)) {
            CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
            CRef<objects::CUser_object> new_user (new objects::CUser_object());
            new_user->Assign (u);
            new_desc->SetUser(*new_user);
            objects::CUser_field& new_field = new_user->SetField(field_name);
            new_field.SetData().SetStr(value);
            eh.RemoveSeqdesc(*desc_ci);
            eh.AddSeqdesc(*new_desc);
            found = true;
            break;
        }            
    }
    if (!found) {
        CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
        CRef<objects::CUser_object> new_user = MakeWizardObject();
        new_desc->SetUser(*new_user);
        objects::CUser_field& new_field = new_user->SetField(field_name);
        new_field.SetData().SetStr(value);
        entry.GetEditHandle().SetDescr().Set().push_back(new_desc);
    }   
}


const string kBankitSubmissionLabel = "Submission";
const string kBankItAltEmailPrefix = "ALT EMAIL:";

bool IsBankItSubmissionObject(const objects::CUser_object& user)
{
    if (!user.IsSetType() || !user.GetType().IsStr() || !NStr::EqualNocase(user.GetType().GetStr(), kBankitSubmissionLabel)) {
        return false;
    } else {
        return true;
    }
}


CRef<objects::CUser_object> MakeBankItSubmissionObject ()
{
    CRef<objects::CUser_object> user(new objects::CUser_object());
    user->SetType().SetStr(kBankitSubmissionLabel);
    return user;
}


string GetFieldFromWizardObject (const objects::CUser_object& user, string field_name)
{
    string value = "";
    ITERATE(objects::CUser_object::TData, fit, user.GetData()) {
        if ((*fit)->IsSetLabel() 
            && (*fit)->GetLabel().IsStr() 
            && NStr::EqualNocase((*fit)->GetLabel().GetStr(), field_name)
            && (*fit)->IsSetData()
            && (*fit)->GetData().IsStr()) {
            value = (*fit)->GetData().GetStr();
            break;            
        }
    }
    return value;
}


const string kSMARTComment = "SmartComment";

string GetDescAlternateEmailAddress(const objects::CSeqdesc& desc)
{
    string alt_email = "";
    if (desc.IsUser() && IsBankItSubmissionObject(desc.GetUser())) {
        string str = GetFieldFromWizardObject(desc.GetUser(), kSMARTComment);
        size_t pos = NStr::Find(str, kBankItAltEmailPrefix);
        if (pos != string::npos) {
            alt_email = str.substr(pos + kBankItAltEmailPrefix.length());
        }
    }
    return alt_email;
}


string GetAlternateEmailAddress(objects::CSeq_entry_Handle entry)
{
    string alt_email = "";
    objects::CSeqdesc_CI it (entry, objects::CSeqdesc::e_User);

    while (it && NStr::IsBlank(alt_email)) {
        alt_email = GetDescAlternateEmailAddress(*it);
        ++it;
    }
    return alt_email;
}


void SetUserAlternateEmailAddress(objects::CUser_object& u, string alt_email)
{
    if (!NStr::StartsWith(alt_email, kBankItAltEmailPrefix, NStr::eNocase)) {
        alt_email = kBankItAltEmailPrefix + alt_email;
    }
    objects::CUser_field& new_field = u.SetField(kSMARTComment);
    new_field.SetData().SetStr(alt_email);
}


void SetAlternateEmailAddress(objects::CSeq_entry_Handle entry, string alt_email)
{
    bool found = false;
    objects::CSeq_entry_EditHandle eh = entry.GetEditHandle();
    for (objects::CSeqdesc_CI desc_ci( entry, objects::CSeqdesc::e_User);
         desc_ci;
         ++desc_ci) {
        const objects::CUser_object& u = desc_ci->GetUser();
        if (IsBankItSubmissionObject (u)) {
            CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
            CRef<objects::CUser_object> new_user (new objects::CUser_object());
            new_user->Assign (u);
            new_desc->SetUser(*new_user);
            SetUserAlternateEmailAddress(*new_user, alt_email);
            eh.RemoveSeqdesc(*desc_ci);
            eh.AddSeqdesc(*new_desc);
            found = true;
            break;
        }            
    }
    if (!found) {
        CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
        CRef<objects::CUser_object> new_user = MakeBankItSubmissionObject();
        new_desc->SetUser(*new_user);
        SetUserAlternateEmailAddress(*new_user, alt_email);
        entry.GetEditHandle().SetDescr().Set().push_back(new_desc);
    }   
}


bool IsValidEmail(string email)
{
    if (NStr::IsBlank(email)) {
        return false;
    }
    size_t pos = NStr::Find(email, "@");
    if (pos == string::npos) {
        return false;
    }
    string tmp = email.substr(pos);
    pos = NStr::Find(tmp, ".");
    if (pos == string::npos) {
        return false;
    } else {
        return true;
    }
}


static void AddIdsFromSeqEntryToTable
(const objects::CSeq_entry& entry, 
 CRef<objects::CSeqTable_column> id_col, 
 CRef<objects::CSeqTable_column> pos_col,
 CRef<objects::CSeqTable_column> np_col,
 CRef<objects::CSeqTable_column> title_col,
 int& pos,
 size_t& row)
{
    if (entry.IsSeq()) {
        string np = "nuc";
        if (entry.GetSeq().IsAa()) {
            np = "prot";
        }
        string title = "";
        if (entry.GetSeq().IsSetDescr()) {
            ITERATE (objects::CBioseq::TDescr::Tdata, desc_it, entry.GetSeq().GetDescr().Get()) {
                if ((*desc_it)->IsTitle()) {
                    title = (*desc_it)->GetTitle();
                    break;
                }
            }
        }
        ITERATE(objects::CBioseq::TId, id_it, entry.GetSeq().GetId()) {
            CRef<objects::CSeq_id> id(new objects::CSeq_id());
            id->Assign(**id_it);
            id_col->SetData().SetId().push_back(id);
            pos_col->SetData().SetInt().push_back(pos);
            np_col->SetData().SetString().push_back(np);
            title_col->SetData().SetString().push_back(title);
            row++;
        }
        pos++;
    } else if (entry.IsSet() && entry.GetSet().IsSetSeq_set()) {
        ITERATE(objects::CBioseq_set::TSeq_set, seq_it, entry.GetSet().GetSeq_set()) {
            AddIdsFromSeqEntryToTable(**seq_it, id_col, pos_col, np_col, title_col, pos, row);
        }
    }
}


static CRef<objects::CSeqTable_column> s_MakeIdTableIdCol()
{
    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    id_col->SetData().SetId();
    return id_col;
}


static CRef<objects::CSeqTable_column> s_MakePosCol()
{
    CRef<objects::CSeqTable_column> pos_col(new objects::CSeqTable_column());
    pos_col->SetHeader().SetTitle("Pos");
    pos_col->SetData().SetInt();
    return pos_col;
}


static CRef<objects::CSeqTable_column> s_MakeNpCol()
{
    CRef<objects::CSeqTable_column> np_col(new objects::CSeqTable_column());
    np_col->SetHeader().SetTitle("NucOrProt");
    np_col->SetData().SetString();
    return np_col;
}


static CRef<objects::CSeqTable_column> s_MakeTitleCol()
{
    CRef<objects::CSeqTable_column> title_col(new objects::CSeqTable_column());
    title_col->SetHeader().SetTitle("Title");
    title_col->SetData().SetString();
    return title_col;
}


void AddIdsFromSeqEntryToTable
(const objects::CSeq_entry& entry, CRef<objects::CSeq_table> table)
{
    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName(table, kSequenceIdColLabel);
    if (!id_col) {
        id_col = s_MakeIdTableIdCol();
        table->SetColumns().push_back(id_col);
    }
    CRef<objects::CSeqTable_column> pos_col = FindSeqTableColumnByName(table, "Pos");
    if (!pos_col) {
        pos_col = s_MakePosCol();
        table->SetColumns().push_back(pos_col);
    }
    CRef<objects::CSeqTable_column> np_col = FindSeqTableColumnByName(table, "NucOrProt");
    if (!np_col) {
        np_col = s_MakeNpCol();
        table->SetColumns().push_back(np_col);
    }
    CRef<objects::CSeqTable_column> title_col = FindSeqTableColumnByName(table, "title");
    if (!title_col) {
        title_col = s_MakeTitleCol();
        table->SetColumns().push_back(title_col);
    }
    size_t row = table->GetNum_rows();
    int pos = 0;
    if (row > 0) {
        pos = pos_col->GetData().GetInt()[row - 1] + 1;
    }
    AddIdsFromSeqEntryToTable (entry, id_col, pos_col, np_col, title_col, pos, row);
    table->SetNum_rows(row);
}


CRef<objects::CSeq_table> GetIdsFromSeqEntry(const objects::CSeq_entry& entry)
{
    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    CRef<objects::CSeqTable_column> id_col = s_MakeIdTableIdCol();
    table->SetColumns().push_back(id_col);
    CRef<objects::CSeqTable_column> pos_col = s_MakePosCol();
    table->SetColumns().push_back(pos_col);
    CRef<objects::CSeqTable_column> np_col = s_MakeNpCol();
    table->SetColumns().push_back(np_col);
    CRef<objects::CSeqTable_column> title_col = s_MakeTitleCol();
    table->SetColumns().push_back(title_col);

    int pos = 1;
    size_t row = 0;
    AddIdsFromSeqEntryToTable(entry, id_col, pos_col, np_col, title_col, pos, row);
    table->SetNum_rows(row);
    return table;
}


const string kIdTooLong = "ID is too long";
const string kIdDuplicate = "duplicate";
const string kIdMissing = "Missing ID";


static string GetIdValueFromColumn (CRef<objects::CSeqTable_column> id_col, int pos) 
{
    string str = "";

    if (id_col->GetData().IsString()) {
        str = id_col->GetData().GetString()[pos];
    } else {
        id_col->GetData().GetId()[pos]->GetLabel(&str, objects::CSeq_id::eContent);
    }
    return str;
}


CRef<objects::CSeqTable_column> GetSeqIdProblems(CRef<objects::CSeq_table> new_ids, CRef<objects::CSeq_table> old_ids, size_t max_len)
{
    CRef<objects::CSeqTable_column> problems(new objects::CSeqTable_column());
    problems->SetHeader().SetTitle("Problems");
    problems->SetData().SetString();
    vector<string> id_strings;
    bool any_problems = false;
    bool any_nuc = false;

    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName(new_ids, kNewId);
    CRef<objects::CSeqTable_column> orig_id_col = FindSeqTableColumnByName(new_ids, kSequenceIdColLabel);
    if (!id_col) {
        id_col = orig_id_col;
    }
    CRef<objects::CSeqTable_column> np_col = FindSeqTableColumnByName(new_ids, "NucOrProt");

    // check length on new_ids only
    for (int i = 0; i < new_ids->GetNum_rows(); i++) {
        string str = GetIdValueFromColumn(id_col, i);
        id_strings.push_back(str);
        if (NStr::EqualNocase(np_col->GetData().GetString()[i], "nuc")) {
            any_nuc = true;
        }
        if (NStr::IsBlank(str)) {
            problems->SetData().SetString().push_back(kIdMissing);
            any_problems = true;
        } else if (str.length() > max_len) {
            problems->SetData().SetString().push_back(kIdTooLong);
            any_problems = true;
        } else {
            problems->SetData().SetString().push_back("");
        }
    }
        
    // only care about duplicates if there are nucs present
    if (any_nuc) {
        if (old_ids) {
            id_col = FindSeqTableColumnByName(old_ids, kSequenceIdColLabel);
            for (unsigned int i = 0; i < id_col->GetData().GetSize(); i++) {
                string str;
                id_col->GetData().GetId()[i]->GetLabel(&str, objects::CSeq_id::eContent);
                id_strings.push_back(str);
            }
        }              
        vector<string> dup_ids = FindNonUniqueStrings(id_strings);
        if (dup_ids.size() > 0) {
            for (int i = 0; i < new_ids->GetNum_rows(); i++) {
                bool found = false;
                ITERATE (vector<string>, s_it, dup_ids) {
                    if (NStr::EqualNocase(id_strings[i], *s_it)) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    string val = problems->GetData().GetString()[i];
                    if (!NStr::IsBlank(val)) {
                        val += ", ";
                    }
                    val += kIdDuplicate;
                    problems->SetData().SetString()[i] = val;
                    any_problems = true;
                }
            }
        }
    }
    if (!any_problems) {
        problems.Reset(NULL);
    }

    return problems;
}


string SummarizeIdProblems(CRef<objects::CSeqTable_column> problems) 
{
    int num_duplicates = 0;
    int num_too_long = 0;

    if (problems) {
        ITERATE(objects::CSeqTable_column::TData::TString, s_it, problems->GetData().GetString()) {
            if (NStr::FindNoCase(*s_it, kIdDuplicate) != string::npos) {
                num_duplicates++;
            }
            if (NStr::FindNoCase(*s_it, kIdTooLong) != string::npos) {
                num_too_long++;
            }
        }
    }
    string rval = "";
    if (num_duplicates > 0) {
        rval = NStr::NumericToString(num_duplicates) + " duplicate IDs";
        if (num_too_long > 0) {
            rval += ", ";
        }
    }
    if (num_too_long > 0) {
        rval += NStr::NumericToString(num_too_long) + " IDs are too long";
    }

    return rval;
}


CRef<objects::CSeqTable_column> GetReplacementSeqIds (CRef<objects::CSeq_table> new_ids, CRef<objects::CSeq_table> old_ids, size_t max_len)
{
    bool any_changes = false;
    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName(new_ids, kSequenceIdColLabel);
    CRef<objects::CSeqTable_column> np_col = FindSeqTableColumnByName(new_ids, "NucOrProt");
    CRef<objects::CSeqTable_column> pos_col = FindSeqTableColumnByName(new_ids, "Pos");

    CRef<objects::CSeqTable_column> new_str(new objects::CSeqTable_column());
    new_str->SetHeader().SetTitle("new_id");
    new_str->SetData().SetString();

    size_t num_seq = pos_col->GetData().GetInt().back();  
    CRef<objects::CSeqTable_column> old_pos_col = FindSeqTableColumnByName(old_ids, "Pos");
    if (old_pos_col && old_pos_col->GetData().GetInt().size() > 0) {
        num_seq += old_pos_col->GetData().GetInt().back();
    }

    int reserve_char = ceil(log ((double)num_seq)) + 1;

    vector<string> id_strings;

    // check length on new_ids only
    for (int i = 0; i < new_ids->GetNum_rows(); i++) {
        string str;
        id_col->GetData().GetId()[i]->GetLabel(&str, objects::CSeq_id::eContent);
        if (str.length() > max_len) {
            str = str.substr(0, max_len - reserve_char);
            new_str->SetData().SetString().push_back(str);
            any_changes = true;
        } else {
            new_str->SetData().SetString().push_back("");
        }
        id_strings.push_back(str);
    }
        
    if (old_ids) {
        id_col = FindSeqTableColumnByName(old_ids, kSequenceIdColLabel);
        for (unsigned int i = 0; i < id_col->GetData().GetSize(); i++) {
            string str;
            id_col->GetData().GetId()[i]->GetLabel(&str, objects::CSeq_id::eContent);
            id_strings.push_back(str);
        }
    }              
    vector<string> dup_ids = FindNonUniqueStrings(id_strings);
    if (dup_ids.size() > 0) {
        vector<int> high_offset;
        ITERATE (vector<string>, s_it, dup_ids) {
            high_offset.push_back(0);
        }
        ITERATE(vector<string>, i_it, id_strings) {
            int dup_pos = 0;
            ITERATE (vector<string>, s_it, dup_ids) {
                if (NStr::StartsWith(*i_it, *s_it + "_")) {
                    try {
                        int offset = NStr::StringToInt((*i_it).substr((*s_it).length() + 1));
                        if (offset > high_offset[dup_pos]) {
                            high_offset[dup_pos] = offset;
                        }
                    } catch (exception &) {
                        // not an integer suffix, ignore
                    }
                    break;
                }
                dup_pos++;
            }
        }
        for (int i = 0; i < new_ids->GetNum_rows(); i++) {
            int dup_pos = 0;
            ITERATE (vector<string>, s_it, dup_ids) {
                if (NStr::EqualNocase(id_strings[i], *s_it)) {
                    high_offset[dup_pos]++;
                    new_str->SetData().SetString()[i] = id_strings[i] + "_" + NStr::NumericToString(high_offset[dup_pos]);
                    any_changes = true;
                    break;
                }
                dup_pos++;
            }
        }
    }
    if (!any_changes) {
        new_str.Reset(NULL);
    }
    return new_str;
}


static void TransformSeqAnnot( objects::CSeq_annot& annot, 
                            CRef<objects::CSeq_id> seq_id,
                            CRef<objects::CSeq_id> new_seq_id )
{
    for (CTypeIterator<objects::CSeq_id> seq_id_it(Begin(annot));
         seq_id_it; ++seq_id_it)
    {
        if( seq_id_it->Equals(*seq_id) ) {
            seq_id_it->Assign(*new_seq_id);
        }
    }
}


static void TransformSeqAnnots( objects::CBioseq & bioseq,
                            CRef<objects::CSeq_id> seq_id,
                            CRef<objects::CSeq_id> new_seq_id )
{
    // fix annots directly on the sequence
    NON_CONST_ITERATE(objects::CBioseq::TAnnot, annot_it, bioseq.SetAnnot()) {
        TransformSeqAnnot(**annot_it, seq_id, new_seq_id);
    }

    // then, climb up, changing it within higher-up bioseq-set annots
    objects::CSeq_entry *pEntry = bioseq.GetParentEntry();
    for( ; pEntry; pEntry = pEntry->GetParentEntry() ) {
        if( ! pEntry->IsSet() ) {
            continue;
        }
        objects::CBioseq_set & bioseq_set = pEntry->SetSet();
        if( ! bioseq_set.IsSetAnnot() ) {
            continue;
        }
        NON_CONST_ITERATE(objects::CBioseq_set::TAnnot, annot_it, bioseq_set.SetAnnot() ) {
            TransformSeqAnnot(**annot_it, seq_id, new_seq_id);
        }
    }
}


void ApplyReplacementIds(objects::CSeq_entry& entry, CRef<objects::CSeq_table> table)
{
    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName(table, kSequenceIdColLabel);    
    CRef<objects::CSeqTable_column> new_id_col = FindSeqTableColumnByName(table, kNewId);    

    if (entry.IsSeq()) {
        NON_CONST_ITERATE (objects::CBioseq::TId, id_it, entry.SetSeq().SetId()) {
            objects::CSeqTable_column::TData::TId::iterator orig_it = id_col->SetData().SetId().begin();
            objects::CSeqTable_column::TData::TString::iterator new_it = new_id_col->SetData().SetString().begin();
            while(orig_it != id_col->SetData().SetId().end() && new_it != new_id_col->SetData().SetString().end()) {
                if ((*id_it)->Compare(**orig_it) == objects::CSeq_id::e_YES) {
                    CRef<objects::CSeq_id> new_id(new objects::CSeq_id());
                    new_id->SetLocal().SetStr(*new_it);
                    (*id_it)->Assign(*new_id);
                    TransformSeqAnnots (entry.SetSeq(), *orig_it, new_id);
                    // remove entry from list, so that we only make this replacement once
                    orig_it = id_col->SetData().SetId().erase(orig_it);
                    new_it = new_id_col->SetData().SetString().erase(new_it);
                    break;
                } else {
                    orig_it++;
                    new_it++;
                }
            }
        }
    } else if (entry.IsSet()) {
        NON_CONST_ITERATE(objects::CBioseq_set::TSeq_set, it, entry.SetSet().SetSeq_set()) {
            ApplyReplacementIds(**it, table);
        }
    }

}


bool AlreadyHasFeature(objects::CBioseq_Handle bh, string key, string comment)
{
    bool already_got_one = false;
    objects::CFeat_CI f(bh, objects::SAnnotSelector(objects::CSeqFeatData::e_Imp));
    while (f) {
        if (f->GetData().GetImp().IsSetKey() 
            && NStr::EqualNocase(key, f->GetData().GetImp().GetKey())
            && ((NStr::IsBlank(comment) && !f->IsSetComment())
                || (f->IsSetComment() && NStr::Equal(f->GetComment(), comment)))) {
            already_got_one = true;
            break;
        }
        ++f;
    }
    return already_got_one;
}


bool AlreadyHasFeature(objects::CBioseq_Handle bh, objects::CSeqFeatData::ESubtype subtype)
{
    bool already_got_one = false;
    objects::CFeat_CI f(bh, objects::SAnnotSelector(subtype));
    if (f) {
        already_got_one = true;
    }
    return already_got_one;
}


END_NCBI_SCOPE
