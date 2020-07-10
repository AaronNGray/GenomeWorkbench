#ifndef GUI_WIDGETS_WX___STRING_HISTORY__HPP
#define GUI_WIDGETS_WX___STRING_HISTORY__HPP

/*  $Id: string_history.hpp 28681 2013-08-15 15:42:34Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <gui/gui.hpp>

#include <wx/string.h>

class wxTextCompleter;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CStringHistory: public list<wxString>
{
public:
    CStringHistory(size_t maxSize, const char* regPath);

    void  Load();
    void  Save();

    wxTextCompleter* CreateCompleter();
    void AddString(const wxString& str);

protected:
    bool    m_Initialized;
    size_t  m_MaxSize;
    string  m_RegPath;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___STRING_HISTORY__HPP
