#ifndef GUI_WIDGETS_GL_WX_DEMO___GL3D_WINDOW__HPP
#define GUI_WIDGETS_GL_WX_DEMO___GL3D_WINDOW__HPP


/*  $Id: gl3d_window.hpp 17956 2008-09-30 18:41:17Z yazhuk $
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

#include <gui/widgets/wx/glcanvas.hpp>

#include <gui/opengl/glarcball.hpp>
#include <gui/opengl/gldlist.hpp>
#include <gui/opengl/gllight.hpp>
#include <gui/opengl/glmaterial.hpp>
#include <gui/opengl/glcamera.hpp>


BEGIN_NCBI_SCOPE

class CGl3dWindow : public CGLCanvas
{
    DECLARE_EVENT_TABLE();
public:
    CGl3dWindow(wxWindow* parent, wxWindowID id);

    void SetXRot(float rot);
    void SetYRot(float rot);
    void SetZRot(float rot);

    void SetPerspective();
    void SetOrtho();

protected:
    void OnSize(wxSizeEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);

    virtual void x_Render();

private:
    CGlCamera m_Camera;

    // internal event states for better processing of drag events
    enum EState {
        eNormal,
        eRotate,
        ePan,
        eZoom
    };

    // the current state of the window
    EState m_State;

    // arc ball for rotation
    CGlArcBall m_Arcball;

    // diffuse light
    CGlLight   m_Light1;

    // spot light
    CGlLight   m_Light2;

    // material
    CGlMaterial m_Material;

    /// we also combine Euler angles with this
    float m_XRot;
    float m_YRot;
    float m_ZRot;

    // center point for panning
    CVect3<float> m_CenterPoint;

    // our current zoom factor
    float m_Zoom;

    // display list for our scene
    CGlDisplayList m_Dlist;

    /// previous mouse position
    int m_PrevMouseX;
    int m_PrevMouseY;


    // handlers for event states
    void x_Rotate(long x, long y);
    void x_Zoom(long x, long y);
    void x_Pan(long x, long y);

    void x_SetupScene();
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL_WX_DEMO___GL3D_WINDOW__HPP
