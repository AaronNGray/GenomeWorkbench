/*  $Id: alnmulti_renderer.cpp 45044 2020-05-14 17:58:58Z shkeda $
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
#include <corelib/ncbistl.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/widgets/aln_multiple/alnmulti_renderer.hpp>
#include <gui/opengl/irender.hpp>
#include <objects/seq/seqport_util.hpp>
#include <objects/seqcode/Seq_code_type.hpp>
#include <gui/objutils/tooltip.hpp>

#include <algorithm>
#include <sstream>

BEGIN_NCBI_SCOPE

static const int kRulerSpace = 4;
static const int kMasterRowSpace = 0;

using namespace objects;

CAlnMultiRenderer::CAlnMultiRenderer(const TVPRect &rc, bool add_columns)
    : m_Context(NULL),
      m_ResizableColumnIndex(-1),
      m_bFocused(true),
      m_HitResult(eNone),
      m_TooltipRow(NULL),
      m_TooltipColumn(-1)
{
    m_Header.SetContext(this);
    m_RulerPane.EnableOffset();

    SetBackColor(CRgbaColor(0.95f, 1.0f, 0.95f));       //light green
    SetMasterBackColor(CRgbaColor(0.85f, 1.0f, 0.85f)); // slightly darker green

    m_Ruler.SetHorizontal(true, CRuler::eTop);
    m_Ruler.SetFont(CGlTextureFont::eFontFace_Helvetica, 10);

    if (add_columns)
        CAlnMultiRenderer::SetupColumns(rc);
}

CAlnMultiRenderer::~CAlnMultiRenderer()
{
}

void CAlnMultiRenderer::SetupColumns(const TVPRect &rc)
{
    AddColumn(120, "Description", IAlignRow::eDescr);
    //    AddColumn(20, "Strand", IAlignRow::eIconStrand);
    //    AddColumn(20, "+", IAlignRow::eIconExpand);
    AddColumn(40, "Markers", IAlignRow::eIcons);
    AddColumn(50, "Seq. Start", IAlignRow::eSeqStart);
    AddColumn(60, "First", IAlignRow::eStart);

    int index = AddColumn(0, "Alignment", IAlignRow::eAlignment); // resizable, takes 100% of free space
    SetResizableColumn(index);

    AddColumn(50, "Last", IAlignRow::eEnd);
    AddColumn(50, "Seq. End", IAlignRow::eSeqEnd);
    AddColumn(50, "Seq. Length", IAlignRow::eSeqLength);
    AddColumn(150, "Org. Name", IAlignRow::eTaxLabel);

    const char *v = (const char *)glGetString(GL_VERSION);
    if (!v)
        return;

    Resize(rc);
}

void CAlnMultiRenderer::SetContext(IAlnMultiRendererContext *context)
{
    m_Context = context;
}

void CAlnMultiRenderer::SetBackColor(const CRgbaColor color)
{
    m_BackColor = color;
}

const CRgbaColor &CAlnMultiRenderer::GetBackColor() const
{
    return m_BackColor;
}

void CAlnMultiRenderer::SetMasterBackColor(const CRgbaColor color)
{
    m_MasterBackColor = color;
}

const CRgbaColor &CAlnMultiRenderer::GetMasterBackColor() const
{
    return m_MasterBackColor;
}

CRuler &CAlnMultiRenderer::GetRuler()
{
    return m_Ruler;
}

void CAlnMultiRenderer::Resize(const TVPRect &rc)
{
    m_rcBounds = rc;
    x_LayoutColumns();
    x_Layout();
}

TVPRect CAlnMultiRenderer::GetRect() const
{
    return m_rcBounds;
}

CAlnMultiHeader &CAlnMultiRenderer::GetHeader()
{
    return m_Header;
}

/// Graphics is rendered inthe current OpenGL context in the viewport defined
/// by the / following rectangle
/// (0, 0, m_rcBounds.Width() - 1, m_rcBounds.Height() - 1)
void CAlnMultiRenderer::Render()
{
    x_Render(NULL);
}

void CAlnMultiRenderer::Render(TAreaVector &areas)
{
    x_Render(&areas);
}

void CAlnMultiRenderer::x_Render(TAreaVector *p_areas)
{
    string error;
    try
    {
        glClearColor(m_BackColor.GetRed(), m_BackColor.GetGreen(),
                     m_BackColor.GetBlue(), m_BackColor.GetAlpha());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        int shown = x_GetContext() ? x_GetContext()->GetShownElements() : IAlnMultiRendererContext::fShownElement_All;

        if (shown & IAlnMultiRendererContext::fShownElement_Header)
            x_RenderHeader(p_areas);
        if (shown & IAlnMultiRendererContext::fShownElement_Ruler)
            x_RenderRuler(p_areas);

        x_ResetRowListMap();
        if (shown & IAlnMultiRendererContext::fShownElement_MasterRow)
        {
            x_RenderMasterRow(p_areas);
        }
        if (shown & (IAlnMultiRendererContext::fShownElement_Main))
        {
            x_RenderItems(p_areas);
            x_RenderAlignAreaTopLine();
        }
        x_PurgeRowListMap();
    }
    catch (CException &e)
    {
        error = e.GetMsg();
    }
    catch (std::exception &e)
    {
        error = e.what();
    }
    if (!error.empty())
    {
        ERR_POST("CAlnMultiRenderer::x_Render() " << error);
    }
}

void CAlnMultiRenderer::x_RenderAlignAreaTopLine()
{
    if (!x_GetContext() || m_RowToList.empty())
        return;
    IAlignRow *row = x_GetContext()->GetMasterRow();
    if (!row)
        row = x_GetContext()->GetRowByLine(0);
    if (!row)
        return;
    const CRowDisplayStyle *style = row->GetDisplayStyle();
    if (!style)
        return;
    if (style->IsWebRendering())
        return;
    // render a top line
    IRender &gl = GetGl();
    CGlPane pane(CGlPane::eNeverUpdate);
    TVPRect rect = x_GetContext()->GetMasterRow() ? m_MasterRect : m_ListAreaRect;
    rect.SetTop(rect.Top() + 1);
    pane.SetViewport(rect);
    pane.OpenPixels();
    gl.ColorC(style->GetColor(CWidgetDisplayStyle::eFrame));
    gl.Begin(GL_LINES);
    gl.Vertex2d(rect.Left(), rect.Top());
    gl.Vertex2d(rect.Right(), rect.Top());
    gl.End();
    pane.Close();
}

void CAlnMultiRenderer::Update(bool layout_only)
{
    x_Layout();

    x_InvalidateRows(layout_only);
}

void CAlnMultiRenderer::UpdateOnDataChanged()
{
    x_Layout();

    m_RowToList.clear();
}

bool CAlnMultiRenderer::NeedTooltip(int vp_x, int vp_y)
{
    m_TooltipVPPos.Init(vp_x, vp_y);
    m_TooltipRow = NULL;

    m_HitResult = HitTest(vp_x, vp_y, m_TooltipColumn);

    switch (m_HitResult)
    {
    case eHeader:
    {
        CGlPane pane; // TODO
        return m_Header.NeedTooltip(pane, vp_x, vp_y);
    }
    case eRows:
    {
        int line = x_GetLineByVPY(vp_y);
        if (line != -1)
        {
            m_TooltipRow = x_GetContext()->GetRowByLine(line);
        }
        CIRef<IAlignRow> row(m_TooltipRow.Lock());
        return row.NotNull();
        //return m_TooltipRow != 0;
    }
    case eMasterRow:
    {
        m_TooltipRow = x_GetContext()->GetMasterRow();
        //return m_TooltipRow != 0;
        CIRef<IAlignRow> row(m_TooltipRow.Lock());
        return row.NotNull();
    }
    default:
        return false;
    }
}

string CAlnMultiRenderer::GetTooltip()
{
    switch (m_HitResult)
    {
    case eHeader:
        return m_Header.GetTooltip();
    case eMasterRow:
    {
        int row_top_y = GetMasterArea().Top(); // -kMasterRowSpace;
        CIRef<IAlignRow> row(m_TooltipRow.Lock());
        if (row.IsNull())
            break;
        return x_GetRowTooltip(m_TooltipVPPos, row, m_TooltipColumn, row_top_y);
    }
    case eRows:
    {
        CIRef<IAlignRow> row(m_TooltipRow.Lock());
        if (row.IsNull())
            break;
        int line = x_GetContext()->GetLineByRowNum(row->GetRowNum());
        if (line == -1)
        { // not a visible row...
            break;
        }
        int model_y = x_GetContext()->GetLinePosY(line);
        int vp_top = GetVPListTop() + (int)x_GetContext()->GetAlignPort().GetVisibleRect().Top();
        int row_top_y = vp_top - model_y;
        return x_GetRowTooltip(m_TooltipVPPos, row, m_TooltipColumn, row_top_y);
    }
    default:
        break;
    };
    return "";
}

string CAlnMultiRenderer::GetAlignmentTooltip(int x, int y, IAlignRow *row, bool master)
{
    int row_top_y;
    // Our y coordinate is actually a "model" coordinate, so convert it to VP
    int vp_top = m_rcBounds.Top();
    int vp_y = vp_top - y;
    if (master)
    {
        row_top_y = GetMasterArea().Top() - kMasterRowSpace;
    }
    else
    {
        int line = x_GetContext()->GetLineByRowNum(row->GetRowNum());
        if (line == -1)
        { // not a visible row...
            return "";
        }
        int model_y = x_GetContext()->GetLinePosY(line);
        int vp_top1 = GetVPListTop() + (int)x_GetContext()->GetAlignPort().GetVisibleRect().Top();
        row_top_y = vp_top1 - model_y;
    }
    TVPPoint vpt(x, vp_y);
    return x_GetRowTooltip(vpt, row, GetColumnIndexByType(IAlignRow::eAlignment), row_top_y);
}

void CAlnMultiRenderer::x_AddStatisticsRow(ITooltipFormatter &formatter, const string &tag, int part, int total)
{
    stringstream statistics;
    statistics << part << " of " << total << " rows (" << NStr::DoubleToString((double)100 * part / total, 1, NStr::fWithCommas) + "%)";
    formatter.AddRow(tag, statistics.str());
}

void CAlnMultiRenderer::x_PrepareMasterPane(CGlPane &pane)
{
    const CGlPane &port = x_GetContext()->GetAlignPort();

    auto rc_vp = m_MasterRect;
    rc_vp.SetBottom(port.GetViewport().Bottom());
    pane.SetViewport(rc_vp);

    //pane.SetViewport(m_MasterRect);
    int H = x_GetContext()->GetMasterRow()->GetHeightPixels();
    // setup model space
    TModelRect rcV = port.GetVisibleRect();
    rcV.SetVert(rc_vp.Height() - 1, 0);
    pane.SetVisibleRect(rcV);

    TModelRect rcLim = port.GetModelLimitsRect();
    rcLim.SetVert(H - 1, 0);
    pane.SetModelLimitsRect(rcLim);
    pane.SetExactOrthoProjection(m_bCgiMode == false); // need this to avoid text 'popping up and down' effect on window resize
}

string CAlnMultiRenderer::x_GetRowTooltip(const TVPPoint &pt, IAlignRow *row, int i_col, int vp_top_y)
{
    _ASSERT(row);

    CGlPane pane(x_GetContext()->GetAlignPort());
    pane.EnableOffset();
    if (x_GetContext()->GetMasterRow() == row)
    {
        x_PrepareMasterPane(pane);
    }
    row->PrepareRendering(pane, vp_top_y, 0);
    SetupPaneForColumn(pane, i_col);

    CIRef<ITooltipFormatter> formatter = CHtmlTooltipFormatter::CreateTooltipFormatter();
    TColumnType type = GetColumnTypeByIndex(i_col);
    row->GetTooltip(pt, type, pane, *formatter);
    if (type != IAlignRow::eAlignment)
    {
        return formatter->Render();
    }

    string colbases;
    typedef map<char, int> MapCharInt;
    MapCharInt counters;

    TSeqPos pos = (TSeqPos)pane.UnProjectX(pt.X());
    string base;
    row->GetStringAtPos(base, pos);
    if (base.empty())
        return formatter->Render();

    int num_rows = x_GetContext()->GetLinesCount();
    if (!formatter->IsEmpty())
        formatter->AddDividerRow();
    char symbol = 0;
    if (base.empty())
    {
        symbol = ' ';
    }
    else if (!(symbol = base[0]))
    {
        symbol = '-';
    }

    if (row->UsesAATranslation())
    {
        string colstat = string(1, symbol);
        try
        {
            colstat = CSeqportUtil::GetName(eSeq_code_type_iupacaa, CSeqportUtil::GetIndex(eSeq_code_type_iupacaa, colstat));
        }
        catch (const CSeqportUtil::CBadSymbol &)
        {
            //colstat += "n/a";
        }
        formatter->AddRow("Amino acid:", colstat);
    }
    else
    {
        string colstat = string(1, symbol);
        try
        {
            colstat = CSeqportUtil::GetName(eSeq_code_type_iupacna, CSeqportUtil::GetIndex(eSeq_code_type_iupacna, colstat));
        }
        catch (const CSeqportUtil::CBadSymbol &)
        {
        }
        formatter->AddRow("Base:", colstat);
    }

    auto base_width = row->UsesAATranslation() && row->IsNucProtAlignment() ? 3 : 1;
    if (base_width != 3)
    { // count mis/matches only if it's not mixed alignment
        IAlnExplorer::TSignedRange posrange(pos, pos);

        IAlignRow *consensus_row = x_GetContext()->GetConsensusRow();
        for (int i = -1; i < num_rows; i++)
        {
            IAlignRow *itrow =
                i < 0
                    ? x_GetContext()->GetMasterRow()
                    : x_GetContext()->GetRowByLine(i);
            if (itrow == NULL || itrow == consensus_row)
                continue;

            string base;
            itrow->GetAlnSeqString(base, posrange);

            char symbol = 0;
            if (base.empty())
            {
                symbol = ' ';
            }
            else if (!(symbol = base[0]))
            {
                symbol = '-';
            }

            colbases += symbol;
            counters[symbol]++;
        }

        int match = 0;
        int mismatch = 0;
        int gaps = 0;
        int unaligned = 0;
        int total = 0;
        string colstat;

        ITERATE(MapCharInt, itr, counters)
        {
            char sym = itr->first;
            int num = itr->second;

            if (sym == ' ')
            {
                unaligned += num;
            }
            else if (sym == '-')
            {
                gaps += num;
            }
            else if (sym == symbol)
            {
                match = num;
            }
            else
            {
                mismatch += num;
            }

            total += num;
        }

        ITooltipFormatter &tip_formatter = *formatter;
        x_AddStatisticsRow(tip_formatter, "Matches:", match, total);
        x_AddStatisticsRow(tip_formatter, "Mismatches:", mismatch, total);
        x_AddStatisticsRow(tip_formatter, "Gaps:", gaps, total);
        x_AddStatisticsRow(tip_formatter, "Unaligned:", unaligned, total);
        formatter->AddRow("Total rows:", NStr::IntToString(total, NStr::fWithCommas));
    }

    row->GetRowStatistics(*formatter);

    return formatter->Render();
}

////////////////////////////////////////////////////////////////////////////////
/// protected members
int CAlnMultiRenderer::GetListTop() const
{
    return m_rcBounds.Height() - m_ListAreaRect.Height();
}

int CAlnMultiRenderer::GetColumnIndexByType(TColumnType type) const
{
    int n_col = GetColumnsCount();
    for (int i = 0; i < n_col; i++)
    {
        if (m_Columns[i].m_UserData == type)
            return i;
    }
    return -1;
}

TVPRect CAlnMultiRenderer::GetColumnRect(int i_col, bool include_header) const
{
    const SColumn &Col = GetColumn(i_col);
    TVPUnit h_h = include_header ? 0 : m_HeaderRect.Height();
    TVPUnit h = m_rcBounds.Height() - 1 - h_h;
    return TVPRect(Col.m_Pos, 0, Col.m_Pos + Col.m_Width - 1, h);
}

TVPRect CAlnMultiRenderer::GetColumnRectByType(TColumnType Type, bool include_header) const
{
    int iCol = GetColumnIndexByType(Type);
    return GetColumnRect(iCol, include_header);
}

IAlignRow::TColumnType CAlnMultiRenderer::GetColumnTypeByIndex(int i_col) const
{
    _ASSERT(i_col > -1);
    int type = (int)GetColumn(i_col).m_UserData;
    return (IAlignRow::TColumnType)type;
}

IAlignRow::TColumnType CAlnMultiRenderer::GetColumnTypeByX(int x) const
{
    int i_col = GetColumnIndexByX(x);
    if (i_col >= 0)
    {
        int type = (int)GetColumn(i_col).m_UserData;
        return (IAlignRow::TColumnType)type;
    }
    else
        return IAlignRow::eInvalid;
}

void CAlnMultiRenderer::x_Layout()
{
    int shown = x_GetContext() ? x_GetContext()->GetShownElements() : IAlnMultiRendererContext::fShownElement_All;

    // we arrange things from the top down
    TVPUnit h = 0, top = m_rcBounds.Top();

    // Header is placed at the top and takes as much space as needed
    if (shown & IAlnMultiRendererContext::fShownElement_Header)
    {
        m_HeaderRect = m_rcBounds;
        h = m_Header.PreferredSize().Y();
        m_HeaderRect.SetBottom(m_rcBounds.Top() - h + 1);
        m_Header.SetVPRect(m_HeaderRect);
        top = m_HeaderRect.Bottom() - 1;
    }
    else
    {
        m_HeaderRect.Init(0, 0);
    }

    bool align_visible = false;
    const SColumn *col = 0;
    int iAlign = GetColumnIndexByType(IAlignRow::eAlignment);
    if (iAlign >= 0)
    {
        col = &GetColumn(iAlign);
        align_visible = col->VisibleWidth();
    }

    // Ruler is placed below the header
    if (shown & IAlnMultiRendererContext::fShownElement_Ruler)
    {
        m_RulerRect = m_rcBounds;
        h = m_Ruler.GetPreferredSize().Y() + kRulerSpace;
        m_RulerRect.SetRight(m_RulerRect.Left()); // reset width to 0

        // determine horizontal extent of the Ruler
        if (align_visible)
        {
            _ASSERT(col);
            m_RulerRect.SetVert(top - h + 1, top);
            m_RulerRect.SetHorz(col->m_Pos, col->m_Pos + col->m_Width - 1);
        }
        else
        {
            m_RulerRect.SetVert(top, top);
        }
        m_Ruler.SetVPRect(m_RulerRect);
        top = m_RulerRect.Bottom() - 1;
    }
    else
    {
        m_RulerRect.Init(0, 0);
    }

    if (shown & IAlnMultiRendererContext::fShownElement_MasterRow)
    {
        // If Master is set the Master row are is placed right below the Ruler
        h = kMasterRowSpace * 2;
        if (x_GetContext())
        {
            IAlignRow *row = x_GetContext()->GetMasterRow();
            if (row)
                h += (row->GetHeightPixels() - 1);
        }
        if (h > 0)
        {
            m_MasterRect = m_rcBounds;
            m_MasterRect.SetVert(top - h, top);
            top = m_MasterRect.Bottom() - 1;
        }
        else
        {
            m_MasterRect.SetVert(top, top);
        }
    }
    else
    {
        m_MasterRect.Init(0, 0);
    }

    // other rows occupy all the remaining space
    m_ListAreaRect = m_rcBounds;
    m_ListAreaRect.SetTop(top);
}

void CAlnMultiRenderer::x_RenderHeader(TAreaVector * /*p_areas*/)
{
    CGlPane pane(CGlPane::eNeverUpdate);
    pane.SetViewport(m_HeaderRect);
    pane.SetExactOrthoProjection(m_bCgiMode == false); // need this to avoid text 'popping up and down' effect on window resize
    m_Header.Render(pane);
}

