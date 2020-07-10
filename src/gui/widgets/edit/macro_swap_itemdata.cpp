/*  $Id: macro_swap_itemdata.cpp 43315 2019-06-12 14:02:14Z asztalos $
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
#include <gui/objutils/macro_edit_fn_base.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_swap_itemdata.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(macro);
USING_SCOPE(NMacroArgs);

// CSwapQualTreeItemData
CSwapQualTreeItemData::CSwapQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CSwapQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_SwapQual::GetFuncName(), GetSwapArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);
}

string CSwapQualTreeItemData::GetMacroDescription() const
{
    return "Swap " + m_ArgList[kFromField].GetValue() + " with " + m_ArgList[kToField].GetValue();
}

string CSwapQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool remove_constraint = false;
    pair<string, string> source = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kFromField].GetValue(), "src", constraints, remove_constraint);
    pair<string, string> dest = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kToField].GetValue(), "dest", constraints, remove_constraint);

    string function;
    function += source.first;
    function += dest.first;
    function += CMacroFunction_SwapQual::GetFuncName() + "(" + source.second + ", " + dest.second + ");";
    return function;
}

// CSwapBsrcTreeItemData
CSwapBsrcTreeItemData::CSwapBsrcTreeItemData()
    : CSwapQualTreeItemData("Swap biosource qualifiers", EMacroFieldType::eBiosourceText)
{
}

const vector<string>& CSwapBsrcTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetBsrcKeywords();
}

void CSwapBsrcTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSwapArgs());
    copy(GetDualBsrcArgs().begin(), GetDualBsrcArgs().end(), back_inserter(args));
    x_LoadPanel(parent, "SwapStringSourceQual", args);

    m_FieldType = EMacroFieldType::eBiosourceText;
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceText);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);
}

string CSwapBsrcTreeItemData::GetFunction(TConstraints& constraints) const
{
    //get the source field
    string src_var = "src";
    m_FieldType = NMItemData::GetSourceTypeInDoublePanel(m_Panel->GetArgumentList(), NMItemData::eFrom);

    string src_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), m_FieldType);
    string src_resolve_fnc;

    if (src_path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(src_path, ",", tokens);
        src_resolve_fnc = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], src_var, constraints, false);
    }

    // get the destination field
    string dest_var = "dest";
    m_FieldType = NMItemData::GetSourceTypeInDoublePanel(m_Panel->GetArgumentList(), NMItemData::eTo);
    string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kToField].GetValue(), m_FieldType);
    string dest_resolve_func;

    if (dest_path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(dest_path, ",", tokens);
        dest_resolve_func = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], dest_var, constraints, false);
    }

    string function;
    function += src_resolve_fnc;
    function += dest_resolve_func;

    function += CMacroFunction_SwapQual::GetFuncName() + "(";
    if (!src_resolve_fnc.empty()) {
        function += src_var + ", \"" + m_ArgList[kFromField].GetValue() + "\", ";
    }
    else {
        function += "\"" + src_path + "\", ";
    }

    if (!dest_resolve_func.empty()) {
        function += dest_var + ", \"" + m_ArgList[kToField].GetValue() + "\"";
    }
    else {
        function += "\"" + dest_path + "\"";
    }
    function += ");";
    return function;
}


// CSwapProteinQualTreeItemData
CSwapProteinQualTreeItemData::CSwapProteinQualTreeItemData()
    : CSwapQualTreeItemData("Swap protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CSwapProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSwapArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "SwapProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

string CSwapProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CSwapQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CSwapProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    pair<string, string> source = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kFromField].GetValue(), "src", constraints, !update_mrna);
    pair<string, string> dest = NMItemData::GetResolveFuncForMultiValuedQuals(m_FieldType, m_ArgList[kToField].GetValue(), "dest", constraints, !update_mrna);

    string function;
    function += source.first;
    function += dest.first;
    function += CMacroFunction_SwapQual::GetFuncName() + "(" + source.second + ", " + dest.second + ");";
    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}


// CSwapRNAQualTreeItemData
CSwapRNAQualTreeItemData::CSwapRNAQualTreeItemData()
    : CSwapQualTreeItemData("Swap RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CSwapRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSwapArgs());
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "SwapStringRNAQual", args);

    vector<string> rnas = CMacroEditorContext::GetInstance().GetRNATypes();
    m_Panel->SetControlValues(kRNAType, rnas);
    m_Panel->SetSelection(kRNAType, 0);
    vector<string> class_types = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
    m_Panel->SetControlValues(kncRNAClass, class_types);
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
}

bool CSwapRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForDualRnas(m_ArgList[kRNAType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CSwapRNAQualTreeItemData::GetMacroDescription() const
{
    string descr = "Swap";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kFromField].GetValue() + " to " + m_ArgList[kToField].GetValue();
    return descr;
}

string CSwapRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kFromField].Empty() || m_ArgList[kToField].Empty())
        return kEmptyStr;

    if (m_Target != macro::CMacroBioData::sm_Gene) {
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
    string src_var = "src", dest_var = "dest";
    bool remove_constraint = true;
    bool is_src_gbqual = false;
    pair<string, string> source = NMItemData::GetResolveFuncForSwapQuals(m_FieldType, src_fieldname, src_var, constraints, remove_constraint, is_src_gbqual);

    if (src_feat == dest_feat) {
        bool is_dest_gbqual = false;
        pair<string, string> dest = NMItemData::GetResolveFuncForSwapQuals(m_FieldType, dest_fieldname, dest_var, constraints, remove_constraint, is_dest_gbqual);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_SwapQual::GetFuncName() + "(";
        if (is_src_gbqual) {
            function += src_var + ", " + source.second;
        }
        else {
            function += source.second;
        }

        if (is_dest_gbqual) {
            function += ", " + dest_var + ", " + dest.second + "";
        }
        else {
            function += ", " + dest.second;
        }
        function += ");";
    }
    else {
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(dest_fieldname, m_FieldType);

        if (!source.first.empty())
            function += source.first;

        if (dest_path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(dest_path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            dest_path = tokens[1];
        }

        function += CMacroFunction_SwapRelFeatQual::GetFuncName() + "(";
        if (is_src_gbqual) {
            function += src_var + ", " + source.second;
        }
        else {
            function += source.second;
        }
        function += ", \"" + dest_feat + "\", \"" + dest_path + "\", false);";
    }
    return function;
}

// CSwapCdsGeneProtTreeItemData
CSwapCdsGeneProtTreeItemData::CSwapCdsGeneProtTreeItemData()
    : CSwapQualTreeItemData("Swap CDS gene protein mRNA mat-peptide qualifiers", EMacroFieldType::eCdsGeneProt)
{
}

void CSwapCdsGeneProtTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSwapArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "SwapProteinQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kFromField, fieldnames);
    m_Panel->SetControlValues(kToField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameDouble);
}

bool CSwapCdsGeneProtTreeItemData::UpdateTarget()
{
    // save the source and destination fields
    NMItemData::GetFeatureAndField(m_ArgList[kFromField].GetValue(), m_Srcfeat, m_Srcfield);
    NMItemData::GetFeatureAndField(m_ArgList[kToField].GetValue(), m_Destfeat, m_Destfield);

    CTempString new_target = NMItemData::UpdateTargetForCdsGeneProt(m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    return modified;
}

string CSwapCdsGeneProtTreeItemData::GetMacroDescription() const
{
    string descr = CSwapQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

bool CSwapCdsGeneProtTreeItemData::HasVariables() const
{
    return (m_Srcfeat != m_Destfeat);
}

string CSwapCdsGeneProtTreeItemData::GetVariables()
{
    string vars = NMItemData::GetUpdatemRNAVar(m_ArgList);
    NStr::ReplaceInPlace(vars, "\n", "");
    return vars;
}

string CSwapCdsGeneProtTreeItemData::GetFunction(TConstraints& constraints) const
{
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();

    NMItemData::UpdateConstraintsForDualCdsGeneProt(constraints, m_Srcfeat);

    string function;
    string src_var = "src", dest_var = "dest";
    bool is_src_gbqual = false;
    pair<string, string> source = NMItemData::GetResolveFuncForSwapQuals(m_FieldType, m_ArgList[kFromField].GetValue(), src_var, constraints, !update_mrna, is_src_gbqual);

    if (m_Srcfeat == m_Destfeat) {
        bool is_dest_gbqual = false;
        pair<string, string> dest = NMItemData::GetResolveFuncForSwapQuals(m_FieldType, m_ArgList[kToField].GetValue(), dest_var, constraints, !update_mrna, is_dest_gbqual);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_SwapQual::GetFuncName() + "(";
        if (is_src_gbqual) {
            function += src_var + ", " + source.second;
        }
        else {
            function += source.second;
        }

        if (is_dest_gbqual) {
            function += ", " + dest_var + ", " + dest.second + "";
        }
        else {
            function += ", " + dest.second;
        }
        function += ");";
        NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    }
    else {
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kToField].GetValue(), m_FieldType);

        if (!source.first.empty())
            function += source.first;

        if (dest_path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(dest_path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            dest_path = tokens[1];
        }

        function += CMacroFunction_SwapRelFeatQual::GetFuncName() + "(";
        if (is_src_gbqual) {
            function += src_var + ", " + source.second;
        }
        else {
            function += source.second;
        }
        function += ", \"" + m_Destfeat + "\", \"" + dest_path + "\", " + kUpdatemRNA + ");";
    }
    return function;
}


// CSwapFeatQualTreeItemData
CSwapFeatQualTreeItemData::CSwapFeatQualTreeItemData()
    : CSwapQualTreeItemData("Swap feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CSwapFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSwapArgs());
    args.push_back(GetOptionalArgs()[0]);
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "SwapStringFeatureQual", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFromField, qualifiers);
    m_Panel->SetControlValues(kToField, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProductDouble);
}

bool CSwapFeatQualTreeItemData::UpdateTarget()
{
    // save the source and destination fields
    m_Srcfeat = m_Destfeat = m_ArgList[kFeatType].GetValue();
    m_Srcfield = m_ArgList[kFromField].GetValue();
    m_Destfield = m_ArgList[kToField].GetValue();
    NMItemData::UpdateFeatureType(m_Srcfeat, m_Srcfield);
    NMItemData::UpdateFeatureType(m_Destfeat, m_Destfield);

    CTempString new_target = NMItemData::UpdateTargetForDualFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFromField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CSwapFeatQualTreeItemData::GetMacroDescription() const
{
    string descr = "Swap " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kFromField].GetValue();
    descr += " to " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kToField].GetValue();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

bool CSwapFeatQualTreeItemData::HasVariables() const
{
    return (m_Srcfeat != m_Destfeat);
}

string CSwapFeatQualTreeItemData::GetVariables()
{
    string vars = NMItemData::GetUpdatemRNAVar(m_ArgList);
    NStr::ReplaceInPlace(vars, "\n", "");
    return vars;
}

string CSwapFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kFromField].Empty() || m_ArgList[kToField].Empty())
        return kEmptyStr;

    NMItemData::UpdateConstraintsForFeatures(constraints, m_Srcfeat);

    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();

    string function;
    string src_var = "src", dest_var = "dest";
    bool is_src_gbqual = false;
    pair<string, string> source = NMItemData::GetResolveFuncForSwapFeatQuals(m_Srcfeat, m_Srcfield, src_var, constraints, !update_mrna, is_src_gbqual);

    if (m_Srcfeat == m_Destfeat) {
        bool is_dest_gbqual = false;
        pair<string, string> dest = NMItemData::GetResolveFuncForSwapFeatQuals(m_Destfeat, m_Destfield, "dest", constraints, !update_mrna, is_dest_gbqual);

        if (!source.first.empty())
            function += source.first;
        if (!dest.first.empty())
            function += dest.first;

        function += CMacroFunction_SwapQual::GetFuncName() + "(";
        if (is_src_gbqual) {
            function += src_var + ", " + source.second;
        }
        else {
            function += source.second;
        }

        if (is_dest_gbqual) {
            function += ", " + dest_var + ", " + dest.second + "";
        }
        else {
            function += ", " + dest.second;
        }
        function += ");";
        NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    }
    else {
        string dest_path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_Destfeat, m_Destfield);

        if (!source.first.empty())
            function += source.first;

        if (dest_path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(dest_path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            dest_path = tokens[1];
        }

        function += CMacroFunction_SwapRelFeatQual::GetFuncName() + "(";
        if (is_src_gbqual) {
            function += src_var + ", " + source.second;
        }
        else {
            function += source.second;
        }
        function += ", \"" + m_Destfeat + "\", \"" + dest_path + "\", " + kUpdatemRNA + ");";
    }
    return function;
}


END_NCBI_SCOPE
