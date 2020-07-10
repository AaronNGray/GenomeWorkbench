/*  $Id: macro_applytbl_itemdata.cpp 44731 2020-03-03 15:53:48Z asztalos $
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
 * Authors:  Andrea Asztalos

 */


#include <ncbi_pch.hpp>

#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_applysrc_table.hpp>
#include <gui/widgets/edit/macro_applytbl_itemdata.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(macro);
USING_SCOPE(NMacroArgs);

namespace {
    string GetExistingTextVariables(const CArgumentList& args)
    {
        string variables;
        variables = kExistingText + " = \"" + NMItemData::GetHandleOldValue(args[kExistingText].GetValue()) + "\"";
        if (args[kDelimiter].GetEnabled()) {
            variables += "\n" + kDelimiter + " = \"" + NMItemData::GetDelimiterChar(args[kDelimiter].GetValue()) + "\"";
        }
        variables += "\n" + kRmvBlank + " = " + NMItemData::GetHandleBlank(args[kRmvBlank].GetValue()) + "";
        return variables;
    }

    const string kColDel = "col_del";
    const string kMergeDel = "merge_del";
    const string kSplitFirst = "split_first";
    const string kConvertMulti = "convert_multi";
    const string kMergeFirst = "merge_first";

    bool AddMatchField(const CMacroApplySrcTablePanel& panel, IMacroScriptElements::TConstraints& constraints, EMacroFieldType field_type, const string& target = kEmptyStr)
    {
        auto match_field = panel.GetMatchField();
        string del = panel.GetDelimiter();
        if (del == "\t") del = "\\t";

        string new_constraint;
        if (match_field.m_UseMacroName) {
            new_constraint = CMacroFunction_InTable::GetFuncName() + "(";
            new_constraint += match_field.m_MacroName;
        }
        else {
            string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(match_field.m_GUIName, field_type, target);
            new_constraint = CMacroFunction_InTable::GetFuncName() + "(";
            if (path.find(',') != NPOS) {
                vector<string> tokens;
                NStr::Split(path, ",", tokens);
                new_constraint += "\"" + tokens[0] + "\", \"" + tokens[1] + "\"";
            }
            else if (path.find('(') != NPOS) {
                new_constraint += path;
            }
            else {
                new_constraint += "\"" + path + "\"";
            }
        }

        if (!new_constraint.empty()) {
            new_constraint += ", " + kTableName + ", " + match_field.m_Column;
            new_constraint += ", \"" + del + "\"";
            new_constraint += ", " + kMergeDel + ", " + kSplitFirst + ", " + kConvertMulti;
            new_constraint += ", " + kMergeFirst + ")";

            constraints.insert(constraints.begin(), make_pair(kEmptyStr, new_constraint));
        }
        return (!new_constraint.empty());
    }

    void UpdateMatchFieldUsage(SFieldFromTable& match_field, const CMacroApplySrcTablePanel& panel)
    {
        // decide which fieldname to be used for matching field
        string match_feat, match_fieldname;
        string target_feat, target_fieldname;  // this will be the FIRST feature and fieldname to be applied from the table

        if (!match_field.m_GUIName.empty()) {
            vector<string> tokens;
            NStr::Split(match_field.m_GUIName, " ", tokens);
            if (tokens.size() == 2) {
                match_feat = tokens[0];
                match_fieldname = tokens[1];
            }
        }

        auto values = panel.GetValues();
        if (!values.empty()) {
            vector<string> tokens;
            NStr::Split(values.front().first, " ", tokens);
            if (tokens.size() == 2) {
                target_feat = tokens[0];
                target_fieldname = tokens[1];
            }
        }

        NMItemData::UpdateFeatureType(match_feat, match_fieldname);
        NMItemData::UpdateFeatureType(target_feat, target_fieldname);

        if (!match_feat.empty() && !target_feat.empty()) {
            match_field.m_UseMacroName = !NStr::EqualNocase(match_feat, target_feat);
        }
    }

