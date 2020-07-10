/*  $Id: cpg_search_tool.cpp 39744 2017-10-31 21:12:13Z katargir $
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

#include "cpg_search_tool.hpp"


#include <algo/sequence/cpg.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbireg.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seq/Seq_annot.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/extension_impl.hpp>

#include <objmgr/util/sequence.hpp>

#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <algo/sequence/orf.hpp>


#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>


#define ID_COMBOBOX 10003
#define ID_TEXT             11414
#define ID_COMBOBOX_UPDATE  10111

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kCpgSizeOfSlidingWindow = "CpgSizeOfSlidingWindow";
static const char* kCpgMinLenOfIsland      = "CpgMinLenOfIsland";
static const char* kCpgMinGC               = "CpgMInGC";
static const char* kCpgMinPercentage       = "CpgMinPercentage";
static const char* kCpgAdjMergeTHreshold   = "CpgAdjMergeThreshold";

/*
class CCpgSearchTableModel : public CObjectListTableModel
{
    virtual int GetNumExtraColumns() const;
};

int CCpgSearchTableModel::GetNumExtraColumns() const
{
    // we can use just the first column from the set of standard ones
    // ie "Label"
    return 1;
}
*/

/*
class CCpgSearchColumnHandler : public IObjectColumnHandler
{
public:
    virtual int         GetColumnsCount();
    virtual void        SetObjectList(CObjectList& obj_list);
    virtual wxString    GetColumnName(int col);
    virtual wxString    GetImageAlias(int row);
    virtual void        GetValue(wxString* str, int col, int row);

    static int      GetColumnsCountStatic();
    //static void     GetValueStatic(string* str, CNetBlastJobDescriptor& descr, int col);
protected:
    CRef<CObjectList>   m_ObjectList;
};


static string s_Columns[] = {"Label", "Start", "Stop", "ORF Length",
                             "Begining Codon", "Terminal Codon", "Translated Sequence",
                             "Nucleotide Sequence", "Context"};


void CCpgSearchColumnHandler::SetObjectList(CObjectList& obj_list)
{
    m_ObjectList.Reset(&obj_list);
}


int CCpgSearchColumnHandler::GetColumnsCount()
{
   return GetColumnsCountStatic();
}


wxString CCpgSearchColumnHandler::GetColumnName(int col)
{
    return s_Columns[col];
}

wxString CCpgSearchColumnHandler::GetImageAlias(int row)
{
    return "";   //TODO
}

void CCpgSearchColumnHandler::GetValue(wxString * str, int col, int row)
{
    if(col < 0  ||  col >= GetColumnsCount())  {
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Invalid column index");
    }
    if (str) {
        switch (col) {
        case 0:
            {{
                CObject* obj = m_ObjectList->GetObject(row);
                CScope* scope = m_ObjectList->GetScope(row);

                string label;
                CLabel::GetLabel( *obj, &label, CLabel::eContent, scope );
                *str = label;
            }}
            break;

        case 1: // start
        case 2: // stop
        case 3: // length
            /// treat as int
            *str = NStr::IntToString(m_ObjectList->GetInteger(col+2, row),
                                     NStr::fWithCommas);
            break;

        default:
            /// treat as string
            *str = m_ObjectList->GetString(col+2, row);
            break;
        }
    }
}

int CCpgSearchColumnHandler::GetColumnsCountStatic()
{
    return sizeof(s_Columns) / sizeof(string);
}
*/


///////////////////////////////////////////////////////////////////////////////
/// CCpgSearchTool
CCpgSearchTool::CCpgSearchTool()
{
    m_Conversions.options.clear();
    m_Conversions.options.push_back(CSeqFeatData::eSubtype_region);
    m_Conversions.selected = CSeqFeatData::eSubtype_region;
}


string CCpgSearchTool::GetName() const
{
    static string name("CpG Islands Search");
    return name;
}


IUITool* CCpgSearchTool::Clone() const
{
    return new CCpgSearchTool();
}


string CCpgSearchTool::GetDescription() const
{
    return "";
}


CIRef<IDMSearchForm> CCpgSearchTool::CreateSearchForm()
{
    CIRef<IDMSearchForm> form(new CCpgSearchForm(*this));
    return form;
}


bool CCpgSearchTool::IsCompatible(IDataMiningContext* context)
{
    ISeqLocSearchContext* sl_ctx =
        dynamic_cast<ISeqLocSearchContext*>(context);
    return sl_ctx != NULL;
}


CRef<CSearchJobBase> CCpgSearchTool::x_CreateJob(IDMSearchQuery& query)
{
    CRef<CSearchJobBase> job;
    CCpgSearchQuery* f_query = dynamic_cast<CCpgSearchQuery*>(&query);
    if(f_query) {
        job.Reset(new CCpgSearchJob(*f_query));
    }
    return job;
}



