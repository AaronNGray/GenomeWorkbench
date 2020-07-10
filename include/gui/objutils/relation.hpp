#ifndef GUI_UTILS___RELATION__HPP
#define GUI_UTILS___RELATION__HPP

/*  $Id: relation.hpp 38266 2017-04-19 15:59:44Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/objects.hpp>
#include <util/icanceled.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

class CObjectList;

class NCBI_GUIOBJUTILS_EXPORT CRelation : public CObject
{
public:
    enum {
        /// retrieve all possible identities if multiple can be shown
        fConvert_All = 0x01,

        /// retrieve the best single entity if multiple can be shown
        fConvert_Best = 0x02,

        /// do not perform any expensive tests (such as fetching from
        /// the network)
        fConvert_NoExpensive = 0x03,

        /// combined sets of flags
        eDefault = fConvert_Best
    };
    typedef int TFlags;

    /// struct SObject provides an interface for defining what is returned from
    /// object conversion.  Rather than simply provide the object, we can
    /// provide some contextual information as well.
    struct NCBI_GUIOBJUTILS_EXPORT SObject
    {
    public:
        SObject();
        SObject(const CObject& obj);
        SObject(const CObject& obj, const string& comm);

        /// retrieve the source for this object
        /// this is the original object that was used to generate this object
        const CObject& GetSource() const
        {
            return *source;
        }

        /// retrieve a comment concerning what this object means
        /// this may be, for example, a description of the relationship of
        /// the object to its source
        const string& GetComment() const
        {
            return comment;
        }

        /// return the object through a couple of equivalent functions:

        /// dereference operator
        const CObject& operator*() const
        {
            return *object;
        }

        /// cast operator
        operator const CObject&(void) const
        {
            return *object;
        }

        /// named function
        const CObject& GetObject(void) const
        {
            return *object;
        }

        /// named function
        const CObject* GetObjectPtr(void) const
        {
            return object;
        }

    private:
        friend class CObjectConverter;

        /// the converted object
        CConstRef<CObject> object;

        /// the source object
        CConstRef<CObject> source;

        /// a comment concerning this object's origin
        string comment;
    };
    typedef vector<SObject> TObjects;

    CRelation() {}

    virtual string    GetName() const = 0;
    virtual string    GetDescription() const = 0;

    virtual string    GetTypeName() const = 0;
    virtual string    GetRelatedTypeName() const = 0;

    virtual void      GetRelated(objects::CScope& scope, const CObject& obj,
                                 TObjects& related,
                                 TFlags flags = eDefault,
                                 ICanceled* cancel = NULL) const = 0;

    virtual string GetProperty(const string& /*key*/) const { return ""; }

    virtual void Dump(ostream& /*ostream*/) const {}

    static const CRelation* ReadObject(CObjectIStream& in);
    void WriteObject(CObjectOStream& out) const;

protected:
    static objects::CUser_object* UserObjectFromRelation(const CRelation& relation);
    static const CRelation* RelationFromUserObject(const objects::CUser_object& user_obj);
    virtual void ToUserObject(objects::CUser_object& /*user_obj*/) const {}
    virtual void FromUserObject(const objects::CUser_object& /*user_obj*/) {}

private:
    static const char* m_szUserObjectType;
    static const char* m_szRelationName;
};

//
// Identity Relation
//

class NCBI_GUIOBJUTILS_EXPORT CIdentityRelation : public CRelation
{
public:
    virtual string    GetName() const { return m_Name; }
    virtual string    GetDescription() const { return "Identity Relation"; }

    virtual string    GetTypeName() const { return "Object"; }
    virtual string    GetRelatedTypeName() const { return "Object"; }

    virtual void      GetRelated(objects::CScope& scope, const CObject& obj,
                                 TObjects& related,
                                 TFlags flags = eDefault,
                                 ICanceled* cancel = NULL) const;
    virtual string    GetProperty(const string& /*key*/) const { return "Not Converted"; }
    virtual void Dump(ostream& ostream) const;

    static const char* m_Name;
};

