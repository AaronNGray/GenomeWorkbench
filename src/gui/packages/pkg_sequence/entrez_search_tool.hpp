#ifndef GUI_SERVICES___ENTREZ_SEARCH_TOOL_HPP
#define GUI_SERVICES___ENTREZ_SEARCH_TOOL_HPP

/*  $Id: entrez_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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
#include <misc/xmlwrapp/xmlwrapp.hpp>


class wxTextCtrl;
class wxChoice;

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */


///////////////////////////////////////////////////////////////////////////////
/// CEntrezSearchTool
class CEntrezSearchTool : public CSearchToolBase
{
public:
    CEntrezSearchTool();

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

    void    GetDbNames(vector<string>& name);

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
/// CEntrezSearchTool
class CEntrezSearchQuery :
    public CObject,
    public IDMSearchQuery
{
public:
    CEntrezSearchQuery(const string& terms, const string& db_name);

    string  GetTerms()  const { return m_Terms; }
    string  GetDbName() const { return m_DbName; }

    virtual string ToString() const { return m_Terms + " at " + m_DbName; }
protected:
    string  m_Terms;
    string  m_DbName;
};


///////////////////////////////////////////////////////////////////////////////
/// CTestSearchForm

class CEntrezSearchForm : public CSearchFormBase
{
public:
    CEntrezSearchForm(){}
    CEntrezSearchForm(CEntrezSearchTool& tool);

    virtual void SetDictionary(list<string>* /*ddd*/) {}
    virtual void SetMainValue(string /*val*/) {}
    
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
    CRef<CEntrezSearchTool>         m_Tool;
    typedef pair<string, string>    TStrPair;
    typedef vector<TStrPair>        TNamePairs;
    
    TNamePairs  m_DbNames;    /// Entrez db names
    string      m_CurrDbName; /// techical name
    wxChoice* m_DbCombo;
};


///////////////////////////////////////////////////////////////////////////////
/// CEntrezSearchJob
///
class CEntrezSearchJob : public CSearchJobBase
{
public:
    typedef pair<const char*, const char*> TPair;

    CEntrezSearchJob( CEntrezSearchQuery& query );

protected:
    /// @name CSearchJobBase overridables
    /// @{
    virtual bool x_ValidateParams();
    virtual EJobState x_DoSearch();

    virtual CObjectListTableModel* x_GetNewOLTModel() const;
    /// @}

	/// Adds an additional child node, indicating the release type (RefSeq or GenBank)
	void SetReleaseType(xml::node& ds);
protected:
    CRef<CEntrezSearchQuery> m_Query;
};



/* @} */

END_NCBI_SCOPE


/*
 * ===========================================================================
 * ===========================================================================
 */

#endif  // GUI_SERVICES___ENTREZ_SEARCH_TOOL_HPP
