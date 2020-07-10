#ifndef GUI_OPENGL___I3DTEXTURE___HPP
#define GUI_OPENGL___I3DTEXTURE___HPP


/*  $Id: i3dtexture.hpp 43126 2019-05-17 18:16:36Z katargir $
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


#include <gui/opengl.h>
#include <gui/gui.hpp>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CImage;

//
// class CGlTexture defines a specialization of CImage that is useful for
// textures.
//
// This class holds a CImage as well as its texture information.
//

class I3DTexture
{
public:
    virtual ~I3DTexture() {}

    virtual void MakeCurrent() = 0;
    virtual void Unload() = 0;

    virtual bool IsValid(void) const = 0;

    virtual void Load() = 0;

    virtual void SetFilterMin(GLenum f) = 0;
    virtual void SetFilterMag(GLenum f) = 0;

    virtual void SetWrapS(GLenum e) = 0;
    virtual void SetWrapT(GLenum e) = 0;

    virtual void SetTexEnv(GLenum e) = 0;

    virtual void SetParams() = 0;

    virtual CImage* GenerateImage() = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___I3DTEXTURE___HPP
