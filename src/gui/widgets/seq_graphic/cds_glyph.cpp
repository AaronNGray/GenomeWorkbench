/*  $Id: cds_glyph.cpp 44783 2020-03-12 19:40:03Z shkeda $
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
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/cds_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objmgr/util/sequence.hpp>
#include <util/sequtil/sequtil_manip.hpp>
#include <serial/iterator.hpp>
#include <gui/widgets/seq_graphic/alngraphic_ds.hpp>
BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kVertSpace = 1;


CCdsGlyph::CCdsGlyph(const CMappedFeat& f)
    : CFeatGlyph(f)
{
}

CCdsGlyph::CCdsGlyph(const CMappedFeat& f, const CSeq_loc& loc)
    : CFeatGlyph(f, loc)
{
}

void CCdsGlyph::x_Draw() const
{
    CFeatGlyph::x_Draw();
    if (!x_WillFit())
        return;

    IRender& gl = GetGl();

    TModelUnit trans_full = gl.GetMetric(&(m_CdsConfig->m_TransFont),
        IGlFont::eMetric_FullCharHeight);
    TModelUnit prot_h = gl.TextHeight(&(m_CdsConfig->m_ProdFont));

    // Translated protein sequence Y position (at the bottom)
    TModelUnit trans_y = GetHeight() + GetTop();
    // Genomic sequence Y postion (between the translated and original)
    TModelUnit gen_y = trans_y - prot_h - kVertSpace;
    // Original protein sequence Y position (inside the bar)
    TModelUnit prot_y = gen_y - trans_full;
    // adjust to CDS feature bar center line vertically
    prot_y -= (m_Config->GetBarHeight(false) - prot_h) * 0.5;

    if (x_IsProjected()) {
        x_DrawProjectedProtSeq(trans_y, gen_y, prot_y);
        return;
    }

    if ( !GetMappingInfo().empty() ) {
        x_DrawProtSeqWithMapping(trans_y, gen_y, prot_y);
        return;
    }

    // Nov-28-2014
    // We generate mapping always now, so there should be no cases
    // when control is here.

    // otherwise, draw protein sequence based on feature location
    bool flip_strand = m_Context->IsFlippedStrand();
    string prot_seq, trans_seq;
    TSeqPos offset = x_GetProtOffset(); // initial translation offset
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    const CSeqVector& vec = seq_ds->GetSeqVector();
    CSeqTranslator::Translate(GetFeature(),
        seq_ds->GetScope(),
        trans_seq,
        true, true);

    if (GetFeature().IsSetProduct()) {
        try {
            const CSeq_loc& product = GetFeature().GetProduct();
            CScope& scope = GetMappedFeature().GetAnnot().GetScope();
            CSeqVector prot_vec(product, scope,
                CBioseq_Handle::eCoding_Iupac);
            prot_vec.GetSeqData (0, prot_vec.size(), prot_seq);
        }
        catch (CException&) {
            // ignore exceptions - these arise if the product doesn't resolve
        }
    } else {
        prot_seq = trans_seq;
    }

    TSeqPos seq_start = 0;
    size_t idx = 0;

    // TODO: should we use intervals instead of location ?
    for (CSeq_loc_CI lit(GetLocation()); lit; ++lit) {
        TSeqPos ifrom = lit.GetRange().GetFrom();
        TSeqPos ito   = lit.GetRange().GetTo();

        char basesIN[2], basesOUT[2];
        basesIN [1] = '\0';
        basesOUT[1] = '\0';

        bool neg = lit.GetStrand () == eNa_strand_minus;
        for (TSeqPos pos = neg ? ito : ifrom;
            (neg ? (pos - offset >= ifrom) : (pos + offset <= ito))  &&
            idx != prot_seq.length() && idx != trans_seq.length() ;
            (neg ? pos -= 3 : pos += 3), ++idx)
        {
            bool mismatch = (prot_seq[idx] != trans_seq[idx]);
            seq_start = neg ? pos - offset : pos + offset;

            // skip out of visible range intervals
            if ( !m_Context->IsInVisibleRange(seq_start) )
                continue;

            basesIN [0] = prot_seq[idx];
            basesOUT[0] = trans_seq[idx];

            if (mismatch) {
                gl.ColorC(m_CdsConfig->m_SeqProtMismatch); // Mismatch
                // Draw protein sequence inside
                m_Context->TextOut(&m_CdsConfig->m_ProdFont, basesIN,
                    seq_start + 0.5, prot_y, true);
            }

            // draw translation sequence underneath the bar
            if (true/*IsSelected()*/) {
                CRgbaColor col("dark blue");
                gl.ColorC(col);
                m_Context->TextOut(&m_CdsConfig->m_ProdFont, basesOUT,
                    seq_start + 0.5, trans_y, true);

                gl.ColorC(m_CdsConfig->m_SeqProtTrans);
                try {
                    string main_seq, tmp_seq;
                    vec.GetSeqData(seq_start - 1, seq_start + 2, tmp_seq);

                    if (neg) {  // reverse sequence on negative strand
                        CSeqManip::Complement(tmp_seq, CSeqUtil::e_Iupacna,
                            0, tmp_seq.length(), main_seq);
                    } else {
                        main_seq = tmp_seq;
                    }

                    if (flip_strand) { // handle the flipped strand mode
                        reverse(main_seq.begin(), main_seq.end());
                    }

                    m_Context->TextOut(&m_CdsConfig->m_TransFont, main_seq.c_str(),
                        seq_start + 0.5, gen_y, true);
                }
                catch (CException&) {
                    /// ignore exceptions - these are objmgr related
                    /// and indicate sequences that don't resolve
                }
            }
        }

        // Use correct offset for the next interval
        int diff = neg ? seq_start - ifrom : ito - seq_start;
        if (diff == 2) {
            offset = 0;
        } else if (diff == 1) {
            offset = 1;
        } else if (diff == 0) {
            offset = 2;
        }
    } // for
}


