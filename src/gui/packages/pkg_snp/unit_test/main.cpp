/*  $Id: main.cpp 19838 2009-08-12 12:30:51Z quintosm $
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
 * Authors:  Melvin Quintos
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

#include <corelib/ncbistre.hpp>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test_log.hpp>

using namespace boost::unit_test;

using boost::unit_test::test_suite;

///////////////////////////////////////////////////////////////////////////////
/// main entry point for tests

static ncbi::CNcbiOfstream out("unit_test_snp.xml");  // TODO place in a non global area


test_suite* init_unit_test_suite(int /* argc */, char * /* argv */ [])
{
    typedef boost::unit_test_framework::unit_test_log_t TLog;
    TLog& log = boost::unit_test_framework::unit_test_log;
    log.set_stream(out);
    log.set_format(boost::unit_test_framework::XML);
    log.set_threshold_level(boost::unit_test_framework::log_test_units);

    test_suite* test = BOOST_TEST_SUITE("unit_test_snp");

    return test;
}
