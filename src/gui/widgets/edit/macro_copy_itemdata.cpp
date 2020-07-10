/*  $Id: macro_copy_itemdata.cpp 43544 2019-07-29 19:47:27Z asztalos $
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
#include <gui/widgets/edit/macro_copy_itemdata.hpp>
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
        return variables;
    }
}

// CCopyQualTreeItemData
CCopyQualTreeItemData::CCopyQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CCopyQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_CopyStringQual::GetFuncName(), GetCopyArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CCopyQualTreeItemData::GetMacroDescription() const
{
    string descr = "Copy " + m_ArgList[kFromField].GetValue() + " to " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CCopyQualTreeItemData::GetVariables()
{
    return GetExistingTextVariables(m_ArgList);
}

string CCopyQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool remove_constraint = true;
    pair<string, string> source = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kFromField].GetValue(), "src", constraints, remove_constraint);
    pair<string, string> dest = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kToField].GetValue(), "dest", constraints, remove_constraint);

    string function;
    function += source.first;
    function += dest.first;

    function += CMacroFunction_CopyStringQual::GetFuncName();
    function += "(" + source.second + ", " + dest.second + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";
    return function;
}


// CCopyBsrcTreeItemData
CCopyBsrcTreeItemData::CCopyBsrcTreeItemData()
    : CCopyQualTreeItemData("Copy biosource qualifiers", EMacroFieldType::eBiosourceText)
{
}

const vector<string>& CCopyBsrcTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetBsrcKeywords();
}

void CCopyBsrcTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetDualBsrcArgs());
    copy(GetCopyArgs().begin(), GetCopyArgs().end(), back_inserter(args));
    x_LoadPanel(parent, "CopyStringSourceQual", args);

    m_FieldType = EMacroFieldType::eBiosourceText;
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CCopyBsrcTreeItemData::GetFunction(TConstraints& constraints) const
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

    function += CMacroFunction_CopyStringQual::GetFuncName();
    function += "(" + src_var_name + ", " + dest + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";
    return function;
}


// CCopyProteinQualTreeItemData
CCopyProteinQualTreeItemData::CCopyProteinQualTreeItemData()
    : CCopyQualTreeItemData("Copy protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CCopyProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetCopyArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "CopyStringProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

string CCopyProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CCopyQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CCopyProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    pair<string, string> source = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kFromField].GetValue(), "src", constraints, !update_mrna);
    pair<string, string> dest = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kToField].GetValue(), "dest", constraints, !update_mrna);

    string function;
    function += source.first;
    function += dest.first;

    function += CMacroFunction_CopyStringQual::GetFuncName();
    function += "(" + source.second + ", " + dest.second + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}


// CCopyRNAQualTreeItemData
CCopyRNAQualTreeItemData::CCopyRNAQualTreeItemData()
    : CCopyQualTreeItemData("Copy RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CCopyRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetCopyArgs());
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "CopyStringRNAQual", args);

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

bool CCopyRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForDualRnas(m_ArgList[kRNAType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CCopyRNAQualTreeItemData::GetMacroDescription() const
{
    string descr = "Copy";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kFromField].GetValue() + " to " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CCopyRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
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
    bool remove_constraint = true;
    pair<string, string> source = NMItemData::GetResolveFuncForRNAQuals(src_fieldname, "src", constraints, remove_constraint);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForRNAQuals(dest_fieldname, "dest", constraints, remove_constraint);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_CopyStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kExistingText;
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

        function += CMacroFunction_CopyFeatQual::GetFuncName();
        function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\"";
        function += ", false, " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
    }
    return function;
}

// CCopyCdsGeneProtTreeItemData
CCopyCdsGeneProtTreeItemData::CCopyCdsGeneProtTreeItemData()
    : CCopyQualTreeItemData("Copy CDS gene protein mRNA mat-peptide qualifiers", EMacroFieldType::eCdsGeneProt)
{
}

void CCopyCdsGeneProtTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetCopyArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "CopyStringProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

bool CCopyCdsGeneProtTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForCdsGeneProt(m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    return modified;
}

string CCopyCdsGeneProtTreeItemData::GetMacroDescription() const
{
    string descr = CCopyQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CCopyCdsGeneProtTreeItemData::GetVariables()
{
    string vars = CCopyQualTreeItemData::GetVariables();
    string src_feat, src_field;
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), src_feat, src_field);
    string dest_feat, dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);
    if (src_feat != dest_feat) {
        vars += NMItemData::GetUpdatemRNAVar(m_ArgList);
    }
    return vars;
}

string CCopyCdsGeneProtTreeItemData::GetFunction(TConstraints& constraints) const
{
    string src_feat, src_field;
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), src_feat, src_field);
    string dest_feat, dest_field;
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), dest_feat, dest_field);

    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();

    NMItemData::UpdateConstraintsForDualCdsGeneProt(constraints, src_feat);

    string function;
    pair<string, string> source = NMItemData::GetResolveFuncForCdsGeneProtQuals(m_ArgList[kFromField].GetValue(), "src", constraints, !update_mrna);

    if (src_feat == dest_feat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForCdsGeneProtQuals(m_ArgList[kToField].GetValue(), "dest", constraints, !update_mrna);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_CopyStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        NMItemData::AddConstraintIgnoreNewText(dest.second, constraints, m_Panel->GetArgumentList());
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

        function += CMacroFunction_CopyFeatQual::GetFuncName();
        function += "(" + source.second + ", \"" + dest_feat + "\", \"" + dest_path + "\", " + kUpdatemRNA + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
    }
    return function;
}


// CCopyFeatQualTreeItemData
CCopyFeatQualTreeItemData::CCopyFeatQualTreeItemData()
    : CCopyQualTreeItemData("Copy feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CCopyFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetCopyArgs());
    args.push_back(GetOptionalArgs()[0]);
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "CopyStringFeatureQual", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFromField, qualifiers);
    m_Panel->SetControlValues(kToField, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProductDouble);
    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

bool CCopyFeatQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForDualFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CCopyFeatQualTreeItemData::GetMacroDescription() const
{
    string descr = "Copy " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kFromField].GetValue();
    descr += " to " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CCopyFeatQualTreeItemData::GetVariables()
{
    string vars = CCopyQualTreeItemData::GetVariables();
    m_Srcfeat = m_Destfeat = m_ArgList[kFeatType].GetValue();
    m_Srcfield = m_ArgList[kFromField].GetValue();
    m_Destfield = m_ArgList[kToField].GetValue();
    NMItemData::UpdateFeatureType(m_Srcfeat, m_Srcfield);
    NMItemData::UpdateFeatureType(m_Destfeat, m_Destfield);
    if (m_Srcfeat != m_Destfeat) {
        vars += NMItemData::GetUpdatemRNAVar(m_ArgList);
    }
    return vars;
}

string CCopyFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kFromField].Empty() || m_ArgList[kToField].Empty())
        return kEmptyStr;

    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();

    NMItemData::UpdateConstraintsForFeatures(constraints, m_Srcfeat);

    string function;
    pair<string, string> source = NMItemData::GetResolveFuncForFeatQuals(m_Srcfeat, m_Srcfield, "src", constraints, !update_mrna);

    if (m_Srcfeat == m_Destfeat) {
        pair<string, string> dest = NMItemData::GetResolveFuncForFeatQuals(m_Destfeat, m_Destfield, "dest", constraints, !update_mrna);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_CopyStringQual::GetFuncName();
        function += "(" + source.second + ", " + dest.second + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";

        NMItemData::AddConstraintIgnoreNewText(dest.second, constraints, m_Panel->GetArgumentList());
        NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    }
    else {
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_Destfeat, m_Destfield);

        NMItemData::AddConstraintIgnoreNewText(dest_path, constraints, m_Panel->GetArgumentList(), m_Destfeat);

        if (!source.first.empty())
            function += source.first;

        if (dest_path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(dest_path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            dest_path = tokens[1];
        }

        function += CMacroFunction_CopyFeatQual::GetFuncName();
        function += "(" + source.second + ", \"" + m_Destfeat + "\", \"" + dest_path + "\", " + kUpdatemRNA + ", " + kExistingText;
        if (m_ArgList[kDelimiter].GetEnabled()) {
            function += ", " + kDelimiter;
        }
        function += ");";
    }
    return function;
}


END_NCBI_SCOPE
