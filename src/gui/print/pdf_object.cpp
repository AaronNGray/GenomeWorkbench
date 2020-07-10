/*  $Id: pdf_object.cpp 43844 2019-09-10 14:06:00Z evgeniev $
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
 *   CPdfObject - Stream for output of Adobe PDF objects
 */


#include <ncbi_pch.hpp>
#include <gui/print/pdf_object.hpp>
#include <gui/print/pdf_element.hpp>
#include <gui/utils/vect2.hpp>

#include "pdf_object_writer.hpp"
#include <util/compress/stream.hpp>
#include <util/compress/zlib.hpp>
#include <util/ascii85.hpp>
#include <gui/print/pdf_font_handler.hpp>


BEGIN_NCBI_SCOPE



CPdfObject::CPdfObject(unsigned int obj_num,
                       unsigned int generation,
                       bool allowCompression
                      )
: m_ObjNum(obj_num)
, m_Generation(generation)
, m_AllowCompression(allowCompression)
, m_DrawColor(0.0f, 0.0f, 0.0f, 1.0f) 
, m_DrawColorSet(false)
, m_FillColor(0.0f, 0.0f, 0.0f, 1.0f)       
, m_FillColorSet(false)
, m_FontSize(0)
, m_PdfFilter(eNone)
, m_BitsPerCoord(e16Bit)
{
}


enum EPdfObjectFormat
{
    eUncompressed,
    eCompressed
};

void CPdfObject::PushGraphicsState() 
{
    m_Buffer << "q" << pdfeol;
}

void CPdfObject::PopGraphicsState()
{
    m_Buffer << "Q" << pdfeol;
}

void CPdfObject::SetGraphicsState(const string& state)
{
    m_Buffer << "/" << state << " gs " << pdfeol;
}

void CPdfObject::SetColor(const CRgbaColor& c)
{
    // Only change when needed to keep size down. 
    if (!m_DrawColorSet || !(c == m_DrawColor)) {
        m_Buffer << CPdfNumber(c.GetRed(), 3) << " " << 
                    CPdfNumber(c.GetGreen(), 3) << " " <<
                    CPdfNumber(c.GetBlue(), 3) << " RG" << pdfeol;
        m_DrawColor = c;
        m_DrawColorSet = true; 
    }
}

void CPdfObject::SetFillColor(const CRgbaColor& c)
{
    // Only change when needed to keep size down.
    if (!m_FillColorSet || !(c == m_FillColor)) {
        m_Buffer << CPdfNumber(c.GetRed(), 3) << " " << 
                    CPdfNumber(c.GetGreen(), 3) << " " <<
                    CPdfNumber(c.GetBlue(), 3) << " rg" << pdfeol;
        m_FillColor = c;
        m_FillColorSet = true;
    }
}

void CPdfObject::SetLineWidth(float w)
{
    m_Buffer << CPdfNumber(w) << " w" << pdfeol;
}

void CPdfObject::SetLineCapStyle(int cap_style)
{
     m_Buffer << cap_style << " J" << pdfeol;
}

void CPdfObject::SetLineJoinStyle(int join_style)
{
    m_Buffer << join_style << " j" << pdfeol;
}

void CPdfObject::SetLineDashStyle(int factor, short pattern)
{
    if (pattern == 0) {
        // solid line
        m_Buffer << "[] 0 d" << pdfeol;
        return;
    }

    string str_pattern = "[";
    // If a pattern starts with 'off' stipples (0's), then those are placed at 
    // the end of the pattern since the first number in pdf patterns is always 
    // 'on'.  The pdf initial offset number then jumps the pattern to that
    // point to start in 'off'.
    string trailing_num = "";
    int mask = 1;
    int start_bit = 0;
    bool is_on = (pattern&mask) != 0;
    int initial_offset = 0;
    int total = 0;

    for (int i=0; i<16; ++i) {
        if (((pattern&mask)!=0) != is_on) {

            int delta = (i - start_bit)*factor;                
            string sval = NStr::IntToString(delta);

            if (start_bit == 0 && !is_on) {
                trailing_num = sval;
            }
            else {
                str_pattern += " " + sval;
                total += delta;
            }

            start_bit = i;
            is_on = ((pattern&mask)!=0);
        }        

        mask<<=1;
    }

    int delta = (16 - start_bit)*factor;
    total += delta;
    string sval = NStr::IntToString(delta);
    str_pattern += " " + sval;

    if (trailing_num != "") {
        str_pattern += " " + trailing_num;
        initial_offset = total;
    }

    str_pattern += " ]";

    m_Buffer << str_pattern << " " << NStr::IntToString(initial_offset) << " d" << pdfeol;
}

