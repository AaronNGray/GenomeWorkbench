/*  $Id: choicetype.cpp 569649 2018-08-27 14:47:05Z gouriano $
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
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Author: Eugene Vasilchenko
*
* File Description:
*   Type description for CHOIE type
*
*/

#include <ncbi_pch.hpp>
#include "exceptions.hpp"
#include "choicetype.hpp"
#include <serial/impl/autoptrinfo.hpp>
#include <serial/impl/choice.hpp>
#include "value.hpp"
#include "choicestr.hpp"
#include "choiceptrstr.hpp"
#include "srcutil.hpp"
#include <serial/impl/member.hpp>
#include <typeinfo>
#include "aliasstr.hpp"

BEGIN_NCBI_SCOPE

struct CAnyTypeChoice {
    AnyType data;
    TMemberIndex index;
    CAnyTypeChoice(void)
        : index(kEmptyChoice)
        {
        }
};

static TObjectPtr CreateAnyTypeChoice(TTypeInfo /*typeInfo*/,
                                      CObjectMemoryPool* /*memoryPool*/)
{
    return new CAnyTypeChoice();
}

static
TMemberIndex GetIndexAnyTypeChoice(const CChoiceTypeInfo* /*choiceType*/,
                                   TConstObjectPtr choicePtr)
{
    const CAnyTypeChoice* choice =
        static_cast<const CAnyTypeChoice*>(choicePtr);
    return choice->index;
}

static
void SetIndexAnyTypeChoice(const CChoiceTypeInfo* /*choiceType*/,
                           TObjectPtr choicePtr,
                           TMemberIndex index,
                           CObjectMemoryPool* /*memoryPool*/)
{
    CAnyTypeChoice* choice = static_cast<CAnyTypeChoice*>(choicePtr);
    choice->index = index;
}

static
void ResetIndexAnyTypeChoice(const CChoiceTypeInfo* /*choiceType*/,
                             TObjectPtr choicePtr)
{
    CAnyTypeChoice* choice = static_cast<CAnyTypeChoice*>(choicePtr);
    choice->index = kEmptyChoice;
}

const char* CChoiceDataType::GetASNKeyword(void) const
{
    return "CHOICE";
}

string CChoiceDataType::GetSpecKeyword(void) const
{
    return GetASNKeyword();
}

const char* CChoiceDataType::GetDEFKeyword(void) const
{
    return "_CHOICE_";
}

void CChoiceDataType::PrintASN(CNcbiOstream& out, int indent) const
{
    const CDataMember& m = *GetMembers().front();
    if (!m.Attlist()) {
        CParent::PrintASN(out, indent);
        return;
    }
    out << "SEQUENCE" << " {";
    ++indent;
    PrintASNNewLine(out, indent);
    m.PrintASN(out, indent, false);
    PrintASNNewLine(out, indent);
    out << GetMemberName() << " ";
    out << GetASNKeyword() << " {";
    ++indent;
    ITERATE ( TMembers, i, GetMembers() ) {
        TMembers::const_iterator next = i;
        bool last = ++next == GetMembers().end();
        const CDataMember& member = **i;
        if (!member.Attlist()) {
            PrintASNNewLine(out, indent);
            member.PrintASN(out, indent, last);
        }
    }
    --indent;
    PrintASNNewLine(out, indent);
    m_LastComments.PrintASN(out, indent, CComments::eMultiline);
    out << "}";
    --indent;
    PrintASNNewLine(out, indent);
    out << "}";
}

