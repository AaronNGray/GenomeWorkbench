#ifndef PKG_ALIGNMENT___NGALIGN_PARAMS__HPP
#define PKG_ALIGNMENT___NGALIGN_PARAMS__HPP

/*  $Id: ngalign_params.hpp 37341 2016-12-27 17:19:44Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
*
* @{
*/

/*!
* CNGAlignParams class declaration
*/

class CNGAlignParams : public IRegSettings
{
    friend class CNGAlignPanel;
    friend class CNGAlignBLASTPanel;
public:
    CNGAlignParams();

    CNGAlignParams(const CNGAlignParams& data);

    ~CNGAlignParams();

    void operator=(const CNGAlignParams& data);

    bool operator==(const CNGAlignParams& data) const;

    void Copy(const CNGAlignParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const SConstScopedObject& GetSubject() const { return m_Subject; }
    SConstScopedObject& SetSubject() { return m_Subject; }

    const TConstScopedObjects& GetQueries() const { return m_Queries; }
    TConstScopedObjects& SetQueries() { return m_Queries; }

    ////@begin CNGAlignParams member function declarations
    int GetFilterMode() const { return m_FilterMode ; }
    void SetFilterMode(int value) { m_FilterMode = value ; }

    long GetWordSize() const { return m_WordSize ; }
    void SetWordSize(long value) { m_WordSize = value ; }

    double GetEValue() const { return m_eValue ; }
    void SetEValue(double value) { m_eValue = value ; }

    bool GetBestHit() const { return m_BestHit ; }
    void SetBestHit(bool value) { m_BestHit = value ; }

    wxString GetJobTitle() const { return m_JobTitle ; }
    void SetJobTitle(wxString value) { m_JobTitle = value ; }

    wxString GetAdvParams() const { return m_AdvParams ; }
    void SetAdvParams(wxString value) { m_AdvParams = value ; }

    int GetWMTaxId() const { return m_WM_TaxId ; }
    void SetWMTaxId(int value) { m_WM_TaxId = value ; }

    string GetWMDir() const { return m_WM_Dir ; }
    void SetWMDir(string value) { m_WM_Dir = value ; }

    ////@end CNGAlignParams member function declarations

    void LogDump();

    ////@begin CNGAlignParams member variables
    bool m_BestHit;
private:
    int m_FilterMode;
    long m_WordSize;
    double m_eValue;
    wxString m_JobTitle;
    wxString m_AdvParams;
    int m_WM_TaxId;
    string m_WM_Dir;
    ////@end CNGAlignParams member variables

private:
    string m_RegPath;
    SConstScopedObject m_Subject;
    TConstScopedObjects m_Queries;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___NGALIGN_PARAMS__HPP
