/*  $Id: hit_matrix_widget.cpp 33117 2015-05-28 19:53:58Z shkeda $
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

#include "save_hit_matrix_pdf_dlg.hpp"
#include <gui/opengl/glfont.hpp>
#include <gui/graph/igraph_utils.hpp>
#include <gui/types.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_widget.hpp>
#include <gui/widgets/hit_matrix/wx_choose_seq_dlg.hpp>
#include <gui/widgets/hit_matrix/wx_score_dlg.hpp>
#include <gui/widgets/hit_matrix/wx_histogram_dlg.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/utils/menu_item.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/impl/handle_range_map.hpp>


#include <list>


#include <wx/msgdlg.h>
#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////////////////////////////////////////////////////////////////////////////////
/// class CHitMatrixWidget

/// register custom commands
void CHitMatrixWidget::RegisterCommands(CUICommandRegistry& cmd_reg,
                                       wxFileArtProvider& provider)
{
    bool static initialized = false;
    if( ! initialized)  {
        cmd_reg.RegisterCommand(eCmdChooseSeq, "Choose Alignments to Display...",
                                "Choose Alignments to Display", "", "");
        cmd_reg.RegisterCommand(eCmdColorByScore, "Color by Score...", "Color by Score", "", "");
        cmd_reg.RegisterCommand(eCmdDisableColoring, "Disable Coloring", "Disable Coloring", "", "");
        cmd_reg.RegisterCommand(eCmdSetupGraphs, "Setup Graphs...", "Setup Graphs", "", "");

        initialized = true;
    }
}


CHitMatrixWidget::CHitMatrixWidget(wxWindow* parent, wxWindowID id,
                                   const wxPoint& pos, const wxSize& size, long style)
:   CGlWidgetBase(parent, id, pos, size, style),
    m_DataSource(NULL),
    m_MatrixPane(NULL)
{
    // setup Port
    m_Port.SetAdjustmentPolicy(CGlPane::fAdjustAll, CGlPane::fAdjustAll);
    m_Port.SetMinScaleX(1 / 30.0);
    m_Port.SetOriginType(CGlPane::eOriginLeft, CGlPane::eOriginBottom);
    m_Port.EnableZoom(true, true);
}


CHitMatrixWidget::~CHitMatrixWidget()
{
    x_ClearScoreToParamsMap();
}


BEGIN_EVENT_TABLE(CHitMatrixWidget, CGlWidgetBase)
    EVT_CONTEXT_MENU(CHitMatrixWidget::OnContextMenu)
    EVT_MENU(eCmdZoomSel, CHitMatrixWidget::OnZoomSelection)
    EVT_MENU(eCmdZoomSelObjects, CHitMatrixWidget::OnZoomToSelectedHitElems)
    EVT_MENU(eCmdSetEqualScale, CHitMatrixWidget::OnSetEqualScale)
    EVT_MENU(eCmdZoomObjects, CHitMatrixWidget::OnZoomToHits)

    EVT_MENU(eCmdChooseSeq, CHitMatrixWidget::OnChooseSeq)
    EVT_MENU(eCmdColorByScore, CHitMatrixWidget::OnColorByScore)
    EVT_MENU(eCmdDisableColoring, CHitMatrixWidget::OnDisableColoring)
    EVT_MENU(eCmdSetupGraphs, CHitMatrixWidget::OnSetupGraphs)
    EVT_MENU(wxID_CLEAR, CHitMatrixWidget::OnResetSelection)

    EVT_UPDATE_UI(eCmdZoomSel, CHitMatrixWidget::OnUpdateZoomSelection)
    EVT_UPDATE_UI(eCmdZoomSelObjects, CHitMatrixWidget::OnUpdateZoomToSelectedHitElems)
    EVT_UPDATE_UI(eCmdDisableColoring, CHitMatrixWidget::OnUpdateDisableColoring)

    EVT_MENU(eCmdSavePdf, CHitMatrixWidget::OnSavePdf)
    EVT_UPDATE_UI(eCmdSavePdf, CHitMatrixWidget::OnEnableSavePdfCmdUpdate)

END_EVENT_TABLE()


void CHitMatrixWidget::x_CreatePane()
{
    m_MatrixPane.reset(new CHitMatrixPane(this));
}


CGlWidgetPane*    CHitMatrixWidget::x_GetPane()
{
    return static_cast<CGlWidgetPane*>(m_MatrixPane.get());
}


void CHitMatrixWidget::x_SetPortLimits()
{
    TModelRect rc_m(0, 0, 1000, 1000);

    IHitMatrixDataSource* pDS = GetDS();
    if(pDS) {
        CBioseq_Handle s_handle = pDS->GetSubjectHandle();
        if(s_handle)    {
            rc_m.SetHorz(0, s_handle.GetBioseqLength());
        } else {
            TSeqRange s_r = pDS->GetSubjectHitsRange();
            rc_m.SetHorz(s_r.GetFrom(), s_r.GetToOpen());
        }

        CBioseq_Handle q_handle = pDS->GetQueryHandle();
        if(q_handle)    {
            rc_m.SetVert(0, q_handle.GetBioseqLength());
        } else {
            TSeqRange q_r = pDS->GetQueryHitsRange();
            rc_m.SetVert(q_r.GetFrom(), q_r.GetToOpen());
        }
    }
    m_Port.SetModelLimitsRect(rc_m);
}


CGlPane& CHitMatrixWidget::GetPort(void)
{
    return m_Port;
}


const CGlPane& CHitMatrixWidget::GetPort(void) const
{
    return m_Port;
}


void CHitMatrixWidget::SetDataSource(IHitMatrixDataSource* p_ds)
{
    CIRef<IHitMatrixDataSource> guard = m_DataSource; // keep it alive until update is completed

    m_DataSource = p_ds;

    x_Update();
}


CHitMatrixRenderer& CHitMatrixWidget::GetRenderer()
{
    return m_MatrixPane->GetRenderer();
}


IHitMatrixDataSource*  CHitMatrixWidget::GetDS(void)
{
    return m_DataSource;
}


void CHitMatrixWidget::OnZoomToHits(wxCommandEvent& event)
{
    if(m_DataSource)   {
        x_ZoomToHits();
        x_UpdateOnZoom();
    }
}


void CHitMatrixWidget::OnZoomSelection(wxCommandEvent& event)
{
    if(m_DataSource)   {
        const CHitMatrixPane::TRangeColl& horz_sel
            = m_MatrixPane->GetRangeSelection(CHitMatrixPane::eSubject);
        const CHitMatrixPane::TRangeColl& vert_sel
            = m_MatrixPane->GetRangeSelection(CHitMatrixPane::eQuery);

        bool en = horz_sel.size()  ||  vert_sel.size();
        if(en)  {
            TModelRect rc_m = m_Port.GetVisibleRect();
            if(horz_sel.size())    {
                rc_m.SetHorz(horz_sel.GetFrom(), horz_sel.GetToOpen());
            }
            if(vert_sel.size())    {
                rc_m.SetVert(vert_sel.GetFrom(), vert_sel.GetToOpen());
            }
            m_Port.ZoomRect(rc_m);
            x_UpdateOnZoom();
        }
    }
}


void CHitMatrixWidget::OnUpdateZoomSelection(wxUpdateUIEvent& event)
{
    const CHitMatrixPane::TRangeColl& horz_sel
        = m_MatrixPane->GetRangeSelection(CHitMatrixPane::eSubject);
    const CHitMatrixPane::TRangeColl& vert_sel
        = m_MatrixPane->GetRangeSelection(CHitMatrixPane::eQuery);
    bool en = horz_sel.size()  ||  vert_sel.size();
    event.Enable(en);
}


void CHitMatrixWidget::OnZoomToSelectedHitElems(wxCommandEvent& event)
{
    TModelRect rc_m = GetRenderer().GetSelectedHitElemsRect();

    // inflate 5% from every side
    TModelUnit off_x = rc_m.Width() * 0.05;
    TModelUnit off_y = rc_m.Height() * 0.05;
    rc_m.Inflate(off_x, off_y);

    if(! rc_m.IsEmpty())    {
        m_Port.ZoomRect(rc_m);
        x_UpdateOnZoom();
    }
}


void CHitMatrixWidget::OnUpdateZoomToSelectedHitElems(wxUpdateUIEvent& event)
{
    TModelRect rc_m = GetRenderer().GetSelectedHitElemsRect();
    event.Enable(! rc_m.IsEmpty());
}


void CHitMatrixWidget::x_ZoomToHits(void)
{
    if(m_DataSource)   {
        TModelRect rc_m = m_Port.GetModelLimitsRect();

        TSeqRange s_r = m_DataSource->GetSubjectHitsRange();
        if( ! s_r.Empty()) {
            rc_m.SetHorz(s_r.GetFrom(), s_r.GetToOpen());
        }
        TSeqRange q_r = m_DataSource->GetQueryHitsRange();
        if( ! q_r.Empty())  {
            rc_m.SetVert(q_r.GetFrom(), q_r.GetToOpen());
        }

        m_Port.ZoomRect(rc_m);
    }
}


void CHitMatrixWidget::OnSetEqualScale(wxCommandEvent& event)
{
    TModelUnit scale = min(m_Port.GetScaleX(), m_Port.GetScaleY());
    m_Port.SetScale(scale, scale);
    x_UpdateOnZoom();
}


void CHitMatrixWidget::OnChooseSeq(wxCommandEvent& event)
{
    if(m_DataSource.GetPointer()  &&  m_MatrixPane.get())   {

        // get current settings
        const IHitSeqId* subject_id = &m_DataSource->GetSubjectId();
        const IHitSeqId* query_id = &m_DataSource->GetQueryId();
        IHitMatrixDataSource::SParams old_params, params;
        m_DataSource->GetParams(old_params);

        // create and show the Dialog
        CwxChooseSeqDlg dlg(this);
        dlg.SetTitle(wxT("Choose Alignments to Display..."));

        dlg.SetSeqs(m_DataSource);

        if(dlg.ShowModal() == wxID_OK) {
            const IHitSeqId* new_subject_id = dlg.GetSubjectId();
            const IHitSeqId* new_query_id = dlg.GetQueryId();
            dlg.GetParams(params);

            //LOG_POST(Info << "dlg.ShowModal() subject "
            //         << m_DataSource->GetLabel(*new_subject_id)
            //         << ", query " << m_DataSource->GetLabel(*new_query_id));

            if(! new_subject_id->Equals(*subject_id)  ||
               ! new_query_id->Equals(*query_id)  || ! (old_params == params))   {
                query_id = new_query_id;
                subject_id = new_subject_id;

                m_DataSource->SetParams(params, false);
                m_DataSource->SelectIds(*subject_id, *query_id);

                x_Update(); // create new graph
            }
        }
    }
}


static const wxChar* kMsgNoScores = wxT("The alignment does not contain any scores.");
static const wxChar* kColorByScore = wxT("Color by Score...");
static const int kGradSteps = 64;


void CHitMatrixWidget::OnColorByScore(wxCommandEvent& event)
{
    if(m_DataSource.GetPointer()  &&  m_MatrixPane.get())   {
        string  curr_name = m_MatrixPane->GetRenderer().GetScoreName();

        vector<string> names;
        m_DataSource->GetScoreNames(names);

        if(names.size())    {
            if(m_ScoreToParams.size() == 0) { // initailize params
                ITERATE(vector<string>, it, names)  {
                    const string& name = *it;
                    SHitColoringParams* params = new SHitColoringParams;
                    params->m_ScoreName = name;

                    // TODO load from configuration

                    pair<double, double> range = m_DataSource->GetScoreRange(name);
                    params->SetValueRange(range.first, range.second);
                    params->m_MinGrad = range.first;
                    params->m_MaxGrad = range.second;

                    m_ScoreToParams[name] = params;
                }
            }

            // create and show the Dialog
            CwxScoreDlg dlg(this);
            dlg.SetTitle(kColorByScore);

            vector<SHitColoringParams*> params;
            NON_CONST_ITERATE(TScoreToParamsMap, it_p, m_ScoreToParams) {
                SHitColoringParams* p = it_p->second;
                p->m_Steps = kGradSteps;
                params.push_back(p);
            }
            dlg.SetItems(params, curr_name);

            if(dlg.ShowModal() == wxID_OK)  {
                curr_name = dlg.GetSelectedName();
                SHitColoringParams* ptr =
                    curr_name.empty() ? NULL : &*m_ScoreToParams[curr_name];
                m_MatrixPane->GetRenderer().ColorByScore(ptr);
                x_RedrawControls();
            }
        } else {
            wxMessageBox(kMsgNoScores, kColorByScore,
                         wxOK | wxICON_EXCLAMATION);
        }
    }
}


void CHitMatrixWidget::OnDisableColoring(wxCommandEvent& event)
{
    if(m_DataSource.GetPointer()  &&  m_MatrixPane.get())   {
        m_MatrixPane->GetRenderer().ColorByScore(NULL);
        x_RedrawControls();
    }
}


void CHitMatrixWidget::OnSetupGraphs(wxCommandEvent& event)
{
    CBioseq_Handle s_handle = m_DataSource->GetSubjectHandle();
    CBioseq_Handle q_handle = m_DataSource->GetQueryHandle();

    vector<CHitMatrixRenderer::SGraphDescr> graph_types;
    GetRenderer().GetGraphTypes(graph_types);

    vector<string> s_graphs, q_graphs;
    bool en_s = false, en_q = false;

    if(s_handle)    {
        GetRenderer().GetGraphs(CHitMatrixRenderer::eSubject, s_graphs);
        en_s = true;
    }
    if(q_handle)    {
        GetRenderer().GetGraphs(CHitMatrixRenderer::eQuery, q_graphs);
        en_q = true;
    }

    CwxHistogramDlg dlg(this);

    dlg.SetItems(graph_types, s_graphs, q_graphs, en_s, en_q);

    if(dlg.ShowModal() == wxID_OK) {
        s_graphs.clear();
        q_graphs.clear();
        dlg.GetSelected(s_graphs, q_graphs);

        // reconfigure the pane
        for( size_t i = 0; i < graph_types.size(); i++ )    {
            const string& name = graph_types[i].m_Type;
            GetRenderer().SetGraphColor(name, dlg.GetGraphColor(name));
        }

        if(en_s)    {
            GetRenderer().SetGraphs(CHitMatrixRenderer::eSubject, s_graphs);
        }
        if(en_q)    {
            GetRenderer().SetGraphs(CHitMatrixRenderer::eQuery, q_graphs);
        }
        if(en_s  ||  en_q)  {
            GetRenderer().Layout(m_Port);
        }

        x_RedrawControls(); // ? TODO
    }
}


void CHitMatrixWidget::OnResetSelection(wxCommandEvent& event)
{
    TRangeColl empty;
    SetSubjectRangeSelection(empty);
    SetQueryRangeSelection(empty);
}


void CHitMatrixWidget::OnUpdateDisableColoring(wxUpdateUIEvent& event)
{
    bool bEn = m_MatrixPane.get() ?
        ( ! GetRenderer().GetScoreName().empty()) : false;
    event.Enable(bEn);
}

static
WX_DEFINE_MENU(sPopupMenu)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Zoom")
    WX_MENU_ITEM(eCmdZoomInMouse)
    WX_MENU_ITEM(eCmdZoomOutMouse)
    WX_MENU_ITEM(eCmdZoomAll)
    WX_MENU_ITEM(eCmdZoomObjects)
    WX_MENU_SEPARATOR_L("Zoom Selection")
    WX_MENU_ITEM(eCmdZoomSel)
    WX_MENU_ITEM(eCmdZoomSelObjects)
    WX_MENU_SEPARATOR_L("Zoom Special")
    WX_SUBMENU("Zoom Special")
        WX_MENU_ITEM(eCmdZoomInX)
        WX_MENU_ITEM(eCmdZoomOutX)
        WX_MENU_ITEM(eCmdZoomAllX)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdZoomInY)
        WX_MENU_ITEM(eCmdZoomOutY)
        WX_MENU_ITEM(eCmdZoomAllY)
        WX_MENU_ITEM(eCmdSetEqualScale)
        WX_MENU_SEPARATOR()
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdResetSelection)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdChooseSeq)
    WX_MENU_ITEM(eCmdSetupGraphs)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdColorByScore)
    WX_MENU_ITEM(eCmdDisableColoring)
WX_END_MENU()


void CHitMatrixWidget::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint ms_pos = ScreenToClient(event.GetPosition());

    m_PopupPoint.m_X = m_MatrixPane->SHH_GetModelByWindow(ms_pos.x, eHorz);
    m_PopupPoint.m_Y = m_MatrixPane->SHH_GetModelByWindow(ms_pos.y, eVert);

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* menu = cmd_reg.CreateMenu(sPopupMenu);

    x_ShowDecoratedPopupMenu(menu);
}


/*CConstRef<CSeq_align_set>    CHitMatrixWidget::GetSelectedHits() const
{
    _ASSERT(m_MatrixPane.get());
    return m_MatrixPane->GetSelectedHits();
}*/


