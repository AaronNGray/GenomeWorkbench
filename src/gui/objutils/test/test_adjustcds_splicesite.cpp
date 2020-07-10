/*  $Id: test_adjustcds_splicesite.cpp 43721 2019-08-22 17:22:05Z asztalos $
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
* Author:  Andrea Asztalos
*
* File Description:
*   Test for adjusting the cds for consensus splice sites
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/objutils/adjust_consensus_splicesite.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

CRef<CSeq_entry> ReadEntryFromFile(const string& fname)
{
    CRef<CSeq_entry> entry(new CSeq_entry);

    try {
        CNcbiIfstream istr(fname.c_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *entry;
    }
    catch (const CException& e) {
        LOG_POST(Error << e.ReportAll());
        return CRef<CSeq_entry>();
    }
    return entry;
}

// Prints ranges that were modified
void PrintRanges(const vector<TSeqRange>& orig_ranges, const CSeq_loc& loc)
{
    vector<TSeqRange> new_ranges;
    for (CSeq_loc_CI loc_it(loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological); loc_it; ++loc_it) {
        new_ranges.push_back(loc_it.GetRange());
    }

    auto orig_it = orig_ranges.begin();
    auto new_it = new_ranges.begin();
    for (; orig_it != orig_ranges.end() && new_it != new_ranges.end(); ++orig_it, ++new_it) {
        if (orig_it->GetFrom() != new_it->GetFrom() || orig_it->GetTo() != new_it->GetTo()) {
            cout << "(" << orig_it->GetFrom() << ", " << orig_it->GetTo() << ") --> ("
                << new_it->GetFrom() << ", " << new_it->GetTo() << ")\n";
        }
    }
}

void CheckSpliceSites(const CSeq_loc& loc, CScope& scope, const vector<TSeqRange>& orig_ranges)
{
    CBioseq_Handle bsh = scope.GetBioseqHandle(loc);
    auto strand = loc.GetStrand();
    TSeqPos start = 0;
    TSeqPos stop = bsh.GetBioseqLength();

    const auto& begin_rng = orig_ranges.front();
    const auto& end_rng = orig_ranges.back();
    if (strand == eNa_strand_minus) {
        if (end_rng.GetFrom() - start > 4) {
            start = end_rng.GetFrom() - 6;
        }
        if (stop - begin_rng.GetTo() > 4) {
            stop = begin_rng.GetTo() + 6;
        }
    }
    else {
        if (begin_rng.GetFrom() - start > 4) {
            start = begin_rng.GetFrom() - 6;
        }
        if (stop - end_rng.GetTo() > 4) {
            stop = end_rng.GetTo() + 6;
        }
    }

    CSeqVector seq_vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    string seqdata;
    seq_vec.GetSeqData(start, stop, seqdata);

    vector<TSeqRange> new_ranges;
    for (CSeq_loc_CI loc_it(loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological); loc_it; ++loc_it) {
        new_ranges.push_back(loc_it.GetRange());
    }


    CNcbiOstrstream log;
    if (strand == eNa_strand_minus) {
        auto new_it = new_ranges.rbegin();
        unsigned int i = 0;
        while (i < seqdata.size() && new_it != new_ranges.rend()) {

            if (i + start == new_it->GetFrom()) {
                log << new_it->GetFrom() << ":\t\t";
                if (i > 2) {
                    log << seqdata[i - 2];
                    BOOST_CHECK(seqdata[i - 2] == 'A');
                }
                if (i > 1) {
                    log << seqdata[i - 1];
                    BOOST_CHECK(seqdata[i - 1] == 'C');
                }
                log << "__[";

                if (i < seqdata.size()) {
                    log << seqdata[i];
                }

                if (++i < seqdata.size()) {
                    log << seqdata[i];
                }

                if (++i < seqdata.size()) {
                    log << seqdata[i];
                    log << "...";
                }
            }


            if (i + start == new_it->GetTo()) {
                for (int index = 3; index >= 0; --index) {
                    log << seqdata[i - index];
                }
                log << "]__";
                if (++i < seqdata.size()) {
                    log << seqdata[i];
                    BOOST_CHECK(seqdata[i] == 'C');
                }
                if (++i < seqdata.size()) {
                    log << seqdata[i];
                    BOOST_CHECK(seqdata[i] == 'T');
                }
                log << "\t\t:" << new_it->GetTo() << "\n";
                ++new_it;
            }
            ++i;
        }
    }
    else {
        auto new_it = new_ranges.begin();
        unsigned int i = 0;
        while (i < seqdata.size() && new_it != new_ranges.end()) {

            if (i + start == new_it->GetFrom()) {
                log << new_it->GetFrom() << ":\t\t";
                if (i > 2) {
                    log << seqdata[i - 2];
                    BOOST_CHECK(seqdata[i - 2] == 'A');
                }
                if (i > 1) {
                    log << seqdata[i - 1];
                    BOOST_CHECK(seqdata[i - 1] == 'G');
                }
                log << "__[";

                if (i < seqdata.size()) {
                    log << seqdata[i];
                }

                if (++i < seqdata.size()) {
                    log << seqdata[i];
                }

                if (++i < seqdata.size()) {
                    log << seqdata[i];
                    log << "...";
                }
            }


            if (i + start == new_it->GetTo()) {
                for (int index = 3; index >= 0; --index) {
                    log << seqdata[i - index];
                }
                log << "]__";
                if (++i < seqdata.size()) {
                    log << seqdata[i];
                    BOOST_CHECK(seqdata[i] == 'G');
                }
                if (++i < seqdata.size()) {
                    log << seqdata[i];
                    BOOST_CHECK(seqdata[i] == 'T' || seqdata[i] == 'C');
                }
                log << "\t\t:" << new_it->GetTo() << "\n";
                ++new_it;
            }
            ++i;
        }
    }

    cout << "\n" + string(CNcbiOstrstreamToString(log));
}

CSeq_entry_Handle AddEntryToScope(CSeq_entry& entry)
{
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(entry);
    BOOST_REQUIRE(seh);

    return seh;
}

const CSeq_feat& GetCDS(CBioseq_Handle bsh)
{
    CFeat_CI feat_it(bsh, SAnnotSelector(CSeqFeatData::e_Cdregion));
    return feat_it->GetOriginalFeature();
}

using TRanges = vector<TSeqRange>;

TRanges GetRanges(const CSeq_feat& feat)
{
    TRanges ranges;
    for (CSeq_loc_CI loc_it(feat.GetLocation(), CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological); loc_it; ++loc_it) {
        ranges.push_back(loc_it.GetRange());
    }
    return ranges;
}

void AdjustFeaturesInside(const CSeq_feat& cds, CScope& scope)
{
    CAdjustForConsensusSpliceSite worker(scope);
    CRef<CCmdComposite> cmd = worker.GetCommand(cds);
    BOOST_REQUIRE(cmd);
    cmd->Execute();
}

void AdjustFeaturesAtEnds(const CSeq_feat& cds, CScope& scope)
{
    CAdjustForConsensusSpliceSite worker(scope);
    CRef<CCmdComposite> cmd = worker.GetCommandToAdjustCDSEnds(cds);
    BOOST_REQUIRE(cmd);
    cmd->Execute();
}

void CheckAdjustedCDSmRNA(const TRanges& orig_ranges, CBioseq_Handle& bsh, bool full /* mRNA location matches CDS exactly or only at the ends*/)
{
    const CSeq_loc& adjusted_loc = GetCDS(bsh).GetLocation();
    //PrintRanges(orig_ranges, adjusted_loc);
    CheckSpliceSites(adjusted_loc, bsh.GetScope(), orig_ranges);

    CFeat_CI mrna_it(bsh, CSeqFeatData::eSubtype_mRNA);
    if (mrna_it) {
        if (full)
            BOOST_CHECK(mrna_it->GetOriginalFeature().GetLocation().Equals(adjusted_loc));
        else {
            BOOST_CHECK(mrna_it->GetLocation().GetStart(eExtreme_Biological) == adjusted_loc.GetStart(eExtreme_Biological));
            BOOST_CHECK(mrna_it->GetLocation().GetStop(eExtreme_Biological) == adjusted_loc.GetStop(eExtreme_Biological));
        }
    }
}


