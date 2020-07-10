/*  $Id: test_bm.cpp 40942 2018-05-02 15:48:55Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>

#include "bmsparsevec.h"
#include "bmsparsevec_serial.h"
#include "bmsparsevec_compr.h"

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/util/sequence.hpp>

#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <objects/variation/Variation.hpp>
#include <objects/variation/VariantPlacement.hpp>
#include <objtools/snputil/snp_utils.hpp>

#include <gui/objutils/utils.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

typedef bm::sparse_vector<unsigned, bm::bvector<> > svector;
typedef bm::compressed_sparse_vector<unsigned, svector > compsvector;

class CTestBMApp : public CNcbiApplication
{
public:
    CTestBMApp();

private:
    virtual void Init(void);
    virtual int  Run(void);

    void x_SetupArgDescriptions();

    void x_Serialize(const unsigned char* buff, size_t size, const string& fileName);

    string m_HomeDir;
    int m_MaxCount = 100000;
};


CTestBMApp::CTestBMApp()
{
}


/////////////////////////////////////////////////////////////////////////////
//  Init test for all different types of arguments


void CTestBMApp::Init(void)
{
    x_SetupArgDescriptions();

    string dir, dir2;
    CDirEntry::SplitPath(GetProgramExecutablePath(), &dir);
    dir = CDirEntry::DeleteTrailingPathSeparator(dir);
    CDirEntry::SplitPath(dir, &dir2);
    dir = CDirEntry::ConcatPath(dir2, "etc");
    dir = CDirEntry::ConcatPath(dir, "accguide.txt");

    try {
        CSeq_id::LoadAccessionGuide(dir);
        LOG_POST( Info << "loaded sequence accession map from " << dir );
    }
    STD_CATCH_ALL( "CGBenchApplication::x_LoadGuiRegistry(): failed to load accession guide" );

    m_HomeDir = CDirEntry::ConcatPath(GetEnvironment().Get("APPDATA"), "GenomeWorkbench2");
}


void CTestBMApp::x_SetupArgDescriptions()
{
    unique_ptr<CArgDescriptions> arg_desc(new CArgDescriptions());
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "BitMagic test app");
    arg_desc->AddKey("acc", "acc", "Genbank accession", CArgDescriptions::eString);
    arg_desc->AddOptionalKey("max_count", "max_count", "Maximun number of SNPs", CArgDescriptions::eInteger);
    SetupArgDescriptions(arg_desc.release());
}



void CTestBMApp::x_Serialize(const unsigned char* buff, size_t size, const string& fileName)
{
    if (size == 0)
        return;

    string fullPath = CDirEntry::ConcatPath(m_HomeDir, fileName);
    CFileIO fio;
    fio.Open(fullPath, CFileIO::eCreate, CFileIO::eWrite);
    fio.Write(buff, size);
}


int CTestBMApp::Run(void)
{
    const CArgs& args = GetArgs();
    string accession = args["acc"].AsString();
    if (args["max_count"].HasValue()) {
        int max_count = args["max_count"].AsInteger();
        if (max_count > 0)
            m_MaxCount = max_count;
    }

    CRef<CObjectManager> objectManager = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*objectManager);

    CRef<CScope> scope(new CScope(*objectManager));
    scope->AddDefaults();

    CRef<CSeq_id> seqId;

    try {
        CSeq_id id(accession);
        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
        idh = sequence::GetId(idh, *scope, sequence::eGetId_Best);
        if (idh) {
            seqId.Reset(new CSeq_id());
            seqId->Assign(*idh.GetSeqId());
        }

    }
    catch (CSeqIdException&) {
    }

    if (!seqId) {
        cout << "Invalid accession: " << accession;
        return -1;
    }

    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeqFeatData::eSubtype_variation);
    sel.AddNamedAnnots("SNP").SetExcludeExternal(false).SetMaxSize(m_MaxCount);

    CRef<CSeq_loc> seqLoc(new CSeq_loc());
    seqLoc->SetWhole(*seqId);
    CFeat_CI feat_it(*scope, *seqLoc, sel);

    svector sv, svRsId, svRsId2(bm::use_null);

    // temp buffer to avoid unnecessary re-allocations
    BM_DECLARE_TEMP_BLOCK(tb)

    svector::size_type index = 0;
    for (; feat_it; ++feat_it) {

        if ((index % 10000) == 0)
            cout << index << endl;

        const CSeq_feat& or_feat(feat_it->GetOriginalFeature());

        // Grab the database info
        CConstRef<CDbtag> tag;
        CRef<CVariation> pVariation(new CVariation());
        if(!NSNPVariationHelper::ConvertFeat(*pVariation, or_feat))
            continue;

        // Retrieve gene associated with SNP
        tag.Reset();  // reuse tag reference.
        CConstRef<CSeq_feat> gene_feat;
        gene_feat = sequence::GetBestOverlapForSNP(or_feat,
                        CSeqFeatData::eSubtype_gene,
                        *scope);
        if (gene_feat) {
            tag = gene_feat->GetNamedDbxref("GeneID");
            if (!tag) {
                tag = gene_feat->GetNamedDbxref("LocusLink");
            }
        }
        if (tag) {
            pVariation->SetOther_ids().push_back(CRef<CDbtag>(const_cast<CDbtag*>(tag.GetPointer())));
        }

        int location = -1;
        if (pVariation->CanGetPlacements()) {
            const CVariation::TPlacements& VarPlacements(pVariation->GetPlacements());
            if (!VarPlacements.empty()) {
                CRef<CVariantPlacement> pPlacement(VarPlacements.front());
                if (pPlacement->CanGetLoc())
                    location = pPlacement->GetLoc().GetTotalRange().GetFrom() + 1;
            }
        }

        if (location < 0)
            continue;

        int rsid = -1;

        if (pVariation->GetId().CanGetTag()) {
            const auto& id = pVariation->GetId().GetTag();
            if (id.IsId())
                rsid = id.GetId();
        }

        if (rsid < 0)
            continue;

        sv[index] = location;
        svRsId[index] = rsid;
        svRsId2[location] = rsid;
        ++index;
    }

    cout << "Total number of SNPs: " << index << endl;

    {
        sv.optimize();
        bm::sparse_vector_serial_layout<svector> sv_lay;
        bm::sparse_vector_serialize(sv, sv_lay, tb);
        x_Serialize(sv_lay.buf(), sv_lay.size(), "snp_pos.bin");
        double bpsnp = sv_lay.size()*8.0 / index;
        cout << "SNP positions size: " << sv_lay.size() << "     Bits per SNP: " << bpsnp << endl;
    }

    {
        svRsId.optimize();
        bm::sparse_vector_serial_layout<svector> sv_lay;
        bm::sparse_vector_serialize(svRsId, sv_lay, tb);
        x_Serialize(sv_lay.buf(), sv_lay.size(), "snp_rsid.bin");
        double bpsnp = sv_lay.size()*8.0 / index;
        cout << "SNP RsIds size: " << sv_lay.size() << "     Bits per SNP: " << bpsnp << endl;
    }

    {
        compsvector compv;
        compv.load_from(svRsId2);
        compv.optimize();

        bm::sparse_vector_serial_layout<svector> sv_lay;
        bm::sparse_vector_serialize(compv.sv_, sv_lay, tb);
        x_Serialize(sv_lay.buf(), sv_lay.size(), "snp_rsid_sparse.bin");
        double bpsnp = sv_lay.size()*8.0 / index;
        cout << "Sparse SNP RsIds size: " << sv_lay.size() << "     Bits per SNP: " << bpsnp << endl;
    }

    return 0;
}

END_NCBI_SCOPE

USING_SCOPE(ncbi);

/////////////////////////////////////////////////////////////////////////////
//  MAIN

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    // Execute main application function
    return CTestBMApp().AppMain(argc, argv);
}
