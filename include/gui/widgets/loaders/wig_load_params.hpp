#ifndef GUI_WIDGETS___LOADERS___WIG_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___WIG_LOAD_PARAMS__HPP

/*  $Id: wig_load_params.hpp 28586 2013-08-02 19:32:29Z katargir $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

#include <gui/widgets/loaders/map_assembly_params.hpp>

BEGIN_NCBI_SCOPE
/*!
 * CWigLoadParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CWigLoadParams: public IRegSettings
{    
    friend class CWigParamsPanel;
public:
    /// Default constructor for CWigLoadParams
    CWigLoadParams();

    /// Copy constructor for CWigLoadParams
    CWigLoadParams(const CWigLoadParams& data);

    /// Destructor for CWigLoadParams
    ~CWigLoadParams();

    /// Assignment operator for CWigLoadParams
    void operator=(const CWigLoadParams& data);

    /// Equality operator for CWigLoadParams
    bool operator==(const CWigLoadParams& data) const;

    /// Copy function for CWigLoadParams
    void Copy(const CWigLoadParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin CWigLoadParams member function declarations
    int GetNumErrors() const { return m_NumErrors ; }
    void SetNumErrors(int value) { m_NumErrors = value ; }

    CMapAssemblyParams GetMapAssembly() const { return m_MapAssembly ; }
    void SetMapAssembly(CMapAssemblyParams value) { m_MapAssembly = value ; }

////@end CWigLoadParams member function declarations

////@begin CWigLoadParams member variables
private:
    int m_NumErrors;
    CMapAssemblyParams m_MapAssembly;
////@end CWigLoadParams member variables

private:
    string m_RegPath;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___WIG_LOAD_PARAMS__HPP
