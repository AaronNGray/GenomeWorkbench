/*  $Id: glfont.cpp 39087 2017-07-25 20:39:56Z falkrb $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 *    CGlutFont -- GLUT based bitmap fonts
 */


#include <ncbi_pch.hpp>
#include <gui/opengl/glfont.hpp>

#include <gui/opengl.h>

#include <math.h>

BEGIN_NCBI_SCOPE


//
// text block
//
// size:
//    1 for begin token
//    1 for size
//    1 for text length
//    5 for position (token + 4 floats)
//    5 for color (token + 4 floats)
//    N for text
//    1 for end token
//
static const size_t s_TEXTBLOCK_SIZE = 14;

vector<float> CGlFeedbackFont::EncodeText(GLfloat pos[4],
                                          const CRgbaColor& color,
                                          const char* text,
                                          size_t length)
{
    vector<float> vec;
    vec.push_back(eBeginText);

    const size_t size = size_t(s_TEXTBLOCK_SIZE + ceil(double(length) / 4));
    vec.push_back(float(size));

    vec.push_back(float(length));

    vec.push_back(ePosition);
    for (int i = 0; i < 4; ++i) {
        vec.push_back(pos[i]);
    }

    vec.push_back(eColor);
    const GLfloat* c = color.GetColorArray();
    for (int i = 0; i < 4; ++i) {
        vec.push_back(c[i]);
    }

    const char* textptr = text;

    for (size_t i = 0; i < length; ) {
        union FPackChar fpc = { 0 };
        for (size_t j = 0; j < 4 && i < length; ++j, ++i, ++textptr) {
            fpc.c[j] = *textptr;
        }
        vec.push_back(fpc.f);
    }

    vec.push_back(eEndText);
    return vec;
}


vector<float> CGlFeedbackFont::EncodeText(GLfloat pos[4],
                                          const CRgbaColor& color,
                                          const string& text)
{
    return EncodeText(pos, color, text.data(), text.size());
}


void CGlFeedbackFont::DecodeText(const vector<float>& textbuf,
                                 GLfloat* pos,
                                 GLfloat* color,
                                 string& text)
{
    vector<float>::const_iterator it = textbuf.begin();
    if (*it != eBeginText) {
        LOG_POST(Info << "expecting BEGIN_TEXT token");
        return;
    }

    // skip the first item
    //const size_t size = size_t(*++it);
    *++it;
    const size_t textlen = size_t(*++it);

    text.erase();
    // reserve a little more than needed
    text.reserve(textlen + 10);

    if (*++it != ePosition) {
        LOG_POST(Info << "expecting POSITION token");
        return;
    }
    for (int i = 0; i < 4; ++i) {
        pos[i] = *++it;
    }

    if (*++it != eColor) {
        LOG_POST(Info << "expecting COLOR token");
        return;
    }
    for (int i = 0; i < 4; ++i) {
        color[i] = *++it;
    }

    vector<float>::const_iterator end = textbuf.end() - 1;
    if (*end != eEndText) {
        LOG_POST(Info << "expecting END_TEXT token at end of text");
        return;
    }

    // i needs to be a signed type
    for (int i = int(textlen); i > 0 && it != end; i -= 4) {
        union FPackChar fpc = { *++it };
        text.append((const char*) &fpc.c, i < 4 ? i : 4);
    }
}


END_NCBI_SCOPE