void CChoiceDataType::PrintJSONSchema(CNcbiOstream& out, int indent, list<string>& required, bool contents_only) const
{
    TMembers::const_iterator att = find_if( m_Members.begin(), m_Members.end(), [](const TMembers::value_type& e) { return e->Attlist();});
    bool first = true;
#if 0
/*
    tried to combine allOf and oneOf
    I have failed to make it work
    I get valid schema, but instance validation fails
*/
    if (att != m_Members.end()) {
        if (first) {
            first = false;
            PrintASNNewLine(out, indent++) << "\"definitions\": {";
        } else {
            out << ',';
        }
        PrintASNNewLine(out, indent++) << "\"" << "attributes" << "\": {";
        (*att)->PrintJSONSchema(out,indent);
        PrintASNNewLine(out, --indent) << "}";
        PrintASNNewLine(out, --indent) << "},";
    }
#endif

    PrintASNNewLine(out, indent++) << "\"oneOf\": [";
    ITERATE ( TMembers, i, m_Members ) {
        const CDataMember& member = **i;
        if (!member.Attlist()) {
            if (!first) {
                out << ",";
            } else {
                first = false;
            }
#if 0
/*
tried to combine allOf and oneOf
I have failed to make it work
I get valid schema, but instance validation fails
*/
            if (member.Notag()) {
                PrintASNNewLine(out, indent++) << "{";
                member.PrintJSONSchema(out, indent);
                PrintASNNewLine(out, --indent) << "}";
            } else {
                PrintASNNewLine(out, indent++) << "{";
                if (att) {
                    PrintASNNewLine(out, indent++) << "\"allOf\": [";
                    PrintASNNewLine(out, indent)   << "{\"$ref\": \"#/definitions/" << this->GetMemberName() << "/definitions/attributes\"},";
                    PrintASNNewLine(out, indent++) << "{";
                }


                PrintASNNewLine(out, indent)   << "\"type\": \"object\",";
                PrintASNNewLine(out, indent++) << "\"properties\": {";
                PrintASNNewLine(out, indent++) << "\"" << member.GetName() << "\": {";
                member.PrintJSONSchema(out, indent);
                PrintASNNewLine(out, --indent) << "}";
                PrintASNNewLine(out, --indent) << "},";
                PrintASNNewLine(out, indent) << "\"additionalProperties\": false";

                if (att) {
                    PrintASNNewLine(out, --indent) << "}";
                    PrintASNNewLine(out, --indent) << "]";
                }


                PrintASNNewLine(out, --indent) << "}";
            }
#else
            if (member.Notag()) {
                PrintASNNewLine(out, indent++) << "{";
                member.PrintJSONSchema(out, indent, required);
                PrintASNNewLine(out, --indent) << "}";
            } else {
                list<string> req;
                PrintASNNewLine(out, indent++) << "{";
                PrintASNNewLine(out, indent)   << "\"type\": \"object\",";
                PrintASNNewLine(out, indent++) << "\"properties\": {";
                if (att != m_Members.end()) {
                    (*att)->PrintJSONSchema(out, indent, req, true);
                    out << ",";
                }
                PrintASNNewLine(out, indent++) << "\"" << member.GetName() << "\": {";
                member.PrintJSONSchema(out, indent, req);
                PrintASNNewLine(out, --indent) << "}";
                PrintASNNewLine(out, --indent) << "},";
                if (!req.empty()) {
                    PrintASNNewLine(out, indent) << "\"required\": [";
                    transform(req.begin(), req.end(), Dt_ostream_iterator<string>(out, ", "),
                        [](const string& e) {
                            return string("\"").append(e).append("\"");
                        });
                    out << "],";
                }
                PrintASNNewLine(out, indent) << "\"additionalProperties\": false";
                PrintASNNewLine(out, --indent) << "}";
            }
#endif
        }
    }
    PrintASNNewLine(out, --indent) << "]";
}

void CChoiceDataType::FixTypeTree(void) const
{
    CParent::FixTypeTree();
    ITERATE ( TMembers, m, GetMembers() ) {
        (*m)->GetType()->SetInChoice(this);
    }
}

const char* CChoiceDataType::XmlMemberSeparator(void) const
{
    return " | ";
}

bool CChoiceDataType::CheckValue(const CDataValue& value) const
{
    const CNamedDataValue* choice =
        dynamic_cast<const CNamedDataValue*>(&value);
    if ( !choice ) {
        value.Warning("CHOICE value expected", 11);
        return false;
    }
    for ( TMembers::const_iterator i = GetMembers().begin();
          i != GetMembers().end(); ++i ) {
        if ( (*i)->GetName() == choice->GetName() )
            return (*i)->GetType()->CheckValue(choice->GetValue());
    }
    return false;
}

