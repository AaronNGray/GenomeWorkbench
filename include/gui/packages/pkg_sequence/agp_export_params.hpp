#ifndef PKG_SEQUENCE___AGP_EXPORT_PARAMS__HPP
#define PKG_SEQUENCE___AGP_EXPORT_PARAMS__HPP

/*  $Id: agp_export_params.hpp 37296 2016-12-21 20:01:53Z katargir $
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

/*!
 * CAgpExportParams class declaration
 */

BEGIN_NCBI_SCOPE

class CAgpExportParams: public IRegSettings
{
    friend class CAgpExportPage;
public:
    /// Default constructor for CAgpExportParams
    CAgpExportParams();

    /// Copy constructor for CAgpExportParams
    CAgpExportParams(const CAgpExportParams& data);

    /// Destructor for CAgpExportParams
    ~CAgpExportParams();

    /// Assignment operator for CAgpExportParams
    void operator=(const CAgpExportParams& data);

    /// Equality operator for CAgpExportParams
    bool operator==(const CAgpExportParams& data) const;

    /// Copy function for CAgpExportParams
    void Copy(const CAgpExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    SConstScopedObject GetSeqLoc() const { return m_SeqLoc; }

////@begin CAgpExportParams member function declarations
    string GetGapType() const { return m_GapType ; }
    void SetGapType(string value) { m_GapType = value ; }

    wxString GetAltObjId() const { return m_AltObjId ; }
    void SetAltObjId(wxString value) { m_AltObjId = value ; }

    bool GetAssertEvidence() const { return m_AssertEvidence ; }
    void SetAssertEvidence(bool value) { m_AssertEvidence = value ; }

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

////@end CAgpExportParams member function declarations

////@begin CAgpExportParams member variables
private:
    string m_GapType;
    wxString m_AltObjId;
    bool m_AssertEvidence;
    wxString m_FileName;
////@end CAgpExportParams member variables

private:
    string m_RegPath;
    SConstScopedObject m_SeqLoc;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___AGP_EXPORT_PARAMS__HPP
