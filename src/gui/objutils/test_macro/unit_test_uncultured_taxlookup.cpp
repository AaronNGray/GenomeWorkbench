/*  $Id: unit_test_uncultured_taxlookup.cpp 41896 2018-11-07 16:08:25Z asztalos $
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
*   Unit tests for macro utility functions
*/

#include <ncbi_pch.hpp>

#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/OrgName.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_lookup.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);
USING_SCOPE(macro);

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

static const string uncultured_taxnames[] = {
    "bacteria",
    "bacterium",
    "uncultured bacterium",
    "Uncultured Actinobacteria",
    "Uncultured Actinobacteridae",
    "Uncultured Actinomycetales",
    "Uncultured Anaerolineaceae",
    "Uncultured Chloroflexi",
    "Uncultured Clostridium",
    "Uncultured Deltaproteobacteria",
    "Uncultured Desulfomonile",
    "Uncultured Firmicutes",
    "Uncultured Gemmatimonadetes",
    "Uncultured Hyphomicrobiaceae",
    "Uncultured Hyphomicrobium",
    "Uncultured Legionella",
    "Uncultured Marinobacterium",
    "Uncultured Methylocystaceae",
    "Uncultured Methylophilaceae",
    "Uncultured Nitrospiraceae",
    "Uncultured Nitrospirae",
    "Uncultured Piscirickettsiaceae",
    "Uncultured Porphyrobacter",
    "Uncultured Rhizobiaceae",
    "Uncultured Rhodobacter",
    "Uncultured Rhodobacteraceae",
    "Uncultured Rhodobiaceae",
    "Uncultured Rhodocyclaceae",
    "Uncultured Rubrobacteraceae",
    "Uncultured Sphingobacteriia",
    "Uncultured Thiohalophilus",
    "uncultured Acidobacteria bacterium",
    "uncultured Mycobacterium sp.",
    "uncultured Pseudomonas sp.",
    "uncultured Roseobacter sp.",
    "uncultured alpha proteobacterium",
    "uncultured bacterium",
    "uncultured beta proteobacterium",
    "uncultured gamma proteobacterium",
    "Uncultured Parachlamydia acanthamoebae",
    "Uncultured Uncultured Trichococcus sp.",
    "Uncultured Zymophilus paucivorans",
    "uncultured Anaerovorax sp.",
    "uncultured Clostridium sp.",
    "uncultured Microbacterium sp.",
    "uncultured Peptostreptococcaceae bacterium",
    "uncultured Prosthecobacter sp.",
    "uncultured Verrucomicrobia bacterium",
    "Methanobacterium formicicum phage",
    "Myrtapenidiella tenuiramis",
    "Parateratosphaeria bellula"
};

static const string uncultured_suggestions[] = {
    "uncultured bacterium",
    "uncultured bacterium",
    "uncultured bacterium",
    "uncultured actinobacterium",
    "uncultured actinobacterium",
    "uncultured Actinomycetales bacterium",
    "uncultured Anaerolineaceae bacterium",
    "uncultured Chloroflexi",
    "uncultured Clostridium sp.",
    "uncultured delta proteobacterium",
    "uncultured Desulfomonile sp.",
    "uncultured Firmicutes bacterium",
    "uncultured Gemmatimonadetes",
    "uncultured Hyphomicrobiaceae bacterium",
    "uncultured Hyphomicrobium sp.",
    "uncultured Legionella sp.",
    "uncultured Marinobacterium sp.",
    "uncultured Methylocystaceae bacterium",
    "uncultured Methylophilaceae bacterium",
    "uncultured Nitrospiraceae bacterium",
    "uncultured Nitrospirae bacterium",
    "Uncultured Piscirickettsiaceae",
    "uncultured Porphyrobacter sp.",
    "uncultured Rhizobiaceae bacterium",
    "uncultured Rhodobacter sp.",
    "uncultured Rhodobacteraceae bacterium",
    "uncultured Rhodobiaceae bacterium",
    "uncultured Rhodocyclaceae bacterium",
    "uncultured Rubrobacteraceae bacterium",
    "uncultured Sphingobacteriia bacterium",
    "uncultured Thiohalophilus sp.",
    "uncultured Acidobacteria bacterium",
    "uncultured Mycobacterium sp.",
    "uncultured Pseudomonas sp.",
    "uncultured Roseobacter sp.",
    "uncultured Alphaproteobacteria bacterium",
    "uncultured bacterium",
    "uncultured beta proteobacterium",
    "uncultured gamma",
    "uncultured Parachlamydia sp.",
    "uncultured Trichococcus sp.",
    "uncultured Propionispira sp.",
    "uncultured Anaerovorax sp.",
    "uncultured Clostridium sp.",
    "uncultured Microbacterium sp.",
    "uncultured Peptostreptococcaceae bacterium",
    "uncultured Prosthecobacter sp.",
    "uncultured Verrucomicrobia bacterium",
    "uncultured Methanobacterium formicicum phage",
    "uncultured Myrtapenidiella",
    "uncultured Parateratosphaeria"
};