void CPdfObject::Line(CVect2<float>& p1, CVect2<float>& p2)
{
    m_Buffer << CPdfNumber(p1.X()) << " " << 
                CPdfNumber(p1.Y()) << " m " << 
                CPdfNumber(p2.X()) << " " << 
                CPdfNumber(p2.Y()) << " l s" << pdfeol;
}

void CPdfObject::SetClipBox(int x, int y, int width, int height)
{
    m_Buffer << CPdfNumber(x) << " " <<
                CPdfNumber(y) << " m" << pdfeol;
    m_Buffer << CPdfNumber(x + width) << " " <<
                CPdfNumber(y) << " l" << pdfeol;
    m_Buffer << CPdfNumber(x + width) << " " <<
                CPdfNumber(y + height) << " l" << pdfeol;
    m_Buffer << CPdfNumber(x) << " " <<
                CPdfNumber(y+height) << " l h W n" << pdfeol;
}

void CPdfObject::Text(CRef<CPdfFontHandler> font_handler,
                      EFontFace face, 
                      float font_size, 
                      CVect2<float>& p, 
                      const char* txt)
{
    // output the text, font and font size
    m_Buffer << "BT" << pdfeol;
    const string fontname = font_handler->GetFontName(face);
    
    // Only change font if state changed since last write
    if (m_FontName != fontname || m_FontSize != font_size) {
        m_Buffer << CPdfName(fontname) << " " << 
               CPdfNumber(font_size) << " Tf" << pdfeol;
        m_FontName = fontname;
        m_FontSize = font_size;
    }

    // Set text matrix to identity before transforming
    CMatrix3<double> imat;
    imat.Identity();
    m_Buffer << setiosflags(ios::fixed) << CPdfTextTransform(imat) << '\n';
    m_Buffer << CPdfNumber(p[0]) << ' ' << CPdfNumber(p[1]) << " Td" << pdfeol;

    m_Buffer << "(";
    // Parentheses and backslashes must be escaped.  Balanced parantheses
    // don't actually NEED to be escaped, but it's easier to just escape them
    // all.
    for (size_t i=0; i<strlen(txt); ++i) {
        if (txt[i] == '\\' || txt[i] == '(' || txt[i] == ')') 
            m_Buffer << "\\";
        m_Buffer << txt[i];
    }

    m_Buffer << ") Tj" << pdfeol;
    m_Buffer << "ET" << pdfeol;
}

void CPdfObject::Text(CRef<CPdfFontHandler> font_handler,
                      EFontFace face, 
                      float font_size, 
                      CMatrix3<double>& mat, 
                      const char* txt)
{
    // output the text, font and font size
    m_Buffer << "BT" << pdfeol;
    m_Buffer << setiosflags(ios::fixed) << CPdfTextTransform(mat) << '\n';
    const string fontname = font_handler->GetFontName(face);

    // Only change font if state changed since last write
    if (m_FontName != fontname || m_FontSize != font_size) {
        m_Buffer << CPdfName(fontname) << " " << 
               CPdfNumber(font_size) << " Tf" << pdfeol;
        m_FontName = fontname;
        m_FontSize = font_size;
    }

     m_Buffer << "(";
    // Parentheses and backslashes must be escaped.  Balanced parantheses
    // don't actually NEED to be escaped, but it's easier to just escape them
    // all.
    for (size_t i=0; i<strlen(txt); ++i) {
        if (txt[i] == '\\' || txt[i] == '(' || txt[i] == ')') 
            m_Buffer << "\\";
        m_Buffer << txt[i];
    }

    m_Buffer << ") Tj" << pdfeol;
    m_Buffer << "ET" << pdfeol;
}

void CPdfObject::Tri(CVect2<float>& p1, CVect2<float>& p2, CVect2<float>& p3)
{
    m_Buffer << setiosflags(ios::fixed) << CPdfNumber(p1.X()) << " " << CPdfNumber(p1.Y()) << " m ";
    m_Buffer << CPdfNumber(p2.X()) << " " << CPdfNumber(p2.Y()) << " l " << pdfeol;
    m_Buffer << CPdfNumber(p3.X()) << " " << CPdfNumber(p3.Y()) << " l h f" << pdfeol;
}

