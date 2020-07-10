/*  $Id: alnmulti_widget.cpp 45038 2020-05-13 01:22:22Z evgeniev $
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

#include <gui/widgets/aln_multiple/alnmulti_widget.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <gui/widgets/aln_multiple/wx_aln_properties_dlg.hpp>

#include <gui/widgets/aln_score/scoring_methods_dlg.hpp>

#include <gui/widgets/data/data_commands.hpp>
#include <gui/widgets/aln_score/properties_panel_dlg.hpp>

#include <gui/widgets/wx/popup_event.hpp>
#include <gui/widgets/wx/ui_tool.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/types.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/utils/menu_item.hpp>

#include <wx/menu.h>

#include "wx_export_alnmulti_img_dlg.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


////////////////////////////////////////////////////////////////////////////////
/// class CAlnMultiWidget

void CAlnMultiWidget::RegisterCommands(CUICommandRegistry& cmd_reg,
                                       wxFileArtProvider& provider)
{
    WidgetsData_RegisterCommands(cmd_reg, provider);

    bool static initialized = false;
    if( ! initialized)  {
        cmd_reg.RegisterCommand(eCmdMoveSelectedUp, "Move Selected Items Up", "Move Selected Items Up", "", "");

        cmd_reg.RegisterCommand(eCmdHideSelected, "Hide Selected", "Hide Selected", "", "");
        cmd_reg.RegisterCommand(eCmdUnhideSelected, "Unhide Selected", "Unhide Selected", "", "");
        cmd_reg.RegisterCommand(eCmdShowOnlySelected, "Show Only Selected", "Show Only Selected", "", "");
        cmd_reg.RegisterCommand(eCmdShowAll, "Show All", "Show All", "", "");

        cmd_reg.RegisterCommand(eCmdSetSelMaster, "Set Selected", "Set Selected", "", "");
        cmd_reg.RegisterCommand(eCmdUnsetMaster, "Unset", "Unset", "", "");

        cmd_reg.RegisterCommand(eCmdMarkSelected, "Mark Selected Regions", "Mark Selected", "", "");
        cmd_reg.RegisterCommand(eCmdUnMarkSelected, "UnMark Selected Regions", "UnMark Selected", "", "");
        cmd_reg.RegisterCommand(eCmdUnMarkAll, "UnMark All", "UnMark All", "", "");

        cmd_reg.RegisterCommand(eCmdRebuildAlignment, "Rebuild Alignment...", "Rebuild Alignment", "", "");

        initialized = true;
    }
}


CAlnMultiWidget::CAlnMultiWidget(wxWindow* parent,
                                  wxWindowID id,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  long style,
                                  const wxString& name)
:   CGlWidgetBase(parent, id, pos, size, style, name),
    m_DataSource(NULL),
    m_Model(NULL),
    m_AlignPane(NULL)
{
}


void CAlnMultiWidget::CreateWidget(CAlnMultiModel* model)
{
    // Really model needs to be created After the pane is created in x_CreateControls otherwise
    // we can have font problems.
    if (model != NULL) {
        m_Model = model;
        m_Model->SetListener(this);
    }

    x_CreateControls();

    TVPRect rcVP;
    rcVP.SetSize(m_AlignPane->GetAlignVPWidth(), m_AlignPane->GetAlignVPHeight());
    GetPort().SetViewport(rcVP);

    m_Model->SetPortLimits();
}

// Since the model creates fonts this has to be called After
// the pane (OpenGL context) is created, otherwise some fonts
// may not work correctly
CAlnMultiModel* CAlnMultiWidget::x_CreateModel()
{
    if (m_Model == NULL) {
        m_Model = new CAlnMultiModel();
        m_Model->Init();
        m_Model->SetListener(this);
    }

    return m_Model;   
}


CAlnMultiWidget::~CAlnMultiWidget()
{
    if (m_AlignPane) {
        m_AlignPane->RemoveListener(this);
        RemoveListener(m_AlignPane);
    }
    if (m_Model) {
        m_Model->SetListener(NULL);
        m_Model->ClearRows();
        delete m_Model;
    }
}


BEGIN_EVENT_TABLE(CAlnMultiWidget, CGlWidgetBase)
    EVT_CONTEXT_MENU(CAlnMultiWidget::OnContextMenu)

    EVT_MENU(eCmdSavePdf, CAlnMultiWidget::OnSavePdf)
    EVT_UPDATE_UI(eCmdSavePdf, CAlnMultiWidget::OnEnableSavePdfCmdUpdate)
    EVT_MENU(eCmdSaveSvg, CAlnMultiWidget::OnSaveSvg)
    EVT_UPDATE_UI(eCmdSaveSvg, CAlnMultiWidget::OnEnableSaveSvgCmdUpdate)

    EVT_MENU(eCmdZoomSel, CAlnMultiWidget::OnZoomSelection)
    EVT_UPDATE_UI(eCmdZoomSel, CAlnMultiWidget::OnUpdateZoomSelection)
    EVT_MENU(eCmdZoomSeq, CAlnMultiWidget::OnZoomSequence)
    EVT_MENU(eCmdSetSelMaster,    CAlnMultiWidget::OnMakeSelectedRowMaster)
    EVT_UPDATE_UI(eCmdSetSelMaster, CAlnMultiWidget::OnUpdateMakeSelectedRowMaster)
    EVT_MENU(eCmdUnsetMaster,     CAlnMultiWidget::OnUnsetMaster)
    EVT_UPDATE_UI(eCmdUnsetMaster,     CAlnMultiWidget::OnUpdateUnsetMaster)

    EVT_MENU(eCmdResetSelection,  CAlnMultiWidget::OnResetSelection)
    EVT_UPDATE_UI(eCmdResetSelection, CAlnMultiWidget::OnUpdateZoomSelection)

    EVT_MENU(eCmdMarkSelected,    CAlnMultiWidget::OnMarkSelected)
    EVT_UPDATE_UI(eCmdMarkSelected,    CAlnMultiWidget::OnUpdateMarkSelected)
    EVT_MENU(eCmdUnMarkSelected,  CAlnMultiWidget::OnUnMarkSelected)
    EVT_UPDATE_UI(eCmdUnMarkSelected,    CAlnMultiWidget::OnUpdateMarkSelected)
    EVT_MENU(eCmdUnMarkAll,   CAlnMultiWidget::OnUnMarkAll)
    EVT_MENU(eCmdHideSelected,   CAlnMultiWidget::OnHideSelected)
    EVT_MENU(eCmdUnhideSelected, CAlnMultiWidget::OnUnhideSelected)
    EVT_UPDATE_UI(eCmdUnhideSelected, CAlnMultiWidget::OnUpdateUnhideSelected)
    EVT_UPDATE_UI(eCmdShowOnlySelected, CAlnMultiWidget::OnHasSelected)
    EVT_MENU(eCmdShowOnlySelected, CAlnMultiWidget::OnShowOnlySelected)
    EVT_MENU(eCmdShowAll,   CAlnMultiWidget::OnShowAll)
    EVT_MENU(eCmdMoveSelectedUp,   CAlnMultiWidget::OnMoveSelectedUp)
    EVT_MENU(eCmdSettings,   CAlnMultiWidget::OnSettings)
    EVT_UPDATE_UI(eCmdSettings,   CAlnMultiWidget::OnUpdateSettings)
    EVT_MENU(eCmdAlnShowMethodsDlg, CAlnMultiWidget::OnShowMethodDlg)
    EVT_UPDATE_UI(eCmdAlnShowMethodsDlg,
                         CAlnMultiWidget::OnUpdateShowMethodDlg)
    EVT_MENU(eCmdMethodProperties, CAlnMultiWidget::OnMethodProperties)
    EVT_UPDATE_UI(eCmdMethodProperties,
                         CAlnMultiWidget::OnUpdateMethodProperties)
    EVT_MENU(eCmdDisableScoring, CAlnMultiWidget::OnDisableScoring)
    EVT_UPDATE_UI(eCmdDisableScoring, CAlnMultiWidget::OnUpdateDisableScoring)
    EVT_MENU(eCmdSetAsDefaultMethod, CAlnMultiWidget::OnSetAsDefaultMethod)
    EVT_MENU_RANGE(eCmdScoringMethodXXXX, eCmdScoringMethodLast,
                     CAlnMultiWidget::OnScoringMethod)
    EVT_UPDATE_UI_RANGE(eCmdScoringMethodXXXX,   eCmdScoringMethodLast,
                               CAlnMultiWidget::OnUpdateScoringMethod)
END_EVENT_TABLE()


////////////////////////////////////////////////////////////////////////////////
// IAlnMultiPaneParent implementation

IAlnMultiDataSource*  CAlnMultiWidget::GetDataSource()
{
    return m_DataSource;
}


const IAlnMultiDataSource*  CAlnMultiWidget::GetDataSource() const
{
    return m_DataSource;
}


/// changes visibility state for given vector of rows to "b_visible"
void CAlnMultiWidget::SetVisible(const vector<TNumrow>& rows, bool b_visible,
                                    bool b_invert_others)
{
    m_Model->SetVisible(rows, b_visible, b_invert_others);

    x_AdjustVisible(false);
    x_UpdateScrollbars();
    x_RedrawControls();
}


void CAlnMultiWidget::Select(const vector<TNumrow>& rows,
                             bool /*b_select*/,
                             bool /*b_invert_others*/)
{
    if(m_Model)    {
        CAlnMultiModel::TIndexVector  vsel;
        vsel.reserve(rows.size());

        for( size_t i = 0; i < rows.size();  i++  ) {
            CAlnMultiModel::TIndex index = m_Model->GetLineByRowNum(rows[i]);
            if(index >= 0)  { // for master row it can be -1
                vsel.push_back(index);
            }
        }

        m_Model->SLM_SelectItems(vsel, true);
        x_RedrawControls();
    }
}


