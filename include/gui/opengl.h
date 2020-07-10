#ifndef GUI___OPENGL__HPP
#define GUI___OPENGL__HPP

/*  $Id: opengl.h 43515 2019-07-22 16:21:38Z katargir $
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
 *    Standard mechanism to include OpenGL headers for all platforms.
 */

/**
 * @file opengl.h
 *
 * Standard mechanism to include OpenGL headers for all platforms.
 *
 */

#include <ncbiconf.h>
#include <gui/gui_export.h>

/*
 * gl.h
 * Darwin places theirs in a non-standard location
 *
 */

#  include <GL/glew.h>

#ifdef GLEW_MX

#ifdef __cplusplus
extern "C" {
#endif

// FYI: GLEW fails to define this (sigh...)
NCBI_GUIOPENGL_EXPORT GLEWContext*  glewGetContext();

#ifdef __cplusplus
};
#endif

#endif

/*
 * glu.h
 * Darwin places theirs in a non-standard location
 */

#if defined(NCBI_OS_DARWIN)
#  include <OpenGL/glu.h>
#  include <gui/osx_api.h>
#else
#  include <GL/glu.h>
#endif


#endif  /* GUI___OPENGL__HPP */
