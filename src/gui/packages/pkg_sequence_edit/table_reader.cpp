/*  $Id: table_reader.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
* Authors:  Colleen Bollin, Andrea Asztalos
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbiexpt.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objtools/edit/string_constraint.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/table_reader.hpp>
#include <gui/widgets/edit/propagate_descriptors.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  

#include <wx/progdlg.h>
#include <wx/evtloop.h>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CMiscSeqTable::CMiscSeqTable(CRef<CSeq_table> table, CSeq_entry_Handle entry, edit::CSeqIdGuesser  &id_guesser, CRef<CUser_object> column_properties, 
    const string& constraint_field, CRef<edit::CStringConstraint> string_constraint, ICommandProccessor* cmd_processor)
    : m_Table(table), m_Seh(entry), m_id_guesser(id_guesser), m_ColProperties(column_properties), 
    m_ConstrField(constraint_field), m_StrConstraint(string_constraint),
    m_CmdProccessor(cmd_processor)
{
}

void CMiscSeqTable::ApplyTableToEntry()
{
    CRef<CMacroCmdComposite> apply_cmd(new CMacroCmdComposite("Apply table to entry"));
    bool status = false;
    try {
        status = x_ApplyMiscSeqTableToSeqEntry(apply_cmd);
    }
    catch (CException& e) {
        apply_cmd->Execute();
        apply_cmd->Unexecute();
        NCBI_RETHROW_SAME(e, e.GetMsg());
    }
    
    if (status) {
        // Introduce Undo Manager or undo changes in case of error
        if (m_CmdProccessor) {
            m_CmdProccessor->Execute(apply_cmd);
        }
    }
    else {
        // There was a problem and Undo Manager was not involved
        apply_cmd->Execute(); // this call resets state to let Unexecute be run
        apply_cmd->Unexecute();
        NcbiMessageBox("Unable to apply table");
    }
}

namespace {
    const char* kMatchString = "match ";

    string s_AdjustFieldName(const string& field_name, CFieldNamePanel::EFieldType field_type)
    {
        string adjust_field = field_name;

        if (field_type != CFieldNamePanel::eFieldType_CDSGeneProt &&
            field_type != CFieldNamePanel::eFieldType_Pub &&
            field_type != CFieldNamePanel::eFieldType_StructuredComment &&
            field_type != CFieldNamePanel::eFieldType_Misc) {
            size_t pos = adjust_field.find(' ');
            if (pos != string::npos) {
                adjust_field = adjust_field.substr(pos + 1, string::npos);
            }
        }
        return adjust_field;
    }

    bool s_GetFieldType(string field_name, CFieldNamePanel::EFieldType& guess_field_type)
    {
        switch (guess_field_type) {
        case CFieldNamePanel::eFieldType_Unknown:
        {
            vector<CFieldNamePanel::EFieldType> guess_preference;
            guess_preference.push_back(CFieldNamePanel::eFieldType_Source);
            guess_preference.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
            guess_preference.push_back(CFieldNamePanel::eFieldType_RNA);
            guess_preference.push_back(CFieldNamePanel::eFieldType_Feature);
            guess_preference.push_back(CFieldNamePanel::eFieldType_Misc);
            guess_preference.push_back(CFieldNamePanel::eFieldType_Pub);
            guess_preference.push_back(CFieldNamePanel::eFieldType_DBLink);
            guess_preference.push_back(CFieldNamePanel::eFieldType_MolInfo);
            for (auto& it : guess_preference) {
                if (s_GetFieldType(field_name, it)) {
                    guess_field_type = it;
                    return true;
                }
            }
        }
        break;
        case CFieldNamePanel::eFieldType_Source:
            return (!CFieldChoicePanel::AutoMatchSrc(field_name).empty());
        case CFieldNamePanel::eFieldType_Feature:
            return (!CFieldChoicePanel::AutoMatchFeat(field_name).empty());
        case CFieldNamePanel::eFieldType_CDSGeneProt:
            return (!CFieldChoicePanel::AutoMatchCGP(field_name).empty());
        case CFieldNamePanel::eFieldType_RNA:
            return (!CFieldChoicePanel::AutoMatchRNA(field_name).empty());
        case CFieldNamePanel::eFieldType_Misc:
            return (!CFieldChoicePanel::AutoMatchMisc(field_name).empty());
        case CFieldNamePanel::eFieldType_Pub:
            return (!CFieldChoicePanel::AutoMatchPub(field_name).empty());
        case CFieldNamePanel::eFieldType_DBLink:
            return (!CFieldChoicePanel::AutoMatchDBLink(field_name).empty());
        case CFieldNamePanel::eFieldType_MolInfo:
            return (!CFieldChoicePanel::AutoMatchMolInfo(field_name).empty());
        default:
            break;
        }

        return false;
    }

    CFieldNamePanel::EFieldType s_GuessFieldType(const string& field_name)
    {
        CFieldNamePanel::EFieldType type = CFieldNamePanel::eFieldType_Unknown;
        if (NStr::EqualNocase(field_name, "taxname")) {
            type = CFieldNamePanel::eFieldType_Source;
        }
        else if (NStr::EqualNocase(field_name, "protein name")) {
            type = CFieldNamePanel::eFieldType_CDSGeneProt;
        }
        else if (NStr::StartsWith(field_name, "author ", NStr::eNocase) || 
            NStr::EqualNocase(field_name, "journal") ||
            NStr::EqualNocase(field_name, "volume") || NStr::EqualNocase(field_name, "issue") ||
            NStr::EqualNocase(field_name, "pages") || NStr::EqualNocase(field_name, "title") ||
            NStr::EqualNocase(field_name, "pmid") || NStr::EqualNocase(field_name, "status")) {
            type = CFieldNamePanel::eFieldType_Pub;
        }
        else if (NStr::StartsWith(field_name, "Structured", NStr::eNocase)) {
            type = CFieldNamePanel::eFieldType_StructuredComment;
        }
        else if (NStr::StartsWith(field_name, kDefinitionLineLabel, NStr::eNocase)) {
            type = CFieldNamePanel::eFieldType_DefLine;
        }
        else if (NStr::EqualNocase(field_name, kBankITComment)) {
            type = CFieldNamePanel::eFieldType_BankITComment;
        }
        else if (NStr::EqualNocase(field_name, kComment)) {
            type = CFieldNamePanel::eFieldType_Comment;
        }
        else if (NStr::EqualNocase(field_name, kFileSeqId)) {
            type = CFieldNamePanel::eFieldType_FileId;
        }
        else if (NStr::EqualNocase(field_name, kGeneralId)) {
            type = CFieldNamePanel::eFieldType_GeneralId;
        }
        else if (NStr::EqualNocase(field_name, kLocalId)) {
            type = CFieldNamePanel::eFieldType_LocalId;
        }
        else if (NStr::EqualNocase(field_name, kFieldTypeSeqId)
            || NStr::EqualNocase(field_name, "accession")) {
            type = CFieldNamePanel::eFieldType_SeqId;
        }
        return type;
    }

    CFieldNamePanel::EFieldType s_GetFieldType(const string& field_name)
    {
        // Note: field_name does not contain the field type names, as in 'Source qualifier acronym'
        auto field_type = s_GuessFieldType(field_name);

        if (field_type == CFieldNamePanel::eFieldType_Unknown) {
            s_GetFieldType(field_name, field_type);
        }

        return field_type;
    }

    int s_GetSubtype(const string& field_name, CFieldNamePanel::EFieldType field_type)
    {
        int subtype = -1;

        if (field_type == CFieldNamePanel::eFieldType_Feature) {
            string field(field_name);
            size_t pos = field_name.find(' ');
            if (pos != string::npos) {
                field = field_name.substr(0, pos);
            }
            return CSeqFeatData::SubtypeNameToValue(field);
        }
        else if (field_type == CFieldNamePanel::eFieldType_CDSGeneProt) {
            if (NStr::StartsWith(field_name, "protein")) {
                return CSeqFeatData::eSubtype_prot;
            }
            else if (NStr::StartsWith(field_name, "CDS") || field_name == "codon-start") {
                return CSeqFeatData::eSubtype_cdregion;
            }
            else if (NStr::StartsWith(field_name, "gene")) {
                return CSeqFeatData::eSubtype_gene;
            }
            else if (NStr::StartsWith(field_name, "mRNA")) {
                return CSeqFeatData::eSubtype_mRNA;
            }
            else if (NStr::StartsWith(field_name, "mat_peptide")) {
                return CSeqFeatData::eSubtype_mat_peptide_aa;
            }
        }
        else if (field_type == CFieldNamePanel::eFieldType_RNA) {
            if (NStr::StartsWith(field_name, "preRNA")) {
                return CSeqFeatData::eSubtype_preRNA;
            }
            else if (NStr::StartsWith(field_name, "mRNA")) {
                return CSeqFeatData::eSubtype_mRNA;
            }
            else if (NStr::StartsWith(field_name, "tRNA")) {
                return CSeqFeatData::eSubtype_tRNA;
            }
            else if (NStr::StartsWith(field_name, "rRNA")) {
                return CSeqFeatData::eSubtype_rRNA;
            }
            else if (NStr::StartsWith(field_name, "ncRNA")) {
                if (field_name.find("snRNA ") != string::npos) {
                    return CSeqFeatData::eSubtype_snRNA;
                }
                else if (field_name.find("scRNA ") != string::npos) {
                    return  CSeqFeatData::eSubtype_scRNA;
                }
                else if (field_name.find("snoRNA ") != string::npos) {
                    return CSeqFeatData::eSubtype_snoRNA;
                }
                return CSeqFeatData::eSubtype_ncRNA;
            }
            else if (NStr::StartsWith(field_name, "tmRNA")) {
                return CSeqFeatData::eSubtype_tmRNA;
            }
            else if (NStr::StartsWith(field_name, "miscRNA")) {
                return CSeqFeatData::eSubtype_otherRNA;
            }
        }
        return subtype;
    }


    string s_GetPropFromMetaInfo(const string& meta_string, const string& prop_name)
    {
        string value;

        string tag = "&" + prop_name + "=";

        size_t tag_idx = meta_string.find(tag);
        if (tag_idx != string::npos) {
            // End of value is the start of the next value qualifier (this allows
            // embedded blanks in values)
            size_t tag_end_idx = meta_string.find_first_of("&", tag_idx + 1);
            size_t start_idx = tag_idx + string(tag).length();

            if (tag_end_idx == string::npos) {
                value = meta_string.substr(start_idx, meta_string.length() - start_idx);
            }
            else {
                value = meta_string.substr(start_idx, tag_end_idx - start_idx);
            }
        }

        return value;
    }

    edit::EExistingText s_GetExistingTextInstructionFromText(const string& update_rule, const string& separator)
    {
        edit::EExistingText rval = edit::eExistingText_cancel;

        if (NStr::EqualNocase(update_rule, "append")) {
            if (NStr::Equal(separator, ";")) {
                rval = edit::eExistingText_append_semi;
            }
            else if (NStr::Equal(separator, " ")) {
                rval = edit::eExistingText_append_space;
            }
            else if (NStr::Equal(separator, ":")) {
                rval = edit::eExistingText_append_colon;
            }
            else if (NStr::Equal(separator, ",")) {
                rval = edit::eExistingText_append_comma;
            }
            else if (NStr::Equal(separator, "")) {
                rval = edit::eExistingText_append_none;
            }
        }
        else if (NStr::Equal(update_rule, "prefix")) {
            if (NStr::Equal(separator, ";")) {
                rval = edit::eExistingText_prefix_semi;
            }
            else if (NStr::Equal(separator, " ")) {
                rval = edit::eExistingText_prefix_space;
            }
            else if (NStr::Equal(separator, ":")) {
                rval = edit::eExistingText_prefix_colon;
            }
            else if (NStr::Equal(separator, ",")) {
                rval = edit::eExistingText_prefix_comma;
            }
            else if (NStr::Equal(separator, "")) {
                rval = edit::eExistingText_prefix_none;
            }
        }
        else if (NStr::Equal(update_rule, "ignore")) {
            rval = edit::eExistingText_leave_old;
        }
        else if (NStr::Equal(update_rule, "add_new_qual")) {
            rval = edit::eExistingText_add_qual;
        }
        else if (NStr::Equal(update_rule, "replace")) {
            rval = edit::eExistingText_replace_old;
        }

        return rval;
    }

    bool s_IsSequenceIDField(const string& field)
    {
        return (QualifierNamesAreEquivalent(field, kSequenceIdColLabel)
            || QualifierNamesAreEquivalent(field, kFieldTypeSeqId));
    }
} // end of namespace

void  CMiscSeqTable::x_CheckDuplicateIds(CRef<CSeqTable_column> id_col)
{
    set<string> seen_id, duplicate_id;
    for (int row = 0; row < m_Table->GetNum_rows(); row++) {
        string id_value;

        if (id_col->GetData().IsId()) {
            id_col->GetData().GetId()[row]->GetLabel(&id_value, CSeq_id::eContent);
        }
        else if (id_col->GetData().IsInt()) {
            id_value = NStr::NumericToString(id_col->GetData().GetInt()[row]);
        }
        else if (id_col->GetData().IsString()) {
            id_value = id_col->GetData().GetString()[row];
        }

        if (!NStr::IsBlank(id_value)) {
            if (seen_id.find(id_value) != seen_id.end()) {
                duplicate_id.insert(id_value);
            }
            seen_id.insert(id_value);
        }
    }
    if (!duplicate_id.empty())  {
        CGenericReportDlg* report = new CGenericReportDlg(NULL); 
        report->SetTitle(wxT("Duplicate row ids"));
        report->SetText(wxString(NStr::Join(duplicate_id, "\n")));
        report->Show(true);
        NCBI_THROW(CException, eInvalid, "Multiple rows apply to the same object. Cannot continue.");
    }
}

bool CMiscSeqTable::x_ApplyMiscSeqTableToSeqEntry(CRef<CMacroCmdComposite> apply_cmd)
{
    wxBusyCursor wait;
    if (!wxEventLoopBase::GetActive() || !wxEventLoopBase::GetActive()->IsYielding())
        wxTheApp->Yield();
    bool modified = false;

    size_t id_col_num = x_FindIDColumn();
    
    x_CreateEditingActions(id_col_num);
    bool desc_present = false;
    for (auto action : m_ActionList)
    {
	if (action && dynamic_cast<IEditingActionDesc*>(action.GetPointer()))
	    desc_present = true;
    }
    if (desc_present)
    {
	CRef<CCmdComposite> cmd = CPropagateDescriptors::GetPropagateDownCommand(m_Seh);
	if (cmd)
	{
	    cmd->Execute();
	    apply_cmd->AddCommand(*cmd);
	}
    }
    x_InterpretColumnProperties(id_col_num);
    x_GetIdColumnProps(id_col_num);
    map<string, set<CSeq_entry_Handle> > value_to_sehs;
    s_GetValueToSehMap(m_Seh, value_to_sehs, m_IdFieldName, m_IdFieldType, m_IdSubtype);

    wxGenericProgressDialog progress("Applying table values", wxEmptyString);
    CRef<CSeqTable_column> id_col = m_Table->GetColumns()[id_col_num];
    x_CheckDuplicateIds(id_col);
    for (int row = 0; row < m_Table->GetNum_rows(); row++) {
        string id_value;

        if (id_col->GetData().IsId()) {
            id_col->GetData().GetId()[row]->GetLabel(&id_value, CSeq_id::eContent);
        }
        else if (id_col->GetData().IsInt()) {
            id_value = NStr::NumericToString(id_col->GetData().GetInt()[row]);
        }
        else if (id_col->GetData().IsString()) {
            id_value = id_col->GetData().GetString()[row];
        }

       
        CRef<edit::CStringConstraint> id_constraint(new edit::CStringConstraint(id_value, edit::CStringConstraint::eMatchType_Equals));
        set<CSeq_entry_Handle> sehs;
        x_SetTopSeqEntryForActions(id_value, sehs, value_to_sehs);
        for (auto &seh : sehs)
        {
            for (auto& it : m_ActionList) 
            {
                if (it)
                    it->SetTopSeqEntry(seh);
            }
            modified |= x_SetConstraints(id_col_num, id_constraint) && x_ApplyRowToEntry(row, id_col_num, apply_cmd);
            for (auto& it : m_ActionList) 
            {
                if (it)
                {
                    it->ResetChangedDescriptors();
                    it->ResetChangedFeatures();
                }
            }
        }
	if (row % 100 == 0)
	{
	    progress.Update(100*row/m_Table->GetNum_rows());
	    if (!wxEventLoopBase::GetActive() || !wxEventLoopBase::GetActive()->IsYielding())
		wxTheApp->Yield();
	}
    }
    if (desc_present)
    {
	CRef<CCmdComposite> cmd = CPropagateDescriptors::GetPropagateUpCommand(m_Seh);
	if (cmd)
	{
	    cmd->Execute();
	    apply_cmd->AddCommand(*cmd);
	}
    }

    return modified;
}

size_t CMiscSeqTable::x_FindIDColumn()
{
    for (size_t col_num = 0; col_num < m_Table->GetColumns().size(); col_num++) {
        if (NStr::StartsWith(m_Table->GetColumns()[col_num]->GetHeader().GetTitle(), kMatchString)) {
            return col_num;
        }
    }
    // by default, id column is 0 if no other column is specified
    return 0;
}

void CMiscSeqTable::x_GetIdColumnProps(size_t id_col_num)
{
    CRef<CSeqTable_column> id_col = m_Table->GetColumns()[id_col_num];

    m_IdFieldName = id_col->GetHeader().GetTitle();
    if (NStr::StartsWith(m_IdFieldName, kMatchString)) {
        m_IdFieldName = m_IdFieldName.substr(CTempString(kMatchString).length());
    }

    m_IdFieldType = s_GetFieldType(m_IdFieldName);
    m_IdSubtype = s_GetSubtype(m_IdFieldName, m_IdFieldType);
    m_IdFieldName = s_AdjustFieldName(m_IdFieldName, m_IdFieldType);
}

void CMiscSeqTable::x_CreateEditingActions(size_t id_col_num)
{
    m_ActionList.clear();

    for (size_t col_num = 0; col_num < m_Table->GetColumns().size(); col_num++) {
        if (col_num != id_col_num) {
            string field_name;
            if (m_Table->GetColumns()[col_num]->IsSetHeader()
                && m_Table->GetColumns()[col_num]->GetHeader().IsSetTitle()) {
                field_name = m_Table->GetColumns()[col_num]->GetHeader().GetTitle();
            }

            CIRef<IEditingAction> action;
            if (!field_name.empty()) {
                CFieldNamePanel::EFieldType field_type = s_GetFieldType(field_name);
                int field_subtype = s_GetSubtype(field_name, field_type);
                field_name = s_AdjustFieldName(field_name, field_type);
                action = CreateAction(m_Seh, field_name, field_type, field_subtype);
            }
            _ASSERT(action);
            m_ActionList.push_back(action);
        }
    }
}

void CMiscSeqTable::x_InterpretColumnProperties(size_t id_col_num)
{
    m_EraseIfBlank.clear();
    m_ExistingText.clear();

    for (size_t col_num = 0; col_num < m_Table->GetColumns().size(); col_num++) {
        if (col_num != id_col_num) {
            if (col_num < m_ColProperties->GetData().size()) {
                const CUser_field& field_meta_info = m_ColProperties->GetData()[col_num].GetObject();

                string col_meta_info = field_meta_info.GetData().GetStr();
                string ur = s_GetPropFromMetaInfo(col_meta_info, "update_rule");
                string ts = s_GetPropFromMetaInfo(col_meta_info, "text_separator");
                m_ExistingText.push_back(s_GetExistingTextInstructionFromText(ur, ts));
                string blanks = s_GetPropFromMetaInfo(col_meta_info, "blanks");
                if (NStr::EqualNocase(blanks, "erase")) {
                    m_EraseIfBlank.push_back(true);
                }
                else {
                    m_EraseIfBlank.push_back(false);
                }
            }
            else {
                m_ExistingText.push_back(edit::eExistingText_cancel);
                m_EraseIfBlank.push_back(false);
            }
        }
    }
}

void CMiscSeqTable::x_SetTopSeqEntryForActions(const string& id_value, set<CSeq_entry_Handle> &sehs, const map<string, set<CSeq_entry_Handle> > &value_to_sehs)
{    
    sehs.clear();
    bool found(false);
    if (s_IsSequenceIDField(m_IdFieldName)) {
        CRef<CSeq_id> seqid = m_id_guesser.Guess(id_value);
        if (seqid) {
            CBioseq_Handle bsh = m_Seh.GetScope().GetBioseqHandle(*seqid);
            CSeq_entry_Handle bseh = bsh.GetSeq_entry_Handle();
            sehs.insert(bseh);
            found = true;
        }
    }

    if (!found) {
	auto it = value_to_sehs.find(id_value);
	if (it != value_to_sehs.end())
	    sehs = it->second;       
    }
}

void CMiscSeqTable::s_GetValueToSehMap(objects::CSeq_entry_Handle seh,  map<string, set<CSeq_entry_Handle> > &value_to_sehs, string id_field_name, CFieldNamePanel::EFieldType id_field_type, int id_subtype)
{
    value_to_sehs.clear();   
    if (s_IsSequenceIDField(id_field_name))
    {
        id_field_name = "Local ID";
        id_field_type = CFieldNamePanel::eFieldType_LocalId;
        id_subtype = -1;
    }


    CIRef<IEditingAction> id_action = CreateAction(seh, id_field_name, id_field_type, id_subtype);
    id_action->NOOP();
    
    const map<CSeq_feat_Handle, CRef<CSeq_feat>> feathdlesmap = id_action->GetChangedFeatures();
    const map<const CSeqdesc*, CRef<CSeqdesc>> descrmap = id_action->GetChangedDescriptors();
    
    for (auto &fh : feathdlesmap)
    {
	CBioseq_Handle bsh = seh.GetScope().GetBioseqHandle(fh.first.GetLocation());
	CSeq_entry_Handle bseh = bsh.GetSeq_entry_Handle();
	id_action->SetTopSeqEntry(bseh);
	id_action->NOOP();
	const vector<string>& values = id_action->GetChangedValues();
	for (auto v : values)
	    value_to_sehs[v].insert(bseh);
	id_action->ResetChangedValues();
    } 
    if (!descrmap.empty())
    {
	for ( CSeq_entry_CI entry_it(seh, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it ) 
	{
	    for ( CSeqdesc_CI desc_iter(*entry_it, CSeqdesc::e_not_set, 1); desc_iter; ++desc_iter) 
                {
                    if (descrmap.find(&*desc_iter) != descrmap.end())
                    {
                        CSeq_entry_Handle bseh = desc_iter.GetSeq_entry_Handle();
			id_action->SetTopSeqEntry(bseh);
			id_action->NOOP();
			const vector<string>& values = id_action->GetChangedValues();
			for (auto v : values)
			    value_to_sehs[v].insert(bseh);
			id_action->ResetChangedValues();
                    }
                }
	}
    }
}

bool CMiscSeqTable::x_SetConstraints(size_t id_col_num, CRef<edit::CStringConstraint> id_constraint)
{
    bool ID_is_Seqid = s_IsSequenceIDField(m_IdFieldName);
    if (!m_StrConstraint && ID_is_Seqid) {
        return true;
    }
    
    CFieldNamePanel::EFieldType constr_field_type = CFieldNamePanel::eFieldType_Unknown;
    int constr_field_subtype = -1;
    
    if (m_StrConstraint) {
        constr_field_type = s_GetFieldType(m_ConstrField);
        constr_field_subtype = s_GetSubtype(m_ConstrField, constr_field_type);
        m_ConstrField = s_AdjustFieldName(m_ConstrField, constr_field_type);
    }

    bool rval = true;
    TEditingActionVec::iterator iter = m_ActionList.begin();
    for (size_t col_num = 0; col_num < m_Table->GetColumns().size() && rval && iter != m_ActionList.end(); col_num++) {
        if (col_num != id_col_num) {
            string field_name;
            if (m_Table->GetColumns()[col_num]->IsSetHeader()
                && m_Table->GetColumns()[col_num]->GetHeader().IsSetTitle()) {
                field_name = m_Table->GetColumns()[col_num]->GetHeader().GetTitle();
            }

            if (!field_name.empty()) {
                CFieldNamePanel::EFieldType field_type = s_GetFieldType(field_name);
                int field_subtype = s_GetSubtype(field_name, field_type);
                field_name = s_AdjustFieldName(field_name, field_type);

                CRef<CEditingActionConstraint> constraint;
                if (m_StrConstraint) {
                    CRef<CEditingActionConstraint> field_constraint =
                        CreateEditingActionConstraint(field_name, field_type, field_subtype, m_ConstrField, constr_field_type, constr_field_subtype, CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(m_StrConstraint)));
                    if (!field_constraint) {
                        rval = false;
                    }
                    constraint = field_constraint;
                }

                if (!ID_is_Seqid) {
                    CRef<CEditingActionConstraint> idfield_constraint =
                        CreateEditingActionConstraint(field_name, field_type, field_subtype, m_IdFieldName, m_IdFieldType, m_IdSubtype, CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(id_constraint)));
                    if (!idfield_constraint) {
                        rval = false;
                    }
                    if (constraint) {
                        *constraint += idfield_constraint;
                    }
                    else {
                        constraint = idfield_constraint;
                    }
                }

                if (constraint && *iter) {
                    (*iter)->SetConstraint(constraint);
                }
            }
            ++iter;
        }
    }
    
    return rval;
}

bool CMiscSeqTable::x_ApplyRowToEntry(int row, size_t id_col_num, CRef<CMacroCmdComposite> apply_cmd)
{
    bool rval = false;
    size_t col_num = 0;

    vector<bool>::iterator eb_it = m_EraseIfBlank.begin();
    vector<edit::EExistingText>::iterator existing_text_it = m_ExistingText.begin();

    NON_CONST_ITERATE(vector<CIRef<IEditingAction>>, action, m_ActionList) {
        string new_val;
        if (col_num < id_col_num) {
            new_val = m_Table->GetColumns()[col_num]->GetData().GetString()[row];
        }
        else {
            new_val = m_Table->GetColumns()[col_num + 1]->GetData().GetString()[row];
        }

        if (*action)
        {
            if (new_val.empty()) {
                rval = true;
                if (*eb_it) {
                    (*action)->Remove();
                    CRef<CCmdComposite> cmd = (*action)->GetActionCommand();
                    if (cmd)
                    {
                        cmd->Execute();
                        apply_cmd->AddCommand(*cmd);
                    }
                }
            }
            else if (!new_val.empty()) {
                (*action)->SetExistingText(*existing_text_it);
                (*action)->Apply(new_val);
                CRef<CCmdComposite> cmd = (*action)->GetActionCommand();
                if (cmd)
                {
                    cmd->Execute();
                    apply_cmd->AddCommand(*cmd);
                }
                rval = true;
            }
        }

        col_num++;
        eb_it++;
        existing_text_it++;
    }

    return rval;
}

END_NCBI_SCOPE

