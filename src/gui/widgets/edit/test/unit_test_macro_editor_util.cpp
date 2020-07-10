/*  $Id: unit_test_macro_editor_util.cpp 44736 2020-03-03 19:44:00Z asztalos $
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
*   Unit tests for utility functions used in the macro editor
*/

#include <ncbi_pch.hpp>
// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/widgets/edit/macro_treeitemdata.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

BOOST_AUTO_TEST_CASE(Test_GetRNAFields)
{
    string rna_type, ncRNA_class, rna_qual;
    NMItemData::GetRNASpecificFields("product", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "any");
    BOOST_CHECK(ncRNA_class.empty());
    BOOST_CHECK(rna_qual == "product");

    NMItemData::GetRNASpecificFields("ncRNA class", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "any");
    BOOST_CHECK(ncRNA_class.empty());
    BOOST_CHECK(rna_qual == "ncRNA class");

    NMItemData::GetRNASpecificFields("miscRNA tag-peptide", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "miscRNA");
    BOOST_CHECK(ncRNA_class.empty());
    BOOST_CHECK(rna_qual == "tag-peptide");

    NMItemData::GetRNASpecificFields("mRNA gene locus", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "mRNA");
    BOOST_CHECK(ncRNA_class.empty());
    BOOST_CHECK(rna_qual == "gene locus");

    NMItemData::GetRNASpecificFields("gene locus tag", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "any");
    BOOST_CHECK(ncRNA_class.empty());
    BOOST_CHECK(rna_qual == "gene locus tag");

    NMItemData::GetRNASpecificFields("ncRNA gene locus tag", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "ncRNA");
    BOOST_CHECK(ncRNA_class == "any");
    BOOST_CHECK(rna_qual == "gene locus tag");

    NMItemData::GetRNASpecificFields("ncRNA ncRNA class", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "ncRNA");
    BOOST_CHECK(ncRNA_class == "any");
    BOOST_CHECK(rna_qual == "ncRNA class");

    NMItemData::GetRNASpecificFields("ncRNA piRNA gene locus tag", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "ncRNA");
    BOOST_CHECK(ncRNA_class == "piRNA");
    BOOST_CHECK(rna_qual == "gene locus tag");

    NMItemData::GetRNASpecificFields("ncRNA antisense_RNA product", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "ncRNA");
    BOOST_CHECK(ncRNA_class == "antisense_RNA");
    BOOST_CHECK(rna_qual == "product");

    NMItemData::GetRNASpecificFields("ncRNA any codons recognized", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "ncRNA");
    BOOST_CHECK(ncRNA_class == "any");
    BOOST_CHECK(rna_qual == "codons recognized");

    NMItemData::GetRNASpecificFields("pre_RNA gene description", rna_type, ncRNA_class, rna_qual);
    BOOST_CHECK(rna_type == "preRNA");
    BOOST_CHECK(ncRNA_class.empty());
    BOOST_CHECK(rna_qual == "gene description");
}
