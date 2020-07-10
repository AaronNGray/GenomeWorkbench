/*  $Id: glshader.cpp 41823 2018-10-17 17:34:58Z katargir $
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

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <gui/opengl/glshader.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>

BEGIN_NCBI_SCOPE


CGlShader::~CGlShader() 
{
    Clear();
}


GLuint CGlShader::CreateShader(const string& file_name)
{
    Clear();

    IRender& gl = GetGl();
    if (gl.GetApi() != eOpenGL20)
        return 0;

    m_CompileLinkLog = ""; 
    
    // Assume we have the root file name in 'file name' and that we will load
    // only a vertex and pixel shader.
    std::string vert_shader = file_name + ".vert";
    std::string frag_shader = file_name + ".frag";

    CFile  vert_file(vert_shader);
    CFile  frag_file(frag_shader);
   
    if (!vert_file.Exists() || !frag_file.Exists()) {     
        return 0;
    }

    CFileIO fio_vs;
    CFileIO fio_fs;

    char* buf = NULL;

    try {
        fio_vs.Open(vert_shader, CFileIO::eOpen, CFileIO::eRead);
        fio_fs.Open(frag_shader, CFileIO::eOpen, CFileIO::eRead);

        // obtain file sizes:
        GLint fsize1  = (GLint)fio_vs.GetFileSize();
        GLint fsize2  = (GLint)fio_fs.GetFileSize();
    
        size_t max_size = size_t(std::max(fsize1, fsize2));
        buf = new char[max_size];
        fio_vs.Read(buf, size_t(fsize1));
        string vshader = string(buf, fsize1);
        fio_fs.Read(buf, size_t(fsize2));
        string hshader = string(buf, fsize2);
        delete [] buf;

        return CreateShader(vshader, hshader);
    }
    catch (CException& c) {
        m_CompileLinkLog = "File IO Error reading shader: " + c.GetMsg();
        delete [] buf;

        Clear();
    }

    return 0;
}

GLuint CGlShader::CreateShader(const string& vertex_shader_str, 
                               const string& fragment_shader_str)
{
    Clear();

    IRender& gl = GetGl();
    if (gl.GetApi() != eOpenGL20)
        return 0;

    m_CompileLinkLog = "";

    m_VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    m_FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // load the vertex shader text
    const GLchar* vs_cptr = vertex_shader_str.c_str();
    GLint shader_len = (GLint)vertex_shader_str.length();
    glShaderSource(m_VertexShaderId, 1, &vs_cptr, &shader_len);

    // load the fragment shader text
    const GLchar* fs_cptr = fragment_shader_str.c_str();
    shader_len = (GLint)fragment_shader_str.length();
    glShaderSource(m_FragmentShaderId, 1, &fs_cptr, &shader_len);

    GLint status;
    glCompileShader(m_VertexShaderId);
    glGetShaderiv(m_VertexShaderId, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_CompileLinkLog += "VertexShader: \n" + 
            x_GetShaderInfoLog(m_VertexShaderId) + "\n";
        Clear();
        return 0;
    }
    glCompileShader(m_FragmentShaderId);
    glGetShaderiv(m_FragmentShaderId, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_CompileLinkLog += "Fragment Shader: \n"+ 
            x_GetShaderInfoLog(m_FragmentShaderId) + "\n";
        Clear();
        return 0;
    }

    m_ShaderProgramId = glCreateProgram();
    glAttachShader(m_ShaderProgramId,m_VertexShaderId);
    glAttachShader(m_ShaderProgramId,m_FragmentShaderId);

    glLinkProgram(m_ShaderProgramId);
    glGetProgramiv(m_ShaderProgramId, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_CompileLinkLog += "link error:\n" + 
            x_GetProgramInfoLog(m_ShaderProgramId) + "\n";       
        Clear();
        return 0;
    }

    return m_ShaderProgramId;
}

void CGlShader::Clear()
{
    IRender& gl = GetGl();
    if (gl.GetApi() != eOpenGL20)
        return;

    if (m_ShaderProgramId > 0) {
        glDeleteProgram(m_ShaderProgramId);
        m_ShaderProgramId = 0;
    }

    if (m_VertexShaderId > 0) {
        glDeleteShader(m_VertexShaderId);
        m_VertexShaderId = 0;
    }

    if (m_FragmentShaderId > 0) {
        glDeleteShader(m_FragmentShaderId);
        m_FragmentShaderId = 0;
    }
    CGlUtils::CheckGlError();
}

void CGlShader::MakeCurrent() const
{ 
    IRender& gl = GetGl();

    // Get prvious program ID so we can reset it on Release()
    GLint get_prog_id;
    glGetIntegerv(GL_CURRENT_PROGRAM, &get_prog_id);
    m_PrevProgramId = (GLuint)get_prog_id;

    gl.UseProgram(m_ShaderProgramId); 
}

void CGlShader::Release() 
{ 
    IRender& gl = GetGl();
    gl.UseProgram(m_PrevProgramId); 
}

GLint CGlShader::GetUniformLocation(const GLchar *name)
{
    if (!IsValid())
        return -1;

    return glGetUniformLocation(m_ShaderProgramId, name);
}

string CGlShader::x_GetShaderInfoLog(GLuint obj) const
{
    IRender& gl = GetGl();
    if (gl.GetApi() != eOpenGL20)
        return "";

    int infologLength = 0;
    string shader_log;
    
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    
    if (infologLength > 0)
    {
        int charsWritten  = 0;
        char* infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        shader_log = infoLog;
        free(infoLog);
    }

    return shader_log;
}

string CGlShader::x_GetProgramInfoLog(GLuint obj) const
{
    IRender& gl = GetGl();
    if (gl.GetApi() != eOpenGL20)
        return "";

    int infologLength = 0;
    string shader_log;
    
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    
    if (infologLength > 0)
    {
        int charsWritten  = 0;
        char *infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        shader_log = infoLog;
        free(infoLog);
    }

    return shader_log;
}


END_NCBI_SCOPE
