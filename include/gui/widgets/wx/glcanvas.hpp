#ifndef GUI_WX_DEMO___GL_CANVAS__HPP
#define GUI_WX_DEMO___GL_CANVAS__HPP

/*  $Id: glcanvas.hpp 43252 2019-06-05 19:21:13Z katargir $
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

#include <gui/opengl.h>

// Disable deprecated warning for AGLDrawable in wx code (MAC specific)
#if defined(__WXMAC__)  &&  NCBI_COMPILER_VERSION >= 421
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <wx/glcanvas.h>
#if defined(__WXMAC__)  &&  NCBI_COMPILER_VERSION >= 421
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
 
#include <gui/gui.hpp>
#include <gui/opengl/glpane.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CGLCanvas
class NCBI_GUIWIDGETS_WX_EXPORT CGLCanvas :
    public wxGLCanvas
{
    DECLARE_EVENT_TABLE();
public:
    CGLCanvas(wxWindow* parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0);

    virtual ~CGLCanvas();

    void OnPaint(wxPaintEvent& event);

    void DoNotUpdate() { m_DoNotUpdate = true; }

    void SetClearColor(float r, float g, float b, float a);

protected:
    void x_SetupGLContext();
    void x_CheckGlVersion();
    virtual void x_Render();
    bool m_DoNotUpdate;

    float m_ClearColor[4];

    static const int* x_GetAttribList();
};


END_NCBI_SCOPE

#endif // GUI_WX_DEMO___GL_CANVAS__HPP
