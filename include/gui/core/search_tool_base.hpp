#ifndef GUI_SERVICES___SEARCH_TOOL_BASE_HPP
#define GUI_SERVICES___SEARCH_TOOL_BASE_HPP

/*  $Id: search_tool_base.hpp 31040 2014-08-22 18:27:17Z katargir $
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

#include <gui/core/dm_search_tool.hpp>

#include <gui/utils/app_job.hpp>
#include <gui/utils/event_translator.hpp>
#include <gui/utils/extension.hpp>

#include <objects/seqloc/Seq_loc.hpp>


BEGIN_NCBI_SCOPE

class IDMSearchForm;
class CAppJobDispatcher;
class CSearchJobBase;


///////////////////////////////////////////////////////////////////////////////
/// CSearchToolBase

class  NCBI_GUICORE_EXPORT  CSearchToolBase :
    public CObjectEx,
    public IExtension,
    public CEventHandler,
    public IDMSearchTool
{
    DECLARE_EVENT_MAP();
public:
    enum EPatternType   {
        eExactMatch,
        eWildcard,
        eRegexp,
        eNamed
    };

    CSearchToolBase();
    virtual ~CSearchToolBase();

    /// @name IUITool interface
    /// @{
    virtual IUITool* Clone() const = 0;
    virtual string   GetName() const = 0;
    virtual string   GetDescription() const = 0;
    /// @}

    /// @name IDMSearchTool implementation
    /// @{
    virtual CIRef<IDMSearchForm>  CreateSearchForm() = 0;
    virtual bool    IsCompatible(IDataMiningContext* context) = 0;
    virtual bool    StartSearch(IDMSearchQuery& query, IDMSearchFormController& listener);
    virtual bool    CancelSearch(IDMSearchFormController& listener);
    virtual bool    PullSearch(IDMSearchQuery& query, IDMSearchFormController& listener);
    virtual string  GetJobDescr(IDMSearchFormController& form);
    /// @}

    /// CAppJobDispatcher notification handler
    void    OnAJNotification(CEvent* evt);
    void    OnProgress(CEvent* evt);    

    /// special flags
    virtual IDMSearchTool::TUIToolFlags GetFlags(void);

    // conversions
    virtual IDMSearchTool::TConversions & GetConversions(void);

    // filters
    virtual const IDMSearchTool::TFilters& GetFilters(void) const;
    virtual IDMSearchTool::TFilters& SetFilters();

    /// String representation of pattern type
    static string GetPatternTypeStr(EPatternType ptype);

protected:
    struct SSession    {
        CIRef<IDMSearchQuery>       m_Query;
        IDMSearchFormController*    m_Listener;
        int m_JobID;
        string  m_Descr;
    };

    /// create a job based on the given query, override in derived classes
    virtual CRef<CSearchJobBase> x_CreateJob(IDMSearchQuery& query) = 0;

    int x_GetSessionIndexByID(int job_id);
    int x_GetSessionIndexByQuery(IDMSearchQuery& query);
    int x_GetSessionIndexByListener(IDMSearchFormController& listener);

    virtual void    x_OnJobCompleted(CAppJobNotification& notify, SSession& session);
    virtual void    x_OnJobFailed(CAppJobNotification& notify, SSession& session);
    virtual void    x_OnJobCanceled(CAppJobNotification& notify, SSession& session);

protected:
    string  m_Name;

    int                             m_ReportPeriod;
    vector<SSession*>               m_Sessions;
    IDMSearchTool::TConversions     m_Conversions;
    IDMSearchTool::TFilters         m_Filters;
    
    CRef<CObjectList>               m_Cache;  
};


///////////////////////////////////////////////////////////////////////////////
/// CSearchQueryBase
class NCBI_GUICORE_EXPORT CSearchQueryBase :
    public CObject,
    public IDMSearchQuery
{
public:
   struct  SScopedLoc  {
        CRef<objects::CSeq_loc> m_Loc;
        CRef<objects::CScope>   m_Scope;
        string  m_ContextName;
    };
    typedef vector<SScopedLoc>   TScopedLocs;

    CSearchQueryBase(TScopedLocs& locs) {   m_ScopedLocs = locs;    }

    TScopedLocs&  GetScopedLocs()   {   return m_ScopedLocs;    }

protected:
    TScopedLocs m_ScopedLocs;
};


///////////////////////////////////////////////////////////////////////////////
/// CSearchJobBase
///
class NCBI_GUICORE_EXPORT CSearchJobBase : public CJobCancelable
{
public:
    typedef CSearchToolBase::EPatternType   TPatternType;

    CSearchJobBase();
    virtual ~CSearchJobBase();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    /// @}

    void  SetFilters(IDMSearchTool::TFilters & filters) {m_Filters = filters;}

    int GetResultsCount() const { return m_ResultsCount; }
    int GetMaxResultsCount() const { return m_MaxResultsCount; }


protected:
    /// returns true if Job params are correct, implement in derived classes
    virtual bool    x_ValidateParams() = 0;

    virtual void    x_PrepareSearch();

    /// performs searching, assuming that params are correct;
    /// Implement in derived classes
    virtual EJobState   x_DoSearch() = 0;

    /// add custom columns if needed
    virtual void    x_SetupColumns(CObjectList& obj_list);

    /// transfer data from Temp Results to Final
    virtual void    x_TransferResults();

    /// factory method creating new column handler for CObjectListWidget
    //NCBI_DEPRECATED virtual IObjectColumnHandler* x_GetColumnHandler() const;

    /// factory method creating new model for CObjectListWidget
    virtual CObjectListTableModel* x_GetNewOLTModel() const;
    
    // this function just needs info about OLT model
    friend bool CSearchToolBase::PullSearch(IDMSearchQuery& query, IDMSearchFormController& listener);

   
protected:
    /// human-readable description of the Job
    string  m_Descr;

    /// synchronizes access to the Job members
    CMutex  m_Mutex;

    CRef<CAppJobError>  m_Error;

    /// total number of results
    int m_ResultsCount;
    /// Max possible results count
    int m_MaxResultsCount;

    /// accumulates found objects before they are transferred to m_ResultList
    CObjectList m_AccList;

    /// holds temporary results, guarded by Mutex
    CRef<CDMSearchResult> m_TempResult;

    string  m_ProgressStr; /// guarded by Mutex

    CRef<CDMSearchResult> m_Result; /// holds the final results
    
    IDMSearchTool::TFilters         m_Filters;
};


END_NCBI_SCOPE

#endif  // GUI_SERVICES___SEARCH_TOOL_BASE_HPP