CTypeInfo* CChoiceDataType::CreateTypeInfo(void)
{
    unique_ptr<CChoiceTypeInfo>
        typeInfo(new CChoiceTypeInfo(sizeof(CAnyTypeChoice),
                                     HasExternalName() ? GlobalName() : kEmptyStr,
                                     TObjectPtr(0), &CreateAnyTypeChoice,
                                     typeid(CAnyTypeChoice),
                                     &GetIndexAnyTypeChoice,
                                     &SetIndexAnyTypeChoice,
                                     &ResetIndexAnyTypeChoice));
    for ( TMembers::const_iterator i = GetMembers().begin();
          i != GetMembers().end(); ++i ) {
        CDataMember* member = i->get();
        if (member->Attlist()) {
            CMemberInfo* memInfo =
                typeInfo->AddMember(member->GetName(),0,
                                    member->GetType()->GetTypeInfo());
            if (!IsASNDataSpec()) {
                memInfo->SetNoPrefix();
            }
            if (member->Attlist()) {
                memInfo->SetAttlist();
            }
            if (member->Notag()) {
                memInfo->SetNotag();
            }
        } else {
            CVariantInfo* varInfo = 
                typeInfo->AddVariant(member->GetName(), 0,
                                     member->GetType()->GetTypeInfo());
            if (!IsASNDataSpec()) {
                varInfo->SetNoPrefix();
            }
            if (member->Notag()) {
                varInfo->SetNotag();
            }
        }
    }
    typeInfo->AssignItemsTags();
    return UpdateModuleName(typeInfo.release());
}

AutoPtr<CTypeStrings> CChoiceDataType::GenerateCode(void) const
{
#if 0
    return GetFullCType();
#else
    string alias = GetVar("_fullalias");
    if (alias.empty()) {
        return GetFullCType();
    }
    const CDataType* aliastype = ResolveGlobal(alias);
    if (!aliastype) {
        NCBI_THROW(CDatatoolException,eWrongInput,
            "cannot create type info of _fullalias " + alias);
    }
    AutoPtr<CTypeStrings> dType = aliastype->GetRefCType();
    dType->SetDataType(aliastype);
    AutoPtr<CAliasTypeStrings> code(new CAliasTypeStrings(GlobalName(),
                                                          ClassName(),
                                                          *dType.release(),
                                                          Comments()));
    code->SetNamespaceName( GetNamespaceName());
    code->SetFullAlias();
    return AutoPtr<CTypeStrings>(code.release());
#endif
}

AutoPtr<CTypeStrings> CChoiceDataType::GetRefCType(void) const
{
    if ( GetBoolVar("_virtual_choice") ) {
        AutoPtr<CTypeStrings> cls(new CClassRefTypeStrings(ClassName(),
                                                           Namespace(),
                                                           FileName(),
                                                           Comments()));
        return AutoPtr<CTypeStrings>(new CChoicePtrRefTypeStrings(cls));
    }
    else {
        return AutoPtr<CTypeStrings>(new CChoiceRefTypeStrings(ClassName(),
                                                               Namespace(),
                                                               FileName(),
                                                               Comments()));
    }
}

AutoPtr<CTypeStrings> CChoiceDataType::GetFullCType(void) const
{
    if ( GetBoolVar("_virtual_choice") ) {
        AutoPtr<CChoicePtrTypeStrings>
            code(new CChoicePtrTypeStrings(
                GlobalName(), ClassName(), GetNamespaceName(), this, Comments()));
        ITERATE ( TMembers, i, GetMembers() ) {
            AutoPtr<CTypeStrings> varType = (*i)->GetType()->GetFullCType();
            code->AddVariant((*i)->GetName(), varType);
        }
        SetParentClassTo(*code);
        return AutoPtr<CTypeStrings>(code.release());
    }
    else {
        bool rootClass = GetParentType() == 0;
        AutoPtr<CChoiceTypeStrings> code(new CChoiceTypeStrings(
            GlobalName(), ClassName(), GetNamespaceName(), this, Comments()));
        bool haveUserClass = rootClass;
        code->SetHaveUserClass(haveUserClass);
        code->SetObject(true);
        ITERATE ( TMembers, i, GetMembers() ) {
            AutoPtr<CTypeStrings> varType = (*i)->GetType()->GetFullCType();
            string external_name = (*i)->GetName();
            string member_name = (*i)->GetType()->DefClassMemberName();
            if (member_name.empty()) {
                member_name = external_name;
            }
            bool delayed = GetBoolVar((*i)->GetName()+"._delay");
            bool in_union = GetBoolVar((*i)->GetName()+"._in_union", true);
            code->AddVariant(external_name, member_name, varType, delayed, in_union,
                             (*i)->GetType()->GetTag(),
                             !IsASNDataSpec(), (*i)->Attlist(), (*i)->Notag(),
                             (*i)->SimpleType(),(*i)->GetType(),
                             (*i)->Comments());
            (*i)->GetType()->SetTypeStr(&(*code));
        }
        SetTypeStr(&(*code));
        SetParentClassTo(*code);
        return AutoPtr<CTypeStrings>(code.release());
    }
}

END_NCBI_SCOPE
