#ifndef GUI_SERVICES___DATA_MINING_SERVICE_HPP
#define GUI_SERVICES___DATA_MINING_SERVICE_HPP

/*  $Id: data_mining_service.hpp 31795 2014-11-14 20:56:52Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <gui/framework/service.hpp>
#include <gui/framework/view.hpp>
#include <gui/framework/ui_data_source.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/core/data_mining_context.hpp>
#include <gui/core/dm_search_tool.hpp>
#include <gui/core/selection_service.hpp>


BEGIN_NCBI_SCOPE

class CDMSearchResult;

///////////////////////////////////////////////////////////////////////////////
/// IDMContextMenuContributor - contributes menu to Data Mining Service.
class IDMContextMenuContributor
{
public:
    /// returns a menu (must be deleted by the caller)
    virtual wxMenu*   GetContextMenu() = 0;

    virtual ~IDMContextMenuContributor() {};
};

class CCDataMiningEvent : public CEvent
{
public:
    enum EEventId {
        eStateChanged
    };

    CCDataMiningEvent() : CEvent(eStateChanged) {}
};

///////////////////////////////////////////////////////////////////////////////
/// CDataMiningService
class NCBI_GUICORE_EXPORT CDataMiningService :
        public CObjectEx,
        public IService,
        public IServiceLocatorConsumer,
        public CEventHandler,
        public IRegSettings
{
public:
    typedef vector<IDataMiningContext*>  TContexts;
    typedef vector<ISelectionClient*> TClients;
    typedef map<string, CIRef<IDMSearchTool> >  TNameToToolMap;
    typedef vector<IDMContextMenuContributor*>  TContributors;


    CDataMiningService();
    virtual ~CDataMiningService();

    /// the service does NOT assume ownership of the contributor
    void AddContributor(IDMContextMenuContributor* contributor);
    void RemoveContributor(IDMContextMenuContributor* contributor);
    TContributors & GetContributors() {return m_Contributors;}

     /// @name IService implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IServiceLocatorConsumer implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* locator);
    /// @}
    
    /// @name Data Mining Service API
    /// assumes ownwership of the tool instance, manager by CIRef<IDMSearchTool>
    virtual bool    RegisterTool(IDMSearchTool* tool);

    virtual string  GetDefaultToolName() const;
    virtual void    GetToolNames(vector<string>& names) const;

    virtual CIRef<IDMSearchTool>    GetToolByName(const string& name);

    virtual void AttachContext(IDataMiningContext& context);
    virtual void DetachContext(IDataMiningContext& context);
    virtual IDataMiningContext*   GetLastActiveContext();

    /// this function is called by GUI components to notify server
    virtual void    OnActiveContextChanged(IDataMiningContext* active);

    virtual void    GetContexts(TContexts& contexts);
    /// @}

    /// @name IRegSettings implementation
    /// @{
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    virtual void    SetRegistryPath(const string& path);
    /// @}

protected:
    void    x_PostStateChanged();

    void    x_UnRegisterTools();

protected:
    IServiceLocator* m_ServiceLocator;

    
    IDataMiningContext*  m_LastActiveContext;
    TContexts m_Contexts;

    TNameToToolMap  m_NameToTool;
    string  m_RegPath;

    TContributors m_Contributors;
};


END_NCBI_SCOPE


#endif  // GUI_SERVICES___DATA_MINING_SERVICE_HPP