void CAlnMultiRenderer::x_RenderRuler(TAreaVector *p_areas)
{
    if (m_RulerRect.Width() > 1 && m_RulerRect.Height() > 1)
    {
        m_RulerPane.SetViewport(m_RulerRect);

        const CGlPane &port = x_GetContext()->GetAlignPort();

        TModelRect rcM = port.GetModelLimitsRect();
        rcM.SetVert(0, m_RulerRect.Height() - 1);
        m_RulerPane.SetModelLimitsRect(rcM);

        TModelRect rcV = port.GetVisibleRect();
        rcV.SetVert(0, m_RulerRect.Height() - 1);
        m_RulerPane.SetVisibleRect(rcV);

        m_Ruler.Render(m_RulerPane); // render

        if (p_areas)
        {
            CHTMLActiveArea area(eHtmlArea_Link, m_RulerRect,
                                 "Ruler", "Ruler", "");
            p_areas->push_back(area);
        }
    }
}

void CAlnMultiRenderer::x_RenderMasterRow(TAreaVector *p_areas)
{
    IAlignRow *p_row = x_GetContext()->GetMasterRow();
    if (p_row)
    {
        IRender &gl = GetGl();

        CGlPane pane(CGlPane::eNeverUpdate);
        pane.EnableOffset();
        x_PrepareMasterPane(pane);
        // fill the background
        pane.OpenPixels();
        gl.Color3f(m_MasterBackColor.GetRed(), m_MasterBackColor.GetGreen(), m_MasterBackColor.GetBlue());
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gl.Rectd(m_MasterRect.Left(), m_MasterRect.Top() - p_row->GetHeightPixels(), m_MasterRect.Right(), m_MasterRect.Top());
        pane.Close();

        // now render the row
        int state = x_GetContext()->IsRendererFocused() ? IAlignRow::fWidgetFocused : IAlignRow::fNone;

        int row_state = p_row->GetRowState();
        if (row_state & IAlignRow::fItemSelected)
        { // master row selected ?
            state |= IAlignRow::fItemSelected;
        }

        //TVPUnit top_y = m_MasterRect.Bottom() + p_row->GetHeightPixels();
        x_RenderRow(p_row, pane, state, m_MasterRect.Top() - 1, p_areas);
    }
}

