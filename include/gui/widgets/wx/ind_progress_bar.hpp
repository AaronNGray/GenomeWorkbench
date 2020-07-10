#ifndef GUI_WIDGETS_WX___IND_PROGRESS_BAR__HPP
#define GUI_WIDGETS_WX___IND_PROGRESS_BAR__HPP

/*  $Id: ind_progress_bar.hpp 35344 2016-04-26 19:53:21Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <wx/control.h>
#include <wx/timer.h>
#include <wx/bitmap.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CIndProgressBar : public wxControl
{
public:
    CIndProgressBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, int width);

    void Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, int width);

protected:
    void OnPaint(wxPaintEvent& event);
    void OnTimer(wxTimerEvent& event);

    wxBitmap m_Bitmap;
    wxTimer  m_Timer;
    int      m_CurPos;
    int      m_BmWidth;

    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___IND_PROGRESS_BAR__HPP
