#ifndef PKG_SEQUENCE___CLEANUP_PARAMS__HPP
#define PKG_SEQUENCE___CLEANUP_PARAMS__HPP

/*  $Id: cleanup_params.hpp 37331 2016-12-23 19:52:17Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

/*!
 * CCleanupParams class declaration
 */

class CCleanupParams: public IRegSettings
{
    friend class CCleanupPanel;
public:
    /// Default constructor for CCleanupParams
    CCleanupParams();

    /// Copy constructor for CCleanupParams
    CCleanupParams(const CCleanupParams& data);

    /// Destructor for CCleanupParams
    ~CCleanupParams();

    /// Assignment operator for CCleanupParams
    void operator=(const CCleanupParams& data);

    /// Equality operator for CCleanupParams
    bool operator==(const CCleanupParams& data) const;

    /// Copy function for CCleanupParams
    void Copy(const CCleanupParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void                SetRegistryPath(const string& path);
    virtual void                SaveSettings() const;
    virtual void                LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

    ////@begin CCleanupParams member function declarations
    int GetCleanupMode() const { return m_CleanupMode ; }
    void SetCleanupMode(int value) { m_CleanupMode = value ; }

////@end CCleanupParams member function declarations

////@begin CCleanupParams member variables
private:
    int m_CleanupMode;
////@end CCleanupParams member variables

    string              m_RegPath;
    TConstScopedObjects m_Objects;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_SEQUENCE___CLEANUP_PARAMS__HPP
