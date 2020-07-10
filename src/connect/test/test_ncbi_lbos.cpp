/* $Id: test_ncbi_lbos.cpp 507192 2016-07-15 16:35:54Z elisovdn $
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
 * Author:  Dmitriy Elisov
 *
 * File Description:
 *   lbos mapper tests
 *
 */

#include <ncbi_pch.hpp>
#include "test_ncbi_lbos_common.hpp"                                                        
#include    <corelib/test_boost.hpp>
#include "test_assert.h"  /* This header must go last */

USING_NCBI_SCOPE;


NCBITEST_INIT_TREE()
{}


NCBITEST_INIT_CMDLINE(args)
{
    args->AddOptionalPositional("lbos", "Primary address to LBOS",
                                CArgDescriptions::eString);

    
    args->AddOptionalPositional("onlyhealthcheck", 
                                "Only healthcheck response thread, no tests",
                                CArgDescriptions::eBoolean);
}


NCBITEST_AUTO_FINI()
{
#ifdef NCBI_THREADS
    s_HealthcheckThread->Stop(); // Stop listening on the socket
    s_HealthcheckThread->Join();
#endif
    s_PrintPortsLines();
    s_Print500sCount();
    s_PrintResolutionErrors();
}

/* We might want to clear ZooKeeper from nodes before running tests.
 * This is generally not good, because if this test application runs
 * on another host at the same moment, it will miss a lot of nodes and
 * tests will fail.
 */

NCBITEST_AUTO_INIT()
{
    s_Tls->SetValue(new int, TlsCleanup);
    *s_Tls->GetValue() = kMainThreadNumber;
    CNcbiRegistry& config = CNcbiApplication::Instance()->GetConfig();
    CONNECT_Init(&config);
#ifdef NCBI_MONKEY
    CMonkey::Instance()->
        RegisterThread(kMainThreadNumber);
#endif /* NCBI_MONKEY */
#ifdef NCBI_OS_MSWIN
    srand(NULL);
#else
    srand(time(NULL));
#endif
    boost::unit_test::framework::master_test_suite().p_name->assign(
        "lbos mapper Unit Test");
    if (CNcbiApplication::Instance()->GetArgs()["lbos"]) {
        string custom_lbos = 
            CNcbiApplication::Instance()->GetArgs()["lbos"].AsString();
        config.Set("CONN", "LBOS", custom_lbos);
    }
    if (CNcbiApplication::Instance()->GetArgs()["onlyhealthcheck"]) {
        s_HealthcheckThread = new CHealthcheckThread;
#ifdef NCBI_THREADS
        s_HealthcheckThread->Run();
#endif
        while (true) {
            SleepSec(1);
        }
    }
    LOG_POST(Error << "Checking LBOS primary address... LBOS=" << 
             config.Get("CONN", "LBOS"));
    CCObjHolder<char> lbos_answer(NULL);
    CCObjHolder<char> status_message(NULL);
    LBOS_ServiceVersionGet("/lbostest", 
                           &lbos_answer.Get(), 
                           &status_message.Get());
    lbos_answer = NULL;
    status_message = NULL;
    string lbostest_version = LBOSPrivate::GetServiceVersion("/lbostest");
    LOG_POST(Error << "Current /lbostest version is " << lbostest_version);
    if (lbostest_version != "1.0.0") {
        LOG_POST(Error << "Updating /lbostest version to 1.0.0...");
        LBOSPrivate::SetServiceVersion("/lbostest", "1.0.0");
        LOG_POST(Error << "/lbostest version successfully updated!");
    }
    s_HealthcheckThread = new CHealthcheckThread;
#ifdef NCBI_THREADS
    s_HealthcheckThread->Run();
#endif
#ifdef DEANNOUNCE_ALL_BEFORE_TEST
    s_ClearZooKeeper();
#endif
    s_CleanDTabs();
    s_ReadLBOSVersion();
}


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( SelfTest ) /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(TESTVERSIONCHECK)
{
    CMockFunction<SLBOSVersion> lbos_mock(s_LBOSVersion, 
                                          SLBOSVersion{ 1, 0, 2 });
    string versions;
    bool active = false;
    ///No elements - ON by default
    versions = "";
    //1
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);

    ///One element
    versions = "1.0.0";
    //1
    (*lbos_mock).major = 0;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //2
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 0;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //3
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 1;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);

    ///Two elements
    versions = "1.0.0, 1.0.2";
    //1
    (*lbos_mock).major = 0;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //2
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 0;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //3
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 1;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //4
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //5
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 3;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);

    /// Three elements
    versions = "1.0.0, 1.0.2, 1.0.4";
    //1
    (*lbos_mock).major = 0;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //2
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 0;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //3
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 1;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //4
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //5
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 3;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //6
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 4;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //6
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 5;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);

    /// Four elements
    versions = "1.0.0, 1.0.2, 1.0.4, 1.0.6";
    //1
    (*lbos_mock).major = 0;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //2
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 0;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //3
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 0;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //4
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 2;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //5
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 3;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //6
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 4;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //6
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 5;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, true);
    //7
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 6;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
    //8
    (*lbos_mock).major = 1;
    (*lbos_mock).minor = 0;
    (*lbos_mock).patch = 7;
    active = s_CheckTestVersion(s_ParseVersionsString(versions));
    NCBITEST_CHECK_EQUAL(active, false);
}
BOOST_AUTO_TEST_SUITE_END()


#ifdef LBOS_METADATA
///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Metadata ) /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* Tests:
 *   1. Announce with known 'extra' meta parameter using a special function for
 *      "known" metas. Run service discovery and get the announced metadata
 *      parameter. The same for "rate" and "type"
 *   2. Announce with known 'extra' meta parameter NOT using a special function
 *      for "known" metas. Run service discovery and get the announced metadata
 *      parameter. The same for "rate" and "type"
 *   3. Announce with unknown meta parameter. Run service discovery and get 
 *      the announced metadata parameter.
 *   4. Announce with both known and unknown metas. Run service discovery and
 *      get the announced metadata parameters.
 *   5. Add meta parameter to CMetaData, then delete this parameter (to remove 
 *      "meta", use Set or SetRate(-1) )
 *   6. Set "type" meta using a bad enum value - get exception
 *   7. C announce: meta is NULL- find no metas on service discovery
 *   8. C++ announce: provide no metas (not define this parameter) - find no 
 *      metas on service discovery
 *   9. SetType(enum) - check all valid enum values. Announce and then check 
 *      type on discovery
 *  10. SetType(string) - remove meta on empty value
 *  11. SetExtra(string) - remove meta on empty value
 *  12. Set - provide one name in different case combinations and check
 *      case insensitivity (the last provided value should be used with 
 *      lower-cased name)
 *  13. GetRate() - check normal value (if meta parameter was not set)
 *  14. GetType(bool) - check normal value. Check if meta parameter is not set
 *  15. GetType() - check all valid enum values
 *  16. Check GetMetaString in different combinations of parameters
 *  17. Try to write a "version" meta parameter - get exception.
 *      The same for "ip","port", "check", "format", "name"
 *  18. SetExtra with extra that has \s or \t or \n - throw exception
 */

 /* 1. Announce with known 'extra' meta parameter using a special function for
  *    "known" metas. Run service discovery and get the announced metadata
  *    parameter. The same for "rate" and "type" */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__KnownMetaSpecFunc__SeeMetaInDescovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::KnownMetaSpecFunc__SeeMetaInDescovery();
}


/* 2. Announce with known 'extra' meta parameter NOT using a special function
 *    for "known" metas. Run service discovery and get the announced metadata
 *    parameter.The same for "rate" and "type" */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__KnownMetaMainFunc__SeeMetaInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::KnownMetaMainFunc__SeeMetaInDiscovery();
}


/*3. Announce with unknown meta parameter. Run service discovery and get
 *   the announced metadata parameter. 
 *   NOTE: DISCOVERY IS NOT AVAILABLE, CHECKING VIA HTTP */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__UnknownMetaMainFunc__SeeMetaInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::UnknownMetaMainFunc__SeeMetaInDiscovery();
}

/* 4. Announce with both known and unknown metas.Run service discovery and
 *    get the announced metadata parameters. */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__KnownAndUnknown__SeeMetaInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::KnownAndUnknown__SeeMetaInDiscovery();
}

/*  5. Add meta parameter to CMetaData, then delete this parameter 
 *     (to remove "meta", use Set or SetRate(-1)) */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__DeleteMeta__NotSeeMetaInMetastring)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::DeleteMeta__NotSeeMetaInMetastring();
}


/*  6. Set "type" meta using a bad enum value - get exception */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetTypeBadEnumVal__Exception)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetTypeBadEnumVal__Exception();
}


/*  7. C announce : meta is NULL - find no metas on service discovery */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__MetaNull__NoMetaInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::MetaNull__NoMetaInDiscovery();
}


