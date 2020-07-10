#ifndef PKG_ALIGNMENT___PROSPLIGN_PARAMS__HPP
#define PKG_ALIGNMENT___PROSPLIGN_PARAMS__HPP

/*  $Id: prosplign_params.hpp 35635 2016-06-03 19:05:47Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/objutils/reg_settings.hpp>

BEGIN_NCBI_SCOPE

class CProSplignScoring;
class CProSplignOutputOptions;

class CProSplignParams : public CDebugDumpable, public IRegSettings
{
public:
    CProSplignParams();

    ~CProSplignParams() {}

    void Init();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    virtual void DebugDump(CDebugDumpContext ddc, unsigned int depth) const;

    CRef<CProSplignScoring> GetScoringParams() const;
    CRef<CProSplignOutputOptions> GetOutputOptions() const;

private:
    string m_RegPath;

public:
    bool m_WithIntrons;
    int m_Strand;
    int m_GeneticCode;

    int m_FrameshiftCost;
    int m_GapOpenCost;
    int m_GapExtendCost;

    bool m_RefineAlignment;
    bool m_RemoveFlanks;
    bool m_RemoveNs;
    int m_FlankPositives;
    int m_TotalPositives;
    int m_MinGoodLength;
    int m_MinExonIdentity;
    int m_MinExonPositives;
    int m_MinFlankingExonLength;

};

END_NCBI_SCOPE

#endif
    // PKG_ALIGNMENT___PROSPLIGN_PARAMS__HPP

