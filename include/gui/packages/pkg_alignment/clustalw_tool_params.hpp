#ifndef PKG_ALIGNMENT___CLUSTALW_TOOL_PARAMS__HPP
#define PKG_ALIGNMENT___CLUSTALW_TOOL_PARAMS__HPP

/*  $Id: clustalw_tool_params.hpp 43795 2019-08-30 19:22:56Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

/*!
 * CClustalwToolParams class declaration
 */

class CClustalwToolParams: public IRegSettings
{    
    friend class CClustalwToolPanel;
public:
    CClustalwToolParams();

    CClustalwToolParams(const CClustalwToolParams& data);

    ~CClustalwToolParams();

    void operator=(const CClustalwToolParams& data);

    bool operator==(const CClustalwToolParams& data) const;

    void Copy(const CClustalwToolParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

////@begin CClustalwToolParams member function declarations
    wxString GetClustalWPath() const { return m_ClustalWPath ; }
    void SetClustalWPath(wxString value) { m_ClustalWPath = value ; }

    wxString GetCommandLine() const { return m_CommandLine ; }
    void SetCommandLine(wxString value) { m_CommandLine = value ; }

    bool GetGenerateTree() const { return m_GenerateTree ; }
    void SetGenerateTree(bool value) { m_GenerateTree = value ; }

////@end CClustalwToolParams member function declarations

    wxString GetClustalWExecutable() const;

////@begin CClustalwToolParams member variables
private:
    wxString m_ClustalWPath;
    wxString m_CommandLine;
    bool m_GenerateTree;
////@end CClustalwToolParams member variables

    string m_RegPath;
    TConstScopedObjects m_Objects;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___CLUSTALW_TOOL_PARAMS__HPP
