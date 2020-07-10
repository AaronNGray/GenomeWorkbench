#ifndef GUI_UTILS___RGBA_COLOR__HPP
#define GUI_UTILS___RGBA_COLOR__HPP

/*  $Id: rgba_color.hpp 43891 2019-09-16 13:50:00Z evgeniev $
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
 * Authors:  Robert Smith, Mike DiCuccio
 *
 * File Description:
 *    CRgbaColor - colors in the RGB colorspace, including transparency
 */


#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <gui/utils/vect4.hpp>
#include <gui/utils/vect3.hpp>


/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


///
/// class CRgbaColor provides a simple abstraction for managing colors.
///
/// Keep the class concrete to make it efficient for use in  arrays/buffers
/// (do not derive from this class or add virtual functions.)
///
class NCBI_GUIUTILS_EXPORT CRgbaColor
{
public:

    /// default ctor
    CRgbaColor();

    /// construct around a given float array
    CRgbaColor(const float* color, size_t size);

    /// construct an RGBA color around an Fl_Color
    //CRgbaColor(Fl_Color c);

    /// construct with explicit (floating point) RGB values
    CRgbaColor(float r, float g, float b);

    /// construct with explicit (floating point) RGB+alpha values
    CRgbaColor(float r, float g, float b, float a);

    // construct using rgb from another color object and a new alpha
    CRgbaColor(const CRgbaColor& rgb, float alpha);

    /// construct with explicit (unsigned char) RGB values.  These are
    /// normalized to 0-1 internally.
    CRgbaColor(unsigned char r, unsigned char g, unsigned char b);

    /// construct with explicit (unsigned char) RGB+alpha values.  These are
    /// normalized from 0-1 internally.
    CRgbaColor(unsigned char r, unsigned char g,
               unsigned char b, unsigned char a);

    /// construct with explicit (int) RGB values.  These are
    /// normalized to 0-1 internally.
    /// The values should be in the rane of 0-255.
    CRgbaColor(int r, int g, int b);

    /// construct with explicit (int) RGB+alpha values.  These are
    /// normalized from 0-1 internally.
    /// The values should be in the rane of 0-255.
   CRgbaColor(int r, int g, int b, int a);

    /// construct from a string encoded in the form "r g b"
    explicit CRgbaColor(const string& s);

    /// set the color from an Fl_Color
    //void Set(Fl_Color c);

    /// set the values from explicit (floating point) RGB values
    void Set(float r, float g, float b);

    /// set the values from explicit (floating point) RGB+alpha values
    void Set(float r, float g, float b, float a);

    /// set the values from explicit (unsigned char) RGB values.  These
    /// are normalized internally to the range 0-1.
    void Set(unsigned char r, unsigned char g, unsigned char b);

    /// set the values from explicit (unsigned char) RGB+alpha values.  These
    /// are normalized internally to the range 0-1.
    void Set(unsigned char r, unsigned char g,
             unsigned char b, unsigned char a);

    /// Set specific channels from floating point values
    void SetRed  (float r);
    void SetGreen(float r);
    void SetBlue (float r);
    void SetAlpha(float r);

    /// Set specific channels from floating unsigned char values.  These
    /// are normalized internally to the range 0-1.
    void SetRed  (unsigned char r);
    void SetGreen(unsigned char r);
    void SetBlue (unsigned char r);
    void SetAlpha(unsigned char r);

    /// Get specific channels in floating point values.
    float GetRed  (void) const;
    float GetGreen(void) const;
    float GetBlue (void) const;
    float GetAlpha(void) const;

    /// Get specific channels in unsigned char values.
    unsigned char GetRedUC  (void) const;
    unsigned char GetGreenUC(void) const;
    unsigned char GetBlueUC (void) const;
    unsigned char GetAlphaUC(void) const;
	CVect4<unsigned char> GetRgbaUC();
	CVect3<unsigned char> GetRgbUC();

    /// Access the color array directly.
    const float* GetColorArray(void) const;