    bool AddMatchFieldForFeatures(const CMacroApplySrcTablePanel& panel, IMacroScriptElements::TConstraints& constraints, EMacroFieldType field_type, const string& target = kEmptyStr)
    {
        auto match_field = panel.GetMatchField();
        UpdateMatchFieldUsage(match_field, panel);

        string del = panel.GetDelimiter();
        if (del == "\t") del = "\\t";

        string new_constraint;
        if (match_field.m_UseMacroName) {
            new_constraint = CMacroFunction_InTable::GetFuncName() + "(";
            new_constraint += match_field.m_MacroName;
        }
        else {
            string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(match_field.m_GUIName, field_type, target);
            new_constraint = CMacroFunction_InTable::GetFuncName() + "(";
            if (path.find(',') != NPOS) {
                vector<string> tokens;
                NStr::Split(path, ",", tokens);
                new_constraint += "\"" + tokens[0] + "\", \"" + tokens[1] + "\"";
            }
            else if (path.find('(') != NPOS) {
                new_constraint += path;
            }
            else {
                new_constraint += "\"" + path + "\"";
            }
        }

        if (!new_constraint.empty()) {
            new_constraint += ", " + kTableName + ", " + match_field.m_Column;
            new_constraint += ", \"" + del + "\"";
            new_constraint += ", " + kMergeDel + ", " + kSplitFirst + ", " + kConvertMulti;
            new_constraint += ", " + kMergeFirst + ")";

            constraints.insert(constraints.begin(), make_pair(kEmptyStr, new_constraint));
        }
        return (!new_constraint.empty());
    }

    bool AddMatchFieldForRnas(const CMacroApplySrcTablePanel& panel, 
        IMacroScriptElements::TConstraints& constraints, 
        const string& target,
        const string& target_rnatype,
        const string& target_ncRNAclass)
    {
        auto match_field = panel.GetMatchField();
        string del = panel.GetDelimiter();
        if (del == "\t") del = "\\t";

        // the RNA type of the matching field and of the applied values should be the same
        if (!NStr::EqualNocase(match_field.m_GUIName, "SeqId")) {
            string rna_type, ncRNA_class, rna_qual;
            NMItemData::GetRNASpecificFields(match_field.m_GUIName, rna_type, ncRNA_class, rna_qual);
            if (rna_type != target_rnatype || ncRNA_class != target_ncRNAclass) {
                NcbiWarningBox("Applied values and the matching field should refer to the same RNA type");
                return false;
            }
            if (NStr::StartsWith(rna_qual, "gene") ||
                !ncRNA_class.empty() ||
                NStr::StartsWith(match_field.m_MacroName, macro::CMacroFunction_GetRnaProduct::GetFuncName())) {
                match_field.m_UseMacroName = true;
            }
        }

        string new_constraint;
        if (match_field.m_UseMacroName) {
            new_constraint = CMacroFunction_InTable::GetFuncName() + "(";
            new_constraint += match_field.m_MacroName;
        }
        else {
            string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(match_field.m_GUIName, EMacroFieldType::eRNA, target);
            new_constraint = CMacroFunction_InTable::GetFuncName() + "(";
            if (path.find(',') != NPOS) {
                vector<string> tokens;
                NStr::Split(path, ",", tokens);
                new_constraint += "\"" + tokens[0] + "\", \"" + tokens[1] + "\"";
            }
            else if (path.find('(') != NPOS) {
                new_constraint += path;
            }
            else {
                new_constraint += "\"" + path + "\"";
            }
        }

        if (!new_constraint.empty()) {
            new_constraint += ", " + kTableName + ", " + match_field.m_Column;
            new_constraint += ", \"" + del + "\"";
            new_constraint += ", " + kMergeDel + ", " + kSplitFirst + ", " + kConvertMulti;
            new_constraint += ", " + kMergeFirst + ")";

            constraints.insert(constraints.begin(), make_pair(kEmptyStr, new_constraint));
        }
        return (!new_constraint.empty());
    }
}

