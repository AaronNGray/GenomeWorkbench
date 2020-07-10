/*  $Id: project_view_factory.cpp 37251 2016-12-20 18:29:21Z katargir $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software / database is freely available
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
* Authors:  Liangshou Wu
*
* File Description:
*/


#include <ncbi_pch.hpp>

#include <gui/core/project_view_factory.hpp>
#include <gui/core/open_view_manager_impl.hpp>
#include <gui/objutils/label.hpp>

#include <objmgr/util/sequence.hpp>
#include <objects/seqalign/Seq_align.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const CViewTypeDescriptor& IProjectViewFactory::GetViewTypeDescriptor() const
{
    return GetProjectViewTypeDescriptor();
}

IOpenViewManager* IProjectViewFactory::GetOpenViewManager()
{
    return NULL;
}

bool IProjectViewFactory::IsCompatibleWith( const CObject& object, objects::CScope& scope )
{
    TConstScopedObjects objects;
    objects.push_back(SConstScopedObject(&object, &scope));

    return TestInputObjects( objects ) != 0;
}


vector<int> IProjectViewFactory::CombineInputObjects( const TConstScopedObjects& objs )
{
    size_t num_objects = objs.size();

    if( num_objects <= 0 ){
        return vector<int>();
    } else if( num_objects == 1 ){
        return vector<int>( 1, 0 );
    }

    vector<int> groups( num_objects, -1 );
    vector<CSeq_id_Handle> loc_handles;
    vector< set<CSeq_id_Handle> > aln_handles;

    for( size_t ix = 0; ix < num_objects; ix++ ){
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>( objs[ix].object.GetPointer() );
        if( loc ){
            CScope* scope = const_cast<CScope*>( objs[ix].scope.GetPointer() );
            CSeq_id_Handle idh = sequence::GetIdHandle( *loc, scope );
            idh = sequence::GetId( idh, *scope, sequence::eGetId_Canonical );

            vector<CSeq_id_Handle>::iterator found = 
                find( loc_handles.begin(), loc_handles.end(), idh )
            ;
            if( found != loc_handles.end() ){
                groups[ix] = found - loc_handles.begin();
            } else {
                groups[ix] = (int)loc_handles.size();
                loc_handles.push_back( idh );
            }

            continue;
        }

        const CSeq_align* aln = dynamic_cast<const CSeq_align*>( objs[ix].object.GetPointer() );
        if( aln ){
            CScope* scope = const_cast<CScope*>( objs[ix].scope.GetPointer() );

            int num_seqs = aln->CheckNumRows();
            if( num_seqs <= 0 ){
                continue;
            }

            set<CSeq_id_Handle> idh_set;
            for( int q = 0; q < num_seqs; q++ ){
                const CSeq_id& seq_id = aln->GetSeq_id( q );
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( seq_id );
                idh = sequence::GetId( idh, *scope, sequence::eGetId_Canonical );
                idh_set.insert( idh );
            }

            vector< set<CSeq_id_Handle> >::iterator found = 
                find( aln_handles.begin(), aln_handles.end(), idh_set )
                ;
            if( found != aln_handles.end() ){
                groups[ix] = (int)(found - aln_handles.begin() + num_objects);
            } else {
                groups[ix] = (int)(aln_handles.size() + num_objects);
                aln_handles.push_back( idh_set );
            }

            continue;
        }
    }

    return groups;
}

void IProjectViewFactory::CombineObjects( 
    const TConstScopedObjects& objects, TObjectsMap& objects_map 
){
    typedef map<vector<CSeq_id_Handle>, TConstScopedObjects> TIdsToObjectsMap;
    TIdsToObjectsMap handle_groups;
    
    ITERATE( TConstScopedObjects, obtr, objects ){

        CConstRef<CObject> object = obtr->object;
        CRef<CScope> scope = obtr->scope;

        if( object.IsNull() || scope.IsNull() ){
            continue;
        }

        vector<CSeq_id_Handle> idh_vec;

        const CSeq_id* id = dynamic_cast<const CSeq_id*>( &*object );
        if( id ){
            CSeq_id_Handle idh = sequence::GetId( *id, *scope, sequence::eGetId_Canonical );
            idh_vec.push_back( idh ? idh : CSeq_id_Handle::GetHandle( *id ) );
        }

        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>( &*object );
        if( loc ){
            //! If loc is mix, we need to iterate ids
            const CSeq_id& seq_id = *loc->GetId();
            CSeq_id_Handle idh = sequence::GetId( seq_id, *scope, sequence::eGetId_Canonical );
            idh_vec.push_back( idh ? idh : CSeq_id_Handle::GetHandle( seq_id ) );
        }

        const CSeq_align* aln = dynamic_cast<const CSeq_align*>( &*object );
        if( aln ){
            int num_seqs = aln->CheckNumRows();
            if( num_seqs <= 0 ){
                continue;
            }

            set<CSeq_id_Handle> idh_set;
            for( int seqix = 0; seqix < num_seqs; seqix++ ){
                const CSeq_id& seq_id = aln->GetSeq_id( seqix );
                CSeq_id_Handle idh = sequence::GetId( seq_id, *scope, sequence::eGetId_Canonical );
                idh_set.insert( idh ? idh : CSeq_id_Handle::GetHandle( seq_id ) );
            }

            ITERATE( set<CSeq_id_Handle>, idh_itr, idh_set ){
                idh_vec.push_back( *idh_itr );
            }

            if( idh_vec.size() == 1 ){
                idh_vec.push_back( idh_vec.front() );
            }
        }


        handle_groups[idh_vec].push_back( *obtr );
    }

    objects_map.clear();
    NON_CONST_ITERATE( TIdsToObjectsMap, sidhtr, handle_groups ){
        const vector<CSeq_id_Handle>& idh_vec = sidhtr->first;
        TConstScopedObjects& group = sidhtr->second;
        
        string label;

        bool comma = false;
        ITERATE( vector<CSeq_id_Handle>, idhtr, idh_vec ){
            if( comma ){
                label += " x ";
            } else {
                comma = true;
            }

            CSeq_id_Handle idh = sequence::GetId( *idhtr, *group[0].scope, sequence::eGetId_Best );
            label += (idh ? idh : *idhtr).GetSeqId()->GetSeqIdString( true );
        }

        if( !label.empty() ){
            objects_map[label] = group;

        } else {
            NON_CONST_ITERATE( TConstScopedObjects, scobtr, group ){
                TConstScopedObjects single;
                single.push_back( *scobtr );

                CLabel::GetLabel( *scobtr->object, &label, CLabel::eContent, scobtr->scope );

                objects_map[label].push_back( *scobtr );
                label.clear();
            }
        }
    }
}


END_NCBI_SCOPE
