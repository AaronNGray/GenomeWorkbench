/*  $Id: test_snp_filters.cpp 19838 2009-08-12 12:30:51Z quintosm $
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

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <gui/widgets/snp/filter/snp_filter.hpp>
#include <vector>

USING_NCBI_SCOPE;

//////////////////////////////////////////////////////////////////////////////
// TEST SUITE FOR SNP Filters
//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE ( test_snp_filter )

static bool s_IsEqual(const SSnpFilter& f1, const SSnpFilter& f2)
{
    if (f1.checkLinks               == f2.checkLinks
         && f1.checkGeneFxn         == f2.checkGeneFxn         
         && f1.checkMapping         == f2.checkMapping         
         && f1.checkWeight          == f2.checkWeight          
         && f1.checkAlleleFreqList  == f2.checkAlleleFreqList  
         && f1.checkAlleleFreqChoice== f2.checkAlleleFreqChoice
         && f1.checkGaP_Hapmap      == f2.checkGaP_Hapmap      
         && f1.checkVarClass        == f2.checkVarClass        
         && f1.checkQualityCheck    == f2.checkQualityCheck    
         && f1.choiceWeight         == f2.choiceWeight         
         && f1.choiceAlleleFreq     == f2.choiceAlleleFreq     
         && f1.choiceVarClass       == f2.choiceVarClass       
         && f1.listLinks            == f2.listLinks
         && f1.listGeneFxns         == f2.listGeneFxns
         && f1.listMappings         == f2.listMappings
         && f1.listAlleleFreqs      == f2.listAlleleFreqs
         && f1.listGaP_Hapmaps      == f2.listGaP_Hapmaps
         && f1.listQualityChecks    == f2.listQualityChecks)
    {
        return true;
    }
    else {
        return false;    
    }
}

BOOST_AUTO_TEST_CASE (filter_version_1)
{   
    SSnpFilter f1;
    string s1 = "1|My Custom Filter|100000011 -1 -1 0 12:14:43:48 _ _ _ _ 2:3";
    string s2;

    f1.SerializeFrom(s1);    
    f1.SerializeTo(s2);

    BOOST_CHECK( s1==s2 );
}

BOOST_AUTO_TEST_CASE (filter_version_1upgrade)
{
    // This test will make sure older filters are supported with the
    // new version changes
    SSnpFilter f1;
    string s  = "My Custom Filter|100000011 -1 -1 0 12:14:43:48 _ _ _ _ 2:3";    
    string s1 = "1|My Custom Filter|100000011 -1 -1 0 12:14:43:48 _ _ _ _ 2:3";
    
    f1.SerializeFrom(s);    
    f1.SerializeTo(s);

    BOOST_CHECK( s==s1 );
}

BOOST_AUTO_TEST_SUITE_END()
