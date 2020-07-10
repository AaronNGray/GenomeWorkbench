#ifndef GUI_OPENGL___GLTEXTUREFONT__HPP
#define GUI_OPENGL___GLTEXTUREFONT__HPP

/*  $Id: gltexturefont.hpp 45022 2020-05-08 02:01:25Z evgeniev $
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
 * CGlTextureFont class for rendering text in OpenGL
 *
 */

#include <gui/opengl/glfont.hpp>

#include <gui/opengl/gltypes.hpp>
#include <gui/utils/matrix4.hpp>

class FTFont;

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

//
//
class NCBI_GUIOPENGL_EXPORT CGlTextureFont : public IGlFont
{
public:
    /// Set of pre-defined fonts for which we know we have valid font files.
    /// Other fonts may also be loaded by providing the file name directly.
    enum EFontFace {               
        eFontFace_Helvetica = 0,
        eFontFace_Helvetica_Bold,
        eFontFace_Helvetica_Italic,
        eFontFace_Helvetica_BoldItalic,
        eFontFace_Lucida,
        eFontFace_Lucida_Bold,
        eFontFace_Lucida_Italic,
        eFontFace_Lucida_BoldItalic,
        eFontFace_Courier,
        eFontFace_Courier_Bold,
        eFontFace_Courier_Italic,
        eFontFace_Courier_BoldItalic,
        eFontFace_Fixed,
        eFontFace_Fixed_Bold,
        eFontFace_Fixed_Italic,
        eFontFace_Fixed_BoldItalic,
        eFontFace_TimesRoman,
        eFontFace_TimesRoman_Bold,
        eFontFace_TimesRoman_Italic,
        eFontFace_TimesRoman_BoldItalic,
        eFontFace_BPMono,
        eFontFace_BPMono_Bold,
        eFontFace_BPMono_Italic,
        eFontFace_BPMono_BoldStencil,
        // Fonts after eFontFace_LastFont are internally selected, we don't
        // want them to show up on selection lists
        eFontFace_LastFont,  
        eFontFace_Helvetica6,
        eFontFace_Helvetica8,
        eFontFace_Helvetica10,
        eFontFace_LastBitmapFont
    };

    /// Font rotate options
    enum EFontRotateFlags {
        fFontRotateBase    = (1 << 0), ///< Rotate around lower-left corner of unrotated text box
        fFontRotateMid     = (1 << 1), ///< Rotate around mid-left of unrotated text box
        fFontRotateCap     = (1 << 2), ///< Rotate around upper-left corner of unrotated text box
        fReorientText      = (1 << 4)  ///< Ensure rotated text reads L to R and is rightside-up   
    };
    typedef int TFontRotateFlags;

public:

    /// default constructor. 
    CGlTextureFont();

    /// create a font with a face loaded from a file
    CGlTextureFont(const string& font_file_name, unsigned int font_size=12);

    /// create a font with one of the pre-defined faces
    CGlTextureFont(EFontFace face, unsigned int font_size=12);

    /// use default copy ctor and operator=()
    //CGlTextureFont(const CGlTextureFont&);
    //CGlTextureFont& operator= (const CGlTextureFont&);

    /// destructor does nothing since CFtglFontManager owns font objects
    ~CGlTextureFont() {}

    // Set/get font face to use
    void        SetFontFace(EFontFace face, bool use_bitmap_overrides=true);
    EFontFace   GetFontFace() const;
    
    /// Set/get font size in points
    void         SetFontSize(unsigned int size);
    unsigned int GetFontSize() const;
    
    /// Load font from file explicitly. Returns true on success, false otherwise
    bool SetFont(string font_file_name, 
                 unsigned int font_size,
                 bool use_bitmap_overrides=true);

    /// Get current font filename. Fonts set via EFontFace also have filenames
    string GetFontFile() const { return m_FontFile; }

    /// Get font file for given face
    static string GetFontFileForFace(EFontFace face);

    /// Returns true if the currently loaded font is from a bitmap
    virtual bool IsBitmapFont() const;

    /// Set rotation flags (for center of rotation and re-orienting)
    void SetFontRotate(TFontRotateFlags rot) { m_Rotate = rot; }
    TFontRotateFlags GetFontRotate(void) const { return m_Rotate; }

    /// If true (the default) text output position is rounded to nearest
    /// pixel coordinate, which improves text appearance (slightly)
    void SetSnapToPixel(bool xpix, bool ypix);

