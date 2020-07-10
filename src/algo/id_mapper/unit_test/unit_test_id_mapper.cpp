/*  $Id: unit_test_id_mapper.cpp 606392 2020-04-21 11:30:33Z ivanov $
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
 * Author: Nathan Bouk
 *
 * File Description:
 *
 * ===========================================================================
 */

#include <ncbi_pch.hpp>
#include <corelib/test_boost.hpp>
#include <objects/general/general__.hpp>
#include <objects/seqloc/seqloc__.hpp>
#include <objects/genomecoll/genome_collection__.hpp>
#include <objects/genomecoll/genomic_collections_cli.hpp>
#include <algo/id_mapper/id_mapper.hpp>


#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

USING_NCBI_SCOPE;
USING_SCOPE(objects);


BOOST_AUTO_TEST_SUITE(TestSuiteGencollIdMapper)


BOOST_AUTO_TEST_CASE(TestCaseUcscToRefSeqMapping)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "SequenceNames")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetWhole().SetLocal().SetStr("chr1");
    CRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);

    // Check that Map results meet expectations
    BOOST_CHECK_EQUAL(Result->GetId()->GetSeqIdString(true), "NC_000001.10");

    // Check for the unplaced UCSC naming convention
    OrigLoc->SetWhole().SetLocal().SetStr("chr8_gl000196_random");
    CGencollIdMapper::SIdSpec PrimarySpec;
    PrimarySpec.Primary = true;
    Result = Mapper.Map(*OrigLoc, PrimarySpec);
    BOOST_CHECK_EQUAL(Result->GetId()->GetGi(), GI_CONST(89028421));
}


BOOST_AUTO_TEST_CASE(TestCaseUcscToRefSeqToUcscMapping)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "SequenceNames")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetWhole().SetLocal().SetStr("chr1");

    CRef<CSeq_loc> Mapped = Mapper.Map(*OrigLoc, MapSpec);

    // Check that Map results meet expectations
    BOOST_CHECK_EQUAL(Mapped->GetId()->GetSeqIdString(true), "NC_000001.10");

    // Guess the original ID's spec
    CGencollIdMapper::SIdSpec GuessSpec;
    Mapper.Guess(*OrigLoc, GuessSpec);

    // Map back with the guessed spec
    CRef<CSeq_loc> RoundTripped = Mapper.Map(*Mapped, GuessSpec);

    // Check that Round tripped is equal to original
    BOOST_CHECK(RoundTripped->Equals(*OrigLoc));
}


BOOST_AUTO_TEST_CASE(TestCaseUcscUnTest_Scaffold)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000003205.2", "SequenceNames")
    );
    
    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Gi;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetWhole().SetLocal().SetStr("chrUn.004.10843");

    CRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);

    // Check that Map results meet expectations
    BOOST_CHECK_EQUAL(Result->GetId()->GetGi(), GI_CONST(119941349)); // NW_001500741.1
}


BOOST_AUTO_TEST_CASE(TestCaseUcscPseudoTest_Scaffold)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.12", "SequenceNames")
    );
    
    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;
    
    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetInt().SetId().SetLocal().SetStr("chr1_random");
    OrigLoc->SetInt().SetFrom(500000);
    OrigLoc->SetInt().SetTo(510000);

    CRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());

    CRef<CSeq_loc> Expected(new CSeq_loc());
    Expected->SetInt().SetId().Set("NT_113872.1");
    Expected->SetInt().SetFrom(57066);
    Expected->SetInt().SetTo(67066);
    Expected->SetInt().SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(*Expected));
    
    // chr6_random     65878   67001   chr6_random:67103       12330   + 
    OrigLoc.Reset(new CSeq_loc());
    OrigLoc->SetInt().SetId().SetLocal().SetStr("chr6_random");
    OrigLoc->SetInt().SetFrom(65878);
    OrigLoc->SetInt().SetTo(67000);
    OrigLoc->SetInt().SetStrand(eNa_strand_plus);
   
    CGencollIdMapper::SIdSpec GuessSpec;
    Mapper.Guess(*OrigLoc, GuessSpec);
    
    Result = Mapper.Map(*OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());
    
    Expected.Reset(new CSeq_loc());
    Expected->SetInt().SetId().Set("NT_113898.1");
    Expected->SetInt().SetFrom(65878);
    Expected->SetInt().SetTo(67000);
    Expected->SetInt().SetStrand(eNa_strand_plus);

    BOOST_CHECK(Result->Equals(*Expected));
    Result = Mapper.Map(*Result, GuessSpec);
    BOOST_CHECK(Result.NotNull());
    BOOST_CHECK(OrigLoc->Equals(*Result));

    // chr5_random   113060  114326  chr5:180363135  34903   + 
    OrigLoc.Reset(new CSeq_loc());
    OrigLoc->SetInt().SetId().SetLocal().SetStr("chr5_random");
    OrigLoc->SetInt().SetFrom(113060);
    OrigLoc->SetInt().SetTo(114325);
    OrigLoc->SetInt().SetStrand(eNa_strand_plus);
   
    Mapper.Guess(*OrigLoc, GuessSpec);

    Result = Mapper.Map(*OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());
    
    Expected.Reset(new CSeq_loc());
    Expected->SetInt().SetId().Set("NT_113890.1");
    Expected->SetInt().SetFrom(113060);
    Expected->SetInt().SetTo(114325);
    Expected->SetInt().SetStrand(eNa_strand_plus);

    BOOST_CHECK(Result->Equals(*Expected));
    Result = Mapper.Map(*Result, GuessSpec);
    BOOST_CHECK(Result.NotNull());
    BOOST_CHECK(OrigLoc->Equals(*Result));

}


