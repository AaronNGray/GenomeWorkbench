/*  $Id: obj_conv_profile.cpp 34678 2016-02-01 22:03:51Z katargir $
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
 * File Description:
 *
 */


#include "/usr/local/valgrind-3.5.0/include/valgrind/callgrind.h"

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbitime.hpp>
#include <serial/objistr.hpp>

#include <objmgr/object_manager.hpp>
#include <gui/objutils/obj_convert.hpp>
#include <gui/objutils/init.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
//  CObjConvProfile::


class CObjConvProfile : public CNcbiApplication
{
public:
    CObjConvProfile();

private:
    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);

    void x_CreateRelatedMap(TConstScopedObjects& input, const string& type, TRelatedObjectsMap& map);

    CRef<CObjectManager> m_ObjectManager;
};


CObjConvProfile::CObjConvProfile()
{
}


/////////////////////////////////////////////////////////////////////////////
//  Init test for all different types of arguments


void CObjConvProfile::Init(void)
{
    // Create command - line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Math library test app");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}



/////////////////////////////////////////////////////////////////////////////


int CObjConvProfile::Run(void)
{

    m_ObjectManager = CObjectManager::GetInstance();
    
    auto_ptr<CObjectIStream> istr(
            CObjectIStream::Open(eSerial_AsnText,
               "/net/snowman/vol/export2/win-coremake/App/Ncbi/gbench/data/test_for_cleanup.asn"));

    string header = istr->ReadFileHeader();
    TTypeInfo type_info = CClassTypeInfo::GetClassInfoByName(header);
    CRef<CSerialObject> so((CSerialObject*)type_info->Create());
    istr->ReadObject(so.GetPointer(), type_info);

    CRef<CScope> scope(new CScope(*m_ObjectManager));
    scope->AddDefaults();

    TConstScopedObjects input;
    input.push_back(SConstScopedObject(*so, *scope));


    CStopWatch sw;
    sw.Start();
            
    TRelatedObjectsMap map;
    CALLGRIND_START_INSTRUMENTATION;
    x_CreateRelatedMap(input, CSeq_loc::GetTypeInfo()->GetName(), map); 
    CALLGRIND_STOP_INSTRUMENTATION;
    CALLGRIND_DUMP_STATS;

    string t = NStr::DoubleToString(sw.Elapsed(), 3);
    cout << "Converted in " << t << " sec" << endl;

    ITERATE(TRelatedObjectsMap, it, map) {
        cout << it->first << ": " << it->second.size() << " Seq-locs" << endl;
    }
    
    return 0;
}


void CObjConvProfile::x_CreateRelatedMap( 
        TConstScopedObjects& input,
        const string& type,
        TRelatedObjectsMap& map 
        ){
    map.clear();

    set<const CObject*> objects_used;
    ITERATE( TConstScopedObjects, it, input ){
        const CObject* p_obj = it->object;
        objects::CScope* p_scope = const_cast<objects::CScope*>(it->scope.GetPointer());
        if( !p_obj || !p_scope || objects_used.find( p_obj ) != objects_used.end() ){
            continue;
        }
        const CObject& object = *p_obj;
        objects::CScope& scope = *p_scope;

        CObjectConverter::TRelationVector relations;
        CObjectConverter::FindRelations( scope, object, type, relations );

        ITERATE( CObjectConverter::TRelationVector, itr, relations ){
            if( relations.size() > 1 && itr == relations.begin() ){
                continue;
            }

            const CRelation& rel = **itr;
            string relName = rel.GetProperty( type );

            CRelation::TObjects related;
            rel.GetRelated( scope, object, related, CRelation::fConvert_NoExpensive );

            if( !related.empty() ){
                TConstScopedObjects& rel_vec = map[relName];

                ITERATE( CRelation::TObjects, rel_itr, related ){
                    rel_vec.push_back( 
                            SConstScopedObject( rel_itr->GetObject(), scope ) 
                            );
                }
            }
        }
    }
}

//  Cleanup


void CObjConvProfile::Exit(void)
{
    SetDiagStream(0);
}

END_NCBI_SCOPE

USING_SCOPE(ncbi);

/////////////////////////////////////////////////////////////////////////////
//  MAIN


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CObjConvProfile().AppMain(argc, argv);
}
