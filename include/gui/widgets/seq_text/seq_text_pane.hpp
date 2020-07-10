#ifndef __GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_PANE__HPP
#define __GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_PANE__HPP

/*  $Id: seq_text_pane.hpp 43713 2019-08-20 19:46:06Z katargir $
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

#include <gui/objutils/objects.hpp>

#include <gui/graph/axis.hpp>
#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>
#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/gl/graph_container.hpp>

#include <gui/widgets/seq_text/seq_text_defs.hpp>
#include <gui/widgets/seq_text/seq_text_ds.hpp>
#include <gui/widgets/seq_text/seq_graph.hpp>
#include <gui/widgets/seq_text/seq_tick_graph.hpp>
#include <gui/widgets/seq_text/text_sel_handler.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>

#include <gui/utils/iclipboard_handler.hpp>

BEGIN_NCBI_SCOPE

class CSeqTextWidget;

////////////////////////////////////////////////////////////////////////////////
/// class CSeqTextPane
class NCBI_GUIWIDGETS_SEQTEXT_EXPORT  CSeqTextPane
                        : public CGlWidgetPane,
                          public CEventHandler,
                          public ISeqTextGeometry
{
    DECLARE_EVENT_TABLE()
public:
    typedef CTextSelHandler::TRangeColl   TRangeColl;

    CSeqTextPane(CSeqTextWidget *parent);
    virtual ~CSeqTextPane();


    // interface for a Widget
    virtual void    Update(void);
    virtual TVPPoint GetPortSize(void);

    const   TRangeColl&   GetSelection();
    void    SetRangeSelection(const TRangeColl& C);
    void    ResetRangeSelection();
    void    ResetObjectSelection();
    void    DeSelectObject(const CObject* obj);
    void    SelectObject(const CObject* obj);
    void    ResetSelection();

    /// @}

    /// @name ISeqTextGeometry implementation
    /// @{
    virtual void             STG_Redraw();
    virtual void             STG_OnChanged();
    virtual TSeqPos          STG_GetSequenceByWindow(int x, int y);
    virtual TModelPoint      STG_GetModelPointBySourcePos(TSeqPos z);
    virtual TModelPoint      STG_GetModelPointBySequencePos(TSeqPos z, bool *found_in_source);
    virtual TSeqPos          STG_GetSourcePosBySequencePos (TSeqPos z, bool *found_in_source);
    virtual void             STG_GetVisibleRange (TSeqPos& seq_start, TSeqPos &seq_stop);
    virtual void             STG_GetLineInfo(TSeqPos &chars_in_line, TSeqPos &lines_in_seq);
    virtual objects::CScope& STG_GetScope();
    virtual const objects::CSeq_loc*  STG_GetDataSourceLoc();
    virtual TModelUnit       STG_GetTextHeight();
    virtual void             STG_ReportMouseOverPos(TSeqPos pos);
    virtual bool             STG_IsPointInRightMargin(int x, int y);
    virtual void             STG_RenderSelectedFeature(const objects::CSeq_feat& feat, const objects::CSeq_loc& mapped_loc);
    virtual void             STG_RenderMouseOverFeature(const objects::CSeq_feat& feat);
    virtual vector<CConstRef<objects::CSeq_feat> >       STG_GetFeaturesAtPosition(TSeqPos pos);
    virtual void             STG_SetSubtypesForFeature (CSeqTextDefs::TSubtypeVector &subtypes, const objects::CSeq_loc& loc, int subtype, TSeqPos start_offset, TSeqPos stop_offset);
    virtual void             STG_RenderFeatureExtras(const objects::CMappedFeat& feat);
    virtual int              STG_GetLinesInPane ();
    virtual void             STG_SetDefaultCursor();
    virtual void             STG_SetResizeCursor();


    /// @}

    /* used to determine number of rows and number of characters per row for model */
    virtual TModelPoint    CharacterSize();
    TModelRect GetPreferredModelRect ();
    TSeqPos GetScrollAdjustmentForLastLineVariations ();

    void ScrollToPosition (TSeqPos pos, bool notify = true);
    void ShowFeaturesInUpperCase();
    void ShowFeaturesInLowerCase();
    bool GetShowFeaturesInLowerCase();
    void ChooseCaseFeature(objects::CSeqFeatData::ESubtype subtype);
    int  GetCaseFeatureSubtype();
    void ReloadSettings();
    void SetFontSize (int font_size);
    int  GetFontSize ();
    bool GetShowAbsolutePosition();
    void Configure();
    void OnSize(wxSizeEvent& event);

    void x_CalculateOffsetList(CSeqTextDefs::TVariationGraphVector& variations);

    TModelUnit x_TopBlank();
    TModelUnit GetBottomRulerModelAdjustmentHeight();

    void x_OnShowPopup(wxMouseEvent &event);
    unsigned int x_GetOffsetForVariations(TModelUnit y);
    virtual void    x_Render(void);

    virtual int OnCopy(void);

