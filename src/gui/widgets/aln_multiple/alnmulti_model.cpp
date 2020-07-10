/*  $Id: alnmulti_model.cpp 43940 2019-09-23 15:40:49Z shkeda $
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

#include <gui/widgets/aln_multiple/alnmulti_model.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <gui/widgets/aln_multiple/align_row.hpp>
#include <gui/widgets/aln_multiple/trace_graph.hpp>
#include <gui/widgets/aln_multiple/feature_graph.hpp>

#include <corelib/ncbithr.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kColorGradNumber = 64; /// number of gradient colors in alignment

 ////////////////////////////////////////////////////////////////////////////////
/// class CAlnMultiModel

CAlnMultiModel::CAlnMultiModel()
:   m_Style(NULL),
    m_StyleCatalog(NULL),
    m_DataSource(NULL),
    m_Listener(NULL),
    m_MasterRow(NULL), // TODO: should we initialize CIRef?
    m_AutoSort(false),
    m_SortColumn(IAlignRow::eInvalid),
    m_SortState(IAlnExplorer::eNotSupported),
    m_EnableScoring(true),
    m_ScoreCache(new CScoreCache()),
    m_isDataReadSync(false)
{
    m_ScoreCache->SetListener(this);
    m_ScoreCache->SetGradNumber(kColorGradNumber);
    m_ConsensusRow.Reset(new CConsensusRow(NULL));

    // for debugging only
    /*CSimpleScoringMethod* method = new CSimpleScoringMethod();
    method->CreateColorTable(32);
    AddScoringMethod(method);
    SetCurrentMethod(method->GetName());*/
}


void CAlnMultiModel::Init()
{
    x_CreateDisplayStyle();

    // setup Port
    m_Port.SetAdjustmentPolicy(CGlPane::fAdjustAll, CGlPane::fShiftToLimits);
    m_Port.SetOriginType(CGlPane::eOriginLeft, CGlPane::eOriginTop);
    m_Port.EnableZoom(true, false);

    x_UpdateMinScale();

    SetPortLimits();
}


void CAlnMultiModel::x_UpdateMinScale()
{
    double max_sym_w = 3 * m_Style->m_SeqFont.GetMetric(CGlTextureFont::eMetric_MaxCharWidth);
    double min_scale =  1 / (max_sym_w * 0.75);
    min_scale = max(min_scale, 0.01);
    m_Port.SetMinScaleX(min_scale);
}


CAlnMultiModel::~CAlnMultiModel()
{
    // we do not want to receive any updates
    m_ScoreCache->SetListener(NULL);

    // delete methods
    m_ScoreCache->SetScoringMethod(NULL);
    m_Methods.clear();

    ClearRows();
    delete m_Style;
}


// factory method for creation rows
IAlignRow*  CAlnMultiModel::x_CreateRow(TNumrow row)
{
    _ASSERT(m_DataSource  &&  m_StyleCatalog);

    const IAlignRowHandle* handle = m_DataSource->GetRowHandle(row);

    _ASSERT(handle);

    // create CAlnVecRow
    CAlnVecRow* p_row = new CAlnVecRow(*handle);

    // create Styles
    const CRowDisplayStyle* style = m_StyleCatalog->GetStyleForRow(row);
    p_row->SetDisplayStyle(style);

    p_row->SetScoreCache(m_ScoreCache);
    p_row->SetConsensus(m_ConsensusRow.GetPointer());

    return p_row;
}


////////////////////////////////////////////////////////////////////////////////
// IAlnMultiPaneParent implementation
void CAlnMultiModel::SetStyleCatalog(const CRowStyleCatalog* catalog)
{
    m_StyleCatalog = catalog;
}


CWidgetDisplayStyle*  CAlnMultiModel::GetDisplayStyle()
{
    return m_Style;
}


void CAlnMultiModel::x_CreateDisplayStyle()
{
    m_Style = new CWidgetDisplayStyle();
}


int  CAlnMultiModel::GetRowState(TNumrow row)    const
{
    return x_GetRecordByRow(row).m_Row->GetRowState();
}


bool  CAlnMultiModel::SRowRecord::PRecLineLess(const SRowRecord* rec1,
                                                 const SRowRecord* rec2)
{
    return rec1->m_Line < rec2->m_Line;
}


