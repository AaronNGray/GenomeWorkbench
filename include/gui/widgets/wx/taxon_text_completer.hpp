#ifndef GUI_CORE___TAXON_TEXT_COMPLETER_HPP
#define GUI_CORE___TAXON_TEXT_COMPLETER_HPP

/*  $Id: taxon_text_completer.hpp 25622 2012-04-13 16:45:13Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistr.hpp>

#include <set>

#include <gui/gui_export.h>

#include <wx/dlimpexp.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/textcompleter.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CTaxonTextCompleter : public wxTextCompleter
{
public:
    typedef set<string, PNocase> TContainer;

    CTaxonTextCompleter();
    virtual ~CTaxonTextCompleter();

    // The virtual functions to be implemented by the derived classes: the
    // first one is called to start preparing for completions for the given
    // prefix and, if it returns true, GetNext() is called until it returns an
    // empty string indicating that there are no more completions.
    virtual bool Start(const wxString& prefix);
    virtual wxString GetNext();

private:
    static bool x_Init();

    string m_Prefix;
    TContainer::const_iterator m_CurPos;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___TAXON_TEXT_COMPLETER_HPP