/*  8. C++ announce: provide no metas (not define this parameter) - find no
 *     metas on service discovery */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__NoMetaProvided__NoMetaInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::NoMetaProvided__NoMetaInDiscovery();
}


/*  9. SetType and GetType - check all valid enum values. Announce and then
 *     check type on discovery */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetTypeValidVal__SeeMetaInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetTypeValidVal__SeeMetaInDiscovery();
}


/* 10. type meta parameter - on re-announce the parameter is saved, even if
 *     re-announced without type */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetTypeEmpty__TypeStandaloneInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetTypeEmpty__TypeStandaloneInDiscovery();
}


/* 11. SetExtra(string) - remove meta on empty value. First announce with extra,
 *     then re-announce without extra */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetExtraEmpty__ExtraChangesToEmpty)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetExtraEmpty__ExtraChangesToEmpty();
}


/* 12. Set - provide one name in different case combinations and check
 *     case insensitivity(the last provided value should be used with
 *     lower - cased name) */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__Set__CaseInsensitive)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::Set__CaseInsensitive();
}


/* 13. Check default values for meta parameters  */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__GetDefaultMetas__DefaultValEmpty)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::GetDefaultMetas__DefaultValEmpty();
}


/* 14. Check GetMetaString in different combinations of parameters */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__GetMetaString__ValuesAsExpected)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::GetMetaString__ValuesAsExpected();
}


/* 15. Try to write a "version" meta parameter - get exception.
 *     The same for "ip", "port", "check", "format", "name" */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetNotMeta__Exception)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetNotMeta__Exception();
}

/* 16. SetExtra or SetType with extra that has a whitespace - throw 
 *     exception */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__ExtraOrTypeWhitespace__ThrowException)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::ExtraOrTypeWhitespace__ThrowException();
}

/* 17. Set for extra and type - works fine */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetWithWhitespace__SeeInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetWithWhitespace__SeeInDiscovery();
}

/* 18. SetRate - string that cannot be parsed - throw exception */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetRateInvalidString__ThrowException)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetRateInvalidString__ThrowException();
}

/* 19. Set rate via Set and get via GetRate - good on good value */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetRateGetRateInt__AllOK)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetRateGetRateInt__AllOK();
}


/* 20. Set rate via Set and get via GetRate - throw exception on unexpected 
 *     values */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__SetRateGetRateNonInt__Exception)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::SetRateGetRateNonInt__Exception();
}

/** 21. Announce from registry test - announce and see metedata in discovery */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__AnnounceFromRegistry__SeeMetaInDiscovery)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::AnnounceFromRegistry__SeeMetaInDiscovery();
}

/** 22. Announce from registry when section does not exist - 
 *      throw "Invalid arg" */
