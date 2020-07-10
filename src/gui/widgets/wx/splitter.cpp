/*  $Id: splitter.cpp 23985 2011-06-30 17:59:06Z kuznets $
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


#include <gui/widgets/wx/splitter.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>

#include <algorithm>

#include <wx/dcclient.h>
#include <wx/event.h>
#include <wx/settings.h>

#ifdef NCBI_OS_DARWIN
#endif

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///  CSplitter

#ifdef NCBI_OS_DARWIN
    const static int kDefSepSize = 8; // bigger splitter for Mac OS X
#else
    const static int kDefSepSize = 5;
#endif

const static int kMaxCells = 64;

BEGIN_EVENT_TABLE(CSplitter, CSplitter::TParent)
    EVT_PAINT(CSplitter::OnPaint)
    //EVT_SIZE(CSplitter::OnSize)
    EVT_LEFT_DOWN(CSplitter::OnLeftDown)
    EVT_LEFT_DCLICK(CSplitter::OnLeftDown)
    EVT_LEFT_UP(CSplitter::OnLeftUp)
    EVT_MOTION(CSplitter::OnMouseMove)
    EVT_CONTEXT_MENU(CSplitter::OnContextMenu)
    EVT_ENTER_WINDOW(CSplitter::OnMouseEnter)
    EVT_LEAVE_WINDOW(CSplitter::OnMouseLeave)
    EVT_KEY_DOWN(CSplitter::OnKeyDown)
END_EVENT_TABLE()


CSplitter::CSplitter()
:   TParent(),
    m_Type(eVertical)
{
    x_Init();
}

CSplitter::CSplitter(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                         const wxSize& size, long style, const wxString& name)
:   wxPanel(parent, id, pos, size, style, name)
{
    x_Init();
}


CSplitter::CSplitter(wxWindow* parent, wxWindowID id, ESplitType type)
:   TParent(parent, id),
    m_Type(type)
{
    x_Init();
}


void CSplitter::x_Init()
{
    m_BlockLayout = false;
    m_LayoutedSize = wxSize(-1, -1);
    m_SplitSize = wxSize(-1, -1);

    m_SepSize = kDefSepSize;

    m_ResizableColumn = m_ResizableRow = -1;

    m_PrevFocus = NULL;
    m_iDragSepX = -1;
    m_iDragSepY = -1;
    m_MinSepPosX = -1;
    m_MaxSepPosX = -1;
    m_MinSepPosY = -1;
    m_MaxSepPosY = -1;
    m_HighlightCell = -1;

    m_Cells.resize(1, NULL); // by default it has a single cell
    m_vNormSizeX.resize(1, -1);
    m_vNormSizeY.resize(1, -1);

    //SetBackgroundColour(wxColour(255, 255, 255));
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    SetAutoLayout(true);
}


CSplitter::~CSplitter()
{
}


CSplitter::ESplitType CSplitter::GetSplitType()    const
{
    return m_Type;
}


int CSplitter::GetSeparatorSize() const
{
    return m_SepSize;
}


void CSplitter::Split(ESplitType type)
{
    if (m_Type != type) {
        x_Clear();
        m_Type = type;
    }
}

void CSplitter::Split(ESplitType type, const TPosVector& sizes)
{
    x_Clear();

    switch(type) {
    case eVertical:
        x_Split(type, TPosVector(), sizes);
        break;
    case eHorizontal:
        x_Split(type, sizes, TPosVector());
        break;
    case eGrid:
        x_Split(type, sizes, sizes);
        break;
    };
}


void CSplitter::Split(ESplitType type, const int ar_size[])
{
    TPosVector sizes;
    int i = 0;
    for(; i < kMaxCells; i++ ) {
        if (ar_size[i] >= 0)
            sizes.push_back(ar_size[i]);
        else break;
    }
    _ASSERT(i < kMaxCells);

    Split(type, sizes);
}


void CSplitter::Split(ESplitType type, const TPosVector& widths, const TPosVector& heights)
{
    x_Split(type, widths, heights);
}


void CSplitter::Split(ESplitType type, const int ar_w[], const int ar_h[])
{
    TPosVector widths;
    int i = 0;
    for(; i < kMaxCells; i++ ) {
        if (ar_w[i] >= 0)
            widths.push_back(ar_w[i]);
        else break;
    }
    _ASSERT(i < kMaxCells);

    TPosVector heights;
    for( i = 0; i < kMaxCells; i++ ) {
        if (ar_h[i] >= 0)
            heights.push_back(ar_h[i]);
        else break;
    }
    _ASSERT(i < kMaxCells);

    x_Split(type, widths, heights);
}


void CSplitter::Split(int col_n, int row_n)
{
    vector<int> widths;
    vector<int> heights;

    col_n = max(1, col_n);
    row_n = max(1, row_n);

    for( int i = 0; i < col_n; i++)
        widths.push_back(10);
    for( int i = 0; i < row_n; i++)
        heights.push_back(10);

    ESplitType type;
    if(col_n <= 1)  {
        type = (row_n <= 1) ? m_Type : eHorizontal;
    } else {
        type = (row_n <= 1) ? eVertical : eGrid;
    }

    x_Split(type, widths, heights);

    x_DistributeEvenly(true, true);
}


void CSplitter::x_Split(ESplitType type, const TPosVector& widths, const TPosVector& heights)
{
    m_Type = type;
    int n_w = (int) widths.size();
    int n_h = (int) heights.size();

    if (n_w == 0)    n_w++;
    if (n_h == 0)    n_h++;

    _ASSERT(n_w < kMaxCells  &&  n_h < kMaxCells); // keep 'em reasonable

    x_Clear();

    int pos = 0;
    for( int i = 0; i < n_w - 1; i++) {
        pos += widths[i];
        m_vSplitPosX.push_back(pos);
        pos += m_SepSize;
    }
    m_vNormSizeX.resize(n_w, -1);
    m_SplitSize.x = widths.empty() ? pos : pos + widths.back();

    pos = 0;
    for( int i = 0; i < n_h - 1; i++) {
        pos += heights[i];
        m_vSplitPosY.push_back(pos);
        pos += m_SepSize;
    }
    m_vNormSizeY.resize(n_h, -1);
    m_SplitSize.y = heights.empty() ? pos : pos + heights.back();

    int n_cells = GetColumnsCount() * GetRowsCount();
    m_Cells.resize(n_cells, NULL);
}


int CSplitter::GetColumnsCount() const
{
    return (int) m_vSplitPosX.size() + 1;
}


int CSplitter::GetRowsCount() const
{
    return (int) m_vSplitPosY.size() + 1;
}


void CSplitter::GetWidths(TPosVector& widths) const
{
    x_SplitPositionsToSizes(m_vSplitPosX, GetSize().x, widths);
}


void CSplitter::SetWidths(const TPosVector& widths)
{
    size_t n_w = widths.size();
    if (n_w == m_vSplitPosX.size() + 1)    {
        m_vSplitPosX.clear();

        int pos = 0;
        for( size_t i = 0;  i < n_w - 1;  i++) {
            pos += widths[i];
            m_vSplitPosX.push_back(pos);
            pos += m_SepSize;
        }
        m_vNormSizeX.resize(n_w, -1); // reset
        m_SplitSize.x = pos + widths.back();

        x_ResizeAllCells();
        x_RequestUpdate();
    }
    // else _ASSERT(false);
}


// is set, this cell will resize first trying to "consume" any changes in availble space
    // caused by the splitter resizing or separator dragging
void CSplitter::SetResizableCell(int col, int row)
{
    if(col >= -1  &&  col < GetColumnsCount())  {
        m_ResizableColumn = col;
    } else {
        m_ResizableColumn = -1;
        _ASSERT(false);
    }
    if(row >= -1  &&  row < GetRowsCount())  {
        m_ResizableRow = row;
    } else {
        m_ResizableRow = -1;
        _ASSERT(false);
    }
}


void CSplitter::GetHeights(TPosVector& heights) const
{
    x_SplitPositionsToSizes(m_vSplitPosY, GetSize().y, heights);
}


void CSplitter::SetHeights(const TPosVector& heights)
{
    size_t n_h = heights.size();
    if(n_h == m_vSplitPosY.size() + 1)   {
        m_vSplitPosY.clear();

        int pos = 0;
        for( size_t i = 0;  i < n_h - 1;  i++) {
            pos += heights[i];
            m_vSplitPosY.push_back(pos);
            pos += m_SepSize;
        }
        m_vNormSizeY.resize(n_h, -1); // reset

        m_SplitSize.y = pos + heights.back();

        x_ResizeAllCells();
        x_RequestUpdate();
    }
    //else _ASSERT(false);
}


bool CSplitter::IsValidCell(int i_x, int i_y) const
{
    return i_x >= 0  &&  i_x <= (int) m_vSplitPosX.size()
            &&  i_y >= 0  &&  i_y <= (int) m_vSplitPosY.size();
}


bool CSplitter::InsertToCell(wxWindow* w, int i_x, int i_y)
{
    int i_cell = x_GetCellIndex(i_x, i_y);
    if (w && i_cell >= 0) {
        if (m_Cells[i_cell] == NULL) {// cell is  vacant
            w->Hide();

            w->Reparent(this);
            m_Cells[i_cell] = w;
            x_ResizeToCell(i_x, i_y);

            w->Show();
            return true;
        }
    } else {
        _ASSERT(false);
        ERR_POST("CSplitter::InsertToCell() - cell index is out of range");
    }
    return false;
}

void CSplitter::AddRow()
{
    if(m_Type == eHorizontal)   {
        m_vSplitPosY.push_back(m_vSplitPosY.empty() ? 0 : m_vSplitPosY.back());
        m_vNormSizeY.push_back(-1);
        int n_cells = GetColumnsCount() * GetRowsCount();
        m_Cells.resize(n_cells, NULL);
    } else {
        _ASSERT(false);
    }
}

void CSplitter::AddColumn()
{
    if(m_Type == eVertical) {
        m_vSplitPosX.push_back(m_vSplitPosX.empty() ? 0 : m_vSplitPosX.back());
        m_vNormSizeX.push_back(-1);
        int n_cells = GetColumnsCount() * GetRowsCount();
        m_Cells.resize(n_cells, NULL);
    } else {
        _ASSERT(false);
    }
}


bool CSplitter::RemoveFromCell(int i_x, int i_y)
{
    int i_cell = x_GetCellIndex(i_x, i_y);
    return x_RemoveChild(i_cell);
}


void CSplitter::RemoveChild(wxWindowBase* child)
{
    TCells::iterator it = std::find(m_Cells.begin(), m_Cells.end(), child);
    if (it != m_Cells.end()) {
        int index = int(it - m_Cells.begin());
        x_RemoveChild(index);
    } else _ASSERT(false);
}


static bool s_Contains(wxWindow* parent, wxWindow* child)
{
    _ASSERT(parent);
    wxWindow* w = child, *p = 0;
    while(w)    {
        p = w->GetParent();
        if(parent == p)
            return true;
        w = p;
    }
    return false;
}


bool CSplitter::x_RemoveChild(int index)
{
    if(index >= 0  && index < (int) m_Cells.size())  {
        wxWindow* w = m_Cells[index];
        if(w)   {
            wxWindow* w_focus = FindFocus();
            bool keep_focus = (w == w_focus)  || s_Contains(w, w_focus);

            // remove the child
            m_Cells[index] = NULL;
            TParent::RemoveChild(w);

            //restore focus - move to the next child
            if(keep_focus)  {
                wxWindow* new_focus = NULL;
                for( int i = index - 1;  i >= 0; i--)    {
                    if(m_Cells[i] != NULL) {
                        new_focus = m_Cells[i];
                        break;
                    }
                }
                if(! new_focus) {
                    for( size_t i = index + 1;  i < m_Cells.size(); i++)    {
                        if(m_Cells[i] != NULL) {
                            new_focus = m_Cells[i];
                            break;
                        }
                    }
                }
                if(new_focus)   {
                    //new_focus->SetFocus();
                }
            }
            return true;
        }
    }
    return false;
}


void CSplitter::RemoveAll()
{
    for( size_t i = 0; i < m_Cells.size(); i++ ) {
        wxWindow* w = m_Cells[i];
        if (w) {
            m_Cells[i] = NULL;
            TParent::RemoveChild(w);
        }
    }
    x_RequestUpdate();
}


bool CSplitter::FindChild(const wxWindow* w, int& i_x, int& i_y)   const
{
    i_x = i_y = -1;
    if (w) {
        for( int i = 0; i < (int) m_Cells.size();  i++ ) {
            if (m_Cells[i] == w) {
                i_x = x_GetColumn(i);
                i_y = x_GetRow(i);
                return true;
            }
        }
    }
    return false;
}



void CSplitter::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    
    wxBrush brush(GetBackgroundColour());   
    
    int n_cols = GetColumnsCount();
    int n_rows = GetRowsCount();

    for( int i_x = 0;  i_x < n_cols;  i_x++ ) {
        int left = x_GetLeft(i_x);
        int right = x_GetRight(i_x);

        for( int i_y = 0;  i_y < n_rows;  i_y++ ) {
            int top = x_GetTop(i_y);
            int bottom = x_GetBottom(i_y);

            // empty area
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(brush);
            dc.DrawRectangle(left, top, right - left + 1, bottom - top + 1);            

            // horizontal segment
            if(i_y != n_rows - 1)   {
                x_DrawSeparator(dc, left, bottom + 1, right - left + 1, m_SepSize);
            }
            // vertical segment
            if(i_x != n_cols - 1)   {
                x_DrawSeparator(dc, right + 1, top, m_SepSize, bottom - top + 1);
            }
            // intersection
            if(i_y != n_rows - 1  &&  i_x != n_cols - 1)   {
                x_DrawSeparator(dc, right + 1, bottom + 1, m_SepSize, m_SepSize);
            }
        }
    }
}


void CSplitter::x_DrawSeparator(wxDC& dc, int x, int y, int w, int h)
{   
    wxColour cl_back = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    wxColour cl_line = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);

    wxPen pen(cl_line);
    wxBrush brush(cl_back);

    dc.SetPen(pen);
    dc.SetBrush(brush);
    dc.DrawRectangle(x, y, w, h);

#ifdef __WXMAC__THIS_CODE_NEEDS_TO_BE_CHANGED_EVERYTIME_OSX_UPGRADED
    /// draw a Mac-style round "dot" in the middle of the splitter
    int yy = GetClientSize().y - y + h + 2; // center splitter notch
    HIRect rect = CGRectMake(x, yy, w, h);

    HIThemeSplitterDrawInfo info;
    info.state = kThemeStateActive;// : kThemeStateInactive;
    info.adornment = kThemeAdornmentNone;

    WindowRef wr = WindowRef(this->MacGetTopLevelWindowRef());
    CGContextRef context;
    QDBeginCGContext (GetWindowPort(wr), &context);
    HIThemeDrawPaneSplitter(&rect, &info, context, kHIThemeOrientationInverted);
    QDEndCGContext(GetWindowPort(wr), &context);
#endif
}


bool CSplitter::Layout()
{
    if(IsShown()  &&  ! m_BlockLayout)   {
        wxSize new_size = GetSize();
        bool x_changed = (m_Type == eVertical  ||  m_Type == eGrid)
            &&  (m_SplitSize.x != -1)  && (m_SplitSize.x != new_size.x);
        bool y_changed = (m_Type == eHorizontal  ||  m_Type == eGrid)
            &&  (m_SplitSize.y != -1)  &&  (m_SplitSize.y != new_size.y);
        bool split_changed = x_changed  ||  y_changed;

        bool size_changed = (m_LayoutedSize != wxSize(-1, -1))  &&  (m_LayoutedSize != new_size);

        wxSize old_size = split_changed ? m_SplitSize : m_LayoutedSize;
        x_Resize(old_size, new_size, split_changed | size_changed);

        m_LayoutedSize = new_size;
        m_SplitSize = wxSize(-1, -1);
    }
    return false;
}


bool CSplitter::Show(bool show)
{
    if(show)    {
        Layout();
    }
    return TParent::Show(show);
}


void CSplitter::BlockLayout(bool block)
{
    if(m_BlockLayout != block)  {
        m_BlockLayout = block;
        if( ! m_BlockLayout)    {
            // unlocking - force layout
            Layout();
        }
    }
}


void CSplitter::x_Resize(const wxSize& old_size, const wxSize& new_size, bool b_size_changed)
{
    //LOG_POST("\nCwxSplitter::x_Resize()  "  << this << ", new_w " << new_w << ", new_h " << new_h
    //    << ", size_changed " << b_size_changed);LOG_POST("   " << ToString(ncbi::GetScreenRect(*this)));
    if (b_size_changed) {
        x_CalculateMinSizes(m_MinWidths, m_MinHeights);
        TPosVector sizes;

        x_SplitPositionsToSizes(m_vSplitPosX, old_size.x, sizes);
        x_DoResize(sizes, m_vNormSizeX, m_MinWidths, old_size.x, new_size.x, m_ResizableColumn); // horizontal
        x_SizesToSplitPositions(sizes, m_vSplitPosX);

        x_SplitPositionsToSizes(m_vSplitPosY, old_size.y, sizes);
        x_DoResize(sizes, m_vNormSizeY, m_MinHeights, old_size.y, new_size.y, m_ResizableRow); // vertical
        x_SizesToSplitPositions(sizes, m_vSplitPosY);
    }

    //if (b_size_changed) {
        x_ResizeAllCells();
        x_RequestUpdate();
    //}
}


wxWindow* CSplitter::GetCell(int col, int row)
{
    bool b_valid = IsValidCell(col, row);
    _ASSERT(b_valid);
    if (b_valid) {
        int index = x_GetCellIndex(col, row);
        return m_Cells[index];
    }
    return NULL;
}


const wxWindow* CSplitter::GetCell(int col, int row) const
{
    bool b_valid = IsValidCell(col, row);
    _ASSERT(b_valid);
    if (b_valid) {
        int index = x_GetCellIndex(col, row);
        return m_Cells[index];
    }
    return NULL;
}


int CSplitter::x_GetCellIndex(int col, int row) const
{
    bool b_valid = IsValidCell(col, row);
    _ASSERT(b_valid);
    return b_valid ? (col + row * GetColumnsCount()) : -1;
}


int CSplitter::x_GetColumn(int i_cell) const
{
    _ASSERT(i_cell >= 0  &&  i_cell < (int) m_Cells.size());
    int n_cols = (int) m_vSplitPosX.size() + 1;
    return i_cell % n_cols;
}


int CSplitter::x_GetRow(int i_cell) const
{
    _ASSERT(i_cell >= 0  &&  i_cell < (int) m_Cells.size());
    int n_cols = (int) m_vSplitPosX.size() + 1;
    return i_cell / n_cols;
}


int CSplitter::x_GetCellIndex(wxWindow* widget) const
{
    for( size_t i =0 ; i < m_Cells.size(); i++ )   {
        if(m_Cells[i] == widget)   {
            return (int) i;
        }
    }
    return -1;
}


int CSplitter::x_GetLeft(int i_x)     const
{
    _ASSERT(i_x >= 0  &&  i_x <= (int) m_vSplitPosX.size());
    int loc_x = (i_x == 0) ? 0 : (m_vSplitPosX[i_x - 1] + m_SepSize);
    return loc_x;
}


int CSplitter::x_GetRight(int i_x)  const
{
    _ASSERT(i_x >= 0  &&  i_x <= (int) m_vSplitPosX.size());
    int loc_next =  (i_x == (int) m_vSplitPosX.size()) ? GetSize().x : m_vSplitPosX[i_x];
    return loc_next - 1;
}


int CSplitter::x_GetWidth(int i_x)  const
{
    _ASSERT(i_x >= 0  &&  i_x <= (int) m_vSplitPosX.size());
    int top = (i_x == 0) ? 0 : m_vSplitPosX[i_x - 1] + m_SepSize;
    int next =  (i_x == (int) m_vSplitPosX.size()) ? GetSize().x : m_vSplitPosX[i_x];
    return next - top;
}


int CSplitter::x_GetTop(int i_y)     const
{
    _ASSERT(i_y >= 0  &&  i_y <= (int) m_vSplitPosY.size());
    int loc_y = (i_y == 0) ? 0 : (m_vSplitPosY[i_y - 1] + m_SepSize);
    return loc_y;
}


int CSplitter::x_GetBottom(int i_y)  const
{
    _ASSERT(i_y >= 0  &&  i_y <= (int) m_vSplitPosY.size());
    int loc_next =  (i_y == (int) m_vSplitPosY.size()) ? GetSize().y : m_vSplitPosY[i_y];
    return loc_next - 1;
}


int CSplitter::x_GetHeight(int i_y)  const
{
    _ASSERT(i_y >= 0  &&  i_y <= (int) m_vSplitPosY.size());
    int top = (i_y == 0) ? 0 : m_vSplitPosY[i_y - 1] + m_SepSize;
    int next =  (i_y == (int) m_vSplitPosY.size()) ? GetSize().y : m_vSplitPosY[i_y];
    return next - top;
}


// removes all child widgets and separators
void CSplitter::x_Clear()
{
    wxWindowList& children = GetChildren();
    // make a copy to avoid side effects
    vector<wxWindow*> v_children;
    for( wxWindowList::iterator it = children.begin(); it != children.end();  ++it) {
        wxWindow* child = *it;
        v_children.push_back(child);        
    }
    for( size_t i = 0;  i < v_children.size();  i++ )   {
        wxWindow* child = v_children[i];
        TParent::RemoveChild(child);
    }

    m_vSplitPosX.clear();
    m_vSplitPosY.clear();

    m_Cells.resize(1, NULL);

    m_vNormSizeX.clear();
    m_vNormSizeX.push_back(-1);

    m_vNormSizeY.clear();
    m_vNormSizeY.push_back(-1);
}


void CSplitter::x_ResizeToCell(int i_cell)
{
    int i_x = x_GetColumn(i_cell);
    int i_y = x_GetRow(i_cell);
    x_ResizeToCell(i_x, i_y);
}


void CSplitter::x_ResizeToCell(int i_x, int i_y)
{
    wxWindow* w = GetCell(i_x, i_y);
    _ASSERT(w == NULL  ||  w->GetParent() == this);
    if (w) {
        int left = x_GetLeft(i_x);
        int width = x_GetWidth(i_x);
        int top = x_GetTop(i_y);
        int height = x_GetHeight(i_y);

        //LOG_POST("CSplitter::x_ResizeToCell(" << i_x << ", " << i_y << "), w  " << width  << ", h " << height);

        w->SetSize(left, top, width, height);
        w->Refresh();
    }
}


void CSplitter::x_ResizeAllCells()
{
    for(int i = 0; i < (int) m_Cells.size(); i++ ) {
        if (m_Cells[i]) {
                x_ResizeToCell(i);
            }
        }
}


void CSplitter::OnContextMenu(wxContextMenuEvent& event)
{
}


void CSplitter::OnLeftDown(wxMouseEvent& event)
{
    event.Skip();

    m_MouseDownX = event.GetX();
    m_MouseDownY = event.GetY();

    int i_sep_x = x_HitTestSeparator(m_MouseDownX/* - x()*/, m_vSplitPosX);
    int i_sep_y = x_HitTestSeparator(m_MouseDownY/* - y()*/, m_vSplitPosY);

    bool b_hit_x_sep = (i_sep_x != -1);
    bool b_hit_y_sep = (i_sep_y != -1);

    if (b_hit_x_sep  ||  b_hit_y_sep) {
        if (event.LeftDClick()) {
            x_DistributeEvenly(b_hit_x_sep, b_hit_y_sep);
        } else {
            x_StartDrag(i_sep_x, i_sep_y);
            m_PrevSepPos.x = (i_sep_x == -1) ? -1 : m_vSplitPosX[i_sep_x];
            m_PrevSepPos.y = (i_sep_y == -1) ? -1 : m_vSplitPosY[i_sep_y];
        }
    }
}


