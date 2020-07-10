/*  $Id: srcedit_util.cpp 42186 2019-01-09 19:34:50Z asztalos $
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
 * Authors:  Colleen Bollin, J. Chen
 */


#include <ncbi_pch.hpp>

#include <util/static_map.hpp>
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
#include <objects/seqfeat/PCRPrimer.hpp>
#include <objects/seqfeat/PCRPrimerSeq.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>
#include <gui/widgets/edit/biosource_autocomplete.hpp>

#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/Dbtag.hpp>

#include <objtools/edit/source_edit.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(edit);

CSrcQual::CSrcQual()
{
    m_Name = "";
    m_Required = false;
    m_ReportMissing = false;
    m_Example = "";
}


CSrcQual::CSrcQual(string name, bool required, bool report_missing, string example)
: m_Name (name), m_Required (required), m_ReportMissing (report_missing), m_Example (example)
{
}

CSrcQual::~CSrcQual()
{
}


string CSrcQual::GetValue(const CBioSource& src)
{
    return "";
}

string CTaxnameQual::GetValue(const CBioSource& src)
{
    if (src.IsSetOrg() && src.GetOrg().IsSetTaxname()) {
        return src.GetOrg().GetTaxname();
    } else {
        return "";
    }
}


string CUnculturedTaxnameQual::GetFormatErrors(string value)
{
    vector<string> unwise_names;
    unwise_names.push_back("unknown");
    unwise_names.push_back("uncultured organism");
    unwise_names.push_back("uncultured sample");
    unwise_names.push_back("uncultured");
    unwise_names.push_back("unidentified");
    
    vector<string>::iterator s = unwise_names.begin();
    while (s != unwise_names.end()) {
        if (NStr::EqualNocase(value, *s)) {
            return "Ambiguous organism name";
        }
        ++s;
    }
    if (NStr::StartsWith (value, "uncultured ")) {
        return "";
    } else {
        return "For uncultured samples, organism name should start with 'uncultured'";
    }
}


string CGenomeQual::GetValue(const CBioSource& src)
{
    string val = "";

    if (src.IsSetGenome()) {
        int genome = src.GetGenome();
        val = CBioSource::GetOrganelleByGenome (genome);
    }

    return val;
}


COrgModQual::COrgModQual(string name, bool required, bool report_missing, string example)
    : CSrcQual(name, required, report_missing, example)
{
    m_Subtype = COrgMod::GetSubtypeValue(name, COrgMod::eVocabulary_insdc);
}


string COrgModQual::GetValue(const CBioSource& src)
{
    if (src.IsSetOrg() && src.GetOrg().IsSetOrgname() && src.GetOrg().GetOrgname().IsSetMod()) {
        ITERATE(COrgName::TMod, mit, src.GetOrg().GetOrgname().GetMod()) {
            if ((*mit)->GetSubtype() == m_Subtype && (*mit)->IsSetSubname()) {
                return (*mit)->GetSubname();
            }
        }
    }
    return "";
}


static bool s_IsAllNumbers (string val)
{
    bool all_numbers = true;
    if (NStr::IsBlank (val)) {
        return false;
    }
    string::iterator it = val.begin();
    while (it != val.end() && all_numbers) {
        if (!isdigit (*it)) {
            all_numbers = false;
        }
        ++it;
    }
    return all_numbers;
}


string COrgModQual::GetFormatErrors(string value)
{
    string rval = "";

    switch (m_Subtype) {
        case COrgMod::eSubtype_nat_host:
            if (s_IsAllNumbers(value)) {
                rval = "Invalid host";
            }
            break;
        default:
            break;
    }
    return rval;           
}


CSubSrcQual::CSubSrcQual(string name, bool required, bool report_missing, string example)
    : CSrcQual(name, required, report_missing, example)
{
    m_Subtype = CSubSource::GetSubtypeValue(name, CSubSource::eVocabulary_insdc);

    m_BadIsolationSourceValues.push_back("uncultured bacterium");
    m_BadIsolationSourceValues.push_back("uncultured bacteria");
    m_BadIsolationSourceValues.push_back("uncultured fungus");
    m_BadIsolationSourceValues.push_back("uncultured fungi");
    m_BadIsolationSourceValues.push_back("uncultured bacterium clone");
    m_BadIsolationSourceValues.push_back("uncultured bacteria clone");
    m_BadIsolationSourceValues.push_back("uncultured fungus clone");
    m_BadIsolationSourceValues.push_back("uncultured fungi clone");
    m_BadIsolationSourceValues.push_back("isolation source");

}


string CSubSrcQual::GetValue(const CBioSource& src)
{
    if (src.IsSetSubtype()) {
        ITERATE(CBioSource::TSubtype, mit, src.GetSubtype()) {
            if ((*mit)->GetSubtype() == m_Subtype && (*mit)->IsSetName()) {
                return (*mit)->GetName();
            }
        }
    }
    return "";
}


string CSubSrcQual::GetFormatErrors(string value)
{
    string rval = "";

    switch (m_Subtype) {
        case CSubSource::eSubtype_isolation_source:
            if (value.length() < 3) {
                rval = "Suspiciously short isolation source";
            } else {
                ITERATE (vector<string>, it, m_BadIsolationSourceValues) {
                    if (NStr::EqualNocase(value, *it)) {
                        rval = "Invalid isolation source";
                        break;
                    }
                }
            }
            break;
        case CSubSource::eSubtype_country:
            if (s_IsAllNumbers(value)) {
                rval = "Invalid country";
            }
            break;
        default:
            break;
    }
    return rval;           
}


CSourceRequirements::CSourceRequirements()
{
    m_Requirements.clear();
}


CSourceRequirements::~CSourceRequirements()
{
    for (size_t col = 0; col < m_Requirements.size(); col++) {
        delete(m_Requirements[col]);
    }
    m_Requirements.clear();
}


void CSourceRequirements::AddRequirement(string name, bool required, string example)
{
    // first, check to see if we already have requirement, increase requirement if necessary   
    NON_CONST_ITERATE (TSrcQualList, rit, m_Requirements) {
        if (MatchColumnName((*rit)->GetName(), name)) {
            if (required) {
                (*rit)->SetRequired(true);
            }
            if (!NStr::IsBlank(example)) {
                (*rit)->SetExample(example);
            }
            return;
        }
    }

    if (NStr::EqualNocase(name, "uncultured taxname")) {
        m_Requirements.push_back(new CUnculturedTaxnameQual("taxname", required, false, example));
    } else if (IsOrgColumnName(name)) {
        m_Requirements.push_back(new CTaxnameQual(name, required, false, example));
    } else {
        try {
            m_Requirements.push_back(new CSubSrcQual(name, required, false, example));
        } catch (CException& ) {
            m_Requirements.push_back(new COrgModQual(name, required, false, example));
        }        
    }
}


void CSourceRequirements::AddUniquenessList(vector<string> list)
{
    ITERATE(vector<string>, it, list) {
        AddRequirement(*it, false);
    }
    m_UniquenessLists.push_back(list);
}


void  CSourceRequirements::AddUniquenessList(int num, ... )
{
    va_list arguments;
    vector<string> list;

    va_start ( arguments, num );           
    for ( int x = 0; x < num; x++ ) {  
        string item = va_arg ( arguments, char * );
        AddRequirement(item, false);
        list.push_back(item ); 
    }
    va_end ( arguments ); 

    m_UniquenessLists.push_back(list);
}


void CSourceRequirements::AddOneOfList(vector<string> list)
{
    ITERATE(vector<string>, it, list) {
        AddRequirement(*it, false);
    }
    m_OneOfLists.push_back(list);
}


void  CSourceRequirements::AddOneOfList(int num, ... )
{
    va_list arguments;
    vector<string> list;

    va_start ( arguments, num );           
    for ( int x = 0; x < num; x++ ) {  
        string item = va_arg ( arguments, char * );
        AddRequirement(item, false);
        list.push_back(item ); 
    }
    va_end ( arguments ); 

    m_OneOfLists.push_back(list);
}


static string s_MakeUniquenessVal (vector<CRef<CSeqTable_column> > cols, size_t row)
{
    size_t i;
    string this_val = "";

    for (i = 0; i < cols.size(); i++) {
        // later, remove non-uniqueing characters
        if (row < cols[i]->GetData().GetSize()) {
            this_val += cols[i]->GetData().GetString()[row] + ";";
        }
    }
    NStr::ToUpper(this_val);
    return this_val;
}


int CSourceRequirements::x_AddUniquenessProblems(CRef<CSeq_table> values_table, vector<string> uniqueness_list, vector<string>& row_problems)
{
    int num_duplicates = 0;

    if (values_table->GetNum_rows() < 2 || uniqueness_list.size() < 1) {
        return 0;
    }

    size_t j;
    string description = uniqueness_list[0];
    for (j = 1; j < uniqueness_list.size(); j++) {
       description += "/" + uniqueness_list[j];
    }

    vector<CRef<CSeqTable_column> > cols;
    for (j = 0; j < uniqueness_list.size(); j++) {
        CRef<CSeqTable_column> col = FindSeqTableColumnByName(values_table, uniqueness_list[j]);
        if (col) {
            cols.push_back(col);
        }
    }

    CRef<CSeqTable_column> id_col = FindSeqTableColumnByName(values_table, kSequenceIdColLabel);

    vector<string> u_list;
    int row;
    for (row = 0; row < values_table->GetNum_rows(); row++) {
        u_list.push_back(s_MakeUniquenessVal (cols, row));
    }
    sort(u_list.begin(), u_list.end());
    vector<string>::iterator sit = u_list.begin();
    vector<string>::iterator sit2 = sit;
    sit2++;
    bool is_dup = false;
    while (sit2 != u_list.end()) {
        if (NStr::Equal(*sit, *sit2)) {
            // if the next value matches this one, delete it
            sit2 = u_list.erase(sit2);
            is_dup = true;
        } else {
            if (!is_dup) {
                // if the next value does not match this one,
                // and there were no matches, delete from the list
                sit = u_list.erase(sit);
            } else {
                sit++;
            }
            sit2 = sit;
            if (sit2 != u_list.end()) {
                sit2++;
            }
            is_dup = false;
        }
    }
    if (!is_dup && sit != u_list.end()) {
        sit = u_list.erase(sit);
    }
    sit = u_list.begin();
    while (sit != u_list.end()) {
        bool is_first = true;
        int first_row = 0;
        for (row = 0; row < values_table->GetNum_rows(); row++) {
            if (NStr::Equal(*sit, s_MakeUniquenessVal (cols, row))) {
                if (is_first) {
                    first_row = row;
                    row_problems[row] += description + " is duplicated";
                    is_first = false;
                } else {
                    string id_label;
                    if (id_col) {
                        id_col->GetData().GetId()[first_row]->GetLabel(&id_label);
                    } else {
                        id_label = NStr::NumericToString(first_row + 1);
                    }
                    row_problems[row] += description + " duplicates row " + id_label;
                }
                num_duplicates++;
            }
        }
        sit++;
    }
    return num_duplicates;
}


void CSourceRequirements::x_AddOneOfProblems(CRef<CSeq_table> values_table, vector<string> one_of_list, CRef<CSeqTable_column> problems)
{
    if (one_of_list.size() < 1) {
        return;
    }
    size_t j;
    string description = "Missing " + one_of_list[0];
    for (j = 1; j < one_of_list.size(); j++) {
       description += " or " + one_of_list[j];
    }

    vector<CRef<CSeqTable_column> > cols;
    for (j = 0; j < one_of_list.size(); j++) {
        CRef<CSeqTable_column> col = FindSeqTableColumnByName(values_table, one_of_list[j]);
        if (col) {
            cols.push_back(col);
        }
    }

    int row;
    for (row = 0; row < values_table->GetNum_rows(); row++) {
        bool any = false;
        for (j = 0; j < cols.size() && !any; j++) {
            if (!NStr::IsBlank(cols[j]->GetData().GetString()[row])) {
                any = true;
            }
        }
        if (!any) {
            if (!NStr::IsBlank(problems->SetData().SetString()[row])) {
                problems->SetData().SetString()[row] += ", ";
            }
            problems->SetData().SetString()[row] += description;
        }
    }
}


