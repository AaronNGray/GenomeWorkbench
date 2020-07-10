/*  $Id: obj_convert.cpp 40222 2018-01-10 18:45:15Z katargir $
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

#include <ncbi_pch.hpp>
#include <corelib/ncbimtx.hpp>
#include <gui/objutils/obj_convert.hpp>
#include "convert_graph.hpp"
#include <serial/iterator.hpp>

#include <algorithm>



BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


DEFINE_STATIC_MUTEX(s_ObjCvtMutex);
CObjectConverter::TTypeAliases CObjectConverter::sm_TypeAliases;
CObjectConverter::TFlags       CObjectConverter::sm_DefaultFlags = ITypeConverter::eDefault;

CObjectConverter::TRelationVector CObjectConverter::sm_Relations;

// New Interface
//////////////////////////////////////////////////////////////////////////
//
// converter registration
//

void CObjectConverter::Register(CRelation* rel)
{
    CMutexGuard LOCK(s_ObjCvtMutex);
    _ASSERT(numeric_limits<size_t>::max() == x_FindRelationByName(rel->GetName()));
    sm_Relations.push_back(CRef<CRelation>(rel));
}

const CRelation* CObjectConverter::FindRelationByName(const string& name)
{
    CMutexGuard LOCK(s_ObjCvtMutex);

    size_t idx = x_FindRelationByName(name);
    if (idx == numeric_limits<size_t>::max())
        return 0;

    return sm_Relations[idx];
}


static bool PCompare(const CConvGraph::TPath& p1, const CConvGraph::TPath& p2)
{
    return p1.size() < p2.size();
}

static void x_BuildRelations
    (const CObjectConverter::TRelationVector& registered,
     const vector<CConvGraph::TPath>& paths,
     CObjectConverter::TRelationVector& relations)
{
    if (paths.size() == 0)
        return;

    // Find out if there are several paths with smallest length
    size_t count = 0, length;
    length = paths[0].size();
    ITERATE(vector<CConvGraph::TPath>, iter, paths) {
        if ((*iter).size() > length)
            break;
        ++count;
    }

    // Add composite relation with paths of smallest length
    CRef<CComplexRelation> composite(new CComplexRelation(true));
    if (count > 1)
        relations.push_back(CRef<CRelation>(composite));

    ITERATE(vector<CConvGraph::TPath>, iter, paths) {
        if ((*iter).size() > length)
            break;

        CRef<CRelation> rel;

        if ((*iter).size() == 1) {
            rel = registered[(*iter).front()];
        }
        else {
            CRef<CComplexRelation> complex(new CComplexRelation());
            ITERATE(CConvGraph::TPath, iter2, *iter)
                complex->AddRelation(registered[*iter2]);
            rel = complex;
        }

        relations.push_back(rel);
        if (count > 1)
            composite->AddRelation(rel);
    }
}

void CObjectConverter::FindRelations(objects::CScope& scope,
                                     const CObject& obj,
                                     const string& to_type_in,
                                     TRelationVector& relations)
{
    CMutexGuard LOCK(s_ObjCvtMutex);

    size_t idxIdentity = x_FindRelationByName(CIdentityRelation::m_Name);
    if (idxIdentity == numeric_limits<size_t>::max())
        return;

    const string& to_type = x_NormalizeTypeName(to_type_in);

    const CSerialObject* so = dynamic_cast<const CSerialObject*>(&obj);
    string from_type = so ? so->GetThisTypeInfo()->GetName() : typeid(obj).name();

    CConvGraph graph;
    map<string, size_t> vertices;
    x_BuildGraph(graph, vertices);

    map<string, size_t>::const_iterator it;
    it = vertices.find(from_type);
    size_t from_vertex = (it != vertices.end()) ?
        it->second : numeric_limits<size_t>::max();

    it = vertices.find(to_type);
    size_t to_vertex = (it != vertices.end()) ?
        it->second : numeric_limits<size_t>::max();

    //
    // check to see if our object is a serial object first
    //

    vector<CConvGraph::TPath> paths;

    if (so && (to_type == "Object" || to_type == "SerialObject")) {
        graph.FindPaths(from_vertex, to_vertex, paths);
        if (paths.size() > 0 && paths[0].size() <= 2)
            x_BuildRelations (sm_Relations, paths, relations);
        else
            relations.push_back(sm_Relations[idxIdentity]);
        return;
    }

    // If types not present among converters we still can handle identity case
    if (from_type == to_type) {
        relations.push_back(sm_Relations[idxIdentity]);
        return;
    }

    graph.FindPaths(from_vertex, to_vertex, paths);

    if (paths.size() > 0) {
        x_BuildRelations (sm_Relations, paths, relations);
        return;
    }

    //
    // Handle Container Object (CDocument, CGBProjectHandle, CProjectItem)
    //

    size_t idxContainer2Object = x_FindRelationByName("Any Container --> Object");
    if (idxContainer2Object == numeric_limits<size_t>::max())
        return;

    CRelation::TObjects related;
    sm_Relations[idxContainer2Object]->GetRelated(scope, obj, related);
    if (related.size() == 0)
        return;

    set<CConvGraph::TPath> pathSet;

    ITERATE(CRelation::TObjects, iter, related) {
        const CObject& o = **iter;
        const CSerialObject* so = dynamic_cast<const CSerialObject*>(&o);
        from_type = so ? so->GetThisTypeInfo()->GetName() : typeid(o).name();

        if (from_type == to_type) {
            pathSet.insert(CConvGraph::TPath(1, idxIdentity));
            continue;
        }

        it = vertices.find(from_type);
        from_vertex = (it != vertices.end()) ?
            it->second : numeric_limits<size_t>::max();

        vector<CConvGraph::TPath> paths2;
        graph.FindPaths(from_vertex, to_vertex, paths2);
        ITERATE(vector<CConvGraph::TPath>, iter2, paths2)
            pathSet.insert(*iter2);
    }

    if (pathSet.size() == 0)
        return;

    // paths.insert(paths.begin(), pathSet.begin(), pathSet.end());
    ITERATE(set<CConvGraph::TPath>, it, pathSet)
        paths.push_back(*it);

    NON_CONST_ITERATE(vector<CConvGraph::TPath>, it, paths)
        (*it).insert((*it).begin(), idxContainer2Object);

    sort(paths.begin(), paths.end(), PCompare);
    x_BuildRelations (sm_Relations, paths, relations);
}

void CObjectConverter::DumpDotGraph(ostream& ostream, bool dumpIDs)
{
    CMutexGuard LOCK(s_ObjCvtMutex);

    CConvGraph graph;
    map<string, size_t> vertices;
    x_BuildGraph(graph, vertices);

    ostream << "digraph {" << endl;
    if (dumpIDs) {
        ITERATE(TRelationVector, iter, sm_Relations) {
            ostream << "    \"" << vertices[(*iter)->GetTypeName()]
                 << "\" -> \"" << vertices[(*iter)->GetRelatedTypeName()] << "\";" << endl;
        }
    }
    else {
        ITERATE(TRelationVector, iter, sm_Relations) {
            ostream << "    \"" << (*iter)->GetTypeName()
                 << "\" -> \"" << (*iter)->GetRelatedTypeName() << "\";" << endl;
        }
    }
    ostream << "}" << endl;
}

// Old Interface
//////////////////////////////////////////////////////////////////////////
//
// converter registration
//

void CObjectConverter::Register(const CTypeInfo* from_type,
                                const CTypeInfo* to_type,
                                ITypeConverter* cvt)
{
    Register(from_type->GetName(), to_type->GetName(), cvt);
}


void CObjectConverter::Register(const string& from_type,
                                const CTypeInfo* to_type,
                                ITypeConverter* cvt)
{
    Register(from_type, to_type->GetName(), cvt);
}


void CObjectConverter::Register(const CTypeInfo* from_type,
                                const string& to_type,
                                ITypeConverter* cvt)
{
    Register(from_type->GetName(), to_type, cvt);
}

class NCBI_GUIOBJUTILS_EXPORT CRelationTypeConverterAdapter : public CRelation
{
public:
    CRelationTypeConverterAdapter() {}
    CRelationTypeConverterAdapter(const string& from_type,
                                  const string& to_type,
                                  ITypeConverter* typeConverter) :
        m_FromType(from_type),
        m_ToType(to_type),
        m_TypeConverter(typeConverter) {}

    virtual string    GetName() const
    {
        const CObject& o = m_TypeConverter.GetObject();
        return typeid(o).name();
    }
    virtual string    GetDescription() const { return "ITypeConverterAdaptor"; }

    virtual string    GetTypeName() const { return m_FromType; }
    virtual string    GetRelatedTypeName() const { return m_ToType; }

    virtual void      GetRelated(objects::CScope& scope, const CObject& obj,
                                 TObjects& related,
                                 TFlags flags = eDefault,
                                 ICanceled* cancel = NULL) const;

    virtual void Dump(ostream& ostream) const
    {
        const CObject& o = m_TypeConverter.GetObject();
        ostream << typeid(o).name() << endl;
    }

private:
    string m_FromType;
    string m_ToType;
    CConstRef<ITypeConverter> m_TypeConverter;
};

void CRelationTypeConverterAdapter::GetRelated(
        objects::CScope& scope,
        const CObject& obj,
        TObjects& related,
        TFlags flags,
        ICanceled*) const
{
    ITypeConverter::TObjList obj_list;
    m_TypeConverter->Convert(scope, obj, obj_list, flags);
    ITERATE(ITypeConverter::TObjList, iter, obj_list) {
        related.push_back(SObject(iter->GetObject()));
    }
}

void CObjectConverter::Register(const string& from_type,
                                const string& to_type,
                                ITypeConverter* cvt)
{
    Register(new CRelationTypeConverterAdapter(from_type, to_type, cvt));
}

void CObjectConverter::RegisterTypeAlias(const string& from_type,
                                         const string& alias)
{
    CMutexGuard LOCK(s_ObjCvtMutex);
    sm_TypeAliases[alias] = from_type;
}


void CObjectConverter::SetDefaultFlags(TFlags flags)
{
    sm_DefaultFlags = flags;
}


CObjectConverter::TFlags CObjectConverter::GetDefaultFlags()
{
    return sm_DefaultFlags;
}


//////////////////////////////////////////////////////////////////////////
//
// object conversion
//

bool CObjectConverter::CanConvert(CScope& scope, const CObject& obj,
                                  const CTypeInfo* info)
{
    return CanConvert(scope, obj, info->GetName());
}


bool CObjectConverter::CanConvert(CScope& scope, const CObject& obj,
                                  const string& to_type_in)
{
    TRelationVector relations;
    FindRelations(scope, obj, to_type_in, relations);
    return (relations.size() > 0);
}


void CObjectConverter::Convert(CScope& scope, const CObject& obj,
                               const CTypeInfo* info, TObjList& objs,
                               TFlags flags)
{
    Convert(scope, obj, info->GetName(), objs, flags);
}


void CObjectConverter::Convert(CScope& scope, const CObject& obj,
                               const string& to_type_in, TObjList& objs,
                               TFlags flags)
{
    TRelationVector relations;
    FindRelations(scope, obj, to_type_in, relations);

#if 0
    CNcbiOfstream ostr("C:\\temp\\conversions_log.txt", IOS_BASE::out | IOS_BASE::app);
    ostr << endl << endl;

    const CSerialObject* so = dynamic_cast<const CSerialObject*>(&obj);
    string from_type = so ? so->GetThisTypeInfo()->GetName() : typeid(obj).name();
    ostr << "Conversion: " << from_type << " -- > " << to_type_in << endl << "{" << endl;

    //int count = min((size_t)3, relations.size());
    int count = relations.size();
    if (count == 0)
        ostr << "*** Conversion not found ***" << endl;
    else {
        for (int i = 0; i < count; ++i) {
            relations[i]->Dump(ostr);
            ostr << endl;
        }
    }

    ostr << "}" << endl;
#endif

    if (relations.size() == 0)
        return;

    CRelation::TObjects related;
    relations[0]->GetRelated(scope, obj, related, flags);

    ITERATE(CRelation::TObjects, iter, related) {
        objs.push_back(ITypeConverter::SObject(iter->GetObject(),
                                               iter->GetComment()));
    }
}

void CObjectConverter::x_BuildGraph(CConvGraph& graph, map<string, size_t>& vertices)
{
    ITERATE(TRelationVector, iter, sm_Relations) {
        size_t from_index, to_index;
        string from_type = (*iter)->GetTypeName();
        string to_type = (*iter)->GetRelatedTypeName();

        map<string, size_t>::const_iterator it = vertices.find(from_type);
        if (it == vertices.end()) {
            from_index = vertices.size();
            vertices[from_type] = from_index;
        }
        else
            from_index = it->second;

        it = vertices.find(to_type);
        if (it == vertices.end()) {
            to_index = vertices.size();
            vertices[to_type] = to_index;
        }
        else
            to_index = it->second;

        graph.add_edge(from_index, to_index);
    }
}

size_t CObjectConverter::x_FindRelationByName(const string& name)
{
    ITERATE(TRelationVector, iter, sm_Relations) {
        if ((*iter)->GetName() == name) {
            return iter - sm_Relations.begin();
        }
    }
    return numeric_limits<size_t>::max();
}

const string& CObjectConverter::x_NormalizeTypeName(const string& str)
{
    TTypeAliases::const_iterator iter = sm_TypeAliases.find(str);
    if (iter != sm_TypeAliases.end()) {
        return iter->second;
    }

    return str;
}

//////////////////////////////////////////////////////////////////////////
//
//  conversion cache
//
const CConvertCache::TObjList&
CConvertCache::Convert(CScope& scope,
                       const CObject& obj,
                       const CTypeInfo* info,
                       CObjectConverter::TFlags flags)
{
    if (info) {
        return Convert(scope, obj, info->GetName(), flags);
    }
    return m_EmptyObjList;
}


const CConvertCache::TObjList&
CConvertCache::Convert(CScope& scope,
                       const CObject& obj,
                       const string& type_name,
                       CObjectConverter::TFlags flags)
{
    SCacheKey key(scope, obj, type_name);

    CConvertCache::TCache::iterator pos;
    if ((pos = m_ObjCache.find(key)) == m_ObjCache.end()) {
        TCache::value_type val(key, m_EmptyObjList);
        CObjectConverter::Convert(scope, obj, type_name, val.second, flags);
        pair<CConvertCache::TCache::iterator, bool>
            r(m_ObjCache.insert(val));

        if (!r.second) {
            return m_EmptyObjList;
        }
        pos = r.first;
    }

    return pos->second;
}


bool CConvertCache::SCacheKeySort::operator() (const SCacheKey& key1,
                                               const SCacheKey& key_) const
{
    if (key1.m_Scope.GetPointer() < key_.m_Scope.GetPointer()) {
        return true;
    }

    if (key1.m_Scope.GetPointer() >  key_.m_Scope.GetPointer()) {
        return false;
    }

    // key1.m_Scope == key_.m_Scope
    if (key1.m_Obj.GetPointer() < key_.m_Obj.GetPointer()) {
        return true;
    }
    if (key1.m_Obj.GetPointer() > key_.m_Obj.GetPointer()) {
        return false;
    }

    // (key1.m_Scope == key_.m_Scope) && (key1.m_Obj == key_.m_Obj)
    return (NStr::CompareCase(key1.m_Type, key_.m_Type) < 0);
};

END_NCBI_SCOPE