void CSplitter::OnMouseMove(wxMouseEvent& event)
{
    if(event.Dragging()) {
        if(event.LeftIsDown()  &&  x_IsDragging())  {
            x_DoDragSeparator(event, false);
        }
    } else {
        // handle mouse hovering
        int i_sep_x = x_HitTestSeparator(event.GetX(), m_vSplitPosX);
        int i_sep_y = x_HitTestSeparator(event.GetY(), m_vSplitPosY);

        wxCursor cursor;
        if (i_sep_x == -1) {
            cursor = (i_sep_y == -1) ? wxCURSOR_ARROW : wxCURSOR_SIZENS;
        } else {
            cursor = (i_sep_y == -1) ? wxCURSOR_SIZEWE : wxCURSOR_SIZING ;
        }
        SetCursor(cursor);
    }
}


void CSplitter::OnLeftUp(wxMouseEvent& event)
{
    if (x_IsDragging()) { // if dragging - do not share events with children
        x_DoDragSeparator(event, true);
        x_EndDrag();        
    }
}


void CSplitter::OnMouseEnter( wxMouseEvent& WXUNUSED(event) )
{
}


void CSplitter::OnMouseLeave( wxMouseEvent& WXUNUSED(event) )
{
    SetCursor(wxCURSOR_ARROW);
}