/// changes visibility state for given vector of rows to "b_visible"
void CAlnMultiModel::SetVisible(vector<TNumrow> rows, bool b_visible,
                                bool b_invert_others)
{
    int consensus_idx = m_DataSource->GetConsensusRow();

    // If we are set to not show consensus at all, hide it 
    // in all cases
    if (consensus_idx >= 0 && !m_Style->m_ShowConsensus) {
        vector<TNumrow>::iterator iter = std::find(rows.begin(), rows.end(), TNumrow(consensus_idx));
               
        if (!b_invert_others) {
            if (b_visible) {
                if (iter != rows.end())
                    rows.erase(iter);
            }
        }
        else {
            if (!b_visible) {
                if (iter == rows.end())
                    rows.push_back(consensus_idx);
            }
        }
    }

    size_t n_rows = rows.size();
    if(n_rows || b_invert_others) {
        vector<TNumrow> v_items;
        TNumrow master_row = m_DataSource->GetAnchor();

        if(b_invert_others) { // all rows in alignment are affected
            // by default set state for all rows to "! b_visible"
            vector<bool>    new_visible(GetNumRows(), ! b_visible);

            ITERATE(vector<TNumrow>, it, rows)  {
                new_visible[*it] = b_visible;
            }

            vector<SRowRecord*> vis_recs;
            vis_recs.reserve(b_visible ? rows.size()
                                                : new_visible.size() - rows.size());

            for( TNumrow row = 0; row < (TNumrow) new_visible.size(); row++ )    { // for all rows
                if(row != master_row  &&  new_visible[row])  {
                    SRowRecord& rec = x_GetRecordByRow(row);
                    vis_recs.push_back(&rec);
                }
            }
            // sort records by line number
            sort(vis_recs.begin(), vis_recs.end(), SRowRecord::PRecLineLess);

            // copy row numbers to v_items
            v_items.reserve(vis_recs.size());
            ITERATE(vector<SRowRecord*>, it, vis_recs)  {
                IAlignRow* p_row = (*it)->m_Row;
                v_items.push_back(p_row->GetRowNum());
            }

            TNumrowModelImpl::SetItems(v_items, false, true);
        }
        else { // only "rows" will be affected
            v_items.reserve(n_rows);
            vector<TLine>   v_indices;
            v_indices.reserve(n_rows);

            ITERATE(vector<TNumrow>, it, rows)  {
                TNumrow row = *it;
                SRowRecord& rec = x_GetRecordByRow(row);
                bool b_old_vis = rec.IsVisible();

                if(b_old_vis != b_visible  &&  row != master_row)  {
                    // state needs to be changed
                    TLine line = rec.m_Line;
                    // clip line, it can be invalid
                    line = min(line, (TLine) m_vVisibleRows.size());
                    v_indices.push_back(line);
                    if(b_visible) {
                        v_items.push_back(row);
                        rec.m_Row->SetRowState(IAlignRow::fItemHidden, false);
                    }
                }
            }
            if(b_visible) {
                TNumrowModelImpl::InsertItems(v_items, v_indices, false);
            } else  {
                TNumrowModelImpl::DeleteItems(v_indices, false);
            }
        }
        x_DoUpdateRowPositions();

        if(x_UsingAutoSort()) {
            x_DoUpdateRowsOrder(); // applying sorting
        }

        SetPortLimits();
        x_UpdateOnVisibilityChanged();

    }
}

CGlPane& CAlnMultiModel::GetAlignPort()
{
    return m_Port;
}

const CGlPane& CAlnMultiModel::GetAlignPort() const
{
    return m_Port;
}

CAlnMultiModel::TNumrow CAlnMultiModel::GetLinesCount() const
{
    return (TNumrow) m_vVisibleRows.size();
}

int CAlnMultiModel::GetFocusedItemIndex() const
{
    return SLM_GetFocusedItemIndex();
}

bool CAlnMultiModel::IsItemSelected(int index) const
{
    return SLM_IsItemSelected(index);
}

IAlignRow*  CAlnMultiModel::GetRowByLine(int i_line)
{
    return x_GetRowByLine(i_line);
}

int   CAlnMultiModel::GetLineByRowNum(TNumrow row) const
{
    const CAlnMultiModel::SRowRecord& rec = x_GetRecordByRow(row);
    return rec.IsVisible() ? rec.m_Line : -1;
}

IAlignRow* CAlnMultiModel::GetMasterRow()
{
    return m_MasterRow.GetPointerOrNull();
}


