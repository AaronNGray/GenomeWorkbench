/*  $Id: macro_apply_itemdata.cpp 44990 2020-05-04 17:26:18Z asztalos $
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
#include <objects/valid/Comment_rule.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/objutils/macro_fn_seq_constr.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include "structuredcomment_panel.hpp"
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_authors_panel.hpp>
#include <gui/widgets/edit/macro_apply_itemdata.hpp>
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

// CApplyQualTreeItemData
CApplyQualTreeItemData::CApplyQualTreeItemData(const string& description, EMacroFieldType type)
    : CMacroActionItemData(description, type)
{
}

void CApplyQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_SetStringQual::GetFuncName(), GetSetArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CApplyQualTreeItemData::GetMacroDescription() const
{
    string descr = "Apply '" + m_ArgList[kNewValue].GetValue() + "' to ";
    if (m_FieldType == EMacroFieldType::ePubdesc) {
        descr += "publication ";
    }
    descr += m_ArgList[kField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CApplyQualTreeItemData::GetVariables()
{
    string variables;
    variables += kNewValue + " = %" + m_ArgList[kNewValue].GetValue() + "%\n";
    variables += GetExistingTextVariables(m_ArgList);
    return variables;
}

string CApplyQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_ArgList[kField].Empty()) return kEmptyStr;

    string function = CMacroFunction_SetStringQual::GetFuncName();
    function += "(\"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), m_FieldType, m_Target) + "\", ";
    function += kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";
    return function;
}

// CApplyProteinQualTreeItemData
CApplyProteinQualTreeItemData::CApplyProteinQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to protein (CDS product) fields", EMacroFieldType::eProtein)
{
}

void CApplyProteinQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSetArgs());
    args.push_back(GetOptionalArgs()[2]);
    x_LoadPanel(parent, "SetStringProteinQual", args);
    
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

string CApplyProteinQualTreeItemData::GetMacroDescription() const
{
    string descr = CApplyQualTreeItemData::GetMacroDescription();
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CApplyProteinQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;

    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kField].GetValue(), m_FieldType);
    if (NMItemData::MultipleValuesAllowed(path)) {
        // place the constraint in the Do section
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, !update_mrna);

        if (function.empty()) {
            function += CMacroFunction_SetStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }
        else {
            function += CMacroFunction_SetStringQual::GetFuncName();
            function += "(" + rt_var + ", ";
        }
    }
    else {
        function += CMacroFunction_SetStringQual::GetFuncName();
        function += "(\"" + path + "\", ";
    }

    function += kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}

// CApplyBsrcQualTreeItemData
CApplyBsrcQualTreeItemData::CApplyBsrcQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to biosource qualifiers", EMacroFieldType::eBiosourceTextAll)
{
}

const vector<string>& CApplyBsrcQualTreeItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetBsrcKeywords();
}

void CApplyBsrcQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSetArgs());
    copy(GetBsrcArgs().begin(), GetBsrcArgs().end(), back_inserter(args));
    x_LoadPanel(parent, CMacroFunction_AddorSetBsrcModifier::GetFuncName(), args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eBiosourceTextAll);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CApplyBsrcQualTreeItemData::GetMacroDescription() const
{
    string descr;
    descr = "Apply ";
    
    m_FieldType = NMItemData::GetSourceTypeInSinglePanel(m_ArgList);
    switch (m_FieldType) {
    case EMacroFieldType::eBiosourceLocation:
        descr += "location '" + m_ArgList[kNewValue].GetValue() + "'";
        break;
    case EMacroFieldType::eBiosourceOrigin:
        descr += "origin '" + m_ArgList[kNewValue].GetValue() + "'";
        break;
    case EMacroFieldType::eBiosourceText:
    case EMacroFieldType::eBiosourceTax:
        descr += "'" + m_ArgList[kNewValue].GetValue() + "' to " + m_ArgList[kField].GetValue();
        break;
    default:
        LOG_POST(Error << "Wrong biosource type selection");
        return kEmptyStr;
    }
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CApplyBsrcQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_FieldType == EMacroFieldType::eNotSet) {
        return kEmptyStr;
    }

    string field_name = m_ArgList[kField].GetValue();
    string function;
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field_name, m_FieldType);
    if (NMacroUtil::IsStructVoucherPart(field_name)) {
        string rt_var = "obj";
        bool found_constraint = false;
        string resolve_func = NMItemData::GetResolveFuncForSVPart(field_name, rt_var, constraints, found_constraint);
        if (found_constraint) {
            function += resolve_func;
        }

        auto pos = field_name.find_last_of('-');
        function += CMacroFunction_ApplyStrucVoucherPart::GetFuncName();
        if (found_constraint) {
            function += "(" + rt_var + ", ";
        }
        else {
            function += "(\"" + field_name.substr(0, pos) + "\", "; 
        }
        function += "\"" + field_name.substr(pos + 1) + "\", ";
    }
    else if (path.find(',') != NPOS) {
        vector<string> tokens;
        NStr::Split(path, ",", tokens);

        _ASSERT(tokens.size() == 2);
        // it is either an orgmod or a soubsource modifier
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

        function += CMacroFunction_AddorSetBsrcModifier::GetFuncName();
        function += "(" + rt_var + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", ";
    }
    else {
        function = CMacroFunction_SetStringQual::GetFuncName();
        function += "(\"" + path + "\", ";
    }

    function += kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    return function;
}


// CApplyMolinfoQualTreeItemData
CApplyMolinfoQualTreeItemData::CApplyMolinfoQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to molinfo qualifiers", EMacroFieldType::eMolinfo)
{
}

void CApplyMolinfoQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, "SetStringQualMolinfo", GetSetArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CApplyMolinfoQualTreeItemData::GetVariables()
{
    string variables;
    variables += kNewValue + " = %" + CMacroEditorContext::GetInstance().GetAsnMolinfoValue(m_ArgList[kField].GetValue(), m_ArgList[kNewValue].GetValue()) + "%\n";
    variables += GetExistingTextVariables(m_ArgList);
    return variables;
}

// CApplyDBLinkQualTreeItemData
CApplyDBLinkQualTreeItemData::CApplyDBLinkQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to DBLink fields", EMacroFieldType::eDBLink)
{
}

void CApplyDBLinkQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, "SetDBLinkStringQual", GetSetArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

void CApplyDBLinkQualTreeItemData::UpdateTarget(const TConstraints& constraints)
{
    CTempString new_target = CMacroBioData::sm_SeqNa;
    // if there is relevant constraint, change the target
    const string& field = m_ArgList[kField].GetValue();
    for (auto& it : constraints)  {
        if (it.second.find(field) != NPOS) {
            new_target = CMacroBioData::sm_DBLink;
            break;
        }
    }
    m_Target = new_target;
}

string CApplyDBLinkQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    if (m_Target == CMacroBioData::sm_DBLink) {
        string rt_var;
        bool found_constraint = false;
        function = NMItemData::GetResolveFuncForDBLink(m_ArgList[kField].GetValue(), rt_var, constraints, found_constraint);
        function += CMacroFunction_SetStringQual::GetFuncName() + "(";
        if (found_constraint) {
            function += rt_var;
        }
        else {
            function += "\"" + rt_var + ".data.strs\"";
        }
    }
    else {
        function = CMacroFunction_AddDBLink::GetFuncName() + "(";
        function += "\"" + m_ArgList[kField].GetValue() + "\"";
    }

    function += ", " + kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";
    
    return function;
}


// CApplyPubQualTreeItemData
CApplyPubQualTreeItemData::CApplyPubQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to publication fields", EMacroFieldType::ePubdesc)
{
}

void CApplyPubQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, CMacroFunction_SetStringQual::GetFuncName(), GetSetArgs());

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CApplyPubQualTreeItemData::GetVariables()
{
    string variables;
    const string field = m_ArgList[kField].GetValue();
    CPubFieldType::EPubFieldType type = CPubFieldType::GetTypeForLabel(field);
    if (type == CPubFieldType::ePubFieldType_Date) {
        try {
            CRef<CDate> date = CSubSource::DateFromCollectionDate(m_ArgList[kNewValue].GetValue());
            if (date && date->IsStd()) {
                const auto& std_date = date->GetStd();
                if (std_date.IsSetYear() && std_date.IsSetMonth() && std_date.IsSetDay()) {
                    variables = "year = %" + NStr::IntToString(std_date.GetYear()) + "%\n";
                    variables += "month = %" + NStr::IntToString(std_date.GetMonth()) + "%\n";
                    variables += "day = %" + NStr::IntToString(std_date.GetDay()) + "%";
                }
            }
        }
        catch (const CException&) {}
    }
    else if (type == CPubFieldType::ePubFieldType_DateDay ||
        type == CPubFieldType::ePubFieldType_DateMonth ||
        type == CPubFieldType::ePubFieldType_DateYear ||
        type == CPubFieldType::ePubFieldType_Status) {
        variables = kNewValue + " = %" + m_ArgList[kNewValue].GetValue() + "%";
    } else {
        variables = CApplyQualTreeItemData::GetVariables();
    }
    return variables;
}

string CApplyPubQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    const string field = m_ArgList[kField].GetValue();
    if (field.empty()) return kEmptyStr;

    CPubFieldType::EPubFieldType type = CPubFieldType::GetTypeForLabel(field);
    switch (type) { 
    case CPubFieldType::ePubFieldType_Pmid:
        return CMacroFunction_SetPubPMID::GetFuncName() + "(" + kNewValue + ");";
    case CPubFieldType::ePubFieldType_Date:
        return CMacroFunction_SetPubDate::GetFuncName() + "(year, month, day);";
    case CPubFieldType::ePubFieldType_DateYear:
        return CMacroFunction_SetPubDateField::GetFuncName() + "(\"year\"," + kNewValue + ");";
    case CPubFieldType::ePubFieldType_DateMonth:
        return CMacroFunction_SetPubDateField::GetFuncName() + "(\"month\"," + kNewValue + ");";
    case CPubFieldType::ePubFieldType_DateDay:
        return CMacroFunction_SetPubDateField::GetFuncName() + "(\"day\"," + kNewValue + ");";
    case CPubFieldType::ePubFieldType_Status:
        return CMacroFunction_SetPubStatus::GetFuncName() + "(" + kNewValue + ");";
    case CPubFieldType::ePubFieldType_Authors:
        return CMacroFunction_AddAuthorList::GetFuncName() + "(" + kNewValue + ");";
    default:
        break;
    }

    if (type == CPubFieldType::ePubFieldType_Title) {
        string rt_var;
        function = NMItemData::GetResolveFuncForPubQual(field, rt_var, constraints);
        function += CMacroFunction_SetPubTitle::GetFuncName() + "(" + rt_var + ", ";
    }
    else if (type == CPubFieldType::ePubFieldType_AuthorLastName
        || type == CPubFieldType::ePubFieldType_AuthorFirstName
        || type == CPubFieldType::ePubFieldType_AuthorSuffix
        || type == CPubFieldType::ePubFieldType_AuthorConsortium) {
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForApplyAuthors(type, rt_var, constraints);

        function += CMacroFunction_SetPubAuthor::GetFuncName();
        function += "(" + rt_var + ", \"" + CMacroEditorContext::GetInstance().GetAsnPathToAuthorFieldName(field) + "\", ";
    }
    else if (type == CPubFieldType::ePubFieldType_AuthorMiddleInitial) {
        string rt_var = "obj";
        function = NMItemData::GetResolveFuncForApplyAuthors(type, rt_var, constraints);

        function += CMacroFunction_SetPubAuthorMI::GetFuncName();
        function += "(" + rt_var + ", ";
    }
    else if (type == CPubFieldType::ePubFieldType_Journal) {
        function = CMacroFunction_SetPubJournal::GetFuncName() + "(";
    }
    else if (type == CPubFieldType::ePubFieldType_Volume) {
        function = CMacroFunction_SetPubVolIssuePages::sm_FuncVolume + string("(");
    }
    else if (type == CPubFieldType::ePubFieldType_Issue) {
        function = CMacroFunction_SetPubVolIssuePages::sm_FuncIssue + string("(");
    }
    else if (type == CPubFieldType::ePubFieldType_Pages) {
        function = CMacroFunction_SetPubVolIssuePages::sm_FuncPages + string("(");
    }
    else if (type == CPubFieldType::ePubFieldType_SerialNumber) {
        function = CMacroFunction_SetSerialNumber::GetFuncName() + "(";
    }
    else if (type == CPubFieldType::ePubFieldType_Citation) {
        function = CMacroFunction_SetPubCitation::GetFuncName() + "(";
    }
    else {
        function = CMacroFunction_SetPubAffil::GetFuncName();
        function += "(\"" + CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, EMacroFieldType::ePubAffilFields) + "\", ";
    }

    function += kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";
    return function;
}

CApplyAuthorsTreeItemData::CApplyAuthorsTreeItemData()
    : CMacroActionItemData("Apply new value to publication authors", EMacroFieldType::ePubAuthors)
{
}

void CApplyAuthorsTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent, "AddAuthors", { GetSetArgs()[2] });
    m_AuthorsPanel = dynamic_cast<CMacroAuthorNamesPanel*>(m_Panel->LoadCustomPanel(ECustomPanelType::eAddAuthorsPanel));
}

static const char* kLast = "last";
static const char* kFirst = "first";
static const char* kMiddle = "middle";
static const char* kSuffix = "suffix";

static string s_GetNameForDescription(CMacroAuthorNamesPanel::TNameTuple names)
{
    string res;
    if (!get<0>(names).empty()) {
        if (!get<1>(names).empty()) { // first name
            res += get<1>(names);
        }
        if (!get<2>(names).empty()) {
            if (!res.empty())
                res += " ";
            res += get<2>(names);
        }
        if (!res.empty())
            res += " ";
        res += get<0>(names);

        if (!get<3>(names).empty()) {
            res += " " + get<3>(names);
        }
        res = "'" + res + "'";
    }
    return res;
}

string CApplyAuthorsTreeItemData::GetMacroDescription() const
{
    string descr;
    const CMacroAuthorNamesPanel::TAuthorsVec& authors = m_AuthorsPanel->GetAuthorNames();
    for (auto& it : authors) {
        string author_name = s_GetNameForDescription(it);
        if (!descr.empty())
            descr += ", ";
        descr += author_name;
    }
    descr = "Apply " + descr + " to publication authors (";
    
    string existing_text = m_ArgList[kExistingText].GetValue();
    if (NStr::EqualNocase(existing_text, "overwrite")) {
        descr += "overwrite existing text";
    }
    else if (NStr::StartsWith(existing_text, "ignore", NStr::eNocase)) {
        descr += "ignore new text when existing text is present";
    }
    else {
        NStr::ToLower(existing_text);
        descr += existing_text;
    }
    descr += ")";
    return descr;
}

string CApplyAuthorsTreeItemData::GetVariables()
{
    string variables;
    m_NumAuthors = 0;
    const CMacroAuthorNamesPanel::TAuthorsVec& authors = m_AuthorsPanel->GetAuthorNames();
    for (unsigned int i = 0; i < authors.size(); ++i) {
        string nr = NStr::IntToString(i + 1);
        if (get<0>(authors[i]).empty()) {
            continue;
        }
        variables += kLast + nr + " = %" + get<0>(authors[i]) + "%\n";
        variables += kFirst + nr + " = %" + get<1>(authors[i]) + "%\n";
        variables += kMiddle + nr + " = %" + get<2>(authors[i]) + "%\n";
        variables += kSuffix + nr + " = %" + get<3>(authors[i]) + "%\n";
        m_NumAuthors++;
    }
    if (!variables.empty()) {
        variables.pop_back();
    }
    return variables;
}

static string s_GetAddAuthorFunction(const string& nr, const string& existing_text)
{
    string func = CMacroFunction_AddPubAuthor::GetFuncName();
    func += "(" + CTempString(kLast) + nr + ", " + CTempString(kFirst) + nr + ", " + CTempString(kMiddle) + nr + ", " + CTempString(kSuffix) + nr;
    func += ", \"" + NMItemData::GetHandleOldValue(existing_text) + "\");";
    return func;
}

string CApplyAuthorsTreeItemData::GetFunction(TConstraints& constraints) const
{
    if (m_NumAuthors == 0) return kEmptyStr;

    string function;
    const string& existing_text = m_ArgList[kExistingText].GetValue();
    if (NStr::EqualNocase(existing_text, "overwrite")) {
        function = s_GetAddAuthorFunction(NStr::IntToString(1), existing_text);
        for (unsigned i = 1; i < m_NumAuthors; ++i) {
            if (!function.empty()) function += "\n";
            function += s_GetAddAuthorFunction(NStr::IntToString(i+1), "append");
        }
    }
    else if (NStr::EqualNocase(existing_text, "append")
        || NStr::EqualNocase(existing_text, "prefix")) {
        for (unsigned i = 0; i < m_NumAuthors; ++i) {
            if (!function.empty()) function += "\n";
            function += s_GetAddAuthorFunction(NStr::IntToString(i + 1), existing_text);
        }
    }
    return function;
}

// CApplyStrCommQualTreeItemData
CApplyStrCommQualTreeItemData::CApplyStrCommQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to structured comment fields", EMacroFieldType::eStructComment)
{
}

void CApplyStrCommQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSetArgs());
    args.push_back(GetOptionalArgs()[5]);
    x_LoadPanel(parent, "SetStringStrCommQual", args);

    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);
    m_Panel->SetSelection(kField, 0);

    m_Panel->GetArgumentList().Attach(NMItemData::OnStructCommFieldChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
}

string CApplyStrCommQualTreeItemData::GetMacroDescription() const
{
    string descr = "Apply '" + m_ArgList[kNewValue].GetValue() + "' to ";
    descr += NMItemData::GetStructCommQualDescr(m_ArgList);
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CApplyStrCommQualTreeItemData::GetVariables()
{
    string variables = CApplyQualTreeItemData::GetVariables();
    if (NStr::EqualNocase(m_ArgList[kField].GetValue().get(), "Field") && m_ArgList[kStrCommField].GetShown()) {
        variables += "\n" + kStrCommField + " = %" + m_ArgList[kStrCommField].GetValue() + "%";
    }
    return variables;
}

string CApplyStrCommQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string rt_var = "obj";
    string field = m_ArgList[kField].GetValue();
    if (NStr::EqualNocase(field, "Field")) {
        function += CMacroFunction_SetStructCommField::GetFuncName();
        function += "(" + kStrCommField + ", ";
    }
    else if (NStr::EqualNocase(field, "Database name")) {
        function += CMacroFunction_SetStructCommDb::GetFuncName();
        function += "(";
    }
    else if (NStr::EqualNocase(field, "Field name")) {
        function = NMItemData::GetResolveFuncForStrCommQual(field, kEmptyStr, rt_var, constraints) + "\n";
        function += CMacroFunction_SetStringQual::GetFuncName();
        function += "(\"" + rt_var + ".label.str\", ";
    }

    function += kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";
    return function;
}

// CApplyStructCommentTreeItemData
CApplyStructCommentTreeItemData::CApplyStructCommentTreeItemData()
    : CMacroActionItemData("Apply new structured comment"), m_Seqdesc(new CSeqdesc)
{
}

void CApplyStructCommentTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args;
    x_LoadPanel(parent, "AddStructuredComment", args);
    m_StructCommentPanel = dynamic_cast<CStructuredCommentPanel*>(m_Panel->LoadCustomPanel(ECustomPanelType::eAddStructCommPanel));
}

string CApplyStructCommentTreeItemData::GetMacroDescription() const
{
    CRef<CSeqdesc> desc(new CSeqdesc);
    m_StructCommentPanel->UpdateSeqdesc(desc.GetNCObject());
    if (desc && desc->IsUser()) {
        desc->SetUser().SetType().SetStr("StructuredComment");
    }

    string prefix = CComment_rule::GetStructuredCommentPrefix(desc->GetUser(), true);
    return "Apply new structured comment with '" + prefix + "' database name";
}

static string s_GetFieldVar(unsigned int n)
{
    static const char* s_field = "strcomm_field";
    return s_field + NStr::UIntToString(n);
}

static string s_GetValueVar(unsigned int n)
{
    static const char* s_value = "new_value";
    return s_value + NStr::UIntToString(n);
}

string CApplyStructCommentTreeItemData::GetVariables()
{
    string variables;
    m_Index = 0;
    m_Seqdesc.Reset(new CSeqdesc);
    m_StructCommentPanel->UpdateSeqdesc(m_Seqdesc.GetNCObject());
    if (m_Seqdesc && m_Seqdesc->IsUser()) {
        m_Seqdesc->SetUser().SetType().SetStr("StructuredComment");
    }

    for (auto& it : m_Seqdesc->GetUser().GetData()) {
        if (it->IsSetData() && it->GetData().IsStr() &&
            it->IsSetLabel() && it->GetLabel().IsStr()) {
            string fieldname = it->GetLabel().GetStr();

            if (!NStr::Equal(fieldname, "StructuredCommentPrefix") && !NStr::Equal(fieldname, "StructuredCommentSuffix")) {
                string value = it->GetData().GetStr();
                if (!variables.empty()) {
                    variables += "\n";
                }
                variables += s_GetFieldVar(++m_Index) + " = %" + fieldname + "%\n";
                variables += s_GetValueVar(m_Index) + " = %" + value + "%";
            }
        }
    }
    return variables;
}

string CApplyStructCommentTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function;
    string prefix = CComment_rule::GetStructuredCommentPrefix(m_Seqdesc->GetUser(), true);
    if (prefix.empty()) return kEmptyStr;

    string rt_var = "obj";
    function = rt_var + " = " + CMacroFunction_SetStructCommDb::GetFuncName();
    function += "(\"" + prefix + "\", \"eAddQual\");";

    for (size_t i = 1; i <= m_Index; ++i) {
        function += "\n" + CMacroFunction_SetStructCommField::GetFuncName();
        function += "(" + rt_var + ", " + s_GetFieldVar(i) + ", " + s_GetValueVar(i) + ", \"eReplace\");";
    }
    return function;
}

//CApplyFeatQualTreeItemData
CApplyFeatQualTreeItemData::CApplyFeatQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to feature qualifiers", EMacroFieldType::eFeatQualifiers)
{
}

void CApplyFeatQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetOptionalArgs().begin(), GetOptionalArgs().begin() + 3);
    copy(GetSetArgs().begin() + 1, GetSetArgs().end(), back_inserter(args));
    x_LoadPanel(parent, "SetStringFeatQual", args);

    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes();
    m_Panel->SetControlValues(kFeatType, features);
    vector<string> qualifiers = CMacroEditorContext::GetInstance().GetFeatQualifiers();
    m_Panel->SetControlValues(kFeatQual, qualifiers);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnProteinNameSingle);
}

bool CApplyFeatQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForFeatures(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kFeatType].GetValue();
    return modified;
}

string CApplyFeatQualTreeItemData::GetMacroDescription() const
{
    string descr;
    descr = "Apply '" + m_ArgList[kNewValue].GetValue();
    descr += "' to " + m_ArgList[kFeatType].GetValue() + " " + m_ArgList[kFeatQual].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    descr += NMItemData::GetUpdatemRNADescription(m_ArgList[kUpdatemRNA]);
    return descr;
}

string CApplyFeatQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForFeatures(constraints, m_ArgList[kFeatType].GetValue());

    string function;
    bool update_mrna = m_ArgList[kUpdatemRNA].GetEnabled() && m_ArgList[kUpdatemRNA].IsTrue();
    string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kFeatType].GetValue(), m_ArgList[kFeatQual].GetValue());
    
    if (NMItemData::IsGeneField(m_ArgList[kFeatQual].GetValue()) && m_Target != CMacroBioData::sm_Gene) {
        function = CMacroFunction_SetRelFeatQual::GetFuncName();
        string gene_field = m_ArgList[kFeatQual].GetValue();
        NStr::ReplaceInPlace(gene_field, "-", " ");
        if (!NStr::StartsWith(gene_field, "gene ")) {
            gene_field = "gene " + gene_field;
        }
        gene_field = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(gene_field, EMacroFieldType::eGene);
        NMItemData::WrapInQuotes(gene_field);
        function += "(\"gene\", " + gene_field + ", ";
    }
    else {
        if (NMItemData::MultipleValuesAllowed(path)) {
            // place the constraint in the Do section
            string rt_var = "obj";
            function = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, !update_mrna);

            if (function.empty()) {
                function += CMacroFunction_SetStringQual::GetFuncName();
                function += "(\"" + path + "\", ";
            }
            else {
                function += CMacroFunction_SetStringQual::GetFuncName();
                function += "(" + rt_var + ", ";
            }
        }
        else if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            string rt_var = "obj";
            if (macro::NMacroUtil::IsSatelliteSubfield(tokens[1]) || macro::NMacroUtil::IsMobileElementTSubfield(tokens[1])) {
                function = NMItemData::GetResolveFuncForQual(tokens[0], m_ArgList[kFeatQual].GetValue(), rt_var, constraints);

                function += macro::CMacroFunction_AddorSetGbQual::GetFuncName();
                function += "(" + rt_var + ", \"" + tokens[0] + "\", \"" + m_ArgList[kFeatQual].GetValue() + "\", ";
            }
            else {
                function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

                function += CMacroFunction_AddorSetGbQual::GetFuncName();
                function += "(" + rt_var + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", ";
            }
        }
        else {
            function += CMacroFunction_SetStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }
    }

    function += kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";

    NMItemData::AppendUpdatemRNAFnc(update_mrna, function);
    return function;
}

// CApplyRNAQualTreeItemData
CApplyRNAQualTreeItemData::CApplyRNAQualTreeItemData()
    : CApplyQualTreeItemData("Apply new value to RNA qualifiers", EMacroFieldType::eRNA)
{
}

void CApplyRNAQualTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(GetSetArgs());
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    x_LoadPanel(parent, "SetStringRnaQual", args);

    vector<string> rnas = CMacroEditorContext::GetInstance().GetRNATypes();
    m_Panel->SetControlValues(kRNAType, rnas);
    m_Panel->SetSelection(kRNAType, 0);
    vector<string> class_types = CMacroEditorContext::GetInstance().GetncRNAClassTypes();
    m_Panel->SetControlValues(kncRNAClass, class_types);
    vector<string> fieldnames = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
    m_Panel->SetControlValues(kField, fieldnames);

    m_Panel->GetArgumentList().Attach(NMItemData::OnExistingTextChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
}

bool CApplyRNAQualTreeItemData::UpdateTarget()
{
    CTempString new_target = NMItemData::UpdateTargetForRnas(m_ArgList[kRNAType].GetValue());
    bool modified = (new_target != m_Target);
    m_Target = new_target;
    m_SelectedField = m_ArgList[kRNAType].GetValue();
    return modified;
}

string CApplyRNAQualTreeItemData::GetMacroDescription() const
{
    string descr;
    descr = "Apply '" + m_ArgList[kNewValue].GetValue() + "' to";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " " + m_ArgList[kField].GetValue();
    descr += NMItemData::GetExistingTextDescr(m_ArgList);
    return descr;
}

string CApplyRNAQualTreeItemData::GetFunction(TConstraints& constraints) const
{
    NMItemData::UpdateConstraintsForRnas(constraints, m_ArgList);

    string function;
    string field = m_ArgList[kField].GetValue();

    if (NStr::StartsWith(field, "gene")) {
        string gene_field = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, EMacroFieldType::eGene);
        NMItemData::WrapInQuotes(gene_field);
        function = CMacroFunction_SetRelFeatQual::GetFuncName();
        function += "(\"gene\", " + gene_field + ", ";
    }
    else {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(m_ArgList[kRNAType].GetValue() + " " + field, m_FieldType);
        if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            _ASSERT(tokens.size() == 2);
            string rt_var = "obj";
            function = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);

            function += CMacroFunction_AddorSetGbQual::GetFuncName();
            function += "(" + rt_var + ", \"" + tokens[0] + "\", \"" + tokens[1] + "\", ";
        }
        else if (path.find("::") != NPOS) {
            function = CMacroFunction_SetRnaProduct::GetFuncName();
            function += "(";
        }
        else {
            function += CMacroFunction_SetStringQual::GetFuncName();
            function += "(\"" + path + "\", ";
        }
    }

    function += kNewValue + ", " + kExistingText;
    if (m_ArgList[kDelimiter].GetEnabled()) {
        function += ", " + kDelimiter;
    }
    function += ");";
    return function;
}


namespace {
    const string kPartial5 = CMacroFunction_MakeInterval::sm_Arguments[0].m_Name;
    const string kPartial3 = CMacroFunction_MakeInterval::sm_Arguments[1].m_Name;
    const string kStrand = CMacroFunction_MakeInterval::sm_Arguments[2].m_Name;
    const string kWholeSeq = CMacroFunction_MakeInterval::sm_Arguments[3].m_Name;
    const string kInterval = CMacroFunction_MakeInterval::sm_Arguments[4].m_Name;
    const string kFromCoord = CMacroFunction_MakeInterval::sm_Arguments[5].m_Name;
    const string kToCoord = CMacroFunction_MakeInterval::sm_Arguments[6].m_Name;

    const string kRnaName = CMacroFunction_ApplyRNA::sm_Arguments[0].m_Name;
    const string kComment = CMacroFunction_ApplyOtherFeature::sm_Arguments[0].m_Name;
    const string kGeneLocus = CMacroFunction_ApplyOtherFeature::sm_Arguments[1].m_Name;
    const string kGeneDescr = CMacroFunction_ApplyOtherFeature::sm_Arguments[2].m_Name;
    const string kAddRedundant = CMacroFunction_ApplyOtherFeature::sm_Arguments[3].m_Name;
    const string kFeatureType = CMacroFunction_ApplyOtherFeature::sm_Arguments[4].m_Name;

    const string kQual1 = CMacroFunction_ApplyOtherFeature::sm_Arguments[5].m_Name;
    const string kQVal1 = CMacroFunction_ApplyOtherFeature::sm_Arguments[6].m_Name;
    const string kQual2 = CMacroFunction_ApplyOtherFeature::sm_Arguments[7].m_Name;
    const string kQVal2 = CMacroFunction_ApplyOtherFeature::sm_Arguments[8].m_Name;
    const string kQual3 = CMacroFunction_ApplyOtherFeature::sm_Arguments[9].m_Name;
    const string kQVal3 = CMacroFunction_ApplyOtherFeature::sm_Arguments[10].m_Name;
    const string kQual4 = CMacroFunction_ApplyOtherFeature::sm_Arguments[11].m_Name;
    const string kQVal4 = CMacroFunction_ApplyOtherFeature::sm_Arguments[12].m_Name;

    const string kProtName = CMacroFunction_ApplyCDS::sm_Arguments[0].m_Name;
    const string kProtDescr = CMacroFunction_ApplyCDS::sm_Arguments[1].m_Name;
    const string kCodonStart = CMacroFunction_ApplyCDS::sm_Arguments[2].m_Name;
    const string kAddmRNA = CMacroFunction_ApplyCDS::sm_Arguments[3].m_Name;


    string GetGeneDescr(const CArgumentList& args)
    {
        if (!args[kGeneLocus].GetShown())
            return kEmptyStr;

        string descr;
        if (!args[kGeneLocus].Empty() || !args[kGeneDescr].Empty()) {
            descr += ", apply gene with ";
            if (!args[kGeneLocus].Empty()) {
                descr += "'" + args[kGeneLocus].GetValue() + "' gene locus ";
            }
            if (!args[kGeneDescr].Empty()) {
                descr += "'" + args[kGeneDescr].GetValue() + "' gene description";
            }
        }
        return descr;
    }

    string GetQualifierDescription(const CArgumentList& args, const string& qual, const string& val)
    {
        if (!args[qual].Empty()) {
            return " with '" + args[val].GetValue() + "' " + args[qual].GetValue();
        }
        return kEmptyStr;
    }

    string GetFeatLocation(const CArgumentList& args, const string& rt_var)
    {
        string loc_fnc = rt_var + " = ";
        if (args[kWholeSeq].IsTrue()) {
            loc_fnc += CMacroFunction_MakeWholeSeqInterval::GetFuncName() + "(";
        }
        else {
            loc_fnc += CMacroFunction_MakeInterval::GetFuncName();
            if (!args[kFromCoord].Empty() && !args[kToCoord].Empty()) {
                loc_fnc += "(" + args[kFromCoord].GetValue() + ", " + args[kToCoord].GetValue() + ", ";
            }
        }
        loc_fnc += kPartial5 + ", " + kPartial3 + ", " + kStrand + ");\n";
        return loc_fnc;
    }

    string GetQualifierForFunction(const CArgumentList& args, const string& qual, const string& val)
    {
        if (args[val].Empty())
            return kEmptyStr;

        const string feat_name = args[kFeatureType].GetValue();
        const string qualifier = args[qual].GetValue();
        if (NStr::EqualNocase(feat_name, "gene")) {
            if (qualifier == "allele") {
                return ", \"data.gene.allele\", " + val;
            }
            else if (qualifier == "gene_synonym") {
                return ", \"data.gene.syn\", " + val;
            }
            else if (qualifier == "locus_tag") {
                return ", \"data.gene.locus-tag\", " + val;
            }
        }

        return ", \"qual.qual\", \"" + qualifier + "\", \"qual.val\", " + val;
    }

    string GetApplyGeneFunction(const CArgumentList& args, const string& rt_var)
    {
        string applygene_func = CMacroFunction_ApplyGene::GetFuncName();
        applygene_func += "(" + rt_var;
        if (args[kGeneLocus].GetShown() && !args[kGeneLocus].Empty()) {
            applygene_func += ", \"data.gene.locus\", " + kGeneLocus;
        }
        if (args[kGeneDescr].GetShown() && !args[kGeneDescr].Empty()) {
            applygene_func += ", \"data.gene.desc\", " + kGeneDescr;
        }
        applygene_func += ");";
        return applygene_func;
    }
}

// CAddRNAFeatTreeItemData
CAddRNAFeatTreeItemData::CAddRNAFeatTreeItemData()
    : CMacroActionItemData("Apply RNA feature to nucleotide sequences")
{
}

void CAddRNAFeatTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(CMacroFunction_ApplyRNA::sm_Arguments);
    args.push_back(GetOptionalArgs()[3]);
    args.push_back(GetOptionalArgs()[4]);
    copy(CMacroFunction_ApplyOtherFeature::sm_Arguments.begin(), CMacroFunction_ApplyOtherFeature::sm_Arguments.begin() + 4, back_inserter(args));
    copy(CMacroFunction_MakeInterval::sm_Arguments.begin(), CMacroFunction_MakeInterval::sm_Arguments.end(), back_inserter(args));
    x_LoadPanel(parent, CMacroFunction_ApplyRNA::GetFuncName(), args);

    vector<string> fields = CMacroEditorContext::GetInstance().GetRNATypes();
    fields.erase(fields.begin());
    m_Panel->SetControlValues(kRNAType, fields);
    fields = CMacroEditorContext::GetInstance().GetncRNAClassTypes(false);
    m_Panel->SetControlValues(kncRNAClass, fields);

    // select rRNA by default
    m_Panel->SetSelection(kRNAType, 3);
    fields = CMacroEditorContext::GetInstance().GetRNAProductnames("rRNA");
    m_Panel->SetControlValues(kRnaName, fields);

    m_Panel->GetArgumentList().Attach(NMItemData::OnRnaTypeChanged);
    m_Panel->GetArgumentList().Attach(NMItemData::OnControlsAddRNAPanelSelected);
}

string CAddRNAFeatTreeItemData::GetMacroDescription() const
{
    string descr = "Apply '" + m_ArgList[kRnaName].GetValue() + "'";
    if (m_ArgList[kncRNAClass].GetEnabled() && !m_ArgList[kncRNAClass].Empty()) {
        descr += " " + m_ArgList[kncRNAClass].GetValue();
    }
    descr += " " + m_ArgList[kRNAType].GetValue() + " feature";
    descr += GetGeneDescr(m_ArgList);
    return descr;
}

string CAddRNAFeatTreeItemData::GetVariables()
{
    string vars = x_GetVariables({ kPartial5, kPartial3, kStrand });
    if (m_ArgList[kInterval].IsTrue() &&
        !m_ArgList[kFromCoord].Empty() &&
        !m_ArgList[kToCoord].Empty()) {
        vars += x_GetVariables({ kFromCoord, kToCoord });
    }
    vars += x_GetVariables({ kRnaName, kAddRedundant });
    if (m_ArgList[kGeneLocus].GetShown() && !m_ArgList[kGeneLocus].Empty()) {
        vars += x_GetVariables({ kGeneLocus });
    }
    if (m_ArgList[kGeneDescr].GetShown() && !m_ArgList[kGeneDescr].Empty()) {
        vars += x_GetVariables({ kGeneDescr });
    }
    vars.pop_back();
    return vars;
}

string CAddRNAFeatTreeItemData::GetFunction(TConstraints& constraints) const
{
    const string rt_var = "location";
    string function = GetFeatLocation(m_ArgList, rt_var);

    function += CMacroFunction_ApplyRNA::GetFuncName();
    function += "(\"" + m_ArgList[kRNAType].GetValue() + "\", " + kRnaName + ", " + rt_var + ", " + kAddRedundant;

    if (m_ArgList[kRNAType].GetValue() == "ncRNA" && m_ArgList[kncRNAClass].GetEnabled()) {
        function += ", \"data.rna.ext.gen.class\", \"" + m_ArgList[kncRNAClass].GetValue() + "\"";
    }

    if (!m_ArgList[kComment].Empty()) {
        function += ", \"comment\", \"" + m_ArgList[kComment].GetValue() + "\"";
    }
    function += ");";

    string applygene_func = GetApplyGeneFunction(m_ArgList, rt_var);
    if (applygene_func.find(",") != NPOS) {
        function += "\n" + applygene_func;

        if (!m_ArgList[kAddRedundant].IsTrue()) {
            string constraint = CMacroFunction_NumberOfFeatures::sm_FunctionName;
            constraint += "(\"" + m_ArgList[kRNAType].GetValue() + "\") = 0";
            constraints.insert(constraints.begin(), make_pair(kEmptyStr, constraint));
        }
    }
    return function;
}

// CAddCDSFeatTreeItemData
CAddCDSFeatTreeItemData::CAddCDSFeatTreeItemData()
    : CMacroActionItemData("Apply CDS feature to nucleotide sequences")
{
}

void CAddCDSFeatTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(CMacroFunction_ApplyCDS::sm_Arguments);
    copy(CMacroFunction_ApplyOtherFeature::sm_Arguments.begin(), CMacroFunction_ApplyOtherFeature::sm_Arguments.begin() + 4, back_inserter(args));
    copy(CMacroFunction_MakeInterval::sm_Arguments.begin(), CMacroFunction_MakeInterval::sm_Arguments.end(), back_inserter(args));
    x_LoadPanel(parent, CMacroFunction_ApplyCDS::GetFuncName(), args);
}

string CAddCDSFeatTreeItemData::GetMacroDescription() const
{
    string descr = "Apply cds feature with '" + m_ArgList[kProtName].GetValue() + "' product name";
    if (m_ArgList[kAddmRNA].IsTrue()) {
        descr += "(add mRNA)";
    }
    else {
        descr += "(do not add mRNA)";
    }
    descr += GetGeneDescr(m_ArgList);
    return descr;
}

string CAddCDSFeatTreeItemData::GetVariables()
{
    string vars = x_GetVariables({ kPartial5, kPartial3, kStrand });
    if (m_ArgList[kInterval].IsTrue() &&
        !m_ArgList[kFromCoord].Empty() &&
        !m_ArgList[kToCoord].Empty()) {
        vars += x_GetVariables({ kFromCoord, kToCoord });
    }
    vars += x_GetVariables({ kProtName, kProtDescr, kAddmRNA, kAddRedundant });
    if (m_ArgList[kCodonStart].GetValue().get() == "Best") {
        vars += kCodonStart + " = \"best\"\n";
    }
    else {
        vars += kCodonStart + " = " + m_ArgList[kCodonStart].GetValue() + "\n";
    }
    if (!m_ArgList[kGeneLocus].Empty()) {
        vars += x_GetVariables({ kGeneLocus });
    }
    if (!m_ArgList[kGeneDescr].Empty()) {
        vars += x_GetVariables({ kGeneDescr });
    }
    vars.pop_back();
    return vars;
}

string CAddCDSFeatTreeItemData::GetFunction(TConstraints& constraints) const
{
    const string rt_var = "location";
    string function = GetFeatLocation(m_ArgList, rt_var);

    function += CMacroFunction_ApplyCDS::GetFuncName();
    function += "(" + kProtName + ", " + kProtDescr;
    function += ", " + kCodonStart + ", " + rt_var;
    function += ", " + kAddRedundant + ", " + kAddmRNA;

    if (!m_ArgList[kComment].Empty()) {
        function += ", \"comment\", \"" + m_ArgList[kComment].GetValue() + "\"";
    }
    function += ");";

    string applygene_func = GetApplyGeneFunction(m_ArgList, rt_var);
    if (applygene_func.find(",") != NPOS) {
        function += "\n" + applygene_func;

        if (!m_ArgList[kAddRedundant].IsTrue()) {
            string constraint = CMacroFunction_NumberOfFeatures::sm_FunctionName;
            constraint += "(\"cds\") = 0";
            constraints.insert(constraints.begin(), make_pair(kEmptyStr, constraint));
        }
    }
    return function;
}


// CAddFeatureTreeItemData
CAddFeatureTreeItemData::CAddFeatureTreeItemData()
    : CMacroActionItemData("Apply feature to nucleotide sequences", EMacroFieldType::eFeatQualifiers)
{
}

void CAddFeatureTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args(CMacroFunction_ApplyOtherFeature::sm_Arguments);
    copy(CMacroFunction_MakeInterval::sm_Arguments.begin(), CMacroFunction_MakeInterval::sm_Arguments.end(), back_inserter(args));
    x_LoadPanel(parent, CMacroFunction_ApplyOtherFeature::GetFuncName(), args);

    bool for_removal = false, to_create = true;
    vector<string> features = CMacroEditorContext::GetInstance().GetFeatureTypes(for_removal, to_create);
    m_Panel->SetControlValues(kFeatureType, features);
}

string CAddFeatureTreeItemData::GetMacroDescription() const
{
    string descr = "Apply " + m_ArgList[kFeatureType].GetValue() + " feature";
    descr += GetQualifierDescription(m_ArgList, kQual1, kQVal1);
    descr += GetQualifierDescription(m_ArgList, kQual2, kQVal2);
    descr += GetQualifierDescription(m_ArgList, kQual3, kQVal3);
    descr += GetQualifierDescription(m_ArgList, kQual4, kQVal4);
    descr += GetGeneDescr(m_ArgList);
    return descr;
}

string CAddFeatureTreeItemData::GetVariables()
{
    string vars = x_GetVariables({ kPartial5, kPartial3, kStrand });
    if (m_ArgList[kInterval].IsTrue() &&
        !m_ArgList[kFromCoord].Empty() &&
        !m_ArgList[kToCoord].Empty()) {
        vars += x_GetVariables({ kFromCoord, kToCoord });
    }
    
    vector<CTempString> args;
    if (!m_ArgList[kQVal1].Empty()) {
        args.push_back(kQVal1);
    }
    if (!m_ArgList[kQVal2].Empty()) {
        args.push_back(kQVal2);
    }
    if (!m_ArgList[kQVal3].Empty()) {
        args.push_back(kQVal3);
    }
    if (!m_ArgList[kQVal4].Empty()) {
        args.push_back(kQVal4);
    }
    if (!m_ArgList[kGeneLocus].Empty()) {
        args.push_back(kGeneLocus);
    }
    if (!m_ArgList[kGeneDescr].Empty()) {
        args.push_back(kGeneDescr);
    }
    vars += x_GetVariables(args);
    vars += x_GetVariables({ kAddRedundant });
    vars.pop_back();
    return vars;
}

string CAddFeatureTreeItemData::GetFunction(TConstraints& constraints) const
{
    const string rt_var = "location";
    string function = GetFeatLocation(m_ArgList, rt_var);

    const string feat_type = m_ArgList[kFeatureType].GetValue();
    bool feat_is_gene = NStr::EqualNocase(feat_type, "gene");
    function += CMacroFunction_ApplyOtherFeature::GetFuncName();
    string asn_path_to_feature;
    if (feat_is_gene) {
        if (!m_ArgList[kGeneLocus].Empty()) {
            asn_path_to_feature = "\"data.gene.locus\", " + kGeneLocus;
        }
        else if (!m_ArgList[kGeneDescr].Empty()) {
            asn_path_to_feature = "\"data.gene.desc\", " + kGeneDescr;
        }
        else {
            asn_path_to_feature = "\"data.gene.locus\", \"\"";
        }
    }
    else {
        asn_path_to_feature = CMacroEditorContext::GetInstance().GetAsnPathToFeature(feat_type);
    }

    function += "(" + asn_path_to_feature + ", " + rt_var + ", " + kAddRedundant;
    if (feat_is_gene && (!m_ArgList[kGeneDescr].Empty() && !m_ArgList[kGeneLocus].Empty())) {
        function += ", \"data.gene.desc\", " + kGeneDescr;
    }
    if (!m_ArgList[kComment].Empty()) {
        function += ", \"comment\", \"" + m_ArgList[kComment].GetValue() + "\"";
    }
    function += GetQualifierForFunction(m_ArgList, kQual1, kQVal1);
    function += GetQualifierForFunction(m_ArgList, kQual2, kQVal2);
    function += GetQualifierForFunction(m_ArgList, kQual3, kQVal3);
    function += GetQualifierForFunction(m_ArgList, kQual4, kQVal4);
    function += ");";

    if (!feat_is_gene) {
        string applygene_func = GetApplyGeneFunction(m_ArgList, rt_var);
        if (applygene_func.find(",") != NPOS) {
            function += "\n" + applygene_func;

            if (!m_ArgList[kAddRedundant].IsTrue()) {
                string constraint = CMacroFunction_NumberOfFeatures::sm_FunctionName;
                constraint += "(\"" + m_ArgList[kFeatureType].GetValue() + "\") = 0";
                constraints.insert(constraints.begin(), make_pair(kEmptyStr, constraint));
            }
        }
    }
    return function;
}

// CApplyPmidToEntryTreeItemData
CApplyPmidToEntryTreeItemData::CApplyPmidToEntryTreeItemData()
    : CMacroActionItemData("Apply PMID to the record and lookup publications by PMID")
{
}

void CApplyPmidToEntryTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_ApplyPmidToEntry::GetFuncName(),
        CMacroFunction_ApplyPmidToEntry::sm_Arguments);
}

string CApplyPmidToEntryTreeItemData::GetMacroDescription() const
{
    return "Apply pmid " + m_Args[0]->GetValue() + " and do PMID lookup";
}

string CApplyPmidToEntryTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function = CMacroFunction_ApplyPmidToEntry::GetFuncName();
    function += "(" + m_Args[0]->GetValue() + ");";
    return function;
}

// CApplyDOIToEntryTreeItemData
CApplyDOIToEntryTreeItemData::CApplyDOIToEntryTreeItemData()
    : CMacroActionItemData("Add publication identified by its DOI name")
{
}

void CApplyDOIToEntryTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_ApplyDOIToEntry::GetFuncName(),
        CMacroFunction_ApplyDOIToEntry::sm_Arguments);
}

string CApplyDOIToEntryTreeItemData::GetMacroDescription() const
{
    return "Apply publication with DOI name " + m_Args[0]->GetValue() + " and do DOI lookup";
}

string CApplyDOIToEntryTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function = CMacroFunction_ApplyDOIToEntry::GetFuncName();
    function += "(\"" + m_Args[0]->GetValue() + "\");";
    return function;
}

// CApplyDOILookupTreeItemData
CApplyDOILookupTreeItemData::CApplyDOILookupTreeItemData()
    : CMacroActionItemData("Lookup DOI number and replace unpublished references with the looked up article")
{
}

void CApplyDOILookupTreeItemData::x_AddParamPanel(wxWindow* parent)
{
    x_LoadPanel(parent,
        CMacroFunction_ApplyDOIToEntry::GetFuncName(),
        CMacroFunction_ApplyDOIToEntry::sm_Arguments);
}

string CApplyDOILookupTreeItemData::GetMacroDescription() const
{
    return "Look up DOI number '" + m_Args[0]->GetValue() + "'";
}

string CApplyDOILookupTreeItemData::GetFunction(TConstraints& constraints) const
{
    string function = CMacroFunction_DOILookup::GetFuncName();
    function += "(\"" + m_Args[0]->GetValue() + "\");";
    return function;
}

END_NCBI_SCOPE
