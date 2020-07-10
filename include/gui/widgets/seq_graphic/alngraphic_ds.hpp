#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALNGRAPHIC_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALNGRAPHIC_DS__HPP

/*  $Id: alngraphic_ds.hpp 43001 2019-05-03 19:55:12Z shkeda $
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
* Authors:  Liangshou Wu
*
* File Description:
*       Borrowed from alnmulti_ds
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objtools/alnmgr/aln_explorer.hpp>
#include <objtools/alnmgr/alnmap.hpp>
#include <util/sequtil/sequtil_manip.hpp>
#include <util/range_coll.hpp>
#include <gui/widgets/aln_score/scoring_method.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// IAlnGraphicDataSource - interface to a data source representing an abstract
/// alignment used in graphical view.
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IAlnGraphicDataSource
    : public CObjectEx
    , public IScoringAlignment
{
public:
    /// Cetantiy of having unaligned polyA tail.
    enum EPolyATail {
        ePolyA_Yes,
        ePolyA_No,
        ePolyA_Unknown
    };

    enum ECigarOp 
    {
        eCigar_M = 'M',
        eCigar_I = 'I',
        eCigar_D = 'D',
        eCigar_N = 'N',
        eCigar_S = 'S',
        eCigar_H = 'H',
        eCigar_P = 'P',
        eCigar_Eq = '=',
        eCigar_X = 'X'
    };

    typedef pair<ECigarOp, size_t> TCigarPair;
    typedef vector<TCigarPair>     TCigar;

    typedef IAlnExplorer::TSignedRange TSignedRange;
    typedef IAlnExplorer::TNumrow    TNumrow;
    typedef IAlnExplorer::TResidue   TResidue;
    typedef IAlnExplorer::ESearchDirection   TSearchDirection;
    typedef CRangeCollection<TSeqPos>   TRangeColl;
    virtual IAlnExplorer::EAlignType  GetAlignType() const
    {
        if (GetNumRows() < 2)
            return IAlnExplorer::fInvalid;
        if (HasCigar()) 
            return IAlnExplorer::fDNA;
        objects::CBioseq_Handle bsh0;
        try {
            bsh0 = GetBioseqHandle(0);
        } catch (objects::CAlnException& e) {
            auto e_code = e.GetErrCode();
            if (e_code == objects::CAlnException::eInvalidRequest || e_code == objects::CAlnException::eInvalidSeqId)
                return IAlnExplorer::fInvalid;
            ERR_POST(Error << e.GetMsg());
        }
        if (!bsh0)
            return IAlnExplorer::fInvalid;
        // not sure if this is the best solution
        IAlnExplorer::EAlignType type = IAlnExplorer::fMixed;
        switch (bsh0.GetBioseqCore()->GetInst().GetMol())
        {
        case objects::CSeq_inst::eMol_dna:
        case objects::CSeq_inst::eMol_rna:
        case objects::CSeq_inst::eMol_na:
            type = IAlnExplorer::fDNA;
            break;

        case objects::CSeq_inst::eMol_aa:
            type = IAlnExplorer::fProtein;
            break;

        default:
            break;
        }

        if (type == IAlnExplorer::fMixed) {
            return type;
        }

        TNumrow row = 1;
        for (;  row < GetNumRows();  ++row) {
            objects::CBioseq_Handle bsh;
            try {
                bsh = GetBioseqHandle(row);
            } catch (objects::CAlnException&) {
                // Sequence ID without sequences loaded into the scope
            }
            if (!bsh)
                continue;
            IAlnExplorer::EAlignType this_type = IAlnExplorer::fMixed;
            switch (bsh.GetBioseqCore()->GetInst().GetMol())
            {
            case objects::CSeq_inst::eMol_dna:
            case objects::CSeq_inst::eMol_rna:
            case objects::CSeq_inst::eMol_na:
                this_type = IAlnExplorer::fDNA;
                break;

            case objects::CSeq_inst::eMol_aa:
                this_type = IAlnExplorer::fProtein;
                break;

            default:
                break;
            }
            if (this_type != type) {
                return IAlnExplorer::fMixed;
            }
        }

        return type;        
    }

    virtual TSeqPos GetSeqStart(TNumrow row) const = 0;
    virtual TSeqPos GetSeqStop(TNumrow row) const = 0;

    virtual TSignedRange GetAlnRange(void) const = 0;
    virtual TSignedRange GetSeqAlnRange(TNumrow row) const = 0;

    virtual TSeqPos GetSeqLength(TNumrow row) const = 0;

    virtual const objects::CSeq_id&  GetSeqId(TNumrow row) const = 0;
    virtual bool    IsPositiveStrand(TNumrow row) const = 0;
    virtual bool    IsNegativeStrand(TNumrow row) const = 0;

    virtual TSignedSeqPos  GetAlnPosFromSeqPos(TNumrow row,
        TSeqPos seq_pos,
        IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
        bool try_reverse_dir = true) const = 0;

    /// Get sequence string for a given row in sequence range.
    /// The method can optionally return the sequence string in 
    /// anchor sequence's direction (if anchor row and selected
    /// row have different strands.
    virtual string&  GetSeqString(string &buffer, TNumrow row,
        const TSignedRange& seq_rng,
        const TSignedRange& aln_rng,
        bool anchor_direct) const
    {
        auto& bsh = GetBioseqHandle(row);
        if (!bsh)
            return buffer;
        objects::CSeqVector seq_vec =
            bsh.GetSeqVector(objects::CBioseq_Handle::eCoding_Iupac);

        TSignedRange rng{ seq_rng };
        if (rng.GetFrom() > rng.GetTo()) {
            rng.SetFrom(seq_rng.GetTo());
            rng.SetTo(seq_rng.GetFrom());
        }

        seq_vec.GetSeqData(rng.GetFrom(), rng.GetToOpen(), buffer);
        TNumrow anchor = GetAnchor();
        if (anchor_direct  &&  row != anchor) {
            bool reversed = IsPositiveStrand(anchor) == IsPositiveStrand(row);
            if (reversed) {
                string tmp_seq;
                if (GetBaseWidth(row) == 3) {
                    // protein sequence, just reverse it
                    CSeqManip::Reverse(buffer, CSeqUtil::e_Iupacaa,
                        0, buffer.length(), tmp_seq);
                } else {
                    // nucleotide sequence, get reverse and complement
                    CSeqManip::ReverseComplement(buffer, CSeqUtil::e_Iupacna,
                        0, buffer.length(), tmp_seq);
                }
                swap(buffer, tmp_seq);
            }
        }
        return buffer;
    }

    /// Convert alignment (genomic) coordinate on the selected row to real sequence position.
    virtual TSignedSeqPos AlnPosToNativeSeqPos(TNumrow row, TSignedSeqPos aln_pos) const
    {
        return aln_pos / GetBaseWidth(row);
    }
    
    /// Convert sequence position to alignment (genomic) coordinate.
    virtual TSignedSeqPos NativeSeqPosToAlnPos(TNumrow row, TSignedSeqPos seq_pos, int frame = 0) const
    {
        int w = GetBaseWidth(row);
        TSignedSeqPos ret = seq_pos*w;
        if (w == 3 && frame) ret += frame - 1;
            return ret;
    }

    virtual TSignedSeqPos GetSeqPosFromSeqPos(TNumrow for_row,
                                              TNumrow row,
                                              TSeqPos seq_pos) const = 0;

    virtual IAlnExplorer::TNumrow GetQuery() const
    {
        IAlnExplorer::TNumrow anchor = GetAnchor();
        IAlnExplorer::TNumrow query = anchor == 0 ? 1 : 0;
        if (2 == GetNumRows())
            return query;

        const objects::CSeq_id &anchor_id = GetSeqId(anchor);
        for (query = 0; query < GetNumRows(); ++query) {
            const objects::CSeq_id &row_id = GetSeqId(query);
            if (objects::CSeq_id::e_YES == row_id.Compare(anchor_id))
                continue;
            break;
        }
        
        return query;
    }

    virtual void GetUnalignedTails(TSignedSeqPos &start_tail, TSignedSeqPos &end_tail, bool clipped = true) const
    {
        start_tail = 0;
        end_tail = 0;
        do {
            try {
                int anchor = GetAnchor();
                int aligned_seq = anchor == 0 ? 1 : 0;
                const objects::CBioseq_Handle& bsh_seq = GetBioseqHandle(aligned_seq);
                if (!bsh_seq)
                    break;
                const objects::CBioseq_Handle& bsh_anchor = GetBioseqHandle(anchor);
                if (!bsh_anchor)
                    break;
                TSeqPos seq_len = bsh_seq.GetBioseqLength();
                TSeqPos aln_len = bsh_anchor.GetBioseqLength();

                TSeqPos base_width_curr = GetBaseWidth(aligned_seq);
                TSeqPos base_width_anchor = GetBaseWidth(anchor);
                TSeqPos aln_start = GetSeqStart(anchor) / base_width_anchor;
                TSeqPos aln_end = GetSeqStop(anchor) / base_width_anchor;
                TSeqPos seq_start = GetSeqStart(aligned_seq) / base_width_curr;
                TSeqPos seq_end = GetSeqStop(aligned_seq) / base_width_curr;
                bool reverse_strand = IsNegativeStrand(0) != IsNegativeStrand(1);

                TSeqPos start_tail_len = reverse_strand ? seq_len - seq_end - 1 : seq_start;
                TSeqPos end_tail_len = reverse_strand ? seq_start : seq_len - seq_end - 1;

                TSeqPos aln_5_len = aln_start;
                TSeqPos aln_3_len = aln_len - aln_end - 1;

                if (clipped) {
                    start_tail = start_tail_len < aln_5_len ? start_tail_len : aln_5_len;
                    end_tail = end_tail_len < aln_3_len ? end_tail_len : aln_3_len;
                    // Apply the length limit
                    if (start_tail > m_kUnalignedTailLimit)
                        start_tail = m_kUnalignedTailLimit;
                    if (end_tail > m_kUnalignedTailLimit)
                        end_tail = m_kUnalignedTailLimit;
                }
                else {
                    start_tail = start_tail_len;
                    end_tail = end_tail_len;
                }
            }
            catch (const objects::CAlnException& error) {
                auto error_code = error.GetErrCode();
                if (error_code == objects::CAlnException::eInvalidRequest || error_code == objects::CAlnException::eInvalidSeqId)
                    break;
                LOG_POST(Warning << error);
            }
            catch (const CException &error) {
                LOG_POST(Warning << error);
            }
        } while (false);
    }


    const TSignedSeqPos GetUnalignedTailLimit() const
    {
        return m_kUnalignedTailLimit;
    }

    virtual TSeqRange GetAlnRangeWithTails() const
    {
        int anchor = GetAnchor();
        int aligned_seq = anchor == 0 ? 1 : 0;
        TSignedSeqPos start_tail = 0, end_tail = 0;
        GetUnalignedTails(start_tail, end_tail);
        TSeqPos base_width_curr = GetBaseWidth(aligned_seq);
        TSeqPos base_width_anchor = GetBaseWidth(anchor);
        TSeqPos aln_start = GetSeqStart(anchor) / base_width_anchor;
        TSeqPos aln_end = GetSeqStop(anchor) / base_width_anchor;

        return TSeqRange(aln_start - start_tail*base_width_curr, aln_end + end_tail*base_width_curr);
    }


    /// Check if there is any unaligned polyA tail.
    /// By default, it returns unknown to ask for further verification.
    virtual EPolyATail HasPolyATail() const = 0;

    /// Check if there is a CIGAR string.
    virtual bool HasCigar() const = 0;

    /// Check if cigar doesn't have ambiguous M's
    virtual bool IsCigarUnambiguous() const {
        return false;
    }

    /// Get CIGAR vector.
    virtual const TCigar* GetCigar() const = 0;

    virtual bool IsSimplified() const 
    { return false; }

    virtual bool IsRegular() const 
    { return true; }

    virtual const TCigarPair* GetCigarAtAlnPos(int row, TSignedSeqPos pos) const
    {
        if (!HasCigar())
            return nullptr;
        
        const TCigar &cigar = *GetCigar();
        TCigar::const_iterator cig_it = cigar.begin();

        auto_ptr<IAlnSegmentIterator> p_it(CreateSegmentIterator(row, GetAlnRange(), IAlnSegmentIterator::eAllSegments));
        for (IAlnSegmentIterator& seg_it = *p_it; seg_it && (cig_it != cigar.end()); ++seg_it, ++cig_it) {
            const IAlnSegment::TSignedRange& aln_r = seg_it->GetAlnRange();
            if ((pos >= aln_r.GetFrom()) && (pos <= aln_r.GetTo()))
                return &(*cig_it);
        }
        return nullptr;
    }

protected:
    /// Unaligned tail length limit
    const TSignedSeqPos m_kUnalignedTailLimit = 200;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALNGRAPHIC_DS__HPP
