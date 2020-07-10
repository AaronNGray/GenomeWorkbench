/*  $Id: macro_general_itemdata.cpp 44823 2020-03-23 17:27:53Z asztalos $
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
#include <objects/seq/Seq_gap.hpp>
#include <util/xregexp/regexp.hpp>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_entry.hpp>
#include <gui/objutils/macro_fn_string_constr.hpp>
#include <gui/objutils/macro_fn_where.hpp>

#include <misc/discrepancy/discrepancy.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/rawseqtodeltabyn_panel.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_general_itemdata.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(macro);
USING_SCOPE(NMacroArgs);


// CFixPrimerTreeItemData
CFixPrimerTreeItemData::CFixPrimerTreeItemData(const string& description, bool fixI)
    : CMacroActionSimpleItemData(kEmptyStr, description), m_FixI(fixI)
{
}

string CFixPrimerTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function = x_GetFncName();
    function += "(\"pcr-primers..forward..seq\");\n";
    function += x_GetFncName();
    function += "(\"pcr-primers..reverse..seq\");";
    return function;
}

CTempString CFixPrimerTreeItemData::x_GetFncName() const
{
    if (m_FixI) {
        return CTempString(CMacroFunction_FixIInPrimers::GetFuncName());
    }
    return CTempString(CMacroFunction_TrimJunkFromPrimers::GetFuncName());
}

// CFixSubsrcFormatTreeItemData
CFixSubsrcFormatTreeItemData::CFixSubsrcFormatTreeItemData(const string& description)
    : CMacroActionItemData(description)
{}

void CFixSubsrcFormatTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_FixFormat::GetFuncName(),
        CMacroFunction_FixFormat::sm_Arguments);
    
    vector<string> fields{ "altitude", "collection-date", "lat-lon" };
    m_Panel->SetControlValues(m_Args[0]->GetName(), fields);
}

string CFixSubsrcFormatTreeItemData::GetMacroDescription() const
{
    return "Fix " + m_Args[0]->GetValue() + " format";
}

string CFixSubsrcFormatTreeItemData::GetFunction(TConstraints& constraints) const
{
    string rt_var = "obj";
    string function = NMItemData::GetResolveFuncForQual("subtype", m_Args[0]->GetValue(), rt_var, constraints);
    function += CMacroFunction_FixFormat::GetFuncName() + CTempString("(" + rt_var + ");");
    return function;
}


// CFixSrcQualsTreeItemData
CFixSrcQualsTreeItemData::CFixSrcQualsTreeItemData(const string& description)
    : CMacroActionItemData(description, EMacroFieldType::eBiosourceText)
{
}

void CFixSrcQualsTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_FixSourceQualCaps::GetFuncName(),
        CMacroFunction_FixSourceQualCaps::sm_Arguments);
}

string CFixSrcQualsTreeItemData::GetMacroDescription() const
{
    return "Fix capitalization in " + m_Args[0]->GetValue() + " source qualifier";
}

string CFixSrcQualsTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_Args[0]->GetValue(), m_FieldType);
    if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        _ASSERT(tokens.size() == 2);
        // it is either an orgmod or a soubsource modifier
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

        function += CMacroFunction_FixSourceQualCaps::GetFuncName() + "(" + rt_var + ");";
    }
    return function;
}


// CFixMouseStrainTreeItemData
CFixMouseStrainTreeItemData::CFixMouseStrainTreeItemData(const string& description)
    : CMacroActionSimpleItemData(kEmptyStr, description)
{
}

string CFixMouseStrainTreeItemData::GetFunction(TConstraints& constraints) const
{
    string rt_var = "obj";
    string function = NMItemData::GetResolveFuncForQual("org.orgname.mod", "strain", rt_var, constraints);
    function += CMacroFunction_FixMouseStrain::GetFuncName() + CTempString("(" + rt_var + ");");
    return function;
}


// CFixPubCapsTreeItemData
CFixPubCapsTreeItemData::CFixPubCapsTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CFixPubCapsTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_FixPubCapsTitle::GetFuncName(),
        CMacroFunction_FixPubCapsTitle::sm_Arguments);
}

string CFixPubCapsTreeItemData::GetMacroDescription() const
{
    string descr = "Fix pub ";
    switch (m_FieldType) {
    case EMacroFieldType::ePubAffil:
        descr += "affiliation";
        break;
    case EMacroFieldType::ePubAffilCountry:
        descr += "affiliation country";
        break;
    case EMacroFieldType::ePubAffilExcept:
        descr += "affiliation except institute and department";
        break;
    case EMacroFieldType::ePubAuthors:
        descr += "authors";
        break;
    case EMacroFieldType::ePubTitle:
        descr += "title";
        break;
    default:
        descr.resize(0);
    }
    return descr;
}

string CFixPubCapsTreeItemData::GetVariables()
{
    return m_Args[0]->GetName() + " = %" + m_Args[0]->GetValue() + "%";
}

string CFixPubCapsTreeItemData::GetFunction(TConstraints& constraints) const
{
    string rt_var = "obj";
    string function;

    switch (m_FieldType) {
    case EMacroFieldType::ePubAffil:
        function = NMItemData::GetResolveFuncForPubQualConst("affiliation", rt_var, constraints, m_FieldType);
        function += CMacroFunction_FixPubCapsAffil::GetFuncName();
        break;
    case EMacroFieldType::ePubAffilCountry:
        function = NMItemData::GetResolveFuncForPubQualConst(CPubFieldType::GetLabelForType(CPubFieldType::ePubFieldType_AffilCountry), rt_var, constraints, m_FieldType);
        function += CMacroFunction_FixPubCapsAffilCountry::GetFuncName();
        break;
    case EMacroFieldType::ePubAffilExcept:
        function = NMItemData::GetResolveFuncForPubQualConst("affiliation", rt_var, constraints, m_FieldType);
        function += CMacroFunction_FixPubCapsAffilWithExcept::GetFuncName();
        break;
    case EMacroFieldType::ePubTitle:
        function = NMItemData::GetResolveFuncForPubQualConst(CPubFieldType::GetLabelForType(CPubFieldType::ePubFieldType_Title), rt_var, constraints, m_FieldType);
        function += CMacroFunction_FixPubCapsTitle::GetFuncName();
        break;
    case EMacroFieldType::ePubAuthors:
        function = NMItemData::GetResolveFuncForPubQualConst(CPubFieldType::GetLabelForType(CPubFieldType::ePubFieldType_Authors), rt_var, constraints, m_FieldType);
        function += CMacroFunction_FixPubCapsAuthor::GetFuncName();
        break;
    default:
        break;
    }
    function += "(" + rt_var + ", " + m_Args[0]->GetName() + ");";

    return function;
}

// CFixCapsLastNameTreeItemData
CFixCapsLastNameTreeItemData::CFixCapsLastNameTreeItemData(const string& description)
    : CMacroActionSimpleItemData(kEmptyStr, description)
{
}

string CFixCapsLastNameTreeItemData::GetFunction(TConstraints& constraints) const
{
    string rt_var;
    string function = NMItemData::GetResolveFuncForPubQual("author last name", rt_var, constraints);

    function += CMacroFunction_FixAuthorCaps::GetFuncName();
    function += "(" + rt_var + ");";
    return function;
}

// CRetranslateCDSTreeItemData
CRetranslateCDSTreeItemData::CRetranslateCDSTreeItemData(const string& description)
    : CMacroActionItemData(description)
{
}

void CRetranslateCDSTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args{ GetOptionalArgs()[7] };
    x_LoadPanel(parent, CMacroFunction_RetranslateCDS::GetFuncName(), args);
}

string CRetranslateCDSTreeItemData::GetVariables()
{
    return kObeyStopCodon + " = %" + m_ArgList[kObeyStopCodon].GetValue() + "%";
}

string CRetranslateCDSTreeItemData::GetFunction(TConstraints& constraints) const
{
    return CMacroFunction_RetranslateCDS::GetFuncName() + "(" + kObeyStopCodon + ");";
}


// CUpdateReplacedECTreeItemData
CUpdateReplacedECTreeItemData::CUpdateReplacedECTreeItemData(const string& description)
    : CMacroActionItemData(description)
{
}

void CUpdateReplacedECTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_UpdateReplacedECNumbers::GetFuncName(), GetUpdateRpldECNumbersArgs());
}

string CUpdateReplacedECTreeItemData::GetMacroDescription() const
{
    return m_Description;
}

string CUpdateReplacedECTreeItemData::GetVariables()
{
    string variables;
    variables += kDelImproper + " = " + m_ArgList[kDelImproper].GetValue() + "\n";
    variables += kDelUnrecog + " = " + m_ArgList[kDelUnrecog].GetValue() + "\n";
    variables += kDelMultRepl + " = " + m_ArgList[kDelMultRepl].GetValue() + "\n";
    return variables;
}

string CUpdateReplacedECTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function = CMacroFunction_UpdateReplacedECNumbers::GetFuncName();
    function += "(" + kDelImproper + ", " + kDelUnrecog + ", " + kDelMultRepl + ");";
    return function;
}

// CAddGeneXrefTreeItemData
CAddGeneXrefTreeItemData::CAddGeneXrefTreeItemData(const string& description)
    : CMacroActionItemData(description, EMacroFieldType::eFeatQualifiers)
{
}

void CAddGeneXrefTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args{ GetOptionalArgs()[0] };
    x_LoadPanel(parent, CMacroFunction_AddGeneXref::GetFuncName(), args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(false);
    m_Panel->SetControlValues(kFeatType, features);
}

bool CAddGeneXrefTreeItemData::UpdateTarget()
{
    m_Target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), kEmptyStr);
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    // this will always be modified, otherwise the macro might contain constraints that are invalid
    return true;
}

string CAddGeneXrefTreeItemData::GetMacroDescription() const
{
    return "Add gene Xrefs from overlapping gene features for " + m_ArgList[kFeatType].GetValue() + " features";
}

string CAddGeneXrefTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());
    return CMacroFunction_AddGeneXref::GetFuncName() + "();";
}

// CRemoveGeneXrefTreeItemData
CRemoveGeneXrefTreeItemData::CRemoveGeneXrefTreeItemData(const string& description)
    : CMacroActionItemData(description, EMacroFieldType::eFeatQualifiers)
{
}

void CRemoveGeneXrefTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_RemoveGeneXref::GetFuncName(), GetRemoveGeneXrefArgs());

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(true);
    m_Panel->SetControlValues(kFeatType, features);
}

bool CRemoveGeneXrefTreeItemData::UpdateTarget()
{
    m_Target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), kEmptyStr);
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    // this will always be modified, otherwise the macro might contain constraints that are invalid
    return true;
}

string CRemoveGeneXrefTreeItemData::GetMacroDescription() const
{
    string descr = "Remove";
    string suppr_type = x_TranslateType(m_ArgList[kSuppressingType].GetValue());
    string necess_type = x_TranslateType(m_ArgList[kNecessaryType].GetValue());

    if (suppr_type != "any") {
        descr += " " + suppr_type;
    }
    if (necess_type != "any") {
        descr += " " + necess_type;
    }

    descr += " gene xrefs from " + m_ArgList[kFeatType].GetValue() + " features";
    return descr;
}

string CRemoveGeneXrefTreeItemData::GetVariables()
{
    string variables;
    variables += kSuppressingType + " = %" + x_TranslateType(m_ArgList[kSuppressingType].GetValue()) + "%\n";
    variables += kNecessaryType + " = %" + x_TranslateType(m_ArgList[kNecessaryType].GetValue()) + "%";
    return variables;
}

string CRemoveGeneXrefTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());
    string function = CMacroFunction_RemoveGeneXref::GetFuncName();
    function += "(" + kSuppressingType + ", " + kNecessaryType + ");";
    return function;
}

string CRemoveGeneXrefTreeItemData::x_TranslateType(const string& dlg_type) const
{
    string type = dlg_type;
    NStr::ToLower(type);
    NStr::ReplaceInPlace(type, "-", "");
    if (type.find(" or ") != NPOS) {
        type = "any";
    }
    return type;
}


// CConvertClassTreeItemData
CConvertClassTreeItemData::CConvertClassTreeItemData()
    : CMacroActionItemData("Convert set class type")
{
}

void CConvertClassTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetConvertSetArgs());
    x_LoadPanel(parent, "ConvertClassType", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eSetFrom);
    m_Panel->SetControlValues(kFromField, fieldnames);
    fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eSetTo);
    m_Panel->SetControlValues(kToField, fieldnames);
    m_FieldType = EMacroFieldType::eSetFrom;
}

string CConvertClassTreeItemData::GetMacroDescription() const
{
    string descr = "Convert " + m_ArgList[kFromField].GetValue();
    descr += "s to " + m_ArgList[kToField].GetValue() + "s";
    if (m_ArgList[kInconsTaxa].IsTrue()) {
        descr += " when taxnames are inconsistent";
    }
    return descr;
}

string CConvertClassTreeItemData::GetFunction(TConstraints& constraints) const
{
    string new_constraint = CMacroFunction_StringConstraints::sm_Equal;
    new_constraint += "(\"class\", \"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFromField].GetValue(), m_FieldType) + "\")";
    constraints.emplace_back("class", new_constraint);
    if (m_ArgList[kInconsTaxa].IsTrue()) {
        constraints.emplace_back(kEmptyStr, CMacroFunction_InconsistentTaxa::GetFuncName() + CTempString("()"));
    }

    string func = CMacroFunction_SetQual::GetFuncName();
    func += "(\"class\", \"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kToField].GetValue(), m_FieldType) + "\");";
    return func;
}


/// CDiscrepancyAutofixTreeItemData
CDiscrepancyAutofixTreeItemData::CDiscrepancyAutofixTreeItemData(const string& description)
    : CMacroActionItemData(description)
{
}

void CDiscrepancyAutofixTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_DiscrepancyAutofix::GetFuncName(),
        CMacroFunction_DiscrepancyAutofix::sm_Arguments);

    vector<string> test_names = NDiscrepancy::GetDiscrepancyNames(NDiscrepancy::eAutofix);
    m_Panel->SetControlValues(m_Args[0]->GetName(), test_names);
}

string CDiscrepancyAutofixTreeItemData::GetMacroDescription() const
{
    string descr = "Perform autofix for ";
    if (!m_Args.empty()) {
        descr += m_Args.front()->GetValue();
    }
    descr += " discrepancy report test";
    return descr;
}
    
string CDiscrepancyAutofixTreeItemData::GetVariables()
{
    m_TestVars.resize(0);
    NStr::Split(m_Args.front()->GetValue().get(), ", ", m_TestVars, NStr::fSplit_Tokenize);

    string variables;
    if (m_TestVars.size() == 1) {
        variables = m_Args[0]->GetName() + "= \"" + m_TestVars[0] + "\"";
    }
    else {
        unsigned int count = 0;
        for (auto& it : m_TestVars) {
            if (!variables.empty()) {
                variables += "\n";
            }
            variables += m_Args[0]->GetName() + NStr::IntToString(++count) + " = \"" + it + "\"";
        }
    }
    return variables;
}

string CDiscrepancyAutofixTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    if (m_Args.empty()) {
        function += CMacroFunction_DiscrepancyAutofix::GetFuncName() + "();";
    }
    else {
        if (m_TestVars.size() == 1) {
            function += CMacroFunction_DiscrepancyAutofix::GetFuncName();
            function += "(" + m_Args[0]->GetName() + ");";
        }
        else {
            for (size_t count = 0; count < m_TestVars.size(); ++count) {
                if (!function.empty()) {
                    function += "\n";
                }
                function += CMacroFunction_DiscrepancyAutofix::GetFuncName();
                function += "(" + m_Args[0]->GetName() + NStr::SizetToString(count + 1) + ");";
            }
        }

    }

    return function;
}


// CAutodefTreeItemData
CAutodefTreeItemData::CAutodefTreeItemData()
    : CMacroActionItemData("Generate definition lines", EMacroFieldType::eBiosourceAutodef)
{
}

void CAutodefTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_Autodef::GetFuncName(),
        CMacroFunction_Autodef::sm_Arguments);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(m_Args[0]->GetName(), fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnMiscFeatRuleSelected);
}

string CAutodefTreeItemData::GetMacroDescription() const
{
    string descr = "Autodef " + m_Args[1]->GetValue();
    if (!m_Args[0]->GetValue().get().empty()) {
        descr += " with modifier " + m_Args[0]->GetValue();
    }

    string misc_feat_rule = (m_Args[3]->GetEnabled()) ? m_Args[3]->GetValue().get() : "Delete";
    if (NStr::StartsWith(misc_feat_rule, "Look for")) {
        misc_feat_rule = "look for non-coding product in misc-feature comment";
    }
    else if (NStr::StartsWith(misc_feat_rule, "Use comment")) {
        misc_feat_rule = "use misc-feature comment before first semicolon";
    }

    if (misc_feat_rule != "Delete") {
        descr += ", " + misc_feat_rule;
    }
    return descr;
}

string CAutodefTreeItemData::GetVariables()
{
    string variables;
    variables = m_Args[1]->GetName() + " = \"" + m_Args[1]->GetValue() + "\"\n";
    string misc_feat_rule = (m_Args[3]->GetEnabled()) ? m_Args[3]->GetValue().get() : "Delete";
    if (NStr::StartsWith(misc_feat_rule, "Look for")) {
        misc_feat_rule = "NonCodingProductFeat";
    }
    else if (NStr::StartsWith(misc_feat_rule, "Use comment")) {
        misc_feat_rule = "CommentFeat";
    }
    variables += m_Args[3]->GetName() + " = \"" + misc_feat_rule + "\"";
    return variables;
}

string CAutodefTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    function = CMacroFunction_Autodef::GetFuncName();
    function += "(" + m_Args[1]->GetName() + ", " + m_Args[3]->GetName();

    vector<string> modifiers;
    NStr::Split(m_Args[0]->GetValue().get(), ", ", modifiers, NStr::fSplit_Tokenize);

    if (!modifiers.empty()) {
        for (auto& it : modifiers) {
            function += ", \"" + it + "\"";
        }
    }
    function += ");";
    return function;
}


// CAddAssemblyGapsNTreeItemData
CAddAssemblyGapsNTreeItemData::CAddAssemblyGapsNTreeItemData()
    : CMacroActionItemData("Convert raw sequence to delta by Ns")
{
}

void CAddAssemblyGapsNTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args;
    x_LoadPanel(parent, CMacroFunction_ConvertRawToDeltabyNs::GetFuncName(), args);
    m_RawSeqToDeltaPanel = dynamic_cast<CRawSeqToDeltaByNPanel*>(m_Panel->LoadCustomPanel(ECustomPanelType::eAddAssmGapsByNPanel));
}

string CAddAssemblyGapsNTreeItemData::GetMacroDescription() const
{
    string descr = "Add assembly gaps ";
    bool is_assembly_gap = false;
    string gap_type, linkage, linkage_evidence;
    if (m_RawSeqToDeltaPanel)
        m_RawSeqToDeltaPanel->GetAssemblyParams(is_assembly_gap, gap_type, linkage, linkage_evidence);
    if (!is_assembly_gap) {
        return kEmptyStr;
    }

    descr += "with gap_type '" + gap_type + "', ";
    if (!linkage.empty()) {
        descr += "with linkage '" + linkage + "', ";
    }
    else {
        descr += "with no linkage, ";
    }

    if (!linkage_evidence.empty()) {
        descr += "with linkage_evidence '" + linkage_evidence + "'";
    }
    else {
        descr += "with no linkage evidence";
    }
    if (m_RawSeqToDeltaPanel && m_RawSeqToDeltaPanel->GetAdjustCDS()) {
        descr += ", adjust coding regions";
    }
    if (m_RawSeqToDeltaPanel && m_RawSeqToDeltaPanel->GetKeepGapLength()) {
        descr += ", keep gap length";
    }
    return descr;
}

string CAddAssemblyGapsNTreeItemData::GetVariables()
{
    if (!m_RawSeqToDeltaPanel)
        return kEmptyStr;
    long min_known;
    int max_known;
    long min_unknown;
    int max_unknown;
    m_RawSeqToDeltaPanel->GetMinMax(min_known, max_known, min_unknown, max_unknown);

    int gap_type = CSeq_gap::eType_unknown;
    int linkage = -1;
    int linkage_evidence = -1;
    m_RawSeqToDeltaPanel->GetAssemblyParams(m_IsAssemblyGap, gap_type, linkage, linkage_evidence);

    string variables;
    size_t index = 0;
    variables += m_Vars[index] + " = " + NStr::Int8ToString(min_unknown) + "\n";
    variables += m_Vars[++index] + " = " + NStr::Int8ToString(max_unknown) + "\n";
    variables += m_Vars[++index] + " = " + NStr::Int8ToString(min_known) + "\n";
    variables += m_Vars[++index] + " = " + NStr::Int8ToString(max_known) + "\n";
    variables += m_Vars[++index] + " = " + NStr::BoolToString(m_RawSeqToDeltaPanel->GetAdjustCDS()) + "\n";
    variables += m_Vars[++index] + " = " + NStr::BoolToString(m_RawSeqToDeltaPanel->GetKeepGapLength()) + "\n";
    variables += m_Vars[++index] + " = \"" + CSeq_gap::ENUM_METHOD_NAME(EType)()->FindName(gap_type, true) + "\"\n";
    variables += m_Vars[++index] + " = \"" + CSeq_gap::ENUM_METHOD_NAME(ELinkage)()->FindName(linkage, true) +"\"\n";
    variables += m_Vars[++index] + " = \"" + CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindName(linkage_evidence, true) + "\"";
    return variables;
}

string CAddAssemblyGapsNTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (!m_IsAssemblyGap) {
        return kEmptyStr;
    }

    string function;
    function = CMacroFunction_ConvertRawToDeltabyNs::GetFuncName();
    function += "(" + m_Vars.front();
    for (size_t index = 1; index < m_Vars.size(); ++index) {
        function += ", " + m_Vars[index];
    }
    function += ");";
    return function;
}


// CSetExceptionsTreeItemData
CSetExceptionsTreeItemData::CSetExceptionsTreeItemData()
    : CMacroActionItemData("Set feature exception", EMacroFieldType::eFeatQualifiers)
{
}

void CSetExceptionsTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, "SetFeatException", GetSetExceptionArgs());

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(false);
    m_Panel->SetControlValues(kFeatType, features);

    bool is_refseq = false;
    vector<string> explanation_strings = CSeq_feat::GetListOfLegalExceptions(is_refseq);
    m_Panel->SetControlValues(kExceptionType, explanation_strings);
}

bool CSetExceptionsTreeItemData::UpdateTarget()
{
    m_Target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), kEmptyStr);
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    // this will always be modified, otherwise the macro might contain constraints that are invalid
    return true;
}

string CSetExceptionsTreeItemData::GetMacroDescription() const
{
    string descr = "Set exception to '";
    descr += m_ArgList[kExceptionType].GetValue() + "' for " + m_ArgList[kFeatType].GetValue() + " features ";

    if (m_ArgList[kMoveExplanation].IsTrue()) {
        descr += ", copy exception to note";
    }

    string feat_product = m_ArgList[kWhereFeatProduct].GetValue();
    if (!NStr::EqualNocase(feat_product, "Either")) {
        feat_product[0] = tolower((unsigned char)feat_product[0]);
        descr += " where feature product is " + feat_product;
    }
    return descr;
}

string CSetExceptionsTreeItemData::GetVariables()
{
    return kExceptionType + " = %" + m_ArgList[kExceptionType].GetValue() + "%";
}

string CSetExceptionsTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kExceptionType].Empty())
        return kEmptyStr;

    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());

    string feat_product = m_ArgList[kWhereFeatProduct].GetValue();
    feat_product[0] = tolower((unsigned char)feat_product[0]);

    if (!NStr::EqualNocase(feat_product, "Either")) {
        string new_constraint = CMacroFunction_IsPresent::GetFuncName();
        new_constraint += "(\"product\")";

        if (NStr::EqualNocase(feat_product, "Absent")) {
            new_constraint = "NOT " + new_constraint;
        }
        constraints.emplace_back("product is " + feat_product, new_constraint);
    }

    string function;
    if (m_ArgList[kMoveExplanation].IsTrue()) {
        function += CMacroFunction_CopyStringQual::GetFuncName();
        function += "(\"except-text\", \"comment\", \"eAppend\", \";\");\n";
    }

    function += CMacroFunction_SetStringQual::GetFuncName();
    function += "(\"except-text\", " + kExceptionType + ", \"eReplace\");\n";
    
    function += CMacroFunction_SetQual::GetFuncName();
    function += "(\"except\", true);";
    return function;
}


// CConvertGapsTreeItemData
CConvertGapsTreeItemData::CConvertGapsTreeItemData()
    : CMacroActionItemData("Convert estimated length gaps to unknown length gaps by gap size")
{
}

void CConvertGapsTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args { GetOptionalArgs()[9] };
    x_LoadPanel(parent, "ConvertGapsBySize", args);
}

string CConvertGapsTreeItemData::GetMacroDescription() const
{
    return "Convert gaps of estimated length " + m_ArgList[kGapSizeCnv].GetValue() + " to unknown length gaps";
}

string CConvertGapsTreeItemData::GetVariables()
{
    return kGapSizeCnv + " = %" + m_ArgList[kGapSizeCnv].GetValue() + "%";
}

string CConvertGapsTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kGapSizeCnv].Empty())
        return kEmptyStr;

    return CMacroFunction_ToUnknownLengthGap::GetFuncName() + "(" + kGapSizeCnv + ");";
}

END_NCBI_SCOPE
