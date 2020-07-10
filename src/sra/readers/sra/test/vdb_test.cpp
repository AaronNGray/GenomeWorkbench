/*  $Id: vdb_test.cpp 593797 2019-09-24 17:59:28Z vasilche $
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
 *   Sample test application for cSRA reader
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbi_system.hpp>
#include <sra/readers/sra/csraread.hpp>
#include <sra/readers/ncbi_traces_path.hpp>
#include <insdc/sra.h>

#include <objects/general/general__.hpp>
#include <objects/seq/seq__.hpp>
#include <objects/seqset/seqset__.hpp>
#include <objects/seqalign/seqalign__.hpp>
#include <objects/seqres/seqres__.hpp>
#include <objects/seqloc/Seq_interval.hpp>

#include <serial/serial.hpp>
#include <serial/iterator.hpp>
#include <serial/objostrasnb.hpp>
#include <serial/objistrasnb.hpp>

#include <common/test_assert.h>  /* This header must go last */

USING_NCBI_SCOPE;
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
//  CCSRATestApp::


class CCSRATestApp : public CNcbiApplication
{
public:
    CCSRATestApp();

    virtual void Init(void);
    virtual int  Run(void);
    virtual void Exit(void);
};


/////////////////////////////////////////////////////////////////////////////
//  Init test

//#define DEFAULT_FILE NCBI_TRACES01_PATH "/compress/DATA/ASW/NA19909.mapped.illumina.mosaik.ASW.exome.20110411/s-quantized.csra"
#define DEFAULT_FILE NCBI_TRACES01_PATH "/compress/1KG/ASW/NA19909/exome.ILLUMINA.MOSAIK.csra"

CCSRATestApp::CCSRATestApp()
{
    SetVersion( CVersionInfo(1,23,45) );
}

void CCSRATestApp::Init(void)
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "vdb_test");

    arg_desc->AddOptionalKey("vol_path", "VolPath",
                             "Search path for volumes",
                             CArgDescriptions::eString);
    arg_desc->AddOptionalKey("file", "File",
                             "cSRA file name",
                             CArgDescriptions::eString);
    arg_desc->AddOptionalKey("acc", "SRZAccession",
                             "SRZ accession to the cSRA file",
                             CArgDescriptions::eString);
    arg_desc->AddFlag("no_acc", "SRZ accession is supposed to be absent");

    arg_desc->AddOptionalKey("mapfile", "MapFile",
                             "IdMapper config filename",
                             CArgDescriptions::eInputFile);
    arg_desc->AddDefaultKey("genome", "Genome",
                            "UCSC build number",
                            CArgDescriptions::eString, "");

    arg_desc->AddFlag("refseq_table", "Dump RefSeq table");
    arg_desc->AddFlag("spot_groups", "List spot groups");
    arg_desc->AddFlag("make_graphs", "Generate coverage graphs");

    arg_desc->AddOptionalKey("q", "Query",
                             "Query coordinates in form chr1:100-10000",
                             CArgDescriptions::eString);
    arg_desc->AddOptionalKey("refseq", "RefSeqId",
                             "RefSeq id",
                             CArgDescriptions::eString);
    arg_desc->AddDefaultKey("refpos", "RefSeqPos",
                            "RefSeq position",
                            CArgDescriptions::eInteger,
                            "0");
    arg_desc->AddOptionalKey("refposs", "RefSeqPoss",
                            "RefSeq positions",
                            CArgDescriptions::eString);
    arg_desc->AddDefaultKey("refwindow", "RefSeqWindow",
                            "RefSeq window",
                            CArgDescriptions::eInteger,
                            "0");
    arg_desc->AddOptionalKey("refend", "RefSeqEnd",
                            "RefSeq end position",
                            CArgDescriptions::eInteger);
    arg_desc->AddFlag("by-start", "Search range by start position");
    arg_desc->AddFlag("primary", "Only primary alignments");
    arg_desc->AddFlag("secondary", "Only secondary alignments");
    arg_desc->AddFlag("count-unique", "Count number of unique reads");
    
    arg_desc->AddFlag("over-start", "Dump overlap start positions");
    arg_desc->AddFlag("over-start-verify", "Verify overlap start positions");
    arg_desc->AddDefaultKey("limit_count", "LimitCount",
                            "Number of cSRA entries to read (0 - unlimited)",
                            CArgDescriptions::eInteger,
                            "100000");
    arg_desc->AddOptionalKey("check_id", "CheckId",
                             "Compare alignments with the specified sequence",
                             CArgDescriptions::eString);
    arg_desc->AddDefaultKey("min_quality", "MinQuality",
                            "Minimal quality of alignments to check",
                            CArgDescriptions::eInteger,
                            "1");
    arg_desc->AddFlag("verbose_weak_matches", "Print light mismatches");
    arg_desc->AddFlag("no_spot_id_detector",
                      "Do not use SpotId detector to resolve short id conflicts");
    arg_desc->AddFlag("verbose", "Print info about found alignments");
    arg_desc->AddFlag("ref_seq", "Make reference Bioseq");
    arg_desc->AddFlag("short_seq", "Make Bioseq with short read sequence");
    arg_desc->AddFlag("seq_annot", "Make Seq-annot with match alignment");
    arg_desc->AddFlag("seq_entry",
                      "Make Seq-entry with both read and alignment");
    arg_desc->AddFlag("quality_graph", "Make read quality Seq-graph");
    arg_desc->AddFlag("stat_graph", "Make base statistics Seq-graph");
    arg_desc->AddFlag("seq_iter", "Make Bioseq from short read iterator");
    arg_desc->AddFlag("print_objects", "Print generated objects");
    arg_desc->AddFlag("ignore_errors", "Ignore errors in individual entries");

    arg_desc->AddFlag("scan_reads", "Scan reads");
    arg_desc->AddOptionalKey("start_id", "StartId",
                             "Start short read id",
                             CArgDescriptions::eInteger);
    arg_desc->AddFlag("fasta", "Print reads in fasta format");
    arg_desc->AddFlag("count_ids", "Count alignments ids");

    arg_desc->AddFlag("get-cache", "Get cache root");
    arg_desc->AddOptionalKey("set-cache", "SetCache",
                             "Set cache root", CArgDescriptions::eString);

    arg_desc->AddDefaultKey("o", "OutputFile",
                            "Output file of ASN.1",
                            CArgDescriptions::eOutputFile,
                            "-");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}



