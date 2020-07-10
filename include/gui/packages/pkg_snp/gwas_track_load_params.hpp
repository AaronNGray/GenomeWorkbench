#ifndef PKG_SNP__GWAS_LOAD_PARAMS__HPP
#define PKG_SNP__GWAS_LOAD_PARAMS__HPP

/*  $Id: gwas_track_load_params.hpp 18397 2008-11-28 19:45:17Z dicuccio $
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
 * Authors:  Melvin Quintos
 */

#include <corelib/ncbistd.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <wx/string.h>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_SNP
 *
 * @{
 */

/*!
 * CGwasLoadParams class declaration
 */

class CGwasLoadParams: public IRegSettings
{
    friend class CGwasTrackParamsPanel;
public:

    enum ETrackType {
        eAssociation,
        eLinkage
    };

    enum EAccessType {
        ePublic,
        eRestricted
    };

public:
    /// Default constructor for CGwasLoadParams
    CGwasLoadParams();

    /// Copy constructor for CGwasLoadParams
    CGwasLoadParams(const CGwasLoadParams& data);

    /// Destructor for CGwasLoadParams
    ~CGwasLoadParams();

    /// Assignment operator for CGwasLoadParams
    void operator=(const CGwasLoadParams& data);

    /// Equality operator for CGwasLoadParams
    bool operator==(const CGwasLoadParams& data) const;

    /// Copy function for CGwasLoadParams
    void Copy(const CGwasLoadParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    ////@begin CGwasLoadParams member function declarations
    wxString GetTitle() const { return m_sTitle ; }
    void SetTitle(wxString value) { m_sTitle = value ; }

    int GetETrackType() const { return m_eTrackType ; }
    void SetETrackType(int value) { m_eTrackType = value ; }

////@end CGwasLoadParams member function declarations

////@begin CGwasLoadParams member variables
private:
    wxString m_sTitle;
    int m_eTrackType;
////@end CGwasLoadParams member variables

    string m_RegPath;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_SNP__GWAS_LOAD_PARAMS__HPP
