/*  $Id: fileartprov.cpp 25482 2012-03-27 15:00:20Z kuznets $
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

#include <ncbi_pch.hpp>

// ---------------------------------------------------------------------------
// headers
// ---------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#include <gui/widgets/wx/fileartprov.hpp>
#include "wx/hashset.h"
#include "wx/filename.h"

#ifndef WX_PRECOMP
    #include "wx/hashmap.h"
    #include "wx/image.h"
#endif


// ===========================================================================
// implementation
// ===========================================================================

WX_DECLARE_STRING_HASH_MAP( wxString, wxFileArtNameHash );
WX_DECLARE_STRING_HASH_MAP( wxImage, wxFileArtImageHash );

class wxFileArtNameAndImageCache
{
public:
    static wxString ConstructHashID(
        const wxArtID& anId, const wxArtClient& aClient = wxART_OTHER, const wxSize& aSize = wxDefaultSize
    );

    wxImage GetImage( const wxString& full_id );
    void PutImage( const wxString& full_id, const wxImage& anImage )
    {
        m_ImageHash[full_id] = anImage;
    }

    wxString GetName( const wxString& full_id );
    void PutName( const wxString& full_id, const wxString& aName )
    {
        m_NameHash[full_id] = aName;
    }
    void Remove( const wxString& full_id )
    {
        m_NameHash.erase( full_id );
    }

    void Clear()
    {
        m_NameHash.clear();
    }

private:
    wxFileArtNameHash m_NameHash;
    wxFileArtImageHash m_ImageHash;
};

wxImage wxFileArtNameAndImageCache::GetImage( const wxString& aFullId )
{
    wxFileArtImageHash::iterator entry = m_ImageHash.find( aFullId );

    return entry != m_ImageHash.end() ? entry->second : wxImage();
}

wxString wxFileArtNameAndImageCache::GetName( const wxString& aFullId )
{
    wxFileArtNameHash::iterator entry = m_NameHash.find( aFullId );

    return entry != m_NameHash.end() ? entry->second : (wxString)wxEmptyString;
}


/**
    Creates a string key from three parameters;
    follows internal code for wxArtProviderCache::ConstructHashID()
*/
wxString wxFileArtNameAndImageCache::ConstructHashID(
    const wxArtID& anId, const wxArtClient& aClient, const wxSize& aSize
){
    wxString str;
    str.Printf( wxT("%s-%s-%i-%i"), anId.c_str(), aClient.c_str(), aSize.x, aSize.y );
    return str;
}

WX_DECLARE_HASH_SET( wxString, wxStringHash, wxStringEqual, typedef_wxFileArtDirNameSet );

class wxFileArtDirNameSet : public typedef_wxFileArtDirNameSet {};

IMPLEMENT_DYNAMIC_CLASS(wxFileArtProvider, wxArtProvider)

wxFileArtProvider::wxFileArtProvider()
{
    m_nameCache = new wxFileArtNameAndImageCache();
    m_DirNameSet = new wxFileArtDirNameSet();

    wxInitAllImageHandlers();
}


wxFileArtProvider::~wxFileArtProvider()
{
    delete m_DirNameSet;
    delete m_nameCache;
}

int wxFileArtProvider::AddDirectory( wxString aDirName )
{
    wxDir dir( aDirName );
    if( !dir.IsOpened() ){
        return -1;
    }

    m_DirNameSet->insert( aDirName );

    return 0;
}

void wxFileArtProvider::RegisterFileAlias(
    const wxArtID& anId, const wxArtClient& aClient, const wxSize& aSize,
    const wxString& aName, long WXUNUSED(aType), int WXUNUSED(anIndex)
){
    // TODO Add type and index support for images

    wxString hash_id = wxFileArtNameAndImageCache::ConstructHashID( anId, aClient, aSize );

    m_nameCache->PutName( hash_id, aName );
}

void wxFileArtProvider::RemoveAlias(
    const wxArtID& anId, const wxArtClient& aClient, const wxSize& aSize
){
    wxString hash_id = wxFileArtNameAndImageCache::ConstructHashID( anId, aClient, aSize );

    m_nameCache->Remove( hash_id );
}

wxBitmap wxFileArtProvider::CreateBitmap(
    const wxArtID& anId, const wxArtClient& aClient, const wxSize& aSize
){
    wxString hash_id = wxFileArtNameAndImageCache::ConstructHashID( anId, aClient, aSize );
    wxString name = m_nameCache->GetName( hash_id );

    if( name.IsEmpty() ){
        hash_id = wxFileArtNameAndImageCache::ConstructHashID( anId, aClient );
        name = m_nameCache->GetName( hash_id );
    }

    if( name.IsEmpty() ){
        hash_id = wxFileArtNameAndImageCache::ConstructHashID( anId );
        name = m_nameCache->GetName( hash_id );
    }

    if( !name.IsEmpty() ){

        // Check if filename comes with a full directory path 
        //   or we need to seek in the art catalog
        wxFileName wxfname(name);
        if (!wxfname.DirExists()) {

            for( wxFileArtDirNameSet::iterator it = m_DirNameSet->begin(); it != m_DirNameSet->end(); ++it ){
                wxString dir_name = *it;

                wxDir dir( dir_name );
                if( !dir.IsOpened() ){
                    continue;
                }

                if( dir.HasFiles( name ) )  {
                    wxString sep = wxFileName::GetPathSeparators();
                    name = dir.GetName() + sep[0] + name;
                    break;
                }
            }
        }

        wxImage image( name );
        if( image.Ok() ){

            #if( !defined(__WXMSW__) || wxUSE_WXDIB )

            if( aSize != wxDefaultSize
                &&( image.GetWidth() != aSize.x || image.GetHeight() != aSize.y )
            ){
                image.Rescale( aSize.x, aSize.y );
            }
            #endif

            return wxBitmap( image );
        }
    }

    return wxNullBitmap;
}