const CHitMatrixWidget::TRangeColl&
    CHitMatrixWidget::GetSubjectRangeSelection() const
{
    return m_MatrixPane->GetRangeSelection(CHitMatrixPane::eSubject);
}


const CHitMatrixWidget::TRangeColl&
    CHitMatrixWidget::GetQueryRangeSelection() const
{
    return m_MatrixPane->GetRangeSelection(CHitMatrixPane::eQuery);
}


void   CHitMatrixWidget::SetSubjectRangeSelection(const TRangeColl& segs)
{
    m_MatrixPane->SetRangeSelection(segs, CHitMatrixPane::eSubject);
}


void   CHitMatrixWidget::SetQueryRangeSelection(const TRangeColl& segs)
{
    m_MatrixPane->SetRangeSelection(segs, CHitMatrixPane::eQuery);
}


void CHitMatrixWidget::ResetObjectSelection()
{
    m_MatrixPane->ResetObjectSelection();
}


void CHitMatrixWidget::GetObjectSelection(TConstObjects& objs) const
{
    m_MatrixPane->GetObjectSelection(objs);
}


void CHitMatrixWidget::SetObjectSelection(const vector<const CSeq_align*> sel_aligns)
{
    m_MatrixPane->SetObjectSelection(sel_aligns);
}


