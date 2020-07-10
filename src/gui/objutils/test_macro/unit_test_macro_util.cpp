/*  $Id: unit_test_macro_util.cpp 44696 2020-02-24 16:23:45Z asztalos $
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

template<class T>
string MakeAsn(const T& object)
{
    CNcbiOstrstream str;
    str << MSerial_AsnText << object;
    return CNcbiOstrstreamToString(str);
}


BOOST_AUTO_TEST_CASE(Test_FixProteinNames)
{
    vector<string> taxnames;
    taxnames.push_back("Banana bunchy top virus");
    taxnames.push_back("Escherichia coli");
    string orig_prot("Banana Bunchy Top Virus Coat Protein");
    string new_prot = CMacroFunction_FixProteinFormat::s_FixProteinNameFormat(orig_prot, taxnames);
    BOOST_CHECK_EQUAL(new_prot, "Coat Protein");

    orig_prot.assign("Coat protein (Banana buncHY Top virus)");
    new_prot = CMacroFunction_FixProteinFormat::s_FixProteinNameFormat(orig_prot, taxnames);
    BOOST_CHECK_EQUAL(new_prot, "Coat protein");

    orig_prot.assign("Coat protein  [Banana bunchy top VIRUS] gene");
    new_prot = CMacroFunction_FixProteinFormat::s_FixProteinNameFormat(orig_prot, taxnames);
    BOOST_CHECK_EQUAL(new_prot, "Coat protein gene");
}

BOOST_AUTO_TEST_CASE(Test_EditText)
{
    string text = "Drs3 isolate and DrS3 and DRS3";
    string find = "Drs3";
    string replace = "ssss";
    bool case_sens = false;
    bool is_regex = false;
    bool res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eAnywhere, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(text, "ssss isolate and ssss and ssss");

    text = "Drs3 isolate and DrS3 and Drs3";
    case_sens = true;
    res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eAnywhere, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(text, "ssss isolate and DrS3 and ssss");

    text = "Putative terminase";
    find = "Putative";
    replace = "putative";
    case_sens = false;
    res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eAnywhere, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(text, "putative terminase");

    text = "llumina technology";
    find = "llumina";
    replace = "Illumina";
    case_sens = false;
    res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eBeginning, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(text, "Illumina technology");

    text = "some isolate";
    find = "isolate";
    replace = "";
    case_sens = true;
    res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eEnd, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(text, "some");

    text = "some isolate";
    find = "isolate";
    replace = "";
    case_sens = true;
    res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eBeginning, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, false);
    BOOST_CHECK_EQUAL(text, "some isolate");

    text = kEmptyStr;
    find = "isolate";
    replace = "";
    case_sens = true;
    res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eAnywhere, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, false);
    BOOST_CHECK_EQUAL(text, kEmptyStr);

    text = "Bubalus bubalis";
    find = "ba";
    replace = "test";
    case_sens = true;
    res = CMacroFunction_EditStringQual::s_EditText(text, find, replace, CMacroFunction_EditStringQual::eAnywhere, case_sens, is_regex);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(text, "Butestlus butestlis");
}

BOOST_AUTO_TEST_CASE(Test_MoveMiddleToFirstName)
{
    CName_std name;
    name.SetLast("Sotomayor-Gonzalez");
    name.SetFirst("Alicia");
    name.SetInitials("A.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Alicia");
    BOOST_CHECK_EQUAL(name.GetInitials(), "A.");

    name.SetFirst("Alicia");
    name.ResetInitials();
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Alicia");
    BOOST_CHECK_EQUAL(name.IsSetInitials(), false);

    name.SetFirst("Alicia");
    name.SetInitials("A.Carmen.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Alicia Carmen");
    BOOST_CHECK_EQUAL(name.GetInitials(), "A.C.");

    name.SetFirst("Alicia");
    name.SetInitials("A.Carmen");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Alicia Carmen");
    BOOST_CHECK_EQUAL(name.GetInitials(), "A.C.");

    name.SetFirst("Alicia");
    name.SetInitials("A.Carmen.Ana.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Alicia Carmen Ana");
    BOOST_CHECK_EQUAL(name.GetInitials(), "A.C.A.");

    name.SetFirst("Maria Elena");
    name.SetInitials("M.E.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Maria Elena");
    BOOST_CHECK_EQUAL(name.GetInitials(), "M.E.");

    name.SetFirst("Maria del Carmen");
    name.SetInitials("Md.C.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Maria del Carmen");
    BOOST_CHECK_EQUAL(name.GetInitials(), "Md.C.");

    name.SetFirst("Maria da Luz");
    name.SetInitials("M.d.L.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Maria da Luz");
    BOOST_CHECK_EQUAL(name.GetInitials(), "M.d.L.");

    name.SetFirst("Eloiza Lopes de Lira");
    name.SetInitials("E.Ld.L.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Eloiza Lopes de Lira");
    BOOST_CHECK_EQUAL(name.GetInitials(), "E.Ld.L.");

    name.SetFirst("Eloiza Lopes De Lira");
    name.SetInitials("E.L.D.L.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Eloiza Lopes De Lira");
    BOOST_CHECK_EQUAL(name.GetInitials(), "E.L.D.L.");

    name.SetFirst("Eloiza Lopes de Lira");
    name.SetInitials("E.Ld.L.A.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Eloiza Lopes de Lira");
    BOOST_CHECK_EQUAL(name.GetInitials(), "E.Ld.L.A.");

    name.SetFirst("Eloiza Lopes De Lira");
    name.SetInitials("E.L.D.L.A.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), false);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Eloiza Lopes De Lira");
    BOOST_CHECK_EQUAL(name.GetInitials(), "E.L.D.L.A.");

    name.SetFirst("Eloiza Lopes de Lira");
    name.SetInitials("E.Ld.L.Ana.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Eloiza Lopes de Lira Ana");
    BOOST_CHECK_EQUAL(name.GetInitials(), "E.Ld.L.A.");

    name.SetFirst("Eloiza Lopes de Lira");
    name.SetInitials("E.Ld.L.Ana");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Eloiza Lopes de Lira Ana");
    BOOST_CHECK_EQUAL(name.GetInitials(), "E.Ld.L.A.");

    name.SetFirst("Wei");
    name.SetInitials("W.Ning.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Wei Ning");
    BOOST_CHECK_EQUAL(name.GetInitials(), "W.N.");

    name.SetFirst("Wei Ning");
    name.SetInitials("W.Ning.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Wei Ning Ning");
    BOOST_CHECK_EQUAL(name.GetInitials(), "W.N.N.");

    name.SetFirst("J");
    name.SetInitials("Jiage.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "J Jiage");
    BOOST_CHECK_EQUAL(name.GetInitials(), "J.J.");

    name.ResetFirst();
    name.SetInitials("Lei.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_MoveMiddleToFirst(name), true);
    BOOST_CHECK_EQUAL(name.GetFirst(), "Lei");
    BOOST_CHECK_EQUAL(name.GetInitials(), "L.");

}

BOOST_AUTO_TEST_CASE(Test_TruncateMiddleInitials)
{
    CName_std name;
    name.SetLast("Meng");
    name.SetFirst("Xian");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), false);
    // it does not modify the field unless the field exists

    name.SetInitials("X.Jing");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), true);
    BOOST_CHECK_EQUAL(name.GetInitials(), "X.J.");

    name.SetInitials("X.Alvarez-Santos.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), true);
    BOOST_CHECK_EQUAL(name.GetInitials(), "X.A.-S.");

    name.SetInitials("X.A.-S.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), false);
    BOOST_CHECK_EQUAL(name.GetInitials(), "X.A.-S.");

    name.SetInitials("X.Alvarez.Santos.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), true);
    BOOST_CHECK_EQUAL(name.GetInitials(), "X.A.S.");

    name.SetInitials("X.AS.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), true);
    BOOST_CHECK_EQUAL(name.GetInitials(), "X.A.S.");

    name.SetInitials("X.As.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), true);
    BOOST_CHECK_EQUAL(name.GetInitials(), "X.A.");   // last letter is dropped

    name.SetFirst("Wen bing");
    name.SetInitials("Wb.AS.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_TruncateMiddleInitials(name), true);
    BOOST_CHECK_EQUAL(name.GetInitials(), "Wb.A.S.");
}

BOOST_AUTO_TEST_CASE(Test_FixInitials)
{
    CName_std stdname;
    stdname.SetLast("Freitas");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), false);

    stdname.SetFirst("Thiago");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), false);
    // initials are only fixed when that field has a value

    stdname.SetInitials(""); // empty value but field is set
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), false);

    stdname.SetInitials("Moises");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "T.M.");

    stdname.SetInitials("Moises Souza");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "T.M.S.");

    stdname.ResetFirst();
    stdname.SetInitials("Moises");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "M.");

    stdname.SetFirst("Robert");
    stdname.SetInitials("R.Alvarez Santos.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "R.A.S.");

    stdname.SetInitials("R.Alvarez Santos");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "R.A.S.");

    stdname.SetInitials("R.AS.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "R.A.S.");

    stdname.SetInitials("R.AS");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "R.A.S.");

    stdname.SetInitials("R.Alvarez-Santos.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "R.A.-S.");

    stdname.SetInitials("R.Alvarez-santos.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "R.A.-S."); // and not "R.A.-s."

    stdname.SetInitials("R.Alvarez santos.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "R.A.S."); // and not "R.As."

    stdname.SetFirst("Maria del Carmen");
    stdname.SetInitials("Md.C.Anna");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), true);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "Md.C.A.");

    stdname.SetFirst("Wen bing");
    stdname.SetInitials("Wb.");
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_FixInitials(stdname), false);
    BOOST_CHECK_EQUAL(stdname.GetInitials(), "Wb.");

}

BOOST_AUTO_TEST_CASE(Test_GetFirstNameInitials)
{
    string firstname;
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "");

    firstname = "Robert";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.");

    firstname = "Robert Alvarez";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.A.");

    firstname = "Robert-Alvarez";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.-A.");

    firstname = "Robert-alvarez";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.-a.");

    firstname = "Robert alvarez";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "Ra.");

    firstname = "Robert Alvarez Santos";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.A.S.");

    firstname = "Robert Alvarez-Santos";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.A.-S.");

    firstname = "Robert-Alvarez-Santos";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.-A.-S.");

    firstname = "Robert Alvarez-santos";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.A.-s.");

    firstname = "Robert-alvarez santos";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "R.-as.");

    firstname = "Maria del Carmen";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "Md.C.");

    firstname = "Maria de los Angeles";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "Mdl.A.");

    firstname = "Maria de los";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "Mdl.");

    firstname = "Maria de La Luz";
    BOOST_CHECK_EQUAL(CMacroFunction_AuthorFix::s_GetFirstNameInitials(firstname), "Md.L.L.");
}


BOOST_AUTO_TEST_CASE(Test_BuildName)
{
    CName_std stdname;
    string lastname = "Freitas";
    string firstname = "Robert";
    string mid_initials;  // as it appears in a dialog
    string suffix;

    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert\",\n"
        "  initials \"R.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "A";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert\",\n"
        "  initials \"R.A.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "Alvarez Santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert\",\n"
        "  initials \"R.A.S.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "AS";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert\",\n"
        "  initials \"R.A.S.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "Alvarez-Santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert\",\n"
        "  initials \"R.A.-S.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "Alvarez-santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert\",\n"
        "  initials \"R.A.-S.\"\n"  // and not "R.A.-s"
        "}\n");

    stdname.Reset();
    mid_initials.clear();
    firstname = "Robert Alvarez Santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert Alvarez Santos\",\n"
        "  initials \"R.A.S.\"\n"
        "}\n");

    stdname.Reset();
    firstname = "Robert Alvarez-Santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert Alvarez-Santos\",\n"
        "  initials \"R.A.-S.\"\n"
        "}\n");

    stdname.Reset();
    firstname = "Robert Alvarez santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert Alvarez santos\",\n"
        "  initials \"R.As.\"\n"
        "}\n");

    stdname.Reset();
    firstname = "Robert Alvarez-santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert Alvarez-santos\",\n"
        "  initials \"R.A.-s.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "Jose";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert Alvarez-santos\",\n"
        "  initials \"R.A.-s.J.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "Jose";
    firstname = "Robert Alvarez Santos";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Robert Alvarez Santos\",\n"
        "  initials \"R.A.S.J.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials.clear();
    firstname = "Maria del Carmen";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria del Carmen\",\n"
        "  initials \"Md.C.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "Anna";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria del Carmen\",\n"
        "  initials \"Md.C.A.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials.clear();
    firstname = "Maria de los Angeles";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria de los Angeles\",\n"
        "  initials \"Mdl.A.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials = "A";
    firstname = "Maria de los Angeles";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria de los Angeles\",\n"
        "  initials \"Mdl.A.A.\"\n"
        "}\n");

    stdname.Reset();
    mid_initials.clear();
    firstname = "Maria de La Luz";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria de La Luz\",\n"
        "  initials \"Md.L.L.\"\n"
        "}\n");

    stdname.Reset();
    firstname = "Maria de los";
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria de los\",\n"
        "  initials \"Mdl.\"\n"
        "}\n");

    stdname.Reset();
    firstname.clear();
    CMacroFunction_AuthorFix::s_BuildName(firstname, mid_initials, lastname, suffix, stdname);
    BOOST_CHECK_EQUAL(MakeAsn(stdname),
        "Name-std ::= {\n"
        "  last \"Freitas\"\n"
        "}\n");
}

          
// to see the messages, run the command with --log_level=message option
BOOST_AUTO_TEST_CASE(Test_ApplyMiddleInitial)
{
    CName_std stdname;
    stdname.SetLast("Freitas");
    BOOST_TEST_MESSAGE("Apply middle initial to a name with a last name:");

    BOOST_TEST_MESSAGE("when the first name and initials fields are not set");
    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetInitials() == "M.");

    stdname.SetFirst("Adam");
    {
        stdname.ResetInitials();
        BOOST_TEST_MESSAGE("and a first name and the initials field is not set");
        BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_replace_old));
        BOOST_CHECK(stdname.GetInitials() == "A.M.");

        stdname.ResetInitials();
        stdname.SetInitials("A.");
        BOOST_TEST_MESSAGE("and a first name and the first initial is set (replace old)");
        BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_replace_old));
        BOOST_CHECK(stdname.GetInitials() == "A.M.");

        stdname.ResetInitials();
        stdname.SetInitials("A.");
        BOOST_TEST_MESSAGE("and a first name and the first initial is set (append with no delimiter)");
        BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_append_none));
        BOOST_CHECK(stdname.GetInitials() == "A.M.");

        stdname.ResetInitials();
        stdname.SetInitials("A.B.");
        BOOST_TEST_MESSAGE("and a first name and a middle initial");
        BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_replace_old));
        BOOST_CHECK(stdname.GetInitials() == "A.M.");

        stdname.ResetInitials();
        stdname.SetInitials("A.B.E.");
        BOOST_TEST_MESSAGE("and a first name and two middle initials");
        BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_replace_old));
        BOOST_CHECK(stdname.GetInitials() == "A.M.");

    }

    stdname.ResetFirst();
    {
        stdname.ResetInitials();
        stdname.SetInitials("A.");
        BOOST_TEST_MESSAGE("with no first name and a middle initial");
        BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_replace_old));
        BOOST_CHECK(stdname.GetInitials() == "M.");

        stdname.ResetInitials();
        stdname.SetInitials("A.");
        BOOST_TEST_MESSAGE("with no first name and a middle initial (leave old)");
        BOOST_CHECK(!NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_leave_old));
        BOOST_CHECK(stdname.GetInitials() == "A.");

        stdname.ResetInitials();
        stdname.SetInitials("A.");
        BOOST_TEST_MESSAGE("with no first name and a middle initial (cancel)");
        BOOST_CHECK(!NMacroUtil::ApplyMiddleInitial(stdname, "M", edit::eExistingText_cancel));
        BOOST_CHECK(stdname.GetInitials() == "A.");
    }
}

BOOST_AUTO_TEST_CASE(Test_ApplyMiddleInitial_WithDot)
{
    CName_std stdname;
    stdname.SetLast("Freitas");
    BOOST_TEST_MESSAGE("Dot at the end of the new value will be silenced");

    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M.", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetInitials() == "M.");

    stdname.ResetInitials();
    stdname.SetFirst("Adam");
    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M.", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetInitials() == "A.M.");

    stdname.ResetInitials();
    stdname.SetInitials("A.");
    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M.", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetInitials() == "A.M.");

    stdname.ResetInitials();
    stdname.SetInitials("A.");
    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M.", edit::eExistingText_append_none));
    BOOST_CHECK(stdname.GetInitials() == "A.M.");

    stdname.ResetInitials();
    stdname.SetInitials("A.B.");
    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M.", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetInitials() == "A.M.");

    stdname.ResetInitials();
    stdname.SetInitials("A.B.E.");
    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M.", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetInitials() == "A.M.");

    stdname.ResetInitials();
    stdname.ResetFirst();
    stdname.SetInitials("A.");
    BOOST_CHECK(NMacroUtil::ApplyMiddleInitial(stdname, "M.", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetInitials() == "M.");
}

BOOST_AUTO_TEST_CASE(Test_ApplyFirstName)
{
    CName_std stdname;
    stdname.SetLast("Benites");
    BOOST_TEST_MESSAGE("Apply first name to a name with a last name:");

    BOOST_TEST_MESSAGE("when the first name and initials fields are not set");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Maria", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetFirst() == "Maria");
    BOOST_CHECK(stdname.GetInitials() == "M.");

    stdname.ResetFirst();
    stdname.SetInitials("M.");
    BOOST_TEST_MESSAGE("when the first name is not set but the initials fields is set");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Adam", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetFirst() == "Adam");
    BOOST_CHECK(stdname.GetInitials() == "A.M.");

    stdname.ResetFirst();
    stdname.SetInitials("M.");
    BOOST_TEST_MESSAGE("when the first name is not set but the initials fields is set");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Maria", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetFirst() == "Maria");
    BOOST_CHECK(stdname.GetInitials() == "M.M.");

    stdname.ResetInitials();
    stdname.SetFirst("Adam");
    BOOST_TEST_MESSAGE("when there is a first name but the initials fields is not set");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Maria", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetFirst() == "Maria");
    BOOST_CHECK(stdname.GetInitials() == "M.");


    stdname.ResetInitials();
    stdname.SetFirst("Adam");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Marc", edit::eExistingText_append_space));
    BOOST_CHECK(stdname.GetFirst() == "Adam Marc");
    BOOST_CHECK(stdname.GetInitials() == "A.M.");

    stdname.SetFirst("Adam");
    stdname.SetInitials("A.");
    BOOST_TEST_MESSAGE("when there is a first name and no middle initial");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Marc", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetFirst() == "Marc");
    BOOST_CHECK(stdname.GetInitials() == "M.");

    stdname.SetFirst("Adam");
    stdname.SetInitials("A.L.");
    BOOST_TEST_MESSAGE("when there is a first name and there is a middle initial");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Marc", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetFirst() == "Marc");
    BOOST_CHECK(stdname.GetInitials() == "M.L.");

    stdname.SetFirst("Adam");
    stdname.SetInitials("A.A.Linda.");
    BOOST_TEST_MESSAGE("when there is a first name and there is a middle initial");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Marc", edit::eExistingText_replace_old));
    BOOST_CHECK(stdname.GetFirst() == "Marc");
    BOOST_CHECK(stdname.GetInitials() == "M.A.L.");

    stdname.SetFirst("Adam");
    stdname.SetInitials("A.L.");
    BOOST_CHECK(NMacroUtil::ApplyFirstName(stdname, "Marc", edit::eExistingText_append_space));
    BOOST_CHECK(stdname.GetFirst() == "Adam Marc");
    BOOST_CHECK(stdname.GetInitials() == "A.M.L.");
}

BOOST_AUTO_TEST_CASE(Test_RemoveMiddleInitial)
{
    CName_std stdname;
    stdname.SetLast("Freitas");
    BOOST_TEST_MESSAGE("Remove middle initial from a name with a last name:");

    stdname.ResetFirst();
    {
        BOOST_TEST_MESSAGE("when the first name and initials fields are not set");
        BOOST_CHECK(!NMacroUtil::RemoveMiddleInitial(stdname));
        BOOST_CHECK(!stdname.IsSetInitials());

        stdname.SetInitials("M.");
        BOOST_TEST_MESSAGE("with no first name and a middle initial");
        BOOST_CHECK(NMacroUtil::RemoveMiddleInitial(stdname));
        BOOST_CHECK(!stdname.IsSetInitials());
    }

    stdname.SetFirst("Adam");
    {
        stdname.ResetInitials();
        BOOST_TEST_MESSAGE("and a first name and the initials field is not set");
        BOOST_CHECK(!NMacroUtil::RemoveMiddleInitial(stdname));
        BOOST_CHECK(!stdname.IsSetInitials());

        stdname.ResetInitials();
        stdname.SetInitials("A.");
        BOOST_TEST_MESSAGE("and a first name and the initials field is set");
        BOOST_CHECK(!NMacroUtil::RemoveMiddleInitial(stdname));
        BOOST_CHECK(stdname.IsSetInitials());

        stdname.ResetInitials();
        stdname.SetInitials("A.M.");
        BOOST_TEST_MESSAGE("and a first name and a middle initial");
        BOOST_CHECK(NMacroUtil::RemoveMiddleInitial(stdname));
        BOOST_CHECK(stdname.GetInitials() == "A.");

        stdname.ResetInitials();
        stdname.SetInitials("A.M.Bn.R.");
        BOOST_TEST_MESSAGE("and a first name and multiple middle initials");
        BOOST_CHECK(NMacroUtil::RemoveMiddleInitial(stdname));
        BOOST_CHECK(stdname.GetInitials() == "A.");
    }

}

BOOST_AUTO_TEST_CASE(Test_RemoveFirstName)
{
    CName_std stdname;
    stdname.SetLast("Freitas");
    BOOST_TEST_MESSAGE("Remove first name from a name with a last name:");
    BOOST_CHECK(!NMacroUtil::RemoveFirstName(stdname));

    stdname.SetFirst("Adam");
    BOOST_TEST_MESSAGE("Remove first name when there is a first name:");
    BOOST_CHECK(NMacroUtil::RemoveFirstName(stdname));
    BOOST_CHECK(!stdname.IsSetFirst());
    BOOST_CHECK(!stdname.IsSetInitials());

    stdname.SetFirst("Adam");
    stdname.SetInitials("A.");
    BOOST_CHECK(NMacroUtil::RemoveFirstName(stdname));
    BOOST_CHECK(!stdname.IsSetFirst());
    BOOST_CHECK(!stdname.IsSetInitials());

    stdname.SetFirst("Adam");
    stdname.SetInitials("A.M.K.");
    BOOST_CHECK(NMacroUtil::RemoveFirstName(stdname));
    BOOST_CHECK(!stdname.IsSetFirst());
    BOOST_CHECK(stdname.GetInitials() == "M.K.");

    stdname.SetFirst("Adam John");
    stdname.ResetInitials();
    BOOST_CHECK(NMacroUtil::RemoveFirstName(stdname));
    BOOST_CHECK(!stdname.IsSetFirst());
    BOOST_CHECK(!stdname.IsSetInitials());

    stdname.SetFirst("Adam John");
    stdname.SetInitials("A.J.");
    BOOST_CHECK(NMacroUtil::RemoveFirstName(stdname));
    BOOST_CHECK(!stdname.IsSetFirst());
    BOOST_CHECK(!stdname.IsSetInitials());

    stdname.SetFirst("Maria de La Luz");
    stdname.SetInitials("Md.L.L.");
    BOOST_CHECK(NMacroUtil::RemoveFirstName(stdname));
    BOOST_CHECK(!stdname.IsSetFirst());
    BOOST_CHECK(!stdname.IsSetInitials());

    stdname.SetFirst("Maria de La Luz");
    stdname.SetInitials("Md.L.L.M.");
    BOOST_CHECK(NMacroUtil::RemoveFirstName(stdname));
    BOOST_CHECK(!stdname.IsSetFirst());
    BOOST_CHECK(stdname.GetInitials() == "M.");

}

BOOST_AUTO_TEST_CASE(Test_ReverseAuthorNames)
{
    CName_std name;
    name.SetLast("Freitas");
    BOOST_TEST_MESSAGE("Reverse names when the first name is missing");
    BOOST_CHECK(!CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    
    name.SetFirst("Albert");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert\",\n"
        "  first \"Freitas\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Albert");
    name.SetInitials("A.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Albert Bela");
    name.SetInitials("A.B.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert Bela\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Albert Bela");
    name.SetInitials("A.B.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert Bela\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Albert");
    name.SetInitials("A.Gabriel");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.G.\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Albert");
    name.SetInitials("A.G.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.G.\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Albert Bela");
    name.SetInitials("A.B.Geza");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert Bela\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.G.\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Albert Bela");
    name.SetInitials("A.B.B.R.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Albert Bela\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.B.R.\"\n"
        "}\n");

    
    name.SetLast("Freitas");
    name.SetFirst("Maria de La Luz");
    name.ResetInitials();
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Maria de La Luz\",\n"
        "  first \"Freitas\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Maria de La Luz");
    name.SetInitials("Anna");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Maria de La Luz\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.A.\"\n"
        "}\n");


    name.SetLast("Freitas");
    name.SetFirst("Maria de La Luz");
    name.SetInitials("A.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Maria de La Luz\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.A.\"\n"
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Maria de La Luz");
    name.SetInitials("Md.L.L.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Maria de La Luz\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.\"\n" 
        "}\n");

    name.SetLast("Freitas");
    name.SetFirst("Maria de La Luz");
    name.SetInitials("Md.L.L.A");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Maria de La Luz\",\n"
        "  first \"Freitas\",\n"
        "  initials \"F.A.\"\n"
        "}\n");

    name.SetLast("Maria de La Luz");
    name.SetFirst("Freitas");
    name.SetInitials("F.");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria de La Luz\",\n"
        "  initials \"Md.L.L.\"\n"
        "}\n");

    name.SetLast("Maria de los");
    name.SetFirst("Freitas");
    name.SetInitials("F.Carmen");
    BOOST_CHECK(CMacroFunction_AuthorFix::s_ReverseAuthorNames(name));
    BOOST_CHECK_EQUAL(MakeAsn(name),
        "Name-std ::= {\n"
        "  last \"Freitas\",\n"
        "  first \"Maria de los\",\n"
        "  initials \"Mdl.C.\"\n" 
        "}\n");
}


BOOST_AUTO_TEST_CASE(Test_AddAuthorList)
{
    CAuth_list auth_list;
    string authors;
    BOOST_CHECK_EQUAL(NMacroUtil::ApplyAuthorNames(auth_list, authors), 0);

    authors = "Jeffrey M. Lorch";
    BOOST_CHECK_EQUAL(NMacroUtil::ApplyAuthorNames(auth_list, authors), 1);
    BOOST_CHECK_EQUAL(MakeAsn(auth_list),
        "Auth-list ::= {\n"
        "  names std {\n"
        "    {\n"
        "      name name {\n"
        "        last \"Lorch\",\n"
        "        first \"Jeffrey\",\n"
        "        initials \"J.M.\"\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n");


    authors = "Jeffrey M. Lorch, Jonathan M. Palmer";
    BOOST_CHECK_EQUAL(NMacroUtil::ApplyAuthorNames(auth_list, authors), 2);
    BOOST_CHECK_EQUAL(MakeAsn(auth_list),
        "Auth-list ::= {\n"
        "  names std {\n"
        "    {\n"
        "      name name {\n"
        "        last \"Lorch\",\n"
        "        first \"Jeffrey\",\n"
        "        initials \"J.M.\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      name name {\n"
        "        last \"Palmer\",\n"
        "        first \"Jonathan\",\n"
        "        initials \"J.M.\"\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n");
    
    // Observe that Suffix is only be parsed correctly when four names are specified
    authors = "Anne Ballmann, George, Kathryn Griffin Jr., Susan Knowles Maria, ";
    BOOST_CHECK_EQUAL(NMacroUtil::ApplyAuthorNames(auth_list, authors), 4);
    BOOST_CHECK_EQUAL(MakeAsn(auth_list),
        "Auth-list ::= {\n"
        "  names std {\n"
        "    {\n"
        "      name name {\n"
        "        last \"Ballmann\",\n"
        "        first \"Anne\",\n"
        "        initials \"A.\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      name name {\n"
        "        last \"George\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      name name {\n"
        "        last \"Jr.\",\n"   // the intended suffix ends up being the last name
        "        first \"Kathryn\",\n"
        "        initials \"K.G.\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      name name {\n"
        "        last \"Maria\",\n"
        "        first \"Susan\",\n"
        "        initials \"S.K.\"\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n");
}

BOOST_AUTO_TEST_CASE(Test_EscapedChar)
{
    string str;
    BOOST_CHECK(NMacroUtil::TransformForCSV(str).empty());

    str = "apple";
    BOOST_CHECK(NMacroUtil::TransformForCSV(str) == "apple");

    str = "apple, orange";
    BOOST_CHECK(NMacroUtil::TransformForCSV(str) == "\"apple, orange\"");

    str = "apple \"orange\"";
    BOOST_CHECK(NMacroUtil::TransformForCSV(str) == "\"apple \"\"orange\"\"\"");

    str = "apple, \"pear";
    BOOST_CHECK(NMacroUtil::TransformForCSV(str) == "\"apple, \"\"pear\"");

    str = "apple\npear";
    BOOST_CHECK(NMacroUtil::TransformForCSV(str) == "\"apple\npear\"");
}

BOOST_AUTO_TEST_CASE(Test_MakeMobileElement)
{
    string str = CMacroFunction_MobileElementType::s_MakeMobileElementT("", "");
    BOOST_CHECK(str.empty());
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTType(str) == "");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTName(str) == "");

    str = CMacroFunction_MobileElementType::s_MakeMobileElementT("integron", "");
    BOOST_CHECK(str == "integron");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTType(str) == "integron");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTName(str) == "");

    str = CMacroFunction_MobileElementType::s_MakeMobileElementT("", "name");
    BOOST_CHECK(str == "name");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTType(str) == "");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTName(str) == "name");

    str = CMacroFunction_MobileElementType::s_MakeMobileElementT("other", "name");
    BOOST_CHECK(str == "other:name");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTType(str) == "other");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTName(str) == "name");

    // a bad type will not be inserted
    str = CMacroFunction_MobileElementType::s_MakeMobileElementT("apple", "");
    BOOST_CHECK(str.empty());

    str = CMacroFunction_MobileElementType::s_MakeMobileElementT("", "apple");
    BOOST_CHECK(str == "apple");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTType(str) == "");
    BOOST_CHECK(CMacroFunction_MobileElementType::s_GetMobileElemTName(str) == "apple");
}