BOOST_AUTO_TEST_CASE(TestCaseUcscPseudoTest_Comp)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.12", "SequenceNames" )
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;
    
    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetInt().SetId().SetLocal().SetStr("chr1_random");
    OrigLoc->SetInt().SetFrom(500000);
    OrigLoc->SetInt().SetTo(510000);

    CRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);

    CRef<CSeq_loc> Expected(new CSeq_loc());
    Expected->SetInt().SetId().Set("NT_113872.1");
    Expected->SetInt().SetFrom(57066);
    Expected->SetInt().SetTo(67066);
    Expected->SetInt().SetStrand(eNa_strand_plus);
    
    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(*Expected));
}


// map down  test
BOOST_AUTO_TEST_CASE(TestCaseDownMapTest)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "AllSequences")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_component;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().Set("NC_000001.10");
    OrigLoc.SetInt().SetFrom(50000000);
    OrigLoc.SetInt().SetTo(50000001);

    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);

    // Expected component level result
    CSeq_loc Expected;
    Expected.SetInt().SetId().Set("AL356789.16");
    Expected.SetInt().SetFrom(56981);
    Expected.SetInt().SetTo(56982);
    Expected.SetInt().SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(Expected));
}


// map down scaf  test
BOOST_AUTO_TEST_CASE(TestCaseDownScafMapTest)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "AllSequences")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_scaffold;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().Set("NC_000001.10");
    OrigLoc.SetInt().SetFrom(50000000);
    OrigLoc.SetInt().SetTo(50000001);

    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);

    // Expected component level result
    CSeq_loc Expected;
    Expected.SetInt().SetId().Set("GL000006.1");
    Expected.SetInt().SetFrom(19971918);
    Expected.SetInt().SetTo(19971919);
    Expected.SetInt().SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(Expected));
}

// upmap test
BOOST_AUTO_TEST_CASE(TestCaseUpMapTest_RefSeqAssm)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "AllSequences")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CSeq_loc OrigLoc;
    CSeq_interval& orig_ival = OrigLoc.SetInt();
    orig_ival.SetId().Set(CSeq_id::e_Local, "AL451051.6");
    orig_ival.SetFrom(5000);
    orig_ival.SetTo(5001);

    CConstRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);

    // Expected component level result
    CSeq_loc Expected;
    CSeq_interval& exp_ival = Expected.SetInt();
    exp_ival.SetId().Set("CM000663.1");
    exp_ival.SetFrom(100236283);
    exp_ival.SetTo(100236284);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(Expected));
}


// upmap test
BOOST_AUTO_TEST_CASE(TestCaseUpMapTest_GenBankAssm)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCA_000001405.1", "AllSequences")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CSeq_loc OrigLoc;
    CSeq_interval& orig_ival = OrigLoc.SetInt();
    orig_ival.SetId().Set(CSeq_id::e_Local, "AL451051.6");
    orig_ival.SetFrom(5000);
    orig_ival.SetTo(5001);

    CConstRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);

    // Expected component level result
    CSeq_loc Expected;
    CSeq_interval& exp_ival = Expected.SetInt();
    exp_ival.SetId().Set("CM000663.1");
    exp_ival.SetFrom(100236283);
    exp_ival.SetTo(100236284);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(Expected));
}


