/*  $Id: sequence_search_tool.cpp 39744 2017-10-31 21:12:13Z katargir $
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

#include <gui/core/data_mining_service.hpp>
#include "sequence_search_tool.hpp"

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbireg.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/extension_impl.hpp>

#include <objmgr/util/sequence.hpp>
#include <util/sequtil/sequtil_manip.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>
#include <wx/filename.h>
#include <wx/dir.h>

#define ID_COMBOBOX 10003
#define ID_TEXT             11414
#define ID_COMBOBOX_UPDATE  10111

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kSeqPattern = "SeqPattern";

/*
class CSeqSearchTableModel : public CObjectListTableModel
{
    virtual int GetNumExtraColumns() const;
    virtual wxString GetExtraColumnName( int col ) const;
};

int CSeqSearchTableModel::GetNumExtraColumns() const
{
    // we can use just the first column from the set of standard ones
    // ie "Label"
    return 1;
}

wxString CSeqSearchTableModel::GetExtraColumnName( int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW( CException, eUnknown, "Invalid extra column index" );
    }

    return wxT("Location");
}
*/

///////////////////////////////////////////////////////////////////////////////
/// CSequenceSearchTool
CSequenceSearchTool::CSequenceSearchTool()
{
    m_Conversions.options.clear();
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_region);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_polyA_site);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_site_ref);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_site);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_rsite);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_misc_binding);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_primer_bind);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_protein_bind);
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_txinit);
    m_Conversions.selected = CSeqFeatData::eSubtype_region;

    m_Filters.filters.push_back(pair<string,string>("Show only positive strand results", 
        "Strand='+'"));
    
    m_Filters.filters.push_back(pair<string,string>("Show only negative strand results",
        "Strand='-'"));
}


string CSequenceSearchTool::GetName() const
{
    static string name("Sequence Search");
    return name;
}


IUITool* CSequenceSearchTool::Clone() const
{
    return new CSequenceSearchTool();
}


string CSequenceSearchTool::GetDescription() const
{
    return "";
}


CIRef<IDMSearchForm> CSequenceSearchTool::CreateSearchForm()
{
    CIRef<IDMSearchForm> form(new CSequenceSearchForm(*this));
    return form;
}


bool CSequenceSearchTool::IsCompatible(IDataMiningContext* context)
{
    ISeqLocSearchContext* sl_ctx =
        dynamic_cast<ISeqLocSearchContext*>(context);
    return sl_ctx != NULL;
}

CRef<CSearchJobBase> CSequenceSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    CSequenceSearchQuery* f_query = dynamic_cast<CSequenceSearchQuery*>(&query);
    if(f_query) {
        job.Reset(new CSequenceSearchJob(*f_query));
    }
    return job;
}



string  CSequenceSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::sequence_search_tool";
}

string  CSequenceSearchTool::GetExtensionLabel() const
{
    return "Datamining Tool - Sequence Search search";
}

IDMSearchTool::TUIToolFlags CSequenceSearchTool::GetFlags(void)
{
    return (IDMSearchTool::eFilter | IDMSearchTool::eCache);
}

///////////////////////////////////////////////////////////////////////////////
/// CSequenceSearchQuery

CSequenceSearchQuery::CSequenceSearchQuery(TScopedLocs& locs,
                                           const string& pattern,
                                           TPatternType pt_type,
                                           const string& file )
:   CSearchQueryBase(locs),
    m_Pattern(pattern),
    m_PatternType(pt_type),
    m_PatternFile(file)
{
}

string CSequenceSearchQuery::ToString() const
{
    string s;
    s.append(CSearchToolBase::GetPatternTypeStr(m_PatternType));
    s.append(" ");
    s.append(m_Pattern);
    s.append(" at '");
    s.append(m_PatternFile);
    s.append("'");
    return s;
}


///////////////////////////////////////////////////////////////////////////////
/// CSequenceSearchForm

CSequenceSearchForm::TFileList CSequenceSearchForm::m_FileList;

CSequenceSearchForm::CSequenceSearchForm(CSequenceSearchTool& tool)
:   m_Tool(&tool),
    m_TypeCombo(NULL)
{
}


CSequenceSearchForm::~CSequenceSearchForm()
{
}

void CSequenceSearchForm::Create()
{
}

