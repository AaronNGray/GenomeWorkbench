/*  $Id: seq_loc_unit_test.cpp 475416 2015-08-07 17:47:39Z vasilche $
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
 * Author:  Eugene Vasilchenko, NCBI
 *
 * File Description:
 *   Unit test for CSeq_loc and some closely related code
 *
 * ===========================================================================
 */
#define NCBI_TEST_APPLICATION
#include <ncbi_pch.hpp>

#include <objects/seqloc/seqloc__.hpp>

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbithr.hpp>
#include <util/random_gen.hpp>
#include <corelib/test_boost.hpp>

#include <boost/test/parameterized_test.hpp>

#include <common/test_assert.h>  /* This header must go last */

USING_NCBI_SCOPE;
USING_SCOPE(objects);


CRef<CSeq_loc> MakeLoc(const char* text)
{
    string input = text;
    if ( input.find("::=") == NPOS ) {
        input = "Seq-loc::="+input;
    }
    CNcbiIstrstream str(input.c_str());
    CRef<CSeq_loc> loc(new CSeq_loc);
    str >> MSerial_AsnText >> *loc;
    return loc;
}


template<class Obj>
string MakeASN(const Obj& loc)
{
    CNcbiOstrstream str;
    str << MSerial_AsnText << loc;
    return CNcbiOstrstreamToString(str);
}


BOOST_AUTO_TEST_CASE(TestSingle)
{
    CRef<CSeq_loc> loc =
        MakeLoc("int { from 10, to 20, id gi 2 }");
    
    CSeq_loc_CI it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(2));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 20));
    BOOST_CHECK(!it.IsSetStrand());
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 20,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);
}


BOOST_AUTO_TEST_CASE(TestDouble)
{
    CRef<CSeq_loc> loc =
        MakeLoc("mix {"
                " int { from 10, to 20, strand plus, id gi 2 },"
                " pnt { point 30, strand minus, id gi 3}"
                "}");
    
    CSeq_loc_CI it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(2));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 20));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 20,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(3));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);
}


BOOST_AUTO_TEST_CASE(TestEquiv)
{
    CRef<CSeq_loc> loc =
        MakeLoc("mix {"
                " int { from 10, to 20, strand plus, id gi 2 },"
                " equiv {"
                "  int { from 25, to 27, strand plus, id gi 2 },"
                "  mix {"
                "   int { from 25, to 26, strand plus, id gi 2 },"
                "   int { from 27, to 27, strand minus, id gi 2 }"
                "  }"
                " },"
                " pnt { point 30, strand minus, id gi 3}"
                "}");
    
    CSeq_loc_CI it(*loc);
    BOOST_CHECK(it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(2));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 20));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 20,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(2));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(25, 27));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 2u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 25,\n"
                      "  to 27,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(2));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(25, 26));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 2u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 25,\n"
                      "  to 26,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(2));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(27, 27));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 2u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 27,\n"
                      "  to 27,\n"
                      "  strand minus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(3));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);
}