// upmap scaffold test
BOOST_AUTO_TEST_CASE(TestCaseUpMapScaffoldTest)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "AllSequences")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_scaffold;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().Set("AL451051.6");
    OrigLoc.SetInt().SetFrom(5000);
    OrigLoc.SetInt().SetTo(5001);

    CConstRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);

    // Expected component level result
    CSeq_loc Expected;
    Expected.SetInt().SetId().Set("GL000006.1");
    Expected.SetInt().SetFrom(70208201);
    Expected.SetInt().SetTo(70208202);
    Expected.SetInt().SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(Expected));
}


// pattern text
BOOST_AUTO_TEST_CASE(TestCasePatternMapping)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "SequenceNames")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetWhole().SetLocal().SetStr("1");
    CConstRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);
    // Check that Map results meet expectations
    BOOST_CHECK_EQUAL(Result->GetId()->GetSeqIdString(true), "NC_000001.10");


    OrigLoc->SetWhole().SetLocal().SetStr("C1");
    Result = Mapper.Map(*OrigLoc, MapSpec);
    BOOST_CHECK_EQUAL(Result->GetId()->GetSeqIdString(true), "NC_000001.10");


    OrigLoc->SetWhole().SetLocal().SetStr("LG1");
    Result = Mapper.Map(*OrigLoc, MapSpec);
    BOOST_CHECK_EQUAL(Result->GetId()->GetSeqIdString(true), "NC_000001.10");


    OrigLoc->SetWhole().SetLocal().SetStr("WAKKAWAKKA1");
    Result = Mapper.Map(*OrigLoc, MapSpec);
    BOOST_CHECK_EQUAL(Result->GetId()->GetSeqIdString(true), "NC_000001.10");

    OrigLoc->SetWhole().SetLocal().SetStr("chrUn_KN707606v1_decoy");
    Result = Mapper.Map(*OrigLoc, MapSpec);
    BOOST_CHECK_EQUAL(Result.IsNull(), true);
}


// Up/Down, Round Trip, Pattern test
BOOST_AUTO_TEST_CASE(TestCaseEverythingTest)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "SequenceNames")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_scaffold;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetInt().SetId().SetLocal().SetStr("LG2");
    OrigLoc->SetInt().SetFrom(123456789);
    OrigLoc->SetInt().SetTo(123456798);
    OrigLoc->SetInt().SetStrand(eNa_strand_plus);

    CRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);

    CRef<CSeq_loc> Expected(new CSeq_loc());
    Expected->SetInt().SetId().Set("NT_022135.16");
    Expected->SetInt().SetFrom(13205452);
    Expected->SetInt().SetTo(13205461);
    Expected->SetInt().SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(*Expected));

    CGencollIdMapper::SIdSpec GuessSpec;
    Mapper.Guess(*OrigLoc, GuessSpec);
    BOOST_CHECK_EQUAL(GuessSpec.ToString(), "NotPrim:Private:NotSet::LG%s:CHRO:TOP");

    CRef<CSeq_loc> RoundTrip = Mapper.Map(*Result, GuessSpec);

    // Check that Map results meet expectations
    BOOST_CHECK(RoundTrip->Equals(*OrigLoc));
}


// primary map test
BOOST_AUTO_TEST_CASE(TestCaseSpecPrimaryMap)
{
    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.Primary = true;
 
    // Simple ID
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetWhole().SetLocal().SetStr("1");
    

    // Map both RefSeq and Genbank assemblies with the same spec and id
    CGenomicCollectionsService GCService;
   
    {{
        CConstRef<CGC_Assembly> GenColl(
            GCService.GetAssembly("GCF_000001405.13", "SequenceNames" )
        );
       
        // Do a Map
        CGencollIdMapper Mapper(GenColl);
        CConstRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);
        // Check that Map results meet expectations
        BOOST_CHECK_EQUAL(Result->GetId()->GetGi(), GI_CONST(224589800));
    }}


    {{
        CConstRef<CGC_Assembly> GenColl(
            GCService.GetAssembly("GCA_000001405.1", "SequenceNames" )
        );

        // Do a Map
        CGencollIdMapper Mapper(GenColl);
        CConstRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, MapSpec);
        // Check that Map results meet expectations
        BOOST_CHECK_EQUAL(Result->GetId()->GetGi(), GI_CONST(224384768));
    }}

}