CRef<CSeqTable_column> CSourceRequirements::CheckSourceQuals(CRef<CSeq_table> values_table)
{
    string qual_report = "";
    int row;
    CRef<CSeqTable_column> problems (new CSeqTable_column());
    problems->SetHeader().SetTitle(kProblems);
    for (row = 0; row < values_table->GetNum_rows(); row++) {
        problems->SetData().SetString().push_back("");
    }

    // check for missing or invalid values
    ITERATE (TSrcQualList, rit, m_Requirements) {
        CRef<CSeqTable_column> col = FindSeqTableColumnByName (values_table, (*rit)->GetName());
        if (col) {
            // found value column, now validate
            for (row = 0; row < values_table->GetNum_rows(); row++) {
                string val = "";
                if (col->GetData().GetSize() > (size_t) row) {
                    val = col->GetData().GetString()[row];
                }
                if ((*rit)->IsRequired() && NStr::IsBlank(val)) {
                    if (!NStr::IsBlank(problems->SetData().SetString()[row])) {
                        problems->SetData().SetString()[row] += ", ";
                    }
                    problems->SetData().SetString()[row] += (*rit)->GetName() + " is missing";
                } else {
                    string tmp = (*rit)->GetFormatErrors(val);
                    if (!NStr::IsBlank(tmp)) {
                        if (!NStr::IsBlank(problems->SetData().SetString()[row])) {
                            problems->SetData().SetString()[row] += ", ";
                        }
                        problems->SetData().SetString()[row] += tmp;
                    }
                }
            }
        } else {
            for (row = 0; row < values_table->GetNum_rows(); row++) {
                if (!NStr::IsBlank(problems->SetData().SetString()[row])) {
                    problems->SetData().SetString()[row] += ", ";
                }
                problems->SetData().SetString()[row] += (*rit)->GetName() + " is missing";
            }
        }
    }

    size_t i;

    // check lists where at least one qualifier in the list is required
    for (i = 0; i < m_OneOfLists.size(); i++) {
        x_AddOneOfProblems(values_table, m_OneOfLists[i], problems);
    }

    // if any uniqueness lists exist, at least one must be satisfied
    if (values_table->GetNum_rows() > 1) {
        bool found_good_combo = false;
        int best_num = 0;
        vector<string > uniqueness_problems;
        vector<vector<int> > num_problems;
        for (i = 0; i < m_UniquenessLists.size() && !found_good_combo; i++) {
            vector<string> this_list;
            for (row = 0; row < values_table->GetNum_rows(); row++) {
                this_list.push_back("");
            }
            int num_problems = x_AddUniquenessProblems(values_table, m_UniquenessLists[i], this_list);
            if (num_problems > 0) {
                if (best_num == 0 || best_num > num_problems) {
                    uniqueness_problems = this_list;
                    best_num = num_problems;
                }
            } else {
                found_good_combo = true;
            }
        }
        if (!found_good_combo) {
            for (i = 0; i < uniqueness_problems.size(); i++) {
                if (!NStr::IsBlank(uniqueness_problems[i])) {
                    if (!NStr::IsBlank(problems->GetData().GetString()[i])) {
                        problems->SetData().SetString()[i] += ", ";
                    }
                    problems->SetData().SetString()[i] += uniqueness_problems[i];
                }
            }
        }                
    }

    return problems;
}


void CSourceRequirements::PreferentiallyAddRequirement(CRef<CSeq_table> values_table, string choice1, string choice2, bool required)
{
    CRef<CSeqTable_column> col1 = FindSeqTableColumnByName (values_table, choice1);
    CRef<CSeqTable_column> col2 = FindSeqTableColumnByName (values_table, choice2);

    if (!col2) {
        AddRequirement (choice1, required);
    } else if (!col1) {
        AddRequirement (choice2, required);
    } else {
        bool any1 = false, any2 = false;
        for (int row = 0; row < values_table->GetNum_rows() && !any1; row++) {
            if (!NStr::IsBlank(col1->GetData().GetString()[row])) {
                any1 = true;
            } else if (!NStr::IsBlank(col2->GetData().GetString()[row])) {
                any2 = true;
            }
        }
        if (any1 || !any2) {
            AddRequirement(choice1, required);
        } else {
            AddRequirement(choice2, required);
        }
    }
}


void CSourceRequirements::AddColumnsToSeqTable(CRef<CSeq_table> values_table)
{
    if (!values_table) {
        return;
    }

    NON_CONST_ITERATE (TSrcQualList, rit, m_Requirements) {
        string val_name = (*rit)->GetName();
        string example = (*rit)->GetExample();
        if (NStr::IsBlank(example)) {
            val_name += "\n";
        } else {
            val_name += "\n[" + example + "]";
        }
        CRef<CSeqTable_column> col = FindSeqTableColumnByName (values_table, val_name);
        if (col) {
            col->SetHeader().SetTitle(val_name);
        } else {
            CRef<CSeqTable_column> new_col(new CSeqTable_column());
            new_col->SetHeader().SetTitle(val_name);
            while (new_col->SetData().SetString().size() < (size_t) values_table->GetNum_rows()) {
                new_col->SetData().SetString().push_back ("");
            }
            values_table->SetColumns().push_back(new_col);
        }
    }
}


typedef struct exampletable {
  string field_name;
  int wizard_type;
  int src_type;
  string example;
} ExampleTableData;


static const ExampleTableData s_ExampleTable[] = {
  { "organism name", CSourceRequirements::eWizardType_viruses,  CSourceRequirements::eWizardSrcType_virus_influenza, "Influenza A virus" } ,
  { "organism name", CSourceRequirements::eWizardType_viruses,  CSourceRequirements::eWizardSrcType_virus_norovirus, "Norovirus" } ,
  { "organism name", CSourceRequirements::eWizardType_viruses,  CSourceRequirements::eWizardSrcType_virus_rotavirus, "Rotavirus A" } ,
  { "organism name", CSourceRequirements::eWizardType_viruses,  CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "Foot-and-mouth disease virus - type O" } ,
  { "organism name", CSourceRequirements::eWizardType_viruses,  -1, "Tula virus" } ,
  { "organism name", CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_bacteria_or_archaea, "Bacillus cereus" } ,
  { "organism name", -1, CSourceRequirements::eWizardSrcType_cultured_fungus, "Morchella esculenta" } ,
  { "organism name", -1, CSourceRequirements::eWizardSrcType_vouchered_fungus, "Morchella esculenta" } ,
  { "organism name", CSourceRequirements::eWizardType_rrna_its_igs, -1, "Taxodium distichum" } ,
  { "organism name", CSourceRequirements::eWizardType_igs, -1, "Taxodium distichum" } ,
  { "organism name", CSourceRequirements::eWizardType_tsa, -1, "Homo sapiens" } ,
  { "organism name", CSourceRequirements::eWizardType_d_loop, -1, "Coffea arabica" } ,
  { "organism name", CSourceRequirements::eWizardType_microsatellite, -1, "Coffea arabica" } ,
  { "Collection-date", -1, -1, "05-Feb-2005" } ,
  { "Collection-date", CSourceRequirements::eWizardType_viruses, -1, "01-Jan-2011" } ,
  { kHost, CSourceRequirements::eWizardType_viruses, -1, "Microtus arvalis" } ,
  { "Serotype", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "O" } ,
  { "Collection-date", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "05-Nov-2007" } ,
  { kHost, CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "Bos taurus" } ,
  { "Country", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "Iran" } ,
  { "Serotype", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "H1N1" } ,
  { "Collection-date", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "02-Feb-2011" } ,
  { kHost, CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "Homo sapiens; male" } ,
  { "Country", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "Italy" } ,
  { "segment", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "4" } ,
  { "Passage History", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "E1" } ,
  { "organism", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "Influenza A virus" } ,
  { "strain", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "A/Milan/2a18/2011" } ,
  { "isolate", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "A/Milan/2a18/2011" } ,
  { "Collection-date", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_norovirus, "09-Jan-2003" } ,
  { "Country", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_norovirus, "Brazil" } ,
  { kHost, CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_norovirus, "Homo sapiens" } ,
  { "Genotype", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_norovirus, "G1" } ,
  { "Collection-date", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_rotavirus, "15-Jul-2008" } ,
  { "Country", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_rotavirus, "USA: Idaho" } ,
  { kHost, CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_rotavirus, "Bos taurus" } ,
  { "Genotype", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_rotavirus, "G1" } ,
  { "Specimen-voucher", CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_vouchered_fungus, "AMNH 000000" } ,
  { "Specimen-voucher", CSourceRequirements::eWizardType_igs, CSourceRequirements::eWizardSrcType_vouchered_fungus, "AMNH 000000" } ,
  { kHost, CSourceRequirements::eWizardType_uncultured_samples, -1, "Cocos nucifera" } ,
  { "serotype", CSourceRequirements::eWizardType_viruses, -1, "Ex1" } , 
  { "genotype", CSourceRequirements::eWizardType_viruses, -1, "D9" } ,
  { "segment", CSourceRequirements::eWizardType_viruses, -1, "10" } ,
  { "genotype", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "VII" } ,
  { "serotype", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_norovirus, "1" } ,
  { "serotype", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_rotavirus, "Ex2a" } ,
  { "segment", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_rotavirus, "10" } ,
  { kHost, CSourceRequirements::eWizardType_igs, CSourceRequirements::eWizardSrcType_bacteria_or_archaea, "Nepenthes sp." } ,
  { kHost, CSourceRequirements::eWizardType_igs, CSourceRequirements::eWizardSrcType_cultured_fungus, "Ulmus sp." } ,
  { kHost, CSourceRequirements::eWizardType_igs, CSourceRequirements::eWizardSrcType_vouchered_fungus, "Ulmus sp." } ,
  { kHost, CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_cultured_fungus, "Ulmus sp." } ,
  { kHost, CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_vouchered_fungus, "Ulmus sp." } ,
  { "dev-stage", CSourceRequirements::eWizardType_tsa, -1, "seed" } ,
  { "cell-line", CSourceRequirements::eWizardType_tsa, -1, "HK234" } ,
  { "cell-type", CSourceRequirements::eWizardType_tsa, -1, "leukocyte" } ,
  { "cultivar", CSourceRequirements::eWizardType_tsa, -1, "Microtom" } ,
  { "tissue-type", CSourceRequirements::eWizardType_tsa, -1, "liver" } ,
  { "haplotype", CSourceRequirements::eWizardType_d_loop, -1, "A1" } ,
  { "specimen-voucher", CSourceRequirements::eWizardType_d_loop, -1, "A1" } ,
  { kHost, -1, -1, "Homo sapiens" } ,
  { "lat-lon", -1, -1, "39.00 N 77.10 W" } ,
  { "strain", -1, -1, "ABC123" } ,
  { "country", -1, -1, "USA: Ann Arbor, MI" } ,
  { "country", CSourceRequirements::eWizardType_viruses, -1, "Finland" } ,
  { "country", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "Iran" } ,
  { "country", CSourceRequirements::eWizardType_viruses, -1, "Finland" } ,
  { "Isolate", -1, -1, "SDZ123" } ,
  { "Isolate", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "IND19" } ,
  { "Isolate", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "A/Milan/2a18/2011" } ,
  { "Isolate", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_norovirus, "Hu/G1/T65/BRA/2003" } ,
  { "Isolate", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_rotavirus, "cow/C1/USA/2008/G1" } ,
  { "Isolate", CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_vouchered_fungus, "xyz1a" } ,
  { "Isolate", CSourceRequirements::eWizardType_rrna_its_igs, -1, "EX-A" } ,
  { "Isolate", CSourceRequirements::eWizardType_igs, CSourceRequirements::eWizardSrcType_vouchered_fungus, "xyz1a" } ,
  { "Isolate", CSourceRequirements::eWizardType_igs, -1, "EX-A" } ,
  { "Isolate", CSourceRequirements::eWizardType_d_loop, -1, "xyz1a" } ,
  { "Specimen-voucher", -1, -1, "USNM:12345" } ,
  { "sex", -1, -1, "female" } ,
  { "breed", -1, -1, "Holstein" } ,
  { "cultivar", -1, -1, "Granny Smith" } ,
  { "isolation-source", CSourceRequirements::eWizardType_uncultured_samples, -1, "diseased leaf" } ,
  { "isolation-source", CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_bacteria_or_archaea, "leaf surface" } ,
  { "isolation-source", CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_cultured_fungus, "soil under elm tree" } ,
  { "isolation-source", CSourceRequirements::eWizardType_rrna_its_igs, CSourceRequirements::eWizardSrcType_vouchered_fungus, "soil under elm tree" } ,
  { "isolation-source", CSourceRequirements::eWizardType_rrna_its_igs, -1, "lake shoreline" } ,
  { "isolation-source", CSourceRequirements::eWizardType_igs, CSourceRequirements::eWizardSrcType_bacteria_or_archaea, "leaf surface" } ,
  { "isolation-source", CSourceRequirements::eWizardType_igs, CSourceRequirements::eWizardSrcType_cultured_fungus, "soil under elm tree" } ,
  { "isolation-source", CSourceRequirements::eWizardType_igs, -1, "lake shoreline" } ,
  { "isolation-source", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "skin" } ,
  { "isolation-source", CSourceRequirements::eWizardType_viruses, CSourceRequirements::eWizardSrcType_virus_influenza, "nasal swab" } ,
  { "isolation-source", CSourceRequirements::eWizardType_viruses, -1, "feces" } ,
  { "isolation-source", -1, -1, "soil" } ,
  { "clone", CSourceRequirements::eWizardType_uncultured_samples, -1, "abc-1" } ,
  { "clone", CSourceRequirements::eWizardType_microsatellite, -1, "Ca-789" } ,
};


