#ifndef GUI_PRINT___PRINT_OPTIONS__HPP
#define GUI_PRINT___PRINT_OPTIONS__HPP

/* $Id: print_options.hpp 42695 2019-04-03 19:45:15Z evgeniev $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *    CPrintOptions -- printing options
 */


#include <corelib/ncbistd.hpp>
#include <gui/print/print_utils.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/print/print_media.hpp>

BEGIN_NCBI_SCOPE


class CMedia;


class NCBI_GUIPRINT_EXPORT CPrintOptions
{
public:
    typedef CUnit::TPdfUnit TPdfUnit;

public:
    enum EPageOrientation {
        ePortrait = 0,
        eLandscape = 270,
        eUpsideDown = 180,
        eSeascape = 90
    };
    typedef enum EPageOrientation TPageOrientation;

    enum {
        eVector = 0x10,
        eRaster = 0x20
    };

    enum EOutputFormat {
        eUnknown    = 0,
        ePostscript = eVector | 1,
        ePdf        = eVector | 2,
        eSvg        = eVector | 3,
        eSvgz       = eVector | 4,
        ePng        = eRaster | 5,
        eJpeg       = eRaster | 6,
        eDefault    = ePng
    };
    typedef enum EOutputFormat TOutputFormat;

    enum EAlignment
    {
        // horizontal
        eLeft,
        eCenter,
        eRight,
        // vertical
        eTop,
        eMiddle,
        eBottom
    };
    typedef enum EAlignment TAlignment;

    struct SText
    {
        string text;
        CGlBitmapFont::EFontFace font;
        unsigned int font_size;
    };

    static TOutputFormat GetFormatFromName(const string& fmtname);


    CPrintOptions();
    virtual ~CPrintOptions();

    void SetGenerateIndex(bool generateIdx);
    bool GetGenerateIndex(void) const;

    void SetGrayscale(bool greyscale);
    bool GetGrayscale(void) const;

    void SetPageOrientation(TPageOrientation orient);
    TPageOrientation GetPageOrientation(void) const;

    void SetFilename(const string& filename);
    const string& GetFilename(void) const;

    void SetTitle(const string& title);
    const string& GetTitle(void) const;

    void SetHeader(const string& header);
    void SetHeader(const SText& header);
    const string& GetHeader(void) const;
    const SText& GetHeaderDetails(void) const;

    void SetFooter(const string& footer);
    void SetFooter(const SText& footer);
    const string& GetFooter(void) const;
    const SText& GetFooterDetails(void) const;

    void SetOutputFormat(TOutputFormat fmt);
    TOutputFormat GetOutputFormat(void) const;
    static bool s_IsVectorFormat(TOutputFormat fmt);
    static bool s_IsRasterFormat(TOutputFormat fmt);

    void SetPrintOutput(bool print_output);
    bool GetPrintOutput(void) const;

    void SetPagesWide(unsigned int w);
    unsigned int GetPagesWide(void) const;
    void SetPagesTall(unsigned int w);
    unsigned int GetPagesTall(void) const;
    unsigned int GetNumPages(void) const;

    void SetMedia(const CMedia& media);
    const CMedia& GetMedia(void) const;
    TPdfUnit GetMediaHeight(void) const;
    TPdfUnit GetMediaWidth(void) const;

    bool HasRasterDimensions(void) const;
    void UnsetRasterDimensions(void);
    void SetRasterWidth(unsigned int w);
    unsigned int GetRasterWidth(void) const;
    void SetRasterHeight(unsigned int h);
    unsigned int GetRasterHeight(void) const;

    void SetHeaderOffset(TPdfUnit h);
    TPdfUnit GetHeaderOffset(void) const;

    void SetFooterOffset(TPdfUnit h);
    TPdfUnit GetFooterOffset(void) const;

    void SetMarginTop(TPdfUnit h);
    TPdfUnit GetMarginTop(void) const;

    void SetMarginBottom(TPdfUnit w);
    TPdfUnit GetMarginBottom(void) const;

    void SetMarginLeft(TPdfUnit h);
    TPdfUnit GetMarginLeft(void) const;

    void SetMarginRight(TPdfUnit w);
    TPdfUnit GetMarginRight(void) const;

    TPdfUnit GetPageTrimWidth(void) const;
    TPdfUnit GetPageTrimHeight(void) const;

    void SetUserUnit(TPdfUnit userunit) { m_UserUnit = userunit; }
    TPdfUnit GetUserUnit() const { return m_UserUnit; }

    // If true, all gouraud shaded tris (shadingtype 4) will be drawn
    // as flat polygons (same color at each vertex)
    void DisableGouraudShading(bool b) { m_GouraudDisabled = b;}
    bool GetGouraudShadingDisabled() const { return m_GouraudDisabled; }

public:
    static const string& GetMimeType(EOutputFormat format);
    static const string& GetFileExtension(EOutputFormat format);

protected:
    TPdfUnit x_GetPageWidth(void) const;
    TPdfUnit x_GetPageHeight(void) const;

private:
    // general attributes
    string m_Filename;
    string m_TempFilename;
    bool m_Grayscale;
    TOutputFormat m_Format;

    // raster-output attributes
    bool m_HasRasterDims;
    unsigned int m_RasterWidth;
    unsigned int m_RasterHeight;

    // vector-output attributes
    string m_Title;
    SText m_Header;
    SText m_Footer;
    TPageOrientation m_PageOrientation;
    bool m_PrintOutput;
    bool m_GenerateIndex;

    unsigned int m_PagesWide;
    unsigned int m_PagesTall;
    CMedia m_Media;
    TPdfUnit m_HeaderOffset;
    TPdfUnit m_FooterOffset;
    TPdfUnit m_MarginTop;
    TPdfUnit m_MarginBottom;
    TPdfUnit m_MarginLeft;
    TPdfUnit m_MarginRight;

    // output scaler, defaults to 1.0
    TPdfUnit m_UserUnit;

    // If true, gouraud tris rendered as flat polys
    bool m_GouraudDisabled;
};



END_NCBI_SCOPE


#endif  // GUI_PRINT___PRINT_OPTIONS__HPP
