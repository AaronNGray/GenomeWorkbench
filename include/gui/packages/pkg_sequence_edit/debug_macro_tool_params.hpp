#ifndef PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_PARAMS__HPP
#define PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_PARAMS__HPP

/*  $Id: debug_macro_tool_params.hpp 37345 2016-12-27 18:24:36Z katargir $
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

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/*!
 * CDebugMacroToolParams class declaration
 */

class CDebugMacroToolParams: public IRegSettings
{    
    friend class CDebugMacroToolPanel;
public:
    /// Default constructor for CDebugMacroToolParams
    CDebugMacroToolParams();

    /// Copy constructor for CDebugMacroToolParams
    CDebugMacroToolParams(const CDebugMacroToolParams& data);

    /// Destructor for CDebugMacroToolParams
    ~CDebugMacroToolParams();

    /// Assignment operator for CDebugMacroToolParams
    void operator=(const CDebugMacroToolParams& data);

    /// Equality operator for CDebugMacroToolParams
    bool operator==(const CDebugMacroToolParams& data) const;

    /// Copy function for CDebugMacroToolParams
    void Copy(const CDebugMacroToolParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }


////@begin CDebugMacroToolParams member function declarations
    wxString GetText() const { return m_Text ; }
    void SetText(wxString value) { m_Text = value ; }

////@end CDebugMacroToolParams member function declarations

////@begin CDebugMacroToolParams member variables
private:
    wxString m_Text;
////@end CDebugMacroToolParams member variables

private:
    string m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE_EDIT___DEBUG_MACRO_TOOL_PARAMS__HPP
