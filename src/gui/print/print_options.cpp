/*  $Id: print_options.cpp 42695 2019-04-03 19:45:15Z evgeniev $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPrintOptions - output base class
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <gui/print/print_options.hpp>
#include <gui/print/print_utils.hpp>
#include <gui/print/print_media.hpp>


BEGIN_NCBI_SCOPE



CPrintOptions::CPrintOptions()
    : m_Grayscale(false),
      m_Format(ePdf),
      m_HasRasterDims(false),
      m_RasterWidth(0),
      m_RasterHeight(0),
      m_PageOrientation(eLandscape),
      m_PrintOutput(false),
      m_GenerateIndex(false),
      m_PagesWide(1),
      m_PagesTall(1),
      m_Media(CMedia::GetMedia("Letter")),
      m_HeaderOffset(InchesToPdfUnits(0.5)),
      m_FooterOffset(InchesToPdfUnits(0.5)),
      m_MarginTop(InchesToPdfUnits(0.75)),
      m_MarginBottom(InchesToPdfUnits(0.75)),
      m_MarginLeft(InchesToPdfUnits(0.75)),
      m_MarginRight(InchesToPdfUnits(0.75)),
      m_UserUnit(1.0f),
      m_GouraudDisabled(false)
{
    SText dflt_txt = { "", CGlBitmapFont::eFontFace_Helvetica, 8 };
    SetHeader(dflt_txt);
    SetFooter(dflt_txt);

    CDirEntry fname;
    do {
        fname.Reset(CFile::GetTmpNameEx(kEmptyStr) + ".pdf");
    } while (fname.Exists());
    m_TempFilename = fname.GetPath();

    SetMedia(Letter);
}


CPrintOptions::~CPrintOptions()
{
}


void CPrintOptions::SetGenerateIndex(bool generateIdx)
{
    m_GenerateIndex = generateIdx;
}


bool CPrintOptions::GetGenerateIndex(void) const
{
    return m_GenerateIndex;
}


void CPrintOptions::SetGrayscale(bool greyscale)
{
    m_Grayscale = greyscale;
}


bool CPrintOptions::GetGrayscale(void) const
{
    return m_Grayscale;
}


void CPrintOptions::SetPageOrientation(TPageOrientation orient)
{
    m_PageOrientation = orient;
}


CPrintOptions::TPageOrientation CPrintOptions::GetPageOrientation(void) const
{
    return m_PageOrientation;
}


void CPrintOptions::SetFilename(const string& filename)
{
    m_Filename = filename;
    SetPrintOutput(false);
}


const string& CPrintOptions::GetFilename(void) const
{
    return m_PrintOutput ? m_TempFilename : m_Filename;
}


void CPrintOptions::SetTitle(const string& title)
{
    m_Title = title;
}


const string& CPrintOptions::GetTitle(void) const
{
    return m_Title;
}


void CPrintOptions::SetHeader(const SText& header)
{
    m_Header = header;
}


void CPrintOptions::SetHeader(const string& header)
{
    m_Header.text = header;
}


const string& CPrintOptions::GetHeader(void) const
{
    return m_Header.text;
}


const CPrintOptions::SText& CPrintOptions::GetHeaderDetails(void) const
{
    return m_Header;
}


void CPrintOptions::SetFooter(const SText& footer)
{
    m_Footer = footer;
}


void CPrintOptions::SetFooter(const string& footer)
{
    m_Footer .text= footer;
}


const string& CPrintOptions::GetFooter(void) const
{
    return m_Footer.text;
}


const CPrintOptions::SText& CPrintOptions::GetFooterDetails(void) const
{
    return m_Footer;
}


CPrintOptions::TOutputFormat
CPrintOptions::GetFormatFromName(const string& fmtname)
{
    static const string kPS("Postscript");
    static const string kPdf("PDF");
    static const string kSvg("SVG");
    static const string kPng("PNG");

    if (NStr::Equal(kPS, fmtname, NStr::eNocase)) {
        return ePostscript;
    }
    if (NStr::Equal(kPdf, fmtname, NStr::eNocase)) {
        return ePdf;
    }
    if (NStr::Equal(kSvg, fmtname, NStr::eNocase)) {
        return eSvg;
    }
    if (NStr::Equal(kPng, fmtname, NStr::eNocase)) {
        return ePng;
    }

    // format not found
    THROW0_TRACE(runtime_error("Cannot find format named '" + fmtname + "'"));
}


void CPrintOptions::SetOutputFormat(TOutputFormat fmt)
{
    m_Format = fmt;
}


CPrintOptions::TOutputFormat CPrintOptions::GetOutputFormat(void) const
{
    return (m_PrintOutput) ? ePdf : m_Format;
}


bool CPrintOptions::s_IsVectorFormat(TOutputFormat fmt)
{
    return ((fmt & CPrintOptions::eVector) == CPrintOptions::eVector);
}


bool CPrintOptions::s_IsRasterFormat(TOutputFormat fmt)
{
    return ((fmt & CPrintOptions::eRaster) == CPrintOptions::eRaster);
}


void CPrintOptions::SetPrintOutput(bool print_output)
{
    m_PrintOutput = print_output;
}


bool CPrintOptions::GetPrintOutput(void) const
{
    return m_PrintOutput;
}


void CPrintOptions::SetPagesWide(unsigned int w)
{
    m_PagesWide = w;
}


unsigned int CPrintOptions::GetPagesWide(void) const
{
    return m_PagesWide;
}


void CPrintOptions::SetPagesTall(unsigned int t)
{
    m_PagesTall = t;
}


unsigned int CPrintOptions::GetPagesTall(void) const
{
    return m_PagesTall;
}


void CPrintOptions::SetMedia(const CMedia& media)
{
    m_Media = media;
}


unsigned int CPrintOptions::GetNumPages(void) const
{
    return m_PagesTall * m_PagesWide;
}


const CMedia& CPrintOptions::GetMedia(void) const
{
    return m_Media;
}


CUnit::TPdfUnit CPrintOptions::GetMediaHeight(void) const
{
    if (m_PageOrientation == eLandscape  ||  m_PageOrientation == eSeascape) {
        return m_Media.GetWidth();
    }
    return m_Media.GetHeight();
}


CUnit::TPdfUnit CPrintOptions::GetMediaWidth(void) const
{
    if (m_PageOrientation == eLandscape  ||  m_PageOrientation == eSeascape) {
        return m_Media.GetHeight();
    }
    return m_Media.GetWidth();
}


bool CPrintOptions::HasRasterDimensions(void) const
{
    return m_HasRasterDims;
}


void CPrintOptions::UnsetRasterDimensions(void)
{
    m_HasRasterDims = false;
}


void CPrintOptions::SetRasterWidth(unsigned int w)
{
    m_HasRasterDims = true;
    m_RasterWidth = w;
}


unsigned int CPrintOptions::GetRasterWidth(void) const
{
    return m_HasRasterDims ? m_RasterWidth : 0;
}


void CPrintOptions::SetRasterHeight(unsigned int h)
{
    m_HasRasterDims = true;
    m_RasterHeight = h;
}


unsigned int CPrintOptions::GetRasterHeight(void) const
{
    return m_HasRasterDims ? m_RasterHeight : 0;
}


void CPrintOptions::SetHeaderOffset(TPdfUnit offset)
{
    m_HeaderOffset = offset;
}


CUnit::TPdfUnit CPrintOptions::GetHeaderOffset(void) const
{
    return m_HeaderOffset;
}


void CPrintOptions::SetFooterOffset(TPdfUnit offset)
{
    m_FooterOffset = offset;
}


CUnit::TPdfUnit CPrintOptions::GetFooterOffset(void) const
{
    return m_FooterOffset;
}


void CPrintOptions::SetMarginTop(TPdfUnit t)
{
    m_MarginTop = t;
}


CUnit::TPdfUnit CPrintOptions::GetMarginTop(void) const
{
    return m_MarginTop;
}


void CPrintOptions::SetMarginBottom(TPdfUnit b)
{
    m_MarginBottom = b;
}


CUnit::TPdfUnit CPrintOptions::GetMarginBottom(void) const
{
    return m_MarginBottom;
}


void CPrintOptions::SetMarginLeft(TPdfUnit l)
{
    m_MarginLeft = l;
}


CUnit::TPdfUnit CPrintOptions::GetMarginLeft(void) const
{
    return m_MarginLeft;
}


void CPrintOptions::SetMarginRight(TPdfUnit r)
{
    m_MarginRight = r;
}


CUnit::TPdfUnit CPrintOptions::GetMarginRight(void) const
{
    return m_MarginRight;
}


CUnit::TPdfUnit CPrintOptions::GetPageTrimWidth(void) const
{
    return GetMediaWidth() - GetMarginLeft() - GetMarginRight();
}


CUnit::TPdfUnit CPrintOptions::GetPageTrimHeight(void) const
{
    return GetMediaHeight() - GetMarginTop() - GetMarginBottom();
}

const string& CPrintOptions::GetMimeType(EOutputFormat format)
{
    unsigned index = format & 0xF;
    static vector<string> mime_types = { "application/octet-stream", "application/postscript", "application/pdf", "image/svg+xml", "image/svg+xml", "image/png", "image/jpeg" };
    _ASSERT(index < mime_types.size());
    return mime_types[index];
}

const string& CPrintOptions::GetFileExtension(EOutputFormat format)
{
    unsigned index = format & 0xF;
    static vector<string> file_exts = { "bin", "ps", "pdf", "svg", "svgz", "png", "jpg" };
    _ASSERT(index < file_exts.size());
    return file_exts[index];
}

END_NCBI_SCOPE
