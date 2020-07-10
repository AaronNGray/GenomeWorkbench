/*  $Id: unit_test_pkg_seqedit.cpp 43678 2019-08-14 15:03:47Z asztalos $
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
* Author:  Andrea Asztalos, NCBI
*
* File Description:
*    Unit tests for functions in the editing package
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiapp.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seqtable/SeqTable_column.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/packages/pkg_sequence_edit/strain_serotype_influenza.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/add_deflinedlg.hpp>
#include <gui/packages/pkg_sequence_edit/prefix_deflinedlg.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>


USING_NCBI_SCOPE;

extern const char* sc_TestEntry_multiplestrains;
extern const char* sc_TestEntry_mtplstrains_empty;
extern const char* sc_TestEntry_addstrainserotype;
extern const char* sc_TestEntry_fixorganismnames;
extern const char* sc_TestEntry_noparsing;
extern const char* sc_TestEntry_parsingnoconflicts;
extern const char* sc_TestEntry_parsingwithconflicts;
extern const char* sc_TestEntry_splitstructcollections;
extern const char* sc_TestEntry_trimorgnames;
extern const char* sc_TestEntry_addfeat;
extern const char* sc_TestEntry_adddefline;
extern const char* sc_TestEntry_removedeflines;
extern const char* sc_TestEntry_prefixtodeflines;

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

void ReadEntry(const string& name, CSeq_entry_Handle& seh)
{
    CRef<CSeq_entry> entry(new CSeq_entry);
    CNcbiIstrstream istr(name.c_str());
    istr >> MSerial_AsnText >> entry;

    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    seh = scope->AddTopLevelSeqEntry(*entry);
}

BOOST_AUTO_TEST_CASE(Test_MultipleStrains)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_multiplestrains, seh);
    BOOST_REQUIRE(seh);

    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandSplitQualsAtCommas);
    CRef<CCmdComposite> composite = worker.GetCommand(seh, "Testing splitting multiple strains");
    BOOST_REQUIRE(composite);
    composite->Execute();
    
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    // test the changes
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            vector<string> strains;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource()){
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_strain){
                    strains.push_back((*orgmod)->GetSubname());
                }
            }
            BOOST_REQUIRE(strains.size() == 4);
            BOOST_CHECK_EQUAL(strains[0], string("DSM 7876"));
            BOOST_CHECK_EQUAL(strains[1], string("ATCC VR-111"));
            BOOST_CHECK_EQUAL(strains[2], string("strain3"));
            BOOST_CHECK_EQUAL(strains[3], string("C768L/7"));    
            strains.clear();
        }    
    }        
}


BOOST_AUTO_TEST_CASE(Test_MultipleStrainsEmpty)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_mtplstrains_empty, seh);
    BOOST_REQUIRE(seh);
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandSplitQualsAtCommas);
    CRef<CCmdComposite> composite = worker.GetCommand(seh, "Testing splitting multiple strains when empty");
    BOOST_REQUIRE(composite);
    composite->Execute();
    
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            vector<string> strains;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource()){
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_strain){
                    strains.push_back((*orgmod)->GetSubname());
                }
            }
            BOOST_REQUIRE(!strains.empty());
            BOOST_CHECK_EQUAL(strains[0], string("DSM 7876"));
            BOOST_CHECK_EQUAL(strains.size(), 1);
            strains.clear();
        }
    }
}

BOOST_AUTO_TEST_CASE(Test_AddStrainSerotype)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_addstrainserotype, seh);
    BOOST_REQUIRE(seh);
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_add);
    CRef<CCmdComposite> composite = worker.GetCommand(seh, "Adding strain serotype to Influenza (A) names");
    BOOST_REQUIRE(composite);
    composite->Execute();
    
    
    char seqs='0';
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            string  taxname = (*sd).GetSource().GetTaxname();
            
            bool is_oldname = false;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource())
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_old_name)
                    is_oldname = true;
            bool is_dbtaxon = false;
            FOR_EACH_DBXREF_ON_ORGREF (db, (*sd).GetSource().GetOrg())
                if ((*db)->IsSetDb() && NStr::Equal((*db)->GetDb(), "taxon", NStr::eNocase))
                    is_dbtaxon = true;
            
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus (X-33(H5N9))")); 
                    BOOST_CHECK_EQUAL (is_oldname, false);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus (X-33(H7N8))")); 
                    BOOST_CHECK_EQUAL (is_oldname, false);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus (Y546)")); 
                    BOOST_CHECK_EQUAL (is_oldname, true);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;    
                case ('3'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus")); 
                    BOOST_CHECK_EQUAL (is_oldname, true);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case ('4'):    
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus")); 
                    BOOST_CHECK_EQUAL (is_oldname, true);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
            }
        seqs++;
        }
    }
}

BOOST_AUTO_TEST_CASE(Test_FixInfluenzaNames)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_fixorganismnames, seh);
    BOOST_REQUIRE(seh);
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_fixup);
    CRef<CCmdComposite> composite = worker.GetCommand(seh, "Fix Influenza (A) names");
    BOOST_REQUIRE(composite);
    composite->Execute();

    // checking
    char seqs='0';
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            string  taxname = (*sd).GetSource().GetTaxname();
                        
            bool is_oldname = false;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource())
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_old_name)
                    is_oldname = true;
            bool is_dbtaxon = false;
            FOR_EACH_DBXREF_ON_ORGREF (db, (*sd).GetSource().GetOrg())
                if ((*db)->IsSetDb() && NStr::Equal((*db)->GetDb(), "taxon", NStr::eNocase))
                    is_dbtaxon = true;
            
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus (DSM 7866(H2N8))"));
                    BOOST_CHECK_EQUAL (is_oldname, true);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus (DSM 7866(H2N8))"));
                    BOOST_CHECK_EQUAL (is_oldname, false);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus (DSM 7866)"));
                    BOOST_CHECK_EQUAL (is_oldname, false);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case ('3'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza A virus (DSM 7866(H2N8))"));
                    BOOST_CHECK_EQUAL (is_oldname, false);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case ('4'):
                    BOOST_CHECK_EQUAL (taxname, string("Influenza B virus (DSM 7866(H2N8))"));
                    BOOST_CHECK_EQUAL (is_oldname, true);
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break; 
            }
        seqs++;
        }
    
    }
}

BOOST_AUTO_TEST_CASE(Test_ParsingNoConflicts)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_parsingnoconflicts, seh);
    BOOST_REQUIRE(seh);
   
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_parse);
    CRef<CCmdComposite> composite = worker.GetCommand(seh, "Parse Influenza names without conflicts");
    BOOST_REQUIRE(composite);
    composite->Execute();
    
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            int count_strains=0;
            int count_serotypes=0;
            string strain, serotype;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource()){
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_strain){
                    count_strains++; strain = (*orgmod)->GetSubname(); 
                }
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_serotype){    
                    count_serotypes++; serotype = (*orgmod)->GetSubname(); 
                }
            }    
            
            BOOST_CHECK_EQUAL (count_strains, 1);
            BOOST_CHECK_EQUAL (count_serotypes, 1);
            BOOST_CHECK_EQUAL (strain, string("DSM 7866"));
            BOOST_CHECK_EQUAL (serotype, string("H2N8"));
        }
    }    
}


BOOST_AUTO_TEST_CASE(Test_ParsingWithConflictsUpdate)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_parsingwithconflicts, seh);
    BOOST_REQUIRE(seh);
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_parse);
    CRef<CCmdComposite> composite = TestCStrainSerotypeInfluenza_Parse(worker, seh, "Parse Influenza names with conflicts", true);
    BOOST_REQUIRE(composite);
    composite->Execute();
    
    char seqs='0';
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            int count_strains=0;
            int count_serotypes=0;
            string strain, serotype;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource()){
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_strain){
                    count_strains++; strain = (*orgmod)->GetSubname(); 
                }
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_serotype){    
                    count_serotypes++; serotype = (*orgmod)->GetSubname(); 
                }
            }
            
            BOOST_CHECK_EQUAL (count_strains, 1);
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL (count_serotypes, 1);
                    BOOST_CHECK_EQUAL (strain, string("X-33")); 
                    BOOST_CHECK_EQUAL (serotype, string("sero/H5N9"));    
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL (count_serotypes, 0);
                    BOOST_CHECK_EQUAL (strain, string("X-33"));
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL (count_serotypes, 1);
                    BOOST_CHECK_EQUAL (strain, string("X-33"));
                    BOOST_CHECK_EQUAL (serotype, string("H2N8"));
                    break;
            }
            seqs++;    
        }
    }
}

BOOST_AUTO_TEST_CASE(Test_ParsingWithConflictsNoUpdate)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_parsingwithconflicts, seh);
    BOOST_REQUIRE(seh);
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_parse);
    CRef<CCmdComposite> composite = TestCStrainSerotypeInfluenza_Parse(worker, seh, "Parse Influenza names with conflicts", false);
    BOOST_REQUIRE(composite);
    composite->Execute();
    
    
//    test_worker.DoParsingOnSeqEntry (seh, composite, false); // not overwriting existing strains and serotypes
    
    char seqs='0';
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            string  taxname = (*sd).GetSource().GetTaxname();
            
            int count_strains=0;
            int count_serotypes=0;
            string strain, serotype;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource()){
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_strain){
                    count_strains++; strain = (*orgmod)->GetSubname(); 
                }
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_serotype){    
                    count_serotypes++; serotype = (*orgmod)->GetSubname(); 
                }
            }
            
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL (count_strains, 1);
                    BOOST_CHECK_EQUAL (count_serotypes, 1);
                    BOOST_CHECK_EQUAL (strain, string("DSM 7866"));
                    BOOST_CHECK_EQUAL (serotype, string("H2N8")); 
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL (count_strains, 1);
                    BOOST_CHECK_EQUAL (count_serotypes, 0);
                    BOOST_CHECK_EQUAL (strain, string("DSM_strain"));
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL (count_strains, 1);
                    BOOST_CHECK_EQUAL (count_serotypes, 1);
                    BOOST_CHECK_EQUAL (strain, string("X-33"));
                    BOOST_CHECK_EQUAL (serotype, string("H2N8"));
                    break;
            }
        seqs++;    
        }        
    }        

}

BOOST_AUTO_TEST_CASE(Test_SplittingStructuredCollections)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_splitstructcollections, seh);
    BOOST_REQUIRE(seh);
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandSplitStructuredCollections);
    CRef<CCmdComposite> composite = worker.GetCommand(seh, "Testing splitting structured collections");
    BOOST_REQUIRE(composite);
    composite->Execute();
    
    // test the changes
    char seqs='0';
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            vector<string> bio_str;
            vector<string> ccol_str;
            vector<string> spec_str;
            FOR_EACH_ORGMOD_ON_BIOSOURCE (orgmod, (*sd).GetSource()){
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_bio_material)
                    bio_str.push_back((*orgmod)->GetSubname());
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_culture_collection)
                    ccol_str.push_back((*orgmod)->GetSubname());
                if ((*orgmod)->IsSetSubname() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_specimen_voucher)
                    spec_str.push_back((*orgmod)->GetSubname());    
            }
            
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL(bio_str[0], string("biom1:GH7899"));
                    BOOST_CHECK_EQUAL(bio_str[1], string("biom2/458966, Birmingham, England"));
                    BOOST_CHECK_EQUAL(bio_str[2], string("biom3:98jkk"));
                    BOOST_CHECK_EQUAL(ccol_str[0], string("ccol1:GH7899"));
                    BOOST_CHECK_EQUAL(ccol_str[1], string("ccol2/458966, Birmingham, England"));
                    BOOST_CHECK_EQUAL(ccol_str[2], string("ccol3:98jkk"));
                    BOOST_CHECK_EQUAL(ccol_str[3], string("new_ccol"));
                    BOOST_CHECK_EQUAL(spec_str[0], string("specv1:GH7899"));
                    BOOST_CHECK_EQUAL(spec_str[1], string("specv2/458966, Birmingham, England"));
                    BOOST_CHECK_EQUAL(spec_str[2], string("specv3:98jkk"));
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL(bio_str[0], string("biom1:AD987"));
                    BOOST_CHECK_EQUAL(bio_str[1], string("biom2:AD987"));
                    BOOST_CHECK_EQUAL(ccol_str[0], string("cult:AD987"));
                    BOOST_CHECK_EQUAL(spec_str[0], string("spec:AD987"));
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL(bio_str[0], string(""));
                    BOOST_CHECK_EQUAL(ccol_str[0], string("ccol:4566"));
                    BOOST_CHECK_EQUAL(spec_str[0], string("spec:4566"));
                    SIZE_TYPE size=1;
                    BOOST_CHECK_EQUAL(ccol_str.size(), size);
                    BOOST_CHECK_EQUAL(spec_str.size(), size);
                break;  
            } 
            bio_str.clear();
            ccol_str.clear();
            spec_str.clear();    
            seqs++;
        }
    
    }
} 

BOOST_AUTO_TEST_CASE(Test_TrimmingOrganismNames)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_trimorgnames, seh);
    BOOST_REQUIRE(seh);
   
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandTrimOrganismNames);
    CRef<CCmdComposite> composite = worker.GetCommand(seh, "Testing trimming organism names");
    BOOST_REQUIRE(composite);
    composite->Execute();

    short seqs=0;
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsSource()){
            string taxname =(*sd).GetSource().GetTaxname();
     
            bool is_dbtaxon = false;
            FOR_EACH_DBXREF_ON_ORGREF (db, (*sd).GetSource().GetOrg())
                if ((*db)->IsSetDb() && NStr::Equal((*db)->GetDb(), "taxon", NStr::eNocase))
                    is_dbtaxon = true;
            switch (seqs){
                case (0):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (1):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case (2):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (3):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus aff."));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true);
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (4):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus aff. burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (5):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus aff. burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;    
                case (6):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus burgeri sp."));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (7):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus burgeri sp. hagfish"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (8):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus burgeri sp. hagfish"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case (9):
                    BOOST_CHECK_EQUAL (taxname, string("Eptatretus burgeri sp. hagfish"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case (10):
                    BOOST_CHECK_EQUAL (taxname, string("uncultured Eptatretus burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (11):
                    BOOST_CHECK_EQUAL (taxname, string("uncultured Eptatretus burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case (12):
                    BOOST_CHECK_EQUAL (taxname, string("uncultured Eptatretus"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (13):
                    BOOST_CHECK_EQUAL (taxname, string("uncultured Eptatretus sp."));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true);
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (14):
                    BOOST_CHECK_EQUAL (taxname, string("Uncultured Eptatretus cf. burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (15):
                    BOOST_CHECK_EQUAL (taxname, string("Uncultured Eptatretus cf. burgeri"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;    
                case (16):
                    BOOST_CHECK_EQUAL (taxname, string("Uncultured Eptatretus burgeri sp."));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (17):
                    BOOST_CHECK_EQUAL (taxname, string("Uncultured Eptatretus burgeri sp. fish"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), true); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, true);
                    break;
                case (18):
                    BOOST_CHECK_EQUAL (taxname, string("uncultured Eptatretus burgeri sp. hagfish"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case (19):
                    BOOST_CHECK_EQUAL (taxname, string("uncultured Eptatretus burgeri sp. hagfish"));
                    BOOST_CHECK_EQUAL ((*sd).GetSource().GetOrg().IsSetCommon(), false); 
                    BOOST_CHECK_EQUAL (is_dbtaxon, false);
                    break;
                case (20) :
                    BOOST_CHECK_EQUAL(taxname, string("Gibbula divaricata"));
                    BOOST_CHECK_EQUAL((*sd).GetSource().GetOrg().IsSetCommon(), false);
                    BOOST_CHECK_EQUAL(is_dbtaxon, false);
                    break;
            } 
            seqs++;
        }
    } 
} 

BOOST_AUTO_TEST_CASE(Test_AddrRNa)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_addfeat, seh);
    BOOST_REQUIRE(seh);
    
    string rna("16S ribosomal RNA");
    CRef<CCmdComposite> composite = AddNamedrRna(seh, rna, true, true);
    composite->Execute();
    
    bool isftableset = false, isrna = false, isinterval = false, isstrandplus = false;
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_BIOSEQS_WITHIN_SEQENTRY (bs, entry){
        if ((*bs).IsNa()){
            BOOST_CHECK_EQUAL ((*bs).GetLength() != 204, true);
            isftableset = (*bs).IsSetAnnot() && ((*bs).GetAnnot().front())->GetData().IsFtable();
            BOOST_CHECK_EQUAL (isftableset, true);
            
            CRef<CSeq_feat> check_feat = ((*bs).GetAnnot().front())->GetData().GetFtable().front();
            isrna = check_feat->IsSetData() && check_feat->GetData().IsRna();
            BOOST_CHECK_EQUAL (isrna, true);
            BOOST_CHECK_EQUAL (check_feat->GetData().GetRna().GetType(), CRNA_ref::eType_rRNA);
            BOOST_CHECK_EQUAL (check_feat->GetData().GetRna().GetExt().GetName(), string("16S ribosomal RNA"));

            BOOST_CHECK_EQUAL (check_feat->IsSetPartial(), true);
            BOOST_CHECK_EQUAL (check_feat->GetPartial(), true);
            
            isinterval = check_feat->IsSetLocation() && check_feat->GetLocation().IsInt();
            BOOST_CHECK_EQUAL (isinterval, true);    
            CRef<CSeq_loc> check_loc = CRef<CSeq_loc>(&(check_feat->SetLocation()));
            
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetFrom(), 0);
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetTo(), (*bs).GetLength()-1);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsSetStrand(), true);
            isstrandplus = check_loc->GetInt().GetStrand() == eNa_strand_plus;
            BOOST_CHECK_EQUAL (isstrandplus,true);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsPartialStart(eExtreme_Biological), true);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsPartialStop(eExtreme_Biological), true);
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetId().Which(), (*bs).GetId().front()->Which());  
        }
    } 
}    

BOOST_AUTO_TEST_CASE(Test_AddControlRegion)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_addfeat, seh);
    BOOST_REQUIRE(seh);
    
    CRef<CCmdComposite> composite = CreateControlRegion(seh);
    composite->Execute();
    
    bool isftableset = false, isrna = false, isinterval = false, isstrandplus = false;
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_BIOSEQS_WITHIN_SEQENTRY (bs, entry){
        if ((*bs).IsNa()){
            BOOST_CHECK_EQUAL ((*bs).GetLength() != 204, true);
            isftableset = (*bs).IsSetAnnot() && ((*bs).GetAnnot().front())->GetData().IsFtable();
            BOOST_CHECK_EQUAL (isftableset, true);
            
            CRef<CSeq_feat> check_feat = ((*bs).GetAnnot().front())->GetData().GetFtable().front();
            isrna = check_feat->IsSetData() && check_feat->GetData().IsImp();
            BOOST_CHECK_EQUAL (isrna, true);
            BOOST_CHECK_EQUAL (check_feat->GetData().GetImp().GetKey(), string("misc_feature"));
            BOOST_CHECK_EQUAL (check_feat->GetComment(), string("control region"));
            BOOST_CHECK_EQUAL (check_feat->IsSetPartial(), true);
            BOOST_CHECK_EQUAL (check_feat->GetPartial(), true); 
            
            isinterval = check_feat->IsSetLocation() && check_feat->GetLocation().IsInt();
            BOOST_CHECK_EQUAL (isinterval, true);    
            CRef<CSeq_loc> check_loc = CRef<CSeq_loc>(&(check_feat->SetLocation()));
            
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetFrom(), 0);
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetTo(), (*bs).GetLength()-1);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsSetStrand(), true);
            isstrandplus = check_loc->GetInt().GetStrand() == eNa_strand_plus;
            BOOST_CHECK_EQUAL (isstrandplus, true);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsPartialStart(eExtreme_Biological), true);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsPartialStop(eExtreme_Biological), true);
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetId().Which(), (*bs).GetId().front()->Which());   
        }
    } 
}    


BOOST_AUTO_TEST_CASE(Test_AddMicrosatellite)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_addfeat, seh);
    BOOST_REQUIRE(seh);
    
    CRef<CCmdComposite> composite = CreateMicrosatellite(seh);
    composite->Execute();
    
    bool isftableset = false, isrna = false, isinterval = false, isstrandplus = false;
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_BIOSEQS_WITHIN_SEQENTRY (bs, entry){
        if ((*bs).IsNa()){
            BOOST_CHECK_EQUAL ((*bs).GetLength() != 204, true);
            isftableset = (*bs).IsSetAnnot() && ((*bs).GetAnnot().front())->GetData().IsFtable();
            BOOST_CHECK_EQUAL (isftableset, true);
            
            CRef<CSeq_feat> check_feat = ((*bs).GetAnnot().front())->GetData().GetFtable().front();
            isrna = check_feat->IsSetData() && check_feat->GetData().IsImp();
            BOOST_CHECK_EQUAL (isrna, true);
            BOOST_CHECK_EQUAL (check_feat->GetData().GetImp().GetKey(), string("repeat_region"));
            BOOST_CHECK_EQUAL (check_feat->IsSetPartial(), false);
            
            isinterval = check_feat->IsSetLocation() && check_feat->GetLocation().IsInt();
            BOOST_CHECK_EQUAL (isinterval, true);    
            CRef<CSeq_loc> check_loc = CRef<CSeq_loc>(&(check_feat->SetLocation()));
            
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetFrom(), 0);
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetTo(), (*bs).GetLength()-1);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsSetStrand(), true);
            isstrandplus = check_loc->GetInt().GetStrand() == eNa_strand_plus;
            BOOST_CHECK_EQUAL (isstrandplus, true);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsPartialStart(eExtreme_Biological), false);
            BOOST_CHECK_EQUAL (check_loc->GetInt().IsPartialStop(eExtreme_Biological), false);
            BOOST_CHECK_EQUAL (check_loc->GetInt().GetId().Which(), (*bs).GetId().front()->Which());   
            
            BOOST_CHECK_EQUAL (check_feat->GetQual().size(), 2);
            BOOST_CHECK_EQUAL (check_feat->GetQual().at(0)->GetQual(), string("rpt_type"));
            BOOST_CHECK_EQUAL (check_feat->GetQual().at(0)->GetVal(), string("tandem"));
            BOOST_CHECK_EQUAL (check_feat->GetQual().at(1)->GetQual(), string("satellite"));
            BOOST_CHECK_EQUAL (check_feat->GetQual().at(1)->GetVal(), string("microsatellite"));
        }
    } 
}    


BOOST_AUTO_TEST_CASE(Test_AddDefLine)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_adddefline, seh);
    BOOST_REQUIRE(seh);
    
    string title("random title");
    int count_conflicts = CountDefLineConflicts(seh, title);
    BOOST_CHECK_EQUAL (count_conflicts, 7);
    title = "new definition line";
    count_conflicts = CountDefLineConflicts(seh, title);
    BOOST_CHECK_EQUAL (count_conflicts, 5);
    
    CRef<CCmdComposite> cmd(new CCmdComposite("Adding definition line"));
    bool update = UpdateAllDefinitionLine(seh, cmd, title, edit::eExistingText_replace_old);
    BOOST_CHECK_EQUAL (update, true);
    cmd->Execute();
    
    char seqs = '0';
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsTitle()){
            if (seqs >='0' && seqs <'9')
                BOOST_CHECK_EQUAL ((*sd).GetTitle(), string("new definition line"));
            else if (seqs=='9')
                BOOST_CHECK_EQUAL ((*sd).GetTitle(), string("some other title"));    
        seqs++;        
        }
        BOOST_CHECK(!(*sd).IsUser());
    }
}            


BOOST_AUTO_TEST_CASE(Test_RemoveDefLines)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_removedeflines, seh);
    BOOST_REQUIRE(seh);
    
    CRef<CCmdComposite> composite(new CCmdComposite("Testing removal of definition lines"));
    RemoveTitlesSeqEntry(seh, *(seh.GetCompleteSeq_entry()), composite);
    composite->Execute();
    
    short nrTitle = 0, nrCrDate = 0, nrSource = 0, nrMolinfo = 0, other = 0;
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsTitle())
            nrTitle++;
        else if ((*sd).IsCreate_date())
            nrCrDate++;
        else if ((*sd).IsMolinfo())
            nrMolinfo++;
        else if ((*sd).IsSource())
            nrSource++;
        else
            other++;
    }
    BOOST_CHECK_EQUAL (nrTitle, 0);
    BOOST_CHECK_EQUAL (nrCrDate, 3);
    BOOST_CHECK_EQUAL (nrSource, 3);
    BOOST_CHECK_EQUAL (nrMolinfo, 6);
    BOOST_CHECK_EQUAL (other, 0);
}

BOOST_AUTO_TEST_CASE(Test_PrefixDefline)
{
    CSeq_entry_Handle seh;
    ReadEntry(sc_TestEntry_prefixtodeflines, seh);
    BOOST_REQUIRE(seh);
    
    CRef<CCmdComposite> composite(new CCmdComposite("Testing prefixing organism name to definition line"));
    AddOrgToDefline(seh, composite);
    composite->Execute();
    
    char seqs='0';
    string title;
    const CSeq_entry& entry = *seh.GetCompleteSeq_entry();
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsTitle()){
            title = (*sd).GetTitle();
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL (title, string("Mixed organisms SET_title_1"));
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL (title, string("Influenza A virus title for seq_1"));
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL (title, string("New orgname title for seq_2"));
                    break;
                case ('3'):
                    BOOST_CHECK_EQUAL (title, string("Cryptomeria japonica SET_title_1"));
                    break;    
                case ('4'):
                    BOOST_CHECK_EQUAL (title, string("New orgname SET_title_1"));
                    break;    
                case ('5'):
                    BOOST_CHECK_EQUAL (title, string("Mixed organisms set title_2"));
                    break;
                case ('6'):
                    BOOST_CHECK_EQUAL (title, string("New orgname set title_2"));
                    break;    
                case ('7'):
                    BOOST_CHECK_EQUAL (title, string("Influenza C virus title for seq_6"));
                    break;
                case ('8'):
                    BOOST_CHECK_EQUAL (title, string("New orgname set title_2"));
                    break;    
                case ('9'):
                    BOOST_CHECK_EQUAL (title, string("New orgname title for seq_8")); 
                    break;
                case (':'):    
                    BOOST_CHECK_EQUAL (title, string("newtitle")); 
                    break;
                } 
            
            seqs++;                
        }
    } 
    
    composite->Unexecute();
    composite.Reset(new CCmdComposite("Testing prefixing strain to definition line"));
    
    CAutoDefAvailableModifier modifier_a(COrgMod::eSubtype_strain, true);
    AddModToDefline(seh, composite, modifier_a, true);
    composite->Execute();
    
    seqs='0';
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if (entry.IsSeq() && (*sd).IsTitle()){
            title = (*sd).GetTitle();
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL (title, string("SET_title_1"));
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL (title, string("strain A/X-31 title for seq_1"));
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL (title, string("strain Set strain title for seq_2"));
                    break;
                case ('3'):
                    BOOST_CHECK_EQUAL (title, string("strain strain_989 SET_title_1"));
                    break;    
                case ('4'):
                    BOOST_CHECK_EQUAL (title, string("strain Set strain SET_title_1"));
                    break;    
                case ('5'):
                    BOOST_CHECK_EQUAL (title, string("set title_2"));
                    break;
                case ('6'):
                    BOOST_CHECK_EQUAL (title, string("strain Set strain set title_2"));
                    break;    
                case ('7'):
                    BOOST_CHECK_EQUAL (title, string("strain strain_6 title for seq_6"));
                    break;
                case ('8'):
                    BOOST_CHECK_EQUAL (title, string("strain Set strain set title_2"));
                    break;    
                case ('9'):
                    BOOST_CHECK_EQUAL (title, string("strain Set strain title for seq_8")); 
                    break;
                case (':'):    
                    BOOST_CHECK_EQUAL (title, string("newtitle")); 
                    break;
                }             
            seqs++;                
        }
    } 

    composite->Unexecute();
    composite.Reset(new CCmdComposite("Testing prefixing strain and haplotype to definition line"));

    CAutoDefSourceDescription::TAvailableModifierVector mod_vector;
    mod_vector.push_back(CAutoDefAvailableModifier(CSubSource::eSubtype_haplotype, false));
    mod_vector.push_back(CAutoDefAvailableModifier(COrgMod::eSubtype_strain, true));
    mod_vector.push_back(CAutoDefAvailableModifier(COrgMod::eSubtype_isolate, true));
    AddModVectorToDefline(seh, composite, mod_vector, true, true);
    composite->Execute();
    
    seqs='0';
    VISIT_ALL_SEQDESCS_WITHIN_SEQENTRY (sd, entry){
        if ((*sd).IsTitle()){
            title = (*sd).GetTitle();
            switch (seqs){
                case ('0'):
                    BOOST_CHECK_EQUAL (title, string("SET_title_1"));
                    break;
                case ('1'):
                    BOOST_CHECK_EQUAL (title, 
                    string("Influenza A virus haplotype haplo99 strain A/X-31 isolate iso_seq1 title for seq_1"));
                    break;
                case ('2'):
                    BOOST_CHECK_EQUAL (title, 
                    string("New orgname haplotype set_haplot strain Set strain isolate iso45 title for seq_2"));
                    break;
                case ('3'):
                    BOOST_CHECK_EQUAL (title, 
                    string("Cryptomeria japonica haplotype set_haplot strain strain_989 SET_title_1"));
                    break;    
                case ('4'):
                    BOOST_CHECK_EQUAL (title, 
                    string("New orgname haplotype set_haplot strain Set strain SET_title_1"));
                    break;    
                case ('5'):
                    BOOST_CHECK_EQUAL (title, string("set title_2"));
                    break;
                case ('6'):
                    BOOST_CHECK_EQUAL (title, 
                    string("New orgname haplotype haplot.987 strain Set strain isolate iso456 set title_2"));
                    break;    
                case ('7'):
                    BOOST_CHECK_EQUAL (title, 
                    string("Influenza C virus haplotype haplot.987 strain strain_6 isolate iso456 title for seq_6"));
                    break;
                case ('8'):
                    BOOST_CHECK_EQUAL (title, 
                    string("New orgname haplotype haplot.987 strain Set strain isolate iso456 set title_2"));
                    break;    
                case ('9'):
                    BOOST_CHECK_EQUAL (title, 
                    string("New orgname haplotype haplot.987 strain Set strain isolate iso456 title for seq_8")); 
                    break;
                case (':'):    
                    BOOST_CHECK_EQUAL (title, string("newtitle")); 
                    break;
                }             
            seqs++;                
        }
    }
}

const char* sc_TestEntry_prefixtodeflines = "\
Seq-entry ::= set {\
    class phy-set,\
     descr { \
      title \"SET_title_1\" , \
      source { \
        org { \
          taxname \"New orgname\" , \
          orgname { \
            mod { \
             { \
               subtype strain , \
               subname \"Set strain\" } } } } , \
        subtype { \
          {  \
            subtype haplotype , \
            name \"set_haplot\" } } } } , \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"title for seq_1\" , \
              source { \
                genome genomic , \
                org { \
                  taxname \"Influenza A virus\"     , \
                  orgname { \
                    mod { \
                     { \
                        subtype strain , \
                        subname \"A/X-31\" } , \
                     { \
                        subtype isolate , \
                        subname \"iso_seq1\" } , \
                     { \
                        subtype serotype , \
                        subname \"H3N2\" } } } } , \
                subtype { \
                 { \
                   subtype clone , \
                   name \"clon45\" } , \
                 { \
                   subtype haplotype , \
                   name \"haplo99\" } } }, \
              molinfo { \
                  biomol genomic } } , \
            inst { \
                repr raw,\
                mol rna,\
                length 150 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
            descr{ \
              title \"title for seq_2\" , \
              source { \
                genome genomic , \
                org { \
                  orgname { \
                    mod { \
                     { \
                        subtype cultivar , \
                        subname \"Elegans\" }    , \
                     { \
                        subtype isolate , \
                        subname \"iso45\" } , \
                     { \
                        subtype serotype , \
                        subname \"H3N2\" } } } } , \
                subtype { \
                 { \
                   subtype haplotype , \
                   name \"\" } } } , \
              molinfo { \
                  biomol genomic } } , \
         inst {\
            repr raw,\
             mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_3\" },\
          descr { \
          source { \
            genome genomic , \
             org { \
              taxname \"Cryptomeria japonica\" , \
              orgname { \
                mod { \
                 { \
                    subtype cultivar , \
                    subname \"Elegans\" }    , \
                 { \
                    subtype strain , \
                    subname \"strain_989\" } , \
                 { \
                    subtype serotype , \
                    subname \"H3N2\" } } } } } }, \
         inst {\
            repr raw,\
            mol na,\
            length 204 } } , \
        seq { \
          id {\
            local str \"seq_4\" } , \
            descr { \
              title \"\" } , \
         inst {\
            repr raw,\
            mol na,\
            length 204 } }    , \
        set { \
          class pop-set , \
          descr { \
            title \"set title_2\" , \
            source {  genome genomic , \
              org { \
                  orgname { \
                    mod { \
                      { \
                        subtype cultivar , \
                        subname \"Elegans\" }    , \
                      { \
                        subtype isolate , \
                        subname \"iso456\" } , \
                      { \
                        subtype strain , \
                           subname \"\" } } } } , \
                subtype { \
                    { \
                        subtype clone , \
                        name \"agVLRB.198.short\" } , \
                    { \
                        subtype haplotype , \
                        name \"haplot.987\" } } } , \
            create-date std{ \
              year 2013, \
              month 6, \
              day 4 } } , \
          seq-set { \
            seq { \
               id { \
                    local str \"seq_5\" } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq { \
               id { \
                    local str \"seq_6\" } , \
               descr{ \
                title \"title for seq_6\" , \
                source {   genome genomic , \
                    org { \
                      taxname \"Influenza C virus\" , \
                      orgname { \
                        mod { \
                          { \
                            subtype cultivar , \
                            subname \"Elegans\" }    , \
                          { \
                            subtype strain , \
                               subname \"strain_6\" } } } } , \
                    subtype { \
                        { \
                            subtype clone , \
                            name \"clone9999\" } } } , \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq { \
               id { \
                    local str \"seq_7\" } , \
               descr{ \
                title \"\" , \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol na, \
                   length 100 } } , \
            seq { \
               id { \
                    local str \"seq_8\" } , \
               descr{ \
                title \"title for seq_8\" , \
                source {   genome genomic , \
                    org { \
                      orgname { \
                        mod { \
                          { \
                            subtype cultivar , \
                            subname \"Elegans\" }    , \
                          { \
                            subtype serotype , \
                               subname \"sero8\" } } } } }, \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } } } , \
        seq { \
          id {\
            local str \"seq_9\" },\
           descr{ \
              title \"newtitle\" , \
              source { \
                org { \
                      taxname \"Influenza B virus\" } } , \
                molinfo { \
                  biomol genomic } } , \
         inst {\
            repr raw,\
            mol aa,\
            length 204 } } } } \
}";        


const char* sc_TestEntry_removedeflines = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      title \"set title0\"  , \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } , \
      title \"set title1\" } , \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
              molinfo { \
                  biomol genomic } } , \
            inst { \
                repr raw,\
                mol rna,\
                length 150 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
            descr{ \
              title \"new definition line\" , \
              title \"some other title\" , \
              title \"some other interesting title\" , \
              create-date std{ \
                year 2012, \
                month 6, \
                day 4 } , \
              molinfo { \
                  biomol genomic } } , \
         inst {\
            repr raw,\
             mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_3\" },\
         inst {\
            repr raw,\
            mol na,\
            length 204 } } , \
        set { \
          descr { \
            title \"set title2\"  , \
            create-date std{ \
              year 2013, \
              month 6, \
              day 4 } , \
            title \"\" } , \
          seq-set { \
            seq { \
               id { \
                    local str \"seq_4\" } , \
               descr{ \
                title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source {   genome genomic , \
                 org { \
                      taxname \"Eptatretus burgeri\" } } , \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq { \
               id { \
                    local str \"seq_5\" } , \
               descr{ \
                source {   genome genomic , \
                    org { \
                      taxname \"Eptatretus burgeri\" } } , \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq { \
               id { \
                    local str \"seq_6\" } , \
               descr{ \
                title \"\" , \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol na, \
                   length 100 } } , \
            seq {\
              id {\
                local str \"seq_7\" },\
              inst {\
                repr raw,\
                mol rna,\
                length 3455 } } } } , \
        seq { \
          id {\
            local str \"seq_8\" },\
           descr{ \
                title \"new definition line\" , \
                title \"some other title\" , \
                source {  genome genomic , \
                  org { \
                      taxname \"Eptatretus burgeri\" } } , \
                molinfo { \
                  biomol genomic } } , \
         inst {\
            repr raw,\
            mol aa,\
            length 204 } } } } \
}";        

const char* sc_TestEntry_adddefline = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      title \"set title\" , \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
              molinfo { \
                  biomol genomic } } , \
            inst { \
                repr raw,\
                mol rna,\
                length 150 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
            descr{ \
              title \"new definition line\" , \
              title \"some other title\" , \
              title \"some other interesting title\" , \
              create-date std{ \
                year 2012, \
                month 6, \
                day 4 } , \
              molinfo { \
                  biomol genomic } } , \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_3\" },\
         inst {\
            repr raw,\
            mol na,\
            length 204 } } , \
        set { \
          seq-set { \
            seq { \
               id { \
                    local str \"seq_4\" } , \
               descr{ \
                title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source {   genome genomic , \
                 org { \
                      taxname \"Eptatretus burgeri\" } } , \
                user { \
                   class \"1.0\" , \
                   type str \"AutodefOptions\" , \
                     data { \
                        { \
                        label str \"FeatureListType\" , \
                        data str \"List All Features\" } } } } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq { \
               id { \
                    local str \"seq_5\" } , \
               descr{ \
                source {   genome genomic , \
                    org { \
                      taxname \"Eptatretus burgeri\" } } , \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq { \
               id { \
                    local str \"seq_6\" } , \
               descr{ \
                title \"\" , \
                molinfo { \
                    biomol genomic } } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq {\
              id {\
                local str \"seq_7\" },\
              descr{ \
                title \"new definition line\" , \
                molinfo { \
                  biomol genomic } } , \
              inst {\
                repr raw,\
                mol rna,\
                length 3455 } } } } , \
        seq { \
          id {\
            local str \"seq_8\" },\
           descr{ \
                title \"new definition line\" , \
                title \"some other title\" , \
                source {  genome genomic , \
                  org { \
                      taxname \"Eptatretus burgeri\" } } , \
                molinfo { \
                  biomol genomic } } , \
         inst {\
            repr raw,\
            mol aa,\
            length 204 } } } } \
}";        


const char * sc_TestEntry_addfeat = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
             inst {\
                repr raw,\
                mol rna,\
                length 150 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_3\" },\
         inst {\
            repr raw,\
            mol aa,\
            length 204 } } , \
        set { \
          seq-set { \
            seq { \
               id { \
                    local str \"title1\" } , \
                inst { \
                   repr virtual, \
                   mol dna, \
                   length 100 } } , \
            seq {\
              id {\
                local str \"seq_15\" },\
              inst {\
                repr raw,\
                mol rna,\
                length 3455 } } } } } } \
}";        


const char * sc_TestEntry_trimorgnames = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Eptatretus burgeri\" , \
                      common \"hagfish\" , \
                      db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } } ,\
                      orgname { \
                         mod { \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus burgeri vertebrate\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } }  } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_3\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_4\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus aff.\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_5\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus aff. burgeri\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_6\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus aff. burgeri hagfish\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_7\" },\
          descr{ \
            title \"Some_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus burgeri sp.\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_8\" },\
          descr{ \
            title \"Some_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus burgeri sp. hagfish\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_9\" },\
          descr{ \
            title \"Some new title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus burgeri sp. hagfish fish\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq{ \
          id {\
            local str \"seq_10\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Eptatretus burgeri sp. hagfish aff. some\" , \
                  common \"hagfish\" , \
                  db { \
                    { \
                     db \"taxon\" , \
                     tag id 5966 } } ,\
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_11\" },\
          descr{ \
            title \"Some new title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"uncultured Eptatretus burgeri\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_12\" },\
          descr{ \
            title \"Some new title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"uncultured Eptatretus burgeri hagfish\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_13\" },\
          descr{ \
            title \"Some new title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"uncultured Eptatretus\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_14\" },\
          descr{ \
            title \"Some new title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"uncultured Eptatretus sp.\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq{ \
          id {\
            local str \"seq_15\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Uncultured Eptatretus cf. burgeri\" , \
                  common \"hagfish\" , \
                  db { \
                    { \
                     db \"taxon\" , \
                     tag id 5966 } } ,\
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_16\" },\
          descr{ \
            title \"Some_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Uncultured Eptatretus cf. burgeri hagfish\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_17\" },\
          descr{ \
            title \"Some_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Uncultured Eptatretus burgeri sp.\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_18\" },\
          descr{ \
            title \"Some_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Uncultured Eptatretus burgeri sp. fish\" , \
                  common \"hagfish\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 3652 } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq{ \
          id {\
            local str \"seq_19\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"uncultured Eptatretus burgeri sp. hagfish fish\" , \
                  common \"hagfish\" , \
                  db { \
                    { \
                     db \"taxon\" , \
                     tag id 5966 } } ,\
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq{ \
          id {\
            local str \"seq_20\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"uncultured Eptatretus burgeri sp. hagfish aff. some\" , \
                  common \"hagfish\" , \
                  db { \
                    { \
                     db \"taxon\" , \
                     tag id 5964 } } ,\
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_21\" },\
          descr{ \
            source { \
                org { \
                  taxname \"Gibbula divaricata: Trochidae: Vetigastropoda: Mollusca\" } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } } } \
}";                                    

const char * sc_TestEntry_splitstructcollections = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Mus Musculus\" , \
                      common \"house mouse\" , \
                      orgname { \
                         mod { \
                            { \
                              subtype strain , \
                              subname \"DSM 7866\" } , \
                            { \
                              subtype serotype , \
                              subname \"H2N8\" } , \
                            { \
                              subtype bio-material , \
                              subname \"biom1:GH7899; biom2/458966, Birmingham, England  ;  biom3:98jkk \" } , \
                            { \
                              subtype culture-collection , \
                              subname \"ccol1:GH7899; ccol2/458966, Birmingham, England  ;  ccol3:98jkk \" } ,\
                            { \
                              subtype specimen-voucher , \
                              subname \"specv1:GH7899; specv2/458966, Birmingham, England  ;specv3:98jkk\" } , \
                            { \
                              subtype culture-collection , \
                              subname \"new_ccol\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } } , \
        seq { \
          id {\
            local str \"seq_3\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Escherichia coli\" , \
                  common \"flu\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype bio-material , \
                        subname \"biom1:AD987; \" } , \
                      { \
                        subtype strain , \
                        subname \"DSM_strain\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } , \
                      { \
                        subtype specimen-voucher , \
                        subname \" spec:AD987  ;\" } , \
                      { \
                        subtype bio-material , \
                        subname \"biom2:AD987; \" } , \
                      { \
                        subtype culture-collection , \
                        subname \"cult:AD987;  \" }    } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_5\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Gorilla gorilla\" , \
                  common \"flu\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } , \
                      { \
                        subtype bio-material , \
                        subname \"\" } ,\
                      { \
                        subtype culture-collection , \
                        subname \"    ; ccol:4566   \" } , \
                      { \
                        subtype specimen-voucher , \
                        subname \"  ; spec:4566 ;  \" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" }  \
         }    } } \
}";



const char * sc_TestEntry_parsingwithconflicts = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Influenza A virus (X-33(sero/H5N9))\" , \
                      common \"flu\" , \
                      orgname { \
                         mod { \
                            { \
                              subtype strain , \
                              subname \"DSM 7866\" } , \
                            { \
                              subtype serotype , \
                              subname \"H2N8\" } , \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } } , \
        seq { \
          id {\
            local str \"seq_3\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus (X-33())  \" , \
                  common \"flu\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"DSM_strain\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_5\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus (X-33)\" , \
                  common \"flu\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } , \
                      { \
                        subtype isolate , \
                        subname \"my_isolate\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" }  \
         }    } } \
}";    

const char * sc_TestEntry_parsingnoconflicts = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Influenza A virus (DSM 7866(H2N8))\" , \
                      common \"flu\" , \
                      orgname { \
                         mod { \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus (DSM 7866(H2N8))\" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } }  } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_3\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus (DSM 7866(H2N8))  \" , \
                  common \"flu\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"DSM 7866\" } , \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq{ \
          id {\
            local str \"seq_4\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus (DSM 7866)  \" , \
                  common \"flu\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } } } \
}";    

            
            





const char * sc_TestEntry_noparsing="\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Influenza A virus\" , \
                      common \"flu\" , \
                      db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } , \
                        { \
                         db \"access\" , \
                         tag id 4581 } \
                        } , \
                      orgname { \
                         mod { \
                            { \
                              subtype old-name , \
                              subname \"a_flu\" } , \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
              org { \
                common \"flu\" \
                } \
              }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } } } \
}";    

const char * sc_TestEntry_fixorganismnames = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Influenza A virus (DSM 7866(H2N8))\" , \
                      common \"flu\" , \
                      db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } , \
                        { \
                         db \"access\" , \
                         tag id 4581 } \
                        } , \
                      orgname { \
                         mod { \
                            { \
                              subtype strain , \
                              subname \"DSM 7866\" } , \
                            { \
                              subtype serotype , \
                              subname \"H2N8\" } , \
                              { \
                              subtype old-name , \
                              subname \"a_flu\" } , \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus(DSM 7866   (    H2N8  )   )    \" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } } , \
                  orgname { \
                    mod { \
                      { \
                        subtype isolate , \
                        subname \"my_isolate\" }, \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_3\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus(  DSM 7866  )  \" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 }, \
                        { \
                         db \"access\" , \
                         tag id 4581 } },\
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"DSM 7866\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_4\" } , \
          descr{ \
            title \"different sequence 4\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus   (   DSM 7866 (  H2N8  )  )  \" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 }, \
                        { \
                         db \"access\" , \
                         tag id 4581 } } ,\
                  orgname { \
                    mod { \
                      { \
                          subtype old-name , \
                        subname \"a_flu\" } , \
                      { \
                        subtype isolate , \
                        subname \"my_isolate\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_5\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza B virus (DSM 7866(H2N8))\" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } , \
                        { \
                         db \"access\" , \
                         tag id 4581 } } , \
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H2N8\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } , \
                      { \
                        subtype isolate , \
                        subname \"my_isolate\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" }  \
         }    } } \
}";    



const char * sc_TestEntry_addstrainserotype = "\
Seq-entry ::= set {\
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Influenza A virus\" , \
                      common \"flu\" , \
                      db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } , \
                        { \
                         db \"access\" , \
                         tag id 4581 } \
                        } , \
                      orgname { \
                         mod { \
                            { \
                              subtype strain , \
                              subname \"X-33\" } , \
                            { \
                              subtype serotype , \
                              subname \"H5N9\" } , \
                              { \
                              subtype old-name , \
                              subname \"a_flu\" } , \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } } , \
        seq { \
          id {\
            local str \"seq_2\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"influenZa A Virus (X-33)\" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } } , \
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"X-33\" } , \
                      { \
                        subtype serotype , \
                        subname \"H7N8\" } , \
                      { \
                        subtype isolate , \
                        subname \"my_isolate\" }, \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_3\" },\
          descr{ \
            title \"New_title\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus (Y546)\" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 }, \
                        { \
                         db \"access\" , \
                         tag id 4581 } },\
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"X-33\" } , \
                      { \
                        subtype serotype , \
                        subname \"H5N9\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
         seq { \
          id {\
            local str \"seq_4\" } , \
          descr{ \
            title \"different sequence 4\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus\" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 }, \
                        { \
                         db \"access\" , \
                         tag id 4581 } } ,\
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"X-33\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } , \
                      { \
                        subtype isolate , \
                        subname \"my_isolate\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
        seq { \
          id {\
            local str \"seq_5\" },\
          descr{ \
            title \"toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus\" , \
                  common \"flu\" , \
                  db { \
                        { \
                         db \"taxon\" , \
                         tag id 5966 } , \
                        { \
                         db \"access\" , \
                         tag id 4581 } } , \
                  orgname { \
                    mod { \
                      { \
                        subtype serotype , \
                        subname \"H3N5\" } , \
                      { \
                        subtype old-name , \
                        subname \"a_flu\" } , \
                      { \
                        subtype isolate , \
                        subname \"my_isolate\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw , \
            mol dna , \
            length 20 , \
            topology circular , \
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" }  \
         }    } } \
}";    





const char* sc_TestEntry_multiplestrains = " \
Seq-entry ::= set { \
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"TitleA: toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Influenza A virus\" , \
                        orgname { \
                          mod { \
                            { \
                              subtype strain , \
                              subname \"DSM 7876, ATCC VR-111, strain3, C768L/7\" } , \
                            { \
                              subtype serotype , \
                              subname \"this is my serotype\" } , \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } },\
        seq { \
          id {\
            local str \"seq_2\" },\
          descr{ \
            title \"TitleB: toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Influenza A virus\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"  DSM 7876  , ATCC VR-111  , strain3  , C768L/7  \" } , \
                      { \
                        subtype serotype , \
                        subname \"this is my serotype\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" }  } } } \
}";    

const char* sc_TestEntry_mtplstrains_empty = "\
Seq-entry ::= set { \
    class phy-set,\
    descr { \
      create-date std{ \
        year 2013, \
        month 6, \
        day 4 } }, \
    seq-set {\
        seq {\
            id {\
              local str \"seq_1\" },\
            descr{ \
              title \"TitleA: toll-like receptor 5 (TLR5) gene, complete cds\" , \
                source { \
                  genome genomic , \
                    org { \
                      taxname \"Mus Musculus\" , \
                        orgname { \
                          mod { \
                            { \
                              subtype strain , \
                              subname \"DSM 7876, \" } , \
                            { \
                              subtype serotype , \
                              subname \"this is my serotype\" } , \
                            { \
                              subtype isolate , \
                              subname \"BoBM478\" } } } } }, \
                molinfo { \
                  biomol genomic } }, \
             inst {\
                repr raw,\
                mol rna,\
                length 15 } },\
        seq { \
          id {\
            local str \"seq_2\" },\
          descr{ \
            title \"TitleB: toll-like receptor 5 (TLR5) gene, complete cds\" , \
            source { \
              genome genomic , \
                org { \
                  taxname \"Giraffa\" , \
                  orgname { \
                    mod { \
                      { \
                        subtype strain , \
                        subname \"  DSM 7876  ,   \" } , \
                      { \
                        subtype isolate , \
                        subname \"78GB98\" } } } } }, \
            molinfo { \
              biomol genomic } }, \
         inst {\
            repr raw,\
            mol dna,\
            length 20,\
            topology circular,\
            seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" }  } } } \
}";    


BOOST_AUTO_TEST_CASE(Test_Automatch)
{
    BOOST_CHECK_EQUAL (CFieldChoicePanel::AutoMatch("acronym"), "Source Qualifier acronym");
    BOOST_CHECK_EQUAL (CFieldChoicePanel::AutoMatch("protein description"), "CDS-Gene-Prot Qualifier protein description");

}


BOOST_AUTO_TEST_CASE(Test_LooksLikeHeader) 
{
    vector<string> field_names;
    field_names.push_back("SeqID");
    field_names.push_back("taxname");
    field_names.push_back("product");
    field_names.push_back("acronym");

    BOOST_CHECK_EQUAL(CFieldChoicePanel::LooksLikeHeader(field_names), true);

    field_names.push_back("something odd");
    field_names.push_back("another mismatch");
    field_names.push_back("unrecognizable");
    field_names.push_back("also bad");
    field_names.push_back("not matching");
    field_names.push_back("a mistake");
    field_names.push_back("a typo");
    field_names.push_back("wrong");
    field_names.push_back("not found");
    BOOST_CHECK_EQUAL(CFieldChoicePanel::LooksLikeHeader(field_names), false);

}


const char* sc_TestEntry_apply_table = "Seq-entry ::= set { \
  class phy-set , \
  seq-set { \
    seq {\
      id { \
        local str \"seq_1\" } ,\
        descr{ \
            source { \
              org { \
                taxname \"foo\" } , \
              subtype { \
                { \
                  subtype chromosome , \
                  name \"1\" } } } ,\
            molinfo { \
                biomol genomic } } , \
      inst { \
        repr raw,\
        mol dna,\
        length 20 , \
        seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
    seq {\
      id { \
        local str \"seq_2\" } ,\
        descr{ \
            source { \
              org { \
                taxname \"bar\" } , \
              subtype { \
                { \
                  subtype chromosome , \
                  name \"2\" } } } ,\
            molinfo { \
                biomol genomic } } , \
      inst { \
        repr raw,\
        mol dna,\
        length 20 , \
        seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } } , \
    seq {\
      id { \
        local str \"seq_3\" } ,\
        descr{ \
            source { \
              org { \
                taxname \"foobar\" } , \
              subtype { \
                { \
                  subtype chromosome , \
                  name \"3\" } } } ,\
            molinfo { \
                biomol genomic } } , \
      inst { \
        repr raw,\
        mol dna,\
        length 20 , \
        seq-data iupacna \"AAAATTTTGGGGCCCCAAAA\" } }\
} }";
  

CRef<CUser_object> x_MakeDefaultTextOptions(size_t num_cols)
{
    CRef<CUser_object> text_options (new CUser_object());
    for (size_t i = 0; i < num_cols; i++) {
        text_options->SetData().push_back(MakeTableMetaInfoField(edit::eExistingText_replace_old, true));
    }
    return text_options;
}


void x_TestPair(CSeq_entry_Handle seh, 
                const string& match, const string& field, 
                const vector<string>& match_vals,
                const vector<string>& field_vals)
{
    CRef<CSeq_table> table(new CSeq_table());
    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetTitle("match " + match);
    id_col->SetData().SetString().insert(id_col->SetData().SetString().begin(),
                                         match_vals.begin(), match_vals.end());
    table->SetColumns().push_back(id_col);

    CRef<CSeqTable_column> data_col(new CSeqTable_column());
    data_col->SetHeader().SetTitle(field);
    data_col->SetData().SetString().insert(data_col->SetData().SetString().begin(),
                                         field_vals.begin(), field_vals.end());
    table->SetColumns().push_back(data_col);

    table->SetNum_rows(match_vals.size());
    CRef<CUser_object> text_options = x_MakeDefaultTextOptions(table->GetColumns().size());

    /*CRef<CCmdComposite> cmd = ApplyMiscSeqTableToSeqEntry(table, seh, text_options);
    if (!cmd) {
        BOOST_CHECK_EQUAL("Expected command", "No command returned");
        return;
    }
    cmd->Execute();
    */
    vector<string> retrieve_fields;
    if (!NStr::Equal(match, kSequenceIdColLabel)) {
        retrieve_fields.push_back(match);
    }
    retrieve_fields.push_back(field);
    CRef<CSeq_table> return_table = GetMiscTableFromSeqEntry(retrieve_fields, seh);
    CRef<CSeqTable_column> r_match_col;
    CRef<CSeqTable_column> r_data_col;
    if (return_table->GetColumns().size() == 2) {
        r_match_col = return_table->GetColumns()[0];
        r_data_col = return_table->GetColumns()[1];
    } else {
        r_match_col = return_table->GetColumns()[1];
        r_data_col = return_table->GetColumns()[2];
    }
    for (unsigned int row = 0; row < id_col->GetData().GetString().size(); row++) {
        bool found = false;
        string search = id_col->GetData().GetString()[row];
        for (unsigned int result_row = 0; result_row < r_data_col->GetData().GetString().size(); result_row++) {
            string this_id = "";
            if (r_match_col->GetData().IsId()) {
                r_match_col->GetData().GetId()[result_row]->GetLabel(&this_id, CSeq_id::eContent);
            } else {
                this_id = r_match_col->GetData().GetString()[result_row];
            } 
            if (NStr::Equal(search, this_id)) {
                found = true;
                string was_applied = data_col->GetData().GetString()[row];
                string is_now = r_data_col->GetData().GetString()[result_row];
                BOOST_CHECK_EQUAL (was_applied, is_now);
            }
        }
        if (!found) {
            BOOST_CHECK_EQUAL ("All data found", "Can't find row for " + search);
        }
    }

}


