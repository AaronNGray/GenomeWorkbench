/*  $Id: unit_test_field_handler.cpp 584191 2019-04-08 16:55:07Z bollin $
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
* Author:  Colleen Bollin, NCBI
*
* File Description:
*   Unit tests for the field handlers.
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include "unit_test_field_handler.hpp"

#include <corelib/ncbi_system.hpp>

// This macro should be defined before inclusion of test_boost.hpp in all
// "*.cpp" files inside executable except one. It is like function main() for
// non-Boost.Test executables is defined only in one *.cpp file - other files
// should not include it. If NCBI_BOOST_NO_AUTO_TEST_MAIN will not be defined
// then test_boost.hpp will define such "main()" function for tests.
//
// Usually if your unit tests contain only one *.cpp file you should not
// care about this macro at all.
//
//#define NCBI_BOOST_NO_AUTO_TEST_MAIN


// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>

#include <objects/biblio/Id_pat.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/User_object.hpp>
#include <objects/medline/Medline_entry.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/GIBB_mol.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seq/Ref_ext.hpp>
#include <objects/seq/Map_ext.hpp>
#include <objects/seq/Seg_ext.hpp>
#include <objects/seq/Seq_gap.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seq/Seq_hist.hpp>
#include <objects/seq/Seq_hist_rec.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <objects/seqblock/EMBL_block.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/PDB_seq_id.hpp>
#include <objects/seqloc/Giimport_id.hpp>
#include <objects/seqloc/Patent_seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/valid/Comment_set.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objects/seq/seqport_util.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objtools/unit_test_util/unit_test_util.hpp>
#include <corelib/ncbiapp.hpp>

#include <objtools/unit_test_util/unit_test_util.hpp>
#include <objtools/edit/field_handler.hpp>
#include <objtools/edit/dblink_field.hpp>
#include <objtools/edit/struc_comm_field.hpp>
#include <objtools/edit/gb_block_field.hpp>

#include <common/test_assert.h>  /* This header must go last */


// for writing out tmp files

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)





NCBITEST_INIT_TREE()
{
    if ( !CNcbiApplication::Instance()->GetConfig().HasEntry("NCBI", "Data") ) {
    }
}

static bool s_debugMode = false;

NCBITEST_INIT_CMDLINE(arg_desc)
{
    // Here we make descriptions of command line parameters that we are
    // going to use.

    arg_desc->AddFlag( "debug_mode",
        "Debugging mode writes errors seen for each test" );
}

NCBITEST_AUTO_INIT()
{
    // initialization function body

    const CArgs& args = CNcbiApplication::Instance()->GetArgs();
    if (args["debug_mode"]) {
        s_debugMode = true;
    }
}


BOOST_AUTO_TEST_CASE(Test_AddBiosample)
{

    CRef<CSeq_entry> entry = unit_test_util::BuildGoodNucProtSet();
    STANDARD_SETUP

    CBioseq_CI bi(seh, CSeq_inst::eMol_na);
    edit::CDBLinkField dblink_field(edit::CDBLinkField::eDBLinkFieldType_BioSample);

    vector<CRef<edit::CApplyObject> > apply_objects = dblink_field.GetApplyObjects(*bi);
    
    BOOST_CHECK_EQUAL(apply_objects.size(), 1);

    CRef<CUser_object> user = edit::CDBLinkField::MakeUserObject();
    dblink_field.SetVal(*user, "biosample value");

    BOOST_CHECK_EQUAL(user->GetType().GetStr(), "DBLink");
    
    BOOST_CHECK_EQUAL(user->GetData().front()->GetLabel().GetStr(), "BioSample");
    BOOST_CHECK_EQUAL(user->GetData().front()->GetData().GetStrs().front(), "biosample value");
}