void CAlnMultiWidget::SetStyleCatalog(const CRowStyleCatalog* catalog)
{
    _ASSERT(m_Model);
     m_Model->SetStyleCatalog(catalog);
}


const CWidgetDisplayStyle*      CAlnMultiWidget::GetDisplayStyle() const
{
    return m_Model->GetDisplayStyle();
}


void CAlnMultiWidget::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    CWidgetDisplayStyle* style = (CWidgetDisplayStyle*) m_Model->GetDisplayStyle();
    style->LoadSettings(view);

    x_UpdateOnStylesChanged();
}


void CAlnMultiWidget::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    if (m_DataSource && m_Model)    {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        IAlnExplorer::EAlignType align_type = IAlnExplorer::fInvalid;
        if (m_Model->GetNumRows() > 0) try {
            align_type = m_DataSource->GetAlignType();
        } catch (...) {
        }
        CWidgetDisplayStyle* style = (CWidgetDisplayStyle*) m_Model->GetDisplayStyle();
        style->SaveSettings(view, align_type);
    }
}


CGlPane& CAlnMultiWidget::GetPort()
{
    return m_Model->GetAlignPort();
}


const CGlPane& CAlnMultiWidget::GetPort() const
{
    return m_Model->GetAlignPort();
}


CGlWidgetPane* CAlnMultiWidget::x_GetPane()
{
    return m_AlignPane;
}


void CAlnMultiWidget::SetDataSource(IAlnMultiDataSource* p_ds, CScope* scope)
{
    _ASSERT(m_Model);

    m_Model->SetDataSource(NULL); // release the old one

    m_DataSource = p_ds;
    m_AlignPane->Init(scope);

    if(m_DataSource) {
        if (!m_DataSource->IsEmpty())   {
            m_Model->SetDataSource(m_DataSource);
            string method = m_Model->GetDefaultMethod();
            x_SetScoringMethod(method, false);
        }

        if (m_Model->GetDisplayStyle() != NULL) {
            bool create_consensus = m_Model->GetDisplayStyle()->m_ShowConsensus;
            m_DataSource->SetCreateConsensus(create_consensus);
        }        
    }

    x_UpdateOnDataChanged();
}


CAlnMultiModel* CAlnMultiWidget::GetModel()
{
    return m_Model;
}


const CAlnMultiModel* CAlnMultiWidget::GetModel() const
{
    return m_Model;
}


CAlnMultiWidget::TNumrow     CAlnMultiWidget::GetRowNumByLine(TLine line) const
{
    IAlignRow* p_row = m_Model->GetRowByLine(line);
    return p_row->GetRowNum();
}


/// return -1 if "row" does not exist or is not visible
int     CAlnMultiWidget::GetLineByRowNum(TNumrow row) const
{
    return m_Model->GetLineByRowNum(row);
}


void CAlnMultiWidget::OnChildResize()
{
    x_OnResize();
}


void CAlnMultiWidget::SetScaleX(TModelUnit scale_x, const TModelPoint& point)
{
    GetPort().SetScaleRefPoint(scale_x, GetPort().GetScaleY(), point);
    x_UpdateOnZoom();
}


void CAlnMultiWidget::ZoomRect(const TModelRect& rc)
{
    TSeqPos left = (TSeqPos) floor(rc.Left());
    TSeqPos right = (TSeqPos) ceil(rc.Right());
    x_ZoomToRange(left, right);
}


void CAlnMultiWidget::ZoomPoint(const TModelPoint& point, TModelUnit factor)
{
    GetPort().ZoomPoint(point, factor);
}


void CAlnMultiWidget::Scroll(TModelUnit d_x, TModelUnit d_y)
{
    GetPort().Scroll(d_x, d_y);
    x_UpdateOnZoom();
}


void CAlnMultiWidget::OnRowChanged(IAlignRow* p_row)
{
    if (p_row == m_Model->GetMasterRow()) {
        m_AlignPane->Update();
        x_AdjustVisible(false);
        x_UpdateScrollbars();
        x_RedrawControls();
    }
    else {
        x_UpdateOnRowHChanged();
    }
}


