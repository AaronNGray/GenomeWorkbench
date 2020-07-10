/*  $Id: cross_aln_widget.cpp 42766 2019-04-10 20:45:59Z katargir $
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
 * Authors:  Vlad Lebedev
 *
 */

#include <ncbi_pch.hpp>

#include "cross_aln_pane.hpp"

#include <gui/widgets/aln_crossaln/cross_aln_widget.hpp>

#include <gui/widgets/hit_matrix/wx_choose_seq_dlg.hpp>
#include <gui/widgets/hit_matrix/wx_score_dlg.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/utils/menu_item.hpp>
#include <objmgr/util/sequence.hpp>

#include <wx/sizer.h>
#include <wx/menu.h>

BEGIN_NCBI_SCOPE

void CCrossAlnWidget::RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    cmd_reg.RegisterCommand(eCmdZoomInQuery, "Zoom In Query", "Zoom In Query", "", "");
    cmd_reg.RegisterCommand(eCmdZoomOutQuery, "Zoom Out Query", "Zoom Out Query", "", "");
    cmd_reg.RegisterCommand(eCmdZoomAllQuery, "Zoom All Query", "Zoom All Query", "", "");
    cmd_reg.RegisterCommand(eCmdZoomSeqQuery, "Zoom To Sequence Query", "Zoom To Sequence Query", "", "");


    cmd_reg.RegisterCommand(eCmdZoomInSubject, "Zoom In Subject", "Zoom In Subject", "", "");
    cmd_reg.RegisterCommand(eCmdZoomOutSubject, "Zoom Out Subject", "Zoom Out Subject", "", "");
    cmd_reg.RegisterCommand(eCmdZoomAllSubject, "Zoom All Subject", "Zoom All Subject", "", "");
    cmd_reg.RegisterCommand(eCmdZoomSeqSubject, "Zoom To Sequence Subject", "Zoom To Sequence Subject", "", "");

    cmd_reg.RegisterCommand(eCmdChooseSeq, "Choose Sequence To Display...", "Choose Sequence To Display...", "", "");
}

static
WX_DEFINE_MENU(sPopupMenu)
    WX_MENU_ITEM(eCmdZoomIn)
    WX_MENU_ITEM(eCmdZoomOut)
    WX_MENU_ITEM(eCmdZoomAll)
    WX_MENU_ITEM(eCmdZoomSel)
    WX_MENU_ITEM(eCmdZoomSeq)
    WX_MENU_SEPARATOR()
    WX_SUBMENU("Zoom Special")
        WX_MENU_ITEM(eCmdZoomInQuery)
        WX_MENU_ITEM(eCmdZoomOutQuery)
        WX_MENU_ITEM(eCmdZoomAllQuery)
        WX_MENU_ITEM(eCmdZoomSeqQuery)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdZoomInSubject)
        WX_MENU_ITEM(eCmdZoomOutSubject)
        WX_MENU_ITEM(eCmdZoomAllSubject)
        WX_MENU_ITEM(eCmdZoomSeqSubject)
    WX_END_SUBMENU()
    WX_MENU_ITEM(eCmdChooseSeq)
WX_END_MENU()

BEGIN_EVENT_TABLE(CCrossAlnWidget, CGlWidgetBase)
    EVT_SIZE(CCrossAlnWidget::OnSize)
    EVT_COMMAND_SCROLL(ID_SCROLLBAR_Q, CCrossAlnWidget::OnScrollQ)
    EVT_COMMAND_SCROLL(ID_SCROLLBAR_S, CCrossAlnWidget::OnScrollS)

    EVT_CONTEXT_MENU(CCrossAlnWidget::OnContextMenu)
    EVT_MENU(eCmdZoomIn, CCrossAlnWidget::OnZoomIn)
    EVT_MENU(eCmdZoomOut, CCrossAlnWidget::OnZoomOut)
    EVT_MENU(eCmdZoomAll, CCrossAlnWidget::OnZoomAll)

    EVT_MENU(eCmdZoomInQuery, CCrossAlnWidget::OnZoomIn_Q)
    EVT_MENU(eCmdZoomOutQuery, CCrossAlnWidget::OnZoomOut_Q)
    EVT_MENU(eCmdZoomAllQuery, CCrossAlnWidget::OnZoomAll_Q)

    EVT_MENU(eCmdZoomInSubject, CCrossAlnWidget::OnZoomIn_S)
    EVT_MENU(eCmdZoomOutSubject, CCrossAlnWidget::OnZoomOut_S)
    EVT_MENU(eCmdZoomAllSubject, CCrossAlnWidget::OnZoomAll_S)

    EVT_MENU(eCmdZoomSeq, CCrossAlnWidget::OnZoomToSeq)
    EVT_MENU(eCmdZoomSeqQuery, CCrossAlnWidget::OnZoomToSeq_Q)
    EVT_MENU(eCmdZoomSeqSubject, CCrossAlnWidget::OnZoomToSeq_S)

    EVT_MENU(eCmdZoomSel, CCrossAlnWidget::OnZoomSelection)
    EVT_UPDATE_UI(eCmdZoomSel, CCrossAlnWidget::OnUpdateZoomSelection)

    EVT_MENU(eCmdChooseSeq, CCrossAlnWidget::OnChooseSeq)
    //EVT_MENU(eCmdColorUsingScore, CCrossAlnWidget::OnColorByScore)
    //EVT_MENU(eCmdColorUsingSegments, CCrossAlnWidget::OnColorBySegments)
