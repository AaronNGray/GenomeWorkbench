#ifndef PKG_SEQUENCE___5COL_EXPORT_PARAMS__HPP
#define PKG_SEQUENCE___5COL_EXPORT_PARAMS__HPP

/*  $Id: 5col_export_params.hpp 26968 2012-11-30 18:22:25Z katargir $
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
 * C5ColExportParams class declaration
 */

class C5ColExportParams: public IRegSettings
{    
    friend class C5ColExportPage;
public:
    /// Default constructor for C5ColExportParams
    C5ColExportParams();

    /// Copy constructor for C5ColExportParams
    C5ColExportParams(const C5ColExportParams& data);

    /// Destructor for C5ColExportParams
    ~C5ColExportParams();

    /// Assignment operator for C5ColExportParams
    void operator=(const C5ColExportParams& data);

    /// Equality operator for C5ColExportParams
    bool operator==(const C5ColExportParams& data) const;

    /// Copy function for C5ColExportParams
    void Copy(const C5ColExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin C5ColExportParams member function declarations
    wxString GetFeatureDepth() const { return m_FeatureDepth ; }
    void SetFeatureDepth(wxString value) { m_FeatureDepth = value ; }

    bool GetExactFlevel() const { return m_ExactFlevel ; }
    void SetExactFlevel(bool value) { m_ExactFlevel = value ; }

////@end C5ColExportParams member function declarations

////@begin C5ColExportParams member variables
private:
    wxString m_FeatureDepth;
    bool m_ExactFlevel;
////@end C5ColExportParams member variables

private:
    string m_RegPath;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___GFF_EXPORT_PARAMS__HPP
