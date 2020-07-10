#ifndef GUI_CORE___RNA_NAME_COMPLETER_HPP
#define GUI_CORE___RNA_NAME_COMPLETER_HPP

/*  $Id: rna_name_completer.hpp 29131 2013-10-25 15:12:35Z bollin $
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
 * Authors:  Colleen Bollin
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

class NCBI_GUIWIDGETS_WX_EXPORT CRNANameCompleter : public wxTextCompleter
{
public:
    typedef set<string, PNocase> TContainer;

    CRNANameCompleter ();
    virtual ~CRNANameCompleter ();

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

#endif  // GUI_CORE___RNA_NAME_COMPLETER_HPP
