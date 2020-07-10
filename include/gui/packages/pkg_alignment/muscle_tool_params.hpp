#ifndef PKG_ALIGNMENT___MUSCLE_TOOL_PARAMS__HPP
#define PKG_ALIGNMENT___MUSCLE_TOOL_PARAMS__HPP

/*  $Id: muscle_tool_params.hpp 43792 2019-08-30 18:31:36Z katargir $
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
 * CMuscleToolParams class declaration
 */

class CMuscleToolParams: public IRegSettings
{    
    friend class CMuscleToolPanel;
public:
    CMuscleToolParams();

    CMuscleToolParams(const CMuscleToolParams& data);

    ~CMuscleToolParams();

    void operator=(const CMuscleToolParams& data);

    bool operator==(const CMuscleToolParams& data) const;

    void Copy(const CMuscleToolParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

////@begin CMuscleToolParams member function declarations
    wxString GetCommandLine() const { return m_CommandLine ; }
    void SetCommandLine(wxString value) { m_CommandLine = value ; }

    bool GetGenerateTree() const { return m_GenerateTree ; }
    void SetGenerateTree(bool value) { m_GenerateTree = value ; }

    wxString GetMusclePath() const { return m_MusclePath ; }
    void SetMusclePath(wxString value) { m_MusclePath = value ; }

    int GetScoringMethod() const { return m_ScoringMethod ; }
    void SetScoringMethod(int value) { m_ScoringMethod = value ; }

////@end CMuscleToolParams member function declarations

    wxString GetMuscleExecutable() const;

////@begin CMuscleToolParams member variables
private:
    wxString m_CommandLine;
    bool m_GenerateTree;
    wxString m_MusclePath;
    int m_ScoringMethod;
////@end CMuscleToolParams member variables

    string m_RegPath;
    TConstScopedObjects m_Objects;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___MUSCLE_TOOL_PARAMS__HPP