BOOST_AUTO_TEST_CASE(StructuredCommentField)
{
    vector<string> field_names = CComment_set::GetFieldNames("Genome-Assembly-Data");
    if (field_names.size() == 15) {
        BOOST_CHECK_EQUAL(field_names[0], "Assembly Provider");
        BOOST_CHECK_EQUAL(field_names[1], "Finishing Goal");
    } else {
        BOOST_CHECK_EQUAL(field_names.size(), 16);
    }

    string keyword = CComment_rule::KeywordForPrefix("MIGS:3.0-Data");
    BOOST_CHECK_EQUAL(keyword, "GSC:MIxS;MIGS:3.0");

    string prefix = CComment_rule::PrefixForKeyword("GSC:MIxS;MIMS:3.0");
    BOOST_CHECK_EQUAL(prefix, "MIMS:3.0-Data");

    vector<string> keywords = CComment_rule::GetKeywordList();
    BOOST_CHECK_EQUAL(keywords[0], "GSC:MIGS:2.1");
}




BOOST_AUTO_TEST_CASE(Test_GenomeAssemblyData)
{
    CRef<CUser_object> user = edit::CGenomeAssemblyComment::MakeEmptyUserObject();
    BOOST_CHECK_EQUAL(user->GetType().GetStr(), "StructuredComment");
    BOOST_CHECK_EQUAL(user->GetData().front()->GetLabel().GetStr(), "StructuredCommentPrefix");
    BOOST_CHECK_EQUAL(user->GetData().front()->GetData().GetStr(), "##Genome-Assembly-Data-START##");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetLabel().GetStr(), "StructuredCommentSuffix");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetData().GetStr(), "##Genome-Assembly-Data-END##");

    edit::CGenomeAssemblyComment::SetAssemblyMethod(*user, "method");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetLabel().GetStr(), "Assembly Method");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetData().GetStr(), "method");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetAssemblyMethod(*user), "method");

    edit::CGenomeAssemblyComment::SetGenomeCoverage(*user, "coverage");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetLabel().GetStr(), "Genome Coverage");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetData().GetStr(), "coverage");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetGenomeCoverage(*user), "coverage");

    edit::CGenomeAssemblyComment::SetSequencingTechnology(*user, "tech");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetLabel().GetStr(), "Sequencing Technology");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetData().GetStr(), "tech");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetSequencingTechnology(*user), "tech");

    // false because Assembly Method is not valid
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::IsValid(*user), false);
    edit::CGenomeAssemblyComment::SetAssemblyMethod(*user, "program v. version");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::IsValid(*user), true);

    // alternate creation method
    edit::CGenomeAssemblyComment gnm_asm_cmt;
    CRef<CUser_object> other_user = gnm_asm_cmt.SetAssemblyMethodProgram("program")
                    .SetAssemblyMethodVersion("version")
                    .SetGenomeCoverage("cv")
                    .SetSequencingTechnology("st")
                    .MakeUserObject();
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetAssemblyMethodProgram(*other_user), "program");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetAssemblyMethodVersion(*other_user), "version");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetAssemblyMethod(*other_user), "program v. version");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetGenomeCoverage(*other_user), "cv");
    BOOST_CHECK_EQUAL(edit::CGenomeAssemblyComment::GetSequencingTechnology(*other_user), "st");


    // check the order
    edit::CGenomeAssemblyComment gnm_asm_cmt2;
    other_user = gnm_asm_cmt2.SetAssemblyMethodProgram("program")
                    .SetAssemblyMethodVersion("version")
                    .SetSequencingTechnology("st")
                    .SetGenomeCoverage("cv")
                    .MakeUserObject();

    BOOST_CHECK_EQUAL(other_user->GetData()[2]->GetLabel().GetStr(), "Assembly Method");
    BOOST_CHECK_EQUAL(other_user->GetData()[3]->GetLabel().GetStr(), "Genome Coverage");
    BOOST_CHECK_EQUAL(other_user->GetData()[4]->GetLabel().GetStr(), "Sequencing Technology");

}


