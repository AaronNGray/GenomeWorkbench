/*  $Id: fingp_seqloc.cpp 27635 2013-03-15 19:33:00Z katargir@NCBI.NLM.NIH.GOV $
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

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqloc/Giimport_id.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_point.hpp>
#include <objects/seqloc/Seq_loc_equiv.hpp>
#include <objects/seqloc/Seq_bond.hpp>
#include <objects/seqfeat/Feat_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static void s_Fingerprint_Seq_point(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_point*>(&object));
    const CSeq_point& obj = static_cast<const CSeq_point&>(object);

    TSeqPos pos = obj.GetPoint();
    CKSUM_ADD_INTEGER(checksum, pos);

    if (obj.IsSetId())
        fingerP.Fingerprint(checksum, obj.GetId(), scope);
}

static void s_Fingerprint_Packed_seqpnt(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CPacked_seqpnt*>(&object));
    const CPacked_seqpnt& obj = static_cast<const CPacked_seqpnt&>(object);

    ITERATE(CPacked_seqpnt::TPoints, it,  obj.GetPoints()) {
        TSeqPos pos = *it;
        CKSUM_ADD_INTEGER(checksum, pos);
    }

    if (obj.IsSetId())
        fingerP.Fingerprint(checksum, obj.GetId(), scope);
}

static void s_Fingerprint_Seq_interval(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_interval*>(&object));
    const CSeq_interval& obj = static_cast<const CSeq_interval&>(object);

    TSeqPos from = obj.GetFrom(), to = obj.GetTo();
    CKSUM_ADD_INTEGER(checksum, from);
    CKSUM_ADD_INTEGER(checksum, to);

    if (obj.IsSetId())
        fingerP.Fingerprint(checksum, obj.GetId(), scope);
}

static void s_Fingerprint_Packed_seqint(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CPacked_seqint*>(&object));
    const CPacked_seqint& obj = static_cast<const CPacked_seqint&>(object);

    ITERATE(CPacked_seqint::Tdata, it,  obj.Get())
        s_Fingerprint_Seq_interval(checksum, **it, scope, fingerP);
}

static void s_Fingerprint_Seq_loc(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP);

static void s_Fingerprint_Seq_loc_mix(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_loc_mix*>(&object));
    const CSeq_loc_mix& obj = static_cast<const CSeq_loc_mix&>(object);

    ITERATE(CSeq_loc_mix::Tdata, it,  obj.Get())
        s_Fingerprint_Seq_loc(checksum, **it, scope, fingerP);
}

static void s_Fingerprint_Seq_loc_equiv(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_loc_equiv*>(&object));
    const CSeq_loc_equiv& obj = static_cast<const CSeq_loc_equiv&>(object);

    ITERATE(CSeq_loc_equiv::Tdata, it,  obj.Get())
        s_Fingerprint_Seq_loc(checksum, **it, scope, fingerP);
}

static void s_Fingerprint_Seq_bond(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_bond*>(&object));
    const CSeq_bond& obj = static_cast<const CSeq_bond&>(object);

    s_Fingerprint_Seq_point(checksum, obj.GetA(), scope, fingerP);
}

static void s_Fingerprint_Feat_id(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CFeat_id*>(&object));
    const CFeat_id& obj = static_cast<const CFeat_id&>(object);

    CFeat_id::E_Choice choice = obj.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    switch(choice) {
    case CFeat_id::e_Gibb:
        {
            CFeat_id::TGibb value = obj.GetGibb();
            CKSUM_ADD_INTEGER(checksum, value);
        }
        break;
    case CFeat_id::e_Giim:
        fingerP.Fingerprint(checksum, obj.GetGiim(), scope);
        break;
    case CFeat_id::e_Local:
        fingerP.Fingerprint(checksum, obj.GetLocal(), scope);
        break;
    case CFeat_id::e_General:
        fingerP.Fingerprint(checksum, obj.GetGeneral(), scope);
        break;
    case CFeat_id::e_not_set:
        break;
    }
}

static void s_Fingerprint_Seq_loc(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_loc*>(&object));
    const CSeq_loc& obj = static_cast<const CSeq_loc&>(object);

    CSeq_loc::E_Choice choice = obj.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    switch(choice) {
    case CSeq_loc::e_Empty:
        fingerP.Fingerprint(checksum, obj.GetEmpty(), scope);
        break;
    case CSeq_loc::e_Whole:
        fingerP.Fingerprint(checksum, obj.GetWhole(), scope);
        break;
    case CSeq_loc::e_Int:
        s_Fingerprint_Seq_interval(checksum, obj.GetInt(), scope, fingerP);
        break;
    case CSeq_loc::e_Packed_int:
        s_Fingerprint_Packed_seqint(checksum, obj.GetPacked_int(), scope, fingerP);
        break;
    case CSeq_loc::e_Pnt:
        s_Fingerprint_Seq_point(checksum, obj.GetPnt(), scope, fingerP);
        break;
    case CSeq_loc::e_Packed_pnt:
        s_Fingerprint_Packed_seqpnt(checksum, obj.GetPacked_pnt(), scope, fingerP);
        break;
    case CSeq_loc::e_Mix:
        s_Fingerprint_Seq_loc_mix(checksum, obj.GetMix(), scope, fingerP);
        break;
    case CSeq_loc::e_Equiv:
        s_Fingerprint_Seq_loc_equiv(checksum, obj.GetEquiv(), scope, fingerP);
        break;
    case CSeq_loc::e_Bond:
        s_Fingerprint_Seq_bond(checksum, obj.GetBond(), scope, fingerP);
        break;
    case CSeq_loc::e_Feat:
        s_Fingerprint_Feat_id(checksum, obj.GetFeat(), scope, fingerP);
        break;
    default:
        break;
    }
}

void init_Seq_locFP()
{
    CObjFingerprint::Register(CSeq_point::GetTypeInfo(), s_Fingerprint_Seq_point);
    CObjFingerprint::Register(CPacked_seqpnt::GetTypeInfo(), s_Fingerprint_Packed_seqpnt);
    CObjFingerprint::Register(CSeq_interval::GetTypeInfo(), s_Fingerprint_Seq_interval);
    CObjFingerprint::Register(CPacked_seqint::GetTypeInfo(), s_Fingerprint_Packed_seqint);
    CObjFingerprint::Register(CSeq_loc_mix::GetTypeInfo(), s_Fingerprint_Seq_loc_mix);
    CObjFingerprint::Register(CSeq_loc_equiv::GetTypeInfo(), s_Fingerprint_Seq_loc_equiv);
    CObjFingerprint::Register(CSeq_bond::GetTypeInfo(), s_Fingerprint_Seq_bond);
    CObjFingerprint::Register(CFeat_id::GetTypeInfo(), s_Fingerprint_Feat_id);
    CObjFingerprint::Register(CSeq_loc::GetTypeInfo(), s_Fingerprint_Seq_loc);
}

END_NCBI_SCOPE