template<class T>
string MakeAsn(const T& object)
{
    CNcbiOstrstream str;
    str << MSerial_AsnText << object;
    return CNcbiOstrstreamToString(str);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_TaxnameCorrections)
{
    CRef<CBioSource> bsrc(new CBioSource());
    CUnculturedTool uncultured_tool;
    string correction;
    for (size_t index = 0; index < sizeof(uncultured_taxnames) / sizeof(string); ++index) {
        bsrc->SetOrg().SetTaxname(uncultured_taxnames[index]);
        uncultured_tool.SetBioSource(*bsrc);

        correction = uncultured_tool.GetSuggestedCorrection();
        BOOST_CHECK_EQUAL(uncultured_suggestions[index], correction);
    }

    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    for (size_t index = 0; index < sizeof(uncultured_taxnames) / sizeof(string); ++index) {
        bsrc->SetOrg().SetTaxname(uncultured_taxnames[index]);

        uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);
        BOOST_CHECK_EQUAL(uncultured_suggestions[index], bsrc->GetTaxname());
    }
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_Taxon1)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("uncultured type I methanotroph");
    bsrc->SetOrg().SetTaxId(192503);
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (using universal primers)]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // taxname should not change, and the universal primer note should be removed
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), "uncultured type I methanotroph");
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(removed_note_univ);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_Taxon2)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("uncultured bacterium");
    bsrc->SetOrg().SetTaxId(77133);
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[BankIt_uncultured16S_wizard]; [universal primers]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // taxname should not change, and the universal primer note should be removed
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), "uncultured bacterium");
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(removed_note_univ);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_Taxon3)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Clonothrix fusca");
    bsrc->SetOrg().SetTaxId(415611);
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[BankIt_uncultured16S_wizard]; [universal primers]"))); 

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // taxname should change, and the universal primer note should be removed
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), "uncultured Clonothrix");
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(removed_note_univ);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_Taxon4)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Clonothrix fusca");
    bsrc->SetOrg().SetTaxId(415611);
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (with species-specific primers)]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // taxname should not change, and the uncultured and species specific primer note should be converted
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), "Clonothrix fusca");
    BOOST_CHECK(converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_SpeciesSpecific1)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Pandoraea pnomenusa");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (with species-specific primers)]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    BOOST_CHECK(converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  org {\n"
                "    taxname \"Pandoraea pnomenusa\"\n"
                "  },\n"
                "  subtype {\n"
                "    {\n"
                "      subtype other,\n"
                "      name \"amplified with species-specific primers\"\n"
                "    },\n"
                "    {\n"
                "      subtype environmental-sample,\n"
                "      name \"\"\n"
                "    }\n"
                "  }\n"
                "}\n");

}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_SpeciesSpecific2)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Pandoraea pnomenusa sp.");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (with species-specific primers)]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    BOOST_CHECK(converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  org {\n"
                "    taxname \"Pandoraea pnomenusa\"\n"
                "  },\n"
                "  subtype {\n"
                "    {\n"
                "      subtype other,\n"
                "      name \"amplified with species-specific primers\"\n"
                "    },\n"
                "    {\n"
                "      subtype environmental-sample,\n"
                "      name \"\"\n"
                "    }\n"
                "  }\n"
                "}\n");

}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_SpeciesSpecific3)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Anaplasma phagocytophilum");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_environmental_sample, kEmptyStr)));
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "amplified with species-specific primers")));
    
    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // taxname would not change, and the note should not change either
    BOOST_CHECK(converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  org {\n"
                "    taxname \"Anaplasma phagocytophilum\"\n"
                "  },\n"
                "  subtype {\n"
                "    {\n"
                "      subtype environmental-sample,\n"
                "      name \"\"\n"
                "    },\n"
                "    {\n"
                "      subtype other,\n"
                "      name \"amplified with species-specific primers\"\n"
                "    }\n"
                "  }\n"
                "}\n");
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_SpeciesSpecific4)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Bacillus sp.");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "amplified with species-specific primers")));

    // note is removed and taxname changed
    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    BOOST_CHECK(!converted_note);
    BOOST_CHECK(removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Bacillus sp."));
    BOOST_CHECK(!bsrc->IsSetSubtype());
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_SpeciesSpecific_Misspelled)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Eschericia coli");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (using species-specific primers) bacterial source]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // taxname would change and the subsource note would be converted
    BOOST_CHECK(converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  org {\n"
                "    taxname \"Escherichia coli\"\n"
                "  },\n"
                "  subtype {\n"
                "    {\n"
                "      subtype other,\n"
                "      name \"amplified with species-specific primers\"\n"
                "    },\n"
                "    {\n"
                "      subtype environmental-sample,\n"
                "      name \"\"\n"
                "    }\n"
                "  }\n"
                "}\n");
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_NoteInPlace)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Anaplasma phagocytophilum");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "some note")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // taxname does not change, and the note is not modified
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  org {\n"
                "    taxname \"uncultured Anaplasma sp.\"\n"
                "  },\n"
                "  subtype {\n"
                "    {\n"
                "      subtype other,\n"
                "      name \"some note\"\n"
                "    }\n"
                "  }\n"
                "}\n");
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_NotSpeciesSpecific1)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Anaplasma sp.");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (using species-specific primers) bacterial source]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // change taxname and remove species specific note
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Anaplasma sp."));
    // this step is performed by the TaxLookup
    //BOOST_CHECK(NStr::FindNoCase(bsrc->GetLineage(), "environmental samples") != NPOS);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_NotSpeciesSpecific2)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("uncultured Chlamydiales");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[BankIt_uncultured16S_wizard]; [species_specific primers]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // change taxname and remove species specific note
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Chlamydiales bacterium"));
    // this step is performed by the TaxLookup
    //BOOST_CHECK(NStr::FindNoCase(bsrc->GetLineage(), "environmental samples") != NPOS);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_NotSpeciesSpecific3)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("uncultured Enterococcus sp.");
    bsrc->SetOrg().SetTaxId(167972);
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[BankIt_uncultured16S_wizard]; [species_specific primers]; [dgge]")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // do not change taxname but remove the species specific note
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Enterococcus sp."));
    // this step is performed by the TaxLookup
    //BOOST_CHECK(NStr::FindNoCase(bsrc->GetLineage(), "environmental samples") != NPOS);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_OrgModQualifiers)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Campylobacter jejuni");
    bsrc->SetOrg().SetOrgname().SetMod().push_back(Ref(new COrgMod(COrgMod::eSubtype_isolate, "IQDC-1")));
    bsrc->SetOrg().SetOrgname().SetMod().push_back(Ref(new COrgMod(COrgMod::eSubtype_nat_host, "Domestic chicken")));

    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    // change taxname and do not remove the orgmod qualifiers
    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Campylobacter sp."));
    BOOST_CHECK(bsrc->IsSetOrgMod());
    BOOST_CHECK(bsrc->GetOrgname().GetMod().size() == 2);
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_UniversalPrimers1)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Actinomadura madurae");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (using universal primers)]")));

    // change taxname and remove universal primers note
    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Actinomadura sp."));
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_UniversalPrimers2)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("Bacillus sp.");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (using universal primers)]")));

    // change taxname and remove universal primers note
    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Bacillus sp."));
}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_NoTaxId1)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("uncultured somename");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[amplified with species-specific primers]; [uncultured; wizard]")));

    // do not remove note as taxname does not look up, taxname should not change
    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured somename"));
    BOOST_CHECK(bsrc->IsSetSubtype());
    BOOST_CHECK_EQUAL(bsrc->GetSubtype().front()->GetSubtype(), CSubSource::eSubtype_other);

}

