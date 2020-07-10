#ifndef PKG_SEQUENCE___GFF_EXPORT_PARAMS__HPP
#define PKG_SEQUENCE___GFF_EXPORT_PARAMS__HPP

/*  $Id: gff_export_params.hpp 37285 2016-12-21 17:56:47Z katargir $
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
 * CGffExportParams class declaration
 */

class CGffExportParams: public IRegSettings
{
    friend class CGffExportPage;
public:
    /// Default constructor for CGffExportParams
    CGffExportParams();

    /// Copy constructor for CGffExportParams
    CGffExportParams(const CGffExportParams& data);

    /// Destructor for CGffExportParams
    ~CGffExportParams();

    /// Assignment operator for CGffExportParams
    void operator=(const CGffExportParams& data);

    /// Equality operator for CGffExportParams
    bool operator==(const CGffExportParams& data) const;

    /// Copy function for CGffExportParams
    void Copy(const CGffExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }

////@begin CGffExportParams member function declarations
    wxString GetFeatureDepth() const { return m_FeatureDepth ; }
    void SetFeatureDepth(wxString value) { m_FeatureDepth = value ; }

    bool GetExactFlevel() const { return m_ExactFlevel ; }
    void SetExactFlevel(bool value) { m_ExactFlevel = value ; }

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    bool GetExtraQuals() const { return m_ExtraQuals ; }
    void SetExtraQuals(bool value) { m_ExtraQuals = value ; }

////@end CGffExportParams member function declarations

////@begin CGffExportParams member variables
private:
    wxString m_FeatureDepth;
    bool m_ExactFlevel;
    wxString m_FileName;
    bool m_ExtraQuals;
////@end CGffExportParams member variables

private:
    string m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___GFF_EXPORT_PARAMS__HPP