void CPdfObject::Poly(vector<CVect2<float> >& verts)
{
    if (verts.size() < 2)
        return;

    m_Buffer << setiosflags(ios::fixed) << CPdfNumber(verts[0].X()) << " " << CPdfNumber(verts[0].Y()) << " m ";

    for (size_t i=1; i<verts.size(); ++i) 
        m_Buffer << CPdfNumber(verts[i].X()) << " " << CPdfNumber(verts[i].Y()) << " l " << pdfeol;

    m_Buffer << " h f" << pdfeol;
}

void CPdfObject::Quad(CVect2<float>& p1, 
                      CVect2<float>& p2, 
                      CVect2<float>& p3, 
                      CVect2<float>& p4)
{
    m_Buffer << setiosflags(ios::fixed) << CPdfNumber(p1.X()) << " " << CPdfNumber(p1.Y()) << " m ";
    m_Buffer << CPdfNumber(p2.X()) << " " << CPdfNumber(p2.Y()) << " l " << pdfeol;
    m_Buffer << CPdfNumber(p3.X()) << " " << CPdfNumber(p3.Y()) << " l " << pdfeol;
    m_Buffer << CPdfNumber(p4.X()) << " " << CPdfNumber(p4.Y()) << " l h f" << pdfeol;
}

void CPdfObject::StartTris(EPdfFilter filter,
                           EBitCount bits_per_coord, 
                           int range_minx, 
                           int range_maxx,
                           int range_miny,
                           int range_maxy)
{
    m_PdfFilter = filter;
    m_BitsPerCoord = bits_per_coord;
    m_RangeMinX = range_minx;
    m_RangeMaxX = range_maxx;
    m_RangeMinY = range_miny;
    m_RangeMaxY = range_maxy;   

    // multiplier for vertex values based on ratio of range max value   
    // Use doubles/Int8 so that you can get multiply by 2^32 and
    // retain precision
    int shift = (int)bits_per_coord;  
    Uint8  pow2 = (Uint8(1)<<shift) - 1;  // (16=0xFFFF, 24=0xFFFFFF ...)

    // I think I lose a little precision multiplying by the larger
    // numbers (24 && 32 bit).  And VStudio doesn't support
    // long double so maybe some other approach is needed. e16Bit
    // currently gives the best result.
    m_PositionMultX = double(pow2)/double(range_maxx-range_minx);
    m_PositionMultY = double(pow2)/double(range_maxy-range_miny);

    string xyrange = NStr::Int8ToString(range_minx) + " " +
                     NStr::Int8ToString(range_maxx) + " " +
                     NStr::Int8ToString(range_miny) + " " +
                     NStr::Int8ToString(range_maxy) + " ";


    (*this)["ShadingType"] = new CPdfNumber(4);
    (*this)["ColorSpace"] = new CPdfName("DeviceRGB");
    (*this)["Decode"] = new CPdfElement(" [ " + xyrange + " 0 1 0 1 0 1 ]");
    (*this)["BitsPerComponent"] = new CPdfNumber(8);
    (*this)["BitsPerCoordinate"] = new CPdfNumber(m_BitsPerCoord);
    (*this)["BitsPerFlag"] = new CPdfNumber(8);

    if (filter == eASCIIHex)
        (*this)["Filter"] = new CPdfName("ASCIIHexDecode");
}

void CPdfObject::EndTris()
{
    if (m_PdfFilter != eNone) {
        m_Buffer << " >" << pdfeol;
    }
}

string CPdfObject::x_GetHex(double v)
{
    return x_GetHex(v, int(m_BitsPerCoord));
}

string CPdfObject::x_GetHex(double v, int bits)
{
    size_t digits = (size_t)(bits/4);

    string val;    
    //should round: Int8(floor(v+1.0)) but would also need to clamp results to range.
    NStr::NumericToString(val, Int8(v), 0, 16);   

    // pad left with 0s
    for (size_t i=val.length(); i<digits; ++i)
        val = "0" + val;
    
    // truncate
    if (val.length() > digits)
        val = val.substr(0, digits);

    return val;
}