protected:
    void    x_ConfigureBottomRuler();

    void    x_SetRulerText();

    /// CGlWidgetPane overridables

    //virtual void    x_OnShowPopup();


    CSeqTextWidget*       x_GetParent()    {   return m_pParent;   }
    const CSeqTextWidget* x_GetParent()  const    {   return m_pParent;   }

    virtual bool Layout(void);

    // Rendering functions
    void    x_RenderContent(void);
    void    x_RenderBottomRuler();
    void    x_RenderLeftRuler();
    void    x_RenderSelHandler();
    void    x_RenderSelHandler(CGlPane& pane);

    void    x_RenderFeatureExtras (const objects::CSeq_feat& feat, const objects::CSeq_loc& mapped_loc);
    void    x_RenderFeatureExtras (const objects::CSeq_feat& feat);
    void    x_RenderCodons (const objects::CSeq_loc& loc, objects::CCdregion_Base::EFrame frame);
    void    x_RenderCodonTick(TSeqPos cds_start, objects::ENa_strand strand);

    void    x_RecalculateOffsets (void);

    // for rendering found list
    void x_UnderlineInterval (TSeqPos start, TSeqPos stop);
    void x_RenderFoundLocations(CGlPane& pane);

    void    x_AdjustLeftRuler (CGlPane& pane);
    void    x_AdjustBottomRuler (CGlPane& pane);

    virtual int     x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y);

    CGlTextureFont *x_ChooseFontForSize(int font_size);


protected:
    // for Clipboard
    virtual int x_OnPaste(void);


    enum EHandlerArea   {
        fSequenceArea     = 0x01,
        fSubjectRuler   = 0x02,
        fQueryRuler     = 0x04,
        fSubjectGraphs  = 0x08,
        fQueryGraphs    = 0x10,
        fOther = 0x20,
        fAllAreas = fSequenceArea | fSubjectRuler | fQueryRuler | fSubjectGraphs | fQueryGraphs | fOther
    };

    CSeqTextWidget    *m_pParent; // parent widget

    CGlPane m_SequencePane;

    TRangeColl  m_Selection;    // translated selection

    // layout information
    TVPRect m_rcSequence;
    TVPRect m_rcAxes;
    TVPRect m_rcLeftRuler;
    TVPRect m_rcBottomRuler;

    CRgbaColor m_BackColor;
    CRef<CGlTextureFont> m_pSeqFont;


    CRuler  m_BottomRuler;

    CSequenceTextGraph* m_SubjectGraph;
    CSequenceTickGraph* m_SequenceTickGraph;

    CRef<CSeqTextConfig> m_Config;

private:
    // class for managing offsets due to variations
    class COffsetStart
    {
    public:
        COffsetStart(TSeqPos start_row, unsigned int cumulative_extra_rows) { m_StartRow = start_row; m_CumulativeExtraRows = cumulative_extra_rows;};
        TSeqPos m_StartRow;
        unsigned int m_CumulativeExtraRows;
    };
    typedef vector<COffsetStart> TOffsetStart;

    TOffsetStart m_OffsetList;

    TSeqPos x_GetSequenceRowFromScrollLine(TModelUnit scroll_line);
    void    x_CalculateOffsetList(CSeqTextDefs::TVariationGraphVector& variations, TOffsetStart& offset_list);

public:
    // for text search
    class CSeqTextSearch
    {
    public:
        CSeqTextSearch() 
            : m_CurrFindPos(0), m_LastSearch(kEmptyStr) { m_FoundList.clear(); }
    
        const CSeqTextDefs::TSeqPosVector& GetFoundList() const { return m_FoundList; }
        void Reset()
        {
            m_CurrFindPos = 0;
            m_LastSearch.clear();
            m_FoundList.clear();
        }
        CSeqTextDefs::TSeqPosVector m_FoundList;
        TSeqPos m_CurrFindPos;
        // string that was searched the last time
        string m_LastSearch;  

    };
  
    CSeqTextSearch m_SearchObj;

    void ResetTextSearchData() { m_SearchObj.Reset(); }
    void SetTextSearchData(const CSeqTextSearch& data) { m_SearchObj = data; }
    const CSeqTextSearch& GetTextSearchData() const { return m_SearchObj; }

    // for selection
    void    OnMousePush(wxMouseEvent &event);
    void    OnMouseDrag(wxMouseEvent &event);
    void    OnMouseRelease(wxMouseEvent &event);
protected:
    CTextSelHandler     m_SelHandler;

    // for Tooltips
public:
    // necessary for
    virtual bool    TC_NeedTooltip(const wxPoint &pt);
    virtual string  TC_GetTooltip(const wxRect &rc);

protected:
    // event handler
    CTooltipHandler     m_TooltipHandler;
};



END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_PANE__HPP
