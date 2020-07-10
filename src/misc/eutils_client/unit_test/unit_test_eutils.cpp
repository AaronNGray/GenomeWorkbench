/* 
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
 * Author:  Alex Kotliarov
 *
 * File Description: test for 16S_summaries, BOOST Test framework. 
 */

#include <ncbi_pch.hpp>
#include <corelib/test_boost.hpp>

#include <misc/xmlwrapp/xmlwrapp.hpp>
#include <misc/eutils_client/eutils_client.hpp>

#include <sstream>


USING_NCBI_SCOPE;

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

BOOST_AUTO_TEST_CASE(TestSearchHistory)
{
    CEutilsClient cli;
    stringstream content;
    
    BOOST_REQUIRE_NO_THROW(cli.Search("pubmed", "asthma", content, CEutilsClient::eUseHistoryEnabled));
    string body = content.str();
    xml::document doc(body.c_str(), body.size(), NULL);
    const xml::node& root = doc.get_root_node();

    xml::node_set items = root.run_xpath_query("./WebEnv");
    BOOST_CHECK ( 1 == items.size() );
    
    items  = root.run_xpath_query("./QueryKey");
    BOOST_CHECK ( 1 == items.size() );
}

BOOST_AUTO_TEST_CASE(TestSearchHistoryIterate)
{
    CEutilsClient cli;
    stringstream content;
   
    cli.SetMaxReturn(101); 
    BOOST_REQUIRE_NO_THROW(cli.Search("pubmed", "asthma", content, CEutilsClient::eUseHistoryEnabled));
    string body = content.str();
    xml::document doc(body.c_str(), body.size(), NULL);
    const xml::node& root = doc.get_root_node();

    xml::node_set items = root.run_xpath_query("./WebEnv/text()");
    BOOST_CHECK ( 1 == items.size() );
    string web_env = ( items.size() > 0 ) ? items.begin()->get_content() : "";

    items  = root.run_xpath_query("./QueryKey/text()");
    BOOST_CHECK ( 1 == items.size() );
    int query_key = (items.size() > 0) ? NStr::StringToNumeric<int>(items.begin()->get_content()) : 0;

    if ( !web_env.empty() && query_key > 0 ) {
        items = root.run_xpath_query("./Count/text()");
        BOOST_CHECK ( 1 == items.size() );
        int count = NStr::StringToNumeric<int>(items.size() ? items.begin()->get_content() : "0");

        items = root.run_xpath_query("./RetMax/text()");
        BOOST_CHECK ( 1 == items.size() );
        int retmax = NStr::StringToNumeric<int>(items.size() ? items.begin()->get_content() : "0");

        items = root.run_xpath_query("./RetStart/text()");
        BOOST_CHECK ( 1 == items.size() );
 
        int retstart = NStr::StringToNumeric<int>(items.size() ? items.begin()->get_content() : "0");
        BOOST_CHECK ( retstart + retmax <= count );

        // Get next chunk from the history server.
        if ( retstart + retmax < count ) {
            int next_start = count - retmax;
            stringstream next_chunk;
            BOOST_REQUIRE_NO_THROW(cli.SearchHistory("pubmed", "asthma", web_env, query_key, next_start, next_chunk));

            string body = next_chunk.str();
            xml::document doc(body.c_str(), body.size(), NULL);
            const xml::node& root = doc.get_root_node();
            int retstart = NStr::StringToNumeric<int>(root.run_xpath_query("./RetStart/text()").begin()->get_content());
            BOOST_CHECK ( retstart == next_start );
        }
    }
}

