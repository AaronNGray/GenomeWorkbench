#ifndef ALGO_GNOMON___CONSENSUS__HPP
#define ALGO_GNOMON___CONSENSUS__HPP

#include <algo/gnomon/gnomon_model.hpp>
#include <corelib/ncbiargs.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(gnomon)

class CLiteIndel 
{
public:
    CLiteIndel(int loc, int len, const string& indelv = "") : m_loc(loc), m_len(len), m_indelv(indelv) {}
    CLiteIndel(const CInDelInfo& indl) : m_loc(indl.Loc()), m_len(indl.Len()), m_indelv(indl.IsDeletion() ? indl.GetInDelV() : "") {}
    TSignedSeqPos Loc() const { return m_loc; }
    int Len() const { return m_len; }
    const string& GetInDelV() const { return m_indelv; }
    bool IsInsertion() const { return m_indelv.empty(); }
    bool IsDeletion() const { return !m_indelv.empty(); }
    bool operator<(const CLiteIndel& fsi) const
    {
        if(m_loc != fsi.m_loc)
            return m_loc < fsi.m_loc;
        else if(IsDeletion() != fsi.IsDeletion())
            return IsDeletion();  // if location is same deletion first
        else if(m_len != fsi.m_len)
            return m_len < fsi.m_len;
        else
            return m_indelv < fsi.m_indelv;
    }

private:
    TSignedSeqPos m_loc;  // left location for insertion, deletion is before m_loc
                          // insertion - when there are extra bases in the genome
    int m_len;
    string m_indelv;
};


typedef vector<CLiteIndel> TLiteInDels;
typedef vector<const CLiteIndel*> TLiteInDelsP;

struct SSamData {
    double m_weight;
    int m_contigp;
    string m_cigar;
    string m_seq;
};

class CLiteAlign {
public:
    CLiteAlign(TSignedSeqRange range, const TLiteInDels& indels, set<CLiteIndel>& indel_holder, double weight, double ident);
    CLiteAlign(const SSamData& ad, const string& contig, set<CLiteIndel>& indel_holder);
    double Weight() const { return m_weight; }
    void SetWeight(double w) { m_weight = w; }
    double Ident() const { return m_ident; }
    TSignedSeqRange Limits() const { return m_range; }
    TLiteInDelsP GetInDels() const {
        return m_indels;
    }
    string TranscriptSeq(const string& contig) const;

private:
    double m_weight;
    double m_ident;
    TSignedSeqRange m_range;
    TLiteInDelsP m_indels;
};

typedef list<CLiteAlign> TLiteAlignList;


struct AlignsLeftEndFirst {
    bool operator()(const CLiteAlign* ap, const CLiteAlign* bp) { // left end increasing
        return ap->Limits().GetFrom() < bp->Limits().GetFrom();
    }
};

struct AlignsHighIdentFirst {
    bool operator()(const CLiteAlign* ap, const CLiteAlign* bp) {
        if(ap->Ident() == bp->Ident()) {
            return ap->Weight() > bp->Weight();
        } else {
            return ap->Ident() > bp->Ident();
        }
    }
};

typedef map<string,int> TSIMap;

class CMultAlign {

public:
    CMultAlign() : m_max_len(0) { SetDefaultParams(); };
    CMultAlign(const CConstRef<CSeq_id>& seqid, CScope& scope) { 
        SetDefaultParams();
        SetGenomic(seqid, scope); 
    }
    CMultAlign(const string& contig_seq, const string& contig_acc) {
        SetDefaultParams();
        m_contig_id = contig_acc;
        m_contigt = contig_seq;
        m_base = m_contigt;
    }
    void AddAlignment(const SSamData& align);
    void SetGenomic(const CConstRef<CSeq_id>& seqid, CScope& scope); // makes full reset keeping the parameters
    void AddAlignment(const CAlignModel& align); // adds each exon as a separate CLiteAlign; doesn't do left/right notaligned
    TAlignModelList GetVariationAlignList(bool correctionsonly);
    void Variations(map<TSignedSeqRange,TSIMap>& variations, list<TSignedSeqRange>& confirmed_ranges);

    static void SetupArgDescriptions(CArgDescriptions* arg_desc);
    void ProcessArgs(const CArgs& args);

private:
    void SetDefaultParams();
    void SelectAligns(vector<const CLiteAlign*>& all_alignsp);
    void PrepareReads(const vector<const CLiteAlign*>& all_alignsp);
    void InsertDashesInBase();
    void InsertDashesInReads();
    void GetCounts();
    void PrepareStats();

    TSignedSeqRange LegitRange(int ir);
    string EmitSequenceFromRead(int ir, const TSignedSeqRange& word_range);
    string EmitSequenceFromBase(const TSignedSeqRange& word_range);
    bool CheckWord(const TSignedSeqRange& word_range, const string& word);
    int FindNextStrongWord(int nextp, const string& maximal_bases, string& strong_word, TSignedSeqRange& strong_word_range, int& first_gap);
    void SeqCountsBetweenTwoStrongWords(const TSignedSeqRange& prev_strong_word_range, const string& prev_strong_word, const TSignedSeqRange& strong_word_range, const string& strong_word,  TSIMap& seq_counts, int& total_cross, int& accepted_cross);

    int m_max_len;
    vector<string> m_reads;
    vector<int> m_starts;
    vector<const CLiteAlign*> m_alignsp;
    typedef map<int,int> TIntMap;
    TIntMap m_contig_to_align;
    TIntMap m_align_to_contig;
    typedef map<char,int> TCharIntMap;
    map<int,TCharIntMap> m_counts;
    TLiteAlignList m_aligns;
    set<CLiteIndel> m_indel_holder;

    string m_base;
    string m_contigt;
    string m_contig_id;

    //parameters
    int m_min_edge;
    int m_min_coverage;
    int m_word;
    int m_maxNs;
    double m_min_rel_support_for_variant;
    int m_min_abs_support_for_variant;
    double m_strong_consensus;
};

END_SCOPE(gnomon)
END_NCBI_SCOPE

#endif // ALGO_GNOMON___CONSENSUS__HPP