bool CCdsGlyph::x_WillFit() const
{
    bool will_fit = false;
    if (x_IsProjected()) {
        auto aln_mgr = m_ProjectedMappingInfo.GetAlignmentDataSource();
        int base_width = aln_mgr->GetBaseWidth(aln_mgr->GetAnchor());
        static const float kSequenceLevel = 0.125; // = 1.0f / 8.0;
        will_fit = (m_Context->GetScale() < kSequenceLevel / base_width);
    } else {
        will_fit = m_Context->WillSeqLetterFit();
    }
    // currrently we do not support rendering of the prot/codon for trans-spliced CDS
    if (will_fit && GetFeature().IsSetExcept() && GetFeature().HasExceptionText("trans-splicing"))
        will_fit = false;
    return will_fit;
}


void CCdsGlyph::x_UpdateBoundingBox()
{
    CFeatGlyph::x_UpdateBoundingBox();
    if (x_WillFit()) {
        IRender& gl = GetGl();

        TModelUnit trans_full = gl.GetMetric(&(m_CdsConfig->m_TransFont),
            IGlFont::eMetric_FullCharHeight);
        SetHeight(GetHeight() + trans_full +
                                gl.TextHeight(&(m_CdsConfig->m_ProdFont)) + kVertSpace);
    }
}


//#define HIGHLIGHT_MISMATCHES

