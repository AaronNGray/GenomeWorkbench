/*  $Id: net_blast_dm_search_tool.cpp 39744 2017-10-31 21:12:13Z katargir $
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

#include <gui/packages/pkg_alignment/net_blast_dm_search_tool.hpp>

#include <gui/packages/pkg_alignment/net_blast_dm_search_form_panel.hpp>

#include <gui/core/data_mining_service.hpp>
#include <gui/core/ui_data_source_service.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/entrez_conversion.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/extension_impl.hpp>

#include <objmgr/object_manager.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/srchctrl.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define ID_COMBOBOX 11003
#define ID_TEXT     11414

/// Register tool as an Extension
static CExtensionDeclaration decl("data_mining_tool",
                                  new CNetBLAST_DMSearchTool());

static string skColumnNames[] = { "Job Title", "Status", "RID" , "Description", "Submitted" };
static const int skNumColumns = sizeof(skColumnNames) / sizeof(string);
// BTW, it is const expression, compiler calculates it in compile time

class CNetBlastJobTableModel : public CObjectListTableModel
{
public:
    virtual int GetNumExtraColumns() const;
    virtual wxString GetExtraColumnName( int col ) const;
    virtual wxVariant GetExtraValueAt( int row, int col ) const;
};

int CNetBlastJobTableModel::GetNumExtraColumns() const
{
    return skNumColumns;
}

wxString CNetBlastJobTableModel::GetExtraColumnName( int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Invalid extra column index");
    }
    
    return ToWxString(skColumnNames[col]);
}

static string GetNetBlastJobDescription( const CNetBlastJobDescriptor& descr, int col )
{
    static CTimeFormat time_format("b D, H:m:s p");

    switch( col ){
    case 0:
        return descr.GetJobTitle();

    case 1:
        return descr.GetStateLabel( descr.GetState() );

    case 2:
        return descr.GetRID();

    case 3:
        return descr.GetDescription();

    case 4: {{
        time_t t = descr.GetSubmitTime();
        //if (t < 0) return "";
        CTime time(t);
        return time.ToLocalTime().AsString(time_format);
    }}
    default:
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Invalid job description index");
    }
}

wxVariant CNetBlastJobTableModel::GetExtraValueAt( int row, int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Invalid extra column index");
    }

    const CObject* obj = m_ObjectList->GetObject(row);
    const CNetBlastJobDescriptor* descr = dynamic_cast<const CNetBlastJobDescriptor*>(obj);

    if( descr ){
        return ToWxString( GetNetBlastJobDescription( *descr, col ) );
    }

    return ToWxString( "NULL" );
}


///////////////////////////////////////////////////////////////////////////////
/// IDMSearchTool
CNetBLAST_DMSearchTool::CNetBLAST_DMSearchTool()
:   m_SrvLocator(NULL)
{
}


IUITool* CNetBLAST_DMSearchTool::Clone() const
{
    return new CNetBLAST_DMSearchTool();
}


string CNetBLAST_DMSearchTool::GetName() const
{
    static string s_name(NET_BLAST_DM_TOOL_NAME);
    return s_name;
}


string CNetBLAST_DMSearchTool::GetDescription() const
{
    return "TODO";
}


CIRef<IDMSearchForm> CNetBLAST_DMSearchTool::CreateSearchForm()
{
    CRef<CNetBLAST_DMSearchForm> form(new CNetBLAST_DMSearchForm(*this));

    CNetBLASTUIDataSource* blast_ds = x_GetDataSource();
    _ASSERT(blast_ds);
    form->SetDataSource(blast_ds);

    return CIRef<IDMSearchForm>(form);
}


bool CNetBLAST_DMSearchTool::IsCompatible(IDataMiningContext* context)
{
    return false;
}


CRef<CSearchJobBase> CNetBLAST_DMSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    CNetBLAST_DMSearchQuery* e_query = dynamic_cast<CNetBLAST_DMSearchQuery*>(&query);
    if(e_query) {
        CNetBLASTUIDataSource* blast_ds = x_GetDataSource();

        _ASSERT(blast_ds);

        job.Reset(new CNetBLAST_DMSearchJob(*e_query, *blast_ds));
    }
    return job;
}


CNetBLASTUIDataSource* CNetBLAST_DMSearchTool::x_GetDataSource()
{
    _ASSERT(m_SrvLocator);

    CIRef<CUIDataSourceService> ds_srv =
        m_SrvLocator->GetServiceByType<CUIDataSourceService>();

    CIRef<IUIDataSource> ds = ds_srv->GetUIDataSourceByLabel("NCBI Net BLAST");

    CNetBLASTUIDataSource* blast_ds = dynamic_cast<CNetBLASTUIDataSource*>(ds.GetPointer());
    return blast_ds;
}


string  CNetBLAST_DMSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::net_blast_dm_search_tool";
}


string CNetBLAST_DMSearchTool::GetExtensionLabel() const
{
    return "NCBI Net BLAST Search Tool";
}


void CNetBLAST_DMSearchTool::SetServiceLocator(IServiceLocator* locator)
{
    m_SrvLocator = locator;
}

IDMSearchTool::TUIToolFlags CNetBLAST_DMSearchTool::GetFlags(void)
{
    return IDMSearchTool::eAutorun;
}


///////////////////////////////////////////////////////////////////////////////
/// CNetBLAST_DMSearchForm

BEGIN_EVENT_MAP(CNetBLAST_DMSearchForm, CEventHandler)
    ON_EVENT(CNetBLASTUIDataSourceEvent, CNetBLASTUIDataSourceEvent::eJobDescrChanged, &CNetBLAST_DMSearchForm::x_OnJobDescrChanged)
END_EVENT_MAP()

CNetBLAST_DMSearchForm::CNetBLAST_DMSearchForm(CNetBLAST_DMSearchTool& tool)
:   m_Tool(&tool),
    m_Panel(NULL)
{
}


CNetBLAST_DMSearchForm::~CNetBLAST_DMSearchForm()
{
    if (m_NetBlastDS)
        m_NetBlastDS->RemoveListener(this);
}


void CNetBLAST_DMSearchForm::Create()
{
}


void CNetBLAST_DMSearchForm::Init()
{
    CSearchFormBase::Init();
}

void CNetBLAST_DMSearchForm::Update()
{
    _ASSERT(m_Panel);
    m_Panel->TransferDataToWindow();
}

//static const char* kDatabaseTag = "Database";


void CNetBLAST_DMSearchForm::x_LoadSettings(const CRegistryReadView& /*view*/)
{   
    //m_CurrDbName = view.GetString("", kDatabaseTag);
}


