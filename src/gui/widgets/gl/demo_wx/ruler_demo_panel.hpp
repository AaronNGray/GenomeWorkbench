#ifndef GUI_WIDGETS_GL_WX_DEMO___RULER_DEMO_PANEL__HPP
#define GUI_WIDGETS_GL_WX_DEMO___RULER_DEMO_PANEL__HPP


/*  $Id: ruler_demo_panel.hpp 42131 2018-12-26 20:15:42Z katargir $
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

#include <gui/opengl/glpane.hpp>
#include <gui/widgets/wx/3dcanvas.hpp>
#include <gui/widgets/gl/ruler.hpp>

BEGIN_NCBI_SCOPE

class CRulerDemoPanel : public C3DCanvas
{
    DECLARE_EVENT_TABLE();
public:
    CRulerDemoPanel(wxWindow* parent, wxWindowID id);
    void OnSize(wxSizeEvent& event);

protected:
    virtual void x_Render();
    CGlPane  m_Port;
    CRuler   m_Ruler;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL_WX_DEMO___RULER_DEMO_PANEL__HPP
