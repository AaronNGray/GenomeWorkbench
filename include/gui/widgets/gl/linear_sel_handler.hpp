#ifndef __GUI_WIDGETS_ALNMULTI___LINEAR_SEL_HANDLER__HPP
#define __GUI_WIDGETS_ALNMULTI___LINEAR_SEL_HANDLER__HPP

/*  $Id: linear_sel_handler.hpp 40801 2018-04-12 15:07:28Z katargir $
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

#include <gui/widgets/gl/ievent_handler.hpp>

#include <gui/widgets/wx/gui_event.hpp>

#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/glpane.hpp>

#include <util/range_coll.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class ISelHandlerHost

class ISelHandlerHost
{
public:
    virtual ~ISelHandlerHost() { }

    virtual void    SHH_OnChanged() = 0;
    virtual TModelUnit  SHH_GetModelByWindow(int z, EOrientation orient) = 0;
    virtual TVPUnit     SHH_GetWindowByModel(TModelUnit z, EOrientation orient) = 0;
};


////////////////////////////////////////////////////////////////////////////////
/// Class CLinearSelHandler represents in GUI a one-dimnesional multi-segment
/// selection. CLinearSelHandler renders the selection in a given CGlPane and
/// allows to interactively modify the selection.
///
/// CLinearSelHandler implements IGlEventHandler what makes it compatible with
/// IGlEventHandler-supproting widgets.
/// CLinearSelHandler provides API for programmatical manipulation of the
//  selection.
class   NCBI_GUIWIDGETS_GL_EXPORT   CLinearSelHandler :
        public wxEvtHandler,
        public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    typedef CRangeCollection<TSeqPos>   TRangeColl;

    enum    ERenderingOption
    {
        eActiveState,
        ePassiveState
    };

    enum EColorType {
        eSelection,
        ePasssiveSelection,
        eSymbol
    };

    enum    EExtState   {
        eNoExt,
        eExtRangeStart,
        eExtRangeEnd
    };
    enum    EOpType {
        eNoOp,
        eAdd,   // add a new segment
        eRemove,
        eReplace, // replace existing segments with a new one
        eChange //resize and existing segment
    };

public:
    CLinearSelHandler(EOrientation orient = eHorz);
    virtual ~CLinearSelHandler();

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*    GetEvtHandler();
    /// @}

    void    SetHost(ISelHandlerHost* host);
    IGenericHandlerHost*    GetGenericHost();
    void    SetOrientation(EOrientation orient);

    void    Render(CGlPane& Pane, ERenderingOption option = eActiveState);

    // colors
    void    SetColor(EColorType type, const CRgbaColor& color);
    void    SetShowRangeCoords(bool flag);

    // commands
    TSeqRange  GetSelectionLimits()    const;
    const   TRangeColl&   GetSelection() const;
    void    SetSelection(const TRangeColl& C, bool bRedraw);
    void    ResetSelection(bool bRedraw);

    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftDoubleClick(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);

protected:
    // signal handlers - invoked by event handlers
    void    x_OnStartSel(const wxPoint& ms_pos, CGUIEvent::EGUIState state);
    void    x_OnChangeSelRange(const wxPoint& ms_pos);
    void    x_OnEndSelRange();
    void    x_OnResetAll();
    void    x_OnOpChange(CGUIEvent& event);

    void    x_OnSelectCursor(const wxPoint& ms_pos);
    void    x_SetCursor();

protected:

    EOpType     x_GetOpTypeByEvent(CGUIEvent& event)    const;
    EOpType     x_GetOpTypeByState(CGUIEvent::EGUIState state) const;

    TModelUnit  x_MouseToSeqPos(const wxPoint& ms_pos);
    TModelUnit  x_ClipPosByRange(TModelUnit Pos);

    void    x_HitTest(const wxPoint& ms_pos, TSeqRange& Range, bool& bHitStart);

    void    x_AddToSelection(const TSeqRange& Range);
    void    x_RemoveFromSelection(const TSeqRange& Range);

private:
    EOrientation   m_Orientation;
    TRangeColl     m_Selection;
    TSeqRange      m_CurrRange; ///< range being modified

    EExtState   m_ExtState;     ///< current range extension state
    EOpType     m_OpType;       ///< operation type
    bool        m_bResizeCursor;
    bool        m_ActiveSel;
    /// Flag indicating to show range coordinates during selection.
    bool        m_ShowCoordinates;

    ISelHandlerHost* m_Host;

    CRgbaColor    m_SelColor;
    CRgbaColor    m_BorderColor;
    CRgbaColor    m_PassiveSelColor;
    CRgbaColor    m_SymbolColor;

    CGlTextureFont m_TexFont;
    CGlPane*      m_Pane;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___LINEAR_SEL_HANDLER__HPP
