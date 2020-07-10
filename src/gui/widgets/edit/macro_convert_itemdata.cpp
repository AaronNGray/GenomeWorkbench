/*  $Id: macro_convert_itemdata.cpp 44072 2019-10-21 17:57:10Z asztalos $
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
#include <gui/objutils/convert_feat.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_convert_itemdata.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(macro);
USING_SCOPE(NMacroArgs);

// CConvertQualTreeItemData
CConvertQualTreeItemData::CConvertQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CConvertQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_ConvertStringQual::GetFuncName(), GetConvertArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CConvertQualTreeItemData::GetMacroDescription() const
{
    string descr = "Convert " + m_ArgList[kFromField].GetValue();
    descr += " to " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CConvertQualTreeItemData::GetVariables()
{
    return s_GetConvertQualVariables(m_Panel->GetArgumentList());
}

static const string s_StripName = "strip_name";

string CConvertQualTreeItemData::s_GetConvertQualVariables(const CArgumentList& args)
{
    string variables;
    variables += kCapChange + " = \"" + NMItemData::GetSelectedCapType(args) + "\"\n";
    variables += s_StripName + " = false\n";
    variables += kExistingText + " = \"" + NMItemData::GetHandleOldValue(args[kExistingText].GetValue()) + "\"";
    if (args[kDelimiter].GetEnabled()) {
        variables += "\n" + kDelimiter + " = \"" + NMItemData::GetDelimiterChar(args[kDelimiter].GetValue()) + "\"";
    }
    return variables;
}

string CConvertQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool remove_constraint = false;
    pair<string, string> source = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kFromField].GetValue(), "src", constraints, remove_constraint);
    pair<string, string> dest = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kToField].GetValue(), "dest", constraints, remove_constraint);

    string function;
    function += source.first;
    function += dest.first;

    function += CMacroFunction_ConvertStringQual::GetFuncName();
    function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + s_StripName + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    if (!m_ArgList[kLeaveOrig].IsTrue()) {
        function += "\n";
        function += CMacroFunction_RemoveQual::GetFuncName();
        function += "(" + source.second + ");";
    }
    return function;
}

// CConvertProteinQualTreeItemData
CConvertProteinQualTreeItemData::CConvertProteinQualTreeItemData()
    : CConvertQualTreeItemData("Convert protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CConvertProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetConvertArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "ConvertStringProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

string CConvertProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CConvertQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CConvertProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    pair<string, string> source = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kFromField].GetValue(), "src", constraints, !update_mrna);
    pair<string, string> dest = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kToField].GetValue(), "dest", constraints, !update_mrna);

    string function;
    function += source.first;
    function += dest.first;

    function += CMacroFunction_ConvertStringQual::GetFuncName();
    function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + s_StripName + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    if (!m_ArgList[kLeaveOrig].IsTrue()) {
        function += "\n";
        function += CMacroFunction_RemoveQual::GetFuncName();
        function += "(" + source.second + ");";
    }
    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}

// CConvertRNAQualTreeItemData
CConvertRNAQualTreeItemData::CConvertRNAQualTreeItemData()
    : CConvertQualTreeItemData("Convert RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CConvertRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetConvertArgs());
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "ConvertStringRNAQual", args);

    vector<string> rnas = CMacroEditorContext::GetInstance().GetRNATypes();
    m_Panel->SetControlValues(kRNAType, rnas);
    m_Panel->SetSelection(kRNAType, 0);
    vector<string> class_types = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
    m_Panel->SetControlValues(kncRNAClass, class_types);
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
}

bool CConvertRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForDualRnas(m_ArgList[kRNAType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CConvertRNAQualTreeItemData::GetMacroDescription() const
{
    string descr = "Convert";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kFromField].GetValue() + " to " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CConvertRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
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

    bool rmv_src = !m_ArgList[kLeaveOrig].IsTrue();

    string function;
    bool remove_constraint = false;
    string src_var = "src";
    pair<string, string> source = NMItemData::GetResolveFuncForRNAQuals(src_fieldname, src_var, constraints, remove_constraint);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForRNAQuals(dest_fieldname, "dest", constraints, remove_constraint);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_ConvertStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + s_StripName + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
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

        function += CMacroFunction_ConvertFeatQual::GetFuncName();
        function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
        function += kCapChange + ", " + s_StripName + ", false, " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
    }

    if (rmv_src) {
        function += "\n";
        if (source.second.find("::") != NPOS) {
            function += CMacroFunction_RemoveRnaProduct::GetFuncName();
            function += "();";
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
    return function;
}


// CConvertCdsGeneProtTreeItemData
CConvertCdsGeneProtTreeItemData::CConvertCdsGeneProtTreeItemData()
    : CConvertQualTreeItemData("Convert between CDS gene protein mRNA mat-peptide qualifiers", EMacroFieldType::eCdsGeneProt)
{
}

void CConvertCdsGeneProtTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetConvertArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "ConvertStringProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

bool CConvertCdsGeneProtTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForCdsGeneProt(m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    return modified;
}

string CConvertCdsGeneProtTreeItemData::GetMacroDescription() const
{
    string descr = CConvertQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CConvertCdsGeneProtTreeItemData::GetVariables()
{
    string vars = CConvertQualTreeItemData::GetVariables();
    string src_feat, src_field;
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), src_feat, src_field);
    string dest_feat, dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);
    if (src_feat != dest_feat) {
        vars += NMItemData::GetUpdatemRNAVar(m_ArgList);
    }
    return vars;
}

string CConvertCdsGeneProtTreeItemData::GetFunction(TConstraints& constraints) const
{
    string src_feat, src_field;
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), src_feat, src_field);
    string dest_feat, dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);

    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    bool rmv_src = !m_ArgList[kLeaveOrig].IsTrue();

    NMItemData::UpdateConstraintsForDualCdsGeneProt(constraints, src_feat);

    string function;
    string src_var = "src";
    pair<string, string> source = NMItemData::GetResolveFuncForCdsGeneProtQuals(m_ArgList[kFromField].GetValue(), src_var, constraints, !update_mrna);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForCdsGeneProtQuals(m_ArgList[kToField].GetValue(), "dest", constraints, !update_mrna);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty()) 
            function += dest.first;

        function += CMacroFunction_ConvertStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kCapChange +  ", " + s_StripName + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        NMItemData::AddConstraintIgnoreNewText(dest.second, constraints, m_Panel->GetArgumentList());

        if (rmv_src) {
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

        function += CMacroFunction_ConvertFeatQual::GetFuncName();
        function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
        function += kCapChange + ", " + s_StripName + ", " + kUpdatemRNA + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        if (rmv_src) {
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
    return function;
}


// CConvertFeatQualTreeItemData
CConvertFeatQualTreeItemData::CConvertFeatQualTreeItemData()
    : CConvertQualTreeItemData("Convert feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CConvertFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetConvertArgs());
    args.push_back(GetOptionalArgs()[0]);
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "ConvertStringFeatureQual", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFromField, qualifiers);
    m_Panel->SetControlValues(kToField, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProductDouble);
}

bool CConvertFeatQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForDualFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CConvertFeatQualTreeItemData::GetMacroDescription() const
{
    string descr = "Convert " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kFromField].GetValue();
    descr += " to " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CConvertFeatQualTreeItemData::GetVariables()
{
    string vars = CConvertQualTreeItemData::GetVariables();
    string src_feat = m_ArgList[kFeatType].GetValue(), src_field = m_ArgList[kFromField].GetValue();
    string dest_feat = m_ArgList[kFeatType].GetValue(), dest_field = m_ArgList[kToField].GetValue();

    NMItemData::UpdateFeatureType(src_feat, src_field);
    NMItemData::UpdateFeatureType(dest_feat, dest_field);
    if (src_feat != dest_feat) {
        vars += NMItemData::GetUpdatemRNAVar(m_ArgList);
    }
    return vars;
}

string CConvertFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string src_feat = m_ArgList[kFeatType].GetValue(), src_field = m_ArgList[kFromField].GetValue();
    string dest_feat = m_ArgList[kFeatType].GetValue(), dest_field = m_ArgList[kToField].GetValue();

    if (src_feat.empty() || src_field.empty() || dest_field.empty()) return kEmptyStr;

    NMItemData::UpdateFeatureType(src_feat, src_field);
    NMItemData::UpdateFeatureType(dest_feat, dest_field);
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    bool rmv_src = !m_ArgList[kLeaveOrig].IsTrue();

    NMItemData::UpdateConstraintsForFeatures(constraints, src_feat);

    string function;
    string src_var = "src";
    pair<string, string> source = NMItemData::GetResolveFuncForFeatQuals(src_feat, src_field, src_var, constraints, !update_mrna);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForFeatQuals(dest_feat, dest_field, "dest", constraints, !update_mrna);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_ConvertStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kCapChange + ", " + s_StripName + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        NMItemData::AddConstraintIgnoreNewText(dest.second, constraints, m_Panel->GetArgumentList());

        if (rmv_src) {
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

        function += CMacroFunction_ConvertFeatQual::GetFuncName();
        function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\", ";
        function += kCapChange + ", " + s_StripName + ", " + kUpdatemRNA + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        if (rmv_src) {
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
    return function;
}

// CConvertBsrcTreeItemData
CConvertBsrcTreeItemData::CConvertBsrcTreeItemData()
    : CConvertQualTreeItemData("Convert biosource qualifiers", EMacroFieldType::eBiosourceText)
{
}

const vector<string>& CConvertBsrcTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetBsrcKeywords();
}

void CConvertBsrcTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetConvertArgs());
    copy(GetDualBsrcArgs().begin(), GetDualBsrcArgs().end(), back_inserter(args));
    x_LoadPanel(parent, "ConvertStringSourceQual", args);

    m_FieldType = EMacroFieldType::eBiosourceText;
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CConvertBsrcTreeItemData::GetFunction(TConstraints& constraints) const
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

    function += CMacroFunction_ConvertStringQual::GetFuncName();
    function += "(" + src_var_name + ", " + dest + ", " + kCapChange + ", " + s_StripName + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    if (!m_ArgList[kLeaveOrig].IsTrue()) {
        function += "\n";
        if (src_resolve_fnc.empty()) {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + src_var_name + ");";
        }
        else {
            function += CMacroFunction_RemoveModifier::GetFuncName();
            function += "(" + rt_var + ");";
        }
    }
    return function;
}


// CConvertFeatureTreeItemData
CConvertFeatureTreeItemData::CConvertFeatureTreeItemData()
    : CMacroActionItemData("Convert features")
{
}

void CConvertFeatureTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_ConvertFeature::GetFuncName(), GetConvertFeatArgs());

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(true);
    features.erase(features.begin());
    m_Panel->SetControlValues(kFromFeat, features);
    m_Panel->SetControlValues(kToFeat, features);

    m_Panel->GetArgumentList().Attach(NMItemData::OnConvertFeatFromCDS);
    m_Panel->GetArgumentList().Attach(NMItemData::OnConvertFeatToChanged);
}

bool CConvertFeatureTreeItemData::UpdateTarget()
{
    m_Target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFromFeat].GetValue(), kEmptyStr, true);
    // this will always be modified, otherwise the macro might contain constraints that are invalid
    return true;
}

string CConvertFeatureTreeItemData::GetMacroDescription() const
{
    string descr = "Convert " + m_ArgList[kFromFeat].GetValue();
    descr += " to ";
    if (m_ArgList[kChoices].GetShown()) {
        descr += m_ArgList[kChoices].GetValue() + " ";
    }
    descr += m_ArgList[kToFeat].GetValue();
    
    if (m_ArgList[kCreateOnProtein].GetShown()) {
        if (m_ArgList[kCreateOnProtein].IsTrue()) {
            descr += " on protein sequence";
        }
        else {
            descr += " on nucleotide sequence";
        }
    }

    if (m_ArgList[kRemmRNA].GetShown()) {
        descr += ", (";
        descr += (m_ArgList[kRemmRNA].IsTrue()) ? "" : "do not ";
        descr += "remove overlapping mRNA";
    }
    if (m_ArgList[kRemGene].GetShown()) {
        descr += ", ";
        descr += (m_ArgList[kRemGene].IsTrue()) ? "" : "do not " ;
        descr += "remove overlapping gene";
    }
    if (m_ArgList[kRemTranscriptid].GetShown()) {
        descr += ", ";
        descr += (m_ArgList[kRemTranscriptid].IsTrue()) ? "" : "do not ";
        descr += "remove overlapping transcript ID)";
    }

    descr += (m_ArgList[kLeaveFeat].IsTrue()) ? ", keep" : ", remove";
    descr += " original feature";
    return descr;
}

string CConvertFeatureTreeItemData::GetVariables()
{
    string variables;
    variables += kToFeat + " = \"" + m_ArgList[kToFeat].GetValue() + "\"";
    if (m_ArgList[kRemmRNA].GetEnabled()) {
        variables += "\n" + kRemmRNA + " = %" + m_ArgList[kRemmRNA].GetValue() + "%";
    }
    if (m_ArgList[kRemGene].GetEnabled()) {
        variables += "\n" + kRemGene + " = %" + m_ArgList[kRemGene].GetValue() + "%";
    }
    if (m_ArgList[kRemTranscriptid].GetEnabled()) {
        variables += "\n" + kRemTranscriptid + " = %" + m_ArgList[kRemTranscriptid].GetValue() + "%";
    }

    if (m_ArgList[kCreateOnProtein].GetShown()) {
        variables += "\n" + kCreateOnProtein + " = %" + m_ArgList[kCreateOnProtein].GetValue() + "%";
    }
    return variables;
}

string CConvertFeatureTreeItemData::GetFunction(TConstraints& constraints) const
{
    const string& descr = m_ArgList[kConvText].GetValue();
    if (NStr::Equal(descr, kConversionNotSupported)) {
        return kEmptyStr;
    }
    
    const string& feat_from = m_ArgList[kFromFeat].GetValue();
    const string& feat_to = m_ArgList[kToFeat].GetValue();
    if (feat_from.empty() || feat_to.empty())
        return kEmptyStr;

    NMItemData::UpdateConstraintsForFeatures(constraints, feat_from);

    string function;
    if (NStr::EqualNocase(feat_from, "cds")) {
        function += CMacroFunction_ConvertCDS::GetFuncName() + "(" + kToFeat;
    }
    else {
        function += CMacroFunction_ConvertFeature::GetFuncName() + "(" + kToFeat;
    }

    if (m_ArgList[kCreateOnProtein].GetShown()) {
        function += ", " + kCreateOnProtein;
    }
    if (m_ArgList[kChoices].GetShown()) {
        function += ", \"" + m_ArgList[kChoices].GetValue() + "\"";
    }

    if (NStr::EqualNocase(feat_from, "cds")) {
        function += ", " + kRemmRNA + ", " + kRemGene + ", " + kRemTranscriptid;
    }

    function += ");";
    if (!m_ArgList[kLeaveFeat].IsTrue()) {
        function += "\n";
        function += CMacroFunction_RemoveFeature::GetFuncName() + "();";
    }
    return function;
}

END_NCBI_SCOPE
