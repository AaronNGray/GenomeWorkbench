#ifndef GUI_OPENGL___GL_FONT_UTILS__HPP
#define GUI_OPENGL___GL_FONT_UTILS__HPP

/*  $Id: glfontutils.hpp 17876 2008-09-24 12:55:42Z dicuccio $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */


#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/glpane.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

void TextOutAligned(
    CGlBitmapFont& font,
    TModelUnit x, TModelUnit y, const char* text, IGlFont::TAlign align,
    TModelUnit scale_x, TModelUnit scale_y
);

inline void TextOutAligned(
    CGlPane& pane, CGlBitmapFont& font,
    TModelUnit x, TModelUnit y, const char* text, IGlFont::TAlign align = IGlFont::eAlign_Center
){
    TextOutAligned( font, x, y, text, align, pane.GetScaleX(), pane.GetScaleY() );
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_FONT_UTILS__HPP
