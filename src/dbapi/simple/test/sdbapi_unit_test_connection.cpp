/*  $Id: sdbapi_unit_test_connection.cpp 494895 2016-03-11 15:30:00Z ucko $
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
 * Author: Sergey Sikorskiy
 *
 * File Description: DBAPI unit-test
 *
 * ===========================================================================
 */

#include "sdbapi_unit_test_pch.hpp"


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Test_ConnParamsDatabase)
{
    try {
        const string target_db_name("DBAPI_Sample");

        // Check method Connect() ...
        {
            CSDB_ConnectionParam params(GetDatabase().GetConnectionParam());
            params.Set(CSDB_ConnectionParam::eDatabase, target_db_name);
            CDatabase db(params);
            BOOST_CHECK( !db.IsConnected(CDatabase::eFullCheck) );
            BOOST_CHECK( !db.IsConnected(CDatabase::eFastCheck) );
            BOOST_CHECK( !db.IsConnected(CDatabase::eNoCheck) );
            db.Connect();
            BOOST_CHECK(db.IsConnected(CDatabase::eNoCheck));
            BOOST_CHECK(db.IsConnected(CDatabase::eFastCheck));
            BOOST_CHECK(db.IsConnected(CDatabase::eFullCheck));

            CQuery query = db.NewQuery("select db_name()");

            query.Execute();
            query.RequireRowCount(1);
            BOOST_CHECK( query.HasMoreResultSets() );
            CQuery::iterator it = query.begin();
            BOOST_CHECK( it != query.end() );
            const  string db_name = it[1].AsString();
            BOOST_CHECK_EQUAL(db_name, target_db_name);
            BOOST_CHECK_NO_THROW(query.VerifyDone(CQuery::eAllResultSets));

            db.Close();
            BOOST_CHECK( !db.IsConnected(CDatabase::eFullCheck) );
            BOOST_CHECK( !db.IsConnected(CDatabase::eFastCheck) );
            BOOST_CHECK( !db.IsConnected(CDatabase::eNoCheck) );
        }
    }
    catch(const CException& ex) {
        DBAPI_BOOST_FAIL(ex);
    }
}

///////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Test_CloneConnection)
{
    try {
        const string target_db_name("DBAPI_Sample");

        // Create new connection ...
        CSDB_ConnectionParam params(GetDatabase().GetConnectionParam());
        params.Set(CSDB_ConnectionParam::eDatabase, target_db_name);
        CDatabase db(params);
        // db.Connect();

        // Clone connection ...
        CDatabase new_db = db.Clone();
        CQuery query = new_db.NewQuery("select db_name()");

        // Check that database was set correctly with the new connection ...
        {
            query.Execute();
            query.RequireRowCount(1);
            BOOST_CHECK( query.HasMoreResultSets() );
            CQuery::iterator it = query.begin();
            BOOST_CHECK( it != query.end() );
            const  string db_name = it[1].AsString();
            BOOST_CHECK_EQUAL(db_name, target_db_name);
        }
        BOOST_CHECK_NO_THROW(query.VerifyDone(CQuery::eAllResultSets));
    }
    catch(const CException& ex) {
        DBAPI_BOOST_FAIL(ex);
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Test_ConnParams)
{
    {
        CSDB_ConnectionParam params(
                "dbapi://" + GetArgs().GetUserName() +
                ":" + GetArgs().GetUserPassword() +
                "@" + GetArgs().GetServerName()
                );

        CDatabase db(params);
        // db.Connect();
        CQuery query = db.NewQuery("SELECT @@version");
        query.Execute();
        query.RequireRowCount(1);
        query.PurgeResults();
    }

    {
        CSDB_ConnectionParam params(
                "dbapi://" + GetArgs().GetUserName() +
                ":" + GetArgs().GetUserPassword() +
                "@" + GetArgs().GetServerName() +
                "/" + GetArgs().GetDatabaseName()
                );

        CDatabase db(params);
        // db.Connect();
        CQuery query = db.NewQuery("SELECT @@version");
        query.Execute();
        query.RequireRowCount(1);
        query.PurgeResults();
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Test_ConnParamOps)
{
    CSDB_ConnectionParam params1("dbapi://u1:p1@s1:123/?io_timeout=17&x1=v1"),
                         params2("dbapi://s2/d2?password_file=pf2&x2=v2");
    BOOST_CHECK_THROW(
        params1.ComposeUrl(CSDB_ConnectionParam::fThrowIfIncomplete),
        CSDB_Exception);
    BOOST_CHECK_EQUAL(params1.ComposeUrl(),
                      "dbapi://u1:p1@s1:123/?io_timeout=17&x1=v1");
    BOOST_CHECK_EQUAL(params1.ComposeUrl(CSDB_ConnectionParam::fHidePassword),
                      "dbapi://u1:(redacted)@s1:123/?io_timeout=17&x1=v1");
    BOOST_CHECK_EQUAL(params2.ComposeUrl(),
                      params2.ComposeUrl(CSDB_ConnectionParam::fHidePassword));

    CSDB_ConnectionParam merge12(params1), merge12d(params1),
                         merge21(params2), merge21d(params2);
    merge12 .Set(params2);
    merge12d.Set(params2, CSDB_ConnectionParam::fAsDefault);
    // gets database name

    merge21 .Set(params1);
    merge21d.Set(params1, CSDB_ConnectionParam::fAsDefault);
    // gets username, port, and io_timeout

    BOOST_CHECK_EQUAL(merge12.ComposeUrl(), params2.ComposeUrl());
    BOOST_CHECK_EQUAL(
        merge12d.ComposeUrl(CSDB_ConnectionParam::fThrowIfIncomplete),
        "dbapi://u1:p1@s1:123/d2?io_timeout=17&x1=v1");
    BOOST_CHECK_EQUAL(merge21.ComposeUrl(), params1.ComposeUrl());
    BOOST_CHECK_EQUAL(
        merge21d.ComposeUrl(CSDB_ConnectionParam::fThrowIfIncomplete),
        "dbapi://u1@s2:123/d2?password_file=pf2&x2=v2&io_timeout=17");
}

END_NCBI_SCOPE
