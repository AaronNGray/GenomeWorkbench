#ifndef GUI_CORE___UI_DATA_SOURCE_SERVICE__HPP
#define GUI_CORE___UI_DATA_SOURCE_SERVICE__HPP

/*  $Id: ui_data_source_service.hpp 38460 2017-05-11 18:58:47Z evgeniev $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/service.hpp>
#include <gui/framework/ui_data_source.hpp>

#include <gui/objutils/reg_settings.hpp>


BEGIN_NCBI_SCOPE;

BEGIN_SCOPE(objects);
    class CUser_object;
END_SCOPE(objects);

/// Extension Point ID for IUIDataSourceType objects
///
/// Components that need to add new Data Source Types to the application
/// should provide Extensions for this Extension Point. The Extensions shall
/// implement IUIDataSourceType interface.
#define EXT_POINT__UI_DATA_SOURCE_TYPE    "ui_data_source_type"

class IExtension;

///////////////////////////////////////////////////////////////////////////////
/// CUIDataSourceService
///
class  NCBI_GUICORE_EXPORT  CUIDataSourceService :
    public CObject,
    public IService,
    public IServiceLocatorConsumer,
    public IRegSettings
{
public:
    typedef CIRef<IUIDataSourceType>    TUIDataSourceTypeRef;
    typedef vector<TUIDataSourceTypeRef>  TDataSourceTypeVec;

    typedef CIRef<IUIDataSource>    TUIDataSourceRef;
    typedef vector<TUIDataSourceRef>  TUIDataSourceVec;

    typedef CIRef<IUIToolManager>   TUIToolManagerRef;
    typedef vector<TUIToolManagerRef>  TUIToolManagerVec;

public:
    CUIDataSourceService();
    ~CUIDataSourceService();

    /// @name IService implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IServiceLocatorConsumer implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* locator);
    /// @}  

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    SaveSettings() const;
    virtual void    LoadSettings();
    /// @}

    virtual void    GetDataSourceTypes(TDataSourceTypeVec& type_vec);

    /// finds a Data Source Type by its UI label
    virtual TUIDataSourceTypeRef    GetUIDataSourceTypeByLabel(const string& label);

    /// registers a new data source
    virtual void    AddDataSource(IUIDataSource& data_source);

    /// retrieve Data Sources registered in the application
    virtual void    GetDataSources(TUIDataSourceVec& data_sources);

    /// finds a Data Source by its UI label
    virtual TUIDataSourceRef    GetUIDataSourceByLabel(const string& label);

    /// retrieve Load Managers (both system Managers the Data Sources Managers)
    virtual void    GetLoadManagers(TUIToolManagerVec& managers);

protected:
    void    x_InitDataSourceTypes();
    void    x_InitDefaultDataSources();
    void    x_InitLoadManagers();

    virtual void x_OpenDataSources();
    void x_CloseDataSources();

protected:
    IServiceLocator*    m_SrvLocator;

    string  m_RegPath;

    TDataSourceTypeVec  m_DataSourceTypes;
    TUIDataSourceVec    m_DataSources;
    TUIToolManagerVec   m_LoadManagers;

private:
    CUIDataSourceService(const CUIDataSourceService&);
    CUIDataSourceService& operator=(const CUIDataSourceService&);

    IExtension* m_OptionsDlgPage;
};


END_NCBI_SCOPE;


#endif  // GUI_CORE___UI_DATA_SOURCE_SERVICE__HPP