// renders all visible rows in the List Area
void CAlnMultiRenderer::x_RenderItems(TAreaVector *p_areas)
{
    if (x_GetContext())
    {

        // calculate visible range
        const CGlPane &port = x_GetContext()->GetAlignPort();
        TModelRect rcM = port.GetVisibleRect();

        int iFirst = x_GetContext()->GetLineByModelY((int)rcM.Top());
        iFirst = max(iFirst, 0);

        int iLast = x_GetContext()->GetLineByModelY((int)rcM.Bottom());
        iLast = (iLast == -1) ? x_GetContext()->GetLinesCount() - 1 : iLast;

        if (iLast >= iFirst)
        {
            x_RenderItemsRange(iFirst, iLast, p_areas);
        }
    }
}

// renders the specified range of the alignment rows
void CAlnMultiRenderer::x_RenderItemsRange(int iFirst, int iLast,
                                           TAreaVector *p_areas)
{
    if (x_GetContext())
    {
        const CGlPane &port = x_GetContext()->GetAlignPort();

        // create clipping rectangle
        TVPRect rc_clip = port.GetViewport();
        rc_clip.SetHorz(0, m_rcBounds.Width() - 1);
        rc_clip.SetTop(rc_clip.Top() - 1);

        if (rc_clip.Height() && rc_clip.Width())
        {
            int y1 = GetVPListTop() + (int)port.GetVisibleRect().Top();

            const int base_state = m_bFocused ? IAlignRow::fWidgetFocused : IAlignRow::fNone;
            int i_focused = x_GetContext()->GetFocusedItemIndex();

            // create a single CGlPane for rendering all cells in all rows
            CGlPane pane(p_areas ? CGlPane::eAlwaysUpdate : CGlPane::eNeverUpdate);
            pane.EnableOffset();
            pane.SetClipRect(&rc_clip);
            pane.SetExactOrthoProjection(m_bCgiMode == false); // need this to avoid text 'popping up and down' effect on window resize
            for (int i = iFirst; i <= iLast; i++)
            {
                IAlignRow *p_row = x_GetContext()->GetRowByLine(i);
                pane.SetViewport(m_ListAreaRect);
                pane.SetVisibleRect(port.GetVisibleRect());
                pane.SetModelLimitsRect(port.GetModelLimitsRect());

                int state = base_state;
                if (x_GetContext()->IsItemSelected(i))
                    state |= IAlignRow::fItemSelected;
                if (i_focused == i)
                    state |= IAlignRow::fItemFocused;

                TVPUnit row_top = y1 - x_GetContext()->GetLinePosY(i);

                x_RenderRow(p_row, pane, state, row_top, p_areas);
            }

            pane.SetClipRect(NULL);
        }
    }
}

