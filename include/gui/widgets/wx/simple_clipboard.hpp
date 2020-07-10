#ifndef GUI_WIDGETS_WX___SIMPLE_CLIPBOARD__HPP
#define GUI_WIDGETS_WX___SIMPLE_CLIPBOARD__HPP

/*  $Id: simple_clipboard.hpp 44935 2020-04-21 20:16:21Z asztalos $
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
 * Authors: Yury Voronov
 *
 * File Description: this is a simple implementation of a clipboard,
 * madde primarily for wrapping of wxClipboard. Lately it can be
 * extended to (covered by) advanced Clipboard existed in version 1.0.
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/filename.h>
#include <wx/metafile.h>
#include <wx/dataobj.h>

#include <wx/clipbrd.h>


BEGIN_NCBI_SCOPE

template<typename T> class CwxClipboardHelper
{
    friend class CSimpleClipboard;

protected:
    static bool x_Available();
    static T x_Get();
    static void x_Set( const T& val );
};

class NCBI_GUIWIDGETS_WX_EXPORT CSimpleClipboard
{
public:
    // singleton access
    static CSimpleClipboard& Instance();

    template<typename T> bool IsAvailable() const
    {
        wxClipboardLocker guard;
        if( !guard ) return false;

        return CwxClipboardHelper<T>::x_Available();
    }

    template<typename T> T Get() const
    {
        wxClipboardLocker guard;
        if( !guard ){
            NCBI_THROW(
                CException, eUnknown,
                "CSimpleClipboard::Get() - cannot open"
            );
        }

        if( !CwxClipboardHelper<T>::x_Available() ){
            NCBI_THROW(
                CException, eUnknown,
                "CSimpleClipboard::Get() - cannot get value"
            );
        }

        return CwxClipboardHelper<T>::x_Get();
    }

    template<typename T> void Set( const T& val )
    {
        wxClipboardLocker guard;
        if( !guard ) return;
        
        CwxClipboardHelper<T>::x_Set( val );
    }

    void Clear();

    wxString MakeWxString() const;
    /// !!! name is not good !!!
    string MakeString() const { return ToStdString( MakeWxString() ); }

private:
    // forbidden
    CSimpleClipboard() {}

private:
    static auto_ptr<CSimpleClipboard> m_Clipboard;
};


//template<> bool CSimpleClipboard::IsAvailable<wxString>() const;
//template<> wxString CSimpleClipboard::Get<wxString>() const;

template<> bool CSimpleClipboard::IsAvailable<wxBitmap>() const;
template<> wxBitmap CSimpleClipboard::Get<wxBitmap>() const;

#if wxUSE_METAFILE
template<> bool CSimpleClipboard::IsAvailable<wxMetafile>() const;
template<> wxMetafile CSimpleClipboard::Get<wxMetafile>() const;
#endif

template<> bool CSimpleClipboard::IsAvailable<wxFileName>() const;
template<> wxFileName CSimpleClipboard::Get<wxFileName>() const;

template<> inline bool CSimpleClipboard::IsAvailable<string>() const
{
    return IsAvailable<wxString>();
}

template<> inline string CSimpleClipboard::Get<string>() const
{
    return ToStdString( Get<wxString>() );
}

template<> class CwxClipboardHelper<wxString>
{
    friend class CSimpleClipboard;

protected:
    static bool x_Available()
    {
        return wxTheClipboard->IsSupported( wxDF_UNICODETEXT );
    }

    static wxString x_Get()
    {
        wxTextDataObject data;
        wxTheClipboard->GetData( data );
        return data.GetText();
    }

    static void x_Set( const wxString& val )
    {
        wxTheClipboard->SetData( new wxTextDataObject( val ) );
    }
};

template<> class CwxClipboardHelper<wxBitmap>
{
    friend class CSimpleClipboard;

protected:
    static void x_Set( const wxBitmap& val )
    {
        wxTheClipboard->SetData( new wxBitmapDataObject( val ) );
    }
};


//template<> void CSimpleClipboard::Set<wxString>( const wxString& val );
//template<> void CSimpleClipboard::Set<wxBitmap>( const wxBitmap& val );
//template<> void CSimpleClipboard::Set<wxMetafile>( const wxMetafile& val );
//template<> void CSimpleClipboard::Set<wxFileName>( const wxFileName& val );

template<> inline void CSimpleClipboard::Set<string>( const string& val )
{
    Set<wxString>( ToWxString( val ) );
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___SIMPLE_CLIPBOARD__HPP