void CAlnMultiWidget::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint point  = event.GetPosition();

    bool from_keyboard = (point == wxPoint(-1, -1));
    if (! from_keyboard) {
        point = ScreenToClient(point);

        m_PopupPoint.m_X = m_AlignPane->SHH_GetModelByWindow(point.x, eHorz);
        m_PopupPoint.m_Y = m_AlignPane->SHH_GetModelByWindow(point.y, eVert);
    }

    // give listeners a chance to customize the menu //TODO
    wxMenu* menu = x_CreatePopupMenu();

    x_ShowDecoratedPopupMenu(menu);
}


void CAlnMultiWidget::OnColumnsChanged()
{
    x_UpdateStyleColumns();
    SaveSettings();
}

void CAlnMultiWidget::OnDSChanged(CDataChangeNotifier::CUpdate& update)
{
    switch(update.GetType())    {
    case CDataChangeNotifier::eError:
        NcbiMessageBox(CDCN_GetMessage(update),
                       //update.GetMessage(), 
                       eDialog_Ok, eIcon_Stop, "Error", eWrap);
        m_AlignPane->HideProgressPanel();
        break;
    case CDataChangeNotifier::eProgressReport:
        if (!CDCN_GetMessage(update).empty())
            m_AlignPane->ShowProgressPanel((float) update.GetProgressNorm(), 
            CDCN_GetMessage(update)
            );
        break;
    case CDataChangeNotifier::eChanged:
        try {
            m_Model->SetDataSource(nullptr);
            if (m_DataSource  &&  !m_DataSource->IsEmpty()) {
                m_Model->SetDataSource(m_DataSource);
                string method = m_Model->GetDefaultMethod();
                x_SetScoringMethod(method, false);
            }
            x_UpdateOnDataChanged();
        } catch (CException& e) {
            NcbiMessageBox(e.GetMsg(), eDialog_Ok, eIcon_Stop, "Error", eWrap);
        } catch (exception& e) {
            NcbiMessageBox(string(e.what()), eDialog_Ok, eIcon_Stop, "Error", eWrap);
        }
        m_AlignPane->HideProgressPanel();
        break;
    default:
        _ASSERT(false); //unexpected
    }
}


void CAlnMultiWidget::OnScoringProgress(float progress, const string& msg)
{
    m_AlignPane->ShowProgressPanel(progress, msg);
}


void CAlnMultiWidget::OnScoringFinished()
{
    m_AlignPane->HideProgressPanel();
}


/// overriding base class implementation
bool CAlnMultiWidget::IsRendererFocused()
{
    return FindFocus() == m_AlignPane;
}


// end IAlnMultiPaneParent implementation
////////////////////////////////////////////////////////////////////////////////


void CAlnMultiWidget::OnSavePdf(wxCommandEvent& /* evt */)
{
    x_OnSaveVectorImage(CPrintOptions::ePdf);
}

void CAlnMultiWidget::OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void CAlnMultiWidget::OnSaveSvg(wxCommandEvent& /* evt */)
{
    x_OnSaveVectorImage(CPrintOptions::eSvg);
}

void CAlnMultiWidget::OnEnableSaveSvgCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void CAlnMultiWidget::OnZoomSequence(wxCommandEvent& event)
{
    double w = 1.5 * GetDisplayStyle()->m_SeqFont.GetMetric(CGlTextureFont::eMetric_MaxCharWidth);
    GetPort().SetScale(1 / w, GetPort().GetScaleY());
    x_UpdateOnZoom();
}


void CAlnMultiWidget::OnZoomSelection(wxCommandEvent&)
{
    ZoomToSelection();
}

void CAlnMultiWidget::ZoomToSelection()
{
    CRange<TSeqPos> sel_r = m_AlignPane->GetSelectionLimits();
    if(sel_r.NotEmpty()) {
        x_ZoomToRange(sel_r.GetFrom(), sel_r.GetTo());
        x_UpdateOnZoom();
    }
}


void CAlnMultiWidget::OnUpdateZoomSelection(wxUpdateUIEvent& event)
{
    if( m_AlignPane ){
        CRange<TSeqPos> sel_r = m_AlignPane->GetSelectionLimits();
        event.Enable(sel_r.NotEmpty());
    } else {
        event.Enable( false );
    }
}

void CAlnMultiWidget::x_ZoomToRange(TSeqPos from, TSeqPos to)
{
    TModelRect rcV = GetPort().GetVisibleRect();
    rcV.SetLeft(from);
    rcV.SetRight(to + 1);
    GetPort().ZoomRect(rcV);

    x_UpdateOnZoom();
}


void CAlnMultiWidget::x_MakeVisibleHorz(TSeqPos pos)
{
    x_MakeVisibleHorz(pos, pos);
}


void CAlnMultiWidget::x_MakeVisibleHorz(TSeqPos from, TSeqPos to)
{
    CGlPane& port = GetPort();
    TModelRect rcV = port.GetVisibleRect();
    TModelUnit sh_left = from - rcV.Left();
    TModelUnit sh_right = to + 1 - rcV.Right();

    TModelUnit sh = 0;
    if(sh_left < 0) { // shift left
        sh = sh_left;
    } else if(sh_right > 0) {
        sh = min(sh_right, sh_left);
    }
    if(sh != 0) {
        port.Scroll(sh, 0);
        x_UpdateOnZoom();
    }
}


void CAlnMultiWidget::OnMakeSelectedRowMaster(wxCommandEvent& event)
{
    int sel_n = m_Model->SLM_GetSelectedCount();

    if(sel_n == 1)  {
        // perform command
        CAlnMultiModel::TIndexVector vIndices;
        m_Model->SLM_GetSelectedIndices(vIndices);
        int SelIndex = vIndices[0]; // take the first selected item
        TNumrow iRow = GetRowNumByLine(SelIndex);

        x_UpdateOnSwitchMaster(iRow);
    } else { // show warning
        string msg = "Please select a ";
        msg += (sel_n == 0) ? "row!" : "single row!";
        NcbiMessageBox(msg, eDialog_Ok | eDialog_Modal, eIcon_Exclamation, "Warning");
    }
}


void CAlnMultiWidget::OnUpdateMakeSelectedRowMaster(wxUpdateUIEvent& event)
{
    event.Enable( m_Model ? m_Model->CanChangeMasterRow() : false );
}


void CAlnMultiWidget::OnMakeConsensusRowMaster(wxCommandEvent& event)
{
    int iRow = GetDataSource()->GetConsensusRow();
    if(iRow > -1)  {
        x_UpdateOnSwitchMaster(iRow);
    }
}


void CAlnMultiWidget::OnUpdateMakeConsensusRowMaster(wxUpdateUIEvent& event)
{
    bool en = m_Model ? m_Model->CanChangeMasterRow() : false;
    event.Enable(en);
}


void CAlnMultiWidget::OnUnsetMaster(wxCommandEvent& event)
{
    x_UpdateOnSwitchMaster(-1);
}


void CAlnMultiWidget::OnUpdateUnsetMaster(wxUpdateUIEvent& event)
{
    bool en = m_Model ? (m_Model->CanChangeMasterRow() && m_Model->GetMasterRow() ) : false;
    event.Enable(en);
}


void CAlnMultiWidget::OnResetSelection(wxCommandEvent& event)
{
    m_AlignPane->ResetSelection(true);
}


