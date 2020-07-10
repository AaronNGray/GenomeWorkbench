/*  $Id: feature_search_tool.cpp 39744 2017-10-31 21:12:13Z katargir $
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

#include "feature_search_tool.hpp"

#include <gui/core/data_mining_service.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/seq/feature_check_dialog.hpp>

#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/extension_impl.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/feat_ci.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>

#define ID_COMBOBOX 10003
#define ID_TEXT     11414
#define ID_BUTTON1  10005
#define ID_HYPERLINKCTRL 10999

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kFetPattern  = "FetPattern";
static const char* kFetTypes    = "FetTypes";

///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchTool
CFeatureSearchTool::CFeatureSearchTool()
{
}


string CFeatureSearchTool::GetName() const
{
    static string name("Feature Search");
    return name;
}


IUITool* CFeatureSearchTool::Clone() const
{
    return new CFeatureSearchTool();
}


string CFeatureSearchTool::GetDescription() const
{
    return "";
}


CIRef<IDMSearchForm> CFeatureSearchTool::CreateSearchForm()
{
    CIRef<IDMSearchForm> form(new CFeatureSearchForm(*this));
    return form;
}


bool CFeatureSearchTool::IsCompatible(IDataMiningContext* context)
{
    ISeqLocSearchContext* sl_ctx =
        dynamic_cast<ISeqLocSearchContext*>(context);
    return sl_ctx != NULL;
}


CRef<CSearchJobBase> CFeatureSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    CFeatureSearchQuery* f_query = dynamic_cast<CFeatureSearchQuery*>(&query);
    if(f_query) {
        job.Reset(new CFeatureSearchJob(*f_query));
    }
    return job;
}


string  CFeatureSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::feature_search_tool";
}


string  CFeatureSearchTool::GetExtensionLabel() const
{
    return "Search Tool - Feature Search";
}


///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchQuery

CFeatureSearchQuery::CFeatureSearchQuery(TScopedLocs& locs,
                                         const string& pattern,
                                         bool case_sensitive,
                                         TPatternType pt_type,
                                         const TFeatTypeItemSet& feat_types)
:   CSearchQueryBase(locs),
    m_Pattern(pattern),
    m_PatternType(pt_type),
    m_CaseSensitive(case_sensitive),
    m_FeatTypesSet(feat_types)
{
}

string CFeatureSearchQuery::ToString() const
{
    string s(m_Pattern);
    s.append(" ");
    // TODO: write a string dumper for Feature Types Set

    return s;
}


///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchForm

static const int eCmdSelectFeatureTypes = 10000;
/*
BEGIN_EVENT_MAP(CFeatureSearchForm, CSearchFormBase)
    ON_COMMAND(eCmdSelectFeatureTypes, &CFeatureSearchForm::OnSelectFeatureType)
END_EVENT_MAP()
*/


CFeatureSearchForm::CFeatureSearchForm(CFeatureSearchTool& tool)
:   m_Tool(&tool)
{
}


CFeatureSearchForm::~CFeatureSearchForm()
{
}


void CFeatureSearchForm::Create()
{
    x_CreateWidgets();
}


void CFeatureSearchForm::Init()
{
    CSearchFormBase::Init();
}

void CFeatureSearchForm::Update()
{
    UpdateContextCombo(m_DbCombo);

    string strText = "";

    ITERATE(TFeatTypeItemSet, ft_it, m_FeatTypesSet) {
        string lbl = ft_it->GetDescription();
        if (!lbl.empty()) {
            strText += strText.empty()?"":", ";
            strText += lbl;
        }
    }

    if (strText.empty()) {
        strText = "Click To Select ...";
    }
    
    list<string> strList;
    NStr::Wrap(strText, 25, strList);
    
    m_HyperLink->SetLabel(ToWxString(*strList.begin() + "..."));
    m_HyperLink->SetToolTip(ToWxString(NStr::Join(strList, "\n")));
    m_HyperLink->GetParent()->Layout();
    m_HyperLink->Update();
}