// renders all columns within the given Row, vp_top_y is OpenGL viewport
// coordinate of the top pixel in the row
void CAlnMultiRenderer::x_RenderRow(IAlignRow *row, CGlPane &pane, int state, TVPUnit row_top,
                                    TAreaVector *areas)
{
    string error;
    try
    {
        _ASSERT(row);

        m_RowToList[row] = true; // update "on-screen" attribute
        row->PrepareRendering(pane, row_top, state);
        row->RenderRow();
        int col_n = GetColumnsCount();
        for (int i_col = 0; i_col < col_n; i_col++)
        {
            const SColumn &col = GetColumn(i_col);
            SetupPaneForColumn(pane, i_col);
            TColumnType type = x_GetColumnType(col);
            if (col.VisibleWidth())
            {
                row->RenderColumn(type);
            }
            if (areas)
            {
                row->GetHTMLActiveAreas(type, pane, *areas);
            }
        }
    }
    catch (CException &e)
    {
        error = e.GetMsg();
    }
    catch (std::exception &e)
    {
        error = e.what();
    }
    if (!error.empty())
    {
        ERR_POST("CAlnMultiRenderer::x_RenderRow() " << error);
    }
}


void CAlnMultiRenderer::AutoFitColumns(const CGlTextureFont& font, bool for_printer)
{
    vector<string> col_text;
    int col_n = GetColumnsCount();
    col_text.resize(col_n);
    int num_rows = x_GetContext()->GetLinesCount();
    for (auto i = 0; i < num_rows; ++i) {
        IAlignRow *p_row = x_GetContext()->GetRowByLine(i);
        for (int i_col = 0; i_col < col_n; i_col++) {
            const SColumn &col = GetColumn(i_col);
            if (col.m_Visible && i_col != m_ResizableColumnIndex) {
                TColumnType col_type = x_GetColumnType(col);
                string text;
                p_row->GetColumnText(col_type, text, for_printer);
                text += "00";
                if (col_text[i_col].length() < text.length())
                    col_text[i_col] = text;
            }
        }
    }

    for (size_t i = 0; i < col_text.size(); ++i)  {
        if (col_text[i].empty())
            continue;
        TModelUnit w = ceil(font.GetMetric(IGlFont::eMetric_FullTextWidth, col_text[i].c_str()));
        SetColumnWidth(i, int(w) + 12);
    }
    UpdateColumns();
}