IAlignRow* CAlnMultiModel::GetConsensusRow()
{
    return m_DataConsensusRow.GetPointerOrNull();
}


int CAlnMultiModel::GetLineByModelY(int Y) const
{
    const TModelRect& rcL = m_Port.GetModelLimitsRect();
    if(Y < rcL.Top()  || Y > rcL.Bottom())
        return -1;
    else {
        vector<int>::const_iterator it = lower_bound(m_vVisibleRowPos.begin(), m_vVisibleRowPos.end(), Y);
        return int(it - m_vVisibleRowPos.begin()) - 1;
    }
}

int CAlnMultiModel::GetLinePosY(int index) const
{
    if (m_vVisibleRowPos.empty())
        return 0;
    return m_vVisibleRowPos[index];
}

int CAlnMultiModel::GetLineHeight(TLine line) const
{
    const IAlignRow* row = x_GetRowByLine(line);
    if (!row)
        return 0;
    return row->GetHeightPixels();
}


bool CAlnMultiModel::IsRendererFocused()
{
    return true;
}


void CAlnMultiModel::ShowAll()
{
    vector<TNumrow> nothing;
    SetVisible(nothing, false, true); // hide nothing and make other visible
}


void CAlnMultiModel::EnableAutoSorting(bool b_en)
{
    m_AutoSort = b_en;
}


void CAlnMultiModel::UpdateSortOrder(void)
{
    m_AutoSort = true;

    x_DoUpdateRowsOrder();
    x_DoUpdateRowPositions();
}


// override this function in a derived class
bool CAlnMultiModel::SortByColumn(TColumnType type, bool ascending)
{
    m_SortColumn = type;
    if(m_SortColumn == IAlignRow::eInvalid) {
        m_SortState = IAlnExplorer::eNotSupported;
    } else {
        m_SortState = ascending ? IAlnExplorer::eAscending : IAlnExplorer::eDescending;
    }

    size_t n = m_vVisibleRows.size();

    switch(m_SortColumn)    {
    case IAlignRow::eDescr: 
    case IAlignRow::eTaxLabel: {
        CPairSorter<SRowRecord*, string> sorter(ascending);
        CPairSorter<SRowRecord*, string>::TPairVector data;
        data.reserve(n);

        for( size_t i = 0;  i < n;  i++ )    {
            SRowRecord* rec = m_vVisibleRows[i];
            IAlignRow* row = rec->m_Row;
            string s = row ? row->GetColumnAsString(type) : "";

            data.push_back(make_pair(rec, s));
        }

        sorter.Sort(data);

        for( size_t i = 0;  i < n;  i++ )    {
            m_vVisibleRows[i] = data[i].first;
        }
        x_DoUpdateRowLines();
        x_DoUpdateRowPositions();
        return true; // sorted
    }
    case IAlignRow::eStart:
    case IAlignRow::eEnd:
    case IAlignRow::eSeqStart:
    case IAlignRow::eSeqEnd:
    case IAlignRow::eSeqLength: {
    case IAlignRow::eInvalid:
        CPairSorter<SRowRecord*, TSeqPos> sorter(ascending);
        CPairSorter<SRowRecord*, TSeqPos>::TPairVector data;
        data.reserve(n);

        const TModelRect& rc_vis = m_Port.GetVisibleRect();
        for( size_t i = 0;  i < n;  i++ )    {
            SRowRecord* rec = m_vVisibleRows[i];
            IAlignRow* row = rec->m_Row;

            TSeqPos pos = 0xFFFFFFFF;

            if (row) {
                pos = (m_SortColumn == IAlignRow::eInvalid) ?
                    row->GetRowNum() : row->GetColumnAsSeqPos(type, rc_vis);
            }

            data.push_back(make_pair(rec, pos));
        }

        sorter.Sort(data);

        for( size_t i = 0;  i < n;  i++ )    {
            m_vVisibleRows[i] = data[i].first;
        }
        x_DoUpdateRowLines();
        x_DoUpdateRowPositions();
        return true; // sorted
    }
    default:
        break;
    }

    return false; // did not do anything
}


