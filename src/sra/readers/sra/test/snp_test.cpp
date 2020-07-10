/*  $Id: snp_test.cpp 593619 2019-09-20 19:31:56Z vasilche $
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
 * Authors:  Eugene Vasilchenko
 *
 * File Description:
 *   Sample test application for SNP reader
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbi_system.hpp>
#include <sra/readers/sra/snpread.hpp>

#include <objects/general/general__.hpp>
#include <objects/seqloc/seqloc__.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqres/seqres__.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqtable/seqtable__.hpp>
#include <objmgr/impl/snp_annot_info.hpp>

#include <serial/serial.hpp>
#include <thread>

#include <common/test_assert.h>  /* This header must go last */

USING_NCBI_SCOPE;
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
//  CSNPTestApp::


class CSNPTestApp : public CNcbiApplication
{
private:
    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


/////////////////////////////////////////////////////////////////////////////
//  Init test

void CSNPTestApp::Init(void)
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "snp_test");

    arg_desc->AddKey("file", "File",
                     "SNP file name, accession, or prefix",
                     CArgDescriptions::eString);

    arg_desc->AddFlag("seq_table", "Dump sequence table");
    arg_desc->AddFlag("track_table", "Dump filtered tracks table");

    arg_desc->AddOptionalKey("q", "Query",
                             "Query coordinates in form chr1:100-10000",
                             CArgDescriptions::eString);
    arg_desc->AddOptionalKey("seq", "SeqId",
                             "Seq id",
                             CArgDescriptions::eString);
    arg_desc->AddOptionalKey("seq-index", "SeqIindex",
                             "Sequence index in the file",
                             CArgDescriptions::eInteger);
    arg_desc->AddDefaultKey("pos", "SeqPos",
                            "Seq position",
                            CArgDescriptions::eInteger,
                            "0");
    arg_desc->AddOptionalKey("poss", "SeqPoss",
                            "Seq positions",
                            CArgDescriptions::eString);
    arg_desc->AddDefaultKey("window", "SeqWindow",
                            "Seq window",
                            CArgDescriptions::eInteger,
                            "0");
    arg_desc->AddOptionalKey("end", "SeqEnd",
                            "Seq end position",
                            CArgDescriptions::eInteger);

    arg_desc->AddOptionalKey("track", "Track",
                             "Filtered track",
                             CArgDescriptions::eString);

    arg_desc->AddDefaultKey("limit_count", "LimitCount",
                            "Number of entries to read (0 - unlimited)",
                            CArgDescriptions::eInteger,
                            "100");

    arg_desc->AddOptionalKey("filter", "Filter",
                             "Quality codes filter",
                             CArgDescriptions::eInt8);
    arg_desc->AddOptionalKey("filter-mask", "FilterMask",
                             "Quality codes filter mask",
                             CArgDescriptions::eInt8);

    arg_desc->AddFlag("verbose", "Print info about found data");

    arg_desc->AddFlag("make_feat", "Make feature object");
    arg_desc->AddDefaultKey("chunk", "ChunkSize",
                            "chunk size in sequence coordinates",
                            CArgDescriptions::eInteger, "2000000000");
    arg_desc->AddFlag("make_feat_annot", "Make feature annot");
    arg_desc->AddFlag("make_table_feat_annot", "Make feature Seq-table annot");
    arg_desc->AddFlag("make_packed_feat_annot", "Make packed feature annot");
    arg_desc->AddFlag("make_cov_graph", "Make coverage graph");
    arg_desc->AddFlag("make_cov_annot", "Make coverage annot");
    arg_desc->AddFlag("make_overview_graph", "Make overview graph");
    arg_desc->AddFlag("make_overview_annot", "Make overview annot");
    arg_desc->AddFlag("no_shared_objects", "Do not share created objects");
    arg_desc->AddFlag("print_objects", "Print generated objects");

    arg_desc->AddFlag("test", "Test consistency");
