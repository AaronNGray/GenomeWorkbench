#ifndef PKG_ALIGNMENT___MAFFT_TOOL_PARAMS__HPP
#define PKG_ALIGNMENT___MAFFT_TOOL_PARAMS__HPP

/*  $Id: mafft_tool_params.hpp 43793 2019-08-30 19:02:28Z katargir $
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
 * Authors:  Roman Katargin, Vladislav Evgeniev
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
 * CMafftToolParams class declaration
 */

class CMafftToolParams: public IRegSettings
{    
    friend class CMafftToolPanel;
public:
    CMafftToolParams();

    CMafftToolParams(const CMafftToolParams& data);

    ~CMafftToolParams();

    void operator=(const CMafftToolParams& data);

    bool operator==(const CMafftToolParams& data) const;

    void Copy(const CMafftToolParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

////@begin CMafftToolParams member function declarations
    bool GetAutoFlag() const { return m_AutoFlag ; }
    void SetAutoFlag(bool value) { m_AutoFlag = value ; }

    wxString GetCommandLine() const { return m_CommandLine ; }
    void SetCommandLine(wxString value) { m_CommandLine = value ; }

    wxString GetMafftPath() const { return m_MafftPath ; }
    void SetMafftPath(wxString value) { m_MafftPath = value ; }

    bool GetQuietFlag() const { return m_QuietFlag ; }
    void SetQuietFlag(bool value) { m_QuietFlag = value ; }

////@end CMafftToolParams member function declarations

    wxString GetMafftExecutable() const;

////@begin CMafftToolParams member variables
private:
    bool m_AutoFlag;
    wxString m_CommandLine;
    wxString m_MafftPath;
    bool m_QuietFlag;
////@end CMafftToolParams member variables

    string              m_RegPath;
    TConstScopedObjects m_Objects;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___MAFFT_TOOL_PARAMS__HPP
