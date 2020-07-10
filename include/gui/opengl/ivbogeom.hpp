#ifndef GUI_OPENGL___IVBOGEOM___HPP
#define GUI_OPENGL___IVBOGEOM___HPP


/*  $Id: ivbogeom.hpp 43891 2019-09-16 13:50:00Z evgeniev $
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


#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>

#include <gui/utils/rgba_color.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class IVboGeom
{
public:
    virtual ~IVboGeom() {}

    enum EVertexFormat
    {
        kVertexFormatNone = -1,
        kVertexFormatVertex2D,
        kVertexFormatVertex3D
    };

    enum ESecondaryFormat
    {
        kSecondaryFormatNone = -1,
        kSecondaryFormatColorFloat,
        kSecondaryFormatColorUChar,
        kSecondaryFormatTexture2D,
        kSecondaryFormatTexture1D
    };

    virtual string GetName() const = 0;

    /// Render data in buffers
    virtual void Render(const float* modelView) = 0;

    virtual GLenum GetDrawMode() const = 0;
    virtual EVertexFormat GetVertexFormat() const = 0;
    virtual ESecondaryFormat GetSecondaryFormat() const = 0;

    virtual size_t GetVertexCount() const = 0;

    virtual void SetVertexBuffer2D(const vector<CVect2<float> >& data) = 0;
    virtual void GetVertexBuffer2D(vector<CVect2<float> >& data) const = 0;

    virtual void SetVertexBuffer3D(const vector<CVect3<float> >& data) = 0;
    virtual void GetVertexBuffer3D(vector<CVect3<float> >& data) const = 0;

    virtual void SetColorBuffer(const vector<CRgbaColor>& data) = 0;
    virtual void GetColorBuffer(vector<CRgbaColor>& data) const = 0;

    virtual void SetColorBufferUC(const vector<CVect4<unsigned char> >& data) = 0;
    virtual void GetColorBufferUC(vector<CVect4<unsigned char> >& data) const = 0;

    virtual void SetTexCoordBuffer1D(const vector<float>& data) = 0;
    virtual void GetTexCoordBuffer1D(vector<float>& data) const = 0;

    virtual void SetTexCoordBuffer(const vector<CVect2<float> >& data) = 0;
    virtual void GetTexCoordBuffer(vector<CVect2<float> >& data) const = 0;

private:
    friend class CRenderCommon;
    virtual void x_SetDrawMode(GLenum drawMode) = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___IVBOGEOM___HPP
