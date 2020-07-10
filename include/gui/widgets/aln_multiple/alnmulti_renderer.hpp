#ifndef __GUI_WIDGETS_ALNMULTI___ALNMULTI_RENDERER__HPP
#define __GUI_WIDGETS_ALNMULTI___ALNMULTI_RENDERER__HPP

/*  $Id: alnmulti_renderer.hpp 45043 2020-05-14 15:23:30Z shkeda $
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

#include <corelib/ncbistl.hpp>


#include <gui/widgets/gl/ruler.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>

#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <gui/widgets/aln_multiple/ialign_row.hpp>
#include <gui/widgets/aln_multiple/alnmulti_header.hpp>

//#include <gui/widgets/gl/graph_container.hpp>
#include <gui/widgets/seq_graphic/selection_manager.hpp>

BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////
/// IAlnMultiRendererContext - this interface represents context in which
/// CAlnMultiRenderer lives. It provides callbacks necessary for Renderer's
/// functioning.

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT IAlnMultiRendererContext
{
public:
    typedef IAlnExplorer::TNumrow    TNumrow;
    typedef IAlignRow::TColumnType  TColumnType;

    enum EShownElements {
        fShownElement_Header     = 1, 
        fShownElement_Ruler      = 2,
        // Do not touch these enums, there are equivalents for them in row_display_style.hpp
        // and in JavaScript client code
        fShownElement_MasterRow  = 4,
        fShownElement_Alignment  = 8,
        fShownElement_Track      = 16,
        fShownElement_Main       = 32, // as opposed to MasterRow, both on for legacy case
        fShownElement_All        = 0xffff
    };


public:
    virtual ~IAlnMultiRendererContext()  {};

    virtual const CGlPane& GetAlignPort() const = 0;
    virtual TNumrow     GetLinesCount() const = 0;
    virtual bool    IsRendererFocused() = 0;
    virtual int     GetFocusedItemIndex() const = 0;
    virtual bool    IsItemSelected(int index) const = 0;

    virtual IAlignRow*  GetMasterRow() = 0;
    virtual IAlignRow*  GetConsensusRow() = 0;

    /// all Y coordinates are OpenGL Viewport coordinates (not widget coords)
    /// "index" is a line index (not row in alignment)

    virtual IAlignRow*  GetRowByLine(int index) = 0;
    virtual int     GetLineByRowNum(TNumrow row) const = 0;
    virtual int     GetLineByModelY(int y) const = 0;
    virtual int     GetLinePosY(int index) const = 0;
    virtual int     GetLineHeight(int index) const = 0;

    virtual bool    SortByColumn(TColumnType type, bool ascending) = 0;
    virtual IAlnExplorer::ESortState   GetColumnSortState(TColumnType type) = 0;

    virtual int GetShownElements() = 0;
};

class ITooltipFormatter;

////////////////////////////////////////////////////////////////////////////////
/// class CAlnMultiRenderer - renders multiple alignment represented by
/// IAlnMultiRendererContext (the Model).
///
/// IAlnMultiRenderer renders a grid where rows corresponds to IAlignRow objects
/// and columns represent different types of information. Columns can be
/// configured using "Column management" API.

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiRenderer
        :   public IAlnMultiHeaderContext
{
public:
    enum EHtmlAreaTypes {
        eHtmlArea_Link,
        eHtmlArea_CheckBox
    };
    typedef IAlignRow::TColumnType      TColumnType;
    typedef vector<CHTMLActiveArea>     TAreaVector;

public:
    /// Constructor, with column add, icons setup and resize
    ///
    CAlnMultiRenderer(const TVPRect& rc, bool add_columns = true);

    /// Empty object constructor -- for two-phase construction
    /// call SetupColums() to finish the class construction process
    CAlnMultiRenderer();
    virtual ~CAlnMultiRenderer();

    /// Adds default columns for the renderer, allows custom setup
    /// (column icon override, etc)
    ///
    virtual void SetupColumns(const TVPRect& rc);

    void    SetContext(IAlnMultiRendererContext* pContext);

    void    SetBackColor(const CRgbaColor Color);
    const CRgbaColor&    GetBackColor() const;

    void    SetMasterBackColor(const CRgbaColor Color);
    const CRgbaColor&    GetMasterBackColor() const;

    void    SetFocused(bool b_set)    {   m_bFocused  = b_set;  }
    void    SetCgiMode(bool b_set)    {   m_bCgiMode  = b_set;  }

    virtual void    Resize(const TVPRect& rc);
    virtual TVPRect GetRect() const;

    virtual void    Update(bool layout_only = false);
    virtual void    UpdateOnDataChanged();

    /// renders OpenGL graphics
    virtual void    Render();

    /// renders OpenGL graphics and generate CHTMLActiveArea objects
    virtual void    Render(TAreaVector& areas);

    virtual bool    NeedTooltip(int vp_x, int vp_y);
    virtual string  GetTooltip(); // returns tooltip for previously tested position
    // MSA method
    virtual string  GetAlignmentTooltip(int x, int y, IAlignRow* row, bool master);

    CAlnMultiHeader&    GetHeader();

    CRuler&    GetRuler();
    CGlPane&   GetRulerPane() {   return m_RulerPane;   }

    int     GetRulerAreaHeight()    const   {   return m_RulerRect.Height();  }
    int     GetMasterAreaHeight()    const   {   return m_MasterRect.Height();  }
    int     GetListAreaHeight() const   {   return m_ListAreaRect.Height();    }
    int     GetHeaderAreaHeight() const   {   return m_HeaderRect.Height();    }
    TVPRect GetMasterArea() const;

    int     GetListTop() const;
    int     GetColumnIndexByType(TColumnType type) const;
    TVPRect         GetColumnRect(int i_col, bool include_header)   const;
    TVPRect         GetColumnRectByType(TColumnType type, bool include_header)   const;
    TColumnType     GetColumnTypeByIndex(int i_col)   const;
    TColumnType     GetColumnTypeByX(int vp_x)   const;

    /// return OpenGL viewport coordinate of the top pixel in the list area
    int     GetVPListTop() const;

    enum EHitResult {
        eNone,
        eHeader,
        eRuler,
        eMasterRow,
        eRows
    };

    virtual int  HitTest(int vp_x, int vp_y, int& col);

    void    SetupPaneForColumn(CGlPane& pane, int i_col) const;
    void    AutoFitColumns(const CGlTextureFont& font, bool for_printer); 


    /// @name IAlnMultiHeaderContext implementation
    /// @{
    virtual int     GetColumnsCount() const;
    virtual const   SColumn&  GetColumn(int index) const;
    virtual SColumn&  GetColumn(int index);
    virtual int     GetColumnIndexByX(int x) const;
    virtual int     GetResizableColumnIndex() const;
    /// @}

    /// @name Columns management
    /// @{
    /// Columns can be "fixed size" or "resizable". Fixed size columns are
    /// rendered with specifed width. Remaining screen space is divided
    /// between "resizable" columns proportionally to m_Share.

    int     AddColumn();
    int     AddColumn(int width, const string& label, int data);
    int     InsertColumn(int pos, int width, const string& label, int data);
    void    SetResizableColumn(int index);

    void    SetColumns(const vector<SColumn>& columns, int resizable_index);

    void    SetColumnWidth(int index, int width);
    void    SetColumnPos(int index, int pos);
    void    SetColumnUserData(int index, int data);
    void    SetColumnVisible(int index, bool b_visible);
    bool    IsColumnVisible(int index) const;

    void    SetColumnsByStyle(CWidgetDisplayStyle& style);
    //void    SetHiddenColumns(const vector<string>& labels);
    void    GetVisibleColumns(vector<string>& labels);

    void    UpdateColumns(); // update positions
    /// @}



protected:
    IAlnMultiRendererContext* x_GetContext()    {   return m_Context;   }
    const IAlnMultiRendererContext* x_GetContext()  const    {   return m_Context;   }

    void    x_Layout();

    // Rendering functions
    void    x_Render(TAreaVector* p_areas);
    void    x_RenderHeader(TAreaVector* p_areas);
    void    x_RenderRuler(TAreaVector* p_areas);
    void    x_RenderItems(TAreaVector* p_areas);
    void    x_RenderItemsRange(int iFisrt, int iLast, TAreaVector* p_areas);
    void    x_RenderRow(IAlignRow* row, CGlPane& pane, int state, TVPUnit row_top, TAreaVector* areas);
    void    x_RenderMasterRow(TAreaVector* p_areas);
    void    x_PrepareMasterPane(CGlPane& pane);
    void    x_RenderAlignAreaTopLine();

    void    x_ResetRowListMap();
    void    x_InvalidateRows(bool layout_only = false);
    void    x_PurgeRowListMap();

    int     x_GetLineByWindowY(int WinY) const;
    int     x_GetLineByVPY(int vp_y) const;
    TVPRect x_GetLineRect(int Index);

    inline TColumnType     x_GetColumnType(const SColumn& C)
    {
        return (IAlignRow::TColumnType) C.m_UserData;
    }

    void    x_LayoutColumns();

    string x_GetRowTooltip(const TVPPoint& pt, IAlignRow* p_row, int i_col, int vp_top_y);
    void x_AddStatisticsRow(ITooltipFormatter &formatter, const string &tag, int part, int total);

protected:
    IAlnMultiRendererContext* m_Context;
    TVPRect m_rcBounds; // size of renderer in parents's coordinates

    CRgbaColor m_BackColor;
    CRgbaColor m_MasterBackColor;

    CAlnMultiHeader m_Header;

    CRuler  m_Ruler;
    CGlPane m_RulerPane;

    // Layout information
    TVPRect m_HeaderRect;
    TVPRect m_RulerRect;
    TVPRect m_MasterRect;
    TVPRect m_ListAreaRect;

    typedef     map<IAlignRow*, bool>     TRowToList;
    typedef vector<SColumn> TColumnVector;

    TRowToList  m_RowToList; // value == "true" if key has an OpenGL Display List
                             // columns
    TColumnVector m_Columns;
    int m_ResizableColumnIndex;

    bool m_bFocused; /// indicate whether it shoulde be rendered using "focused" color or not
    bool m_bCgiMode = false;  /// is it invoked by alnmulti.cgi

    TVPPoint m_TooltipVPPos;
    int m_HitResult;
    //IAlignRow* m_TooltipRow;
    CWeakIRef<IAlignRow> m_TooltipRow;
    int m_TooltipColumn;

};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALNMULTI_RENDERER__HPP
