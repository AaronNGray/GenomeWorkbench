#ifndef GUI_OPENGL___FTGLFONTMANAGER__HPP
#define GUI_OPENGL___FTGLFONTMANAGER__HPP

/*  $Id: ftglfontmanager.hpp 42352 2019-02-06 21:15:07Z katargir $
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
 * This class is a singleton that manages a set of currently loaded fonts. 
 * When a font is needed, it checks to see if it is already loaded and,
 * if not, it is read from the corresponding file and added to the list 
 * for the required font size. This allows all objects to share a single set
 * of loaded fonts which improves effciency since reading in font files and
 * creating texture maps from them is much more time-consuming than drawing
 * a string from those textures.
 * 
 * To get a font give the name of the font file, the font size, and the 
 * the monitor resolution.  The monitor resolution can be retrieved using
 * the wxWidgets function wxGetDisplayPPI().
 */

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <map>
#include <string>

using namespace std;

class FTFont;

BEGIN_NCBI_SCOPE

class NCBI_GUIOPENGL_EXPORT CFtglFontManager
{
public:
    typedef map<string, FTFont*> FontList;
    typedef FontList::const_iterator FontIter;

    /// FTGL can create different types of fonts from files read in
    /// using freetype, but the texture fonts are the best quality
    enum EFtglFontType { eTextureFont, ePixmapFont, eBitmapFont,
            eOutlineFont, ePolygonFont, eExtrudedFont };

public:
    /// Get an instance of the manager so you can retrieve fonts
    static CFtglFontManager& Instance();
    ~CFtglFontManager();

    /// Delete all current fonts
    void Clear();

    /// This path will be prepended to filename when calling GetFont
    /// It should be set to "ToStdString(CSysPath::GetResourcePath())"
    /// which depends on wxWidgets and so can't be called here.
    void SetFontPath(const string& p);
    string GetFontpath() const { return m_FontPath; }

    /// Get/set device resolution in pixels per inch (defaults to 72)
    /// You can get this value from wxWidgets via: wxGetDisplayPPI();
    void SetDeviceResolution(unsigned int res) { m_DeviceResolution = res; }
    unsigned int GetDeviceResolution() const { return m_DeviceResolution; }

    // Return a font a the specified size - load from a file if needed
    // or retrieve from the array if it has already been loaded.
    FTFont* GetFont(const char *filename, 
                    unsigned int size, 
                    EFtglFontType ft = eTextureFont);

private:
    /// Hide ctors to make object singleton
    CFtglFontManager() : m_DeviceResolution(72) {}

    /// private and unimplemented
    CFtglFontManager(const CFtglFontManager&);
    CFtglFontManager& operator=(const CFtglFontManager&);

    /// Loaded to memeory font files
    map<string, tuple<unique_ptr<unsigned char[]>, size_t> > m_BinaryData;
    /// All the currently loaded fonts
    FontList fonts;

    /// target device (screen) resolution in pixels per inch (defaults to 72)
    unsigned int m_DeviceResolution;

    /// Directory from which to read font files.
    string m_FontPath;
};

END_NCBI_SCOPE

#endif //GUI_OPENGL___FTGLFONTMANAGER__HPP

