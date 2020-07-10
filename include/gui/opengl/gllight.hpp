#ifndef GUI_OPENGL___GL_LIGHT__HPP
#define GUI_OPENGL___GL_LIGHT__HPP

/*  $Id: gllight.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <util/image/image.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl.h>
#include <gui/gui.hpp>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class NCBI_GUIOPENGL_EXPORT CGlLight : public CObject
{
public:
    /// 3d vector
    typedef CVect3<float> TVect;

    CGlLight()
    {
        Init();
    }

    /// initialization
    void    Init();

    /// light on/off switches
    void    On(void);
    void    Off(void);
    GLenum  IsOn(void);

    /// apply changes to OpenGL
    void Apply(void);

    /// get interface
    const GLenum   & GetID(void)        {return m_eID;}
    const CRgbaColor & GetDiffuse(void)   {return m_clDiffuse;}
    const CRgbaColor & GetSpecular(void)  {return m_clSpecular;}
    const CRgbaColor & GetAmbient(void)   {return m_clAmbient;}
    const TVect    & GetPosition(void)  {return m_vtPosition;}
    const TVect    & GetDirection(void) {return m_vtDirection;}
    GLfloat        & GetCutoff(void)    {return m_fCutoff;}
    GLfloat        & GetExponent(void)  {return m_fExponent;}

    // set interface
    void SetDiffuse(const CRgbaColor cl)  {m_clDiffuse   = cl; }
    void SetSpecular(const CRgbaColor cl) {m_clSpecular  = cl; }
    void SetAmbient(const CRgbaColor cl)  {m_clAmbient   = cl; }
    void SetPosition(const TVect vt)    {m_vtPosition  = vt; }
    void SetDirection(const TVect vt)   {m_vtDirection = vt; }
    void SetExponent(const GLfloat vl)  {m_fExponent   = vl; }
    void SetCutoff(const GLfloat vl)    {m_fCutoff     = vl; }

    void SetAttenuation(GLfloat a1, GLfloat a2, GLfloat a3)
    {
        m_fAttenuation1 = a1;
        m_fAttenuation2 = a2;
        m_fAttenuation3 = a3;
    }

    // static members
    static void     EnableLighting(void);
    static void     DisableLighting(void);
    static GLint    CountLights(void);
    static GLint    GetMaxLightsNumber(void);

protected:
    /// needed for light ID generation
    const static GLenum s_Lights[8];
    static GLint        s_LightIndex;

    /// light ID
    GLenum   m_eID;

    /// ready
    GLenum   m_eReady;

    /// on/off flag
    GLenum   m_eOn;

    /// diffuse color component
    CRgbaColor m_clDiffuse;

    /// specular color
    CRgbaColor m_clSpecular;

    /// ambient color component
    CRgbaColor m_clAmbient;

    /// light position
    TVect m_vtPosition;

    /// light direction
    TVect m_vtDirection;

    /// intensity distribution, 0 - uniform
    float m_fExponent;

    /// how intensity changes over distance
    float m_fAttenuation1;
    float m_fAttenuation2;
    float m_fAttenuation3;

    /// spread angle [0-90], 180
    float m_fCutoff;
};

inline GLenum CGlLight::IsOn(void)
{
    return m_eOn;
}

inline void CGlLight::On(void)
{
    if (m_eOn == GL_FALSE) {
        if (m_eReady == GL_FALSE){
            Apply();
        }
        glEnable(m_eID);
        m_eOn = GL_TRUE;
    }
}

inline void CGlLight::Off(void)
{
    if (m_eOn == GL_TRUE) {
        glDisable(m_eID);
        m_eOn = GL_FALSE;
    }
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_LIGHT__HPP