BOOST_AUTO_TEST_CASE(AnnounceMetadata__RegistryNoSuchSection__Exception)
{
    CHECK_LBOS_VERSION();
    AnnounceMetadata::RegistryNoSuchSection__Exception();
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* LBOS_METADATA */
///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( ExtraResolveDataTest ) /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(ExtraResolveData__ExtraData__DoesNotCrash)
{
    ExtraResolveData::ExtraData__DoesNotCrash();
}

BOOST_AUTO_TEST_CASE(ExtraResolveData__EmptyExtra__DoesNotCrash)
{
    ExtraResolveData::EmptyExtra__DoesNotCrash();
}

BOOST_AUTO_TEST_CASE(ExtraResolveData__EmptyServInfoPart__Skip)
{
    ExtraResolveData::EmptyServInfoPart__Skip();
}


BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( ExceptionCodeTests )////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ExceptionCodeTests__CheckCodes)
{
    CHECK_LBOS_VERSION();
    ExceptionCodes::CheckCodes();
}

BOOST_AUTO_TEST_CASE(ExceptionCodeTests__CheckErrorCodeStrings)
{
    CHECK_LBOS_VERSION();
    ExceptionCodes::CheckErrorCodeStrings();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( IPCacheTests )//////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/** Announce with host empty - resolving works, gets host from healthcheck,
 *  resolves host to IP and  saves result to cache. We compare real 
 *  IP with what was saved in cache
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__HostInHealthcheck__TryFindReturnsHostIP)
{
    CHECK_LBOS_VERSION();
    IPCache::HostInHealthcheck__TryFindReturnsHostIP();
}

/** Announce with host difficult from the one in healthcheck - resolving works, 
 *  resolves host to IP and saves result to cache. We compare real IP 
 *  with what was saved in cache
 * @attention 
 *  No multithread, test uses private methods that are not thread-safe by 
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__HostSeparate__TryFindReturnsHostkIP)
{
    CHECK_LBOS_VERSION();
    IPCache::HostSeparate__TryFindReturnsHostkIP();
}

/** Do not announce host - HostnameTryFind returns the same hostname.
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__NoHost__TryFindReturnsTheSame)
{
    CHECK_LBOS_VERSION();
    IPCache::NoHost__TryFindReturnsTheSame();
}

/** Resolve hostname that can be resolved to multiple options - 
 *  the resolved IP gets saved and then is returned again and again for 
 *  the same service name, version and port
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__ResolveHost__TryFindReturnsIP)
{
    CHECK_LBOS_VERSION();
    IPCache::ResolveHost__TryFindReturnsIP();
}

/** Resolve and cache an IP address - it must be resolved to itself
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__ResolveIP__TryFindReturnsIP)
{
    CHECK_LBOS_VERSION();
    IPCache::ResolveIP__TryFindReturnsIP();
}

/** Resolve an empty string - get "Unknown error".
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__ResolveEmpty__Error)
{
    CHECK_LBOS_VERSION();
    IPCache::ResolveEmpty__Error();
}


/** Actually, this behavior is not used anywhere, but this is the contract,
 *  and must be tested so it works if ever needed
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__Resolve0000__ReturnInvalidIP)
{
    CHECK_LBOS_VERSION();
    IPCache::Resolve0000__Return0000();
}

/** Real-life test. Announce a host and deannounce it: cache MUST forget
 *  deannounced host
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__DeannounceHost__TryFindDoesNotFind)
{
    CHECK_LBOS_VERSION();
    IPCache::DeannounceHost__TryFindDoesNotFind();
}


/** Test that for the second time rsolution result is taken from cache. Tested 
 *  with google.com which is very often resolved to different IPs
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__ResolveTwice__SecondTimeNoOp)
{
    CHECK_LBOS_VERSION();
    IPCache::ResolveTwice__SecondTimeNoOp();
}

/** Add host to cache and then remove it multiple times - no crashes should 
 *  happen. The value MUST be deleted and not found.
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__DeleteTwice__SecondTimeNoOp)
{
    CHECK_LBOS_VERSION();
    IPCache::DeleteTwice__SecondTimeNoOp();
}

/** Finding a resolved hostname multiple times - just a stress test that can 
 *  show leaking memory
 * @attention
 *  No multithread, test uses private methods that are not thread-safe by
 *  themselves */
BOOST_AUTO_TEST_CASE(IPCacheTests__TryFindTwice__SecondTimeNoOp)
{
    CHECK_LBOS_VERSION();
    IPCache::TryFindTwice__SecondTimeNoOp();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( ConfigureEndpoint )/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * 1. Set version then check version - should show the version that was 
 *    just set.
 * 2. Check version, then set different version, then check version -
 *    should show new version.
 * 3. Set version, check that it was set, then delete version - check
 *    that no version exists.
 * 4. Announce two servers with different version. First, set one version
 *    and discover server with that version. Then, set the second version
 *    and discover server with that version.
 * 5. Announce one server. Discover it. Then delete version. Try to
 *    discover it again, should not find.
 * 6. Set with no service - invalid args
 * 7. Get with no service - invalid args 
 * 8. Delete with no service - invalid args 
 * 9. Set with empty version - OK 
 * 10. Set with empty version no service - invalid args 
 * 11. Get, set, delete with service that does not exist, providing
 *     "exists" parameter - this parameter should be false and version
 *     should be empty
 * 12. Get, set, delete with service that does exist, providing
 *     "exists" parameter - this parameter should be true and version
 *     should be filled
 * 13. Get, set, delete with service that does not exist, not providing
 *     "exists" parameter -  version should be empty and no crash should
 *     happen
 * 14. Get, set, delete with service that does exist, not providing
 *     "exists" parameter - this parameter should be true and version
 *     should be filled */
 
/* 1. Set version then check version - should show the version that was 
 *    just set.
 *   Test is not for multi-threading                                        */
BOOST_AUTO_TEST_CASE(Configure__SetThenCheck__ShowsSetVersion)
{
    CHECK_LBOS_VERSION();
    Configure::SetThenCheck__ShowsSetVersion();
}

/* 2. Check version, then set different version, then check version -
 *    should show new version.
 *   Test is not for multi-threading                                       */
BOOST_AUTO_TEST_CASE(Configure__CheckSetNewCheck__ChangesVersion)
{
    CHECK_LBOS_VERSION();
    Configure::CheckSetNewCheck__ChangesVersion();
}

/* 3. Set version, check that it was set, then delete version - check
 *    that no version exists.
 *   Test is not for multi-threading                                       */
BOOST_AUTO_TEST_CASE(Configure__DeleteThenCheck__SetExistsFalse)
{
    CHECK_LBOS_VERSION();
    Configure::DeleteThenCheck__SetExistsFalse();
}

/* 4. Announce two servers with different version. First, set one version
 *    and discover server with that version. Then, set the second version
 *    and discover server with that version.
 *   Test is not for multi-threading                                      */
BOOST_AUTO_TEST_CASE(Configure__AnnounceThenChangeVersion__DiscoverAnotherServer)
{
    CHECK_LBOS_VERSION();
    Configure::AnnounceThenChangeVersion__DiscoverAnotherServer();
}

/* 5. Announce one server. Discover it. Then delete version. Try to
 *    discover it again, should not find.
 *   Test is not for multi-threading                                      */
BOOST_AUTO_TEST_CASE(Configure__AnnounceThenDeleteVersion__DiscoverFindsNothing)
{
    CHECK_LBOS_VERSION();
    Configure::AnnounceThenDeleteVersion__DiscoverFindsNothing();
}

/* 6. Set with no service - invalid args */
BOOST_AUTO_TEST_CASE(Configure__SetNoService__InvalidArgs)
{
    CHECK_LBOS_VERSION();
    Configure::SetNoService__InvalidArgs();
}

/* 7. Get with no service - invalid args */
BOOST_AUTO_TEST_CASE(Configure__GetNoService__InvalidArgs)
{
    CHECK_LBOS_VERSION();
    Configure::GetNoService__InvalidArgs();
}

/* 8. Delete with no service - invalid args */
BOOST_AUTO_TEST_CASE(Configure__DeleteNoService__InvalidArgs)
{
    CHECK_LBOS_VERSION();
    Configure::DeleteNoService__InvalidArgs();
}

/* 9. Set with empty version - OK */
BOOST_AUTO_TEST_CASE(Configure__SetEmptyVersion__OK)
{
    CHECK_LBOS_VERSION();
    Configure::SetEmptyVersion__OK();
}

/* 10. Set with empty version no service - invalid args */
BOOST_AUTO_TEST_CASE(Configure__SetNoServiceEmptyVersion__InvalidArgs)
{
    CHECK_LBOS_VERSION();
    Configure::SetNoServiceEmptyVersion__InvalidArgs();
}

/* 11. Get, set, delete with service that does not exist, providing
*     "exists" parameter - this parameter should be false and version
*     should be empty */
BOOST_AUTO_TEST_CASE(Configure__ServiceNotExistsAndBoolProvided__EqualsFalse)
{
    CHECK_LBOS_VERSION();
    Configure::ServiceNotExistsAndBoolProvided__EqualsFalse();
}

/* 12. Get, set, delete with service that does exist, providing
*     "exists" parameter - this parameter should be true and version
*     should be filled */
BOOST_AUTO_TEST_CASE(Configure__ServiceExistsAndBoolProvided__EqualsTrue)
{
    CHECK_LBOS_VERSION();
    Configure::ServiceExistsAndBoolProvided__EqualsTrue();
}

/* 13. Get, set, delete with service that does not exist, not providing
*     "exists" parameter -  version should be empty and no crash should
*     happen*/
BOOST_AUTO_TEST_CASE(Configure__ServiceNotExistsAndBoolNotProvided__NoCrash)
{
    CHECK_LBOS_VERSION();
    Configure::ServiceNotExistsAndBoolNotProvided__NoCrash();
}

/* 14. Get, set, delete with service that does exist, not providing
*     "exists" parameter - this parameter should be true and version
*     should be filled */
BOOST_AUTO_TEST_CASE(Configure__ServiceExistsAndBoolNotProvided__NoCrash)
{
    CHECK_LBOS_VERSION();
    Configure::ServiceExistsAndBoolNotProvided__NoCrash();
}
 BOOST_AUTO_TEST_SUITE_END()

     
///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( ResetIterator)/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * 1. Should make capacity of elements in data->cand equal zero
 * 2. Should be able to reset iter N times consequently without crash
 * 3. Should be able to "reset iter, then getnextinfo" N times consequently
 *    without crash
 */

BOOST_AUTO_TEST_CASE(SERV_Reset__NoConditions__IterContainsZeroCandidates)
{
    CHECK_LBOS_VERSION();
    ResetIterator::NoConditions__IterContainsZeroCandidates();
}


BOOST_AUTO_TEST_CASE(SERV_Reset__MultipleReset__ShouldNotCrash)
{
    CHECK_LBOS_VERSION();
    ResetIterator::MultipleReset__ShouldNotCrash();
}


BOOST_AUTO_TEST_CASE(SERV_Reset__Multiple_AfterGetNextInfo__ShouldNotCrash)
{
    CHECK_LBOS_VERSION();
    ResetIterator::Multiple_AfterGetNextInfo__ShouldNotCrash();
}

BOOST_AUTO_TEST_SUITE_END()

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( CloseIterator )/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * 1. Should work immediately after Open
 * 2. Should work immediately after Reset
 * 3. Should work immediately after Open, GetNextInfo
 * 4. Should work immediately after Open, GetNextInfo, Reset
 */
BOOST_AUTO_TEST_CASE(SERV_CloseIter__AfterOpen__ShouldWork)
{
    CHECK_LBOS_VERSION();
    CloseIterator::AfterOpen__ShouldWork();
}

BOOST_AUTO_TEST_CASE(SERV_CloseIter__AfterReset__ShouldWork)
{
    CHECK_LBOS_VERSION();
    CloseIterator::AfterReset__ShouldWork();
}

BOOST_AUTO_TEST_CASE(SERV_CloseIter__AfterGetNextInfo__ShouldWork)
{
    CHECK_LBOS_VERSION();
    CloseIterator::AfterGetNextInfo__ShouldWork();
}

/* In this test we check three different situations: when getnextinfo was
 * called once, was called to see half of all found servers, and was called to
 * iterate through all found servers. */
BOOST_AUTO_TEST_CASE(SERV_CloseIter__FullCycle__ShouldWork)
{
    CHECK_LBOS_VERSION();
    CloseIterator::FullCycle__ShouldWork();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Dtab )//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* 1. Mix of registry DTab and ConnNetInfo Dtab: registry goes first
 *     (lowest priority), then ConnNetInfo (higher priority)                 */
BOOST_AUTO_TEST_CASE(DTab__DTabRegistryAndHttp__RegistryGoesFirst) 
{
    CHECK_LBOS_VERSION();
    DTab::DTabRegistryAndHttp__RegistryGoesFirst();
}

/** 2. Announce a server with a non-standard version and a server with no  
    standard version at all. Both should be found via DTab                   */
BOOST_AUTO_TEST_CASE(DTab__NonStandardVersion__FoundWithDTab)
{
    CHECK_LBOS_VERSION();
    DTab::NonStandardVersion__FoundWithDTab();
}

/** 3. Mix of registry DTab and ConnNetInfo Dtab and RequestContext DTab: 
 *    registry goes first (lowest priority), then ConnNetInfo (higher priority),
 *    then RequestContext (highest priority)                                 */
BOOST_AUTO_TEST_CASE
                 (DTab__DTabRegistryAndHttpAndRequestContext__RegistryGoesFirst)
{
    CHECK_LBOS_VERSION();
    DTab::DTabRegistryAndHttpAndRequestContext__RegistryGoesFirst();
}

/** 4. Announce server with non-standard version and server with no standard
 *     version at all. Both should be found via Request Context DTab         */
BOOST_AUTO_TEST_CASE
(DTab__NonStandardVersion__FoundWithRequestContextDTab)
{
    CHECK_LBOS_VERSION();
    DTab::NonStandardVersion__FoundWithRequestContextDTab();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( ResolveViaLBOS )////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* 1. Should return string with IP:port if OK
 * 2. Should return NULL if lbos answered "not found"
 * 3. Should return NULL if lbos is not reachable
 * 4. Should be able to support up to M IP:port combinations (not checking for
 *    repeats) with storage overhead not more than same as size needed (that
 *    is, all space consumed is twice as size needed, used and unused space
 *    together)
 * 5. Should be able to correctly process incorrect lbos output. If from 5
 *    servers one is not valid, 4 other servers should be still returned
 *    by mapper
 */

BOOST_AUTO_TEST_CASE(s_LBOS_ResolveIPPort__ServiceExists__ReturnHostIP)
{
    CHECK_LBOS_VERSION();
    ResolveViaLBOS::ServiceExists__ReturnHostIP();
}


BOOST_AUTO_TEST_CASE(s_LBOS_ResolveIPPort__LegacyService__ReturnHostIP)
{
    CHECK_LBOS_VERSION();
    ResolveViaLBOS::LegacyService__ReturnHostIP();
}


BOOST_AUTO_TEST_CASE(s_LBOS_ResolveIPPort__ServiceDoesNotExist__ReturnNULL)
{
    CHECK_LBOS_VERSION();
    ResolveViaLBOS::ServiceDoesNotExist__ReturnNULL();
}

BOOST_AUTO_TEST_CASE(s_LBOS_ResolveIPPort__NoLBOS__ReturnNULL)
{
    CHECK_LBOS_VERSION();
    ResolveViaLBOS::NoLBOS__ReturnNULL();
}

BOOST_AUTO_TEST_CASE(s_LBOS_ResolveIPPort__FakeMassiveInput__ShouldProcess)
{
    CHECK_LBOS_VERSION();
    ResolveViaLBOS::FakeMassiveInput__ShouldProcess();
}


BOOST_AUTO_TEST_CASE(s_LBOS_ResolveIPPort__FakeMassiveInput__ShouldShuffle)
{
    CHECK_LBOS_VERSION();
    ResolveViaLBOS::FakeMassiveInput__ShouldShuffle();
}


BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Get_LBOS_address )//////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * 1. Should try only one current zone
 * 2. If could not read files, give up this industry and return NULL
 * 3. If unexpected content, return NULL
 * 4. If nothing found, return NULL
 */

/* Composing lbos address from /etc/ncbi/role + /etc/ncbi/domain:
 * Should try only one current zone and return it  */
BOOST_AUTO_TEST_CASE(g_LBOS_GetLBOSAddresses__SpecificMethod__FirstInResult)
{
    CHECK_LBOS_VERSION();
    GetLBOSAddress::SpecificMethod__FirstInResult();
}

/* Composing lbos address from /etc/ncbi/role + /etc/ncbi/domain:
 * Should return NULL if fail on ZONE  */
BOOST_AUTO_TEST_CASE(
        g_LBOS_GetLBOSAddresses__CustomHostNotProvided__SkipCustomHost)
{
    CHECK_LBOS_VERSION();
    GetLBOSAddress::CustomHostNotProvided__SkipCustomHost();
}

/* Composing lbos address from /etc/ncbi/role + /etc/ncbi/domain:
 * Should return NULL if fail on DOMAIN  */
BOOST_AUTO_TEST_CASE(g_LBOS_GetLBOSAddresses__NoConditions__AddressDefOrder)
{
    CHECK_LBOS_VERSION();
    GetLBOSAddress::NoConditions__AddressDefOrder();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Get_candidates )////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* 1. Iterate through received lbos's addresses, if there is not response from
 *    current lbos
 * 2. If one lbos works, do not try another lbos
 * 3. If net_info was provided for Serv_OpenP, the same net_info should be
 *    available while getting candidates via lbos to provide DTABs*/

/* We need this variables and function to count how many times algorithm will
 * try to resolve IP, and to know if it uses the headers we provided.
 */

 BOOST_AUTO_TEST_CASE(s_LBOS_FillCandidates__LBOSNoResponse__ErrorNoLBOS)
{
    CHECK_LBOS_VERSION();
    GetCandidates::LBOSNoResponse__ErrorNoLBOS();
}

BOOST_AUTO_TEST_CASE(s_LBOS_FillCandidates__LBOSResponse__Finish) 
{
    CHECK_LBOS_VERSION();
    GetCandidates::LBOSResponds__Finish();
}

BOOST_AUTO_TEST_CASE(s_LBOS_FillCandidates__NetInfoProvided__UseNetInfo)
{
    CHECK_LBOS_VERSION();
    GetCandidates::NetInfoProvided__UseNetInfo();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Get_Next_Info )/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * GetNextInfo:
 * 1. If no candidates found yet, or reset was just made, get candidates
 *    and return first
 * 2. If no candidates found yet, or reset was just made, and unrecoverable
 *    error while getting candidates, return 0
 * 3. If candidates already found, return next
 * 4. If last candidate was already returned, return 0
 * 5. If data is NULL for some reason, construct new data
 * 6. If iter is NULL, return NULL
 * 7. If SERV_MapperName(iter) returns name of another mapper, return NULL
 */

/* To be sure that mapper returns objects correctly, we need to be sure of
 * what lbos sends to mapper. The best way is to emulate lbos
 */

BOOST_AUTO_TEST_CASE(SERV_GetNextInfoEx__EmptyCands__RunGetCandidates)
{
    CHECK_LBOS_VERSION();
    GetNextInfo::EmptyCands__RunGetCandidates();
}

BOOST_AUTO_TEST_CASE(SERV_GetNextInfoEx__ErrorUpdating__ReturnNull)
{
    CHECK_LBOS_VERSION();
    GetNextInfo::ErrorUpdating__ReturnNull();
}

BOOST_AUTO_TEST_CASE(SERV_GetNextInfoEx__HaveCands__ReturnNext)
{
    CHECK_LBOS_VERSION();
    GetNextInfo::HaveCands__ReturnNext();
}

BOOST_AUTO_TEST_CASE(SERV_GetNextInfoEx__LastCandReturned__ReturnNull)
{
    CHECK_LBOS_VERSION();
    GetNextInfo::LastCandReturned__ReturnNull();
}

BOOST_AUTO_TEST_CASE(SERV_GetNextInfoEx__DataIsNull__ReconstructData)
{
    CHECK_LBOS_VERSION();
    GetNextInfo::DataIsNull__ReconstructData();
}

BOOST_AUTO_TEST_CASE(SERV_GetNextInfoEx__WrongMapper__ReturnNull)
{
    CHECK_LBOS_VERSION();
    GetNextInfo::WrongMapper__ReturnNull();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Open )//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* Open:
 * 1.If net_info is NULL, construct own net_info
 * 2. If read from lbos successful, return s_op
 * 3. If read from lbos successful and host info pointer != NULL, write NULL
 *    to host info
 * 4. If read from lbos unsuccessful or no such service, return 0
 * 5. Open with dbaf - should append DB name to service name
 * 6. Name is a mask - return NULL
 */

BOOST_AUTO_TEST_CASE(SERV_LBOS_Open__NetInfoNull__ConstructNetInfo)
{
    CHECK_LBOS_VERSION();
    Open::NetInfoNull__ConstructNetInfo();
}

BOOST_AUTO_TEST_CASE(SERV_LBOS_Open__ServerExists__ReturnLbosOperations)
{
    CHECK_LBOS_VERSION();
    Open::ServerExists__ReturnLbosOperations();
}

BOOST_AUTO_TEST_CASE(SERV_LBOS_Open__InfoPointerProvided__WriteNull)
{
    CHECK_LBOS_VERSION();
    Open::InfoPointerProvided__WriteNull();
}

BOOST_AUTO_TEST_CASE(SERV_LBOS_Open__NoSuchService__ReturnNull)
{
    CHECK_LBOS_VERSION();
    Open::NoSuchService__ReturnNull();
}

BOOST_AUTO_TEST_CASE(SERV_LBOS_Open__Dbaf__AppendDBNameToServiceName)
{
    CHECK_LBOS_VERSION();
    Open::Dbaf__AppendDBNameToServiceName();
}

BOOST_AUTO_TEST_CASE(SERV_LBOS_Open__NameIsMask__ReturnNull)
{
    CHECK_LBOS_VERSION();
    Open::NameIsMask__ReturnNull();
}


BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( GeneralLBOS )///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * We use such name of service that we MUST get lbos's answer
 * We check that it finds service and returns non-NULL list of
 * operations
 */
BOOST_AUTO_TEST_CASE(GeneralTest__ServerExists__ServOpenPReturnsLbosOperations)
{
    CHECK_LBOS_VERSION();
    GeneralLBOS::ServerExists__ServOpenPReturnsLbosOperations();
}

BOOST_AUTO_TEST_CASE(GeneralTest__ServerDoesNotExist__ShouldReturnNull)
{
    CHECK_LBOS_VERSION();
    GeneralLBOS::ServerDoesNotExist__ShouldReturnNull();
}

BOOST_AUTO_TEST_CASE(GeneralTest__LbosExist__ShouldWork)
{
    CHECK_LBOS_VERSION();
    GeneralLBOS::LbosExist__ShouldWork();
}


BOOST_AUTO_TEST_CASE(GeneralTest__DbafUnknownDB__ReturnNull)
{
    CHECK_LBOS_VERSION();
    GeneralLBOS::DbafUnknownDB__ReturnNull();
}


BOOST_AUTO_TEST_CASE(GeneralTest__DbafKnownDB__ShouldWork)
{
    CHECK_LBOS_VERSION();
    GeneralLBOS::DbafKnownDB__ShouldWork();
}


BOOST_AUTO_TEST_CASE(GeneralTest__NameIsMask__ReturnNull)
{
    CHECK_LBOS_VERSION();
    GeneralLBOS::NameIsMask__ReturnNull();
}

BOOST_AUTO_TEST_SUITE_END()



///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( InitializationTests )////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* 1. Multithread simultaneous SERV_LBOS_Open() when lbos is not yet
*    initialized should not crash
* 2. At initialization if no lbos found, mapper must turn OFF
* 3. At initialization if lbos found, mapper should be ON
* 4. If lbos has not yet been initialized, it should be initialized
*    at SERV_LBOS_Open()
* 5. If lbos turned OFF, it MUST return NULL on SERV_LBOS_Open()
* 6. s_LBOS_InstancesList MUST not be NULL at beginning of s_LBOS_
*    Initialize()
* 7. s_LBOS_InstancesList MUST not be NULL at beginning of
*    s_LBOS_FillCandidates()
* 8. s_LBOS_FillCandidates() should switch first and good lbos
*    addresses, if first is not responding
*/

/** Multithread simultaneous SERV_LBOS_Open() when lbos is not yet
 * initialized should not crash                                              */
BOOST_AUTO_TEST_CASE(Initialization__MultithreadInitialization__ShouldNotCrash)
{
    CHECK_LBOS_VERSION();
     Initialization::MultithreadInitialization__ShouldNotCrash();
}
/** At initialization if no lbos found, mapper must turn OFF                 */
BOOST_AUTO_TEST_CASE(Initialization__InitializationFail__TurnOff)
{
    CHECK_LBOS_VERSION();
     Initialization::InitializationFail__TurnOff();
}
/** At initialization if lbos found, mapper should be ON                     */
BOOST_AUTO_TEST_CASE(Initialization__InitializationSuccess__StayOn)
{
    CHECK_LBOS_VERSION();
     Initialization::InitializationSuccess__StayOn();
}
/** If lbos has not yet been initialized, it should be initialized at
 * SERV_LBOS_Open()                                                          */
BOOST_AUTO_TEST_CASE(Initialization__OpenNotInitialized__ShouldInitialize)
{
    CHECK_LBOS_VERSION();
     Initialization::OpenNotInitialized__ShouldInitialize();
}
/** If lbos turned OFF, it MUST return NULL on SERV_LBOS_Open()              */
BOOST_AUTO_TEST_CASE(Initialization__OpenWhenTurnedOff__ReturnNull)
{
    CHECK_LBOS_VERSION();
     Initialization::OpenWhenTurnedOff__ReturnNull();
}
/** s_LBOS_InstancesList MUST not be NULL at beginning of s_LBOS_Initialize()*/
BOOST_AUTO_TEST_CASE(
                Initialization__s_LBOS_Initialize__s_LBOS_InstancesListNotNULL)
{
    CHECK_LBOS_VERSION();
     Initialization::s_LBOS_Initialize__s_LBOS_InstancesListNotNULL();
}
/** s_LBOS_InstancesList MUST not be NULL at beginning of
 * s_LBOS_FillCandidates()                                                   */
BOOST_AUTO_TEST_CASE(
            Initialization__s_LBOS_FillCandidates__s_LBOS_InstancesListNotNULL)
{
    CHECK_LBOS_VERSION();
    Initialization::s_LBOS_FillCandidates__s_LBOS_InstancesListNotNULL();
}
BOOST_AUTO_TEST_SUITE_END()



///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( AnnounceTest )//////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*  1. Successfully announced: return SUCCESS
 *  2. Successfully announced: SLBOS_AnnounceHandle deannounce_handle 
 *     contains info needed to later deannounce announced server
 *  3. Successfully announced: char* lbos_answer contains answer of lbos
 *  4. Successfully announced: information about announcement is saved to 
 *     hidden lbos mapper's storage
 *  5. Could not find lbos: return NO_LBOS
 *  6. Could not find lbos: char* lbos_answer is set to NULL
 *  7. Could not find lbos: SLBOS_AnnounceHandle deannounce_handle is set to
 *     NULL
 *  8. lbos returned error: return LBOS_ERROR
 *  9. lbos returned error: char* lbos_answer contains answer of lbos
 * 10. lbos returned error: SLBOS_AnnounceHandle deannounce_handle is set to 
 *     NULL
 * 11. Server announced again (service name, IP and port coincide) and 
 *     announcement in the same zone, replace old info about announced 
 *     server in internal storage with new one.
 * 12. Server announced again and trying to announce in another 
 *     zone - return MULTIZONE_ANNOUNCE_PROHIBITED
 * 13. Was passed incorrect healthcheck URL (NULL or empty not starting with 
 *     "http(s)://"): do not announce and return INVALID_ARGS
 * 14. Was passed incorrect port (zero): do not announce and return 
 *     INVALID_ARGS
 * 15. Was passed incorrect version(NULL or empty): do not announce and 
 *     return INVALID_ARGS
 * 16. Was passed incorrect service nameNULL or empty): do not announce and 
 *     return INVALID_ARGS
 * 17. Real-life test: after announcement server should be visible to 
 *     resolve
 * 18. If was passed "0.0.0.0" as IP, should replace it with local IP or 
 *     hostname
 * 19. Was passed "0.0.0.0" as IP and could not manage to resolve local host 
 *     IP: do not announce and return DNS_RESOLVE_ERROR
 * 20. lbos is OFF - return eLBOS_Off                                        
 * 21. Announced successfully, but LBOS return corrupted answer - 
 *     return SERVER_ERROR                                                   */

/*  1. Successfully announced : return SUCCESS                               */
BOOST_AUTO_TEST_CASE(Announcement__AllOK__ReturnSuccess)
{
    CHECK_LBOS_VERSION();
    Announcement::AllOK__ReturnSuccess();
}

/*  3. Successfully announced : char* lbos_answer contains answer of lbos    */
BOOST_AUTO_TEST_CASE(Announcement__AllOK__LBOSAnswerProvided)
{
    CHECK_LBOS_VERSION();
    Announcement::AllOK__LBOSAnswerProvided();
}

/*  3. Successfully announced : char* lbos_answer contains answer of lbos    */
BOOST_AUTO_TEST_CASE(Announcement__AllOK__LBOSStatusMessageIsOK)
{
    CHECK_LBOS_VERSION();
    Announcement::AllOK__LBOSStatusMessageIsOK();
}

/*  4. Successfully announced: information about announcement is saved to
 *     hidden lbos mapper's storage                                          */
BOOST_AUTO_TEST_CASE(Announcement__AllOK__AnnouncedServerSaved)
{
    CHECK_LBOS_VERSION();
    Announcement::AllOK__AnnouncedServerSaved();
}
/*  5. Could not find lbos: return NO_LBOS                                   */
BOOST_AUTO_TEST_CASE(Announcement__NoLBOS__ReturnNoLBOSAndNotFind)
{
    CHECK_LBOS_VERSION();
    Announcement::NoLBOS__ReturnNoLBOSAndNotFind();
}
/*  6. Could not find lbos : char* lbos_answer is set to NULL                */
BOOST_AUTO_TEST_CASE(Announcement__NoLBOS__LBOSAnswerNull)
{
    CHECK_LBOS_VERSION();
    Announcement::NoLBOS__LBOSAnswerNull();
}

BOOST_AUTO_TEST_CASE(Announcement__NoLBOS__LBOSStatusMessageNull)
{
    CHECK_LBOS_VERSION();
    Announcement::NoLBOS__LBOSStatusMessageNull();
}

/*  8. lbos returned error: return eLBOS_ServerError                         */
BOOST_AUTO_TEST_CASE(Announcement__LBOSError__ReturnServerErrorCode)
{
    CHECK_LBOS_VERSION();
    Announcement::LBOSError__ReturnServerErrorCode();
}

BOOST_AUTO_TEST_CASE(Announcement__LBOSError__ReturnServerStatusMessage)
{
    CHECK_LBOS_VERSION();
    Announcement::LBOSError__ReturnServerStatusMessage();
}

/*  9. lbos returned error : char* lbos_answer contains answer of lbos       */
BOOST_AUTO_TEST_CASE(Announcement__LBOSError__LBOSAnswerProvided)
{
    CHECK_LBOS_VERSION();
    Announcement::LBOSError__LBOSAnswerProvided();
}
/* 11. Server announced again(service name, IP and port coincide) and
 *     announcement in the same zone, replace old info about announced
 *     server in internal storage with new one.                              */
BOOST_AUTO_TEST_CASE(
                 Announcement__AlreadyAnnouncedInTheSameZone__ReplaceInStorage)
{
    CHECK_LBOS_VERSION();
    Announcement::AlreadyAnnouncedInTheSameZone__ReplaceInStorage();
}
/* 12. Trying to announce in another domain - do nothing                     */
BOOST_AUTO_TEST_CASE(
        Announcement__ForeignDomain__NoAnnounce)
{
    CHECK_LBOS_VERSION();
    Announcement::ForeignDomain__NoAnnounce();
}
/* 13. Was passed incorrect healthcheck URL(NULL or empty not starting with
 *     "http(s)://") : do not announce and return INVALID_ARGS               */
BOOST_AUTO_TEST_CASE(Announcement__IncorrectURL__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement::IncorrectURL__ReturnInvalidArgs();
}
/* 14. Was passed incorrect port(zero) : do not announce and return
 *     INVALID_ARGS                                                          */
BOOST_AUTO_TEST_CASE(Announcement__IncorrectPort__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement::IncorrectPort__ReturnInvalidArgs();
}
/* 15. Was passed incorrect version(NULL or empty) : do not announce and
 *     return INVALID_ARGS                                                   */
BOOST_AUTO_TEST_CASE(Announcement__IncorrectVersion__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement::IncorrectVersion__ReturnInvalidArgs();
}
/* 16. Was passed incorrect service name (NULL or empty): do not announce and
 *     return INVALID_ARGS                                                   */
BOOST_AUTO_TEST_CASE(Announcement__IncorrectServiceName__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement::IncorrectServiceName__ReturnInvalidArgs();
}
/* 17. Real - life test : after announcement server should be visible to
 *     resolve                                                               */
BOOST_AUTO_TEST_CASE(Announcement__RealLife__VisibleAfterAnnounce)
{
    CHECK_LBOS_VERSION();
    Announcement::RealLife__VisibleAfterAnnounce();
}
/* 18. If was passed "0.0.0.0" as IP, should replace it with local IP or
 *     hostname                                                              */
BOOST_AUTO_TEST_CASE(Announcement__IP0000__ReplaceWithIP)
{
    CHECK_LBOS_VERSION();
    Announcement::IP0000__ReplaceWithIP();
}
/* 19. Was passed "0.0.0.0" as IP and could not manage to resolve local host
 *     IP : do not announce and return DNS_RESOLVE_ERROR                     */
BOOST_AUTO_TEST_CASE(Announcement__ResolveLocalIPError__ReturnDNSError)
{
    CHECK_LBOS_VERSION();
    Announcement::ResolveLocalIPError__ReturnDNSError();
}
/* 20. lbos is OFF - return eLBOS_Off                                        */
BOOST_AUTO_TEST_CASE(Announcement__LBOSOff__ReturnKLBOSOff)
{
    CHECK_LBOS_VERSION();
    Announcement::LBOSOff__ReturnKLBOSOff();
}
/*21. Announced successfully, but LBOS return corrupted answer -
      return SERVER_ERROR                                                    */
BOOST_AUTO_TEST_CASE(Announcement__LBOSAnnounceCorruptOutput__Return454)
{
    CHECK_LBOS_VERSION();
    Announcement::LBOSAnnounceCorruptOutput__Return454();
}
/*22. Trying to announce server and providing dead healthcheck URL - 
      return eLBOS_NotFound                                                  */
BOOST_AUTO_TEST_CASE(Announcement__HealthcheckDead__ReturnKLBOSSuccess)
{
    CHECK_LBOS_VERSION();
    Announcement::HealthcheckDead__ReturnKLBOSSuccess();
}
/*23. Trying to announce server and providing dead healthcheck URL -
      server should not be announced                                         */
BOOST_AUTO_TEST_CASE(Announcement__HealthcheckDead__AnnouncementOK)
{
    CHECK_LBOS_VERSION();
    Announcement::HealthcheckDead__AnnouncementOK();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( AnnounceRegistryTest )//////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*   1. All parameters good (Custom section has all parameters correct in 
 *      config) - return eLBOS_Success
 *   2. Custom section has nothing in config - return eLBOS_InvalidArgs
 *   3. Section empty or NULL (should use default section and return 
 *      eLBOS_Success)
 *   4. Service is empty or NULL - return eLBOS_InvalidArgs
 *   5. Version is empty or NULL - return eLBOS_InvalidArgs
 *   6. port is empty or NULL - return eLBOS_InvalidArgs
 *   7. port is out of range - return eLBOS_InvalidArgs
 *   8. port contains letters - return eLBOS_InvalidArgs
 *   9. healthcheck is empty or NULL - return eLBOS_InvalidArgs
 *  10. healthcheck does not start with http:// or https:// - return 
 *      eLBOS_InvalidArgs                                                    
 *  11. Trying to announce server and providing dead healthcheck URL -
 *      return eLBOS_NotFound                                                */

/*  1.  All parameters good (Custom section has all parameters correct in 
        config) - return eLBOS_Success                                       */
BOOST_AUTO_TEST_CASE(AnnouncementRegistry__ParamsGood__ReturnSuccess) 
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::ParamsGood__ReturnSuccess();
}
/*  2.  Custom section has nothing in config - return eLBOS_InvalidArgs      */
BOOST_AUTO_TEST_CASE(
                  AnnouncementRegistry__CustomSectionNoVars__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::CustomSectionNoVars__ReturnInvalidArgs();
}
/*  3.  Section empty or NULL (should use default section and return 
        eLBOS_Success)                                                       */