void CNetBLAST_DMSearchForm::x_SaveSettings(CRegistryWriteView /*view*/) const
{   
}


wxSizer*  CNetBLAST_DMSearchForm::GetWidget(wxWindow * parent)
{
    if (!m_Sizer) {     
        m_Sizer = new wxBoxSizer(wxVERTICAL);
        m_Panel = new CNetBlastDMSearchFormPanel(parent);
        m_Sizer->Add(m_Panel, 1, wxGROW, 0);
    }
    return m_Sizer;
}


void CNetBLAST_DMSearchForm::UpdateContexts()
{
    CSearchFormBase::UpdateContexts();
    // context - independent
    if (m_Controller) {
        m_Controller->OnSearchEnabled(true);
    }
}

void CNetBLAST_DMSearchForm::SetDataSource(CNetBLASTUIDataSource* dataSource)
{
    if (m_NetBlastDS)
        m_NetBlastDS->RemoveListener(this);

    m_NetBlastDS.Reset(dataSource);

    if (m_NetBlastDS)
        m_NetBlastDS->AddListener(this);
}

void CNetBLAST_DMSearchForm::x_OnJobDescrChanged(CEvent*)
{
    m_Panel->RestartSearch();
}

CIRef<IDMSearchQuery> CNetBLAST_DMSearchForm::ConstructQuery()
{   
    CIRef<IDMSearchQuery> ref;
    if(m_Panel) {
        m_Panel->TransferDataFromWindow();

        CNetBLAST_DMSearchQuery::TStates states;
        if(m_Panel->GetCompleted()) {
            states.insert(CNetBlastJobDescriptor::eCompleted);
        }
        if(m_Panel->GetSubmitted()) {
            states.insert(CNetBlastJobDescriptor::eSubmitted);
        }
        if(m_Panel->GetRetrieved()) {
            states.insert(CNetBlastJobDescriptor::eRetrieved);
        }
        if(m_Panel->GetFailed()) {
            states.insert(CNetBlastJobDescriptor::eFailed);
        }
        if(m_Panel->GetExpired()) {
            states.insert(CNetBlastJobDescriptor::eExpired);
        }

        string filter_str = ToStdString(m_Panel->GetSearchValue());

        ref.Reset(new CNetBLAST_DMSearchQuery(states, filter_str));
    }
    return ref;
}