BOOST_AUTO_TEST_CASE(TestEdit1)
{
    CRef<CSeq_loc> loc =
        MakeLoc("mix {"
                " int { from 10, to 20, strand plus, id gi 2 },"
                " pnt { point 30, strand minus, id gi 3}"
                "}");
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(2));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 20));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 20,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    it.SetTo(10);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 10,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(3));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    it.SetSeq_id_Handle(CSeq_id_Handle::GetGiHandle(TIntId(2)));
    it.SetStrand(eNa_strand_plus);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc());
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= packed-pnt {\n"
                      "  strand plus,\n"
                      "  id gi 2,\n"
                      "  points {\n"
                      "    10,\n"
                      "    30\n"
                      "  }\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestEdit2)
{
    CRef<CSeq_loc> loc =
        MakeLoc("mix {"
                " int { from 10, to 20, strand plus, id gi 2 },"
                " pnt { point 30, strand minus, id gi 3}"
                "}");
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 20));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 20,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    it.SetPoint(10);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 10,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(3)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    it.SetSeq_id_Handle(CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc());
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= packed-int {\n"
                      "  {\n"
                      "    from 10,\n"
                      "    to 10,\n"
                      "    strand plus,\n"
                      "    id gi 2\n"
                      "  },\n"
                      "  {\n"
                      "    from 30,\n"
                      "    to 30,\n"
                      "    strand minus,\n"
                      "    id gi 2\n"
                      "  }\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestEdit3)
{
    CRef<CSeq_loc> loc =
        MakeLoc("mix {"
                " int { from 10, to 20, strand plus, id gi 2 },"
                " pnt { point 30, strand minus, id gi 3}"
                "}");
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 20));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 20,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    it.SetTo(10);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 10,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(3)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    it.SetSeq_id_Handle(CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(it.eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "  int {\n"
                      "    from 10,\n"
                      "    to 10,\n"
                      "    strand plus,\n"
                      "    id gi 2\n"
                      "  },\n"
                      "  pnt {\n"
                      "    point 30,\n"
                      "    strand minus,\n"
                      "    id gi 2\n"
                      "  }\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestBond)
{
    CRef<CSeq_loc> loc =
        MakeLoc("bond {"
                " a { point 10, strand plus, id gi 2 },"
                " b { point 30, strand minus, id gi 3}"
                "}");
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 10));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(it.IsInBond());
    BOOST_CHECK(it.IsBondA());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 10,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(3)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(it.IsInBond());
    BOOST_CHECK(it.IsBondB());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(it.eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= bond {\n"
                      "  a {\n"
                      "    point 10,\n"
                      "    strand plus,\n"
                      "    id gi 2\n"
                      "  },\n"
                      "  b {\n"
                      "    point 30,\n"
                      "    strand minus,\n"
                      "    id gi 3\n"
                      "  }\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeBond)
{
    CRef<CSeq_loc> loc =
        MakeLoc("mix {"
                " pnt { point 10, strand plus, id gi 2 },"
                " pnt { point 30, strand minus, id gi 3},"
                " pnt { point 40, id gi 4}"
                "}");
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 10));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 10,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(3)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(4)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(40, 40));
    BOOST_CHECK(!it.IsSetStrand());
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 40,\n"
                      "  id gi 4\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    it.SetPos(0);
    it.MakeBondAB();
    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 10));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 10,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(3)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(4)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(40, 40));
    BOOST_CHECK(!it.IsSetStrand());
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 40,\n"
                      "  id gi 4\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(it.eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "  bond {\n"
                      "    a {\n"
                      "      point 10,\n"
                      "      strand plus,\n"
                      "      id gi 2\n"
                      "    },\n"
                      "    b {\n"
                      "      point 30,\n"
                      "      strand minus,\n"
                      "      id gi 3\n"
                      "    }\n"
                      "  },\n"
                      "  pnt {\n"
                      "    point 40,\n"
                      "    id gi 4\n"
                      "  }\n"
                      "}\n");

    it.SetPos(0);
    it.RemoveBond();
    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 10));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 10,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(3)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(4)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(40, 40));
    BOOST_CHECK(!it.IsSetStrand());
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 40,\n"
                      "  id gi 4\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    loc2 = MakeASN(*it.MakeSeq_loc(it.eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "  pnt {\n"
                      "    point 10,\n"
                      "    strand plus,\n"
                      "    id gi 2\n"
                      "  },\n"
                      "  pnt {\n"
                      "    point 30,\n"
                      "    strand minus,\n"
                      "    id gi 3\n"
                      "  },\n"
                      "  pnt {\n"
                      "    point 40,\n"
                      "    id gi 4\n"
                      "  }\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeEquiv1)
{
    CRef<CSeq_loc> loc =
        MakeLoc("mix {"
                " int { from 10, to 20, strand plus, id gi 2 },"
                " equiv {"
                "  int { from 25, to 27, strand plus, id gi 2 },"
                "  mix {"
                "   int { from 25, to 26, strand plus, id gi 2 },"
                "   int { from 27, to 27, strand minus, id gi 2 }"
                "  }"
                " },"
                " pnt { point 30, strand minus, id gi 3}"
                "}");
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(10, 20));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 10,\n"
                      "  to 20,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(25, 27));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 2u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 25,\n"
                      "  to 27,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(25, 26));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 2u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 25,\n"
                      "  to 26,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(27, 27));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 2u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 4u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 27,\n"
                      "  to 27,\n"
                      "  strand minus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(3)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(30, 30));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(!it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= pnt {\n"
                      "  point 30,\n"
                      "  strand minus,\n"
                      "  id gi 3\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "  int {\n"
                      "    from 10,\n"
                      "    to 20,\n"
                      "    strand plus,\n"
                      "    id gi 2\n"
                      "  },\n"
                      "  equiv {\n"
                      "    int {\n"
                      "      from 25,\n"
                      "      to 27,\n"
                      "      strand plus,\n"
                      "      id gi 2\n"
                      "    },\n"
                      "    mix {\n"
                      "      int {\n"
                      "        from 25,\n"
                      "        to 26,\n"
                      "        strand plus,\n"
                      "        id gi 2\n"
                      "      },\n"
                      "      int {\n"
                      "        from 27,\n"
                      "        to 27,\n"
                      "        strand minus,\n"
                      "        id gi 2\n"
                      "      }\n"
                      "    }\n"
                      "  },\n"
                      "  pnt {\n"
                      "    point 30,\n"
                      "    strand minus,\n"
                      "    id gi 3\n"
                      "  }\n"
                      "}\n");
    loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_CompactType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "  int {\n"
                      "    from 10,\n"
                      "    to 20,\n"
                      "    strand plus,\n"
                      "    id gi 2\n"
                      "  },\n"
                      "  equiv {\n"
                      "    int {\n"
                      "      from 25,\n"
                      "      to 27,\n"
                      "      strand plus,\n"
                      "      id gi 2\n"
                      "    },\n"
                      "    packed-int {\n"
                      "      {\n"
                      "        from 25,\n"
                      "        to 26,\n"
                      "        strand plus,\n"
                      "        id gi 2\n"
                      "      },\n"
                      "      {\n"
                      "        from 27,\n"
                      "        to 27,\n"
                      "        strand minus,\n"
                      "        id gi 2\n"
                      "      }\n"
                      "    }\n"
                      "  },\n"
                      "  pnt {\n"
                      "    point 30,\n"
                      "    strand minus,\n"
                      "    id gi 3\n"
                      "  }\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeEquiv2)
{
    CRef<CSeq_loc> loc =
        MakeLoc("equiv {"
                " equiv {"
                "  int { from 25, to 27, strand plus, id gi 2 },"
                "  mix {"
                "   int { from 25, to 26, strand plus, id gi 2 },"
                "   int { from 27, to 27, strand minus, id gi 2 }"
                "  }"
                " }"
                "}");
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(it.HasEquivSets());

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(25, 27));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 2u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 0u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 3u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 0u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 1u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 25,\n"
                      "  to 27,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(25, 26));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_plus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(!it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 2u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 0u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 3u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 3u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 25,\n"
                      "  to 26,\n"
                      "  strand plus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_REQUIRE(it);
    BOOST_CHECK_EQUAL(it.GetSeq_id_Handle(), CSeq_id_Handle::GetGiHandle(TIntId(2)));
    BOOST_CHECK(it.GetSeq_id().IsGi());
    BOOST_CHECK_EQUAL(it.GetRange(), CRange<TSeqPos>(27, 27));
    BOOST_CHECK(it.IsSetStrand());
    BOOST_CHECK_EQUAL(int(it.GetStrand()), int(eNa_strand_minus));
    BOOST_CHECK(!it.IsWhole());
    BOOST_CHECK(!it.IsEmpty());
    BOOST_CHECK(it.IsPoint());
    BOOST_CHECK(it.IsInEquivSet());
    BOOST_CHECK(!it.IsInBond());
    BOOST_CHECK_EQUAL(it.GetEquivSetsCount(), 2u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).first.GetPos(), 0u);
    BOOST_CHECK_EQUAL(it.GetEquivSetRange(0).second.GetPos(), 3u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).first.GetPos(), 1u);
    BOOST_CHECK_EQUAL(it.GetEquivPartRange(0).second.GetPos(), 3u);
    BOOST_CHECK_EQUAL(MakeASN(*it.GetRangeAsSeq_loc()),
                      "Seq-loc ::= int {\n"
                      "  from 27,\n"
                      "  to 27,\n"
                      "  strand minus,\n"
                      "  id gi 2\n"
                      "}\n");
    ++it;

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= equiv {\n"
                      "  equiv {\n"
                      "    int {\n"
                      "      from 25,\n"
                      "      to 27,\n"
                      "      strand plus,\n"
                      "      id gi 2\n"
                      "    },\n"
                      "    mix {\n"
                      "      int {\n"
                      "        from 25,\n"
                      "        to 26,\n"
                      "        strand plus,\n"
                      "        id gi 2\n"
                      "      },\n"
                      "      int {\n"
                      "        from 27,\n"
                      "        to 27,\n"
                      "        strand minus,\n"
                      "        id gi 2\n"
                      "      }\n"
                      "    }\n"
                      "  }\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeEquiv3)
{
    CRef<CSeq_loc> loc =
        MakeLoc("equiv {"
                " equiv {"
                " }"
                "}");
    // empty equiv is omitted

    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeEquiv4)
{
    CRef<CSeq_loc> loc =
        MakeLoc("equiv {"
                " equiv {"
                "   mix {"
                "   }"
                " }"
                "}");
    // empty equiv is omitted
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeEquiv5)
{
    CRef<CSeq_loc> loc =
        MakeLoc("equiv {"
                " equiv {"
                "   mix {"
                "   },"
                "   mix {"
                "   }"
                " }"
                "}");
    // empty parts and empty equivs are omitted
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeEquiv6)
{
    CRef<CSeq_loc> loc =
        MakeLoc("equiv {"
                " equiv {"
                "   mix {"
                "   }"
                " },"
                " mix {"
                " }"
                "}");
    // empty parts and empty equivs are omitted
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "}\n");
}


