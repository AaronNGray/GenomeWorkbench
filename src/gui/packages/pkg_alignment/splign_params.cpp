/*  $Id: splign_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_alignment/splign_params.hpp>

BEGIN_NCBI_SCOPE

CSplignParams::CSplignParams()
{
    Init();
}
CSplignParams::CSplignParams(const CSplignParams& data)
{
    Init();
    Copy(data);
}
CSplignParams::~CSplignParams()
{
}
void CSplignParams::operator=(const CSplignParams& data)
{
    Copy(data);
}
bool CSplignParams::operator==(const CSplignParams& data) const
{
////@begin CSplignParams equality operator
     if (!(m_CompPenalty == data.m_CompPenalty)) return false;
     if (!(m_DiscType == data.m_DiscType)) return false;
     if (!(m_EndGapDetect == data.m_EndGapDetect)) return false;
     if (!(m_MaxGenomicExtent == data.m_MaxGenomicExtent)) return false;
     if (!(m_MaxIntron == data.m_MaxIntron)) return false;
     if (!(m_MinCompIdentity == data.m_MinCompIdentity)) return false;
     if (!(m_MinExonIdentity == data.m_MinExonIdentity)) return false;
     if (!(m_PolyADetect == data.m_PolyADetect)) return false;
     if (!(m_cDNA_strand == data.m_cDNA_strand)) return false;
////@end CSplignParams equality operator
    return true;
}
void CSplignParams::Copy(const CSplignParams& data)
{
////@begin CSplignParams copy function
    m_CompPenalty = data.m_CompPenalty;
    m_DiscType = data.m_DiscType;
    m_EndGapDetect = data.m_EndGapDetect;
    m_MaxGenomicExtent = data.m_MaxGenomicExtent;
    m_MaxIntron = data.m_MaxIntron;
    m_MinCompIdentity = data.m_MinCompIdentity;
    m_MinExonIdentity = data.m_MinExonIdentity;
    m_PolyADetect = data.m_PolyADetect;
    m_cDNA_strand = data.m_cDNA_strand;
////@end CSplignParams copy function
}
void CSplignParams::Init()
{
////@begin CSplignParams member initialisation
    m_CompPenalty = 0.55;
    m_DiscType = 0;
    m_EndGapDetect = true;
    m_MaxGenomicExtent = 35000;
    m_MaxIntron = 1200000;
    m_MinCompIdentity = 0.5;
    m_MinExonIdentity = 0.75;
    m_PolyADetect = true;
    m_cDNA_strand = 0;
////@end CSplignParams member initialisation
}

static const char
*kcDNA_strandTag = "cDNA strand",
*kEndGapDetectTag = "EndGap Detection",
*kMinCompIdentityTag = "Min compartment identity",
*kMinExonIdentityTag = "Min exon identity",
*kMaxGenomicExtentTag = "Maximum Genomic Extent",
*kCompPenaltyTag = "Compartment penalty",
*kPolyADetectTag = "PolyA Detection",
*kMaxIntron = "MaxIntron",
*kDiscType = "DiskType";


void CSplignParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kcDNA_strandTag, m_cDNA_strand);
        view.Set(kEndGapDetectTag, m_EndGapDetect);
        view.Set(kPolyADetectTag, m_PolyADetect);
        view.Set(kMinCompIdentityTag, m_MinCompIdentity);
        view.Set(kMinExonIdentityTag, m_MinExonIdentity);
        view.Set(kMaxGenomicExtentTag, (int)m_MaxGenomicExtent);
        view.Set(kCompPenaltyTag, m_CompPenalty);
        view.Set(kMaxIntron, (int)m_MaxIntron);
        view.Set(kDiscType, m_DiscType);
    }
}

void CSplignParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_cDNA_strand = view.GetInt(kcDNA_strandTag, m_cDNA_strand);
        m_EndGapDetect = view.GetBool(kEndGapDetectTag, m_EndGapDetect);
        m_PolyADetect = view.GetBool(kPolyADetectTag, m_PolyADetect);
        m_MinCompIdentity = view.GetReal(kMinCompIdentityTag, m_MinCompIdentity);
        m_MinExonIdentity = view.GetReal(kMinExonIdentityTag, m_MinExonIdentity);
        m_MaxGenomicExtent = view.GetInt(kMaxGenomicExtentTag, m_MaxGenomicExtent);
        m_CompPenalty = view.GetReal(kCompPenaltyTag, m_CompPenalty);
        m_MaxIntron = view.GetInt(kMaxIntron, m_MaxIntron);
        m_DiscType = view.GetInt(kDiscType, m_DiscType);
    }
}

void CSplignParams::DebugDump( CDebugDumpContext ddc, unsigned int /*depth*/) const
{
	ddc.SetFrame("CSplignParams");

	ddc.Log( "cDNA strand", m_cDNA_strand );
	ddc.Log( "End Gap Detect", m_EndGapDetect );
	ddc.Log( "PolyA Detect", m_PolyADetect );
	ddc.Log( "Min Comp Identity", m_MinCompIdentity );
	ddc.Log( "Min Exon Identity", m_MinExonIdentity );
	ddc.Log( "Max Genomic Extent", m_MaxGenomicExtent );
	ddc.Log( "Comp Penalty", m_CompPenalty );
	ddc.Log( "Max Intron", m_MaxIntron );
	ddc.Log( "Disc Type", m_DiscType );
	
	//ddc.Log( "", 
	//SConstScopedObject m_GenomicSeq;
	//TConstScopedObjects m_cDNASeqs;
}


END_NCBI_SCOPE