static const int k_NumExampleTableRows = sizeof (s_ExampleTable) / sizeof (ExampleTableData);


static bool IsExampleTableRowAcceptable (const ExampleTableData * e1, string field_name, CSourceRequirements::EWizardType wizard_type, CSourceRequirements::EWizardSrcType src_type)
{
    if (e1 == NULL) {
        return false;
    } 
    if (!NStr::EqualNocase(field_name, e1->field_name)) {
        return false;
    }
    if (e1->wizard_type != -1 && e1->wizard_type != wizard_type) {
        return false;
    }
    if (e1->src_type != -1 && e1->src_type != src_type) {
        return false;
    }
    return true;
}


static int CompareExampleTableRows (const ExampleTableData * e1, const ExampleTableData * e2)
{
    if (e1->wizard_type == -1 && e2->wizard_type != -1) {
        return -1;
    } else if (e1->wizard_type != -1 && e2->wizard_type == -1) {
        return 1;
    } else if (e1->src_type == -1 && e2->wizard_type != -1) {
        return -1;
    } else if (e1->src_type != -1 && e2->wizard_type == -1) {
        return 1;
    } else {
        return 0;
    }
}


void CSourceRequirements::SetExamples(CSourceRequirements::EWizardType wizard_type, CSourceRequirements::EWizardSrcType src_type)
{
    NON_CONST_ITERATE (TSrcQualList, rit, m_Requirements) {
        if (NStr::IsBlank((*rit)->GetExample())) {
            int best_k = -1;
            for (int k = 0; k < k_NumExampleTableRows; k++) {
                if (IsExampleTableRowAcceptable(s_ExampleTable + k, (*rit)->GetName(), wizard_type, src_type)
                    && (best_k == -1 || CompareExampleTableRows(s_ExampleTable + best_k, s_ExampleTable + k) < 0)) {
                    best_k = k;
                }
            }
            if (best_k > -1) {
                (*rit)->SetExample(s_ExampleTable[best_k].example);
            }
        }
    }

}


CSourceRequirements *GetSrcRequirements(CSourceRequirements::EWizardType wizard_type, 
                                        CSourceRequirements::EWizardSrcType source_type, 
                                        CRef<CSeq_table> values_table)
{
    CSourceRequirements *requirements = new CSourceRequirements();
    if (wizard_type == CSourceRequirements::eWizardType_uncultured_samples) {
        requirements->AddRequirement("uncultured taxname", true, "uncultured bacterium");
    } else {
        requirements->AddRequirement("organism name", true);
    }

#if 0
    string host = kHost;

    switch (wizard_type) {
        case CSourceRequirements::eWizardType_uncultured_samples:
          {
              bool has_gelband_isolate = false;
              CRef<CSeqTable_column> isolate_col = FindSeqTableColumnByName (values_table, "isolate");
              if (isolate_col) {
                  for (size_t row = 0; row < isolate_col->GetData().GetSize() && !has_gelband_isolate; row++) {
                      string isolate_val = isolate_col->GetData().GetString()[row];
                      if (NStr::StartsWith(isolate_val, "DGGE") || NStr::StartsWith(isolate_val, "TGGE")) {
                          has_gelband_isolate = true;
                      }
                  }
              }
              if (has_gelband_isolate) {
                  requirements->AddRequirement("isolate", true);
              } else {
                  requirements->AddRequirement("clone", true);
              }
              requirements->AddRequirement("isolation-source", true);
              break;
          }
        case CSourceRequirements::eWizardType_viruses:
            if (source_type == CSourceRequirements::eWizardSrcType_virus_influenza) {
                requirements->PreferentiallyAddRequirement (values_table, "strain", "isolate", true);
            } else {
                requirements->PreferentiallyAddRequirement (values_table, "isolate", "strain", true);
            }
            switch (source_type) {
                case CSourceRequirements::eWizardSrcType_virus_norovirus:
                  requirements->AddRequirement("collection-date", true);
                  requirements->AddRequirement("country", true);
                  requirements->AddRequirement("genotype", true);
                  requirements->AddOneOfList(2, host, "isolation-source");
                  break;
                case CSourceRequirements::eWizardSrcType_virus_foot_and_mouth:
                  requirements->AddRequirement("serotype", true);
                  requirements->AddRequirement("collection-date", true);
                  requirements->AddRequirement(host, true);
                  requirements->AddRequirement("country", true);
                  break;
                case CSourceRequirements::eWizardSrcType_virus_influenza:
                  requirements->AddRequirement("serotype", true);
                  requirements->AddRequirement("collection-date", true);
                  requirements->AddRequirement("country", true);
                  requirements->AddRequirement("segment", true);
                  requirements->AddOneOfList(2, host, "isolation-source");
                  break;
                case CSourceRequirements::eWizardSrcType_virus_rotavirus:
                  requirements->AddRequirement("collection-date", true);
                  requirements->AddRequirement("country", true);
                  requirements->AddRequirement("genotype", true);
                  requirements->AddOneOfList(2, host, "isolation-source");
                  break;
                default:
                  requirements->AddRequirement("country", true);
                  requirements->AddRequirement("collection-date", true);
                  requirements->AddRequirement(host, true);
                  break;
            }
            requirements->AddUniquenessList(1, "segment");
            requirements->AddUniquenessList(2, "segment", "isolate");
            requirements->AddUniquenessList(2, "segment", "strain");
            break;
        case CSourceRequirements::eWizardType_rrna_its_igs:
            switch (source_type) {
                case CSourceRequirements::eWizardSrcType_bacteria_or_archaea:
                    requirements->PreferentiallyAddRequirement (values_table, "strain", "isolate", true);
                    requirements->AddRequirement("isolation-source", true);
                    break;
                case CSourceRequirements::eWizardSrcType_cultured_fungus:
                    requirements->PreferentiallyAddRequirement (values_table, "strain", "isolate", true);
                    requirements->AddRequirement("isolation-source", true);
                    break;
                case CSourceRequirements::eWizardSrcType_vouchered_fungus:
                    requirements->AddRequirement("specimen-voucher", true);
                    break;
                default:
                    requirements->AddRequirement("isolate", true);
                    requirements->AddRequirement("isolation-source", true);
                    break;
            }
            break;
        case CSourceRequirements::eWizardType_igs:
            switch (source_type) {
                case CSourceRequirements::eWizardSrcType_cultured_fungus:
                    requirements->PreferentiallyAddRequirement (values_table, "strain", "isolate", true);
                    requirements->AddRequirement("isolation-source", true);
                    break;
                case CSourceRequirements::eWizardSrcType_vouchered_fungus:
                    requirements->AddRequirement("specimen-voucher", true);
                    requirements->AddUniquenessList(5, "organism", "specimen-voucher", "isolate", "bio-material", "culture-collection");
                    break;
                case CSourceRequirements::eWizardSrcType_plant:
                case CSourceRequirements::eWizardSrcType_animal:
                default:
                    requirements->AddRequirement("isolate", true);
                    requirements->AddRequirement("isolation-source", true);
                    requirements->AddRequirement("specimen-voucher", false);
                    requirements->AddRequirement("cultivar", false);
                    requirements->AddRequirement("bio-material", false);
                    requirements->AddRequirement("culture-collection", false);
                    requirements->AddUniquenessList(1, "organism");
                    requirements->AddUniquenessList(2, "organism", "isolate");
                    requirements->AddUniquenessList(2, "organism", "specimen-voucher");
                    requirements->AddUniquenessList(2, "organism", "cultivar");
                    requirements->AddUniquenessList(2, "organism", "bio-material");
                    requirements->AddUniquenessList(2, "organism", "culture-collection"); 
                    break;
            }
            break;
        case CSourceRequirements::eWizardType_d_loop:
            requirements->AddUniquenessList(1, "organism");
            requirements->AddUniquenessList(2, "organism", "isolate");
            requirements->AddUniquenessList(2, "organism", "specimen-voucher");
            requirements->AddUniquenessList(2, "organism", "haplotype");            
            break;
        default:
            break;
    }
#endif
    requirements->SetExamples(wizard_type, source_type);
    return requirements;
}


string GetPrimerSetNameValues(const CPCRPrimerSet& primer_set)
{
    vector<string> names;
    ITERATE (CPCRPrimerSet::Tdata, sit, primer_set.Get()) 
    {
        if ((*sit)->IsSetName()) 
        {
            string new_name = (*sit)->GetName();
            if (!new_name.empty())
                names.push_back(new_name);
        }
    }
    return NStr::Join(names, ",");
}


string GetPrimerSetSeqValues(const CPCRPrimerSet& primer_set)
{
    string this_seq = "";

    ITERATE (CPCRPrimerSet::Tdata, sit, primer_set.Get()) {
        string new_seq = "";
        if ((*sit)->IsSetSeq()) {
            new_seq = (*sit)->GetSeq();
        }
        if (!NStr::IsBlank(this_seq)) {
           this_seq += ",";
        }
        this_seq += new_seq;
    }
    return this_seq;
}


void RemoveLastCharacter(string& str)
{
    size_t len = str.length();
    if (len > 0) {
        str = str.substr(0, len - 1);
    }
}

string JoinValues(const string &name, const vector<string>& values)
{
    if (name == "culture_collection" || name == "culture-collection" || name == "culture collection" ||
        name == "bio_material" || name == "bio-material" || name == "biomaterial" ||
        name == "specimen_voucher" || name == "specimen-voucher" || name == "specimen voucher")
        return NStr::Join(values, "|");
    return NStr::Join(values, ";");
}

void  SplitValues(const string& name, const string& newValue, vector<string> &values)
{
    if (name == "culture_collection" || name == "culture-collection" || name == "culture collection" ||
        name == "bio_material" || name == "bio-material" || name == "biomaterial" ||
        name == "specimen_voucher" || name == "specimen-voucher" || name == "specimen voucher")
        NStr::Split(newValue, "|", values);
    else
        values.push_back(newValue);
}