CAlnMultiModel::TSortState CAlnMultiModel::GetColumnSortState(TColumnType type)
{
    if(type != IAlignRow::eInvalid  &&  type == m_SortColumn)   {
        return m_SortState;
    } else {
        switch(type)    {
            case IAlignRow::eDescr:
            case IAlignRow::eStart:
            case IAlignRow::eEnd:
            case IAlignRow::eSeqStart:
            case IAlignRow::eSeqEnd:
            case IAlignRow::eSeqLength:
            case IAlignRow::eTaxLabel:
                return IAlnExplorer::eUnSorted;
            default:
                return IAlnExplorer::eNotSupported;
        }
    }
}


int CAlnMultiModel::GetShownElements()
{
    return m_ShownElements;
}


void CAlnMultiModel::SetShownElements(int shown_mask)
{
    m_ShownElements = shown_mask;
}


/// Moves specified rows into a specified postion.
/// order can be changed only for visible rows, hidden will be ignored.
/// This operation cancel Auto Sorting mode.
void CAlnMultiModel::SetRowOrder(const vector<TNumrow>& rows, int pos)
{
    m_AutoSort = false; // cancel auto sorting

    int n_vis = (int) m_vVisibleRows.size();
    if(pos >= 0  &&  pos < n_vis)   {
        vector<SRowRecord*> new_vis(n_vis, (SRowRecord*)NULL);
        // new container to replace m_vVisibleRows

        // move specified rows to new_order and replace them with NULL in m_vVisibleRows
        int i = 0, line = pos;
        for( ; i < (int) rows.size()  &&  line < n_vis; i++ )  {
            TNumrow row = rows[i];
            SRowRecord* p_rec = &x_GetRecordByRow(row);

            if(p_rec->IsVisible())  {
                new_vis[line] = p_rec;
                m_vVisibleRows[p_rec->m_Line] = NULL; // remove from m_vVisibleRows
                line++;
            }
        }

        // move the rest of the rows (those that are not NULL) preserving existing order
        // move first "pos" elements and insert them before "pos" position
        i = 0; // current postion in m_vVisibleRow
        int ins_i = 0; // insertion position in new_vis
        while(ins_i < pos)  {
            for( ; m_vVisibleRows[i] == NULL  &&  i < n_vis; i++ )   { // skip
            }
            _ASSERT(i < n_vis);
            new_vis[ins_i++] = m_vVisibleRows[i++];
        }
        // copy the rest of rows to new_vis after "line"
        ins_i = line;
        while(ins_i < n_vis)    {
            for( ; m_vVisibleRows[i] == NULL  &&  i < n_vis; i++ )   { // skip
            }
            _ASSERT(i < n_vis);
            new_vis[ins_i++] = m_vVisibleRows[i++];
        }

        // assign new_vis to m_vVisibleRows
        m_vVisibleRows = new_vis;

        x_DoUpdateRowLines();
        x_DoUpdateRowPositions();
    }
}


void CAlnMultiModel::SetDataSource(IAlnMultiDataSource* ds)
{
    m_DataSource = ds;

    if(m_DataSource)    {
        //TODO m_DataSource->SetGapChar('-');
        m_isDataReadSync = m_DataSource->IsDataReadSync();

        if (m_Style != NULL)
            m_DataSource->SetCreateConsensus(m_Style->m_ShowConsensus);
    }

    m_ScoreCache->SetAlignment(ds);

    UpdateOnDataChanged(); // before it called Widget's version of function
}


void CAlnMultiModel::SetListener(CScoreCache::IListener* listener)
{
    m_Listener = listener;
}


////////////////////////////////////////////////////////////////////////////////
/// Update handlers

void CAlnMultiModel::ClearRows()
{
    TNumrowModelImpl::DeleteAllItems();
/*
    NON_CONST_ITERATE(TRowToRecMap, it, m_RowToRec)  {
        // Exception when element is not initialized possible. JIRA : GB-1255
        if (*it) delete (*it)->m_Row;
        delete *it;
    }
    m_RowToRec.clear();
*/    

    m_MasterRow.Reset();
    m_DataConsensusRow.Reset();
    m_vVisibleRows.clear();
    m_vVisibleRowPos.clear();

    m_RowToRec.clear();
}