void CSplitter::OnKeyDown(wxKeyEvent& event)
{
    if(x_IsDragging())  {
        if(event.GetKeyCode() == WXK_ESCAPE)    {
            x_EndDrag();
        }
    }
}


void CSplitter::x_RequestUpdate()
{
    Refresh();
    //Update(); // comment this line to defer repainting
}


inline int  ElementsSum(vector<int> v, int start_i, int end_i)
{
    _ASSERT(start_i >=0  &&  end_i < (int) v.size());
    int sum = 0;
    for( int i = start_i; i <= end_i;  i++ )    {
        sum += v[i];
    }
    return sum;
}


void CSplitter::x_StartDrag(int i_sep_x, int i_sep_y)
{
    CaptureMouse(); //TODO - handle capture lost

    //save and capture keyboard focus
    m_PrevFocus = FindFocus();
    
    m_iDragSepX = i_sep_x;
    m_iDragSepY = i_sep_y;

    // remeber split positions
    m_StartPosX = (m_iDragSepX == -1) ? -1 : m_vSplitPosX[m_iDragSepX];
    m_StartPosY = (m_iDragSepY == -1) ? -1 : m_vSplitPosY[m_iDragSepY];

    // get minimal sizes by X and Y
    x_CalculateMinSizes(m_MinWidths, m_MinHeights);

    int col_n = GetColumnsCount();
    int row_n = GetRowsCount();

    // calculate constraints
    m_MinSepPosX = m_iDragSepX * m_SepSize;
    m_MinSepPosX += ElementsSum(m_MinWidths, 0, m_iDragSepX);

    int reserved = (col_n - m_iDragSepX - 1) * m_SepSize;
    reserved += ElementsSum(m_MinWidths, m_iDragSepX + 1, col_n - 1);
    m_MaxSepPosX = GetSize().x - reserved;

    m_MinSepPosY = m_iDragSepY * m_SepSize;
    m_MinSepPosY += ElementsSum(m_MinHeights, 0, m_iDragSepY);

    reserved = (row_n - m_iDragSepY - 1) * m_SepSize;
    reserved += ElementsSum(m_MinHeights, m_iDragSepY + 1, row_n - 1);
    m_MaxSepPosY = GetSize().y - reserved;

    x_PreserveNormalSize(true);
}