void CPdfObject::Tri(int edge_flag,
                     CVect2<float> p1, const CRgbaColor& c1,
                     CVect2<float> p2, const CRgbaColor& c2, 
                     CVect2<float> p3, const CRgbaColor& c3)
{
    // convert numbers from initial range to values from 0 to 2^m_BitsPerCoord-1 (eg 0xFF for 16 bits)
    double p1x = (double(p1.X())-double(m_RangeMinX))*m_PositionMultX;
    double p1y = (double(p1.Y())-double(m_RangeMinY))*m_PositionMultY;
    double p2x = (double(p2.X())-double(m_RangeMinX))*m_PositionMultX;
    double p2y = (double(p2.Y())-double(m_RangeMinY))*m_PositionMultY;
    double p3x = (double(p3.X())-double(m_RangeMinX))*m_PositionMultX;
    double p3y = (double(p3.Y())-double(m_RangeMinY))*m_PositionMultY;    

    if (m_PdfFilter == eNone) {
        unsigned char ef = (unsigned char)edge_flag;
        m_Buffer.write((char*)&ef, 1);
        unsigned short sx = (unsigned short)p1.X();
        m_Buffer.write((char*)&sx, 2);
        unsigned short sy = (unsigned short)p1.Y();
        m_Buffer.write((char*)&sy, 2);

        unsigned char c = c1.GetRedUC();
        m_Buffer.write((char*)&c, 1);
        c = c1.GetGreenUC();
        m_Buffer.write((char*)&c, 1);
        c = c1.GetBlueUC();
        m_Buffer.write((char*)&c, 1);

        sx = (unsigned short)p2.X();
        m_Buffer.write((char*)&sx, 2);
        sy = (unsigned short)p2.Y();
        m_Buffer.write((char*)&sy, 2);

        c = c2.GetRedUC();
        m_Buffer.write((char*)&c, 1);
        c = c2.GetGreenUC();
        m_Buffer.write((char*)&c, 1);
        c = c2.GetBlueUC();
        m_Buffer.write((char*)&c, 1);

        sx = (unsigned short)p3.X();
        m_Buffer.write((char*)&sx, 2);
        sy = (unsigned short)p3.Y();
        m_Buffer.write((char*)&sy, 2);

        c = c3.GetRedUC();
        m_Buffer.write((char*)&c, 1);
        c = c3.GetGreenUC();
        m_Buffer.write((char*)&c, 1);
        c = c3.GetBlueUC();
        m_Buffer.write((char*)&c, 1);

        //m_Buffer << pdfeol;
    }
    else {            
        string ehex = NStr::IntToString(edge_flag, 0, 16);
        if (ehex.length() == 1)
            ehex = "0" + ehex;

        m_Buffer << ehex << x_GetHex(p1x) << x_GetHex(p1y) << x_GetHex(c1.GetRedUC(),8) << x_GetHex(c1.GetGreenUC(),8) << x_GetHex(c1.GetBlueUC(),8) << pdfeol;
        m_Buffer << ehex << x_GetHex(p2x) << x_GetHex(p2y) << x_GetHex(c2.GetRedUC(),8) << x_GetHex(c2.GetGreenUC(),8) << x_GetHex(c2.GetBlueUC(),8) << pdfeol;
        m_Buffer << ehex << x_GetHex(p3x) << x_GetHex(p3y) << x_GetHex(c3.GetRedUC(),8) << x_GetHex(c3.GetGreenUC(),8) << x_GetHex(c3.GetBlueUC(),8) << pdfeol;       
    }
}

void CPdfObject::Tri(int edge_flag,
                     CVect2<float> p1, const CRgbaColor& c1)
{
    // convert numbers from initial range to values from 0 to 2^m_BitsPerCoord-1 (eg 0xFF for 16 bits)
    double p1x = (double(p1.X())-double(m_RangeMinX))*m_PositionMultX;
    double p1y = (double(p1.Y())-double(m_RangeMinY))*m_PositionMultY;

    if (m_PdfFilter == eNone) {
        unsigned char ef = (unsigned char)edge_flag;
        m_Buffer.write((char*)&ef, 1);
        unsigned short sx = (unsigned short)p1.X();
        m_Buffer.write((char*)&sx, 2);
        unsigned short sy = (unsigned short)p1.Y();
        m_Buffer.write((char*)&sy, 2);

        unsigned char c = c1.GetRedUC();
        m_Buffer.write((char*)&c, 1);
        c = c1.GetGreenUC();
        m_Buffer.write((char*)&c, 1);
        c = c1.GetBlueUC();
        m_Buffer.write((char*)&c, 1);
    }
    else {            
        string ehex = NStr::IntToString(edge_flag, 0, 16);
        if (ehex.length() == 1)
            ehex = "0" + ehex;

        m_Buffer << ehex << x_GetHex(p1x) << x_GetHex(p1y) << x_GetHex(c1.GetRedUC(),8) << x_GetHex(c1.GetGreenUC(),8) << x_GetHex(c1.GetBlueUC(),8) << pdfeol;
    }
}


