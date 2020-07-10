/*  $Id: gui_object_info_seq_align.cpp 44100 2019-10-25 18:51:46Z evgeniev $
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
 * Authors: Roman Katargin, Liangshou Wu, Victor Joukov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/utils.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/create_defline.hpp>
#include <algo/align/util/score_builder.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Spliced_seg.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>

#include <gui/objutils/gui_object_info_seq_align.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void initCGuiObjectInfoSeq_align()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CSeq_align::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoSeq_align>());
}

CGuiObjectInfoSeq_align* CGuiObjectInfoSeq_align::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CGuiObjectInfoSeq_align* gui_info = new CGuiObjectInfoSeq_align();
    gui_info->m_Align.Reset(dynamic_cast<const CSeq_align*>(object.object.GetPointer()));
    gui_info->m_Scope = object.scope;
    gui_info->m_AnchorRow = -1;
    return gui_info;
}

string CGuiObjectInfoSeq_align::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Align, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}


static size_t s_CountSegments(const CSeq_align& align)
{
    switch (align.GetSegs().Which()) {
    default:
        return 1;

    case CSeq_align::TSegs::e_Disc:
        return align.GetSegs().GetDisc().Get().size();

    case CSeq_align::TSegs::e_Spliced:
        return align.GetSegs().GetSpliced().GetExons().size();
    }
}


void s_AlignToSeqRanges(const CSeq_align& align, int row, vector<TSeqRange>& ranges)
{
    _ASSERT(align.CanGetSegs());
    // this should be added to CSeq_align as a list instead of RangeColl version of  GetAlignedBases
    switch (align.GetSegs().Which()) {
    case CSeq_align::TSegs::e_Denseg:
    {{
            const CDense_seg& ds = align.GetSegs().GetDenseg();
            for (CDense_seg::TNumseg i = 0; i < ds.GetNumseg(); ++i) {
                bool is_gapped = false;
                for (CDense_seg::TDim j = 0; j < ds.GetDim(); ++j) {
                    if (ds.GetStarts()[i * ds.GetDim() + j] == -1)
                    {
                        is_gapped = true;
                        break;
                    }
                }
                if (!is_gapped) {
                    TSignedSeqPos start = ds.GetStarts()[i * ds.GetDim() + row];
                    TSeqRange range;
                    range.SetFrom(start);
                    range.SetLength(ds.GetLens()[i]);
                    ranges.push_back(range);
                }
            }
        }}
    break;
    case CSeq_align::TSegs::e_Disc:
    {{
            ITERATE(CSeq_align::TSegs::TDisc::Tdata, iter,
                align.GetSegs().GetDisc().Get()) {
                s_AlignToSeqRanges(*(*iter), row, ranges);
            }
        }}
    break;
    case CSeq_align::TSegs::e_Spliced:
    {{
            ITERATE(CSpliced_seg::TExons, iter, align.GetSegs().GetSpliced().GetExons()) {
                ranges.push_back((*iter)->GetRowSeq_range(row, true));
                // should this be fixed?
                //ranges -= (*iter)->GetRowSeq_insertions(row, align.GetSegs().GetSpliced());
            }
        }}
    break;
    default:
        return;
    }
}


bool CGuiObjectInfoSeq_align::IsPolyA(const objects::CBioseq_Handle& bsh, TSeqPos start, TSeqPos stop)
{
    // we only apply testing on RNA sequences
    bool is_rna = false;    
    const CMolInfo* info = sequence::GetMolInfo(bsh);
    if (info) {
        if (info->GetBiomol() == CMolInfo::eBiomol_mRNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_pre_RNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_tRNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_snRNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_scRNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_cRNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_snoRNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_ncRNA  ||
            info->GetBiomol() == CMolInfo::eBiomol_tmRNA) {
            is_rna = true;
        }
    }

    if ( !is_rna) {
        CConstRef<CSeq_id> seq_id;
        CSeq_id_Handle idh = sequence::GetId(*bsh.GetSeqId(),
            bsh.GetScope(), sequence::eGetId_Best);
        if (idh) {
            seq_id = idh.GetSeqId();
        } else {
            seq_id = bsh.GetSeqId();
        }

        CSeq_id::EAccessionInfo id_info = seq_id->IdentifyAccession();

        if ((id_info & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_est  ||
            id_info == CSeq_id::eAcc_refseq_mrna  ||
            id_info == CSeq_id::eAcc_refseq_mrna_predicted  ||
            id_info == CSeq_id::eAcc_gpipe_mrna) {
                is_rna = true;
        }

        if ( !is_rna  &&  id_info & CSeq_id::eAcc_ambig  &&
            info->GetBiomol() == CMolInfo::eBiomol_other) {
                is_rna = true;
        }
    }                                                                                                                              
   
    if (is_rna) {
        CSeqVector s_vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        string seq;
        s_vec.GetSeqData(start, stop + 1, seq);
        int a_cnt = 0;
        int t_cnt = 0;
        for (TSeqPos bp = 0;  bp != seq.length();  bp++) {
            if (seq[bp] == 'A') ++a_cnt;
            else if (seq[bp] == 'T') ++t_cnt;
        }
        if (a_cnt > (int)(seq.length() * 0.9)  ||
            t_cnt > (int)(seq.length() * 0.9)) {
            return true;
        }
    }

    return false;
}


static CFastMutex s_DFLock;
static sequence::CDeflineGenerator s_DefGen; ///< This class seems to be non-thread safe even as a stack var

static
inline void s_GetDefline(CBioseq_Handle bsh, string& defline )
{
    CFastMutexGuard lock(s_DFLock);
    defline = s_DefGen.GenerateDefline(bsh);
}


static string s_ShortenIdLabel(const string& label)
{
    string out_label = label;
    // make title/label shorter in case it is a cSRA alignment on panfs
    if (out_label.length() > 20  &&  out_label.find("\\panfs\\") != string::npos) {
        size_t pos = out_label.find_last_of("\\");
        if (pos != string::npos) {
            out_label = out_label.substr(pos + 1);
            if (out_label.length() > 20) {
                pos = out_label.rfind('.');
                if (pos != string::npos  &&  pos > 0) {
                    pos = out_label.rfind('.', pos - 1);
                    if (pos != string::npos) {
                        out_label = out_label.substr(pos + 1);
                    }
                }
            }
        }
    }
    return out_label;
}

static string s_RemovePanfsBasePath(const string& label)
{
    string out_label = label;
    // make title/label shorter in case it is a cSRA alignment on panfs
    size_t pos = out_label.find("\\panfs\\");
    if (pos != string::npos) {
        pos = out_label.find("\\", pos + 7);
        if (pos != string::npos) {
            out_label = out_label.substr(pos);
        }
    }
    return out_label;
}

template<typename Iterator> size_t GetUnalignedRegionLength(Iterator subj_begin, Iterator subj_end, Iterator query_begin, Iterator query_end, bool reverse, TSeqPos at_p)
{
    using TAlnRangesPair = std::pair<TSeqRange, TSeqRange>;
    using TAlnRangesVector = std::vector<TAlnRangesPair>;   

    TAlnRangesVector aln_ranges;
    {
        Iterator subj;
        Iterator query;
        for (subj = subj_begin, query = query_begin; subj != subj_end; ++subj, ++query)
        {
            aln_ranges.push_back(std::make_pair(*subj, *query));
        }
        // Sort the alignment ranges by the from value of the subject
        sort(aln_ranges.begin(), aln_ranges.end(),
            [](TAlnRangesPair const& a, TAlnRangesPair const& b) -> bool
        {
            return a.first.GetFrom() < b.first.GetFrom();
        });
    }

    TSeqPos previous = 0;
    for (TAlnRangesVector::const_iterator aln_range = aln_ranges.begin(); aln_range<aln_ranges.end(); ++aln_range)
    {
        if (previous > at_p)
            break;

        if (previous != 0 && previous != aln_range->first.GetFrom()) {
            TSeqPos from{ previous };
            TSeqPos to{ aln_range->first.GetFrom() - 1 };
            if (from > to) {
                swap(from, to);
            }
            if ((from <= at_p) && (at_p <= to)) {
                _ASSERT(aln_range != aln_ranges.begin());
                auto prev_range = aln_range - 1;
                TSeqRange unaligned_query;
                if (reverse)
                    unaligned_query = TSeqRange(aln_range->second.GetToOpen(), prev_range->second.GetFrom() - 1);
                else
                    unaligned_query = TSeqRange(prev_range->second.GetToOpen(), aln_range->second.GetFrom() - 1);

                return unaligned_query.GetLength();
            }
        }
        previous = aln_range->first.GetToOpen();
    }

    return 0;
}

void CGuiObjectInfoSeq_align::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    const CSeq_align::TDim num_rows_limit = 10;

    ///
    /// first, format our seq-ids
    ///
    vector<bool> strands;
    vector<string> ids;
    CSeq_align::TDim num_rows = m_Align->CheckNumRows();
    string curr_text;
    {{
        for(CSeq_align::TDim i = 0;  i < num_rows  &&  i < num_rows_limit;  ++i ){
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( m_Align->GetSeq_id(i) );
            CSeq_id_Handle idh_best = sequence::GetId(idh, *m_Scope, sequence::eGetId_Best);
            if( idh_best ){
                idh = idh_best;
            }

            if ( !curr_text.empty() ) {
                curr_text += " x ";
            }
            CConstRef<CSeq_id> seq_id(idh.GetSeqIdOrNull());
            string label;
            if (!seq_id) {
                label = "UNK";
            } else {
                idh.GetSeqId()->GetLabel(&label, CSeq_id::eContent);
            }
            ids.push_back(label);
            curr_text += s_ShortenIdLabel(label);
            try {
                objects::ENa_strand strand = m_Align->GetSeqStrand(i);
                if ((eNa_strand_plus == strand) || (eNa_strand_minus == strand))
                    strands.push_back(strand == eNa_strand_plus);
            } catch (CException&) {
                // ignore the exception
            }
        }
    }}

    if (num_rows > ids.size()) {
        curr_text += " x ... [total ";
        curr_text += NStr::IntToString(num_rows, NStr::fWithCommas);
        curr_text += "]";
    }
    tooltip.AddRow("Alignment:", curr_text);

    if (num_rows > ids.size()) { // early escape?
        CSeqUtils::CreateTableEnd();
        return;
    }

    bool is_protein = false;
    if (ids.size() == 2  &&  num_rows == 2  &&  m_AnchorRow >= 0  &&  m_AnchorRow < 2) {
        TSeqRange range = m_Align->GetSeqRange(m_AnchorRow);
        curr_text = ids[m_AnchorRow] + " (";
        curr_text += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
        curr_text += "..";
        curr_text += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);
        curr_text += ")";
        tooltip.AddRow("Anchor:", curr_text);

        range = m_Align->GetSeqRange(1 - m_AnchorRow);
        t_title = s_ShortenIdLabel(ids[1 - m_AnchorRow]);
        curr_text = s_RemovePanfsBasePath(ids[1 - m_AnchorRow]) + " (";
        curr_text += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
        curr_text += "..";
        curr_text += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);
        curr_text += ")";
        tooltip.AddRow("Query:", curr_text);

        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(m_Align->GetSeq_id(1 - m_AnchorRow));
        if (bsh) {
            if (bsh.IsProtein()) {
                is_protein = true;
            }
            s_GetDefline(bsh, curr_text);
            if ( !curr_text.empty() ) {
                tooltip.AddRow("", curr_text);
            }
        }

        if (m_Align->CanGetSegs()) {
            vector<TSeqRange> subj_ranges;
            s_AlignToSeqRanges(*m_Align, m_AnchorRow, subj_ranges);
            vector<TSeqRange> query_ranges;
            s_AlignToSeqRanges(*m_Align, 1 - m_AnchorRow, query_ranges);

            size_t unaligned_region{ 0 };
            // attempt the check for unaligned regions only if all strands have been exactly identified
            if (strands.size() == num_rows) {
                if (strands[m_AnchorRow])
                    unaligned_region = GetUnalignedRegionLength<vector<TSeqRange>::iterator>(subj_ranges.begin(), subj_ranges.end(), query_ranges.begin(), query_ranges.end(), strands[0] != strands[1], at_p);
                else
                    unaligned_region = GetUnalignedRegionLength<vector<TSeqRange>::reverse_iterator>(subj_ranges.rbegin(), subj_ranges.rend(), query_ranges.rbegin(), query_ranges.rend(), strands[0] != strands[1], at_p);
            }
            if (unaligned_region > 0) {
                string label = NStr::NumericToString(unaligned_region, NStr::fWithCommas);
                label += " bp";
                tooltip.AddRow("Unaligned region:", label);
            }
        }
    }

    /// Report and SRA read issues:
    string read_status = GetSRAReadStatus();
    if (read_status != "") {
        tooltip.AddRow("SRA read status:", read_status);
    }

    /// only report strand for pair-wise alignments
    if (!is_protein  &&  num_rows == 2  &&  strands.size() == 2) {
        tooltip.AddRow("Relative orientation:", strands[0] == strands[1] ? "forward" : "reverse");
    }

    string tag_name;
    ///
    /// next, add a remark about the total aligned range
    ///
    CScoreBuilder builder;
    // new method
    TSeqPos align_length = builder.GetAlignLength(*m_Align);
    tag_name = "Span on  ";
    if (ids.size() == 2 && num_rows == 2 && m_AnchorRow >= 0 && m_AnchorRow < 2) {
        tag_name += ids[m_AnchorRow] ;
    }
    else {
        tag_name += "master";
    }

    tooltip.AddRow(tag_name + ':', NStr::IntToString(align_length, NStr::fWithCommas));

    size_t segs = s_CountSegments(*m_Align);
    tooltip.AddRow("Segments:", NStr::SizetToString(segs, NStr::fWithCommas));

    char buf[255];
    double coverage = -1.0;
    if ( !m_Align->GetNamedScore(CSeq_align::eScore_PercentCoverage, coverage) && m_Scope ){
        try {
            coverage = builder.GetPercentCoverage( *m_Scope, *m_Align, 1 - m_AnchorRow );
        } catch (const CException&) {
            coverage = -1.0;
        }
    }
    if (coverage >= 0.0) {
        if (coverage < 100.0  &&  coverage > 99.9) {
            curr_text = "99.9+";
        } else {
            sprintf(buf, "%2.1f", coverage);
            curr_text = buf;
        }
        curr_text += "%";
        tooltip.AddRow("Coverage:", curr_text);
    }

    if (align_length < 1000000) {
        //int identities = builder.GetIdentityCount(*m_Scope, align);
        //int mismatches = builder.GetMismatchCount(*m_Scope, align);
        int identities = -1;
        int mismatches = -1;
        if (!m_Align->GetNamedScore(CSeq_align::eScore_IdentityCount, identities)  ||
            !m_Align->GetNamedScore(CSeq_align::eScore_MismatchCount, mismatches)) {
                try {
                    builder.GetMismatchCount(*m_Scope, *m_Align, identities, mismatches);
                } catch (const exception&) {
                    identities = -1;
                    mismatches = -1;
                }
        }

        if (identities >= 0) {
            double identity = 0.0;
            if ( !m_Align->GetNamedScore(CSeq_align::eScore_PercentIdentity, identity) ) {
                TSeqPos align_length_no_gaps = builder.GetAlignLength(*m_Align, true);
                identity = identities * 100.0 / align_length_no_gaps;
            }
            if (identity < 100.0  &&  identity > 99.9) {
                curr_text = "99.9+";
            } else {
                sprintf(buf, "%2.1f", identity);
                curr_text = buf;
            }
            curr_text += "%";
            tooltip.AddRow("Identity:", curr_text);
        } 
        if (mismatches >= 0) {
            tooltip.AddRow("Mismatches:", NStr::NumericToString(mismatches, NStr::fWithCommas));
        }
    }

    try {
        int gap_count = builder.GetGapCount(*m_Align);
        // Count introns and gaps from CIGAR string
        CConstRef<CUser_object> tracebacks = m_Align->FindExt("Tracebacks");
        if (tracebacks  &&  tracebacks->HasField("CIGAR") && tracebacks->GetField("CIGAR").GetData().IsStr()) {
            string cigar{ tracebacks->GetField("CIGAR").GetData().GetStr() };
            size_t introns_count = std::count(cigar.begin(), cigar.end(), 'N');
            if (introns_count > 0) {
                tooltip.AddRow("Introns:", NStr::NumericToString(introns_count, NStr::fWithCommas));
                gap_count = (int)std::count(cigar.begin(), cigar.end(), 'D');
            }
        }
        tooltip.AddRow("Gaps:", NStr::IntToString(gap_count, NStr::fWithCommas));
    } catch (const CException&) {
        // ignore
    }
    
    {
        CConstRef<CUser_object> tracebacks = m_Align->FindExt("Tracebacks");
        if (tracebacks  &&  tracebacks->HasField("PC") && tracebacks->GetField("PC").GetData().IsInt()) {
            tooltip.AddRow("Phred:", NStr::IntToString(tracebacks->GetField("PC").GetData().GetInt()));
        }
        if (tracebacks  &&  tracebacks->HasField("PS") && tracebacks->GetField("PS").GetData().IsInt()) {
            tooltip.AddRow("Phase set:", NStr::IntToString(tracebacks->GetField("PS").GetData().GetInt()));
        }
        if (tracebacks  &&  tracebacks->HasField("HP") && tracebacks->GetField("HP").GetData().IsInt()) {
            tooltip.AddRow("Haplotype:", NStr::IntToString(tracebacks->GetField("HP").GetData().GetInt()));
        }
        if (tracebacks  &&  tracebacks->HasField("MI") && tracebacks->GetField("MI").GetData().IsInt()) {
            tooltip.AddRow("Global molecule identifier:", NStr::IntToString(tracebacks->GetField("MI").GetData().GetInt()));
        }

        if (tracebacks  &&  tracebacks->HasField("CIGAR")  &&  tracebacks->GetField("CIGAR").GetData().IsStr()) {
            tooltip.AddRow("CIGAR:", tracebacks->GetField("CIGAR").GetData().GetStr());
        }
    }

    if (at_p != (TSeqPos)-1) {
        tooltip.AddRow("Position:", NStr::UIntToString(at_p + 1, NStr::fWithCommas));
    }

    CConstRef< CUser_object > user_object = m_Align->FindExt("GBenchTooltips");
    if (!user_object || !user_object->IsSetData())
        return;
    ITERATE(CUser_object_Base::TData, it_field, user_object->GetData())
        x_AddUserObject(tooltip, *(*it_field));
}


void CGuiObjectInfoSeq_align::x_AddUserObject(ITooltipFormatter& tooltip, const CUser_field &field) const
{
    if (!field.IsSetData())
        return;

    if (field.GetData().IsFields()) {
        if (field.IsSetLabel() && field.GetLabel().IsStr())
            tooltip.AddSectionRow(field.GetLabel().GetStr());
        ITERATE(CUser_field::TData::TFields, it_field, field.GetData().GetFields()) {
            x_AddUserObject(tooltip, *(*it_field));
        }
    }
    else {
        if (!field.IsSetLabel() || !field.GetLabel().IsStr())
            return;
        if (field.GetData().IsStr())
            tooltip.AddRow(field.GetLabel().GetStr() + ':', field.GetData().GetStr());
        else if (field.GetData().IsInt())
            tooltip.AddRow(field.GetLabel().GetStr() + ':', NStr::NumericToString(field.GetData().GetInt()));

    }
}

void CGuiObjectInfoSeq_align::GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const
{
    if (m_Align->CheckNumRows() != 2 || m_AnchorRow == -1)
        return;
    int aligned_seq = 1 - m_AnchorRow;
    const CSeq_id& seq_id = m_Align->GetSeq_id(aligned_seq);
    CSeq_id_Handle sih = sequence::GetId(seq_id, *m_Scope, sequence::eGetId_Best);
    if ( !sih ) {
        sih = sequence::GetId(seq_id, *m_Scope, sequence::eGetId_Canonical);
        if ( !sih )
            return;
    }

    string seq_id_str;
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(seq_id);
    if (!bsh)
        return;

    try {
        string seq_id_str = bsh.GetSeqId()->AsFastaString();
        bool is_protein = bsh.IsProtein();

        TSeqRange range = m_Align->GetSeqRange(aligned_seq);
        unsigned unaligned_bases_5(0);
        unsigned unaligned_bases_3(0);
        bool polyA(false);
        if (range.GetFrom() > 0) {
            unaligned_bases_5 = range.GetFrom();
        }

        if (range.GetTo() < bsh.GetBioseqLength() - 1) {
            unaligned_bases_3 = bsh.GetBioseqLength() - 1 - range.GetTo();
            if (!is_protein && CGuiObjectInfoSeq_align::IsPolyA(bsh, range.GetToOpen(), bsh.GetBioseqLength() - 1))
                polyA = true;
        }
        if (!unaligned_bases_5 && !unaligned_bases_3)
            return;

        string tag_name("Unaligned ");
        tag_name += is_protein ? "residues" : "bases";

        links.AddLinksTitle(tag_name);

        bool reverse = false;
        if (!is_protein)
            reverse = ((m_Align->GetSeqStrand(0) == eNa_strand_plus) != (m_Align->GetSeqStrand(1) == eNa_strand_plus));

        if (unaligned_bases_5) {
            string text(NStr::IntToString(unaligned_bases_5));
            string name(is_protein ? "N-terminal end" : "5'");
            name += ':';
            string url("$UNALIGNED_REGION$?id=");
            url += seq_id_str;
            url += "&from=0&to=";
            url += NStr::IntToString(unaligned_bases_5);
            url += "&reverse=";
            url += NStr::BoolToString(reverse);
            url += "&flip=";
            url += NStr::BoolToString(m_FlippedStrands);
            links.AddLinkRow(name, text, url);
        }
        if (unaligned_bases_3) {
            string text(NStr::IntToString(unaligned_bases_3));
            string name(is_protein ? "C-terminal end" : "3'");
            name += ':';
            string url("$UNALIGNED_REGION$?id=");
            url += seq_id_str;
            url += "&from=";
            url += NStr::IntToString(range.GetTo() + 1);
            url += "&to=";
            url += NStr::IntToString(bsh.GetBioseqLength());
            url += "&reverse=";
            url += NStr::BoolToString(reverse);
            url += "&flip=";
            url += NStr::BoolToString(m_FlippedStrands);
            if (polyA) {
                url += "&polyA=true";
                text += " (polyA)";
            }
            links.AddLinkRow(name, text, url);
        }
    }
    catch (const CException &error)
    {
        LOG_POST(Warning << "CGuiObjectInfoSeq_align::GetLinks() error: " << error);
    }
}

// PCR errors in CSRA data saved in user object.  Only 1 will be set
// so return first one found. SV-2153
string CGuiObjectInfoSeq_align::GetSRAReadStatus() const
{
    if (m_Align->IsSetExt()) {
        const CSeq_align_Base::TExt& exts = m_Align->GetExt();
        ITERATE (CSeq_align_Base::TExt, iter, exts) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "PCR duplicate") {
                    return "PCR duplicate";
            }
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "Poor sequence quality") {
                    return "Poor sequence quality";
            }
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "Hidden") {
                    return "Hidden";
            }
        }
    }

    return "";
}

double CGuiObjectInfoSeq_align::GetCoverage() const
{
    string tag_name;
    ///
    /// next, add a remark about the total aligned range
    ///
    CScoreBuilder builder;
    // new method
    TSeqPos align_length = builder.GetAlignLength(*m_Align);

    double coverage = -1.0;
    if ( !m_Align->GetNamedScore(CSeq_align::eScore_PercentCoverage, coverage) && m_Scope ){
        try {
            coverage = builder.GetPercentCoverage( *m_Scope, *m_Align );
        } catch (CException&) {
            // ignore
        }
    }

    return coverage;
}


END_NCBI_SCOPE
