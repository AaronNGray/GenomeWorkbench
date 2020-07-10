#ifndef GUI_WIDGETS___LOADERS___ASSEMBLY_TERM_COMPLETER_HPP
#define GUI_WIDGETS___LOADERS___ASSEMBLY_TERM_COMPLETER_HPP

/*  $Id: assembly_term_completer.hpp 28562 2013-08-02 15:03:07Z katargir $
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
#include <gui/gui_export.h>

#include <list>

#include <wx/dlimpexp.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/textcompleter.h>

BEGIN_NCBI_SCOPE

class CTaxonTextCompleter;

class NCBI_GUIWIDGETS_LOADERS_EXPORT CAssemblyTermCompleter : public wxTextCompleter
{
public:
    CAssemblyTermCompleter(const list<string>& history);
    ~CAssemblyTermCompleter();

    // The virtual functions to be implemented by the derived classes: the
    // first one is called to start preparing for completions for the given
    // prefix and, if it returns true, GetNext() is called until it returns an
    // empty string indicating that there are no more completions.
    virtual bool Start(const wxString& prefix);
    virtual wxString GetNext();

private:
    string m_Prefix;
    list<string> m_History;
    list<string>::const_iterator m_CurPos;
    CTaxonTextCompleter* m_TaxonTextCompleter;
    bool m_HaveTaxons;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___ASSEMBLY_TERM_COMPLETER_HPP
