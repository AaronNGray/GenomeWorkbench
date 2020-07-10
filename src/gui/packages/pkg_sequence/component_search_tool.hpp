#ifndef GUI_SERVICES___COMPONENTS_SEARCH_TOOL_HPP
#define GUI_SERVICES___COMPONENTS_SEARCH_TOOL_HPP

/*  $Id: component_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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

#include <gui/core/seqloc_search_context.hpp>
#include <gui/core/search_form_base.hpp>


class wxTextCtrl;
class wxComboBox;
class wxChoice;

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CComponentSearchTool
class CComponentSearchTool : public CSearchToolBase
{
public:
    CComponentSearchTool();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string  GetName() const;
    virtual string  GetDescription() const;
    /// @}

    /// @name IDMSearchTool implementation
    /// @{
    virtual CIRef<IDMSearchForm>  CreateSearchForm();
    virtual bool    IsCompatible(IDataMiningContext* context);
    /// @}

    /// @name IExtension implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

protected:
    /// implementing CSearchToolBase pure virtual function
    virtual CRef<CSearchJobBase> x_CreateJob(IDMSearchQuery& query);
};


///////////////////////////////////////////////////////////////////////////////
/// CComponentSearchTool
class CComponentSearchQuery
    : public CSearchQueryBase
{
public:
    CComponentSearchQuery(TScopedLocs& locs, const string& ids);

    string      GetIds() const {   return m_Ids;   }

    virtual string ToString() const { return GetIds(); }

protected:
    TScopedLocs m_ScopedLocs;
    string      m_Ids;
};


///////////////////////////////////////////////////////////////////////////////
/// CComponentSearchForm
class CComboBox;

class CComponentSearchForm : public CSearchFormBase
{
public:
    typedef CComponentSearchQuery::TScopedLocs    TScopedLocs;

    CComponentSearchForm(CComponentSearchTool& tool);
    
    /// @name IDMSearchForm implementation
    /// @{
    virtual void    Create();
    virtual void    Init();
    virtual void    Update();
    /// @}

    virtual CIRef<IDMSearchQuery>   ConstructQuery();
    virtual wxSizer *  GetWidget(wxWindow * parent);
    virtual void UpdateContexts();
protected:
    /// @name CSearchFormBase overridables
    /// @{
    //virtual string  x_GetRunningText();
    virtual void    x_CreateWidgets();

//    virtual void    x_UpdateData(EDDXDirection dir);

    virtual IDMSearchTool*  x_GetTool();
    /// @}


protected:
    virtual void    x_LoadSettings(const CRegistryReadView& /*view*/) {}
    virtual void    x_SaveSettings(CRegistryWriteView /*view*/) const {}

    CRef<CComponentSearchTool>  m_Tool;
    wxChoice*                 m_DbCombo;

    vector<ISeqLocSearchContext*>   m_SeqLocContexts;
};


///////////////////////////////////////////////////////////////////////////////
/// CComponentSearchJob
///
class CComponentSearchJob : public CSearchJobBase
{
public:
    CComponentSearchJob(CComponentSearchQuery& query);

protected:
    typedef CComponentSearchQuery::SScopedLoc     TScopedLoc;
    typedef CComponentSearchQuery::TScopedLocs    TScopedLocs;

    /// @name CSearchJobBase overridables
    /// @{
    virtual bool    x_ValidateParams();
    virtual EJobState    x_DoSearch();
    virtual void    x_SetupColumns(CObjectList& obj_list);
    /// @}

    void    x_AddToResults(CObject& obj,
                           objects::CScope& scope,
                           const string& loc_name,
                           const string& ctx_name);

protected:
    CRef<CComponentSearchQuery>   m_Query;
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_SERVICES___COMPONENTS_SEARCH_TOOL_HPP