/////////////////////////////////////////////////////////////////////////////
//  Run test
/////////////////////////////////////////////////////////////////////////////

inline bool Matches(char ac, char rc)
{
    static int bits[26] = {
        0x1, 0xE, 0x2, 0xD,
        0x0, 0x0, 0x4, 0xB,
        0x0, 0x0, 0xC, 0x0,
        0x3, 0xF, 0x0, 0x0,
        0x0, 0x5, 0x6, 0x8,
        0x0, 0x7, 0x9, 0x0,
        0xA, 0x0
    };
    
    //if ( ac == rc || ac == 'N' || rc == 'N' ) return true;
    if ( ac < 'A' || ac > 'Z' || rc < 'A' || rc > 'Z' ) return false;
    if ( bits[ac-'A'] & bits[rc-'A'] ) return true;
    return false;
}
/*
string GetShortSeqData(const CBamAlignIterator& it)
{
    string ret;
    const CBamString& src = it.GetShortSequence();
    //bool minus = it.IsSetStrand() && IsReverse(it.GetStrand());
    TSeqPos src_pos = it.GetCIGARPos();

    const CBamString& CIGAR = it.GetCIGAR();
    const char* ptr = CIGAR.data();
    const char* end = ptr + CIGAR.size();
    char type;
    TSeqPos len;
    while ( ptr != end ) {
        type = *ptr;
        for ( len = 0; ++ptr != end; ) {
            char c = *ptr;
            if ( c >= '0' && c <= '9' ) {
                len = len*10+(c-'0');
            }
            else {
                break;
            }
        }
        if ( type == 'M' || type == '=' || type == 'X' ) {
            // match
            for ( TSeqPos i = 0; i < len; ++i ) {
                ret += src[src_pos++];
            }
        }
        else if ( type == 'I' || type == 'S' ) {
            // insert
            src_pos += len;
        }
        else if ( type == 'D' || type == 'N' ) {
            ret.append(len, 'N');
        }
    }
    return ret;
}
*/

char Complement(char c)
{
    switch ( c ) {
    case 'N': return 'N';
    case 'A': return 'T';
    case 'T': return 'A';
    case 'C': return 'G';
    case 'G': return 'C';
    case 'B': return 'V';
    case 'V': return 'B';
    case 'D': return 'H';
    case 'H': return 'D';
    case 'K': return 'M';
    case 'M': return 'K';
    case 'R': return 'Y';
    case 'Y': return 'R';
    case 'S': return 'S';
    case 'W': return 'W';
    default: Abort();
    }
}


string Reverse(const string& s)
{
    size_t size = s.size();
    string r(size, ' ');
    for ( size_t i = 0; i < size; ++i ) {
        r[i] = Complement(s[size-1-i]);
    }
    return r;
}

#include <klib/rc.h>
#include <klib/writer.h>
#include <vdb/manager.h>
#include <vdb/vdb-priv.h>
#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <vfs/manager.h>
#include <vfs/path.h>
#ifdef _MSC_VER
# include <io.h>
#else
# include <unistd.h>
#endif

// low level SRA SDK test
void CheckRc(rc_t rc, const char* code, const char* file, int line)
{
    if ( rc ) {
        char buffer1[4096];
        size_t error_len;
        RCExplain(rc, buffer1, sizeof(buffer1), &error_len);
        char buffer2[8192];
        unsigned len = sprintf(buffer2, "%s:%d: %s failed: %#x: %s\n",
                             file, line, code, rc, buffer1);
        int exit_code = 1;
        if ( write(2, buffer2, len) != len ) {
            ++exit_code;
        }
        exit(exit_code);
    }
}

#define RC_CALL(call) CheckRc((call), #call, __FILE__, __LINE__)

#if 0
#include <klib/rc.h>
#include <klib/writer.h>
#include <align/align-access.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#ifdef _MSC_VER
# include <io.h>
CRITICAL_SECTION sdk_mutex;
# define SDKLock() EnterCriticalSection(&sdk_mutex)
# define SDKUnlock() LeaveCriticalSection(&sdk_mutex)
#else
# include <unistd.h>
# define SDKLock() do{}while(0)
# define SDKUnlock() do{}while(0)
#endif

