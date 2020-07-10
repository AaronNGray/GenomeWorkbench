/*  $Id: unit_test_import_feattable.cpp 45001 2020-05-05 20:50:53Z asztalos $
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
* File Description:
*   Unit tests for importing feature tables
*/

#include <ncbi_pch.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/packages/pkg_sequence_edit/import_feat_table.hpp>
#include <objtools/unit_test_util/unit_test_util.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

CRef<CSeq_entry> ReadEntryFromFile(const string& fname)
{
    CRef<CSeq_entry> entry(new CSeq_entry);

    try {
        CNcbiIfstream istr(fname.c_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *entry;
    }
    catch (const CException& e)  {
        LOG_POST(Error << e.ReportAll());
        return CRef<CSeq_entry>();
    }
    return entry;
}

CRef<CSeq_annot> ReadAnnotFromFile(const string& fname)
{
    CRef<CSeq_annot> annot(new CSeq_annot);
    try {
        CNcbiIfstream istr(fname.c_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *annot;
    }
    catch (const CException& e) {
        LOG_POST(Error << e.ReportAll());
        return CRef<CSeq_annot>();
    }
    return annot;
}

template<class T>
string MakeAsn(const T& object)
{
    CNcbiOstrstream str;
    str << MSerial_AsnText << object;
    return CNcbiOstrstreamToString(str);
}


void DoImportFTable(CSeq_entry_Handle& seh, const CSeq_annot& annot)
{
    CImportFeatTable importer(seh);
    CImportFeatTable::TFeatSeqIDMap seqid_map;

    CSeq_id_Handle ftb_idh;
    auto it = annot.GetData().GetFtable().front();
    const CSeq_loc& loc = it->GetLocation();
    ftb_idh = CSeq_id_Handle::GetHandle(*loc.GetId());


    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    CSeq_id_Handle seq_idh = sequence::GetId(*b_iter, sequence::eGetId_Seq_id_BestRank);
    seqid_map[ftb_idh] = seq_idh;
	unsigned int startingLocusTagNumber = 1;
	unsigned int startingFeatureId = 1;
	string locus_tag;
	bool euk = false;
    CIRef<IEditCommand> cmd = importer.TestImport(annot, seqid_map, startingLocusTagNumber, startingFeatureId, locus_tag, euk);
    if (cmd)
        cmd->Execute();
}

void ImportFeatTblTesting(const string& entry_file, const string& annot_file, const string& output_file)
{
    CRef<CSeq_entry> entry = ReadEntryFromFile(entry_file);
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    CSeq_entry_Handle orig_seh = scope->AddTopLevelSeqEntry(*entry);
    BOOST_REQUIRE(orig_seh);

    CRef<CSeq_annot> annot = ReadAnnotFromFile(annot_file);

    DoImportFTable(orig_seh, *annot);

    CRef<CSeq_entry> output = ReadEntryFromFile(output_file);
    CConstRef<CSeq_entry> updated_entry = orig_seh.GetCompleteSeq_entry();
    //cout << MSerial_AsnText << updated_entry.GetObject() << endl;
    BOOST_CHECK_EQUAL(MakeAsn(*output), MakeAsn(*updated_entry));
}

// Keep both the genbank accession number and the general tag of the protein ids
BOOST_AUTO_TEST_CASE(Test_GB_7376)
{
    ImportFeatTblTesting("FeatTableData/gb_7376.asn", "FeatTableData/gb_7376_tbl.txt", "FeatTableData/gb_7376_imported.asn");
}

//Keep general protein ids general, don't convert them into local ids
BOOST_AUTO_TEST_CASE(Test_GB_7453)
{
    ImportFeatTblTesting("FeatTableData/gb_7453.asn", "FeatTableData/gb_7453_tbl.txt", "FeatTableData/gb_7453_imported.asn");
}

// Use proper protein id for tRNA anticodon location
BOOST_AUTO_TEST_CASE(Test_GB_7668)
{
    ImportFeatTblTesting("FeatTableData/gb_7376.asn", "FeatTableData/gb_7668_tbl.txt", "FeatTableData/gb_7668_imported.asn");
}

// Remove orig_protein_id qualifiers
BOOST_AUTO_TEST_CASE(Test_GB_7030)
{
    ImportFeatTblTesting("FeatTableData/gb_7030.asn", "FeatTableData/gb_7030_tbl.txt", "FeatTableData/gb_7030_imported.asn");
}

BOOST_AUTO_TEST_CASE(Test_GB_8265)
{
    ImportFeatTblTesting("FeatTableData/gb_8265.asn", "FeatTableData/gb_8265_tbl.txt", "FeatTableData/gb_8265_imported.asn");
}

BOOST_AUTO_TEST_CASE(Test_GB_8871)
{
    ImportFeatTblTesting("FeatTableData/gb_8871.asn", "FeatTableData/gb_8871_tbl.txt", "FeatTableData/gb_8871_imported.asn");  
}

BOOST_AUTO_TEST_CASE(Test_GB_8922)
{
    ImportFeatTblTesting("FeatTableData/gb_8922.asn", "FeatTableData/gb_8922_tbl.txt", "FeatTableData/gb_8922_imported.asn");
}

BOOST_AUTO_TEST_CASE(Test_GB_8265_withoutproteinid)
{
    ImportFeatTblTesting("FeatTableData/gb_8265.asn", "FeatTableData/gb_8265_withoutproteinid_tbl.txt", "FeatTableData/gb_8265_withoutproteinid_imported.asn");
}
