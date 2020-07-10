/*  $Id: macro_lib.cpp 40862 2018-04-23 18:51:38Z evgeniev $
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
 * Authors:  Anatoly Osipov
 *
 * File Description: Macro library for storing parsed macros
 *
 */

#include <ncbi_pch.hpp>
#include <util/line_reader.hpp>
#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/macro_engine.hpp>

#undef _TRACE
#define _TRACE(arg) ((void)0)

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

bool CMacroLib::AppendToLibrary(const string& filename)
{
    m_LastError.resize(0);
    CMacroEngine engine;
    if (!engine.AppendToLibrary(filename, m_MacroLibrary)) {
        m_LastError += engine.GetErrorMessage();
        return false;
    }
    x_InitializeMacrosNameMap();
    return true;
}

bool CMacroLib::AppendToLibrary(const vector<string>& filenames)
{
    m_LastError.resize(0);
    CMacroEngine engine;
    for (auto&& it : filenames) {
        if (!engine.AppendToLibrary(it, m_MacroLibrary)) {
            LOG_POST(Error << "Error: " + it + ": " + engine.GetErrorMessage());
            if (m_LastError.empty()) {
                m_LastError = "Parsing error in: \n";
            }
            m_LastError += it + "\n";
        }
    }
    x_InitializeMacrosNameMap();
    return (m_LastError.empty()) ? true : false;
}

void CMacroLib::GetMacroNames(vector<string>& macro_names) const
{
    macro_names.clear();
    macro_names.reserve(m_MacroLibrary.size());
    for (auto& it : m_MacroLibrary) {
        macro_names.push_back(it->GetName());
    }
}

CMacroRep* CMacroLib::GetMacroRep(const string& macro_name) const
{
    auto it = m_MacroByName.find(macro_name);
    return (it == m_MacroByName.end()) ? 0 : (*it).second.GetNCPointerOrNull();
}

const list<string> CMacroLib::GetSynonymListForPhrase(const string& phrase) const
{
    auto it = m_MacroSynonymMap.find(phrase);
    if (it != m_MacroSynonymMap.end()) {
        return it->second;
    }
    return list<string>();
}

bool CMacroLib::StoreSynonymList(const string& filename)
{
    m_LastError.resize(0);
    m_MacroSynonymMap.clear();

    string line, phrase;
    try {
        CRef<ILineReader> line_reader(ILineReader::New(filename));
        bool syn_line = false;
        list<string> syn_list;

        while (!line_reader->AtEOF()) {
            line_reader->ReadLine();
            line = line_reader->GetCurrentLine();

            if (!syn_line && NStr::StartsWith(line, "word: ")) {
                phrase = line.substr(6, NPOS);
                NStr::TruncateSpacesInPlace(phrase);
                syn_line = true;
            }
            else if (syn_line) {
                NStr::TruncateSpacesInPlace(line);
                if (NStr::IsBlank(line)) {
                    m_MacroSynonymMap.insert(CMacroLib::TStrSynList::value_type(phrase, syn_list));
                    syn_list.clear();
                    syn_line = false;
                }
                else {
                    syn_list.push_back(line);
                }
            }
        }
    }
    catch (const CException& ex) {
        ERR_POST(ex.ReportAll());
        m_LastError = "Problem with reading file:" + filename;
        return false;
    }

    return true;
}

void CMacroLib::x_InitializeMacrosNameMap()
{
    m_MacroByName.clear();
    if (m_MacroLibrary.empty())
        return;
    for (auto&& it : m_MacroLibrary) {
        m_MacroByName.insert(TMacroNameMap::value_type(it->GetName(), it));
    }
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
