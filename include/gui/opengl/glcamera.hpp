#ifndef GUI_OPENGL___GL_CAMPERA__HPP
#define GUI_OPENGL___GL_CAMPERA__HPP

/*  $Id: glcamera.hpp 14810 2007-08-13 12:27:15Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/opengl.h>
#include <gui/gui.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//
// class CGlCamera abstracts the options required for camera set-up.
//
// This class supports orthographic and perspective view setups, and provides a
// variety of means to control the layout.  When this class is made current,
// the values of the matrices are optionally overwritten rather than saved.
//
class NCBI_GUIOPENGL_EXPORT CGlCamera : public CObject
{
public:
    // enum controlling the type of layout we use (orthographic or perspective)
    // the default is perspective.
    enum EType {
        eOrtho,
        ePerspective
    };

    // default ctor
    CGlCamera();

    // set up the camera for rendering.  the boolean flag passed here indicates
    // whether the current matrices are to be saved.  If they are saved, they
    // must be restored using ReleaseCurrent(true)
    void MakeCurrent(bool save_matrices = false);

    // restore the previously saved projection matrices.  The default is for
    // this to be a null operation; if MakeCurrent(true) was called,
    // ReleaseCurrent(true) must also be called.
    void ReleaseCurrent(bool restore_matrices = false);

    // access the type of layout we use
    EType GetLayout(void) const;
    void SetLayout(EType type);

    //
    // access the clipping planes
    // this is specific to the kind of camera set-up we're using
    //

    //
    // Common view setup
    // The two clipping planes defining the near and far aspects of the frustum
    // are shared by perspective and orthographic views.
    GLdouble GetNearPlane  (void) const;
    GLdouble GetFarPlane   (void) const;
    void     SetNearPlane  (GLdouble);
    void     SetFarPlane   (GLdouble);

    //
    // Orthographic Setup
    // An orthographic view is an architectural view that lacks perspective.
    // Orthographic displays are most commonly used to set up 2D displays that
    // lack perspective, but they are also valid for 3D scenes in which
    // perspective hints are not required.
    //
    // Orthographic views are defined as a cube, bounded by six planes (near,
    // far, left, right, top, bottom).  The corrdinates used for these are up
    // to the user to determine; OpenGL makes no assumptions about the
    // rendering coordinates.
    GLdouble GetLeftPlane  (void) const;
    GLdouble GetRightPlane (void) const;
    GLdouble GetTopPlane   (void) const;
    GLdouble GetBottomPlane(void) const;

    void     SetLeftPlane  (GLdouble f);
    void     SetRightPlane (GLdouble f);
    void     SetTopPlane   (GLdouble f);
    void     SetBottomPlane(GLdouble f);

    //
    // Perspective setup
    // Perspective views are defined by a different set of parameters.  The
    // perspective camera setup creates a frustum (= truncated pyramid) defined
    // by the following parameters:
    //  - near and far clipping planes (from common set-up)
    //  - field of view (in degrees)
    //  - aspect ratio (width / height)
    GLdouble GetFieldOfView(void) const;
    GLdouble GetAspectRatio(void) const;

    void     SetFieldOfView(GLdouble f);
    void     SetAspectRatio(GLdouble f);

    void    Init2dDefault();
    void    Init3dDefault();

private:

    // type of viewing setup we use (default = perspective)
    EType m_Type;

    // common params
    GLdouble m_NearPlane;
    GLdouble m_FarPlane;

    // ortho params
    GLdouble m_LeftPlane;
    GLdouble m_RightPlane;
    GLdouble m_TopPlane;
    GLdouble m_BottomPlane;

    // perspective params
    GLdouble m_Fov;
    GLdouble m_Aspect;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_CAMPERA__HPP