void CAlnMultiRenderer::SetupPaneForColumn(CGlPane &pane, int i_col) const
{
    const SColumn &col = GetColumn(i_col);
    pane.GetViewport().SetHorz(col.m_Pos, col.m_Pos + col.m_Width - 1);
}

TVPRect CAlnMultiRenderer::GetMasterArea() const
{
    return m_MasterRect;
}

int CAlnMultiRenderer::GetVPListTop() const
{
    return m_ListAreaRect.Top();
}

//TODO
int CAlnMultiRenderer::HitTest(int vp_x, int vp_y, int &col)
{
    col = GetColumnIndexByX(vp_x);
    if (col == -1)
        return eNone;
    int limit = m_rcBounds.Top();

    int shown = x_GetContext() ? x_GetContext()->GetShownElements() : IAlnMultiRendererContext::fShownElement_All;

    if (shown & IAlnMultiRendererContext::fShownElement_Header)
    {
        limit -= m_HeaderRect.Height();
        if (vp_y > limit)
            return eHeader;
    }

    if (shown & IAlnMultiRendererContext::fShownElement_Ruler)
    {
        limit -= m_RulerRect.Height();
        if (vp_y > limit)
            return eRuler;
    }

    if (shown & IAlnMultiRendererContext::fShownElement_MasterRow)
    {
        int top = limit - kMasterRowSpace;
        limit -= m_MasterRect.Height() - kMasterRowSpace;
        if (vp_y < top && vp_y > limit)
            return eMasterRow;
    }

    if (shown & IAlnMultiRendererContext::fShownElement_Alignment)
    {
        if (vp_y < m_ListAreaRect.Top() && vp_y > m_ListAreaRect.Bottom())
            return eRows;
    }

    return eNone;
}

