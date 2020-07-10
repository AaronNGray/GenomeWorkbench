#ifndef GUI_UTILS___OBJ_CONVERT__HPP
#define GUI_UTILS___OBJ_CONVERT__HPP

/*  $Id: obj_convert.hpp 23138 2011-02-10 21:46:45Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */


#include <corelib/ncbiobj.hpp>
#include <objmgr/scope.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/relation.hpp>


BEGIN_NCBI_SCOPE


//
// base interface for a conversion object
//

class ITypeConverter : public CObject
{
public:
    enum {
        /// retrieve all possible identities if multiple can be shown
        fConvert_All = 0x01,

        /// retrieve the best single entity if multiple can be shown
        fConvert_Best = 0x02,

        /// do not perform any expensive tests tests (such as fetching from
        /// the network)
        fConvert_NoExpensive = 0x03,

        /// combined sets of flags
        eDefault = fConvert_Best
    };
    typedef int TFlags;

    /// struct SObject provides an interface for defining what is returned from
    /// object conversion.  Rather than simply provide the object, we can
    /// provide some contextual information as well.
    struct SObject
    {
    public:
        SObject()
        {
        }

        SObject(const CObject& obj)
            : object(&obj)
        {
        }

        SObject(const CObject& obj, const string& comm)
            : object(&obj)
            , comment(comm)
        {
        }

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

        /// a description of this object
        string comment;
    };
    typedef vector<SObject> TObjList;

    virtual void Convert(objects::CScope& scope, const CObject& obj,
                         TObjList& objs,
                         TFlags flags = eDefault) const = 0;
};



///
/// single conversion interface
///

class CConvGraph;

class NCBI_GUIOBJUTILS_EXPORT CObjectConverter
{
public:
    enum {
        /// default here has a different value than above
        /// this is intentional
        eDefault = 0
    };
    typedef int TFlags;

// New Interface
    typedef CRef<CRelation> TRelation;
    typedef vector<TRelation> TRelationVector;

    static void Register(CRelation* rel);
    static const CRelation* FindRelationByName(const string& name);

    static void FindRelations(objects::CScope& scope,
                              const CObject& obj,
                              const string& to_type_in,
                              TRelationVector& relations);

    static void DumpDotGraph(ostream& ostream, bool dumpIDs = false);

// Old Interface
    typedef ITypeConverter::TObjList TObjList;
    /// Convert an object of potentially unknown type to a set of objects of
    /// known type
    static void Convert(objects::CScope& scope, const CObject& obj,
                        const CTypeInfo* info, TObjList& objs,
                        TFlags flags = eDefault);

    /// Convert an object of potentially unknown type to a set of objects of
    /// known type
    static void Convert(objects::CScope& scope, const CObject& obj,
                        const string& type_name, TObjList& objs,
                        TFlags flags = eDefault);

    /// Determine whether an indicated conversion can be performed
    static bool CanConvert(objects::CScope& scope, const CObject& obj,
                           const CTypeInfo* type_info);

    static bool CanConvert(objects::CScope& scope, const CObject& obj,
                           const string& type_name);

    /// Register a conversion function object from a given type to
    /// another given type
    static void Register(const CTypeInfo* from_type,
                         const CTypeInfo* to_type,
                         ITypeConverter* cvt);

    static void Register(const CTypeInfo* from_type,
                         const string& to_type,
                         ITypeConverter* cvt);

    static void Register(const string& from_type,
                         const CTypeInfo* to_type,
                         ITypeConverter* cvt);

    static void Register(const string& from_type,
                         const string& to_type,
                         ITypeConverter* cvt);

    static void RegisterTypeAlias(const string& real_name,
                                  const string& alias);

    static void SetDefaultFlags(TFlags flags);
    static TFlags GetDefaultFlags(void);

private:
    /// the default conversion flags
    static TFlags sm_DefaultFlags;

    /// a list of type aliases
    /// aliases are applied to interpret the to type only!
    typedef map<string, string> TTypeAliases;
    static TTypeAliases sm_TypeAliases;

    static const string& x_NormalizeTypeName(const string& str);

    static TRelationVector sm_Relations;
    static void x_BuildGraph(CConvGraph& graph, map<string, size_t>& vertices);
    static size_t x_FindRelationByName(const string& name);
};



//
// CConvertCache holds a cache of converted objects
// This is useful in situation in which a given object may be requested
// as a given type multiple times in a row.
//

class NCBI_GUIOBJUTILS_EXPORT CConvertCache : public CObject
{
public:

     // re-typedef from CObjectConvert
     typedef CObjectConverter::TObjList TObjList;

     // convert an object using a CTypeInfo object
     virtual const TObjList& Convert(objects::CScope& scope, const CObject& obj,
                                     const CTypeInfo* info,
                                     CObjectConverter::TFlags flags = CObjectConverter::eDefault);

     // convert an object using a string-based type name
     virtual const TObjList& Convert(objects::CScope& scope, const CObject& obj,
                                     const string& type_name,
                                     CObjectConverter::TFlags flags = CObjectConverter::eDefault);



    // SCacheKey holds the information relevant for a single converted
    // object
    struct SCacheKey
    {
        SCacheKey(objects::CScope& scope, const CObject& obj,
                  const string& type)
        : m_Scope(&scope),
          m_Obj(&obj),
          m_Type(type)
        {}

        SCacheKey (const SCacheKey& key)
        : m_Scope(key.m_Scope),
          m_Obj(key.m_Obj),
          m_Type(key.m_Type)
        {}

        CConstRef<objects::CScope>  m_Scope;
        CConstRef<CObject> m_Obj;
        string             m_Type;
    };

    // functor for sorting SCacheKey objects
    struct SCacheKeySort
    {
        bool operator() (const SCacheKey& key1, const SCacheKey& key2) const;
    };

private:

    // the cache itself
    typedef map<SCacheKey, TObjList, SCacheKeySort> TCache;
    TCache m_ObjCache;

    // empty list of objects to be returned if no hits found
    TObjList m_EmptyObjList;
};



END_NCBI_SCOPE

#endif  // GUI_UTILS___OBJ_CONVERT__HPP