BOOST_AUTO_TEST_CASE(Test_UncultTaxTool_NoTaxId2)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("uncultured Somename");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[uncultured (using universal primers)]")));

    // do not remove note as taxname does not look up, taxname should not change
    CUnculturedTool uncultured_tool;
    bool converted_note(false), removed_note_sp(false), removed_note_univ(false);
    uncultured_tool.CorrectBioSource(*bsrc, converted_note, removed_note_sp, removed_note_univ);

    BOOST_CHECK(!converted_note);
    BOOST_CHECK(!removed_note_sp);
    BOOST_CHECK(!removed_note_univ);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("uncultured Somename"));
    BOOST_CHECK(bsrc->IsSetSubtype());
    BOOST_CHECK_EQUAL(bsrc->GetSubtype().front()->GetSubtype(), CSubSource::eSubtype_other);
}


BOOST_AUTO_TEST_CASE(Test_CulturedTaxTool_TaxnameCorrections)
{
    CRef<CBioSource> bsrc(new CBioSource());
    bsrc->SetOrg().SetTaxname("novel Bacillus");
    bsrc->SetSubtype().push_back(Ref(new CSubSource(CSubSource::eSubtype_other,
        "[cultured bacterial source]")));

    CCulturedTool cultured_tool;
    bool removed_note(false);
    cultured_tool.CorrectBioSource(*bsrc, removed_note);

    BOOST_CHECK(removed_note);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("Bacillus"));

    ///////////////////////////////////////////////////////////////////////

    bsrc->SetOrg().SetTaxname("Legionella_species");
    bsrc->SetOrg().SetOrgname().SetMod().push_back(Ref(new COrgMod(COrgMod::eSubtype_other,
        "[cultured bacterial source]")));

    removed_note = false;
    cultured_tool.CorrectBioSource(*bsrc, removed_note);

    BOOST_CHECK(removed_note);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("Legionella sp."));

    ///////////////////////////////////////////////////////////////////////

    bsrc->SetOrg().SetTaxname("unclassified Piscirickettsiaceae");
    bsrc->SetOrg().SetOrgname().SetMod().push_back(Ref(new COrgMod(COrgMod::eSubtype_other,
        "[BankIt_cultured16S_wizard]; [species_specific primers]; [tgge]")));

    removed_note = false;
    cultured_tool.CorrectBioSource(*bsrc, removed_note);

    BOOST_CHECK(removed_note);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("Piscirickettsiaceae"));

    ///////////////////////////////////////////////////////////////////////

    bsrc->SetOrg().SetTaxname("Azotobactor Sp.");

    removed_note = false;
    cultured_tool.CorrectBioSource(*bsrc, removed_note);

    BOOST_CHECK(!removed_note);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("Azotobactor sp."));

    ///////////////////////////////////////////////////////////////////////

    bsrc->SetOrg().SetTaxname("Deltaproteobacteria spp");
    
    removed_note = false;
    cultured_tool.CorrectBioSource(*bsrc, removed_note);

    BOOST_CHECK(!removed_note);
    BOOST_CHECK_EQUAL(bsrc->GetTaxname(), string("Deltaproteobacteria sp."));
}

