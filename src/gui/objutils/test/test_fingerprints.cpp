/*  $Id: test_fingerprints.cpp 34688 2016-02-01 22:13:07Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/annot_selector.hpp>
#include <objmgr/feat_ci.hpp>

#include <util/checksum.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/obj_fingerprint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CTestFingerprintsApp : public CNcbiApplication
{
private:
    void Init();
    int Run();
};


void CTestFingerprintsApp::Init()
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Test: CObjFingerprint::GetFingerprint()");

    arg_desc->AddKey("acc", "Accession",
                     "Accession to test",
                     CArgDescriptions::eString);

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}

int CTestFingerprintsApp::Run()
{
    // Get arguments
    CArgs args = GetArgs();
    string id_str = args["acc"].AsString();

    cout << "Loading accession: " << id_str << endl;

    CSeq_id id;

    try {
        id.Set(id_str);
    }
    catch (CSeqIdException&) {
        cout << "Can't interpret id: " << id_str << endl;
        return -1;
    }

    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*obj_mgr);

    CScope scope(*obj_mgr);
    scope.AddDefaults();

    CBioseq_Handle handle = scope.GetBioseqHandle(id);
    if (!handle) {
        cout << "Failed to retrieve sequence for: " << id_str << endl;
        return -1;
    }

    SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    sel.SetResolveAll().SetNoMapping().SetSortOrder(sel.eSortOrder_None);
    sel.SetAnnotType(CSeq_annot::TData::e_Ftable);

    int count = 0;
    set<Uint4> crcs;
    vector<Uint4> failed;
    for (CFeat_CI iter(handle, sel);  iter;  ++iter) {
        CChecksum cs;
        _ASSERT(dynamic_cast<const CSeq_feat*>(&iter->GetOriginalFeature()));
        CObjFingerprint::GetFingerprint(cs, iter->GetOriginalFeature(), &scope);
        Uint4 crc = cs.GetChecksum();

        if (!crcs.insert(crc).second)
            failed.push_back(crc);

        count++;
    }

    if (failed.empty())
        cout << "Test succeeded!" << endl;
    else {
        cout << "Test failed: " << failed.size() << " feature(s) have the same CRC32." << endl;
        cout << "Dumping failed features..." << endl;

        CNcbiOfstream os("FPtest.txt");
        os << "Seq-id: " << id_str << endl << endl << MSerial_AsnText;

        vector<vector<CConstRef<CSeq_feat> > > feats;
        feats.resize(failed.size());

        for (CFeat_CI iter(handle, sel);  iter;  ++iter) {
            CChecksum cs;
            CObjFingerprint::GetFingerprint(cs, iter->GetOriginalFeature(), &scope);
            Uint4 crc = cs.GetChecksum();
            for (size_t i = 0; i < failed.size(); ++i) {
                if (failed[i] == crc)
                    feats[i].push_back(CConstRef<CSeq_feat>(&iter->GetOriginalFeature()));
            }
        }

        for (size_t i = 0; i < feats.size(); ++i) {
            os << "*** Duplicate fingerprints ***" << endl << endl;
            for (size_t j = 0; j < feats[i].size(); ++j)
                os << *feats[i][j] << endl;
        }

        os.close();
    }

    cout << "Processed " << count << " features." << endl;

    return 0;
}


END_NCBI_SCOPE
USING_SCOPE(ncbi);

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return CTestFingerprintsApp().AppMain(argc, argv);
}