void CSequenceSearchForm::Init()
{
    CSearchFormBase::Init();    
    x_UpdateFileList();
}


void CSequenceSearchForm::x_LoadSettings(const CRegistryReadView& view)
{
    m_SearchPattern = view.GetString(kSeqPattern);  
}


void CSequenceSearchForm::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_TypeCombo)
        view.Set(kSeqPattern, ToStdString( m_TypeCombo->GetStringSelection() ));
}


IDMSearchTool* CSequenceSearchForm::x_GetTool()
{
    return m_Tool.GetPointer();
}


wxSizer *  CSequenceSearchForm::GetWidget(wxWindow * parent)
{
    if (!m_Sizer) {
        wxBoxSizer * vSz = new wxBoxSizer(wxVERTICAL);
        m_Sizer = vSz;

        wxFlexGridSizer * sz = new wxFlexGridSizer(1, 4, 0, 0);     
        sz->AddGrowableCol(3);

        m_Sizer->Add(sz, 0, wxGROW|wxALL, 0);


        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Search Context:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   

        m_DbCombo = new wxChoice(parent, ID_COMBOBOX,
                                 wxDefaultPosition, wxDefaultSize,
                                 0, (const wxString*)NULL);

        sz->Add(m_DbCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);      

        // search type
        sz->Add(new wxStaticText( parent, wxID_STATIC, wxT("Search Type:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   


        m_TypeCombo = new  wxChoice(parent, ID_COMBOBOX_UPDATE,
                                    wxDefaultPosition, wxDefaultSize,
                                    0, (const wxString*)NULL);
        m_TypeCombo->Append(wxT("Exact Match"));
        m_TypeCombo->Append(wxT("Regular Expression"));
        m_TypeCombo->Append(wxT("Wildcard"));        
        m_TypeCombo->Append(wxT("Named Pattern Sets"));      


        if (!m_SearchPattern.empty()) {
            m_TypeCombo->SetStringSelection(ToWxString(m_SearchPattern));
        }
        else {
            m_TypeCombo->Select(0);
        }

        sz->Add(m_TypeCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);    


        m_pSearchSizer = new wxFlexGridSizer(1, 2, 0, 0);       
        m_pSearchSizer->AddGrowableCol(1);
        m_pSearchSizer->Add(new wxStaticText( parent, wxID_STATIC,
                                              wxT("Search Expression:"),
                                              wxDefaultPosition, wxDefaultSize, 0 ),
                            0, wxALIGN_CENTER_VERTICAL|wxALL, 5);       
        m_Text = new CSearchControl(parent, ID_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
        m_pSearchSizer->Add(m_Text,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_pPatternSizer = new wxFlexGridSizer(1, 2, 0, 0);      
        m_pPatternSizer->AddGrowableCol(1);

        m_pPatternSizer->Add(new wxStaticText( parent, wxID_STATIC, wxT("Select Named Pattern:"), wxDefaultPosition, wxDefaultSize, 0 ),
                             0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   
        m_PatternCombo = new wxChoice(parent, ID_COMBOBOX,
                                      wxDefaultPosition, wxDefaultSize,
                                      0, (const wxString*)NULL);

        ITERATE(TFileList, it, m_FileList) {
            m_PatternCombo->Append(ToWxString(it->first));
        }
        m_PatternCombo->Select(0);
        m_pPatternSizer->Add(m_PatternCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_Sizer->Add(m_pSearchSizer,  0, wxGROW|wxALL, 0);
        m_Sizer->Add(m_pPatternSizer, 0, wxGROW|wxALL, 0);

    }
    return m_Sizer;
}


void CSequenceSearchForm::Update()
{
    UpdateContextCombo(m_DbCombo);

    if (m_TypeCombo) {
        bool bPattern = (m_TypeCombo->GetSelection()==3);
        m_Sizer->Show(m_pSearchSizer, !bPattern);
        m_Sizer->Show(m_pPatternSizer, bPattern);   
        m_Sizer->Layout();
    }
}


CIRef<IDMSearchQuery> CSequenceSearchForm::ConstructQuery()
{
    m_SeqLocContexts.clear();
    for( size_t i = 0;  i < m_Contexts.size();  i++ )   {
        if (m_DbCombo->GetSelection() != 0  &&
            m_DbCombo->GetSelection() != int(i + 1)) {
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
        CRef<CSeq_loc> search_loc = sl_ctx->GetSearchLoc();
        if (search_loc.IsNull()) {
            continue;
        }

        vector<CRef<CSeq_loc> > locs;
        if (m_bRange) {
            if (search_loc->IsPacked_int()) {
                ITERATE (CSeq_loc::TPacked_int::Tdata, iter, search_loc->GetPacked_int().Get()) {
                    locs.push_back(CRef<CSeq_loc>(new CSeq_loc()));
                    CRef<CSeq_interval> ref = *iter;
                    locs.back()->SetInt(*ref);
                }
            }
            else
                locs.push_back(search_loc);
        }
        else {
            CRef<CSeq_id> id(new CSeq_id());
            id->Assign(*(search_loc->GetId()));
            locs.push_back(CRef<CSeq_loc>(new CSeq_loc()));
            locs.back()->SetWhole(*id);
        }

        ITERATE(vector<CRef<CSeq_loc> >, it, locs) {
            CSearchQueryBase::SScopedLoc sl;
            sl.m_Loc = *it;       
            sl.m_Scope = sl_ctx->GetSearchScope();
            sl.m_ContextName = sl_ctx->GetDMContextName();
            sc_locs.push_back(sl);
        }
    }

    string searchString = ToStdString(m_Text->GetValue());

    switch (m_TypeCombo->GetSelection()){
    case 0: m_PatternType = CSearchToolBase::eExactMatch;   break;
    case 2: m_PatternType = CSearchToolBase::eWildcard;     break;
    case 1: m_PatternType = CSearchToolBase::eRegexp;       break;
    case 3: 
            {
                m_PatternType = CSearchToolBase::eNamed;
                searchString  = "#undefined#";
                break;
            }
    }


    CIRef<IDMSearchQuery> ref;    
    ref = new CSequenceSearchQuery(sc_locs,
                                   searchString,
                                   m_PatternType,
                                   ToStdString(m_PatternCombo->GetStringSelection()));
    return ref;
}

void CSequenceSearchForm::UpdateContexts()
{
    CSearchFormBase::UpdateContexts();
    /*
       bool bEnable = false;

       ITERATE(TContexts, cont, m_Contexts) {
       ISeqLocSearchContext* seq_ctx =
       dynamic_cast<ISeqLocSearchContext*>(*cont);
       if (seq_ctx && !seq_ctx->GetSearchLoc().Empty()){
       bEnable=true;
       break;
       }
       }
       if (m_Controller) {
       m_Controller->OnSearchEnabled(bEnable);
       }
     */
}


void CSequenceSearchForm::x_UpdateFileList(void)
{
    string dir;

    m_FileList.clear();

    CNcbiApplication* app = CNcbiApplication::Instance();
    _ASSERT(app);
    CNcbiRegistry& registry = app->GetConfig();

    if ( (dir = registry.Get("Patterns", "PatternPath")).empty() ) {
        registry.Set("Patterns", "PatternPath", "<std>, <home>",
                     CNcbiRegistry::ePersistent, " default external_path");
    }
    dir = registry.Get("Patterns", "PatternPath");

    list<string> paths;
    NStr::Split(dir, ", \t\n\r", paths, NStr::fSplit_Tokenize);

    ITERATE (list<string>, iter, paths) {
        wxString dir_name;
        if (*iter == "<std>"  ||  *iter == "<home>") {
            dir_name = CSysPath::ResolvePath(wxString::FromAscii(iter->c_str()), wxT("etc/patterns"));
        } else {
            dir_name = CSysPath::ResolvePath(wxString::FromAscii(iter->c_str()), wxEmptyString);
        }
        if ( dir_name.empty() ) {
            continue;
        }

        wxFileName fname(dir_name, wxEmptyString);
        if ( !fname.DirExists(dir_name) ) {
            continue;
        }

        wxDir dir(dir_name);
        wxString filename;

        bool cont = dir.GetFirst(&filename, wxT("*.ini"), wxDIR_FILES);
        while ( cont ) {
            fname.SetFullName(filename);
            wxString full_path = fname.GetFullPath();

            CNcbiIfstream reg_stream(full_path.fn_str());
            CNcbiRegistry patterns(reg_stream);

            // iterate over patterns
            list<string> pat_ids;
            patterns.EnumerateSections(&pat_ids);
            string fName="";

            ITERATE (list<string>, pat_id, pat_ids) {
                if (*pat_id == "-") {
                    fName = patterns.Get("-", "name");
                    if (!fName.empty()) {
                        m_FileList[fName].first = full_path;
                    }   
                }
                else {
                    string pattern = patterns.Get(*pat_id, "pattern");
                    string desc = patterns.Get(*pat_id, "description");
                    if (!fName.empty()) {
                        m_FileList[fName].second.push_back(TDescPattern(desc, pattern));
                    }   
                }
            }
            cont = dir.GetNext(&filename);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CSequenceSearchJob

CSequenceSearchJob::CSequenceSearchJob(CSequenceSearchQuery& query)
:   m_Query(&query),
    m_PatternType(query.GetPatternType())
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
    m_Descr = "Search Sequence for \"" + s_pat + "\" on " + s_locs;
}


bool CSequenceSearchJob::x_ValidateParams()
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


IAppJob::EJobState CSequenceSearchJob::x_DoSearch()
{
    m_SearchStr              = m_Query->GetPattern();
    TScopedLocs& scoped_locs = m_Query->GetScopedLocs();
    m_PatternType            = m_Query->GetPatternType();
    m_PatternFile            = m_Query->GetPatternFile();
    EJobState res_state      = IAppJob::eFailed;

    if (m_PatternType!=CSearchToolBase::eNamed) {
        switch(m_PatternType)   {
        case CSearchToolBase::eWildcard:    {
                                                string pattern = CRegexp::WildcardToRegexp(m_SearchStr);
                                                m_Pattern = new CRegexp(pattern, CRegexp::fCompile_ignore_case);
                                                break;
                                            }
        case CSearchToolBase::eRegexp:  {
                                            m_Pattern = new CRegexp(m_SearchStr, CRegexp::fCompile_ignore_case);
                                            break;
                                        }
        default:
                                        m_Pattern = NULL;
                                        break;
        }

        res_state = x_SearchSequence(scoped_locs);

        if (m_Pattern) {
            delete m_Pattern;
            m_Pattern = NULL;
        }
    }
    else {
        // named pattern search
        ITERATE (CSequenceSearchForm::TDescPatList, pat, CSequenceSearchForm::m_FileList[m_PatternFile].second){            
            m_SearchStr = pat->first;
            m_Pattern   = new CRegexp(pat->second, CRegexp::fCompile_ignore_case);
            res_state   = x_SearchSequence(scoped_locs);
            delete m_Pattern;
            m_Pattern = NULL;           
        }
    }

    return res_state;
}


void CSequenceSearchJob::x_SetupColumns(CObjectList& obj_list)
{
	obj_list.ClearObjectLabels();
	obj_list.AddObjectLabel( "Location", CLabel::eContent );

    obj_list.AddColumn(CObjectList::eString, "Sequence");


    // special case for Kozak pattern search
    if (m_PatternType==CSearchToolBase::eNamed) {
        obj_list.AddColumn(CObjectList::eInteger, "Pattern Start");
        obj_list.AddColumn(CObjectList::eString, "Pattern Name");
    }

    obj_list.AddColumn( CObjectList::eInteger, "Start" );
    obj_list.AddColumn( CObjectList::eInteger, "Stop" );
    obj_list.AddColumn( CObjectList::eString,  "Strand" );

    obj_list.AddColumn(CObjectList::eString, "Context");
}


static const int kMaxResults = 20000;

IAppJob::EJobState CSequenceSearchJob::x_SearchSequence(TScopedLocs& scoped_locs)
{
    {
        CMutexGuard Guard(m_Mutex);
        m_ProgressStr = "Loading sequence.";
    }

    bool searchForward = true, searchReverse = true;

    ITERATE(vector<int>, iflt, m_Filters.selected) {
        string expr = m_Filters.filters[*iflt].second;
        if (expr.find("Strand=") == 0 && expr.size() > 7) {
            string value = expr.substr(7);
            if (value == "'+'")
                searchReverse = false;
            else if (value == "'-'")
                searchForward = false;
        }
    }

    if (!searchForward && !searchReverse) 
        searchForward = searchReverse = true;

    /// for every Seq_loc
    NON_CONST_ITERATE(TScopedLocs, it_sl, scoped_locs)    {
        /// extract context data
        TScopedLoc& sc_loc = *it_sl;
        CSeq_loc& seq_loc = *sc_loc.m_Loc;
        CScope& scope = *sc_loc.m_Scope;

        string loc_label;
        CLabel::GetLabel(seq_loc, &loc_label, CLabel::eDefault, &scope);

        CSeq_id_Handle loc_id;
        CConstRef<CSeq_id> id(seq_loc.GetId());
        if (id) {
            loc_id = CSeq_id_Handle::GetHandle(*id);
        } else {
            loc_id = CSeq_id_Handle::GetHandle(sequence::GetId(seq_loc, sc_loc.m_Scope));
            id = loc_id.GetSeqId();
        }

        // create an id that will be used in results
        CRef<CSeq_id> res_id(new CSeq_id());
        res_id->Assign(*id);


        if(IsCanceled()) {
            return eCanceled;
        }

        CSeqVector vec(seq_loc, scope, CBioseq_Handle::eCoding_Iupac);

        TSeqPos seq_offset = 0;
        if (seq_loc.IsInt())
            seq_offset = seq_loc.GetInt().GetFrom();

        TRangeCollection coll;
        TSeqPos len = vec.size();
        TSeqPos span = 10000; //TODO
        TSeqPos step =  span - 1000;
        string data;

        if (searchForward) {
            for (TSeqPos start = 0;  start < len  &&  ! IsCanceled();  start += step) {
                data.erase();
                span = min(span, TSeqPos(vec.size() - start));
                vec.GetSeqData(start, start + span, data);

                x_GetMatches(data, start, coll, false);
                if(m_ResultsCount + (int) coll.size() >= kMaxResults)  {
                    break; // end the loop and add the accumulated results
                }

                if(m_ResultsCount + (int) coll.size() >= kMaxResults)  {
                    break; // end the loop and add the accumulated results
                }
            }

            // create results from the collection
            ITERATE (TRangeCollection, it, coll) {
                CRange<TSeqPos> range(it->first, it->second);

                CRef<CSeq_loc> loc(new CSeq_loc(*res_id,
                                        range.GetFrom() + seq_offset,
                                        range.GetTo() + seq_offset));
                string seq;
                x_GetSequence(vec, range, &seq);

                x_AddToResults(*loc, scope, seq, loc_label, "+", sc_loc.m_ContextName);
                m_ResultsCount++;
                if(m_ResultsCount >= kMaxResults)  {
                    return eCompleted;
                }
            }
        }

        coll.clear();

        span = 10000; //TODO
        step =  span - 1000;

        if (searchReverse) {
            for (TSeqPos start = 0;  start < len  &&  ! IsCanceled();  start += step) {
                data.erase();
                span = min(span, TSeqPos(vec.size() - start));
                vec.GetSeqData(start, start + span, data);

                CSeqManip::ReverseComplement(data, CSeqUtil::e_Iupacna, 0, span);
                x_GetMatches(data, start, coll, true);

                if(m_ResultsCount + (int) coll.size() >= kMaxResults)  {
                    break; // end the loop and add the accumulated results
                }
            }
            ITERATE (TRangeCollection, it, coll) {
                CRange<TSeqPos> range(it->first, it->second);

                CRef<CSeq_loc> loc(new CSeq_loc(*res_id,
                                        range.GetFrom() + seq_offset,
                                        range.GetTo() + seq_offset));
                string seq;
                x_GetSequence(vec, range, &seq);

                x_AddToResults(*loc, scope, seq, loc_label, "-", sc_loc.m_ContextName);
                m_ResultsCount++;
                if(m_ResultsCount >= kMaxResults)  {
                    return eCompleted;
                }
            }
        }
    }
    return IsCanceled() ? eCanceled : eCompleted;
}


/// "offset" argument defines the sequence position corresponding to the fisrt
/// character in "data"
void CSequenceSearchJob::x_GetMatches(const string& data, int offset,
                                      TRangeCollection& coll, bool reverse)
{
    // calculate the upper limit on the expected number of matches
    int max_coll_size = kMaxResults - m_ResultsCount;

    switch(m_PatternType)   {
    case CSearchToolBase::eExactMatch:
        {
            SIZE_TYPE pos = 0; // start of a match
            SIZE_TYPE occurrence = 0; // search position
            SIZE_TYPE len = m_SearchStr.size();

            while((int) coll.size() < max_coll_size)   {
                pos = NStr::Find(data, m_SearchStr, NStr::eNocase, NStr::eForwardSearch, occurrence++);
                if(pos != NPOS) {
                    TSeqPos from = (int)(reverse ? offset + data.size() - pos - len : offset + pos);
                    TSeqPos to = from + len - 1;
                    coll.insert(make_pair(from, to));
                } else {
                    break;
                }
            }
            break;
        }
    case CSearchToolBase::eWildcard:
    case CSearchToolBase::eRegexp:
    case CSearchToolBase::eNamed:
        {
            int start_pos = 0, num = 0;
            while(num >= 0) {            
                m_Pattern->GetMatch(data, start_pos, 0, CRegexp::fMatch_default, true);
                num = m_Pattern->NumFound();

                for( int i = 0;  i < num  &&  max_coll_size > 0  && ! IsCanceled();  ++i ) {
                    const int* p = m_Pattern->GetResults(i);
                    int p0, p1;
                    if (reverse) {
                        p0 = (int)(data.size() - p[1]);
                        p1 = (int)(data.size() - p[0]);
                    }
                    else {
                        p0 = p[0];
                        p1 = p[1];
                    }

                    TSeqPos from = p0 + offset;
                    TSeqPos to = p1 + offset - 1;

                    coll.insert(make_pair(from, to));

                    start_pos = p[1];
                    if((int) coll.size() >= max_coll_size)    {
                        return;
                    }
                }
            }
            break;
        }
    default:
        _ASSERT(false);
        break;
    }
}

// generate sequence label to be disaplayed in UI
void CSequenceSearchJob::x_GetSequence(CSeqVector& vec,
                                       const CRange<TSeqPos>& range,
                                       string* s)
{
    static const TSeqPos kMaxLen = 40;
    if(s)   {
        if (range.GetLength() > kMaxLen) {
            string s1, s2;
            vec.GetSeqData(range.GetFrom(), range.GetFrom() + kMaxLen / 2, s1);
            vec.GetSeqData(range.GetTo() - kMaxLen / 2, range.GetTo() + 1, s2);
            *s = s1 + "..." + s2;
        } else {
            vec.GetSeqData(range.GetFrom(), range.GetTo() + 1, *s);
        }
    }
}


void CSequenceSearchJob::x_AddToResults(CObject& obj, CScope& scope,
                                        const string& sequence,
                                        const string& loc_name,
                                        const string& strand,
                                        const string& ctx_name)
{
    static const int kUpdateIncrement = 250;

    /// adding the result to the Accumulator
    int row = m_AccList.AddRow(&obj, &scope);

    CSeq_loc * loc = dynamic_cast<CSeq_loc*>(&obj);

    int col = 0;
    //m_AccList.SetString(col++, row, loc_name);
    m_AccList.SetString(col++, row, sequence);

    if (m_PatternType==CSearchToolBase::eNamed) {
        m_AccList.SetInteger(col++, row, (int)loc->GetTotalRange().GetFrom() + 4);
        m_AccList.SetString(col++, row, m_SearchStr);
    }

    m_AccList.SetInteger(col++, row, (int)loc->GetTotalRange().GetFrom()+1);
    m_AccList.SetInteger(col++, row, (int)loc->GetTotalRange().GetTo()+1);
    m_AccList.SetString(col++, row, strand);
    m_AccList.SetString(col++, row, ctx_name);


    int count = m_AccList.GetNumRows();
    if(count >= kUpdateIncrement)   {
        // time to update the Result
        CMutexGuard Guard(m_Mutex);

        /// transfer results from Accumulator to m_TempResult
        m_TempResult->GetObjectList()->Append(m_AccList);
        m_AccList.ClearRows();

        /// update progress string
        m_ProgressStr = NStr::IntToString(count, NStr::fWithCommas) + " match";
        if (count != 1) {
            m_ProgressStr += "es";
        }
        m_ProgressStr += " found.";
    }
}

CObjectListTableModel* CSequenceSearchJob::x_GetNewOLTModel() const
{
    return new CObjectListTableModel();
}

END_NCBI_SCOPE
