/*  $Id: pdf_element.cpp 29243 2013-11-18 19:56:58Z falkrb $
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
 *   CPdfElement - Stream for output of Adobe PDF objects
 */

#include <ncbi_pch.hpp>
#include <gui/print/pdf_element.hpp>
#include <gui/print/pdf_object.hpp>
#include "pdf_object_writer.hpp"
#include <math.h>


BEGIN_NCBI_SCOPE



CPdfElement::CPdfElement(const string& str) : m_Str(str)
{
}


CPdfElement::CPdfElement()
{
}


CPdfElement::~CPdfElement()
{
}


void CPdfElement::PrintTo(CNcbiOstream& stream) const
{
    stream << m_Str;
}


string CPdfElement::GetValue(void) const
{
    return m_Str;
}


CPdfNumber::CPdfNumber(unsigned int i, int precision)
    : m_Num(i)
    , m_Precision(precision)
{
}


CPdfNumber::CPdfNumber(int i, int precision)
    : m_Num(i)
    , m_Precision(precision)
{
}


CPdfNumber::CPdfNumber(double d, int precision)
    : m_Num(d)
    , m_Precision(precision)
{
}


CPdfNumber::~CPdfNumber()
{
}


void CPdfNumber::PrintTo(CNcbiOstream& stream) const
{
    char buf[256];

    // Precision not specified, use the default (6) which is pretty high.
    // We don't pick a smaller one since the needed precision varies
    // based on viewing matrices.
    if (m_Precision == -1) {
        ::sprintf(buf, "%f", m_Num);
    }
    else {
        // But if user asks for a precision, use it.
        ::sprintf(buf, "%.*f", m_Precision, m_Num);
    }
  
    // Remove any trailing 0's since they waste space in the file, e.g
    // 3.950=> 3.95, 6.000 => 6, 90=>90 and 0.=>0
    if (strchr(buf, '.') != NULL) {
        char *pos;
        int len = (int)strlen(buf);

        pos = buf + len - 1;

        while(*pos == '0') {
            *pos-- = '\0';
        }

        if(*pos == '.') {
            *pos = '\0';
        }
    }

    stream << buf;
}


CPdfRotate::CPdfRotate(double angle) : m_Rot(angle)
{
}


CPdfRotate::~CPdfRotate()
{
}


void CPdfRotate::PrintTo(CNcbiOstream& stream) const
{
    const double rad = m_Rot * 3.1415926535 / 180;
    const double c = cos(rad);
    const double s = sin(rad);
    stream << setiosflags(ios::fixed) << CPdfNumber(c) << 
        ' ' << CPdfNumber(s) << ' ' << CPdfNumber(-s) << ' ' << CPdfNumber(c) << " 0 0 cm" << pdfeol;
}


CPdfTranslate::CPdfTranslate(double xoff, double yoff)
    : m_XOff(xoff),
      m_YOff(yoff)
{
}


CPdfTranslate::CPdfTranslate(unsigned int xoff, unsigned int yoff)
    : m_XOff(xoff),
      m_YOff(yoff)
{
}


CPdfTranslate::CPdfTranslate(int xoff, int yoff)
    : m_XOff(xoff),
      m_YOff(yoff)
{
}


CPdfTranslate::~CPdfTranslate()
{
}


void CPdfTranslate::PrintTo(CNcbiOstream& stream) const
{
    stream << "1 0 0 1 " << setiosflags(ios::fixed)
           << CPdfNumber(m_XOff) << ' ' << CPdfNumber(m_YOff) << " cm" << pdfeol;
}

CPdfTransform::CPdfTransform(const CMatrix3<double>& m)
: m_Mat(m)
{
}

void CPdfTransform::PrintTo(CNcbiOstream& stream) const
{
    stream << setiosflags(ios::fixed); 

    // Matrices need a little extra precision than other numbers (can get errors if viewing
    // area is very large)
    for (int c=0; c<3; ++c) 
        for (int r=0; r<2; ++r)
            stream << CPdfNumber(m_Mat(r,c), 12) << " ";

    stream << " cm" << pdfeol;
}


CPdfTextTransform::CPdfTextTransform(const CMatrix3<double>& m)
: m_Mat(m)
{
}

void CPdfTextTransform::PrintTo(CNcbiOstream& stream) const
{
    stream << setiosflags(ios::fixed);

    for (int c=0; c<3; ++c) 
        for (int r=0; r<2; ++r) {           
            stream << CPdfNumber(m_Mat(r,c), 12) << " ";
        }

    stream << " Tm" << pdfeol;
}



CPdfString::CPdfString(const string& str) : CPdfElement(str)
{
}


CPdfString::~CPdfString()
{
}


void CPdfString::PrintTo(CNcbiOstream& stream) const
{
    stream << '(' << m_Str << ')';
}


CPdfName::CPdfName(const string& str) : CPdfElement(str)
{
}


CPdfName::~CPdfName()
{
}


void CPdfName::PrintTo(CNcbiOstream& stream) const
{
    stream << '/' << m_Str;
}


CPdfIndirectObj::CPdfIndirectObj(const CRef<CPdfObject>& obj) : m_Obj(obj)
{
}


CPdfIndirectObj::~CPdfIndirectObj()
{
}


void CPdfIndirectObj::PrintTo(CNcbiOstream& stream) const
{
    stream << m_Obj->GetObjNum() << ' ' << m_Obj->GetGeneration() << " R";
}


CPdfArray::TArray& CPdfArray::GetArray(void)
{
    return m_Array;
}


void CPdfArray::PrintTo(CNcbiOstream& stream) const
{
    if (m_Array.size() == 0) {
        return;
    }

    stream << '[';

    bool first = true;
    ITERATE(vector<TPdfEltRef>, it, m_Array) {
        if (first) {
            first = false;
        } else {
            stream << ' ';
        }

        stream << **it;
    }

    stream << ']' << pdfeol;
}


void CPdfArray::Add(const CRef<CPdfArray>& array)
{
    m_Array.insert(m_Array.end(),
                   array->m_Array.begin(), array->m_Array.end());
}


CPdfDictionary::TPdfEltRef& CPdfDictionary::operator[](const string& key)
{
    return m_Dict[key];
}


void CPdfDictionary::PrintTo(CNcbiOstream& stream) const
{
    if (m_Dict.size() == 0) {
        return;
    }

    stream << "<<";

    ITERATE(TDict, it, m_Dict) {
        const CPdfElement* val = it->second.GetPointer();

        bool insert_space = false;
        if (dynamic_cast < const CPdfIndirectObj*>(val)  ||
            dynamic_cast < const CPdfNumber*>(val)) {
            insert_space = true;
        }

        stream << '/' << it->first;
        if (insert_space) {
            stream << ' ';
        }
        stream << *val;
    }

    stream << ">>" << pdfeol;
}


void CPdfDictionary::Add(const CRef<CPdfDictionary>& dict)
{
    //
    // std::map::insert(beg, end) doesn't seem to work on MSVC++ 6.0.
    // Iterate "manually" and copy each element instead.
    //

    ITERATE(TDict, it, dict->m_Dict) {
        m_Dict.insert(*it);
    }
}

string CPdfDictionary::GenShaderID()
{
    return "sh" + NStr::IntToString(m_MaxShaderID++);
}



END_NCBI_SCOPE
