#ifndef GUI_CORE___ASN_EXPORT_PARAMS__HPP
#define GUI_CORE___ASN_EXPORT_PARAMS__HPP

/*  $Id: asn_export_params.hpp 39341 2017-09-13 19:09:28Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/*!
 * CAsnExportParams class declaration
 */

class CAsnExportParams: public IRegSettings
{
    friend class CAsnExportPage;
public:
    CAsnExportParams();

    CAsnExportParams(const CAsnExportParams& data);

    ~CAsnExportParams();

    void operator=(const CAsnExportParams& data);

    bool operator==(const CAsnExportParams& data) const;

    void Copy(const CAsnExportParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

////@begin CAsnExportParams member function declarations
    int GetAsnType() const { return m_AsnType ; }
    void SetAsnType(int value) { m_AsnType = value ; }

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

////@end CAsnExportParams member function declarations

    void UpdateFileName(const wxString& fname);

////@begin CAsnExportParams member variables
private:
    int m_AsnType;
    wxString m_FileName;
////@end CAsnExportParams member variables

private:
    string m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif // GUI_CORE___ASN_EXPORT_PARAMS__HPP