/// Creates IAlignRow objects for each row in the data source.
void CAlnMultiModel::CreateRows()
{
//    _ASSERT(CThread::GetSelf() == 0);
    if(m_DataSource)    {
        _ASSERT(m_RowToRec.size() == 0); //  && m_MasterRow == NULL);

        const int aln_rows_n = m_DataSource->GetNumRows();
        int pane_rows_n = aln_rows_n; // rows shown in align pane (Master row is not counted)

        TNumrow MasterRow = -1;
        int consensus_idx = m_DataSource->GetConsensusRow();

        //int consensus_idx = m_DataSource->GetConsensusRow();
        //        if (consensus_idx >= 0 && m_Style->m_ShowConsensus) {
        //  m_DataSource->SetAnchor(consensus_idx);
        //        } 
        
        if (m_DataSource->IsSetAnchor())   {
            MasterRow = m_DataSource->GetAnchor();
            const IAlignRowHandle* handle = m_DataSource->GetRowHandle(MasterRow);
            m_ConsensusRow->SetHandle(handle);
            --pane_rows_n;
        }

        TRowRecVector   vVisibleRows;
        vector<int>     vVisibleRowPos;
        TRowToRecMap    RowToRec;
        try
        {

//            m_vVisibleRows.resize(pane_rows_n);
//            m_vVisibleRowPos.resize(pane_rows_n);

            vVisibleRows.resize(pane_rows_n);
            vVisibleRowPos.resize(pane_rows_n);

            TLine i_line = 0;
            RowToRec.resize(aln_rows_n);
            //m_RowToRec.resize(aln_rows_n);

            for( TNumrow row = 0; row < aln_rows_n; row++ )    {
                // call a factory method to create a row instance
                IAlignRow* p_row = 0;
                // since RADAR might have multiple update events 
                // RADAR codes retains m_MasterRow to avoid rebuilding tracks on every update event
                if (row == MasterRow && m_MasterRow)
                    p_row = m_MasterRow;
                else 
                    p_row = x_CreateRow(row);
                // TODO: Do we need same logic as for master row here?
                if (row == consensus_idx)
                    m_DataConsensusRow = p_row;
//                m_RowToRec[row] = SRowRecord(p_row, i_line);
                RowToRec[row] = SRowRecord(p_row, i_line);

                if(row == MasterRow)    {
                   m_MasterRow = p_row;
                   //m_RowToRec[row].m_Row->SetRowState(IAlignRow::fItemHidden, true);
                   RowToRec[row].m_Row->SetRowState(IAlignRow::fItemHidden, true);
                } else {
                    vVisibleRows[i_line] = &RowToRec[row];
                    //m_vVisibleRows[i_line] = &m_RowToRec[row];
                    i_line++;
                }
            }
        } catch (CException& e) {
            // if error in the loop all results discarded
            this->ClearRows();
            NCBI_THROW(CCoreException, eNullPtr, "Failed to create alignment rows: " + e.GetMsg());
        }  catch (std::exception& e) {
            // if error in the loop all results discarded
            this->ClearRows();
            NCBI_THROW(CCoreException, eNullPtr, "Failed to create alignment rows: " + string(e.what()));
        }

        m_vVisibleRows.swap(vVisibleRows);
        m_vVisibleRowPos.swap(vVisibleRowPos);
        m_RowToRec.swap(RowToRec);

        if(x_UsingAutoSort()) {
            x_DoUpdateRowsOrder(); // applying sorting
        }

        // updating CSelListModelImpl
        TNumrowModelImpl::TItemVector vItems(pane_rows_n);
        for( size_t i = 0; i < m_vVisibleRows.size(); i++ ) {
            vItems[i] = m_vVisibleRows[i]->m_Row->GetRowNum();
        }
        TNumrowModelImpl::SetItems(vItems, false, false);

        x_DoUpdateRowPositions();
    }
}

void CAlnMultiModel::CreateConsensus(void)
{
    m_DataSource->SetCreateConsensus(true);
    m_DataSource->CreateConsensus();
}


bool CAlnMultiModel::x_SortRows()
{
    return false; // nothing has been changed
}


void CAlnMultiModel::x_DoUpdateRowsOrder()
{
    _ASSERT(x_UsingAutoSort()); // must be called only when auto sorting is enabled

    // reme
    TIndex i_focused = SLM_GetFocusedItemIndex();
    const SRowRecord* p_rec = (i_focused >= 0) ? m_vVisibleRows[i_focused] : NULL;

    if(x_SortRows())  {
        x_DoUpdateRowLines();

        // restore focus
        int line = p_rec ? p_rec->m_Line : -1;
        SLM_FocusItem(line);
    }
}


