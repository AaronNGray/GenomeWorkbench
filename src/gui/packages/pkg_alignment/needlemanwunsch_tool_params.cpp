/*  $Id: needlemanwunsch_tool_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_alignment/needlemanwunsch_tool_params.hpp>

BEGIN_NCBI_SCOPE

CNeedlemanWunschToolParams::CNeedlemanWunschToolParams()
{
    Init();
}
CNeedlemanWunschToolParams::CNeedlemanWunschToolParams(const CNeedlemanWunschToolParams& data)
{
    Init();
    Copy(data);
}
CNeedlemanWunschToolParams::~CNeedlemanWunschToolParams()
{
}
void CNeedlemanWunschToolParams::operator=(const CNeedlemanWunschToolParams& data)
{
    Copy(data);
}
bool CNeedlemanWunschToolParams::operator==(const CNeedlemanWunschToolParams& data) const
{
////@begin CNeedlemanWunschToolParams equality operator
     if (!(m_BandSize == data.m_BandSize)) return false;
     if (!(m_FirstSeqFreeEnds == data.m_FirstSeqFreeEnds)) return false;
     if (!(m_GapExtendCost == data.m_GapExtendCost)) return false;
     if (!(m_GapOpenCost == data.m_GapOpenCost)) return false;
     if (!(m_MatchCost == data.m_MatchCost)) return false;
     if (!(m_MismatchCost == data.m_MismatchCost)) return false;
     if (!(m_SecondSeqFreeEnds == data.m_SecondSeqFreeEnds)) return false;
////@end CNeedlemanWunschToolParams equality operator
    return true;
}
void CNeedlemanWunschToolParams::Copy(const CNeedlemanWunschToolParams& data)
{
////@begin CNeedlemanWunschToolParams copy function
    m_BandSize = data.m_BandSize;
    m_FirstSeqFreeEnds = data.m_FirstSeqFreeEnds;
    m_GapExtendCost = data.m_GapExtendCost;
    m_GapOpenCost = data.m_GapOpenCost;
    m_MatchCost = data.m_MatchCost;
    m_MismatchCost = data.m_MismatchCost;
    m_SecondSeqFreeEnds = data.m_SecondSeqFreeEnds;
////@end CNeedlemanWunschToolParams copy function
    m_Objects = data.m_Objects;
}
void CNeedlemanWunschToolParams::Init()
{
////@begin CNeedlemanWunschToolParams member initialisation
    m_BandSize = 0;
    m_FirstSeqFreeEnds = 0;
    m_GapExtendCost = -2;
    m_GapOpenCost = -5;
    m_MatchCost = 1;
    m_MismatchCost = -2;
    m_SecondSeqFreeEnds = 0;
////@end CNeedlemanWunschToolParams member initialisation
}

static const char
*kMatchCost = "MatchCost",
*kMismatchCost = "MismatchCost",
*kGapOpenCost = "GapOpenCost",
*kGapExtendCost = "GapExtendCost",
*kBandSize = "BandSize",
*kFirstSeqFreeEnds = "FirstSeqFreeEnds",
*kSecondSeqFreeEnds = "SecondSeqFreeEnds";

void CNeedlemanWunschToolParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kMatchCost, m_MatchCost);
        view.Set(kMismatchCost, m_MismatchCost);
        view.Set(kGapOpenCost, m_GapOpenCost);
        view.Set(kGapExtendCost, m_GapExtendCost);
        view.Set(kBandSize, m_BandSize);
        view.Set(kFirstSeqFreeEnds, m_FirstSeqFreeEnds);
        view.Set(kSecondSeqFreeEnds, m_SecondSeqFreeEnds);
    }
}

void CNeedlemanWunschToolParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_MatchCost = view.GetInt(kMatchCost, m_MatchCost);
        m_MismatchCost = view.GetInt(kMismatchCost, m_MismatchCost);
        m_GapOpenCost = view.GetInt(kGapOpenCost, m_GapOpenCost);
        m_GapExtendCost = view.GetInt(kGapExtendCost, m_GapExtendCost);
        m_BandSize = view.GetInt(kBandSize, m_BandSize);
        m_FirstSeqFreeEnds = view.GetInt(kFirstSeqFreeEnds, m_FirstSeqFreeEnds);
        m_SecondSeqFreeEnds = view.GetInt(kSecondSeqFreeEnds, m_SecondSeqFreeEnds);
    }
}

END_NCBI_SCOPE