void CAlnMultiWidget::GetSelectedIds(TRSeqIdVector& ids) const
{
    if(GetModel()  &&  GetDataSource())  {
        CAlnMultiModel::TIndexVector vindices;
        GetModel()->SLM_GetSelectedIndices(vindices);
        ITERATE(CAlnMultiModel::TIndexVector, itInd, vindices)  {
            TNumrow row = GetRowNumByLine(*itInd);
            if(GetDataSource()->CanGetId(row))
            {
                const CSeq_id& id = GetDataSource()->GetSeqId(row);
                ids.push_back(CConstRef<CSeq_id>(&id));
            }
        }
    } else _ASSERT(false);
}


void CAlnMultiWidget::GetSelectedRows(vector<TNumrow>& rows)
{
    if(GetModel()  &&  GetDataSource())  {
        IAlignRow* master_row = m_Model->GetMasterRow();
        if (master_row &&
            (master_row->GetRowState()& IAlignRow::fItemSelected) != 0)
            rows.push_back(master_row->GetRowNum());

        CAlnMultiModel::TIndexVector vindices;
        GetModel()->SLM_GetSelectedIndices(vindices);
        ITERATE(CAlnMultiModel::TIndexVector, itInd, vindices)  {
            TNumrow row = GetRowNumByLine(*itInd);
            rows.push_back(row);
        }
    }
}


void CAlnMultiWidget::SetSelectedIds(const TRSeqIdVector& ids, CScope& scope)
{
    if(GetModel()  &&  GetDataSource())  {
        vector<TNumrow> sel_rows;

        ITERATE(TRSeqIdVector, it, ids) {
            const CSeq_id& id = **it;

            //find row corresponding to id
            int n_rows = GetDataSource()->GetNumRows();
            for( TNumrow row = 0; row < n_rows; row++ )  {
                if((GetModel()->GetRowState(row) & IAlignRow::fItemHidden) == 0) {
                    const CSeq_id&  row_id = GetDataSource()->GetSeqId(row);
                    if(CSeqUtils::Match(row_id, id, &scope)) {
                        sel_rows.push_back(row);
                        break;
                    }
                }
            }
        }

        Select(sel_rows, true);
    }
}


const   CAlnMultiWidget::TRangeColl&   CAlnMultiWidget::GetSelection(void) const
{
    return m_AlignPane->GetSelection();
}


void CAlnMultiWidget::SetSelection(const CAlnMultiWidget::TRangeColl& coll)
{
    m_AlignPane->SetSelection(coll, true);
}


void CAlnMultiWidget::OnMarkSelected(wxCommandEvent& event)
{
    const TRangeColl&  C = m_AlignPane->GetSelection();
    m_AlignPane->MarkSelectedRows(C, true);

    m_AlignPane->Refresh();
}


void CAlnMultiWidget::OnUnMarkSelected(wxCommandEvent& event)
{
    const TRangeColl&  C = m_AlignPane->GetSelection();
    m_AlignPane->MarkSelectedRows(C, false);

    m_AlignPane->Refresh();
}


void CAlnMultiWidget::OnUpdateMarkSelected(wxUpdateUIEvent& event)
{
    bool en  = false;
    if( m_Model && m_Model->SLM_GetSelectedCount() > 0 ){
        const TRangeColl&  C = m_AlignPane->GetSelection();
        en = C.size() > 0;
    }
    event.Enable(en);
}

void CAlnMultiWidget::OnHasSelected(wxUpdateUIEvent& event) 
{
    event.Enable(m_Model && m_Model->SLM_GetSelectedCount() > 0);
}

void CAlnMultiWidget::OnUnMarkAll(wxCommandEvent& event)
{
    m_AlignPane->UnMarkAll();

    m_AlignPane->Refresh();
}


void CAlnMultiWidget::OnHideSelected(wxCommandEvent& event)
{
    vector<TNumrow> rows;
    m_Model->GetSelectedRows(rows);
    m_Model->SLM_SelectAll(false); // reset selection

    SetVisible(rows, false);
}


void CAlnMultiWidget::x_GetMatchingHiddenRows(vector<TNumrow>& rows)
{
    rows.clear();
    if (!GetModel() || !GetDataSource() || GetDataSource()->IsEmpty())
        return;
    multimap<string, TNumrow> hidden;
    set<string> selected;
    //find row corresponding to id
    int n_rows = GetDataSource()->GetNumRows();
    for (TNumrow row = 0; row < n_rows; row++) {
        IAlignRow* aln_row = GetModel()->GetRow(row);
        int state = aln_row->GetRowState();
        if (state & IAlignRow::fItemHidden) {
            string acc = aln_row->GetColumnAsString(IAlignRow::eDescr);
            hidden.insert(pair<string, TNumrow>(acc, row));
        } else if (state & IAlignRow::fItemSelected) {
            selected.insert(aln_row->GetColumnAsString(IAlignRow::eDescr));
        }
    }
    ITERATE(set<string>, it, selected)
    {
        pair<multimap<string, int>::iterator, multimap<string, int>::iterator> ppp;
        ppp = hidden.equal_range(*it);
        while (ppp.first != ppp.second) {
            rows.push_back(ppp.first->second);
            ++ppp.first;
        }
    }
}


void CAlnMultiWidget::OnUnhideSelected(wxCommandEvent& event)
{
    vector<TNumrow> rows;
    x_GetMatchingHiddenRows(rows);
    if (!rows.empty())
        SetVisible(rows, true);
}

void CAlnMultiWidget::OnUpdateUnhideSelected(wxUpdateUIEvent& event)
{
    vector<TNumrow> selected; 
    GetSelectedRows(selected);
    string text = "Unhide ";
    if (selected.empty()) {
        text += "selected";
    } else if (selected.size() == 1) {
        text += "All '" + GetModel()->GetRow(selected.front())->GetColumnAsString(IAlignRow::eDescr) + "' Rows";
    } else {
        text += "All Rows Matching " + NStr::NumericToString(selected.size()) + " Selected Rows";
    }
    event.SetText(ToWxString(text));
    vector<TNumrow> rows;
    x_GetMatchingHiddenRows(rows);
    event.Enable(!rows.empty());
}


void CAlnMultiWidget::OnShowOnlySelected(wxCommandEvent& event)
{
    vector<TNumrow> rows;
    m_Model->GetSelectedRows(rows);

    SetVisible(rows, true, true); // hide those that are not selected
}


void CAlnMultiWidget::OnShowAll(wxCommandEvent& event)
{
    m_Model->ShowAll();

    x_AdjustVisible(false);
    x_UpdateScrollbars();
    x_RedrawControls();
}


void CAlnMultiWidget::OnMoveSelectedUp(wxCommandEvent& event)
{
    if(m_Model)    {
        vector<TNumrow> rows;
        m_Model->GetSelectedRows(rows);

        if(rows.size()) {
            m_Model->SetRowOrder(rows);
            x_RedrawControls();
        }
    }
}


