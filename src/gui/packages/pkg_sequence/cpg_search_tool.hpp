#ifndef GUI_SERVICES___CPG_SEARCH_TOOL_HPP
#define GUI_SERVICES___CPG_SEARCH_TOOL_HPP

/*  $Id: cpg_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <gui/core/seqloc_search_context.hpp>
#include <gui/core/search_form_base.hpp>

#include <util/xregexp/regexp.hpp>
#include <util/range_coll.hpp>

#include <objmgr/seq_vector.hpp>



class wxTextCtrl;
class wxChoice;
class wxSearchCtrl;
class wxFlexGridSizer;


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */


///////////////////////////////////////////////////////////////////////////////
/// CCpgSearchTool
class CCpgSearchTool : public CSearchToolBase
{
public:
    CCpgSearchTool();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string    GetName() const;
    virtual string    GetDescription() const;
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
/// CCpgSearchTool
class CCpgSearchQuery :
    public CSearchQueryBase
{
public:
    CCpgSearchQuery(TScopedLocs& locs,
                        const string&  xSizeOfSlidingWindow,
                        const string&  xMinLenOfIsland,
                        const string&  xMinGC,
                        const string&  xMinPercentage,
                        const string&  xAdjMergeThreshold);

    string          GetSizeOfSlidingWindow(){   return m_SizeOfSlidingWindow;   }
    string          GetMinLenOfIsland()     {   return m_MinLenOfIsland;        }
    string          GetMinGC()              {   return m_MinGC;                 }
    string          GetMinPercentage()      {   return m_MinPercentage;         }
    string          GetAdjMergeThreshold()      {   return m_AdjMergeThreshold;     }

    virtual string ToString() const;

protected:      
    string m_SizeOfSlidingWindow;
    string m_MinLenOfIsland;
    string m_MinGC;
    string m_MinPercentage;
    string m_AdjMergeThreshold;
};


///////////////////////////////////////////////////////////////////////////////
/// CCpgSearchForm
class CComboBox;

class CCpgSearchForm : public CSearchFormBase
{
public:
    typedef CCpgSearchQuery::TScopedLocs    TScopedLocs;    

    CCpgSearchForm(CCpgSearchTool& tool);
    ~CCpgSearchForm();

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
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    virtual IDMSearchTool*  x_GetTool();            
protected:
    CRef<CCpgSearchTool>    m_Tool;

    wxChoice * m_DbCombo;
    wxTextCtrl * m_SizeOfSlidingWindowCtl;
    wxTextCtrl * m_MinLenOfIslandCtl;
    wxTextCtrl * m_MinGCCtl;
    wxTextCtrl * m_MinPercentageCtl;
    wxTextCtrl * m_AdjMergeThresholdCtl;

    vector<ISeqLocSearchContext*>   m_SeqLocContexts;

    string m_SizeOfSlidingWindow;
    string m_MinLenOfIsland;
    string m_MinGC;
    string m_MinPercentage;
    string m_AdjMergeThreshold;
    
};



///////////////////////////////////////////////////////////////////////////////
/// CCpgSearchJob
///
class CCpgSearchJob : public CSearchJobBase
{
public:
    CCpgSearchJob(CCpgSearchQuery& query);

protected:
    typedef CCpgSearchQuery::SScopedLoc     TScopedLoc;
    typedef CCpgSearchQuery::TScopedLocs    TScopedLocs;
    typedef CRangeCollection<TSeqPos>       TRangeCollection;

    /// @name CSearchJobBase overridables
    /// @{
    virtual bool    x_ValidateParams();
    virtual IAppJob::EJobState    x_DoSearch();
    virtual void    x_SetupColumns(CObjectList& obj_list);
    virtual CObjectListTableModel* x_GetNewOLTModel() const;
    /// @}

    void    x_AddToResults(CObject& obj,
                           objects::CScope& scope,
                           const string& loc_name,
                           int   start,
                           int   stop,
                           int   length,
                           int   gc,  
                           const string& ctx_name);

protected:
    CRef<CCpgSearchQuery>   m_Query;
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_SERVICES___CPG_SEARCH_TOOL_HPP
