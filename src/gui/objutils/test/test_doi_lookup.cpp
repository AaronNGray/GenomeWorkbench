/*  $Id: test_doi_lookup.cpp 43082 2019-05-13 17:11:47Z filippov $
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
* Author:  Igor Filippov
*
* File Description:
*   DOI lookup unit test.
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/objutils/doi_lookup.hpp>
#include <cstdio>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>

#include <objtools/unit_test_util/unit_test_util.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);

NCBITEST_AUTO_INIT()
{
}


NCBITEST_INIT_CMDLINE(arg_descrs)
{   
}

NCBITEST_AUTO_FINI()
{
}

static CRef <CPubdesc> GetGroundTruth(const string& filename)
{
    CNcbiIfstream ifstr(filename.c_str());
    CRef <CPubdesc> ground(new CPubdesc);
    ifstr >> MSerial_AsnText >> *ground;
    ifstr.close();    
    return ground;
}

BOOST_AUTO_TEST_CASE(RunTests)
{
    CRef <CPubdesc> ground1 = GetGroundTruth("test_doi_lookup-1.asn");
    pair<CRef<CPubdesc>,string> pub1 = CDoiLookup::GetPubFromCrossRef("10.1007/s11557-011-0743-4");
    BOOST_CHECK_EQUAL(pub1.second, "");
    BOOST_CHECK(ground1 && pub1.first && ground1->Equals(*pub1.first));

    CRef <CPubdesc> ground2 = GetGroundTruth("test_doi_lookup-2.asn");
    pair<CRef<CPubdesc>,string> pub2 = CDoiLookup::GetPubFromCrossRef("10.1016/j.protis.2017.07.005");
    BOOST_CHECK_EQUAL(pub2.second, "");
    BOOST_CHECK(ground2 && pub2.first && ground2->Equals(*pub2.first));

    CRef <CPubdesc> ground3 = GetGroundTruth("test_doi_lookup-3.asn");
    pair<CRef<CPubdesc>,string> pub3 = CDoiLookup::GetPubFromCrossRef("10.3391/bir.2018.7.1.11");
    BOOST_CHECK_EQUAL(pub3.second, "");
    BOOST_CHECK(ground3 && pub3.first && ground3->Equals(*pub3.first));

    CRef <CPubdesc> ground4 = GetGroundTruth("test_doi_lookup-4.asn");
    pair<CRef<CPubdesc>,string> pub4 = CDoiLookup::GetPubFromCrossRef("10.1039/C7NP00047B");
    BOOST_CHECK_EQUAL(pub4.second, "");
    BOOST_CHECK(ground4 && pub4.first && ground4->Equals(*pub4.first));

    CRef <CPubdesc> ground5 = GetGroundTruth("test_doi_lookup-5.asn");
    pair<CRef<CPubdesc>,string> pub5 = CDoiLookup::GetPubFromCrossRef("10.1515/ap-2017-0051");
    BOOST_CHECK_EQUAL(pub5.second, "");
    BOOST_CHECK(ground5 && pub5.first && ground5->Equals(*pub5.first));

    CRef <CPubdesc> ground6 = GetGroundTruth("test_doi_lookup-6.asn");
    pair<CRef<CPubdesc>,string> pub6 = CDoiLookup::GetPubFromCrossRef("10.1101/616904");
    BOOST_CHECK_EQUAL(pub6.second, "");
    BOOST_CHECK(ground6 && pub6.first && ground6->Equals(*pub6.first));
}
