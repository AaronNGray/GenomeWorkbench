#ifndef GUI_APP_GBENCH_NEW___APP_SERVICES__HPP
#define GUI_APP_GBENCH_NEW___APP_SERVICES__HPP

/*  $Id: app_services.hpp 38756 2017-06-15 18:00:11Z katargir $
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

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <gui/utils/event_handler.hpp>

#include <wx/panel.h>
#include <wx/animate.h>
#include <wx/sizer.h>


BEGIN_NCBI_SCOPE

class CAppTaskService;

///////////////////////////////////////////////////////////////////////////////
/// CAppTaskServiceSlot - a Status Bar slot that displays information about
/// tasks in Application Task Service


class CAppTaskServiceSlot : public wxPanel, public CEventHandler
{
    DECLARE_EVENT_TABLE();
    DECLARE_EVENT_MAP();
public:
    CAppTaskServiceSlot();
    ~CAppTaskServiceSlot();

    bool    Create(wxWindow* parent);

    void    OnPaint(wxPaintEvent& event);
    void    OnMouseEnter(wxMouseEvent& event);
    void    OnMouseLeave(wxMouseEvent& event);
    void    OnLeftDown(wxMouseEvent& event);

    void    SetAppTaskService(CAppTaskService* appTaskService);

protected:
    wxSize  x_GetTextAreaSize();
    string  x_StatusText(int running_n, int pending_n);

    int     x_GetPendingTasksCount();

    void    x_Update();

    void    x_OnUpdate(CEvent*);

protected:
    wxBoxSizer*     m_Sizer;
    wxSizerItem*    m_TextItem;
    wxAnimationCtrl*    m_Animation;

    CAppTaskService*    m_AppTaskService;

    wxString m_Text; // current status text

    bool m_Hot; // hot track (mouse inside)
    bool m_Idle; // no running tasks
};

END_NCBI_SCOPE;


#endif  // GUI_APP_GBENCH_NEW___APP_SERVICES__HPP

