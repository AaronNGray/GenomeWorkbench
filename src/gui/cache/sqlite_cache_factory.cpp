/*  $Id: sqlite_cache_factory.cpp 22981 2011-01-19 15:48:48Z kuznets $
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
 * Author: Mike DiCuccio
 *
 * File Description:  Class factory for SQLITE3 based ICache interface
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/plugin_manager_impl.hpp>
#include <corelib/ncbistr.hpp>
#include <corelib/plugin_manager_store.hpp>
#include <gui/cache/sqlite_cache.hpp>


#define NCBI_USE_ERRCODE_X   SQLITE3_Cache

BEGIN_NCBI_SCOPE

const string kSQLITE3_BlobCacheDriverName("sqlite3");


/// Class factory for SQLITE3 BLOB cache
///
/// @internal
///

class CSQLITE3_BlobCacheCF :
    public CSimpleClassFactoryImpl<ICache, CSQLITE3_Cache>
{
public:
    typedef CSimpleClassFactoryImpl<ICache, CSQLITE3_Cache> TParent;

public:
    CSQLITE3_BlobCacheCF()
      : CSimpleClassFactoryImpl<ICache, CSQLITE3_Cache>(
                                        kSQLITE3_BlobCacheDriverName, -1)
    {
    }

    /// Create instance of TDriver
    virtual
    ICache* CreateInstance(const string& driver = kEmptyStr,
                   CVersionInfo     version = NCBI_INTERFACE_VERSION(ICache),
                   const TPluginManagerParamTree* params = 0) const;
};



// List of parameters accepted by the CF

static const char* kCFParam_database  = "database";
static const char* kCFParam_cache_age = "cache_age";


////////////////////////////////////////////////////////////////////////////
//
//  CSQLITE3_BlobCacheCF


ICache* CSQLITE3_BlobCacheCF::CreateInstance(const string& driver,
           CVersionInfo                   version,
           const TPluginManagerParamTree* params) const
{
    auto_ptr<CSQLITE3_Cache> drv(new CSQLITE3_Cache());

    if ( !params ) {
        LOG_POST(Warning << "created sqlite3 cache driver, but no params provided");
        return drv.release();
    }

    const string& tree_id = params->GetKey();
    if (NStr::CompareNocase(tree_id, kSQLITE3_BlobCacheDriverName) != 0) {
        LOG_POST(Warning
          << "ICache class factory: Top level Id does not match driver name."
          << " Id = " << tree_id << " driver=" << kSQLITE3_BlobCacheDriverName
          << " parameters ignored." );

        return drv.release();
    }

    // cache configuration
    string database = GetParam(params, kCFParam_database, true);
    // hack: tweak path for 64-bit version to use alternative directory 
    if (sizeof(void*) == 8) {
        string dir, base, ext;
        CDirEntry::SplitPath(database, &dir, &base, &ext);
        dir = CDirEntry::DeleteTrailingPathSeparator(dir);
        string::value_type ch = dir[dir.length()-1];
        if (ch != '4') {
            dir.append("64");
        }
        database = CDirEntry::MakePath(dir, base, ext);
    }
    else { // for the 32-bit version switch from 64-bit cache
        NStr::ReplaceInPlace(database, "cache64", "cache");
    }

    drv->Open(database);

    const string& cache_age_str =
        GetParam(params, kCFParam_cache_age, false, "0");
    unsigned cache_age = 0;

    try {
        cache_age = NStr::StringToUInt(cache_age_str);
    } catch(const CStringException&) {
        // ignore
    }
    if ( !cache_age ) {
        cache_age = 5 * (24*60*60);
    }
    if (cache_age) {
        drv->SetTimeStampPolicy(0 /* default policy */, cache_age);
    }

    return drv.release();
}


END_NCBI_SCOPE
USING_SCOPE(ncbi);


extern "C" {

NCBI_SQLITE3_CACHE_EXPORT
void NCBI_EntryPoint_SQLITE3_BlobCache(
     CPluginManager<ICache>::TDriverInfoList&   info_list,
     CPluginManager<ICache>::EEntryPointRequest method)
{
    CHostEntryPointImpl<CSQLITE3_BlobCacheCF>::NCBI_EntryPointImpl(
                                                            info_list,
                                                            method);
}

NCBI_SQLITE3_CACHE_EXPORT
void NCBI_EntryPoint_xcache_sqlite3(
     CPluginManager<ICache>::TDriverInfoList&   info_list,
     CPluginManager<ICache>::EEntryPointRequest method)
{
    NCBI_EntryPoint_SQLITE3_BlobCache(info_list, method);
}

void SQLITE3_Register_Cache(void)
{
    RegisterEntryPoint<ICache>(NCBI_EntryPoint_SQLITE3_BlobCache);
}

}
