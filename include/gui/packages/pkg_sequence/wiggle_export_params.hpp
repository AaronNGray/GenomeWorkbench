#ifndef PKG_SEQUENCE___WIGGLE_EXPORT_PARAMS__HPP
#define PKG_SEQUENCE___WIGGLE_EXPORT_PARAMS__HPP

/*  $Id: wiggle_export_params.hpp 37888 2017-02-28 15:44:33Z shkeda $
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

#include <corelib/ncbistd.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>
#include <wx/string.h>

BEGIN_NCBI_SCOPE

/*!
* CWiggleExportParams - parameters of the Wiggle exporter.
*/

class CWiggleExportParams: public IRegSettings
{    
    friend class CWiggleExportPage;
public:
    /// Default constructor for CWiggleExportParams
    CWiggleExportParams();

    /// Copy constructor for CWiggleExportParams
    CWiggleExportParams(const CWiggleExportParams& data);

    /// Destructor for CWiggleExportParams
    ~CWiggleExportParams();

    /// Assignment operator for CWiggleExportParams
    void operator=(const CWiggleExportParams& data);

    /// Equality operator for CWiggleExportParams
    bool operator==(const CWiggleExportParams& data) const;

    /// Copy function for CWiggleExportParams
    void Copy(const CWiggleExportParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    void UpdateFileName(const wxString& fname);

    const TConstScopedObjects& GetObjects() { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

////@begin CWiggleExportParams member function declarations
    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    wxString GetFrom() const { return m_From ; }
    void SetFrom(wxString value) { m_From = value ; }

    wxString GetTo() const { return m_To ; }
    void SetTo(wxString value) { m_To = value ; }

    int GetBinSize() const { return m_BinSize ; }
    void SetBinSize(int value) { m_BinSize = value ; }

////@end CWiggleExportParams member function declarations

////@begin CWiggleExportParams member variables
private:
    wxString m_FileName;
    wxString m_From;
    wxString m_To;
    int m_BinSize;
////@end CWiggleExportParams member variables

private:
    string m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___WIGGLE_EXPORT_PARAMS__HPP
