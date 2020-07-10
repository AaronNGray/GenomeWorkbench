/*  $Id: unit_test_field_resolver.cpp 45050 2020-05-18 17:58:56Z asztalos $
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
*   Unit tests for resolving fields within the asn tree in order to get, set or
*   remove the value of the field.
*/

#include <ncbi_pch.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <serial/objistr.hpp>
#include <serial/serial.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seqfeat/PCRReaction.hpp>
#include <objects/seqfeat/PCRPrimer.hpp>
#include <objects/seqfeat/PCRPrimerSet.hpp>
#include <objects/seqfeat/PCRReactionSet.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_biodata.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(objects);
USING_SCOPE(macro);

NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}

template<class T>
string MakeAsn(const T& object)
{
    CNcbiOstrstream str;
    str << MSerial_AsnText << object;
    return CNcbiOstrstreamToString(str);
}

size_t TestFeaturesInRange(const CMacroBioData& data, const SFeatInterval& interval)
{
    CIRef<IMacroBioDataIter> data_iter(data.CreateIterator(CMacroBioData::sm_Gene, kEmptyStr, interval));
    BOOST_CHECK(data_iter);

    vector<const CSeq_feat*> features;

    data_iter->Begin();
    while (!data_iter->IsEnd()) {
        CConstRef<CObject> obj = data_iter->GetScopedObject().object;
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
        features.push_back(feat);
        data_iter->Next();
    }

    /*for (auto& it : features) {
        cout << MSerial_AsnText << *it << endl;
    }
    */

    return features.size();
}

BOOST_AUTO_TEST_CASE(Test_FeatureIterator)
{
    CRef<CSeq_entry> entry(new CSeq_entry);
    string fname("genes.asn");
    try {
        CNcbiIfstream istr(fname.c_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *entry;
    }
    catch (const CException& e) {
        LOG_POST(Error << e.ReportAll());
        return;
    }

    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    CSeq_entry_Handle tse = scope->AddTopLevelSeqEntry(*entry);
    BOOST_CHECK(tse);

    CMacroBioData bio_data(tse);

    // Full sequence length: [0, 509]
    TSeqPos start = 0;
    TSeqPos stop = 509;
    SFeatInterval feat_int(TSeqRange(start, stop));
    feat_int.left_closed = true;
    feat_int.right_closed = true;
    cout << "[" << start << ", " << stop << "]" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 31);

    // [100, 400]
    start = 100;
    stop = 400;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = true;
    feat_int.right_closed = true;
    cout << "\n\n[" << start << ", " << stop << "]" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 23);

    // [0, 100]
    start = 0;
    stop = 100;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = true;
    feat_int.right_closed = true;
    cout << "\n\n[" << start << ", " << stop << "]" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 14);

    // (100, 400]
    start = 100;
    stop = 400;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = false;
    feat_int.right_closed = true;
    cout << "\n\n(" << start << ", " << stop << "]" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 13);
    // the genes with [0, 509] +|- positions are not included

    // [0, 400]
    start = 0;
    stop = 400;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = true;
    feat_int.right_closed = true;
    cout << "\n\n[" << start << ", " << stop << "]" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 27);

    // [100, 400)
    start = 100;
    stop = 400;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = true;
    feat_int.right_closed = false;
    cout << "\n\n[" << start << ", " << stop << ")" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 13);
    // the genes with [0, 509] +|- positions are not included

    // (100, 400)
    start = 100;
    stop = 400;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = false;
    feat_int.right_closed = false;
    cout << "\n\n(" << start << ", " << stop << ")" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 5);

    // (100, 100]
    start = 100;
    stop = 100;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = false;
    feat_int.right_closed = true;
    cout << "\n\n(" << start << ", " << stop << "]" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 0);
    // no features are selected as features that start at 100 are not included

    // [100, 100]
    start = 100;
    stop = 100;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = true;
    feat_int.right_closed = true;
    cout << "\n\n[" << start << ", " << stop << "]" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 10);
    // include all features that start at 100 or end at 100 or they cross the 100 sequence position

    // [100, 100)
    start = 100;
    stop = 100;
    feat_int.m_Range = TSeqRange(start, stop);
    feat_int.left_closed = true;
    feat_int.right_closed = false;
    cout << "\n\n[" << start << ", " << stop << ")" << endl;
    BOOST_CHECK(TestFeaturesInRange(bio_data, feat_int) == 0);
    // no features are selected as features that end at 100 are not included

}

