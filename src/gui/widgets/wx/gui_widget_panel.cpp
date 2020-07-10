/*  $Id: gui_widget_panel.cpp 36855 2016-11-04 16:57:25Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/gui_widget_panel.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_TABLE( CGuiWidgetPanel, wxPanel )
    EVT_CHILD_FOCUS( CGuiWidgetPanel::OnChildFocus )
    EVT_CLOSE( CGuiWidgetPanel::OnClose )
END_EVENT_TABLE()

void CGuiWidgetPanel::OnChildFocus(wxChildFocusEvent& event)
{
    if (m_WidgetHost) m_WidgetHost->WidgetActivated();
}

void CGuiWidgetPanel::OnClose(wxCloseEvent& event)
{
    x_CloseDialogView();
}

bool CGuiWidgetPanel::Show(bool show)
{
    if (!show) {
        x_CloseDialogView();
        return false;
    }
    return wxPanel::Show(show);
}

void CGuiWidgetPanel::x_CloseDialogView()
{
    if (m_Closing) 
        return;
    m_Closing = true;
    if (m_WidgetHost)
        m_WidgetHost->CloseWidget();   
}

END_NCBI_SCOPE
