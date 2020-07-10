#ifndef PKG_SEQUENCE___FLATFILE_EXPORT_PARAMS__HPP
#define PKG_SEQUENCE___FLATFILE_EXPORT_PARAMS__HPP

/*  $Id: flatfile_export_params.hpp 37300 2016-12-21 20:55:52Z katargir $
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
 * CFlatFileExportParams class declaration
 */

class CFlatFileExportParams: public IRegSettings
{
    friend class CFlatFileExportPage;
public:
    /// Default constructor for IRegSettings
    CFlatFileExportParams();

    /// Copy constructor for IRegSettings
    CFlatFileExportParams(const CFlatFileExportParams& data);

    /// Destructor for IRegSettings
    ~CFlatFileExportParams();

    /// Assignment operator for IRegSettings
    void operator=(const CFlatFileExportParams& data);

    /// Equality operator for IRegSettings
    bool operator==(const CFlatFileExportParams& data) const;

    /// Copy function for IRegSettings
    void Copy(const CFlatFileExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    SConstScopedObject GetSeqLoc() const { return m_SeqLoc; }

    ////@begin CFlatFileExportParams member function declarations
    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    int GetMode() const { return m_Mode ; }
    void SetMode(int value) { m_Mode = value ; }

////@end CFlatFileExportParams member function declarations

////@begin CFlatFileExportParams member variables
private:
    wxString m_FileName;
    int m_Mode;
////@end CFlatFileExportParams member variables

private:
    string m_RegPath;
    SConstScopedObject m_SeqLoc;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___FLATFILE_EXPORT_PARAMS__HPP