int CAlnMultiRenderer::x_GetLineByWindowY(int WinY) const
{
    int vpY = WinY - GetListTop();
    int offset_y = (int)x_GetContext()->GetAlignPort().GetVisibleRect().Top();
    return x_GetContext()->GetLineByModelY(vpY + offset_y);
}

int CAlnMultiRenderer::x_GetLineByVPY(int vp_y) const
{
    int offset_y = (int)x_GetContext()->GetAlignPort().GetVisibleRect().Bottom();
    int model_y = offset_y - vp_y;

    return x_GetContext()->GetLineByModelY(model_y);
}

TVPRect CAlnMultiRenderer::x_GetLineRect(int Index)
{
    int Top = 0, H = 0;
    if (x_GetContext() && Index >= 0)
    {
        int offset_y = (int)x_GetContext()->GetAlignPort().GetVisibleRect().Top();
        Top = x_GetContext()->GetLinePosY(Index) - offset_y;
        H = x_GetContext()->GetLineHeight(Index);
    }
    return TVPRect(0, Top + H - 1, m_rcBounds.Width() - 1, Top);
}

void CAlnMultiRenderer::x_ResetRowListMap()
{
    NON_CONST_ITERATE(TRowToList, it, m_RowToList)
    {
        it->second = false;
    }
}