unsigned int CPdfObject::GetObjNum(void) const
{
    return m_ObjNum;
}


unsigned int CPdfObject::GetGeneration(void) const
{
    return m_Generation;
}


void CPdfObject::PrintTo(CNcbiOstream& stream) const
{
    // start the page content object
    stream << m_ObjNum << ' ' << m_Generation << " obj" << endl;
    const_cast<CPdfObject*>(this)->x_PrintTo(stream);
    stream << "endobj" << endl;
}


void CPdfObject::x_PrintTo(CNcbiOstream& stream)
{
    Int8 os_len = GetOssSize(m_Buffer);
    const bool compress = m_AllowCompression && os_len > 4096;
    AutoPtr< char, ArrayDeleter<char> > strmbuf;

    if (os_len > 0) {
        // add the stream contents
        if (compress) {
            const size_t cbuflen = size_t(os_len * 1.03);
            AutoPtr< char, ArrayDeleter<char> > cbuf(new char[cbuflen]);
            const string str = CNcbiOstrstreamToString(m_Buffer);

            // compress the data
            CZipCompression c;
            size_t len;
            c.CompressBuffer(str.data(), os_len, cbuf.get(), cbuflen, &len);

            // encode the data using ASCII-85
            // ASCII-85 encoding produces five ASCII characters for
            // every four bytes of binary data
            const size_t buflen = size_t(len * 1.3);
            strmbuf.reset(new char[buflen]);
            os_len = CAscii85::s_Encode(cbuf.get(), len, strmbuf.get(), buflen);

            // add stream-related attributes to dictionary
            m_Dictionary["Length"] = new CPdfNumber(int(os_len));
            m_Dictionary["Filter"] = new CPdfElement("[/ASCII85Decode/FlateDecode]");
        }
        else {
            // add stream-related attributes to dictionary
            // don't count the last newline in the stream as part of the length
            m_Dictionary["Length"] = new CPdfNumber(int(os_len - 1));
        }
    }

    // write dictionary
    stream << m_Dictionary << endl;

    // write the stream
    if (os_len > 0) {
        stream << "stream" << endl;

        // add the stream contents
        if (compress && strmbuf.get()) {
            const unsigned int line_len = 72;
            const char* ptr = strmbuf.get();
            size_t l = os_len;
            for ( ; l > line_len; l -= line_len, ptr += line_len) {
                stream.write(ptr, line_len) << pdfeol;
            }
            stream.write(ptr, l) << pdfeol;
        }
        else {
            // write uncompressed stream
            // stream may be ostrstream or ostringstream and in the
            // first case .str() returns a non-null terminated char*
            // otherwise a string.
#ifndef NCBI_SHUN_OSTRSTREAM // ostrstream
            stream << string(m_Buffer.str(), os_len);
#else            // ostringstream
            stream << m_Buffer.str();
#endif
            //stream << string(m_Buffer.str().c_str(), os_len);
        }
        stream << "endstream" << endl;
    }
}


CPdfDictionary& CPdfObject::GetDictionary(void)
{
    return m_Dictionary;
}


CPdfObject::TPdfEltRef& CPdfObject::operator[](const string& key)
{
    return m_Dictionary[key];
}


CPdfObject& CPdfObject::operator<<(CPdfObject& (*pf)(CPdfObject&))
{
    return (*pf)(*this);
}


CNcbiOstream& CPdfObject::GetWriteBuffer(void)
{
    return m_Buffer;
}


string CPdfObject::GetSeparator(void) const
{
    return "\n";
}

void CPdfTrailer::PrintTo(CNcbiOstream& stream) const
{
    stream << "trailer" << endl;
    const_cast<CPdfTrailer*>(this)->x_PrintTo(stream);
}



END_NCBI_SCOPE