BOOST_AUTO_TEST_CASE(Test_ANIComment)
{
    edit::CANIComment ani_cmt;
    ani_cmt.SetThisGenome("this_genome");

    CRef<CUser_object> obj = ani_cmt.MakeUserObject();
    BOOST_CHECK_EQUAL(edit::CANIComment::IsValid(*obj), false);

    ani_cmt.SetCurrentName("current_name");
    ani_cmt.SetA1ANI("A1_ANI");
    ani_cmt.SetPreviousName("previous_name");
    ani_cmt.SetDateUpdated("date_updated");
    ani_cmt.SetAnalysisType("a");
    ani_cmt.SetAnalysis1("a");
    ani_cmt.SetA1Genome("a");
    ani_cmt.SetA1Name("a");
    ani_cmt.SetA1QueryCoverage("a");
    ani_cmt.SetA1SubjectCoverage("a");
    ani_cmt.SetAnalysis2("a");
    ani_cmt.SetA2Genome("a");
    ani_cmt.SetA2Name("a");
    ani_cmt.SetA2ANI("a");
    ani_cmt.SetA2QueryCoverage("a");
    ani_cmt.SetA2SubjectCoverage("a");

    obj = ani_cmt.MakeUserObject();
    BOOST_CHECK_EQUAL(edit::CANIComment::IsValid(*obj), true);
    BOOST_CHECK_EQUAL(obj->GetData()[0]->GetLabel().GetStr(), "StructuredCommentPrefix");
    BOOST_CHECK_EQUAL(obj->GetData()[1]->GetLabel().GetStr(), "This Genome (query)");
    BOOST_CHECK_EQUAL(obj->GetData()[2]->GetLabel().GetStr(), "Current Name");
    BOOST_CHECK_EQUAL(obj->GetData()[3]->GetLabel().GetStr(), "Previous Name");
    BOOST_CHECK_EQUAL(obj->GetData()[4]->GetLabel().GetStr(), "Date Updated");
    BOOST_CHECK_EQUAL(obj->GetData()[5]->GetLabel().GetStr(), "Analysis Type");
    BOOST_CHECK_EQUAL(obj->GetData()[6]->GetLabel().GetStr(), "Analysis 1 (A1)");
    BOOST_CHECK_EQUAL(obj->GetData()[7]->GetLabel().GetStr(), "A1 Genome (subject)");
    BOOST_CHECK_EQUAL(obj->GetData()[8]->GetLabel().GetStr(), "A1 Name");
    BOOST_CHECK_EQUAL(obj->GetData()[9]->GetLabel().GetStr(), "A1 ANI");
    BOOST_CHECK_EQUAL(obj->GetData()[10]->GetLabel().GetStr(), "A1 Query Coverage");
    BOOST_CHECK_EQUAL(obj->GetData()[11]->GetLabel().GetStr(), "A1 Subject Coverage");
    BOOST_CHECK_EQUAL(obj->GetData()[12]->GetLabel().GetStr(), "Analysis 2 (A2)");
    BOOST_CHECK_EQUAL(obj->GetData()[13]->GetLabel().GetStr(), "A2 Genome (subject)");
    BOOST_CHECK_EQUAL(obj->GetData()[14]->GetLabel().GetStr(), "A2 Name");
    BOOST_CHECK_EQUAL(obj->GetData()[15]->GetLabel().GetStr(), "A2 ANI");
    BOOST_CHECK_EQUAL(obj->GetData()[16]->GetLabel().GetStr(), "A2 Query Coverage");
    BOOST_CHECK_EQUAL(obj->GetData()[17]->GetLabel().GetStr(), "A2 Subject Coverage");
    BOOST_CHECK_EQUAL(obj->GetData()[18]->GetLabel().GetStr(), "StructuredCommentSuffix");


    edit::CANIComment ani_cmt2(*obj);
    BOOST_CHECK_EQUAL(ani_cmt2.GetThisGenome(), "this_genome");
}