// primary guess test
BOOST_AUTO_TEST_CASE(TestCaseSpecPrimaryGuess)
{
    // Simple ID
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetWhole().SetGi(GI_CONST(224589800));
    

    // Guess both RefSeq and Genbank assemblies with the same spec and id
    CGenomicCollectionsService GCService;
   
    {{
        CConstRef<CGC_Assembly> GenColl(
            GCService.GetAssembly("GCF_000001405.13", "SequenceNames" )
        );
       
        // Do a Map
        CGencollIdMapper Mapper(GenColl);
        CGencollIdMapper::SIdSpec GuessSpec;
        bool Result = Mapper.Guess(*OrigLoc, GuessSpec);
        // Check that Guess results meet expectations
        BOOST_CHECK_EQUAL(Result, true);
        BOOST_CHECK_EQUAL(GuessSpec.ToString(), "Prim:RefSeq:Gi:::CHRO:TOP");
    }}


    {{
        CConstRef<CGC_Assembly> GenColl(
            GCService.GetAssembly("GCA_000001405.1", "SequenceNames" )
        );

        // Do a Map
        CGencollIdMapper Mapper(GenColl);
        CGencollIdMapper::SIdSpec GuessSpec;
        bool Result = Mapper.Guess(*OrigLoc, GuessSpec);
        // Check that Map results meet expectations
        BOOST_CHECK_EQUAL(Result, true);
        BOOST_CHECK_EQUAL(GuessSpec.ToString(), "NotPrim:RefSeq:Gi:::CHRO:TOP");
    }}

}

// GetSynonyms test
BOOST_AUTO_TEST_CASE(TestCaseGetSynonyms)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "SequenceNames" )
    );


    // Do a GetSynonyms
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_id> OrigId(new CSeq_id());
    OrigId->SetLocal().SetStr("chr1");

    list<CConstRef<CSeq_id> > Synonyms;
    
    
    // Check default NCBI-Only synonyms
    Mapper.GetSynonyms(*OrigId, Synonyms);
    BOOST_CHECK_EQUAL(Synonyms.size(), (size_t)5);


    // Check all synonyms
    Synonyms.clear();
    Mapper.GetSynonyms(*OrigId, Synonyms, false);
    BOOST_CHECK_EQUAL(Synonyms.size(), (size_t)7);
}


// dont actually upmap test
BOOST_AUTO_TEST_CASE(TestCaseUpMapTest_AltsDontUpmap)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.25", "AllSequences")
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CSeq_loc OrigLoc;
    CSeq_interval& orig_ival = OrigLoc.SetInt();
    orig_ival.SetId().Set("gi|224515577");
    orig_ival.SetFrom(3478538);
    orig_ival.SetTo(3478538);
    orig_ival.SetStrand(eNa_strand_plus);

    CConstRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);

    CSeq_loc ExpectLoc;
    CSeq_interval& expect_ival = ExpectLoc.SetInt();
    expect_ival.SetId().Set("NT_113891.2");
    expect_ival.SetFrom(3478538);
    expect_ival.SetTo(3478538);
    expect_ival.SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(ExpectLoc));
}

// guess top over scaffold
BOOST_AUTO_TEST_CASE(TestCaseUpMapTest_GuessTopOverScaffold)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> OldGenColl(
        GCService.GetAssembly("GCF_000001405.11", "AllSequences")
    );
 
    CConstRef<CGC_Assembly> NewGenColl(
        GCService.GetAssembly("GCF_000001405.25", "AllSequences")
    );


    CGencollIdMapper OldMapper(OldGenColl);
    CGencollIdMapper NewMapper(NewGenColl);
    
    
    CSeq_loc OrigLoc;
    CSeq_interval& orig_ival = OrigLoc.SetInt();
    orig_ival.SetId().Set("NT_078074.1");
    orig_ival.SetFrom(95619);
    orig_ival.SetTo(146371);
    orig_ival.SetStrand(eNa_strand_plus);
    
    CGencollIdMapper::SIdSpec GuessSpec;
    OldMapper.Guess(OrigLoc, GuessSpec);

    CSeq_loc RemapLoc;
    CSeq_interval& remap_ival = RemapLoc.SetInt();
    remap_ival.SetId().Set("gi|224589821");
    remap_ival.SetFrom(40301888);
    remap_ival.SetTo(40507252);
    remap_ival.SetStrand(eNa_strand_plus);
    
    // Do a Map
    
    CConstRef<CSeq_loc> Result = NewMapper.Map(RemapLoc, GuessSpec);


    CSeq_loc ExpectLoc;
    CSeq_interval& expect_ival = ExpectLoc.SetInt();
    expect_ival.SetId().Set("NC_000009.11");
    expect_ival.SetFrom(40301888);
    expect_ival.SetTo(40507252);
    expect_ival.SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(ExpectLoc));
}