void CCdsGlyph::x_DrawProjectedProtSeq(
    TModelUnit trans_y,
    TModelUnit gen_y,
    TModelUnit prot_y) const
{

    IRender& gl = GetGl();
    bool flip_strand = m_Context->IsFlippedStrand();

    string cds_seq;
    string prot_seq;
    size_t gen_len = 0;
    size_t prot_len = 0;
    CScope& scope = GetMappedFeature().GetAnnot().GetScope();
    TSeqPos cds_start = 0;

    CConstRef<CSeq_feat> original_cds;
    try {
        CBioseq_Handle prot_bsh = scope.GetBioseqHandle(GetFeature().GetProduct());
        if (prot_bsh) { // if product defined

            const CSeq_loc& product = GetFeature().GetProduct();
            CSeqVector prot_vec(product, scope, CBioseq_Handle::eCoding_Iupac);
            prot_len = prot_vec.size();
            prot_vec.GetSeqData(0, prot_len, prot_seq);
            gen_len = prot_len * 3;

            // protein sequence (that CDS product is pointing to)
            // is expected to be packed in nuc_prot set
            // GetCDSProduct finds cds annotated on this nuc_prot 
            // so we could the original mrna annotation for this cds

            //string prot_acc = sequence::GetAccessionForId(*prot_bsh.GetSeqId(), prot_bsh.GetScope());
            const CSeq_feat* sfp = sequence::GetCDSForProduct(prot_bsh);
            if (sfp) {
                cds_start = sfp->GetLocation().GetTotalRange().GetFrom();
                CSeqVector v(sfp->GetLocation(), scope, CBioseq_Handle::eCoding_Iupac);
                v.GetSeqData(0, v.size(), cds_seq);
            }
        } 
        
    } catch (CException&) {
        // ignore exceptions - these arise if the product doesn't resolve
    }

    if (cds_seq.empty()) {
        CSeqVector v(GetLocation(), scope, CBioseq_Handle::eCoding_Iupac);
        v.GetSeqData(0, v.size(), cds_seq);
        const CGenetic_code* genetic_code = GetFeature().GetData().GetCdregion().IsSetCode() ?
            &GetFeature().GetData().GetCdregion().GetCode() : nullptr;
        CSeqTranslator::Translate(cds_seq, prot_seq, CSeqTranslator::fIs5PrimePartial, genetic_code);
    }
    _ASSERT(!cds_seq.empty());
    if (cds_seq.empty()) {
        // unexpected error to get translated sequence
        return;
    }

    if (gen_len == 0) {
        gen_len = GetLocation().GetTotalRange().GetLength();
        prot_len = gen_len / 3 + 1;
    }

#ifdef HIGHLIGHT_MISMATCHES
    string gen_seq;
    m_ProjectedMappingInfo.GetAnchorSequence(scope, gen_seq);
    // Get genetic code
    string trans_seq;
    const CGenetic_code* genetic_code = GetFeature().GetData().GetCdregion().IsSetCode() ?
        &GetFeature().GetData().GetCdregion().GetCode() : nullptr;
    CSeqTranslator::Translate(gen_seq, trans_seq, CSeqTranslator::fIs5PrimePartial, genetic_code);
    if (cds_seq.empty())  // no original cds present
        cds_seq = gen_seq;
    if (prot_seq.empty())
        prot_seq = trans_seq;
    char prot_out[2];
    prot_out[1] = '\0';
#endif

    auto aln_mgr = m_ProjectedMappingInfo.GetAlignmentDataSource();

    int anchor = aln_mgr->GetAnchor();
    int aligned_seq = aln_mgr->GetQuery();
    int base_width = aln_mgr->GetBaseWidth(anchor);
    int anchor_step = base_width == 3 ? 1 : 3;

    // Temp strings holding the one-letter protein sequence
    char prot_in[2];
    prot_in[1] = '\0';

    static const CRgbaColor codon_mismatch_col("red");
    static const CRgbaColor translated_col("dark blue");

    TSeqPos prot_offset = x_GetProtOffset() - 1;

     bool aligned_neg = aln_mgr->IsNegativeStrand(aligned_seq);
     bool anchor_neg = aln_mgr->IsNegativeStrand(anchor);

    for (auto&& map_it : m_ProjectedMappingInfo) {
        auto& anchor_range = map_it.m_AnchorRange;
        if (m_Context->IntersectVisible(anchor_range).Empty())
            continue;

        auto& prod_int = *map_it.m_ProductInt;
        auto& gen_int = *map_it.m_MappedInt;
        auto& prod_range = map_it.m_MappedProdRange;

        bool neg = gen_int.IsSetStrand() && gen_int.GetStrand() == eNa_strand_minus;
        int anchor_sign = (anchor_neg != aligned_neg && anchor_neg) || (aligned_neg != neg) ? -1 : 1;

        int step = neg ? -3 : 3 ;
        step /= base_width;
        TSeqRange gen_range(gen_int.GetFrom(), gen_int.GetTo());

        // prod_pos - position on transcript
        TSeqPos prod_pos = prod_int.GetFrom();
        TSeqPos prod_stop = prod_int.GetTo() + 1;
        
        prod_stop = min<int>(prod_stop, prod_pos + round(float(gen_range.GetLength()) / base_width));

        TSignedSeqPos pos = anchor_sign < 0 ?
            anchor_range.GetTo() : anchor_range.GetFrom();

        // Align the first position to frame 1, middle of codon.
        // If frame is already 2 (last codon position), don't draw it here,
        // use the next frame 1 position

        int prod_from = prod_range.GetFrom();
        int frame = prod_from > 0 ? prod_from % 3 : prot_offset;
        TSeqPos prod_pos_f1 = (prod_from / 3) * 3 + 1;
        pos += ((anchor_sign * anchor_step) / 3) * (prod_pos_f1 - prod_from);
        if (frame > 1) {
            pos += (anchor_sign * anchor_step);
        }
        while (prod_pos < prod_stop) {
            if (prod_pos >= prot_seq.size())
                break;
            
            if (anchor_sign > 0 && pos > (TSignedSeqPos)anchor_range.GetTo())
                break;
            if (anchor_sign < 0 && pos < (TSignedSeqPos)anchor_range.GetFrom())
                break;
                         //   if (pos >= m_Context->GetVisSeqFrom() && pos < m_Context->GetVisSeqTo()) {
                prot_in[0] = 'X';
                if (prod_pos < prot_seq.size())
                    prot_in[0] = prot_seq[prod_pos];
#ifdef HIGHLIGHT_MISMATCHES
                prot_out[0] = 'X'
                if (prod_pos < trans_seq.size())
                    prot_out[0] = trans_seq[prod_pos];
                bool mismatch = (prot_in[0] != prot_out[0]);

                // draw original protein sequence
                if (mismatch) {
                    gl.ColorC(m_CdsConfig->m_SeqProtMismatch);
                    m_Context->TextOut(&m_CdsConfig->m_ProdFont, prot_in,
                                       pos + 0.5, prot_y, true);
                }
#endif
                TSeqPos p = prod_pos > 0 ? (prod_pos * 3) : 0;
                string codon = cds_seq.substr(p, 3);
#ifdef HIGHLIGHT_MISMATCHES
                string gen_codon = gen_seq.substr(p, 3);
                gl.ColorC(codon != gen_codon ? codon_mismatch_col : m_CdsConfig->m_SeqProtTrans);
#else
                gl.ColorC(m_CdsConfig->m_SeqProtTrans);
#endif
                x_PutCodon(codon, pos, gen_y, flip_strand != (anchor_sign < 0), false);

                // draw translated protein sequence
                gl.ColorC(translated_col);
                m_Context->TextOut(&m_CdsConfig->m_ProdFont, prot_in,
                                   pos + 0.5, trans_y, true);
           // }
            prod_pos += 1;// base_width;
            pos += anchor_sign * anchor_step;
        }
    }
}