class NCBI_GUIOBJUTILS_EXPORT CComplexRelation : public CRelation
{
public:
    CComplexRelation(bool parallel = false) : m_Parallel(parallel) {}
    virtual string    GetName() const { return m_Name; }
    virtual string    GetDescription() const { return "Complex Relation"; }

    virtual string    GetTypeName() const;
    virtual string    GetRelatedTypeName() const;

    virtual void      GetRelated(objects::CScope& scope, const CObject& obj,
                                 TObjects& related,
                                 TFlags flags = eDefault,
                                 ICanceled* cancel = NULL) const;

    virtual string GetProperty(const string& key) const;

    virtual void Dump(ostream& ostream) const;

    void AddRelation(const CRelation* relation);

    static const char* m_Name;

protected:
    virtual void ToUserObject(objects::CUser_object& user_obj) const;
    virtual void FromUserObject(const objects::CUser_object& user_obj);

private:
    bool m_Parallel;
    typedef vector<CConstRef<CRelation> > RelVector;
    RelVector m_relations;
    static const char* m_szRelationsFieldName;
    static const char* m_szParallelFieldName;
};

class NCBI_GUIOBJUTILS_EXPORT CBasicRelation : public CRelation
{
public:
    typedef void (*TFNConvert)(objects::CScope& scope,
                               const CObject& obj,
                               TObjects& related,
                               TFlags flags,
                               ICanceled* cancel);

    struct SPropPair { const string typeName; const string value; };

    CBasicRelation() : m_pFnCvt(0) {}

    CBasicRelation(const string& name,
                   const string& from_type,
                   const string& to_type,
                   TFNConvert pfncvt,
                   const SPropPair* properties = 0) :
        m_Name(name),
        m_FromType(from_type),
        m_ToType(to_type),
        m_pFnCvt(pfncvt),
        m_Properties(properties) {}

    virtual string    GetName() const { return m_Name; }
    virtual string    GetDescription() const { return ""; }

    virtual string    GetTypeName() const { return m_FromType; }
    virtual string    GetRelatedTypeName() const { return m_ToType; }

    virtual void      GetRelated(objects::CScope& scope, const CObject& obj,
                                 TObjects& related,
                                 TFlags flags = eDefault,
                                 ICanceled* cancel = NULL) const
    {
        m_pFnCvt(scope, obj, related, flags, cancel);
    }

    virtual string GetProperty(const string& key) const;

    virtual void Dump(ostream& ostream) const
    {
        ostream << m_Name;
    }

private:
    string     m_Name;
    string     m_FromType;
    string     m_ToType;
    TFNConvert m_pFnCvt;
    const SPropPair* m_Properties;
};

/////////////////////////////////////////////////////////////////////////////
///
/// Macros to assist in building property lists for relation converters
///

#define BEGIN_RELATION_PROPERTY_LIST(name)\
static const CBasicRelation::SPropPair name[]={\

#define RELATION_PROPERTY_ENTRY(typeName,value)\
{typeName,value},

#define END_RELATION_PROPERTY_LIST()\
{"",""}}




inline CObjectOStream& operator<<(CObjectOStream& out, CRelation* pOb)
{
    pOb->WriteObject(out);
    return out;
}
inline CObjectOStream& operator<<(CObjectOStream& out, const CRelation* pOb)
{
    pOb->WriteObject(out);
    return out;
}
inline CObjectOStream& operator<<(CObjectOStream& out, CRef<CRelation>& pOb)
{
    pOb->WriteObject(out);
    return out;
}
inline CObjectOStream& operator<<(CObjectOStream& out, CConstRef<CRelation>& pOb)
{
    pOb->WriteObject(out);
    return out;
}
inline CObjectIStream& operator>>(CObjectIStream& in, const CRelation* &pOb)
{
    pOb = CRelation::ReadObject(in);
    return in;
}
inline CObjectIStream& operator>>(CObjectIStream& in, CConstRef<CRelation>& ptr)
{
    ptr.Reset(CRelation::ReadObject(in));
    return in;
}

END_NCBI_SCOPE

#endif  // GUI_UTILS___RELATION__HPP