BOOST_AUTO_TEST_CASE(
AnnouncementRegistry__CustomSectionEmptyOrNullAndDefaultSectionIsOk__ReturnSuccess)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::
                 CustomSectionEmptyOrNullAndDefaultSectionIsOk__ReturnSuccess();
}
/*  4.  Service is empty or NULL - return eLBOS_InvalidArgs                  */
BOOST_AUTO_TEST_CASE(
                   AnnouncementRegistry__ServiceEmptyOrNull__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::ServiceEmptyOrNull__ReturnInvalidArgs();
}
/*  5.  Version is empty or NULL - return eLBOS_InvalidArgs                  */
BOOST_AUTO_TEST_CASE(
                   AnnouncementRegistry__VersionEmptyOrNull__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::VersionEmptyOrNull__ReturnInvalidArgs();
}
/*  6.  port is empty or NULL - return eLBOS_InvalidArgs                     */
BOOST_AUTO_TEST_CASE(AnnouncementRegistry__PortEmptyOrNull__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::PortEmptyOrNull__ReturnInvalidArgs();
}
/*  7.  port is out of range - return eLBOS_InvalidArgs                      */
BOOST_AUTO_TEST_CASE(AnnouncementRegistry__PortOutOfRange__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::PortOutOfRange__ReturnInvalidArgs();
}
/*  8.  port contains letters - return eLBOS_InvalidArgs                     */
BOOST_AUTO_TEST_CASE(
                  AnnouncementRegistry__PortContainsLetters__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::PortContainsLetters__ReturnInvalidArgs();
}
/*  9.  healthcheck is empty or NULL - return eLBOS_InvalidArgs              */
BOOST_AUTO_TEST_CASE(
              AnnouncementRegistry__HealthchecktEmptyOrNull__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::HealthchecktEmptyOrNull__ReturnInvalidArgs();
}
/*  10. healthcheck does not start with http:// or https:// - return         
        eLBOS_InvalidArgs                                                    */  
