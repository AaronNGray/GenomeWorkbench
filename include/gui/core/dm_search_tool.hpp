#ifndef GUI_SERVICES___DM_SEARCH_TOOL_HPP
#define GUI_SERVICES___DM_SEARCH_TOOL_HPP

/*  $Id: dm_search_tool.hpp 31040 2014-08-22 18:27:17Z katargir $
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

#include <gui/gui_export.h>

#include <gui/objutils/object_list.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/widgets/wx/ui_tool.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

#include <gui/utils/app_job_impl.hpp>

class wxSizer;
class wxSize;
class wxWindow;


BEGIN_NCBI_SCOPE

class ISelectionService;

class IDMSearchForm;
class IDMSearchFormController;
class IDMSearchQuery;
class CDMSearchResult;
class IDataMiningContext;

class CAppJobDispatcher;


///////////////////////////////////////////////////////////////////////////////
/// IDMSearchTool
/// interface representing a single search tool in Data Mining Service. The tool
/// manages multiple Search Sessions and provides a mechanism for asynchronous
/// parallel execution of search jobs.

class IDMSearchTool : public IUITool
{
public:
    typedef enum {
        eNone       = 0,
        eAutorun    = 1,
        eCache      = 2,
        eFilter     = 4
    }  TUIFlagValue;
    
    typedef int TUIToolFlags;

    typedef struct {
        vector <objects::CSeqFeatData::ESubtype> options;
        objects::CSeqFeatData::ESubtype          selected;
    } TConversions;

    typedef pair<string, string> TFilter;

    typedef struct {
        vector <TFilter>  filters;
        vector <int>      selected;
    } TFilters;

    virtual ~IDMSearchTool() {};

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const = 0;
    virtual string GetName() const = 0;
    virtual string GetDescription() const = 0;
    /// @}

    /// factory method for creating a form representing the tool
    virtual CIRef<IDMSearchForm> CreateSearchForm() = 0;

    /// retuns true if the tool is compatible with the provided Search Context
    virtual bool IsCompatible(IDataMiningContext* context) = 0;

    /// Starts search Job, the job is identified by the query object
    virtual bool StartSearch(IDMSearchQuery& query, IDMSearchFormController& listener) = 0;
    virtual bool CancelSearch(IDMSearchFormController& listener) = 0;

    /// Pulls cached result from cache if possible (true)
    virtual bool PullSearch(IDMSearchQuery& query, IDMSearchFormController& listener) = 0;

    virtual string GetJobDescr(IDMSearchFormController& listener) = 0;

    /// special flags
    virtual TUIToolFlags GetFlags(void) = 0;

    /// list of acceptable feat types top convert and default type
    virtual TConversions & GetConversions(void) = 0;

    /// list of all tool-provided filters
    virtual const TFilters & GetFilters(void) const = 0;
    virtual TFilters& SetFilters() = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// Extension Point ID for Data Mining Tools
/// Components that need to add new search tools to Data Mining Service should
/// provide Extensions for this Extension Point. The Extensions shall
/// implement IDMSearchTool interface.

#define EXT_POINT__DATA_MINING_TOOL "data_mining_tool"


///////////////////////////////////////////////////////////////////////////////
/// CDMSearchResult - result object delivered using job progress object
///
/// @sa CDMSearchProgress
///
class CDMSearchResult : public CObject
{
public:
	CDMSearchResult();

    /// Construct result object with the data (with ptr ownership transfer)
    ///
	CDMSearchResult(const string&          query_descr, 
                    CObjectListTableModel* olt_model,
                    CObjectList*           obj_list
                    );

  	virtual ~CDMSearchResult();

    CObjectList * GetObjectList() { return m_ObjectList; }

    /// Return OLT model and forget the ptr.association 
    /// (ownership transfer)
    CObjectListTableModel* DetachOLTModel();

    const string& GetQueryDescr() const { return m_QueryDescr; }

    bool IsIncomplete() const { return m_IsIncomplete; }
    void SetIncomplete(bool incomplete = true) { m_IsIncomplete = incomplete; }

    size_t GetMaxSearchResult() const { return m_MaxSearchResult; }
    void SetMaxSearchResult(size_t maxsr) { m_MaxSearchResult = maxsr; }

private:
    CDMSearchResult& operator=(const CDMSearchResult&);
    CDMSearchResult(const CDMSearchResult & res);
private:
    string                    m_QueryDescr;
    CObjectListTableModel*    m_OLTModel;
    CRef <CObjectList>        m_ObjectList;	
    bool                      m_IsIncomplete;   ///< result set is limited or incomplete
    size_t                    m_MaxSearchResult;///< max-search result  
};


///////////////////////////////////////////////////////////////////////////////
/// CDMSearchProgress
/// CDMSearchProgress represents the specialized progress report that may
/// contain intermediate data mining results. Every CDMSearchProgress assumes
/// ownership of the results, the search job must create a separate copy of its
/// results in order to supply it with the Progress notification.
class CDMSearchProgress : public CAppJobProgress
{
public:
    CDMSearchProgress( const string& msg, CDMSearchResult* result )
        : CAppJobProgress(0.0, msg), m_Result(result)
    {}

public:
    CRef<CDMSearchResult>  m_Result;
};


///////////////////////////////////////////////////////////////////////////////
/// IDMSearchForm - interface for abstract Search From
class IDMSearchForm : public IRegSettings
{
public:
    enum ECommands  {
        eCmdDMSearch = 11000,
    };

    //TODO remove
    typedef enum {
        FEATURE_SEL_DIALOG
    } TDialogType;

    virtual ~IDMSearchForm()    {};

    virtual void    SetController(IDMSearchFormController* /*controller*/) {};

    virtual void    Create(){};
    virtual void    Init() {};
    virtual void    Update() = 0;

    /// return a widget associated with the form; the form controls the lifetime
    /// of the widget (do not delete the widget)
    virtual wxSizer *  GetWidget(wxWindow * /*parent*/) {return NULL;}

    //virtual wxSize  GetPreferredSize() {return wxSize(0,0);}

    /// called by Data Mining View to update the list of search contexts
    virtual void    UpdateContexts() = 0;

    /// ask something in dialog
    virtual void    PopupADialog(TDialogType dt) = 0;

    /// the tool associated with the form is running
    virtual bool    IsRunning() const {return false;}

    /// return a section name for the current mode or ""
    virtual string  GetCurrentModeKey() {return "";};

    /// get/set main search value, this is separate from settings
    virtual string  GetMainValue()           {return "";}
    virtual void    SetMainValue(string /*val*/) {}

    virtual void Push() = 0;
    virtual void SetDictionary(list<string>*)=0;
    virtual void SetRangeLimit(bool bLimit)=0;

    virtual CIRef<IDMSearchQuery> ConstructQuery() = 0;

    /// @name IRegSettings interface
    /// @{
    virtual void    SetRegistryPath(const string& reg_path) = 0;
    virtual void    LoadSettings() = 0;
    virtual void    SaveSettings() const = 0;
    /// @}
};


