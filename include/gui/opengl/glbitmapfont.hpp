#ifndef GUI_OPENGL___GLBITMAPFONT__HPP
#define GUI_OPENGL___GLBITMAPFONT__HPP

/*  $Id: glbitmapfont.hpp 42673 2019-04-01 19:18:57Z katargir $
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

#include <gui/opengl/glfont.hpp>

#include <set>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

//
// Stand-in for GLUT fonts.  This uses glBitmap directly.
//
class NCBI_GUIOPENGL_EXPORT CGlBitmapFont : public IGlFont
{
public:
    enum EFontSize {
        eFontSize_6  = 6,
        eFontSize_8  = 8,
        eFontSize_10 = 10,
        eFontSize_12 = 12,
        eFontSize_14 = 14,
        eFontSize_18 = 18,
        eFontSize_20 = 20,
        eFontSize_24 = 24
    };

    enum EFontFace {
        eFontFace_Bitmap     = 0x0100,
        eFontFace_Helvetica  = 0x0200,
        eFontFace_Lucida     = 0x0300,
        eFontFace_Courier    = 0x0400,
        eFontFace_Clean      = 0x0500,
        eFontFace_Fixed      = 0x0600,
        eFontFace_TimesRoman = 0x0700
    };

    enum EFontRotate {
        eFontRotate_0,
        eFontRotate_90_Cap,  // new baseline along cap height
        eFontRotate_90_Base, // original base line
        eFontRotate_180_Cap,
        eFontRotate_180_Base,
        eFontRotate_270_Cap,
        eFontRotate_270_Base,
        eFontRotate_90 = eFontRotate_90_Cap,
        eFontRotate_180 = eFontRotate_180_Cap,
        eFontRotate_270 = eFontRotate_270_Base
    };

    enum EFont {
        // fixed size fonts
        // here, size doesn't map 1:1
        eBitmap5x7   = eFontFace_Bitmap |  8, //eFontSize_8,
        eBitmap8x13  = eFontFace_Bitmap | 12, //eFontSize_12,
        eBitmap9x15  = eFontFace_Bitmap | 14, //eFontSize_14,
        eBitmap10x20 = eFontFace_Bitmap | 18, //eFontSize_18,
        eBitmap12x24 = eFontFace_Bitmap | 24, //eFontSize_24,

        // helvetica
        eHelvetica6  = eFontFace_Helvetica |  6, //eFontSize_6,
        eHelvetica8  = eFontFace_Helvetica |  8, //eFontSize_8,
        eHelvetica10 = eFontFace_Helvetica | 10, //eFontSize_10,
        eHelvetica12 = eFontFace_Helvetica | 12, //eFontSize_12,
        eHelvetica14 = eFontFace_Helvetica | 14, //eFontSize_14,
        eHelvetica18 = eFontFace_Helvetica | 18, //eFontSize_18,
        eHelvetica20 = eFontFace_Helvetica | 20, //eFontSize_20,
        eHelvetica24 = eFontFace_Helvetica | 24, //eFontSize_24,

        // lucida
        eLucida6  = eFontFace_Lucida |  6, //eFontSize_6,
        eLucida8  = eFontFace_Lucida |  8, //eFontSize_8,
        eLucida10 = eFontFace_Lucida | 10, //eFontSize_10,
        eLucida12 = eFontFace_Lucida | 12, //eFontSize_12,
        eLucida14 = eFontFace_Lucida | 14, //eFontSize_14,
        eLucida18 = eFontFace_Lucida | 18, //eFontSize_18,
        eLucida20 = eFontFace_Lucida | 20, //eFontSize_20,
        eLucida24 = eFontFace_Lucida | 24, //eFontSize_24,

        // courier
        eCourier6  = eFontFace_Courier |  6, //eFontSize_6,
        eCourier8  = eFontFace_Courier |  8, //eFontSize_8,
        eCourier10 = eFontFace_Courier | 10, //eFontSize_10,
        eCourier12 = eFontFace_Courier | 12, //eFontSize_12,
        eCourier14 = eFontFace_Courier | 14, //eFontSize_14,
        eCourier18 = eFontFace_Courier | 18, //eFontSize_18,
        eCourier20 = eFontFace_Courier | 20, //eFontSize_20,
        eCourier24 = eFontFace_Courier | 24, //eFontSize_24,

        // clean
        eClean6  = eFontFace_Clean |  6, //eFontSize_6,
        eClean8  = eFontFace_Clean |  8, //eFontSize_8,
        eClean10 = eFontFace_Clean | 10, //eFontSize_10,
        eClean12 = eFontFace_Clean | 12, //eFontSize_12,
        eClean14 = eFontFace_Clean | 14, //eFontSize_14,
        eClean18 = eFontFace_Clean | 18, //eFontSize_18,
        eClean20 = eFontFace_Clean | 20, //eFontSize_20,
        eClean24 = eFontFace_Clean | 24, //eFontSize_24,

        // fixed
        eFixed6  = eFontFace_Fixed |  6, //eFontSize_6,
        eFixed8  = eFontFace_Fixed |  8, //eFontSize_8,
        eFixed10 = eFontFace_Fixed | 10, //eFontSize_10,
        eFixed12 = eFontFace_Fixed | 12, //eFontSize_12,
        eFixed14 = eFontFace_Fixed | 14, //eFontSize_14,
        eFixed18 = eFontFace_Fixed | 18, //eFontSize_18,
        eFixed20 = eFontFace_Fixed | 20, //eFontSize_20,
        eFixed24 = eFontFace_Fixed | 24, //eFontSize_24,

        // Times Roman
        eTimesRoman6  = eFontFace_TimesRoman |  6, //eFontSize_6,
        eTimesRoman8  = eFontFace_TimesRoman |  8, //eFontSize_8,
        eTimesRoman10 = eFontFace_TimesRoman | 10, //eFontSize_10,
        eTimesRoman12 = eFontFace_TimesRoman | 12, //eFontSize_12,
        eTimesRoman14 = eFontFace_TimesRoman | 14, //eFontSize_14,
        eTimesRoman18 = eFontFace_TimesRoman | 18, //eFontSize_18,
        eTimesRoman20 = eFontFace_TimesRoman | 20, //eFontSize_20,
        eTimesRoman24 = eFontFace_TimesRoman | 24  //eFontSize_24
    };

    // default constructor.  This creates a font for the 8x13 bitmap font
    // face
    CGlBitmapFont(void);

    // create a bitmap font with a particular face in mind
    CGlBitmapFont(EFont font);

    //CGlBitmapFont(const CGlBitmapFont&);
    CGlBitmapFont& operator= (const CGlBitmapFont&);

    // destructor
    ~CGlBitmapFont();

    // set the font to one of the predefined options
    void    SetFont(EFont font);
    EFont   GetFont() const;
    void        SetFontFace(EFontFace face);
    EFontFace   GetFontFace() const;
    void        SetFontSize(EFontSize size);
    EFontSize   GetFontSize() const;

    void SetFontRotate(EFontRotate rot);
    EFontRotate GetFontRotate(void) const;

    void SetCondensed(bool condensed);
    bool IsCondensed(void) const;

    // Dump some text somewhere
    virtual void TextOut(const char* text) const;
    virtual void TextOut(TModelUnit x, TModelUnit y, const char* text) const;

    // Output text into a given rectangle using a particular alignment.
    // The alignment is a bitmask of FLTK's Fl_Align enums.  The only
    // distinction is that text is *always* inside. Alignment is specified
    // reative to the text
    void TextOut(TModelUnit x1, TModelUnit y1, TModelUnit x2, TModelUnit y2, const char* text,
                 TAlign align = eAlign_Center,
                 ETruncate trunc = eTruncate_Ellipsis,
                 TModelUnit scale_x = 1.0, TModelUnit scale_y = 1.0) const;

    string Truncate(const char*, TModelUnit w, ETruncate trunc = eTruncate_Ellipsis) const;
    string Truncate(const string& str, TModelUnit w, ETruncate trunc = eTruncate_Ellipsis) const;

    /// prints array of characters in positions (x + i*dx, y + i*dy) where
    /// "i" is index of a character in the given string
    void    ArrayTextOut(TModelUnit x, TModelUnit y, TModelUnit dx, TModelUnit dy,
                         const char* text,
                         const vector<CRgbaColor*>* colors = NULL,
                         TModelUnit scale_x = 1.0f,
                         TModelUnit scale_y = 1.0f) const;

    // Determine how much space a piece of text will occupy
    virtual TModelUnit TextWidth (const char* text) const;
    virtual TModelUnit TextWidth (const char* text, int n) const;
    virtual TModelUnit TextHeight(void) const;
    virtual TModelUnit GetFontDescender() const;
    TModelUnit   GetAdvance(char c) const;

    // generic metric retrieval
    virtual TModelUnit GetMetric(EMetric metric, const char* text_start = NULL,
                            int len = -1) const;

    // funtions below provide conversion of font parameters to strings and
    // back. This can be handy for representing them in GUI.
    static string       FaceToString(EFontFace face);
    static EFontFace    FaceFromString(const string& str);

    static string       SizeToString(EFontSize size);
    static EFontSize    SizeFromString(const string& str);
    static EFontSize    SizeFromInt(int i);

    // saves and restores font face and size in(from) a string
    string  ToString() const;
    void    FromString(const string& value);

    // GetAllFaces() and GetAllSizes() fill given container with strings
    // repesenting all faces/sizes available
    static void     GetAllFaces(vector<string>& faces);
    static void     GetAllSizes(vector<string>& sizes);

protected:
    const void* GetFontPtr(const CGlBitmapFont::EFontRotate rot) const;
    const void* GetFontPtr() const;

private:
    // Truncate a string for display.  This returns the maximum
    // number of characters that can fit into the given width.  If the
    // parameter 'str' is not null, the string will be filled out with
    // the truncated text.
    int x_Truncate(const char* text, TModelUnit w, ETruncate trunc,
                   string* str = NULL) const;

private:
    EFont m_Font;   /// The font  - combination of face and size

    EFontRotate m_FontRotate;   /// rotation state of the font

    bool m_Condensed; /// render condensed
};


/// temporary placed here, later should be moved to the appropriate location
class NCBI_GUIOPENGL_EXPORT CTextUtils
{
public:
    static string   FormatSeparatedNumber(int number, bool b_postfix = false);

    /// returns number of characters in symbolic representation of the given
    /// number including non-digit separators ( 1,234 or 1,234,567)
    static int      GetCharsCount(int number);

    /// returns minimal space sufficient to render any number
    /// in the range [0, max_num]. It is assumed that text is symbolic
    /// representation of a number is produced by FormatSeparatedNumber().
    static TModelUnit   GetMaxWidth(int max_num, const CGlBitmapFont& font);

    // Set raster position safely - normally if the raster position is outside
    // of the window boundary it is invalid and no text is displayed - even
    // subsequent text which should be in the window.
    static void SetRasterPosSafe(TModelUnit x, TModelUnit y);
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLBITMAPFONT__HPP