#define LOW_LEVEL_TEST
int LowLevelTest(void)
{
    cout << "LowLevelTest SRR1551783.SEQUENCE.TRIM_LEN[38013]..." << endl;
    const char* file_name = "SRR1551783";
    const VDBManager* mgr = 0;
    RC_CALL(VDBManagerMakeRead(&mgr, 0));
        
    const VDatabase* db = 0;
    RC_CALL(VDBManagerOpenDBRead(mgr, &db, 0, file_name));
        
    const VTable* seq_table = 0;
    RC_CALL(VDatabaseOpenTableRead(db, &seq_table, "SEQUENCE"));
    
    const VCursor* seq_cursor = 0;
    RC_CALL(VTableCreateCursorRead(seq_table, &seq_cursor));
    RC_CALL(VCursorPermitPostOpenAdd(seq_cursor));
    RC_CALL(VCursorOpen(seq_cursor));
    
    uint32_t SPOT_GROUP;
    RC_CALL(VCursorAddColumn(seq_cursor, &SPOT_GROUP, "SPOT_GROUP"));
    uint32_t READ_TYPE;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ_TYPE, "READ_TYPE"));
    uint32_t READ_LEN;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ_LEN, "READ_LEN"));
    uint32_t READ_START;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ_START, "READ_START"));
    uint32_t READ;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ, "READ"));
    uint32_t QUALITY;
    RC_CALL(VCursorAddColumn(seq_cursor, &QUALITY, "QUALITY"));
    uint32_t TRIM_LEN;
    RC_CALL(VCursorAddColumn(seq_cursor, &TRIM_LEN, "TRIM_LEN"));
    uint32_t TRIM_START;
    RC_CALL(VCursorAddColumn(seq_cursor, &TRIM_START, "TRIM_START"));
    uint32_t NAME;
    RC_CALL(VCursorAddColumn(seq_cursor, &NAME, "NAME"));

    int64_t seq_row = 38012;
    {
        const void* data;
        uint32_t bit_offset, bit_length;
        uint32_t elem_count;
        for ( int i = 0; i < 2; ++i ) {
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                          READ_TYPE,
                                          &bit_length, &data, &bit_offset,
                                          &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                          READ_TYPE,
                                          &bit_length, &data, &bit_offset,
                                          &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                          READ_LEN,
                                          &bit_length, &data, &bit_offset,
                                          &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                          READ_START,
                                          &bit_length, &data, &bit_offset,
                                          &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                          TRIM_START,
                                          &bit_length, &data, &bit_offset,
                                          &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                          TRIM_LEN,
                                          &bit_length, &data, &bit_offset,
                                          &elem_count));
        }

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      NAME,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      NAME,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      TRIM_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      TRIM_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ_TYPE,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      TRIM_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      TRIM_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      NAME,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      NAME,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      TRIM_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      TRIM_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                                      READ,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row+1,
                                      READ_TYPE,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row+1,
                                      READ_TYPE,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row+1,
                                      READ_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row+1,
                                      READ_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row+1,
                                      READ_START,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row+1,
                                      TRIM_LEN,
                                      &bit_length, &data, &bit_offset,
                                      &elem_count));
    }
    RC_CALL(VCursorRelease(seq_cursor));
    RC_CALL(VTableRelease(seq_table));
    RC_CALL(VDatabaseRelease(db));
    RC_CALL(VDBManagerRelease(mgr));
    cout << "LowLevelTest done" << endl;
    return 0;
}
#endif

struct SBaseStat
{
    enum {
        kStat_A = 0,
        kStat_C = 1,
        kStat_G = 2,
        kStat_T = 3,
        kStat_Insert = 4,
        kStat_Match = 5,
        kNumStat = 6
    };
    SBaseStat(void) {
        for ( int i = 0; i < kNumStat; ++i ) {
            cnts[i] = 0;
        }
    }

    unsigned total() const {
        unsigned ret = 0;
        for ( int i = 0; i < kNumStat; ++i ) {
            ret += cnts[i];
        }
        return ret;
    }

    void add_base(char b) {
        switch ( b ) {
        case 'A': cnts[kStat_A] += 1; break;
        case 'C': cnts[kStat_C] += 1; break;
        case 'G': cnts[kStat_G] += 1; break;
        case 'T': cnts[kStat_T] += 1; break;
        case '=': cnts[kStat_Match] += 1; break;
        }
    }
    void add_match() {
        cnts[kStat_Match] += 1;
    }
    void add_gap() {
        cnts[kStat_Insert] += 1;
    }

    unsigned cnts[kNumStat];
};