void CAlnMultiWidget::x_UpdateStyleColumns()
{
    CWidgetDisplayStyle& style = *m_Model->GetDisplayStyle();
    CWidgetDisplayStyle::TColumns& st_cols = style.m_Columns;

    CAlnMultiRenderer& renderer = m_AlignPane->GetRenderer();
    int cols_n = renderer.GetColumnsCount();
    st_cols.resize(cols_n);

    for( int i = 0;  i < cols_n;  i++ )  {
        const CAlnMultiRenderer::SColumn& col = renderer.GetColumn(i);

        st_cols[i].m_Name = col.m_Name;
        st_cols[i].m_Width = col.m_Width;
        st_cols[i].m_Visible = col.m_Visible;
    }
}


void CAlnMultiWidget::OnSettings(wxCommandEvent& event)
{
    CWidgetDisplayStyle& style = *m_Model->GetDisplayStyle();
    CWidgetDisplayStyle::TColumns& st_cols = style.m_Columns;

    x_UpdateStyleColumns();

    // update style columns
    CAlnMultiRenderer& renderer = m_AlignPane->GetRenderer();
    int cols_n = renderer.GetColumnsCount();
    st_cols.resize(cols_n);
    for( int i = 0;  i < cols_n;  i++ )  {
        const CAlnMultiRenderer::SColumn& col = renderer.GetColumn(i);
        st_cols[i].m_Name = col.m_Name;
        st_cols[i].m_Width = col.m_Width;
        st_cols[i].m_Visible = col.m_Visible;
    }

    // show the dialog
    CAlnPropertiesDlg dlg(this);
    dlg.SetRegistryPath(m_RegPath + ".Properties Dialog");
    dlg.SetParams(style);

    if(dlg.ShowModal() == wxID_OK) {
        x_UpdateOnStylesChanged();
        SaveSettings();
    }
}

void CAlnMultiWidget::OnUpdateSettings(wxUpdateUIEvent& event)
{
    event.Enable( true );
}


void CAlnMultiWidget::UpdateSortOrder()
{
    m_Model->UpdateSortOrder();
    m_AlignPane->UpdateHeaderSortState();

    x_RedrawControls();
}


void CAlnMultiWidget::UpdateOnScoringChanged()
{
    m_AlignPane->Update();
    x_RedrawControls();
}


void CAlnMultiWidget::GetMarks(TPSeqLocList& ls_locs) const
{
    const CAlnMultiPane::TRowToMarkMap& map = m_AlignPane->GetMarks();

    ITERATE(CAlnMultiPane::TRowToMarkMap, it_m, map)  { //for each marked row
        TNumrow row = it_m->first;
        if(GetDataSource()->CanGetId(row))  {
            const CSeq_id& seq_id = GetDataSource()->GetSeqId(row);
            // create CSeq_loc
            ls_locs.push_back(new CSeq_loc());
            CSeq_loc& seq_loc = *ls_locs.back();

            // put intervals
            CSeq_loc::TPacked_int& p_int = seq_loc.SetPacked_int();

            const TRangeColl& mark = it_m->second;
            ITERATE(TRangeColl, it_r, mark) { // for each range in mark
                p_int.AddInterval(seq_id, it_r->GetFrom(), it_r->GetTo());
            }
        }
    }
}

void CAlnMultiWidget::UpdateView(void)
{
    x_UpdateOnStylesChanged();
    m_AlignPane->Update();
}


void CAlnMultiWidget::x_CreatePane(void)
{
    m_AlignPane = new CAlnMultiPane(this, wxID_ANY);

    // Pane is created, now we can model (which includes fonts which require
    // an active opengl context)
    if(m_Model == NULL)   {
        m_Model = x_CreateModel();
    }
    _ASSERT(m_Model);

    IAlnMultiPaneParent* widget = static_cast<IAlnMultiPaneParent*>(this);
    m_AlignPane->SetWidget(widget);
    m_AlignPane->SetContext(m_Model);

    m_Model->SLM_AddSLView(static_cast<TNumrowSelListView*>(m_AlignPane));
}

/*void CAlnMultiWidget::x_CreateControls()
{
    CFlCurrentGuard GUARD;
    _ASSERT(! m_AlignPane  && ! m_ScrollX  && ! m_ScrollY);

    const CWidgetSettings& sys_st = CWidgetSettings::GetSystemSettings();
    const int ScrollbarSize = sys_st.GetMetric(eSysMetric_ScrollbarSize);

    int client_w = w() - ScrollbarSize;
    int client_h = h() - ScrollbarSize;

    m_AlignPane = new CAlnMultiPane(x(), y(), client_w, client_h);
    m_AlignPane->end();
    add(m_AlignPane);

    IAlnMultiPaneParent* pWidget = static_cast<IAlnMultiPaneParent*>(this);
    m_AlignPane->SetWidget(pWidget);
    m_AlignPane->SetContext(m_Model);

    m_Model->SLM_AddSLView(static_cast<TNumrowSelListView*>(m_AlignPane));

    // scrollbars
    m_ScrollX = new Fl_Scrollbar(x(), y() + client_h, client_w, ScrollbarSize);
    m_ScrollX->type(FL_HORIZONTAL);
    m_ScrollX->callback(CAlnMultiWidget::x_OnScrollX, this );
    add(m_ScrollX);

    m_ScrollY = new Fl_Scrollbar(x() + client_w, y(), ScrollbarSize, client_h);
    m_ScrollY->type(FL_VERTICAL);
    m_ScrollY->callback(CAlnMultiWidget::x_OnScrollY, this );
    add(m_ScrollY);

    resizable(m_AlignPane);
}

void CAlnMultiWidget::x_SetScrollbars()
{
    TModelRect rcAll = x_GetAlignPort().GetModelLimitsRect();
    TModelRect rcVisible = x_GetAlignPort().GetVisibleRect();

    if (x_GetAlignPort().NeedsScrollX()) {
        m_ScrollX->value((int) rcVisible.Left(), (int) rcVisible.Width(),
                          (int) rcAll.Left(), (int)rcAll.Width());
    } else {
        m_ScrollX->value(0, 0, 0, 0);
    }

    if (x_GetAlignPort().NeedsScrollY()) {
        m_ScrollY->value((int) rcVisible.Top(), (int) -rcVisible.Height(),
                          (int) rcAll.Top(), (int) -rcAll.Height());
    } else {
        m_ScrollY->value(0, 0, 0, 0);
    }
}*/


void CAlnMultiWidget::x_OnResize()
{
    TVPRect rcVP;
    rcVP.SetSize(m_AlignPane->GetAlignVPWidth(), m_AlignPane->GetAlignVPHeight());
    GetPort().SetViewport(rcVP);

    x_AdjustVisible(false);
    x_UpdateScrollbars();

    // do not redraw, FLTK will do it for us -
}


void CAlnMultiWidget::x_RedrawControls()
{
    m_AlignPane->Refresh();
}


