#ifndef GUI_WIDGETS_WX___CONT_TEXT_COMPLETER__HPP
#define GUI_WIDGETS_WX___CONT_TEXT_COMPLETER__HPP

/*  $Id: cont_text_completer.hpp 28672 2013-08-14 15:47:36Z katargir $
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

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/textcompleter.h>

BEGIN_NCBI_SCOPE

template <typename Container> class CContTextCompleter: public wxTextCompleter
{
public:
    typedef Container T;

    CContTextCompleter(const T& container) : m_Container(container) {}

    virtual bool Start(const wxString& prefix) {
        m_Prefix = prefix;
        for (m_It = m_Container.begin(); m_It != m_Container.end(); ++m_It) {
            if ((*m_It).StartsWith(m_Prefix))
                break;
        }
        return (m_It != m_Container.end());
    }

    virtual wxString GetNext() {
        if (m_It == m_Container.end())
            return wxString();

        wxString value = *m_It++;
        for (; m_It != m_Container.end(); ++m_It) {
            if ((*m_It).StartsWith(m_Prefix))
                break;
        }
        return value;
    }

protected:
    wxString m_Prefix;
    const T& m_Container;
    typename T::const_iterator m_It;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___CONT_TEXT_COMPLETER__HPP
