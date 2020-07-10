/*  $Id: snp_search_tool.cpp 41475 2018-08-02 20:01:15Z rudnev $
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

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence/snp_search_tool.hpp>
#include <gui/core/data_mining_service.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/snp_gui.hpp>

#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/utils/extension_impl.hpp>

#include <objmgr/object_manager.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>

#include <gui/widgets/wx/wx_utils.hpp>

#include <connect/ncbi_conn_stream.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>

#include <objects/variation/Variation.hpp>
#include <objects/variation/VariantPlacement.hpp>
#include <objects/seqfeat/VariantProperties.hpp>

#include <sstream>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


//!! known problems:
// - create feature does not work

#define ID_COMBOBOX 11003 //!! ?? which IDs to use, is there any significance in these particular values?
#define ID_TEXT     11414


///////////////////////////////////////////////////////////////////////////////
/// CSNPTableModel


class CSNPTableModel : public CObjectListTableModel
{
public:
    CSNPTableModel( );

    virtual wxVariant GetExtraValueAt( int row, int col ) const;
};


///////////////////////////////////////////////////////////////////////////////
/// IDMSearchTool
CSNPSearchTool::CSNPSearchTool()
{
}


IUITool* CSNPSearchTool::Clone() const
{
    return new CSNPSearchTool();
}


string CSNPSearchTool::GetName() const
{
    static string s_name("Search dbSNP");
    return s_name;
}


string CSNPSearchTool::GetDescription() const
{
    return "Search for rs or ss accessions from dbSNP";
}


CIRef<IDMSearchForm> CSNPSearchTool::CreateSearchForm()
{
    CIRef<IDMSearchForm> form(new CSNPSearchForm(*this));
    return form;
}


bool CSNPSearchTool::IsCompatible(IDataMiningContext* context)
{
    return false;
}


CRef<CSearchJobBase> CSNPSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    CSNPSearchQuery* e_query = dynamic_cast<CSNPSearchQuery*>(&query);
    if(e_query) {
        job.Reset(new CSNPSearchJob(*e_query));
    }
    return job;
}

string  CSNPSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::SNP_search_tool";
}

string  CSNPSearchTool::GetExtensionLabel() const
{
    return "Datamining Tool - dbSNP Search";
}


///////////////////////////////////////////////////////////////////////////////
/// CSNPSearchTool
CSNPSearchQuery::CSNPSearchQuery(const string& terms)
:   m_Terms(terms)
{
}


///////////////////////////////////////////////////////////////////////////////
/// CSNPSearchForm

CSNPSearchForm::CSNPSearchForm(CSNPSearchTool& tool)
:   m_Tool(&tool)
{
}


void CSNPSearchForm::Create()
{
}


void CSNPSearchForm::Init()
{
    CSearchFormBase::Init();
}


void CSNPSearchForm::x_LoadSettings(const CRegistryReadView& /*view*/)
{
}


void CSNPSearchForm::x_SaveSettings(CRegistryWriteView /*view*/) const
{
}


wxSizer* CSNPSearchForm::GetWidget(wxWindow * parent)
{
    if ( !m_Sizer) {        
        wxFlexGridSizer * sz = new wxFlexGridSizer(1, 1, 0, 0);     
        sz->AddGrowableCol(0);
        m_Sizer = sz;
        m_Text = new CSearchControl(parent, ID_TEXT, wxT(""),
                                    wxDefaultPosition, wxDefaultSize,
                                    wxTE_PROCESS_ENTER );                       
        m_Sizer->Add(m_Text,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);        
    }
    return m_Sizer;
}

void  CSNPSearchForm::Update()
{
}

void CSNPSearchForm::UpdateContexts()
{
    CSearchFormBase::UpdateContexts();
    // context - independent
    if (m_Controller) m_Controller->OnSearchEnabled(true);
}


CIRef<IDMSearchQuery> CSNPSearchForm::ConstructQuery()
{
    string q_s = ToStdString(m_Text->GetValue());
    CIRef<IDMSearchQuery> ref(new CSNPSearchQuery(q_s));
    return ref;
}


///////////////////////////////////////////////////////////////////////////////
/// CSNPSearchJob

CSNPSearchJob::CSNPSearchJob(CSNPSearchQuery& query)
:   m_Query(&query)
{
    m_Descr = "Query: " + m_Query->GetTerms();
}


bool CSNPSearchJob::x_ValidateParams()
{
    if(m_Query->GetTerms().empty()) {
        m_Error = new CAppJobError("Invalid input parameters - no search terms specified.");
        return false;
    }
    return true;
}


