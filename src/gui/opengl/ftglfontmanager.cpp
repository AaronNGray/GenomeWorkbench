/*  $Id: ftglfontmanager.cpp 42799 2019-04-16 13:34:52Z shkeda $
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
 *
 */

#include <ncbi_pch.hpp>
#include <gui/opengl/ftglfontmanager.hpp>

#include <corelib/ncbistr.hpp>
#include <corelib/ncbifile.hpp>

#ifdef HAVE_LIBFTGL
#include <FTGL/ftgl.h> 
#endif

#ifdef USE_METAL
#include "MTLTextureFont.h"
#endif


BEGIN_NCBI_SCOPE

static CRWLock s_FontMapLock;

CFtglFontManager& CFtglFontManager::Instance()
{
    static CFtglFontManager tm;
    return tm;
}

CFtglFontManager::~CFtglFontManager()
{
    // Since this object is a singleton, memory (especially OpenGL) may already
    // be cleaned up by the system by the time this dtor is called.  User should call
    // 'Clear' to delete fonts.
    /*
    Clear();
    */
}

void CFtglFontManager::Clear()
{
    FontIter font;
    for(font = fonts.begin(); font != fonts.end(); ++font)
    {
        delete (*font).second;
    }

    fonts.clear();
    m_BinaryData.clear();
}

void CFtglFontManager::SetFontPath(const string& p) 
{ 
    m_FontPath = p; 
    
    // If it is not blank, make sure there is a termination separator, e.g. '/':
    if (!NStr::IsBlank(m_FontPath)) {
        if (!CDirEntry::IsPathSeparator(m_FontPath[m_FontPath.length()-1])) { 
            m_FontPath += CDir::GetPathSeparator();
        }
        m_FontPath += "fonts";
        m_FontPath += CDir::GetPathSeparator();      
    }
}

FTFont* CFtglFontManager::GetFont(const char *filename, 
                                  unsigned int size, 
                                  EFtglFontType ft)
{
#ifdef HAVE_LIBFTGL
    static const char* errMsg = " failed to open. To change font directory call "
                                " CFtglFontManager::Instance().SetFontPath(\"..font dir..\") before loading fonts.";

    char buf[256];
    sprintf(buf, "%s %i %i", filename, size, ft);
    string fontKey = string(buf);
    {{
        CReadLockGuard lock(s_FontMapLock);
        FontIter result = fonts.find(fontKey);
        if(result != fonts.end())
        {
            //_TRACE("Found font " << fontKey << " in list");
            return result->second;
        }
    }}

    string fullname = m_FontPath + string(filename);

    // Look for file first in default directory.  If not found
    // try to use filename as the full path name. If neither
    // work, return null
    {
        CDirEntry f1(fullname);
        if (!f1.Exists()) {
            CDirEntry f2(filename);
            if (!f2.Exists()) {
                ERR_POST(Error << "Font file: " << fullname << errMsg);
                return nullptr;
            }
            else {
                fullname = filename;
            }
        }
    }

    unsigned char* buffer = nullptr;
    size_t bufferSize = 0;

    CWriteLockGuard lock(s_FontMapLock);
    {
        FontIter result = fonts.find(fontKey);
        if(result != fonts.end())
            return result->second;
    }

    auto it = m_BinaryData.find(fullname);
    if (it != m_BinaryData.end()) {
        buffer = std::get<0>(it->second).get();
        bufferSize = std::get<1>(it->second);
    }
    else {
        unique_ptr<unsigned char[]> data;
        try {
            CFileIO file;
            file.Open(fullname, CFileIO::eOpen, CFileIO::eRead);
            bufferSize = (size_t)file.GetFileSize();
            if (bufferSize == 0)
                NCBI_THROW(CException, eUnknown, "Font file is empty");
            data.reset(new unsigned char[bufferSize]);
            if (file.Read(data.get(), bufferSize) != bufferSize)
                NCBI_THROW(CException, eUnknown, "Failed to read font file");
        }
        catch (const CException&) {
            ERR_POST(Error << "Font file: " << fullname << errMsg);
            return nullptr;
        }
        buffer = data.get();

        std::get<0>(m_BinaryData[fullname]) = std::move(data);
        std::get<1>(m_BinaryData[fullname]) = bufferSize;
    }

    FTFont* font = nullptr;
    switch (ft) {
#ifdef USE_METAL
        case eTextureFont:
            font = new MTLTextureFont(buffer , bufferSize);
        default:
            break;
#else
        case eTextureFont:
            font = new FTTextureFont(buffer , bufferSize);
            break;
        case ePixmapFont:
            font = new FTPixmapFont(buffer, bufferSize);
            break;
        case eBitmapFont:
            font = new FTBitmapFont(buffer, bufferSize);
            break;
        case eOutlineFont:
            font = new FTOutlineFont(buffer, bufferSize);
            break;
        case ePolygonFont:
            font = new FTPolygonFont(buffer, bufferSize);
            break;
        case eExtrudedFont:
            font = new FTExtrudeFont(buffer, bufferSize);
            break;
#endif
    };

    //_TRACE("Created font " << fontKey);

    if(font->Error())
    {
        ERR_POST(Error << "Font file: " << fullname << 
            " failed to open. Default font path: " << m_FontPath );
        delete font;
        return NULL;
    }

    if(!font->FaceSize(size, m_DeviceResolution))
    {
        ERR_POST(Error << "Font " << filename << " failed to set size " << size);
        delete font;
        return NULL;
    }
    // If we want to actually use device resolution, have to call this twice (or apply fix)
    // and use a fake device resolution the first time (since it is ignored).  But for now we ignore
    // device resolution because all the non-font rendering ignores it as well and we would want
    // all our rendering to scale together.
    // font->FaceSize(size, m_DeviceResolution);

    font->UseDisplayList(true);

    fonts[fontKey] = font;

    return font;
#else
    ERR_POST(Error << "FTGL is unavailable.");
    return NULL;
#endif
}

END_NCBI_SCOPE