string  CCpgSearchTool::GetExtensionIdentifier() const
{
    return "search_tool::cpg_search_tool";
}

string  CCpgSearchTool::GetExtensionLabel() const
{
    return "Datamining Tool - CpG Islands search";
}


///////////////////////////////////////////////////////////////////////////////
/// CCpgSearchQuery

CCpgSearchQuery::CCpgSearchQuery(TScopedLocs& locs,
                        const string&  xSizeOfSlidingWindow,
                        const string&  xMinLenOfIsland,
                        const string&  xMinGC,
                        const string&  xMinPercentage,
                        const string&  xAdjMergeThreshold)
:   CSearchQueryBase(locs),
    m_SizeOfSlidingWindow(xSizeOfSlidingWindow),
    m_MinLenOfIsland(xMinLenOfIsland),
    m_MinGC(xMinGC),
    m_MinPercentage(xMinPercentage),
    m_AdjMergeThreshold(xAdjMergeThreshold)
{
}

string CCpgSearchQuery::ToString() const
{
    string s;

    s.append(" SizeOfSlidingWindow=");
    s.append(m_SizeOfSlidingWindow);
    s.append(" MinLenOfIsland=");
    s.append(m_MinLenOfIsland);
    s.append(" MinGC=");
    s.append(m_MinGC);
    s.append(" MinPercentage=");
    s.append(m_MinPercentage);
    s.append(" AdjMergeThreshold=");
    s.append(m_AdjMergeThreshold);

    return s;
}


///////////////////////////////////////////////////////////////////////////////
/// CCpgSearchForm


CCpgSearchForm::CCpgSearchForm(CCpgSearchTool& tool)
:   m_Tool(&tool),
    m_DbCombo(NULL),
    m_SizeOfSlidingWindowCtl(NULL),
    m_MinLenOfIslandCtl(NULL),
    m_MinGCCtl(NULL),
    m_MinPercentageCtl(NULL),
    m_AdjMergeThresholdCtl(NULL)
{
}


CCpgSearchForm::~CCpgSearchForm()
{
}

void CCpgSearchForm::Create()
{
}

void CCpgSearchForm::Init()
{
    CSearchFormBase::Init();
}


void CCpgSearchForm::x_LoadSettings(const CRegistryReadView& view)
{
    m_SizeOfSlidingWindow   = view.GetString(kCpgSizeOfSlidingWindow);
    m_MinLenOfIsland        = view.GetString(kCpgMinLenOfIsland);
    m_MinGC                 = view.GetString(kCpgMinGC);
    m_MinPercentage         = view.GetString(kCpgMinPercentage);
    m_AdjMergeThreshold     = view.GetString(kCpgAdjMergeTHreshold);
}


void CCpgSearchForm::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_SizeOfSlidingWindowCtl) {
        view.Set(kCpgSizeOfSlidingWindow, ToStdString( m_SizeOfSlidingWindowCtl->GetValue() ));
    }
    if (m_MinLenOfIslandCtl) {
        view.Set(kCpgMinLenOfIsland, ToStdString( m_MinLenOfIslandCtl->GetValue() ));
    }
    if (m_MinGCCtl) {
        view.Set(kCpgMinGC, ToStdString( m_MinGCCtl->GetValue() ));
    }
    if (m_MinPercentageCtl) {
        view.Set(kCpgMinPercentage, ToStdString( m_MinPercentageCtl->GetValue() ));
    }
    if (m_AdjMergeThresholdCtl) {
        view.Set(kCpgAdjMergeTHreshold, ToStdString( m_AdjMergeThresholdCtl->GetValue() ));
    }
}


IDMSearchTool* CCpgSearchForm::x_GetTool()
{
    return m_Tool.GetPointer();
}


