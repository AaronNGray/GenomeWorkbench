/*  $Id: component_search_tool.cpp 36583 2016-10-12 15:11:14Z evgeniev $
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
#include "component_search_tool.hpp"

#include <corelib/ncbitime.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/utils/app_job_dispatcher.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_map_ci.hpp>

#include <gui/utils/extension_impl.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>


#define ID_COMBOBOX 10003
#define ID_TEXT     11414


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// IDMSearchTool
CComponentSearchTool::CComponentSearchTool()
{
}


string CComponentSearchTool::GetName() const
{
    static string name("Component Search");
    return name;
}


IUITool* CComponentSearchTool::Clone() const
{
    return new CComponentSearchTool();
}


string CComponentSearchTool::GetDescription() const
{
    return "";
}


CIRef<IDMSearchForm> CComponentSearchTool::CreateSearchForm()
{
    CIRef<IDMSearchForm> form(new CComponentSearchForm(*this));
    return form;
}


bool CComponentSearchTool::IsCompatible(IDataMiningContext* context)
{
    ISeqLocSearchContext* sl_ctx =
        dynamic_cast<ISeqLocSearchContext*>(context);
    return sl_ctx != NULL;
}


CRef<CSearchJobBase> CComponentSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    CComponentSearchQuery* f_query = dynamic_cast<CComponentSearchQuery*>(&query);
    if(f_query) {
        job.Reset(new CComponentSearchJob(*f_query));
    }
    return job;
}



string  CComponentSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::component_search_tool";
}

string  CComponentSearchTool::GetExtensionLabel() const
{
    return "Datamining Tool - Component search";
}


///////////////////////////////////////////////////////////////////////////////
/// CComponentSearchQuery

CComponentSearchQuery::CComponentSearchQuery(TScopedLocs& locs, const string& ids)
:   CSearchQueryBase(locs),
    m_Ids(ids)
{
}


///////////////////////////////////////////////////////////////////////////////
/// CComponentSearchForm

CComponentSearchForm::CComponentSearchForm(CComponentSearchTool& tool)
:   m_Tool(&tool)
{
}


void CComponentSearchForm::Create()
{
    x_CreateWidgets();
}


void CComponentSearchForm::Init()
{
    CSearchFormBase::Init();
    


  //  x_UpdateData(eDDX_Set);
}


void CComponentSearchForm::UpdateContexts()
{
    CSearchFormBase::UpdateContexts();
}


void CComponentSearchForm::x_CreateWidgets()
{
//    CSearchFormBase::x_CreateWidgets(2, 3, true);
}


IDMSearchTool* CComponentSearchForm::x_GetTool()
{
    return m_Tool.GetPointer();
    
}


wxSizer *  CComponentSearchForm::GetWidget(wxWindow * parent)
{
    if (!m_Sizer) {     
        wxFlexGridSizer * sz = new wxFlexGridSizer(1, 4, 0, 0);     
        sz->AddGrowableCol(3);
        m_Sizer = sz;
        
        m_Sizer->Add(new wxStaticText( parent, wxID_STATIC, wxT("Search Context:"), wxDefaultPosition, wxDefaultSize, 0 ),
            0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   

        m_DbCombo = new wxChoice(parent, ID_COMBOBOX,
                                   wxDefaultPosition, wxDefaultSize,
                                   0, (const wxString*)NULL);
        
        m_Sizer->Add(m_DbCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);     
        
        m_Sizer->Add(new wxStaticText( parent, wxID_STATIC, wxT("Search Expression:"), wxDefaultPosition, wxDefaultSize, 0 ),
            0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   
    
        m_Text = new CSearchControl(parent, ID_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
        m_Sizer->Add(m_Text,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
        
    }
    return m_Sizer;
}

void  CComponentSearchForm::Update()
{
    UpdateContextCombo(m_DbCombo);
}

CIRef<IDMSearchQuery> CComponentSearchForm::ConstructQuery()
{

    CIRef<IDMSearchQuery> ref;

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
    
    ref = new CComponentSearchQuery(sc_locs, ToStdString(m_Text->GetValue()));
    return ref;
}


///////////////////////////////////////////////////////////////////////////////
/// CComponentSearchJob

CComponentSearchJob::CComponentSearchJob(CComponentSearchQuery& query)
:   m_Query(&query)
{
    string s_locs, id;
    static string sep(", ");
    NON_CONST_ITERATE(TScopedLocs, it, m_Query->GetScopedLocs())    {
        CLabel::GetLabel(*it->m_Loc, &id, CLabel::eDefault,
            it->m_Scope.GetPointer());

        if ( !s_locs.empty() ) {
            s_locs += sep;
        }
        s_locs += id;
    }
    m_Descr = "Search Components \"" + m_Query->GetIds() + "\" on " + s_locs;
}


bool CComponentSearchJob::x_ValidateParams()
{
    m_Error.Reset();

    if( m_Query->GetScopedLocs().empty())   {
        // this should never happen
        m_Error = new CAppJobError("Invalid input parameters - search context is undefined.");
    } else if(m_Query->GetIds().empty())  {
        m_Error = new CAppJobError("No sequence ids are provided.");
    }
    return m_Error ? false : true;
}


void CComponentSearchJob::x_SetupColumns(CObjectList& obj_list)
{
    obj_list.AddColumn(CObjectList::eString, "Component");
    obj_list.AddColumn(CObjectList::eString, "Context");
}


//TODO this functions is too long
IAppJob::EJobState CComponentSearchJob::x_DoSearch()
{
    CStopWatch sw;
    sw.Start();
    {
        CMutexGuard Guard(m_Mutex);
        m_ProgressStr = "Loading data." ;
    }

    TScopedLocs& scoped_locs = m_Query->GetScopedLocs();
    string ids = m_Query->GetIds();

    /// for every Seq_loc
    int count = 0;
    int found = 0;
    NON_CONST_ITERATE(TScopedLocs, it_sl, scoped_locs)    {
        if(IsCanceled()) {
            break;
        }
        /// extract context data
        TScopedLoc& sc_loc = *it_sl;
        CSeq_loc& seq_loc = *sc_loc.m_Loc;
        CScope& scope = *sc_loc.m_Scope;

        string loc_label;
        CLabel::GetLabel(seq_loc, &loc_label, CLabel::eDefault, &scope);

        /// determine our list of seq-ids from the input query
        list<CSeq_id_Handle> idhs;
        list<string> toks;
        NStr::Split(ids, " \t\n;:,", toks, NStr::fSplit_Tokenize);
        ITERATE (list<string>, iter, toks) {
            const string& s = *iter;
            try {
                CSeq_id id(s);
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);

                // make sure seq_id's lifetime is long enough to ensure
                // a valid tex_id
                CConstRef<CSeq_id> seq_id = idh.GetSeqId();
                const CTextseq_id* txt_id = seq_id->GetTextseq_Id();
                if (txt_id  &&  txt_id->IsSetVersion()) {
                    try {
                        CSeq_id_Handle gi = sequence::GetId(idh, scope, sequence::eGetId_ForceGi);
                        if (gi) {
                            idh = gi;
                        }
                    }
                    catch (CException& e) {
                        LOG_POST(Info << "accession: " << s
                                 << "not a gi: " << e.GetMsg());
                    }
                }

                idhs.push_back(idh);
            }
            catch (CException& e) {
                LOG_POST(Error << "error handling accession: " << s
                         << ": " << e.GetMsg());
            }
        }

        /// retrieve the sequence of interest and establish our search range
        CSeq_id_Handle bsh_idh = sequence::GetIdHandle(seq_loc, &scope);
        CBioseq_Handle bsh = scope.GetBioseqHandle(bsh_idh);
        const CSeqMap& seqmap = bsh.GetSeqMap();
        TSeqRange range;
        if (seq_loc.IsWhole()) {
            range.SetFrom(0);
            range.SetTo(bsh.GetBioseqLength());
        } else {
            range = seq_loc.GetTotalRange();
        }

        /// scan up to five levels deep in the seq-map structure
        for (size_t level = 0;
             level < 5  &&  ! IsCanceled()  &&  idhs.size();  ++level) {
            CSeqMap_CI map_it =
                seqmap.ResolvedRangeIterator(&scope, range.GetFrom(),
                                             range.GetLength(), eNa_strand_plus,
                                             level, CSeqMap::fFindRef);

            for ( ;  idhs.size()  &&  map_it  &&  ! IsCanceled();  ++map_it) {
                CSeq_id_Handle handle = map_it.GetRefSeqid();

                if (map_it.GetType() != CSeqMap::eSeqRef) {
                    continue;
                }

                list<CSeq_id_Handle>::iterator id_it = idhs.begin();
                for ( ;  id_it != idhs.end();  ) {
                    if (CSeqUtils::Match(*id_it, handle, &scope)) {

                        /// format the label for this entry
                        string seq_id_str;
                        CLabel::GetLabel(*handle.GetSeqId(), &seq_id_str,
                                         CLabel::eDefault, &scope);

                        /// get a location for display
                        CRef<CSeq_loc> loc(new CSeq_loc);
                        loc->SetInt().SetFrom(map_it.GetPosition());
                        loc->SetInt().SetTo  (map_it.GetEndPosition() - 1);
                        loc->SetId(*bsh_idh.GetSeqId());
                        x_AddToResults(*loc, scope, seq_id_str,
                                       sc_loc.m_ContextName);
                        idhs.erase(id_it++);
                        ++found;
                    } else {
                        ++id_it;
                        ++count;
                        if (count % 20 == 0) {
                            // time to update the Result
                            CMutexGuard Guard(m_Mutex);
                            m_ProgressStr =
                                NStr::IntToString(count, NStr::fWithCommas) +
                                " components searched, " +
                                NStr::IntToString(found, NStr::fWithCommas) +
                                " component";
                            if (found != 1) {
                                m_ProgressStr += "s";
                            }
                            m_ProgressStr += " found.";
                        }
                    }
                }
            }
        }
    }

    LOG_POST(Info << "CComponentSearchJob::x_DoSearch(): done: "
             << sw.Elapsed() << " seconds. Found = " << found);
    return IsCanceled() ? eCanceled : eCompleted;
}


void CComponentSearchJob::x_AddToResults(CObject& obj,
                                         CScope& scope,
                                         const string& loc_name,
                                         const string& ctx_name)
{
    static const int kUpdateIncrement = 20;

    int row = m_AccList.AddRow(&obj, &scope);
    m_AccList.SetString(0, row, loc_name);
    m_AccList.SetString(1, row, ctx_name);

    int count = m_AccList.GetNumRows();
    if(count >= kUpdateIncrement)   {
        // time to update the Result
        CMutexGuard Guard(m_Mutex);

        /// transfer results from Accumulator to m_TempResult
        m_TempResult->GetObjectList()->Append(m_AccList);
        m_AccList.ClearRows();
    }
}


END_NCBI_SCOPE