void x_TestList(CSeq_entry_Handle seh, 
                const string& match, const vector<string>& field_list, 
                const vector<string>& match_vals,
                const vector<vector<string> > & field_vals)
{
    CRef<CSeq_table> table(new CSeq_table());
    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetTitle("match " + match);
    id_col->SetData().SetString().insert(id_col->SetData().SetString().begin(),
                                         match_vals.begin(), match_vals.end());
    table->SetColumns().push_back(id_col);

    for (size_t col = 0; col < field_list.size(); col++) {
        string field = field_list[col];
        CRef<CSeqTable_column> data_col(new CSeqTable_column());
        data_col->SetHeader().SetTitle(field);
        data_col->SetData().SetString().insert(data_col->SetData().SetString().end(),
                                             field_vals[col].begin(), field_vals[col].end());
        table->SetColumns().push_back(data_col);
    }

    table->SetNum_rows(match_vals.size());
    CRef<CUser_object> text_options = x_MakeDefaultTextOptions(table->GetColumns().size());
    /*
    CRef<CCmdComposite> cmd = ApplyMiscSeqTableToSeqEntry(table, seh, text_options);
    if (!cmd) {
        BOOST_CHECK_EQUAL("Expected command", "No command returned");
        return;
    }
    cmd->Execute();
    */
    vector<string> retrieve_fields;
    int col_start = 0;
    if (!NStr::Equal(match, kSequenceIdColLabel)) {
        retrieve_fields.push_back(match);
        col_start++;
    }
    retrieve_fields.insert(retrieve_fields.end(), field_list.begin(), field_list.end());
    CRef<CSeq_table> return_table = GetMiscTableFromSeqEntry(retrieve_fields, seh);
    CRef<CSeqTable_column> r_match_col = return_table->GetColumns()[col_start];

    for (unsigned int row = 0; row < id_col->GetData().GetString().size(); row++) {
        bool found = false;
        string search = id_col->GetData().GetString()[row];
        for (unsigned int result_row = 0; result_row < r_match_col->GetData().GetSize(); result_row++) {
            string this_id = "";
            if (r_match_col->GetData().IsId()) {
                r_match_col->GetData().GetId()[result_row]->GetLabel(&this_id, CSeq_id::eContent);
            } else {
                this_id = r_match_col->GetData().GetString()[result_row];
            } 
            if (NStr::Equal(search, this_id)) {
                found = true;
                for (unsigned int col = 1; col < table->GetColumns().size(); col++) {
                    string was_applied = table->GetColumns()[col]->GetData().GetString()[row];
                    string is_now = return_table->GetColumns()[col + col_start]->GetData().GetString()[result_row];
                    BOOST_CHECK_EQUAL (was_applied, is_now);
                }
            }
        }
        if (!found) {
            BOOST_CHECK_EQUAL ("All data found", "Can't find row for " + search);
        }
    }
}