int CCSRATestApp::Run(void)
{
    const CArgs& args = GetArgs();

    bool verbose = args["verbose"];
    bool fasta = args["fasta"];

    CVDBMgr mgr;
    if ( args["get-cache"] ) {
        cout << "VDB cache: " << mgr.GetCacheRoot() << endl;
    }
    if ( args["set-cache"] ) {
        string path = args["set-cache"].AsString();
        cout << "setting VDB cache to: " << path << endl;
        mgr.SetCacheRoot(path);
        mgr.CommitConfig();
    }

    string acc_or_path = DEFAULT_FILE;
    if ( args["file"] ) {
        acc_or_path = args["file"].AsString();
    }
    else if ( args["acc"] ) {
        acc_or_path = args["acc"].AsString();
        if ( verbose ) {
            try {
                string path = mgr.FindAccPath(acc_or_path);
                cout << "Resolved "<<acc_or_path<<" -> "<<path<<endl;
            }
            catch ( CSraException& exc ) {
                ERR_POST("FindAccPath failed: "<<exc);
            }
        }
    }

#ifdef LOW_LEVEL_TEST
    return LowLevelTest();
#endif

    string query_id;
    CRange<TSeqPos> query_range = CRange<TSeqPos>::GetWhole();
    CSeq_id_Handle query_idh;
    
    if ( args["refseq"] ) {
        query_id = args["refseq"].AsString();
        query_range.SetFrom(args["refpos"].AsInteger());
        if ( args["refwindow"] ) {
            TSeqPos window = args["refwindow"].AsInteger();
            if ( window != 0 ) {
                query_range.SetLength(window);
            }
            else {
                query_range.SetTo(kInvalidSeqPos);
            }
        }
        if ( args["refend"] ) {
            query_range.SetTo(args["refend"].AsInteger());
        }
    }
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
    CCSraAlignIterator::ESearchMode mode = CCSraAlignIterator::eSearchByOverlap;
    if ( args["by-start"] ) {
        mode = CCSraAlignIterator::eSearchByStart;
    }
    int min_quality = args["min_quality"].AsInteger();
    bool make_ref_seq = args["ref_seq"];
    bool make_short_seq = args["short_seq"];
    bool make_seq_annot = args["seq_annot"];
    bool make_seq_entry = args["seq_entry"];
    bool make_seq_iter = args["seq_iter"];
    bool make_quality_graph = args["quality_graph"];
    bool make_stat_graph = args["stat_graph"];
    bool print_objects = args["print_objects"];
    bool count_unique = args["count-unique"];

    CNcbiOstream& out = cout;

    map<string, Uint8> unique_counts;

    const bool kUseVDB = false;
    if ( kUseVDB ) {
        CVDB db(mgr, acc_or_path);
        if ( args["refseq_table"] ) {
            CVDBTable ref_tbl(db, "REFERENCE");
            CVDBCursor ref_curs(ref_tbl);
            CVDBColumn name_col(ref_curs, "NAME");
            CVDBColumn name_range_col(ref_curs, "NAME_RANGE");
            CVDBColumn seq_id_col(ref_curs, "SEQ_ID");
            CVDBColumn aln_ids_col(ref_curs, "PRIMARY_ALIGNMENT_IDS");
            CVDBColumn seq_len_col(ref_curs, "SEQ_LEN");
            string ref_name, ref_seq_id;
            uint64_t slot_size = 0, slot_count = 0, aln_count = 0;
            for ( uint64_t row = 1, max_row = ref_curs.GetMaxRowId();
                  true; ++row ) {
                bool more = row <= max_row && ref_curs.TryOpenRow(row);
                if ( more &&
                     *CVDBStringValue(ref_curs, name_col) == ref_name ) {
                    CVDBValueFor<uint64_t> aln_ids(ref_curs, aln_ids_col);
                    slot_count += 1;
                    aln_count += aln_ids.size();
                    continue;
                }
                else if ( slot_count ) {
                    out << ref_name << " " << ref_seq_id
                        << " " << slot_count*slot_size
                        << " " << aln_count
                        << NcbiEndl;
                }

                if ( !more ) {
                    break;
                }
                CVDBStringValue name(ref_curs, name_col);
                CVDBStringValue seq_id(ref_curs, seq_id_col);
                CVDBValueFor<int32_t> seq_len(ref_curs, seq_len_col);
                ref_name = *name;
                ref_seq_id = *seq_id;
                slot_size = *seq_len;
                
                CVDBValueFor<uint64_t> aln_ids(ref_curs, aln_ids_col);
                slot_count = 1;
                aln_count = aln_ids.size();
            }
        }
        else {
        }
    }
    else {
        CStopWatch sw;
        
        sw.Restart();
        CCSraDb csra_db;
        if ( args["no_acc"] ) {
            try {
                csra_db = CCSraDb(mgr, acc_or_path);
                ERR_POST(Fatal<<
                         "CCSraDb succeeded for an absent SRA acc: "<<
                         acc_or_path);
            }
            catch ( CSraException& exc ) {
                if ( exc.GetErrCode() != CSraException::eNotFoundDb ) {
                    ERR_POST(Fatal<<
                             "CSraException is wrong for an absent SRA acc: "<<
                             acc_or_path<<": "<<exc);
                }
                out << "Correctly detected absent SRA acc: "<<acc_or_path
                    << NcbiEndl;
                return 0;
            }
        }
        else {
            csra_db = CCSraDb(mgr, acc_or_path);
        }
        out << "Opened CSRA in "<<sw.Elapsed()
            << NcbiEndl;
        
        if ( args["spot_groups"] ) {
            CCSraDb::TSpotGroups spot_groups;
            csra_db.GetSpotGroups(spot_groups);
            ITERATE ( CCSraDb::TSpotGroups, it, spot_groups ) {
                out << *it << "\n";
            }
            out << "Total "<<spot_groups.size()<<" spot groups."<<endl;
        }

        if ( args["refseq_table"] ) {
            sw.Restart();
            for ( CCSraRefSeqIterator it(csra_db); it; ++it ) {
                out << it->m_Name << " " << it->m_SeqId
                    << " len: "<<it.GetSeqLength()
                    << " @(" << it->m_RowFirst << "," << it->m_RowLast << ")"
                    << NcbiEndl;
            }
            out << "Scanned reftable in "<<sw.Elapsed()
                << NcbiEndl;
            sw.Restart();
        }
        
        if ( query_idh && (args["over-start"] || args["over-start-verify"]) ) {
            sw.Restart();
            bool verify = args["over-start-verify"];
            CCSraRefSeqIterator ref_it(csra_db, query_idh);
            TSeqPos step = csra_db.GetRowSize();
            TSeqPos ref_len = ref_it.GetSeqLength();
            vector<TSeqPos> vv = ref_it.GetAlnOverStarts();
            out << "Got overlap array in "<<sw.Elapsed() << endl;
            vector<TSeqPos> vv_exp;
            if ( verify ) {
                for ( TSeqPos i = 0; i*step < ref_len; ++i ) {
                    vv_exp.push_back(i*step);
                }
                for ( CCSraAlignIterator it(csra_db, query_idh, 0, ref_len, CCSraAlignIterator::eSearchByStart); it; ++it ) {
                    TSeqPos aln_beg = it.GetRefSeqPos();
                    TSeqPos aln_len = it.GetRefSeqLen();
                    _ASSERT(aln_beg+aln_len <= ref_len);
                    TSeqPos slot_beg = aln_beg/step;
                    TSeqPos slot_end = (aln_beg+aln_len-1)/step;
                    for ( TSeqPos i = slot_beg; i <= slot_end; ++i ) {
                        vv_exp[i] = min(vv_exp[i], aln_beg/*-aln_beg%step*/);
                    }
                }
            }
            for ( size_t i = 0; i < vv.size(); ++i ) {
                if ( verify ) {
                    if ( vv[i]/step != vv_exp[i]/step ) {
                        out << "Overlap pos["<<i<<" / "<<(i*step)<<"]"
                            << " = "<<vv[i] <<" "<<int(vv[i]-i*step)
                            << " exp "<<vv_exp[i] <<" "<<int(vv_exp[i]-i*step)
                            << endl;
                    }
                }
                else {
                    if ( vv[i]/step != i ) {
                        out << "Overlap pos["<<i<<" / "<<(i*step)<<"]"
                            << " = "<<vv[i] <<" "<<int(vv[i]-i*step)
                            << endl;
                    }
                }
            }
            sw.Restart();
        }

        CCSraRefSeqIterator::TAlignType align_type;
        if ( args["primary"] ) {
            align_type = CCSraRefSeqIterator::fPrimaryAlign;
        }
        else if ( args["secondary"] ) {
            align_type = CCSraRefSeqIterator::fSecondaryAlign;
        }
        else {
            align_type = CCSraRefSeqIterator::fAnyAlign;
        }
        if ( query_idh && args["count_ids"] ) {
            CCSraRefSeqIterator ref_it(csra_db, query_idh);
            sw.Restart();
            TSeqPos step = csra_db.GetRowSize();
            uint64_t count = 0;
            TSeqPos end = min(query_range.GetToOpen(), ref_it.GetSeqLength());
            for ( TSeqPos p = query_range.GetFrom(); p < end; p += step ) {
                count += ref_it.GetAlignCountAtPos(p, align_type);
            }
            out << "Alignment count: " << count << " in " << sw.Elapsed()
                << NcbiEndl;
            sw.Restart();
        }

        if ( args["make_graphs"] ) {
            sw.Restart();
            for ( CCSraRefSeqIterator it(csra_db); it; ++it ) {
                CRef<CSeq_graph> graph = it.GetCoverageGraph();
                Uint8 total_count_b = 0, max_b = 0;
                ITERATE ( CByte_graph::TValues, it2, graph->GetGraph().GetByte().GetValues() ) {
                    Uint1 b = *it2;
                    total_count_b += b;
                    if ( b > max_b ) {
                        max_b = b;
                    }
                }
                if ( 0 ) {
                    out << MSerial_AsnText << *graph;
                }
                out << it.GetRefSeqId()
                    << ": total_b = "<<total_count_b
                    << " max_b = "<<max_b
                    << NcbiEndl;
                Uint8 total_count_a = 0, max_a = 0;
                double k_min = 0, k_max = 1e9;
                for ( TVDBRowId row = it.GetInfo().m_RowFirst; row <= it.GetInfo().m_RowLast; ++row ) {
                    Uint1 b = graph->GetGraph().GetByte().GetValues()[size_t(row-it.GetInfo().m_RowFirst)];
                    size_t a = it.GetAlignCountAtPos(row*csra_db.GetRowSize());
                    total_count_a += a;
                    if ( a > max_a ) {
                        max_a = a;
                    }
                    if ( a ) {
                        k_min = max(k_min, (b-.5)/a);
                        k_max = min(k_max, (b+.5)/a);
                        if ( 0 ) {
                            out << row<<": " << a << " " << b+0
                                << " " << (b-.5)/a << " - " << (b+.5)/a
                                << "\n";
                        }
                    }
                }
                out << it.GetRefSeqId()
                    << ": total_a = "<<total_count_a
                    << " max_a = "<<max_a
                    << NcbiEndl;
                out << "k: " << k_min << " - " << k_max
                    << NcbiEndl;
            }
            out << "Scanned graph in "<<sw.Elapsed()
                << NcbiEndl;
        }

        if ( query_idh ) {
            sw.Restart();
            if ( make_seq_entry || make_seq_annot || make_quality_graph ||
                 make_ref_seq || make_short_seq || !make_stat_graph ) {
                if ( make_ref_seq ) {
                    CRef<CBioseq> obj =
                        CCSraRefSeqIterator(csra_db, query_idh).GetRefBioseq();
                    if ( print_objects )
                        out << MSerial_AsnText << *obj;
                }
                size_t count = 0, skipped = 0;
                size_t limit_count = args["limit_count"].AsInteger();
                for ( CCSraAlignIterator it(csra_db, query_idh,
                                            query_range.GetFrom(),
                                            query_range.GetLength(),
                                            mode, align_type);
                      it; ++it ) {
                    if ( it.GetMapQuality() < min_quality ) {
                        ++skipped;
                        continue;
                    }
                    if ( 0 ) {
                        CRef<CBioseq> seq = it.GetShortBioseq();
                        seq->SetAnnot().push_back(it.GetMatchAnnot());
                        CRef<CBioseq> seq2(SerialClone(*seq));
                        for ( CTypeIterator<CSeq_id> it(Begin(*seq2)); it; ++it ) {
                            CSeq_id& id = *it;
                            if ( id.IsLocal() ) {
                                id.Set("gnl|SRA|SRR389414.136780466.1");
                                break;
                            }
                        }
                        NcbiCout << "With local:"
                                 << MSerial_AsnText << *seq << endl;
                        char buf[1024];
                        for ( int t = 0; t < 10; ++t ) {
                            size_t mem0, rss0, shr0;
                            GetMemoryUsage(&mem0, &rss0, &shr0);
                            vector< CRef<CBioseq> > bb;
                            vector<CSeq_id_Handle> hh;
                            for ( int i = 0; i < 100000; ++i ) {
                                bb.push_back(Ref(SerialClone(*seq)));
                                sprintf(buf, "lcl|%d", 136780466+i);
                                hh.push_back(CSeq_id_Handle::GetHandle(buf));
                            }
                            size_t mem1, rss1, shr1;
                            GetMemoryUsage(&mem1, &rss1, &shr1);
                            NcbiCout << "mem: " << mem0 << " -> " << mem1
                                     << " = " << mem1-mem0 << endl;
                            NcbiCout << "rss: " << rss0 << " -> " << rss1
                                     << " = " << rss1-rss0 << endl;
                            NcbiCout << "shr: " << shr0 << " -> " << shr1
                                     << " = " << shr1-shr0 << endl;
                        }
                        NcbiCout << "With general:"
                                 << MSerial_AsnText << *seq2 << endl;
                        for ( int t = 0; t < 10; ++t ) {
                            size_t mem0, rss0, shr0;
                            GetMemoryUsage(&mem0, &rss0, &shr0);
                            vector< CRef<CBioseq> > bb;
                            vector<CSeq_id_Handle> hh;
                            for ( int i = 0; i < 100000; ++i ) {
                                bb.push_back(Ref(SerialClone(*seq2)));
                                sprintf(buf, "gnl|SRA|SRR389414.%d.1", 136780466+i);
                                hh.push_back(CSeq_id_Handle::GetHandle(buf));
                            }
                            size_t mem1, rss1, shr1;
                            GetMemoryUsage(&mem1, &rss1, &shr1);
                            NcbiCout << "mem: " << mem0 << " -> " << mem1
                                     << " = " << mem1-mem0 << endl;
                            NcbiCout << "rss: " << rss0 << " -> " << rss1
                                     << " = " << rss1-rss0 << endl;
                            NcbiCout << "shr: " << shr0 << " -> " << shr1
                                     << " = " << shr1-shr0 << endl;
                        }
                        break;
                    }
                    ++count;
                    if ( verbose ) {
                        out << it.GetAlignmentId() << ": "
                            << it.GetShortId1()<<"."<<it.GetShortId2() << ": "
                            << it.GetRefSeqId() << "@" << it.GetRefSeqPos()
                            << "+" << it.GetRefSeqLen()
                            << " vs " << it.GetShortSeq_id()->AsFastaString();
                        if ( args["spot_groups"] ) {
                            out << "  GROUP: "<<it.GetSpotGroup();
                        }
                        out << "\n    CIGAR: "<<it.GetCIGARLong();
                        out << "\n mismatch: "<<it.GetMismatchRaw();
                        if ( it.GetReadFilter() != SRA_READ_FILTER_PASS ) {
                            static const char* const filter_name[] = {
                                "good",
                                "Poor sequence quality",
                                "PCR duplicate",
                                "Hidden"
                            };
                            out << " " << filter_name[it.GetReadFilter()];
                        }
                        out << NcbiEndl;
                    }
                    if ( count_unique ) {
                        string key = NStr::NumericToString(it.GetRefSeqPos())+'/'+it.GetCIGARLong()+'/'+it.GetMismatchRaw();
                        unique_counts[key] += 1;
                    }
                    if ( make_seq_annot ) {
                        CRef<CSeq_annot> obj = it.GetMatchAnnot();
                        if ( print_objects )
                            out << MSerial_AsnText << *obj;
                    }
                    if ( make_short_seq ) {
                        CRef<CBioseq> obj = it.GetShortBioseq();
                        if ( print_objects )
                            out << MSerial_AsnText << *obj;
                    }
                    if ( make_seq_entry ) {
                        CRef<CSeq_entry> obj = it.GetMatchEntry();
                        if ( print_objects )
                            out << MSerial_AsnText << *obj;
                    }
                    if ( make_quality_graph ) {
                        CRef<CSeq_annot> obj = it.GetQualityGraphAnnot("q");
                        if ( print_objects )
                            out << MSerial_AsnText << *obj;
                    }
                    if ( make_seq_iter ) {
                        CCSraShortReadIterator it2(csra_db,
                                                   it.GetShortId1(),
                                                   it.GetShortId2());
                        CRef<CBioseq> obj = it2.GetShortBioseq();
                        if ( print_objects )
                            out << MSerial_AsnText << *obj;
                    }
                    if ( limit_count && count >= limit_count ) {
                        break;
                    }
                }
                out << "Found "<<count<<" alignments." << NcbiEndl;
                if ( skipped )
                    out << "Skipped "<<skipped<<" alignments." << NcbiEndl;
            }
            if ( make_stat_graph ) {
                size_t count = 0, skipped = 0;
                vector<SBaseStat> ss(query_range.GetLength());
                CRef<CSeq_id> ref_id =
                    CCSraRefSeqIterator(csra_db, query_idh).GetRefSeq_id();
                for ( CCSraAlignIterator it(csra_db, query_idh,
                                            query_range.GetFrom(),
                                            query_range.GetLength(),
                                            mode, align_type);
                      it; ++it ) {
                    if ( it.GetMapQuality() < min_quality ) {
                        ++skipped;
                        continue;
                    }
                    ++count;
                    TSeqPos ref_pos = it.GetRefSeqPos()-query_range.GetFrom();
                    TSeqPos read_pos = it.GetShortPos();
                    CTempString read = it.GetMismatchRead();
                    CTempString cigar = it.GetCIGARLong();
                    const char* ptr = cigar.data();
                    const char* end = cigar.end();
                    while ( ptr != end ) {
                        char type = 0;
                        int seglen = 0;
                        for ( ; ptr != end; ) {
                            char c = *ptr++;
                            if ( c >= '0' && c <= '9' ) {
                                seglen = seglen*10+(c-'0');
                            }
                            else {
                                type = c;
                                break;
                            }
                        }
                        if ( seglen == 0 ) {
                            NCBI_THROW_FMT(CSraException, eDataError,
                                           "Bad CIGAR length: " << type <<
                                           "0 in " << cigar);
                        }
                        if ( type == 'M' || type == 'X' ) {
                            // match
                            for ( int i = 0; i < seglen; ++i ) {
                                if ( ref_pos < ss.size() ) {
                                    ss[ref_pos].add_base(read[read_pos]);
                                }
                                ++ref_pos;
                                ++read_pos;
                            }
                        }
                        else if ( type == '=' ) {
                            // match
                            for ( int i = 0; i < seglen; ++i ) {
                                if ( ref_pos < ss.size() ) {
                                    ss[ref_pos].add_match();
                                }
                                ++ref_pos;
                                ++read_pos;
                            }
                        }
                        else if ( type == 'I' || type == 'S' ) {
                            if ( type == 'S' ) {
                                // soft clipping already accounted in seqpos
                                continue;
                            }
                            read_pos += seglen;
                        }
                        else if ( type == 'N' ) {
                            // intron
                            ref_pos += seglen;
                        }
                        else if ( type == 'D' ) {
                            // delete
                            for ( int i = 0; i < seglen; ++i ) {
                                if ( ref_pos < ss.size() ) {
                                    ss[ref_pos].add_gap();
                                }
                                ++ref_pos;
                            }
                        }
                        else if ( type != 'P' ) {
                            NCBI_THROW_FMT(CSraException, eDataError,
                                           "Bad CIGAR char: " <<type<< " in " <<cigar);
                        }
                    }
                }
                out << "Found "<<count<<" alignments." << NcbiEndl;
                if ( skipped )
                    out << "Skipped "<<skipped<<" alignments." << NcbiEndl;

                CRef<CSeq_annot> obj(new CSeq_annot);
                obj->SetData().SetGraph();
                
                int c_min[SBaseStat::kNumStat], c_max[SBaseStat::kNumStat];
                for ( int i = 0; i < SBaseStat::kNumStat; ++i ) {
                    c_min[i] = kMax_Int;
                    c_max[i] = 0;
                }
                for ( size_t j = 0; j < ss.size(); ++j ) {
                    const SBaseStat& s = ss[j];
                    if ( verbose && s.total() ) {
                        out << j+query_range.GetFrom()<<": ";
                        for ( int i = 0; i < SBaseStat::kNumStat; ++i ) {
                            int c = s.cnts[i];
                            for ( int k = 0; k < c; ++k )
                                out << "ACGT-="[i];
                        }
                        out << '\n';
                    }
                    for ( int i = 0; i < SBaseStat::kNumStat; ++i ) {
                        int c = s.cnts[i];
                        c_min[i] = min(c_min[i], c);
                        c_max[i] = max(c_max[i], c);
                    }
                }
                for ( int i = 0; i < SBaseStat::kNumStat; ++i ) {
                    CRef<CSeq_graph> graph(new CSeq_graph);
                    static const char* const titles[6] = {
                        "Number of A bases",
                        "Number of C bases",
                        "Number of G bases",
                        "Number of T bases",
                        "Number of inserts",
                        "Number of matches"
                    };
                    graph->SetTitle(titles[i]);
                    CSeq_interval& loc = graph->SetLoc().SetInt();
                    loc.SetId(*ref_id);
                    loc.SetFrom(query_range.GetFrom());
                    loc.SetTo(query_range.GetTo());
                    graph->SetNumval(TSeqPos(ss.size()));

                    if ( c_max[i] < 256 ) {
                        CByte_graph& data = graph->SetGraph().SetByte();
                        CByte_graph::TValues& vv = data.SetValues();
                        vv.reserve(ss.size());
                        for ( size_t j = 0; j < ss.size(); ++j ) {
                            vv.push_back(ss[j].cnts[i]);
                        }
                        data.SetMin(c_min[i]);
                        data.SetMax(c_max[i]);
                        data.SetAxis(0);
                    }
                    else {
                        CInt_graph& data = graph->SetGraph().SetInt();
                        CInt_graph::TValues& vv = data.SetValues();
                        vv.reserve(ss.size());
                        for ( size_t j = 0; j < ss.size(); ++j ) {
                            vv.push_back(ss[j].cnts[i]);
                        }
                        data.SetMin(c_min[i]);
                        data.SetMax(c_max[i]);
                        data.SetAxis(0);
                    }
                    obj->SetData().SetGraph().push_back(graph);
                }

                if ( print_objects )
                    out << MSerial_AsnText << *obj;
            }
            out << "Scanned aligns in "<<sw.Elapsed()
                << NcbiEndl;
            sw.Restart();
        }

        if ( count_unique ) {
            for ( auto& v : unique_counts ) {
                out << "unique " << v.second << " * " << v.first <<'\n';
            }
        }
        
        if ( 0 ) {
            typedef uint64_t TShortId;
            typedef map<TShortId, TSeqPos> TShortId2Pos;
            TShortId2Pos id2pos;
            for ( CCSraAlignIterator it(csra_db, query_idh,
                                        query_range.GetFrom(),
                                        query_range.GetLength(),
                                        mode, align_type);
                  it; ++it ) {
                CTempString ref_seq_id = it.GetRefSeqId();
                TSeqPos ref_pos = it.GetRefSeqPos();
                TSeqPos ref_len = it.GetRefSeqLen();
                bool ref_minus = it.GetRefMinusStrand();
                CTempString cigar = it.GetCIGAR();
                TShortId id(it.GetShortId1());//, it.GetShortId2());
                pair<TShortId2Pos::iterator, bool> ins =
                    id2pos.insert(make_pair(id, ref_pos));
                if ( ref_pos < ins.first->second ) {
                    ins.first->second = ref_pos;
                }
                if ( 1 ) {
                    out << ref_seq_id << " "
                        << ref_pos << " " << ref_len << " " << ref_minus
                        << " " << cigar << " "
//                     << *spot_group << " " << *spot_id << "." << *read_id
                        << MSerial_AsnText << *it.GetMatchEntry()
//                     << MSerial_AsnText << *it.GetShortBioseq()
                        << NcbiEndl;
                }
            }
            ITERATE ( TShortId2Pos, it, id2pos ) {
                out << it->second << ": "
                    << it->first
                    << NcbiEndl;
            }
        }
        if ( args["scan_reads"] ) {
            int count = args["limit_count"].AsInteger();
            Uint8 scanned = 0, clipped = 0;
            Uint8 rejected = 0, duplicate = 0, hidden = 0;
            Uint8 spot_id = 0;
            Uint4 read_id = 0;
            CCSraShortReadIterator::TBioseqFlags flags = make_quality_graph?
                CCSraShortReadIterator::fQualityGraph:
                CCSraShortReadIterator::fDefaultBioseqFlags;
            CCSraShortReadIterator it;
            if ( args["start_id"] ) {
                Uint8 start_id = args["start_id"].AsInteger();
                it = CCSraShortReadIterator(csra_db, start_id);
            }
            else {
                it = CCSraShortReadIterator(csra_db);
            }
            for ( ; it; ++it ) {
                if ( !spot_id ) {
                    spot_id = it.GetShortId1();
                    read_id = it.GetShortId2();
                }
                if ( verbose ) {
                    out << "Short read "<<it.GetSpotId()<<"."<<it.GetReadId()
                        << " of "<<it.GetMateCount() << " mate(s).";
                    if ( it.GetReadFilter() != SRA_READ_FILTER_PASS ) {
                        static const char* const filter_name[] = {
                            "good",
                            "Poor sequence quality",
                            "PCR duplicate",
                            "Hidden"
                        };
                        out << " " << filter_name[it.GetReadFilter()];
                    }
                    out << '\n';
                }
                ++scanned;
                if ( it.HasClippingInfo() ) {
                    ++clipped;
                }
                switch ( it.GetReadFilter() ) {
                case SRA_READ_FILTER_REJECT: ++rejected; break;
                case SRA_READ_FILTER_CRITERIA: ++duplicate; break;
                case SRA_READ_FILTER_REDACTED: ++hidden; break;
                default: break;
                }
                CRef<CBioseq> seq = it.GetShortBioseq(flags);
                if ( print_objects ) {
                    out << MSerial_AsnText << *seq;
                }
                if ( fasta ) {
                    out << '>' << seq->GetId().front()->AsFastaString() << " length="<<seq->GetInst().GetLength() << '\n';
                    out << seq->GetInst().GetSeq_data().GetIupacna().Get() << '\n';
                }
                if ( !--count ) break;
            }
            if ( spot_id ) {
                CCSraShortReadIterator it(csra_db, spot_id, read_id);
                CRef<CBioseq> seq = it.GetShortBioseq(flags);
                out << "First: " << MSerial_AsnText << *seq;
            }
            out << "Clipped: "<<clipped<<"/"<<scanned<<" = "<<100.*clipped/scanned<<"%" << endl;
            if ( rejected ) {
                out << "Rejected: "<<rejected<<"/"<<scanned<<" = "<<100.*rejected/scanned<<"%" << endl;
            }
            if ( duplicate ) {
                out << "Duplicate: "<<duplicate<<"/"<<scanned<<" = "<<100.*duplicate/scanned<<"%" << endl;
            }
            if ( hidden ) {
                out << "Hidden: "<<hidden<<"/"<<scanned<<" = "<<100.*hidden/scanned<<"%" << endl;
            }
        }
    }

    out << "Success." << NcbiEndl;
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//  Cleanup


void CCSRATestApp::Exit(void)
{
    SetDiagStream(0);
}


/////////////////////////////////////////////////////////////////////////////
//  MAIN


int main(int argc, const char* argv[])
{
    // Execute main application function
    return CCSRATestApp().AppMain(argc, argv);
}
