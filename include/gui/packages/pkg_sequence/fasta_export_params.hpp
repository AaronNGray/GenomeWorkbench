#ifndef PKG_SEQUENCE___FASTA_EXPORT_PARAMS__HPP
#define PKG_SEQUENCE___FASTA_EXPORT_PARAMS__HPP

/*  $Id: fasta_export_params.hpp 38529 2017-05-23 14:48:11Z evgeniev $
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
 * CFastaExportParams class declaration
 */

class CFastaExportParams: public IRegSettings
{    
    friend class CFastaExportPage;
public:
    enum EExportType {
        eUniqueLocations = 0,
        eWholeSequences
    };

    /// Default constructor for CFastaExportParams
    CFastaExportParams();

    /// Copy constructor for CFastaExportParams
    CFastaExportParams(const CFastaExportParams& data);

    /// Destructor for CFastaExportParams
    ~CFastaExportParams();

    /// Assignment operator for CFastaExportParams
    void operator=(const CFastaExportParams& data);

    /// Equality operator for CFastaExportParams
    bool operator==(const CFastaExportParams& data) const;

    /// Copy function for CFastaExportParams
    void Copy(const CFastaExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }

////@begin CFastaExportParams member function declarations
    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    bool GetSaveMasterRecord() const { return m_SaveMasterRecord ; }
    void SetSaveMasterRecord(bool value) { m_SaveMasterRecord = value ; }

    int GetExportType() const { return m_ExportType ; }
    void SetExportType(int value) { m_ExportType = value ; }

////@end CFastaExportParams member function declarations

////@begin CFastaExportParams member variables
private:
    wxString m_FileName;
    bool m_SaveMasterRecord;
    int m_ExportType;
////@end CFastaExportParams member variables

private:
    string m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___FASTA_EXPORT_PARAMS__HPP