void CAlnMultiModel::x_DoUpdateRowLines()
{
    for(size_t i = 0; i < m_vVisibleRows.size(); i++ )  {
        SRowRecord& rec = *m_vVisibleRows[i];
        rec.m_Line = (int) i;
    }
}


void CAlnMultiModel::UpdateOnStyleChanged(void)
{
    NON_CONST_ITERATE(TRowToRecMap, it, m_RowToRec)  {
        IAlignRow* row = it->m_Row.GetPointer();

        TNumrow row_num = row->GetRowNum();
        const CRowDisplayStyle* style = m_StyleCatalog->GetStyleForRow(row_num);
        row->SetDisplayStyle(style);

        row->UpdateOnStyleChanged();
    }
    x_DoUpdateRowPositions();

    x_UpdateMinScale();
    SetPortLimits();
}


void CAlnMultiModel::UpdateOnRowHChanged(void)
{
    x_DoUpdateRowPositions();
    SetPortLimits();
}


/// recalculates vertical positions of row objects based on their order,
/// visibility and height
void CAlnMultiModel::x_DoUpdateRowPositions(void)
{
    int N = (int) m_vVisibleRows.size();
    m_vVisibleRowPos.resize(N);
    for( int  total_H = 0, i = 0; i < N; i++ )    {
        IAlignRow* pRow = x_GetRowByLine(i);
        m_vVisibleRowPos[i] =  total_H;
        total_H += pRow->GetHeightPixels();
    }
}


void CAlnMultiModel::SetPortLimits()
{
    if (m_DataSource) {
        double Start = m_DataSource->GetAlnStart();
        double Stop = m_DataSource->GetAlnStop();

        double H = 0;
        int iLast = (int) m_vVisibleRows.size() - 1;
        if(iLast >= 0)
            H = GetLinePosY(iLast) + GetLineHeight(iLast);

        m_Port.SetModelLimitsRect(TModelRect(Start, H - 1, Stop + 1, 0));
    } else  {
        m_Port.SetModelLimitsRect(TModelRect(0, -1, 0, 0));
    }
}


bool CAlnMultiModel::CanChangeMasterRow() const
{
    return m_DataSource  &&  m_DataSource->CanChangeAnchor();
}


bool CAlnMultiModel::SetMasterRow(TNumrow new_master_row)
{
    //Anchor the row to make it "Master"
    m_DataSource->SetAnchor(new_master_row);

    // rearrange Row objects and items
    IAlignRow* pNewMaster = NULL;
    if(new_master_row != -1)    {     // remove row from the Align pane
        SRowRecord& rec = x_GetRecordByRow(new_master_row);
        pNewMaster = rec.m_Row;

        rec.m_Row->SetRowState(IAlignRow::fItemHidden, true);
      
        const IAlignRowHandle* handle = m_DataSource->GetRowHandle(new_master_row);
        m_ConsensusRow->SetHandle(handle);

        vector<int> vDelItems;
        vDelItems.push_back(rec.m_Line);
        DeleteItems(vDelItems, false);
    }
    else {
        m_ConsensusRow->SetHandle(NULL);
    }

    if(m_MasterRow)    { // insert current Master into Align pane
        TNumrow row = m_MasterRow->GetRowNum();
        SRowRecord& rec = x_GetRecordByRow(row);
        TLine line = min(rec.m_Line, (TLine) m_vVisibleRows.size());

        rec.m_Row->SetRowState(IAlignRow::fItemHidden, false);
        InsertItem(line, m_MasterRow->GetRowNum(), false);
    }

    m_MasterRow = pNewMaster;

    // update rows - they may be affected by this change
    NON_CONST_ITERATE(TRowToRecMap, it, m_RowToRec)   {
        IAlignRow* row = it->m_Row.GetPointer();
        row->UpdateOnAnchorChanged();
    }

    if(x_UsingAutoSort()) {
        x_DoUpdateRowsOrder(); // applying sorting
    }
    x_DoUpdateRowPositions();

    SetPortLimits(); // set new limits

    x_UpdateOnCoordsChanged();
    return true;
}


void CAlnMultiModel::UpdateOnDataChanged()
{
    ClearRows();

    if(m_DataSource) {
        CreateRows();
        SetPortLimits();
    }

    m_ScoreCache->SetAlignment(m_DataSource);

    if(m_DataSource  &&  m_EnableScoring)    {
        m_ScoreCache->CalculateScores();
    }
}