void CApplyTableItemData::x_LoadParamPanel(wxWindow* parent, ECustomPanelType type)
{
    x_LoadPanel(parent, "ApplyQualTable", GetApplyTableArgs());
    m_TablePanel = dynamic_cast<CMacroApplySrcTablePanel*>(m_Panel->LoadCustomPanel(type));
    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CApplyTableItemData::GetMacroDescription() const
{
    string descr;
    if (m_TablePanel) {
        auto match_field = m_TablePanel->GetMatchField();
        descr = "Apply table from file " + m_TablePanel->GetFilename();

#if defined __WXMSW__
        descr += " (WINDOWS)";
#elif defined __WXGTK__
        descr += " (LINUX)";
#elif defined __WXMAC__
        descr += " (MAC)";
#else
        descr += " ()"
#endif 
        descr += ", match to " + match_field.m_GUIName;
    }
    return descr;
}

string CApplyTableItemData::GetVariables()
{
    if (m_TablePanel) {
        string variables;
        variables += kTableName + " = \"" + m_TablePanel->GetFilename() + "\"\n";
        variables += kMergeDel + " = " + NStr::BoolToString(m_TablePanel->IsMergeDelimiterSet()) + "\n";
        variables += kSplitFirst + " = " + NStr::BoolToString(m_TablePanel->IsSplitFirstSet()) + "\n";
        variables += kConvertMulti + " = " + NStr::BoolToString(m_TablePanel->IsConvertMultiSet()) + "\n";
        variables += kMergeFirst + " = " + NStr::BoolToString(m_TablePanel->IsMergeFirstSet()) + "\n";
        variables += GetExistingTextVariables(m_ArgList);
        return variables;
    }
    return kEmptyStr;
}

// CApplySrcTableTreeItemData
CApplySrcTableTreeItemData::CApplySrcTableTreeItemData()
    : CApplyTableItemData("Apply source qualifier table", EMacroFieldType::eBiosourceText)
{
}

void CApplySrcTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadParamPanel(parent, ECustomPanelType::eApplySrcQualTable);
}

string CApplySrcTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchField(*m_TablePanel, constraints, EMacroFieldType::eBiosourceAll)) return kEmptyStr;

    string function;
    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        string rt_var = "value";
        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);
        
        string field_name = it.first;
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field_name, EMacroFieldType::eBiosourceAll);
        if (NMacroUtil::IsStructVoucherPart(field_name)) {
            string obj_var = "obj_str";
            bool found_constraint = false;
            string resolve_func = NMItemData::GetResolveFuncForSVPart(field_name, obj_var, constraints, found_constraint);
            if (found_constraint) {
                function += resolve_func;
            }

            auto pos = field_name.find_last_of('-');
            function += CMacroFunction_ApplyStrucVoucherPart::GetFuncName();
            if (found_constraint) {
                function += "(" + obj_var + ", ";
            }
            else {
                function += "(\"" + field_name.substr(0, pos) + "\", ";
            }
            function += "\"" + field_name.substr(pos + 1) + "\", ";

        } else if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            // it is either an orgmod or a soubsource modifier
            string rt_obj = "obj";
            function += NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_obj, constraints);

            function += CMacroFunction_AddorSetBsrcModifier::GetFuncName();
            function += "(" + rt_obj + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", ";
        }
        else {
            function += CMacroFunction_SetStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }

        function += rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ", " + kRmvBlank + ");";
    }

    return function;
}


// CApplyStrCommTableTreeItemData
CApplyStrCommTableTreeItemData::CApplyStrCommTableTreeItemData()
    : CApplyTableItemData("Apply structured comment qualifier table", EMacroFieldType::eStructComment)
{
}

void CApplyStrCommTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadParamPanel(parent, ECustomPanelType::eApplyStrCommTable);
}

string CApplyStrCommTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchField(*m_TablePanel, constraints, EMacroFieldType::eStructComment)) return kEmptyStr;

    string function;
    const string strcomm_field = "Structured comment Field";
    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        string rt_var = "value";
        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);

        string field = it.first;

        if (NStr::EqualCase(field, "Structured comment Database Name")) {
            function += CMacroFunction_SetStructCommDb::GetFuncName() + "(";
        }
        else if (NStr::EqualCase(field, "Structured comment Field Name")) {
            string field_var = "field";
            function += NMItemData::GetResolveFuncForStrCommQual("Field name", kEmptyStr, field_var, constraints) + "\n";
            function += CMacroFunction_SetStringQual::GetFuncName();
            function += "(\"" + field_var + ".label.str\", ";

        } else if (NStr::StartsWith(field, strcomm_field)) {
            string field_type = field.substr(strcomm_field.length() + 1);
            function += CMacroFunction_SetStructCommField::GetFuncName();
            function += "(\"" + field_type + "\", ";
        }
        
        function += rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ", " + kRmvBlank + ");";
    }
    return function;
}


// CApplyPubTableTreeItemData
CApplyPubTableTreeItemData::CApplyPubTableTreeItemData()
    : CApplyTableItemData("Apply publication qualifier table", EMacroFieldType::ePubdesc)
{
}

void CApplyPubTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadParamPanel(parent, ECustomPanelType::eApplyPubQualTable);
}

string CApplyPubTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchField(*m_TablePanel, constraints, EMacroFieldType::ePubdesc)) return kEmptyStr;

    string function;
    string rt_var = "value";
    string func_end;
    {
        func_end = rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            func_end += ", " + kDelimiter;
        }
        func_end += ", " + kRmvBlank + ");";
    }
    for (auto& it : values) {
        if (!function.empty())
            function += "\n";
        
        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);

        string field = it.first;
        CPubFieldType::EPubFieldType type = CPubFieldType::GetTypeForLabel(field);
        switch (type) {
        case CPubFieldType::ePubFieldType_Pmid:
            function += CMacroFunction_SetPubPMID::GetFuncName() + "(" + rt_var + ", " + kRmvBlank + ");";
            break;
        case CPubFieldType::ePubFieldType_Date:
            //TODO: not handled
            break;
        case CPubFieldType::ePubFieldType_DateYear:
            function += CMacroFunction_SetPubDateField::GetFuncName() + "(\"year\"," + rt_var + ", " + kRmvBlank + ");";
            break;
        case CPubFieldType::ePubFieldType_DateMonth:
            function += CMacroFunction_SetPubDateField::GetFuncName() + "(\"month\"," + rt_var + ", " + kRmvBlank + ");";
            break;
        case CPubFieldType::ePubFieldType_DateDay:
            function += CMacroFunction_SetPubDateField::GetFuncName() + "(\"day\"," + rt_var + ", " + kRmvBlank + ");";
            break;
        case CPubFieldType::ePubFieldType_Status:
            // it does not handle empty values
            function += CMacroFunction_SetPubStatus::GetFuncName() + "(" + rt_var + ");";
            break;
        case CPubFieldType::ePubFieldType_Authors:
            function += CMacroFunction_AddAuthorList::GetFuncName() + "(" + rt_var + ", " + kRmvBlank + ");";
            break;
        case CPubFieldType::ePubFieldType_Title: {
            string title_var;
            function += NMItemData::GetResolveFuncForPubQual(field, title_var, constraints);
            function += CMacroFunction_SetPubTitle::GetFuncName() + "(" + title_var + ", " + func_end;
            break;
        } 
        case CPubFieldType::ePubFieldType_AuthorLastName:
        case CPubFieldType::ePubFieldType_AuthorFirstName:
        case CPubFieldType::ePubFieldType_AuthorSuffix:
        case CPubFieldType::ePubFieldType_AuthorConsortium: {
            string auth_var = "auth_obj";
            function += NMItemData::GetResolveFuncForApplyAuthors(type, auth_var, constraints);

            function += CMacroFunction_SetPubAuthor::GetFuncName();
            function += "(" + auth_var + ", \"" + CMacroEditorContext::GetInstance().GetAsnPathToAuthorFieldName(field) + "\", " + func_end;
            break;
        }
        case CPubFieldType::ePubFieldType_AuthorMiddleInitial: {
            string mi_var = "mi_obj";
            function += NMItemData::GetResolveFuncForApplyAuthors(type, mi_var, constraints);
            function += CMacroFunction_SetPubAuthorMI::GetFuncName() + "(" + mi_var + ", " + func_end;
            break;
        }
        case CPubFieldType::ePubFieldType_Journal:
            function += CMacroFunction_SetPubJournal::GetFuncName() + "(" + func_end;
            break;
        case CPubFieldType::ePubFieldType_Volume:
            function += CMacroFunction_SetPubVolIssuePages::sm_FuncVolume + string("(") + func_end;
            break;
        case CPubFieldType::ePubFieldType_Issue:
            function += CMacroFunction_SetPubVolIssuePages::sm_FuncIssue + string("(") + func_end;
            break;
        case CPubFieldType::ePubFieldType_Pages:
            function += CMacroFunction_SetPubVolIssuePages::sm_FuncPages + string("(") + func_end;
            break;
        case CPubFieldType::ePubFieldType_SerialNumber:
            function += CMacroFunction_SetSerialNumber::GetFuncName() + "(" + func_end;
            break;
        case CPubFieldType::ePubFieldType_Citation:
            function += CMacroFunction_SetPubCitation::GetFuncName() + "(" + func_end;
            break;
        default:
            function += CMacroFunction_SetPubAffil::GetFuncName();
            function += "(\"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, EMacroFieldType::ePubAffilFields) + "\", " + func_end;
        }
    }
    return function;
}


// CApplyDBlinkTableTreeItemData
CApplyDBlinkTableTreeItemData::CApplyDBlinkTableTreeItemData()
    : CApplyTableItemData("Apply DBLink field table", EMacroFieldType::eDBLink)
{
}

void CApplyDBlinkTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadParamPanel(parent, ECustomPanelType::eApplyDBLinkTable);
}

string CApplyDBlinkTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchField(*m_TablePanel, constraints, EMacroFieldType::eDBLink, macro::CMacroBioData::sm_SeqNa)) return kEmptyStr;

    string function;
    string rt_var = "value";
    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);
        function += CMacroFunction_AddDBLink::GetFuncName() + "(\"" + it.first + "\"";
        function += ", " + rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ", " + kRmvBlank + ");";
    }
    return function;
}


// CApplyMolinfoTableTreeItemData
CApplyMolinfoTableTreeItemData::CApplyMolinfoTableTreeItemData()
    : CApplyTableItemData("Apply molinfo qualifier table", EMacroFieldType::eMolinfo)
{
}

void CApplyMolinfoTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadParamPanel(parent, ECustomPanelType::eApplyMolinfoQualTable);
}


string CApplyMolinfoTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchField(*m_TablePanel, constraints, EMacroFieldType::eMolinfo, macro::CMacroBioData::sm_Seq)) return kEmptyStr;

    string function;
    string rt_var = "value";
    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);

        string field_name = it.first;
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field_name, EMacroFieldType::eMolinfo, macro::CMacroBioData::sm_Seq);
        function += CMacroFunction_SetStringQual::GetFuncName();
        function += "(\"" + path + "\", ";
        function += rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ", " + kRmvBlank + ");";
    }
    return function;
}


// CApplyMiscTableTreeItemData
CApplyMiscTableTreeItemData::CApplyMiscTableTreeItemData()
    : CApplyTableItemData("Apply misc qualifier table", EMacroFieldType::eMiscDescriptors)
{
}

void CApplyMiscTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadParamPanel(parent, ECustomPanelType::eApplyMiscQualTable);
}

string CApplyMiscTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchField(*m_TablePanel, constraints, EMacroFieldType::eMiscDescriptors, macro::CMacroBioData::sm_SeqNa)) return kEmptyStr;

    string function;
    string rt_var = "value";
    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);

        string field_name = it.first;
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field_name, EMacroFieldType::eMiscDescriptors, macro::CMacroBioData::sm_SeqNa);
        function += CMacroFunction_SetStringQual::GetFuncName();
        function += "(\"" + path + "\", ";
        function += rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ", " + kRmvBlank + ");";
    }
    return function;
}

// CApplyFeatTableTreeItemData
CApplyFeatTableTreeItemData::CApplyFeatTableTreeItemData()
    : CApplyTableItemData("Apply feature qualifier table", EMacroFieldType::eFeatQualifiers)
{
}

void CApplyFeatTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetApplyTableArgs());
    args.push_back(GetOptionalArgs()[2]);

    x_LoadPanel(parent, "ApplyFeatQualTable", args);
    m_TablePanel = dynamic_cast<CMacroApplySrcTablePanel*>(m_Panel->LoadCustomPanel(ECustomPanelType::eApplyFeatQualTable));
    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_TargetFeature.resize(0);
}

void CApplyFeatTableTreeItemData::UpdateTarget(const TConstraints& constraints)
{
    if (m_TablePanel) {
        auto values = m_TablePanel->GetValues();
        // the target is decided based on the first to be applied field
        if (!values.empty()) {
            vector<string> tokens;
            NStr::Split(values.front().first, " ", tokens);
            if (tokens.size() == 2) {
                CTempString new_target = NMItemData::UpdateTargetForFeatures(tokens[0], tokens[1]);
                m_Target = new_target;
                m_SelectedField = tokens[0];
                m_TargetFeature = tokens[0];
                NMItemData::UpdateFeatureType(m_TargetFeature, tokens[1]);
            }
        }
    }
}

string CApplyFeatTableTreeItemData::GetVariables()
{
    string variables = CApplyTableItemData::GetVariables();
    variables += "\n" + kUpdatemRNA + " = " + m_ArgList[kUpdatemRNA].GetValue();
    return variables;
}

string CApplyFeatTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchFieldForFeatures(*m_TablePanel, constraints, EMacroFieldType::eFeatQualifiers, m_Target)) return kEmptyStr;

    bool update_mrna = m_ArgList[kUpdatemRNA].IsTrue();

    NMItemData::UpdateConstraintsForFeatures(constraints, m_TargetFeature);

    string function;
    string rt_var = "value";
    string func_end;
    {
        func_end = rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            func_end += ", " + kDelimiter;
        }
        func_end += ", " + kRmvBlank;
    }

    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);

        vector<string> field_parts;
        NStr::Split(it.first, " ", field_parts);
        if (field_parts.size() != 2)
            continue;

        auto feat_type = field_parts[0];
        auto feat_qual = field_parts[1];
        NMItemData::UpdateFeatureType(feat_type, feat_qual);

        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(feat_type, feat_qual);
        if (NMItemData::IsGeneField(feat_qual) && m_Target != macro::CMacroBioData::sm_Gene) {
            function += CMacroFunction_SetRelFeatQual::GetFuncName();
            string gene_field = feat_qual;
            NStr::ReplaceInPlace(gene_field, "-", " ");
            if (!NStr::StartsWith(gene_field, "gene ")) {
                gene_field = "gene " + gene_field;
            }
            gene_field = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(gene_field, EMacroFieldType::eGene);
            NMItemData::WrapInQuotes(gene_field);
            function += "(\"gene\", " + gene_field + ", " + func_end + ");";
        }
        else {
            string desired_target = NMItemData::UpdateTargetForFeatures(feat_type, feat_qual);
            if (desired_target == m_Target) {
                if (NMItemData::MultipleValuesAllowed(path)) {
                    // place the constraint in the Do section
                    string rt_obj = "obj";
                    string constr_func = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_obj, constraints, !update_mrna);

                    if (constr_func.empty()) {
                        function += CMacroFunction_SetStringQual::GetFuncName();
                        function += "(\"" + path + "\", " + func_end + ");";
                    }
                    else {
                        function += constr_func;
                        function += CMacroFunction_SetStringQual::GetFuncName();
                        function += "(" + rt_obj + ", " + func_end + ");";
                    }
                }
                else if (path.find(',') != NPOS) {
                    vector<string> tokens;
                    NStr::Split(path, ",", tokens);

                    _ASSERT(tokens.size() == 2);
                    string rt_obj = "obj";
                    if (macro::NMacroUtil::IsSatelliteSubfield(tokens[1]) || macro::NMacroUtil::IsMobileElementTSubfield(tokens[1])) {
                        NStr::ReplaceInPlace(feat_qual, "_", "-");
                        function += NMItemData::GetResolveFuncForQual(tokens[0], feat_qual, rt_obj, constraints);

                        function += macro::CMacroFunction_AddorSetGbQual::GetFuncName();
                        function += "(" + rt_obj + ", \"" + tokens[0] + "\", \"" + feat_qual + "\", " + func_end + ");";
                    }
                    else {
                        function += NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_obj, constraints);

                        function += CMacroFunction_AddorSetGbQual::GetFuncName();
                        function += "(" + rt_obj + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", " + func_end + ");";
                    }
                }
                else {
                    function += CMacroFunction_SetStringQual::GetFuncName();
                    function += "(\"" + path + "\", " + func_end + ");";
                }

                if (desired_target == macro::CMacroBioData::sm_Protein) {
                    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
                }
            }
            else {
                string updated_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(desired_target, feat_qual);
                NMItemData::WrapInQuotes(feat_type);

                if (NMItemData::MultipleValuesAllowed(updated_path)) {
                    // place the constraint in the Do section
                    string rt_obj = "obj";
                    string constr_func = NMItemData::GetResolveFuncForMultiValuedQual(updated_path, rt_obj, constraints, !update_mrna);

                    if (constr_func.empty()) {
                        function += CMacroFunction_SetRelFeatQual::GetFuncName() + "(" + feat_type + ", ";
                        function += "\"" + updated_path + "\", " + func_end + ", " + kUpdatemRNA + ");";
                    }
                    else {
                        function += constr_func;
                        function += CMacroFunction_SetRelFeatQual::GetFuncName() + "(" + feat_type + ", ";
                        function += rt_obj + ", " + func_end + ", " + kUpdatemRNA + ");";
                    }
                }
                else if (updated_path.find(',') != NPOS) {
                    vector<string> tokens;
                    NStr::Split(path, ",", tokens);

                    _ASSERT(tokens.size() == 2);
                    string rt_obj = "obj";
                    function += NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_obj, constraints);

                    function += CMacroFunction_AddorSetGbQual::GetFuncName();
                    function += "(" + rt_obj + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", " + func_end + ");";
                }
                else {
                    function += CMacroFunction_SetRelFeatQual::GetFuncName() + "(" + feat_type + ", ";
                    function += "\"" + updated_path + "\", " + func_end + ", " + kUpdatemRNA + ");";
                }
            }
        }
    }
    return function;
}


