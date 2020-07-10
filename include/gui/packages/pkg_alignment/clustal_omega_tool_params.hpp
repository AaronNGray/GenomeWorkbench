#ifndef PKG_ALIGNMENT____CLUSTAL_OMEGA_TOOL_PARAMS__HPP
#define PKG_ALIGNMENT____CLUSTAL_OMEGA_TOOL_PARAMS__HPP
/*  $Id: clustal_omega_tool_params.hpp 43796 2019-08-30 19:31:56Z katargir $
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
 * Authors:  Vladislav Evgeniev
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
 * CClustalOmegaToolParams class declaration
 */

class CClustalOmegaToolParams : public IRegSettings
{    
    friend class CClustalOmegaToolPanel;
public:
    CClustalOmegaToolParams();

    CClustalOmegaToolParams(const CClustalOmegaToolParams& data);

    ~CClustalOmegaToolParams();

    void operator=(const CClustalOmegaToolParams& data);

    bool operator==(const CClustalOmegaToolParams& data) const;

    void Copy(const CClustalOmegaToolParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

////@begin CClustalOmegaToolParams member function declarations
    wxString GetClustalOmegaPath() const { return m_ClustalOmegaPath ; }
    void SetClustalOmegaPath(wxString value) { m_ClustalOmegaPath = value ; }

    wxString GetCommandLine() const { return m_CommandLine ; }
    void SetCommandLine(wxString value) { m_CommandLine = value ; }

    bool GetGenerateTree() const { return m_GenerateTree ; }
    void SetGenerateTree(bool value) { m_GenerateTree = value ; }

////@end CClustalOmegaToolParams member function declarations

    wxString GetClustalOmegaExecutable() const;

////@begin CClustalOmegaToolParams member variables
private:
    wxString m_ClustalOmegaPath;
    wxString m_CommandLine;
    bool m_GenerateTree;
////@end CClustalOmegaToolParams member variables

    string              m_RegPath;
    TConstScopedObjects m_Objects;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT____CLUSTAL_OMEGA_TOOL_PARAMS__HPP