BOOST_AUTO_TEST_CASE(Test_AdjustCDS)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjustcds1.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesInside(orig_cds, seh.GetScope());

    cout << "\nTesting Test_Adjust CDS ----------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, true);
}

BOOST_AUTO_TEST_CASE(Test_Adjust2NegativeStrand)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjustcds2_negstrand.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    CFeat_CI exon_it(*b_iter, CSeqFeatData::eSubtype_exon);
    auto exon_1_orig_rng = GetRanges(exon_it->GetOriginalFeature());
    
    ++exon_it;
    auto exon_2_orig_rng = GetRanges(exon_it->GetOriginalFeature());

    ++exon_it;
    auto exon_3_orig_rng = GetRanges(exon_it->GetOriginalFeature());

    AdjustFeaturesInside(orig_cds, seh.GetScope());

    cout << "\nTesting Test_Adjust2NegativeStrand -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, true);
    
    cout << "\nExon1:\n";
    // check exons
    exon_it.Rewind();
    //PrintRanges(exon_1_orig_rng, exon_it->GetOriginalFeature().GetLocation());
    CheckSpliceSites(exon_it->GetMappedFeature().GetLocation(), seh.GetScope(), exon_1_orig_rng);
    cout << "\nExon2:\n";
    
    ++exon_it;
    //PrintRanges(exon_2_orig_rng, exon_it->GetOriginalFeature().GetLocation());
    CheckSpliceSites(exon_it->GetMappedFeature().GetLocation(), seh.GetScope(), exon_2_orig_rng);
    cout << "\nExon3:\n";

    ++exon_it;
    //PrintRanges(exon_3_orig_rng, exon_it->GetOriginalFeature().GetLocation());
    CheckSpliceSites(exon_it->GetMappedFeature().GetLocation(), seh.GetScope(), exon_3_orig_rng);
}

