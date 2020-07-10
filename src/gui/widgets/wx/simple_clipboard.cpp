/*  $Id: simple_clipboard.cpp 25483 2012-03-27 15:02:30Z kuznets $
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
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/simple_clipboard.hpp>
#include <corelib/ncbimtx.hpp>

BEGIN_NCBI_SCOPE

auto_ptr<CSimpleClipboard> CSimpleClipboard::m_Clipboard;

// singleton object access
CSimpleClipboard& CSimpleClipboard :: Instance()
{
    // as far as Simple Clipboard does not contain any data
    // it is safe even to replace it accidentally, so we
    // don't need any protective mutex

    if( m_Clipboard.get() == NULL ){
        m_Clipboard.reset( new CSimpleClipboard() );
    }
    return *m_Clipboard;
}

void CSimpleClipboard::Clear()
{
    wxClipboardLocker guard;
    if( !guard ) return;

    wxTheClipboard->Clear();
}

wxString CSimpleClipboard::MakeWxString() const
{
    try {
        return Get<wxString>();

    } catch( CException& ){
        return wxT("");
    }
}

END_NCBI_SCOPE
