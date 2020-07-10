/*  $Id: validerror_align.cpp 589209 2019-07-08 19:07:39Z kans $
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
 * Author:  Jonathan Kans, Clifford Clausen, Aaron Ucko......
 *
 * File Description:
 *   validation of seq_align
 *   .......
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Dense_diag.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Packed_seg.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Std_seg.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <objtools/alnmgr/aln_user_options.hpp>
#include <objtools/alnmgr/aln_stats.hpp>
#include <objtools/alnmgr/alnvec.hpp>
#include <objtools/alnmgr/sparse_aln.hpp>
#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>

#include <map>
#include <vector>
#include <algorithm>

#include <objtools/validator/validerror_align.hpp>
#include <objtools/validator/utilities.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(validator)

USING_SCOPE(sequence);


// ================================  Public  ================================


CValidError_align::CValidError_align(CValidError_imp& imp) :
    CValidError_base(imp)
{
}


CValidError_align::~CValidError_align(void)
{
}

static void GetInvalidSeqAlignTypeStr(CSeq_align::C_Segs::E_Choice segtype, int order, string& msg)
{
    msg = "Segs: This alignment has an undefined or unsupported Seqalign segtype " + NStr::IntToString(segtype);
    if (order > 0) {
        msg += " (alignment number " + NStr::IntToString(order) + ')';
    }
}

void CValidError_align::ValidateSeqAlign(const CSeq_align& align, int order)
{
    if (!align.IsSetSegs()) {
        PostErr (eDiag_Error, eErr_SEQ_ALIGN_NullSegs, 
                 "Segs: This alignment is missing all segments.  This is a non-correctable error -- look for serious formatting problems.",
                 align);
        return;
    }
                 
    const CSeq_align::TSegs& segs = align.GetSegs();
    CSeq_align::C_Segs::E_Choice segtype = segs.Which();
    switch ( segtype ) {
    
    case CSeq_align::C_Segs::e_Dendiag:
        x_ValidateDendiag(segs.GetDendiag(), align);
        break;

    case CSeq_align::C_Segs::e_Denseg:
        x_ValidateDenseg(segs.GetDenseg(), align);
        break;

    case CSeq_align::C_Segs::e_Std:
        x_ValidateStd(segs.GetStd(), align);
        break;
    case CSeq_align::C_Segs::e_Packed:
        x_ValidatePacked(segs.GetPacked(), align);
        break;

    case CSeq_align::C_Segs::e_Disc:
        // call recursively
        {

        int order = 1;
        ITERATE(CSeq_align_set::Tdata, sali, segs.GetDisc().Get()) {
            ValidateSeqAlign(**sali, order++);
        }

        }
        return;

    case CSeq_align::C_Segs::e_not_set:
        PostErr (eDiag_Error, eErr_SEQ_ALIGN_NullSegs, 
                 "Segs: This alignment is missing all segments.  This is a non-correctable error -- look for serious formatting problems.",
                 align);
        return;
        break;
    case CSeq_align::C_Segs::e_Sparse:
    case CSeq_align::C_Segs::e_Spliced:
        // ignore new segtype warnings in genomic gpipe sequence
        if (m_Imp.IsGpipe() && m_Imp.IsGenomic()) {
            return;
        }

        {
            string msg;
            GetInvalidSeqAlignTypeStr(segtype, order, msg);
            PostErr(eDiag_Warning, eErr_SEQ_ALIGN_Segtype, msg, align);
        }
        return;
        break;
    default:
        // ignore new segtype warnings in genomic gpipe sequence
        if (m_Imp.IsGpipe() && m_Imp.IsGenomic()) {
            return;
        }

        {
            string msg;
            GetInvalidSeqAlignTypeStr(segtype, order, msg);
            PostErr(eDiag_Error, eErr_SEQ_ALIGN_Segtype, msg, align);
        }
        return;
        break;
    }  // end of switch statement

    if (segtype != CSeq_align::C_Segs::e_Denseg 
        && align.IsSetType() 
        && (align.GetType() == CSeq_align::eType_partial
            || align.GetType() == CSeq_align::eType_global)) {
		    PostErr(eDiag_Error, eErr_SEQ_ALIGN_UnexpectedAlignmentType, "UnexpectedAlignmentType: This is not a DenseSeg alignment.", align);
    }
	  try {
        x_ValidateAlignPercentIdentity (align, false);
	  } catch (CException &) {
	  } catch (std::exception &) {
	  }

}


// ================================  Private  ===============================

typedef struct ambchar {
  char ambig_char;
  const char * match_list;
} AmbCharData;

static const AmbCharData ambiguity_list[] = {
 { 'R', "AG" },
 { 'Y', "CT" },
 { 'M', "AC" },
 { 'K', "GT" },
 { 'S', "CG" },
 { 'W', "AT" },
 { 'H', "ACT" },
 { 'B', "CGT" },
 { 'V', "ACG" },
 { 'D', "AGT" }};

static const int num_ambiguities = sizeof (ambiguity_list) / sizeof (AmbCharData);

static bool s_AmbiguousMatch (char a, char b)
{
    if (a == b) {
        return true;
    } else if (a == 'N' || b == 'N') {
        return true;
    } else {
        char search[2];
        search[1] = 0;
        for (int i = 0; i < num_ambiguities; i++) {
            search[0] = b;
            if (a == ambiguity_list[i].ambig_char
                && NStr::Find (ambiguity_list[i].match_list, search) != string::npos) {
                return true;
            }
            search[0] = a;
            if (b == ambiguity_list[i].ambig_char
                && NStr::Find (ambiguity_list[i].match_list, search) != string::npos) {
                return true;
            }
        }
    }
    return false;
}


static size_t s_GetNumIdsToUse (const CDense_seg& denseg)
{
    size_t dim     = denseg.GetDim();
    if (!denseg.IsSetIds()) {
        dim = 0;
    } else if (denseg.GetIds().size() < dim) {
        dim = denseg.GetIds().size();
    }
    return dim;
}


bool CValidError_align::AlignmentScorePercentIdOk(const CSeq_align& align)
{
    if (!align.IsSetScore()) {
        return false;
    }
    ITERATE(CSeq_align::TScore, it, align.GetScore()) {
        if ((*it)->IsSetId() && (*it)->GetId().IsStr() &&
            NStr::EqualNocase((*it)->GetId().GetStr(), "pct_identity_ungap") &&
            (*it)->IsSetValue() && (*it)->GetValue().IsReal()) {
            if ((*it)->GetValue().GetReal() > 50.0) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}


bool CValidError_align::IsTpaAlignment(const CDense_seg& denseg, CScope& scope)
{
    int dim = denseg.GetDim();
    if (dim != s_GetNumIdsToUse(denseg)) {
        return false;
    }

    bool is_tpa = false;
    for (CDense_seg::TDim row = 0; row < dim && !is_tpa; ++row) {
        CRef<CSeq_id> id = denseg.GetIds()[row];
        CBioseq_Handle bsh = scope.GetBioseqHandle(*id);
        if (bsh) {
            CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
            while (desc_ci && !is_tpa) {
                if (desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
                    && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), "TpaAssembly")) {
                    is_tpa = true;
                }
                ++desc_ci;
            }
        }
    }

    return is_tpa;
}


bool CValidError_align::IsTpaAlignment(const CSparseAln& sparse_aln, CScope& scope)
{
    // check to see if alignment is TPA
    bool is_tpa = false;
    for (CSparseAln::TDim row = 0; row < sparse_aln.GetDim() && !is_tpa; ++row) {
        const CSeq_id& id = sparse_aln.GetSeqId(row);
        CBioseq_Handle bsh = scope.GetBioseqHandle(id);
        if (bsh) {
            CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
            while (desc_ci && !is_tpa) {
                if (desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
                    && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), "TpaAssembly")) {
                    is_tpa = true;
                }
                ++desc_ci;
            }
        }
    }
    return is_tpa;
}


void s_CalculateMatchingColumns(const CDense_seg& denseg, TSeqPos &col, size_t &num_match, bool& ids_missing, bool internal_gaps, CScope& scope)
{
    int dim = denseg.GetDim();
    if (dim != s_GetNumIdsToUse(denseg)) {
        return;
    }
    if (denseg.GetStarts().size() != denseg.GetNumseg() * dim) {
        return;
    }

    try {
        CRef<CAlnVec> av(new CAlnVec(denseg, scope));
        av->SetGapChar('-');
        av->SetEndChar('.');

        TSeqPos aln_len = av->GetAlnStop() + 1;

        try {
            while (col < aln_len && !ids_missing) {
                string column;
                av->GetColumnVector(column, col);
                if (internal_gaps && NStr::Find(column, "-") != string::npos) {
                    // do nothing
                } else {
                    bool match = true;
                    // don't care about end gaps, ever
                    NStr::ReplaceInPlace(column, ".", "");
                    // if we cared about internal gaps, it would have been handled above
                    NStr::ReplaceInPlace(column, "-", "");
                    if (!NStr::IsBlank(column)) {
                        string::iterator it1 = column.begin();
                        string::iterator it2 = it1;
                        ++it2;
                        while (match && it2 != column.end()) {
                            if (!s_AmbiguousMatch(*it1, *it2)) {
                                match = false;
                            }
                            ++it2;
                            if (it2 == column.end()) {
                                ++it1;
                                it2 = it1;
                                ++it2;
                            }
                        }
                    }
                    if (match) {
                        ++num_match;
                    }
                }
                col++;
            }
        } catch (CException &x1) {
            // if sequence is not in scope,
            // the above is impossible
            // report 0 %, same as C Toolkit
            col = aln_len;
            if (NStr::StartsWith(x1.GetMsg(), "iterator out of range")) {
                // bad offsets
            } else {
                ids_missing = true;
            }
        } catch (std::exception &) {
            // if sequence is not in scope,
            // the above is impossible
            // report 0 %, same as C Toolkit
            col = aln_len;
            ids_missing = true;
        }
    } catch (CException &) {
        // if AlnVec can't resolve seq id, 
        // the above is impossible
        // report 0 %, same as C Toolkit
        col = 1;
        num_match = 0;
        ids_missing = true;
    }
}


bool s_DensegHasAccessionWithZeroVersion(const CDense_seg& denseg)
{
    if (!denseg.IsSetIds()) {
        return false;
    }
    ITERATE(CDense_seg::TIds, id, denseg.GetIds()) {
        if ((*id)->IsGenbank() && (*id)->GetGenbank().IsSetVersion() && (*id)->GetGenbank().GetVersion() == 0) {
            return true;
        }
    }
    return false;
}


void s_FixZeroVersions(CDense_seg& denseg)
{
    if (!denseg.IsSetIds()) {
        return;
    }
    NON_CONST_ITERATE(CDense_seg::TIds, id, denseg.SetIds()) {
        if ((*id)->IsGenbank() && (*id)->GetGenbank().IsSetVersion() && (*id)->GetGenbank().GetVersion() == 0) {
            (*id)->SetGenbank().ResetVersion();
        }
    }
}


void CValidError_align::x_ValidateAlignPercentIdentity (const CSeq_align& align, bool internal_gaps)
{
    TSeqPos col = 0;
    size_t num_match = 0;
    bool   ids_missing = false;

    // Now calculate Percent Identity
    if (!align.IsSetSegs()) {
        return;
    } else if (AlignmentScorePercentIdOk(align)) {
        return;
    } else if (align.GetSegs().IsDenseg()) {
        const CDense_seg& denseg = align.GetSegs().GetDenseg();
        int dim = denseg.GetDim();
        if (dim != s_GetNumIdsToUse(denseg)) {
            return;
        }
        if (s_DensegHasAccessionWithZeroVersion(denseg)) {
            // This is a dodge, to avoid the problem that Sequin is generating
            // alignments with accessions with version 0
            CRef<CDense_seg> tmp(new CDense_seg());
            tmp->Assign(denseg);
            s_FixZeroVersions(*tmp);
            // make sure this isn't a TPA alignment
            if (IsTpaAlignment(*tmp, *m_Scope)) {
                return;
            }
            s_CalculateMatchingColumns(*tmp, col, num_match, ids_missing, internal_gaps, *m_Scope);
        } else {
            // make sure this isn't a TPA alignment
            if (IsTpaAlignment(denseg, *m_Scope)) {
                return;
            }
            s_CalculateMatchingColumns(denseg, col, num_match, ids_missing, internal_gaps, *m_Scope);
        }
    } else if (align.GetSegs().IsStd() && !(FindSegmentGaps(align.GetSegs().GetStd(), m_Scope)).empty()) {
        col = 1;
        num_match = 0;
        ids_missing = true;
    } else {
        try {
            TIdExtract id_extract;
            TAlnIdMap aln_id_map(id_extract, 1);
            aln_id_map.push_back (align);
            TAlnStats aln_stats (aln_id_map);

            // Create user options
            CAlnUserOptions aln_user_options;
            TAnchoredAlnVec anchored_alignments;

            CreateAnchoredAlnVec (aln_stats, anchored_alignments, aln_user_options);

            /// Build a single anchored aln
            CAnchoredAln out_anchored_aln;

            /// Optionally, create an id for the alignment pseudo sequence
            /// (otherwise one would be created automatically)
            CRef<CSeq_id> seq_id (new CSeq_id("lcl|PSEUDO ALNSEQ"));
            CRef<CAlnSeqId> aln_seq_id(new CAlnSeqId(*seq_id));
            TAlnSeqIdIRef pseudo_seqid(aln_seq_id);

            BuildAln(anchored_alignments,
                     out_anchored_aln,
                     aln_user_options,
                     pseudo_seqid);

            CSparseAln sparse_aln(out_anchored_aln, *m_Scope);

            // check to see if alignment is TPA
            if (IsTpaAlignment(sparse_aln, *m_Scope)) {
                return;
            }

            vector <string> aln_rows;
            vector <TSeqPos> row_starts;
            vector <TSeqPos> row_stops;

            for (CSparseAln::TDim row = 0;  row < sparse_aln.GetDim() && !ids_missing;  ++row) {
                try {
                    string sequence;
                    sparse_aln.GetAlnSeqString
                        (row,
                         sequence,
                         sparse_aln.GetAlnRange());
                    aln_rows.push_back (sequence);
                    TSignedSeqPos aln_start = sparse_aln.GetSeqAlnStart(row);
                    TSignedSeqPos start = sparse_aln.GetSeqPosFromAlnPos(row, aln_start);
                    row_starts.push_back (start);
                    row_stops.push_back (sparse_aln.GetAlnPosFromSeqPos(row, sparse_aln.GetSeqAlnStop(row)));
		            } catch (CException &) {
                    ids_missing = true;
		            } catch (std::exception &) {
                    // if sequence is not in scope,
                    // the above is impossible
                    ids_missing = true;
                }
            }

            bool any_data = false;
            if (!ids_missing) {
                TSeqPos aln_len = sparse_aln.GetAlnRange().GetLength();
                while (col < aln_len) {
                    string column;
                    bool match = true;
                    for (size_t row = 0; row < aln_rows.size() && match; row++) {
                        if (row_starts[row] >= col && row_stops[row] <= col
                            && aln_rows[row].length() > col) {
                            string nt = aln_rows[row].substr(col - row_starts[row], 1);
                            if (NStr::Equal (nt, "-")) {
                                if (internal_gaps) {
                                    match = false;
                                }
                            } else {                    
                                column += nt;
                            }
                            any_data = true;
                        }
                    }
                    if (!any_data) {
                        match = false;
                    }
                    if (match) {
                        if (!NStr::IsBlank (column)) {
                            string::iterator it1 = column.begin();
                            string::iterator it2 = it1;
                            ++it2;
                            while (match && it2 != column.end()) {
                                if (!s_AmbiguousMatch (*it1, *it2)) {
                                    match = false;
                                }
                                ++it2;
                                if (it2 == column.end()) {
                                    ++it1;
                                    it2 = it1;
                                    ++it2;
                                }
                            }
                        }
                        if (match) {
                            ++num_match;
                        }
                    }
                    col++;
                }
            }
        } catch (CException &) {
            ids_missing = true;
        } catch (std::exception &) {
            ids_missing = true;
        }
    }

    if (ids_missing) {
        // if no columns, set col to one, so that we'll get a zero percent id error
        col = 1;
        num_match = 0;
    }

    if (col > 0) {
        size_t pct_id = (num_match * 100) / col;
        if (pct_id < 50) {
            PostErr (eDiag_Warning, eErr_SEQ_ALIGN_PercentIdentity,
                "PercentIdentity: This alignment has a percent identity of " + NStr::NumericToString (pct_id) + "%",
                     align);
        }
    }
}


void CValidError_align::x_ValidateDenseg
(const TDenseg& denseg,
 const CSeq_align& align)
{
    // assert dim >= 2
    x_ValidateDim(denseg, align);

    size_t dim     = denseg.GetDim();
    size_t numseg  = denseg.GetNumseg();
    string label;
    denseg.GetIds()[0]->GetLabel (&label);
 
 
    string context;
    size_t bar_pos = NStr::Find(label, "|");
    if ( bar_pos != string::npos ) {
        context = label.substr(bar_pos+1);
    } else {
        context = label;
    }


    // assert dim == Ids.size()
    if ( dim != denseg.GetIds().size() ) {
        PostErr(eDiag_Error, eErr_SEQ_ALIGN_AlignDimSeqIdNotMatch,
                "SeqId: The Seqalign has more or fewer ids than the number of rows in the alignment (context "
                    + context + ").  Look for possible formatting errors in the ids.", align);
    }

    // assert numseg == Lens.size()
    if ( numseg != denseg.GetLens().size() ) {
        PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsNumsegMismatch,
                "Mismatch between specified numseg (" +
                NStr::SizetToString(numseg) + 
                ") and number of Lens (" + 
                NStr::SizetToString(denseg.GetLens().size()) + ")",
                align);
    }

    // assert dim * numseg == Starts.size()
    if ( dim * numseg != denseg.GetStarts().size() ) {
        PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsStartsMismatch,
                "The number of Starts (" + 
                NStr::SizetToString(denseg.GetStarts().size()) +
                ") does not match the expected size of dim * numseg (" +
                NStr::SizetToString(dim * numseg) + ")", align);
    } 

    x_ValidateStrand(denseg, align);
    x_ValidateFastaLike(denseg, align);
    x_ValidateSegmentGap(denseg, align);
    
	// operations that require remote fetching
    if ( m_Imp.IsRemoteFetch() ) {
        x_ValidateSeqId(align);
        x_ValidateSeqLength(denseg, align);
    }
}




void CValidError_align::x_ValidatePacked
(const TPacked& packed,
 const CSeq_align& align)
{

    // assert dim >= 2
    x_ValidateDim(packed, align);

    size_t dim     = packed.GetDim();
    size_t numseg  = packed.GetNumseg();
    
    // assert dim == Ids.size()
    if ( dim != packed.GetIds().size() ) {
        PostErr(eDiag_Error, eErr_SEQ_ALIGN_AlignDimSeqIdNotMatch,
                "SeqId: The Seqalign has more or fewer ids than the number of rows in the alignment.  Look for possible formatting errors in the ids.", align);
    }
    
    // assert numseg == Lens.size()
    if ( numseg != packed.GetLens().size() ) {
        PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsDimMismatch,
            "Mismatch between specified numseg (" +
            NStr::SizetToString(numseg) + 
            ") and number of Lens (" + 
            NStr::SizetToString(packed.GetLens().size()) + ")",
            align);
    }
    
    x_ValidateSegmentGap(packed, align);
    
    if ( m_Imp.IsRemoteFetch() ) {
        x_ValidateSeqId(align);
        x_ValidateSeqLength(packed, align);
    }
}


void CValidError_align::x_ValidateDendiag
(const TDendiag& dendiags,
 const CSeq_align& align)
{
    size_t num_dendiag = 0;
    ITERATE( TDendiag, dendiag_iter, dendiags ) {
        ++num_dendiag;

        const CDense_diag& dendiag = **dendiag_iter;
        size_t dim = dendiag.GetDim();

        // assert dim >= 2
        x_ValidateDim(dendiag, align, num_dendiag);

        string label;
        dendiag.GetIds()[0]->GetLabel (&label);
        string context;
        size_t bar_pos = NStr::Find(label, "|");
        if ( bar_pos != string::npos ) {
            context = label.substr(bar_pos+1);
        } else {
            context = label;
        }

        // assert dim == Ids.size()
        if ( dim != dendiag.GetIds().size() ) {
            PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsDimSeqIdNotMatch,
                    "SeqId: In segment " + NStr::SizetToString (num_dendiag) 
                    + ", there are more or fewer rows than there are seqids (context "
                    + context + ").  Look for possible formatting errors in the ids.", align);
        }

        // assert dim == Starts.size()
        if ( dim != dendiag.GetStarts().size() ) {
            PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsDimMismatch,
                "Mismatch between specified dimension (" +
                NStr::SizetToString(dim) + 
                ") and number ofStarts (" + 
                NStr::SizetToString(dendiag.GetStarts().size()) + 
                ") in dendiag " + NStr::SizetToString(num_dendiag), align);
        } 

        // assert dim == Strands.size() (if exist)
        if ( dendiag.IsSetStrands() ) {
            if ( dim != dendiag.GetStrands().size() ) {
                PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsDimMismatch,
                    "Mismatch between specified dimension (" +
                    NStr::SizetToString(dim) + 
                    ") and number of Strands (" + 
                    NStr::SizetToString(dendiag.GetStrands().size()) + 
                    ") in dendiag " + NStr::SizetToString(num_dendiag), align);
            } 
        }

        if ( m_Imp.IsRemoteFetch() ) {
            x_ValidateSeqLength(dendiag, num_dendiag, align);
        }
    }
    if ( m_Imp.IsRemoteFetch() ) {
        x_ValidateSeqId(align);
    }
    x_ValidateSegmentGap (dendiags, align);
}


void CValidError_align::x_ValidateStd
(const TStd& std_segs,
 const CSeq_align& align)
{
    size_t num_stdseg = 0;
    ITERATE( TStd, stdseg_iter, std_segs) {
        ++num_stdseg;

        const CStd_seg& stdseg = **stdseg_iter;
        size_t dim = stdseg.GetDim();

        // assert dim >= 2
        x_ValidateDim(stdseg, align, num_stdseg);

        // assert dim == Loc.size()
        if ( dim != stdseg.GetLoc().size() ) {
            string label;
            stdseg.GetLoc()[0]->GetId()->GetLabel(&label);
            string context;
            size_t bar_pos = NStr::Find(label, "|");
            if ( bar_pos != string::npos ) {
                context = label.substr(bar_pos+1);
            } else {
                context = label;
            }
            PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsDimSeqIdNotMatch,
                    "SeqId: In segment " + NStr::SizetToString (num_stdseg) 
                    + ", there are more or fewer rows than there are seqids (context "
                    + context + ").  Look for possible formatting errors in the ids.", align);
        }

        // assert dim == Ids.size()
        if ( stdseg.IsSetIds() ) {
            if ( dim != stdseg.GetIds().size() ) {
                PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegsDimMismatch,
                    "Mismatch between specified dimension (" +
                    NStr::SizetToString(dim) + 
                    ") and number of Seq-ids (" + 
                    NStr::SizetToString(stdseg.GetIds().size()) + ")",
                    align);
            }
        }
    }

    x_ValidateStrand(std_segs, align);
    x_ValidateSegmentGap(std_segs, align);
    
    if ( m_Imp.IsRemoteFetch() ) {
        x_ValidateSeqId(align);
        x_ValidateSeqLength(std_segs, align);
    }
}


template <typename T>
bool CValidError_align::x_ValidateDim
(T& obj,
 const CSeq_align& align,
 size_t part)
{
    bool rval = false;

    if ( !obj.IsSetDim() || obj.GetDim() == 0) {
        if (part > 0) {
            PostErr (eDiag_Error, eErr_SEQ_ALIGN_SegsDimOne,
                     "Segs: Segment " + NStr::SizetToString (part) + "has dimension zero", align);
        } else {
            PostErr (eDiag_Error, eErr_SEQ_ALIGN_AlignDimOne,
                     "Dim: This alignment has dimension zero", align);
        }
    } else if (obj.GetDim() == 1) {
        string msg = "";
        EErrType et;
        if (part > 0) {
            et = eErr_SEQ_ALIGN_SegsDimOne;
            msg = "Segs: Segment " + NStr::SizetToString (part) + " apparently has only one sequence.  Each portion of the alignment must have at least two sequences.";
        } else {
            et = eErr_SEQ_ALIGN_AlignDimOne;
            msg = "Dim: This seqalign apparently has only one sequence.  Each alignment must have at least two sequences.";
        }
        CConstRef<CSeq_id> id = GetReportableSeqIdForAlignment(align, *m_Scope);
        if (id) {
            CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*id);
            if (bsh) {
                int version = 0;
                const string& label = GetAccessionFromObjects(bsh.GetCompleteBioseq(), NULL, *m_Scope, &version);
                msg += "  context " + label;
            }
        }
        PostErr (eDiag_Error, et, msg, align);
    } else {
        rval = true;
    }

    return rval;
}


//===========================================================================
// x_ValidateStrand:
//
//  Check if the  strand is consistent in SeqAlignment of global
//  or partial type.
//===========================================================================

void CValidError_align::x_ValidateStrand
(const TDenseg& denseg,
 const CSeq_align& align)
{
    if ( !denseg.IsSetStrands() ) {
        return;
    }
    
    size_t dim = denseg.GetDim();
    size_t numseg = denseg.GetNumseg();
    const CDense_seg::TStrands& strands = denseg.GetStrands();

    // go through id for each alignment sequence
    for ( size_t id = 0; id < dim; ++id ) {
        ENa_strand strand1 = strands[id];
        
        for ( size_t seg = 0; seg < numseg; ++seg ) {
            ENa_strand strand2 = strands[id + (seg * dim)];

            // skip undefined strand
            if ( strand2 == eNa_strand_unknown  ||
                 strand2 == eNa_strand_other ) {
                continue;
            }

            if ( strand1 == eNa_strand_unknown  ||
                 strand1 == eNa_strand_other ) {
                strand1 = strand2;
                continue;
            }

            // strands should be same for a given seq-id
            if ( strand1 != strand2 ) {
                string sid = "?";
                size_t pos = 0;
                if (denseg.GetIds().size() > id && denseg.GetIds()[id]) {
                    sid = denseg.GetIds()[id]->AsFastaString();
                }
                if (denseg.GetStarts().size() > id + (seg * dim)) {
                    pos = denseg.GetStarts()[id + (seg * dim)];
                }
                PostErr(eDiag_Error, eErr_SEQ_ALIGN_StrandRev,
                    "Strand: The strand labels for SeqId " + 
                    sid +
                    " are inconsistent across the alignment. "
                    "The first inconsistent region is the " + 
                    NStr::SizetToString(seg + 1) + "(th) region, near sequence position "
                    + NStr::SizetToString(pos), align);
                    break;
            }
        }
    }
}


void CValidError_align::x_ValidateStrand
(const TStd& std_segs,
 const CSeq_align& align)
{
    map< string, ENa_strand > strands;
    map< string, bool> reported;
    int region = 1;

    ITERATE ( TStd, stdseg, std_segs ) {
        ITERATE ( CStd_seg::TLoc, loc_iter, (*stdseg)->GetLoc() ) {
            const CSeq_loc& loc = **loc_iter;
            
            if ( !IsOneBioseq(loc, m_Scope) ) {
                // !!! should probably be an error
                continue;
            }
            CConstRef<CSeq_id> id(&GetId(loc, m_Scope));
            string id_label = id->AsFastaString();

            ENa_strand strand = GetStrand(loc, m_Scope);

            if ( strand == eNa_strand_unknown  || 
                 strand == eNa_strand_other ) {
                continue;
            }

            if ( strands[id_label] == eNa_strand_unknown  ||
                 strands[id_label] == eNa_strand_other ) {
                strands[id_label] = strand;
                reported[id_label] = false;
            } else if (!reported[id_label]
                && strands[id_label] != strand ) {
                TSeqPos start = loc.GetStart(eExtreme_Positional);
                PostErr(eDiag_Error, eErr_SEQ_ALIGN_StrandRev,
                    "Strand: The strand labels for SeqId " + id_label + 
                    " are inconsistent across the alignment.  The first inconsistent region is the "
                    + NStr::IntToString (region) + "(th) region, near sequence position "
                    + NStr::IntToString (start), align);
                reported[id_label] = true;
            }
        }
        region++;
    }
}


static size_t s_PercentBioseqMatch (CBioseq_Handle b1, CBioseq_Handle b2)
{
    size_t match = 0;
    size_t min_len = b1.GetInst().GetLength();
    if (b2.GetInst().GetLength() < min_len) {
        min_len = b2.GetInst().GetLength();
    }
    if (min_len == 0) {
        return 0;
    }
    if (b1.IsAa() && !b2.IsAa()) {
        return 0;
    } else if (!b1.IsAa() && b2.IsAa()) {
        return 0;
    }

    try {
        CSeqVector sv1 = b1.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        CSeqVector sv2 = b2.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        for ( CSeqVector_CI sv1_iter(sv1), sv2_iter(sv2); (sv1_iter) && (sv2_iter); ++sv1_iter, ++sv2_iter ) {
            if (*sv1_iter == *sv2_iter || *sv1_iter == 'N' || *sv2_iter == 'N') {
                match++;
            }
        }

        match = (match * 100) / min_len;

    } catch (CException& ) {
        match = 0;
    }
    return match;
}


//===========================================================================
// x_ValidateFastaLike:
//
//  Check if an alignment is FASTA-like. 
//  Alignment is FASTA-like if all gaps are at the end with dimensions > 2.
//===========================================================================

void CValidError_align::x_ValidateFastaLike
(const TDenseg& denseg,
 const CSeq_align& align)
{
    // check only global or partial type
    if ( (align.GetType() != CSeq_align::eType_global  &&
         align.GetType() != CSeq_align::eType_partial) ||
         denseg.GetDim() <= 2 ) {
        return;
    }

    size_t dim = denseg.GetDim();
    size_t numseg = denseg.GetNumseg();

    vector<string> fasta_like;

    for ( int id = 0; id < s_GetNumIdsToUse(denseg); ++id ) {
        bool gap = false;
        
        const CDense_seg::TStarts& starts = denseg.GetStarts();
        for ( size_t seg = 0; seg < numseg; ++ seg ) {
            // if start value is -1, set gap flag to true
            if ( starts[id + (dim * seg)] < 0 ) {
                gap = true;
            } else if ( gap  ) {
                // if a positive start value is found after the initial -1 
                // start value, it's not fasta like. 
                //no need to check this sequence further
                return;
            } 

            if ( seg == numseg - 1) {
                // if no more positive start value are found after the initial
                // -1 start value, it's fasta like
                fasta_like.push_back(denseg.GetIds()[id]->AsFastaString());
            }
        }
    }

    if ( !fasta_like.empty() ) {
        CDense_seg::TIds::const_iterator id_it = denseg.GetIds().begin();
        string context = (*id_it)->GetSeqIdString();
        CBioseq_Handle master_seq = m_Scope->GetBioseqHandle(**id_it);
        bool is_fasta_like = false;
        if (master_seq) {
            ++id_it;
            while (id_it != denseg.GetIds().end() && !is_fasta_like) {
                CBioseq_Handle seq = m_Scope->GetBioseqHandle(**id_it);
                if (!seq || s_PercentBioseqMatch (master_seq, seq) < 50) {
                    is_fasta_like = true;
                }
                ++id_it;
            }
        } else {
            is_fasta_like = true;
        }
        if (is_fasta_like) {
            PostErr(eDiag_Warning, eErr_SEQ_ALIGN_FastaLike,
                    "Fasta: This may be a fasta-like alignment for SeqId: "
                    + fasta_like.front() + " in the context of " + context, align);
        }                    
    }

}           




CValidError_align::TSegmentGapV CValidError_align::FindSegmentGaps(const TDenseg& denseg, CScope* scope)
{
    TSegmentGapV seggaps;
    size_t align_pos = 0;

    int numseg = denseg.GetNumseg();
    int dim = denseg.GetDim();
    const CDense_seg::TStarts& starts = denseg.GetStarts();

    for (size_t seg = 0; seg < numseg; ++seg) {
        bool seggap = true;
        for (int id = 0; id < dim && seg * dim + id < starts.size(); ++id) {
            if (starts[seg * dim + id] != -1) {
                seggap = false;
                break;
            }
        }
        if (seggap) {
            // no sequence is present in this segment
            string label = "";
            if (denseg.IsSetIds() && denseg.GetIds().size() > 0) {
                denseg.GetIds()[0]->GetLabel(&label, CSeq_id::eContent);
            }
            if (NStr::IsBlank(label)) {
                label = "unknown";
            }
            seggaps.push_back(TSegmentGapV::value_type(seg, align_pos, label));
        }
        if (denseg.IsSetLens() && denseg.GetLens().size() > (unsigned int)seg) {
            align_pos += denseg.GetLens()[seg];
        }
    }
    return seggaps;
}

void CValidError_align::x_ReportSegmentGaps(const TSegmentGapV& seggaps, const CSeq_align& align)
{
    ITERATE(TSegmentGapV, itr, seggaps) {
        // no sequence is present in this segment
        PostErr(eDiag_Error, eErr_SEQ_ALIGN_SegmentGap,
            "Segs: Segment " + NStr::SizetToString(itr->seg_num + 1) + " (near alignment position "
            + NStr::SizetToString(itr->align_pos) + ") in the context of "
            + itr->label + " contains only gaps.  Each segment must contain at least one actual sequence -- look for columns with all gaps and delete them.",
            align);
    }
}


//===========================================================================
// x_ValidateSegmentGap:
//
// Check if there is a gap for all sequences in a segment.
//===========================================================================

void CValidError_align::x_ValidateSegmentGap
(const TDenseg& denseg,
 const CSeq_align& align)
{
    TSegmentGapV seggaps = FindSegmentGaps(denseg, m_Scope);
    x_ReportSegmentGaps(seggaps, align);
}


CValidError_align::TSegmentGapV CValidError_align::FindSegmentGaps(const TPacked& packed, CScope* scope)
{
    TSegmentGapV seggaps;

    static Uchar bits[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    size_t numseg = packed.GetNumseg();
    size_t dim = packed.GetDim();
    const CPacked_seg::TPresent& present = packed.GetPresent();

    size_t align_pos = 0;
    for (size_t seg = 0; seg < numseg; ++seg) {
        size_t id = 0;
        for (; id < dim; ++id) {
            size_t i = id + (dim * seg);
            if ((present[i / 8] & bits[i % 8])) {
                break;
            }
        }
        if (id == dim) {
            // no sequence is present in this segment
            string label = "";
            if (packed.IsSetIds() && packed.GetIds().size() > 0) {
                packed.GetIds()[0]->GetLabel(&label, CSeq_id::eContent);
            }
            if (NStr::IsBlank(label)) {
                label = "Unknown";
            }
            seggaps.push_back(TSegmentGapV::value_type(seg, align_pos, label));
        }
        if (packed.IsSetLens() && packed.GetLens().size() > seg) {
            align_pos += packed.GetLens()[seg];
        }
    }

    return seggaps;
}


void CValidError_align::x_ValidateSegmentGap
(const TPacked& packed,
 const CSeq_align& align)
{
    TSegmentGapV seggaps = FindSegmentGaps(packed, m_Scope);
    x_ReportSegmentGaps(seggaps, align);
}


CValidError_align::TSegmentGapV CValidError_align::FindSegmentGaps(const TStd& std_segs, CScope* scope)
{
    TSegmentGapV seggaps;

    size_t seg = 0;
    size_t align_pos = 0;
    ITERATE(TStd, stdseg, std_segs) {
        bool gap = true;
        size_t len = 0;
        string label = "";
        ITERATE(CStd_seg::TLoc, loc, (*stdseg)->GetLoc()) {
            if (!(*loc)->IsEmpty() && !(*loc)->IsNull()) {
                gap = false;
                break;
            } else if (len == 0) {
                len = GetLength(**loc, scope);
                if (NStr::IsBlank(label)) {
                    (*loc)->GetId()->GetLabel(&label, CSeq_id::eContent);
                }
            }
        }
        if (gap) {
            if (NStr::IsBlank(label)) {
                label = "Unknown";
            }
            seggaps.push_back(TSegmentGapV::value_type(seg, align_pos, label));
        }
        align_pos += len;
        ++seg;
    }
    return seggaps;
}


void CValidError_align::x_ValidateSegmentGap
(const TStd& std_segs,
 const CSeq_align& align)
{
    TSegmentGapV seggaps = FindSegmentGaps(std_segs, m_Scope);
    x_ReportSegmentGaps(seggaps, align);
}


CValidError_align::TSegmentGapV CValidError_align::FindSegmentGaps(const TDendiag& dendiags, CScope* scope)
{
    TSegmentGapV seggaps;

    size_t seg = 0;
    TSeqPos align_pos = 1;
    ITERATE(TDendiag, diag_seg, dendiags) {
        if (!(*diag_seg)->IsSetDim() || (*diag_seg)->GetDim() == 0) {
            string label = "";
            if ((*diag_seg)->IsSetIds() && (*diag_seg)->GetIds().size() > 0) {
                (*diag_seg)->GetIds().front()->GetLabel(&label);
            }
            if (NStr::IsBlank(label)){
                label = "Unknown";
            }
            seggaps.push_back(TSegmentGapV::value_type(seg, align_pos, label));
        }
        if ((*diag_seg)->IsSetLen()) {
            align_pos += (*diag_seg)->GetLen();
        }
        ++seg;
    }

    return seggaps;
}


void CValidError_align::x_ValidateSegmentGap
(const TDendiag& dendiags,
 const CSeq_align& align)
{
    TSegmentGapV seggaps = FindSegmentGaps(dendiags, m_Scope);
    x_ReportSegmentGaps(seggaps, align);
}


//===========================================================================
// x_ValidateSeqIdInSeqAlign:
//
//  Validate SeqId in sequence alignment.
//===========================================================================

void CValidError_align::x_ValidateSeqId(const CSeq_align& align)
{
    vector< CRef< CSeq_id > > ids;
    x_GetIds(align, ids);

    int num_nucs = 0;
    int num_prts = 0;

    ITERATE( vector< CRef< CSeq_id > >, id_iter, ids ) {
        const CSeq_id& id = **id_iter;
        if ( id.IsLocal() ) {
            CBioseq_Handle hdl = m_Scope->GetBioseqHandle(id);
            if ( hdl ) {
                if ( hdl.IsNa() ) {
                    num_nucs++;
                }
                if ( hdl.IsAa() ) {
                    num_prts++;
                }
            }
            if ( !hdl ) {
                PostErr(eDiag_Error, eErr_SEQ_ALIGN_SeqIdProblem,
                    "SeqId: The sequence corresponding to SeqId " + 
                    id.AsFastaString() + " could not be found.",
                    align);
            }
        } else {
            CBioseq_Handle hdl = m_Scope->GetBioseqHandle(id);
            if ( hdl ) {
                if ( hdl.IsNa() ) {
                    num_nucs++;
                }
                if ( hdl.IsAa() ) {
                    num_prts++;
                }
            }
        }
    }
    if ( num_nucs > 0 && num_prts > 0 ) {
        PostErr(eDiag_Error, eErr_SEQ_ALIGN_NucProtMixture,
                "MIXTURE OF NUCS AND PROTS", align);
    }
}


void CValidError_align::x_GetIds
(const CSeq_align& align,
 vector< CRef< CSeq_id > >& ids)
{
    ids.clear();

    switch ( align.GetSegs().Which() ) {

    case CSeq_align::C_Segs::e_Dendiag:
        ITERATE( TDendiag, diag_seg, align.GetSegs().GetDendiag() ) {
            const vector< CRef< CSeq_id > >& diag_ids = (*diag_seg)->GetIds();
            copy(diag_ids.begin(), diag_ids.end(), back_inserter(ids));
        }
        break;
        
    case CSeq_align::C_Segs::e_Denseg:
        ids = align.GetSegs().GetDenseg().GetIds();
        break;
        
    case CSeq_align::C_Segs::e_Packed:
        copy(align.GetSegs().GetPacked().GetIds().begin(),
             align.GetSegs().GetPacked().GetIds().end(),
             back_inserter(ids));
        break;
        
    case CSeq_align::C_Segs::e_Std:
        ITERATE( TStd, std_seg, align.GetSegs().GetStd() ) {
            ITERATE( CStd_seg::TLoc, loc, (*std_seg)->GetLoc() ) {
                CSeq_id* idp = const_cast<CSeq_id*>(&GetId(**loc, m_Scope));
                CRef<CSeq_id> ref(idp);
                ids.push_back(ref);
            }
        }
        break;
            
    default:
        break;
    }
}


string s_DescribeSegment(const CSeq_id& id, const CSeq_id& id_context, size_t segment, size_t pos, bool use_in = false)
{
    string label;
    id.GetLabel(&label);
    string context;
    id_context.GetLabel(&context, CSeq_id::eContent);

    string seg_string = "sequence " + label + "," + (use_in ? " in " : " ") +
        "segment " + NStr::NumericToString(segment) + 
        " (near sequence position " + NStr::NumericToString(pos) +
        ")" + (use_in ? ", " : " ") + "context " + context;
    return seg_string;
}


void CValidError_align::x_ReportAlignErr
(const CSeq_align& align,
const CSeq_id& id,
const CSeq_id& id_context,
size_t segment,
size_t pos,
EErrType et,
EDiagSev sev,
const string& prefix,
const string& message)
{
    PostErr(sev, et, prefix + ": In " + s_DescribeSegment(id, id_context, segment, pos) + ", " + message, align);
}

static const string kAlignmentTooLong = "the alignment claims to contain residue coordinates that are past the end of the sequence.  Either the sequence is too short, or there are extra characters or formatting errors in the alignment";

void CValidError_align::x_ReportSumLenStart
(const CSeq_align& align,
 const CSeq_id& id,
 const CSeq_id& id_context,
 size_t segment,
 size_t pos)
{
    x_ReportAlignErr(align, id, id_context, segment, pos,
        eErr_SEQ_ALIGN_SumLenStart, eDiag_Error,
        "Start", kAlignmentTooLong);
}


void CValidError_align::x_ReportStartMoreThanBiolen
(const CSeq_align& align,
const CSeq_id& id,
const CSeq_id& id_context,
size_t segment,
size_t pos)
{
    x_ReportAlignErr(align, id, id_context, segment, pos,
        eErr_SEQ_ALIGN_StartMorethanBiolen, eDiag_Error,
        "Start", kAlignmentTooLong);
}


//===========================================================================
// x_ValidateSeqLength:
//
//  Check segment length, start and end point in Dense_diag, Dense_seg,  
//  Packed_seg and Std_seg.
//===========================================================================

// Make sure that, in Dense_diag alignment, segment length is not greater
// than Bioseq length
void CValidError_align::x_ValidateSeqLength
(const CDense_diag& dendiag,
 size_t dendiag_num,
 const CSeq_align& align)
{
    size_t dim = dendiag.GetDim();
    TSeqPos len = dendiag.GetLen();
    const CDense_diag::TIds& ids = dendiag.GetIds();
    
    const CSeq_id& context_id = *(ids[0]);
    CDense_diag::TStarts::const_iterator starts_iter = 
            dendiag.GetStarts().begin();
    
    for ( size_t id = 0; id < dim; ++id ) {
        TSeqPos bslen = GetLength(*(ids[id]), m_Scope);
        TSeqPos start = *starts_iter;

        const CSeq_id& seq_id = *(ids[id]);

        // verify start
        if ( start >= bslen ) {
            x_ReportStartMoreThanBiolen(align, seq_id, context_id, 1, start);
        }
        
        // verify length
        if ( start + len > bslen ) {
            x_ReportSumLenStart(align, seq_id, context_id, 1, start);
        }
        ++starts_iter;
    }
}


        

void CValidError_align::x_ValidateSeqLength
(const TDenseg& denseg,
 const CSeq_align& align)
{
    int dim     = denseg.GetDim();
    size_t numseg  = denseg.GetNumseg();
    const CDense_seg::TIds& ids       = denseg.GetIds();
    const CDense_seg::TStarts& starts = denseg.GetStarts();
    const CDense_seg::TLens& lens      = denseg.GetLens();
    bool minus = false;
    const CSeq_id& id_context = *ids[0];

    if (numseg > lens.size()) {
        numseg = lens.size();
    }

    for ( int id = 0; id < ids.size(); ++id ) {
        TSeqPos bslen = GetLength(*(ids[id]), m_Scope);
        minus = denseg.IsSetStrands()  &&
            denseg.GetStrands()[id] == eNa_strand_minus;

        for ( int seg = 0; seg < numseg; ++seg ) {
            size_t curr_index = 
                id + (minus ? numseg - seg - 1 : seg) * dim;
            if (curr_index >= starts.size()) {
                break;
            }
            // no need to verify if segment is not present
            if ( starts[curr_index] == -1 ) {
                continue;
            }
            size_t lens_index = minus ? numseg - seg - 1 : seg;

            // verify that start plus segment does not exceed total bioseq len
            if ( starts[curr_index] + lens[lens_index] > bslen ) {
                x_ReportSumLenStart(align, *(ids[id]), id_context, seg + 1, starts[curr_index]);
            }

            // find the next segment that is present
            size_t next_index = curr_index;
            int next_seg;
            for ( next_seg = seg + 1; next_seg < numseg; ++next_seg ) {
                next_index = 
                    id + (minus ? numseg - next_seg - 1 : next_seg) * dim;
                
                if ( starts[next_index] != -1 ) {
                    break;
                }
            }
            if ( next_seg == numseg  ||  next_index == curr_index ) {
                continue;
            }

            // length plus start should be equal to the closest next 
            // start that is not -1
            if ( starts[curr_index] + (TSignedSeqPos)lens[lens_index] !=
                starts[next_index] ) {
                PostErr(eDiag_Error, eErr_SEQ_ALIGN_DensegLenStart,
                        "Start/Length: There is a problem with " +
                        s_DescribeSegment(*(ids[id]), id_context, seg + 1, starts[curr_index], true) +
                        ": the segment is too long or short or the next segment has an incorrect start position", align);
            }
        }
    }
}


void CValidError_align::x_ValidateSeqLength
(const TPacked& packed,
 const CSeq_align& align)
{
    static Uchar bits[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    if (!packed.IsSetDim() || !packed.IsSetIds() || !packed.IsSetPresent() || !packed.IsSetNumseg()) {
        return;
    }

    size_t dim = packed.GetDim();
    size_t numseg = packed.GetNumseg();

    const CPacked_seg::TPresent& present = packed.GetPresent();
    CPacked_seg::TIds::const_iterator id_it = packed.GetIds().begin();
    const CSeq_id& id_context = **id_it;

    for ( size_t id = 0; id < dim && id_it != packed.GetIds().end(); ++id, ++id_it ) {
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle (**id_it);
        if (bsh) {
            string label;
            (*id_it)->GetLabel (&label);
            TSeqPos seg_start = packed.GetStarts()[id];
            if (seg_start >= bsh.GetBioseqLength()) {
                x_ReportStartMoreThanBiolen(align, **id_it, id_context, 1, seg_start);
            }
            for ( size_t seg = 0; seg < numseg; ++seg ) {
                size_t i = id + seg * dim;
                if ( i/8 < present.size() && (present[i / 8] & bits[i % 8]) ) {
                    seg_start += packed.GetLens()[seg];
                    if (seg_start > bsh.GetBioseqLength()) {   
                        x_ReportSumLenStart(align, **id_it, id_context, seg + 1, seg_start);
                    }
                }
            }
        }
    }
}


void CValidError_align::x_ValidateSeqLength
(const TStd& std_segs,
 const CSeq_align& align)
{
    int seg = 1;
    ITERATE( TStd, iter, std_segs ) {
        const CStd_seg& stdseg = **iter;
        const CSeq_id& id_context = *(stdseg.GetLoc().front()->GetId());

        ITERATE ( CStd_seg::TLoc, loc_iter, stdseg.GetLoc() ) {
            const CSeq_loc& loc = **loc_iter;
    
            if (loc.IsEmpty()) {
                CRef<CSeq_id> id(new CSeq_id());
                if (stdseg.IsSetIds() && stdseg.GetIds().size() >= seg) {
                    id->Assign(*(stdseg.GetIds()[seg - 1]));
                } else {
                    id->SetLocal().SetStr("?");
                }
                x_ReportAlignErr(align, *id, id_context, seg, 0,
                    eErr_SEQ_ALIGN_EndLessthanZero, eDiag_Error,
                    "Length", "End point is less than zero in segment");
                x_ReportAlignErr(align, *id, id_context, seg, 0,
                    eErr_SEQ_ALIGN_StartLessthanZero, eDiag_Error,
                    "Start", "Start point is less than zero");
            }
            if ( loc.IsWhole()  || loc.IsEmpty()  ||  loc.IsNull() ) {
                continue;
            }

            if ( !IsOneBioseq(loc, m_Scope) ) {
                continue;
            }

            TSeqPos from = loc.GetTotalRange().GetFrom();
            TSeqPos to   = loc.GetTotalRange().GetTo();
            TSeqPos loclen = GetLength( loc, m_Scope);
            TSeqPos bslen = GetLength(GetId(loc, m_Scope), m_Scope);
            string  bslen_str = NStr::UIntToString(bslen);

            const CSeq_id& id = *(loc.GetId());

            if ( from > bslen - 1 ) { 
                x_ReportStartMoreThanBiolen(align, id, id_context, seg, from);
            }

            if ( to > bslen - 1 ) { 
                x_ReportAlignErr(align, id, id_context, seg, from,
                    eErr_SEQ_ALIGN_EndMorethanBiolen, eDiag_Error,
                    "Length", kAlignmentTooLong);
            }

            if ( loclen > bslen ) {
                x_ReportAlignErr(align, id, id_context, seg, from,
                    eErr_SEQ_ALIGN_LenMorethanBiolen, eDiag_Error,
                    "Length", kAlignmentTooLong);
            }
        }
        seg++;
    }
}


END_SCOPE(validator)
END_SCOPE(objects)
END_NCBI_SCOPE