////////////////////////////////////////////////////////////////////////////////
/// Update handlers
void CAlnMultiWidget::x_UpdateOnDataChanged(bool b_reset)
{
    IAlnMultiDataSource* ds = GetDataSource();
    if(ds  &&  ! ds->IsEmpty())   {
        x_AdjustVisible(b_reset);

        if(m_Model)    {
            TNumrow n_rows = ds->GetNumRows();
            for( TNumrow i = 0;  i < n_rows; i++ )  {
                IAlignRow* p_row = m_Model->GetRow(i);
                p_row->SetHost(m_AlignPane);
            }
        }
    }

    m_AlignPane->ResetSelection(false);
    m_AlignPane->UnMarkAll();

    x_UpdateScrollbars();

    CWidgetDisplayStyle& style = *m_Model->GetDisplayStyle();
    if (!style.m_ShowConsensus) {
        int iRow = GetDataSource()->GetConsensusRow();
        if (iRow > -1)  {
            vector<TNumrow> rows;
            rows.push_back(iRow);
            SetVisible(rows, false);
        }
    }    

    m_AlignPane->UpdateOnDataChanged();
    x_RedrawControls();
}


void CAlnMultiWidget::x_AdjustVisible(bool b_reset)
{
    TModelRect rcV;
    CGlPane& port = GetPort();

    if(b_reset) {
        rcV = port.GetModelLimitsRect();
        rcV.SetTop(0);
    } else {
        rcV = port.GetVisibleRect();
    }
    rcV.SetBottom(rcV.Top() + (m_AlignPane->GetAlignVPHeight() - 1));
    port.SetVisibleRect(rcV);
    port.AdjustToLimits();
}


void CAlnMultiWidget::x_UpdateOnRowHChanged()
{
    //_ASSERT(GetDataSource());

    // updating model
    m_Model->UpdateOnRowHChanged();

    // updating representation
    x_AdjustVisible(false);
    x_UpdateScrollbars();
    x_RedrawControls();
}


void CAlnMultiWidget::x_UpdateOnSwitchMaster(TNumrow NewMasterRow)
{
    CGlPane& port = GetPort();
    TModelRect rcV = port.GetVisibleRect();
    TSignedSeqPos SeqStart = 0;
    TSignedSeqPos SeqStop = 0;

    if(GetDataSource()->GetAnchor() == NewMasterRow)
        return;

    // choose row that will be used for coordinate translations
    TNumrow SelRow = (NewMasterRow != -1)  ? NewMasterRow : GetDataSource()->GetAnchor();
    TRangeColl SeqSelection; //selection in coordinates of SelRow

    if(SelRow != -1)    {
        TSeqPos AlnStart((TSeqPos) rcV.Left());
        TSeqPos AlnStop((TSeqPos) rcV.Right());
        SeqStart = GetDataSource()->GetSeqPosFromAlnPos(SelRow, AlnStart,
                                                     IAlnExplorer::eRight);
        SeqStop = GetDataSource()->GetSeqPosFromAlnPos(SelRow, AlnStop,
                                                     IAlnExplorer::eRight);

        // convert selection to SelRow coordinates
        const TRangeColl& Selection = m_AlignPane->GetSelection();
        ITERATE(TRangeColl, it, Selection)  {
            TSeqPos Start =
                GetDataSource()->GetSeqPosFromAlnPos(SelRow, it->GetFrom(),
                                                   IAlnExplorer::eRight);
            TSeqPos Stop =
                GetDataSource()->GetSeqPosFromAlnPos(SelRow, it->GetTo(),
                                                   IAlnExplorer::eLeft);
            SeqSelection.CombineWith(TSeqRange(Start, Stop));
        }
    }

    m_Model->SetMasterRow(NewMasterRow);

    if(SelRow != -1)    {
        // calculate new visible rect based on saved positions
        TSignedSeqPos AlnStart = GetDataSource()->GetAlnPosFromSeqPos(SelRow, SeqStart);
        TSignedSeqPos AlnStop = GetDataSource()->GetAlnPosFromSeqPos(SelRow, SeqStop);

        TModelRect rcNewV(AlnStart, rcV.Bottom(), AlnStop, rcV.Top());
        port.SetVisibleRect(rcNewV);

        // convert Selection back to Alignment space
        TRangeColl AlnSelection;
        const TRangeColl& C = SeqSelection;
        ITERATE(TRangeColl, it, C)  {
            TSeqPos Start = GetDataSource()->GetAlnPosFromSeqPos(SelRow, it->GetFrom());
            TSeqPos Stop = GetDataSource()->GetAlnPosFromSeqPos(SelRow, it->GetTo());
            AlnSelection.CombineWith(TSeqRange(Start, Stop));
        }
        m_AlignPane->SetSelection(AlnSelection, false);
    }

    m_AlignPane->Update();

    x_AdjustVisible(false);
    x_UpdateScrollbars();

    x_RedrawControls();

    // TODO - use special function of CAlnMultiPane
    CAlnMultiPane::CEvent evt(CAlnMultiPane::eLinearSelChanged);
    Send(&evt, CEventHandler::eDispatch_Default, CEventHandler::ePool_Parent);
}


void CAlnMultiWidget::x_UpdateOnZoom()
{
    x_UpdateScrollbars();
    m_AlignPane->Update();
    x_RedrawControls();
}


void CAlnMultiWidget::x_UpdateOnStylesChanged(void)
{
    CWidgetDisplayStyle& style = *m_Model->GetDisplayStyle();

    m_AlignPane->GetRenderer().SetColumnsByStyle(style);

    m_Model->UpdateOnStyleChanged();

    CRgbaColor color = style.GetColor(CWidgetDisplayStyle::eBack);
    m_AlignPane->SetBackColor(color);

    if (GetDataSource() != NULL) {
        m_DataSource->SetCreateConsensus(style.m_ShowConsensus);

        int iRow = GetDataSource()->GetConsensusRow();
        if (iRow > -1)  {
            if (!style.m_ShowConsensus && 
                m_Model->GetMasterRow() != NULL &&
                m_Model->GetMasterRow()->GetRowNum() == iRow) {
                    x_UpdateOnSwitchMaster(-1);
            }

            vector<TNumrow> rows;
            rows.push_back(iRow);

            SetVisible(rows, style.m_ShowConsensus);
        }
        else if (style.m_ShowConsensus) {
            // User has asked to see consensus row but the row has not
            // been created, so create it and then update data since dense-seg
            // is rebuilt with a new row when consensus is created
            if (m_DataSource->CanCreateConsensus())
                m_Model->CreateConsensus();

            m_Model->UpdateOnDataChanged();
            x_UpdateOnDataChanged();       
        }
    }

    x_UpdateOnRowHChanged();
}


void CAlnMultiWidget::OnShowMethodDlg(wxCommandEvent& event)
{
    _ASSERT(m_DataSource);

    string name = x_GetCurrentMethodName();

    IAlnExplorer::EAlignType sc_type = m_DataSource->GetAlignType();
    _ASSERT(sc_type != IAlnExplorer::fInvalid);

    CScoringMethodsDlg dlg;
    dlg.Setup(name, sc_type);
    dlg.Create(this);

    if(dlg.ShowModal() == wxID_OK)  {
        name = ToStdString(dlg.GetSelectedMethod());
        x_SetScoringMethod(name, true);
    }
}


void CAlnMultiWidget::OnUpdateShowMethodDlg(wxUpdateUIEvent& event)
{
    bool en = false;
    if(GetDataSource()) {
        IAlnExplorer::EAlignType type = m_DataSource->GetAlignType();
        en = (type != IAlnExplorer::fInvalid);
    }
    event.Enable(en);
}