BOOST_AUTO_TEST_CASE(
      AnnouncementRegistry__HealthcheckDoesNotStartWithHttp__ReturnInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::
                          HealthcheckDoesNotStartWithHttp__ReturnInvalidArgs();
}
/*  11. Trying to announce server and providing dead healthcheck URL -
        return eLBOS_NotFound                                                */
BOOST_AUTO_TEST_CASE(
                   AnnouncementRegistry__HealthcheckDead__ReturnKLBOSSuccess)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry::HealthcheckDead__ReturnKLBOSSuccess();
}

BOOST_AUTO_TEST_SUITE_END()

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Deannounce )////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* 1. Successfully de-announced : return 1
 * 2. Successfully de-announced : if announcement was saved in local storage, 
 *    remove it
 * 3. Could not connect to provided lbos : fail and return 0
 * 4. Successfully connected to lbos, but deannounce returned error : return 0
 * 5. Real - life test : after de-announcement server should be invisible 
 *    to resolve                                                            
 * 6. Another domain - do not deannounce 
 * 7. Deannounce without IP specified - deannounce from local host 
 * 8. lbos is OFF - return eLBOS_Off                                         */
 /* 1. Successfully de-announced: return 1                                    */
BOOST_AUTO_TEST_CASE(Deannouncement__Deannounced__Return1)
{
    /* Here we specifiy port (specify that we do not intend to use specific 
     * port), because this function is used inside one another test 
     * (Deannouncement__RealLife__InvisibleAfterDeannounce)    */
    CHECK_LBOS_VERSION();
    Deannouncement::Deannounced__Return1(0);
}
/* 2. Successfully de-announced : if announcement was saved in local storage, 
 *    remove it                                                              */