BOOST_AUTO_TEST_CASE(Test_Adjust_CDS_mRNA_Exons_PosStrand)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust_mrna_exons.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesInside(orig_cds, seh.GetScope());

    cout << "\nTesting Test_AdjustCDS_mRNA_Exons on positive strand ----------------------------\n\n";
    const CSeq_loc& adjusted_CDS_loc = GetCDS(bsh).GetLocation();
    CSeq_loc_CI loc_it(adjusted_CDS_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);
    BOOST_CHECK(loc_it.GetRange().GetFrom() == 453);
    BOOST_CHECK(loc_it.GetRange().GetTo() == 630);

    ++loc_it;
    BOOST_CHECK(loc_it.GetRange().GetFrom() == 1479);
    BOOST_CHECK(loc_it.GetRange().GetTo() == 2479);

    // check mRNA location
    CFeat_CI mrna_it(*b_iter, CSeqFeatData::eSubtype_mRNA);
    BOOST_REQUIRE(mrna_it);
    BOOST_CHECK(mrna_it->GetOriginalFeature().GetLocation().Equals(adjusted_CDS_loc));

    CFeat_CI exon_it(*b_iter, CSeqFeatData::eSubtype_exon);
    const auto& exon_loc_first = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_first.GetStart(eExtreme_Biological) == 453);
    BOOST_CHECK(exon_loc_first.GetStop(eExtreme_Biological) == 630);

    ++exon_it;
    const auto& exon_loc_second = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_second.GetStart(eExtreme_Biological) == 1479);
    BOOST_CHECK(exon_loc_second.GetStop(eExtreme_Biological) == 2479);

}
 