BOOST_AUTO_TEST_CASE(Test_BioSource_Enum)
{
    CRef<CBioSource> bsrc(new CBioSource());
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  org {\n"
                "  }\n"
                "}\n");

    CObjectInfo oi(bsrc.GetPointer(), bsrc->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;
    
    // Get a valid, non-existing field
    bool get = GetFieldsByName(&result, oi, "genome");
    BOOST_CHECK(get);
    BOOST_CHECK(result.empty());

    // Set a field of enumerated type
    bool set = SetFieldsByName(&result, oi, "genome");
    BOOST_CHECK(set);
    BOOST_CHECK(result.front().field.GetPrimitiveValueType() == ePrimitiveValueEnum);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "  }\n"
                "}\n");
    
    bsrc->SetGenome(CBioSource::eGenome_genomic);

    // Get a field of enumerated type
    result.clear();
    get = GetFieldsByName(&result, oi, "genome");
    BOOST_CHECK(get);
    BOOST_CHECK(result.front().field.GetPrimitiveValueString() == "genomic");
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "  }\n"
                "}\n");

    // delete a valid, existing field of enumerated type
    bool remove = RemoveFieldByName(result.front());
    BOOST_CHECK(remove);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "  }\n"
                "}\n");
}

BOOST_AUTO_TEST_CASE(Test_BioSource_Container)
{
    CRef<CBioSource> bsrc(new CBioSource());
    string source_str = 
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  },\n"
                "  pcr-primers {\n"
                "    {\n"
                "      forward {\n"
                "        {\n"
                "          seq \"aggctctctctcaa\",\n"
                "          name \"nbam24\"\n"
                "        }\n"
                "      },\n"
                "      reverse {\n"
                "        {\n"
                "          seq \"aggctctctctcaa\",\n"
                "          name \"nbam24\"\n"
                "        }\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n";


    
    CNcbiIstrstream istr(source_str.c_str());
    istr >> MSerial_AsnText >> *bsrc;

    CObjectInfo oi(bsrc.GetPointer(), bsrc->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;

    // Get an element from a container
    bool get = GetFieldsByName(&result, oi, "pcr-primers..reverse..name");
    BOOST_CHECK(get);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.front().field.GetTypeFamily() == eTypeFamilyPointer);


    CRef<CPCRPrimer> primer(new CPCRPrimer);
    primer->SetName().Set("nbam33");
    primer->SetSeq().Set("aggggaaattt");

    CRef<CPCRReaction> reaction(new CPCRReaction);
    reaction->SetForward().Set().push_back(primer);
    bsrc->SetPcr_primers().Set().push_back(reaction);

    // get multiple elements from a container
    result.clear();
    get = GetFieldsByName(&result, oi, "pcr-primers..forward..name");
    BOOST_CHECK(get);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result.front().field.GetTypeFamily() == eTypeFamilyPointer);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  },\n"
                "  pcr-primers {\n"
                "    {\n"
                "      forward {\n"
                "        {\n"
                "          seq \"aggctctctctcaa\",\n"
                "          name \"nbam24\"\n"
                "        }\n"
                "      },\n"
                "      reverse {\n"
                "        {\n"
                "          seq \"aggctctctctcaa\",\n"
                "          name \"nbam24\"\n"
                "        }\n"
                "      }\n"
                "    },\n"
                "    {\n"
                "      forward {\n"
                "        {\n"
                "          seq \"aggggaaattt\",\n"
                "          name \"nbam33\"\n"
                "        }\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");

    
    // rearrange containers and get multiple elements from it
    bsrc->Reset();
    source_str =
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  },\n"
                "  pcr-primers {\n"
                "    {\n"
                "      forward {\n"
                "        {\n"
                "          seq \"aggctctctctcaa\",\n"
                "          name \"nbam24\""
                "        },\n"
                "        {\n"
                "          seq \"aggctctaagatatt\",\n"
                "          name \"nbam33\""
                "        }\n"
                "      },\n"
                "      reverse {\n"
                "        {\n"
                "          seq \"aggctctctctcaa\",\n"
                "          name \"nbam24\""
                "        },\n"
                "        {\n"
                "          seq \"aggctctaagatatt\",\n"
                "          name \"nbam33\""
                "        }\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n";

    CNcbiIstrstream istr1(source_str.c_str());
    istr1 >> MSerial_AsnText >> *bsrc;
    result.clear();
    get = GetFieldsByName(&result, oi, "pcr-primers..forward..seq");
    BOOST_CHECK(get);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result.front().field.GetTypeFamily() == eTypeFamilyPointer);
}