CRef<CSeq_table> GetSeqTableFromSeqEntry (CSeq_entry_Handle seh)
{
    CRef<CSeq_table> table(new CSeq_table());
    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);

    CRef<CSeqTable_column> expand_col(new CSeqTable_column());
    expand_col->SetHeader().SetTitle("");
    expand_col->SetHeader().SetField_name("expand");
    expand_col->SetData().SetString();
    table->SetColumns().push_back(expand_col);

    CRef<CSeqTable_column> taxname_col(new CSeqTable_column());
    taxname_col->SetHeader().SetTitle("Organism Name");
    taxname_col->SetHeader().SetField_name("org.taxname");
    taxname_col->SetData().SetString();
    table->SetColumns().push_back(taxname_col);

    size_t row = 0;
    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        CSeqdesc_CI it (*b_iter, CSeqdesc::e_Source);
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign (*(b_iter->GetSeqId()));
        id_col->SetData().SetId().push_back(id);
        expand_col->SetData().SetString().push_back("");
        if (it) {
            // populate taxname
            if (it->GetSource().IsSetTaxname()) {
                while (taxname_col->GetData().GetString().size() < row + 1) {
                    taxname_col->SetData().SetString().push_back("");
                }
                taxname_col->SetData().SetString()[row] = it->GetSource().GetTaxname();
            }
            // populate genome
            if (it->GetSource().IsSetGenome() 
                && it->GetSource().GetGenome() != CBioSource::eGenome_unknown) {
                AddValueToTable (table, "genome", 
                                 CBioSource::GetOrganelleByGenome(it->GetSource().GetGenome()), row);                
            }
                
            if (it->GetSource().IsSetSubtype()) {
                map<string, vector<string> > name_to_values;
                ITERATE (CBioSource::TSubtype, sit, it->GetSource().GetSubtype()) {
                    string subtype_name = CSubSource::GetSubtypeName((*sit)->GetSubtype());
                    if (NStr::EqualNocase(subtype_name, "note")) {
                        subtype_name = kSubSourceNote;
                    }
                    string val = "";
                    if ((*sit)->IsSetName()) {
                        val = (*sit)->GetName();
                    }
                    if (NStr::IsBlank(val) && CSubSource::NeedsNoText((*sit)->GetSubtype())) {
                        val = "true";
                    }
                    if (!val.empty())
                        name_to_values[subtype_name].push_back(val);
                }
                for (const auto& name_val : name_to_values)
                {
                    AddValueToTable(table, name_val.first, JoinValues(name_val.first, name_val.second), row);
                }
            }
            if (it->GetSource().IsSetOrgMod()) {
                map<string, vector<string> > name_to_values;
                ITERATE (COrgName::TMod, sit, it->GetSource().GetOrgname().GetMod()) {
                    string subtype_name = COrgMod::GetSubtypeName((*sit)->GetSubtype());
                    if (NStr::EqualNocase(subtype_name, "note")) {
                        subtype_name = kOrgModNote;
                    }
                    if (NStr::EqualNocase(subtype_name, "nat-host")) {
                        subtype_name = "host";
                    }
                    string val = (*sit)->GetSubname();
                    if (!val.empty())
                        name_to_values[subtype_name].push_back(val);
                }
                for (const auto& name_val : name_to_values)
                {
                    AddValueToTable(table, name_val.first, JoinValues(name_val.first, name_val.second), row);
                }
            }
            if (it->GetSource().IsSetOrg() && it->GetSource().GetOrg().IsSetDb())
            {
                set<string> values;
                FOR_EACH_DBXREF_ON_ORGREF(sit, it->GetSource().GetOrg())
                {
                    string db = (*sit)->GetDb();
                    string tag;
                    if ((*sit)->GetTag().IsStr())
                        tag = (*sit)->GetTag().GetStr();
                    if ((*sit)->GetTag().IsId())
                        tag = NStr::IntToString((*sit)->GetTag().GetId());
                    values.insert(db + ":" + tag);
                }
                AddValueToTable(table, kDbXref, NStr::Join(values, ";"), row);
            }
            if (it->GetSource().IsSetPcr_primers()) {
                // list them all
                string fwd_name = "";
                string rev_name = "";
                string fwd_seq = "";
                string rev_seq = "";
                ITERATE (CPCRReactionSet::Tdata, pit, it->GetSource().GetPcr_primers().Get()) {
                    string this_fwd_name = "";
                    string this_fwd_seq = "";
                    string this_rev_name = "";
                    string this_rev_seq = "";
                    if ((*pit)->IsSetForward()) {
                        this_fwd_name = GetPrimerSetNameValues((*pit)->GetForward());
                        this_fwd_seq = GetPrimerSetSeqValues((*pit)->GetForward());
                    }
                    if ((*pit)->IsSetReverse()) {
                        this_rev_name = GetPrimerSetNameValues((*pit)->GetReverse());
                        this_rev_seq = GetPrimerSetSeqValues((*pit)->GetReverse());
                    }
                    fwd_name += this_fwd_name + ";";
                    fwd_seq += this_fwd_seq + ";";
                    rev_name += this_rev_name + ";";
                    rev_seq += this_rev_seq + ";";
                }
                RemoveLastCharacter(fwd_name);
                RemoveLastCharacter(fwd_seq);
                RemoveLastCharacter(rev_name);
                RemoveLastCharacter(rev_seq);
                if (!NStr::IsBlank(fwd_name) || !NStr::IsBlank(fwd_seq) || !NStr::IsBlank(rev_name) || !NStr::IsBlank(rev_seq)) {
                    AddValueToTable(table, "fwd-primer-name", fwd_name, row);
                    AddValueToTable(table, "fwd-primer-seq", fwd_seq, row);
                    AddValueToTable(table, "rev-primer-name", rev_name, row);
                    AddValueToTable(table, "rev-primer-seq", rev_seq, row);
                }
            }
                
        }
        row++;

    }            

    table->SetNum_rows(row);
    return table;
}


CRef<CSeq_table> GetSeqTableForSrcQualFromSeqEntry (CSeq_entry_Handle seh, vector<string> qual_names)
{
    CRef<CSeq_table> table(new CSeq_table());
    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);

    vector< CRef<CSrcTableColumnBase> > handlers;

    ITERATE(vector<string>, q, qual_names) {
        CRef<CSeqTable_column> col(new CSeqTable_column());
        col->SetHeader().SetTitle(*q);
        handlers.push_back(CSrcTableColumnBaseFactory::Create(*col));
        table->SetColumns().push_back(col);
    }

    size_t row = 0;
    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        CSeqdesc_CI it (*b_iter, CSeqdesc::e_Source);
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign (*(b_iter->GetSeqId()));
        id_col->SetData().SetId().push_back(id);
        for (size_t i = 0; i < handlers.size(); i++) {
            if (it) {
                table->SetColumns()[i + 1]->SetData().SetString().push_back(handlers[i]->GetFromBioSource(it->GetSource()));
            } else {
                table->SetColumns()[i + 1]->SetData().SetString().push_back("");
            }
        }
        row++;
    }            

    table->SetNum_rows(row);
    return table;
}


// class CSrcTableColumnBase
vector<CConstRef<CObject> > CSrcTableColumnBase::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objects;
    if (m_Type != CSrcTableColumnBase::eFeature) { 
        CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Source);
        while (desc_ci) {
            CConstRef<CObject> object;
            object.Reset(&(*desc_ci));
            objects.push_back(object);
            ++desc_ci;
        }
    }

    if (m_Type != CSrcTableColumnBase::eDescriptor) {
        CFeat_CI feat_ci(bsh, CSeqFeatData::e_Biosrc);
        while (feat_ci) {
            CConstRef<CObject> object;
            object.Reset(&(feat_ci->GetOriginalFeature()));
            objects.push_back(object);
            ++feat_ci;
        }
    }
    return objects;
}

vector<CRef<CApplyObject> > CSrcTableColumnBase::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;
    if (m_Type != CSrcTableColumnBase::eFeature) {
        CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Source);
        while (desc_ci) {
            CRef<CApplyObject> object(new CApplyObject(bsh, *desc_ci));
            objects.push_back(object);
            ++desc_ci;
        }
        if (objects.empty()) {
            CRef<CApplyObject> object(new CApplyObject(bsh, CSeqdesc::e_Source));
            objects.push_back(object);
        }
    }

    if (m_Type != CSrcTableColumnBase::eDescriptor) {
        CFeat_CI feat_ci(bsh, CSeqFeatData::e_Biosrc);
        while (feat_ci) {
            CRef<CApplyObject> obj(new CApplyObject(bsh, feat_ci->GetOriginalFeature()));
            objects.push_back(obj);
            ++feat_ci;
        }
    }
    return objects;
}

string CSrcTableColumnBase::GetVal(const CObject& object)
{
    string val(kEmptyStr);
    const CSeqdesc* desc = dynamic_cast<const CSeqdesc * >(&object);
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat * >(&object);
    if (desc && desc->IsSource()) {
        val = GetFromBioSource(desc->GetSource());
    } else if (feat && feat->IsSetData() && feat->GetData().IsBiosrc()) {
        val = GetFromBioSource(feat->GetData().GetBiosrc());
    }
    return val;
}

vector<string> CSrcTableColumnBase::GetVals(const CObject& object)
{
    vector<string> vals;
    const CSeqdesc* desc = dynamic_cast<const CSeqdesc * >(&object);
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat * >(&object);
    
    if (desc && desc->IsSource()) {
        vector<string> add = GetValsFromBioSource(desc->GetSource());
        vals.insert(vals.begin(), add.begin(), add.end());
    } else if (feat && feat->IsSetData() && feat->GetData().IsBiosrc()) {
        vector<string> add = GetValsFromBioSource(feat->GetData().GetBiosrc());
        vals.insert(vals.begin(), add.begin(), add.end());
    }
    return vals;
}

vector<string> CSrcTableColumnBase::GetValsFromBioSource(const CBioSource &src) const
{
    vector<string> vals;
    string val = GetFromBioSource(src);
    if (!NStr::IsBlank(val)) {
        vals.push_back(val);
    }
    return vals;
}

void CSrcTableColumnBase::ClearVal(CObject& object)
{
    CSeqdesc* desc = dynamic_cast<CSeqdesc * >(&object);
    CSeq_feat* feat = dynamic_cast<CSeq_feat * >(&object);
    if (desc && desc->IsSource()) {
        ClearInBioSource(desc->SetSource());
    } else if (feat && feat->IsSetData() && feat->GetData().IsBiosrc()) {
        ClearInBioSource(feat->SetData().SetBiosrc());
    }
}


// class CSrcTableOrganismNameColumn
bool CSrcTableOrganismNameColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text )
{
    bool rval = false;
    string orig_val = GetFromBioSource (src);
    if (AddValueToString(orig_val, newValue, existing_text)) {
        src.SetOrg().SetTaxname(orig_val);
        rval = true;
    }
    return rval;
}

void CSrcTableOrganismNameColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    in_out_bioSource.SetOrg().ResetTaxname();
}

string CSrcTableOrganismNameColumn::GetFromBioSource(const CBioSource & in_out_bioSource ) const
{
    string val = kEmptyStr;
    if (in_out_bioSource.IsSetTaxname()) {
        val = in_out_bioSource.GetTaxname();
    }
    return val;
}


// class CSrcTaxnameAfterBinomialColumn
bool CSrcTaxnameAfterBinomialColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text )
{
    bool rval = false;
    string orig_val = GetFromBioSource(src);
    if (NStr::IsBlank(orig_val) && AddValueToString(orig_val, newValue, existing_text)) {
        src.SetOrg().SetTaxname(src.GetTaxname() + orig_val);
        return true;
    }

    SIZE_TYPE pos = NStr::Find(src.GetTaxname(), orig_val);
    if (pos != NPOS && AddValueToString(orig_val, newValue, existing_text)) {
        string new_taxname = src.GetTaxname().substr(0, pos) + orig_val;
        src.SetOrg().SetTaxname(new_taxname);
        rval = true;
    }
    return rval;
}

void CSrcTaxnameAfterBinomialColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    if (in_out_bioSource.IsSetTaxname()) {
        string taxname = in_out_bioSource.GetTaxname();
        string taxname_after_binomial = x_GetTextAfterNomial(taxname);
        SIZE_TYPE pos = NStr::Find(taxname, taxname_after_binomial);
        if (pos != NPOS) {
            string new_taxname = taxname.substr(0, pos);
            NStr::TruncateSpacesInPlace(new_taxname);
            in_out_bioSource.SetOrg().SetTaxname(new_taxname);
        }
    }
}

string CSrcTaxnameAfterBinomialColumn::GetFromBioSource(const CBioSource & in_out_bioSource) const
{
    string taxname = CSrcTableOrganismNameColumn::GetFromBioSource(in_out_bioSource);
    return x_GetTextAfterNomial(taxname);
}

static const string nomial_keywords[] = {
"f. sp. ",
"var",
"pv.",
"bv.",
"serovar",
"subsp."
};

string CSrcTaxnameAfterBinomialColumn::x_GetTextAfterNomial(const string& taxname) const
{
    if (NStr::IsBlank(taxname)) {
        return kEmptyStr;
    }

    SIZE_TYPE pos = NStr::FindNoCase(taxname, " ");
    vector<string> names;
    if (pos != NPOS) {
        NStr::Split(taxname, " ", names, NStr::fSplit_Tokenize);
    }

    if (names.empty()) {
        return kEmptyStr;
    }

    pos = 0;
    if (NStr::EqualNocase(names[0], "uncultured")) {
        pos = 2; // skipping the first three words
    } else {
        pos = 1; // skipping the first two words
    }

    if (names.size() <= pos + 1) {
        return kEmptyStr;
    }
    pos++;
    bool found_keyword = true;
    while (pos < names.size() && found_keyword) {
        /* if the next word is a nomial keyword, skip that plus the first word that follows it */
        found_keyword = false;
        if (NStr::EqualCase(names[pos], "f.") && (pos+1 < names.size() && NStr::EqualCase(names[pos+1],"sp."))) {
            pos = pos + 3;
            found_keyword = true;
        } else {
            for (size_t n = 1; n < sizeof(nomial_keywords)/sizeof(string) && !found_keyword; ++n) {
                if (NStr::EqualCase(nomial_keywords[n], names[pos])) {
                    pos = pos + 2;
                    found_keyword = true;
                }
            }
        }
    }
    string taxname_after_binomial = kEmptyStr;
    while (pos < names.size()) {
        taxname_after_binomial += names[pos] + " ";
        pos++;
    }
    NStr::TruncateSpacesInPlace(taxname_after_binomial, NStr::eTrunc_End);
    return taxname_after_binomial;
}


// class CSrcTableGenomeColumn
CSrcTableGenomeColumn::CSrcTableGenomeColumn(const string& organelle) 
{
    m_organelle = -1;
    if (!organelle.empty())
        m_organelle = objects::CBioSource::ENUM_METHOD_NAME(EGenome)()->FindValue(organelle);
}

bool CSrcTableGenomeColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text)
{
    if (m_organelle == -1 || (src.IsSetGenome() && src.GetGenome() == m_organelle) )
    {
        src.SetGenome(CBioSource::GetGenomeByOrganelle(newValue));
        return true;
    }
    return false;
}

void CSrcTableGenomeColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    if (m_organelle == -1 || (in_out_bioSource.IsSetGenome() && in_out_bioSource.GetGenome() == m_organelle) )
    {
        in_out_bioSource.ResetGenome();
    }
}