// Test cases for adjusting CDS only at the ends of the feature
BOOST_AUTO_TEST_CASE(Test_Adjust3End) 
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust3end.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 3' end to consensus splice site -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, true);

    CFeat_CI exon_it(*b_iter, CSeqFeatData::eSubtype_exon);
    const auto& exon_loc_first = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_first.GetStart(eExtreme_Biological) == 192);
    BOOST_CHECK(exon_loc_first.GetStop(eExtreme_Biological) == 215);

    ++exon_it;
    const auto& exon_loc_second = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_second.GetStart(eExtreme_Biological) == 594);
    BOOST_CHECK(exon_loc_second.GetStop(eExtreme_Biological) == 720);

    // test for not adjusting the end again
    CAdjustForConsensusSpliceSite worker(seh.GetScope());
    auto cmd = worker.GetCommandToAdjustCDSEnds(GetCDS(bsh));
    BOOST_CHECK(!cmd);

}

BOOST_AUTO_TEST_CASE(Test_Adjust3End_NegativeStrand)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust3end_negstrand.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 3' end to consensus splice site on the negative strand ---------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, true);

    CFeat_CI exon_it(*b_iter, CSeqFeatData::eSubtype_exon);
    const auto& exon_loc_first = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_first.GetStart(eExtreme_Biological) == 153);
    BOOST_CHECK(exon_loc_first.GetStop(eExtreme_Biological) == 27);

    ++exon_it;
    const auto& exon_loc_second = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_second.GetStart(eExtreme_Biological) == 555);
    BOOST_CHECK(exon_loc_second.GetStop(eExtreme_Biological) == 532);

    // test for not adjusting the end again
    CAdjustForConsensusSpliceSite worker(seh.GetScope());
    auto cmd = worker.GetCommandToAdjustCDSEnds(GetCDS(bsh));
    BOOST_CHECK(!cmd);
}

BOOST_AUTO_TEST_CASE(Test_Adjust3EndToSeqEnd)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust3end_toseqend.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 3' end to sequence end -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, true);
    const CSeq_loc& adjusted_loc = GetCDS(bsh).GetLocation();
    BOOST_CHECK(adjusted_loc.GetStop(eExtreme_Positional) == bsh.GetInst_Length() - 1);
}

BOOST_AUTO_TEST_CASE(Test_Adjust3EndToSeqEnd_NegativeStrand)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust3end_toseqend_negstrand.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 3' end to sequence end on the negative strand ----------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, true);
    const CSeq_loc& adjusted_loc = GetCDS(bsh).GetLocation();
    BOOST_CHECK(adjusted_loc.GetStart(eExtreme_Positional) == 0);
}

BOOST_AUTO_TEST_CASE(Test_DoNotAdjust3End)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/donot_adjust3end.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    CAdjustForConsensusSpliceSite worker(seh.GetScope());
    CRef<CCmdComposite> cmd = worker.GetCommandToAdjustCDSEnds(orig_cds);
    BOOST_CHECK(cmd == nullptr);
}

BOOST_AUTO_TEST_CASE(Test_Adjust5EndAndAdjustFrame)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust5end_frameshift.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 5' end to consensus splice site -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);

    // check exon locations
    CFeat_CI exon_it(*b_iter, CSeqFeatData::eSubtype_exon);
    const auto& exon_loc_first = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_first.GetStart(eExtreme_Biological) == 32);
    BOOST_CHECK(exon_loc_first.GetStop(eExtreme_Biological) == 164);

    ++exon_it;
    const auto& exon_loc_second = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_second.GetStart(eExtreme_Biological) == 560);
    BOOST_CHECK(exon_loc_second.GetStop(eExtreme_Biological) == 656);

    // check that the protein feature has been extended
    CBioseq_Handle protein = seh.GetScope().GetBioseqHandle(GetCDS(bsh).GetProduct());
    CFeat_CI prot_it(protein, CSeqFeatData::eSubtype_prot);
    BOOST_REQUIRE(prot_it);
    BOOST_CHECK(prot_it->GetLocation().GetStop(eExtreme_Biological) == 76);
    
    // test for not adjusting the end again
    CAdjustForConsensusSpliceSite worker(seh.GetScope());
    auto cmd = worker.GetCommandToAdjustCDSEnds(GetCDS(bsh));
    BOOST_CHECK(!cmd);
}