    /// print the color to a stream in the form "r g b"
    void PrintTo(CNcbiOstream& strm) const;
    void PrintTo(CNcbiOstream& strm, bool printAlpha, bool uchars = true) const;

    /// Return a string representation of the current color
    string ToString(bool printAlpha = true, bool uchars = true) const;
    /// Return a CSS string representation of the current color
    string ToCssString(bool printAlpha = true) const;
    /// Return an HTML string representation of the current color
    string ToHtmlString() const;

    /// Assign color values encoded in a string.
    /// This function is responsible for all string-based processing;
    /// all other string-based interfaces funnel through this.  The formats
    /// accepted include, in order of parsing:
    ///
    /// 1.  A named color, such as "red", "blue", "green", "salmon", etc.
    ///     There are approximately 750 named colors that are currently
    ///     understood.
    ///
    /// 2.  HTML-format colors, such as "#f0f0dd", or "f0f0dd".  These are
    ///     accepted with or without the leading '#'.
    ///
    /// 3.  Bracketed or non-bracketed triplets or quadruplets of integers, in
    ///     any of the formats below (mix and match spaces or commas as separators,
    ///     with or without '[]' or '()'):
    ///
    ///         123 123 123
    ///         123, 123, 123
    ///         [123 123 123]
    ///         (123, 123, 123)
    ///         (123 123 123)
    ///         [123, 123, 123]
    ///
    void FromString(const string& str);

    // add a color to this color
    CRgbaColor& operator+=(const CRgbaColor& c1);

    // multiply this color by a scalar
    CRgbaColor& operator*=(float f);

    // lighten a color by a scaling factor.  Scale must range from 0-1.
    // This is equivalent to interpolating the color with white, with the
    // exception that the alpha channel is not altered.
    void Lighten(float scale);

    // lighten a color by a scaling factor.  Scale must range from 0-1.
    // This is equivalent to interpolating the color with black, with the
    // exception that the alpha channel is not altered.
    void Darken(float scale);

    /// Returns the greyscale equivalent of the current color
    CRgbaColor GetGreyscale() const;

    /// return a mapped color string from a named color
    static const char* ColorStrFromName(const string& desc);

    /// return a color based on a string.  This function will understand two
    /// forms of color type:
    ///  - color names - things like 'red', 'powder blue', etc.
    ///  - color triplets or quads - RGB(A) values encoded as '128 128 128'
    ///    tokens include ' ,[', so all of the following are valid:
    ///      128 128 128
    ///      [128 128 128]
    ///      128, 128, 128
    ///      [128, 128, 128]
    static CRgbaColor GetColor(const string& color_type);

    /// Interpolate two colors. The resulting color returned is
    ///  ( (color1 * alpha) + (color2 * (1 - alpha) )
    static CRgbaColor Interpolate(const CRgbaColor& color1,
                                  const CRgbaColor& color2,
                                  float alpha);

    /// returns XOR complementary color, alpha is not affected
    static CRgbaColor Invert(const CRgbaColor& color1);

    /// @name Color space conversion functions.
    /// @{

    /// convert RGB to HSV.
    /// @param h    Hue. range 0 - 360.
    /// @param s    Saturation. range 0 - 1.0
    /// @param v    Value. range 0 - 1.0
    static void         RgbToHsv(const CRgbaColor& rgb, float& h, float& s, float& v);
    static CRgbaColor   HsvToRgb(float h, float s, float v);

    /// convert RGB to YUV.
    /// @param y        luminance or brightness.  range [0, 1]
    /// @param u        difference between blue component and luminance. range [-.5, .5]
    /// @param v        difference between red component and luminance. range [-.5, .5]
    static void         RgbToYuv(const CRgbaColor& rgb, float& y, float& u, float& v);
    /// Not all possible input values of y, u and v will make a valid RGB color.
    /// in that case the RGB values are clamped to [0, 1].
    /// @return         false if clipping occurs.
    /// see http://developer.apple.com/quicktime/icefloe/dispatch027.html
    static bool         YuvToRgb(float y, float u, float v, CRgbaColor& rgb);

