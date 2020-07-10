/*  $Id: seq_table_grid.hpp 43833 2019-09-09 17:38:14Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */
#ifndef _SEQ_TABLE_GRID_H_
#define _SEQ_TABLE_GRID_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <gui/packages/pkg_sequence_edit/seq_grid_table_navigator.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>

/*!
 * Includes
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/grid.h"
#include <wx/textwrapper.h>
////@end includes


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE


static const int kSkipColumns = 1;

class CSeqGridCellAttrProvider : public wxGridCellAttrProvider
{
public:
    CSeqGridCellAttrProvider();
    virtual ~CSeqGridCellAttrProvider();

    virtual wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind  kind = wxGridCellAttr::Any) const wxOVERRIDE;
    virtual void SetAttr(wxGridCellAttr *attr, int row, int col) wxOVERRIDE;

private:

// http://stackoverflow.com/questions/32685540/c-unordered-map-with-pair-as-key-not-compiling
    struct pair_hash 
    {
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1,T2> &p) const 
            {
                auto h1 = std::hash<T1>{}(p.first);
                auto h2 = std::hash<T2>{}(p.second);
//                return h1 ^ h2;  
                return (h1 ^ (h2 + 0x9e3779b9 + (h1<<6) + (h1>>2))); // http://www.boost.org/doc/libs/1_59_0/boost/functional/hash/hash.hpp
            }
    };
    unordered_map< pair<int, int>, wxGridCellAttr*, pair_hash> m_cache;
};

class CCollapsibleGrid : public wxGrid
{
    wxDECLARE_NO_COPY_CLASS(CCollapsibleGrid);
public:
    CCollapsibleGrid() { wxGrid::Init(); }
    CCollapsibleGrid(wxWindow *parent,
                     wxWindowID id,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxWANTS_CHARS,
                     const wxString& name = wxGridNameStr);
    virtual ~CCollapsibleGrid() {}

    void x_InitRowHeights();
    void FastSetRowSize(int row, int height);
    void UpdateRowBottoms();

private:
    int UpdateRowOrColSize(int& sizeCurrent, int sizeNew);
    vector<int> m_diffs;
};

class  CGridCellWrapStringRenderer : public wxGridCellRenderer
{
public:
    CGridCellWrapStringRenderer() : wxGridCellRenderer() { }

    virtual void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,  
                      bool isSelected) wxOVERRIDE;

    virtual wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) wxOVERRIDE;

    virtual wxGridCellRenderer *Clone() const wxOVERRIDE
        { return new CGridCellWrapStringRenderer; }

private:
    void SetTextColoursAndFont(const wxGrid& grid, const wxGridCellAttr& attr, wxDC& dc, bool isSelected);
    wxString BreakLines(const wxString &text);
};

class CSeqTableGrid : public wxGridTableBase 
{
public:
    CSeqTableGrid(CRef<objects::CSeq_table> values_table);
    virtual ~CSeqTableGrid() {}

    virtual int GetNumberRows();
    virtual int GetNumberCols();
    virtual bool IsEmptyCell(int row, int col);
    
    virtual wxString GetValue(int row, int col);
    virtual void SetValue(int vis_row, int vis_col, const wxString& value);
    
/*
    // Overriding these is optional
    //
    virtual void SetView( wxGrid *grid ) { m_view = grid; }
    virtual wxGrid * GetView() const { return m_view; }

    virtual void Clear() {}
    virtual bool InsertRows( size_t pos = 0, size_t numRows = 1 );
    virtual bool AppendRows( size_t numRows = 1 );
    virtual bool DeleteRows( size_t pos = 0, size_t numRows = 1 );
*/
    virtual bool InsertCols(size_t pos = 0, size_t numCols = 1);
    virtual bool AppendCols(size_t numCols = 1);
    virtual bool DeleteCols(size_t pos = 0, size_t numCols = 1);

    virtual wxString GetRowLabelValue(int row);
    virtual wxString GetColLabelValue(int col);

    virtual void SetColLabelValue(int col, const wxString& label);

    CRef<objects::CSeq_table> GetValuesTable(void) { return m_ValuesTable; }
    CConstRef<objects::CSeq_table> GetValuesTable(void) const { return m_ValuesTable; }


private:
    CRef<objects::CSeq_table> m_ValuesTable;
};


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSEQTABLEGRID 10078
#define ID_GRID 10012
#define SYMBOL_CSEQTABLEGRIDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSEQTABLEGRIDPANEL_TITLE _("SeqTableGrid")
#define SYMBOL_CSEQTABLEGRIDPANEL_IDNAME ID_CSEQTABLEGRID
#define SYMBOL_CSEQTABLEGRIDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSEQTABLEGRIDPANEL_POSITION wxDefaultPosition
////@end control identifiers

enum
{
    eCmdCopyTableValues = 31200,
    eCmdPasteTableValues,
    eCmdAppendTableValues,
    eCmdCopyTableValuesFromId,
    eCmdSearchTable,
    eCmdDeleteTableCol,
    eCmdRenameTableCol,
    eCmdCollapseTableCol,
    eCmdExpandTableCol
};

