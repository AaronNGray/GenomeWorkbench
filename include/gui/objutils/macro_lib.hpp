#ifndef GUI_OBJUTILS___MACRO_LIB__HPP
#define GUI_OBJUTILS___MACRO_LIB__HPP
/*  $Id: macro_lib.hpp 40862 2018-04-23 18:51:38Z evgeniev $
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
 * Authors: Anatoly Osipov
 *
 * File Description: Macro library for storing parsed macros
 *
 */

/// @file macro_lib.hpp
/// Macro library for storing parsed macros
#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/macro_rep.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)


///////////////////////////////////////////////////////////////////////////////
/// CMacroLib - singleton
///
/// stores a set of parsed macros
class NCBI_GUIOBJUTILS_EXPORT CMacroLib
{
public:
    typedef map<string, CRef<CMacroRep> > TMacroNameMap;
    typedef vector< CRef<CMacroRep> >   TMacroLibrary;
    typedef map<string, list<string> >  TStrSynList;
private:
    /// Constructors are private to make the class a singleton.
    CMacroLib() {}
    CMacroLib(const CMacroLib&) {}
    CMacroLib& operator=(const CMacroLib&) { return *this; }
public:
    static CMacroLib& GetInstance() {
        static CMacroLib instance;
        return instance;
    }
public:
    /// Function parses the file with macros and 
    /// stores the results internally in a map.
    /// @param filename
    ///   Full name (with path) for a file to be parsed. 
    /// @return
    ///   true if the file is read and parsed successfully;
    ///   false otherwise.
    bool AppendToLibrary(const string& filename);

    bool AppendToLibrary(const vector<string>& filenames);

    /// Function builds a list of macro names in the library 
    /// to use for example in the toolbar.
    void GetMacroNames (vector<string>& macro_names) const;

    /// Returns all macros in the library
    const TMacroLibrary& GetMacros() const { return m_MacroLibrary; }
    
    /// Function gets the macro representation to 
    /// let the gui resolve "ask" and "choice" variables.
    CMacroRep* GetMacroRep(const string& macro_name) const;

    /// Clears the internal map
    void ClearLibrary() { m_MacroLibrary.clear(); m_MacroByName.clear(); }

    /// Function returns the synonym list of a given phrase
    const list<string> GetSynonymListForPhrase (const string& phrase) const;

    /// Function builds a map of words and their corresponding synonyms
    bool StoreSynonymList(const string& filename);

    /// Accessors for the file that stores the suspect product rules
    void SetSuspectRules(const string& filename) { m_SuspectProductRules = filename; }
    const string& GetSuspectRules() const { return m_SuspectProductRules; }

    /// Returns the last error message
    /// @return
    ///   String, holding the last error
    const string& GetLastError(void) const { return m_LastError; }
    
private:
    void x_InitializeMacrosNameMap();

private:
    TMacroLibrary m_MacroLibrary;
    TMacroNameMap m_MacroByName;
    TStrSynList   m_MacroSynonymMap;
    string m_SuspectProductRules;
    /// String, holding the last error
    string m_LastError;
};


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_LIB__HPP