void CFeatureSearchForm::x_LoadSettings(const CRegistryReadView& view)
{
    m_SearchPattern = view.GetString(kFetPattern);  
    
    view.GetStringList(kFetTypes, m_FeatureTypes);  
    if (!m_FeatureTypes.empty()){
        m_FeatTypesSet.clear();
        const CFeatList* feat_list = CSeqFeatData::GetFeatList();
        ITERATE(CFeatList, ft_it, *feat_list) {
            const CFeatListItem& item = *ft_it;
            string desc = item.GetDescription();

            if (find(m_FeatureTypes.begin(), m_FeatureTypes.end(), desc)!=
                m_FeatureTypes.end()){
                m_FeatTypesSet.insert(item);
            }
        }
    }
}


void CFeatureSearchForm::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_TypeCombo) {      
        view.Set(kFetPattern, ToStdString( m_TypeCombo->GetStringSelection() ));
    }   

    list<string> strList;
    ITERATE(CFeatList, ft_it, m_FeatTypesSet) {
        strList.push_back(ft_it->GetDescription());
    }
    view.Set(kFetTypes, strList);
}


wxSizer*  CFeatureSearchForm::GetWidget(wxWindow * parent)
{
    if (!m_Sizer)   {
        wxFlexGridSizer * sz = new wxFlexGridSizer(0, 4, 0, 0);     
        sz->AddGrowableCol(3);
        m_Sizer = sz;
        
        wxStaticText* stat1 = new wxStaticText(parent, wxID_STATIC,
                                               wxT("Search Context:"));
        m_Sizer->Add(stat1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   

        m_DbCombo = new wxChoice(parent, ID_COMBOBOX,
                                   wxDefaultPosition, wxDefaultSize,
                                    0, (const wxString*)NULL);

        m_Sizer->Add(m_DbCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);     

        // search type
        wxStaticText* stat2 = new wxStaticText( parent, wxID_STATIC,
                                                wxT("Search Type:"));
        m_Sizer->Add(stat2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   

        m_TypeCombo = new  wxChoice(parent, ID_COMBOBOX,
                                      wxDefaultPosition, wxDefaultSize,
                                      0, (const wxString*)NULL);
        m_TypeCombo->Append(wxT("Exact Match"));
        m_TypeCombo->Append(wxT("Regular Expression"));
        m_TypeCombo->Append(wxT("Wildcard"));
        
        if (!m_SearchPattern.empty()) {
            m_TypeCombo->SetStringSelection(ToWxString(m_SearchPattern));
        } else {
            m_TypeCombo->Select(0);
        }

        m_Sizer->Add(m_TypeCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);       

        m_Sizer->Add(new wxStaticText( parent, wxID_STATIC,
                                       wxT("Feature Types:"),
                                       wxDefaultPosition, wxDefaultSize, 0 ),
                     0, wxALIGN_CENTER_VERTICAL|wxALL, 5);  
        
        // editbox with button
        //wxBoxSizer* featTypeSizer = new wxBoxSizer(wxHORIZONTAL);
        //m_Sizer->Add(featTypeSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
        

        //wxStaticText* itemStaticText4 = new wxStaticText( parent, wxID_STATIC, wxT("Static text"), wxDefaultPosition, wxDefaultSize, wxDOUBLE_BORDER );
        //featTypeSizer->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_HyperLink = new wxHyperlinkCtrl( parent, ID_HYPERLINKCTRL,
                                           wxT("Click To Select ..."), wxT(""),
                                           wxDefaultPosition, wxDefaultSize,
                                           wxHL_DEFAULT_STYLE);
        m_Sizer->Add(m_HyperLink, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    

        wxStaticText* stat3 = new wxStaticText( parent, wxID_STATIC,
                                                wxT("Search Expression:"));
        m_Sizer->Add(stat3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   
    
        m_Text = new CSearchControl(parent, ID_TEXT, wxT(""),
                                    wxDefaultPosition, wxDefaultSize,
                                    wxTE_PROCESS_ENTER );
        m_Sizer->Add(m_Text,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);        
    }
    return m_Sizer;
}


void CFeatureSearchForm::x_CreateWidgets()
{
}


IDMSearchTool* CFeatureSearchForm::x_GetTool()
{
    return m_Tool.GetPointer();
}


CIRef<IDMSearchQuery> CFeatureSearchForm::ConstructQuery()
{
     m_SeqLocContexts.clear();
     for( size_t i = 0;  i < m_Contexts.size();  i++ )   {
         if (m_DbCombo->GetSelection() != 0  &&
             m_DbCombo->GetSelection() != int(i + 1) ) {
             continue;
         }
         ISeqLocSearchContext* seq_ctx =
            dynamic_cast<ISeqLocSearchContext*>(m_Contexts[i]);
         m_SeqLocContexts.push_back(seq_ctx);
      }

    _ASSERT(m_SeqLocContexts.size());

    /// accumulate locations from all selected contexts
    TScopedLocs sc_locs;    

    for( size_t i = 0;  i < m_SeqLocContexts.size();  i++ ) {
        ISeqLocSearchContext* sl_ctx = m_SeqLocContexts[i];
        _ASSERT(sl_ctx);

        // tereshko: skip context if it can't participate in search anymore (selection loss, etc) 
        if (sl_ctx->GetSearchLoc().IsNull()) continue;

        CRef<CSeq_loc> loc(new CSeq_loc());
        if (m_bRange) {
            loc = sl_ctx->GetSearchLoc();
        }
        else {
            CRef<CSeq_id> id(new CSeq_id());
            id->Assign(*(sl_ctx->GetSearchLoc()->GetId()));
            loc->SetWhole(*id);
        }

        CSearchQueryBase::SScopedLoc sl;
        sl.m_Loc = loc;       
        sl.m_Scope = sl_ctx->GetSearchScope();
        sl.m_ContextName = sl_ctx->GetDMContextName();
        sc_locs.push_back(sl);
    }

    switch (m_TypeCombo->GetSelection()){
        case 0: m_PatternType = CSearchToolBase::eExactMatch;   break;
        case 1: m_PatternType = CSearchToolBase::eRegexp;     break;
        case 2: m_PatternType = CSearchToolBase::eWildcard;       break;
    }

    string pattern = ToStdString(m_Text->GetValue());
    NStr::TruncateSpacesInPlace(pattern);

    CIRef<IDMSearchQuery> ref;
    ref = new CFeatureSearchQuery(sc_locs, pattern,
                                  false, // case sensitive
                                  m_PatternType, m_FeatTypesSet);
    return ref;
}


///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchJob

CFeatureSearchJob::CFeatureSearchJob(CFeatureSearchQuery& query)
:   m_Query(&query),
    m_PatternType(CSearchToolBase::eExactMatch)
{
    string s_locs, id;
    static string sep(", ");
    NON_CONST_ITERATE(TScopedLocs, it, m_Query->GetScopedLocs())    {
        CLabel::GetLabel(*it->m_Loc, &id, CLabel::eDefault,
            it->m_Scope.GetPointer());
        s_locs += id;
        s_locs += sep;
    }
    if( ! s_locs.empty()) {
        s_locs.resize(s_locs.size() - sep.size());
    }
    string s_pat = m_Query->GetPattern();
    m_Descr = "Search Features \"" + s_pat + "\" on " + s_locs;
}


bool CFeatureSearchJob::x_ValidateParams()
{
    m_Error.Reset();

    if( m_Query->GetScopedLocs().empty()) {
        m_Error = new CAppJobError("Invalid input parameters - no search context specified.");
    }
    if(m_Query->GetPattern().empty())  {
        m_Error = new CAppJobError("Search pattern is empty.");
    }
    return m_Error ? false : true;
}


IAppJob::EJobState CFeatureSearchJob::x_DoSearch()
{
    m_SearchStr = m_Query->GetPattern();
    TScopedLocs& scoped_locs = m_Query->GetScopedLocs();
    m_PatternType = m_Query->GetPatternType();
    m_CaseSensitive = m_Query->GetCaseSensitive();
    m_Pattern = NULL;

    switch(m_PatternType)   {
    case CSearchToolBase::eWildcard:
        m_SearchStr += '*';
        m_SearchStr.insert(m_SearchStr.begin(), '*');
        break;

    case CSearchToolBase::eRegexp:  {{
         CRegexp::ECompile flags = m_CaseSensitive ? CRegexp::fCompile_default
                                               : CRegexp::fCompile_ignore_case;
         m_Pattern = new CRegexp(m_SearchStr, flags);
        break;
    }}
    default:
        break;
    }

    const TFeatTypeItemSet& feat_types = m_Query->GetFeatTypes();

    x_SearchFeatures(scoped_locs, feat_types);

    if (m_Pattern) {
        delete m_Pattern;
        m_Pattern = NULL;
    }

    return eCompleted;
}


void CFeatureSearchJob::x_SetupColumns(CObjectList& obj_list)
{
	obj_list.AddColumn(CObjectList::eString, "Location");
	obj_list.AddColumn(CObjectList::eString, "Strand");
    obj_list.AddColumn(CObjectList::eString, "Accession");
    obj_list.AddColumn(CObjectList::eString, "Context");
}

string CFeatureSearchJob::SeqLocToString(const CSeq_loc& loc)
{
    //_ASSERT(loc.GetId());
   
	string out;

    if (!loc.GetId()) {
        loc.GetLabel(&out);
    }
    else {
        if (loc.Which() == CSeq_loc::e_Whole || loc.GetTotalRange().Empty()) {
            out += "0-0";
        } else {
            out += NStr::UIntToString(loc.GetTotalRange().GetFrom() + 1);
            out += "-";
            out += NStr::UIntToString(loc.GetTotalRange().GetTo() + 1);
            if (loc.GetStrand() == eNa_strand_minus)
                out += " -";
        }
    }

	return out;
}


void CFeatureSearchJob::x_SearchFeatures(TScopedLocs& scoped_locs,
                                         const TFeatTypeItemSet& feat_types)
{
    const string progress_str("Searching features. ");
    {
        CMutexGuard Guard(m_Mutex);
        m_ProgressStr = progress_str;
    }

    // Create Annot Selector filtering the right feature types
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(); /// all feature types

    if( ! feat_types.empty()) {
        // set filter on the subtypes
        ITERATE(TFeatTypeItemSet, it_f, feat_types)   {
            const TFeatTypeItem& item = *it_f;
            if(item.GetSubtype() == CSeqFeatData::eSubtype_any) {
                sel.IncludeFeatType((CSeqFeatData::E_Choice) item.GetType());
            } else {
                sel.IncludeFeatSubtype((CSeqFeatData::ESubtype) it_f->GetSubtype());
            }
        }
    }

    string label;
    string loc_label;

    // for every Seq_loc
    NON_CONST_ITERATE(TScopedLocs, it_sl, scoped_locs)    {
        // extract context data
        TScopedLoc& sc_loc = *it_sl;
        CSeq_loc& seq_loc = *sc_loc.m_Loc;
        CScope& scope = *sc_loc.m_Scope;

        loc_label.clear();
        CLabel::GetLabel(seq_loc, &loc_label, CLabel::eDefault, &scope);

        {
            CMutexGuard Guard(m_Mutex);
            m_ProgressStr = progress_str;
            m_ProgressStr.append(loc_label);
        }
		
        if(IsCanceled()) {
            LOG_POST(Info << m_Descr << " canceled.");
            return;
        }

        // create Feature Iterator
        CFeat_CI feat_iter(scope, seq_loc, sel);
        int cnt = 0;
        int cnt_found = 0;

        for ( ;  feat_iter  &&  ! IsCanceled();  ++feat_iter, ++cnt) {
            if (cnt > 0 && cnt % 100) {
                CMutexGuard Guard(m_Mutex);
                m_ProgressStr = progress_str;
                m_ProgressStr.append(loc_label);
                m_ProgressStr.append(" [");
                m_ProgressStr.append(NStr::IntToString(cnt_found));
                m_ProgressStr.append("/");
                m_ProgressStr.append(NStr::IntToString(cnt));
                m_ProgressStr.append("]");
            }

            bool found = false;
            switch (feat_iter->GetData().Which()) {
            case CSeqFeatData::e_Gene:  {
                // Gene is a special case
                const CGene_ref& gene_ref = feat_iter->GetData().GetGene();
                found = x_Match(gene_ref);
                break;
            }
            default:    {
                // for all other feature types, the label contains all available
                // (searchable) string information
                label.erase();
                CLabel::GetLabel(feat_iter->GetOriginalFeature(), &label, CLabel::eUserTypeAndContent, &scope);
                found = x_Match(label);
                break;
            }
            }
            // make sure we search the comment field
            if ( ! found  &&  feat_iter->IsSetComment() ) {
                found = x_Match(feat_iter->GetComment());
            }

            // also search the exception text, as this is free-form
            if ( ! found  &&  feat_iter->IsSetExcept_text() ) {
                found = x_Match(feat_iter->GetExcept_text());
            }

            if (found) {
                ++cnt_found;
                {
                    CMutexGuard Guard(m_Mutex);
                    m_ProgressStr = progress_str;
                    m_ProgressStr.append(loc_label);
                    m_ProgressStr.append(" [");
                    m_ProgressStr.append(NStr::IntToString(cnt_found));
                    m_ProgressStr.append("/");
                    m_ProgressStr.append(NStr::IntToString(cnt));
                    m_ProgressStr.append("]");
                }

                CSeq_feat& feat = const_cast<CSeq_feat&>(CSeqFeatData::e_Gene == feat_iter->GetData().Which() ? feat_iter->GetMappedFeature() : feat_iter->GetOriginalFeature());

				string locplustrand = SeqLocToString(feat.GetLocation());
				string strand = "+";
				string location = locplustrand;

				if (locplustrand.find_last_of('-') == (locplustrand.length()-1)) {
					location.resize(location.size()-2);
					strand = "-";
				}
				
                x_AddToResults(feat, scope, location, strand, loc_label, sc_loc.m_ContextName);
            }
        }
    }
}


void CFeatureSearchJob::x_AddToResults(CObject& obj, CScope& scope,
									   const string& loc_name,
									   const string& strand,
                                       const string& acc_name,
                                       const string& ctx_name)
{
    static const int kUpdateIncrement = 200;

    // adding the result to the Accumulator
    int row = m_AccList.AddRow(&obj, &scope);
	m_AccList.SetString(0, row, loc_name);
	m_AccList.SetString(1, row, strand);
    m_AccList.SetString(2, row, acc_name);
    m_AccList.SetString(3, row, ctx_name);

    int count = m_AccList.GetNumRows();
    if(count >= kUpdateIncrement)   {
        // time to update the Result
        CMutexGuard Guard(m_Mutex);

        // transfer results from Accumulator to m_TempResult
        m_TempResult->GetObjectList()->Append(m_AccList);
        m_AccList.ClearRows();

        // update progress string
        m_ProgressStr = NStr::IntToString(count, NStr::fWithCommas) + " feature";
        if (count != 1) {
            m_ProgressStr += "s";
        }
        m_ProgressStr += " found.";
    }
}


bool CFeatureSearchJob::x_Match(const string& text)
{
    switch(m_PatternType)   {
    case CSearchToolBase::eExactMatch:  {
        //return text.find(m_SearchStr) != string::npos;
        NStr::ECase use_case = m_CaseSensitive ? NStr::eCase : NStr::eNocase;
        return NPOS != NStr::Find(text, m_SearchStr, use_case);
        }
    case CSearchToolBase::eWildcard:    {
        NStr::ECase use_case = m_CaseSensitive ? NStr::eCase : NStr::eNocase;
        return NStr::MatchesMask(text, m_SearchStr, use_case);
        }
    case CSearchToolBase::eRegexp:  {
        return ! m_Pattern->GetMatch(text).empty();
        }
    default:
        _ASSERT(false);
        return false;
    }
}


/// genes have a lot of embedded labels. many of these are optional, so the
/// checking is a bit tedious This could be done with CStdTypeConstIterator<>,
///  but this would pick up an inadvertent db-tag that we don't really want
/// to search
bool CFeatureSearchJob::x_Match(const objects::CGene_ref& gene_ref)
{
    if (gene_ref.IsSetLocus()  &&  x_Match(gene_ref.GetLocus())) {
        return true;
    }
    if (gene_ref.IsSetAllele()  &&  x_Match(gene_ref.GetAllele())) {
        return true;
    }
    if (gene_ref.IsSetDesc()  &&  x_Match(gene_ref.GetDesc())) {
        return true;
    }
    if (gene_ref.IsSetMaploc()  &&  x_Match(gene_ref.GetMaploc())) {
        return true;
    }
    if (gene_ref.IsSetLocus_tag()  &&  x_Match(gene_ref.GetLocus_tag())) {
        return true;
    }
    if (gene_ref.IsSetSyn()) {
        ITERATE (CGene_ref::TSyn, iter, gene_ref.GetSyn()) {
            if (x_Match(*iter))
                return true;
        }
    }
    return false;
}


END_NCBI_SCOPE
