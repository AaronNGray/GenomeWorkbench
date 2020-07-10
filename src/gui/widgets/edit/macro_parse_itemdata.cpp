/*  $Id: macro_parse_itemdata.cpp 44072 2019-10-21 17:57:10Z asztalos $
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
#include <gui/objutils/macro_fn_feature.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_parse_itemdata.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(macro);
USING_SCOPE(NMacroArgs);

// CParseQualTreeItemData
CParseQualTreeItemData::CParseQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{   
    m_IncludeLeft = make_pair("include_left", false);
    m_IncludeRight = make_pair("include_right", false);
    m_CaseSensitive = make_pair("case_sensitive_parse", true);
}

void CParseQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    x_LoadPanel(parent, CMacroFunction_ParseStringQual::GetFuncName(), args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
}

string CParseQualTreeItemData::GetMacroDescription() const
{
    string descr = x_GetParsedTextDescription();
    descr += " from " + m_ArgList[kFromField].GetValue() + " to " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CParseQualTreeItemData::x_GetParsedTextDescription() const
{
    string descr;
    descr = "Parse";

    if (x_ParseEntireText()) {
        descr += " entire text";
    }
    else {
        descr += " text";
        if (m_ArgList[kJustAfter].IsTrue()) {
            descr += " just after";
        }
        else if (m_ArgList[kStartAt].IsTrue()) {
            descr += " starting with";
        }

        if (m_ArgList[kTextLeft].IsTrue() && !m_ArgList[kDelLeft].Empty()) {
            descr += " '" + m_ArgList[kDelLeft].GetValue() + "'";
        }
        else if (m_ArgList[kDigitsLeft].IsTrue()) {
            descr += " 'numbers'";
        }
        else if (m_ArgList[kLetterLeft].IsTrue()) {
            descr += " 'letters'";
        }

        if (!NStr::EndsWith(descr, " text")) {
            descr += ",";
        }

        if (m_ArgList[kUpTo].IsTrue()) {
            descr += " up to";
        }
        else if (m_ArgList[kInclude].IsTrue()) {
            descr += " up to and including";
        }

        if (m_ArgList[kTextRight].IsTrue() && !m_ArgList[kDelRight].Empty()) {
            descr += " '" + m_ArgList[kDelRight].GetValue() + "'";
        }
        else if (m_ArgList[kDigitsRight].IsTrue()) {
            descr += " 'numbers'";
        }
        else if (m_ArgList[kLetterRight].IsTrue()) {
            descr += " 'letters'";
        }
    }
    return descr;
}

string CParseQualTreeItemData::GetVariables()
{
    string variables;
    variables += kCapChange + " = \"none\"\n";
    variables += kExistingText + " = \"" + NMItemData::GetHandleOldValue(m_ArgList[kExistingText].GetValue()) + "\"";
    if (m_ArgList[kDelimiter].GetEnabled()) {
        variables += "\n" + kDelimiter + " = \"" + NMItemData::GetDelimiterChar(m_ArgList[kDelimiter].GetValue()) + "\"";
    }

    if (!x_ParseEntireText()) {
        if (m_ArgList[kJustAfter].IsTrue()) {
            m_IncludeLeft.second = false;
        }
        else if (m_ArgList[kStartAt].IsTrue()) {
            m_IncludeLeft.second = true;
        }

        variables += "\n" + m_IncludeLeft.first + " = " + NStr::BoolToString(m_IncludeLeft.second) + "\n";

        if (m_ArgList[kUpTo].IsTrue()) {
            m_IncludeRight.second = false;
        }
        else if (m_ArgList[kInclude].IsTrue()) {
            m_IncludeRight.second = true;

        }

        variables += m_IncludeRight.first + " = " + NStr::BoolToString(m_IncludeRight.second) + "\n";
        variables += kRmvParse + " = " + m_ArgList[kRmvParse].GetValue() + "\n";
        variables += kRmvLeft + " = " + m_ArgList[kRmvLeft].GetValue() + "\n";
        variables += kRmvRight + " = " + m_ArgList[kRmvRight].GetValue() + "\n";

        m_CaseSensitive.second = !(m_ArgList[kCaseInsensParse].IsTrue());
        variables += m_CaseSensitive.first + " = " + NStr::BoolToString(m_CaseSensitive.second) + "\n";

        variables += kWholeWordParse + " = " + m_ArgList[kWholeWordParse].GetValue();
    }
    return variables;
}

pair<string, string> CParseQualTreeItemData::x_GetResolveFuncForMultValQuals(const string& field, const string& rt_var,
                TConstraints& constraints, bool remove_constraint) const
{
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType);
    string resolve_fnc, var_name = path;
    NMItemData::WrapInQuotes(var_name);

    if (NMItemData::MultipleValuesAllowed(path)) {
        resolve_fnc = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

        if (!resolve_fnc.empty()) {
            var_name = rt_var;
        }
    } else if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        _ASSERT(tokens.size() == 2);
        var_name = tokens[1];
        NMItemData::WrapInQuotes(var_name);
    }

    return make_pair(resolve_fnc, var_name);
}

string CParseQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool remove_constraint = true;
    pair<string, string> source = x_GetResolveFuncForMultValQuals(m_ArgList[kFromField].GetValue(), "src", constraints, remove_constraint);
    pair<string, string> dest = x_GetResolveFuncForMultValQuals(m_ArgList[kToField].GetValue(), "dest", constraints, remove_constraint);

    NMItemData::AddConstraintIgnoreNewText(dest.second, constraints, m_Panel->GetArgumentList());

    string function;
    function += source.first;
    function += dest.first;

    if (x_ParseEntireText()) {
        // parse the entire text

        function += CMacroFunction_ParseStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kCapChange+ ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        if (m_ArgList[kRmvParse].IsTrue()) {
            function += "\n";
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + source.second + ");";
        }
    }
    else {
        string parsed_text = "text";
        function += x_GetParsedTextFunction(source.second, parsed_text);
        function += CMacroFunction_AddParsedText::GetFuncName();
        function += "(" + parsed_text + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
    }

    return function;
}

string CParseQualTreeItemData::x_GetParsedTextFunction(const string& src_field, const string& parsed_text) const
{
    string parsed_fnc = parsed_text + " = " + string(CMacroFunction_ParsedText::GetFuncName());
    parsed_fnc += "(" + src_field + ", ";
    string left_del;
    if (m_ArgList[kTextLeft].IsTrue()) {
        left_del = m_ArgList[kDelLeft].GetValue();
    }
    else if (m_ArgList[kDigitsLeft].IsTrue()) {
        left_del = "eDigits";
    }
    else if (m_ArgList[kLetterLeft].IsTrue()) {
        left_del = "eLetters";
    }

    NMItemData::WrapInQuotes(left_del);
    parsed_fnc += left_del + ", " + m_IncludeLeft.first + ", ";


    string right_del;
    if (m_ArgList[kTextRight].IsTrue()) {
        right_del = m_ArgList[kDelRight].GetValue();
    }
    else if (m_ArgList[kDigitsRight].IsTrue()) {
        right_del = "eDigits";
    }
    else if (m_ArgList[kLetterRight].IsTrue()) {
        right_del = "eLetters";
    }

    NMItemData::WrapInQuotes(right_del);
    parsed_fnc += right_del + ", " + m_IncludeRight.first + ", ";

    parsed_fnc += m_CaseSensitive.first + ", " + kWholeWordParse;
    parsed_fnc += ", " + kRmvParse;
    parsed_fnc += ", " + kRmvLeft;
    parsed_fnc += ", " + kRmvRight;
    parsed_fnc += ");\n";
    return parsed_fnc;
}

bool CParseQualTreeItemData::x_ParseEntireText() const
{
    return (m_ArgList[kTextLeft].IsTrue() && m_ArgList[kDelLeft].Empty() &&
        m_ArgList[kTextRight].IsTrue() && m_ArgList[kDelRight].Empty());
}


// CParseBsrcTreeItemData
CParseBsrcTreeItemData::CParseBsrcTreeItemData()
    : CParseQualTreeItemData("Parse text between biosource qualifiers", EMacroFieldType::eBiosourceText)
{
}

const vector<string>& CParseBsrcTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetBsrcKeywords();
}

void CParseBsrcTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDualBsrcArgs());
    copy(GetParseArgs().begin(), GetParseArgs().end(), back_inserter(args));
    x_LoadPanel(parent, "ParseStringSourceQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
}

string CParseBsrcTreeItemData::GetFunction(TConstraints& constraints) const
{
    //get the source field
    string rt_var = "src";
    m_FieldType = NMItemData::GetSourceTypeInDoublePanel(m_Panel->GetArgumentList(), NMItemData::eFrom);
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), m_FieldType);
    string src_resolve_fnc, src_var_name = path;
    NMItemData::WrapInQuotes(src_var_name);

    if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        _ASSERT(tokens.size() == 2);
        src_resolve_fnc = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

        if (!src_resolve_fnc.empty()) {
            src_var_name = NMItemData::GetEditableFieldForPair(m_ArgList[kFromField].GetValue(), rt_var);
            NMItemData::WrapInQuotes(src_var_name);
        }
    }

    // get the destination field
    m_FieldType = NMItemData::GetSourceTypeInDoublePanel(m_Panel->GetArgumentList(), NMItemData::eTo);
    string dest = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kToField].GetValue(), m_FieldType);

    NMItemData::AddConstraintIgnoreNewText(dest, constraints, m_Panel->GetArgumentList());
    
    if (dest.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(dest, ",", tokens);
        dest = m_ArgList[kToField].GetValue();
    }
    NMItemData::WrapInQuotes(dest);

    string function;
    function += src_resolve_fnc;

    if (x_ParseEntireText()) {
        // parse the entire text

        function += CMacroFunction_ParseStringQual::GetFuncName();
        function += "(" + src_var_name + ", " + dest + ", " + kCapChange + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        if (m_ArgList[kRmvParse].IsTrue()) {
            function += "\n";
            if (path.find(',') != NPOS) {
                function += CMacroFunction_RemoveModifier::GetFuncName();
                function += "(" + rt_var + ");";
            }
            else {
                function += CMacroFunction_RemoveQual::GetFuncName();
                function += "(" + src_var_name + ");";
            }
        }
    }
    else {
        string parsed_text = "text";
        function += x_GetParsedTextFunction(src_var_name, parsed_text);
        function += CMacroFunction_AddParsedText::GetFuncName();
        function += "(" + parsed_text + ", " + dest + ", " + kCapChange + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
    }

    return function;
}


// CParseToBsrcTreeItemData
CParseToBsrcTreeItemData::CParseToBsrcTreeItemData()
    : CParseQualTreeItemData("Parse text to biosource qualifiers", EMacroFieldType::eBiosourceText)
{
}

const vector<string>& CParseToBsrcTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetParseToBsrcKeywords();
}

void CParseToBsrcTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    args.push_back(GetDualBsrcArgs()[2]);
    args.push_back(GetDualBsrcArgs()[3]);
    x_LoadPanel(parent, "ParseToStringSourceQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eOther);
    m_Panel->SetControlValues(kFromField, fieldnames);
    fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
}

string CParseToBsrcTreeItemData::GetVariables()
{
    string src_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), EMacroFieldType::eOther);
    if (src_path == "localid" && x_ParseEntireText()) {
        string variables;
        variables += kCapChange + " = \"none\"\n";
        variables += kExistingText + " = \"" + NMItemData::GetHandleOldValue(m_ArgList[kExistingText].GetValue()) + "\"";
        if (m_ArgList[kDelimiter].GetEnabled()) {
            variables += "\n" + kDelimiter + " = \"" + NMItemData::GetDelimiterChar(m_ArgList[kDelimiter].GetValue()) + "\"";
        }
        return variables;
    }
    return CParseQualTreeItemData::GetVariables();
}

string CParseToBsrcTreeItemData::GetFunction(TConstraints& constraints) const
{
    //get the source field
    string src_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), EMacroFieldType::eOther);

    // get the destination field
    m_FieldType = NMItemData::GetSourceTypeInDoublePanel(m_Panel->GetArgumentList(), NMItemData::eTo);
    string dest = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kToField].GetValue(), m_FieldType);

    NMItemData::AddConstraintIgnoreNewText(dest, constraints, m_Panel->GetArgumentList());

    if (dest.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(dest, ",", tokens);
        dest = m_ArgList[kToField].GetValue();
    }
    NMItemData::WrapInQuotes(dest);

    string function;
    string rt_var;
    if (x_ParseEntireText() && src_path == "localid") {
        rt_var = "id";
        function = rt_var + " = " + CMacroFunction_LocalID::GetFuncName() + "();\n";
        function += CMacroFunction_ParseStringQual::GetFuncName();
    }
    else {
        rt_var = "text";
        NMItemData::WrapInQuotes(src_path);
        function = x_GetParsedTextFunction(src_path, rt_var);
        function += CMacroFunction_AddParsedText::GetFuncName();
    }

    function += "(" + rt_var + ", " + dest + ", " + kCapChange + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    return function;
}


// CParseToCdsGeneProtTreeItemData
CParseToCdsGeneProtTreeItemData::CParseToCdsGeneProtTreeItemData()
    : CParseQualTreeItemData("Parse text to cds-gene-prot-mRNA qualifiers", EMacroFieldType::eCdsGeneProt)
{
}

const vector<string>& CParseToCdsGeneProtTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetParseToCdsGeneProtKeywords();
}

void CParseToCdsGeneProtTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    x_LoadPanel(parent, CMacroFunction_ParseStringQual::GetFuncName(), args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eOther);
    m_Panel->SetControlValues(kFromField, fieldnames);
    fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
}

bool CParseToCdsGeneProtTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForCdsGeneProt(m_ArgList[kToField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    return modified;
}

string CParseToCdsGeneProtTreeItemData::GetVariables()
{
    string src_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), EMacroFieldType::eOther);
    if (src_path == "localid" && x_ParseEntireText()) {
        string variables;
        variables += kCapChange + " = \"none\"\n";
        variables += kExistingText + " = \"" + NMItemData::GetHandleOldValue(m_ArgList[kExistingText].GetValue()) + "\"";
        if (m_ArgList[kDelimiter].GetEnabled()) {
            variables += "\n" + kDelimiter + " = \"" + NMItemData::GetDelimiterChar(m_ArgList[kDelimiter].GetValue()) + "\"";
        }
        return variables;
    }
    return CParseQualTreeItemData::GetVariables();
}

string CParseToCdsGeneProtTreeItemData::GetFunction(TConstraints& constraints) const
{
    //get the source field
    string src_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), EMacroFieldType::eOther);

    string dest_feat = m_ArgList[kToField].GetValue(), dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);
    NMItemData::UpdateConstraintsForDualCdsGeneProt(constraints, dest_feat);

    // get the destination field
    bool remove_constraint = true;
    pair<string, string> dest = x_GetResolveFuncForMultValQuals(m_ArgList[kToField].GetValue(), "dest", constraints, remove_constraint);
    NMItemData::AddConstraintIgnoreNewText(dest.second, constraints, m_Panel->GetArgumentList());

    string function;
    string rt_var;
    if (x_ParseEntireText() && src_path == "localid") {
        rt_var = "id";
        function = rt_var + " = " + CMacroFunction_LocalID::GetFuncName() + "();\n";
        function += dest.first;
        function += CMacroFunction_ParseStringQual::GetFuncName();
    }
    else {
        rt_var = "text";
        NMItemData::WrapInQuotes(src_path);
        function = x_GetParsedTextFunction(src_path, rt_var);
        function += dest.first;
        function += CMacroFunction_AddParsedText::GetFuncName();
    }

    function += "(" + rt_var + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    return function;
}


// CParseProteinQualTreeItemData
CParseProteinQualTreeItemData::CParseProteinQualTreeItemData()
    : CParseQualTreeItemData("Parse protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CParseProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "ParseStringProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

string CParseProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CParseQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CParseProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    pair<string, string> source = x_GetResolveFuncForMultValQuals(m_ArgList[kFromField].GetValue(), "src", constraints, !update_mrna);
    pair<string, string> dest = x_GetResolveFuncForMultValQuals(m_ArgList[kToField].GetValue(), "dest", constraints, !update_mrna);

    NMItemData::AddConstraintIgnoreNewText(dest.second, constraints, m_Panel->GetArgumentList());

    string function;
    function += source.first;
    function += dest.first;

    if (x_ParseEntireText()) {
        // parse the entire text

        function += CMacroFunction_ParseStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        if (m_ArgList[kRmvParse].IsTrue()) {
            function += "\n";
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + source.second + ");";
        }
    }
    else {
        string parsed_text = "text";
        function += x_GetParsedTextFunction(source.second, parsed_text);
        function += CMacroFunction_AddParsedText::GetFuncName();
        function += "(" + parsed_text + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
    }

    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}

// CParseRNAQualTreeItemData
CParseRNAQualTreeItemData::CParseRNAQualTreeItemData()
    : CParseQualTreeItemData("Parse RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CParseRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "ParseStringRNAQual", args);

    vector<string> rnas = CMacroEditorContext::GetInstance().GetRNATypes();
    m_Panel->SetControlValues(kRNAType, rnas);
    m_Panel->SetSelection(kRNAType, 0);
    vector<string> class_types = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
    m_Panel->SetControlValues(kncRNAClass, class_types);
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
}

bool CParseRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForDualRnas(m_ArgList[kRNAType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CParseRNAQualTreeItemData::GetMacroDescription() const
{
    string descr = x_GetParsedTextDescription() + " from";

    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kFromField].GetValue();
    descr += " to " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CParseRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kFromField].Empty() || m_ArgList[kToField].Empty())
        return kEmptyStr;

    if (m_Target != CMacroBioData::sm_Gene) {
        NMItemData::UpdateConstraintsForRnas(constraints, m_ArgList);
    }

    string src_feat = m_ArgList[kRNAType].GetValue(), src_field;
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), src_feat, src_field);
    string dest_feat = m_ArgList[kRNAType].GetValue(), dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);

    string src_fieldname = m_ArgList[kFromField].GetValue();
    if (src_feat != "gene") {
        src_fieldname = m_ArgList[kRNAType].GetValue() + " " + src_fieldname;
    }

    string dest_fieldname = m_ArgList[kToField].GetValue();
    if (dest_feat != "gene") {
        dest_fieldname = m_ArgList[kRNAType].GetValue() + " " + dest_fieldname;
    }

    string function;
    bool remove_constraint = false;
    string src_var = "src";
    pair<string, string> source = NMItemData::GetResolveFuncForRNAQuals(src_fieldname, src_var, constraints, remove_constraint);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForRNAQuals(dest_fieldname, "dest", constraints, remove_constraint);
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(dest_fieldname, m_FieldType);
        NMItemData::AddConstraintIgnoreNewText(dest_path, constraints, m_Panel->GetArgumentList());
        
        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;
        
        if (x_ParseEntireText()) {
            function += CMacroFunction_ParseStringQual::GetFuncName();
            function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";

            if (m_ArgList[kRmvParse].IsTrue()) {
                function += "\n";
                if (source.second.find("::") != NPOS) {
                    function += CMacroFunction_RemoveRnaProduct::GetFuncName() + "();";
                }
                else {
                    function += CMacroFunction_RemoveQual::GetFuncName() + "(";
                    if (!source.first.empty()) {
                        function += src_var;
                    }
                    else {
                        function += source.second;
                    }
                    function += ");";
                }
            }
        }
        else {
            string parsed_text = "text";
            function += x_GetParsedTextFunction(source.second, parsed_text);
            function += CMacroFunction_AddParsedText::GetFuncName();
            function += "(" + parsed_text + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";
        }
    }
    else {
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(dest_fieldname, m_FieldType);

        NMItemData::AddConstraintIgnoreNewText(dest_path, constraints, m_Panel->GetArgumentList(), dest_feat);

        if (!source.first.empty())
            function += source.first;

        if (dest_path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(dest_path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            dest_path = tokens[1];
        }
        if (x_ParseEntireText()) {
            function += CMacroFunction_ParseFeatQual::GetFuncName();
            function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
            function += kCapChange + ", false, " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";

            if (m_ArgList[kRmvParse].IsTrue()) {
                function += "\n";
                if (source.second.find("::") != NPOS) {
                    function += CMacroFunction_RemoveRnaProduct::GetFuncName() + "();";
                }
                else {
                    function += CMacroFunction_RemoveQual::GetFuncName() + "(";
                    if (!source.first.empty()) {
                        function += src_var;
                    }
                    else {
                        function += source.second;
                    }
                    function += ");";
                }
            }
        }
        else {
            string parsed_text = "text";
            function += x_GetParsedTextFunction(source.second, parsed_text);
            function += CMacroFunction_AddParsedToFeatQual::GetFuncName();
            function += "(" + parsed_text + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
            function += kCapChange + ", false, " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";
        }
    }

    return function;
}


// CParseFeatQualTreeItemData
CParseFeatQualTreeItemData::CParseFeatQualTreeItemData()
    : CParseQualTreeItemData("Parse feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CParseFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    args.push_back(GetOptionalArgs()[0]);
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "ParseStringFeatQual", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFromField, qualifiers);
    m_Panel->SetControlValues(kToField, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProductDouble);
}

bool CParseFeatQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForDualFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CParseFeatQualTreeItemData::GetMacroDescription() const
{
    string descr = x_GetParsedTextDescription() + " from ";
    descr += m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kFromField].GetValue();
    descr += " to " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CParseFeatQualTreeItemData::GetVariables()
{
    string vars = CParseQualTreeItemData::GetVariables();
    string src_feat = m_ArgList[kFeatType].GetValue(), src_field = m_ArgList[kFromField].GetValue();
    string dest_feat = m_ArgList[kFeatType].GetValue(), dest_field = m_ArgList[kToField].GetValue();

    NMItemData::UpdateFeatureType(src_feat, src_field);
    NMItemData::UpdateFeatureType(dest_feat, dest_field);
    if (src_feat != dest_feat) {
        vars += NMItemData::GetUpdatemRNAVar(m_ArgList);
    }
    return vars;
}

string CParseFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string src_feat = m_ArgList[kFeatType].GetValue(), src_field = m_ArgList[kFromField].GetValue();
    string dest_feat = m_ArgList[kFeatType].GetValue(), dest_field = m_ArgList[kToField].GetValue();

    if (src_feat.empty() || src_field.empty() || dest_field.empty()) return kEmptyStr;

    NMItemData::UpdateFeatureType(src_feat, src_field);
    NMItemData::UpdateFeatureType(dest_feat, dest_field);
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();

    NMItemData::UpdateConstraintsForFeatures(constraints, src_feat);

    string function;
    string src_var = "src";
    pair<string, string> source = NMItemData::GetResolveFuncForFeatQuals(src_feat, src_field, src_var, constraints, !update_mrna);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForFeatQuals(dest_feat, dest_field, "dest", constraints, !update_mrna);
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(dest_feat, dest_field);
        NMItemData::AddConstraintIgnoreNewText(dest_path, constraints, m_Panel->GetArgumentList());

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        if (x_ParseEntireText()) {
            function += CMacroFunction_ParseStringQual::GetFuncName();
            function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";

            if (m_ArgList[kRmvParse].IsTrue()) {
                function += "\n";
                function += CMacroFunction_RemoveQual::GetFuncName() + "(";
                if (!source.first.empty()) {
                    function += src_var;
                }
                else {
                    function += source.second;
                }
                function += ");";
            }
        }
        else {
            string parsed_text = "text";
            function += x_GetParsedTextFunction(source.second, parsed_text);
            function += CMacroFunction_AddParsedText::GetFuncName();
            function += "(" + parsed_text + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";
        }
        NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    }
    else {
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(dest_feat, dest_field);

        NMItemData::AddConstraintIgnoreNewText(dest_path, constraints, m_Panel->GetArgumentList(), dest_feat);

        if (!source.first.empty())
            function += source.first;

        if (dest_path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(dest_path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            dest_path = tokens[1];
        }
        if (x_ParseEntireText()) {
            function += CMacroFunction_ParseFeatQual::GetFuncName();
            function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
            function += kCapChange + ", " + kUpdatemRNA + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";

            if (m_ArgList[kRmvParse].IsTrue()) {
                function += "\n";
                function += CMacroFunction_RemoveQual::GetFuncName() + "(";
                if (!source.first.empty()) {
                    function += src_var;
                }
                else {
                    function += source.second;
                }
                function += ");";
            }
        }
        else {
            string parsed_text = "text";
            function += x_GetParsedTextFunction(source.second, parsed_text);
            function += CMacroFunction_AddParsedToFeatQual::GetFuncName();
            function += "(" + parsed_text + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
            function += kCapChange + ", " + kUpdatemRNA + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";
        }
    }
    return function;
}



// CParseCdsGeneProtTreeItemdata
CParseCdsGeneProtTreeItemdata::CParseCdsGeneProtTreeItemdata()
    : CParseQualTreeItemData("Parse between CDS gene protein mRNA mat-peptide qualifiers", EMacroFieldType::eCdsGeneProt)
{
}

void CParseCdsGeneProtTreeItemdata::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "ParseStringProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

bool CParseCdsGeneProtTreeItemdata::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForCdsGeneProt(m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    return modified;
}

string CParseCdsGeneProtTreeItemdata::GetMacroDescription() const
{
    string descr = CParseQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CParseCdsGeneProtTreeItemdata::GetVariables()
{
    string vars = CParseQualTreeItemData::GetVariables();
    string src_feat, src_field;
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), src_feat, src_field);
    string dest_feat, dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);
    if (src_feat != dest_feat) {
        vars += NMItemData::GetUpdatemRNAVar(m_ArgList);
    }
    return vars;
}

string CParseCdsGeneProtTreeItemdata::GetFunction(TConstraints& constraints) const
{
    string src_feat, src_field;
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), src_feat, src_field);
    string dest_feat, dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);

    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();

    NMItemData::UpdateConstraintsForDualCdsGeneProt(constraints, src_feat);

    string function;
    string src_var = "src";
    pair<string, string> source = NMItemData::GetResolveFuncForCdsGeneProtQuals(m_ArgList[kFromField].GetValue(), src_var, constraints, !update_mrna);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForCdsGeneProtQuals(m_ArgList[kToField].GetValue(), "dest", constraints, !update_mrna);

        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kToField].GetValue(), m_FieldType);
        NMItemData::AddConstraintIgnoreNewText(dest_path, constraints, m_Panel->GetArgumentList());

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        if (x_ParseEntireText()) {
            function += CMacroFunction_ParseStringQual::GetFuncName();
            function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";

            if (m_ArgList[kRmvParse].IsTrue()) {
                function += "\n";
                function += CMacroFunction_RemoveQual::GetFuncName() + "(";
                if (!source.first.empty()) {
                    function += src_var;
                }
                else {
                    function += source.second;
                }
                function += ");";
            }
        }
        else {
            string parsed_text = "text";
            function += x_GetParsedTextFunction(source.second, parsed_text);
            function += CMacroFunction_AddParsedText::GetFuncName();
            function += "(" + parsed_text + ", " + dest.second + ", " + kCapChange + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";
        }
        NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    }
    else {
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kToField].GetValue(), m_FieldType);

        NMItemData::AddConstraintIgnoreNewText(dest_path, constraints, m_Panel->GetArgumentList(), dest_feat);

        if (!source.first.empty())
            function += source.first;

        if (dest_path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(dest_path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            dest_path = tokens[1];
        }

        if (x_ParseEntireText()) {
            function += CMacroFunction_ParseFeatQual::GetFuncName();
            function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
            function += kCapChange + ", " + kUpdatemRNA + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";

            if (m_ArgList[kRmvParse].IsTrue()) {
                function += "\n";
                function += CMacroFunction_RemoveQual::GetFuncName() + "(";
                if (!source.first.empty()) {
                    function += src_var;
                }
                else {
                    function += source.second;
                }
                function += ");";
            }
        }
        else {
            string parsed_text = "text";
            function += x_GetParsedTextFunction(source.second, parsed_text);
            function += CMacroFunction_AddParsedToFeatQual::GetFuncName();
            function += "(" + parsed_text + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
            function += kCapChange + ", " + kUpdatemRNA + ", " + kExistingText;
            if (m_ArgList[kDelimiter].GetEnabled()) {
                function += ", " + kDelimiter;
            }
            function += ");";
        }
    }
    return function;
}


// CParseBsrcToStructCommTreeItemData
CParseBsrcToStructCommTreeItemData::CParseBsrcToStructCommTreeItemData()
    : CParseQualTreeItemData("Parse from biosource qualifiers to structured comment fields", EMacroFieldType::eBiosourceText)
{
}

void CParseBsrcToStructCommTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetParseArgs());
    args.push_back(GetDualBsrcArgs()[0]);
    args.push_back(GetDualBsrcArgs()[1]);
    args.push_back(GetOptionalArgs()[5]);
    x_LoadPanel(parent, "ParseBsrcToStructCommQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
    m_Panel->SetControlValues(kFromField, fieldnames);
    fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eStructComment);
    m_Panel->SetControlValues(kToField, fieldnames);
    m_Panel->SetSelection(kToField, 0);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRemoveFromParsedField);
    m_Panel->GetArgumentList().Attach(NMItemData::OnLeftDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRightDigitsLettersInParsePanels);
    m_Panel->GetArgumentList().Attach(NMItemData::OnStructCommFieldDblChanged);
}

string CParseBsrcToStructCommTreeItemData::GetMacroDescription() const
{
    string descr = x_GetParsedTextDescription();
    descr += " from " + m_ArgList[kFromField].GetValue() + " to structured comment ";
    const string& to_field = m_ArgList[kToField].GetValue();
    if (NStr::EqualNocase(to_field, "Field")) {
        if (m_ArgList[kStrCommField].GetShown()) {
            descr += "field '" + m_ArgList[kStrCommField].GetValue() + "'";
        }
    }
    else if (NStr::EqualNocase(to_field, "Database name")) {
        descr += "database name";
    }
    else if (NStr::EqualNocase(to_field, "Field name")) {
        descr += "field names";
    }
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

const vector<string>& CParseBsrcToStructCommTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetParseToBsrcKeywords();
}

string CParseBsrcToStructCommTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kFromField].Empty() || m_ArgList[kToField].Empty() || (m_ArgList[kStrCommField].GetShown() && m_ArgList[kStrCommField].Empty()))
        return kEmptyStr;

    //get the source field
    string rt_var = "src";
    m_FieldType = NMItemData::GetSourceTypeInDoublePanel(m_Panel->GetArgumentList(), NMItemData::eFrom);
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), m_FieldType);
    string src_resolve_fnc, src_var_name = path;
    NMItemData::WrapInQuotes(src_var_name);

    if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        _ASSERT(tokens.size() == 2);
        src_resolve_fnc = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

        if (!src_resolve_fnc.empty()) {
            src_var_name = NMItemData::GetEditableFieldForPair(m_ArgList[kFromField].GetValue(), rt_var);
            NMItemData::WrapInQuotes(src_var_name);
        }
    }

    // get the destination field
    const string& to_field = m_ArgList[kToField].GetValue();
    string dest;
    string dest_field_name = (m_ArgList[kStrCommField].GetShown()) ? m_ArgList[kStrCommField].GetValue() : kEmptyStr;
    if (NStr::EqualNocase(to_field, "Field")) {
        dest = "fieldvalue";
    }
    else if (NStr::EqualNocase(to_field, "Database name")) {
        dest = "dbname";
    }
    else if (NStr::EqualNocase(to_field, "Field name")) {
        dest = "fieldname";
    }

    NMItemData::WrapInQuotes(dest);

    string function;
    function += src_resolve_fnc;

    if (x_ParseEntireText()) {
        // parse the entire text

        function += CMacroFunction_ParseToStructComm::GetFuncName();
        function += "(" + src_var_name + ", " + dest;
        if (!dest_field_name.empty()) {
            NMItemData::WrapInQuotes(dest_field_name);
            function += ", " + dest_field_name;
        }
        function += ", " + kCapChange + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        if (m_ArgList[kRmvParse].IsTrue()) {
            function += "\n";
            if (path.find(',') != NPOS) {
                function += CMacroFunction_RemoveModifier::GetFuncName();
                function += "(" + rt_var + ");";
            }
            else {
                function += CMacroFunction_RemoveQual::GetFuncName();
                function += "(" + src_var_name + ");";
            }
        }
    }
    else {
        string parsed_text = "text";
        function += x_GetParsedTextFunction(src_var_name, parsed_text);
        function += CMacroFunction_ParseToStructComm::GetFuncName();
        function += "(" + parsed_text + ", " + dest;
        if (!dest_field_name.empty()) {
            NMItemData::WrapInQuotes(dest_field_name);
            function += ", " + dest_field_name;
        }
        function += ", " + kCapChange + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");"; 
    }

    return function;
}



END_NCBI_SCOPE
