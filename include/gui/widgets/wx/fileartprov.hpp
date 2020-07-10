#ifndef _WX_FILEARTPROV_H_
#define _WX_FILEARTPROV_H_

/*  $Id: fileartprov.hpp 17877 2008-09-24 13:03:09Z dicuccio $
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
 * Authors:  Yury Voronov
 *
 * File Description: File is supposed to be submitted to wxWidgets main library,
 * that is why it is written with use of wxWidgets modules only, and in wxWidgets
 * naming and coding conventions.
 *
 */
#include <gui/gui.hpp>

#include <wx/artprov.h>
#include <wx/dir.h>

// ----------------------------------------------------------------------------
// custom art provider
// ----------------------------------------------------------------------------

class wxFileArtNameAndImageCache;
class wxFileArtDirNameSet;

class NCBI_GUIWIDGETS_WX_EXPORT wxFileArtProvider : public wxArtProvider
{
public:
    wxFileArtProvider();
    virtual ~wxFileArtProvider();

    void RegisterImageAlias(const wxArtID& anId,
                            const wxArtClient& aClient,
                            const wxSize& aSize,
                            const wxImage& anImage);

    int AddDirectory(wxString aDirName);

    virtual void RegisterFileAlias(const wxArtID& anId,
                                   const wxArtClient& aClient,
                                   const wxSize& aSize,
                                   const wxString& aName,
                                   long aType = wxBITMAP_TYPE_ANY,
                                   int anIndex = -1);

    void RegisterFileAlias(const wxArtID& anId,
                           const wxArtClient& aClient,
                           const wxString& aName,
                           long aType = wxBITMAP_TYPE_ANY,
                           int anIndex = -1)
    {
        RegisterFileAlias( anId, aClient, wxDefaultSize, aName, aType, anIndex );
    }
    void RegisterFileAlias(const wxArtID& anId,
                           const wxString& aName,
                           long aType = wxBITMAP_TYPE_ANY,
                           int anIndex = -1)
    {
        RegisterFileAlias( anId, wxART_OTHER, aName, aType, anIndex );
    }

    void RemoveAlias(const wxArtID& anId,
                     const wxArtClient& aClient = wxART_OTHER,
                     const wxSize& aSize = wxDefaultSize);

protected:
    virtual wxBitmap CreateBitmap(
        const wxArtID& id, const wxArtClient& client, const wxSize& size
    );

private:
    wxFileArtNameAndImageCache* m_nameCache;
    wxFileArtDirNameSet* m_DirNameSet;

    DECLARE_DYNAMIC_CLASS(wxFileArtProvider)

};

#endif // _WX_FILEARTPROV_H_
