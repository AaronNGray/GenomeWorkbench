/*  $Id: relation.cpp 38265 2017-04-19 15:41:13Z katargir $
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
 *  warranties of performance, merchantability or fitness for any partic2ular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/relation.hpp>
#include <gui/objutils/obj_convert.hpp>

#include <gui/objutils/label.hpp>

#include <serial/serial.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

//
// CRelation
//

const char* CRelation::m_szRelationName = "RelationName";
const char* CRelation::m_szUserObjectType = "PolyRelation";

const CRelation* CRelation::ReadObject(CObjectIStream& in)
{
    CRef<CUser_object> user_obj(new CUser_object());
    in >> *user_obj;
    return RelationFromUserObject(*user_obj);
}

void CRelation::WriteObject(CObjectOStream& out) const
{
    CRef<CUser_object> user_obj(UserObjectFromRelation(*this));
    out << *user_obj;
}

CUser_object* CRelation::UserObjectFromRelation(const CRelation& relation)
{
    CRef<CUser_object> user_obj(new CUser_object());
    user_obj->SetType().SetStr(m_szUserObjectType);
    user_obj->AddField(m_szRelationName, string(relation.GetName()));
    relation.ToUserObject(*user_obj);
    return user_obj.Release();
}

const CRelation* CRelation::RelationFromUserObject(const CUser_object& user_obj)
{
    _ASSERT(user_obj.GetType().GetStr() == m_szUserObjectType);

    const string& name = user_obj.GetField(m_szRelationName).GetData().GetStr();

    CConstRef<CRelation> relation;
    if (name == CComplexRelation::m_Name) {
        CRef<CRelation> complex(new CComplexRelation());
        complex->FromUserObject(user_obj);
        relation.Reset(complex);
    }
    else if (name == CIdentityRelation::m_Name) {
        CRef<CRelation> identity(new CIdentityRelation());
        relation.Reset(identity);
    }
    else
        relation.Reset(CObjectConverter::FindRelationByName(name));

    return relation.Release();
}



CRelation::SObject::SObject()
{
}

CRelation::SObject::SObject(const CObject& obj)
    : object(&obj)
{
}

CRelation::SObject::SObject( const CObject& obj, const string& comm )
    : object(&obj)
    , comment(comm)
{
}

//
// Identity Relation
//

const char* CIdentityRelation::m_Name = "IdentityRelation";

void CIdentityRelation::GetRelated(objects::CScope& scope, const CObject& obj,
                                   TObjects& related,
                                   TFlags /*flags*/,
                                   ICanceled*) const
{
    related.push_back(SObject(obj));
}


void CIdentityRelation::Dump(ostream& ostream) const
{
    ostream << "Identity Relation" << endl;
}

//
// CComplexRelation
//

const char* CComplexRelation::m_szRelationsFieldName = "RelationList";
const char* CComplexRelation::m_szParallelFieldName = "Parallel";
const char* CComplexRelation::m_Name = "ComplexRelation";

string CComplexRelation::GetTypeName() const
{
    return (m_relations.size() > 0) ? (*m_relations.front()).GetTypeName() : "";
}

string CComplexRelation::GetRelatedTypeName() const
{
    return (m_relations.size() > 0) ? (*m_relations.back()).GetRelatedTypeName() : "";
}

void CComplexRelation::GetRelated(objects::CScope& scope, const CObject& obj,
                                  TObjects& related,
                                  TFlags flags,
                                  ICanceled* cancel) const
{
    if (m_Parallel) {
        ITERATE (RelVector, iter, m_relations)
            (**iter).GetRelated(scope, obj, related, flags, cancel);
    }
    else if (m_relations.size()) {
        TObjects new_objs;
        new_objs.push_back(SObject(obj));
        ITERATE (RelVector, iter, m_relations) {
            TObjects tmp;
            ITERATE (TObjects, obj_iter, new_objs) {
                (**iter).GetRelated(scope, **obj_iter, tmp, flags, cancel);
            }
            new_objs.swap(tmp);
        }

        related.insert(related.end(), new_objs.begin(), new_objs.end());
    }
}

void CComplexRelation::AddRelation(const CRelation* relation)
{
    m_relations.push_back(CConstRef<CRelation>(relation));
}

void CComplexRelation::ToUserObject(CUser_object& user_obj) const
{
    vector< CRef<CUser_object> > objs;
    ITERATE (RelVector, iter, m_relations)
        objs.push_back(CRef<CUser_object>(CRelation::UserObjectFromRelation(**iter)));
    user_obj.AddField(m_szRelationsFieldName, objs);
    user_obj.AddField(m_szParallelFieldName, m_Parallel);
}

void CComplexRelation::FromUserObject(const CUser_object& user_obj)
{
    const vector<CRef<CUser_object> >& objs =
        user_obj.GetField(m_szRelationsFieldName).GetData().GetObjects();

    m_Parallel = user_obj.GetField(m_szParallelFieldName).GetData().GetBool();

    m_relations.clear();
    ITERATE(CUser_field_Base::C_Data::TObjects, iter, objs)
        AddRelation(CRelation::RelationFromUserObject(**iter));
}

void CComplexRelation::Dump(ostream& ostream) const
{
    ostream << (m_Parallel ? "Parallel" : "Sequential") << endl;

    ITERATE (RelVector, iter, m_relations) {
        ostream << "  ";
        (*iter)->Dump(ostream);
        ostream << endl;
    }
}

string CComplexRelation::GetProperty(const string& key) const
{
    string retVal;

    if (m_Parallel) {
        retVal = "All Conversions";
    }
    else {
        ITERATE (RelVector, iter, m_relations) {
            string prop = (*iter)->GetProperty(key);
            if (!prop.empty())
                retVal = prop;
        }
    }

    return retVal;
}

//
// CBasicRelation
//

string CBasicRelation::GetProperty(const string& key) const
{
    string retVal;

    if (m_Properties == 0)
        return retVal;

    for (int i = 0;; ++i) {
        const SPropPair* iter = m_Properties + i;
        if (iter->typeName.empty())
            break;
        if (key == iter->typeName) {
            retVal = iter->value;
            break;
        }
    }

    return retVal;
}


END_NCBI_SCOPE