void CSplitter::x_DoDragSeparator(wxMouseEvent& event, bool b_final)
{
    wxSize size = GetSize();

    if (m_iDragSepX != -1) {
        int shift = event.m_x - m_MouseDownX;
        int new_pos = m_StartPosX + shift;
        new_pos = min(new_pos, m_MaxSepPosX);
        new_pos = max(new_pos, m_MinSepPosX);
        int delta = new_pos - m_PrevSepPos.x;
        TPosVector sizes;

        x_SplitPositionsToSizes(m_vSplitPosX, size.x, sizes);
        x_MoveSeparator(sizes, m_vNormSizeX, m_MinWidths, m_iDragSepX, delta, m_ResizableColumn); // horizontal
        x_SizesToSplitPositions(sizes, m_vSplitPosX);
        m_PrevSepPos.x = m_vSplitPosX[m_iDragSepX];

        if (b_final) {
            m_vNormSizeX[m_iDragSepX] = -1;
            m_vNormSizeX[m_iDragSepX + 1] = -1;
        }
    }
    if (m_iDragSepY != -1) {
        int shift = event.m_y - m_MouseDownY;
        int new_pos = m_StartPosY + shift;
        new_pos = min(new_pos, m_MaxSepPosY);
        new_pos = max(new_pos, m_MinSepPosY);
        int delta = new_pos - m_PrevSepPos.y;
        TPosVector sizes;

        x_SplitPositionsToSizes(m_vSplitPosY, size.y, sizes);
        x_MoveSeparator(sizes, m_vNormSizeY, m_MinHeights, m_iDragSepY, delta, m_ResizableRow); // vertical
        x_SizesToSplitPositions(sizes, m_vSplitPosY);
        m_PrevSepPos.y = m_vSplitPosY[m_iDragSepY];

        /*int delta = event.GetPosition().y - m_PrevDragPos.y;
        TPosVector sizes;

        x_SplitPositionsToSizes(m_vSplitPosY, size.y, sizes);
        x_MoveSeparator(sizes, m_vNormSizeY, m_MinHeights, m_iDragSepY, delta, m_ResizableRow); // horizontal
        x_SizesToSplitPositions(sizes, m_vSplitPosY);

        int shift = event.m_y - m_MouseDownY; // cumulative shift by Y
        int new_pos = m_StartPosY + shift;
        x_MoveSeparator(m_iDragSepY, new_pos, m_vSplitPosY, m_MinHeights,
                        m_MinSepPosY, m_MaxSepPosY, GetSize().y, start_y, stop_y);
        */
        if (b_final) {
            m_vNormSizeY[m_iDragSepY] = -1;
            m_vNormSizeY[m_iDragSepY + 1] = -1;
        }
    }

    x_UpdateRegion(0, GetColumnsCount()-1, 0, GetRowsCount()-1);
    
    if(b_final) {
        x_PreserveNormalSize(false);
    }
}


