/*  $Id: ui_data_source_service.cpp 44933 2020-04-21 18:56:36Z asztalos $
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
 */

#include <ncbi_pch.hpp>

#include <gui/core/ui_data_source_service.hpp>

#include <gui/core/ui_project_load_manager.hpp>
#include <gui/core/ui_file_load_manager.hpp>
#include <gui/core/ui_gcassembly_load_manager.hpp>
#include <gui/core/data_sources_page.hpp>

#include <gui/framework/options_dlg_extension.hpp>
#include <gui/framework/app_task_impl.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/utils/extension_impl.hpp>

#include <objects/general/User_object.hpp>

#include <corelib/ncbi_system.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CUIDataSourceService
CUIDataSourceService::CUIDataSourceService()
:   m_SrvLocator(NULL)
{
}


CUIDataSourceService::~CUIDataSourceService()
{
}


void CUIDataSourceService::InitService()
{
    LOG_POST(Info << "Initializing Data Source Service...");

    x_InitDataSourceTypes();

    LoadSettings();

    x_InitDefaultDataSources();

    x_OpenDataSources();

    x_InitLoadManagers();

    // contribute Data Source page to Options dialog
    CIRef<IExtensionRegistry> registry = CExtensionRegistry::GetInstance();
    m_OptionsDlgPage = new CDataSourcesOptionsDlgExtension(this);
    registry->AddExtension(EXT_POINT__OPTIONS_DLG_EXTENSION, *m_OptionsDlgPage);

    LOG_POST(Info << "Finished initializing Data Source Service");
}


void CUIDataSourceService::x_InitDataSourceTypes()
{
    /// get Data Source Types from Extension Registry
    GetExtensionAsInterface("ui_data_source_type", m_DataSourceTypes);

    for( size_t i = 0;  i < m_DataSourceTypes.size();  i++ ) {
        IUIDataSourceType& ds_type = *m_DataSourceTypes[i];
        string name = ds_type.GetDescr().GetLabel();
        LOG_POST(Info << "Registered data source type \"" <<  name << "\"");
    }
}


void CUIDataSourceService::x_InitDefaultDataSources()
{
    for( size_t i = 0;  i < m_DataSourceTypes.size();  i++ ) {
        IUIDataSourceType& ds_type = *m_DataSourceTypes[i];

        if(ds_type.AutoCreateDefaultDataSource())   {
            // look if we have a Data Source of this type already
            bool found = false;
            for( size_t j = 0;  ! found  &&  j < m_DataSources.size();  j++ ) {
                IUIDataSource& ds = *m_DataSources[j];
                found = (&ds.GetType() == &ds_type);
            }

            if( ! found)    {
                LOG_POST(Info << "Creating default Data Source for type \"" <<
                         ds_type.GetDescr().GetLabel()  << "\" ...");

                CIRef<IUIDataSource> ds(ds_type.CreateDataSource());

                /// if this source implements IRegSettings - set its registry path
                IRegSettings* rs = dynamic_cast<IRegSettings*>(&*ds);
                if( rs ){
                    string help_id = ds->GetDescr().GetHelpId();
                    if( help_id.empty() ){
                        help_id = "DefaultUIDS";
                    }
                    string reg_path = m_RegPath + "." + help_id;
                    rs->SetRegistryPath( reg_path );
                }

                Consumer_SetLocator(ds.GetPointer(), m_SrvLocator);
                AddDataSource(*ds);
            }
        }
    }
}


void CUIDataSourceService::x_InitLoadManagers()
{
    /// TODO
    // create System Load Managers
    m_LoadManagers.push_back(TUIToolManagerRef(new CProjectLoadManager()));

    CRef<CFileLoadManager> fileManager(new CFileLoadManager());
    fileManager->LoadRegisteredManagers();
    m_LoadManagers.push_back(TUIToolManagerRef(fileManager.GetPointer()));
    m_LoadManagers.push_back(TUIToolManagerRef(new CGCAssemblyLoadManager()));

    //TODO
    // obtain managers coming from Data Sources
    for( size_t i = 0;  i < m_DataSources.size();  i++ ) {
        CIRef<IUIDataSource> ds = m_DataSources[i];
        CIRef<IUIToolManager> manager(ds->GetLoadManager());
        if(manager) {
            m_LoadManagers.push_back(manager);
        }
    }

    for( size_t i = 0;  i < m_LoadManagers.size();  i++ ) {
        string name = m_LoadManagers[i]->GetDescriptor().GetLabel();
        LOG_POST(Info << "Registered data load manager \"" <<  name << "\"");
    }
}


