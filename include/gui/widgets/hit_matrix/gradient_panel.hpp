#ifndef GUI_WIDGETS_HIT_MATRIX___SCORE_DLG__HPP
#define GUI_WIDGETS_HIT_MATRIX___SCORE_DLG__HPP

/*  $Id: gradient_panel.hpp 17877 2008-09-24 13:03:09Z dicuccio $
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

#include <gui/widgets/hit_matrix/hit_coloring.hpp>

#include <wx/panel.h>


class wxDC;

BEGIN_NCBI_SCOPE

class CGradientPanel :
        public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CGradientPanel(wxWindow* parent,
                   wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxPanelNameStr);

    void    SetParams(const SHitColoringParams* params);

    virtual wxSize DoGetBestSize() const;

    void OnPaint(wxPaintEvent& event);

protected:
    void    x_DrawNumber(wxDC& dc, double value, int left, int right,
                         int& clip_left, int& clip_right,
                         int y, wxAlignment align);
private:
    const SHitColoringParams* m_Params;
    wxFont  m_Font;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___SCORE_DLG__HPP