BOOST_AUTO_TEST_CASE(TestMakeEquiv7)
{
    CRef<CSeq_loc> loc =
        MakeLoc("equiv {"
                " equiv {"
                "   mix {"
                "   }"
                " },"
                " mix {"
                " }"
                "}");
    // empty parts and empty equivs are omitted
    
    CSeq_loc_I it(*loc);
    BOOST_CHECK(!it.HasEquivSets());

    BOOST_CHECK(!it);

    string loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "}\n");

    it.SetEquivMode(it.eEquiv_new_equiv);
    it.InsertInterval(CSeq_id_Handle::GetGiHandle(TIntId(10)), 100, 200);
    it.SetEquivMode(it.eEquiv_new_part);
    it.InsertInterval(CSeq_id_Handle::GetGiHandle(TIntId(10)), 100, 149);
    it.InsertInterval(CSeq_id_Handle::GetGiHandle(TIntId(10)), 150, 200, eNa_strand_minus);

    loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= equiv {\n"
                      "  int {\n"
                      "    from 100,\n"
                      "    to 200,\n"
                      "    id gi 10\n"
                      "  },\n"
                      "  mix {\n"
                      "    int {\n"
                      "      from 100,\n"
                      "      to 149,\n"
                      "      id gi 10\n"
                      "    },\n"
                      "    int {\n"
                      "      from 150,\n"
                      "      to 200,\n"
                      "      strand minus,\n"
                      "      id gi 10\n"
                      "    }\n"
                      "  }\n"
                      "}\n");

    it.SetEquivMode(it.eEquiv_none);
    it.InsertInterval(CSeq_id_Handle::GetGiHandle(TIntId(10)), 201, 300, eNa_strand_plus);

    loc2 = MakeASN(*it.MakeSeq_loc(CSeq_loc_I::eMake_PreserveType));
    BOOST_CHECK_EQUAL(loc2,
                      "Seq-loc ::= mix {\n"
                      "  equiv {\n"
                      "    int {\n"
                      "      from 100,\n"
                      "      to 200,\n"
                      "      id gi 10\n"
                      "    },\n"
                      "    mix {\n"
                      "      int {\n"
                      "        from 100,\n"
                      "        to 149,\n"
                      "        id gi 10\n"
                      "      },\n"
                      "      int {\n"
                      "        from 150,\n"
                      "        to 200,\n"
                      "        strand minus,\n"
                      "        id gi 10\n"
                      "      }\n"
                      "    }\n"
                      "  },\n"
                      "  int {\n"
                      "    from 201,\n"
                      "    to 300,\n"
                      "    strand plus,\n"
                      "    id gi 10\n"
                      "  }\n"
                      "}\n");
}


