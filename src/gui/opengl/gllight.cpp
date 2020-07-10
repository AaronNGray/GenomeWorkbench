/*  $Id: gllight.cpp 15168 2007-10-15 19:11:34Z dicuccio $
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

#include <ncbi_pch.hpp>
#include <gui/opengl/gllight.hpp>

BEGIN_NCBI_SCOPE


GLint CGlLight::s_LightIndex = 0;
const GLenum CGlLight::s_Lights[8]  = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
                                       GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};

void CGlLight::EnableLighting(void)
{
    glEnable(GL_LIGHTING);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

void CGlLight::DisableLighting(void)
{
    glDisable(GL_LIGHTING);
}

GLint CGlLight::CountLights(void)
{
    return s_LightIndex;
}

GLint CGlLight::GetMaxLightsNumber(void)
{
    return sizeof(s_Lights) / sizeof(GLenum);
}


// default initialization
void CGlLight::Init(void)
{
    if (s_LightIndex <= GetMaxLightsNumber()){
        m_eID = s_Lights[s_LightIndex++];
        m_clDiffuse.Set(1.0f, 1.0f, 1.0f, 1.0f);
        m_clSpecular.Set(1.0f, 1.0f, 1.0f, 1.0f);
        m_clAmbient.Set(0.0f, 0.0f, 0.0f, 1.0f);
        m_vtPosition = TVect(0.0f, 0.0f, 1.0f);
        m_vtDirection = TVect(0.0f, 0.0f, 1.0f);
        m_fExponent = 0.0f;
        m_fCutoff   = 180.0f;
        m_fAttenuation1 = 1.0f;
        m_fAttenuation2 = 0.0f;
        m_fAttenuation3 = 0.0f;
        m_eReady = GL_FALSE;
        m_eOn    = GL_FALSE;
    }
    else {
        NCBI_THROW(CException, eUnknown,
            "Attempt to create light failed: number of lights exceeded");
    }

}

// apply to OpenGL
void CGlLight::Apply(void)
{
    EnableLighting();
    glLightfv(m_eID, GL_POSITION, m_vtPosition.GetData());
    glLightfv(m_eID, GL_DIFFUSE,  m_clDiffuse.GetColorArray());
    glLightfv(m_eID, GL_SPECULAR, m_clSpecular.GetColorArray());
    glLightfv(m_eID, GL_AMBIENT,  m_clAmbient.GetColorArray());
    glLightfv(m_eID, GL_SPOT_DIRECTION,  m_vtDirection.GetData());
    glLightfv(m_eID, GL_SPOT_CUTOFF,  &m_fCutoff);
    glLightfv(m_eID, GL_SPOT_EXPONENT,  &m_fExponent);
    glLightfv(m_eID, GL_CONSTANT_ATTENUATION,  &m_fAttenuation1);
    glLightfv(m_eID, GL_LINEAR_ATTENUATION,    &m_fAttenuation2);
    glLightfv(m_eID, GL_QUADRATIC_ATTENUATION, &m_fAttenuation3);
    m_eReady = GL_TRUE;
}

END_NCBI_SCOPE
