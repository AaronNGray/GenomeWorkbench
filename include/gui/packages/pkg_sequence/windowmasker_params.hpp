#ifndef PKG_SEQUENCE___WINDOWMASKER_PARAMS__HPP
#define PKG_SEQUENCE___WINDOWMASKER_PARAMS__HPP

/*  $Id: windowmasker_params.hpp 37308 2016-12-22 18:35:43Z katargir $
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
 * Authors:  Vladimir Tereshkov
 */

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/*!
 * CWindowMaskerParams class declaration
 */

class CWindowMaskerParams: public IRegSettings
{
    friend class CWindowMaskerPanel;
public:
    /// Default constructor for CWindowMaskerParams
    CWindowMaskerParams();

    /// Copy constructor for CWindowMaskerParams
    CWindowMaskerParams(const CWindowMaskerParams& data);

    /// Destructor for CWindowMaskerParams
    ~CWindowMaskerParams();

    /// Assignment operator for CWindowMaskerParams
    void operator=(const CWindowMaskerParams& data);

    /// Equality operator for CWindowMaskerParams
    bool operator==(const CWindowMaskerParams& data) const;

    /// Copy function for CWindowMaskerParams
    void Copy(const CWindowMaskerParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void                SetRegistryPath(const string& path);
    virtual void                SaveSettings() const;
    virtual void                LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

    ////@begin CWindowMaskerParams member function declarations
    bool GetStandalone() const { return m_Standalone ; }
    void SetStandalone(bool value) { m_Standalone = value ; }

    int GetOutputType() const { return m_OutputType ; }
    void SetOutputType(int value) { m_OutputType = value ; }

    int GetTaxId() const { return m_TaxId ; }
    void SetTaxId(int value) { m_TaxId = value ; }

////@end CWindowMaskerParams member function declarations

////@begin CWindowMaskerParams member variables
private:
    bool m_Standalone;
    int m_OutputType;
    int m_TaxId;
////@end CWindowMaskerParams member variables

private:
    string              m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif  // PKG_SEQUENCE___WINDOWMASKER_PARAMS__HPP
