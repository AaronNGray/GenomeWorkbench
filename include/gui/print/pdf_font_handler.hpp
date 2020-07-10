#ifndef GUI_UTILS___PDF_FONT__HPP
#define GUI_UTILS___PDF_FONT__HPP

/* $Id: pdf_font_handler.hpp 44822 2020-03-23 16:10:20Z evgeniev $
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
 *   CPdfFontHandler - handle PDF fonts
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/gltexturefont.hpp>

#include <gui/utils/id_generator.hpp>


BEGIN_NCBI_SCOPE


class CPdfObject;


class NCBI_GUIPRINT_EXPORT CPdfFontHandler : public CObject
{
public:
    typedef CRef<CPdfObject> TPdfObjectRef;
    typedef list<TPdfObjectRef> TObjectList;
    typedef CGlTextureFont::EFontFace EFontFace;


public:
    CPdfFontHandler(CIdGenerator* objid_generator);
    virtual ~CPdfFontHandler();

    string GetFontName(EFontFace fontface) const;
    const TObjectList& GetFontObjects(void) const;


protected:
    void Init(void);
    void x_AddFont(EFontFace fontface);


private:
    typedef map<EFontFace, string> TFontMap;

    CRef<CIdGenerator> m_ObjIdGenerator;
    map<EFontFace, string> m_FontNames;
    TObjectList m_Fonts;
    size_t  m_FontNum = 1;
};



END_NCBI_SCOPE


#endif // GUI_UTILS___PDF_FONT__HPP