// Don't match partial chromosome names
BOOST_AUTO_TEST_CASE(TestCasePartialChromosomeTest)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.13", "SequenceNames" )
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_component;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().SetLocal().SetStr("23-499");
    OrigLoc.SetInt().SetFrom(50000000);
    OrigLoc.SetInt().SetTo(50000001);
    
    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.IsNull());

    OrigLoc.SetInt().SetId().SetLocal().SetStr("333");
    Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.IsNull());

    OrigLoc.SetInt().SetId().SetLocal().SetStr("425");
    Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.IsNull());

}



// Bare MT error test
BOOST_AUTO_TEST_CASE(TestCaseBareMTErrorTest)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_001858045.2", "Scaffolds") //"SequenceNames" )
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    //MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    //MapSpec.Role = eGC_SequenceRole_component;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    
    CSeq_loc OrigLoc;
    OrigLoc.SetWhole().SetGenbank().SetAccession("MT");
   
    // do either throw or return null
    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.IsNull());
}


BOOST_AUTO_TEST_CASE(TestCaseBareMTNotErrorTest)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_001858045.2", "Scaffolds") //"SequenceNames" )
    );

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    //MapSpec.Role = eGC_SequenceRole_component;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    
    CSeq_loc OrigLoc;
    OrigLoc.SetWhole().SetGenbank().SetAccession("MT");
   
 
    // this time work 
    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());
    
    CSeq_loc ExpectLoc;
    ExpectLoc.SetWhole().SetGenbank().SetAccession("GU238433");
    ExpectLoc.SetWhole().SetGenbank().SetVersion(1);
    BOOST_CHECK(Result->Equals(ExpectLoc));

}





// Fix-up PDB Seq-ids
BOOST_AUTO_TEST_CASE(TestCase_PDBSeqIdFix)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001215.2", "SequenceNames"));
                              //CGCClient_GetAssemblyRequest::eLevel_scaffold));

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    
    // This PDB Seq-id is a mis-read of "local.str = 2LHet"
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().SetPdb().SetMol().Set("2LHe");
    OrigLoc.SetInt().SetId().SetPdb().SetChain(116);
    OrigLoc.SetInt().SetFrom(1322833);
    OrigLoc.SetInt().SetTo(1322945);
    
    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());
    BOOST_CHECK_EQUAL(Result->GetInt().GetId().GetSeqIdString(true), "NW_001848855.1"); 
}



// Fix-up GI Seq-ids that should be strings
BOOST_AUTO_TEST_CASE(TestCase_GINumberString)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000307585.1", "SequenceNames"));
    BOOST_CHECK(GenColl.NotNull());
    
    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    
    // This PDB Seq-id is a mis-read of "local.str = 2LHet"
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().SetGi(GI_CONST(397912605));
    OrigLoc.SetInt().SetFrom(1);
    OrigLoc.SetInt().SetTo(41937);
    
    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());
    BOOST_CHECK_EQUAL(Result->GetInt().GetId().GetSeqIdString(true), "NC_018264.1"); 
}


BOOST_AUTO_TEST_CASE(TestCaseUcscToRefSeqMapping_ForSlowCat)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000181335.2", "SequenceNames")
    );
    // Make a Spec
    CGencollIdMapper::SIdSpec NameMapSpec;
    NameMapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    NameMapSpec.Alias = CGC_SeqIdAlias::e_Public;
    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetInt().SetId().SetLocal().SetStr("chrA1");
    OrigLoc->SetInt().SetFrom(60268361);
    OrigLoc->SetInt().SetTo(60268361);
    CRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, NameMapSpec);

    // Check that Map results meet expectations
    BOOST_CHECK_EQUAL(Result->GetId()->GetSeqIdString(true), "NC_018723.2");
}


