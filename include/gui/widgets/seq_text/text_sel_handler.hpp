#ifndef __GUI_WIDGETS_SEQTEXT___TEXT_SEL_HANDLER__HPP
#define __GUI_WIDGETS_SEQTEXT___TEXT_SEL_HANDLER__HPP

/*  $Id: text_sel_handler.hpp 32128 2015-01-05 22:23:21Z shkeda $
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
 * Authors:  Colleen Bollin (adapted from a file by Andrey Yazhuk)
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/glpane.hpp>

#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/widgets/seq_text/seq_text_ds.hpp>
#include <gui/widgets/seq_text/seq_text_geometry.hpp>
//#include <gui/widgets/seq_text/seq_graph.hpp>
#include <util/range_coll.hpp>
#include <gui/objutils/object_index.hpp>

#include <gui/widgets/wx/gui_event.hpp>

// for codon rendering
#include <objects/seqfeat/Cdregion.hpp>
#include <objmgr/feat_ci.hpp>

#include <objmgr/seq_loc_mapper.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// Class CTextSelHandler represents in GUI a two-dimnesional multi-segment
/// selection. CTextSelHandler renders the selection in a given CGlPane and
/// allows to interactively modify the selection.
///
/// CTextSelHandler implements IGlEventHandler what makes it compatible with
/// IGlEventHandler-supproting widgets.
/// CTextSelHandler provides API for programmatical manipulation of the
//  selection.
class   NCBI_GUIWIDGETS_SEQTEXT_EXPORT   CTextSelHandler : public wxEvtHandler,
                                                           public ISequenceTextGraphHost
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

public:
    CTextSelHandler();
    virtual ~CTextSelHandler();

    void    SetHost(ISeqTextGeometry* pGeometry);

    void    Render(CGlPane& Pane, ERenderingOption option = eActiveState);

    /// @name IGlEventHandler implementaion
    /// @{
//    virtual void    SetPane(CGlPane* pane);
//    virtual wxEvtHandler    GetEvtHandler();
    /// @}

    // ISequenceTextGraphHost implementation
    void    STGH_GetSelectedFeatureSubtypes(CSeqTextDefs::TSubtypeVector &subtypes);
    void    STGH_GetMouseOverFeatureSubtypes(CSeqTextDefs::TSubtypeVector &subtypes);

    // colors
    void    SetColor(EColorType type, const CRgbaColor& color);

    // commands
    TSeqRange  GetSelectionLimits()    const;
    const   TRangeColl&   GetSelection() const;
    void    SetRangeSelection(const TRangeColl& C, bool bRedraw);
    void    ResetRangeSelection(bool bRedraw);
    void    ResetObjectSelection(bool b_redraw);
    void    DeSelectObject(const CObject* obj, bool b_redraw);
    void    SelectObject(const CObject* obj, bool b_redraw);

    void    ResetSelection(bool bRedraw);

    void    OnKeyDown(wxKeyEvent& event);
    void    OnKeyUp(wxKeyEvent& event);

    void    OnMousePush(wxMouseEvent &event);
    void    OnMouseDrag(wxMouseEvent &event);
    void    OnMouseRelease(wxMouseEvent &event);
protected:
    // user event handlers - translate event to signals
    int x_OnMouseMove(CGUIEvent& event, CGlPane& Pane);

    // signal handlers - invoked by event handlers
    void    x_OnStartSel(wxMouseEvent &event);
    void    x_OnChangeSelRange(wxMouseEvent &event);
    void    x_OnEndSelRange();
    void    x_OnResetAll();
    void    x_OnOpChange(wxMouseEvent& event);

    void    x_OnSelectCursor(wxMouseEvent& event);
    void    x_SetCursor();
    void    x_SetHoverPos(TSeqPos pos);

protected:
    enum    EExtState   {
        eNoExt,
        eExtRangeStart,
        eExtRangeEnd
    };
    enum    EOpType {
        eNoOp,
        eAdd,
        eRemove,
        eChange
    };

    EOpType x_GetOpTypeByEvent(wxKeyEvent& event)    const;
    EOpType x_GetOpTypeByEvent(wxMouseEvent& event)    const;
    TSeqPos x_MouseToSeqPos(wxMouseEvent& event);
    bool    x_MouseInRightMargin(wxMouseEvent& event);
    TSeqPos x_ClipPosByRange(TSeqPos Pos);

    void    x_HitTest(TSeqRange& Range, bool& bHitStart, wxMouseEvent& event);

    void    x_AddToSelection(const TSeqRange& Range);
    void    x_RemoveFromSelection(const TSeqRange& Range);

    void x_RenderRange (TSeqRange r,
                        CRgbaColor& fill_color);
    void x_RenderOpSymbol (TSeqRange r);


    void x_RenderSelectedFeatures ();


private:
    TRangeColl  m_Selection;
    TSeqRange   m_CurrRange; // range being modified
    // Object Selection
    CObjectIndex m_SelectedObjects;

    EExtState   m_ExtState; // current range extension state
    EOpType     m_OpType;   // operation type
    bool        m_bResizeCursor;

    ISeqTextGeometry* m_pGeometry;
    CRef<objects::CSeq_loc_Mapper> m_Mapper;

    CRgbaColor    m_SelColor;
    CRgbaColor    m_BorderColor;
    CRgbaColor    m_PassiveSelColor;
    CRgbaColor    m_SymbolColor;

    CGlTextureFont   m_Font;
    //CGlPane*    m_pPane;
    TSeqPos     m_HoverPos;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_SEQTEXT___TEXT_SEL_HANDLER__HPP