///////////////////////////////////////////////////////////////////////////////
/// CNetBLAST_DMSearchQuery

CNetBLAST_DMSearchQuery::CNetBLAST_DMSearchQuery(const TStates& states, const string& filter_str)
    :   m_States(states), 
        m_FilterStr(filter_str)   
{
}

string CNetBLAST_DMSearchQuery::ToString() const
{
    return m_FilterStr;
}

///////////////////////////////////////////////////////////////////////////////
/// CNetBLAST_DMSearchJob

CNetBLAST_DMSearchJob::CNetBLAST_DMSearchJob(CNetBLAST_DMSearchQuery& query,
                                             CNetBLASTUIDataSource& ds)
:   m_Query(&query),
    m_NetBlastDS(&ds)
{
    //m_Descr = "Query: " + m_Query->GetTerms()+ ", database = " + vis_db_name;
}


bool CNetBLAST_DMSearchJob::x_ValidateParams()
{
    return !m_Query->m_FilterStr.empty();
}


IAppJob::EJobState CNetBLAST_DMSearchJob::x_DoSearch()
{
    typedef CNetBLAST_DMSearchQuery::TStates TStates;

    /// prepare search params and search
    const TStates& states = m_Query->m_States;
    const string& filter_str = m_Query->m_FilterStr;

    
    CNetBLASTUIDataSource::TJobDescrVec  descriptors;
    m_NetBlastDS->GetJobDescriptors(descriptors);

    size_t n = descriptors.size();
    string s;

    for(  size_t i = 0;  i < n  &&  ! IsCanceled();  i++ )   {
        CNetBlastJobDescriptor& descr = *descriptors[i];

        // fisrt match states
        CNetBlastJobDescriptor::EState state = descr.GetState();
        TStates::const_iterator it = states.find(state);
        if(it != states.end())  {
            bool found = true;
            if( ! filter_str.empty())   {
                // now match text - only displayed columns
                found = false;
                for( int col = 0;  col < skNumColumns  &&  ! found;  col++  ){
                    s = GetNetBlastJobDescription( descr, col );
                    found = NStr::FindNoCase(s, filter_str) != string::npos;
                }
            }
            if(found)   {
                m_AccList.AddRow(&descr, NULL);
            }
        }
    }
    return eCompleted;
}

/*
IObjectColumnHandler* CNetBLAST_DMSearchJob::x_GetColumnHandler() const
{
    return new CNetBlastJobDescrColumnHandler();
}
*/

CObjectListTableModel* CNetBLAST_DMSearchJob::x_GetNewOLTModel() const
{
    return new CNetBlastJobTableModel();
}



END_NCBI_SCOPE