END_EVENT_TABLE()




CCrossAlnWidget::CCrossAlnWidget(wxWindow* parent, wxWindowID id,
            const wxPoint& pos, const wxSize& size, long style)
    : CGlWidgetBase(parent, id, pos, size, style)
    , m_CrossPane(NULL)
{
}


CCrossAlnWidget::~CCrossAlnWidget()
{
}


void CCrossAlnWidget::x_CreateControls()
{
    _ASSERT( ! x_GetPane());

    x_CreatePane();

    CGlWidgetPane* child_pane = x_GetPane();
    _ASSERT(child_pane);

    wxFlexGridSizer* sizer = new wxFlexGridSizer(3, 1, 0, 0);
    this->SetSizer(sizer);

    sizer->AddGrowableCol(0);
    sizer->AddGrowableRow(1);


    wxScrollBar* sb = new wxScrollBar(this, ID_SCROLLBAR_Q, wxDefaultPosition,
                                      wxDefaultSize, wxSB_HORIZONTAL);
    sizer->Add(sb, 0, wxEXPAND);

    sizer->Add(child_pane, 1, wxEXPAND);

    sb = new wxScrollBar(this, ID_SCROLLBAR_S, wxDefaultPosition,
                         wxDefaultSize, wxSB_HORIZONTAL);
    sizer->Add(sb, 0, wxEXPAND);
}

void CCrossAlnWidget::x_CreatePane()
{
    m_CrossPane.reset(new CCrossAlnPane(this));
}


CGlWidgetPane* CCrossAlnWidget::x_GetPane()
{
    return static_cast<CGlWidgetPane*>(m_CrossPane.get());
}


void CCrossAlnWidget::x_SetPortLimits()
{
}


CGlPane& CCrossAlnWidget::GetPort(void)
{
    return m_Port;
}


const CGlPane& CCrossAlnWidget::GetPort(void) const
{
    return m_Port;
}


void CCrossAlnWidget::x_Update()
{
    m_CrossPane->Update();
    //x_UpdatePane();
    UpdateScrollbars();
    x_RedrawControls();
}


void CCrossAlnWidget::x_UpdatePane()
{
    //m_CrossPane->Update();
}



void CCrossAlnWidget::OnUpdateZoomSelection(wxUpdateUIEvent& event)
{
    const TRangeColl& sel1 = m_CrossPane->GetQueryRangeSelection();
    const TRangeColl& sel2 = m_CrossPane->GetSubjectRangeSelection();

    bool en = sel1.size()  ||  sel2.size();
    event.Enable(en);
}


void CCrossAlnWidget::OnZoomSelection(wxCommandEvent& event)
{
    const TRangeColl& sel1 = m_CrossPane->GetQueryRangeSelection();
    const TRangeColl& sel2 = m_CrossPane->GetSubjectRangeSelection();

    if (sel1.size()) {
        m_CrossPane->ZoomOnRange_Q(sel1.GetFrom(), sel1.GetToOpen());
    }
    if (sel2.size()) {
        m_CrossPane->ZoomOnRange_S(sel2.GetFrom(), sel2.GetToOpen());
    }
}


void CCrossAlnWidget::OnChooseSeq(wxCommandEvent& event)
{
    CIRef<ICrossAlnDataSource> ds(m_CrossPane->GetDataSource());

    if (ds) {
        // get current settings
        const IHitSeqId* query_id = &ds->GetQueryId();
        const IHitSeqId* subject_id = &ds->GetSubjectId();

        IHitMatrixDataSource::SParams old_params, params;
        ds->GetParams(old_params);

        // create and show the Dialog
        CwxChooseSeqDlg dlg(this);
        dlg.SetTitle(wxT("Choose Alignments to Display..."));

        dlg.SetSeqs(ds);

        if(dlg.ShowModal() == wxID_OK) {
            const IHitSeqId* new_subject_id = dlg.GetSubjectId();
            const IHitSeqId* new_query_id = dlg.GetQueryId();
            dlg.GetParams(params);

            if(! new_subject_id->Equals(*subject_id)  ||
               ! new_query_id->Equals(*query_id)  || ! (old_params == params))   {
                query_id = new_query_id;
                subject_id = new_subject_id;

                if (!(old_params == params))
                    ds->SetParams(params, false);

                ds->SelectIds(*subject_id, *query_id);
                m_CrossPane->SetDataSource(ds.GetPointer());

                Update(); // create new cross-alignment
                //x_Update
            }
        }
    }
}