BOOST_AUTO_TEST_CASE(Test_BioSource_String)
{
    CRef<CBioSource> bsrc(new CBioSource());
    CRef<COrg_ref> org_ref(new COrg_ref());
    org_ref->SetTaxId(371907);
    org_ref->SetTaxname("Bubo scandiacus");
    bsrc->SetOrg(*org_ref);
    bsrc->SetGenome(CBioSource::eGenome_genomic);

    CObjectInfo oi(bsrc.GetPointer(), bsrc->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;

    // Get an existing string
    bool get = GetFieldsByName(&result, oi, "org.taxname");
    BOOST_CHECK(get);
    BOOST_CHECK_EQUAL(result.front().field.GetPrimitiveValueString(), string("Bubo scandiacus"));
    //

    // Try to get a non-valid field
    result.clear();
    get = GetFieldsByName(&result, oi, "org.commn");
    BOOST_CHECK(!get);
    BOOST_CHECK(result.empty());

    // Try to get a valid, nonexisting string
    result.clear();
    get = GetFieldsByName(&result, oi, "org.common");
    BOOST_CHECK(get);
    BOOST_CHECK(result.empty());
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
    //

    // Try to set a non-valid field  
    result.clear();
    bool set = SetFieldsByName(&result, oi, "org.commn");
    BOOST_CHECK(!set);
    BOOST_CHECK(result.empty());
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
    //

    // Set a field of string type
    result.clear();
    set = SetFieldsByName(&result, oi, "org.common");
    BOOST_CHECK(set);
    BOOST_CHECK(result.front().field.GetPrimitiveValueType() == ePrimitiveValueString);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"\",\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
    //
    bsrc->SetOrg().SetCommon("snowy owl");

    // Set a field that is valid and it exists
    result.clear();
    set = SetFieldsByName(&result, oi, "org.common");
    BOOST_CHECK(set);
    BOOST_CHECK_EQUAL(result.front().field.GetPrimitiveValueString(), string("snowy owl"));
    //

    // Remove the valid, existing field
    result.clear();
    get = GetFieldsByName(&result, oi, "org.common");
    BOOST_CHECK(get);
    bool remove = RemoveFieldByName(result.front());
    BOOST_CHECK(remove);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
    //
}


BOOST_AUTO_TEST_CASE(Test_BioSource_Container1)
{
    CRef<CBioSource> bsrc(new CBioSource);
    string src_string =
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n";

    CNcbiIstrstream istr(src_string.c_str());
    istr >> MSerial_AsnText >> *bsrc;

    CObjectInfo oi(bsrc.GetPointer(), bsrc->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;

    // Resolve a container of strings
    result.clear();
    bool get = GetFieldsByName(&result, oi, "org.mod");
    BOOST_CHECK(get);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.front().field.GetTypeFamily() == eTypeFamilyContainer);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
    //

    // Get a non-existent, valid field with similar name to "org.mod"
    result.clear();
    get = GetFieldsByName(&result, oi, "org.orgname.mod");
    BOOST_CHECK(!get);
    BOOST_CHECK(result.empty());
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
    //

    // Set a field with a similar name to "org.mod"
    result.clear();
    bool set = SetFieldsByName(&result, oi, "org.orgname.mod");
    BOOST_CHECK(set);
    BOOST_CHECK(result.front().field.GetTypeFamily() == eTypeFamilyContainer);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");

    // Set a class member of type int
    result.clear();
    set = SetFieldsByName(&result, oi, "org.orgname.mgcode");
    BOOST_CHECK(set);
    BOOST_CHECK(result.front().field.GetPrimitiveValueType() == ePrimitiveValueInteger);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "      },\n"
                "      mgcode 0\n"
                "    }\n"
                "  }\n"
                "}\n");
    

    bsrc->SetOrg().SetOrgname().SetMgcode(5);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "      },\n"
                "      mgcode 5\n"
                "    }\n"
                "  }\n"
                "}\n");


    CRef<COrgMod> orgmod1(new COrgMod(COrgMod::eSubtype_ecotype, "eco_type"));
    CRef<COrgMod> orgmod2(new COrgMod(COrgMod::eSubtype_other, "note about the owl"));
    bsrc->SetOrg().SetOrgname().SetMod().push_back(orgmod1);
    bsrc->SetOrg().SetOrgname().SetMod().push_back(orgmod2);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "        {\n"
                "          subtype ecotype,\n"
                "          subname \"eco_type\"\n"
                "        },\n"
                "        {\n"
                "          subtype other,\n"
                "          subname \"note about the owl\"\n"
                "        }\n"
                "      },\n"
                "      mgcode 5\n"
                "    }\n"
                "  }\n"
                "}\n");
        //


    // Remove an element from the container:
    result.clear();
    get = GetFieldsByName(&result, oi, "org.orgname.mod");
    BOOST_CHECK(get);
    CObjectInfo cont_oi = result.front().field;

    CObjectInfo orgmod_oi1(orgmod1.GetPointer(), orgmod1->GetThisTypeInfo());
    CMQueryNodeValue::SResolvedField res_field1(cont_oi, orgmod_oi1);

    bool remove = RemoveFieldByName(res_field1);
    BOOST_CHECK(remove);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "        {\n"
                "          subtype other,\n"
                "          subname \"note about the owl\"\n"
                "        }\n"
                "      },\n"
                "      mgcode 5\n"
                "    }\n"
                "  }\n"
                "}\n");
        //

    // Remove the last element from the container
    CObjectInfo orgmod_oi2(orgmod2.GetPointer(), orgmod2->GetThisTypeInfo());
    CMQueryNodeValue::SResolvedField res_field2(cont_oi, orgmod_oi2);
    remove = RemoveFieldByName(res_field2);
    BOOST_CHECK(remove);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "      },\n"
                "      mgcode 5\n"
                "    }\n"
                "  }\n"
                "}\n");
        //

    // If the Orgname would have had only one orgmod,
    // deleting that orgmod should have deleted the Orgname object
    bsrc->SetOrg().SetOrgname().SetMod().push_back(orgmod2);

    // Remove an int type
    result.clear();
    get = GetFieldsByName(&result, oi, "org.orgname.mgcode");
    BOOST_CHECK(get);

    remove = RemoveFieldByName(result.front());
    BOOST_CHECK(remove);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "        {\n"
                "          subtype other,\n"
                "          subname \"note about the owl\"\n"
                "        }\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
        //

    // Remove the last element from the container:
    // It WILL NOT delete the empty orgname member!
    remove = RemoveFieldByName(res_field2);
    BOOST_CHECK(remove);
    BOOST_CHECK_EQUAL(MakeAsn(*bsrc),
                "BioSource ::= {\n"
                "  genome unknown,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "      }\n"
                "    }\n"
                "  }\n"
                "}\n");
        //
}

