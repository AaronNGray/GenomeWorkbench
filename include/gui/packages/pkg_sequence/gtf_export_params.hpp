#ifndef PKG_SEQUENCE___GTF_EXPORT_PARAMS__HPP
#define PKG_SEQUENCE___GTF_EXPORT_PARAMS__HPP

/*  $Id: gtf_export_params.hpp 37293 2016-12-21 19:29:31Z katargir $
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
 * CGtfExportParams class declaration
 */

class CGtfExportParams: public IRegSettings
{    
    friend class CGtfExportPage;
public:
    /// Default constructor for CGtfExportParams
    CGtfExportParams();

    /// Copy constructor for CGtfExportParams
    CGtfExportParams(const CGtfExportParams& data);

    /// Destructor for CGtfExportParams
    ~CGtfExportParams();

    /// Assignment operator for CGtfExportParams
    void operator=(const CGtfExportParams& data);

    /// Equality operator for CGtfExportParams
    bool operator==(const CGtfExportParams& data) const;

    /// Copy function for CGtfExportParams
    void Copy(const CGtfExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }

////@begin CGtfExportParams member function declarations
    wxString GetFeatureDepth() const { return m_FeatureDepth ; }
    void SetFeatureDepth(wxString value) { m_FeatureDepth = value ; }

    bool GetExactFlevel() const { return m_ExactFlevel ; }
    void SetExactFlevel(bool value) { m_ExactFlevel = value ; }

    bool GetSuppressGeneFeatures() const { return m_SuppressGeneFeatures ; }
    void SetSuppressGeneFeatures(bool value) { m_SuppressGeneFeatures = value ; }

    bool GetSuppressExonNumbers() const { return m_SuppressExonNumbers ; }
    void SetSuppressExonNumbers(bool value) { m_SuppressExonNumbers = value ; }

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

////@end CGtfExportParams member function declarations

////@begin CGtfExportParams member variables
private:
    wxString m_FeatureDepth;
    bool m_ExactFlevel;
    bool m_SuppressGeneFeatures;
    bool m_SuppressExonNumbers;
    wxString m_FileName;
////@end CGtfExportParams member variables

private:
    string m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___GTF_EXPORT_PARAMS__HPP