void CSplitter::x_EndDrag()
{
    SetCursor(wxCURSOR_ARROW);
    
    ReleaseMouse();
    
    // restore saved focus
    if(m_PrevFocus) {
        m_PrevFocus->SetFocus();
    }

    m_iDragSepX = m_iDragSepY = -1;
}


bool CSplitter::x_IsDragging() const
{
    return m_iDragSepX != -1    ||  m_iDragSepY != -1;
}


void CSplitter::x_PreserveNormalSize(bool preserve)
{
    // do not do anything
}


// z is local coordinate in the dimension corresponding to vpos
int CSplitter::x_HitTestSeparator(int z, TPosVector& vpos)
{
    int n = (int) vpos.size();
    for( int i = 0; i < n; i++  ) {
        int pos = vpos[i];
        if (z >= pos  &&  z < pos + m_SepSize)
            return i;
    }
    return -1;
}


// assuming that "z" is within splitter bounds
void CSplitter::x_HitTest(int z, TPosVector& vpos, int& i_cell, int& i_sep)
{
    i_sep = i_cell = -1;
    int n = (int) vpos.size();
    for( int i = 0; i < n; i++  ) {
        int pos = vpos[i];
        if (z < pos) { // belongs to cell
            i_cell = i;
            return;
        } else if (z < pos + m_SepSize)  { // belongs to separator
            i_sep = i;
            return;
        }
    }
    i_cell = n;
    return; // belongs to last cell
}


