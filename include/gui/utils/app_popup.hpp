#ifndef GUI_UTILS__APP_POPUP_HPP
#define GUI_UTILS__APP_POPUP_HPP

/*  $Id: app_popup.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 *    CAppPopup -- launch a url in a web browser
 *    widen() -- convert a string to a wstring
 */


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>


/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


///
/// class CAppInfo defines an interface for application registration information
///
class NCBI_GUIUTILS_EXPORT CAppInfo : public CObject
{
public:
    CAppInfo(const string& path = kEmptyStr);
    virtual ~CAppInfo();
    const string& GetExePath(void) const;

private:
    string m_Exepath;
};


///
/// enumerated file type information
///
BEGIN_SCOPE(filetype)

enum EFileType
{
    eUnknown = 0,
    ePdf
};
typedef enum EFileType TFileType;

CNcbiOstream& operator<<(CNcbiOstream& strm, TFileType& ftype);

END_SCOPE(filetype)


///
/// class CAppRegistry defines the registry of application information for
/// automated application link-outs
///
class NCBI_GUIUTILS_EXPORT CAppRegistry
{
public:
    typedef CRef<CAppInfo> TAppInfoRef;
    typedef filetype::TFileType TFileType;

    CAppRegistry();
    virtual ~CAppRegistry();

    const TAppInfoRef& Find(TFileType filetype) const;
    TAppInfoRef& operator[](const TFileType& type);

private:
    typedef map<TFileType, TAppInfoRef> TRegistry;
    TRegistry m_AppReg;
};


///
/// class CAppPopup allows applications to be launched based on standard
/// registered types
///
class NCBI_GUIUTILS_EXPORT CAppPopup
{
public:
    typedef filetype::TFileType TFileType;

    /// launch an application to handle a file
    static bool PopupFile(const string& file,
                          TFileType filetype = filetype::eUnknown);

    /// launch an application to handle a URL
    static bool PopupURL(const string& url);

    /// retrieve the application registry
    static CAppRegistry& GetRegistry(void);

private:
    static CAppRegistry m_Registry;
};


END_NCBI_SCOPE

/* @} */

#endif /// GUI_UTILS__APP_POPUP_HPP