#ifdef NCBI_THREADS
    arg_desc->AddFlag("test-seq-mt", "Run MT scan of all sequences");
#endif

    arg_desc->AddDefaultKey("o", "OutputFile",
                            "Output file of ASN.1",
                            CArgDescriptions::eOutputFile,
                            "-");
    arg_desc->AddFlag("b", "Binary ASN.1");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


CRef<CSeqTable_column> sx_MakeColumn(CSeqTable_column_info::EField_id id,
                                     const string& name = kEmptyStr)
{
    CRef<CSeqTable_column> col(new CSeqTable_column);
    col->SetHeader().SetField_id(id);
    if ( !name.empty() ) {
        col->SetHeader().SetField_name(name);
    }
    return col;
}


void sx_SetIndexedValues(CSeqTable_column& col,
                         const CIndexedStrings& values)
{
    size_t size = values.GetSize();
    CCommonString_table::TStrings& arr =
        col.SetData().SetCommon_string().SetStrings();
    arr.resize(size);
    for ( size_t i = 0; i < size; ++i ) {
        arr[i] = values.GetString(i);
    }
}


void sx_SetIndexedValues(CSeqTable_column& col,
                         const CIndexedOctetStrings& values)
{
    size_t size = values.GetSize();
    CCommonBytes_table::TBytes& arr =
        col.SetData().SetCommon_bytes().SetBytes();
    arr.resize(size);
    for ( size_t i = 0; i < size; ++i ) {
        arr[i] = new vector<char>;
        values.GetString(i, *arr[i]);
    }
}


CRef<CSeq_table> sx_ConvertToTable(const CSeq_annot_SNP_Info& annot)
{
    CRef<CSeq_table> table(new CSeq_table);
    table->SetFeat_type(CSeqFeatData::e_Imp);
    table->SetFeat_subtype(CSeqFeatData::eSubtype_variation);
    table->SetNum_rows(int(annot.size()));

    CRef<CSeqTable_column> col_imp =
        sx_MakeColumn(CSeqTable_column_info::eField_id_data_imp_key);
    col_imp->SetDefault().SetString("variation");
    CRef<CSeqTable_column> col_id =
        sx_MakeColumn(CSeqTable_column_info::eField_id_location_id);
    col_id->SetDefault().SetId(const_cast<CSeq_id&>(annot.GetSeq_id()));

    CRef<CSeqTable_column> col_from =
        sx_MakeColumn(CSeqTable_column_info::eField_id_location_from);
    CRef<CSeqTable_column> col_to = 
        sx_MakeColumn(CSeqTable_column_info::eField_id_location_to);
    
    CRef<CSeqTable_column> col_allele1(new CSeqTable_column);
    CRef<CSeqTable_column> col_allele2(new CSeqTable_column);
    CRef<CSeqTable_column> col_allele3(new CSeqTable_column);
    CRef<CSeqTable_column> col_allele4(new CSeqTable_column);

    CRef<CSeqTable_column> col_qa_type =
        sx_MakeColumn(CSeqTable_column_info::eField_id_ext_type);
    col_qa_type->SetDefault().SetString("dbSnpQAdata");

    CRef<CSeqTable_column> col_qa =
        sx_MakeColumn(CSeqTable_column_info::eField_id_ext,
                      "E.QualityCodes");
    sx_SetIndexedValues(*col_qa, annot.x_GetQualityCodesOs());
    CCommonBytes_table::TIndexes& arr_qa =
        col_qa->SetData().SetCommon_bytes().SetIndexes();

    CRef<CSeqTable_column> col_dbxref =
        sx_MakeColumn(CSeqTable_column_info::eField_id_dbxref, "D.dbSNP");
    CSeqTable_multi_data::TInt& arr_dbxref = col_dbxref->SetData().SetInt();

    CSeqTable_multi_data::TInt& arr_from = col_from->SetData().SetInt();
    CSeqTable_sparse_index::TIndexes& ind_to = col_to->SetSparse().SetIndexes();
    CSeqTable_multi_data::TInt& arr_to = col_to->SetData().SetInt();
    
    ITERATE ( CSeq_annot_SNP_Info, it, annot ) {
        TSeqPos from = it->GetFrom();
        arr_from.push_back(from);
        TSeqPos to = it->GetTo();
        if ( to != from ) {
            ind_to.push_back(int(arr_to.size()));
            arr_to.push_back(to);
        }
        arr_qa.push_back(it->GetQualityCodesOsIndex());
        arr_dbxref.push_back(it->m_SNP_Id);
    }

    table->SetColumns().push_back(col_imp);
    table->SetColumns().push_back(col_id);
    table->SetColumns().push_back(col_from);
    if ( !arr_to.empty() ) {
        table->SetColumns().push_back(col_to);
    }
    table->SetColumns().push_back(col_qa_type);
    table->SetColumns().push_back(col_qa);
    table->SetColumns().push_back(col_dbxref);
    
    return table;
}