string CSrcTableGenomeColumn::GetFromBioSource(const CBioSource & in_out_bioSource) const
{
    string val = "";
    if (in_out_bioSource.IsSetGenome() && (m_organelle == -1 || in_out_bioSource.GetGenome() == m_organelle)) {
        val = in_out_bioSource.GetOrganelleByGenome( in_out_bioSource.GetGenome());
    }
    return val;
}

vector<string> CSrcTableGenomeColumn::IsValid(const vector<string>& values)
{
    vector<string> problems;
    bool any = false;

    ITERATE(vector<string>, it, values) {
        if (!NStr::IsBlank(*it)
            && CBioSource::GetGenomeByOrganelle (*it, NStr::eNocase, true) == CBioSource::eGenome_unknown) {
            problems.push_back("'" + *it + "'" + " is not a valid value for Genome");
            any = true;
        } else {
            problems.push_back("");
        }
    }
    if (!any) {
        problems.clear();
    }
    return problems;
}


// class CSrcTableOriginColumn
bool CSrcTableOriginColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    string curr = "";
    if (src.IsSetOrigin()) {
        curr = CBioSource::GetStringFromOrigin(src.GetOrigin());
    }
    if (AddValueToString(curr, newValue, existing_text)) {
        CBioSource::TOrigin origin = CBioSource::GetOriginByString(curr);    
        src.SetOrigin(origin);
        rval = true;
    }
    return rval;
}

void CSrcTableOriginColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    in_out_bioSource.ResetOrigin();
}

string CSrcTableOriginColumn::GetFromBioSource(const CBioSource & in_out_bioSource) const
{
    string val = "";
    if (in_out_bioSource.IsSetOrigin()) {
        val = CBioSource::GetStringFromOrigin(in_out_bioSource.GetOrigin());
    }
    return val;
}

vector<string> CSrcTableOriginColumn::IsValid(const vector<string>& values)
{
    vector<string> problems;
    bool any = false;

#if 0
    ITERATE(vector<string>, it, values) {
        if (!NStr::IsBlank(*it)
            && CBioSource::GetOriginByString (*it, NStr::eNocase, true) == CBioSource::eOrigin_unknown) {
            problems.push_back("'" + *it + "'" + " is not a valid value for Origin");
            any = true;
        } else {
            problems.push_back("");
        }
    }
#endif
    if (!any) {
        problems.clear();
    }
    return problems;
}


// class CSrcTableSubSourceColumn
bool CSrcTableSubSourceColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    if (NStr::IsBlank(newValue)) {
        return false;
    }

    size_t i = 0;
    vector<string> values;
    SplitValues(CSubSource::ENUM_METHOD_NAME(ESubtype)()->FindName(m_Subtype, true), newValue, values);
    if (existing_text != eExistingText_add_qual && src.IsSetSubtype()) {
        CBioSource::TSubtype::iterator it = src.SetSubtype().begin();
        while (it != src.GetSubtype().end()) {
            if ((*it)->GetSubtype() == m_Subtype) {
                if (CSubSource::NeedsNoText(m_Subtype)) {
                    // do nothing, already here
                } else {
                    string orig_val = "";
                    if ((*it)->IsSetName()) {
                        orig_val = (*it)->GetName();
                    }
                    string val;
                    if (i < values.size())
                        val = values[i];
                    if (AddValueToString(orig_val, val, existing_text)) {
                        (*it)->SetName(orig_val);
                        rval = true;
                    }
                }
                i++;
            }
            ++it;
        }
    }
    if (i < values.size() || existing_text == eExistingText_add_qual) {
        for (; i < values.size(); i++)
        {
            if (CSubSource::NeedsNoText(m_Subtype)) {
                CRef<CSubSource> s(new CSubSource(m_Subtype, " "));        
                src.SetSubtype().push_back(s);
            } else {
                CRef<CSubSource> s(new CSubSource(m_Subtype, values[i]));        
                src.SetSubtype().push_back(s);
            }
            rval = true;
        }
    }
    return rval;
}


void CSrcTableSubSourceColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    if (in_out_bioSource.IsSetSubtype()) {
        CBioSource::TSubtype::iterator it = in_out_bioSource.SetSubtype().begin();
        while (it != in_out_bioSource.SetSubtype().end()) {
            if ((*it)->GetSubtype() == m_Subtype) {
                it = in_out_bioSource.SetSubtype().erase(it);
            } else {
                ++it;
            }
        }
        if (in_out_bioSource.SetSubtype().empty()) {
            in_out_bioSource.ResetSubtype();
        }
    }
}


string CSrcTableSubSourceColumn::GetFromBioSource(const CBioSource & in_out_bioSource) const
{
    string val = "";
    if (in_out_bioSource.IsSetSubtype()) {
        CBioSource::TSubtype::const_iterator it = in_out_bioSource.GetSubtype().begin();
        while (it != in_out_bioSource.GetSubtype().end()) {
            if ((*it)->GetSubtype() == m_Subtype && (*it)->IsSetName())  {
                val = (*it)->GetName();
                break;
            }
            ++it;
        }
    }
    // independently of the current biosource, these qualifiers should always be set to TRUE
    if (NStr::IsBlank(val) && CSubSource::NeedsNoText(m_Subtype)) {
        val = "true";
    }
    return val;
}


vector <string> CSrcTableSubSourceColumn::GetValsFromBioSource(const CBioSource & biosrc) const
{
    vector<string> vals;

    if (biosrc.IsSetSubtype()) {
        CBioSource::TSubtype::const_iterator it = biosrc.GetSubtype().begin();
        while (it != biosrc.GetSubtype().end()) {
            if ((*it)->GetSubtype() == m_Subtype && (*it)->IsSetName())  {
                string val = (*it)->GetName();
                // independently of the current biosource, these qualifiers should always be set to TRUE
                if (NStr::IsBlank(val) && CSubSource::NeedsNoText(m_Subtype)) {
                    val = "true";
                }
                vals.push_back(val);
            }
            ++it;
        }
    }
    return vals;
}


// class CSrcTableOrgModColumn
bool CSrcTableOrgModColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    if (NStr::IsBlank(newValue)) {
        return rval;
    }
    size_t i = 0;
    vector<string> values;
    SplitValues( COrgMod::ENUM_METHOD_NAME(ESubtype)()->FindName(m_Subtype, true), newValue, values);
    if (existing_text != eExistingText_add_qual 
        && src.IsSetOrg() && src.GetOrg().IsSetOrgname()
        && src.GetOrg().GetOrgname().IsSetMod()) {
        COrgName::TMod::iterator it = src.SetOrg().SetOrgname().SetMod().begin();
        while (it != src.SetOrg().SetOrgname().SetMod().end()) {
            if ((*it)->GetSubtype() == m_Subtype) {
                string orig_val = "";
                if ((*it)->IsSetSubname()) {
                    orig_val = (*it)->GetSubname();
                }
                string val;
                if (i < values.size())
                    val = values[i];
                i++;
                if (AddValueToString(orig_val, val, existing_text)) {
                    (*it)->SetSubname(orig_val);
                    rval = true;
                }
            }
            ++it;
        }
    }

    if (i < values.size() || existing_text == eExistingText_add_qual) {
        for (; i < values.size(); i++)
        {
            CRef<COrgMod> s(new COrgMod(m_Subtype, values[i]));        
            src.SetOrg().SetOrgname().SetMod().push_back(s);
            rval = true;
        }
    }
    return rval;
}

void CSrcTableOrgModColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    if (in_out_bioSource.IsSetOrg() && in_out_bioSource.GetOrg().IsSetOrgname() && in_out_bioSource.GetOrg().GetOrgname().IsSetMod()) {
        COrgName::TMod::iterator it = in_out_bioSource.SetOrg().SetOrgname().SetMod().begin();
        while (it != in_out_bioSource.SetOrg().SetOrgname().SetMod().end()) {
            if ((*it)->GetSubtype() == m_Subtype) {
                it = in_out_bioSource.SetOrg().SetOrgname().SetMod().erase(it);
            } else {
                ++it;
            }
        }
        if (in_out_bioSource.GetOrg().GetOrgname().GetMod().empty()) {
            in_out_bioSource.SetOrg().SetOrgname().ResetMod();
        }
    }
}

string CSrcTableOrgModColumn::GetFromBioSource(const CBioSource & in_out_bioSource) const
{
    string val = "";
    if (in_out_bioSource.IsSetOrg() && in_out_bioSource.GetOrg().IsSetOrgname() && in_out_bioSource.GetOrg().GetOrgname().IsSetMod()) {
        COrgName::TMod::const_iterator it = in_out_bioSource.GetOrg().GetOrgname().GetMod().begin();
        while (it != in_out_bioSource.GetOrg().GetOrgname().GetMod().end()) {
            if ((*it)->GetSubtype() == m_Subtype && (*it)->IsSetSubname()) {
                val = (*it)->GetSubname();
                break;
            }
            ++it;
        }
    }
    return val;
}

vector <string> CSrcTableOrgModColumn::GetValsFromBioSource(const CBioSource & biosrc) const
{
    vector<string> vals;
    if (biosrc.IsSetOrg() && biosrc.GetOrg().IsSetOrgname() && biosrc.GetOrg().GetOrgname().IsSetMod()) {
        COrgName::TMod::const_iterator it = biosrc.GetOrg().GetOrgname().GetMod().begin();
        while (it != biosrc.GetOrg().GetOrgname().GetMod().end()) {
            if ((*it)->GetSubtype() == m_Subtype && (*it)->IsSetSubname()) {
                string val = (*it)->GetSubname();
                vals.push_back(val);
            }
            ++it;
        }
    }
    return vals;
}


// class CSrcStructuredVoucherPartColumn

typedef SStaticPair<const char*, CSrcStructuredVoucherPartColumn::EStructVouchPart> TStructVoucherPart;
static const TStructVoucherPart s_StrVouchPartName[] = {
    { "",        CSrcStructuredVoucherPartColumn::eUnknown },
    { "coll",    CSrcStructuredVoucherPartColumn::eColl },
    { "inst",    CSrcStructuredVoucherPartColumn::eInst },
    { "specid",  CSrcStructuredVoucherPartColumn::eSpecid },
};

typedef CStaticArrayMap<string, CSrcStructuredVoucherPartColumn::EStructVouchPart> TStrcVouchMap;
DEFINE_STATIC_ARRAY_MAP(TStrcVouchMap, sm_VouchMap, s_StrVouchPartName);

const string& CSrcStructuredVoucherPartColumn::GetName_StrVoucherPart( EStructVouchPart stype_part )
{
        TStrcVouchMap::const_iterator iter = sm_VouchMap.begin();
        for (;  iter != sm_VouchMap.end();  ++iter){
            if (iter->second == stype_part){
                return iter->first;
            }
        }
        return kEmptyStr;   
}

CSrcStructuredVoucherPartColumn::EStructVouchPart 
CSrcStructuredVoucherPartColumn::GetStrVoucherPart_FromName ( const string& name )
{
    TStrcVouchMap::const_iterator iter = sm_VouchMap.find(name);
    if (iter != sm_VouchMap.end()){
        return iter->second;
    }
    return 	CSrcStructuredVoucherPartColumn::eUnknown;
}


void CSrcStructuredVoucherPartColumn::x_ParsePartsFromStructuredVoucher(const string& qualifier, string& inst, string& coll, string& id) const
{
    if (!COrgMod::ParseStructuredVoucher(qualifier, inst, coll, id) &&
        NStr::IsBlank(inst) &&
        NStr::IsBlank(coll) &&
        NStr::IsBlank(id)) {
        id = qualifier;
    }
}

string CSrcStructuredVoucherPartColumn::GetFromBioSource( const CBioSource & in_out_bioSource ) const
{
    string inst, coll, id;
    string qualifier = CSrcTableOrgModColumn::GetFromBioSource(in_out_bioSource);
    x_ParsePartsFromStructuredVoucher(qualifier, inst, coll, id);
    switch (m_SubtypePart) {
        case eInst:
            return inst;
        case eColl:
            return coll;
        case eSpecid:
            return id;
        default:
            break;
    }
    return kEmptyStr;
}

vector<string> CSrcStructuredVoucherPartColumn::GetValsFromBioSource(const CBioSource &src) const
{
    vector<string> vals;
    string val = GetFromBioSource(src);
    if (!NStr::IsBlank(val)) {
        vals.push_back(val);
    }
    return vals;
}

void CSrcStructuredVoucherPartColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    if (in_out_bioSource.IsSetOrg() && in_out_bioSource.GetOrg().IsSetOrgname() && in_out_bioSource.GetOrg().GetOrgname().IsSetMod()) {
        COrgName::TMod::iterator it = in_out_bioSource.SetOrg().SetOrgname().SetMod().begin();
        while (it != in_out_bioSource.SetOrg().SetOrgname().SetMod().end()) {
            if ((*it)->GetSubtype() == m_Subtype && (*it)->IsSetSubname()) {
                // delete the inst|coll|id part of the structured voucher
                string inst, coll, id;
                x_ParsePartsFromStructuredVoucher((*it)->GetSubname(), inst, coll, id);
                switch (m_SubtypePart) {
                    case eInst:
                        inst = kEmptyStr;
                        break;
                    case eColl:
                        coll = kEmptyStr; 
                        break;
                    case eSpecid:
                        id = kEmptyStr;
                        break;
                    default:
                        return;
                }
                string new_subname = COrgMod::MakeStructuredVoucher(inst, coll, id);
                if (NStr::IsBlank(new_subname)) {
                    it = in_out_bioSource.SetOrg().SetOrgname().SetMod().erase(it);
                } else {
                    (*it)->SetSubname(new_subname);
                    ++it;
                }
            } else {
                ++it;
            }
        }
        if (in_out_bioSource.GetOrg().GetOrgname().GetMod().empty()) {
            in_out_bioSource.SetOrg().SetOrgname().ResetMod();
        }
    }
}

bool CSrcStructuredVoucherPartColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    if (NStr::IsBlank(newValue)) {
        return rval;
    }

    bool found = false;
    if (existing_text != eExistingText_add_qual 
        && src.IsSetOrg() && src.GetOrg().IsSetOrgname()
        && src.GetOrg().GetOrgname().IsSetMod()) {
        COrgName::TMod::iterator it = src.SetOrg().SetOrgname().SetMod().begin();
        while (it != src.SetOrg().SetOrgname().SetMod().end()) {
            if ((*it)->GetSubtype() == m_Subtype) {
                found = true;
                string inst, coll, id;
                if ((*it)->IsSetSubname()) {
                    x_ParsePartsFromStructuredVoucher((*it)->GetSubname(), inst, coll, id);
                }
                
                switch (m_SubtypePart) {
                    case eInst:
                        if (AddValueToString(inst, newValue, existing_text)) {
                            rval = true;
                        }
                        break;
                    case eColl:
                        if (AddValueToString(coll, newValue, existing_text)) {
                            rval = true;
                        }
                        break;
                    case eSpecid:
                        if (AddValueToString(id, newValue, existing_text)) {
                            rval = true;
                        }
                        break;
                    default:
                        break;
                }

                if (rval) {
                    string new_subname = COrgMod::MakeStructuredVoucher(inst, coll, id);
                    (*it)->SetSubname(new_subname);
                }
            }
            ++it;
        }
    }

    if (!found || existing_text == eExistingText_add_qual) {
        CRef<COrgMod> s(new COrgMod());
        s->SetSubtype(m_Subtype);
        string inst, coll, id;
        inst = coll = id = kEmptyStr;
        switch (m_SubtypePart) {
            case eInst:
                inst = newValue;
                rval = true;
                break;
            case eColl:
                coll = newValue;
                rval = true;
                break;
            case eSpecid:
                id = newValue;
                rval = true;
                break;
            default:
                rval = false;
                break;
        }
        if (rval) {
            string new_subname = COrgMod::MakeStructuredVoucher(inst, coll, id);
            s->SetSubname(new_subname);
            src.SetOrg().SetOrgname().SetMod().push_back(s);
        }
    }
    return rval;
}


// class CSrcTablePrimerColumn
CSrcTablePrimerColumn::EPrimerColType CSrcTablePrimerColumn::GetPrimerColumnType(const string& field_name)
{
    if (IsFwdPrimerName(field_name)) {
        return eFwdName;
    } else if (IsFwdPrimerSeq(field_name)) {
        return eFwdSeq;
    } else if (IsRevPrimerName(field_name)) {
        return eRevName;
    } else if (IsRevPrimerSeq(field_name)) {
        return eRevSeq;
    } else {
        return eNotPrimerCol;
    }
}

void CSrcTablePrimerColumn::SetConstraint(const string& field_name, CConstRef<CStringConstraint> string_constraint)
{
    if (!string_constraint) {
        m_ConstraintCol = eNotPrimerCol;
        m_StringConstraint.Reset(NULL);
    } else {
        m_ConstraintCol = GetPrimerColumnType(field_name);
        if (m_ConstraintCol == eNotPrimerCol) {
            m_StringConstraint.Reset(NULL);
        } else {
            m_StringConstraint = new CStringConstraint(" ");
            m_StringConstraint->Assign(*string_constraint);
        }
    } 
}

bool CSrcTablePrimerColumn::x_DoesReactionHaveSpace(const CPCRReaction& reaction)
{
    bool rval = false;
    switch (m_ColType) {
        case eFwdSeq:
        case eFwdName:
            if (reaction.IsSetForward()) {
                rval = x_DoesPrimerSetHaveSpace(reaction.GetForward());
            } else {
                rval = true;
            }
            break;
        case eRevSeq:
        case eRevName:
            if (reaction.IsSetReverse()) {
                rval = x_DoesPrimerSetHaveSpace(reaction.GetForward());
            } else {
                rval = true;
            }
            break;
        default:
            break;
    }
    return rval;
}


bool CSrcTablePrimerColumn::x_DoesPrimerSetHaveSpace(const CPCRPrimerSet& set)
{
    bool rval = false;
    if (!set.IsSet() || set.Get().size() == 0) {
        return true;
    }
    switch (m_ColType) {
        case eFwdSeq:
        case eRevSeq:
            if (!set.Get().front()->IsSetSeq()) {
                rval = true;
            }
            break;
        case eFwdName:
        case eRevName:
            if (!set.Get().front()->IsSetName()) {
                rval = true;
            }
            break;
        default:
            break;
    }
    return rval;
}


bool CSrcTablePrimerColumn::x_DoesReactionMatchConstraint(const CPCRReaction& reaction) const
{
    if (m_ConstraintCol == eNotPrimerCol || !m_StringConstraint) {
        return true;
    }

    vector<string> vals = x_GetValues(m_ConstraintCol, reaction);
    // intermediate solution for now:
    return const_cast< CRef<CStringConstraint>& >(m_StringConstraint)->DoesListMatch(vals);
}


vector<string> CSrcTablePrimerColumn::x_GetValues(EPrimerColType col, const CBioSource& src) const
{
    vector<string> vals;
    if (src.IsSetPcr_primers()) {
        ITERATE(CBioSource::TPcr_primers::Tdata, it, src.GetPcr_primers().Get()) {
            vector<string> add = x_GetValues(col, **it);
            if (add.size() > 0) {
                vals.insert(vals.end(), add.begin(), add.end());
            }
        }
    }
    return vals;
}


vector<string> CSrcTablePrimerColumn::x_GetValues(EPrimerColType col, const CPCRReaction& reaction) const
{
    vector<string> vals;
    switch (col) {
        case eFwdSeq:
        case eFwdName:
            if (reaction.IsSetForward()) {
                vector<string> add = x_GetValues(col, reaction.GetForward());
                if (add.size() > 0) {
                    vals.insert(vals.end(), add.begin(), add.end());
                }
            }
            break;
        case eRevSeq:
        case eRevName:
            if (reaction.IsSetReverse()) {
                vector<string> add = x_GetValues(col, reaction.GetReverse());
                if (add.size() > 0) {
                    vals.insert(vals.end(), add.begin(), add.end());
                }
            }
            break;
        default:
            break;
    }
    return vals;
}


vector<string> CSrcTablePrimerColumn::x_GetValues(EPrimerColType col, const CPCRPrimerSet& set) const
{
    vector<string> vals;

    switch (col) {
        case eFwdSeq:
        case eRevSeq:
            ITERATE(CPCRPrimerSet::Tdata, it, set.Get()) {
                if ((*it)->IsSetSeq()) {
                    vals.push_back((*it)->GetSeq());
                }
            }
            break;
        case eFwdName:
        case eRevName:
            ITERATE(CPCRPrimerSet::Tdata, it, set.Get()) {
                if ((*it)->IsSetName()) {
                    vals.push_back((*it)->GetName());
                }
            }
            break;
        default:
            break;
    }
    return vals;
}



bool CSrcTablePrimerColumn::x_ApplyStringToReaction(const string& val, CPCRReaction& reaction, EExistingText existing_text)
{
    bool try_this = false;
    bool rval = false;
    if (existing_text == eExistingText_add_qual) {
        if (x_DoesReactionHaveSpace(reaction)) {
            try_this = true;
        }
    } else if (x_DoesReactionMatchConstraint(reaction)) {
        try_this = true;
    }
    if (try_this) {
        switch (m_ColType) {
            case eFwdSeq:
            case eFwdName:
                rval = x_ApplyStringToPrimerSet(val, reaction.SetForward(), existing_text);
                break;
            case eRevSeq:
            case eRevName:
                rval = x_ApplyStringToPrimerSet(val, reaction.SetReverse(), existing_text);
                break;
            default:
                break;
        }
    }

    return rval;
}


bool CSrcTablePrimerColumn::x_AddFieldToPrimerSet(const string& val, CPCRPrimerSet& set)
{
    bool rval = false;
    switch (m_ColType) {
        case eFwdSeq:
        case eRevSeq:
            {{
                CPCRPrimerSeq seq(val);
                /*if (set.IsSet() && set.Get().size() > 0) {
                    if (!set.Get().back()->IsSetSeq()) {
                        set.Set().back()->SetSeq(seq);
                        rval = true;
                    }
                    } else */{
                    CRef<CPCRPrimer> primer (new CPCRPrimer());
                    primer->SetSeq(seq);
                    set.Set().push_back(primer);
                    rval = true;
                }
            }}
            break;
        case eFwdName:
        case eRevName:
            {{
                CPCRPrimerName name(val);
                /* if (set.IsSet() && set.Get().size() > 0) {
                    if (!set.Get().back()->IsSetName()) {
                        set.Set().back()->SetName(name);
                        rval = true;
                    }
                    } else*/ {
                    CRef<CPCRPrimer> primer (new CPCRPrimer());
                    primer->SetName(name);
                    set.Set().push_back(primer);
                    rval = true;
                }
            }}
            break;
        default:
            break;
    }
    return rval;       
}


bool CSrcTablePrimerColumn::x_ApplyStringToPrimerSet(const string& orig_val, CPCRPrimerSet& set, EExistingText existing_text)
{
    size_t i = 0;
    vector<string> values;
    NStr::Split(orig_val, ",", values);
    bool rval = false;
    bool found = false;
    switch (m_ColType) {
        case eFwdSeq:
        case eRevSeq:
            NON_CONST_ITERATE(CPCRPrimerSet::Tdata, it, set.Set()) {
                string val = orig_val;
                if (i < values.size() && !m_StringConstraint)
                    val = values[i];
                i++;
                string str = "";
                if ((*it)->IsSetSeq()) {
                    str = (*it)->GetSeq();
                }
                if (m_ColType != m_ConstraintCol || !m_StringConstraint || m_StringConstraint->DoesTextMatch(str)) {
                    if (NStr::IsBlank(val) && existing_text == eExistingText_replace_old) {
                        (*it)->ResetSeq();
                        rval = true;
                    } else if (AddValueToString(str, val, existing_text)) {
                        CPCRPrimerSeq seq(str);
                        (*it)->SetSeq(seq);
                        rval = true;
                    }
                }
                found = true;
            }
            break;
        case eFwdName:
        case eRevName:
            NON_CONST_ITERATE(CPCRPrimerSet::Tdata, it, set.Set()) {
                string val = orig_val;
                if (i < values.size() && !m_StringConstraint)
                    val = values[i];
                i++;
                string str = "";
                if ((*it)->IsSetName()) {
                    str = (*it)->GetName();
                }
                if (m_ColType != m_ConstraintCol || !m_StringConstraint || m_StringConstraint->DoesTextMatch(str)) {
                    if (NStr::IsBlank(val) && existing_text == eExistingText_replace_old) {
                        (*it)->ResetName();
                        rval = true;
                    } else if(AddValueToString(str, val, existing_text)) {
                        CPCRPrimerName name(str);
                        (*it)->SetName(name);
                        rval = true;
                    }
                }
                found = true;
            }
            break;
        default:
            break;
    }

    if (i < values.size() && (m_ConstraintCol == eNotPrimerCol || !m_StringConstraint)) {
        // no constraint, not found, add if there is an empty space
        for (; i < values.size(); i++)
            rval |= x_AddFieldToPrimerSet(values[i], set);
    }

    return rval;
}


void CSrcTablePrimerColumn::x_RemoveEmptyValues(CBioSource& src)
{
    if (src.IsSetPcr_primers()) {
        if (src.GetPcr_primers().IsSet()) {
            CPCRReactionSet::Tdata::iterator it = src.SetPcr_primers().Set().begin();
            while (it != src.SetPcr_primers().Set().end()) {
                x_RemoveEmptyValues(**it);
                if (!(*it)->IsSetForward() && !(*it)->IsSetReverse()) {
                    it = src.SetPcr_primers().Set().erase(it);
                } else {
                    ++it;
                }
            }
            if (src.GetPcr_primers().Get().empty()) {
                src.SetPcr_primers().Reset();
            }
        }
        if (!src.GetPcr_primers().IsSet()) {
            src.ResetPcr_primers();
        }
    }
}