void CCrossAlnWidget::OnColorBySegments(wxCommandEvent& event)
{
    m_CrossPane->ColorBySegments();
}


void CCrossAlnWidget::OnColorByScore(wxCommandEvent& event)
{
    /*CCrossAlnDataSource* ds = m_CrossPane->GetDataSource();

    if(ds)
    {
        CConstRef<CObject_id> curr_sc_id = m_CrossPane->GetScoreId();
        int i_curr_sc = -1; // index of the currently used score

        // fill the list with all available scores
        int scores_n = ds->GetScoresCount();
        vector<string> items;
        for( int i = 0; i < scores_n; i++ )    {
            const CObject_id& id = ds->GetScoreId(i);
            if(curr_sc_id.NotEmpty()  &&  curr_sc_id->Match(id))    {
                i_curr_sc = i; // this one is current
            }

            items.push_back(id.GetStr());
        }

        // create and show the Dialog
        CScoreDlg dlg;
        dlg.SetTitle("Color by Score...");
        dlg.SetItems(items, i_curr_sc);
        dlg.CenterOnActive();

        if(dlg.ShowModal() == wxID_OK)  {
            int score_index = dlg.GetSelectedIndex();
            if(score_index != i_curr_sc)   { // things did change - update pane
                CConstRef<CObject_id>   id(&ds->GetScoreId(score_index));
                m_CrossPane->ColorByScore(id);
            }
        }
    }*/
}


// All
void CCrossAlnWidget::OnZoomAll(wxCommandEvent& event)
{
    m_CrossPane->ZoomAll();
}


void CCrossAlnWidget::OnZoomIn(wxCommandEvent& event)
{
    m_CrossPane->ZoomIn();
}


void CCrossAlnWidget::OnZoomOut(wxCommandEvent& event)
{
    m_CrossPane->ZoomOut();
}


// Query
void CCrossAlnWidget::OnZoomAll_Q(wxCommandEvent& event)
{
    m_CrossPane->ZoomAll_Q();
}


void CCrossAlnWidget::OnZoomIn_Q(wxCommandEvent& event)
{
    m_CrossPane->ZoomIn_Q();
}


void CCrossAlnWidget::OnZoomOut_Q(wxCommandEvent& event)
{
    m_CrossPane->ZoomOut_Q();
}


// Subject
void CCrossAlnWidget::OnZoomAll_S(wxCommandEvent& event)
{
    m_CrossPane->ZoomAll_S();
}


void CCrossAlnWidget::OnZoomIn_S(wxCommandEvent& event)
{
    m_CrossPane->ZoomIn_S();
}


void CCrossAlnWidget::OnZoomOut_S(wxCommandEvent& event)
{
    m_CrossPane->ZoomOut_S();
}




void CCrossAlnWidget::OnSize(wxSizeEvent& event)
{
    CGlWidgetBase::OnSize(event);

    CGlWidgetPane* child_pane = x_GetPane();
    if (!child_pane)
        return;

    int w, h;
    child_pane->GetClientSize(&w, &h);

    TVPRect rcVP(0, 0, w - 1, h - 1);
    //GetPort().SetViewport(rcVP);
    //GetPort().AdjustToLimits();
    if (!m_CrossPane->GetRDR())
    {
        return;
    } else {
        m_CrossPane->GetRDR()->SetViewport(rcVP);
    }

    UpdateScrollbars();
    Refresh();
}


static const int kScrollRange = 1000000;
static const double kPageInc = 0.8;

void CCrossAlnWidget::UpdateScrollbars()
{
    if (!m_CrossPane->GetRDR())
        return;

    wxScrollBar* sbQ = (wxScrollBar*)FindWindow(ID_SCROLLBAR_Q);
    wxScrollBar* sbS = (wxScrollBar*)FindWindow(ID_SCROLLBAR_S);

    const TModelRect& rcMQ = m_CrossPane->GetRDR()->GetRP_Q().GetModelLimitsRect();
    const TModelRect& rcVQ = m_CrossPane->GetRDR()->GetRP_Q().GetVisibleRect();

    const TModelRect& rcMS = m_CrossPane->GetRDR()->GetRP_S().GetModelLimitsRect();
    const TModelRect& rcVS = m_CrossPane->GetRDR()->GetRP_S().GetVisibleRect();

    if (sbQ) {
        CNormalizer norm(rcMQ.Left(), rcMQ.Right(), kScrollRange);

        int position = norm.RealToInt(rcVQ.Left());
        int thumb_size = norm.SizeToInt(rcVQ.Width());
        int page_size = norm.SizeToInt(rcVQ.Width() * kPageInc);

        sbQ->SetScrollbar(position, thumb_size, kScrollRange, page_size);
    }

    if (sbS) {
        CNormalizer norm(rcMS.Left(), rcMS.Right(), kScrollRange);

        int position = norm.RealToInt(rcVS.Left());
        int thumb_size = norm.SizeToInt(rcVS.Width());
        int page_size = norm.SizeToInt(rcVS.Width() * kPageInc);

        sbS->SetScrollbar(position, thumb_size, kScrollRange, page_size);
    }
}