void CAlnMultiWidget::OnMethodProperties(wxCommandEvent& event)
{
    IUIToolWithGUI* method = dynamic_cast<IUIToolWithGUI*>(m_Model->GetCurrentMethod());
    if(method)  {
        CPropertiesPanelDlg dlg(this, method);
        dlg.SetTitle(wxT("Coloring Method Properties"));

        if(dlg.ShowModal() == wxID_OK)  {
            m_Model->UpdateOnScoringChanged(); // update scores

            UpdateOnScoringChanged(); // update UI
        }
    }
}


void CAlnMultiWidget::OnUpdateMethodProperties(wxUpdateUIEvent& event)
{
    bool bEn = false;
    if(m_Model) {
        IUIToolWithGUI* method = dynamic_cast<IUIToolWithGUI*>(m_Model->GetCurrentMethod());
        bEn = method  &&  method->HasPropertiesPanel();
    }
    event.Enable(bEn);
}


void CAlnMultiWidget::OnDisableScoring(wxCommandEvent& event)
{
    x_SetScoringMethod("", true);
}


void CAlnMultiWidget::OnUpdateDisableScoring(wxUpdateUIEvent& event)
{
    bool en = (m_Model && m_Model->GetCurrentMethod() != NULL);
    event.Enable(en);
    event.Check( ! en);
}


string  CAlnMultiWidget::x_GetCurrentMethodName() const
{
    _ASSERT(m_Model);

    IScoringMethod* method = m_Model->GetCurrentMethod();
    return method ? method->GetName() : "";
}


static const int kDefMRUSize = 5;


// changes current scoring method to "method_name"
void CAlnMultiWidget::x_SetScoringMethod(const string& method_name, bool save)
{
    _ASSERT(m_Model);

    string current_name = x_GetCurrentMethodName();

    if(current_name != method_name)    { // we do need to change it
        if(method_name == "")  {
            m_Model->ResetCurrentMethod();

            UpdateOnScoringChanged(); // update display
        } else {
            CUIToolRegistry* reg = CUIToolRegistry::GetInstance();
            bool ok = m_Model->SetCurrentMethod(method_name);

            if( ! ok)   {
                // method not registered yet - create a new instance
                CIRef<IUITool> tool = reg->CreateToolInstance(method_name);
                IScoringMethod* method =
                    dynamic_cast<IScoringMethod*>(tool.GetPointer());
                if(method)  {
                    // add the method to the model and select it
                    m_Model->AddScoringMethod(method);
                    ok = m_Model->SetCurrentMethod(method_name);
                } else {
                    string msg = string("Method \"") + method_name;
                    msg += "\" is invalid.";
                    NcbiMessageBox(msg, eDialog_Ok, eIcon_Exclamation, "Error");
                }
            }

            if(ok)  {
                string mru_list = x_GetScoringMRUListName();
                if( ! reg->MRUListExists(mru_list)) {
                    reg->CreateMRUList(mru_list, kDefMRUSize);
                }
                reg->AddToolToMRU(mru_list, method_name);

                UpdateOnScoringChanged(); // update display
            }
        }
        if(save)    {
            m_Model->SetDefaultMethod(method_name);
            SaveSettings();
        }
    }
}


void CAlnMultiWidget::OnScoringMethod(wxCommandEvent& event)
{
    int cmd = event.GetId();
    const string& method_name = m_CmdToName[cmd];

    x_SetScoringMethod(method_name, true);
    m_CmdToName.clear();
}


void CAlnMultiWidget::OnUpdateScoringMethod(wxUpdateUIEvent& event)
{
    TCmdID cmd = event.GetId();
    const string& method_name = m_CmdToName[cmd];

    string current_name = x_GetCurrentMethodName();

    event.Check(current_name == method_name);
}


void CAlnMultiWidget::OnSetAsDefaultMethod(wxCommandEvent& event)
{
    string current_name = x_GetCurrentMethodName();

    string* def_name = x_GetDefMethodName();

    _ASSERT(def_name);

    *def_name = current_name;
    SaveSettings();
}


// returns a reference to the default scoring method for this type of alignment
string* CAlnMultiWidget::x_GetDefMethodName()
{
    _ASSERT(m_DataSource);

    IAlnExplorer::EAlignType type = m_DataSource->GetAlignType();
    CWidgetDisplayStyle* style = m_Model->GetDisplayStyle();

    switch(type)    {
    case IAlnExplorer::fDNA:
        return &(style->m_DefDNAMethod);
    case IAlnExplorer::fProtein:
        return &style->m_DefProteinMethod;
    default:
        _ASSERT(false);
        return NULL;
    }
 }


string CAlnMultiWidget::x_GetScoringMRUListName()
{
    string tag =
        IScoringMethod::GetAlignmentTagByType(m_DataSource->GetAlignType());
    string mru_list = "Multiple Alignment Widget Scoring Methods - " + tag;
    return mru_list;
}


////////////////////////////////////////////////////////////////////////////////
/// Scroll callbacks
/*
void CAlnMultiWidget::x_OnScrollX(Fl_Widget* pW, void* pData)
{
    CAlnMultiWidget* pCont = reinterpret_cast<CAlnMultiWidget*>(pData);

    int V = pCont->m_ScrollX->value();
    double dX = V - pCont->x_GetAlignPort().GetVisibleRect().Left();
    pCont->x_GetAlignPort().Scroll(dX, 0);

    pCont->x_RedrawControls();
}

void CAlnMultiWidget::x_OnScrollY(Fl_Widget* pW, void* pData)
{
    CAlnMultiWidget* pCont = reinterpret_cast<CAlnMultiWidget*>(pData);

    int V = pCont->m_ScrollY->value();
    double dY = V - pCont->x_GetAlignPort().GetVisibleRect().Top();
    pCont->x_GetAlignPort().Scroll(0, dY);

    pCont->x_RedrawControls();
}*/


void CAlnMultiWidget::Print(const string& filename)
{
/*    CPrintOptions opts;
    opts.SetOutputFormat(CPrintOptions::GetFormatFromName("pdf"));
    opts.SetFilename(filename);
    opts.SetHeader("header --- SeqViewer --- header");
    opts.SetFooter("footer --- SeqViewer --- footer");

    m_AlignPane->Print(opts);*/
}

static
WX_DEFINE_MENU(kPopupMenu)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Zoom")
    WX_MENU_ITEM(eCmdZoomInMouse)
    WX_MENU_ITEM(eCmdZoomOutMouse)
    WX_MENU_ITEM(eCmdZoomAll)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdZoomSeq)
    WX_MENU_ITEM(eCmdZoomSel)
    WX_MENU_ITEM(eCmdResetSelection)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdMoveSelectedUp)
    WX_SUBMENU("Hide / Show")
        WX_MENU_ITEM(eCmdHideSelected)
        WX_MENU_ITEM(eCmdUnhideSelected)
        WX_MENU_ITEM(eCmdShowOnlySelected)
        WX_MENU_ITEM(eCmdShowAll)
    WX_END_SUBMENU()
    WX_SUBMENU("Master")
        WX_MENU_ITEM(eCmdSetSelMaster)
        WX_MENU_ITEM(eCmdUnsetMaster)
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR()
    WX_SUBMENU("Coloring")
        WX_MENU_ITEM(eCmdAlnShowMethodsDlg)
        WX_MENU_ITEM(eCmdMethodProperties)
        WX_MENU_SEPARATOR()
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR_L("Settings")
    WX_MENU_ITEM(eCmdSettings)