BOOST_AUTO_TEST_CASE(Test_DBLink)
{
    CRef<CUser_object> user = edit::CDBLink::MakeEmptyUserObject();
    BOOST_CHECK_EQUAL(user->GetType().GetStr(), "DBLink");

    edit::CDBLink::SetBioSample(*user, "biosample");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetLabel().GetStr(), "BioSample");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetData().GetStrs().front(), "biosample");
    vector<string> bs_vals = edit::CDBLink::GetBioSample(*user);
    BOOST_CHECK_EQUAL(bs_vals[0], "biosample");

    edit::CDBLink::SetBioProject(*user, "bioproject");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetLabel().GetStr(), "BioProject");
    BOOST_CHECK_EQUAL(user->GetData().back()->GetData().GetStrs().front(), "bioproject");
    vector<string> bp_vals = edit::CDBLink::GetBioProject(*user);
    BOOST_CHECK_EQUAL(bp_vals[0], "bioproject");


    // alternate creation method
    edit::CDBLink dblink;
    CRef<CUser_object> other_user = dblink.SetBioSample("a")
                    .SetBioProject("b")
                    .MakeUserObject();
    bs_vals = edit::CDBLink::GetBioSample(*other_user);
    BOOST_CHECK_EQUAL(bs_vals[0], "a");
    bp_vals = edit::CDBLink::GetBioProject(*user);
    BOOST_CHECK_EQUAL(bp_vals[0], "bioproject");


}


// GBBlock Fields
BOOST_AUTO_TEST_CASE(Test_GBBlock)
{

    CRef<CSeq_entry> entry = unit_test_util::BuildGoodNucProtSet();
    STANDARD_SETUP

    CBioseq_CI bi(seh, CSeq_inst::eMol_na);
    edit::CGBBlockField gb_block_field(edit::CGBBlockField::eGBBlockFieldType_Keyword);

    vector<CRef<edit::CApplyObject> > apply_objects = gb_block_field.GetApplyObjects(*bi);
    
    BOOST_CHECK_EQUAL(apply_objects.size(), 1);    
   
    gb_block_field.SetVal(apply_objects[0]->SetObject(), "my keyword", edit::eExistingText_add_qual);
    apply_objects[0]->ApplyChange();

    CSeqdesc_CI d(*bi, CSeqdesc::e_Genbank);
    if (!d) {
        BOOST_CHECK_EQUAL("Missing Genbank Block Descriptor", "Error");
    } else if (!d->GetGenbank().IsSetKeywords()) {
        BOOST_CHECK_EQUAL("Keywords not set", "Error");
    } else {
        BOOST_CHECK_EQUAL(d->GetGenbank().GetKeywords().front(), "my keyword");
    }


}


// Comment descriptors
BOOST_AUTO_TEST_CASE(Test_CommentDescriptors)
{
    CRef<CSeq_entry> entry = unit_test_util::BuildGoodNucProtSet();
    STANDARD_SETUP

    CBioseq_CI bi(seh, CSeq_inst::eMol_na);

    CRef<edit::CFieldHandler> comment_field = edit::CFieldHandlerFactory::Create("comment descriptor");
    if (!comment_field) {
        BOOST_CHECK_EQUAL("Unable to create comment field handler", "Error");
    } else {
        vector<CRef<edit::CApplyObject> > apply_objects = comment_field->GetApplyObjects(*bi);
    
        BOOST_CHECK_EQUAL(apply_objects.size(), 1);    
   
        comment_field->SetVal(apply_objects[0]->SetObject(), "my comment", edit::eExistingText_replace_old);
        apply_objects[0]->ApplyChange();

        CSeqdesc_CI d(*bi, CSeqdesc::e_Comment);
        if (!d) {
            BOOST_CHECK_EQUAL("Missing Comment Descriptor", "Error");
        } else {
            BOOST_CHECK_EQUAL(d->GetComment(), "my comment");
        }
    }
}


