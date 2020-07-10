#ifndef PKG_ALIGNMENT___FIND_OVERLAP_PARAMS__HPP
#define PKG_ALIGNMENT___FIND_OVERLAP_PARAMS__HPP

/*  $Id: find_overlap_params.hpp 37335 2016-12-23 20:57:45Z katargir $
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
 * CFindOverlapParams class declaration
 */

class CFindOverlapParams: public IRegSettings
{
    friend class CFindOverlapPanel;
public:
    CFindOverlapParams();

    CFindOverlapParams(const CFindOverlapParams& data);

    ~CFindOverlapParams();

    void operator=(const CFindOverlapParams& data);

    bool operator==(const CFindOverlapParams& data) const;

    void Copy(const CFindOverlapParams& data);

    void Init();

/// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const SConstScopedObject& GetSeq1() const { return m_Seq1; }
    SConstScopedObject& SetSeq1() { return m_Seq1; }

    const SConstScopedObject& GetSeq2() const { return m_Seq2; }
    SConstScopedObject& SetSeq2() { return m_Seq2; }

////@begin CFindOverlapParams member function declarations
    wxString GetBlastParams() const { return m_BlastParams ; }
    void SetBlastParams(wxString value) { m_BlastParams = value ; }

    int GetFilterQuality() const { return m_FilterQty ; }
    void SetFilterQuality(int value) { m_FilterQty = value ; }

    wxString GetMaxSlop() const { return m_MaxSlop ; }
    void SetMaxSlop(wxString value) { m_MaxSlop = value ; }

////@end CFindOverlapParams member function declarations

////@begin CFindOverlapParams member variables
private:
    wxString m_BlastParams;
    int m_FilterQty;
    wxString m_MaxSlop;
////@end CFindOverlapParams member variables

    string m_RegPath;
    SConstScopedObject m_Seq1;
    SConstScopedObject m_Seq2;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___FIND_OVERLAP_PARAMS__HPP