BOOST_AUTO_TEST_CASE(Deannouncement__Deannounced__AnnouncedServerRemoved)
{
    CHECK_LBOS_VERSION();
    Deannouncement::Deannounced__AnnouncedServerRemoved();
}
/* 3. Could not connect to provided lbos : fail and return 0                 */
BOOST_AUTO_TEST_CASE(Deannouncement__NoLBOS__Return0)
{
    CHECK_LBOS_VERSION();
    Deannouncement::NoLBOS__Return0();
}
/* 4. Successfully connected to lbos, but deannounce returned error: 
 *    return 0                                                               */
BOOST_AUTO_TEST_CASE(Deannouncement__LBOSExistsDeannounce400__Return400)
{
    CHECK_LBOS_VERSION();
    Deannouncement::LBOSExistsDeannounce400__Return400();
}
/* 5. Real - life test : after de-announcement server should be invisible 
 *    to resolve                                                             */
BOOST_AUTO_TEST_CASE(Deannouncement__RealLife__InvisibleAfterDeannounce)
{
    CHECK_LBOS_VERSION();
    Deannouncement::RealLife__InvisibleAfterDeannounce();
}
/*6. If trying to deannounce in another domain - do not deannounce           */
BOOST_AUTO_TEST_CASE(Deannouncement__ForeignDomain__DoNothing)
{
    CHECK_LBOS_VERSION();
    Deannouncement::ForeignDomain__DoNothing();
}
/* 7. Deannounce without IP specified - deannounce from local host           */
BOOST_AUTO_TEST_CASE(Deannouncement__NoHostProvided__LocalAddress)
{
    CHECK_LBOS_VERSION();
    Deannouncement::NoHostProvided__LocalAddress();
}
/* 8. lbos is OFF - return eLBOS_Off                                         */
BOOST_AUTO_TEST_CASE(Deannouncement__LBOSOff__ReturnKLBOSOff)
{
    CHECK_LBOS_VERSION();
    Deannouncement::LBOSOff__ReturnKLBOSOff();
}
/* 9. Trying to deannounce non-existent service - return eLBOS_NotFound      */
BOOST_AUTO_TEST_CASE(Deannouncement__NotExists__ReturnKLBOSNotFound)
{
    CHECK_LBOS_VERSION();
    Deannouncement::NotExists__ReturnKLBOSNotFound();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( DeannounceAll )/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(DeannounceAll__AllDeannounced__NoSavedLeft)
{
    CHECK_LBOS_VERSION();
    DeannouncementAll::AllDeannounced__NoSavedLeft();
}
BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( AnnounceTest_CXX )//////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*  1. Successfully announced: return SUCCESS
 *  2. Successfully announced: SLBOS_AnnounceHandle deannounce_handle 
 *     contains info needed to later deannounce announced server
 *  3. Successfully announced: char* lbos_answer contains answer of lbos
 *  4. Successfully announced: information about announcement is saved to 
 *     hidden lbos mapper's storage
 *  5. Could not find lbos: return NO_LBOS
 *  6. Could not find lbos: char* lbos_answer is set to NULL
 *  7. Could not find lbos: SLBOS_AnnounceHandle deannounce_handle is set to
 *     NULL
 *  8. lbos returned error: return LBOS_ERROR
 *  9. lbos returned error: char* lbos_answer contains answer of lbos
 * 10. lbos returned error: SLBOS_AnnounceHandle deannounce_handle is set to 
 *     NULL
 * 11. Server announced again (service name, IP and port coincide) and 
 *     announcement in the same zone, replace old info about announced 
 *     server in internal storage with new one.
 * 12. Server announced again and trying to announce in another 
 *     zone - return MULTIZONE_ANNOUNCE_PROHIBITED
 * 13. Was passed incorrect healthcheck URL (NULL or empty not starting with 
 *     "http(s)://"): do not announce and return INVALID_ARGS
 * 14. Was passed incorrect port (zero): do not announce and return 
 *     INVALID_ARGS
 * 15. Was passed incorrect version(NULL or empty): do not announce and 
 *     return INVALID_ARGS
 * 16. Was passed incorrect service nameNULL or empty): do not announce and 
 *     return INVALID_ARGS
 * 17. Real-life test: after announcement server should be visible to 
 *     resolve
 * 18. If was passed "0.0.0.0" as IP, should replace it with local IP or 
 *     hostname
 * 19. Was passed "0.0.0.0" as IP and could not manage to resolve local host 
 *     IP: do not announce and return DNS_RESOLVE_ERROR
 * 20. lbos is OFF - return eLBOS_Off                                        
 * 21. Announced successfully, but LBOS return corrupted answer - 
 *     return SERVER_ERROR                                                   */

