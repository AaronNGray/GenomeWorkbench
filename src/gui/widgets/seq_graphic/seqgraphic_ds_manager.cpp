/*  $Id: seqgraphic_ds_manager.cpp 23352 2011-03-11 14:54:08Z wuliangs $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>


BEGIN_NCBI_SCOPE

CSGDataSourceContext::CSGDataSourceContext()
{
    // get Data Source Types from Extension Registry
    GetExtensionAsInterface(EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE,
        m_DataSourceTypes);
}


CIRef<ISGDataSource> CSGDataSourceContext::GetDS(
        const string& type,
        SConstScopedObject& object)
{
    CIRef<ISGDataSource> ds;

    // first try to find it in the cached registry
    {
        CFastMutexGuard LOCK(m_Mutex);
        TDSRegistry::iterator ds_iter = m_DataSources.find(type);
        if (ds_iter != m_DataSources.end()) {
            ds = ds_iter->second.Lock();
            if (ds.IsNull()) {
                // remove it from registry
                m_DataSources.erase(ds_iter);
            }
        }
    }

    if ( !ds ) {// we need to create one using the corresponding factory.
        for( size_t i = 0;  i < m_DataSourceTypes.size();  i++ ) {
            ISGDataSourceType& ds_type = *m_DataSourceTypes[i];
            if (type == typeid(ds_type).name()) {
                ds.Reset(ds_type.CreateDS(object));

                // if the data source can be shared, cache it.
                if (ds_type.IsSharable()) {
                    CFastMutexGuard LOCK(m_Mutex);
                    m_DataSources[type] = ds.GetPointer();
                }
                break;
            }
        }
    }

    if ( !ds ) {
        NCBI_THROW(CException, eInvalid, "The data source type can't be recognized!");
    }

    return ds;
}


CSGDataSourceContext::~CSGDataSourceContext()
{
}


void CSGDataSourceContext::ClearCache()
{
    CFastMutexGuard LOCK(m_Mutex);
    m_DataSources.clear();
}


END_NCBI_SCOPE
