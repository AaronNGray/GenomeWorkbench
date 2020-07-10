#ifndef GUI_UTILS__PDF_OBJECT_HPP
#define GUI_UTILS__PDF_OBJECT_HPP

/*  $Id: pdf_object.hpp 33070 2015-05-20 19:55:09Z falkrb $
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
 *   CPdfObject - Stream for output of Adobe PDF objects
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/print/pdf_element.hpp>
#include <gui/print/pdf_font_handler.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/matrix3.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/gltexturefont.hpp>


BEGIN_NCBI_SCOPE



class NCBI_GUIPRINT_EXPORT CPdfObject : public CObject
{
public:
    enum EBitCount { e8Bit = 8, e16Bit = 16, e24Bit = 24, e32Bit = 32 };
    enum EPdfFilter { eNone, eASCIIHex };

public:
    typedef CRef<CPdfElement> TPdfEltRef;
    typedef CGlTextureFont::EFontFace EFontFace;



    CPdfObject(unsigned int obj_num,
               unsigned int generation = 0,
               bool allowCompression = false);

    // direct access to write buffer
    CNcbiOstream& GetWriteBuffer(void);

    /// gs
    void PushGraphicsState();
    void PopGraphicsState();
    void SetGraphicsState(const string& state);
    /// RG
    void SetColor(const CRgbaColor& c);
    /// rg
    void SetFillColor(const CRgbaColor& c);
    void SetLineWidth(float w);
    void SetLineCapStyle(int cap_style);
    void SetLineJoinStyle(int join_style);
    // Set dashes with same paramters used by glLineStipple
    void SetLineDashStyle(int factor, short pattern);
    void Line(CVect2<float>& p1, CVect2<float>& p2);
    void SetClipBox(int x, int y, int width, int height);
    void Text(CRef<CPdfFontHandler> font_handler,
              EFontFace face, 
              float font_size, 
              CVect2<float>& p, 
              const char* txt);
    void Text(CRef<CPdfFontHandler> font_handler,
              EFontFace face, 
              float font_size, 
              CMatrix3<double>& mat, 
              const char* txt);
    void StartTris(EPdfFilter filter,
                   EBitCount bits_per_coord, 
                   int range_minx, 
                   int range_maxx,
                   int range_miny,
                   int range_maxy);

    void Tri(CVect2<float>& p1, CVect2<float>& p2, CVect2<float>& p3);
    void Poly(vector<CVect2<float> >& verts);

    void Tri(int edge_flag,
             CVect2<float> p1, const CRgbaColor& c1,
             CVect2<float> p2, const CRgbaColor& c2, 
             CVect2<float> p3, const CRgbaColor& c3);
    void Tri(int edge_flag,
             CVect2<float> p1, const CRgbaColor& c1);
    void EndTris();

    void Quad(CVect2<float>& p1, 
              CVect2<float>& p2, 
              CVect2<float>& p3, 
              CVect2<float>& p4);


    //void SetLineWidth();
    //void SetLineElbow()

    unsigned int GetObjNum(void) const;
    unsigned int GetGeneration(void) const;
    virtual string GetSeparator(void) const;

    // print the CPdfObject
    virtual void PrintTo(CNcbiOstream& stream) const;

    CPdfObject& operator<<(CPdfObject& (*pf)(CPdfObject&));

    CPdfDictionary& GetDictionary(void);
    TPdfEltRef& operator[](const string& key);

protected:
    void x_PrintTo(CNcbiOstream& stream);

    /// Convert floating point values into hexidecimal based on the 
    /// current range and m_BitsPerCood
    string x_GetHex(double f);
    string x_GetHex(double f, int bits);


private:
    unsigned int m_ObjNum;
    unsigned int m_Generation;
    CNcbiOstrstream m_Buffer;  
    CPdfDictionary m_Dictionary;
    bool m_AllowCompression;       

    /// Current State
    CRgbaColor m_DrawColor;
    bool m_DrawColorSet;
    CRgbaColor m_FillColor;
    bool m_FillColorSet;
    string m_FontName;
    float m_FontSize;

    /// Current triangle rendering parameters
    EPdfFilter m_PdfFilter;
    EBitCount m_BitsPerCoord;

    int m_RangeMinX;
    int m_RangeMaxX;
    int m_RangeMinY;
    int m_RangeMaxY;

    /// multipliers for vertex values based on ratio of range max value
    double m_PositionMultX;
    double m_PositionMultY;
};


inline CPdfObject& pdfbrk(CPdfObject& pdfstrm)
{
    pdfstrm.GetWriteBuffer() << pdfstrm.GetSeparator();
    return pdfstrm;
}


template <typename T>
inline CPdfObject& operator<<(CPdfObject& pdfobj, const T& obj)
{
    pdfobj.GetWriteBuffer() << obj;
    return pdfobj;
}


inline CNcbiOstream& operator<<(CNcbiOstream& strm, const CPdfObject& obj)
{
    obj.PrintTo(strm);
    return strm;
}


class CPdfTrailer : public CPdfObject
{
public:
    CPdfTrailer() : CPdfObject(0) {}
    virtual ~CPdfTrailer() {}
    virtual void PrintTo(CNcbiOstream& stream) const;
};


template <typename T>
inline CPdfTrailer& operator<<(CPdfTrailer& pdfobj, const T& obj)
{
    NCBI_THROW(CException, eUnknown, "CPdfObject::operator<< - illegal use of CPdfTrailer");
}



END_NCBI_SCOPE


#endif // GUI_UTILS__PDF_OBJECT_HPP