BOOST_AUTO_TEST_CASE(TestCaseUcscToRefSeqMapping_ForSlowCat_Down)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000181335.2", "SequenceNames")
    );
 
    CGencollIdMapper::SIdSpec DownMapSpec;
    DownMapSpec.TypedChoice = CGC_TypedSeqId::e_Genbank;
    DownMapSpec.Alias = CGC_SeqIdAlias::e_Public;
    DownMapSpec.Role = eGC_SequenceRole_scaffold;
    
    CGencollIdMapper Mapper(GenColl);
    CRef<CSeq_loc> OrigLoc(new CSeq_loc());
    OrigLoc->SetInt().SetId().SetLocal().SetStr("chrA1");
    OrigLoc->SetInt().SetFrom(60268361);
    OrigLoc->SetInt().SetTo(60268361);
 
    // Do a DownMap
    CRef<CSeq_loc> Result = Mapper.Map(*OrigLoc, DownMapSpec);
 //cerr << MSerial_AsnText << *Result << endl;
    // Expected component level result
    CSeq_loc Expected;
    Expected.SetPnt().SetId().Set("KN300524.1");
    Expected.SetPnt().SetPoint(698862);
    Expected.SetPnt().SetStrand(eNa_strand_plus);

    // Check that Map results meet expectations
    BOOST_CHECK(Result->Equals(Expected));
}


// Fix-up Bacteria chromosome names
BOOST_AUTO_TEST_CASE(TestCase_BacteriaChromosomeNames)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000069185.1", "SequenceNames"));
                              //CGCClient_GetAssemblyRequest::eLevel_scaffold));

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    
    // This users expect "1" to work, even though "1" is not in replicon.name
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().SetLocal().SetStr("1");
    OrigLoc.SetInt().SetFrom(1);
    OrigLoc.SetInt().SetTo(2);
    
    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());
    BOOST_CHECK_EQUAL(Result->GetInt().GetId().GetSeqIdString(true), "NC_010397.1"); 
    
    OrigLoc.SetInt().SetId().SetLocal().SetId(1);
    Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.NotNull());
    BOOST_CHECK_EQUAL(Result->GetInt().GetId().GetSeqIdString(true), "NC_010397.1"); 

    // for the record, 'local str ""' is not valid
}


// Fix-up Bacteria chromosome names
BOOST_AUTO_TEST_CASE(TestCase_CXX_11251)
{
    // Fetch Gencoll
    CGenomicCollectionsService GCService;
    CConstRef<CGC_Assembly> GenColl(
        GCService.GetAssembly("GCF_000001405.39", "SequenceNames"));
                              //CGCClient_GetAssemblyRequest::eLevel_scaffold));

    // Make a Spec
    CGencollIdMapper::SIdSpec MapSpec;
    MapSpec.TypedChoice = CGC_TypedSeqId::e_Refseq;
    MapSpec.Alias = CGC_SeqIdAlias::e_Public;
    MapSpec.Role = eGC_SequenceRole_top_level;

    // Do a Map
    CGencollIdMapper Mapper(GenColl);
    
    // 
    CSeq_loc OrigLoc;
    OrigLoc.SetInt().SetId().SetLocal().SetStr("chrUn_KI270752v1");
    OrigLoc.SetInt().SetFrom(1);
    OrigLoc.SetInt().SetTo(2);
    
    CRef<CSeq_loc> Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.IsNull());
    if(Result.NotNull())
        BOOST_CHECK_EQUAL(Result->GetInt().GetId().GetSeqIdString(true), "KI270752.1"); 
    
    // 
    OrigLoc.SetInt().SetId().SetLocal().SetStr("Contig_2_5.15315");
    OrigLoc.SetInt().SetFrom(1);
    OrigLoc.SetInt().SetTo(2);
    
    Result = Mapper.Map(OrigLoc, MapSpec);
    BOOST_CHECK(Result.IsNull());
    if(Result.NotNull())    
        BOOST_CHECK_EQUAL(Result->GetInt().GetId().GetSeqIdString(true), "NC_000002.12"); 
    

}

BOOST_AUTO_TEST_SUITE_END();
