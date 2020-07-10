/*  $Id: fingp_seqfeat.cpp 34076 2015-10-30 12:14:08Z evgeniev $
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
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/label.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/general/Dbtag.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static void s_Fingerprint_Gb_qual(
                             CChecksum& checksum, const CObject& object,
                             CScope*, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CGb_qual*>(&object));
    const CGb_qual& obj = static_cast<const CGb_qual&>(object);

    checksum.AddLine(obj.GetQual());
    checksum.AddLine(obj.GetVal());
}


static void s_Fingerprint_Feat_id(CChecksum& checksum, const CFeat_id& feat_id,
                                  CScope* scope, const CObjFingerprint& fingerP)
{
    switch(feat_id.Which()) {
            case CFeat_id::e_Gibb:
                {{
                    CFeat_id::TGibb gibb = feat_id.GetGibb();
                    CKSUM_ADD_INTEGER(checksum, gibb);
                    break;
                }}
            case CFeat_id::e_Giim:
                fingerP.Fingerprint(checksum, feat_id.GetGiim(), scope);
                break;
            case CFeat_id::e_Local:
                fingerP.Fingerprint(checksum, feat_id.GetLocal(), scope);
                break;
            case CFeat_id::e_General:
                fingerP.Fingerprint(checksum, feat_id.GetGeneral(), scope);
                break;
            default:
                break;
    }
}


static void s_Fingerprint_Seq_feat(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_feat*>(&object));
    const CSeq_feat& obj = static_cast<const CSeq_feat&>(object);

    const CSeqFeatData& data = obj.GetData();
    CSeqFeatData::E_Choice choice = data.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    fingerP.Fingerprint(checksum, obj.GetLocation(), scope);

    if (obj.IsSetId()) {
        s_Fingerprint_Feat_id(checksum, obj.GetId(), scope, fingerP);
    } else {
        // let's use label as the 'unique' id instead
        string label;
        CLabel::GetLabel(object, &label, CLabel::eContent);
        if ( !label.empty() ) {
            checksum.AddLine(label);
        }
    }

    if (obj.IsSetIds()) {
        ITERATE (CSeq_feat::TIds, iter, obj.GetIds()) {
            s_Fingerprint_Feat_id(checksum, **iter, scope, fingerP);
        }
    }

    if (obj.IsSetProduct())
        fingerP.Fingerprint(checksum, obj.GetProduct(), scope);

    // some STS features are placed on the wrong sequence,
    // causing a dublicate fingerprints
    if (data.GetSubtype() == CSeqFeatData::eSubtype_STS) {
        if (obj.IsSetQual()) {
            ITERATE (CSeq_feat::TQual, it, obj.GetQual())
                s_Fingerprint_Gb_qual(checksum, **it, scope, fingerP);
        }
        if (obj.IsSetDbxref()) {
            ITERATE (CSeq_feat::TDbxref, it, obj.GetDbxref())
                fingerP.Fingerprint(checksum, **it, scope);
        }
        if (obj.IsSetComment())
            checksum.AddLine(obj.GetComment());
    }

    // stream dbxref's for variations
    if (((data.GetSubtype() == CSeqFeatData::eSubtype_site) || (data.GetSubtype() == CSeqFeatData::eSubtype_variation)) && obj.IsSetDbxref()) {
        ITERATE (CSeq_feat::TDbxref, it, obj.GetDbxref())
            fingerP.Fingerprint(checksum, **it, scope);
    }

    if (data.GetSubtype() == CSeqFeatData::eSubtype_misc_feature) {
        if (obj.IsSetQual()) {
            ITERATE (CSeq_feat::TQual, it, obj.GetQual())
                s_Fingerprint_Gb_qual(checksum, **it, scope, fingerP);
        }
    }
}

void init_Seq_FeatFP()
{
    CObjFingerprint::Register(CSeq_feat::GetTypeInfo(), s_Fingerprint_Seq_feat);
    CObjFingerprint::Register(CGb_qual::GetTypeInfo(), s_Fingerprint_Gb_qual);
}

END_NCBI_SCOPE