    ///
    /// WriteText interface
    /// The WriteText functions produce the same results as TextOut
    /// but they are more effcient when writing multiple strings since
    /// the OpenGL state is only updated in BeginText and EndText
    /// 
    /// BeginText();
    /// WriteText(...);
    /// WriteText(...);
    /// EndText();
    ///

    /// Set OpenGL state needed for writing text (call before WriteText)
    void BeginText() const;
    /// Use this version if rendering may be non-OpenGL (viewport
    /// for pdf can be larger than max-size for OpenGL)
    void BeginText(const TVPRect& viewport) const;    
    void BeginText(const TVPRect& viewport, 
                   GLdouble* mview, GLdouble* proj ) const;    

    /// Pops matrices and attributes after writing text
    void EndText() const;
    /// Write text at specified model coords
    void WriteText(TModelUnit x, TModelUnit y, 
                   const char* text,
                    TModelUnit rotate_degrees = 0.0) const;

    /// Write text at specified model coords inside box defined by
    /// (x,y,width,height) with specified alignment, truncation and rotation
    void WriteText(TModelUnit x, TModelUnit y, 
                   TModelUnit width, TModelUnit height,
                   const char* text,
                   TAlign align = eAlign_Center,
                   ETruncate trunc = eTruncate_Ellipsis,
                   TModelUnit rotate_degrees = 0.0f) const;

    void WriteText(TModelUnit x, TModelUnit y,
                   const char* text,
                   const float* color,
                   TModelUnit rotate_degrees) const;

    /// Write text at specified model coords inside box defined by
    /// (x,y,width,height) with specified alignment, truncation and rotation
    void WriteText(TModelUnit x, TModelUnit y,
                   TModelUnit width, TModelUnit height,
                   const char* text,
                   const float* color,
                   TAlign align,
                   ETruncate trunc,
                   TModelUnit rotate_degrees) const;

    /// Get the transformation that will be applied to the text to write
    /// it within the box (x,y,width,height).  Also return (possibly truncated)
    /// text.  Needed for output to other mediums (e.g. pdf)
    CMatrix4<double> GetTextXform(TModelUnit x, TModelUnit y, 
                                  TModelUnit width, TModelUnit height,
                                  string& text,
                                  TAlign align = eAlign_Center,
                                  ETruncate trunc = eTruncate_Ellipsis,
                                  TModelUnit rotate_degrees = 0.0f) const;

    ///
    /// TextOut interface
    /// Write the specified text and set up state and transformation
    /// as needed.
    ///

    /// Write text directly - assumes transforms and state already set
    virtual void TextOut(const char* text) const;

    /// Write text at specified model coords (this function sets all necessary state)
    virtual void TextOut(TModelUnit x, TModelUnit y, const char* text) const;

    /// Output text into a given rectangle using a particular alignment.
    /// scale_x and scale_y are ignored in this implementation (they are
    /// overridden using actual viewport and projection matrix values)
    void TextOut(TModelUnit x1, TModelUnit y1, TModelUnit x2, TModelUnit y2, 
                 const char* text,
                 TAlign align = eAlign_Center,
                 ETruncate trunc = eTruncate_Ellipsis,
                 TModelUnit  scale_x = 1.0,
                 TModelUnit  scale_y = 1.0) const;

    /// Truncate text to the secified width 
    string Truncate(const char* text, 
                    TModelUnit w, 
                    ETruncate trunc = eTruncate_Ellipsis) const;
    string Truncate(const string& text, 
                    TModelUnit w, 
                    ETruncate trunc = eTruncate_Ellipsis) const;

    /// prints array of characters in positions (x + i*dx, y + i*dy) where
    /// "i" is index of a character in the given string
    void    ArrayTextOut(TModelUnit x, TModelUnit y, 
                         TModelUnit dx, TModelUnit dy,
                         const char* text,
                         const vector<CRgbaColor*>* colors = NULL,
                         TModelUnit scale_x = 1.0f,
                         TModelUnit scale_y = 1.0f) const;

    ///
    /// Compute and return font metrics
    ///

    virtual TModelUnit TextWidth (const char* text) const;
    virtual TModelUnit TextHeight(void) const;
    virtual TModelUnit GetFontDescender() const;
    TModelUnit   GetAdvance(char c) const;

    void ProjectVertex(CVect2<float>& vertex) const;

    /// returns minimal space sufficient to render any number
    /// in [0, max_num]. It is assumed that numeric text is produced by 
    /// CTextUtils::FormatSeparatedNumber()
    TModelUnit GetMaxWidth(int max_num) const;

