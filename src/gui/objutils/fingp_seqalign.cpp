/*  $Id: fingp_seqalign.cpp 27635 2013-03-15 19:33:00Z katargir@NCBI.NLM.NIH.GOV $
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
#include <util/checksum.hpp>

#include <objects/seqalign/Dense_diag.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Std_seg.hpp>
#include <objects/seqalign/Packed_seg.hpp>
#include <objects/seqalign/Spliced_seg.hpp>
#include <objects/seqalign/Prot_pos.hpp>
#include <objects/seqalign/Product_pos.hpp>
#include <objects/seqalign/Spliced_exon.hpp>
#include <objects/seqalign/Spliced_exon_chunk.hpp>
#include <objects/seqalign/Sparse_align.hpp>
#include <objects/seqalign/Sparse_seg.hpp>
#include <objects/seqalign/Sparse_seg_ext.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static void s_Fingerprint_Dense_diag(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CDense_diag*>(&object));
    const CDense_diag& obj = static_cast<const CDense_diag&>(object);

    CKSUM_INTEGER_FIELD(checksum, CDense_diag, Dim);

    ITERATE(CDense_diag::TIds, it, obj.GetIds())
        fingerP.Fingerprint(checksum, **it, scope);

    ITERATE(CDense_diag::TStarts, it, obj.GetStarts()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }

    TSeqPos len = obj.GetLen();
    CKSUM_ADD_INTEGER(checksum, len);

    if (obj.IsSetStrands()) {
        ITERATE(CDense_diag::TStrands, it, obj.GetStrands()) {
            ENa_strand strand = *it;
            CKSUM_ADD_INTEGER(checksum, strand);
        }
    }
}

static void s_Fingerprint_Dense_seg(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CDense_seg*>(&object));
    const CDense_seg& obj = static_cast<const CDense_seg&>(object);

    CKSUM_INTEGER_FIELD(checksum, CDense_seg, Dim);
    CKSUM_INTEGER_FIELD(checksum, CDense_seg, Numseg);

    ITERATE(CDense_seg::TIds, it, obj.GetIds())
        fingerP.Fingerprint(checksum, **it, scope);

    ITERATE(CDense_seg::TStarts, it, obj.GetStarts()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }

    ITERATE(CDense_seg::TLens, it, obj.GetLens()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }

    if (obj.IsSetStrands()) {
        ITERATE(CDense_seg::TStrands, it, obj.GetStrands()) {
            ENa_strand strand = *it;
            CKSUM_ADD_INTEGER(checksum, strand);
        }
    }
}

static void s_Fingerprint_Std_seg(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CStd_seg*>(&object));
    const CStd_seg& obj = static_cast<const CStd_seg&>(object);

    CKSUM_INTEGER_FIELD(checksum, CStd_seg, Dim);

    ITERATE(CStd_seg::TIds, it, obj.GetIds())
        fingerP.Fingerprint(checksum, **it, scope);

    ITERATE(CStd_seg::TLoc, it, obj.GetLoc())
        fingerP.Fingerprint(checksum, **it, scope);
}

static void s_Fingerprint_Packed_seg(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CPacked_seg*>(&object));
    const CPacked_seg& obj = static_cast<const CPacked_seg&>(object);

    CKSUM_INTEGER_FIELD(checksum, CPacked_seg, Dim);
    CKSUM_INTEGER_FIELD(checksum, CPacked_seg, Numseg);

    ITERATE(CPacked_seg::TIds, it, obj.GetIds())
        fingerP.Fingerprint(checksum, **it, scope);

    ITERATE(CPacked_seg::TStarts, it, obj.GetStarts()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }

    ITERATE(CPacked_seg::TPresent, it, obj.GetPresent()) {
        char present = *it;
        CKSUM_ADD_INTEGER(checksum, present);
    }

    ITERATE(CPacked_seg::TLens, it, obj.GetLens()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }

    if (obj.IsSetStrands()) {
        ITERATE(CPacked_seg::TStrands, it, obj.GetStrands()) {
            ENa_strand strand = *it;
            CKSUM_ADD_INTEGER(checksum, strand);
        }
    }
}

static void s_Fingerprint_Seq_align_set(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP);

static void s_Fingerprint_Prot_pos(
                             CChecksum& checksum, const CObject& object,
                             CScope*, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CProt_pos*>(&object));
    const CProt_pos& obj = static_cast<const CProt_pos&>(object);
    CKSUM_INTEGER_FIELD(checksum, CProt_pos, Amin);
    CKSUM_INTEGER_FIELD(checksum, CProt_pos, Frame);
}

static void s_Fingerprint_Product_pos(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CProduct_pos*>(&object));
    const CProduct_pos& obj = static_cast<const CProduct_pos&>(object);

    CProduct_pos::E_Choice choice = obj.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    switch(choice) {
    case CProduct_pos::e_Nucpos :
        CKSUM_INTEGER_FIELD2(checksum, CProduct_pos, Nucpos);
        break;
    case CProduct_pos::e_Protpos :
        s_Fingerprint_Prot_pos(checksum, obj.GetProtpos(), scope, fingerP);
        break;
    default :
        break;
    }
}

static void s_Fingerprint_Spliced_exon_chunk(
                             CChecksum& checksum, const CObject& object,
                             CScope*, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CSpliced_exon_chunk*>(&object));
    const CSpliced_exon_chunk& obj = static_cast<const CSpliced_exon_chunk&>(object);

    CSpliced_exon_chunk::E_Choice choice = obj.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    switch(choice) {
    case CSpliced_exon_chunk::e_Match :
        CKSUM_INTEGER_FIELD2(checksum, CSpliced_exon_chunk, Match);
        break;
    case CSpliced_exon_chunk::e_Mismatch :
        CKSUM_INTEGER_FIELD2(checksum, CSpliced_exon_chunk, Mismatch);
        break;
    case CSpliced_exon_chunk::e_Diag :
        CKSUM_INTEGER_FIELD2(checksum, CSpliced_exon_chunk, Diag);
        break;
    case CSpliced_exon_chunk::e_Product_ins :
        CKSUM_INTEGER_FIELD2(checksum, CSpliced_exon_chunk, Product_ins);
        break;
    case CSpliced_exon_chunk::e_Genomic_ins:
        CKSUM_INTEGER_FIELD2(checksum, CSpliced_exon_chunk, Genomic_ins);
        break;
    default:
        break;
    }
}

static void s_Fingerprint_Spliced_exon(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSpliced_exon*>(&object));
    const CSpliced_exon& obj = static_cast<const CSpliced_exon&>(object);

    s_Fingerprint_Product_pos(checksum, obj.GetProduct_start(), scope, fingerP);
    s_Fingerprint_Product_pos(checksum, obj.GetProduct_end(), scope, fingerP);
    CKSUM_INTEGER_FIELD(checksum, CSpliced_exon, Genomic_start);
    CKSUM_INTEGER_FIELD(checksum, CSpliced_exon, Genomic_end);

    if (obj.IsSetProduct_id())
        fingerP.Fingerprint(checksum, obj.GetProduct_id(), scope);
    if (obj.IsSetGenomic_id())
        fingerP.Fingerprint(checksum, obj.GetGenomic_id(), scope);

    CKSUM_INTEGER_FIELD(checksum, CSpliced_exon, Product_strand);
    CKSUM_INTEGER_FIELD(checksum, CSpliced_exon, Genomic_strand);

    if (obj.IsSetParts()) {
        ITERATE(CSpliced_exon::TParts, it,  obj.GetParts())
            s_Fingerprint_Spliced_exon_chunk(checksum, **it, scope, fingerP);
    }

    CKSUM_INTEGER_FIELD(checksum, CSpliced_exon, Partial);
}

static void s_Fingerprint_Spliced_seg(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSpliced_seg*>(&object));
    const CSpliced_seg& obj = static_cast<const CSpliced_seg&>(object);

    if (obj.IsSetProduct_id())
        fingerP.Fingerprint(checksum, obj.GetProduct_id(), scope);
    if (obj.IsSetGenomic_id())
        fingerP.Fingerprint(checksum, obj.GetGenomic_id(), scope);

    CKSUM_INTEGER_FIELD(checksum, CSpliced_seg, Product_strand);
    CKSUM_INTEGER_FIELD(checksum, CSpliced_seg, Genomic_strand);

    CKSUM_INTEGER_FIELD(checksum, CSpliced_seg, Product_type);

    ITERATE(CSpliced_seg::TExons, it, obj.GetExons())
        s_Fingerprint_Spliced_exon(checksum, **it, scope, fingerP);

    CKSUM_INTEGER_FIELD(checksum, CSpliced_seg, Poly_a);
    CKSUM_INTEGER_FIELD(checksum, CSpliced_seg, Product_length);
}

static void s_Fingerprint_Sparse_align(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSparse_align*>(&object));
    const CSparse_align& obj = static_cast<const CSparse_align&>(object);

    fingerP.Fingerprint(checksum, obj.GetFirst_id(), scope);
    fingerP.Fingerprint(checksum, obj.GetSecond_id(), scope);
    CKSUM_INTEGER_FIELD(checksum, CSparse_align, Numseg);

    ITERATE(CSparse_align::TFirst_starts, it, obj.GetFirst_starts()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }
    ITERATE(CSparse_align::TSecond_starts, it, obj.GetSecond_starts()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }
    ITERATE(CSparse_align::TLens, it, obj.GetLens()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }

    if (obj.IsSetSecond_strands()) {
        ITERATE(CSparse_align::TSecond_strands, it, obj.GetSecond_strands()) {
            ENa_strand strand = *it;
            CKSUM_ADD_INTEGER(checksum, strand);
        }
    }
}

static void s_Fingerprint_Sparse_seg(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSparse_seg*>(&object));
    const CSparse_seg& obj = static_cast<const CSparse_seg&>(object);

    fingerP.Fingerprint(checksum, obj.GetMaster_id(), scope);
    ITERATE(CSparse_seg::TRows, it, obj.GetRows())
        s_Fingerprint_Sparse_align(checksum, **it, scope, fingerP);

    if (obj.IsSetExt()) {
        ITERATE(CSparse_seg::TExt, it, obj.GetExt()) {
            int value = (*it)->GetIndex();
            CKSUM_ADD_INTEGER(checksum, value);
        }
    }
}

static void s_Fingerprint_Seq_align(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_align*>(&object));
    const CSeq_align& obj = static_cast<const CSeq_align&>(object);
    const CSeq_align::C_Segs& segs = obj.GetSegs();

    CSeq_align::C_Segs::E_Choice choice = segs.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    switch(choice) {
    case CSeq_align::C_Segs::e_Dendiag:
        ITERATE(CSeq_align::C_Segs::TDendiag, it,  segs.GetDendiag())
            s_Fingerprint_Dense_diag(checksum, **it, scope, fingerP);
        break;
    case CSeq_align::C_Segs::e_Denseg:
        s_Fingerprint_Dense_seg(checksum, segs.GetDenseg(), scope, fingerP);
        break;
    case CSeq_align::C_Segs::e_Std:
        ITERATE(CSeq_align::C_Segs::TStd, it,  segs.GetStd())
            s_Fingerprint_Std_seg(checksum, **it, scope, fingerP);
        break;
    case CSeq_align::C_Segs::e_Packed:
        s_Fingerprint_Packed_seg(checksum, segs.GetPacked(), scope, fingerP);
        break;
    case CSeq_align::C_Segs::e_Disc:
        s_Fingerprint_Seq_align_set(checksum, segs.GetDisc(), scope, fingerP);
        break;
    case CSeq_align::C_Segs::e_Spliced:
        s_Fingerprint_Spliced_seg(checksum, segs.GetSpliced(), scope, fingerP);
        break;
    case CSeq_align::C_Segs::e_Sparse:
        s_Fingerprint_Sparse_seg(checksum, segs.GetSparse(), scope, fingerP);
        break;
    default:
    case CSeq_align::C_Segs::e_not_set:
        break;
    }
}

static void s_Fingerprint_Seq_align_set(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_align_set*>(&object));
    const CSeq_align_set& obj = static_cast<const CSeq_align_set&>(object);

    ITERATE(CSeq_align_set::Tdata, it, obj.Get())
        s_Fingerprint_Seq_align(checksum, **it, scope, fingerP);
}

void init_Seq_alignFP()
{
    CObjFingerprint::Register(CSeq_align_set::GetTypeInfo(), s_Fingerprint_Seq_align_set);
    CObjFingerprint::Register(CSeq_align::GetTypeInfo(), s_Fingerprint_Seq_align);
    CObjFingerprint::Register(CDense_diag::GetTypeInfo(), s_Fingerprint_Dense_diag);
    CObjFingerprint::Register(CDense_seg::GetTypeInfo(), s_Fingerprint_Dense_seg);
    CObjFingerprint::Register(CPacked_seg::GetTypeInfo(), s_Fingerprint_Packed_seg);
    CObjFingerprint::Register(CStd_seg::GetTypeInfo(), s_Fingerprint_Std_seg);
    CObjFingerprint::Register(CSpliced_seg::GetTypeInfo(), s_Fingerprint_Spliced_seg);
    CObjFingerprint::Register(CSpliced_exon_chunk::GetTypeInfo(), s_Fingerprint_Spliced_exon_chunk);
    CObjFingerprint::Register(CProt_pos::GetTypeInfo(), s_Fingerprint_Prot_pos);
    CObjFingerprint::Register(CProduct_pos::GetTypeInfo(), s_Fingerprint_Product_pos);
    CObjFingerprint::Register(CSpliced_exon::GetTypeInfo(), s_Fingerprint_Spliced_exon);
    CObjFingerprint::Register(CSparse_align::GetTypeInfo(), s_Fingerprint_Sparse_align);
    CObjFingerprint::Register(CSparse_seg::GetTypeInfo(), s_Fingerprint_Sparse_seg);
}

END_NCBI_SCOPE
