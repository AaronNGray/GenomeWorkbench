#ifndef PKG_ALIGNMENT___SPLIGN_PARAMS__HPP
#define PKG_ALIGNMENT___SPLIGN_PARAMS__HPP

/*  $Id: splign_params.hpp 32655 2015-04-07 18:11:22Z evgeniev $
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
 * CSplignParams class declaration
 */

class CSplignParams: public CDebugDumpable, public IRegSettings
{
    friend class CSplignPanel;
public:
    CSplignParams();

    CSplignParams(const CSplignParams& data);

    ~CSplignParams();

    void operator=(const CSplignParams& data);

    bool operator==(const CSplignParams& data) const;

    void Copy(const CSplignParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

////@begin CSplignParams member function declarations
    double GetCompPenalty() const { return m_CompPenalty ; }
    void SetCompPenalty(double value) { m_CompPenalty = value ; }

    int GetDiscType() const { return m_DiscType ; }
    void SetDiscType(int value) { m_DiscType = value ; }

    bool GetEndGapDetect() const { return m_EndGapDetect ; }
    void SetEndGapDetect(bool value) { m_EndGapDetect = value ; }

    long GetMaxGenomicExtent() const { return m_MaxGenomicExtent ; }
    void SetMaxGenomicExtent(long value) { m_MaxGenomicExtent = value ; }

    long GetMaxIntron() const { return m_MaxIntron ; }
    void SetMaxIntron(long value) { m_MaxIntron = value ; }

    double GetMinCompIdentity() const { return m_MinCompIdentity ; }
    void SetMinCompIdentity(double value) { m_MinCompIdentity = value ; }

    double GetMinExonIdentity() const { return m_MinExonIdentity ; }
    void SetMinExonIdentity(double value) { m_MinExonIdentity = value ; }

    bool GetPolyADetect() const { return m_PolyADetect ; }
    void SetPolyADetect(bool value) { m_PolyADetect = value ; }

    int GetCDNAStrand() const { return m_cDNA_strand ; }
    void SetCDNAStrand(int value) { m_cDNA_strand = value ; }

////@end CSplignParams member function declarations

	virtual void DebugDump( CDebugDumpContext ddc, unsigned int depth ) const;

////@begin CSplignParams member variables
private:
    double m_CompPenalty;
    int m_DiscType;
    bool m_EndGapDetect;
    long m_MaxGenomicExtent;
    long m_MaxIntron;
    double m_MinCompIdentity;
    double m_MinExonIdentity;
    bool m_PolyADetect;
    int m_cDNA_strand;
////@end CSplignParams member variables

    string m_RegPath;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___SPLIGN_PARAMS__HPP
