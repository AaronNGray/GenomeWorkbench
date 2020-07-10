/*  $Id: macro_edit_itemdata.cpp 44686 2020-02-20 20:27:01Z asztalos $
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
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_string_constr.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_edit_itemdata.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(NMacroArgs);


namespace {
    
    string GetFindReplDescr(const CArgumentList& args)
    {
        string find_text;
        for (auto& it : args[kFind].GetValue().get()) {
            if (it == '"') {
                find_text += "\\";
            }
            find_text += it;
        }

        string descr = " replace '" + find_text + "'";
        if (args[kCaseSens].IsTrue()) {
            descr += " (case insensitive) ";
        }
        else {
            descr += " (case sensitive) ";
        }
        descr += " with '" + args[kRepl].GetValue() + "'";
        return descr;
    }
}

// CEditQualTreeItemData
CEditQualTreeItemData::CEditQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CEditQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{   
    x_LoadPanel(parent, macro::CMacroFunction_EditStringQual::GetFuncName(), GetEditArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);
}

bool CEditQualTreeItemData::UpdateTarget()
{
    if (m_FieldType == EMacroFieldType::eMolinfo) {
        CTempString new_target = NMItemData::UpdateTargetForMolinfo(m_ArgList[kField].GetValue());
        bool modified = (new_target != m_Target);
        m_Target = new_target;
        return modified;
    }
    return false;
}

string CEditQualTreeItemData::GetMacroDescription() const
{
    string descr = "Edit ";
    if (m_FieldType == EMacroFieldType::ePubdesc) {
        descr += "publication ";
    }
    descr += m_ArgList[kField].GetValue();
    return descr + GetFindReplDescr(m_ArgList);
}

string CEditQualTreeItemData::GetVariables()
{
    string variables;
    variables += kFind + " = %" + m_ArgList[kFind].GetValue() + "%\n";
    variables += kRepl + " = %" + m_ArgList[kRepl].GetValue() + "%\n";
    variables += kLocation + " = \"" + m_ArgList[kLocation].GetValue() + "\"\n";
    variables += kCaseSens + " = ";
    if (m_ArgList[kCaseSens].IsTrue()) {
        variables += "%false%";
    }
    else {
        variables += "%true%";
    }
    variables += "\n" + kIsRegex + " = %" + m_ArgList[kIsRegex].GetValue() + "%";
    return variables;
}

string CEditQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    const string& field = m_ArgList[kField].GetValue();
    if (field.empty()) return function;

    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType, m_Target);
    if (macro::NMacroUtil::IsStructVoucherPart(field)) {
        string rt_var = "obj";
        bool found_constraint = false;
        function = NMItemData::GetResolveFuncForSVPart(field, rt_var, constraints, found_constraint);

        auto pos = field.find_last_of('-');
        function += macro::CMacroFunction_EditSubfield::GetFuncName();
        function += "(\"" + NMItemData::GetEditableFieldForPair(field.substr(0, pos), rt_var) + "\", \"" + field.substr(pos + 1) + "\", ";
    }
    else if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, true);

        if (function.empty()) {
            function += macro::CMacroFunction_EditStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }
        else {
            function += macro::CMacroFunction_EditStringQual::GetFuncName();
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

        function += macro::CMacroFunction_EditStringQual::GetFuncName();
        function += "(\"" + NMItemData::GetEditableFieldForPair(field, rt_var) + "\", ";
    }
    else if (m_FieldType == EMacroFieldType::ePubdesc) {
        string rt_var;
        function = NMItemData::GetResolveFuncForPubQual(field, rt_var, constraints);

        function += macro::CMacroFunction_EditStringQual::GetFuncName();
        function += "(" + rt_var + ", ";
    }
    else if (m_FieldType == EMacroFieldType::eDBLink) {
        string rt_var;
        bool found_constraint = false;
        function = NMItemData::GetResolveFuncForDBLink(m_ArgList[kField].GetValue(), rt_var, constraints, found_constraint);
        function += macro::CMacroFunction_EditStringQual::GetFuncName();
        if (found_constraint) {
            function += "(" + rt_var + ", ";
        }
        else {
            function += "(\"" + rt_var + ".data.strs\", ";
        }
    }
    else {
        function += macro::CMacroFunction_EditStringQual::GetFuncName();
        function += "(\"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType) + "\", ";
    }
    
    if (m_FieldType == EMacroFieldType::eMiscDescriptors) {
        NMItemData::UpdateConstraintsForMisc(constraints, m_ArgList[kField].GetValue(), m_Target);
    }

    function += kFind + ", " + kRepl + ", " + kLocation + ", " + kCaseSens + ", " + kIsRegex + ");";
    return function;
}

// CEditProteinQualTreeItemData
CEditProteinQualTreeItemData::CEditProteinQualTreeItemData()
    : CEditQualTreeItemData("Edit existing protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CEditProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetEditArgs());
    args.push_back(GetOptionalArgs()[2]);
    args.push_back(GetOptionalArgs()[6]);
    x_LoadPanel(parent, "EditStringProteinQual", args);
        
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

string CEditProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CEditQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    if (m_ArgList[kMoveToNote].GetEnabled() && m_ArgList[kMoveToNote].IsTrue()) {
        descr += ", copy original protein name to CDS note";
    }
    return descr;
}

string CEditProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;

    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    bool move_to_note = m_ArgList[kMoveToNote].GetEnabled() && m_ArgList[kMoveToNote].IsTrue();

    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), m_FieldType);
    if (NMItemData::MultipleValuesAllowed(path)) {
        if (move_to_note) {
            function = macro::CMacroFunction_CopyNameToCDSNote::GetFuncName();
            function += "();\n";
        }

        // place the constraint in the Do section
        string rt_var = "obj";
        string res_function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, !(update_mrna || move_to_note));

        if (res_function.empty()) {
            function += macro::CMacroFunction_EditStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }
        else {
            function += res_function;
            function += macro::CMacroFunction_EditStringQual::GetFuncName();
            function += "(" + rt_var + ", ";
        }

        if (move_to_note || update_mrna) {
            string new_constraint = macro::CMacroFunction_StringConstraints::sm_Contain;
            new_constraint += "(\"" + path + "\", " + kFind + ", " + kCaseSens + ")";
            constraints.emplace_back("\"" + path + "\"", new_constraint);
        }
    }
    else {
        function += macro::CMacroFunction_EditStringQual::GetFuncName();
        function += "(\"" + path + "\", ";
    }

    function += kFind + ", " + kRepl + ", " + kLocation + ", " + kCaseSens + ", " + kIsRegex + ");";
    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;

}

// CEditFeatQualTreeItemData
CEditFeatQualTreeItemData::CEditFeatQualTreeItemData()
    : CEditQualTreeItemData("Edit existing feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CEditFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetOptionalArgs().begin(), GetOptionalArgs().begin() + 3);
    copy(GetEditArgs().begin() + 1, GetEditArgs().end(), back_inserter(args));

    x_LoadPanel(parent, "EditStringQualFeature", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFeatQual, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

bool CEditFeatQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CEditFeatQualTreeItemData::GetMacroDescription() const
{
    string descr = "Edit " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kFeatQual].GetValue();
    descr += GetFindReplDescr(m_ArgList);
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CEditFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());

    string function;
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());
    if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, !update_mrna);

        if (update_mrna) {
            string new_constraint = macro::CMacroFunction_StringConstraints::sm_Contain;
            new_constraint += "(\"" + path + "\", " + kFind + ", " + kCaseSens + ")";
            constraints.emplace_back("\"" + path + "\"", new_constraint);
        }

        if (function.empty()) {
            function += macro::CMacroFunction_EditStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }
        else {
            function += macro::CMacroFunction_EditStringQual::GetFuncName();
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

            function += macro::CMacroFunction_EditStringQual::GetFuncName();
            function += "(" + rt_var + ", ";
        }
        else {
            if (macro::NMacroUtil::IsSatelliteSubfield(tokens[1])) {
                function = NMItemData::GetResolveFuncForQual(tokens[0], m_ArgList[kFeatQual].GetValue(), rt_var, constraints);

                function += macro::CMacroFunction_EditSubfield::GetFuncName();
                function += "(\"" + NMItemData::GetEditableFieldForPair("satellite", rt_var) + "\", \"" + m_ArgList[kFeatQual].GetValue() + "\", ";
            }
            else if (macro::NMacroUtil::IsMobileElementTSubfield(tokens[1])) {
                function = NMItemData::GetResolveFuncForQual(tokens[0], m_ArgList[kFeatQual].GetValue(), rt_var, constraints);

                function += macro::CMacroFunction_EditSubfield::GetFuncName();
                function += "(\"" + NMItemData::GetEditableFieldForPair(macro::kMobileElementTQual, rt_var) + "\", \"" + m_ArgList[kFeatQual].GetValue() + "\", ";
            } 
            else {
                function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

                function += macro::CMacroFunction_EditStringQual::GetFuncName();
                function += "(\"" + NMItemData::GetEditableFieldForPair(m_ArgList[kFeatQual].GetValue(), rt_var) + "\", ";
            }
        }
    }
    else {
        function += macro::CMacroFunction_EditStringQual::GetFuncName();
        function += "(\"" + path + "\", ";
    }

    function += kFind + ", " + kRepl + ", " + kLocation + ", " + kCaseSens + ", " + kIsRegex + ");";
    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}

// CEditRNAQualTreeItemData
CEditRNAQualTreeItemData::CEditRNAQualTreeItemData()
    : CEditQualTreeItemData("Edit existing RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CEditRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetEditArgs());
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "EditStringRNAQual", args);

    vector<string> rnas = CMacroEditorContext::GetInstance().GetRNATypes();
    m_Panel->SetControlValues(kRNAType, rnas);
    m_Panel->SetSelection(kRNAType, 0);
    vector<string> class_types = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
    m_Panel->SetControlValues(kncRNAClass, class_types);
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
}

bool CEditRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForRnas(m_ArgList[kRNAType].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CEditRNAQualTreeItemData::GetMacroDescription() const
{
    string descr = "Edit";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kField].GetValue();
    descr += GetFindReplDescr(m_ArgList);
    return descr;
}

string CEditRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForRnas(constraints, m_ArgList);

    string function;
    if (NStr::StartsWith(m_ArgList[kField].GetValue().get(), "gene")) {
        string gene_field = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), EMacroFieldType::eGene);
        NMItemData::WrapInQuotes(gene_field);
        function = macro::CMacroFunction_EditRelFeatQual::sm_FunctionName;
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

            function += macro::CMacroFunction_EditStringQual::GetFuncName();
            function += "(\"" + NMItemData::GetEditableFieldForPair(tokens[1], rt_var) + "\", ";
        }
        else {
            function += macro::CMacroFunction_EditStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }
    }

    function += kFind + ", " + kRepl + ", " + kLocation + ", " + kCaseSens + ", " + kIsRegex + ");";
    return function;
}

// CEditStrCommQualTreeItemData
CEditStrCommQualTreeItemData::CEditStrCommQualTreeItemData()
    : CEditQualTreeItemData("Edit existing structured comment fields", EMacroFieldType::eStructComment)
{
}

void CEditStrCommQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetEditArgs());
    args.push_back(GetOptionalArgs()[5]);
    x_LoadPanel(parent, "EditStringStrCommQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);
    m_Panel->SetSelection(kField, 0);

    m_Panel->GetArgumentList().Attach(NMItemData::OnStructCommFieldChanged);
}

string CEditStrCommQualTreeItemData::GetMacroDescription() const
{
    string descr = "Edit " + NMItemData::GetStructCommQualDescr(m_ArgList);
    return descr + GetFindReplDescr(m_ArgList);
}

string CEditStrCommQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string field_name = (m_ArgList[kStrCommField].GetShown()) ? m_ArgList[kStrCommField].GetValue() : kEmptyStr;
    string rt_var = "obj";
    string function = NMItemData::GetResolveFuncForStrCommQual(m_ArgList[kField].GetValue(), field_name, rt_var, constraints) + "\n";


    function += macro::CMacroFunction_EditStringQual::GetFuncName();
    if (NStr::EqualNocase(m_ArgList[kField].GetValue().get(), "Field name")) {
        function += "(\"" + rt_var + ".label.str\", ";
    }
    else {
        function += "(\"" + rt_var + ".data.str\", ";
    }

    function += kFind + ", " + kRepl + ", " + kLocation + ", " + kCaseSens + ", " + kIsRegex + ");";
    return function;
}


namespace {

    string GetDescrForSeqType(const string& apply_seq)
    {
        string descr;
        if (NStr::EqualNocase(apply_seq, "any sequence")) {
            descr = "any";
        }
        else {
            descr = apply_seq;
            NStr::ToLower(descr);
        }

        descr = " where sequence type is " + descr;
        return descr;
    }
}

CEditMolinfoFieldsTreeItemData::CEditMolinfoFieldsTreeItemData()
    : CMacroActionItemData("Edit multiple molinfo fields at once", EMacroFieldType::eMolinfo)
{
}

void CEditMolinfoFieldsTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, macro::CMacroFunction_SetQual::GetFuncName(), GetEditMolinfoArgs());

    vector<EMacroFieldType> types
        { EMacroFieldType::eMolinfoMolecule
        , EMacroFieldType::eMolinfoTech
        , EMacroFieldType::eMolinfoCompleteness
        , EMacroFieldType::eMolinfoClass
        , EMacroFieldType::eMolinfoTopology
        , EMacroFieldType::eMolinfoStrand
        };
    
    vector<string> fieldnames;
    size_t index = 0;
    for (auto& it : types) {
        fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(it);
        
        fieldnames.insert(fieldnames.begin(), "Any");
        m_Panel->SetControlValues(m_Args[index]->GetName(), fieldnames);
        m_Panel->SetSelection(m_Args[index]->GetName(), 0);
        fieldnames[0] = "No change";
        m_Panel->SetControlValues(m_Args[++index]->GetName(), fieldnames);
        m_Panel->SetSelection(m_Args[index]->GetName(), 0);
        ++index;
    }
}

bool CEditMolinfoFieldsTreeItemData::UpdateTarget()
{
    CTempString new_target = macro::CMacroBioData::sm_SeqNa;
    const string& target = m_ArgList[kApplySeq].GetValue();
    if (NStr::EqualNocase(target, "Any sequence")) {
        new_target = macro::CMacroBioData::sm_Seq;
    }
    else if (NStr::EqualNocase(target, "proteins")) {
        new_target = macro::CMacroBioData::sm_SeqAa;
    }

    bool modified = (new_target != m_Target);
    m_Target = new_target;
    return modified;
}

string CEditMolinfoFieldsTreeItemData::GetMacroDescription() const
{
    string descr;
    x_GetDescrChangeTo("molecule", kMolFrom, kMolTo, descr);
    x_GetDescrChangeTo("technique", kTechFrom, kTechTo, descr);
    x_GetDescrChangeTo("completedness", kComplFrom, kComplTo, descr);
    x_GetDescrChangeTo("class", kClassFrom, kClassTo, descr);
    x_GetDescrChangeTo("topology", kTopFrom, kTopTo, descr);
    x_GetDescrChangeTo("strand", kStrandFrom, kStrandTo, descr);

    if (!descr.empty()) {
        x_GetDescrChangeFrom("molecule", kMolFrom, kMolTo, descr);
        x_GetDescrChangeFrom("technique", kTechFrom, kTechTo, descr);
        x_GetDescrChangeFrom("completedness", kComplFrom, kComplTo, descr);
        x_GetDescrChangeFrom("class", kClassFrom, kClassTo, descr);
        x_GetDescrChangeFrom("topology", kTopFrom, kTopTo, descr);
        x_GetDescrChangeFrom("strand", kStrandFrom, kStrandTo, descr);
    }
    descr += GetDescrForSeqType(m_ArgList[kApplySeq].GetValue());
    descr[0] = toupper((unsigned char)descr[0]);
    return descr;
}

void CEditMolinfoFieldsTreeItemData::x_GetDescrChangeTo(const string& field, const CTempString& from, const CTempString& to, string& change_to) const
{
    if (m_ArgList[to].GetValue() == "No change")
        return;
    if (m_ArgList[from].Empty() && m_ArgList[to].Empty())
        return;

    string to_value = m_ArgList[to].GetValue();
    if (to_value == " ") {
        to_value = "unknown/not-set";
    }
    if (!change_to.empty()) {
        change_to += " and ";
    }
    if (change_to.find("change") == NPOS) {
        change_to += "change ";
    }
    change_to += "to " + field + " " + to_value;
}

void CEditMolinfoFieldsTreeItemData::x_GetDescrChangeFrom(const string& field, const CTempString& from, const CTempString& to, string& change_from) const
{
    if (m_ArgList[to].GetValue() == "No change")
        return;
    if (m_ArgList[from].Empty() && m_ArgList[to].Empty())
        return;

    if (m_ArgList[from].GetValue() != "Any") {
        string from_value = m_ArgList[from].GetValue();
        if (from_value == " ") {
            from_value = "unknown/not-set";
        }
        if (change_from.find("where") != NPOS) {
            change_from += " and ";
        }
        else {
            change_from += " where ";
        }
        change_from += field + " is " + from_value;
    }
}

// It will always add a molinfo descriptor if there is not one already (Sequin)
string CEditMolinfoFieldsTreeItemData::GetFunction(TConstraints& constraints) const
{
    const string& target = m_ArgList[kApplySeq].GetValue();
    string new_constraint;
    if (NStr::EqualNocase(target, "DNA")) {
        new_constraint = macro::CMacroFunction_StringConstraints::sm_Equal;
        new_constraint += "(\"inst.mol\", \"dna\")";
    }
    else if (NStr::EqualNocase(target, "RNA")) {
        new_constraint = macro::CMacroFunction_StringConstraints::sm_Equal;
        new_constraint += "(\"inst.mol\", \"rna\")";
    }
    if (!new_constraint.empty()) {
        constraints.insert(constraints.begin(), { kEmptyStr, new_constraint });
    }

    string function = x_GetFunctionFor("molecule", kMolFrom, kMolTo, constraints);
    function += x_GetFunctionFor("technique", kTechFrom, kTechTo, constraints);
    function += x_GetFunctionFor("completedness", kComplFrom, kComplTo, constraints);
    function += x_GetFunctionFor("class", kClassFrom, kClassTo, constraints);
    function += x_GetFunctionFor("topology", kTopFrom, kTopTo, constraints);
    function += x_GetFunctionFor("strand", kStrandFrom, kStrandTo, constraints);
    if (!function.empty()) function.pop_back();
    return function;
}

string CEditMolinfoFieldsTreeItemData::x_GetFunctionFor(const string& field, const CTempString& from, const CTempString& to, TConstraints& constraints) const
{
    if (m_ArgList[to].GetValue() == "No change")
        return kEmptyStr;
    if (m_ArgList[from].Empty() && m_ArgList[to].Empty())
        return kEmptyStr;

    string function = macro::CMacroFunction_SetQual::GetFuncName();
    function += "(\"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType, m_Target) + "\", ";
    function += "\"" + CMacroEditorContext::GetInstance().GetAsnMolinfoValue(field, m_ArgList[to].GetValue()) + "\");";
    if (m_ArgList[from].GetValue() != "Any") {
        string new_constraint = macro::CMacroFunction_StringConstraints::sm_Equal;
        new_constraint += "(\"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, m_FieldType, m_Target) + "\", ";
        new_constraint += "\"" + CMacroEditorContext::GetInstance().GetAsnMolinfoValue(field, m_ArgList[from].GetValue()) + "\")";
        bool found = false;
        if (field == "class") {
            for (auto& it : constraints) {
                if (NStr::EqualNocase(it.second, new_constraint)) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            constraints.insert(constraints.begin(), { kEmptyStr, new_constraint });
        }
    }
    function += "\n";
    return function;
}


END_NCBI_SCOPE
