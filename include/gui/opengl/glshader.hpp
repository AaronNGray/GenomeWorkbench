#ifndef GUI_OPENGL___GL_SHADER__HPP
#define GUI_OPENGL___GL_SHADER__HPP

/*  $Id: glshader.hpp 38609 2017-06-02 13:12:34Z falkrb $
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
 * Authors:  Bob Falk
 *
 * File Description:
 * 
 */

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>
#include <gui/utils/matrix3.hpp>
#include <gui/utils/matrix4.hpp>
#include <gui/utils/rgba_color.hpp>

#include <gui/opengl/gltypes.hpp>
#include <gui/opengl/globject.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//////////////////////////////////////////////////////////////////////////////
/// CGlShader
///
/// A simple class to read in and create vertex and fragment shaders.
/// Note this also works (as default/built-in shader) if no shader is specified.
class NCBI_GUIOPENGL_EXPORT CGlShader : public CGlObject
{
public:
    CGlShader() 
        : m_VertexShaderId(0)
        , m_FragmentShaderId(0)
        , m_ShaderProgramId(0)
        , m_PrevProgramId(0) {}

    /// Free OpenGL resources (shader objects and program) and set ids to 0
    virtual ~CGlShader();

    /// file_name should be the root file name of a vertex and a pixel shader with
    /// the same base name, e.g. phong.vert and phong.frag.
    GLuint CreateShader(const string& file_name);

    /// create shaders from passed in strings which have the shader contents.
    GLuint CreateShader(const string& vertex_shader, const string& fragment_shader);

    /// Free OpenGL resources (shader objects and program) and set ids to 0
    void Clear();

    /// Call glUseProgram to make m_ShaderProgramId the current shader
    void MakeCurrent() const;

    /// Restore whatever the current shader was when MakeCurrent() was last called
    void Release();

    /// Returns location ID for uniform var "name". Returns -1 if program 
    /// not valid or "name" not defined.
    GLint GetUniformLocation(const GLchar *name);

    /// true if we have a non-default shader (but you can still call MakeCurrent when
    /// m_ShaderProgramId is 0, you just get default shading).
    bool IsValid() const { return (m_ShaderProgramId>0); }

    GLuint GetProgramId() const { return m_ShaderProgramId; }

    /// Return any logged errors from last call to create shaders
    const string& GetLog() const { return m_CompileLinkLog; }

protected:
    string x_GetShaderInfoLog(GLuint obj) const;
    string x_GetProgramInfoLog(GLuint obj) const;

    /// Shader IDs
    GLuint m_VertexShaderId;
    GLuint m_FragmentShaderId;

    /// Shader program ID passed to glUseProgram()
    GLuint m_ShaderProgramId;

    /// Updated everytime MakeCurrent is called so that the previous program 
    /// can be reset.
    mutable GLuint m_PrevProgramId;

    /// Error log from last compile/link.
    string m_CompileLinkLog;

};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_SHADER__HPP
