#ifndef GUI_SERVICES___SNP_SEARCH_TOOL_HPP
#define GUI_SERVICES___SNP_SEARCH_TOOL_HPP

/*  $Id: snp_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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
 * Authors:  Andrey Yazhuk, modified for SNP by Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/core/search_tool_base.hpp>
#include <gui/core/search_form_base.hpp>

class wxTextCtrl;
class wxChoice;

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */


///////////////////////////////////////////////////////////////////////////////
/// CSNPSearchTool
class CSNPSearchTool : public CSearchToolBase
{
public:
    CSNPSearchTool();

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
/// CSNPSearchTool
class CSNPSearchQuery :
    public CObject,
    public IDMSearchQuery
{
public:
    CSNPSearchQuery(const string& terms);

    string  GetTerms()  const { return m_Terms; }

    virtual string ToString() const { return m_Terms; }
protected:
    string  m_Terms;
};


///////////////////////////////////////////////////////////////////////////////
/// CSNPSearchForm

class CSNPSearchForm : public CSearchFormBase
{
public:
    CSNPSearchForm(){}
    CSNPSearchForm(CSNPSearchTool& tool);
    
    /// @name IDMSearchForm implementation
    /// @{
    virtual void    Create();
    virtual void    Init();
    virtual void    Update();

    virtual CIRef<IDMSearchQuery> ConstructQuery();

    virtual wxSizer *  GetWidget(wxWindow * parent);
    
    virtual void UpdateContexts();

protected:  
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    IDMSearchTool * x_GetTool(void) {return m_Tool.GetPointer();}

protected:
    CRef<CSNPSearchTool>         m_Tool;
    typedef pair<string, string>    TStrPair;
    typedef vector<TStrPair>        TNamePairs;
};


///////////////////////////////////////////////////////////////////////////////
/// CSNPSearchJob
///
class CSNPSearchJob : public CSearchJobBase
{
public:
    typedef pair<const char*, const char*> TPair;

    CSNPSearchJob( CSNPSearchQuery& query );

protected:
    /// @name CSearchJobBase overridables
    /// @{
    virtual bool x_ValidateParams();
    virtual EJobState x_DoSearch();
    virtual void x_SetupColumns(CObjectList& obj_list);

    virtual CObjectListTableModel* x_GetNewOLTModel() const;
    /// @}

protected:
    CRef<CSNPSearchQuery> m_Query;
};


/* @} */

END_NCBI_SCOPE


/*
 * ===========================================================================
 * ===========================================================================
 */

#endif  // GUI_SERVICES___SNP_SEARCH_TOOL_HPP
