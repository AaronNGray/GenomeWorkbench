/*  $Id: macro_rmv_itemdata.cpp 44820 2020-03-23 14:59:16Z asztalos $
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
#include <gui/objutils/macro_fn_do.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>

#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_rmv_itemdata.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(macro);
USING_SCOPE(NMacroArgs);

// CRmvQualTreeItemData
CRmvQualTreeItemData::CRmvQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CRmvQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_RemoveQual::GetFuncName(), { GetSetArgs()[0] } );

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);
}

bool CRmvQualTreeItemData::UpdateTarget()
{
    if (m_FieldType == EMacroFieldType::eMolinfo) {
        CTempString new_target = NMItemData::UpdateTargetForMolinfo(m_ArgList[kField].GetValue());
        bool modified = (new_target != m_Target);
        m_Target = new_target;
        return modified;
    }
    return false;
}

string CRmvQualTreeItemData::GetMacroDescription() const
{
    string descr = "Remove ";
    if (m_FieldType == EMacroFieldType::ePubdesc) {
        descr += "publication ";
    }
    descr += m_ArgList[kField].GetValue();
    return descr;
}

string CRmvQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string field = m_ArgList[kField].GetValue();

    if (field.empty()) return function;
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType, m_Target);

    if (m_FieldType == EMacroFieldType::ePubdesc) {
        if (field == "author name list") {
            function = CMacroFunction_RemovePubAuthors::GetFuncName() + "();";
        }
        else if (field == "author middle initials") {
            string rt_var = "obj";
            function = NMItemData::GetResolveFuncForApplyAuthors(CPubFieldType::ePubFieldType_AuthorMiddleInitial, rt_var, constraints);

            function += CMacroFunction_RemovePubAuthorMI::GetFuncName();
            function += "(" + rt_var + ");";
        } else {
            string rt_var;
            function = NMItemData::GetResolveFuncForPubQual(field, rt_var, constraints);

            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + rt_var + ");";
        }
    }
    else if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, true);

        if (function.empty()) {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(\"" + path + "\");";
        }
        else {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + rt_var + ");";
        }
    }
    else if (m_FieldType == EMacroFieldType::eDBLink) {
        string rt_var;
        bool found_constraint = false;
        function = NMItemData::GetResolveFuncForDBLink(m_ArgList[kField].GetValue(), rt_var, constraints, found_constraint);
        function += CMacroFunction_RemoveQual::GetFuncName();
        function += "(" + rt_var + ");";
    }
    else {
        function = CMacroFunction_RemoveQual::GetFuncName();
        function += "(\"" + path + "\");";
    }
    return function;
}


// CRmvMiscQualTreeItemData
CRmvMiscQualTreeItemData::CRmvMiscQualTreeItemData()
    : CRmvQualTreeItemData("Remove selected descriptors", EMacroFieldType::eMiscDescriptorsToRmv)
{
}

bool CRmvMiscQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForMisc(m_ArgList[kField].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    return modified;
}

string CRmvMiscQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string field = m_ArgList[kField].GetValue();

    if (field.empty()) return function;
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType, m_Target);

    NMItemData::UpdateConstraintsForMisc(constraints, field, m_Target);
    if (NMItemData::MultipleValuesAllowed(path)) {
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, true);

        if (function.empty()) {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(\"" + path + "\");";
        }
        else {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + rt_var + ");";
        }
    }
    else {
        function = CMacroFunction_RemoveDescriptor::GetFuncName() + "();";
    }
    return function;
}

// CRmvBsrcQualTreeItemData
CRmvBsrcQualTreeItemData::CRmvBsrcQualTreeItemData()
    : CMacroActionItemData("Remove biosource qualifiers", EMacroFieldType::eBiosourceText)
{
}

const vector<string>& CRmvBsrcQualTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetBsrcKeywords();
}

void CRmvBsrcQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetBsrcArgs());
    args.push_back(GetSetArgs()[0]);
    x_LoadPanel(parent, CMacroFunction_RemoveModifier::GetFuncName(), args);
    
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceTextAll);
    m_Panel->SetControlValues(kField, fieldnames);
}

string CRmvBsrcQualTreeItemData::GetMacroDescription() const
{
    string descr = "Remove ";
    m_FieldType = NMItemData::GetSourceTypeInSinglePanel(m_ArgList);
    switch (m_FieldType) {
    case EMacroFieldType::eBiosourceLocation:
        descr += "location";
        break;
    case EMacroFieldType::eBiosourceOrigin:
        descr += "origin";
        break;
    case EMacroFieldType::eBiosourceText:
    case EMacroFieldType::eBiosourceTax:
        descr += m_ArgList[kField].GetValue();
        break;
    default:
        break;
    }
    return descr;
}

string CRmvBsrcQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string field_name = m_ArgList[kField].GetValue();

    m_FieldType = NMItemData::GetSourceTypeInSinglePanel(m_ArgList);
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field_name, m_FieldType);
    if (field_name == "dbxref") {
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForDbXref(path, rt_var, constraints);
        if (function.empty()) {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(\"" + path + "\");";
        }
        else {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + rt_var + ");";
        }
    }
    else if (NMacroUtil::IsStructVoucherPart(field_name)) {
        string rt_var = "obj";
        bool found_constraint = false;
        function = NMItemData::GetResolveFuncForSVPart(field_name, rt_var, constraints, found_constraint);

        auto pos = field_name.find_last_of('-');
        function += CMacroFunction_RemoveSubfield::GetFuncName();
        function += "(" + rt_var + ", \"" + field_name.substr(pos + 1) + "\");";
    }
    else {
        if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            // it is either an orgmod or a subsource modifier
            string rt_var = "obj_bsrc";
            function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

            function += CMacroFunction_RemoveModifier::GetFuncName();
            function += "(" + rt_var + ");";
        }
        else {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(\"" + path + "\");";
        }
    }

    return function;
}

// CRmvStrCommQualTreeItemData
CRmvStrCommQualTreeItemData::CRmvStrCommQualTreeItemData()
    : CMacroActionItemData("Remove structured comment fields", EMacroFieldType::eStructComment)
{
}

void CRmvStrCommQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args{ GetSetArgs()[0] };
    args.push_back(GetOptionalArgs()[5]);
    x_LoadPanel(parent, "RemoveStrCommQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);
    m_Panel->SetSelection(kField, 0);

    m_Panel->GetArgumentList().Attach(NMItemData::OnStructCommFieldChanged);
}

string CRmvStrCommQualTreeItemData::GetMacroDescription() const
{
    return "Remove " + NMItemData::GetStructCommQualDescr(m_ArgList);
}

string CRmvStrCommQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string field_name = (m_ArgList[kStrCommField].GetShown()) ? m_ArgList[kStrCommField].GetValue() : kEmptyStr;
    string rt_var = "obj";
    string function = NMItemData::GetResolveFuncForStrCommQual(m_ArgList[kField].GetValue(), field_name, rt_var, constraints) + "\n";
    function += CMacroFunction_RemoveQual::GetFuncName();
    function += "(" + rt_var + ");";
    return function;
}


// CRmvProteinQualTreeItemData
CRmvProteinQualTreeItemData::CRmvProteinQualTreeItemData()
    : CRmvQualTreeItemData("Remove protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CRmvProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args{ GetSetArgs()[0] };
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "RemoveProteinQual", args);
    
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

string CRmvProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CRmvQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CRmvProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();

    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), m_FieldType);
    if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, !update_mrna);

        if (function.empty()) {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(\"" + path + "\");";
        }
        else {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + rt_var + ");";
        }
    }
    else {
        function = CMacroFunction_RemoveQual::GetFuncName();
        function += "(\"" + path + "\");";
    }

    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}

// CRmvFeatQualTreeItemData
CRmvFeatQualTreeItemData::CRmvFeatQualTreeItemData()
    : CRmvQualTreeItemData("Remove feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CRmvFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetOptionalArgs().begin(), GetOptionalArgs().begin() + 3);
    x_LoadPanel(parent, "RemoveFeatureQual", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFeatQual, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

bool CRmvFeatQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CRmvFeatQualTreeItemData::GetMacroDescription() const
{
    string descr = "Remove " + m_ArgList[kFeatType].GetValue().get() + " " + m_ArgList[kFeatQual].GetValue().get();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CRmvFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());

    string function;
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());

    string rt_var = "obj";
    if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, !update_mrna);

        if (function.empty()) {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(\"" + path + "\");";
        }
        else {
            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + rt_var + ");";
        }
    }
    else if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);
        _ASSERT(tokens.size() == 2);

        if (!NStr::EqualNocase(m_ArgList[kFeatType].GetValue().get(), "gene") && NMItemData::IsGeneQualifier(tokens[1])) {
            function = NMItemData::GetResolveFuncForGeneQuals(tokens[0], tokens[1], rt_var, constraints);
            function += CMacroFunction_RemoveQual::GetFuncName() + "(" + rt_var + ");";
        }
        else if (NMacroUtil::IsSatelliteSubfield(tokens[1]) || NMacroUtil::IsMobileElementTSubfield(tokens[1])) {
            function = NMItemData::GetResolveFuncForQual(tokens[0], m_ArgList[kFeatQual].GetValue(), rt_var, constraints);

            function += CMacroFunction_RemoveSubfield::GetFuncName();
            function += "(" + rt_var + ", \"" + m_ArgList[kFeatQual].GetValue() + "\");";
        }
        else {
            function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);
            function += CMacroFunction_RemoveQual::GetFuncName() + "(" + rt_var + ");";
        }
    }
    else if (path == "dbxref") {
        function = NMItemData::GetResolveFuncForDbXref(path, rt_var, constraints);
        if (!function.empty()) {
            function += CMacroFunction_RemoveQual::GetFuncName() + "(" + rt_var + ");";
        }
        else {
            function += CMacroFunction_RemoveQual::GetFuncName() + "(\"" + path + "\");";
        }
    }
    else {
        function = CMacroFunction_RemoveQual::GetFuncName() + "(\"" + path + "\");";
    }

    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}

// CRmvRNAQualTreeItemData
CRmvRNAQualTreeItemData::CRmvRNAQualTreeItemData()
    : CRmvQualTreeItemData("Remove RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CRmvRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args{ GetSetArgs()[0] };
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "RemoveRNAQual", args);

    vector<string> rnas = CMacroEditorContext::GetInstance().GetRNATypes();
    m_Panel->SetControlValues(kRNAType, rnas);
    m_Panel->SetSelection(kRNAType, 0);
    vector<string> class_types = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
    m_Panel->SetControlValues(kncRNAClass, class_types);
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
}

bool CRmvRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForRnas(m_ArgList[kRNAType].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CRmvRNAQualTreeItemData::GetMacroDescription() const
{
    string descr = "Remove";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kField].GetValue(); 
    return descr;
}

string CRmvRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForRnas(constraints, m_ArgList);

    string function;
    if (NStr::StartsWith(m_ArgList[kField].GetValue().get(), "gene")) {
        string gene_field = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), EMacroFieldType::eGene);
        NMItemData::WrapInQuotes(gene_field);
        function = CMacroFunction_RmvRelFeatQual::GetFuncName();
        function += "(\"gene\", " + gene_field + ");";
    }
    else {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kField].GetValue(), m_FieldType);
        if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            string rt_var = "obj";
            function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

            function += CMacroFunction_RemoveQual::GetFuncName();
            function += "(" + rt_var + ");";
        }
        else if (path.find("::") != NPOS) {
            function = CMacroFunction_RemoveRnaProduct::GetFuncName() + "();";
        }
        else {
            function = CMacroFunction_RemoveQual::GetFuncName();
            function += "(\"" + path + "\");";
        }
    }

    return function;
}
    

// CRemoveDescrTreeItemData
CRemoveDescrTreeItemData::CRemoveDescrTreeItemData()
    : CMacroActionItemData("Remove descriptors", EMacroFieldType::eDescriptors)
{
}

void CRemoveDescrTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_RemoveDescriptor::GetFuncName(),
        CMacroFunction_RemoveDescriptor::sm_Arguments);
    
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(m_Args[0]->GetName(), fieldnames);
}

bool CRemoveDescrTreeItemData::UpdateTarget()
{
    // select FOR EACH target based on descriptor type:
    const string& descriptor = m_Args[0]->GetValue();
    CSeqdesc::E_Choice type = CMacroEditorContext::GetInstance().GetDescriptorType(descriptor);
    CTempString new_target = CMacroBioData::sm_Seqdesc;
    switch (type) {
    case CSeqdesc::e_Source:
        new_target = CMacroBioData::sm_BioSource;
        break;
    case CSeqdesc::e_Molinfo:
        new_target = CMacroBioData::sm_MolInfo;
        break;
    case CSeqdesc::e_Pub:
        new_target = CMacroBioData::sm_Pubdesc;
        break;
    case CSeqdesc::e_User:
        if (NStr::EqualNocase(descriptor, "StructuredComment")) {
            new_target = CMacroBioData::sm_StrComm;
        }
        else if (NStr::EqualNocase(descriptor, "DBLink")) {
            new_target = CMacroBioData::sm_DBLink;
        }
        else {
            new_target = CMacroBioData::sm_UserObject;
        }
    default:
        break;
    }

    bool modified = (m_Target != new_target);
    m_Target = new_target;
    return modified;
}

string CRemoveDescrTreeItemData::GetMacroDescription() const
{
    return "Remove " + m_Args[0]->GetValue() + " descriptor";
}

string CRemoveDescrTreeItemData::GetFunction(TConstraints& constraints) const
{
    CSeqdesc::E_Choice type = CMacroEditorContext::GetInstance().GetDescriptorType(m_Args[0]->GetValue());
    switch (type) {
    case CSeqdesc::e_Name:
    case CSeqdesc::e_Title:
    case CSeqdesc::e_Comment:
    case CSeqdesc::e_Num:
    case CSeqdesc::e_Maploc:
    case CSeqdesc::e_Pir:
    case CSeqdesc::e_Genbank:
    case CSeqdesc::e_Region:
    case CSeqdesc::e_Sp:
    case CSeqdesc::e_Dbxref:
    case CSeqdesc::e_Embl:
    case CSeqdesc::e_Create_date:
    case CSeqdesc::e_Update_date:
    case CSeqdesc::e_Prf:
    case CSeqdesc::e_Pdb:
    case CSeqdesc::e_Het:
    {
        string new_constraint = CMacroFunction_ChoiceType::GetFuncName();
        new_constraint += "() = \"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_Args[0]->GetValue(), m_FieldType) + "\"";
        constraints.insert(constraints.begin(), make_pair("", new_constraint));
        break;
    }
    default:
        break;
    }

    return CMacroFunction_RemoveDescriptor::GetFuncName() + "();";
}

// CRemoveFeatureTreeItemData
CRemoveFeatureTreeItemData::CRemoveFeatureTreeItemData()
    : CMacroActionItemData("Remove features", EMacroFieldType::eFeatQualifiers)
{
}

void CRemoveFeatureTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args{ GetOptionalArgs()[0], GetOptionalArgs()[8] };
    x_LoadPanel(parent, CMacroFunction_RemoveFeature::GetFuncName(), args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(true);
    m_Panel->SetControlValues(kFeatType, features);
    m_Panel->GetArgumentList().Attach(NMItemData::OnSelectFeature);
}

bool CRemoveFeatureTreeItemData::UpdateTarget()
{
    m_Target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), kEmptyStr, true);
    // this will always be modified, otherwise the macro might contain constraints that are invalid
    return true;
}

string CRemoveFeatureTreeItemData::GetMacroDescription() const
{
    string descr = "Remove " + m_ArgList[kFeatType].GetValue() + " features";
    if (HasVariables() && m_ArgList[kRmvOverlapGene].IsTrue()) {
        descr += " and the overlapping genes";
    }
    return descr;
}

bool CRemoveFeatureTreeItemData::HasVariables() const
{
    return (m_ArgList[kRmvOverlapGene].GetEnabled());
}

string CRemoveFeatureTreeItemData::GetVariables()
{
    if (HasVariables())
        return kRmvOverlapGene + " = %" + m_ArgList[kRmvOverlapGene].GetValue() + "%";

    return kEmptyStr;
}

string CRemoveFeatureTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kFeatType].GetValue() == "All") {
        return CTempString(CMacroFunction_RemoveAllFeatures::GetFuncName()) + "();";
    }

    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());
    string func = CMacroFunction_RemoveFeature::GetFuncName() + "(";
    if (HasVariables()) {
        func += kRmvOverlapGene;
    }
    func += ");";
    return func;
}


// CRemoveDuplFeatsTreeItemData 
CRemoveDuplFeatsTreeItemData::CRemoveDuplFeatsTreeItemData()
    : CMacroActionItemData("Remove duplicate features", EMacroFieldType::eFeatQualifiers)
{
}

void CRemoveDuplFeatsTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_RemoveDuplFeatures::GetFuncName(), GetRmvDuplFeaturesArgs());
    
    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(true);
    m_Panel->SetControlValues(kFeatType, features);
}

string CRemoveDuplFeatsTreeItemData::GetMacroDescription() const
{
    string descr = "Remove duplicate ";
    if (m_ArgList[kFeatType].GetValue() != "All") {
        descr += m_ArgList[kFeatType].GetValue();
    }
    descr += " features";

    if (m_ArgList[kCheckPartials].IsTrue()) {
        descr += " (check partials)";
    }
    if (m_ArgList[kCaseSens].IsTrue()) {
        if (descr.find("partials") != NPOS) {
            descr += ",";
        }
        descr += " (case sensitive)";
    }
    if (m_ArgList[kRemoveProteins].IsTrue()) {
        descr += " and remove protein products";
    }
    return descr;
}

string CRemoveDuplFeatsTreeItemData::GetVariables()
{
    string variables;
    variables += kCheckPartials + " = %" + m_ArgList[kCheckPartials].GetValue() + "%\n";
    variables += kCaseSens + " = %" + m_ArgList[kCaseSens].GetValue() + "%\n";
    variables += kRemoveProteins + " = %" + m_ArgList[kRemoveProteins].GetValue() + "%";
    return variables;
}

string CRemoveDuplFeatsTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kFeatType].GetValue().get().empty())
        return kEmptyStr;

    string function = CMacroFunction_RemoveDuplFeatures::GetFuncName();
    function += "(\"" + m_ArgList[kFeatType].GetValue() + "\",";
    function += kCheckPartials + ", " + kCaseSens + ", " + kRemoveProteins + ");";
    return function;
}


// CRemoveDbXrefTreeItemData
CRemoveDbXrefTreeItemData::CRemoveDbXrefTreeItemData()
    : CMacroActionItemData("Remove all, illegal dbxrefs from features", EMacroFieldType::eFeatQualifiers)
{
}

void CRemoveDbXrefTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, "RemoveDbXref", GetRmvDbxrefArgs());

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(true);
    m_Panel->SetControlValues(kFeatType, features);

    m_Panel->GetArgumentList().Attach(NMItemData::OnIllegalDbXrefSelected);
}

bool CRemoveDbXrefTreeItemData::UpdateTarget()
{
    m_Target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), kEmptyStr, false);
    // this will always be modified, otherwise the macro might contain constraints that are invalid
    return true;
}

string CRemoveDbXrefTreeItemData::GetMacroDescription() const
{
    string descr;
    
    if (m_ArgList[kAllDbxref].IsTrue()) {
        descr = "Remove all ";
    }
    else {
        descr = "Remove illegal ";
    }
    
    descr += "dbxrefs from " + m_ArgList[kFeatType].GetValue() + " features";
    if (m_ArgList[kMoveToNote].GetEnabled() &&  m_ArgList[kMoveToNote].IsTrue()) {
        descr += " and copy their values to note";
    }
    return descr;
}

string CRemoveDbXrefTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());

    if (m_ArgList[kAllDbxref].IsTrue()) {
        string function = CMacroFunction_RemoveQual::GetFuncName() + "(\"dbxref\");";
        return function;
    }

    string path = "dbxref", rt_var = "obj";
    string function = rt_var + " = " + CMacroFunction_Resolve::GetFuncName() + "(\"" + path + "\")";
    function += " WHERE " + CMacroFunction_IllegalDbXref::GetFuncName() + "(" + rt_var + ")";

    // check if there are any relevant constraints
    TConstraints::iterator it = constraints.begin();
    while (it != constraints.end()) {
        if (it->second.find(path) != NPOS) {
            NStr::ReplaceInPlace(it->second, path, rt_var + ".db");
            function += " AND " + it->second;
            it = constraints.erase(it);
        }
        else {
            ++it;
        }
    }
    function += ";\n";
    if (m_ArgList[kMoveToNote].GetEnabled() && m_ArgList[kMoveToNote].IsTrue()) {
        function += CMacroFunction_CopyStringQual::GetFuncName();
        function += "(" + rt_var + ", \"comment\", \"eAppend\", \";\");\n";
    }

    function += CMacroFunction_RemoveQual::GetFuncName() + "(" + rt_var + ");";
    return function;
}


// CRmvOrgNameTreeItemData
CRmvOrgNameTreeItemData::CRmvOrgNameTreeItemData()
    : CMacroActionItemData("Remove organism name from biosource modifiers", EMacroFieldType::eBiosourceText)
{
}

void CRmvOrgNameTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_RemoveOrgName::GetFuncName(),
        CMacroFunction_RemoveOrgName::sm_Arguments);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(m_Args[0]->GetName(), fieldnames);
}

string CRmvOrgNameTreeItemData::GetMacroDescription() const
{
    return "Remove organism name from " + m_Args[0]->GetValue();
}

string CRmvOrgNameTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_Args[0]->GetValue(), m_FieldType);
    if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

        function += CMacroFunction_RemoveOrgName::GetFuncName();
        function += "(\"" + NMItemData::GetEditableFieldForPair(m_Args[0]->GetValue(), rt_var) + "\");";
    }
    return function;
}


END_NCBI_SCOPE
