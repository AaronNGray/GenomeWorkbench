#ifndef PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PARAMS__HPP
#define PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PARAMS__HPP

/*  $Id: align_tab_export_params.hpp 37334 2016-12-23 20:41:42Z katargir $
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

#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CAlignTabExportParams class declaration
 */

class CAlignTabExportParams: public IRegSettings
{    
    friend class CAlignTabExportPage;
    friend class CAlignTabExportPage2;
public:
    CAlignTabExportParams();

    CAlignTabExportParams(const CAlignTabExportParams& data);

    ~CAlignTabExportParams();

    void operator=(const CAlignTabExportParams& data);

    bool operator==(const CAlignTabExportParams& data) const;

    void Copy(const CAlignTabExportParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const SConstScopedObject& GetObject() const { return m_Object; }
    SConstScopedObject& SetObject() { return m_Object; }

////@begin CAlignTabExportParams member function declarations
    string GetAlignmentName() const { return m_AlignmentName ; }
    void SetAlignmentName(string value) { m_AlignmentName = value ; }

    vector<string> GetFields() const { return m_Fields ; }
    void SetFields(vector<string> value) { m_Fields = value ; }

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

////@end CAlignTabExportParams member function declarations

////@begin CAlignTabExportParams member variables
private:
    string m_AlignmentName;
    vector<string> m_Fields;
    wxString m_FileName;
////@end CAlignTabExportParams member variables

private:
    string m_RegPath;
    SConstScopedObject m_Object;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PARAMS__HPP
