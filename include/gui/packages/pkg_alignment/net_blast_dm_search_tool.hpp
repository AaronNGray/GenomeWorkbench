#ifndef PKG_ALIGNMENT___NET_BLAST_DM_SEARCH_TOOL__HPP
#define PKG_ALIGNMENT___NET_BLAST_DM_SEARCH_TOOL__HPP

/*  $Id: net_blast_dm_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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

#include <gui/core/search_tool_base.hpp>
#include <gui/core/search_form_base.hpp>

#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>

#include <gui/framework/service.hpp>

class wxBoxSizer;

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CNetBlastDMSearchFormPanel;

///////////////////////////////////////////////////////////////////////////////
/// CNetBLAST_DMSearchTool - a search tool for Data Mining Service that
/// searches and explores Net BLAST Data Source.

class CNetBLAST_DMSearchTool :
    public CSearchToolBase,
    public IServiceLocatorConsumer
{
public:
    CNetBLAST_DMSearchTool();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string  GetName() const;
    virtual string  GetDescription() const;
    /// @}

    /// @name IDMSearchTool implementation
    /// @{
    virtual CIRef<IDMSearchForm> CreateSearchForm();
    virtual bool    IsCompatible(IDataMiningContext* context);
    /// @}

    void    GetDbNames(vector<string>& name);

    /// @name IExtension implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IServiceLocatorConsumer implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* locator);
    /// @}

    /// special flags
    virtual IDMSearchTool::TUIToolFlags GetFlags(void);

protected:
    /// implementing CSearchToolBase pure virtual function
    virtual CRef<CSearchJobBase> x_CreateJob(IDMSearchQuery& query);

    CNetBLASTUIDataSource*  x_GetDataSource();

protected:
    IServiceLocator*    m_SrvLocator;
};


///////////////////////////////////////////////////////////////////////////////
/// CNetBLAST_DMSearchQuery
class CNetBLAST_DMSearchQuery :
    public CObject,
    public IDMSearchQuery
{
public:
    typedef set<CNetBlastJobDescriptor::EState> TStates;

    CNetBLAST_DMSearchQuery(const TStates& states, const string& filter_str);

    virtual string ToString() const;

public:
    /// select only Jobs with states from this set
    TStates m_States;

    /// select only Jobs that have attributes containg this string
    string  m_FilterStr;
};


///////////////////////////////////////////////////////////////////////////////
/// CTestSearchForm

class  CNetBLAST_DMSearchForm :
    public CSearchFormBase,
    public CEventHandler
{
    DECLARE_EVENT_MAP();

public:
    CNetBLAST_DMSearchForm(CNetBLAST_DMSearchTool& tool);
    virtual ~CNetBLAST_DMSearchForm();
    
    /// @name IDMSearchForm implementation
    /// @{
    virtual void    Create();
    virtual void    Init();
    virtual void    Update();

    virtual CIRef<IDMSearchQuery> ConstructQuery();

    virtual wxSizer*  GetWidget(wxWindow * parent);
    
    virtual void UpdateContexts();
    /// @}

    void SetDataSource(CNetBLASTUIDataSource* dataSource);
    
protected:  
    void            x_OnJobDescrChanged(CEvent*);

    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    IDMSearchTool * x_GetTool(void) {   return m_Tool.GetPointer(); }

protected:
    CRef<CNetBLAST_DMSearchTool>    m_Tool;
    CNetBlastDMSearchFormPanel*     m_Panel;
    CRef<CNetBLASTUIDataSource>     m_NetBlastDS;
};


///////////////////////////////////////////////////////////////////////////////
/// CNetBLAST_DMSearchJob

class  CNetBLAST_DMSearchJob : public CSearchJobBase
{
public:
    typedef pair<const char*, const char*> TPair;

    CNetBLAST_DMSearchJob(CNetBLAST_DMSearchQuery& query,
                          CNetBLASTUIDataSource& ds);

protected:
    /// @name CSearchJobBase overridables
    /// @{
    virtual bool        x_ValidateParams();
    virtual EJobState   x_DoSearch();

    virtual CObjectListTableModel* x_GetNewOLTModel() const;
    /// @}

protected:
    CRef<CNetBLAST_DMSearchQuery>   m_Query;
    CRef<CNetBLASTUIDataSource>     m_NetBlastDS;
};


/* @} */

END_NCBI_SCOPE

#endif  // #ifndef PKG_ALIGNMENT___NET_BLAST_DM_SEARCH_TOOL__HPP
