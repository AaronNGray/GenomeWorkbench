#ifndef GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_WIDGET__HPP
#define GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_WIDGET__HPP

/*  $Id: seq_text_widget.hpp 42108 2018-12-20 20:35:52Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

#include <gui/utils/command.hpp>

#include <gui/opengl/gldlist.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <gui/widgets/seq_text/seq_text_pane.hpp>
#include <gui/widgets/seq_text/seq_text_ds.hpp>


BEGIN_NCBI_SCOPE

class CSeqTextPane;
class CMenuItem;

////////////////////////////////////////////////////////////////////////////////
/// class ISeqTextWidgetHost

class ISeqTextWidgetHost
{
public:
    virtual ~ISeqTextWidgetHost() { }

    virtual void        STWH_ChangeConfig() = 0;
    virtual void        STWH_ReportMouseOverPos(TSeqPos pos) = 0;
};



class CSeqTextDataSource;

/// Definitions for Seq Text Widget commands
///
enum    ESeqTextCommands {
    eCmdPrint = eBaseCmdLast + 199,
    eCmdEditListBegin = eBaseCmdLast + 200,
    eCmdEditListEnd = eBaseCmdLast + 224,
    eCmdDeleteListBegin = eBaseCmdLast + 225,
    eCmdDeleteListEnd = eBaseCmdLast + 249,
    eCmdEditSeq = eBaseCmdLast + 250,
    eCmdDeleteSeq = eBaseCmdLast + 251
};

////////////////////////////////////////////////////////////////////////////////
/// class CSeqTextWidget

class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextWidget : public CGlWidgetBase
{
public:
    typedef list<CConstRef<objects::CSeq_id> >   TIdRefList;
    typedef CSeqTextPane::TRangeColl  TRangeColl;

    CSeqTextWidget(wxWindow* parent, wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~CSeqTextWidget();

    virtual CSeqTextDataSource*  GetDS();

    void AdjustModelForSequenceAndWindowSize();
    void ReportMouseOverPos(TSeqPos pos);

    virtual void SetDataSource(CSeqTextDataSource &ds);

    // CGLWidget overridables
    virtual CGlPane&    GetPort();
    virtual const CGlPane&    GetPort() const;
    virtual void    SetScaleX(TModelUnit scale_x, const TModelPoint& point);

    // menu items
    void Configure( wxCommandEvent& event );
    void OnShowPopup();

    // for external objects controlling the scroll position of the widget
    void   ScrollToSourcePosition(TSeqPos pos, bool notify = true);
    void   ScrollToSequencePosition(TSeqPos pos, bool notify = true);
    void   ScrollToPosition(TSeqPos pos, bool notify = true);
    void   ScrollToText (const string& fragment, TSeqPos start_search);
    
    int  GetPortScrollValue();
    void UpdateLastSourcePos();

    // for configuring the widget
    void ChooseCaseFeature(objects::CSeqFeatData::ESubtype subtype);
    int  GetCaseFeatureSubtype();
    void SetFontSize(int font_size);
    int  GetFontSize();
    void ShowFeaturesInLowerCase(bool do_lower);
    bool GetShowFeaturesInLowerCase ();

    // for communicating changes to the view
    void SetHost(ISeqTextWidgetHost* pHost);
    void ChangeConfig();

    // selection
    const TRangeColl&   GetSelection() const;
    void   SetRangeSelection(const TRangeColl& segs);
    void   ResetObjectSelection ();
    void   SelectObject(const CObject* obj);

    CSeqTextPane&   GetPane() {return *m_SequencePane;}
protected:
    // scrollbar handling
    int             x_GetVScrollMax ();
    TSeqPos         x_GetScrollValueForSourcePos (TSeqPos source_pos);
    virtual void    x_UpdateScrollbars();
    virtual void    x_OnScrollY(int pos);
    virtual void    x_OnScrollX(int pos);
    void     x_MakePortMatchScrollValue (int pos);
    TSeqPos  x_GetSourcePosForPortScrollValue();
    void     x_SetScrollPosition (int scroll_pos);

    // event handler for menu item
    void    OnScrollToSelection(wxCommandEvent& event);
    void    OnCopySelection(wxCommandEvent& event);
    void    OnClearSelection(wxCommandEvent& event);
    void    OnHasSelection(wxUpdateUIEvent& event);
    void    OnDisableCommands(wxUpdateUIEvent& event);

    void OnUpdateSettings(wxUpdateUIEvent& event);

    // selection
    void   DeSelectObject(const CObject* obj);

    objects::CSeq_loc* GetVisibleRange();



protected:
    // CGlWidgetBase overridables
    virtual void    x_CreatePane();
    virtual CGlWidgetPane*  x_GetPane();

    virtual void    x_SetPortLimits();

    virtual void    x_Update();
    virtual void    x_UpdatePane();

protected:
    CRef<CSeqTextDataSource> m_DataSource;
    auto_ptr<CSeqTextPane>   m_SequencePane;
    TSeqPos                  m_PopupMenuSourcePos;
    CGlPane m_Port;

private:
    ISeqTextWidgetHost* m_pHost;

    TSeqPos m_LastSourcePos;
    // necessary because this class handles events
    DECLARE_EVENT_TABLE()
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_SEQ_TEXT___SEQ_TEXT_WIDGET__HPP