BOOST_AUTO_TEST_CASE(Test_Adjust5EndAndAdjustFrame_NegativeStrand)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust5end_frameshift_negstrand.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 5' end to consensus splice site on the negative strand --------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);

    // check exon locations
    CFeat_CI exon_it(*b_iter, CSeqFeatData::eSubtype_exon);
    const auto& exon_loc_first = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_first.GetStart(eExtreme_Biological) == 137);
    BOOST_CHECK(exon_loc_first.GetStop(eExtreme_Biological) == 41);

    ++exon_it;
    const auto& exon_loc_second = exon_it->GetOriginalFeature().GetLocation();
    BOOST_CHECK(exon_loc_second.GetStart(eExtreme_Biological) == 665);
    BOOST_CHECK(exon_loc_second.GetStop(eExtreme_Biological) == 533);

    // check that the protein feature has been extended
    CBioseq_Handle protein = seh.GetScope().GetBioseqHandle(GetCDS(bsh).GetProduct());
    CFeat_CI prot_it(protein, CSeqFeatData::eSubtype_prot);
    BOOST_REQUIRE(prot_it);
    BOOST_CHECK(prot_it->GetLocation().GetStop(eExtreme_Biological) == 76);

    // test for not adjusting the end again
    CAdjustForConsensusSpliceSite worker(seh.GetScope());
    auto cmd = worker.GetCommandToAdjustCDSEnds(GetCDS(bsh));
    BOOST_CHECK(!cmd);
}

BOOST_AUTO_TEST_CASE(Test_Adjust5EndToSeqEnd)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust5end_toseqend.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 5' end to sequence end -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);
    const CSeq_loc& adjusted_loc = GetCDS(bsh).GetLocation();
    BOOST_CHECK(adjusted_loc.GetStart(eExtreme_Positional) == 0);
}

BOOST_AUTO_TEST_CASE(Test_Adjust5EndToSeqEndNegativeStrand)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust5end_toseqend_negstrand.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 5' end to sequence end on negative strand ---------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);
    const CSeq_loc& adjusted_loc = GetCDS(bsh).GetLocation();
    BOOST_CHECK(adjusted_loc.GetStart(eExtreme_Positional) == 0);
}

BOOST_AUTO_TEST_CASE(Test_Adjust5EndToSeqEnd2)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust5end_frameshift2.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Extending the 5' end to sequence end -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);

    const CSeq_loc& adjusted_loc = GetCDS(bsh).GetLocation();
    BOOST_CHECK(adjusted_loc.GetStart(eExtreme_Positional) == 0);
}

BOOST_AUTO_TEST_CASE(Test_DoNotAdjust5End)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/donot_adjust5end.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    CAdjustForConsensusSpliceSite worker(seh.GetScope());
    CRef<CCmdComposite> cmd = worker.GetCommandToAdjustCDSEnds(orig_cds);
    BOOST_CHECK(cmd == nullptr);
}

BOOST_AUTO_TEST_CASE(Test_Trim5EndAndAdjustFrame)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/trim5end.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);
    
    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Trimming the 5' end to consensus splice site -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);
}

BOOST_AUTO_TEST_CASE(Test_AdjustBothEnds)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust_both_ends.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Adjust both ends to consensus splice sites -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);
}

BOOST_AUTO_TEST_CASE(Test_AdjustSingleInterval)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile("SpliceSiteData/adjust_singleinterval.asn");
    CSeq_entry_Handle seh = AddEntryToScope(*entry);

    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CBioseq_Handle bsh = *b_iter;
    const auto& orig_cds = GetCDS(bsh);
    auto orig_ranges = GetRanges(orig_cds);

    AdjustFeaturesAtEnds(orig_cds, seh.GetScope());

    cout << "\nTesting Adjust 5 end to consensus splice sites -------------------------\n\n";
    CheckAdjustedCDSmRNA(orig_ranges, bsh, false);
}