// set to enable dumping CDS info
// #define CDS_DEBUG

void CCdsGlyph::x_DrawProtSeqWithMapping(
    TModelUnit trans_y,
    TModelUnit gen_y,
    TModelUnit prot_y) const
{
    _ASSERT(!GetMappingInfo().empty());

    IRender& gl = GetGl();
    bool flip_strand = m_Context->IsFlippedStrand();
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    const CSeqVector& vec = seq_ds->GetSeqVector();

    string cds_seq;
    string prot_seq;
    size_t gen_len = 0;
    size_t prot_len = 0;
    CScope& scope = GetMappedFeature().GetAnnot().GetScope();
    AutoPtr<CSeq_loc_Mapper> cds_mapper;
    TSeqPos cds_start = 0;

    CConstRef<CSeq_feat> original_cds;
    if (GetFeature().CanGetProduct()) try {
        CBioseq_Handle prot_bsh = scope.GetBioseqHandle(GetFeature().GetProduct());
        if (prot_bsh) {
            const CSeq_loc& product = GetFeature().GetProduct();
            CSeqVector prot_vec(product, scope, CBioseq_Handle::eCoding_Iupac);
            prot_len = prot_vec.size();
            prot_vec.GetSeqData(0, prot_len, prot_seq);
#ifdef CDS_DEBUG
            cerr << "prot_seq (obtained as feature product): " << endl << prot_seq << endl << "prot_seq.length(): " <<
                prot_seq.length() << ", * 3: " << prot_seq.length() * 3 << endl;
#endif
            gen_len = prot_len * 3;

            // protein sequence (that CDS product is pointing to)
            // is expected to be packed in nuc_prot set
            // GetCDSProduct finds cds annotated on this nuc_prot 
            // so we could the original mrna annotation for this cds
            const CSeq_feat* sfp = sequence::GetCDSForProduct(prot_bsh);
            if (sfp) {
#ifdef CDS_DEBUG
                cerr << "sfp (cds feature obtained for the product): " << endl << MSerial_AsnText << *sfp << endl;
#endif
                cds_start = sfp->GetLocation().GetTotalRange().GetFrom();
                CSeqVector v(sfp->GetLocation(), scope, CBioseq_Handle::eCoding_Iupac);
                v.GetSeqData(0, v.size(), cds_seq);
#ifdef CDS_DEBUG
                cerr << "cds_seq (obtained from product):" << endl << cds_seq << endl << "cds_seq.length(): " << cds_seq.length() <<
                    ", cds_start: " << cds_start << endl;
#endif
                //CConstRef<CSeq_feat> mrna = sequence::GetmRNAforCDS(*sfp, scope);
                if (sfp->IsSetExcept()) {
                    CSeq_loc_CI lit(sfp->GetLocation());
                    // if cds location has multiple segments 
                    // then translationationl framshift should be taken care of 
                    // via seq_loc_mapper
                    if(lit.GetSize() > 1) {
                        cds_mapper.reset(new CSeq_loc_Mapper(*sfp, CSeq_loc_Mapper::eProductToLocation, &scope));
                    }
                }
            }
        } 
    } catch (const CException&) {
        // ignore exceptions - these arise if the product doesn't resolve
    }

    if (gen_len == 0) {
        gen_len = GetLocation().GetTotalRange().GetLength();
        prot_len = gen_len / 3 + 1;
    }

    // Get the mapped genomic sequence based the mapping intervals
    string gen_seq;
#ifdef CDS_DEBUG
        cerr << "gen_sec (parts of original seq obtained via intervals): " << endl;
#endif
    gen_seq.reserve(gen_len);
    {
        TSeqPos pre_t1 = 0;
        for (auto&& iter : GetMappingInfo()) {
            // fill the gap on protein sequence with 'N'
            TSeqPos f1 = iter.first->GetFrom();
            TSeqPos f2 = iter.second->GetFrom();
            TSeqPos t2 = iter.second->GetTo();
            if(f1 > pre_t1) {
                gen_seq.append(f1 - pre_t1, 'N');
#ifdef CDS_DEBUG
                cerr << string(f1 - pre_t1, 'N');
#endif
            }
            string tmp_seq;
            vec.GetSeqData(f2, t2 + 1, tmp_seq);
            if (iter.second->GetStrand() == eNa_strand_minus) {
                string seq;
                CSeqManip::ReverseComplement(tmp_seq, CSeqUtil::e_Iupacna,
                                             0, tmp_seq.length(), seq);
                tmp_seq.swap(seq);
            }
            pre_t1 = iter.first->GetTo() + 1;
            gen_seq += tmp_seq;
#ifdef CDS_DEBUG
            cerr << tmp_seq << "|";
#endif
        }
    }
#ifdef CDS_DEBUG
    cerr << endl << "gen_seq.length(): " <<  gen_seq.length() << endl;
#endif

    // Get genetic code
    string trans_seq;
    const CGenetic_code* genetic_code = GetFeature().GetData().GetCdregion().IsSetCode() ?
        &GetFeature().GetData().GetCdregion().GetCode() : nullptr;
    CSeqTranslator::Translate(gen_seq, trans_seq, CSeqTranslator::fIs5PrimePartial, genetic_code);

#ifdef CDS_DEBUG
    cerr << "trans_seq (translation from gen_seq): " << endl << trans_seq << endl << "trans_seq.length(): " <<
        trans_seq.length() << ", * 3: " << trans_seq.length() * 3 << endl;
#endif    
    if (cds_seq.empty())  // no original cds present
        cds_seq = gen_seq;
    if (prot_seq.empty())
        prot_seq = trans_seq;

    // Temp strings holding the one-letter protein sequence
    char prot_in[2], prot_out[2];
    prot_in [1] = '\0';
    prot_out[1] = '\0';

    static const CRgbaColor codon_mismatch_col("red");
    static const CRgbaColor translated_col("dark blue");

    CConstRef<CSeq_interval> prev_gen(0);
    CConstRef<CSeq_interval> prev_prod(0);
    auto current_exon = m_Intervals.begin();

    TModelUnit font_height = gl.TextHeight(&m_CdsConfig->m_ProdFont);
    font_height *= 0.5;

    for (auto&& map_it : GetMappingInfo()) {
        auto& prod_int = *map_it.first;
        auto& gen_int = *map_it.second;

        bool neg = gen_int.IsSetStrand() && gen_int.GetStrand() == eNa_strand_minus;
        TSeqRange gen_range(gen_int.GetFrom(), gen_int.GetTo());
        int step = neg ? -3 : 3;
        if (neg) {
            while (current_exon != m_Intervals.end() && current_exon->GetFrom() > gen_range.GetTo()) {
                ++current_exon;
                prev_gen = 0;
            }
        } else {
            while (current_exon != m_Intervals.end() && current_exon->GetTo() < gen_range.GetFrom()) {
                ++current_exon;
                prev_gen = 0;
            }
        }
        // if this interval is outside of the visible range, skip it
        if (m_Context->IntersectVisible(gen_range).Empty())
            continue;
        if (current_exon == m_Intervals.end())
            prev_gen = 0;

        // prod_pos - position on transcript
        TSeqPos prod_pos = prod_int.GetFrom();
        TSeqPos prod_stop = min(prod_int.GetTo() + 1, (TSeqPos)gen_len);

        // c_pos - position on genomic sequence
        TSeqPos c_pos = neg ? gen_range.GetTo() : gen_range.GetFrom();

        // Align the first position to frame 1, middle of codon.
        // If frame is already 2 (last codon position), don't draw it here,
        // use the next frame 1 position
        int frame = prod_pos % 3;
        TSeqPos prod_pos_f1 = (prod_pos / 3) * 3 + 1;
        c_pos += (step / 3) * (prod_pos_f1 - prod_pos);
        prod_pos = prod_pos_f1;
        if (frame > 1) {
            prod_pos += 3;
            c_pos += step;
        }
        CRef<CSeq_loc> prod_loc;
        if (cds_mapper)
            prod_loc.Reset(new CSeq_loc(*const_cast<CSeq_id*>(GetFeature().GetProduct().GetId()),
            TSeqPos(0), eNa_strand_plus));
#ifdef CDS_DEBUG
        if(prod_loc) {
            cerr << "prod_loc: " << MSerial_AsnText << *prod_loc << endl;
        } else {
            cerr << "prod_loc is null" << endl;
        }
#endif
        TSignedSeqPos prev_pos = -1;
        size_t prot_idx = prod_pos / 3;
        /* 
        one-base exons uncomment 
        to print the bases 
        if (prod_pos == prod_stop) {
            prod_stop += 1;
            //c_pos += neg ? 1 : -1;
        }
        */
        while (prod_pos < prod_stop) {

            string gen_codon;
            string codon;
            int frameshift = 0;
            bool mapped = false;
            if (cds_mapper) {
                prod_loc->SetPnt().SetPoint(TSeqPos(prot_idx));
                CRef<CSeq_loc> cds_loc = cds_mapper->Map(*prod_loc);
                if (cds_loc) {
                    codon = cds_seq.substr(prot_idx * 3, cds_loc->GetTotalRange().GetLength());
//                    TSeqPos from = neg ? (gen_range.GetTo() - cds_loc->GetTotalRange().GetTo())
//                        : (cds_loc->GetTotalRange().GetFrom() - gen_range.GetFrom());
//!! this is where the wrongness starts!                    
                    TSeqPos from = cds_loc->GetTotalRange().GetFrom() - cds_start;
                    if (prev_pos < 0)
                        prev_pos = from - from % 3;
                    //!! don't do anything if from is out of range
                    if(from > gen_seq.length()) {
#ifdef CDS_DEBUG
                        cerr << "!!! attempt to go beyond gen_sec: from: " << from << ", gen_seq.length(): " << gen_seq.length() << endl;
                        cerr << "prod_loc: " << MSerial_AsnText << *prod_loc << endl;
                        cerr << "prod_int: " << MSerial_AsnText << prod_int << endl;
                        cerr << "gen_int: " << MSerial_AsnText << gen_int << endl;
                        cerr << "gen_range: " << MSerial_AsnText << gen_range << endl;
                        cerr << "cds_loc: " << MSerial_AsnText << *cds_loc << endl;
                        cerr << "cds_start: " << cds_start << endl;
#endif
                        //!! the code that uses mapping is sometimes wrong (see SV-3858)
                        //!! so for the time being we will avoid trying to do anything in this case and fix
                        //!! comprehensively later (SV-3928)
                        return;
                    }
                    gen_codon = gen_seq.substr(from, cds_loc->GetTotalRange().GetLength());
                    if (prev_pos > 0 && prev_pos != from) {
                        frameshift = from - prev_pos;
                        if (frameshift != 0) {
                            c_pos += frameshift * (neg ? -1 : 1);
                            // shift the genomic sequence pos and re-translate
                            string new_seq = gen_seq.substr(prot_idx * 3 + frameshift);
                            string new_trans_seq;
                            CSeqTranslator::Translate(new_seq, new_trans_seq, CSeqTranslator::fIs5PrimePartial, genetic_code);
                            trans_seq = trans_seq.substr(0, prot_idx);
                            trans_seq.append(new_trans_seq);
                        }
                    }
                    prev_pos = (cds_loc->GetTotalRange().GetTo() + 1) - cds_start;
                    mapped = true;
                }
            } 
            prot_in[0] = prot_out[0] = 'X';
            if (prot_idx < prot_seq.size())
                prot_in[0] = prot_seq[prot_idx];
            if (prot_idx < trans_seq.size())
                prot_out[0] = trans_seq[prot_idx];
            bool mismatch = (prot_in[0] != prot_out[0]);

            // draw original protein sequence
            if (mismatch) {
                gl.ColorC(m_CdsConfig->m_SeqProtMismatch);
                m_Context->TextOut(&m_CdsConfig->m_ProdFont, prot_in,
                                   c_pos + 0.5, prot_y, true);
            }
            if (!mapped) {
                gen_codon = gen_seq.substr(prod_pos - 1, 3);
                codon = cds_seq.substr(prod_pos - 1, 3);
            }
            gl.ColorC(codon != gen_codon ? codon_mismatch_col : m_CdsConfig->m_SeqProtTrans);
            x_PutCodon(codon, c_pos, gen_y, flip_strand != neg, false);

            // draw translated protein sequence
            gl.ColorC(translated_col);
            m_Context->TextOut(&m_CdsConfig->m_ProdFont, prot_in,
                               c_pos + 0.5, trans_y, true);
            prod_pos += (3 + frameshift);
            c_pos += step;
            ++prot_idx;
        }
        if (prev_gen != 0) {

            if (prev_prod->GetTo() + 1 != prod_int.GetFrom()) {
                c_pos = neg ? gen_int.GetTo() + 1 : gen_int.GetFrom();
                gl.ColorC(m_CdsConfig->m_SeqProtMismatch);
                x_DrawInsertion(c_pos, prot_y - (font_height + font_height), prot_y, prot_y - font_height);
            } else {
                int gap_length = 0;
                if (neg) {
                    if (prev_gen->GetFrom() - 1 != gen_int.GetTo()) {
                        c_pos = prev_gen->GetFrom() - 1;
                        gap_length = c_pos - gen_int.GetTo();
                    }
                } else {
                    if (prev_gen->GetTo() + 1 != gen_int.GetFrom()) {
                        c_pos = prev_gen->GetTo() + 1;
                        gap_length = gen_int.GetFrom() - c_pos;
                    }
                }
                if (gap_length > 0) {
                    gl.ColorC(m_CdsConfig->m_SeqProtMismatch);

                    while (gap_length > 0) {
                        m_Context->TextOut(&m_CdsConfig->m_ProdFont, "-", c_pos + 0.5, prot_y, true);
                        c_pos += (neg ? -1 : 1);
                        --gap_length;
                    }
                }
            }
        }
        prev_gen.Reset(&gen_int);
        prev_prod.Reset(&prod_int);

    }
}


void CCdsGlyph::x_PutCodon(const string& codon_str, TSeqPos pos,
    TModelUnit y, bool neg, bool append) const
{
    string new_codon = codon_str;
    size_t len = codon_str.size();
    if (len > 0  &&  len < 5) {
        if (len == 2) {
            if (append) new_codon += " ";
            else        new_codon.insert(0, " ");
        }
        if (neg) std::reverse(new_codon.begin(), new_codon.end());
        m_Context->TextOut(&m_CdsConfig->m_ProdFont, new_codon.c_str(),
            pos + 0.5, y, true);
    }
}


END_NCBI_SCOPE