/*
BOOST_AUTO_TEST_CASE(Test_ApplyTable)
{
    CSeq_entry entry;
    CNcbiIstrstream istr(sc_TestEntry_apply_table);
    istr >> MSerial_AsnText >> entry;
    
    CScope scope(*CObjectManager::GetInstance());
    CSeq_entry_Handle seh = scope.AddTopLevelSeqEntry (entry);
    
    vector<string> match_vals;
    match_vals.push_back("seq_1");
    match_vals.push_back("seq_2");
    match_vals.push_back("seq_3");
    vector<string> data_vals;
    data_vals.push_back("a");
    data_vals.push_back("b");
    data_vals.push_back("c");
    x_TestPair(seh, kSequenceIdColLabel, "taxname", match_vals, data_vals);
    x_TestPair(seh, "taxname", kDefinitionLineLabel, data_vals, match_vals);
    x_TestPair(seh, kDefinitionLineLabel, kCommentDescriptorLabel, match_vals, data_vals);
    x_TestPair(seh, kCommentDescriptorLabel, "acronym", data_vals, match_vals);
    vector<string> number_vals;
    number_vals.push_back("1234");
    number_vals.push_back("5678");
    number_vals.push_back("1357");
    x_TestPair(seh, "acronym", kGenomeProjectID, match_vals, number_vals);
    x_TestPair(seh, kGenomeProjectID, "taxname", number_vals, match_vals);
    x_TestPair(seh, "taxname", kGenbankBlockKeyword, match_vals, data_vals);
    x_TestPair(seh, kGenbankBlockKeyword, "DBLink BioSample", data_vals, match_vals);

    // now try feature qualifiers on genes
    for (CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        CRef<CSeq_feat> feat(new CSeq_feat());
        feat->SetLocation().SetInt().SetFrom(0);
        feat->SetLocation().SetInt().SetTo(bi->GetBioseqLength() - 1);
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*(bi->GetSeqId()));
        feat->SetLocation().SetInt().SetId(*id);
        feat->SetData().SetGene();
        CSeq_entry_Handle seh = bi->GetParentEntry();
        CCmdCreateFeat cmd(seh, *feat);
        cmd.Execute();        
    }

    x_TestPair(seh, kSequenceIdColLabel, "gene locus", match_vals, data_vals);
    x_TestPair(seh, "gene locus", "gene allele", data_vals, number_vals);

    // now try coding regions
    CCmdComposite composite_cmd("Add coding regions");
    for (CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        CRef<objects::CSeq_feat> cds(new objects::CSeq_feat());
        cds->SetData().SetCdregion();
        cds->SetLocation().SetInt().SetFrom(0);
        cds->SetLocation().SetInt().SetTo(bi->GetBioseqLength() - 1);
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*(bi->GetSeqId()));
        cds->SetLocation().SetInt().SetId(*id);
        cds->SetData().SetCdregion();
        CRef<objects::CSeq_entry> protein = CreateTranslatedProteinSequence (cds, *bi, create_general_only);
        CRef<objects::CSeq_feat> prot = AddProteinFeatureToProtein (protein,
                          cds->GetLocation().IsPartialStart(objects::eExtreme_Biological), 
                          cds->GetLocation().IsPartialStop(objects::eExtreme_Biological)); 

        prot->SetData().SetProt().SetName().push_back("x");
        CRef<CCmdAddSeqEntry> add_prot(new CCmdAddSeqEntry(protein, seh));
        composite_cmd.AddCommand(*add_prot);
        CRef<CCmdCreateFeat> add_cds(new CCmdCreateFeat(seh, *cds));
        composite_cmd.AddCommand(*add_cds);
    }
    composite_cmd.Execute();

    x_TestPair(seh, "gene allele", "CDS comment", number_vals, match_vals);
    x_TestPair(seh, "gene locus", "protein description", data_vals, number_vals);

    vector<string> field_list;
    vector<vector<string> > val_list;
    field_list.push_back("clone");
    field_list.push_back("strain");
    val_list.push_back(data_vals);
    val_list.push_back(number_vals);
    x_TestList(seh, kSequenceIdColLabel, field_list, match_vals, val_list);

    field_list.clear();
    field_list.push_back("gene allele");
    field_list.push_back("gene locus");
    x_TestList(seh, kSequenceIdColLabel, field_list, match_vals, val_list);

    field_list.push_back("clone");
    field_list.push_back("strain");
    val_list.clear();
    val_list.push_back(number_vals);
    val_list.push_back(data_vals);
    val_list.push_back(number_vals);
    val_list.push_back(data_vals);
    x_TestList(seh, kSequenceIdColLabel, field_list, match_vals, val_list);

}
*/