void CAlnMultiModel::x_UpdateOnCoordsChanged()
{
    // this can be eliminated if scores are saved in seq coords
    if(m_EnableScoring)    {
        m_ScoreCache->CalculateScores();
    }
}


void CAlnMultiModel::UpdateOnScoringChanged()
{
    if(m_EnableScoring  &&  m_ScoreCache->GetAlignment()) {
        m_ScoreCache->CalculateScores();
    }
}


bool CAlnMultiModel::x_UsingAutoSort() const
{
    return m_AutoSort;
}


IAlignRow* CAlnMultiModel::x_GetRowByLine(int index)
{
    if (m_vVisibleRows.empty())
        return 0;
    _ASSERT(index >= 0 && (size_t) index < m_vVisibleRows.size());
    SRowRecord* p_rec = m_vVisibleRows[index];
    return static_cast<IAlignRow*>(p_rec->m_Row);
}


const IAlignRow*  CAlnMultiModel::x_GetRowByLine(int index) const
{
    if (m_vVisibleRows.empty())
        return 0;
    _ASSERT(index >= 0 && (size_t)index < m_vVisibleRows.size());
    SRowRecord* p_rec = m_vVisibleRows[index];
    return static_cast<IAlignRow*>(p_rec->m_Row);
}


bool CAlnMultiModel::AddScoringMethod(IScoringMethod* method)
{
    CIRef<IScoringMethod> ref(method);
    TMethods::const_iterator it = std::find(m_Methods.begin(), m_Methods.end(), ref);
    if(it == m_Methods.end())   {
        m_Methods.push_back(ref);
        return true;
    }
    return false;
}


const CAlnMultiModel::TMethods& CAlnMultiModel::GetMethods()
{
    return  m_Methods;
}


bool CAlnMultiModel::SetCurrentMethod(const string& name)
{
    ITERATE(TMethods, it, m_Methods)   {
        CIRef<IScoringMethod> method = *it;
        if(method->GetName() == name)   { // found it
            m_ScoreCache->SetScoringMethod(method.GetPointer());
            UpdateOnScoringChanged();
            return true;
        }
    }
    return false;
}


void CAlnMultiModel::ResetCurrentMethod()
{
    m_ScoreCache->SetScoringMethod(NULL);
    UpdateOnScoringChanged();
}


string  CAlnMultiModel::GetDefaultMethod(void)
{
    if(m_DataSource)    {
        switch(m_DataSource->GetAlignType())    {
        case IAlnExplorer::fDNA:
            return m_Style->m_DefDNAMethod;
        case IAlnExplorer::fProtein:
            return m_Style->m_DefProteinMethod;
        default:
            break;
        }
    }
    return "";
}


void CAlnMultiModel::SetDefaultMethod(const string& method_name)
{
    if(m_DataSource)    {
        switch(m_DataSource->GetAlignType())    {
        case IAlnExplorer::fDNA:
            m_Style->m_DefDNAMethod = method_name;
            break;
        case IAlnExplorer::fProtein:
            m_Style->m_DefProteinMethod = method_name;
            break;
        default:
            break;
        }
    } else _ASSERT(false);
}


void CAlnMultiModel::OnScoringProgress(float progress, const string& msg)
{
    if(m_Listener)  {
        m_Listener->OnScoringProgress(progress, msg);
    }
}


void CAlnMultiModel::OnScoringFinished()
{
    if(m_Listener)  {
        m_Listener->OnScoringFinished();
    }
}


const IScoringMethod* CAlnMultiModel::GetCurrentMethod() const
{
    return m_ScoreCache->GetScoringMethod();
}


void CAlnMultiModel::EnableBackgroundProcessing(bool en)
{
    m_ScoreCache->EnableBackgoundProcessing(en);
}


IScoringMethod* CAlnMultiModel::GetCurrentMethod()
{
    return m_ScoreCache->GetScoringMethod();
}


////////////////////////////////////////////////////////////////////////////////
// CSelListModelImpl virtual functions

void CAlnMultiModel::SLM_SelectAll(bool bSelect)
{
    if (m_MasterRow)
        m_MasterRow->SetRowState(IAlignRow::fItemSelected, bSelect);

    TNumrowModelImpl::SLM_SelectAll(bSelect);
}

void CAlnMultiModel::SLM_SelectSingleItem(TIndex index)
{
    if (m_MasterRow)
        m_MasterRow->SetRowState(IAlignRow::fItemSelected,
                                 (index == kMasterRowIndex) ? true : false);

    TNumrowModelImpl::SLM_SelectSingleItem(index);
}

