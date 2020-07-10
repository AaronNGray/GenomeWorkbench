#ifndef GUI_OPENGL___GL_FONT__HPP
#define GUI_OPENGL___GL_FONT__HPP

/*  $Id: glfont.hpp 44930 2020-04-21 17:07:30Z evgeniev $
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
 *    CGlFont   -- base class for OpenGL fonts
 *    CGlutFont -- GLUT based bitmap fonts
 */


#include <corelib/ncbiobj.hpp>

#include <gui/opengl.h>
#include <gui/opengl/gltypes.hpp>
#include <gui/utils/rgba_color.hpp>

#include <map>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//
// Interface class for all OpenGL fonts.
//

class IGlFont : public CObject
{
public:
    // text truncation types
    enum ETruncate {
        // do not attempt any kind of truncation
        eTruncate_None,

        // simply chop the string without providing any details
        eTruncate_Empty,

        // truncate and append an ellipsis
        eTruncate_Ellipsis
    };

    // metric types
    enum EMetric {
        // return the height of a capital letter
        eMetric_CharHeight,

        // return the height of all possible character, including
        // ascents and descents
        eMetric_FullCharHeight,

        // return the widths of all characters
        eMetric_AvgCharWidth,

        // return the maximum width of all characters
        eMetric_MaxCharWidth,

        // return the length of a given string without the final advance
        // (this is the entire space taken up by only this string)
        eMetric_TextWidth,

        // return the length of a given string with the final advance
        eMetric_FullTextWidth,
        
        // return the font descener for a given string
        eMetric_Descender
    };

    /// alignment modes
    enum EAlignMode {
        eAlign_HorizMask = 0x000f,
        eAlign_Left      = 0x0001,
        eAlign_HCenter   = 0x0002,
        eAlign_Right     = 0x0004,

        eAlign_VertMask  = 0x00f0,
        eAlign_Top       = 0x0010,
        eAlign_VCenter   = 0x0020,
        eAlign_Bottom    = 0x0040,

        eAlign_Center = eAlign_HCenter | eAlign_VCenter
    };
    typedef int TAlign;

    // must provide default ctor - copy ctor is private
    IGlFont(void) {}

    // destructor
    virtual ~IGlFont(void) {}

    // Print the appropriate text on the screen.  This function uses the
    // current raster position.
    virtual void TextOut(const char* text) const = 0;

    // Print some text on the screen.  This function uses a specified
    // raster position.
    virtual void TextOut(TModelUnit x, TModelUnit y, const char* text) const = 0;

    // Output text into a given rectangle using a particular alignment.
    virtual void TextOut(TModelUnit x, TModelUnit y, TModelUnit w, TModelUnit h,
                         const char* text,
                         TAlign      align = eAlign_Center,
                         ETruncate   trunc = eTruncate_Ellipsis,
                         TModelUnit  scale_x = 1.0,
                         TModelUnit  scale_y = 1.0) const = 0;

    /// This is needed by pdf but really on for gltexture font subclass since those
    /// may be either texture or (for clarity at small screen size) bitmap
    virtual bool IsBitmapFont() const { return true; }

    // Determine how much space a piece of text will occupy.  This
    // is intended *not* to include the final advance!
    virtual TModelUnit TextWidth(const char* text) const = 0;

    // determine the text height
    virtual TModelUnit TextHeight(void) const = 0;

    // query the font for certain metrics
    virtual TModelUnit GetMetric(EMetric     metric,
                                 const char* text = NULL,
                                 int         len = -1) const = 0;
};


//
// Utility class for managing font encoding / interception in feedback mode
//

class CGlFeedbackFont
{
public:

    enum
    {
        eBeginText = 0xBAAB,
        eEndText = 0xCBBC,
        ePosition = 0xFCEB,
        eColor = 0xEFBA
    };

    union FPackChar
    {
        float f;
        char c[4];
    };

    static vector<float> EncodeText(GLfloat pos[4],
                                    const CRgbaColor& color,
                                    const char* text,
                                    size_t length);

    static vector<float> EncodeText(GLfloat pos[4],
                                    const CRgbaColor& color,
                                    const string& text);

    static void DecodeText(const vector<float>& textbuf,
                           GLfloat* pos,
                           GLfloat* color,
                           string& text);
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_FONT__HPP
