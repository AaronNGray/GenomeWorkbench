#ifndef GUI_WIDGETS_ALNMULTI___ALIGN_MARK_HANDLER__HPP
#define GUI_WIDGETS_ALNMULTI___ALIGN_MARK_HANDLER__HPP

/*  $Id: align_mark_handler.hpp 36918 2016-11-14 21:19:11Z katargir $
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
 *  CAlignMarkHandler - class implementing support for Marks in alignments.
 *  IAlignMarkHandlerHost - context in which CAlignMarkHandler operates.
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <util/range_coll.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/widgets/gl/ievent_handler.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>

#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <gui/widgets/aln_multiple/list_mvc.hpp>

#include <objects/seqloc/Seq_loc.hpp>

BEGIN_NCBI_SCOPE

// Mark is collection of ranges representing selection on a sequence
// CRangeCollection<TSeqPos> is used to repreesent marks

////////////////////////////////////////////////////////////////////////////////
/// interface IAlignMarkHandlerHost
// This interface represent context in which CAlignMarkHandler operates.
// This context provides the following things:
// - A way to obtain ISelListModel interface in order to check which sequences are
// currently selected.
// - Functions for conversions Pixels <-> Line numbers <-> Alignment Row Indexes
// - Function converting horz. screen coordinate in the position in alignment.
// - Function forsing redrawing of the context.

class IAlignMarkHandlerHost
{
public:
    typedef IAlnExplorer::TNumrow    TNumrow;
    typedef CRangeCollection<TSeqPos>   TRangeColl;
    typedef map<TNumrow, TRangeColl>    TRowToMarkMap;

    typedef ISelListModel<TNumrow>  TSelListModel;
public:
    virtual ~IAlignMarkHandlerHost()  {};

    virtual const IAlnMultiDataSource* MHH_GetAlnDS() const = 0;
    virtual const TSelListModel*    MHH_GetSelListModel() const = 0;

    virtual TNumrow    MHH_GetRowByLine(int Index) const = 0;
    virtual int MHH_GetLineByRowNum(TNumrow Row) const = 0;

    virtual int MHH_GetLineByWindowY(int Y) const = 0;
    virtual int MHH_GetLinePosY(int Index) const = 0;
    virtual int MHH_GetLineHeight(int Index) const = 0;

    virtual TModelUnit  MHH_GetSeqPosByX(int X) const = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// CAlignMarkHandler manages handling and editing of marks on alignments.
///
/// It holds a collection of marks corresponding to sequences of the alignment
/// and processes events in order to provide interactive editing of marks.
/// CAlignMarkHandler renders marks in the provided CGlPane instance (which needs
/// to be properly setup). Each mark is represented as collection of intervals
/// stored in CRangeCollection container.

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlignMarkHandler :
    public wxEvtHandler,
    public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    typedef IAlnExplorer::TNumrow    TNumrow;

    typedef IAlignMarkHandlerHost::TRangeColl   TRangeColl;
    typedef IAlignMarkHandlerHost::TSelListModel    TSelListModel;

    typedef map<TNumrow, TRangeColl>    TRowToMarkMap;

public:
    CAlignMarkHandler();
    void    SetHost(IAlignMarkHandlerHost* pHost);
    IGenericHandlerHost*    GetGenericHost();

    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*   GetEvtHandler();

    void    Render(CGlPane& Pane);  // renders marks

    /// @name Operations with Mark (don't automaticaly update display)
    /// @{

    /// mark/unmark a collection of intervals in seq. coordinates
    void    MarkRowSeq(TNumrow row, const TRangeColl& C, bool set);

    /// mark/unmark a collection of intervals in alignment coordinates
    void    MarkRow(TNumrow row, const TRangeColl& C, bool set);
    void    UnMarkAll(TNumrow row);
    const TRangeColl*   GetMark(TNumrow row) const;

    void    MarkSelectedRows(const TRangeColl& C, bool set);
    void    UnMarkAll();
    void    UnMarkSelected();
    const   TRowToMarkMap&  GetMarks() const;
    /// @}

    /// @name wxWidgets event handling
    /// @{
    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);
    void    OnKeyDown(wxKeyEvent& event);
    void    OnKeyUp(wxKeyEvent& event);
    /// @}

protected:
    enum    EState {
        eIdle,
        eReady,
        eResize
    };

    bool    x_MarkState();

    // signal handlers - functions doing the real job
    void    x_OnStartSel(const wxPoint& ms_pos);
    void    x_OnChangeSelRange(const wxPoint& ms_pos);
    void    x_OnEndSelRange(EState new_state, const wxPoint& ms_pos);
    void    x_OnResetAll();

protected:
    /// helper class representing range being edited
    struct SMarkDelta    {
        enum    EExtState   {
            eNoExt,
            eExtRangeStart,
            eExtRangeEnd
        };

        TSeqRange   m_HitRange; // preexisted range that was hit by the mouse
        TSeqRange   m_Range;    // current range being edited
        EExtState   m_StartState;   //initial extension state
        EExtState   m_State;        //current extension state

        SMarkDelta()    : m_StartState(eNoExt), m_State(eNoExt)   {}
        SMarkDelta(const TSeqRange& HitR, const TSeqRange& R, EExtState StartS, EExtState S)
           : m_HitRange(HitR), m_Range(R), m_StartState(StartS), m_State(S)   {}
    };

    /// @name Helper functions
    /// @{
    TModelUnit  x_MouseToSeqPos(const wxPoint& ms_pos);
    TSeqPos     x_ClipPosByRange(TSeqPos Pos);
    bool        x_HitRangeBorder(const wxPoint& ms_pos) const;

    void    x_InitDeltaMap(const wxPoint& ms_pos);
    void    x_HitTest(TNumrow Row, const TRangeColl& C, int X, TSeqRange& Range, bool& bHitStart) const;
    bool    x_HitSelectedLine(const wxPoint& ms_pos);
    void    x_UpdateDelta(TSeqRange& R, SMarkDelta::EExtState& State, TSeqPos Pos);
    void    x_UpdateSelection(TSeqPos NewPos);
    void    x_UpdateMarks();
    void    x_UpdateState(bool b_key, const wxPoint& ms_pos);

    TSeqRange   x_AlnRangeFromSeqRange(const IAlnMultiDataSource* pAlnDS,
                                       TNumrow row, const TSeqRange& seq_r);

    void    x_OnSelectCursor(const wxPoint& ms_pos);

    const IAlignMarkHandlerHost*    x_GetHost() const {   return m_Host; }
    IAlignMarkHandlerHost*    x_GetHost() {   return m_Host; }
    /// @}

protected:
    typedef map<TNumrow, SMarkDelta>    TRowToDeltaMap;

    TRowToMarkMap   m_mpRowToMark; /// Marks
    TRowToDeltaMap  m_mpRowToDelta; /// Ranges being edited

    IAlignMarkHandlerHost*  m_Host;
    CGlPane*    m_Pane;

    bool        m_bHitResizable;
    EState      m_State;   /// operation type

    wxStockCursor m_CursorId;
    TSeqPos     m_PrevPos;

    CRgbaColor    m_FillColor;
    CRgbaColor    m_FrameColor;
    int     m_DragArea;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALNMULTI___ALIGN_MARK_HANDLER__HPP