#ifdef NCBI_THREADS
void RunMTScan(CSNPDb& snp_db)
{
    const size_t NT = 4;
    
    vector<thread> tt(NT);
    for ( size_t i = 0; i < NT; ++i ) {
        tt[i] =
            thread([&]
                   ()
                   {
                       for ( CSNPDbSeqIterator it(snp_db); it; ++it ) {
                       }
                   });
    }
    for ( size_t i = 0; i < NT; ++i ) {
        tt[i].join();
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////
//  Run test
/////////////////////////////////////////////////////////////////////////////


int CSNPTestApp::Run(void)
{
    uint64_t error_count = 0;
    const CArgs& args = GetArgs();

    string path = args["file"].AsString();
    bool verbose = args["verbose"];
    bool make_feat = args["make_feat"];
    bool print = args["print_objects"];
    size_t limit_count = args["limit_count"].AsInteger();

    CNcbiOstream* out = 0;
    if ( print ) {
        out = &args["o"].AsOutputFile();
        if ( args["b"] ) {
            *out << MSerial_AsnBinary;
        }
        else {
            *out << MSerial_AsnText;
        }
    }

    CVDBMgr mgr;
    CStopWatch sw;
    
    sw.Restart();

    CSNPDb snp_db(mgr, path);
    if ( verbose ) {
        cout << "Opened SNP in "<<sw.Restart()
             << NcbiEndl;
    }

    string query_id;
    CRange<TSeqPos> query_range = CRange<TSeqPos>::GetWhole();
    CSeq_id_Handle query_idh;
    
    if ( args["seq"] || args["seq-index"] ) {
        if ( args["seq-index"] ) {
            size_t seq_index = args["seq-index"].AsInteger();
            query_id = CSNPDbSeqIterator(snp_db, seq_index).GetSeqId()->AsFastaString();
            cout << "Sequence["<<seq_index<<"]: "<<query_id<<endl;
        }
        else {
            query_id = args["seq"].AsString();
        }
        query_range.SetFrom(args["pos"].AsInteger());
        if ( args["window"] ) {
            TSeqPos window = args["window"].AsInteger();
            if ( window != 0 ) {
                query_range.SetLength(window);
            }
            else {
                query_range.SetTo(kInvalidSeqPos);
            }
        }
        if ( args["end"] ) {
            query_range.SetTo(args["end"].AsInteger());
        }
    }
#ifdef NCBI_THREADS
    if ( args["test-seq-mt"] ) {
        RunMTScan(snp_db);
    }
#endif
    if ( args["q"] ) {
        string q = args["q"].AsString();
        SIZE_TYPE colon_pos = q.find(':');
        if ( colon_pos == 0 ) {
            ERR_POST(Fatal << "Invalid query format: " << q);
        }
        if ( colon_pos == NPOS ) {
            query_id = q;
            query_range = query_range.GetWhole();
        }
        else {
            query_id = q.substr(0, colon_pos);
            SIZE_TYPE dash_pos = q.find('-', colon_pos+1);
            if ( dash_pos == NPOS ) {
                ERR_POST(Fatal << "Invalid query format: " << q);
            }
            string q_from = q.substr(colon_pos+1, dash_pos-colon_pos-1);
            string q_to = q.substr(dash_pos+1);
            TSeqPos from, to;
            if ( q_from.empty() ) {
                from = 0;
            }
            else {
                from = NStr::StringToNumeric<TSeqPos>(q_from);
            }
            if ( q_to.empty() ) {
                to = kInvalidSeqPos;
            }
            else {
                to = NStr::StringToNumeric<TSeqPos>(q_to);
            }
            query_range.SetFrom(from).SetTo(to);
        }
    }
    if ( !query_id.empty() ) {
        query_idh = CSeq_id_Handle::GetHandle(query_id);
    }

    CSNPDbFeatIterator::SFilter filter;
    CSNPDbTrackIterator track;
    if ( args["track"] ) {
        track = CSNPDbTrackIterator(snp_db, args["track"].AsString());
        filter = track.GetFilter();
    }
    else {
        track = CSNPDbTrackIterator(snp_db);
    }
    
    if ( args["filter"] ) {
        if ( args["filter-mask"] ) {
            filter.SetFilter(args["filter"].AsInt8(),
                             args["filter-mask"].AsInt8());
        }
        else {
            filter.SetFilter(args["filter"].AsInt8());
        }
    }
    
    if ( args["seq_table"] ) {
        sw.Restart();
        for ( CSNPDbSeqIterator it(snp_db); it; ++it ) {
            cout << it.GetSeqIdHandle()
                 << " range: "<<it.GetSNPRange()
                 << " @(" << it.GetPageVDBRowRange() << ")"
                 << NcbiEndl;
        }
        cout << "Scanned reftable in "<<sw.Elapsed()
             << NcbiEndl;
        sw.Restart();
    }
    
    if ( args["track_table"] ) {
        sw.Restart();
        for ( CSNPDbTrackIterator it(snp_db); it; ++it ) {
            cout << it.GetName()
                 << " bits: " << hex<<it.GetFilterBits()<<dec
                 << " mask: " << hex<<it.GetFilterMask()<<dec
                 << NcbiEndl;
        }
        cout << "Scanned reftable in "<<sw.Elapsed()
             << NcbiEndl;
        sw.Restart();
    }
    
    if ( query_idh ) {
        sw.Restart();
        
        vector< CRange<TSeqPos> > chunk_ranges;
        {
            CSNPDbSeqIterator it(snp_db, query_idh);
            TSeqPos end = min(it.GetSNPRange().GetToOpen(),
                              query_range.GetToOpen());
            TSeqPos chunk = args["chunk"].AsInteger();
            for ( TSeqPos p = query_range.GetFrom(); p < end; p += chunk ) {
                CRange<TSeqPos> range;
                range.SetFrom(p);
                range.SetToOpen(min(p+chunk, end));
                chunk_ranges.push_back(range);
            }
        }
        
        size_t count = 0;
        CSNPDbFeatIterator::TFlags flags = CSNPDbFeatIterator::fDefaultFlags;
        if ( args["no_shared_objects"] ) {
            flags &= ~CSNPDbFeatIterator::fUseSharedObjects;
        }
        if ( args["make_overview_graph"] ) {
            CSNPDbSeqIterator it(snp_db, query_idh);
            it.SetTrack(track);
            for ( const auto& query_range : chunk_ranges ) {
                CRef<CSeq_graph> graph =
                    it.GetOverviewGraph(query_range);
                if ( graph && print ) {
                    *out << *graph;
                }
            }
        }
        if ( args["make_overview_annot"] ) {
            CSNPDbSeqIterator it(snp_db, query_idh);
            it.SetTrack(track);
            for ( const auto& query_range : chunk_ranges ) {
                CRef<CSeq_annot> annot = it.GetOverviewAnnot(query_range);
                if ( annot && print ) {
                    *out << *annot;
                }
            }
        }
        if ( args["make_cov_graph"] ) {
            CSNPDbSeqIterator it(snp_db, query_idh);
            it.SetTrack(track);
            for ( const auto& query_range : chunk_ranges ) {
                CRef<CSeq_graph> graph =
                    it.GetCoverageGraph(query_range);
                if ( graph && print ) {
                    *out << *graph;
                }
            }
        }
        if ( args["make_cov_annot"] ) {
            CSNPDbSeqIterator it(snp_db, query_idh);
            it.SetTrack(track);
            for ( const auto& query_range : chunk_ranges ) {
                CRef<CSeq_annot> annot = it.GetCoverageAnnot(query_range);
                if ( annot && print ) {
                    *out << *annot;
                }
            }
        }
        if ( args["make_feat_annot"] ) {
            CSNPDbSeqIterator it(snp_db, query_idh);
            it.SetTrack(track);
            for ( const auto& query_range : chunk_ranges ) {
                CRef<CSeq_annot> annot = it.GetFeatAnnot(query_range);
                if ( annot && print ) {
                    *out << *annot;
                }
            }
        }
        if ( args["make_table_feat_annot"] ) {
            CSNPDbSeqIterator it(snp_db, query_idh);
            it.SetTrack(track);
            for ( const auto& query_range : chunk_ranges ) {
                CSNPDbSeqIterator::TAnnotSet annots =
                    it.GetTableFeatAnnots(query_range, filter);
                if ( print ) {
                    ITERATE ( CSNPDbSeqIterator::TAnnotSet, it, annots ) {
                        *out << **it;
                    }
                }
            }
        }
        if ( args["make_packed_feat_annot"] ) {
            CSNPDbSeqIterator it(snp_db, query_idh);
            it.SetTrack(track);
            for ( const auto& query_range : chunk_ranges ) {
                pair<CRef<CSeq_annot>, CRef<CSeq_annot_SNP_Info> > annot =
                    it.GetPackedFeatAnnot(query_range, filter);
                if ( annot.second ) {
                    CSeq_annot::TData::TFtable& feats =
                        annot.first->SetData().SetFtable();
                    ITERATE ( CSeq_annot_SNP_Info, it, *annot.second ) {
                        feats.push_back(it->CreateSeq_feat(*annot.second));
                    }
                }
                if ( annot.first && print ) {
                    *out << *annot.first;
                }
            }
        }
        for ( CSNPDbFeatIterator it(snp_db, query_idh, query_range, filter); it; ++it ) {
            if ( verbose ) {
                cout << it.GetSeqIdHandle();
                cout << " pos: "<<it.GetSNPPosition();
                cout << " len: "<<it.GetSNPLength();
                cout << '\n';
            }
            if ( make_feat ) {
                CRef<CSeq_feat> feat = it.GetSeq_feat(flags);
                if ( print ) {
                    *out << *feat;
                }
            }
            if ( limit_count && ++count >= limit_count ) {
                break;
            }
        }

        cout << "Found "<<count<<" SNPs in "<<sw.Elapsed()
             << NcbiEndl;
        sw.Restart();
    }

    if ( args["test"] ) {
        CSNPDbSeqIterator seq(snp_db, query_idh);
        seq.SetTrack(track);
        CSNPDbSeqIterator::SSelector sel(seq.eSearchByStart, seq.GetFilter());

        TSeqPos length = seq.GetSeqLength();
        COpenRange<TSeqPos> whole(0, length);
        CStopWatch sw;

        vector<Uint8> c5000_0((length-1)/5000+1);
        vector<Uint8> c100_0((length-1)/100+1);
        sw.Restart();
        {
            TSeqPos cur = 0;
            while ( cur < length ) {
                TSeqPos len = rand()%1000000+100;
                CRange<TSeqPos> range;
                range.SetFrom(cur);
                range.SetLength(len);
                for ( CSNPDbFeatIterator it(seq, range, sel); it; ++it ) {
                    TSeqPos snp_pos = it.GetSNPPosition();
                    TSeqPos snp_len = it.GetSNPLength();
                    _ASSERT(snp_pos+snp_len > range.GetFrom());
                    _ASSERT(snp_pos < range.GetToOpen());
                    _ASSERT(snp_len > 0);
                    TSeqPos snp_pos_100 = snp_pos/100;
                    TSeqPos snp_end_100 = (snp_pos+snp_len-1)/100;
                    if ( snp_pos >= range.GetFrom() ) {
                        if ( snp_len == 2 && snp_pos_100 != snp_end_100 ) {
                            snp_pos_100 = snp_end_100;
                        }
                        for ( TSeqPos i = snp_pos_100; i <= snp_end_100; ++i ) {
                            c100_0.at(i) += 1;
                        }
                    }
                    TSeqPos snp_pos_5000 = snp_pos/5000;
                    TSeqPos snp_end_5000 = (snp_pos+snp_len-1)/5000;
                    if ( snp_pos >= range.GetFrom() ) {
                        if ( snp_len == 2 && snp_pos_5000 != snp_end_5000 ) {
                            snp_pos_5000 = snp_end_5000;
                        }
                        for ( TSeqPos i = snp_pos_5000; i <= snp_end_5000; ++i ) {
                            c5000_0.at(i) += 1;
                        }
                    }
                }
                cur = range.GetToOpen();
            }
        }
        LOG_POST("Collected SNPs in "<<sw.Elapsed());

        vector<Uint8> c100_1((length-1)/100+1);
        sw.Restart();
        {
            const TSeqPos kComp = 100;
            TSeqPos cur = 0;
            while ( cur < length ) {
                TSeqPos len = (rand()%(1000000/kComp)+1)*kComp;
                CRange<TSeqPos> range;
                range.SetFrom(cur);
                range.SetLength(len);
                CRef<CSeq_annot> annot;
                if ( rand()%2 ) {
                    LOG_POST(Info<<"Getting cov graph: "<<range);
                    CRef<CSeq_graph> graph = seq.GetCoverageGraph(range);
                    if ( graph ) {
                        annot = new CSeq_annot();
                        annot->SetData().SetGraph().push_back(graph);
                    }
                }
                else {
                    LOG_POST(Info<<"Getting cov annot: "<<range);
                    annot = seq.GetCoverageAnnot(range);
                }
                if ( !annot ) {
                    annot = new CSeq_annot();
                }
                for ( auto graph : annot->SetData().SetGraph() ) {
                    _ASSERT(graph->GetComp() == kComp);
                    size_t count = graph->GetNumval();
                    TSeqPos pos = graph->GetLoc().GetInt().GetFrom();
                    _ASSERT(pos % kComp == 0);
                    size_t off = pos / kComp;
                    TSeqPos end = graph->GetLoc().GetInt().GetTo()+1;
                    _ASSERT(end > pos && end % kComp == 0);
                    _ASSERT(end == pos + count*kComp);
                    if ( graph->GetGraph().IsByte() ) {
                        LOG_POST(Info<<"Byte cov graph: "<<pos<<"-"<<(end-1));
                        auto& gr = graph->GetGraph().GetByte().GetValues();
                        _ASSERT(gr.size() == count);
                        for ( size_t i = 0; i < count; ++i ) {
                            if ( auto v = Uint1(gr[i]) ) {
                                c100_1.at(off+i) += v;
                            }
                        }
                    }
                    else {
                        LOG_POST(Info<<"Int cov graph: "<<pos<<"-"<<(end-1));
                        auto& gr = graph->GetGraph().GetInt().GetValues();
                        _ASSERT(gr.size() == count);
                        for ( size_t i = 0; i < count; ++i ) {
                            if ( auto v = gr[i] ) {
                                c100_1.at(off+i) += v;
                            }
                        }
                    }
                }
                cur = range.GetToOpen();
            }
        }
        LOG_POST("Collected coverage in "<<sw.Elapsed());

        for ( size_t i = 0; i < c100_0.size(); ++i ) {
            if ( c100_1[i] != c100_0[i] ) {
                ERR_POST("Coverage["<<i<<"]: "<<c100_1[i]<<" vs "<<c100_0[i]);
            }
        }
        _ASSERT(c100_1 == c100_0);
        LOG_POST("Coverage is correct");

        vector<Uint8> c5000_1((length-1)/5000+1);
        sw.Restart();
        {
            const TSeqPos kComp = 5000;
            TSeqPos cur = 0;
            while ( cur < length ) {
                TSeqPos len = (rand()%(10000000/kComp)+1)*kComp;
                CRange<TSeqPos> range;
                range.SetFrom(cur);
                range.SetLength(len);
                CRef<CSeq_annot> annot;
                if ( rand()%2 ) {
                    LOG_POST(Info<<"Getting over graph: "<<range);
                    CRef<CSeq_graph> graph = seq.GetOverviewGraph(range);
                    if ( graph ) {
                        annot = new CSeq_annot();
                        annot->SetData().SetGraph().push_back(graph);
                    }
                }
                else {
                    LOG_POST(Info<<"Getting over annot: "<<range);
                    annot = seq.GetOverviewAnnot(range);
                }
                if ( !annot ) {
                    annot = new CSeq_annot();
                }
                for ( auto graph : annot->SetData().SetGraph() ) {
                    _ASSERT(graph->GetComp() == kComp);
                    size_t count = graph->GetNumval();
                    TSeqPos pos = graph->GetLoc().GetInt().GetFrom();
                    _ASSERT(pos % kComp == 0);
                    size_t off = pos / kComp;
                    TSeqPos end = graph->GetLoc().GetInt().GetTo()+1;
                    _ASSERT(end > pos && end % kComp == 0);
                    _ASSERT(end == pos + count*kComp);
                    if ( graph->GetGraph().IsByte() ) {
                        LOG_POST(Info<<"Byte over graph: "<<pos<<"-"<<(end-1));
                        auto& gr = graph->GetGraph().GetByte().GetValues();
                        _ASSERT(gr.size() == count);
                        for ( size_t i = 0; i < count; ++i ) {
                            if ( auto v = Uint1(gr[i]) ) {
                                c5000_1.at(off+i) += v;
                            }
                        }
                    }
                    else {
                        LOG_POST(Info<<"Int over graph: "<<pos<<"-"<<(end-1));
                        auto& gr = graph->GetGraph().GetInt().GetValues();
                        _ASSERT(gr.size() == count);
                        for ( size_t i = 0; i < count; ++i ) {
                            if ( auto v = gr[i] ) {
                                c5000_1.at(off+i) += v;
                            }
                        }
                    }
                }
                cur = range.GetToOpen();
            }
        }
        LOG_POST("Collected overview in "<<sw.Elapsed());

        for ( size_t i = 0; i < c5000_0.size(); ++i ) {
            if ( c5000_1[i] != c5000_0[i] ) {
                ERR_POST("Overview["<<i<<"]: "<<c5000_1[i]<<" vs "<<c5000_0[i]);
            }
        }
        _ASSERT(c5000_1 == c5000_0);
        LOG_POST("Overview is correct");
    }

    cout << "Success." << NcbiEndl;
    return error_count? 1: 0;
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CSNPTestApp::Exit(void)
{
    SetDiagStream(0);
}


/////////////////////////////////////////////////////////////////////////////
//  MAIN


int main(int argc, const char* argv[])
{
    // Execute main application function
    return CSNPTestApp().AppMain(argc, argv);
}
