#ifndef __GUI_WIDGETS_ALNMULTI___IALIGN_ROW__HPP
#define __GUI_WIDGETS_ALNMULTI___IALIGN_ROW__HPP

/*  $Id: ialign_row.hpp 44984 2020-05-01 16:46:19Z shkeda $
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
 * Authors:  Andrey Yazhuk, Anatoliy Kuznetsov
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/aln_multiple/row_display_style.hpp>

#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/widgets/gl/html_active_area.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/event.h>

BEGIN_NCBI_SCOPE


class IAlignRowHost;
class ITooltipFormatter;
class CSelectionVisitor;
class CConsensusRow;

////////////////////////////////////////////////////////////////////////////////
/// Interface  IAlignRow - abstracts row rendering in Multiple Alignment
/// Widget.
///
/// IAlignRow represents an object that can render an alignment row as a set
/// of cells corresponding to enumerated columns. No asumptions is made about
/// data structure used for alignment representation. The concrete classes
/// implementing the interface are responsible for providing means for
/// accessing data.

class IAlignRow
{
public:
    typedef vector<CHTMLActiveArea>    TAreaVector;
    typedef int TColumnType;

    ///  Column identifier for use as TColumnType, defines basic column types
    ///  this set can be extended in derived classes
    enum EColumnType    {
        eInvalid = -1,
        eDescr,
        eIcons,       ///< Strand + Expand icons 
        eSeqStart,
        eStart,
        eAlignment,
        eEnd,
        eSeqEnd,
        eSeqLength,
        eIconStrand,  ///< Icon for Positive/Negative (Mixed?) strand
        eIconExpand,  ///< Icon for expand (+) button
        eTaxLabel,
        eLast
    };

    /// Row state.
    enum EState {
        fNone           = 0,
        fItemSelected   = 0x01,
        fItemFocused    = 0x02,
        fItemHidden     = 0x08,

        fWidgetFocused  = 0x40000000, ///< it isn'e exactly a row state;
        // this flag is added dynamicaly to control row rendering

        fFocused = fItemFocused | fWidgetFocused,
        fSelectedActive = fItemSelected | fWidgetFocused,
        fSelectedFocused = fFocused | fItemSelected
    };

    enum    EGraphCacheCmd  {
        eInvalidate,
        eDelete
    };

    virtual ~IAlignRow()    {};

    virtual void    SetHost(IAlignRowHost* pHost) = 0;
    virtual void    SetConsensus(CConsensusRow* crow) = 0;

    virtual void    SetDisplayStyle(const CRowDisplayStyle* style) = 0;
    virtual const CRowDisplayStyle*    GetDisplayStyle() = 0;

    /// Returns index of row in IAlnMultiDataSource.
    virtual int GetRowNum() const = 0;

    /// Returns height of the row in pixels.
    virtual int GetHeightPixels() const = 0;

    /// Renders a row cell corresponding to column specified by "col_type". "Pane"
    /// represents port corresponding to the cell; model space is Elems x Pixels.
    virtual void    RenderColumn(TColumnType  col_type) = 0;
    virtual void    GetColumnText(TColumnType col_type, string& text, bool for_printer) const = 0;

    virtual void    RenderRow() {}

    virtual void    OnMouseEvent(wxMouseEvent& event, TColumnType  col_type, CGlPane& pane) = 0;

    virtual void    GetTooltip(const TVPPoint& pt, TColumnType col_type, CGlPane& pane, ITooltipFormatter& tooltip) = 0;

    virtual void    GraphicsCacheCmd(EGraphCacheCmd /*cmd*/)    {};

    virtual void    GetHTMLActiveAreas(TColumnType col_type, CGlPane& pane, TAreaVector& areas) = 0;

    virtual void    UpdateOnStyleChanged() = 0;
    virtual void    UpdateOnAnchorChanged() {}; // TODO
    virtual void    Update(const CGlPane& pane, bool layout_only = false) {};

    virtual TSeqPos GetColumnAsSeqPos(TColumnType type, const TModelRect& rc_vis) = 0;
    virtual string  GetColumnAsString(TColumnType type) = 0;

    virtual string& GetAlnSeqString(string &buffer, const IAlnExplorer::TSignedRange &aln_rng) const = 0;
    virtual string& GetStringAtPos(string& buffer, TSignedSeqPos aln_pos) const = 0;

    virtual void    GetRowStatistics(ITooltipFormatter& tooltip) const = 0;

    virtual bool    UsesAATranslation() const = 0;
    virtual bool    IsNucProtAlignment() const = 0;


    // ------------------------------------------------------------------
    /// @name Row State accessors
    /// @{

    /// Returns row state (combination of EState flags)
    virtual int GetRowState() const = 0;

    /// Set/Clear Row State
    virtual void SetRowState(int mask, bool b_set) = 0;
    /// @}

    virtual void PrepareRendering(CGlPane& pane, TVPUnit row_top, int state) = 0;
protected:
    typedef struct {
        CGlPane* m_Pane = 0;
        int      m_Top = 0;
        TVPRect  m_Viewport;
        int      m_State = 0;
    } SRenderCtx;
    SRenderCtx m_RenderCtx;

};


////////////////////////////////////////////////////////////////////////////////
/// IAlignRowHost represents a context in which IAlignRow lives.
///
class IAlignRowHost
{
public:
    virtual ~IAlignRowHost()    {};

    /// provides mouse coords in OpenGL viewport coord system
    virtual TVPPoint    ARH_GetVPPosByWindowPos(const wxPoint& pos) = 0;

    /// notifies a host about changes the row
    virtual void        ARH_OnRowChanged(IAlignRow* row) = 0;

    /// notifies a host about selection change
    virtual void        ARH_OnSelectionChanged() = 0;

    /// notifies a host about selection change
    virtual CSelectionVisitor* ARH_GetSelectionVisitor() = 0;

    /// Returns Y-pos for row
    virtual int         ARH_GetRowTop(IAlignRow* p_row) = 0;

};




END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___IALIGN_ROW__HPP