    /// @}

    /// Rotate the hue of the color by degrees.
    static CRgbaColor   RotateColor(const CRgbaColor& c, float degrees);
    /// return the brightness or luminance of the color.
    /// Same as Y in RgbToYuv
    /// @sa RgbToYuv
    static float Brightness(const CRgbaColor& rgb);
    /// returns the distance in the RGB color cube between the two colors,
    /// scaled to a range [0, 1].
    static float ColorDistance(const CRgbaColor& c1, const CRgbaColor& c2);

    /// Return a color guaranteed to contrast nicely with this color.
    /// @param onlyBW   true - only return black or white, otherwise pick from all colors.
    /// @returns    A color that will show up well against this color.
    CRgbaColor ContrastingColor(bool onlyBW = true) const;

private:
    void        x_Clamp();

    float m_Rgba[4];
};


/// global comparison operator
NCBI_GUIUTILS_EXPORT
    bool operator==(const CRgbaColor& c1, const CRgbaColor& c2);

/// define a some-what arbitrary sort order based on brightness
/// It does have the property of being equivalent to operator==, that is:
/// ( ! a<b  && ! b<a ) iff a == b
NCBI_GUIUTILS_EXPORT
    bool operator<(const CRgbaColor& c1, const CRgbaColor& c2);

/// add two colors
NCBI_GUIUTILS_EXPORT
    CRgbaColor operator+(const CRgbaColor& c1, const CRgbaColor& c2);


/// multiply a color by a scalar
NCBI_GUIUTILS_EXPORT
    CRgbaColor operator*(const CRgbaColor& c1, float f);



inline
void CRgbaColor::SetRed(unsigned char r)
{
    m_Rgba[0] = float(r) / 255.0f;
}


inline
void CRgbaColor::SetGreen(unsigned char g)
{
    m_Rgba[1] = float(g) / 255.0f;
}


inline
void CRgbaColor::SetBlue(unsigned char b)
{
    m_Rgba[2] = float(b) / 255.0f;
}


inline
void CRgbaColor::SetAlpha(unsigned char a)
{
    m_Rgba[3] = float(a) / 255.0f;
}

inline
float CRgbaColor::GetRed(void) const
{
    return m_Rgba[0];
}

inline
float CRgbaColor::GetGreen(void) const
{
    return m_Rgba[1];
}

inline
float CRgbaColor::GetBlue(void) const
{
    return m_Rgba[2];
}

inline
float CRgbaColor::GetAlpha(void) const
{
    return m_Rgba[3];
}

inline
unsigned char CRgbaColor::GetRedUC(void) const
{
    return (unsigned char) (m_Rgba[0] * 255.0f);
}

inline
unsigned char CRgbaColor::GetGreenUC(void) const
{
    return (unsigned char) (m_Rgba[1] * 255.0f);
}

inline
unsigned char CRgbaColor::GetBlueUC(void) const
{
    return (unsigned char) (m_Rgba[2] * 255.0f);
}

inline
unsigned char CRgbaColor::GetAlphaUC(void) const
{
    return (unsigned char) (m_Rgba[3] * 255.0f);
}

inline
CVect4<unsigned char> CRgbaColor::GetRgbaUC()
{
	CVect4<unsigned char> rgba;

	rgba[0] = GetRedUC();
	rgba[1] = GetGreenUC();
	rgba[2] = GetBlueUC();
	rgba[3] = GetAlphaUC();

	return rgba;
}

inline
CVect3<unsigned char> CRgbaColor::GetRgbUC()
{
	CVect3<unsigned char> rgb;

	rgb[0] = GetRedUC();
	rgb[1] = GetGreenUC();
	rgb[2] = GetBlueUC();

	return rgb;
}

inline
const float* CRgbaColor::GetColorArray(void) const
{
    return m_Rgba;
}


END_NCBI_SCOPE


/* @} */

#endif  // GUI_UTILS___RGBA_COLOR__HPP