void CAlnMultiModel::SLM_InvertSingleItem(TIndex index)
{
    if (m_MasterRow && index == kMasterRowIndex) {
        bool selected = (m_MasterRow->GetRowState()&IAlignRow::fItemSelected) != 0;
        m_MasterRow->SetRowState(IAlignRow::fItemSelected, !selected);
    }
    else
        TNumrowModelImpl::SLM_InvertSingleItem(index);
}

void CAlnMultiModel::x_SelectItem(TIndex index, bool b_sel)
{
    if (m_vVisibleRows.empty())
        return;
    m_vVisibleRows[index]->m_Row->SetRowState(IAlignRow::fItemSelected, b_sel);
}


bool CAlnMultiModel::x_IsItemSelected(TIndex index) const
{
    if (m_vVisibleRows.empty())
        return false;
    return m_vVisibleRows[index]->IsSelected();
}


// returns number of visible rows (lines)
CAlnMultiModel::TIndex CAlnMultiModel::x_GetItemsCount()   const
{
    return (TIndex) m_vVisibleRows.size();
}


CAlnMultiModel::TItem CAlnMultiModel::x_GetItem(TIndex index)    const
{
    _ASSERT(x_GetRowByLine(index));
    TNumrow row = x_GetRowByLine(index)->GetRowNum();
    return row;
}


void CAlnMultiModel::x_SetEntries(const TEntryVector& v_entries)
{
    size_t entries_n = v_entries.size();
    m_vVisibleRows.resize(entries_n);
    for( size_t i = 0; i < entries_n; i++ ) {
        TNumrow row = v_entries[i].first;
        SRowRecord& rec = m_RowToRec[row];
        rec.m_Line = (int) i;
        rec.m_Row->SetRowState(IAlignRow::fItemSelected, v_entries[i].second);
        rec.m_Row->SetRowState(IAlignRow::fItemHidden, false);

        m_vVisibleRows[i] = &rec;
    }
}


CAlnMultiModel::TIndex CAlnMultiModel::x_GetItemIndex(const TItem& item)
{
    SRowRecord& rec = m_RowToRec[item];
    return rec.IsVisible() ? rec.m_Line : -1;
}


void CAlnMultiModel::x_InsertItem(TIndex index, const TItemEntry& entry)
{
    TNumrow row = entry.first;
    SRowRecord& p_rec = m_RowToRec[row];
    p_rec.m_Row->SetRowState(IAlignRow::fItemSelected, entry.second);
    p_rec.m_Line = index;
    m_vVisibleRows.insert(m_vVisibleRows.begin() + index, &p_rec);
}


void CAlnMultiModel::x_CompleteInsertion()
{
    x_DoUpdateRowLines();
}


void CAlnMultiModel::x_MarkItemForErase(TIndex index)
{
    m_vVisibleRows[index]->m_Row->SetRowState(IAlignRow::fItemHidden, true);
    m_vVisibleRows[index]->m_Row->SetRowState(IAlignRow::fItemSelected, false);
    m_vVisibleRows[index] = NULL;
}


void CAlnMultiModel::x_EraseMarkedItems()
{
    int shift = 0;
    int count = (int) m_vVisibleRows.size();
    for(int i = 0; i < count; i++ ) {
        if(m_vVisibleRows[i])   {
            if(shift >0)    {
                m_vVisibleRows[i - shift] = m_vVisibleRows[i];
                m_vVisibleRows[i - shift]->m_Line = i - shift;
            }
        } else   shift++;
    }
    m_vVisibleRows.resize(m_vVisibleRows.size() - shift);
}


void CAlnMultiModel::x_ClearItems()
{
    for( size_t i = 0; i < m_vVisibleRows.size(); i++ ) {
        if (m_vVisibleRows[i]) {  // Exception when element is not initialized possible. JIRA : GB-1255
            m_vVisibleRows[i]->m_Row->SetRowState(IAlignRow::fItemHidden, true);
            m_vVisibleRows[i]->m_Row->SetRowState(IAlignRow::fItemSelected, false);
        }
    }

    m_vVisibleRows.clear();
}

void  CAlnMultiModel::x_UpdateOnVisibilityChanged(void)
{
    // do nothing
}



END_NCBI_SCOPE