WX_END_MENU()


wxMenu* CAlnMultiWidget::x_CreatePopupMenu()
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* root = cmd_reg.CreateMenu(kPopupMenu);

    if(GetDataSource()) {
        IAlnExplorer::EAlignType type = GetDataSource()->GetAlignType();

        if(type != IAlnExplorer::fInvalid)   {
            // alignment has been created

            /// add Scoring Methods menu
            wxMenu* color_menu = FindSubItem(*root, wxT("Coloring"))->GetMenu();
            x_CreateScoringMethodsMenu(*color_menu);

            // obtain a list of MRU names from the Method Registry
            m_CmdToName.clear();

            string current_method = x_GetCurrentMethodName();
            string mru_list = x_GetScoringMRUListName();

            vector<string> MRU_names;
            CUIToolRegistry* reg = CUIToolRegistry::GetInstance();
            if(reg->MRUListExists(mru_list)) {
                reg->GetMRUToolNames(mru_list, MRU_names);
            }

            // if current method is not in the MRU - add it
            if(! current_method.empty())   {
                vector<string>::const_iterator it =
                    std::find(MRU_names.begin(), MRU_names.end(), current_method);
                if(it == MRU_names.end())   {
                    MRU_names.push_back(current_method);
                }
            }

            if( ! MRU_names.empty())    {
                // keep it sorted, so that user can easily find a method
                std::sort(MRU_names.begin(), MRU_names.end());

                wxMenuItem* col_item = FindSubItem(*root, wxT("Coloring"));
                wxMenu* submenu = col_item->GetSubMenu();

                int index = 0;
                ITERATE(vector<string>, it_name, MRU_names)  {
                    TCmdID cmd = eCmdScoringMethodXXXX + index;
                    const string& method_name = *it_name;
                    submenu->AppendCheckItem(cmd, ToWxString(method_name));

                    m_CmdToName[cmd] = method_name;
                    index++;
                }

                submenu->AppendSeparator();
                cmd_reg.AppendMenuItem(*submenu, eCmdDisableScoring);

                string* def_name = x_GetDefMethodName();
                _ASSERT(def_name);

                if(*def_name != current_method) {
                    cmd_reg.AppendMenuItem(*submenu, eCmdSetAsDefaultMethod);
                }
            }
        }
    }
    return root;
}


void CAlnMultiWidget::x_CreateScoringMethodsMenu(wxMenu& color_menu)
{
    _ASSERT(m_Model);

    IScoringMethod* curr_method = m_Model->GetCurrentMethod();
    IUIToolWithGUI* gui_method = dynamic_cast<IUIToolWithGUI*>(curr_method);
    if(gui_method)  {
        // if method provides a menu - integrate this menu
        const CMenuItem* method_item = gui_method->GetMenu();

        if(method_item) { // menu is not empty
            wxMenu* item_menu = CreateMenu(method_item);
            color_menu.AppendSubMenu(item_menu,
                                     ToWxString(method_item->GetLabel()));
        }
    }
}

bool CAlnMultiWidget::HasSelectedObjects() const
{
    _ASSERT(m_AlignPane);
    return m_AlignPane ? m_AlignPane->HasSelectedObjects() : false;
}

void  CAlnMultiWidget::GetObjectSelection(TConstObjects &objs) const
{
    _ASSERT(m_AlignPane);
    if (m_AlignPane)
        m_AlignPane->GetObjectSelection(objs);
}

bool CAlnMultiWidget::SaveVectorImage(CPrintOptions::EOutputFormat format, string const &path, string const &file_name, bool show_header, bool simplified)
{
    int original_shown = m_Model->GetShownElements();
    int shown = original_shown;
    if (show_header) {
        shown |= IAlnMultiRendererContext::fShownElement_Header;
    }
    else {
        shown &= ~IAlnMultiRendererContext::fShownElement_Header;
    }
    m_Model->SetShownElements(shown);

    CAlnMultiRenderer &renderer = m_AlignPane->GetRenderer();
    TVPRect original_rect = renderer.GetRect();
    CGlPane &port = GetPort();
    const TModelRect model_limits = port.GetModelLimitsRect();
    const TVPRect vp_rect = port.GetViewport();
    const TModelRect visible_rect = port.GetVisibleRect();

    CGlTextureFont font = m_Model->GetDisplayStyle()->m_TextFont;
    font.SetFontSize(12);
    TModelUnit title_height = font.TextHeight() - font.GetFontDescender();
    title_height += 12.f;

    size_t height = (size_t)-model_limits.Height();
    size_t width = original_rect.Width();
    int header_height = 0;
    if (shown & IAlnMultiRendererContext::fShownElement_Header)
        header_height += renderer.GetHeaderAreaHeight();
    if (shown & IAlnMultiRendererContext::fShownElement_Ruler)
        header_height += renderer.GetRulerAreaHeight();
    if (shown & IAlnMultiRendererContext::fShownElement_MasterRow)
        header_height += renderer.GetMasterAreaHeight();        
    height += header_height;

    // adjust viewport and image size
    renderer.Resize(TVPRect(0, 0, width - 1, height - 1));
    port.SetViewport(TVPRect(0, 0, width - 1, height - 1));

    // adjust visible range - it gets ruined after viewport adjustment
    TModelRect rc_vis = port.GetVisibleRect();
    size_t y = 0;;
    rc_vis.SetVert(y + (height - 1), y);
    port.SetVisibleRect(rc_vis);

    unique_ptr<IImageGrabber> grabber{ CImageGrabberFactory::CreateImageGrabber(format, *m_AlignPane) };

    if (title_height > 0.0) {
        grabber->SetTitleHeight(title_height);
    }
    grabber->SetOutputInfo(path, file_name, CPrintOptions::GetFileExtension(format));
    grabber->DisableGouraudShading(simplified);
    IImageGrabber::eCaptureResult res = grabber->GrabImages(0);
    

    m_Model->SetShownElements(original_shown);
    renderer.Resize(original_rect);
    port.SetViewport(vp_rect);
    port.SetVisibleRect(visible_rect);

    if (res == IImageGrabber::eOpenGLError) {
        wxMessageBox(wxT("Graphics system does not support in-memory image save feature"), wxT("Error Saving"), wxOK);
    }
    else if (res == IImageGrabber::eFileError) {
        wxMessageBox(wxT("File error occured during save. Make sure file and directory names are valid."), wxT("Error Saving"), wxOK);
    }

    return (res == IImageGrabber::eSuccess);
}

void CAlnMultiWidget::x_OnSaveVectorImage(CPrintOptions::EOutputFormat format)
{
    CwxExportAlnmultiImgDlg dlg(format, this, this);
    dlg.ShowModal();
}


END_NCBI_SCOPE