class SrcEditDialog;
/*!
 * CSeqTableGridPanel class declaration
 */

class CSeqTableGridPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSeqTableGridPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqTableGridPanel();
    CSeqTableGridPanel( wxWindow* parent, CRef<objects::CSeq_table> values, CRef<objects::CSeq_table> choices, int glyph_col = -1, 
                   wxWindowID id = SYMBOL_CSEQTABLEGRIDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSEQTABLEGRIDPANEL_POSITION, const wxSize& size = SYMBOL_CSEQTABLEGRIDPANEL_SIZE, long style = SYMBOL_CSEQTABLEGRIDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQTABLEGRIDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSEQTABLEGRIDPANEL_POSITION, const wxSize& size = SYMBOL_CSEQTABLEGRIDPANEL_SIZE, long style = SYMBOL_CSEQTABLEGRIDPANEL_STYLE );

    /// Destructor
    ~CSeqTableGridPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSeqTableGridPanel event handler declarations

    /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID
    void OnCellRightClick( wxGridEvent& event );

    /// wxEVT_GRID_LABEL_RIGHT_CLICK event handler for ID_GRID
    void OnLabelRightClick( wxGridEvent& event );

////@end CSeqTableGridPanel event handler declarations

    void OnCopyCells( wxCommandEvent& event );
    void OnPasteCells( wxCommandEvent& event );
    void OnPasteAppendCells( wxCommandEvent& event );
    void OnCopyCellsFromId( wxCommandEvent& event );
    void OnSearchTable( wxCommandEvent& event );
    void OnSortTableByColumn( wxGridEvent& event );
    void OnLabelLeftClick( wxGridEvent& event );
    void OnLabelLeftDClick( wxGridEvent& event );
    void OnCellLeftClick( wxGridEvent& event );
    void OnCellDoubleLeftClick( wxGridEvent& event );
    void OnCellDoubleLeftClick2( wxMouseEvent& event );
    void OnDeleteColumn( wxCommandEvent& event );
    void OnRenameColumn( wxCommandEvent& event );
    void OnCollapseColumn( wxCommandEvent& event );
    void OnExpandColumn( wxCommandEvent& event );

////@begin CSeqTableGridPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqTableGridPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSeqTableGridPanel member variables
////@end CSeqTableGridPanel member variables
    wxGrid* GetGrid(void) {return m_Grid;}
    CRef<objects::CSeq_table> GetValuesTable();
    void SetValuesTable(CRef<objects::CSeq_table> table);
    void CopyPasteDataByKeyboard(wxKeyEvent& event);

    void UpdateColumnLabelHeight ();
    int GetBestTableWidth();
    void SetBestTableWidth();
    void MakeColumnReadOnly(int pos, bool val = true);
    void MakeColumnReadOnly(string name, bool val = true);
    void MakeAllColumnsReadOnly();
    void SetColumnSizesAndChoices();
    void UpdateColumnChoices(int pos, vector<string> choices);
    void InitColumnCollapse(int col);
    bool GetModified() {return m_Modified;}
    int GetCollapseColAndExpand(void);
    void CollapseByCol(int col);
    int GetRowIndex(int row);
private:
    CRef<objects::CSeq_table> m_Table;
    CRef<objects::CSeq_table> m_Choices;
    CRef<objects::CSeq_table> m_Copied;
    bool m_SortByRowLabel;
    bool m_SortByRowLabelAscend;
    unordered_map<string,int> m_MapRowLabelToIndex;
    wxString m_FindStr;
    void InitMapRowLabelToIndex();
    void x_SortByRowLabel();
    void x_SetUpMenu();
    void x_CreateMenu(wxGridEvent& evt);
    void x_PasteCells(CRef<objects::CSeq_table> copied, bool append = false, string delim = ";");

    bool x_ValuesOkForNewColumn (CConstRef<objects::CSeqTable_column> values, int col_pos);
    void x_OnCopyCells();
    void x_OnPasteCells();
    void x_MakeProblemsColumnReadOnly();
    SrcEditDialog* x_GetParent();
    CSeqGridTableNav* x_GetSeqGridTableNav();
    void x_CollapseTableByColumn(bool initialize = false);
    void x_ExpandTableByColumn(void);
    void x_UpdateCollapsedRow(unsigned int num_cols, int expand_row, vector<bool> &all_present, vector<bool> &all_same, vector<wxString> &values);
    void x_ExpandTable();
    void x_SortTableByColumn(int col, bool ascend);
    void x_UpdateCountSelectedDisplay(void);
    void x_SelectHiddenRows(int row);

    unordered_map<int, int> m_CollapseCell;
    map<pair<int,int>, wxString> m_CollapseCache;
    int m_CollapseGlyphCol, m_CollapseCol;
    set<int> m_SetOfReadOnlyCols;
    bool m_Modified;
    int m_SortCol;
    int m_MenuCol;
    CCollapsibleGrid* m_Grid;
    int m_edit_cell_row, m_edit_cell_col;
};



END_NCBI_SCOPE

#endif
    // _SEQ_TABLE_GRID_H_