#ifdef NCBI_THREADS

typedef vector< CRef<CSeq_loc> > TSeqLocs;

DEFINE_STATIC_MUTEX(s_BoostMutex);

class CTotalRangeThread : public CThread
{
public:
    CTotalRangeThread(const TSeqLocs& locs, CAtomicCounter& start_counter)
        : m_Locs(locs), m_StartCounter(start_counter)
        {
        }

    static void x_CheckPoint(CSeq_loc::TRange range, TSeqPos point)
        {
            if ( range.GetFrom() != point || range.GetTo() != point ) {
                CMutexGuard guard(s_BoostMutex);
                BOOST_REQUIRE_EQUAL(range, CSeq_loc::TRange(point, point));
            }
        }

    virtual void* Main(void)
        {
            m_StartCounter.Add(-1);
            // wait for all threads to start
            while ( m_StartCounter.Get() != 0 ) {
            }

            const size_t LCOUNT = m_Locs.size();
            for ( TSeqPos i = 0; i < LCOUNT; ++i ) {
                x_CheckPoint(m_Locs[i]->GetTotalRange(), i);
            }
            for ( TSeqPos i = 0; i < LCOUNT; ++i ) {
                x_CheckPoint(m_Locs[i]->GetTotalRange(), i);
            }
            return 0;
        }

private:
    const TSeqLocs& m_Locs;
    CAtomicCounter& m_StartCounter;
};

BOOST_AUTO_TEST_CASE(TestTotalRange)
{
    const int LCOUNT = 1000000;
    const int TCOUNT = 4;

    CRandom rnd(1);
    TSeqLocs ll;
    CRef<CSeq_id> id(new CSeq_id("1"));
    for ( int i = 0; i < LCOUNT; ++i ) {
        CRef<CSeq_loc> loc(new CSeq_loc);
        if ( rnd.GetRand(0, 2) > 0 ) {
            loc->SetPnt().SetPoint(i);
        }
        else {
            loc->SetInt().SetFrom(i);
            loc->SetInt().SetTo(i);
        }
        loc->SetId(*id);
        ll.push_back(loc);
    }
    
    CAtomicCounter start_counter;
    start_counter.Set(TCOUNT);
    
    vector< CRef<CThread> > tt(TCOUNT);
    for ( int i = 0; i < TCOUNT; ++i ) {
        tt[i] = new CTotalRangeThread(ll, start_counter);
        tt[i]->Run();
    }
    for ( int i = 0; i < TCOUNT; ++i ) {
        void* exit_data;
        tt[i]->Join(&exit_data);
    }
}
#endif
