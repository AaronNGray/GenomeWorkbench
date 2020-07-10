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
 * Author: Andrei Shkeda
 *
 * File Description:
 *
 * ===========================================================================
 */

#include <ncbi_pch.hpp>
#include <corelib/test_boost.hpp>
#include <gui/objects/assembly_info.hpp>
#include <objects/genomecoll/genome_collection__.hpp>
#include <objects/genomecoll/genomic_collections_cli.hpp>
//#include <objects/genomecoll/GC_AssemblyDesc.hpp>

#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

USING_NCBI_SCOPE;
USING_SCOPE(objects);


BOOST_AUTO_TEST_SUITE(TestSuiteAssemblyInfo)

// CRmtIdMapper with assembly accession 
BOOST_AUTO_TEST_CASE(TestCaseGrch38)
{
    CGenomicCollectionsService GCService;
    {
        CRef<CGC_Assembly> gc_assm(GCService.GetAssembly("GCF_000001405.38", "AssemblyOnly"));
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "hg38") == true);
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "hg19") == false);
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "grch38") == true);
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "grch37") == false);
    }
}
BOOST_AUTO_TEST_CASE(TestCaseGrch37)
{
    CGenomicCollectionsService GCService;
    {
        CRef<CGC_Assembly> gc_assm(GCService.GetAssembly("GCF_000001405.13", "AssemblyOnly"));
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "hg38") == false);
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "hg19") == true);
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "grch38") == false);
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "grch37") == true);
    }
}
BOOST_AUTO_TEST_CASE(TestCaseRefseqAssmAcc)
{
    CGenomicCollectionsService GCService;
    {
        CRef<CGC_Assembly> gc_assm(GCService.GetAssembly("GCF_000001635.26", "AssemblyOnly"));
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "GRCm38") == true);
        BOOST_CHECK(CAssemblyInfo::IsSameAssembly(*gc_assm, "GCF_000001635.24") == true);
    }
}

BOOST_AUTO_TEST_SUITE_END();

