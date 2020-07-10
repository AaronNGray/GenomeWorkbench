#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GRAPHIC_PANEL__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GRAPHIC_PANEL__HPP

/* $Id: graphic_panel.hpp 44186 2019-11-13 18:38:48Z filippov $
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
 * Author:  Roman Katargin
 *
 */

 /**
 * File Description:
 */

#include <gui/gui.hpp>

#include <wx/panel.h>
#include <wx/timer.h>
#include <wx/aui/aui.h>

#include <objects/genomecoll/GC_Assembly.hpp>


BEGIN_NCBI_SCOPE

class CSeqGraphicWidget;
class CRichTextCtrl;

#define SYMBOL_CGRAPHICPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGRAPHICPANEL_TITLE _("Text Panel")
#define SYMBOL_CGRAPHICPANEL_IDNAME ID_CGRAPHICPANEL
#define SYMBOL_CGRAPHICPANEL_SIZE wxDefaultSize
#define SYMBOL_CGRAPHICPANEL_POSITION wxDefaultPosition

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGraphicPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()

public:
    CGraphicPanel(wxWindow* parent,
        wxWindowID id = SYMBOL_CGRAPHICPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CGRAPHICPANEL_POSITION,
        const wxSize& size = SYMBOL_CGRAPHICPANEL_SIZE,
        long style = SYMBOL_CGRAPHICPANEL_STYLE);
    ~CGraphicPanel();

    enum {
        ID_CGRAPHICPANEL = 10005,
        ID_WIDGET,
        ID_TB_AssemblyLabel,
        ID_TB_AssemblyList,
        ID_TB_Help,
        ID_TB_SearchTerm,
        ID_TB_SearchButton,
        ID_TB_SearchModeLabel,
        ID_TB_SearchModeCtrl
    };

    void Init();

    void CreateControls();
    void CreateToolbar();

    virtual bool ProcessEvent(wxEvent &event);

    CSeqGraphicWidget* GetWidget() { return m_SeqWidget; }

private:
    void OnTimer(wxTimerEvent& event);
    void OnConfigureTracks(wxCommandEvent& event);
    void OnAssemblySelected(wxCommandEvent& event);
    void OnHelpClick(wxCommandEvent& event);

    void OnSearchTermCtrlEnter( wxCommandEvent& event );
    void OnFindNextClick( wxCommandEvent& event );

    void x_CreateToolbar();
    bool x_CheckLoaded();

    void x_FindText();

    wxTimer            m_Timer;
    wxAuiManager       m_AuiManager;
    CSeqGraphicWidget* m_SeqWidget;

    CRichTextCtrl* m_FindTextCtrl;
    wxChoice* m_FindModeCtrl;

    bool m_AssemblyLoaded;
    vector<CRef<objects::CGC_Assembly> > m_Assemblies;
    string m_CurrentAssembly;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___GRAPHIC_PANEL__HPP