/*  1. Successfully announced : return SUCCESS                               */
BOOST_AUTO_TEST_CASE(Announcement_CXX__AllOK__ReturnSuccess)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::AllOK__ReturnSuccess();
}

/*  4. Successfully announced: information about announcement is saved to
 *     hidden lbos mapper's storage                                          */
BOOST_AUTO_TEST_CASE(Announcement_CXX__AllOK__AnnouncedServerSaved)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::AllOK__AnnouncedServerSaved();
}

/*  5. Could not find lbos: return NO_LBOS                                   */
BOOST_AUTO_TEST_CASE(Announcement_CXX__NoLBOS__ThrowNoLBOSAndNotFind)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::NoLBOS__ThrowNoLBOSAndNotFind();
}


/*  8. lbos returned error: return eLBOS_ServerError                         */
BOOST_AUTO_TEST_CASE(Announcement_CXX__LBOSError__ThrowServerError)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::LBOSError__ThrowServerError();
}

/*  9. lbos returned error : char* lbos_answer contains answer of lbos       */
BOOST_AUTO_TEST_CASE(Announcement_CXX__LBOSError__LBOSAnswerProvided)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::LBOSError__LBOSAnswerProvided();
}

/* 11. Server announced again(service name, IP and port coincide) and
 *     announcement in the same zone, replace old info about announced
 *     server in internal storage with new one.                              */
BOOST_AUTO_TEST_CASE(
             Announcement_CXX__AlreadyAnnouncedInTheSameZone__ReplaceInStorage)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::AlreadyAnnouncedInTheSameZone__ReplaceInStorage();
}

/* 13. Was passed incorrect healthcheck URL(NULL or empty not starting with
 *     "http(s)://") : do not announce and return INVALID_ARGS               */
BOOST_AUTO_TEST_CASE(Announcement_CXX__IncorrectURL__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::IncorrectURL__ThrowInvalidArgs();
}
/* 14. Was passed incorrect port(zero) : do not announce and return
 *     INVALID_ARGS                                                          */
BOOST_AUTO_TEST_CASE(Announcement_CXX__IncorrectPort__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::IncorrectPort__ThrowInvalidArgs();
}
/* 15. Was passed incorrect version(NULL or empty) : do not announce and
 *     return INVALID_ARGS                                                   */
BOOST_AUTO_TEST_CASE(Announcement_CXX__IncorrectVersion__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::IncorrectVersion__ThrowInvalidArgs();
}
/* 16. Was passed incorrect service name (NULL or empty): do not announce and
 *     return INVALID_ARGS                                                   */
BOOST_AUTO_TEST_CASE(Announcement_CXX__IncorrectServiceName__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::IncorrectServiceName__ThrowInvalidArgs();
}
/* 17. Real - life test : after announcement server should be visible to
 *     resolve                                                               */
BOOST_AUTO_TEST_CASE(Announcement_CXX__RealLife__VisibleAfterAnnounce)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::RealLife__VisibleAfterAnnounce();
}
/* 18. If was passed "0.0.0.0" as IP, should replace it with local IP or
 *     hostname                                                              */
BOOST_AUTO_TEST_CASE(Announcement_CXX__IP0000__ReplaceWithIP)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::IP0000__ReplaceWithIP();
}
/* 19. Was passed "0.0.0.0" as IP and could not manage to resolve local host
 *     IP : do not announce and return DNS_RESOLVE_ERROR                     */
BOOST_AUTO_TEST_CASE(Announcement_CXX__ResolveLocalIPError__ReturnDNSError)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::ResolveLocalIPError__ReturnDNSError();
}
/* 20. lbos is OFF - return eLBOS_Off                                        */
BOOST_AUTO_TEST_CASE(Announcement_CXX__LBOSOff__ThrowKLBOSOff)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::LBOSOff__ThrowKLBOSOff();
}
/*21. Announced successfully, but LBOS return corrupted answer -
      return SERVER_ERROR                                                    */
BOOST_AUTO_TEST_CASE(
                    Announcement_CXX__LBOSAnnounceCorruptOutput__ThrowServerError)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::LBOSAnnounceCorruptOutput__ThrowServerError();
}
/*22. Trying to announce server and providing dead healthcheck URL - 
      throw e_NotFound                                                  */
BOOST_AUTO_TEST_CASE(Announcement_CXX__HealthcheckDead__ThrowE_NotFound)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::HealthcheckDead__ThrowE_NotFound();
}
/*23. Trying to announce server and providing dead healthcheck URL -
      server should not be announced                                         */