void CSrcTablePrimerColumn::x_RemoveEmptyValues(CPCRReaction& reaction)
{
    if (reaction.IsSetForward()) {
        x_RemoveEmptyValues(reaction.SetForward());
        if (!reaction.GetForward().IsSet()) {
            reaction.ResetForward();
        }
    }
    
    if (reaction.IsSetReverse()) {
        x_RemoveEmptyValues(reaction.SetReverse());
        if (!reaction.GetReverse().IsSet()) {
            reaction.ResetReverse();
        }
    }
}


void CSrcTablePrimerColumn::x_RemoveEmptyValues(CPCRPrimerSet& set)
{
    if (set.IsSet()) {
        CPCRPrimerSet::Tdata::iterator it = set.Set().begin();
        while (it != set.Set().end()) {
            bool is_empty = true;
            if ((*it)->IsSetName()) {
                string str = (*it)->GetName();
                if (NStr::IsBlank(str)) {
                    (*it)->ResetName();
                } else {
                    is_empty = false;
                }
            }
            if((*it)->IsSetSeq()) {
                string str = (*it)->GetSeq();
                if (NStr::IsBlank(str)) {
                    (*it)->ResetSeq();
                } else {
                    is_empty = false;
                }
            }
            if (is_empty) {
                it = set.Set().erase(it);
            } else {
                ++it;
            }
        }
        if (set.Set().empty()) {
            set.Reset();
        }
    }
}


bool CSrcTablePrimerColumn::AddToBioSource(CBioSource & src, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    vector<string> values;
    NStr::Split(newValue, ";", values);
    size_t i = 0;
    NON_CONST_ITERATE(CPCRReactionSet::Tdata, it, src.SetPcr_primers().Set()) {
        string value = newValue;
        if (i < values.size() && !m_StringConstraint)
            value = values[i];
        i++;
        rval |= x_ApplyStringToReaction(value, **it, existing_text);
    }
    if (i < values.size() && (m_ConstraintCol == eNotPrimerCol || !m_StringConstraint)) {
        for (; i < values.size(); i++)
        {
            CRef<CPCRReaction> reaction(new CPCRReaction());
            if (x_ApplyStringToReaction(values[i], *reaction, existing_text)) {
                src.SetPcr_primers().Set().push_back(reaction);
                rval = true;
            }
        }
    }
    return rval;
}


string CSrcTablePrimerColumn::GetFromBioSource(const CBioSource & src) const
{
    vector<string> vals = x_GetValues(m_ColType, src);
    if (vals.size() > 0) {
        return vals[0];
    } else {
        return "";
    }
}


vector<string> CSrcTablePrimerColumn::GetValsFromBioSource(const CBioSource & src) const
{
    vector<string> vals;
    if (src.IsSetPcr_primers()) {
        ITERATE(CBioSource::TPcr_primers::Tdata, it, src.GetPcr_primers().Get()) {
            if (x_DoesReactionMatchConstraint(**it)) {
                vector<string> add = x_GetValues(m_ColType, **it);
                if (add.size() > 0) {
                    vals.insert(vals.end(), add.begin(), add.end());
                }
            }
        }
    }
    return vals;
}


void CSrcTablePrimerColumn::ClearInBioSource(CBioSource & src)
{
    AddToBioSource(src, "", eExistingText_replace_old);
    x_RemoveEmptyValues(src);
}


bool IsFwdPrimerName (string name)
{
    return (NStr::EqualNocase(name, "fwd-primer-name") || NStr::EqualNocase(name, "fwd primer name")
        || NStr::EqualNocase(name, "fwd-name") || NStr::EqualNocase(name, "fwd name"));
}


bool IsRevPrimerName (string name)
{
    return (NStr::EqualNocase(name, "rev-primer-name") || NStr::EqualNocase(name, "rev primer name")
        || NStr::EqualNocase(name, "rev-name") || NStr::EqualNocase(name, "rev name"));
}


bool IsFwdPrimerSeq (string name)
{
    return (NStr::EqualNocase(name, "fwd-primer-seq") || NStr::EqualNocase(name, "fwd primer seq")
        || NStr::EqualNocase(name, "fwd-seq") || NStr::EqualNocase(name, "fwd seq"));
}


bool IsRevPrimerSeq (string name)
{
    return (NStr::EqualNocase(name, "rev-primer-seq") || NStr::EqualNocase(name, "rev primer seq")
        || NStr::EqualNocase(name, "rev-seq") || NStr::EqualNocase(name, "rev seq"));
}


// class CSrcTableAllPrimersColumn
string CSrcTableAllPrimersColumn::GetFromBioSource(const CBioSource & src) const
{
    CRef<CSrcTablePrimerColumn> fwdseq(new CSrcTableFwdPrimerSeqColumn());
    CRef<CSrcTablePrimerColumn> revseq(new CSrcTableRevPrimerSeqColumn());
    CRef<CSrcTablePrimerColumn> fwdname(new CSrcTableFwdPrimerNameColumn());
    CRef<CSrcTablePrimerColumn> revname(new CSrcTableRevPrimerNameColumn());

    string primer_value = fwdseq->GetFromBioSource(src);
    if (NStr::IsBlank(primer_value)) {
        primer_value = revseq->GetFromBioSource(src);
        if (NStr::IsBlank(primer_value)) {
            primer_value = fwdname->GetFromBioSource(src);
            if (NStr::IsBlank(primer_value)) {
                primer_value = revname->GetFromBioSource(src);
            }
        }
    }
    return primer_value;
}

vector<string> CSrcTableAllPrimersColumn::GetValsFromBioSource(const CBioSource &src) const
{
    vector <string> vals;

    CRef<CSrcTablePrimerColumn> fwdseq( new CSrcTableFwdPrimerSeqColumn());
    string note = fwdseq->GetFromBioSource(src);
    if (!note.empty()) {
       vals.push_back(note);
    }
    CRef<CSrcTablePrimerColumn> revseq(new CSrcTableRevPrimerSeqColumn());
    note = kEmptyStr;
    note = revseq->GetFromBioSource(src);
    if (!note.empty()) {
       vals.push_back(note);
    }
    CRef<CSrcTablePrimerColumn> fwdname(new CSrcTableFwdPrimerNameColumn());
    note = kEmptyStr;
    note = fwdname->GetFromBioSource(src);
    if (!note.empty()) {
       vals.push_back(note);
    }
    CRef<CSrcTablePrimerColumn> revname(new CSrcTableRevPrimerNameColumn());
    note = kEmptyStr;
    note = revname->GetFromBioSource(src);
    if (!note.empty()) {
       vals.push_back(note);
    }
   
    return vals; 
}

void CSrcTableAllPrimersColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    if (in_out_bioSource.IsSetPcr_primers()) {
        in_out_bioSource.ResetPcr_primers();
    }
}


// J. Chen
string CSrcTableCommonNameColumn :: GetFromBioSource(const CBioSource& biosrc) const
{
   return (biosrc.IsSetCommon() ? biosrc.GetCommon() : kEmptyStr);
}

string CSrcTableLineageColumn :: GetFromBioSource(const CBioSource& biosrc) const
{
   return (biosrc.IsSetLineage() ? biosrc.GetLineage() : kEmptyStr);
}

string CSrcTableDivisionColumn :: GetFromBioSource(const CBioSource& biosrc) const
{
   return (biosrc.IsSetDivision() ? biosrc.GetDivision() : kEmptyStr);
}


// class CSrcTableDbxrefColumn
string CSrcTableDbxrefColumn::GetFromBioSource(const CBioSource& biosrc) const
{
    if (!biosrc.GetOrg().IsSetDb()) {
        return kEmptyStr;
    }

    if (NStr::IsBlank(m_DbName)) {
        string label;
        biosrc.GetOrg().GetDb().front()->GetLabel(&label);
        return label;
    } else {
        FOR_EACH_DBXREF_ON_ORGREF(db, biosrc.GetOrg()) {
            if ((*db)->IsSetDb() && NStr::EqualCase((*db)->GetDb(), m_DbName) && (*db)->IsSetTag()) {
                CNcbiOstrstream oss;
                (*db)->GetTag().AsString(oss);
                return CNcbiOstrstreamToString(oss);
            }
        }
    }
    return kEmptyStr;
};

vector<string> CSrcTableDbxrefColumn::GetValsFromBioSource(const CBioSource& biosrc) const
{
    vector<string> vals;
    if (!biosrc.GetOrg().IsSetDb()) {
        return vals;
    }

    if (NStr::IsBlank(m_DbName)) {
        FOR_EACH_DBXREF_ON_ORGREF(db, biosrc.GetOrg()) {
            string label;
            (*db)->GetLabel(&label);
            vals.push_back(label);
        }
    }
    else {
        vals.push_back(GetFromBioSource(biosrc));
    }
    return vals;
}

bool CSrcTableDbxrefColumn::AllowMultipleValues()
{
    return (NStr::IsBlank(m_DbName)) ? true : false;
}

bool CSrcTableDbxrefColumn::AddToBioSource(CBioSource & in_out_bioSource, const string & newValue, EExistingText existing_text)
{
    vector<string> values;
    NStr::Split(newValue, ";", values);
    for (auto value : values)
    {
        {
            string db, tag;
            NStr::SplitInTwo(value, ":", db, tag);
            if (!db.empty() && !tag.empty())
            {
                m_DbName = db;
                value = tag;
            }
        }
        if (NStr::IsBlank(m_DbName)) {
            continue;
        }

        if (existing_text == edit::eExistingText_leave_old) {
            continue;
        }
        
        // if there is one already present, overwrite it
        EDIT_EACH_DBXREF_ON_ORGREF(db, in_out_bioSource.SetOrg()) {
            if ((*db)->IsSetDb() && NStr::EqualCase((*db)->GetDb(), m_DbName) && (*db)->IsSetTag()) {
                CObject_id& tag = (*db)->SetTag();
                CNcbiOstrstream oss;
                (*db)->GetTag().AsString(oss);
                string orig_val = CNcbiOstrstreamToString(oss);
                if (AddValueToString(orig_val, value, existing_text)) {
                    tag.SetStr(orig_val);
                    continue;
                }
            }
        }

        // otherwise make a new one
        CRef<CDbtag> db_tag(new CDbtag());
        db_tag->SetDb(m_DbName);
        try {
            int val = NStr::StringToInt(value);
            db_tag->SetTag().SetId(val);
        }
        catch (exception &) {
            db_tag->SetTag().SetStr(value);
        }

        in_out_bioSource.SetOrg().SetDb().push_back(db_tag);
    }
    return true;
}

void CSrcTableDbxrefColumn::ClearInBioSource(objects::CBioSource & in_out_bioSource )
{
    if (in_out_bioSource.IsSetOrg() && in_out_bioSource.GetOrg().IsSetDb()) {
        in_out_bioSource.SetOrg().ResetDb();      
    }
}


// class CSrcTableAllNotesColumn
vector <string> CSrcTableAllNotesColumn::GetValsFromBioSource(const CBioSource& biosrc) const
{
   vector <string> vals;

   CRef<CSrcTableColumnBase> 
      subsrc_note( new CSrcTableSubSourceColumn (CSubSource::eSubtype_other));
    string note = subsrc_note->GetFromBioSource(biosrc);
    if (!note.empty()) {
       vals.push_back(note);
    }
    CRef<CSrcTableColumnBase>  
      orgmod_note( new CSrcTableOrgModColumn (COrgMod::eSubtype_other));
    note = kEmptyStr;
    note = orgmod_note->GetFromBioSource(biosrc);
    if (!note.empty()) {
      vals.push_back(note);
    }
   
    return vals;
};
// J. Chen

string CSrcTableAllNotesColumn::GetFromBioSource(const CBioSource & in_out_bioSource) const
{
    // return only the first found note on the biosource
    vector<string> note_values = GetValsFromBioSource(in_out_bioSource);
    if ( !note_values.empty() ){
        return note_values.front();
    } 
    return kEmptyStr;
}

void CSrcTableAllNotesColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    CRef<CSrcTableColumnBase> orgmod_note( new CSrcTableOrgModColumn(COrgMod::eSubtype_other));
    CRef<CSrcTableColumnBase> subsrc_note( new CSrcTableSubSourceColumn (CSubSource::eSubtype_other));
    
    orgmod_note->ClearInBioSource(in_out_bioSource);
    subsrc_note->ClearInBioSource(in_out_bioSource);
}
    

// class CSrcTableTaxonIdColumn
bool CSrcTableTaxonIdColumn::AddToBioSource(CBioSource & in_out_bioSource, const string & newValue, EExistingText existing_text)
{
    try {
        int val = NStr::StringToInt(newValue);
        in_out_bioSource.SetOrg().SetTaxId(val);
        return true;
    } catch (exception &) {
    }
    return false;
}