IAppJob::EJobState CSNPSearchJob::x_DoSearch()
{
    // prepare search params and search
    string terms = m_Query->GetTerms();
    size_t total_uids = 0;

    NSNPWebServices::TSNPSearchCompoundResultList SNPSearchResultList;
    
	NSNPWebServices::Search(terms, "", SNPSearchResultList);
    
    if(!IsCanceled())    {
		CRef<CObjectManager> om(CObjectManager::GetInstance());
		CRef<CScope> scope(new CScope(*om));
		scope->AddDefaults();

        // process results
        CMutexGuard Guard(m_Mutex);

        CObjectList * obj_list = m_TempResult->GetObjectList();
        NON_CONST_ITERATE(NSNPWebServices::TSNPSearchCompoundResultList, iSNPSearchResultList, SNPSearchResultList) {
            NON_CONST_ITERATE(NSNPWebServices::TSNPSearchResultList, iSNPSearchResult, iSNPSearchResultList->second) {
			    CObject* pSearchResultAsObject(iSNPSearchResult->GetPointer());
                int NewRow(obj_list->AddRow(pSearchResultAsObject, scope.GetPointer()));
/*
                cerr << "Placements at point of use: " << endl;
                std::stringstream ostr;
                ostr << MSerial_Json << *(*iSNPSearchResult);
                cerr << ostr.str() << endl;
*/                
                // CVariation used as a search result can have one and only one placement
			    NCBI_ASSERT((*iSNPSearchResult)->CanGetPlacements(), "Unexpected absence of placements in SNP Search Result!");
			    const CVariation::TPlacements& Placements((*iSNPSearchResult)->GetPlacements());
			    NCBI_ASSERT(Placements.size(), "Unexpected number of placements in SNP Search Result!");
			    CRef<CVariantPlacement> pPlacement(Placements.front());
                obj_list->SetString(1, NewRow, (*iSNPSearchResult)->GetId().GetTag().GetStr());
			    obj_list->SetInteger(2, NewRow, pPlacement->GetLoc().GetStart(eExtreme_Positional) + 1);
			    obj_list->SetInteger(3, NewRow, pPlacement->GetLoc().GetStop(eExtreme_Positional) + 1);
                if(pPlacement->CanGetAssembly()) {
                    obj_list->SetString(4, NewRow, pPlacement->GetAssembly().GetTag().GetStr());
                }
			    // columns which are encoded in ext and possibly not present
			    if((*iSNPSearchResult)->CanGetExt()) {
				    const CVariation::TExt& Ext((*iSNPSearchResult)->GetExt());
				    ITERATE(CVariation::TExt, iExt, Ext) {
					    if((*iExt)->HasField(NSNPWebServices::c_SNPVarExt_QueryId)) {
						    const CUser_field& UserField((*iExt)->GetField(NSNPWebServices::c_SNPVarExt_QueryId));
						    // must be a string or we cry
						    NCBI_ASSERT(UserField.CanGetData() && UserField.GetData().IsStr(), "Wrong type of user field for query_id");
						    obj_list->SetString(0, NewRow, UserField.GetData().GetStr());
					    }
					    if((*iExt)->HasField(NSNPWebServices::c_SNPVarExt_GeneSymbol)) {
						    const CUser_field& UserField((*iExt)->GetField(NSNPWebServices::c_SNPVarExt_GeneSymbol));
						    // must be a string or we cry
						    NCBI_ASSERT(UserField.CanGetData() && UserField.GetData().IsStr(), "Wrong type of user field for gene_symbol");
						    obj_list->SetString(5, NewRow, UserField.GetData().GetStr());
					    }
					    if((*iExt)->HasField(NSNPWebServices::c_SNPVarExt_Comment)) {
						    const CUser_field& UserField((*iExt)->GetField(NSNPWebServices::c_SNPVarExt_Comment));
						    // must be a string or we cry
						    NCBI_ASSERT(UserField.CanGetData() && UserField.GetData().IsStr(), "Wrong type of user field for comment");
						    obj_list->SetString(6, NewRow, UserField.GetData().GetStr());
					    }
				    }
			    }
            }
            if(IsCanceled()) {
                return eCanceled;
            }
        }
		
		return eCompleted;
    }
    return eCanceled;
}

void CSNPSearchJob::x_SetupColumns( CObjectList& obj_list )
{
	obj_list.ClearObjectLabels();
	obj_list.AddObjectLabel("Accession", CLabel::eContent );

    obj_list.AddColumn( CObjectList::eString, "Query term" );
    obj_list.AddColumn( CObjectList::eString, "SNP ID" );
    obj_list.AddColumn( CObjectList::eInteger, "Start" );
    obj_list.AddColumn( CObjectList::eInteger, "Stop" );
    obj_list.AddColumn( CObjectList::eString, "Assembly Name" );
    obj_list.AddColumn( CObjectList::eString, "Gene symbol" );
    obj_list.AddColumn( CObjectList::eString, "Comment" );
}


CObjectListTableModel* CSNPSearchJob::x_GetNewOLTModel() const
{
    return new CSNPTableModel();
}


///////////////////////////////////////////////////////////////////////////////
/// CSNPTableModel


CSNPTableModel::CSNPTableModel()
{
}


wxVariant CSNPTableModel::GetExtraValueAt( int row, int col ) const
{
    const CObject* obj = m_ObjectList->GetObject( row );
    const CVariation* pSNPSearchResult = dynamic_cast<const CVariation*>(obj);
	if(pSNPSearchResult) {
		// CVariation used as a search result can have one and only one placement
		NCBI_ASSERT(pSNPSearchResult->CanGetPlacements(), "Unexpected absence of placements in SNP Search Result!");
		const CVariation::TPlacements& Placements(pSNPSearchResult->GetPlacements());
		NCBI_ASSERT(Placements.size(), "Unexpected number of placements in SNP Search Result!");
		CRef<CVariantPlacement> pPlacement(Placements.front());
		const CSeq_id* pId(pPlacement->GetLoc().GetId());
		NCBI_ASSERT(pId, "Unexpected no or multiple IDs per SNP Search Result!");

		return ToWxString(pId->GetSeqIdString(true));
	}
    return wxT("");  
}

END_NCBI_SCOPE
