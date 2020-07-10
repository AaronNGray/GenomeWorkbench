#ifndef GUI_UTILS__PDF_ELEMENT_HPP
#define GUI_UTILS__PDF_ELEMENT_HPP

/*  $Id: pdf_element.hpp 27629 2013-03-14 19:41:33Z falkrb $
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
 *   CPdfElement- Represent compoenents of PDF obejcts
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/utils/matrix3.hpp>

#include <map>


BEGIN_NCBI_SCOPE



class CPdfObject;

class NCBI_GUIPRINT_EXPORT CPdfElement : public CObject
{
public:
    typedef CRef<CPdfElement> TPdfEltRef;

    CPdfElement(const string& str);
    CPdfElement();
    virtual ~CPdfElement();

    // print the CPdfElement
    virtual void PrintTo(CNcbiOstream& stream) const;

    virtual string GetValue(void) const;

protected:
    string m_Str;
};


inline CNcbiOstream& operator<<(CNcbiOstream& strm, const CPdfElement& obj)
{
    obj.PrintTo(strm);
    return strm;
}


class NCBI_GUIPRINT_EXPORT CPdfRotate : public CPdfElement
{
public:
    CPdfRotate(double angle);
    virtual ~CPdfRotate();

    virtual void PrintTo(CNcbiOstream& stream) const;

private:
    double m_Rot;
};


class NCBI_GUIPRINT_EXPORT CPdfTranslate : public CPdfElement
{
public:
    CPdfTranslate(double xoff, double yoff);
    CPdfTranslate(int xoff, int yoff);
    CPdfTranslate(unsigned int xoff, unsigned int yoff);
    virtual ~CPdfTranslate();

    virtual void PrintTo(CNcbiOstream& stream) const;

private:
    double m_XOff;
    double m_YOff;
};

class NCBI_GUIPRINT_EXPORT CPdfTransform : public CPdfElement
{
public:
    CPdfTransform(const CMatrix3<double>& m);  
    virtual ~CPdfTransform() {}

    virtual void PrintTo(CNcbiOstream& stream) const;

private:

    CMatrix3<double> m_Mat;
};


class NCBI_GUIPRINT_EXPORT CPdfTextTransform : public CPdfElement
{
public:
    CPdfTextTransform(const CMatrix3<double>& m);  

    virtual void PrintTo(CNcbiOstream& stream) const;

private:

    CMatrix3<double> m_Mat;
};


class NCBI_GUIPRINT_EXPORT CPdfNumber : public CPdfElement
{
public:
    CPdfNumber(double d, int precision=-1);
    CPdfNumber(int i, int precision=-1);
    CPdfNumber(unsigned int i, int precision=-1);
    virtual ~CPdfNumber();

    virtual void PrintTo(CNcbiOstream& stream) const;

private:
    double m_Num;
    int m_Precision;
};


class NCBI_GUIPRINT_EXPORT CPdfString : public CPdfElement
{
public:
    CPdfString(const string& str);
    virtual ~CPdfString();

    virtual void PrintTo(CNcbiOstream& stream) const;
};


class NCBI_GUIPRINT_EXPORT CPdfName : public CPdfElement
{
public:
    CPdfName(const string& str);
    virtual ~CPdfName();

    virtual void PrintTo(CNcbiOstream& stream) const;
};


class NCBI_GUIPRINT_EXPORT CPdfIndirectObj : public CPdfElement
{
public:
    CPdfIndirectObj(const CRef<CPdfObject>& obj);
    virtual ~CPdfIndirectObj();

    virtual void PrintTo(CNcbiOstream& stream) const;

private:
    CRef<CPdfObject> m_Obj;
};


class NCBI_GUIPRINT_EXPORT CPdfArray : public CPdfElement
{
public:
    typedef vector<TPdfEltRef> TArray;

    TArray& GetArray(void);

    virtual void PrintTo(CNcbiOstream& stream) const;
    void Add(const CRef<CPdfArray>& arr);

private:
    TArray m_Array;
};


class NCBI_GUIPRINT_EXPORT CPdfDictionary : public CPdfElement
{
public:
    CPdfDictionary() : m_MaxShaderID(0) {}

    TPdfEltRef& operator[](const string& key);
    virtual void PrintTo(CNcbiOstream& stream) const;
    void Add(const CRef<CPdfDictionary>& dict);
    /// Get a new (unique) shader ID of form sh##
    string GenShaderID();

private:
    typedef map<string, TPdfEltRef> TDict;
    TDict m_Dict;
    /// Shaders need unique ID strings which will use this #
    int m_MaxShaderID;
};



END_NCBI_SCOPE


#endif // GUI_UTILS__PDF_ELEMENT_HPP
