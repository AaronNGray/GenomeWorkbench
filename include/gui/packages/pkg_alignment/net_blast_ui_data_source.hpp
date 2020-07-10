#ifndef PKG_ALIGNMENT___NET_BLAST_UI_DATA_SOURCE__HPP
#define PKG_ALIGNMENT___NET_BLAST_UI_DATA_SOURCE__HPP

/*  $Id: net_blast_ui_data_source.hpp 38460 2017-05-11 18:58:47Z evgeniev $
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

#include <gui/packages/pkg_alignment/net_blast_job_descr.hpp>


#include <gui/core/view_extensions.hpp>
#include <gui/core/pt_item.hpp>

#include <gui/utils/app_job_dispatcher.hpp>

#include <gui/framework/ui_data_source.hpp>
#include <gui/framework/service.hpp>

#include <objects/blast/Blast4_get_databases_reply.hpp>
#include <objects/general/User_field.hpp>

#include <gui/utils/extension.hpp>
#include <gui/utils/ui_object.hpp>
#include <gui/utils/event_handler.hpp>

#include <gui/objutils/reg_settings.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CNetBLASTUIDataSourceType;
class CBlastSearchTask;

/// commands introduced by Net BLAST data source
enum ENetBlastCommands  {
    eCmdRetriveBlastJobs = 12200,
    eCmdMonitorBlastJobs,
    eCmdDeleteBlastJobs,
    eCmdExploreNetBLAST,
    eCmdLoadRIDs,
    eCmdBlastWinMask
};


#define NET_BLAST_DM_TOOL_NAME "NCBI Net BLAST"

class CNetBLASTUIDataSourceEvent : public CEvent
{
public:
    enum EEventId {
        eJobDescrChanged
    };

    CNetBLASTUIDataSourceEvent() : CEvent(eJobDescrChanged) {}
};

///////////////////////////////////////////////////////////////////////////////
/// CNetBLASTUIDataSource
class  CNetBLASTUIDataSource :
    public CObjectEx,
    public CEventHandler,
    public IExtension,
    public IServiceLocatorConsumer,
    public IUIDataSource,
    public IObjectCmdContributor,
    public IExplorerItemCmdContributor,
    public IRegSettings
{
    // these classes work in collaboration with the Data Source, in a sense,
    // they are functional aspects of the Data Source packaged as separate classes
    friend class CNetBlastJobDescriptor;
    friend class CNetBlastDSEvtHandler;
public:
    typedef vector< CRef<CNetBlastJobDescriptor> >  TJobDescrVec;
    typedef map<int, string> TTaxIdLabelMap;

    CNetBLASTUIDataSource(CNetBLASTUIDataSourceType& type);
    virtual ~CNetBLASTUIDataSource();

    /// @name IExtension implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
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

    /// @name IUIDataSource interface implementation
    /// @{
    virtual IUIDataSourceType&  GetType() const;
    virtual const IUIObject&    GetDescr();
    virtual bool    IsOpen();
    virtual bool    Open();
    virtual bool    Close();
    virtual void    EditProperties();
    virtual IUIToolManager*    GetLoadManager();

    virtual int GetDefaultCommand();
    virtual wxEvtHandler* CreateEvtHandler();
    /// @}

    /// @name IObjectCmdContributor implementation
    /// @{
    virtual IObjectCmdContributor::TContribution
                GetMenu(TConstScopedObjects& objects);
    /// @}

    /// @name IExplorerItemCmdContributor implementation
    /// @{
    virtual IExplorerItemCmdContributor::TContribution GetMenu(wxTreeCtrl& treeCtrl, PT::TItems& items);
    /// @}

    /// @name Interface for accessing Net BLAST services and objects
    /// @{

    CRef<CNetBlastJobDescriptor>    CreateJobDescriptor(const CBLASTParams& params);
    CRef<CNetBlastJobDescriptor>    CreateJobDescriptor(const string& rid);

    /// returns Job Descriptors not marked as Deleted
    void    GetJobDescriptors(TJobDescrVec& descriptors);

    CRef<CNetBlastJobDescriptor>    FindJobDescriptor(const string& rid);

    void    StartRetrievingTask(TJobDescrVec& descriptors);
    void    StartMonitoringTask(TJobDescrVec& descriptors);

    void    DeleteJobDescriptors(TJobDescrVec& descriptors);

    void    OnExplore();
    void    OnLoadRIDs();

    /// @}

protected:
    typedef list< CConstRef<objects::CUser_field> > TUserFields;

    void    x_SaveJobDescriptors();
    void    x_LoadJobDescriptors();

    /// callback - invoked by CNetBlastJobDescriptor
    void    x_OnJobDescrChanged(CNetBlastJobDescriptor& descr);

    void    x_StartTask(CBlastSearchTask& task);
    void    x_AutoStartMonitoring();

protected:
    CUIObject   m_Descr;
    mutable CRef<CNetBLASTUIDataSourceType> m_Type;

    IServiceLocator*    m_SrvLocator;
    string m_RegPath;

    bool    m_Open;

    TJobDescrVec                                 m_JobDescrs;
    mutable CFastMutex                           m_JobDescrMutex;

    //TTaxIdLabelMap                               m_TaxId2Label;
};


///////////////////////////////////////////////////////////////////////////////
/// CNetBLASTUIDataSourceType

class  CNetBLASTUIDataSourceType :
    public CObject,
    public IUIDataSourceType,
    public IExtension
{
public:
    CNetBLASTUIDataSourceType();

    /// @name IUIDataSourceType interface implementation
    /// @{
    virtual const IUIObject&    GetDescr();
    virtual IUIDataSource*  CreateDataSource();
    virtual bool    AutoCreateDefaultDataSource();
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}
protected:
    CUIObject   m_Descr;
};


/* @} */

END_NCBI_SCOPE;

#endif  // PKG_ALIGNMENT___NET_BLAST_UI_DATA_SOURCE__HPP

