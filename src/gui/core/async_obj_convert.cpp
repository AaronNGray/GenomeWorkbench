/*  $Id: async_obj_convert.cpp 43748 2019-08-28 15:47:53Z katargir $
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
 * Authors:  Roman Katargin
 *
 */

#include <ncbi_pch.hpp>

#include <serial/typeinfo.hpp>
#include <gui/core/async_obj_convert.hpp>
#include <gui/objutils/obj_convert.hpp>
#include <objects/seq/seq_id_handle.hpp>

#include <gui/widgets/wx/async_call.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static void s_EliminateDuplicates(TConstScopedObjects& objects)
{
    TConstScopedObjects result;
    set<CBioseq_Handle> seqSet;

    for (auto o : objects) {
        if (!o.scope) continue;
        const CSeq_id* seqId = dynamic_cast<const CSeq_id*>(o.object.GetPointerOrNull());
        if (!seqId) {
            const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(o.object.GetPointerOrNull());
            if (seqLoc && seqLoc->IsWhole()) {
                seqId = seqLoc->GetId();
            }
        }
        if (seqId) {
            CBioseq_Handle h = o.scope->GetBioseqHandle(*seqId);
            if (h && seqSet.find(h) == seqSet.end()) {
                result.push_back(o);
                seqSet.insert(h);
            }
            continue;
        }
        result.push_back(o);
    }

    objects = result;
}

static void s_ConvertObjectsCat(
    const CTypeInfo* typeInfo,
    const TConstScopedObjects& inputObjects,
    map<string, TConstScopedObjects>& results,
    ICanceled& canceled)
{
    results.clear();

    ITERATE(TConstScopedObjects, it, inputObjects) {
        CScope* scope = const_cast<CScope*>(it->scope.GetPointerOrNull());

        CObjectConverter::TRelationVector relations;
        CObjectConverter::FindRelations(*scope, *it->object, typeInfo->GetName(), relations);

        ITERATE(CObjectConverter::TRelationVector, itr, relations) {
            if (relations.size() > 1 && itr == relations.begin()) {
                continue;
            }

            if (canceled.IsCanceled())
                return;

            const CRelation& rel = **itr;
            string relName = rel.GetProperty(typeInfo->GetName());
            TConstScopedObjects& vec = results[relName];

            CRelation::TObjects related;
            rel.GetRelated(*scope, *it->object, related, CRelation::fConvert_All, &canceled);

            ITERATE(CRelation::TObjects, it2, related) {
                const CSerialObject* serial = dynamic_cast<const CSerialObject*>(it2->GetObjectPtr());
                if (serial && serial->GetThisTypeInfo() == typeInfo)
                    vec.push_back(SConstScopedObject(it2->GetObjectPtr(), scope));
                else
                    _ASSERT(false);
            }
        }
    }

    for (auto& rel : results)
        s_EliminateDuplicates(rel.second);
}

static void s_ConvertObjects(
    const CTypeInfo* typeInfo,
    const TConstScopedObjects& inputObjects,
    TConstScopedObjects& results,
    ICanceled& canceled)
{
    ITERATE(TConstScopedObjects, it, inputObjects) {
        CScope* scope = const_cast<CScope*>(it->scope.GetPointerOrNull());

        CObjectConverter::TRelationVector relations;
        CObjectConverter::FindRelations(*scope, *it->object,
            typeInfo->GetName(), relations);

        if (canceled.IsCanceled())
            return;

        if (relations.empty())
            continue;

        CRelation::TObjects related;
        relations.front()->GetRelated(*scope, *it->object, related, CRelation::fConvert_NoExpensive, &canceled);

        ITERATE(CRelation::TObjects, it2, related) {
            const CSerialObject* serial = dynamic_cast<const CSerialObject*>(it2->GetObjectPtr());
            if (serial && serial->GetThisTypeInfo() == typeInfo)
                results.push_back(SConstScopedObject(it2->GetObjectPtr(), scope));
            else
                _ASSERT(false);
        }
    }
    s_EliminateDuplicates(results);
}

void AsyncConvertObjects(
    const CTypeInfo* typeInfo,
    const TConstScopedObjects& inputObjects,
    map<string, TConstScopedObjects>& results)
{
    GUI_AsyncExec(
        [typeInfo, &inputObjects, &results](ICanceled& canceled)
        { s_ConvertObjectsCat(typeInfo, inputObjects, results, canceled); },
        wxT("Doing object conversions..."));

}

void AsyncConvertObjects(
    const CTypeInfo* typeInfo,
    const TConstScopedObjects& inputObjects,
    TConstScopedObjects& results)
{
    GUI_AsyncExec(
        [typeInfo, &inputObjects, &results](ICanceled& canceled)
        { s_ConvertObjects(typeInfo, inputObjects, results, canceled); },
        wxT("Doing object conversions..."));
}

END_NCBI_SCOPE