// CApplyCDSGeneProtTableTreeItemData
CApplyCDSGeneProtTableTreeItemData::CApplyCDSGeneProtTableTreeItemData()
    : CApplyTableItemData("Apply CDS gene protein mRNA mat-peptide qualifier table", EMacroFieldType::eCdsGeneProt)
{
}

void CApplyCDSGeneProtTableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetApplyTableArgs());
    args.push_back(GetOptionalArgs()[2]);

    x_LoadPanel(parent, "ApplyFeatQualTable", args);
    m_TablePanel = dynamic_cast<CMacroApplySrcTablePanel*>(m_Panel->LoadCustomPanel(ECustomPanelType::eApplyCDSGeneProtTable));
    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_TargetFeature.resize(0);
}

void CApplyCDSGeneProtTableTreeItemData::UpdateTarget(const TConstraints& constraints)
{
    if (m_TablePanel) {
        auto values = m_TablePanel->GetValues();
        // the target is decided based on the first to be applied field
        if (!values.empty()) {
            CTempString new_target = NMItemData::UpdateTargetForCdsGeneProt(values.front().first);
            m_Target = new_target;
            string feat_type, feat_qual;
            NMItemData::GetFeatureAndField(values.front().first, feat_type, feat_qual);
            //m_SelectedField = tokens[0];
            m_TargetFeature = feat_type;
        }
    }
}

string CApplyCDSGeneProtTableTreeItemData::GetVariables()
{
    string variables = CApplyTableItemData::GetVariables();
    variables += "\n" + kUpdatemRNA + " = " + m_ArgList[kUpdatemRNA].GetValue();
    return variables;
}

string CApplyCDSGeneProtTableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchFieldForFeatures(*m_TablePanel, constraints, EMacroFieldType::eCdsGeneProt, m_Target)) return kEmptyStr;

    bool update_mrna = m_ArgList[kUpdatemRNA].IsTrue();

    NMItemData::UpdateConstraintsForDualCdsGeneProt(constraints, m_TargetFeature);

    string function;
    string rt_var = "value";
    string func_end;
    {
        func_end = rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            func_end += ", " + kDelimiter;
        }
        func_end += ", " + kRmvBlank;
    }

    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);

        string field_name = it.first;
        string feat_type, feat_qual;
        NMItemData::GetFeatureAndField(field_name, feat_type, feat_qual);
        
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field_name, EMacroFieldType::eCdsGeneProt);
        
        string desired_target = NMItemData::UpdateTargetForCdsGeneProt(field_name);
        if (desired_target == m_Target) {
            if (NMItemData::MultipleValuesAllowed(path)) {
                // place the constraint in the Do section
                string rt_obj = "obj";
                string constr_func = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_obj, constraints, !update_mrna);

                if (constr_func.empty()) {
                    function += CMacroFunction_SetStringQual::GetFuncName();
                    function += "(\"" + path + "\", " + func_end + ");";
                }
                else {
                    function += constr_func;
                    function += CMacroFunction_SetStringQual::GetFuncName();
                    function += "(" + rt_obj + ", " + func_end + ");";
                }
            }
            else if (path.find(',') != NPOS) {
                vector<string> tokens;
                NStr::Split(path, ",", tokens);

                _ASSERT(tokens.size() == 2);
                string rt_obj = "obj";
                function += NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_obj, constraints);

                function += CMacroFunction_AddorSetGbQual::GetFuncName();
                function += "(" + rt_obj + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", " + func_end + ");";
            }
            else {
                function += CMacroFunction_SetStringQual::GetFuncName();
                function += "(\"" + path + "\", " + func_end + ");";
            }

            if (desired_target == macro::CMacroBioData::sm_Protein) {
                NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
            }
        }
        else {
            string updated_path = path;
            NMItemData::WrapInQuotes(feat_type);

            if (NMItemData::MultipleValuesAllowed(updated_path)) {
                // place the constraint in the Do section
                string rt_obj = "obj";
                string constr_func = NMItemData::GetResolveFuncForMultiValuedQual(updated_path, rt_obj, constraints, !update_mrna);

                if (constr_func.empty()) {
                    function += CMacroFunction_SetRelFeatQual::GetFuncName() + "(" + feat_type + ", ";
                    function += "\"" + updated_path + "\", " + func_end + ", " + kUpdatemRNA + ");";
                }
                else {
                    function += constr_func;
                    function += CMacroFunction_SetRelFeatQual::GetFuncName() + "(" + feat_type + ", ";
                    function += rt_obj + ", " + func_end + ", " + kUpdatemRNA + ");";
                }
            }
            else if (updated_path.find(',') != NPOS) {
                vector<string> tokens;
                NStr::Split(path, ",", tokens);

                //_ASSERT(tokens.size() == 2);
                string rt_obj = "obj";
                function += NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_obj, constraints);

                function += CMacroFunction_AddorSetGbQual::GetFuncName();
                function += "(" + rt_obj + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", " + func_end + ");";
            }
            else {
                function += CMacroFunction_SetRelFeatQual::GetFuncName() + "(" + feat_type + ", ";
                function += "\"" + updated_path + "\", " + func_end + ", " + kUpdatemRNA + ");";
            }
        }
    }
    return function;
}


