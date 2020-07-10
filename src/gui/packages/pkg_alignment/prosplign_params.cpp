/*  $Id: prosplign_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <algo/align/prosplign/prosplign.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/packages/pkg_alignment/prosplign_params.hpp>

BEGIN_NCBI_SCOPE

CProSplignParams::CProSplignParams()
{
    Init();
}

void CProSplignParams::Init()
{
    m_WithIntrons = true; // by default, ProSplign looks for introns
    m_Strand = 2;         // check both strands
    m_GeneticCode = 0;    // the one corresponding to the nucleotide sequence

    m_FrameshiftCost = CProSplignScoring::default_frameshift_opening;
    m_GapOpenCost = CProSplignScoring::default_gap_opening;
    m_GapExtendCost = CProSplignScoring::default_gap_extension;

    m_RefineAlignment = true;
    m_RemoveFlanks = CProSplignOutputOptions::default_fill_holes;
    m_RemoveNs = CProSplignOutputOptions::default_cut_ns;

    m_TotalPositives = CProSplignOutputOptions::default_total_positives;
    m_FlankPositives = CProSplignOutputOptions::default_flank_positives;
    m_MinGoodLength = CProSplignOutputOptions::default_min_good_len;

    m_MinExonIdentity = CProSplignOutputOptions::default_min_exon_id;
    m_MinExonPositives = CProSplignOutputOptions::default_min_exon_pos;
    m_MinFlankingExonLength = CProSplignOutputOptions::default_min_flanking_exon_len;

}

CRef<CProSplignScoring> CProSplignParams::GetScoringParams() const
{
    CRef<CProSplignScoring> scoring(new CProSplignScoring);
    scoring->SetGapOpeningCost(m_GapOpenCost);
    scoring->SetGapExtensionCost(m_GapExtendCost);
    scoring->SetFrameshiftOpeningCost(m_FrameshiftCost);
    return scoring;
}

CRef<CProSplignOutputOptions> CProSplignParams::GetOutputOptions() const
{
    if (!m_RefineAlignment) {
        return Ref(new CProSplignOutputOptions(CProSplignOutputOptions::ePassThrough));
    }

    CRef<CProSplignOutputOptions> options(new CProSplignOutputOptions);
    options->SetFillHoles(m_RemoveFlanks);
    options->SetCutNs(m_RemoveNs);

    options->SetTotalPositives(m_TotalPositives);
    options->SetFlankPositives(m_FlankPositives);

    options->SetMinGoodLen(m_MinGoodLength);

    options->SetMinExonId(m_MinExonIdentity);
    options->SetMinExonPos(m_MinExonPositives);
    options->SetMinFlankingExonLen(m_MinFlankingExonLength);
    return options;
}

void CProSplignParams::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

static const char
*kWithIntronsTag = "With introns",
*kNuclStrandTag = "Nucleotide strand",
*kGeneticCodeTag = "Genetic code ",
*kFrameshiftCostTag = "Frameshift penalty",
*kGapOpenCostTag = "Gap opening penalty",
*kGapExtendCostTag = "Gap extension penalty",
*kRefineAlignTag = "Refine alignment",
*kRemoveFlanksTag = "Remove flank regions",
*kRemoveNsTag = "Remove Ns from good regions",
*kTotalPosTag = "Total positives",
*kFlankPosTag = "Flank positives",
*kMinGoodLenTag = "Min good length",
*kMinExonIdTag = "Min exon identity",
*kMinExonPosTag = "Min exon positives",
*kMinFlankExonLenTag = "Min flanking exon length";

void CProSplignParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kWithIntronsTag, m_WithIntrons);
        view.Set(kNuclStrandTag, m_Strand);
        view.Set(kGeneticCodeTag, m_GeneticCode);
        view.Set(kFrameshiftCostTag, m_FrameshiftCost);
        view.Set(kGapOpenCostTag, m_GapOpenCost);
        view.Set(kGapExtendCostTag, m_GapExtendCost);
        view.Set(kRefineAlignTag, m_RefineAlignment);
        view.Set(kRemoveFlanksTag, m_RemoveFlanks);
        view.Set(kRemoveNsTag, m_RemoveNs);
        view.Set(kTotalPosTag, m_TotalPositives);
        view.Set(kFlankPosTag, m_FlankPositives);
        view.Set(kMinGoodLenTag, m_MinGoodLength);
        view.Set(kMinExonIdTag, m_MinExonIdentity);
        view.Set(kMinExonPosTag, m_MinExonPositives);
        view.Set(kMinFlankExonLenTag, m_MinFlankingExonLength);
    }
}

void CProSplignParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_WithIntrons = view.GetBool(kWithIntronsTag, m_WithIntrons);
        m_Strand = view.GetInt(kNuclStrandTag, m_Strand);
        m_GeneticCode = view.GetInt(kGeneticCodeTag, m_GeneticCode);
        m_FrameshiftCost = view.GetInt(kFrameshiftCostTag, m_FrameshiftCost);
        m_GapOpenCost = view.GetInt(kGapOpenCostTag, m_GapOpenCost);
        m_GapExtendCost = view.GetInt(kGapExtendCostTag, m_GapExtendCost);
        m_RefineAlignment = view.GetBool(kRefineAlignTag, m_RefineAlignment);
        m_RemoveFlanks = view.GetBool(kRemoveFlanksTag, m_RemoveFlanks);
        m_RemoveNs = view.GetBool(kRemoveNsTag, m_RemoveNs);
        m_TotalPositives = view.GetInt(kTotalPosTag, m_TotalPositives);
        m_FlankPositives = view.GetInt(kFlankPosTag, m_FlankPositives);
        m_MinGoodLength = view.GetInt(kMinGoodLenTag, m_MinGoodLength);
        m_MinExonIdentity = view.GetInt(kMinExonIdTag, m_MinExonIdentity);
        m_MinExonPositives = view.GetInt(kMinExonPosTag, m_MinExonPositives);
        m_MinFlankingExonLength = view.GetInt(kMinFlankExonLenTag, m_MinFlankingExonLength);
    }
}


void CProSplignParams::DebugDump(CDebugDumpContext ddc, unsigned int depth) const
{
    ddc.SetFrame("CProSplignParams");

    ddc.Log(kWithIntronsTag, m_WithIntrons);
    ddc.Log(kNuclStrandTag, m_Strand);
    ddc.Log(kGeneticCodeTag, m_GeneticCode);
    ddc.Log(kFrameshiftCostTag, m_FrameshiftCost);
    ddc.Log(kGapOpenCostTag, m_GapOpenCost);
    ddc.Log(kGapExtendCostTag, m_GapExtendCost);
    ddc.Log(kRefineAlignTag, m_RefineAlignment);
    ddc.Log(kRemoveFlanksTag, m_RemoveFlanks);
    ddc.Log(kRemoveNsTag, m_RemoveNs);
    ddc.Log(kTotalPosTag, m_TotalPositives);
    ddc.Log(kFlankPosTag, m_FlankPositives);
    ddc.Log(kMinGoodLenTag, m_MinGoodLength);
    ddc.Log(kMinExonIdTag, m_MinExonIdentity);
    ddc.Log(kMinExonPosTag, m_MinExonPositives);
    ddc.Log(kMinFlankExonLenTag, m_MinFlankingExonLength);
}

END_NCBI_SCOPE