void CUIDataSourceService::ShutDownService()
{
    SaveSettings();

    if (m_OptionsDlgPage) {
        CIRef<IExtensionRegistry> registry = CExtensionRegistry::GetInstance();
        registry->RemoveExtension(*m_OptionsDlgPage);
        m_OptionsDlgPage = 0;
    }

    x_CloseDataSources();

    m_LoadManagers.clear();
    m_DataSources.clear();
    m_DataSourceTypes.clear();
}


void CUIDataSourceService::SetServiceLocator(IServiceLocator* locator)
{
    m_SrvLocator = locator;
}


void CUIDataSourceService::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CUIDataSourceService::SaveSettings() const
{
    _ASSERT(! m_RegPath.empty());
}


void CUIDataSourceService::LoadSettings()
{
    _ASSERT(! m_RegPath.empty());
}


void CUIDataSourceService::AddDataSource(IUIDataSource& data_source)
{
    CIRef<IUIDataSource> ds_ref(&data_source);
    string ds_name = data_source.GetDescr().GetLabel();

    TUIDataSourceVec::iterator it =
        std::find(m_DataSources.begin(), m_DataSources.end(), ds_ref);

    if(it != m_DataSources.end())   {
        string msg = "Data Source " + ds_name + " is already registered";
        NCBI_THROW(CException, eUnknown, msg);
    } else {
        m_DataSources.push_back(ds_ref);
        LOG_POST(Info << "Registered data source \"" <<  ds_name << "\"");
    }
}


void CUIDataSourceService::GetDataSourceTypes(TDataSourceTypeVec& type_vec)
{
    type_vec = m_DataSourceTypes;
}


CUIDataSourceService::TUIDataSourceTypeRef
    CUIDataSourceService::GetUIDataSourceTypeByLabel(const string& label)
{
    for( size_t i = 0;  i < m_DataSourceTypes.size();  i++) {
        TUIDataSourceTypeRef& ds_type = m_DataSourceTypes[i];
        if(ds_type->GetDescr().GetLabel() == label) {
            return ds_type;
        }
    }
    return TUIDataSourceTypeRef();
}


void CUIDataSourceService::GetDataSources(TUIDataSourceVec& data_sources)
{
    data_sources = m_DataSources;
}


CUIDataSourceService::TUIDataSourceRef
    CUIDataSourceService::GetUIDataSourceByLabel(const string& label)
{
    for( size_t i = 0;  i < m_DataSources.size();  i++) {
        TUIDataSourceRef& ds = m_DataSources[i];
        if(ds->GetDescr().GetLabel() == label) {
            return ds;
        }
    }
    return TUIDataSourceRef();
}


void CUIDataSourceService::GetLoadManagers(TUIToolManagerVec& managers)
{
    managers = m_LoadManagers;
}


static void sReportDSExceptionToLog(const string& prefix,
                                        const string& ds_name, const string& what)
{
    LOG_POST(Error  << prefix << ds_name <<":\n" << what);
}

static const char* kOpenErr = "Failed to open datasource ";
static const char* kCloseErr = "Failed to close datasource ";


void CUIDataSourceService::x_OpenDataSources()
{
    LOG_POST(Info << "Opening Data Sources...");

    for( size_t i = 0;  i < m_DataSources.size();  i++ ) {
        IUIDataSource& ds = *m_DataSources[i];
        string ds_name = ds.GetDescr().GetLabel();

        try {
            _ASSERT(! ds.IsOpen());
            ds.Open();
        } catch(CException& e) {
            sReportDSExceptionToLog(kOpenErr, ds_name, e.GetMsg());
        }
        catch(std::exception& e) {
            sReportDSExceptionToLog(kOpenErr, ds_name, e.what());
        }
#ifndef _DEBUG
        catch (...) {
            sReportDSExceptionToLog(kOpenErr, ds_name, "Unknown error.");
        }
#endif
    }
    LOG_POST(Info << "Finished closing Data Sources");
}


void CUIDataSourceService::x_CloseDataSources()
{
    LOG_POST(Info << "Closing Data Sources...");

    for( size_t i = m_DataSources.size();  i > 0 ;  ) {
        IUIDataSource& ds = *m_DataSources[--i];
        string ds_name = ds.GetDescr().GetLabel();

        try {
            if(ds.IsOpen()) {
                ds.Close();
            }
        } catch(CException& e) {
            sReportDSExceptionToLog(kCloseErr, ds_name, e.GetMsg());
        }
        catch(std::exception& e) {
            sReportDSExceptionToLog(kCloseErr, ds_name, e.what());
        }
    }

    LOG_POST(Info << "Finished closing Data Sources");
}


END_NCBI_SCOPE
