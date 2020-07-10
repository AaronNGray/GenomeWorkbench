/*  $Id: pdf_font_handler.cpp 44822 2020-03-23 16:10:20Z evgeniev $
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
 *   CPdfFontHandler - handle PDF fonts
 */


#include <ncbi_pch.hpp>
#include <gui/print/pdf_font_handler.hpp>
#include <gui/utils/id_generator.hpp>
#include <gui/print/pdf_object.hpp>
//#include <gui/opengl/ftglfontmanager.hpp>


BEGIN_NCBI_SCOPE



CPdfFontHandler::CPdfFontHandler(CIdGenerator* objid_generator)
    : m_ObjIdGenerator(objid_generator)
{
    Init();
}


CPdfFontHandler::~CPdfFontHandler()
{
}


string CPdfFontHandler::GetFontName(EFontFace fontface) const
{
    TFontMap::const_iterator it = m_FontNames.find(fontface);
    if (it == m_FontNames.end()) {
       NCBI_THROW(CException, eUnknown, "CPdfFontHandler::GetFontName: font face "
                                        + NStr::IntToString(int(fontface)) + " not loaded");
    }

    return it->second;
}


void CPdfFontHandler::x_AddFont(EFontFace fontface)
{
    TPdfObjectRef f(new CPdfObject(m_ObjIdGenerator->NextId()));
    CPdfObject& _f = *f;
    const string name("F" + NStr::NumericToString(m_FontNum));

    EFontFace pdf_face;

    _f["Type"] = new CPdfName("Font");
    _f["Subtype"] = new CPdfName("Type1");
    //_f["Subtype"] = new CPdfName("TrueType");
    _f["Name"] = new CPdfName(name);
    _f["BaseFont"] = new CPdfName(CGlTextureFont::PdfBaseFontName(fontface, pdf_face));

    m_FontNames[fontface] = name;
    m_Fonts.push_back(f);

    ++m_FontNum;
}


void CPdfFontHandler::Init(void)
{
    vector<string> faces;
    CGlTextureFont::GetAllFaces(faces);
    EFontFace pdf_face;

    // Add all fonts that can be specified as type-1 (built-in) fonts
    for (size_t i=0; i<faces.size(); ++i) {
        EFontFace f = CGlTextureFont::FaceFromString(faces[i]);
        if ( CGlTextureFont::PdfBaseFontName(f, pdf_face) != "" )
            x_AddFont(CGlTextureFont::FaceFromString(faces[i]));
    }
}


const CPdfFontHandler::TObjectList& CPdfFontHandler::GetFontObjects(void) const
{
    return m_Fonts;
}


END_NCBI_SCOPE