// invalidates display lists for all rows
void CAlnMultiRenderer::x_InvalidateRows(bool layout_only)
{
    const CGlPane &port = x_GetContext()->GetAlignPort();
    //    TModelRect rcV = port.GetVisibleRect();
    NON_CONST_ITERATE(TRowToList, it, m_RowToList)
    {
        IAlignRow &row = *it->first;
        row.GraphicsCacheCmd(IAlignRow::eInvalidate);
    }

    IAlignRow *master_row = x_GetContext()->GetMasterRow();
    if (master_row)
        master_row->Update(port, layout_only);

    TModelRect rcM = port.GetVisibleRect();
    int iFirst = x_GetContext()->GetLineByModelY((int)rcM.Top());
    iFirst = max(iFirst, 0);

    int iLast = x_GetContext()->GetLineByModelY((int)rcM.Bottom());
    iLast = (iLast == -1) ? x_GetContext()->GetLinesCount() - 1 : iLast;
    for (int i = iFirst; i <= iLast; i++)
    {
        IAlignRow *p_row = x_GetContext()->GetRowByLine(i);
        p_row->Update(port, layout_only);
    }
}

// delete display list for off-screen rows
void CAlnMultiRenderer::x_PurgeRowListMap()
{
    for (TRowToList::iterator it = m_RowToList.begin(); it != m_RowToList.end();)
    {
        if (!it->second)
        {
            //row is off-screen now - purge cached graphics
            (it->first)->GraphicsCacheCmd(IAlignRow::eDelete);
            TRowToList::iterator next = it;
            ++next;
            m_RowToList.erase(it);
            it = next;
        }
        else
            it++;
    }
}

const int kDefColumnWidth = 50;

CAlnMultiRenderer::SColumn::SColumn()
    : m_Pos(0),
      m_Width(kDefColumnWidth),
      m_UserData(0),
      m_Visible(true)
{
}

int CAlnMultiRenderer::GetColumnsCount() const
{
    return (int)m_Columns.size();
}

const CAlnMultiRenderer::SColumn &CAlnMultiRenderer::GetColumn(int index) const
{
    _ASSERT(index >= 0 && index < (int)m_Columns.size());
    return m_Columns[index];
}

CAlnMultiRenderer::SColumn &CAlnMultiRenderer::GetColumn(int index)
{
    _ASSERT(index >= 0 && index < (int)m_Columns.size());
    return m_Columns[index];
}

int CAlnMultiRenderer::AddColumn()
{
    return AddColumn(kDefColumnWidth, "", 0);
}

int CAlnMultiRenderer::AddColumn(int width, const string &label, int data)
{
    int index = GetColumnsCount();
    return InsertColumn(index, width, label, data);
}

int CAlnMultiRenderer::InsertColumn(int index, int width,
                                    const string &label, int data)
{
    _ASSERT(index >= 0 && index <= (int)m_Columns.size());

    SColumn col;
    col.m_Width = width;
    col.m_Name = label;
    col.m_UserData = data;
    col.m_SortState = IAlnExplorer::eNotSupported;

    TColumnVector::const_iterator it =
        m_Columns.insert(m_Columns.begin() + index, col);
    int i = int(it - m_Columns.begin());

    // update m_ResizableColumnIndex
    if (m_ResizableColumnIndex >= 0 && m_ResizableColumnIndex < (int)m_Columns.size())
    {
        if (m_ResizableColumnIndex >= i)
        {
            m_ResizableColumnIndex++;
        }
    }
    else
    {
        m_ResizableColumnIndex = i;
    }
    return i;
}

void CAlnMultiRenderer::SetResizableColumn(int index)
{
    if (index >= 0 && index < (int)m_Columns.size())
    {
        m_ResizableColumnIndex = index;
    }
    else
    {
        _ASSERT(false);
    }
}

void CAlnMultiRenderer::SetColumns(const vector<SColumn> &columns, int resizable_index)
{
    m_Columns = columns;
    SetResizableColumn(resizable_index);

    x_LayoutColumns();
    x_Layout();
}

void CAlnMultiRenderer::SetColumnWidth(int index, int width)
{
    width = max(width, 0);

    m_Columns[index].m_Width = width;
}

void CAlnMultiRenderer::SetColumnPos(int index, int pos)
{
    m_Columns[index].m_Pos = pos;
}

void CAlnMultiRenderer::SetColumnUserData(int index, int data)
{
    _ASSERT(index >= 0 && index < (int)m_Columns.size());
    m_Columns[index].m_UserData = data;
}

void CAlnMultiRenderer::SetColumnVisible(int index, bool b_visible)
{
    _ASSERT(index >= 0 && index < (int)m_Columns.size());

    SColumn &col = m_Columns[index];
    if (col.m_Visible != b_visible)
    {
        col.m_Visible = b_visible;
        int delta = b_visible ? col.m_Width : -col.m_Width;
        if (col.m_Visible && delta == 0)
        {
            delta = col.m_Width = kDefColumnWidth;
        }
        for (int i = index + 1; delta != 0 && i < (int)m_Columns.size(); i++)
        {
            m_Columns[i].m_Pos += delta;
        }
    }

    Resize(m_rcBounds);
}


