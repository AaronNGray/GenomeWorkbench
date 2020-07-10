/*  $Id: macro_rmvoutside_itemdata.cpp 44024 2019-10-09 19:27:38Z asztalos $
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
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_string_constr.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_rmvoutside_itemdata.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(NMacroArgs);

namespace {
    string GetDelimiterDescr(const CArgumentList& args)
    {
        string descr = "Remove text ";
        string match;
        if (args[kBefore].IsTrue()) {
            if (args[kTextLeft].IsTrue() && !args[kDelLeft].Empty()) {
                match = "'" + args[kDelLeft].GetValue() + "' ";
            }
            else if (args[kDigitsLeft].IsTrue()) {
                match = "'numbers' ";
            }
            else if (args[kLetterLeft].IsTrue()) {
                match = "'letters' ";
            }
            if (!match.empty()) {
                descr += "before " + match;
            }
        }
        
        match.resize(0);
        if (args[kAfter].IsTrue()) {
            if (args[kTextRight].IsTrue() && !args[kDelRight].Empty()) {
                match += "'" + args[kDelRight].GetValue() + "' ";
            }
            else if (args[kDigitsRight].IsTrue()) {
                match = "'numbers' ";
            }
            else if (args[kLetterRight].IsTrue()) {
                match = "'letters' ";
            }
            if (!match.empty()) {
                if (descr.find("before ") != NPOS) {
                    descr += "and ";
                }
                descr += "after " + match;
            }
        }
        return descr;
    }
}

// CRmvOutsideQualTreeItemData
CRmvOutsideQualTreeItemData::CRmvOutsideQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CRmvOutsideQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDelimiterArgs());
    copy(GetRmvOutsideArgs().begin(), GetRmvOutsideArgs().end(), back_inserter(args));
    x_LoadPanel(parent, macro::CMacroFunction_RemoveOutside::GetFuncName(), args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnBeforeCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnAfterCheck);
}

bool CRmvOutsideQualTreeItemData::UpdateTarget()
{
    if (m_FieldType == EMacroFieldType::eMolinfo) {
        CTempString new_target = NMItemData::UpdateTargetForMolinfo(m_ArgList[kField].GetValue());
        bool modified = (new_target != m_Target);
        m_Target = new_target;
        return modified;
    }
    return false;
}

string CRmvOutsideQualTreeItemData::GetMacroDescription() const
{
    string descr = GetDelimiterDescr(m_ArgList) + "in ";
    if (m_FieldType == EMacroFieldType::ePubdesc) {
        descr += "publication ";
    }
    descr += m_ArgList[kField].GetValue();
    return descr;
}

string CRmvOutsideQualTreeItemData::GetVariables()
{
    string variables;
    bool value = m_ArgList[kRmvLeft].GetEnabled() && m_ArgList[kRmvLeft].IsTrue() ? true : false;
    variables += kRmvLeft + " = " + NStr::BoolToString(value) + "\n";
    value = m_ArgList[kRmvRight].GetEnabled() && m_ArgList[kRmvRight].IsTrue() ? true : false;
    variables += kRmvRight + " = " + NStr::BoolToString(value) + "\n";
    variables += kCaseInsensRmv + " = " + m_ArgList[kCaseInsensRmv].GetValue() + "\n";
    variables += kWholeWordRmv + " = " + m_ArgList[kWholeWordRmv].GetValue();
    return variables;
}

string CRmvOutsideQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    const string& field = m_ArgList[kField].GetValue();
    if (field.empty()) return function;

    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType, m_Target);
    if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, true);

        if (function.empty()) {
            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(\"" + path + "\", ";
        }
        else {
            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(" + rt_var + ", ";
        }
    }
    else if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        _ASSERT(tokens.size() == 2);
        // it is either an orgmod or a soubsource modifier
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

        function += macro::CMacroFunction_RemoveOutside::GetFuncName();
        function += "(\"" + NMItemData::GetEditableFieldForPair(field, rt_var) + "\", ";
    }
    else if (m_FieldType == EMacroFieldType::ePubdesc) {
        string rt_var;
        function = NMItemData::GetResolveFuncForPubQual(field, rt_var, constraints);

        function += macro::CMacroFunction_RemoveOutside::GetFuncName();
        function += "(" + rt_var + ", ";
    }
    else if (m_FieldType == EMacroFieldType::eDBLink) {
        string rt_var;
        bool found_constraint = false;
        function = NMItemData::GetResolveFuncForDBLink(m_ArgList[kField].GetValue(), rt_var, constraints, found_constraint);
        function += macro::CMacroFunction_RemoveOutside::GetFuncName();
        if (found_constraint) {
            function += "(" + rt_var + ", ";
        }
        else {
            function += "(\"" + rt_var + ".data.strs\", ";
        }
    }
    else {
        function += macro::CMacroFunction_RemoveOutside::GetFuncName();
        function += "(\"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType) + "\", ";
    }

    if (m_FieldType == EMacroFieldType::eMiscDescriptors) {
        NMItemData::UpdateConstraintsForMisc(constraints, m_ArgList[kField].GetValue(), m_Target);
    }

    function += x_GetMarkedTextFunction() + ", " + kCaseInsensRmv + ", " + kWholeWordRmv + ");";
    return function;
}

string CRmvOutsideQualTreeItemData::x_GetMarkedTextFunction() const
{
    string left_del;
    if (m_ArgList[kBefore].IsTrue()) {
        if (m_ArgList[kTextLeft].IsTrue()) {
            left_del = m_ArgList[kDelLeft].GetValue();
        }
        else if (m_ArgList[kDigitsLeft].IsTrue()) {
            left_del = "eDigits";
        }
        else if (m_ArgList[kLetterLeft].IsTrue()) {
            left_del = "eLetters";
        }
    }

    NMItemData::WrapInQuotes(left_del);
    string function = m_ArgList[kBefore].GetValue() + ", "  + left_del + ", " + kRmvLeft + ", ";

    string right_del;
    if (m_ArgList[kAfter].IsTrue()) {
        if (m_ArgList[kTextRight].IsTrue()) {
            right_del = m_ArgList[kDelRight].GetValue();
        }
        else if (m_ArgList[kDigitsRight].IsTrue()) {
            right_del = "eDigits";
        }
        else if (m_ArgList[kLetterRight].IsTrue()) {
            right_del = "eLetters";
        }
    }

    NMItemData::WrapInQuotes(right_del);
    function += m_ArgList[kAfter].GetValue() + ", " + right_del + ", " + kRmvRight;
    return function;
}
 

// CRmvOutsideBsrcTreeItemData
CRmvOutsideBsrcTreeItemData::CRmvOutsideBsrcTreeItemData()
    : CRmvOutsideQualTreeItemData("Remove text outside string in biosource qualifiers", EMacroFieldType::eBiosourceText)
{
}

const vector<string>& CRmvOutsideBsrcTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetBsrcKeywords();
}

void CRmvOutsideBsrcTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDelimiterArgs());
    copy(GetRmvOutsideArgs().begin(), GetRmvOutsideArgs().end(), back_inserter(args));
    args.push_back(GetBsrcArgs()[0]);
    args.push_back(GetBsrcArgs()[1]);
    x_LoadPanel(parent, "RemoveOutsideStringSourceQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnBeforeCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnAfterCheck);
}

string CRmvOutsideBsrcTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kField].Empty()) return kEmptyStr;
    string function;
    m_FieldType = NMItemData::GetSourceTypeInSinglePanel(m_ArgList);
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), m_FieldType);
    if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        _ASSERT(tokens.size() == 2);
        // it is either an orgmod or a soubsource modifier
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

        function += macro::CMacroFunction_RemoveOutside::GetFuncName();
        function += "(\"" + NMItemData::GetEditableFieldForPair(m_ArgList[kField].GetValue(), rt_var) + "\", ";
    }
    else {
        function = macro::CMacroFunction_RemoveOutside::GetFuncName();
        function += "(\"" + path + "\", ";
    }

    function += x_GetMarkedTextFunction() + ", " + kCaseInsensRmv + ", " + kWholeWordRmv + ");";
    return function;
}


// CRmvOutsideStrCommQualTreeItemData
CRmvOutsideStrCommQualTreeItemData::CRmvOutsideStrCommQualTreeItemData()
    : CRmvOutsideQualTreeItemData("Remove text outside string in structured comment fields", EMacroFieldType::eStructComment)
{
}

void CRmvOutsideStrCommQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDelimiterArgs());
    copy(GetRmvOutsideArgs().begin(), GetRmvOutsideArgs().end(), back_inserter(args));
    args.push_back(GetOptionalArgs()[5]);
    x_LoadPanel(parent, "RemoveOutsideStringStrCommQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);
    m_Panel->SetSelection(kField, 0);

    m_Panel->GetArgumentList().Attach(NMItemData::OnStructCommFieldChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnBeforeCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnAfterCheck);
}

string CRmvOutsideStrCommQualTreeItemData::GetMacroDescription() const
{
    string descr = GetDelimiterDescr(m_ArgList) + "in ";
    descr += NMItemData::GetStructCommQualDescr(m_ArgList);
    return descr;
}

string CRmvOutsideStrCommQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string field_name = (m_ArgList[kStrCommField].GetShown()) ? m_ArgList[kStrCommField].GetValue() : kEmptyStr;
    string rt_var = "obj";
    string function = NMItemData::GetResolveFuncForStrCommQual(m_ArgList[kField].GetValue(), field_name, rt_var, constraints) + "\n";


    function += macro::CMacroFunction_RemoveOutside::GetFuncName();
    if (NStr::EqualNocase(m_ArgList[kField].GetValue().get(), "Field name")) {
        function += "(\"" + rt_var + ".label.str\", ";
    }
    else {
        function += "(\"" + rt_var + ".data.str\", ";
    }

    function += x_GetMarkedTextFunction() + ", " + kCaseInsensRmv + ", " + kWholeWordRmv + ");";
    return function;
}

// CRmvOutsideRNAQualTreeItemData
CRmvOutsideRNAQualTreeItemData::CRmvOutsideRNAQualTreeItemData()
    : CRmvOutsideQualTreeItemData("Remove text outside string in RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CRmvOutsideRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDelimiterArgs());
    copy(GetRmvOutsideArgs().begin(), GetRmvOutsideArgs().end(), back_inserter(args));
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "RemoveOutsideStringRNAQual", args);

    vector<string> rnas = CMacroEditorContext::GetInstance().GetRNATypes();
    m_Panel->SetControlValues(kRNAType, rnas);
    m_Panel->SetSelection(kRNAType, 0);
    vector<string> class_types = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
    m_Panel->SetControlValues(kncRNAClass, class_types);
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnBeforeCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnAfterCheck);
}

bool CRmvOutsideRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForRnas(m_ArgList[kRNAType].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CRmvOutsideRNAQualTreeItemData::GetMacroDescription() const
{
    string descr = GetDelimiterDescr(m_ArgList) + "in ";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kField].GetValue();
    return descr;
}

string CRmvOutsideRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForRnas(constraints, m_ArgList);

    string function;
    if (NStr::StartsWith(m_ArgList[kField].GetValue().get(), "gene")) {
        string gene_field = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), EMacroFieldType::eGene);
        NMItemData::WrapInQuotes(gene_field);
        function = macro::CMacroFunction_RmvOutsideRelFeatQual::GetFuncName();
        function += "(\"gene\", " + gene_field + ", ";
    }
    else {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kField].GetValue(), m_FieldType);
        if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            string rt_var = "obj";
            function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(\"" + NMItemData::GetEditableFieldForPair(tokens[1], rt_var) + "\", ";
        }
        else {
            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(\"" + path + "\", ";
        }
    }

    function += x_GetMarkedTextFunction() + ", " + kCaseInsensRmv + ", " + kWholeWordRmv + ");";
    return function;
}


// CRmvOutsideProteinQualTreeItemData
CRmvOutsideProteinQualTreeItemData::CRmvOutsideProteinQualTreeItemData()
    : CRmvOutsideQualTreeItemData("Remove text outside string in protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CRmvOutsideProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDelimiterArgs());
    copy(GetRmvOutsideArgs().begin(), GetRmvOutsideArgs().end(), back_inserter(args));
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "RemoveOutsideStringProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnBeforeCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnAfterCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

string CRmvOutsideProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CRmvOutsideQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CRmvOutsideProteinQualTreeItemData::GetVariables()
{
    return CRmvOutsideQualTreeItemData::GetVariables() + NMItemData::GetUpdatemRNAVar(m_ArgList);
}

string CRmvOutsideProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), m_FieldType);
    if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        bool remove_constraint = true;
        string res_function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

        if (res_function.empty()) {
            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(\"" + path + "\", ";
        }
        else {
            function += res_function;
            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(" + rt_var + ", ";
        }
    }
    else {
        function += macro::CMacroFunction_RemoveOutside::GetFuncName();
        function += "(\"" + path + "\", ";
    }

    function += x_GetMarkedTextFunction();
    function += ", " + kCaseInsensRmv + ", " + kWholeWordRmv + ", " + kUpdatemRNA + ");";
    return function;
}

// CRmvOutsideFeatQualTreeItemData
CRmvOutsideFeatQualTreeItemData::CRmvOutsideFeatQualTreeItemData()
    : CRmvOutsideQualTreeItemData("Remove text outside string in feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CRmvOutsideFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDelimiterArgs());
    copy(GetRmvOutsideArgs().begin() + 1, GetRmvOutsideArgs().end(), back_inserter(args));
    for (size_t index = 0; index < 3; ++index) {
        args.push_back(GetOptionalArgs()[index]);
    }
    x_LoadPanel(parent, "RemoveOutsideStringFeatureQual", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFeatQual, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnBeforeCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnAfterCheck);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

bool CRmvOutsideFeatQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CRmvOutsideFeatQualTreeItemData::GetMacroDescription() const
{
    string descr = GetDelimiterDescr(m_ArgList) + "in ";
    descr += m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kFeatQual].GetValue();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CRmvOutsideFeatQualTreeItemData::GetVariables()
{
    return CRmvOutsideQualTreeItemData::GetVariables() + NMItemData::GetUpdatemRNAVar(m_ArgList);
}

string CRmvOutsideFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());

    string function;
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());
    if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        bool remove_constraint = true;
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

        if (function.empty()) {
            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(\"" + path + "\", ";
        }
        else {
            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(" + rt_var + ", ";
        }
    }
    else if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);
        _ASSERT(tokens.size() == 2);

        string rt_var = "obj";
        if (!NStr::EqualNocase(m_ArgList[kFeatType].GetValue().get(), "gene") && NMItemData::IsGeneQualifier(tokens[1])) {
            function = NMItemData::GetResolveFuncForGeneQuals(tokens[0], tokens[1], rt_var, constraints);

            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(" + rt_var + ", ";
        }
        else {
            function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

            function += macro::CMacroFunction_RemoveOutside::GetFuncName();
            function += "(\"" + NMItemData::GetEditableFieldForPair(m_ArgList[kFeatQual].GetValue(), rt_var) + "\", ";
        }
    }
    else {
        function += macro::CMacroFunction_RemoveOutside::GetFuncName();
        function += "(\"" + path + "\", ";
    }

    function += x_GetMarkedTextFunction();
    function += ", " + kCaseInsensRmv + ", " + kWholeWordRmv + ", " + kUpdatemRNA + ");";
    return function;
}




END_NCBI_SCOPE
