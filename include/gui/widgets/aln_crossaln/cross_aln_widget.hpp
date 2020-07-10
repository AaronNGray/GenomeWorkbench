#ifndef GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_WIDGET__HPP
#define GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_WIDGET__HPP

/*  $Id: cross_aln_widget.hpp 25991 2012-06-21 18:51:29Z falkrb $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *   Cross alignment view widget
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_crossaln/cross_aln_ds.hpp>

#include <util/range_coll.hpp>

#include <gui/gui.hpp>
#include <gui/utils/command.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <wx/scrolbar.h>
#include <wx/panel.h>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqloc/Seq_loc.hpp>


class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;

class CCrossAlnPane;

/// Definitions for Cross Alignment Widget commands
enum ECrossAlignCommands {
    eCmdChooseSeq = eBaseCmdLast + 210,

    eCmdZoomInQuery,
    eCmdZoomOutQuery,
    eCmdZoomAllQuery,
    eCmdZoomSeqQuery,

    eCmdZoomInSubject,
    eCmdZoomOutSubject,
    eCmdZoomAllSubject,
    eCmdZoomSeqSubject
};

enum {
    ID_GLCHILDPANE = wxID_HIGHEST + 300,
    ID_SCROLLBAR_Q,
    ID_SCROLLBAR_S
};

class NCBI_GUIWIDGETS_ALNCROSSALN_EXPORT CCrossAlnWidget :
                    public CGlWidgetBase
{
    DECLARE_EVENT_TABLE()

public:
    typedef CRangeCollection<TSeqPos>   TRangeColl;

    static void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    CCrossAlnWidget(wxWindow* parent,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxTAB_TRAVERSAL);

    virtual ~CCrossAlnWidget();

    virtual CGlPane& GetPort();
    virtual const CGlPane& GetPort() const;


    void SetDataSource(ICrossAlnDataSource* ds);
    //ICrossAlnDataSource*  GetDS();

    void OnChooseSeq(wxCommandEvent& event);
    void OnColorByScore(wxCommandEvent& event);
    void OnColorBySegments(wxCommandEvent& event);

    // Datasource changed
    void Update();

    void OnSize(wxSizeEvent& event);
    void OnScrollQ(wxScrollEvent& event);
    void OnScrollS(wxScrollEvent& event);

    void OnContextMenu(wxContextMenuEvent& event);

    // All
    void OnZoomAll   (wxCommandEvent& event);
    void OnZoomIn    (wxCommandEvent& event);
    void OnZoomOut   (wxCommandEvent& event);
    void OnZoomToSeq (wxCommandEvent& event);
    // Query
    void OnZoomAll_Q   (wxCommandEvent& event);
    void OnZoomIn_Q    (wxCommandEvent& event);
    void OnZoomOut_Q   (wxCommandEvent& event);
    void OnZoomToSeq_Q (wxCommandEvent& event);

    // Subject
    void OnZoomAll_S   (wxCommandEvent& event);
    void OnZoomIn_S    (wxCommandEvent& event);
    void OnZoomOut_S   (wxCommandEvent& event);
    void OnZoomToSeq_S (wxCommandEvent& event);

    // Selection
    void OnZoomSelection(wxCommandEvent& event);
    void OnUpdateZoomSelection(wxUpdateUIEvent& event);

    /// @name Visible Range API
    /// @{
    TSeqRange GetSubjectVisibleRange() const;
    void SetSubjectVisibleRange(const TSeqRange& range);
    TSeqRange GetQueryVisibleRange() const;
    void SetQueryVisibleRange(const TSeqRange& range);
    /// @}

    /// @name Selection API
    /// @{
    const TRangeColl&    GetSubjectRangeSelection() const;
    const TRangeColl&    GetQueryRangeSelection() const;
    void    SetSubjectRangeSelection(const TRangeColl& coll);
    void    SetQueryRangeSelection(const TRangeColl& coll);

    void    ResetObjectSelection();
    void    GetObjectSelection(TConstObjects& objs) const;
    void    SetObjectSelection(const vector<const objects::CSeq_align*> sel_aligns);
    /// @}


    void    UpdateScrollbars();
protected:
    /// creates Pane, Scrollbars and other child widgets, called from Create()
    virtual void    x_CreateControls();

    // CGlWidgetBase overridables
    virtual void    x_CreatePane();
    virtual CGlWidgetPane*  x_GetPane();

    virtual void    x_Update();
    virtual void    x_UpdatePane();

    virtual void    x_SetPortLimits();

    auto_ptr<CCrossAlnPane> m_CrossPane;
    CGlPane m_Port;
};



END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_WIDGET__HPP */