void CCrossAlnWidget::OnScrollQ(wxScrollEvent& event)
{
    bool lock = wxGetKeyState(WXK_SHIFT);
    double pQ = m_CrossPane->NormalizeQ(event.GetPosition());

    m_CrossPane->Scroll(pQ, !lock ? pQ : 0);
    m_CrossPane->Refresh();
    UpdateScrollbars();
}

void CCrossAlnWidget::OnScrollS(wxScrollEvent& event)
{
    bool lock = wxGetKeyState(WXK_SHIFT);
    double pS = m_CrossPane->NormalizeS( event.GetPosition() );

    m_CrossPane->Scroll(!lock ? pS : 0, pS);
    m_CrossPane->Refresh();
    UpdateScrollbars();
}


void CCrossAlnWidget::OnZoomToSeq(wxCommandEvent& event)
{
    m_CrossPane->ZoomToSeq_Q();
    m_CrossPane->ZoomToSeq_S();
}


void CCrossAlnWidget::OnZoomToSeq_Q(wxCommandEvent& event)
{
    m_CrossPane->ZoomToSeq_Q();
}


void CCrossAlnWidget::OnZoomToSeq_S(wxCommandEvent& event)
{
    m_CrossPane->ZoomToSeq_S();
}


void CCrossAlnWidget::SetDataSource(ICrossAlnDataSource* ds)
{
    m_CrossPane->SetDataSource(ds);

    x_Update();
}


TSeqRange CCrossAlnWidget::GetSubjectVisibleRange() const
{
    TSeqRange range;
    const CGlPane& pane = m_CrossPane->GetRDR()->GetRP_S();
    range.SetFrom((TSeqPos)pane.GetVisibleRect().Left());
    range.SetTo  ((TSeqPos)pane.GetVisibleRect().Right());
    return range;
}


void CCrossAlnWidget::SetSubjectVisibleRange(const TSeqRange& range)
{
    m_CrossPane->ZoomOnRange_S(range.GetFrom(), range.GetTo());
}


TSeqRange CCrossAlnWidget::GetQueryVisibleRange() const
{
    TSeqRange range;
    const CGlPane& pane = m_CrossPane->GetRDR()->GetRP_Q();
    range.SetFrom((TSeqPos)pane.GetVisibleRect().Left());
    range.SetTo  ((TSeqPos)pane.GetVisibleRect().Right());
    return range;
}


void CCrossAlnWidget::SetQueryVisibleRange(const TSeqRange& range)
{
    m_CrossPane->ZoomOnRange_Q(range.GetFrom(), range.GetTo());
}


const CCrossAlnWidget::TRangeColl&
    CCrossAlnWidget::GetSubjectRangeSelection() const
{
    return m_CrossPane->GetSubjectRangeSelection();
}


const CCrossAlnWidget::TRangeColl&
    CCrossAlnWidget::GetQueryRangeSelection() const
{
    return m_CrossPane->GetQueryRangeSelection();
}


void CCrossAlnWidget::SetSubjectRangeSelection(const TRangeColl& coll)
{
    m_CrossPane->SetSubjectRangeSelection(coll);
}


void CCrossAlnWidget::SetQueryRangeSelection(const TRangeColl& coll)
{
    m_CrossPane->SetQueryRangeSelection(coll);
}


void CCrossAlnWidget::ResetObjectSelection()
{
    m_CrossPane->ResetObjectSelection();
}


void CCrossAlnWidget::GetObjectSelection(TConstObjects& objs) const
{
    m_CrossPane->GetObjectSelection(objs);
}


void CCrossAlnWidget::SetObjectSelection(const vector<const CSeq_align*> sel_aligns)
{
    m_CrossPane->SetObjectSelection(sel_aligns);
}


void CCrossAlnWidget::Update()
{
    m_CrossPane->Update();
    UpdateScrollbars();
    Refresh();
}


void CCrossAlnWidget::OnContextMenu(wxContextMenuEvent& event)
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    auto_ptr<wxMenu> menu(cmd_reg.CreateMenu(sPopupMenu));
    PopupMenu(menu.get());
}


END_NCBI_SCOPE
