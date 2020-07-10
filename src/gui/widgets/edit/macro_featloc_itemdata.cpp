/*  $Id: macro_featloc_itemdata.cpp 43315 2019-06-12 14:02:14Z asztalos $
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
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/widgets/edit/macro_featloc_itemdata.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// CEditFeatLocTreeItemData
CEditFeatLocTreeItemData::CEditFeatLocTreeItemData(const string& description, const string& panel_name, const TArgumentsVector& arguments)
    : CMacroActionItemData(description), m_PanelName(panel_name), m_Arguments(&arguments)
{
}

void CEditFeatLocTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, m_PanelName, *m_Arguments);
    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(true);
    m_Panel->SetControlValues(m_Args[0]->GetName(), features);
}

bool CEditFeatLocTreeItemData::UpdateTarget()
{
    m_Target = NMItemData::UpdateTargetForFeatures(m_Args[0]->GetValue(), kEmptyStr);
    m_SelectedField = m_Args[0]->GetValue();
    // this will always be modified, otherwise the macro might contain constraints that are invalid
    return true;
}

string CEditFeatLocTreeItemData::x_CommonEditFeatLocDescr(size_t index) const
{
    string descr = " for ";
    if (m_Args[0]->GetValue() == "All") {
        descr += "any";
    }
    else {
        descr += m_Args[0]->GetValue();
    }
    descr += " features";

    if (m_Args[index]->GetValue() == "true") {
        descr += " and retranslate affected coding regions";
    }
    if (m_Args[++index]->GetValue() == "true") {
        descr += " and adjust overlapping gene";
    }
    return descr;
}

string CEditFeatLocTreeItemData::x_CommonEditFeatLocVars(size_t index) const
{
    string variables;
    for (size_t i = index; i < m_Args.size(); ++i) {
        variables += m_Args[i]->GetName() + " = %" + m_Args[i]->GetValue() + "%\n";
    }
    variables.pop_back();
    return variables;
}

string CEditFeatLocTreeItemData::x_CommonEditFeatFunc(size_t index) const
{
    string function;
    for (size_t i = index; i < m_Args.size(); ++i) {
        function += ", " + m_Args[i]->GetName();
    }
    function += ");";
    return function;
}


// CSet5PartialTreeItemData
CSet5PartialTreeItemData::CSet5PartialTreeItemData()
    : CEditFeatLocTreeItemData("Set 5' end partial in feature locations", 
        macro::CMacroFunction_Set5Partial::GetFuncName(),
        macro::CMacroFunction_Set5Partial::sm_Arguments)
{
    m_5PartialMap.emplace("All", make_pair("all", kEmptyStr));
    m_5PartialMap.emplace("Only if at 5' end", make_pair("at-end", " when 5' end of location is at end of sequence"));
    m_5PartialMap.emplace("If bad start codon", make_pair("bad-start", " when coding region has no start codon"));
    m_5PartialMap.emplace("If CDS frame > 1", make_pair("frame-not-one", " when coding region frame > 1"));
}

string CSet5PartialTreeItemData::GetMacroDescription() const
{
    string descr = "Set 5' partial";
    auto it = m_5PartialMap.find(m_Args[1]->GetValue());
    if (it != m_5PartialMap.end()) {
        descr += it->second.second;
    }

    if (m_Args[2]->GetValue() == "true") {
        descr += ", extend 5' end of feature to end of sequence";
    }
    return descr + x_CommonEditFeatLocDescr(3);
}

string CSet5PartialTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(2);
}

string CSet5PartialTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());
    string function = macro::CMacroFunction_Set5Partial::GetFuncName();
    auto it = m_5PartialMap.find(m_Args[1]->GetValue());
    if (it != m_5PartialMap.end()) {
        function += "(\"" + it->second.first + "\"";
    }
    return function + x_CommonEditFeatFunc(2);
}


// CSet3PartialTreeItemData
CSet3PartialTreeItemData::CSet3PartialTreeItemData()
    : CEditFeatLocTreeItemData("Set 3' end partial in feature locations",
        macro::CMacroFunction_Set3Partial::GetFuncName(),
        macro::CMacroFunction_Set5Partial::sm_Arguments)
{
    m_3PartialMap.emplace("All", make_pair("all", kEmptyStr));
    m_3PartialMap.emplace("Only if at 3' end", make_pair("at-end", " when 3' end of location is at end of sequence"));
    m_3PartialMap.emplace("If bad stop codon", make_pair("bad-end", " when coding region has no stop codon"));
}

string CSet3PartialTreeItemData::GetMacroDescription() const
{
    string descr = "Set 3' partial";
    auto it = m_3PartialMap.find(m_Args[1]->GetValue());
    if (it != m_3PartialMap.end()) {
        descr += it->second.second;
    }

    if (m_Args[2]->GetValue() == "true") {
        descr += ", extend 3' end of feature to end of sequence";
    }
    return descr + x_CommonEditFeatLocDescr(3);
}

string CSet3PartialTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(2);
}

string CSet3PartialTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());
    string function = macro::CMacroFunction_Set3Partial::GetFuncName();
    auto it = m_3PartialMap.find(m_Args[1]->GetValue());
    if (it != m_3PartialMap.end()) {
        function += "(\"" + it->second.first + "\"";
    }
    return function + x_CommonEditFeatFunc(2);
}

// CSetBothPartialTreeItemData
CSetBothPartialTreeItemData::CSetBothPartialTreeItemData()
    : CEditFeatLocTreeItemData("Set both ends partial in feature locations",
        macro::CMacroFunction_SetBothPartials::GetFuncName(),
        macro::CMacroFunction_Set5Partial::sm_Arguments)
{
    m_BothPartialMap.emplace("All", make_pair("all", kEmptyStr));
    m_BothPartialMap.emplace("If both ends at end of sequence", make_pair("at-end", "when both ends of location are at end of sequence"));
}

string CSetBothPartialTreeItemData::GetMacroDescription() const
{
    string descr = "Set both ends partial";
    auto it = m_BothPartialMap.find(m_Args[1]->GetValue());
    if (it != m_BothPartialMap.end()) {
        descr += it->second.second;
    }

    if (m_Args[2]->GetValue() == "true") {
        descr += ", extend both ends of feature to end of sequence";
    }
    return descr + x_CommonEditFeatLocDescr(3);
}

string CSetBothPartialTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(2);
}

string CSetBothPartialTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());
    string function = macro::CMacroFunction_SetBothPartials::GetFuncName();
    auto it = m_BothPartialMap.find(m_Args[1]->GetValue());
    if (it != m_BothPartialMap.end()) {
        function += "(\"" + it->second.first + "\"";
    }
    return function + x_CommonEditFeatFunc(2);
}


// CClear5PartialTreeItemData
CClear5PartialTreeItemData::CClear5PartialTreeItemData()
    : CEditFeatLocTreeItemData("Clear 5' end partial in feature locations",
        macro::CMacroFunction_Clear5Partial::GetFuncName(),
        macro::CMacroFunction_Clear5Partial::sm_Arguments)
{
    m_5PartialMap.emplace("All", make_pair("all", kEmptyStr));
    m_5PartialMap.emplace("If not at 5' end", make_pair("not-at-end", "when 5' end of feature is not at end of sequence"));
    m_5PartialMap.emplace("If good start codon", make_pair("good-start", "when coding region has start codon"));
}

string CClear5PartialTreeItemData::GetMacroDescription() const
{
    string descr = "Clear 5' partial ";
    auto it = m_5PartialMap.find(m_Args[1]->GetValue());
    if (it != m_5PartialMap.end()) {
        descr += it->second.second;
    }
    return descr + x_CommonEditFeatLocDescr(2);
}

string CClear5PartialTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(2);
}

string CClear5PartialTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());
    string function = macro::CMacroFunction_Clear5Partial::GetFuncName();
    auto it = m_5PartialMap.find(m_Args[1]->GetValue());
    if (it != m_5PartialMap.end()) {
        function += "(\"" + it->second.first + "\"";
    }
    return function + x_CommonEditFeatFunc(2);
}


// CClear3PartialTreeItemData
CClear3PartialTreeItemData::CClear3PartialTreeItemData()
    : CEditFeatLocTreeItemData("Clear 3' end partial in feature locations",
        macro::CMacroFunction_Clear3Partial::GetFuncName(),
        macro::CMacroFunction_Clear5Partial::sm_Arguments)
{
    m_3PartialMap.emplace("All", make_pair("all", kEmptyStr));
    m_3PartialMap.emplace("If not at 3' end", make_pair("not-at-end", "when 3' end of feature is not at end of sequence"));
    m_3PartialMap.emplace("If good stop codon", make_pair("good-end", "when coding region has stop codon"));
}

string CClear3PartialTreeItemData::GetMacroDescription() const
{
    string descr = "Clear 3' partial ";
    auto it = m_3PartialMap.find(m_Args[1]->GetValue());
    if (it != m_3PartialMap.end()) {
        descr += it->second.second;
    }
    return descr + x_CommonEditFeatLocDescr(2);
}

string CClear3PartialTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(2);
}

string CClear3PartialTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());
    string function = macro::CMacroFunction_Clear3Partial::GetFuncName();
    auto it = m_3PartialMap.find(m_Args[1]->GetValue());
    if (it != m_3PartialMap.end()) {
        function += "(\"" + it->second.first + "\"";
    }
    return function + x_CommonEditFeatFunc(2);
}

// CClearBothPartialTreeItemData
CClearBothPartialTreeItemData::CClearBothPartialTreeItemData()
    : CEditFeatLocTreeItemData("Clear both ends partial in feature locations",
        macro::CMacroFunction_RemoveBothPartials::GetFuncName(),
        macro::CMacroFunction_Clear5Partial::sm_Arguments)
{
    m_BothPartialMap.emplace("All", make_pair("all", kEmptyStr));
    m_BothPartialMap.emplace("If both ends not at end of sequence", make_pair("not-at-end", "when both ends of feature are not at end of sequence"));
}

string CClearBothPartialTreeItemData::GetMacroDescription() const
{
    string descr = "Clear both ends partial ";
    auto it = m_BothPartialMap.find(m_Args[1]->GetValue());
    if (it != m_BothPartialMap.end()) {
        descr += it->second.second;
    }
    return descr + x_CommonEditFeatLocDescr(2);
}

string CClearBothPartialTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(2);
}

string CClearBothPartialTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());
    string function = macro::CMacroFunction_RemoveBothPartials::GetFuncName();
    auto it = m_BothPartialMap.find(m_Args[1]->GetValue());
    if (it != m_BothPartialMap.end()) {
        function += "(\"" + it->second.first + "\"";
    }
    return function + x_CommonEditFeatFunc(2);
}


// CConvertStrandTreeItemData
CConvertStrandTreeItemData::CConvertStrandTreeItemData()
    : CEditFeatLocTreeItemData("Edit feature location strand", 
        macro::CMacroFunction_ConvertLocStrand::GetFuncName(),
        macro::CMacroFunction_ConvertLocStrand::sm_Arguments)
{
}

string CConvertStrandTreeItemData::GetMacroDescription() const
{
    string from_strand = m_Args[1]->GetValue();
    string to_strand = m_Args[2]->GetValue();
    from_strand[0] = tolower((unsigned char)from_strand[0]);
    to_strand[0] = tolower((unsigned char)to_strand[0]);

    string descr = "Convert " + from_strand +" to " + to_strand;
    return descr + x_CommonEditFeatLocDescr(3);
}

string CConvertStrandTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(3);
}

string CConvertStrandTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());
    
    string from_strand = m_Args[1]->GetValue();
    string to_strand = m_Args[2]->GetValue();
    from_strand[0] = tolower((unsigned char)from_strand[0]);
    to_strand[0] = tolower((unsigned char)to_strand[0]);

    NMItemData::WrapInQuotes(from_strand);
    NMItemData::WrapInQuotes(to_strand);

    string function = macro::CMacroFunction_ConvertLocStrand::GetFuncName();
    function += "(" + from_strand + ", " + to_strand + x_CommonEditFeatFunc(3);
    return function;
}

// CConvertLocTypeTreeItemData
CConvertLocTypeTreeItemData::CConvertLocTypeTreeItemData()
    : CEditFeatLocTreeItemData("Convert feature location type",
        macro::CMacroFunction_ConvertLocType::GetFuncName(),
        macro::CMacroFunction_ConvertLocType::sm_Arguments)
{
}

string CConvertLocTypeTreeItemData::GetMacroDescription() const
{
    string type = m_Args[1]->GetValue();
    type[0] = tolower((unsigned char)type[0]);
    
    return "Convert location to " + type + x_CommonEditFeatLocDescr(2);
}

string CConvertLocTypeTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(2);
}

string CConvertLocTypeTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());

    string type = m_Args[1]->GetValue();
    type[0] = tolower((unsigned char)type[0]);
    NStr::ReplaceInPlace(type, " ", "-");
    NMItemData::WrapInQuotes(type);

    string function = macro::CMacroFunction_ConvertLocType::GetFuncName();
    function += "(" + type + x_CommonEditFeatFunc(2);
    return function;
}


// CExtendToSeqEndTreeItemData
CExtendToSeqEndTreeItemData::CExtendToSeqEndTreeItemData(EMActionType type)
    : CEditFeatLocTreeItemData("Extend feature to end of sequence",
        macro::CMacroFunction_Extend5Feature::GetFuncName(),
        macro::CMacroFunction_Extend5Feature::sm_Arguments), m_ActionType(type)
{
}

string CExtendToSeqEndTreeItemData::GetMacroDescription() const
{
    string descr = "Extend ";
    if (m_ActionType == EMActionType::eExtend5ToEnd) {
        descr += "5'";
    }
    else if (m_ActionType == EMActionType::eExtend3ToEnd) {
        descr += "3'";
    }
    descr += " end of feature to end of sequence";
    return descr + x_CommonEditFeatLocDescr(1);
}

string CExtendToSeqEndTreeItemData::GetVariables()
{
    return x_CommonEditFeatLocVars(1);
}

string CExtendToSeqEndTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_Args[0]->GetValue());

    string function;
    if (m_ActionType == EMActionType::eExtend5ToEnd) {
        function = macro::CMacroFunction_Extend5Feature::GetFuncName();
    }
    else if (m_ActionType == EMActionType::eExtend3ToEnd) {
        function = macro::CMacroFunction_Extend3Feature::GetFuncName();
    }
    function += "(" + m_Args[1]->GetName() + ", " + m_Args[2]->GetName() + ");";
    return function;
}

END_NCBI_SCOPE