void CSplitter::x_DistributeEvenly(bool b_x, bool b_y)
{
    if (b_x) {
        x_DoDistributeEvenly(m_vSplitPosX, GetSize().x);
        std::fill(m_vNormSizeX.begin(), m_vNormSizeX.end(), -1);
    }

    if (b_y) {
        x_DoDistributeEvenly(m_vSplitPosY, GetSize().y);
        std::fill(m_vNormSizeY.begin(), m_vNormSizeY.end(), -1);
    }

    if (b_x  ||  b_y) {
        //TODO x_UpdateClientsState(); // set state before actual resizing

        for( int i = 0; i < (int) m_Cells.size(); i++ ) {
            int i_x = x_GetColumn(i);
            int i_y = x_GetRow(i);
            x_ResizeToCell(i_x, i_y);
        }

        x_RequestUpdate();
    }
}


void CSplitter::x_DoDistributeEvenly(TPosVector& vSplitPos, int size)
{
    int n = (int) vSplitPos.size() + 1;
    int space = max(0, size - m_SepSize * (n - 1));
    int w = space / n; // space per row/column
    int rest = space - w * n; // space that cannot be divided evenly

    int pos = w + (rest ? 1 : 0);

    for( int i = 0; i < n - 1; i++ ) {
        vSplitPos[i] = pos;
        pos += w + m_SepSize;
        if (i < rest) // for first "rest" children add 1 pixel
            pos++;
    }
}


inline int GetElemOrZero(CSplitter::TPosVector& v, int index)
{
    return (index > -1  &&  index < (int) v.size()) ? v[index] : 0;
}


void CSplitter::x_ShrinkCell(TPosVector& sizes, TPosVector& min_sizes, TPosVector& norm_sizes,
                                 int index, int& delta)
{
    int sz = sizes[index];
    if(norm_sizes[index] < 0)   {
        norm_sizes[index] = sz; // save size
    }
    int min_sz = min_sizes[index];
    int new_sz = max(min_sz, sz + delta); // no less then min_sz
    sizes[index] = new_sz;
    delta += (sz - new_sz);
}


