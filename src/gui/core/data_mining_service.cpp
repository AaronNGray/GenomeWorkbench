/*  $Id: data_mining_service.cpp 39591 2017-10-13 14:45:28Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/core/data_mining_service.hpp>
#include <gui/core/dm_search_tool.hpp>
#include <gui/core/seqloc_search_context.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/extension_impl.hpp>

#include <algorithm>


BEGIN_NCBI_SCOPE


static CExtensionPointDeclaration decl(EXT_POINT__DATA_MINING_TOOL,
                                       "Data Mining Service - search tools");

///////////////////////////////////////////////////////////////////////////////
// CDMSearchResult

CDMSearchResult::CDMSearchResult() 
    : m_OLTModel(0),
      m_IsIncomplete(false),
      m_MaxSearchResult(0)
{
}

CDMSearchResult::~CDMSearchResult() 
{
    delete m_OLTModel;
}


CDMSearchResult::CDMSearchResult(const string&          query_descr, 
                                 CObjectListTableModel* olt_model,
                                 CObjectList*           obj_list
                                )
 : m_QueryDescr(query_descr),
   m_OLTModel(olt_model),
   m_ObjectList(obj_list),
   m_IsIncomplete(false)
{
}

CObjectListTableModel* CDMSearchResult::DetachOLTModel()
{
    CObjectListTableModel* olt = m_OLTModel;
    m_OLTModel = 0;
    return olt;
}


///////////////////////////////////////////////////////////////////////////////
/// CDataMiningService
class CDMToolSelector : public CUIToolRegistry::ISelector
{
public:
    virtual bool    Select(const IUITool& tool)
    {
        return dynamic_cast<const IDMSearchTool*>(&tool) != NULL;
    }
};


CDataMiningService::CDataMiningService():
    m_ServiceLocator(NULL),
    m_LastActiveContext(NULL)
{
}


CDataMiningService::~CDataMiningService()
{
}

void CDataMiningService::x_PostStateChanged()
{
    Post(CRef<CEvent>(new CCDataMiningEvent()));
}

void CDataMiningService::InitService()
{
    LOG_POST(Info << "Initializing Data Mining Service...");

    // get all tools from extension point
    vector< CIRef<IDMSearchTool> > tools;
    GetExtensionAsInterface("data_mining_tool", tools);

    for(  size_t i = 0;  i < tools.size();  ++i  )   {
        IDMSearchTool* dm_tool = tools[i].GetPointer();
        RegisterTool(dm_tool);
    }

    LoadSettings();

    LOG_POST(Info << "Finished initializing Data Mining Service");
}


void CDataMiningService::ShutDownService()
{
    LOG_POST(Info << "Shutting down Data Mining Service...");

    SaveSettings();

    x_UnRegisterTools();

    m_Contributors.clear();

    LOG_POST(Info << "Finished shutting down Data Mining Service");
}


void CDataMiningService::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_ServiceLocator = srv_locator;
}


#define REG_ERR_PREFIX "CDataMiningService::RegisterTool() - "

bool CDataMiningService::RegisterTool(IDMSearchTool* tool)
{
    if(tool)    {
        const string& name = tool->GetName();
        if(name.empty())    {
            ERR_POST(REG_ERR_PREFIX << "Tool name is empty");
        } else {
            TNameToToolMap::iterator it = m_NameToTool.find(name);
            if(it != m_NameToTool.end())    {
                ERR_POST(REG_ERR_PREFIX << name << "  tool already registered");
            } else {
                m_NameToTool[name] = tool;

                IServiceLocatorConsumer* consumer =
                    dynamic_cast<IServiceLocatorConsumer*>(tool);
                if(consumer)    {
                    consumer->SetServiceLocator(m_ServiceLocator);
                }
                LOG_POST(Info << "   DataMiningTool " << name << " registered.");
                return true;
            }
        }
    } else {
        ERR_POST(REG_ERR_PREFIX << "NULL argument");
    }
    return false;
}


void CDataMiningService::x_UnRegisterTools()
{
    NON_CONST_ITERATE(TNameToToolMap, it, m_NameToTool) {
        IDMSearchTool& tool = *it->second;

        IServiceLocatorConsumer* consumer =
            dynamic_cast<IServiceLocatorConsumer*>(&tool);
        if(consumer)    {
            consumer->SetServiceLocator(NULL);
        }
    }

    m_NameToTool.clear();
}


string CDataMiningService::GetDefaultToolName() const
{
    string name = (m_NameToTool.size()) ? m_NameToTool.begin()->first : "";
    return name;
}


void CDataMiningService::GetToolNames(vector<string>& names) const
{
    ITERATE(TNameToToolMap, it, m_NameToTool)   {
        names.push_back(it->first);
    }
}


CIRef<IDMSearchTool> CDataMiningService::GetToolByName(const string& name)
{
    TNameToToolMap::const_iterator it = m_NameToTool.find(name);
    return (it == m_NameToTool.end()) ? CIRef<IDMSearchTool>(NULL)
                                      : it->second;
}


void CDataMiningService::AttachContext(IDataMiningContext& context)
{
    TContexts::const_iterator it = std::find(m_Contexts.begin(), m_Contexts.end(), &context);
    if(it == m_Contexts.end())   {
        m_Contexts.push_back(&context);
        x_PostStateChanged();
    } else {
        ERR_POST("CDataMiningService::AttachContext() - client already registered");
    }
}


void CDataMiningService::DetachContext(IDataMiningContext& context)
{
    TContexts::iterator it = std::find(m_Contexts.begin(), m_Contexts.end(), &context);
    if(it != m_Contexts.end())   {
        m_Contexts.erase(it);
        if(m_LastActiveContext == &context)    {
            m_LastActiveContext = NULL;
        }
        x_PostStateChanged();
    } else {
        ERR_POST("CDataMiningService::DetachContext() - client is not attached");
    }
}


IDataMiningContext* CDataMiningService::GetLastActiveContext()
{
    return m_LastActiveContext;
}


void CDataMiningService::OnActiveContextChanged(IDataMiningContext* active)
{
    if(active != m_LastActiveContext  &&  active)    {
        m_LastActiveContext = active;
        x_PostStateChanged();
    }
}


void CDataMiningService::GetContexts(CDataMiningService::TContexts& contexts)
{
    contexts = m_Contexts;
}


void CDataMiningService::SaveSettings() const
{
}


void CDataMiningService::LoadSettings()
{
}

void CDataMiningService::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}


void CDataMiningService::AddContributor(IDMContextMenuContributor* contributor)
{
    _ASSERT(contributor);

    if(contributor == NULL) {
        ERR_POST("CDataMiningService::AddContributor()  - NULL contributor.");
    } else {
        TContributors::const_iterator it =
            std::find(m_Contributors.begin(), m_Contributors.end(), contributor);
        if(it == m_Contributors.end())  {
            m_Contributors.push_back(contributor);
            //contributor->SetMenuService(this);
        } else {
            ERR_POST("CDataMiningService::AddContributor()  - contributor already added");
        }
    }
}


void CDataMiningService::RemoveContributor(IDMContextMenuContributor* contributor)
{
    _ASSERT(contributor);

    if(contributor == NULL) {
        ERR_POST("CDataMiningService::AddContributor()  - NULL contributor.");
    } else {
        TContributors::iterator it =
            std::find(m_Contributors.begin(), m_Contributors.end(), contributor);
        if(it == m_Contributors.end())  {
            ERR_POST("CDataMiningService::RemoveContributor()  - not found");
        } else {
            // do not delete Contributors, they have their own life management
            //contributor->SetMenuService(NULL);
            m_Contributors.erase(it);
        }
    }
}


END_NCBI_SCOPE
