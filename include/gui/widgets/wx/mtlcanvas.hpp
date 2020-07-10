#ifndef GUI_WIDGETS_WX__MTL_CANVAS_HPP
#define GUI_WIDGETS_WX__MTL_CANVAS_HPP

/*  $Id: mtlcanvas.hpp 44083 2019-10-23 15:18:44Z katargir $
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

#include <gui/gui.hpp>

#include <wx/window.h>

BEGIN_NCBI_SCOPE

#define MTLCanvasName wxT("MTLCanvas")

class NCBI_GUIWIDGETS_WX_EXPORT CMTLCanvas : public wxWindow
{
public:
    CMTLCanvas(wxWindow *parent,
               wxWindowID winId,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = MTLCanvasName);

    bool Create(wxWindow *parent,
                wxWindowID winId = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = MTLCanvasName);

    virtual ~CMTLCanvas();

    void Redraw();

    void SetClearColor(float r, float g, float b, float a);

protected:
    virtual void x_Render() {}

    void x_SetupGLContext() {} 

    wxDECLARE_CLASS(CMTLCanvas);

    float m_ClearColor[4];

    static int m_InstCount;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX__MTL_CANVAS_HPP