void CSplitter::x_GrowCell(TPosVector& sizes, TPosVector& norm_sizes,
                             int index, int& delta, bool unlimited)
{
    int sz = sizes[index];
    int max_sz = norm_sizes[index];
    int new_sz = sz;
    if(unlimited)   {
        new_sz += delta;
    } else if(max_sz > 0) {
        new_sz = min(sz + delta, max_sz); // no less then min_sz
    }
    sizes[index] = new_sz;
    delta += (sz - new_sz);
}


// handles resizing in one dimension; adjusts split positions based on the old
// and the new sizes
void CSplitter::x_DoResize(TPosVector& sizes, TSizeVector& norm_sizes,
                               TPosVector& min_sizes, int size, int new_size,
                               int resizable_index)
{
    int cells_n = (int)sizes.size();
    _ASSERT(cells_n == 0  || ((int) norm_sizes.size() == cells_n
                              &&  (int)min_sizes.size() == cells_n));

    if(resizable_index == -1)
        resizable_index = cells_n - 1;

    int delta = new_size - size;
    if (delta < 0) {
        // shrinking
        x_ShrinkCell(sizes, min_sizes, norm_sizes, resizable_index, delta);
        for( int i = cells_n - 1;  i >= 0  &&  delta < 0;  i-- ) {
            if(i != resizable_index)  {
                x_ShrinkCell(sizes, min_sizes, norm_sizes, i, delta);
            }
        }
    } else if(delta > 0)    {
        // growing
        for( int i = 0;  i < cells_n  &&  delta > 0;  i++  )   {
            if(i != resizable_index)  {
                x_GrowCell(sizes, norm_sizes, i, delta);
            }
        }
        if(delta > 0)   {
            sizes[resizable_index] += delta; // the rest goes to resizable
        }
    }
}


void CSplitter::x_CalculateMinSizes(TPosVector& min_widths,
                                        TPosVector& min_heights) const
{
    size_t col_n = GetColumnsCount();
    size_t row_n = GetRowsCount();

    min_widths.clear();
    min_widths.resize(col_n, 0);
    min_heights.clear();
    min_heights.resize(row_n, 50);

    for( size_t col = 0;  col < col_n;  col++ )  {
        for( size_t row = 0;  row < row_n;  row++ ) {
            /*const wxWindow* w = GetCell((int) col, (int) row);
            const IMinMaxClient* cl = dynamic_cast<const IMinMaxClient*>(w);
            if(cl)   {
                CPoint min_sz = cl->GetMinimalSize();
                min_widths[col] = max(min_widths[col], min_sz.X());
                min_heights[row] = max(min_heights[row], min_sz.Y());
            }*/
        }
    }
}


void CSplitter::x_GetMinimized(vector<bool>& min_cols,
                               vector<bool>& min_rows) const
{
    /*
    int col_n = GetColumnsCount();
    int row_n = GetRowsCount();

    min_cols.clear();
    min_cols.resize((size_t) col_n, false);
    min_rows.clear();
    min_rows.resize((size_t) row_n, false);

    // get the current layout
    TPosVector widths, heights;
    GetWidths(widths);
    GetHeights(heights);

    for( int col = 0;  col < col_n;  col++ )  {
        for( int row = 0;  row < row_n;  row++ ) {
            wxPoint sz(widths[col], heights[row]);

            const wxWindow* w = GetCell(col, row);
            const IMinMaxClient* cl = dynamic_cast<const IMinMaxClient*>(w);

            if(cl)   {
                IMinMaxClient::EState state = cl->GetStateBySize(sz);
                if(state == IMinMaxClient::eMinimized)  {
                    min_cols[col] = true;
                    min_rows[row] = true;
                }
            }
        }
    }
    */
}


void CSplitter::x_MoveSeparator(TPosVector& sizes, TPosVector& norm_sizes, TPosVector& min_sizes,
                                  int i_sep, int delta, int resizable_index)
{
    int cells_n = (int)sizes.size();
    if(resizable_index == -1)
        resizable_index = i_sep;

    int delta_2 = -delta; // the delta applied on the other side of the resizable
    if(i_sep < resizable_index) {
        if(delta < 0)   {
            // shrinking the cells to the left of the separator (right to left)
            for( int i = i_sep;  i >= 0  &&  delta < 0;  i-- ) {
                x_ShrinkCell(sizes, min_sizes, norm_sizes, i, delta);
            }
            // restore-expand cells to the right of the separator (left to rigth)
            for( int i = cells_n-1;  i > i_sep  &&  delta_2 > 0;  i--  )   {
                if(i != resizable_index)  {
                    x_GrowCell(sizes, norm_sizes, i, delta_2);
                }
            }
            // grow resizable (or component to the right)
            x_GrowCell(sizes, norm_sizes, resizable_index, delta_2, true);
        } else {
            // delta >= 0
            // restoring-growing cells to the left of the separator (left to right)
            for( int i = 0;  i < i_sep  &&  delta > 0;  i++  )   {
                x_GrowCell(sizes, norm_sizes, i, delta);
            }
            // the rest of the space goes to the unrestricted growth of the cell
            // to the left of the separator
            x_GrowCell(sizes, norm_sizes, i_sep, delta, true);

            // first compress the resizable
            x_ShrinkCell(sizes, min_sizes, norm_sizes, resizable_index, delta_2);

            // compress the cells to the right of the separator(left to rigth)
            for( int i = i_sep + 1;  i < cells_n -1  &&  delta_2 < 0;  i++ ) {
                x_ShrinkCell(sizes, min_sizes, norm_sizes, i, delta_2);
            }
        }
    } else {
        if (delta < 0) {
            // first compress the resizable
            x_ShrinkCell(sizes, min_sizes, norm_sizes, resizable_index, delta);

            // shrinking the cells to the left from the separator (right to left)
            for( int i = i_sep;  i >= 0  &&  delta < 0;  i-- ) {
                if(i != resizable_index)  {
                    x_ShrinkCell(sizes, min_sizes, norm_sizes, i, delta);
                }
            }
            // restore (grow) the cells to the right from the separator(right to left)
            for( int i = cells_n-1;  i > i_sep  &&  delta_2 > 0;  i-- ) {
                x_GrowCell(sizes, norm_sizes, i, delta_2);
            }
            // the rest of the delta_2 goes to unrestricted growth of the cell to the right
            // of the separator
            if(delta_2 > 0) {
                x_GrowCell(sizes, norm_sizes, i_sep + 1, delta_2, true);
            }
        } else if(delta > 0)    {
            // restore-expand cells to the left from the separator (left to right)
            for( int i = 0;  i <= i_sep  &&  delta > 0;  i++  )   {
                if(i != resizable_index)  {
                    x_GrowCell(sizes, norm_sizes, i, delta);
                }
            }
            if(delta > 0)   {
                sizes[resizable_index] += delta; // the rest goes to resizable
            }
            // restore-shrink cells to the right from the separator (left to right)
            for( int i = i_sep + 1;  i < cells_n  &&  delta_2 < 0;  i++ ) {
                x_ShrinkCell(sizes, min_sizes, norm_sizes, i, delta_2);
            }
        }
    }
}