BOOST_AUTO_TEST_CASE(Announcement_CXX__HealthcheckDead__AnnouncementOK)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::HealthcheckDead__AnnouncementOK();
}
/* 24. Announce server with separate host and healtcheck - should be found in
*     %LBOS%/text/service                                                   */
BOOST_AUTO_TEST_CASE(Announcement_CXX__SeparateHost__AnnouncementOK)
{
    CHECK_LBOS_VERSION();
    Announcement_CXX::SeparateHost__AnnouncementOK();
}
BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( AnnounceRegistryTest_CXX )//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*  1.  All parameters good (Custom section has all parameters correct in 
 *      config) - return eLBOS_Success
 *  2.  Custom section has nothing in config - return eLBOS_InvalidArgs
 *  3.  Section empty or NULL (should use default section and return 
 *      eLBOS_Success)
 *  4.  Service is empty or NULL - return eLBOS_InvalidArgs
 *  5.  Version is empty or NULL - return eLBOS_InvalidArgs
 *  6.  port is empty or NULL - return eLBOS_InvalidArgs
 *  7.  port is out of range - return eLBOS_InvalidArgs
 *  8.  port contains letters - return eLBOS_InvalidArgs
 *  9.  healthcheck is empty or NULL - return eLBOS_InvalidArgs
 *  10. healthcheck does not start with http:// or https:// - return 
 *      eLBOS_InvalidArgs                                                    
 *  11. Trying to announce server and providing dead healthcheck URL -
 *      return eLBOS_NotFound                                                */

/*  1.  All parameters good (Custom section has all parameters correct in 
        config) - return eLBOS_Success                                       */
BOOST_AUTO_TEST_CASE(AnnouncementRegistry_CXX__ParamsGood__ReturnSuccess) 
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::ParamsGood__ReturnSuccess();
}
/*  2.  Custom section has nothing in config - return eLBOS_InvalidArgs      */
BOOST_AUTO_TEST_CASE(
    AnnouncementRegistry_CXX__CustomSectionNoVars__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::CustomSectionNoVars__ThrowInvalidArgs();
}
/*  3.  Section empty or NULL (should use default section and return 
        eLBOS_Success)                                                       */
BOOST_AUTO_TEST_CASE(
AnnouncementRegistry_CXX__CustomSectionEmptyOrNullAndSectionIsOk__AllOK
                                                                              )
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::
                       CustomSectionEmptyOrNullAndSectionIsOk__AllOK();
}
/*  4.  Service is empty or NULL - return eLBOS_InvalidArgs                  */
BOOST_AUTO_TEST_CASE(
    AnnouncementRegistry_CXX__ServiceEmptyOrNull__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::ServiceEmptyOrNull__ThrowInvalidArgs();
}
/*  5.  Version is empty or NULL - return eLBOS_InvalidArgs                  */
BOOST_AUTO_TEST_CASE(
    AnnouncementRegistry_CXX__VersionEmptyOrNull__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::VersionEmptyOrNull__ThrowInvalidArgs();
}
/*  6.  port is empty or NULL - return eLBOS_InvalidArgs                     */
BOOST_AUTO_TEST_CASE(
    AnnouncementRegistry_CXX__PortEmptyOrNull__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::PortEmptyOrNull__ThrowInvalidArgs();
}
/*  7.  port is out of range - return eLBOS_InvalidArgs                      */
BOOST_AUTO_TEST_CASE(
    AnnouncementRegistry_CXX__PortOutOfRange__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::PortOutOfRange__ThrowInvalidArgs();
}
/*  8.  port contains letters - return eLBOS_InvalidArgs                     */
BOOST_AUTO_TEST_CASE(
              AnnouncementRegistry_CXX__PortContainsLetters__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::PortContainsLetters__ThrowInvalidArgs();
}
/*  9.  healthcheck is empty or NULL - return eLBOS_InvalidArgs              */
BOOST_AUTO_TEST_CASE(
          AnnouncementRegistry_CXX__HealthchecktEmptyOrNull__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::
            HealthchecktEmptyOrNull__ThrowInvalidArgs();
}
/*  10. healthcheck does not start with http:// or https:// - return         
        eLBOS_InvalidArgs                                                    */  
BOOST_AUTO_TEST_CASE(
  AnnouncementRegistry_CXX__HealthcheckDoesNotStartWithHttp__ThrowInvalidArgs)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::
                          HealthcheckDoesNotStartWithHttp__ThrowInvalidArgs();
}      
/*  11. Trying to announce server and providing dead healthcheck URL -
        return eLBOS_NotFound                                                */
BOOST_AUTO_TEST_CASE(
    AnnouncementRegistry_CXX__HealthcheckDead__ThrowE_NotFound)
{
    CHECK_LBOS_VERSION();
    AnnouncementRegistry_CXX::HealthcheckDead__ThrowE_NotFound();
}      
BOOST_AUTO_TEST_SUITE_END()

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Deannounce_CXX )////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* 1. Successfully de-announced : return 1
 * 2. Successfully de-announced : if announcement was saved in local storage, 
 *    remove it
 * 3. Could not connect to provided lbos : fail and return 0
 * 4. Successfully connected to lbos, but deannounce returned error : return 0
 * 5. Real - life test : after de-announcement server should be invisible 
 *    to resolve                                                            
 * 6. Another domain - do not deannounce 
 * 7. Deannounce without IP specified - deannounce from local host 
 * 8. lbos is OFF - return eLBOS_Off                                         */
 /* 1. Successfully de-announced: return 1                                    */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__Deannounced__Return1)
{
    /* Here we specifiy port (specify that we do not intend to use specific 
     * port), because this function is used inside one another test 
     * (Deannouncement_CXX__RealLife__InvisibleAfterDeannounce)    */
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::Deannounced__Return1(0);
}
/* 2. Successfully de-announced : if announcement was saved in local storage, 
 *    remove it                                                              */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__Deannounced__AnnouncedServerRemoved)
{
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::Deannounced__AnnouncedServerRemoved();
}
/* 3. Could not connect to provided lbos : fail and return 0                 */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__NoLBOS__Return0)
{
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::NoLBOS__Return0();
}
/* 4. Successfully connected to lbos, but deannounce returned error: 
 *    return 0                                                               */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__LBOSExistsDeannounceError__Return0)
{
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::LBOSExistsDeannounceError__Return0();
}
/* 5. Real - life test : after de-announcement server should be invisible 
 *    to resolve                                                             */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__RealLife__InvisibleAfterDeannounce)
{
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::RealLife__InvisibleAfterDeannounce();
}
/* 7. Deannounce without IP specified - deannounce from local host           */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__NoHostProvided__LocalAddress)
{
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::NoHostProvided__LocalAddress();
}
/* 8. lbos is OFF - return eLBOS_Off                                         */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__LBOSOff__ThrowKLBOSOff)
{
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::LBOSOff__ThrowKLBOSOff();
}
/* 9. Trying to deannounce non-existent service - throw e_NotFound           */
BOOST_AUTO_TEST_CASE(Deannouncement_CXX__NotExists__ThrowE_NotFound)
{
    CHECK_LBOS_VERSION();
    Deannouncement_CXX::NotExists__ThrowE_NotFound();
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( DeannounceAll_CXX )/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(DeannounceAll_CXX__AllDeannounced__NoSavedLeft)
{
    CHECK_LBOS_VERSION();
    DeannouncementAll_CXX::AllDeannounced__NoSavedLeft();
}
BOOST_AUTO_TEST_SUITE_END()

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Stability )/////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * 1. Just reset
 * 2. Full cycle: open mapper, get all servers, close iterator, repeat.
 */
/*  A simple stability test. We open iterator, and then, not closing it,
 * we get all servers and reset iterator and start getting all servers again
 */
BOOST_AUTO_TEST_CASE(Stability__GetNext_Reset__ShouldNotCrash)
{
    CHECK_LBOS_VERSION();
    Stability::GetNext_Reset__ShouldNotCrash();
}

BOOST_AUTO_TEST_CASE(Stability__FullCycle__ShouldNotCrash)
{
    CHECK_LBOS_VERSION();
    Stability::FullCycle__ShouldNotCrash();
}

BOOST_AUTO_TEST_SUITE_END()

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( Performance )///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 *  Perform full cycle: open mapper, get all servers, close iterator, repeat.
 * Test lasts 60 seconds and measures min, max and avg performance.
 */
BOOST_AUTO_TEST_CASE(Performance__FullCycle__ShouldNotCrash)
{
    CHECK_LBOS_VERSION();
    Performance::FullCycle__ShouldNotCrash();
}

BOOST_AUTO_TEST_SUITE_END()

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( MultiThreading )////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 *  Run all tests at once in concurrent threads and expect all tests to run
 * with the same results, except those in which mocks are not thread safe
 */
BOOST_AUTO_TEST_CASE(MultiThreading_test1)
{
    CHECK_LBOS_VERSION();
#ifdef NCBI_THREADS
    MultiThreading::TryMultiThread();
#endif /* NCBI_THREADS */
}

BOOST_AUTO_TEST_SUITE_END()


///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE( ResolveViaLBOS )////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* Moved to be last because not very important and very long                 */
BOOST_AUTO_TEST_CASE(s_LBOS_ResolveIPPort__FakeErrorInput__ShouldNotCrash)
{
    CHECK_LBOS_VERSION();
    ResolveViaLBOS::FakeErrorInput__ShouldNotCrash();
}
BOOST_AUTO_TEST_SUITE_END()
//#endif