bool CAlnMultiRenderer::IsColumnVisible(int index) const
{
    _ASSERT(index >= 0 && index < (int)m_Columns.size());

    return m_Columns[index].m_Visible;
}


void CAlnMultiRenderer::SetColumnsByStyle(CWidgetDisplayStyle &style)
{
    // get the name of the resizable column, we will use it to update
    // resizable column index after rearranging is completed
    size_t rs_index = GetResizableColumnIndex();
    _ASSERT(rs_index < m_Columns.size());
    string rs_name = m_Columns[rs_index].m_Name;

    vector<SColumn> new_columns;

    // iterate by columns in the style and place them first
    ITERATE(CWidgetDisplayStyle::TColumns, it, style.m_Columns)
    {
        const CWidgetDisplayStyle::SColumn &col = *it;

        for (size_t i = 0; i < m_Columns.size(); i++)
        {
            if (m_Columns[i].m_Name == col.m_Name)
            {
                // update the column and place into the new container
                m_Columns[i].m_Width = col.m_Width;
                m_Columns[i].m_Visible = col.m_Visible;
                new_columns.push_back(m_Columns[i]);
                // delete it from m_Columns
                m_Columns.erase(m_Columns.begin() + i);
            }
        }
    }

    // now in m_Columns we have onl columns not recorded in the style
    // add them to the end without changes
    new_columns.insert(new_columns.end(), m_Columns.begin(), m_Columns.end());

    m_Columns = new_columns;

    // update rs_index
    for (size_t j = 0; j < m_Columns.size(); j++)
    {
        if (m_Columns[j].m_Name == rs_name)
        {
            rs_index = j;
            break;
        }
    }

    // finish rearranging
    SetResizableColumn((int)rs_index);

    Resize(m_rcBounds);
}

/*
void CAlnMultiRenderer::SetHiddenColumns(const vector<string>& names)
{
    // get the name of the resizable column, we will use it to update
    // resizable column index after rearranging is completed
    int rs_index = GetResizableColumnIndex();
    _ASSERT(rs_index >= 0  &&  rs_index < (int) m_Columns.size());
    string rs_name = m_Columns[rs_index].m_Name;

    rs_index = -1;

    // reset Visible flag
    NON_CONST_ITERATE(TColumnVector, it, m_Columns)  {
        it->m_Visible = true;
    }

    vector<SColumn> hid_columns;

    // iterate by hidden columns
    for( size_t i = 0;  i < names.size(); i++ )    {
        TColumnVector::iterator it = m_Columns.begin();
        for(; it != m_Columns.end();  ++it)  {
            if(it->m_Name == names[i])
                break;
        }

        if(it != m_Columns.end())   {
            it->m_Visible = false;
            hid_columns.push_back(*it);
            m_Columns.erase(it);
        }
    }

    m_Columns.insert(m_Columns.end(), hid_columns.begin(), hid_columns.end());

    // update rs_index
    for( size_t j = 0;  j < m_Columns.size();  j++ )  {
        if(m_Columns[j].m_Name == rs_name)   {
            rs_index = j;
            break;
        }
    }

    // finish rearranging
    SetResizableColumn(rs_index);

    Resize(m_rcBounds);
}
*/

void CAlnMultiRenderer::GetVisibleColumns(vector<string> &names)
{
    names.clear();
    ITERATE(TColumnVector, it, m_Columns)
    {
        if (it->m_Visible)
        {
            names.push_back(it->m_Name);
        }
    }
}

int CAlnMultiRenderer::GetColumnIndexByX(int x) const
{
    ITERATE(TColumnVector, it, m_Columns)
    {
        if (it->m_Visible && x >= it->m_Pos && x < it->m_Pos + it->m_Width)
            return int(it - m_Columns.begin());
    }
    return -1;
}

int CAlnMultiRenderer::GetResizableColumnIndex() const
{
    return m_ResizableColumnIndex;
}

void CAlnMultiRenderer::UpdateColumns()
{
    Resize(m_rcBounds);
}

void CAlnMultiRenderer::x_LayoutColumns()
{
    int w = m_rcBounds.Width();
    int n_col = (int)m_Columns.size();

    if (w > 0 && n_col > 0)
    { // resize all columns
        int total_w = 0;
        for (int j = 0; j < n_col; j++)
        {
            SColumn &col = m_Columns[j];
            if (col.m_Visible && j != m_ResizableColumnIndex)
            {
                total_w += col.m_Width;
            }
        }

        int pos = 0;
        for (int j = 0; j < n_col; j++)
        {
            SColumn &col = m_Columns[j];
            if (col.m_Visible)
            {
                col.m_Pos = pos;
                if (j == m_ResizableColumnIndex)
                {
                    int w_rest = w - total_w;
                    col.m_Width = w_rest > 0 ? w_rest : 0;
                }
                pos += col.m_Width;
            }
            _ASSERT(col.m_Width >= 0);
        }
    }
}

END_NCBI_SCOPE