wxSizer *  CCpgSearchForm::GetWidget(wxWindow * parent)
{
    if (!m_Sizer) {
        wxBoxSizer * vSz = new wxBoxSizer(wxVERTICAL);
        m_Sizer = vSz;

        wxFlexGridSizer * sz = new wxFlexGridSizer(3, 4, 0, 0);
        sz->AddGrowableCol(1);
        sz->AddGrowableCol(3);

        m_Sizer->Add(sz, 0, wxGROW|wxALL, 0);


        sz->Add(new wxStaticText( parent, wxID_STATIC, wxT("Search Context:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_DbCombo = new wxChoice(parent, ID_COMBOBOX,
                                   wxDefaultPosition, wxDefaultSize,
                                   0, (const wxString*)NULL);

        sz->Add(m_DbCombo,1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // 1-st param
        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Size of sliding window:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_SizeOfSlidingWindowCtl =
            new wxTextCtrl(parent, wxID_STATIC, wxT("100"),
                           wxDefaultPosition, wxDefaultSize, 0 );
        if (!m_SizeOfSlidingWindow.empty()) {
            m_SizeOfSlidingWindowCtl->SetValue(ToWxString(m_SizeOfSlidingWindow));
        } else {
            m_SizeOfSlidingWindowCtl->SetValue(wxT("200"));
        }
        sz->Add(m_SizeOfSlidingWindowCtl, 1,
                wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // 2-nd param
        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Minimum length of island:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_MinLenOfIslandCtl = new wxTextCtrl(parent, wxID_STATIC, wxT("500"),
                                             wxDefaultPosition, wxDefaultSize,
                                             0 );
        if (!m_MinLenOfIsland.empty()) {
            m_MinLenOfIslandCtl->SetValue(ToWxString(m_MinLenOfIsland));
        } else {
            m_MinLenOfIslandCtl->SetValue(wxT("500"));
        }
        sz->Add(m_MinLenOfIslandCtl, 1,
                wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // 3-rd param
        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Minimum /%G + /%C:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_MinGCCtl = new wxTextCtrl(parent, wxID_STATIC, wxT("100"),
                                    wxDefaultPosition, wxDefaultSize, 0 );
        if (!m_MinGC.empty()) {
            m_MinGCCtl->SetValue(ToWxString(m_MinGC));
        } else {
            m_MinGCCtl->SetValue(wxT("50"));
        }
        sz->Add(m_MinGCCtl, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // 4-th param
        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Minimum CpG percentage:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_MinPercentageCtl = new wxTextCtrl(parent, wxID_STATIC, wxT("60"),
                                            wxDefaultPosition, wxDefaultSize,
                                            0 );
        if (!m_MinPercentage.empty()) {
            m_MinPercentageCtl->SetValue(ToWxString(m_MinPercentage));
        } else {
            m_MinPercentageCtl->SetValue(wxT("60"));
        }
        sz->Add(m_MinPercentageCtl, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // 5-th param
        sz->Add(new wxStaticText( parent, wxID_STATIC,
                                  wxT("Adjacency merge threshold:"),
                                  wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        m_AdjMergeThresholdCtl =
            new wxTextCtrl(parent, wxID_STATIC, wxT(""),
                           wxDefaultPosition, wxDefaultSize, 0 );
        if (!m_AdjMergeThreshold.empty()) {
            m_AdjMergeThresholdCtl->SetValue(ToWxString(m_AdjMergeThreshold));
        } else {
            m_AdjMergeThresholdCtl->SetValue(wxT(""));
        }
        sz->Add(m_AdjMergeThresholdCtl, 1,
                wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }
    return m_Sizer;
}


void CCpgSearchForm::Update()
{
    UpdateContextCombo(m_DbCombo);
}


void CCpgSearchForm::UpdateContexts()
{
    CSearchFormBase::UpdateContexts();
}
 
CIRef<IDMSearchQuery> CCpgSearchForm::ConstructQuery()
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

    CIRef<IDMSearchQuery> ref
        (new CCpgSearchQuery(sc_locs,
                             ToStdString(m_SizeOfSlidingWindowCtl->GetValue()),
                             ToStdString(m_MinLenOfIslandCtl->GetValue()),
                             ToStdString(m_MinGCCtl->GetValue()),
                             ToStdString(m_MinPercentageCtl->GetValue()),
                             ToStdString(m_AdjMergeThresholdCtl->GetValue())));
    return ref;
}



///////////////////////////////////////////////////////////////////////////////
/// CCpgSearchJob

CCpgSearchJob::CCpgSearchJob(CCpgSearchQuery& query)
:   m_Query(&query)
{
}


bool CCpgSearchJob::x_ValidateParams()
{
    m_Error.Reset();

    if( m_Query->GetScopedLocs().empty()) {
        m_Error = new CAppJobError("Invalid input parameters - no search context specified.");
    }

    return m_Error ? false : true;
}


IAppJob::EJobState CCpgSearchJob::x_DoSearch()
{
    EJobState res_state = IAppJob::eFailed;
    TScopedLocs& locs   = m_Query->GetScopedLocs();
    size_t sum_isles = 0;
    size_t row = 0;


     ITERATE (TScopedLocs, iter, locs) {
        TScopedLoc& sc_loc = const_cast<TScopedLoc&>(*iter);
        CSeq_loc& loc = *sc_loc.m_Loc;
        CScope& scope = *sc_loc.m_Scope;

        CBioseq_Handle handle = scope.GetBioseqHandle(loc);
        CSeqVector sv(loc, scope, CBioseq_Handle::eCoding_Iupac);

        string data;
        sv.GetSeqData(0, sv.size(), data);

        //find islands
        CCpGIslands isles(data.data(), data.size(),
                          NStr::StringToInt(m_Query->GetSizeOfSlidingWindow()),
                          NStr::StringToInt(m_Query->GetMinLenOfIsland()),
                          NStr::StringToInt(m_Query->GetMinGC()),
                          NStr::StringToInt(m_Query->GetMinPercentage()));

        // if requested, merge sufficiently close islands
        if (!m_Query->GetAdjMergeThreshold().empty()) {
            isles.MergeIslesWithin( NStr::StringToInt(m_Query->GetAdjMergeThreshold()));
        }

         //report islands
        if (!isles.GetIsles().empty()) {
            sum_isles += isles.GetIsles().size();

            string title("CpG islands on ");
            CLabel::GetLabel(loc, &title,
                             CLabel::eDefault, &scope);

            CRef<CSeq_annot> sa(new CSeq_annot);
            sa->SetNameDesc(title);
            sa->SetCreateDate(CTime(CTime::eCurrent));
            CNcbiOstrstream oss;
            oss << "windowsize: " << m_Query->GetSizeOfSlidingWindow()
                << ", minlen: " << m_Query->GetMinLenOfIsland()
                << ", min %gc: " << m_Query->GetMinGC()
                << ", min observed/expected CpG: "
                << m_Query->GetMinPercentage();
            sa->AddComment(CNcbiOstrstreamToString(oss));

            CSeq_annot::C_Data::TFtable &feats = sa->SetData().SetFtable();
            ITERATE (CCpGIslands::TIsles, i, isles.GetIsles()) {
                CRef<CSeq_feat> feat(new CSeq_feat);
                CSeq_interval &seqInt = feat->SetLocation().SetInt();
                seqInt.SetFrom(i->m_Start);
                seqInt.SetTo(i->m_Stop);
                seqInt.SetId().Assign(sequence::GetId(loc, &scope));

                CRef<CSeq_loc> new_loc =
                    CSeqUtils::RemapChildToParent(loc, feat->GetLocation());
                feat->SetLocation(*new_loc);

                size_t size = (i->m_Stop - i->m_Start);
                size_t pct_gc = (i->m_C + i->m_G) * 100 / size;
                string gc_str = NStr::SizetToString(pct_gc);

                string& label = feat->SetData().SetRegion();
                label = "CpG island: " + gc_str;
                label += "% GC, ";
                label += NStr::SizetToString(size);
                label += " bases";

                feats.push_back(feat);

                ++row;

                x_AddToResults(*new_loc,
                           scope,
                           label,
                           new_loc->GetTotalRange().GetFrom()+1,
                           new_loc->GetTotalRange().GetTo()+1,
                           new_loc->GetTotalRange().GetLength(),
                           pct_gc,
                           sc_loc.m_ContextName);

            }

            /*
            x_AddToResults(*sa,
                           scope,
                           title,
                           sc_loc.m_ContextName);
                           */
        }
        res_state = IAppJob::eCompleted;
    }

    return res_state;
}


void CCpgSearchJob::x_SetupColumns(CObjectList& obj_list)
{   
	obj_list.ClearObjectLabels();
	obj_list.AddObjectLabel( "Label", CLabel::eContent );

    obj_list.AddColumn( CObjectList::eInteger, "Start" );
    obj_list.AddColumn( CObjectList::eInteger, "Stop" );
    obj_list.AddColumn( CObjectList::eInteger, "Length" );
    obj_list.AddColumn( CObjectList::eInteger, "% GC" );
    obj_list.AddColumn(CObjectList::eString, "Context");
    
}

void CCpgSearchJob::x_AddToResults(CObject& obj, objects::CScope& scope,
                           const string& loc_name,
                           int   start,
                           int   stop,
                           int   length,
                           int   gc,                                  
                           const string& ctx_name)
{
    static const int kUpdateIncrement = 250;

    /// adding the result to the Accumulator
    int row = m_AccList.AddRow(&obj, &scope);
    int col = 0;
    m_AccList.SetInteger(col++, row, start);
    m_AccList.SetInteger(col++, row, stop);
    m_AccList.SetInteger(col++, row, length);
    m_AccList.SetInteger(col++, row, gc);
    m_AccList.SetString(col++, row, ctx_name);

    int count = m_AccList.GetNumRows();
    if(count >= kUpdateIncrement)   {
        // time to update the Result
        CMutexGuard Guard(m_Mutex);

        /// transfer results from Accumulator to m_TempResult
        m_TempResult->GetObjectList()->Append(m_AccList);
        m_AccList.ClearRows();

        /// update progress string
        m_ProgressStr = NStr::IntToString(count, NStr::fWithCommas) + " CPG";
        if (count != 1) {
            m_ProgressStr += "'s";
        }
        m_ProgressStr += " found.";
    }
}

CObjectListTableModel* CCpgSearchJob::x_GetNewOLTModel() const
{
    return new CObjectListTableModel();
}


END_NCBI_SCOPE
