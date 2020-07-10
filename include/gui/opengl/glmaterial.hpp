#ifndef GUI_OPENGL___GL_MATERIAL__HPP
#define GUI_OPENGL___GL_MATERIAL__HPP

/*  $Id: glmaterial.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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

class NCBI_GUIOPENGL_EXPORT CGlMaterial : public CObject {
public:
    CGlMaterial()
    {
        Init();
    }

    void Init(void);
    void Apply(void);

    //
    const CRgbaColor & GetDiffuse(void)   { return m_clDiffuse;  }
    const CRgbaColor & GetSpecular(void)  { return m_clSpecular; }
    const CRgbaColor & GetAmbient(void)   { return m_clAmbient;  }
    const CRgbaColor & GetEmissive(void)  { return m_clEmissive; }
    const GLfloat  & GetShininess(void) { return m_fShininess; }

    //
    void SetDiffuse(const CRgbaColor cl)  { m_clDiffuse  = cl;  }
    void SetSpecular(const CRgbaColor cl) { m_clSpecular = cl;  }
    void SetAmbient(const CRgbaColor cl)  { m_clAmbient  = cl;  }
    void SetEmissive(const CRgbaColor cl) { m_clEmissive = cl;  }
    void SetShininess(GLfloat vl)       { m_fShininess = vl;  }

protected:
    /// diffuse color component
    CRgbaColor m_clDiffuse;

    /// specular color
    CRgbaColor m_clSpecular;

    /// ambient color component
    CRgbaColor m_clAmbient;

    /// emissive color component
    CRgbaColor m_clEmissive;

    /// sharpness of specular
    GLfloat m_fShininess;
};

inline void CGlMaterial::Apply(void)
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   m_clAmbient.GetColorArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   m_clDiffuse.GetColorArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  m_clSpecular.GetColorArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  m_clEmissive.GetColorArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &m_fShininess);
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_MATERIAL__HPP