// Comment descriptors
BOOST_AUTO_TEST_CASE(Test_DefinitionLine)
{
    CRef<CSeq_entry> entry = unit_test_util::BuildGoodNucProtSet();
    STANDARD_SETUP

    CBioseq_CI bi(seh, CSeq_inst::eMol_na);

    CRef<edit::CFieldHandler> defline = edit::CFieldHandlerFactory::Create("definition line");
    if (!defline) {
        BOOST_CHECK_EQUAL("Unable to create definition line handler", "Error");
    } else {
        vector<CRef<edit::CApplyObject> > apply_objects = defline->GetApplyObjects(*bi);
    
        BOOST_CHECK_EQUAL(apply_objects.size(), 1);    
   
        defline->SetVal(apply_objects[0]->SetObject(), "my defline", edit::eExistingText_replace_old);
        apply_objects[0]->ApplyChange();

        CSeqdesc_CI d(*bi, CSeqdesc::e_Title);
        if (!d) {
            BOOST_CHECK_EQUAL("Missing Title Descriptor", "Error");
        } else {
            BOOST_CHECK_EQUAL(d->GetTitle(), "my defline");
        }
    }
}


// Structured comments in general
BOOST_AUTO_TEST_CASE(Test_ReorderFields)
{
    string prefix = "Genome-Assembly-Data";
    CRef<CUser_object> user = edit::CStructuredCommentField::MakeUserObject(prefix);

    CRef<CUser_field> f1(new CUser_field());
    f1->SetLabel().SetStr("Sequencing Technology");
    f1->SetData().SetStr("3");
    user->SetData().push_back(f1);
    CRef<CUser_field> f2(new CUser_field());
    f2->SetLabel().SetStr("Assembly Method");
    f2->SetData().SetStr("2");
    user->SetData().push_back(f2);
    CRef<CUser_field> f3(new CUser_field());
    f3->SetLabel().SetStr("Assembly Provider");
    f3->SetData().SetStr("1");
    user->SetData().push_back(f3);
    
    CConstRef<CComment_set> rules = CComment_set::GetCommentRules();
    const CComment_rule& rule = rules->FindCommentRule(prefix);
    BOOST_CHECK_EQUAL(rule.ReorderFields(*user), true);
    BOOST_CHECK_EQUAL(user->GetData()[0]->GetLabel().GetStr(), "StructuredCommentPrefix");
    BOOST_CHECK_EQUAL(user->GetData()[1]->GetLabel().GetStr(), "Assembly Provider");
    BOOST_CHECK_EQUAL(user->GetData()[2]->GetLabel().GetStr(), "Assembly Method");
    BOOST_CHECK_EQUAL(user->GetData()[3]->GetLabel().GetStr(), "Sequencing Technology");
    BOOST_CHECK_EQUAL(user->GetData()[4]->GetLabel().GetStr(), "StructuredCommentSuffix");
    BOOST_CHECK_EQUAL(rule.ReorderFields(*user), false);

}


BOOST_AUTO_TEST_CASE(Test_EquivalentQualifierNames)
{
    string name1("codon_start");
    string name2("codon-start");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name2, name1), true);

    name1.assign("codonstart");
    name2.assign("codon-start");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);

    name1.assign("codonstart");
    name2.assign("coDONstart");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);

    name1.assign("codon start");
    name2.assign("codon-start");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);

    name1.assign("codon start");
    name2.assign("coDONstart");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);
    
    name1.assign("codon start");
    name2.assign("   codonstart ");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);

    name1.assign("codonstart cds");
    name2.assign("   codonstart ");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), false);

    name1.assign("protein activity");
    name2.assign("  activity  ");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name2, name1), true);

    name1.assign("protein co-mment");
    name2.assign("comment");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);

    name1.assign("protein-name");
    name2.assign("protein name");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name2, name1), true);

    name1.assign("protein-name");
    name2.assign("name");
    BOOST_CHECK_EQUAL(edit::CFieldHandler::QualifierNamesAreEquivalent(name1, name2), true);
}

END_SCOPE(objects)
END_NCBI_SCOPE