class CDataMiningService;

///////////////////////////////////////////////////////////////////////////////
/// IDMSearchFormController - interface representing an entity that controls
/// the lifecycle of search Form and Tool
class NCBI_GUICORE_EXPORT IDMSearchFormController
{
public:
    virtual ~IDMSearchFormController()    {};

    virtual CDataMiningService*  GetDataMiningService() = 0;

    /// @name IDMSearchTool callbacks
    /// @{
    virtual void    OnSearchFinished(CDMSearchResult& /*result*/)  {};
    virtual void    OnSearchProgress(CDMSearchResult& /*result*/,
                                     const string& /*prg_text*/) {};
    virtual void    OnSearchCanceled() {};
    virtual void    OnSearchFailed(const string& /*error*/) {};
    virtual void    OnSearchEnabled(bool /*benable = true*/) {}    
    /// @}
};


///////////////////////////////////////////////////////////////////////////////
/// IDMSearchQuery - abstract data mining query
class NCBI_GUICORE_EXPORT IDMSearchQuery
{
public:
    virtual ~IDMSearchQuery() {}

    /// Prepare a string representation of a query (for logging and debugging)
    virtual string ToString() const = 0;
};

/// A variant of IDMSearchQuery with a dummy search string
/// (not advised for real use)
///
class NCBI_GUICORE_EXPORT CSearchQueryNull : public IDMSearchQuery
{
public:
    virtual string ToString() const { return "NULL Search"; }
};



END_NCBI_SCOPE

#endif  // GUI_SERVICES___DM_SEARCH_TOOL_HPP
