#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_DATASOURCE_CONTEXT__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_DATASOURCE_CONTEXT__HPP

/*  $Id: seqgraphic_ds_manager.hpp 23352 2011-03-11 14:54:08Z wuliangs $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/seq_graphic/seqgraphic_data_source.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// ISGDataSourceManager uses the following extension point to obtain
/// registered seqgraphic data source factories (instances of ISGDataSource)

#define EXT_POINT__SEQGRAPHIC_DATA_SOURCE_TYPE "seqgraphic_data_source_type"

///////////////////////////////////////////////////////////////////////////////
/// ISGDSManager is seqgraphic data source manage that serves as an data source
/// context.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ISGDataSourceContext
{
public:
    virtual ~ISGDataSourceContext() {};

    /// Get a seqgraphic data source instance of the specified type.
    /// If a datasource is not sharable, it will just create one based on
    /// the input object.  If a datasource is sharable, it will return a
    /// cached instance if there is a cached one. Otherwise, a new
    /// instance will get created, cached and returned.

    /// TODO, preferably, the context should be able to distinguish different
    /// objects in addition to the types.
    virtual CIRef<ISGDataSource> GetDS(const string& type,
        SConstScopedObject& object) = 0;

    /// clear the cache if any.
    virtual void ClearCache() = 0;
};

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGDataSourceContext :
    public CObject,
    public ISGDataSourceContext
{
public:
    typedef map< string, CWeakIRef<ISGDataSource> > TDSRegistry;
    typedef CIRef<ISGDataSourceType>    TDataSourceTypeRef;
    typedef vector<TDataSourceTypeRef>  TDataSourceTypeVec;

    CSGDataSourceContext();
    virtual ~CSGDataSourceContext();

    /// @name ISGDataSouceContext interface implementation.
    /// @{
    virtual CIRef<ISGDataSource> GetDS(
        const string& type, SConstScopedObject& object);

    virtual void ClearCache();
    /// @}

private:
    /// prohibited copy constructor and assignment operator.
    CSGDataSourceContext(const CSGDataSourceContext&);
    CSGDataSourceContext& operator=(const CSGDataSourceContext&);

private:
    TDataSourceTypeVec m_DataSourceTypes; ///< All the registered data source types.
    TDSRegistry        m_DataSources;     ///< The cached data source instances.
    CFastMutex         m_Mutex;           ///< make sure data source context is thread-safe
};

END_NCBI_SCOPE


#endif	// GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_DATASOURCE_CONTEXT__HPP

