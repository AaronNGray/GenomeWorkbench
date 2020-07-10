/*  $Id: clipboard.cpp 23824 2011-06-09 16:06:43Z kuznets $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/utils/clipboard.hpp>
#include <corelib/ncbimtx.hpp>

BEGIN_NCBI_SCOPE


DEFINE_CLASS_STATIC_MUTEX(CClipboard::sm_ClipboardMutex);

auto_ptr<CClipboard> CClipboard::m_Clipboard;

// singleton object access
CClipboard & CClipboard :: Instance()
{
    CMutexGuard LOCK(sm_ClipboardMutex);
    if (m_Clipboard.get() == NULL) {
        m_Clipboard.reset(new CClipboard());
    }
    return *m_Clipboard;
}


// clearing clipboard
void CClipboard::Clear(void)
{
    CMutexGuard LOCK(sm_ClipboardMutex);
    x_Clear();
}


// emptyness check
bool CClipboard::IsEmpty(void)
{
    CMutexGuard LOCK(sm_ClipboardMutex);
    return m_Contents.empty();
}

// clear clipboard contents
void CClipboard::x_Clear(void)
{
    ITERATE(TContents, item, m_Contents) {
        ITERATE(TItem, flavor, *item) {
            delete *flavor;
        }
    }
    m_Contents.clear();
}

// size
size_t CClipboard::Size(void)
{
    return m_Contents.size();
}

// to-string conversion
string CClipboard::ToString()
{
    string strTxt = "";
    for (TContents::size_type i = 0; i  < m_Contents.size(); i++) {
        try {
            const string* val = x_GetValueAt<string>((int)i);
            if (val) {
                if ( !strTxt.empty() ) {
                    strTxt += '\n';
                }
                strTxt += *val;
            }
        }
        catch (std::exception&){
        }
    }
    return strTxt;
}

END_NCBI_SCOPE