BOOST_AUTO_TEST_CASE(TestSummaryHistory)
{
    CEutilsClient cli;
    stringstream content;
   
    cli.SetMaxReturn(101); 
    BOOST_REQUIRE_NO_THROW(cli.Search("pubmed", "asthma", content, CEutilsClient::eUseHistoryEnabled));
    string body = content.str();
    xml::document doc(body.c_str(), body.size(), NULL);
    const xml::node& root = doc.get_root_node();

    xml::node_set items = root.run_xpath_query("./WebEnv/text()");
    BOOST_CHECK ( items.size() != 0 );
    string web_env = ( items.size() > 0) ? items.begin()->get_content(): "";

    items  = root.run_xpath_query("./QueryKey/text()");
    BOOST_CHECK ( items.size() != 0 );
    int query_key = (items.size() > 0) ? NStr::StringToNumeric<int>(items.begin()->get_content()) : 0;

    if ( !web_env.empty() && query_key > 0 ) {
        stringstream summary;
        cli.SummaryHistory("pubmed", web_env, query_key, 0, "2.0", summary);

        {{
             string body = summary.str();
             xml::document doc(body.c_str(), body.size(), NULL);
             const xml::node& root = doc.get_root_node();

             xml::node_set nodes = root.run_xpath_query("//DocumentSummary");
             BOOST_CHECK ( 101 == nodes.size() );
         }}
    }
}

BOOST_AUTO_TEST_CASE(TestFetchHistory)
{
    CEutilsClient cli;
    stringstream content;
   
    cli.SetMaxReturn(101);
    BOOST_REQUIRE_NO_THROW(cli.Search("pubmed", "asthma", content, CEutilsClient::eUseHistoryEnabled));
    string body = content.str();
    xml::document doc(body.c_str(), body.size(), NULL);
    const xml::node& root = doc.get_root_node();

    xml::node_set items = root.run_xpath_query("./WebEnv/text()");
    BOOST_CHECK ( items.size() != 0 );
    string web_env = ( 1 == items.size() ) ? items.begin()->get_content() : "";

    items  = root.run_xpath_query("./QueryKey/text()");
    BOOST_CHECK ( items.size() != 0 );
    int query_key = ( items.size() > 0 ) ? NStr::StringToNumeric<int>(items.begin()->get_content()) : 0;

    if ( !web_env.empty() && query_key > 0 ) {
        stringstream history;
        BOOST_REQUIRE_NO_THROW(cli.FetchHistory("pubmed", web_env, query_key, 10, CEutilsClient::eContentType_xml, history));

        {{
             string body = history.str();
             xml::document doc(body.c_str(), body.size(), NULL);
             const xml::node& root = doc.get_root_node();

             BOOST_CHECK ( 101 == root.run_xpath_query("//PubmedArticle").size() + root.run_xpath_query("//PubmedBookArticle").size() );
         }}
    }
}

BOOST_AUTO_TEST_CASE(TestLinkHistory)
{
    CEutilsClient cli;
    stringstream content;
   
    vector<int> uids;
    uids.push_back(15718680);
    uids.push_back(157427902);

    cli.SetMaxReturn(101); 
    BOOST_REQUIRE_NO_THROW(cli.Link("protein", "gene", uids, content, "neighbor_history"));
    string body = content.str();
    xml::document doc(body.c_str(), body.size(), NULL);
    const xml::node& root = doc.get_root_node();

    xml::node_set items = root.run_xpath_query("//WebEnv/text()");
    BOOST_CHECK ( 1 == items.size() );

    items  = root.run_xpath_query("//QueryKey/text()");
    BOOST_CHECK ( 1 == items.size() );
}

BOOST_AUTO_TEST_CASE(TestLinkOut)
{
    CEutilsClient cli;
    xml::document doc;

    {
        vector<int> uids = { 124000572, 124000574 };

        cli.SetMaxReturn(101); 
        BOOST_REQUIRE_NO_THROW(cli.LinkOut("nucleotide", uids, doc, "llinks"));
        const xml::node& root = doc.get_root_node();

        xml::node_set items = root.run_xpath_query("//eLinkResult/LinkSet/IdUrlList/IdUrlSet");
        BOOST_CHECK ( 2 == items.size() );
    }
    
    //The same with accessions
    {
        vector<objects::CSeq_id_Handle> acc = { objects::CSeq_id_Handle::GetHandle("DQ896796.1"), objects::CSeq_id_Handle::GetHandle("DQ896797.1") };
        BOOST_REQUIRE_NO_THROW(cli.LinkOut("nucleotide", acc, doc, "llinkslib"));
        const xml::node& root = doc.get_root_node();

        xml::node_set items = root.run_xpath_query("//eLinkResult/LinkSet/IdUrlList/IdUrlSet");
        BOOST_CHECK ( 2 == items.size() );
    }    
}
