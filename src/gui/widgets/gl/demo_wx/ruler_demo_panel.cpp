/*  $Id: ruler_demo_panel.cpp 42139 2018-12-27 18:32:48Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include "ruler_demo_panel.hpp"

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CRulerDemoPanel, C3DCanvas)
    EVT_SIZE(CRulerDemoPanel::OnSize)
END_EVENT_TABLE()

CRulerDemoPanel::CRulerDemoPanel(wxWindow* parent, wxWindowID id) :
    C3DCanvas(parent, id)
{
    SetBackgroundColour(wxColour(230, 230, 255));
    m_Port.SetModelLimitsRect(TModelRect(0, 0, 1000, 50));
    m_Port.SetVisibleRect(TModelRect(0, 0, 1000, 50));
    //pane.SetModelLimitsRect(TModelRect(0, 0, 50, 1000));
    //pane.SetVisibleRect(TModelRect(0, 0, 50, 1000));

    m_Ruler.SetDisplayOptions(CRuler::fShowOrigin);
    m_Ruler.SetColor(CRuler::eRuler, CRgbaColor(0.0f, 0.0f, 0.0f));
    //ruler.SetHorizontal(false);

    typedef CRuler::TAlignColl TAlignColl;
    TAlignColl coll(TAlignColl::fAllowMixedDir | TAlignColl::fKeepNormalized);
    coll.insert(CRuler::TAlignRange(-100, 0, 500, true));
    coll.insert(CRuler::TAlignRange(699, 0, 500, false));
//    coll.insert(TAlignRange(349, 999, 50, false));
//    coll.insert(TAlignRange(499, -100, 300, true));
    m_Ruler.SetMapping(coll);
}

void CRulerDemoPanel::OnSize(wxSizeEvent& event)
{
    wxSize size = event.GetSize();
    TVPRect rc_vp(0, 0, size.GetWidth() - 1 , 50 -1);
    m_Port.SetViewport(rc_vp);
    event.Skip();
}

void CRulerDemoPanel::x_Render()
{
    C3DCanvas::x_Render();
    m_Ruler.Render(m_Port);
}

END_NCBI_SCOPE
