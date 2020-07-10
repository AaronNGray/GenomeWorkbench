#ifndef GUI_OPENGL___I3DFRAMEBUFFER___HPP
#define GUI_OPENGL___I3DFRAMEBUFFER___HPP


/*  $Id: i3dframebuffer.hpp 42756 2019-04-10 16:43:22Z katargir $
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


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class I3DTexture;

class I3DFrameBuffer
{
public:
    virtual ~I3DFrameBuffer() {}

    virtual size_t GetFrameSize() const = 0;
    virtual void SetTextureFiltering(GLint min_filter, GLint mag_filter) = 0;

    virtual void CreateFrameBuffer() = 0;
    virtual void SetClearColor(float red, float green, float blue, float alpha) = 0;
    virtual bool IsValid() = 0;
    virtual void Render(std::function<void()> renderer) = 0;

    virtual void GenerateMipMaps() = 0;

    virtual I3DTexture* Get3DTexture() = 0;
    virtual void ReleaseTexture() = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___I3DFRAMEBUFFER___HPP
