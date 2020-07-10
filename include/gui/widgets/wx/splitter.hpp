#ifndef GUI_WX_DEMO___SPLITTER__HPP
#define GUI_WX_DEMO___SPLITTER__HPP

/*  $Id: splitter.hpp 25479 2012-03-27 14:55:33Z kuznets $
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
 *      CSplitter - and extended version of splitter for wxWidgets.
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <wx/panel.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSplitter - container with resizable separators.
///
/// CSplitter arranges child windows in to one- or two-dimensional array.
/// Cells in CSplitter are divided by separators that can be moved by the
/// mouse.  Every cell in CSplitter can be occupied by a single child window
/// and every window can occupy only one cell.

class NCBI_GUIWIDGETS_WX_EXPORT CSplitter
    : public wxPanel
{   
    DECLARE_EVENT_TABLE()
public:
    typedef wxPanel TParent;
    typedef vector<int> TPosVector;
    typedef vector<int> TSizeVector;

    /// ESplitMode - splitter orientation
    enum ESplitType  {
        eVertical,   /// vertical space is splitted
        eHorizontal, /// horizontal space is splitted
        eGrid        /// cells are separated with in both directions
    };

    CSplitter();

    CSplitter(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxTAB_TRAVERSAL,
                const wxString& name = wxT("panel"));

    CSplitter(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                ESplitType type = eVertical);

    virtual ~CSplitter();

    ESplitType   GetSplitType() const;
    int GetSeparatorSize() const;

    /// Creates a splitter with a single cell
    virtual void    Split(ESplitType type);

    /// Creates a splitter with cells defined by "sizes". If mode == eGrid then
    /// size will be used as both width and heights
    virtual void    Split(ESplitType type, const TPosVector& sizes);

    /// Creates a splitter with cells defined by "sizes". If mode == eGrid then
    /// ar_sizes will be used as both width and heights. "ar_sizes" is -1
    /// terminated array.
    virtual void    Split(ESplitType type, const int ar_size[]);

    /// Creates splitter, "widths" and "height" define number and sizes of
    /// cells.
    virtual void    Split(ESplitType type, const TPosVector& widths, const TPosVector& heights);

    /// Creates splitter, "ar_w" and "ar_h" are arrays terminated by -1.
    virtual void    Split(ESplitType type, const int ar_w[], const int ar_h[]);

    virtual void    Split(int col_n, int row_n);

    int     GetColumnsCount() const;
    int     GetRowsCount() const;

    void    GetWidths(TPosVector& widths)   const;
    void    SetWidths(const TPosVector& widths);

    void    GetHeights(TPosVector& heights) const;
    void    SetHeights(const TPosVector& heights);

    // is set, this cell will resize first trying to "consume" any changes in availble space
    // caused by the splitter resizing or separator dragging
    void    SetResizableCell(int col, int row);

    virtual bool    Layout();
    virtual bool    Show(bool show = true);

    virtual void    BlockLayout(bool block = true);

    /// returns "true" if cell specified by (col, row) exists in the splitter
    bool    IsValidCell(int col, int row) const;

    int GetColumnWidth(int col) const   {   return x_GetWidth(col);     }
    int GetRowHeight(int row) const     {   return x_GetHeight(row);    }

    /// If cell [col, row] exists and vacant - adds widget to the container.
    /// Returns "true" if widget has been added.
    bool    InsertToCell(wxWindow* child, int col, int row);

    void    AddColumn();
    void    AddRow();

    /// overriding wxWindowBase function
    virtual void RemoveChild(wxWindowBase* child);

    virtual bool    RemoveFromCell(int col, int row);
    virtual void    RemoveAll();

    wxWindow*         GetCell(int col, int row);
    const wxWindow*   GetCell(int col, int row) const;

    bool    FindChild(const wxWindow* child, int& col, int& row)  const;

    virtual void x_PreserveNormalSize(bool preserve);

protected:
     virtual void   x_DrawSeparator(wxDC& dc, int x, int y, int w, int h);
     void x_SplitPositionsToSizes(const TPosVector& split_positions,
                                  int total_size, TPosVector& sizes) const;
     void x_SizesToSplitPositions(const TPosVector& sizes,
                                 TPosVector& split_positions);

    void    x_Init();   

    void    x_Split(ESplitType type, const TPosVector& widths, const TPosVector& heights);

    int     x_GetCellIndex(int i_x, int i_y) const;
    int     x_GetColumn(int i_cell) const;
    int     x_GetRow(int i_cell) const;
    int     x_GetCellIndex(wxWindow* widget) const;

    int     x_GetLeft(int i_x)  const;
    int     x_GetRight(int i_x)  const;
    int     x_GetWidth(int i_x)  const;

    int     x_GetTop(int i_y)     const;
    int     x_GetBottom(int i_y)  const;
    int     x_GetHeight(int i_y)  const;

    bool    x_RemoveChild(int index);

    void    x_Clear();
    void    x_ResizeToCell(int i_cell);
    void    x_ResizeToCell(int i_x, int i_y);
    void    x_ResizeAllCells();

    void    OnPaint(wxPaintEvent& event);   
    void    OnMouseMove(wxMouseEvent& event);
    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMouseEnter(wxMouseEvent& event);
    void    OnMouseLeave(wxMouseEvent& event);
    void    OnContextMenu(wxContextMenuEvent& event);
    void    OnKeyDown(wxKeyEvent& event);

    void    x_RequestUpdate();

    /// returns Separator index if hit or -1
    int     x_HitTestSeparator(int z, TPosVector& vpos);
    void    x_HitTest(int z, TPosVector& vpos, int& i_cell, int& i_sep);

    virtual void    x_StartDrag(int i_sep_x, int i_sep_y);
    virtual void    x_DoDragSeparator(wxMouseEvent& event, bool b_final);   
    virtual void    x_EndDrag();
    virtual bool    x_IsDragging() const;
    
    void    x_DistributeEvenly(bool b_x, bool b_y);
    void    x_DoDistributeEvenly(TPosVector& vSplitPos, int size);
    
    virtual void    x_Resize(const wxSize& old_size, const wxSize& new_size, bool b_size_changes);

    static void x_ShrinkCell(TPosVector& sizes, TPosVector& min_sizes, TPosVector& norm_sizes,
                             int index, int& delta);
    static void x_GrowCell(TPosVector& sizes, TPosVector& norm_sizes,
                           int index, int& delta, bool unlimited = false);

    void    x_DoResize(TPosVector& sizes, TSizeVector& norm_sizes,
                       TPosVector& min_sizes, int size, int new_size,
                       int resizable_index);

    void    x_MoveSeparator(TPosVector& sizes, TSizeVector& norm_sizes, TPosVector& min_sizes,
                            int i_sep, int delta, int resizable_index);

    void    x_GetMinimized(vector<bool>& min_cols, vector<bool>& min_rows) const;
    void    x_CalculateMinSizes(TPosVector& min_widths,
                                TPosVector& min_heights) const;
    
    // updates cells belonging to [start_x, stop_x] or to [start_y, stop_y]
    void    x_UpdateRegion(int start_x, int stop_x, int start_y, int stop_y);
    
    void    x_NewSplit(TPosVector& split_positions, TSizeVector& v_norm_size, int size);
    void    x_Deprecated()  const   {   _ASSERT(false); }

protected:
    typedef vector<wxWindow*>  TCells;

    ESplitType m_Type;

    bool    m_BlockLayout;
    wxSize  m_LayoutedSize; // previous size for which layoting has been done
    wxSize  m_SplitSize; // the size corresponding to the current separaror positions

    TPosVector  m_vSplitPosX; // coordinates of first pixels of separators
    TPosVector  m_vSplitPosY; // coordinates of first pixels of separators
    int m_SepSize;
    TSizeVector m_vNormSizeX; // normal (not "shrinked") sizes of cells
    TSizeVector m_vNormSizeY; // normal (not "shrinked") sizes of cells

    TCells  m_Cells;

    int m_ResizableColumn;
    int m_ResizableRow;

    /// members used for event handling
    wxWindow* m_PrevFocus;

    int m_MouseDownX, m_MouseDownY; // window coordinate of last Mouse-Down
    int m_StartPosX, m_StartPosY; // start position of separators being dragged (local)
    int m_iDragSepX, m_iDragSepY; // index of separator being dragged or -1
    wxPoint m_PrevSepPos;

    TPosVector  m_MinWidths, m_MinHeights;
    int m_MinSepPosX, m_MaxSepPosX; // define the range within which the seprator can be move
    int m_MinSepPosY, m_MaxSepPosY;

    int    m_HighlightCell; // index of the highlighted cell
};

END_NCBI_SCOPE;


#endif  // GUI_WX_DEMO___SPLITTER__HPP
