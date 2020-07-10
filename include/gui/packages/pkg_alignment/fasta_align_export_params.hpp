/*  $Id: fasta_align_export_params.hpp 38543 2017-05-24 21:37:01Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description:
 *
 */

#ifndef PKG_ALIGNMENT___FASTA_ALIGN_EXPORT_PARAMS_HPP
#define PKG_ALIGNMENT___FASTA_ALIGN_EXPORT_PARAMS_HPP

#include <corelib/ncbistd.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CFastaAlignExportParams class declaration
 */

class CFastaAlignExportParams: public IRegSettings
{    
    friend class CFastaAlignExportPage;
public:
    CFastaAlignExportParams();

    CFastaAlignExportParams(const CFastaAlignExportParams& data);

    ~CFastaAlignExportParams();

    void operator=(const CFastaAlignExportParams& data);

    bool operator==(const CFastaAlignExportParams& data) const;

    void Copy(const CFastaAlignExportParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const {}
    virtual void LoadSettings() {}

    const SConstScopedObject& GetObject() const { return m_Object; }
    SConstScopedObject& SetObject() { return m_Object; }

////@begin CFastaAlignExportParams member function declarations
    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

////@end CFastaAlignExportParams member function declarations

////@begin CFastaAlignExportParams member variables
private:
    wxString m_FileName;
////@end CFastaAlignExportParams member variables

private:
    string m_RegPath;
    SConstScopedObject m_Object;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___FASTA_ALIGN_EXPORT_PARAMS_HPP
