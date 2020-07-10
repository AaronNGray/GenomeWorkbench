/*  $Id: entrez_search_tool.cpp 39744 2017-10-31 21:12:13Z katargir $
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

#include "entrez_search_tool.hpp"
#include <gui/core/data_mining_service.hpp>

#include <gui/objutils/label.hpp>

#include <gui/objutils/entrez_conversion.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/utils/extension_impl.hpp>

#include <objmgr/object_manager.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>

#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define ID_COMBOBOX 11003
#define ID_TEXT     11414

///////////////////////////////////////////////////////////////////////////////
/// CDocsumTableModel

typedef pair<string,string> TStringPair;

class CDocsumTableModel : public CObjectListTableModel
{
public:
    CDocsumTableModel( const string& aDbName );

    virtual int GetNumExtraColumns() const;
    virtual wxString GetExtraColumnName( int col ) const;
    virtual wxVariant GetExtraValueAt( int row, int col ) const;

    virtual wxString GetImageAlias( int row ) const;

protected:
    string m_DbName;

    /// maps doc summary field names to columns
    vector<TStringPair> m_Fields;
};


///////////////////////////////////////////////////////////////////////////////
/// IDMSearchTool
CEntrezSearchTool::CEntrezSearchTool()
{
}


IUITool* CEntrezSearchTool::Clone() const
{
    return new CEntrezSearchTool();
}


string CEntrezSearchTool::GetName() const
{
    static string s_name("Search NCBI Public Databases");
    return s_name;
}


string CEntrezSearchTool::GetDescription() const
{
    return "";
}


CIRef<IDMSearchForm> CEntrezSearchTool::CreateSearchForm()
{
    CIRef<IDMSearchForm> form(new CEntrezSearchForm(*this));
    return form;
}


bool CEntrezSearchTool::IsCompatible(IDataMiningContext* context)
{
    return false;
}


CRef<CSearchJobBase> CEntrezSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    CEntrezSearchQuery* e_query = dynamic_cast<CEntrezSearchQuery*>(&query);
    if(e_query) {
        job.Reset(new CEntrezSearchJob(*e_query));
    }
    return job;
}

string  CEntrezSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::entrez_search_tool";
}

string  CEntrezSearchTool::GetExtensionLabel() const
{
    return "Datamining Tool - Entrez Search search";
}


///////////////////////////////////////////////////////////////////////////////
/// CEntrezSearchTool
CEntrezSearchQuery::CEntrezSearchQuery(const string& terms, const string& db_name)
:   m_Terms(terms),
    m_DbName(db_name)
{
}


///////////////////////////////////////////////////////////////////////////////
/// CEntrezSearchForm

CEntrezSearchForm::CEntrezSearchForm(CEntrezSearchTool& tool)
:   m_Tool(&tool),
    m_CurrDbName("Entrez Gene"),
    m_DbCombo(NULL)
{
}


void CEntrezSearchForm::Create()
{
}


void CEntrezSearchForm::Init()
{
    CSearchFormBase::Init();

    m_DbNames.clear();
    CEntrezDB::GetDbNames(m_DbNames);
    m_CurrDbName = m_DbNames.begin()->second;
}



static const char* kDatabaseTag = "Database";


void CEntrezSearchForm::x_LoadSettings(const CRegistryReadView& view)
{
    m_CurrDbName = view.GetString(kDatabaseTag);
}


void CEntrezSearchForm::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_DbCombo)
        view.Set(kDatabaseTag, ToStdString( m_DbCombo->GetStringSelection() ));
}


wxSizer* CEntrezSearchForm::GetWidget(wxWindow * parent)
{
    if ( !m_Sizer) {        
        wxFlexGridSizer * sz = new wxFlexGridSizer(1, 3, 0, 0);     
        sz->AddGrowableCol(2);
        m_Sizer = sz;
        
        m_Sizer->Add(new wxStaticText( parent, wxID_STATIC,
                                       wxT("Select NCBI Database:"),
                                       wxDefaultPosition, wxDefaultSize, 0 ),
            0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   
        
        m_DbCombo = new wxChoice(parent, ID_COMBOBOX,
                                   wxDefaultPosition, wxDefaultSize,
                                   0, (const wxString*)NULL);

        m_Sizer->Add(m_DbCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);         
    
        m_Text = new CSearchControl(parent, ID_TEXT, wxT(""),
                                    wxDefaultPosition, wxDefaultSize,
                                    wxTE_PROCESS_ENTER );                       
        m_Sizer->Add(m_Text,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);        
        //m_Text->SetAutoOff();
    }
    return m_Sizer;
}

void  CEntrezSearchForm::Update()
{
    m_DbCombo->Clear();

    m_DbNames.clear();
    CEntrezDB::GetDbNames(m_DbNames);
    ITERATE(TNamePairs, it, m_DbNames)  {
        m_DbCombo->Append(ToWxString(it->second), (void*)it->first.c_str());            
    }

    if ( !m_CurrDbName.empty() ) {
        m_DbCombo->SetStringSelection(ToWxString(m_CurrDbName));
        int sel = m_DbCombo->GetSelection();
        if (sel == wxNOT_FOUND) {
            m_DbCombo->Select(0);
        }
    } else {
        m_DbCombo->Select(0);
    }
    m_CurrDbName = ToStdString(m_DbCombo->GetStringSelection());
}

void CEntrezSearchForm::UpdateContexts()
{
    CSearchFormBase::UpdateContexts();
    // context - independent
    if (m_Controller) m_Controller->OnSearchEnabled(true);
}


CIRef<IDMSearchQuery> CEntrezSearchForm::ConstructQuery()
{
    const char * dbname =
        (const char*)m_DbCombo->GetClientData(m_DbCombo->GetSelection());

    string q_s = ToStdString(m_Text->GetValue());
    CIRef<IDMSearchQuery> ref(new CEntrezSearchQuery(q_s, dbname));
    return ref;
}


///////////////////////////////////////////////////////////////////////////////
/// CEntrezSearchJob

CEntrezSearchJob::CEntrezSearchJob(CEntrezSearchQuery& query)
:   m_Query(&query)
{
    string vis_db_name = CEntrezDB::GetVisibleName(m_Query->GetDbName());
    m_Descr = "Query: " + m_Query->GetTerms()
        + ", database = " + vis_db_name;
}


bool CEntrezSearchJob::x_ValidateParams()
{
    if(m_Query->GetTerms().empty()) {
        m_Error = new CAppJobError("Invalid input parameters - no search terms specified.");
        return false;
    }
    return true;
}


static int kMaxResults = 1000;

IAppJob::EJobState CEntrezSearchJob::x_DoSearch()
{
    // prepare search params and search
    string terms(m_Query->GetTerms());
    string db_name(m_Query->GetDbName());
	bool assemblyDB(db_name == "assembly");
    size_t total_uids = 0;

    xml::document docsums;
    CEntrezDB::Query(db_name, terms, total_uids, docsums, kMaxResults);
    m_ResultsCount = (int)total_uids;
    m_MaxResultsCount = kMaxResults;


    CRef<CObjectManager> om(CObjectManager::GetInstance());
    CRef<CScope> scope(new CScope(*om));
    scope->AddDefaults();
    
    if(total_uids && !IsCanceled())    {
        // process results
        CMutexGuard Guard(m_Mutex);

        CObjectList * obj_list = m_TempResult->GetObjectList();

        xml::node_set nodes ( docsums.get_root_node().run_xpath_query("//DocumentSummary") );
		NON_CONST_ITERATE(xml::node_set, it, nodes)    {
			if (assemblyDB)
				SetReleaseType(*it);
            obj_list->AddRow(new CXmlNodeObject(*it, db_name), scope.GetPointer());
            if(IsCanceled()) {
                return eCanceled;
            }
        }
		return eCompleted;
    }
    return eCanceled;
}

CObjectListTableModel* CEntrezSearchJob::x_GetNewOLTModel() const
{
    return new CDocsumTableModel( m_Query->GetDbName() );
}

void CEntrezSearchJob::SetReleaseType(xml::node& ds)
{
	xml::node::const_iterator itAccession = ds.find("AssemblyAccession");
	if (itAccession == ds.end())
		return;
	if (NPOS != NStr::Find(itAccession->get_content(), "GCF_")) { // RefSeq Accession
		xml::node releaseType("ReleaseType", "RefSeq");
		ds.insert(releaseType);
	}
	else { // GenBank Accession
		xml::node releaseType("ReleaseType", "GenBank");
		ds.insert(releaseType);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// CDocsumTableModel

static const TStringPair skGeneralFields[] = {
    TStringPair("AccessionVersion", "Label"),
    TStringPair("Title", "Description"),
    TStringPair("Extra", "FASTA IDs"),
    TStringPair("TaxId", "Taxonomic ID")
};

static const TStringPair skAssemblyFields[] = {
	TStringPair("AssemblyName", "Name"),
	TStringPair("AssemblyAccession", "Accession"),
	TStringPair("Organism", "Organism"),
	TStringPair("AssemblyDescription", "Description"),
	TStringPair("AssemblyClass", "Class"),
	TStringPair("ReleaseType", "Release Type"),
	TStringPair("NCBIReleaseDate", "Release Date"),
};

static const TStringPair skGeneFields[] = {
    TStringPair("Name", "Label"),
    TStringPair("Description", "Description"),
	// The path to the ScientificName (it is nested in Organism node)
    TStringPair("Organism/ScientificName", "Organism"),
    TStringPair("Chromosome", "Chromosome"),
    TStringPair("OtherAliases", "Aliases"),
    TStringPair("MapLocation", "Map Location")
};

static const TStringPair skGenomeFields[] = {
    TStringPair("Organism_Name", "Name"),
    TStringPair("Organism_Kingdom", "Kingdom"),
    TStringPair("Organism_Group", "Group"),
    TStringPair("Organism_Subgroup", "Subgroup"),
    TStringPair("Defline", "Defline"),
    TStringPair("Assembly_Name", "Assembly Name"),
    TStringPair("Assembly_Accession", "Assembly Accession")
};

CDocsumTableModel::CDocsumTableModel( const string& aDbName )
: m_DbName( aDbName )
{
    const TStringPair* ptr;
    int size;

    if( m_DbName == "gene" ){
        ptr = &skGeneFields[0];
        size = sizeof(skGeneFields);
    }
    else if( m_DbName == "genome" ){
        ptr = &skGenomeFields[0];
        size = sizeof(skGenomeFields);
    } 
	else if (m_DbName == "assembly"){
		ptr = &skAssemblyFields[0];
		size = sizeof(skAssemblyFields);
	}
	else {
        ptr = &skGeneralFields[0];
        size = sizeof(skGeneralFields);
    }

    int num = size /sizeof(TStringPair);
    for( int i = 0; ptr && i < num; i++ ){
        m_Fields.push_back( *(ptr +i) );
    }
}

int CDocsumTableModel::GetNumExtraColumns() const
{
    return (int)m_Fields.size();
}

wxString CDocsumTableModel::GetExtraColumnName( int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Invalid extra column index");
    }
    return ToWxString(m_Fields[col].second);
}

wxVariant CDocsumTableModel::GetExtraValueAt( int row, int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Invalid extra column index");
    }

    const CObject* obj = m_ObjectList->GetObject( row );
    const CXmlNodeObject* doc_sum = dynamic_cast<const CXmlNodeObject*>(obj);
    if( !doc_sum )
        return ToWxString( "" );

    const string& field = m_Fields[col].first;
    xml::node::const_iterator field_node = doc_sum->GetNode().find(field.c_str());
	if ((field_node == doc_sum->GetNode().end())) {
		do {
			if ((string::npos == field.find('/')))
				break;

			// Search for a path (unfortunately XPath queries are supportted only for the root node)
			vector<string> nodes;
			NStr::Split(field, "/", nodes);
			const xml::node* parent = &(doc_sum->GetNode());
			size_t count = nodes.size();
			size_t i;
			for (i = 0; parent && (i<count); ++i) {
				field_node = parent->find(nodes[i].c_str());
				if ((field_node == parent->end()))
					break;
				parent = &(*field_node);
			}

			if (!parent || (i != count))
				break;

			return ToWxString(parent->get_content());
		}
		while (false);
		return ToWxString("");
	}

    return ToWxString( field_node->get_content() );
}

wxString CDocsumTableModel::GetImageAlias( int row ) const
{
    if( m_DbName == "gene" )        return wxT("symbol::feature");
    if( m_DbName == "protein" )     return wxT("symbol::sequence_protein");
    if( m_DbName == "nucleotide" )  return wxT("symbol::sequence_dna");
    if( m_DbName == "assembly" )      return wxT("symbol::sequence");
    
    return wxT("");  
}


END_NCBI_SCOPE
