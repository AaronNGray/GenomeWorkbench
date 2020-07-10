/*  $Id: pythonpp_emb.hpp 590772 2019-08-05 13:19:43Z ucko $
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
* Author: Sergey Sikorskiy
*
* File Description: Tiny Python API wrappers
*
* Status: *Initial*
*
* ===========================================================================
*/

#ifndef PYTHONPP_EMB_H
#define PYTHONPP_EMB_H

#include "pythonpp_config.hpp"

#include <corelib/ncbistr.hpp>

BEGIN_NCBI_SCOPE

namespace pythonpp
{

class CEngine
{
public:
    CEngine(const char* prog_name = NULL)
    {
        x_SetProgramName(prog_name);
    	// Initialize the Python interpreter.  Required.
    	Py_Initialize();
    }
    CEngine(int argc, char *argv[])
    {
	    // Pass argv[0] to the Python interpreter
        x_SetProgramName(argv[0]);

    	// Initialize the Python interpreter.  Required.
    	Py_Initialize();

	    // Define sys.argv.  It is up to the application if you
	    // want this; you can also let it undefined (since the Python
	    // code is generally not a main program it has no business
	    // touching sys.argv...)
#if PY_MAJOR_VERSION >= 3
        m_Args.resize(argc);
        m_Argv.resize(argc);
        for (int i = 0;  i < argc;  ++i) {
            m_Args[i] = CUtf8::AsBasicString<wchar_t>(argv[i]);
            m_Argv[i] = const_cast<wchar_t*>(m_Args[i].c_str());
        }
        PySys_SetArgv(argc, m_Argv.data());
#else
	    PySys_SetArgv(argc, argv);
#endif
    }
    ~CEngine(void)
    {
    	// Exit, cleaning up the interpreter
        Py_Finalize();
    }

public:
    static void ExecuteStr(const char* cmd)
    {
        if ( PyRun_SimpleString( cmd ) == -1 ) {
            // throw CError ("Unable to execute a string");
            throw string("Couldn't execute string '") + cmd + "'";
        }
    }
    static void ExecuteFile(const char* file_name)
    {
        FILE* file = NULL;
        file = fopen(file_name, "r");
        if ( file != NULL ) {
            // PyRun_AnyFileEx will close the file after execution ...
            if ( PyRun_AnyFileEx(file, file_name, 1) == -1 ) {
                // throw CError ("Unable to execute a file");
            throw string("Couldn't execute file '") + file_name + "'";
            }
        }
    }

private:
    void x_SetProgramName(const char* prog_name)
    {
        if ( prog_name ) {
#if PY_MAJOR_VERSION >= 3
            m_ProgName = CUtf8::AsBasicString<wchar_t>(prog_name);
            Py_SetProgramName(const_cast<wchar_t*>(m_ProgName.c_str()));
#else
            Py_SetProgramName(const_cast<char*>(prog_name));
#endif
        }
    }

#if PY_MAJOR_VERSION >= 3
    wstring         m_ProgName;
    vector<wstring> m_Args;
    vector<wchar_t*> m_Argv;
#endif
};

}                                       // namespace pythonpp

END_NCBI_SCOPE

#endif                                  // PYTHONPP_EMB_H