BOOST_AUTO_TEST_CASE(Test_Lineage_Time)
{
    // measure the time to resolve org.orgname.lineage
    CRef<CBioSource> bsrc(new CBioSource);
    string source_str =
            "BioSource ::= {\n"
                "  genome genomic,\n"
                "  org {\n"
                "    taxname \"Bubo scandiacus\",\n"
                "    common \"snowy owl\",\n"
                "    mod {\n"
                "      \"modifier_01\",\n"
                "      \"modifier_02\",\n"
                "      \"modifier_03\"\n"
                "    },\n"
                "    db {\n"
                "      {\n"
                "        db \"taxon\",\n"
                "        tag id 371907\n"
                "      }\n"
                "    },\n"
                "    orgname {\n"
                "      mod {\n"
                "        {\n"
                "          subtype ecotype,\n"
                "          subname \"eco_type\"\n"
                "        },\n"
                "        {\n"
                "          subtype strain,\n"
                "          subname \"test_strain\"\n"
                "        },\n"
                "        {\n"
                "          subtype serovar,\n"
                "          subname \"test_serovar\"\n"
                "        },\n"
                "        {\n"
                "          subtype group,\n"
                "          subname \"II\"\n"
                "        },\n"
                "        {\n"
                "          subtype other,\n"
                "          subname \"note about the owl\"\n"
                "        }\n"
                "      },\n"
                "      mgcode 5\n"
                "    }\n"
                "  }\n"
                "}\n";

    CNcbiIstrstream istr(source_str.c_str());
    istr >> MSerial_AsnText >> *bsrc;

    CObjectInfo oi(bsrc.GetPointer(), bsrc->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;

    CStopWatch timer;

    result.clear();
    timer.Start();
    bool get = GetFieldsByName(&result, oi, "org.orgname.lineage");
    timer.Stop();
    //LOG_POST("Elapsed time: " << timer.AsSmartString(CTimeSpan::eSSP_Microsecond));
    BOOST_CHECK(get);
    BOOST_CHECK(result.empty());
}

BOOST_AUTO_TEST_CASE(Test_Bioseq_Choice)
{
    CRef<CBioseq> bseq(new CBioseq);
    CRef<CSeq_id> seq_id(new CSeq_id);
    seq_id->SetLocal().SetStr("Seq_01");
    bseq->SetId().push_back(seq_id);

    bseq->SetInst().SetMol(CSeq_inst::eMol_dna);
    bseq->SetInst().SetRepr(CSeq_inst::eRepr_raw);
    bseq->SetInst().SetSeq_data().SetIupacna().Set("AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA");
    bseq->SetInst().SetLength(40);

    CObjectInfo oi(bseq.GetPointer(), bseq->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;

    // Get a non-existent, valid choice variant
    bool get = GetFieldsByName(&result, oi, "descr..molinfo");
    BOOST_CHECK(!get); // the choice variant is not set
    BOOST_CHECK(result.empty());
    BOOST_CHECK_EQUAL(MakeAsn(*bseq),
                "Bioseq ::= {\n"
                "  id {\n"
                "    local str \"Seq_01\"\n"
                "  },\n"
                "  inst {\n"
                "    repr raw,\n"
                "    mol dna,\n"
                "    length 40,\n"
                "    seq-data iupacna \"AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA\"\n"
                "  }\n"
                "}\n");
    //

    CRef<CSeqdesc> desc_molinfo(new CSeqdesc);
    desc_molinfo->SetMolinfo().SetBiomol(CMolInfo::eBiomol_genomic);
    desc_molinfo->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_complete);

    CRef<CSeqdesc> desc_title(new CSeqdesc);
    desc_title->SetTitle("The title of this sequence");

    CRef<CSeqdesc> desc_user(new CSeqdesc);
    desc_user->SetUser().SetType().SetStr("Structured Comment");
    desc_user->SetUser().AddField("StructuredCommentPrefix", string("##Assembly-Data-START##"));
    desc_user->SetUser().AddField("Sequencing Technology", string("Sanger dideoxy sequencing"));
    desc_user->SetUser().AddField("StructuredCommentSuffix", string("##Assembly-Data-END##"));

    bseq->SetDescr().Set().push_back(desc_molinfo);
    bseq->SetDescr().Set().push_back(desc_title);
    bseq->SetDescr().Set().push_back(desc_user);
    
    // Get a non-existent, non-valid choice variant

    // Get the first descriptor:
    result.clear();
    get = GetFieldsByName(&result, oi, "descr..molinfo.biomol");
    BOOST_CHECK(get);
    BOOST_CHECK_EQUAL(result.front().field.GetPrimitiveValueString(), "genomic");
    BOOST_CHECK_EQUAL(MakeAsn(*bseq),
                "Bioseq ::= {\n"
                "  id {\n"
                "    local str \"Seq_01\"\n"
                "  },\n"
                "  descr {\n"
                "    molinfo {\n"
                "      biomol genomic,\n"
                "      completeness complete\n"
                "    },\n"
                "    title \"The title of this sequence\",\n"
                "    user {\n"
                "      type str \"Structured Comment\",\n"
                "      data {\n"
                "        {\n"
                "          label str \"StructuredCommentPrefix\",\n"
                "          data str \"##Assembly-Data-START##\"\n"
                "        },\n"
                "        {\n"
                "          label str \"Sequencing Technology\",\n"
                "          data str \"Sanger dideoxy sequencing\"\n"
                "        },\n"
                "        {\n"
                "          label str \"StructuredCommentSuffix\",\n"
                "          data str \"##Assembly-Data-END##\"\n"
                "        }\n"
                "      }\n"
                "    }\n"
                "  },\n"
                "  inst {\n"
                "    repr raw,\n"
                "    mol dna,\n"
                "    length 40,\n"
                "    seq-data iupacna \"AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA\"\n"
                "  }\n"
                "}\n");
    //

    // Get the second descriptor: 
    result.clear();
    get = GetFieldsByName(&result, oi, "descr..title");
    BOOST_CHECK(get);
    BOOST_CHECK_EQUAL(result.front().field.GetPrimitiveValueString(), string("The title of this sequence"));
    // 

    // Get the third descriptor:
    result.clear();
    get = GetFieldsByName(&result, oi, "descr..user.data");
    BOOST_CHECK(get);
    BOOST_CHECK(result.front().field.GetTypeFamily() == eTypeFamilyContainer);


    // Get an int
    result.clear();
    get = GetFieldsByName(&result, oi, "inst.length");
    BOOST_CHECK(get);
    BOOST_CHECK(result.front().field.GetPrimitiveValueType() == ePrimitiveValueInteger);
    //


    // Try to remove a non-optional, valid field
    result.clear();
    get = GetFieldsByName(&result, oi, "inst.repr");
    BOOST_CHECK(get);
    BOOST_CHECK(result.front().field.GetPrimitiveValueString() == "raw");

    bool remove = RemoveFieldByName(result.front());
    BOOST_CHECK(!remove);
    BOOST_CHECK_EQUAL(MakeAsn(bseq->GetInst()),
                "Seq-inst ::= {\n"
                "  repr raw,\n"
                "  mol dna,\n"
                "  length 40,\n"
                "  seq-data iupacna \"AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA\"\n"
                "}\n");
}

BOOST_AUTO_TEST_CASE(Test_Bioseq_SameDescriptor)
{
    CRef<CBioseq> bseq(new CBioseq);
    CRef<CSeq_id> seq_id(new CSeq_id);
    seq_id->SetLocal().SetStr("Seq_01");
    bseq->SetId().push_back(seq_id);

    bseq->SetInst().SetMol(CSeq_inst::eMol_dna);
    bseq->SetInst().SetRepr(CSeq_inst::eRepr_raw);
    bseq->SetInst().SetSeq_data().SetIupacna().Set("AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA");
    bseq->SetInst().SetLength(40);

    CObjectInfo oi(bseq.GetPointer(), bseq->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;
    bool set = SetFieldsByName(&result, oi, "descr..molinfo.biomol");
    BOOST_CHECK(set);
    BOOST_CHECK_EQUAL(result.front().field.GetPrimitiveValueString(), "unknown");
    BOOST_CHECK_EQUAL(MakeAsn(*bseq),
                "Bioseq ::= {\n"
                "  id {\n"
                "    local str \"Seq_01\"\n"
                "  },\n"
                "  descr {\n"
                "    molinfo {\n"
                "      biomol unknown\n"
                "    }\n"
                "  },\n"
                "  inst {\n"
                "    repr raw,\n"
                "    mol dna,\n"
                "    length 40,\n"
                "    seq-data iupacna \"AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA\"\n"
                "  }\n"
                "}\n");

    CRef<CSeqdesc> desc_title(new CSeqdesc);
    desc_title->SetTitle("The title of this sequence");
    bseq->SetDescr().Set().push_back(desc_title);

    // set another molinfo field
    result.clear();
    set = SetFieldsByName(&result, oi, "descr..molinfo.completeness");
    BOOST_CHECK(set);
    BOOST_CHECK_EQUAL(result.front().field.GetPrimitiveValueString(), "unknown");
    BOOST_CHECK_EQUAL(MakeAsn(*bseq),
                "Bioseq ::= {\n"
                "  id {\n"
                "    local str \"Seq_01\"\n"
                "  },\n"
                "  descr {\n"
                "    molinfo {\n"
                "      biomol unknown,\n"
                "      completeness unknown\n"
                "    },\n"
                "    title \"The title of this sequence\"\n"
                "  },\n"
                "  inst {\n"
                "    repr raw,\n"
                "    mol dna,\n"
                "    length 40,\n"
                "    seq-data iupacna \"AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA\"\n"
                "  }\n"
                "}\n");

}

BOOST_AUTO_TEST_CASE(Test_SetGeneFeature)
{
    CRef<CSeq_feat> feat(new CSeq_feat);
    CRef<CSeq_id> seq_id(new CSeq_id);
    seq_id->SetLocal().SetStr("Seq_01");
    CRef<CSeq_interval> interval(new CSeq_interval(*seq_id, 10, 15));
    feat->SetLocation().SetInt(*interval);

    CObjectInfo oi(feat, feat->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;
    bool set = SetFieldsByName(&result, oi, "data.gene.locus");
    BOOST_CHECK(set);
    BOOST_CHECK_EQUAL(MakeAsn(*feat),
                "Seq-feat ::= {\n"
                "  data gene {\n"
                "    locus \"\"\n"
                "  },\n"
                "  location int {\n"
                "    from 10,\n"
                "    to 15,\n"
                "    id local str \"Seq_01\"\n"
                "  }\n"
                "}\n");

    result.clear();
    set = SetFieldsByName(&result, oi, "qual.qual");
    BOOST_CHECK(set);
    result.clear();
    set = SetFieldsByName(&result, oi, "qual.val");
    BOOST_CHECK(set);
    BOOST_CHECK_EQUAL(MakeAsn(*feat),
                "Seq-feat ::= {\n"
                "  data gene {\n"
                "    locus \"\"\n"
                "  },\n"
                "  location int {\n"
                "    from 10,\n"
                "    to 15,\n"
                "    id local str \"Seq_01\"\n"
                "  },\n"
                "  qual {\n"
                "    {\n"
                "      qual \"\",\n"
                "      val \"\"\n"
                "    }\n"
                "  }\n"
                "}\n");
}

BOOST_AUTO_TEST_CASE(Test_SetProteinName)
{
    CRef<CSeq_feat> feat(new CSeq_feat);
    CRef<CSeq_id> seq_id(new CSeq_id);
    seq_id->SetLocal().SetStr("Seq_01");
    CRef<CSeq_interval> interval(new CSeq_interval(*seq_id, 10, 15));
    feat->SetLocation().SetInt(*interval);
    feat->SetData().SetProt();

    CObjectInfo oi(feat, feat->GetThisTypeInfo());
    CMQueryNodeValue::TObs result;
    bool set = SetFieldsByName(&result, oi, "data.prot.name");
    BOOST_CHECK(set);
    BOOST_CHECK(result.front().field.GetTypeFamily() == eTypeFamilyContainer);
    CObjectInfo new_oi(result.front().field.AddNewElement());
    new_oi.SetPrimitiveValueString("new protein name");
    BOOST_CHECK_EQUAL(feat->GetData().GetProt().GetName().front(), "new protein name");
}

BOOST_AUTO_TEST_CASE(Test_Bioseq_MultipleSameDescriptor)
{
    CRef<CBioseq> bseq(new CBioseq);
    CRef<CSeq_id> seq_id(new CSeq_id);
    seq_id->SetLocal().SetStr("Seq_01");
    bseq->SetId().push_back(seq_id);

    bseq->SetInst().SetMol(CSeq_inst::eMol_dna);
    bseq->SetInst().SetRepr(CSeq_inst::eRepr_raw);
    bseq->SetInst().SetSeq_data().SetIupacna().Set("AATTGGCCAAAATTGGCCAAAATTGGCCAAAATTGGCCAA");
    bseq->SetInst().SetLength(40);

    CRef<CSeqdesc> desc_molinfo1(new CSeqdesc);
    desc_molinfo1->SetMolinfo().SetBiomol(CMolInfo::eBiomol_genomic);
    desc_molinfo1->SetMolinfo().SetCompleteness(CMolInfo::eCompleteness_complete);

    CRef<CSeqdesc> desc_molinfo2(new CSeqdesc);
    desc_molinfo2->SetMolinfo().SetBiomol(CMolInfo::eBiomol_mRNA);

    bseq->SetDescr().Set().push_back(desc_molinfo1);
    bseq->SetDescr().Set().push_back(desc_molinfo2);

    CMQueryNodeValue::TObs result;
    CObjectInfo oi(bseq.GetPointer(), bseq->GetThisTypeInfo());
    bool get = GetFieldsByName(&result, oi, "descr..molinfo.biomol");
    BOOST_CHECK(get);
    BOOST_CHECK_EQUAL(result.front().field.GetPrimitiveValueString(), "genomic");
}