    // generic metric retrieval
    virtual TModelUnit GetMetric(EMetric metric, 
                                 const char* text = NULL,
                                 int len = -1) const;

    ///
    /// funtions below provide conversion of font parameters to strings and
    /// back. Handy for representing them in GUI.
    ///
    
    /// Converts enumerated font face to string
    static string       FaceToString(EFontFace face);
    /// Selects a font face given a string or eFontFace_LastFont on failure
    static EFontFace    FaceFromString(const string& str);

    /// Name of basefont for PDF purposes, or "" if not available
    static string       PdfBaseFontName(EFontFace face, EFontFace& pdf_face);

    /// saves and restores font face and size to/from a string
    string  ToString() const;
    bool    FromString(const string& value);

    /// GetAllFaces() and GetAllSizes() fills containers with strings
    /// repesenting all available enumerated (pre-defined)fonts.
    /// Sizes returned are a standard selection set which can be used
    /// for user selection - but any int can be a font size (up to max.
    /// texture sizes)
    static void     GetAllFaces(vector<string>& faces);
    static void     GetAllSizes(vector<string>& sizes);

private:
    /// Truncate a string for display.  This returns the maximum
    /// number of characters that can fit into the given width.  If the
    /// parameter 'str' is not null, the string will be filled out with
    /// the truncated text.
    int x_Truncate(const char* text, 
                   TModelUnit w, 
                   ETruncate trunc,
                   string* str = NULL) const;

    int x_Truncate2(const char* text,
        TModelUnit w,
        ETruncate trunc,
        string* str = NULL) const;

    /// Called by both writetext and gettexttransform to compute transformation
    /// matrix or write the text. If write_text is true, returned matrix is
    /// identity.
    CMatrix4<double> x_WriteText(TModelUnit x, TModelUnit y, 
                                 TModelUnit width, TModelUnit height,
                                 string& text,
                                 TAlign align,
                                 ETruncate trunc,
                                 TModelUnit rotate_degrees,
                                 bool write_text) const;

    /// Return the font file for a font face - some fonts may use bitmap 
    /// files for smaller sizes, so a font face can map to more than one file
    string x_GetFontFile(EFontFace face, 
                         string font_file_name, 
                         unsigned int font_size,
                         bool use_bitmap_overrides=true) const;

	/// Compute metrics for the font upon creation (for performance)
	void x_ComputeMetrics();

private:
    /// This maps EFontFace enums to font file names for loading
    static string s_FontFileNames[];

    /// Maps EFontFace enums to a readable strings for selection lists
    static string s_FontNames[];

    /// Font sizes used as a standard set for user selection - last element in
    /// array is set to 0 as a marker. SetFontSize() can be called with any
    /// reasonable integer though - s_FontSizes are just the most commonly used
    static int s_FontSizes[];

    /// The current font for writing.  These are owned by CFtglFontManager class
    mutable FTFont*  m_Font;

    /// Current font face held by m_Font.  Set to eFontFace_LastFont when there
    /// is no valid font or the font was loaded directly from a file and thus is
    /// is not one of the enumerated options
    EFontFace  m_FontFace;
    /// Size of current font
    unsigned int m_FontSize;

    /// flags for center of rotation and how to handle direction and orientation
    TFontRotateFlags  m_Rotate;

    /// Normally, when text is written to the screen the position is synched to
    /// the nearest pixel.  If either of these is false, the corisponding coord
    /// is not synched.  (Drawing at pixel locations makes font a little sharper)
    bool m_SnapToPixelX;
    bool m_SnapToPixelY;

    /// File from which current font was loaded
    string m_FontFile;

	/// Pre-computed metrics (may be computed first time accessed)
	mutable TModelUnit m_MetricCharHeight;
	mutable TModelUnit m_MetricFullCharHeight;
	mutable TModelUnit m_MetricAvgCharWidth;
	mutable TModelUnit m_MetricMaxCharWidth;
	mutable TModelUnit m_MetricDescender;

    /// Temporary variables initialized by the BeginText()
    /// function and used by the Draw() functions.  Used
    /// for projecting text location to window coordinates.
	mutable GLint m_Viewport[4];
    mutable GLdouble m_ModelviewMatrix[16];
    mutable GLdouble m_ProjectionMatrix[16];

    /// Switch back to previous shader after writing text
    mutable GLint m_PrevShader;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLTEXTUREFONT__HPP