// CApplyRNATableTreeItemData
CApplyRNATableTreeItemData::CApplyRNATableTreeItemData()
    : CApplyTableItemData("Apply RNA qualifier table", EMacroFieldType::eRNA)
{
}

void CApplyRNATableTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadParamPanel(parent, ECustomPanelType::eApplyRNAQualTable);
    m_TargetncRNAclass = m_TargetRNAType = kEmptyStr;
}

void CApplyRNATableTreeItemData::UpdateTarget(const TConstraints& constraints)
{
    if (m_TablePanel) {
        auto values = m_TablePanel->GetValues();
        // the target is decided based on the first to be applied field
        if (!values.empty()) {
            string rna_type, ncRNA_class, rna_qual;
            NMItemData::GetRNASpecificFields(values.front().first, rna_type, ncRNA_class, rna_qual);
            CTempString new_target = NMItemData::UpdateTargetForRnas(rna_type);
            m_Target = new_target;
            m_SelectedField = rna_type;
            m_TargetRNAType = rna_type;
            m_TargetncRNAclass = ncRNA_class;
        }
    }
}

string CApplyRNATableTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_TablePanel) return kEmptyStr;

    auto values = m_TablePanel->GetValues();
    if (m_TablePanel->GetFilename().empty() || values.empty()) return kEmptyStr;
    if (!AddMatchFieldForRnas(*m_TablePanel, constraints, m_Target, m_TargetRNAType, m_TargetncRNAclass)) return kEmptyStr;

    NMItemData::UpdateConstraintsForRnas(constraints, m_TargetRNAType, m_TargetncRNAclass);

    string function;
    string rt_var = "value";
    string func_end;
    {
        func_end = rt_var + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            func_end += ", " + kDelimiter;
        }
        func_end += ", " + kRmvBlank;
    }

    for (auto& it : values) {
        if (!function.empty())
            function += "\n";

        function += NMItemData::GetFuncFromTable(rt_var, ToStdString(m_TablePanel->GetFilename()), it.second, constraints, m_Warning);
        
        string field_name = it.first;
        string rna_type, ncRNA_class, rna_qual;
        NMItemData::GetRNASpecificFields(field_name, rna_type, ncRNA_class, rna_qual);

        string desired_target = NMItemData::UpdateTargetForRnas(rna_type);
        if (rna_type == m_TargetRNAType) {
            if (NStr::StartsWith(rna_qual, "gene")) {
                string gene_field = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(rna_qual, EMacroFieldType::eGene);
                NMItemData::WrapInQuotes(gene_field);
                function += CMacroFunction_SetRelFeatQual::GetFuncName();
                function += "(\"gene\", " + gene_field + ", " + func_end + ", false);";
            }
            else {
                string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(rna_type + " " + rna_qual, EMacroFieldType::eRNA);
                if (path.find(',') != NPOS) {
                    vector<string> tokens;
                    NStr::Split(path, ",", tokens);

                    _ASSERT(tokens.size() == 2);
                    string rt_obj = "obj";
                    function += NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_obj, constraints);

                    function += CMacroFunction_AddorSetGbQual::GetFuncName();
                    function += "(" + rt_obj + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", " + func_end + ");";
                }
                else if (path.find("::") != NPOS) {
                    function += CMacroFunction_SetRnaProduct::GetFuncName();
                    function += "(" + func_end + ");";
                }
                else {
                    function += CMacroFunction_SetStringQual::GetFuncName();
                    function += "(\"" + path + "\", " + func_end + ");";
                }
            }
        }
        else {
            NCBI_THROW(CException, eUnknown, "Applied values should refer to the same RNA type");
        }
    }
    return function;
}


END_NCBI_SCOPE

