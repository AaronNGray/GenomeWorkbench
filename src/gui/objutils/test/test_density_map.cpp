/*  $Id: test_density_map.cpp 34686 2016-02-01 22:10:56Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbitime.hpp>

#include <objmgr/object_manager.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/density_map.hpp>

using namespace ncbi;
using namespace ncbi::objects;

class CTestDensityMap : public CNcbiApplication
{
public:

    void Init();
    int Run();
};


void CTestDensityMap::Init(void)
{
    // Prepare command line descriptions
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // input file containing the SAGE data (default = stdin)
    arg_desc->AddKey("id", "Accession", "Accession to test",
                     CArgDescriptions::eString);

    arg_desc->AddDefaultKey("iters", "Iterations", "Iterations to test",
                            CArgDescriptions::eInteger, "10");

    arg_desc->AddDefaultKey("window", "WindowSize", "Number of bases per bin",
                            CArgDescriptions::eInteger, "10000");

    // Pass argument descriptions to the application
    //

    SetupArgDescriptions(arg_desc.release());
}


int CTestDensityMap::Run(void)
{
    CArgs args = GetArgs();

    string id_str = args["id"].AsString();
    int iters = args["iters"].AsInteger();
    int window = args["window"].AsInteger();

    CSeq_id id;
    try {
        id.Set(id_str);
    }
    catch (CSeqIdException&) {
        LOG_POST(Fatal << "can't interpret id = " << id_str);
    }

    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*obj_mgr);
    CScope scope(*obj_mgr);
    scope.AddDefaults();

    CBioseq_Handle handle = scope.GetBioseqHandle(id);

    //
    // first pass: retrieves all features.  We can ignore this pass
    //
    SAnnotSelector sel;
    sel.SetAnnotType(CSeq_annot::TData::e_Ftable)
        .SetOverlapType(SAnnotSelector::eOverlap_TotalRange)
        .SetResolveMethod(SAnnotSelector::eResolve_All)
        .SetDataSource("")
        .SetFeatType(CSeqFeatData::e_Gene)
        ;

    {{
         CFeat_CI iter(handle, sel);
         cout << "iterations:   " << iters << endl;
         cout << "seq size:     " << handle.GetSeqVector().size() << endl;
         cout << "window:       " << window << endl;
         cout << id_str << ":    " << iter.GetSize() << " features" << endl;
     }}


    CStopWatch sw;
    sw.Start();
    TSeqPos sum = 0;
    int i;
    for (i = 0;  i < iters;  ++i) {
        CFeat_CI iter(handle, sel);
        sum += iter.GetSize();
    }
    double e1 = sw.Elapsed();
    cout << "overhead time: " << e1 << " seconds" << endl;
    cout << "overhead/iter: " << e1 / double(iters) << endl;
    cout << endl;

    sw.Start();

    TSeqPos max = 0;
    vector<TSeqPos> bins;
    for (i = 0;  i < iters;  ++i) {
        sum += max = CDensityMap<int>::GetDensityMap(handle, 0, 0, window, sel, bins);
    }
    double e2 = sw.Elapsed();
    cout << "density map:   " << bins.size() << endl;
    cout << "map max:       " << max << endl;
    cout << "map time:      " << e2 << " seconds" << endl;
    cout << "map time/iter: " << (e2) / double(iters) << endl;
    cout << endl;

    sw.Start();

    CDensityMap<int> new_bins(handle, window);
    for (i = 0; i < iters; ++i ) {
        new_bins.Clear();
        new_bins.AddFeatures(handle, sel);
    }
    double e3 = sw.Elapsed();

    cout << "new bins:      " << new_bins.GetBins() << endl;
    cout << "new bins max:  " << new_bins.GetMax() << endl;
    cout << "new bins time: " << e3 << " seconds" << endl;
    cout << "new time/iter: " << (e3) / double(iters) << endl;

    return 0;
}


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return CTestDensityMap().AppMain(argc, argv);
}