void CSplitter::x_UpdateRegion(int start_x, int stop_x, int start_y, int stop_y)
{
    bool update = false;
    for(int i_x = 0; i_x <= (int) m_vSplitPosX.size(); i_x++ ) {
        for( int i_y = 0; i_y <= (int) m_vSplitPosY.size(); i_y++  ) {
            // if cell belongs to one of the affected ranges - resize it
            if ( (i_x >= start_x  &&  i_x <= stop_x)  ||
                (i_y >= start_y  &&  i_y <= stop_y) ) {
                update = true;
                x_ResizeToCell(i_x, i_y);
            }
        }
    }
    if (update) {
        //TODOx_UpdateClientsState();
    }
    x_RequestUpdate();
}


void CSplitter::x_NewSplit(TPosVector& split_positions, TSizeVector& v_norm_size, int size)
{
    int n_split = (int) split_positions.size();
    int sum_size = GetSize().y - n_split * m_SepSize;
    int av_size = sum_size / (n_split + 1); // average cell size

    // splitting...
    split_positions.push_back(size);
    v_norm_size.push_back(-1);
    n_split++;

    // calculate new sizes
    double K = (double)(sum_size - m_SepSize) / (sum_size + av_size);
    TPosVector sizes(n_split);
    for( int i = 0; i < n_split; i++ ) {
        int start = (i == 0) ? 0 : split_positions[i - 1] + m_SepSize;
        int sz = split_positions[i] - start;
        sizes[i] = (int)( K * sz);
    }

    // moving split points
    for( int pos = 0, i = 0; i < n_split; i++ ) {
        pos += sizes[i];
        split_positions[i] = pos;
        pos += m_SepSize;
    }
}


// calculate sizes of splitter sections based on split positions
void CSplitter::x_SplitPositionsToSizes(const TPosVector& split_positions,
                                        int total_size, TPosVector& sizes) const
{
    size_t split_n = split_positions.size();
    sizes.clear();
    sizes.resize(split_n + 1);

    int prev_end = 0;
    for( size_t i = 0;  i < split_n;  i++ ) {
        sizes[i] = min(total_size, max(0, split_positions[i] - prev_end));
        total_size = max(0, total_size - sizes[i] - m_SepSize);
        prev_end += sizes[i] + m_SepSize;
    }
    sizes[split_n] = total_size;
}


// calculate split positions based on section sizes
void CSplitter::x_SizesToSplitPositions(const TPosVector& sizes,
                                        TPosVector& split_positions)
{
    size_t split_n = sizes.size() - 1;
    split_positions.clear();
    split_positions.resize(split_n);
    if(split_n > 0) {
        int prev_end = 0;
        for( size_t i = 0;  i < split_n;  i++ ) {
            split_positions[i] = prev_end + sizes[i];
            prev_end = split_positions[i] + m_SepSize;
        }
    }
}

/*
wxSize wxSplitterWindow::DoGetBestSize() const
{
    // get best sizes of subwindows
    wxSize size1, size2;
    if ( m_windowOne )
        size1 = m_windowOne->GetEffectiveMinSize();
    if ( m_windowTwo )
        size2 = m_windowTwo->GetEffectiveMinSize();

    // sum them
    //
    // pSash points to the size component to which sash size must be added
    int *pSash;
    wxSize sizeBest;
    if ( m_splitMode == wxSPLIT_VERTICAL )
    {
        sizeBest.y = wxMax(size1.y, size2.y);
        sizeBest.x = wxMax(size1.x, m_minimumPaneSize) +
                        wxMax(size2.x, m_minimumPaneSize);

        pSash = &sizeBest.x;
    }
    else // wxSPLIT_HORIZONTAL
    {
        sizeBest.x = wxMax(size1.x, size2.x);
        sizeBest.y = wxMax(size1.y, m_minimumPaneSize) +
                        wxMax(size2.y, m_minimumPaneSize);

        pSash = &sizeBest.y;
    }

    // account for the border and the sash
    int border = 2*GetBorderSize();
    *pSash += GetSashSize();
    sizeBest.x += border;
    sizeBest.y += border;

    return sizeBest;
}
*/

END_NCBI_SCOPE