void CHitMatrixWidget::SetQueryVisibleRange(const TSeqRange& range)
{
    m_MatrixPane->SetQueryVisibleRange(range);
}


void CHitMatrixWidget::SetSubjectVisibleRange(const TSeqRange& range)
{
    m_MatrixPane->SetSubjectVisibleRange(range);
}


void CHitMatrixWidget::x_Update()
{
    x_SetPortLimits();

    if(m_DataSource)   {
        x_ZoomToHits();
    } else {
        m_Port.ZoomAll();
    }

    x_UpdatePane();
    x_UpdateScrollbars();

    x_RedrawControls();
}

void CHitMatrixWidget::x_UpdatePane()
{
    m_MatrixPane->Update();
}


// zooms X and Y proprotionaly using horizontal scale as master
void CHitMatrixWidget::SetScaleX(TModelUnit scale_x, const TModelPoint& point)
{
    TModelUnit sc_x = m_Port.GetScaleX();
    TModelUnit sc_y = m_Port.GetScaleY();
    TModelUnit scale_y = (scale_x / sc_x) * sc_y;
    m_Port.SetScaleRefPoint(scale_x, scale_y, point);

    x_UpdateOnZoom();
}



void CHitMatrixWidget::x_ClearScoreToParamsMap()
{
    /**
    ITERATE(TScoreToParamsMap, it, m_ScoreToParams) {
        delete it->second;
    }
    **/
    m_ScoreToParams.clear();
}


void CHitMatrixWidget::OnSavePdf(wxCommandEvent& /* evt */)
{
    IHitMatrixDataSource* pDS = GetDS();
    if (!pDS) 
        return;
    m_MatrixPane->Update();
    string file_name = pDS->GetLabel (pDS->GetSubjectId());
    file_name += "x";
    file_name += pDS->GetLabel(pDS->GetQueryId());
    file_name += ".pdf";

    CSaveHitMatrixPdfDlg dlg(m_MatrixPane.get(), this, file_name);
    dlg.ShowModal();
}

void CHitMatrixWidget::OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(GetDS() != 0);
}


END_NCBI_SCOPE
