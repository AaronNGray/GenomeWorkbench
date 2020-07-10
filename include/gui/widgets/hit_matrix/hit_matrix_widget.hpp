#ifndef GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_WIDGET__HPP
#define GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_WIDGET__HPP

/*  $Id: hit_matrix_widget.hpp 42108 2018-12-20 20:35:52Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_renderer.hpp>

#include <gui/widgets/hit_matrix/hit_coloring.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_pane.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_ds.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_renderer.hpp>

#include <objects/seqloc/Seq_loc.hpp>

#include <gui/utils/command.hpp>

#include <gui/opengl/gldlist.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <objects/seqalign/Seq_align_set.hpp>


class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;

/// Definitions for Hit Matrix Widget commands
///
enum    EHitMatrixCommands {
    eCmdChooseSeq = eBaseCmdLast + 200,
    eCmdColorByScore,
    eCmdDisableColoring,
    eCmdSetupGraphs
};

////////////////////////////////////////////////////////////////////////////////
/// class CHitMatrixWidget

class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT CHitMatrixWidget :
        public CGlWidgetBase
{
    DECLARE_EVENT_TABLE()
public:
    typedef list<CConstRef<objects::CSeq_id> >   TIdRefList;
    typedef CHitMatrixPane::TRangeColl  TRangeColl;

    static  void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    CHitMatrixWidget(wxWindow* parent,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxTAB_TRAVERSAL);
    virtual ~CHitMatrixWidget();

    virtual CGlPane&    GetPort();
    virtual const CGlPane&    GetPort() const;

    virtual void SetDataSource(IHitMatrixDataSource* p_ds);

    virtual IHitMatrixDataSource*  GetDS();
    virtual void    SetScaleX(TModelUnit scale_x, const TModelPoint& point);

    virtual CHitMatrixRenderer& GetRenderer();

    /// @name Command and Event handlers
    /// @{
    void    OnContextMenu(wxContextMenuEvent& event);

    void    OnZoomSelection(wxCommandEvent& event);
    void    OnUpdateZoomSelection(wxUpdateUIEvent& event);
    void    OnZoomToHits(wxCommandEvent& event);
    void    OnZoomToSelectedHitElems(wxCommandEvent& event);
    void    OnUpdateZoomToSelectedHitElems(wxUpdateUIEvent& event);
    void    OnSetEqualScale(wxCommandEvent& event);

    void    OnChooseSeq(wxCommandEvent& event);
    void    OnColorByScore(wxCommandEvent& event);
    void    OnDisableColoring(wxCommandEvent& event);
    void    OnUpdateDisableColoring(wxUpdateUIEvent& event);
    void    OnSetupGraphs(wxCommandEvent& event);
    void    OnResetSelection(wxCommandEvent& event);

    void OnSavePdf(wxCommandEvent& /* evt */);
    void OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& event);

    /// @}

    const TRangeColl&   GetSubjectRangeSelection() const;
    const TRangeColl&   GetQueryRangeSelection() const;
    void   SetSubjectRangeSelection(const TRangeColl& segs);
    void   SetQueryRangeSelection(const TRangeColl& segs);

    void SetQueryVisibleRange(const TSeqRange& range);
    void SetSubjectVisibleRange(const TSeqRange& range);

    void    ResetObjectSelection();
    void    GetObjectSelection(TConstObjects& objs) const;
    void    SetObjectSelection(const vector<const objects::CSeq_align*> sel_aligns);

protected:
    // CGlWidgetBase overridables
    virtual void    x_CreatePane();
    virtual CGlWidgetPane*  x_GetPane();

    virtual void    x_SetPortLimits();
    virtual void    x_ZoomToHits();

    virtual void    x_Update();
    virtual void    x_UpdatePane();

    virtual void    x_ClearScoreToParamsMap();

protected:
    CGlPane m_Port;

    CIRef<IHitMatrixDataSource> m_DataSource;
    auto_ptr<CHitMatrixPane>   m_MatrixPane;

    typedef map<string, CRef<SHitColoringParams> >    TScoreToParamsMap;
    TScoreToParamsMap     m_ScoreToParams;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_WIDGET__HPP
