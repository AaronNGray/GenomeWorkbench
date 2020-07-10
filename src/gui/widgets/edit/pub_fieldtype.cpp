/*  $Id: pub_fieldtype.cpp 44038 2019-10-10 19:58:08Z asztalos $
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
#include <serial/serialbase.hpp>
#include <objects/macro/Pub_type.hpp>
#include <objtools/edit/field_handler.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static bool s_OnlyShowInMacroEditor(CPubFieldType::EPubFieldType type)
{
    return (type == CPubFieldType::ePubFieldType_AffilFax ||
        type == CPubFieldType::ePubFieldType_AffilPhone ||
        type == CPubFieldType::ePubFieldType_AffilPostalCode ||
        type == CPubFieldType::ePubFieldType_DateYear ||
        type == CPubFieldType::ePubFieldType_DateMonth ||
        type == CPubFieldType::ePubFieldType_DateDay ||
        type == CPubFieldType::ePubFieldType_SerialNumber ||
        type == CPubFieldType::ePubFieldType_Citation);
}

vector<string> CPubFieldType::GetFieldNames(bool extended)
{
    vector<string> options;
    for (int field_type = ePubFieldType_Title;
        field_type < ePubFieldType_Unknown;
        field_type++) {
        if (extended || !s_OnlyShowInMacroEditor(EPubFieldType(field_type))) {
            string field_name = GetLabelForType((CPubFieldType::EPubFieldType)field_type);
            field_name = NormalizePubFieldName(field_name);
            options.push_back(field_name);
        }
    }
    return options;
}

vector<string> CPubFieldType::GetMacroFieldNames()
{
    vector<string> options;

    for (int field_type = ePubFieldType_Title;
        field_type < ePubFieldType_Unknown;
        field_type++) {
        string field_name = GetMacroLabelForType((CPubFieldType::EPubFieldType)field_type);
        options.push_back(field_name);
    }

    return options;
}


CPubFieldType::EPubFieldType CPubFieldType::GetTypeForLabel(string label)
{
    label = NormalizePubFieldName(label);
    for (int i = ePubFieldType_Title; i < ePubFieldType_Unknown; i++) {
        string match = GetLabelForType((EPubFieldType)i);
        match = NormalizePubFieldName(match);
        if (NStr::EqualNocase(label, match)) {
            return (EPubFieldType)i;
        }
    }
    return ePubFieldType_Unknown;
}

string CPubFieldType::GetLabelForType(EPubFieldType field_type)
{
    string rval = "";
    switch (field_type) {
    case ePubFieldType_Title:
        rval = "title";
        break;
    case ePubFieldType_AffilAffil:
        rval = "institution";
        break;
    case ePubFieldType_AffilDept:
        rval = "department";
        break;
    case ePubFieldType_AffilStreet:
        rval = "street";
        break;
    case ePubFieldType_AffilCity:
        rval = "city";
        break;
    case ePubFieldType_AffilState:
        rval = "state";
        break;
    case ePubFieldType_AffilCountry:
        rval = "country";
        break;
    case ePubFieldType_AffilEmail:
        rval = "email";
        break;
    case ePubFieldType_AffilFax:
        rval = "fax";
        break;
    case ePubFieldType_AffilPhone:
        rval = "phone";
        break;
    case ePubFieldType_AffilPostalCode:
        rval = "postal code";
        break;
    case ePubFieldType_AuthorFirstName:
        rval = "first name";
        break;
    case ePubFieldType_AuthorMiddleInitial:
        rval = "middle initials";
        break;
    case ePubFieldType_AuthorLastName:
        rval = "last name";
        break;
    case ePubFieldType_AuthorSuffix:
        rval = "suffix";
        break;
    case ePubFieldType_AuthorConsortium:
        rval = "consortium";
        break;
    case ePubFieldType_Journal:
        rval = "journal";
        break;
    case ePubFieldType_Volume:
        rval = "volume";
        break;
    case ePubFieldType_Issue:
        rval = "issue";
        break;
    case ePubFieldType_Pages:
        rval = "pages";
        break;
    case ePubFieldType_Date:
        rval = "date";
        break;
    case ePubFieldType_DateYear:
        rval = "year";
        break;
    case ePubFieldType_DateMonth:
        rval = "month";
        break;
    case ePubFieldType_DateDay:
        rval = "day";
        break;
    case ePubFieldType_SerialNumber:
        rval = "serial number";
        break;
    case ePubFieldType_Citation:
        rval = "citation";
        break;
    case ePubFieldType_Status:
        rval = "status";
        break;
    case ePubFieldType_Authors:
        rval = "name list";
        break;
    case ePubFieldType_Pmid:
        rval = "pmid";
        break;
    case ePubFieldType_Unknown:
        break;
    }
    if (IsAffilField(field_type)) {
        rval = "affiliation " + rval;
    }
    else if (IsAuthorField(field_type)) {
        rval = "author " + rval;
    }
    rval = "publication " + rval;
    return rval;
}

bool CPubFieldType::IsAffilField(EPubFieldType field_type)
{
    if (field_type >= ePubFieldType_AffilAffil && field_type <= ePubFieldType_AffilPostalCode) {
        return true;
    }
    return false;
}


bool CPubFieldType::IsAuthorField(EPubFieldType field_type)
{
    if (field_type >= ePubFieldType_AuthorFirstName && field_type <= ePubFieldType_AuthorConsortium) {
        return true;
    }
    return false;
}

string CPubFieldType::NormalizePubFieldName(string orig_label)
{
    if (NStr::StartsWith(orig_label, "publication ")) {
        orig_label = orig_label.substr(12);
    }
    if (NStr::StartsWith(orig_label, "affiliation ")) {
        orig_label = orig_label.substr(12);
    }
    return orig_label;
}

vector<string> CPubFieldType::GetChoicesForField(EPubFieldType field_type, bool& allow_other)
{
    vector<string> choices;

    switch (field_type) {
    case ePubFieldType_Status:
        allow_other = false;
        for (int i = ePubFieldStatus_Published; i < ePubFieldStatus_Any; i++) {
            string match = GetLabelForStatus((EPubFieldStatus)i);
            choices.push_back(match);
        }
        break;
    default:
        break;
    }
    return choices;
}

vector<string> CPubFieldType::GetValuesForField(EPubFieldType field_type)
{
    vector<string> values;

    switch (field_type) {
    case ePubFieldType_Status:
    {
        auto enum_values = ENUM_METHOD_NAME(EPub_type)()->GetValues();
        auto it = enum_values.begin();
        ++it;
        for (; it != enum_values.end(); ++it) {
            values.push_back(it->first);
        }
    }
        break;
    default:
        break;
    }
    return values;
}

string CPubFieldType::GetLabelForStatus(EPubFieldStatus status)
{
    string rval = "";
    switch (status) {
    case ePubFieldStatus_Published:
        rval = "Published";
        break;
    case ePubFieldStatus_Unpublished:
        rval = "Unpublished";
        break;
    case ePubFieldStatus_InPress:
        rval = "In Press";
        break;
    case ePubFieldStatus_Submit:
        rval = "Submitter Block";
        break;
    case ePubFieldStatus_Any:
        rval = "Any";
        break;
    }
    return rval;
}

CPubFieldType::EPubFieldStatus CPubFieldType::GetStatusFromString(const string& str)
{
    for (int i = ePubFieldStatus_Published; i < ePubFieldStatus_Any; i++) {
        string match = GetLabelForStatus((EPubFieldStatus)i);
        if (objects::edit::CFieldHandler::QualifierNamesAreEquivalent(str, match)) {
            return (EPubFieldStatus)i;
        }
    }
    return ePubFieldStatus_Any;
}

string CPubFieldType::GetMacroLabelForType(EPubFieldType field_type)
{
    string rval;
    string affil_field, author_field, date_field, pars("()");
    switch (field_type) {
    case ePubFieldType_Title:
        rval = macro::CMacroFunction_PubFields::sm_PubTitle + pars;
        break;
    case ePubFieldType_AffilAffil:
        affil_field = "affil";
        break;
    case ePubFieldType_AffilDept:
        affil_field = "div";
        break;
    case ePubFieldType_AffilStreet:
        affil_field = "street";
        break;
    case ePubFieldType_AffilCity:
        affil_field = "city";
        break;
    case ePubFieldType_AffilState:
        affil_field = "sub";
        break;
    case ePubFieldType_AffilCountry:
        affil_field = "country";
        break;
    case ePubFieldType_AffilEmail:
        affil_field = "email";
        break;
    case ePubFieldType_AffilFax:
        affil_field = "fax";
        break;
    case ePubFieldType_AffilPhone:
        affil_field = "phone";
        break;
    case ePubFieldType_AffilPostalCode:
        affil_field = "postal-code";
        break;
    case ePubFieldType_AuthorFirstName:
        author_field = "first";
        break;
    case ePubFieldType_AuthorMiddleInitial:
        author_field = "initials";
        break;
    case ePubFieldType_AuthorLastName:
        author_field = "last";
        break;
    case ePubFieldType_AuthorSuffix:
        author_field = "suffix";
        break;
    case ePubFieldType_AuthorConsortium:
        author_field = "consortium";
        break;
    case ePubFieldType_Journal:
        rval = macro::CMacroFunction_PubFields::sm_PubJournal + pars;
        break;
    case ePubFieldType_Volume:
        rval = macro::CMacroFunction_PubFields::sm_PubVolume + pars;
        break;
    case ePubFieldType_Issue:
        rval = macro::CMacroFunction_PubFields::sm_PubIssue + pars;
        break;
    case ePubFieldType_Pages:
        rval = macro::CMacroFunction_PubFields::sm_PubPages + pars;
        break;
    case ePubFieldType_Date:
        rval = "";
        break;
    case ePubFieldType_DateYear:
        date_field = "year";
        break;
    case ePubFieldType_DateMonth:
        date_field = "month";
        break;
    case ePubFieldType_DateDay:
        date_field = "day";
        break;
    case ePubFieldType_SerialNumber:
        rval = macro::CMacroFunction_PubFields::sm_PubSerialNumber + pars;
        break;
    case ePubFieldType_Citation:
        rval = macro::CMacroFunction_PubFields::sm_PubCit + pars;
        break;
    case ePubFieldType_Status:
        rval = macro::CMacroFunction_PubFields::sm_PubClass + pars;
        break;
    case ePubFieldType_Authors:
        rval = macro::CMacroFunction_PubFields::sm_PubAuthors + pars;
        break;
    case ePubFieldType_Pmid:
        rval = macro::CMacroFunction_PubFields::sm_PubPMID + pars;
        break;
    case ePubFieldType_Unknown:
        break;
    }
   
    if (!affil_field.empty()) {
        rval = macro::CMacroFunction_PubFields::sm_PubAffil + string("(\"" + affil_field + "\")");
    }
    else if (!author_field.empty()) {
        rval = macro::CMacroFunction_PubFields::sm_PubAuthors + string("(\"" + author_field + "\")");
    }
    else if (!date_field.empty()) {
        rval = macro::CMacroFunction_PubFields::sm_PubDate + string("(\"" + date_field + "\")");
    }
    return rval;
}

END_NCBI_SCOPE

