#ifndef PKG_ALIGNMENT___NEEDLEMANWUNSCH_TOOL_PARAMS__HPP
#define PKG_ALIGNMENT___NEEDLEMANWUNSCH_TOOL_PARAMS__HPP

/*  $Id: needlemanwunsch_tool_params.hpp 37333 2016-12-23 20:25:59Z katargir $
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

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

 /*!
 * CNeedlemanWunschToolParams class declaration
 */

class CNeedlemanWunschToolParams: public IRegSettings
{    
    friend class CAlignNeedlemanWunschPanel;
public:
    enum EFreeEndType {
        eNone,
        eLeft,
        eRight,
        eBoth
    };

    CNeedlemanWunschToolParams();

    CNeedlemanWunschToolParams(const CNeedlemanWunschToolParams& data);

    ~CNeedlemanWunschToolParams();

    void operator=(const CNeedlemanWunschToolParams& data);

    bool operator==(const CNeedlemanWunschToolParams& data) const;

    void Copy(const CNeedlemanWunschToolParams& data);

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    const TConstScopedObjects& GetObjects() const { return m_Objects; }
    TConstScopedObjects& SetObjects() { return m_Objects; }

    ////@begin CNeedlemanWunschToolParams member function declarations
    int GetBandSize() const { return m_BandSize ; }
    void SetBandSize(int value) { m_BandSize = value ; }

    int GetFirstSeqFreeEnds() const { return m_FirstSeqFreeEnds ; }
    void SetFirstSeqFreeEnds(int value) { m_FirstSeqFreeEnds = value ; }

    int GetGapExtendCost() const { return m_GapExtendCost ; }
    void SetGapExtendCost(int value) { m_GapExtendCost = value ; }

    int GetGapOpenCost() const { return m_GapOpenCost ; }
    void SetGapOpenCost(int value) { m_GapOpenCost = value ; }

    int GetMatchCost() const { return m_MatchCost ; }
    void SetMatchCost(int value) { m_MatchCost = value ; }

    int GetMismatchCost() const { return m_MismatchCost ; }
    void SetMismatchCost(int value) { m_MismatchCost = value ; }

    int GetSecondSeqFreeEnds() const { return m_SecondSeqFreeEnds ; }
    void SetSecondSeqFreeEnds(int value) { m_SecondSeqFreeEnds = value ; }

////@end CNeedlemanWunschToolParams member function declarations

////@begin CNeedlemanWunschToolParams member variables
private:
    int m_BandSize;
    int m_FirstSeqFreeEnds;
    int m_GapExtendCost;
    int m_GapOpenCost;
    int m_MatchCost;
    int m_MismatchCost;
    int m_SecondSeqFreeEnds;
////@end CNeedlemanWunschToolParams member variables

private:
    string m_RegPath;
    TConstScopedObjects m_Objects;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___NEEDLEMANWUNSCH_TOOL_PARAMS__HPP