void CSrcTableTaxonIdColumn::ClearInBioSource(CBioSource & in_out_bioSource)
{
    if (in_out_bioSource.IsSetOrg() && in_out_bioSource.GetOrg().IsSetDb()) {
        COrg_ref::TDb::iterator it = in_out_bioSource.SetOrg().SetDb().begin();
        while (it != in_out_bioSource.SetOrg().SetDb().end()) {
            if ((*it)->IsSetDb() && NStr::EqualNocase((*it)->GetDb(), "taxon")) {
                it = in_out_bioSource.SetOrg().SetDb().erase(it);
            } else {
                ++it;
            }
        }
    }
}

string CSrcTableTaxonIdColumn::GetFromBioSource(const CBioSource & in_out_bioSource) const
{
    string val = "";
    if (in_out_bioSource.IsSetOrg()) {
        try {
            int taxid = in_out_bioSource.GetOrg().GetTaxId();
            val = NStr::NumericToString(taxid);
        } catch (exception &) {
        }
    }
    return val;
}


// class CSrcTableColumnBaseFactory
CRef<CSrcTableColumnBase>
CSrcTableColumnBaseFactory::Create(const string &column_name)
{
    // the default is a CSrcTableEditCommandFactory that does nothing

    string sTitle = column_name;
    SIZE_TYPE nl_pos = NStr::Find(sTitle, "\n");
    if ( nl_pos != NPOS ) {
        sTitle = sTitle.substr(0, nl_pos);
    }

    SIZE_TYPE desc_pos = NStr::FindNoCase(sTitle, " descriptor");
    SIZE_TYPE feat_pos = NStr::FindNoCase(sTitle, " feature");
    CSrcTableColumnBase::ESourceType src_type = CSrcTableColumnBase::eNotSet;
    if (desc_pos != NPOS) {
        sTitle = sTitle.substr(0, desc_pos);
        src_type = CSrcTableColumnBase::eDescriptor;
    } if (feat_pos != NPOS) {
        sTitle = sTitle.substr(0, feat_pos);
        src_type = CSrcTableColumnBase::eFeature;
    }

    if (sTitle.empty()) {
        return CRef<CSrcTableColumnBase>(NULL);
    }

    if (sTitle == kSequenceIdColLabel) {
        return CRef<CSrcTableColumnBase>(NULL);
    }

    if (NStr::EqualNocase(sTitle, kHost)) {
        sTitle = kNatHost;
    }
   
    CRef<CSrcTableColumnBase> rval(NULL);

    if( NStr::EqualNocase(sTitle, "Organism Name") || NStr::EqualNocase(sTitle, "org") || NStr::EqualNocase(sTitle, "taxname") ) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableOrganismNameColumn() );
    } else if( NStr::EqualNocase(sTitle, "Taxname after Binomial")) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTaxnameAfterBinomialColumn() );
    } else if (!NStr::Equal(sTitle, kGenomeProjectID, NStr::eNocase) && NStr::StartsWith( sTitle, "genome", NStr::eNocase )) {
        string organelle = sTitle.substr(6, NPOS);
        NStr::TruncateSpacesInPlace(organelle);
        rval = CRef<CSrcTableColumnBase>( new CSrcTableGenomeColumn(organelle) );
    } else if (NStr::EqualNocase( sTitle, "origin" )) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableOriginColumn() );
    } else if (NStr::EqualNocase(sTitle, kSubSourceNote) 
           || NStr::EqualNocase(sTitle, "subsource-note")
           || NStr::EqualNocase(sTitle, "subsrc-note")
           || NStr::EqualNocase(sTitle, "note-subsrc")) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableSubSourceColumn (CSubSource::eSubtype_other));
    } else if (NStr::EqualNocase(sTitle, kOrgModNote)
           || NStr::EqualNocase(sTitle, "orgmod-note")) {
        rval = CRef<CSrcTableColumnBase>(new CSrcTableOrgModColumn(COrgMod::eSubtype_other));
    } else if (IsFwdPrimerName(sTitle)) {
        rval = CRef<CSrcTableColumnBase>(new CSrcTableFwdPrimerNameColumn());
    } else if (IsFwdPrimerSeq(sTitle)) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableFwdPrimerSeqColumn ());
    } else if (IsRevPrimerName(sTitle)) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableRevPrimerNameColumn());
    } else if (IsRevPrimerSeq(sTitle)) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableRevPrimerSeqColumn());
    } else if (NStr::EqualNocase(sTitle, "common-name") || NStr::EqualNocase(sTitle, "common name")) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableCommonNameColumn());
    } else if (NStr::EqualNocase(sTitle, "lineage")) {
        rval = CRef<CSrcTableColumnBase>( new CSrcTableLineageColumn());
    } else if (NStr::EqualNocase(sTitle, "division")) {
        rval = CRef <CSrcTableColumnBase> (new CSrcTableDivisionColumn());
    } else if (NStr::StartsWith(sTitle, "dbxref", NStr::eNocase)) {
        string dbname = sTitle.substr(6, NPOS);
        NStr::TruncateSpacesInPlace(dbname);
        rval = CRef<CSrcTableColumnBase> ( new CSrcTableDbxrefColumn(dbname));
    } else if (NStr::EqualNocase(sTitle, "taxid")) {
        rval = CRef <CSrcTableColumnBase>(new CSrcTableTaxonIdColumn());
    } else if (NStr::EqualNocase(sTitle, "all-notes") || NStr::EqualNocase(sTitle, kAllNotes)) {
        rval = CRef <CSrcTableColumnBase>(new CSrcTableAllNotesColumn());
    } else if (NStr::EqualNocase(sTitle, kAllPrimers)) {
        rval = CRef<CSrcTableColumnBase>(new CSrcTableAllPrimersColumn());
    }
    // see if it is a structured voucher - an orgmod qualifier
    else if (NStr::EndsWith(sTitle, "-inst") || NStr::EndsWith(sTitle, "-coll") || NStr::EndsWith(sTitle, "-specid")) {
        SIZE_TYPE pos = NStr::Find(sTitle, "-", NStr::eCase, NStr::eReverseSearch);
        if ( pos != NPOS) {
            string subtype = sTitle.substr(0, pos);
            bool isorgmod = COrgMod::IsValidSubtypeName(subtype, COrgMod::eVocabulary_insdc);
            if (isorgmod) {
                COrgMod::TSubtype st = COrgMod::GetSubtypeValue (subtype, COrgMod::eVocabulary_insdc);
                CSrcStructuredVoucherPartColumn::EStructVouchPart epart = 
                    CSrcStructuredVoucherPartColumn::GetStrVoucherPart_FromName(sTitle.substr(pos+1, NPOS));
                rval = CRef<CSrcTableColumnBase>( new CSrcStructuredVoucherPartColumn(st, epart) );
            }
        }
    }
    else if (CSubSource::IsValidSubtypeName(sTitle, CSubSource::eVocabulary_insdc)) {
        CSubSource::TSubtype st = CSubSource::GetSubtypeValue (sTitle, CSubSource::eVocabulary_insdc);
        rval = CRef<CSrcTableColumnBase>( new CSrcTableSubSourceColumn(st) );
    } else if (COrgMod::IsValidSubtypeName(sTitle, COrgMod::eVocabulary_insdc)) {
        COrgMod::TSubtype st = COrgMod::GetSubtypeValue (sTitle, COrgMod::eVocabulary_insdc);
        rval = CRef<CSrcTableColumnBase>( new CSrcTableOrgModColumn(st) );
    }

    if (rval) {
        rval->SetSourceType(src_type);
    }
    return rval;
}


CRef<CSrcTableColumnBase>
CSrcTableColumnBaseFactory::Create(const CSeqTable_column &column)
{
    // extract the title of the column
    if( ! column.IsSetHeader() || ! column.GetHeader().IsSetTitle() ) {
        return CRef<CSrcTableColumnBase>( NULL );
    }

    string sTitle = column.GetHeader().GetTitle();

    return CSrcTableColumnBaseFactory::Create(sTitle);
}


CRef<CCmdComposite> ApplySrcTableToSeqEntry (CRef<CSeq_table>values_table, CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Bulk Source Edit") );

    CRef<CSeqTable_column> id_col = FindSeqTableColumnByName (values_table, kSequenceIdColLabel);

    if (!id_col) {
        return cmd;
    }

    if( ! values_table->IsSetColumns() || values_table->GetColumns().empty() ) {
        return cmd;
    }
    const CSeq_table::TColumns & columns = values_table->GetColumns();
    size_t num_cols = columns.size();

    // create an edit-command maker for every column.  This somewhat byzantine
    // system is here to speed things up to avoid re-parsing the meaning
    // of each column every time.
    vector< CRef<CSrcTableColumnBase> > vecColEditFactories;

    ITERATE( CSeq_table::TColumns, col_iter, columns ) {
        if ((*col_iter)->GetHeader().GetTitle().empty() || NStr::EqualNocase((*col_iter)->GetHeader().GetTitle(), kProblems)) 
        {
            CRef<CSrcTableColumnBase> bogus(NULL);
            vecColEditFactories.push_back(bogus);
        }
        else
        {
            vecColEditFactories.push_back( CSrcTableColumnBaseFactory::Create(**col_iter) );               
        }
    }


    for (int row = 0; row < values_table->GetNum_rows() && (size_t) row < id_col->GetData().GetSize(); row++) {
        CBioseq_Handle bsh = seh.GetBioseqHandle(*(id_col->GetData().GetId()[row]));

        CRef<CSeqdesc> new_source_desc( new CSeqdesc );
        CSeqdesc_CI desc_ci( bsh, CSeqdesc::e_Source);
        if (desc_ci) {
            new_source_desc->Assign(*desc_ci);
        }
        CBioSource & bioSource = new_source_desc->SetSource();

        // iterate through the columns, skipping Seq-id column, to erase current values
        // need to do this in separate steps, in case of multiple qualifier columns in table
        for( size_t col = 1; col < num_cols; ++col ) {
            if (vecColEditFactories[col])
                vecColEditFactories[col]->ClearInBioSource(bioSource);
            // TODO: detect if change occurred to avoid excessive changes
        }

        // iterate through the columns, skipping Seq-id column
        for( size_t col = 1; col < num_cols; ++col ) {
            if (vecColEditFactories[col] && columns[col]->GetData().GetSize() > (size_t) row) {
                vecColEditFactories[col]->AddToBioSource(
                    bioSource, *columns[col]->GetStringPtr(row), eExistingText_replace_old);
            }
            // TODO: detect if change occurred to avoid excessive changes
        }
        // automatically populate taxon ID, div, genetic codes if available
        CBioSourceAutoComplete::AutoFill(bioSource.SetOrg());

        if (desc_ci) {
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_source_desc));
            cmd->AddCommand (*ecmd);
        } else {
            CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
            if (bssh && bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_nuc_prot) {
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bssh.GetParentEntry(), *new_source_desc)) );
            } else {
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_source_desc)) );
            }
        }
    }

    // send composite command
    return cmd;
}


bool RelaxedMatch (CRef<CSeq_id> id1, CRef<CSeq_id> id2)
{
    bool found = false;
    if (id1->IsLocal()) {
        string id1_label = "";
        id1->GetLabel(&id1_label, CSeq_id::eContent);
        string id2_label = "";
        id2->GetLabel(&id2_label, CSeq_id::eContent);
        id2->GetLabel(&id2_label, CSeq_id::eContent);
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


static bool OneRowOk (CRef<CSeq_id> id, CRef<CSeqTable_column> id_col, CSeq_entry_Handle seh) 
{
    if (!id || !id_col) {
        return false;
    }

    size_t row = 0;
    bool found = false;
    while (row < id_col->GetData().GetSize() && !found) {
        CRef<CSeq_id> row_id = id_col->GetData().GetId()[row];
        CSeq_id::E_SIC compare = id->Compare(*row_id);
        if (compare == CSeq_id::e_YES) {
            found = true;
        } else if (compare == CSeq_id::e_DIFF) {
            if (RelaxedMatch(id, row_id)) {
                found = true;
                id->Assign(*row_id);
            }
        }
        row++;
    }
    row = 0;    
    while (row < id_col->GetData().GetSize() && !found) {
        CRef<CSeq_id> row_id = id_col->GetData().GetId()[row];
        CBioseq_Handle bsh = seh.GetBioseqHandle (*row_id);
        if (bsh) {
            CConstRef<CBioseq> b = bsh.GetCompleteBioseq();
            ITERATE (CBioseq::TId, id_it, b->GetId()) {
                CSeq_id::E_SIC compare = id->Compare(**id_it);
                if (compare == CSeq_id::e_YES) {
                    found = true;
                    id->Assign(*row_id);
                    break;
                } else if (compare == CSeq_id::e_DIFF) {
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


END_NCBI_SCOPE
