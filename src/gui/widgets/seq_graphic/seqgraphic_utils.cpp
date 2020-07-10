/*  $Id: seqgraphic_utils.cpp 42838 2019-04-19 17:24:56Z shkeda $
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
 *
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>

#include <objmgr/seq_annot_handle.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


bool CSGUtils::IsChromosome(const CBioseq_Handle& handle, CScope& scope)
{
    CConstRef<CSeq_id> seq_id = handle.GetSeqId();

    CSeq_id_Handle idh = sequence::GetId(handle.GetSeq_id_Handle(),
        scope, sequence::eGetId_Best);
    if (idh) {
        seq_id = idh.GetSeqId();
    }

    CSeq_id::EAccessionInfo info = seq_id->IdentifyAccession();
    if ((info & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_chromosome) {
        return true;
    }

    CSeqdesc_CI desc_it(handle, CSeqdesc::e_Source);
    while (desc_it) {
        if (desc_it->GetSource().GetGenome() == CBioSource::eGenome_chromosome) {
            return true;
        }
        ++desc_it;
    }

    return false;
}


bool CSGUtils::IsSegSet(const CBioseq_Handle& handle, CScope& scope)
{
    // try to resolve the given seq-id
    CSeq_id_Handle idh = handle.GetSeq_id_Handle();
    idh = sequence::GetId(*idh.GetSeqId(), scope, sequence::eGetId_Best);

    if (handle.IsSetInst_Repr()  &&
        handle.GetInst_Repr() == CSeq_inst::eRepr_seg) {
        return true;
    }

    return false;
}


bool CSGUtils::IsmRNA(const CBioseq_Handle& handle, CScope& scope)
{
    const CMolInfo* info = sequence::GetMolInfo(handle);
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
            return true;
        }
    }

    CConstRef<CSeq_id> seq_id = handle.GetSeqId();
    CSeq_id_Handle idh = sequence::GetId(handle.GetSeq_id_Handle(), scope,
        sequence::eGetId_Best);
    if (idh) {
        seq_id = idh.GetSeqId();
    }

    CSeq_id::EAccessionInfo id_info = seq_id->IdentifyAccession();

    if ((id_info & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_est  ||
        id_info == CSeq_id::eAcc_refseq_mrna  ||
        id_info == CSeq_id::eAcc_refseq_mrna_predicted  ||
        id_info == CSeq_id::eAcc_gpipe_mrna) {
        return true;
    }

    return false;
}

bool CSGUtils::IsRefSeq(const objects::CBioseq_Handle& handle)
{
    CConstRef<CSeq_id> seq_id = handle.GetSeqId();
    // this is needed for cases when handle was originally created with a GI so IdentifyAccession() does not return an e_Gi based on
    // the look of the original ID, so we need to get the "best" representation that will return the real meaning of the thing
    CSeq_id_Handle idh = sequence::GetId(handle, sequence::eGetId_Best);
    if (idh) {
        seq_id = idh.GetSeqId();
    }
    return IsRefSeq(*seq_id);
}

bool CSGUtils::IsRefSeq(const string& seqid)
{
    return IsRefSeq(CSeq_id(seqid));
}

bool CSGUtils::IsRefSeq(const objects::CSeq_id& seqid)
{
    CSeq_id::EAccessionInfo info = seqid.IdentifyAccession();
    return info == CSeq_id::eAcc_refseq_chromosome //NC
        || info == CSeq_id::eAcc_refseq_contig //NT
        || info == CSeq_id::eAcc_refseq_genome //NS
        || info == CSeq_id::eAcc_refseq_genomic //NG
        || info == CSeq_id::eAcc_refseq_mrna //NM
        || info == CSeq_id::eAcc_refseq_mrna_predicted //XM
        || info == CSeq_id::eAcc_refseq_ncrna //NR
        || info == CSeq_id::eAcc_refseq_ncrna_predicted //XR
        || info == CSeq_id::eAcc_refseq_prot  //NP
        || info == CSeq_id::eAcc_refseq_prot_predicted //XP
        || info == CSeq_id::eAcc_refseq_unreserved //AA
        || info == CSeq_id::eAcc_refseq_wgs_intermed
        || info == CSeq_id::eAcc_refseq_wgs_nuc //NZ
        || info == CSeq_id::eAcc_refseq_wgs_prot; // ZP
}


bool CSGUtils::IsMitochondrion(const objects::CBioseq_Handle& handle)
{
    CSeqdesc_CI desc_it(handle, CSeqdesc::e_Source);
    while (desc_it) {
        if (desc_it->GetSource().GetGenome() == CBioSource::eGenome_mitochondrion) {
            return true;
        }
        ++desc_it;
    }

    return false;
}


bool CSGUtils::IsPlastid(const objects::CBioseq_Handle& handle)
{
    CSeqdesc_CI desc_it(handle, CSeqdesc::e_Source);
    while (desc_it) {
        if (desc_it->GetSource().GetGenome() == CBioSource::eGenome_plastid) {
            return true;
        }
        ++desc_it;
    }

    return false;
}


/* Obsolete, use gui/objutils/na_utils:CDataTrackUtils::IsGeneModelFeature
bool CSGUtils::IsMainFeature(int type, int subtype)
{
    return (subtype == CSeqFeatData::eSubtype_exon  ||
        subtype == CSeqFeatData::eSubtype_misc_RNA  ||
        subtype == CSeqFeatData::eSubtype_C_region  ||
        subtype == CSeqFeatData::eSubtype_V_segment ||
        subtype == CSeqFeatData::eSubtype_D_segment ||
        subtype == CSeqFeatData::eSubtype_J_segment ||
        type == CSeqFeatData::e_Gene  ||
        type == CSeqFeatData::e_Cdregion  ||
        type == CSeqFeatData::e_Rna);
}
*/


bool CSGUtils::GetFeatFetchDepth(const objects::CBioseq_Handle& handle,
                                 objects::CScope& scope, int& depth)
{
    if (handle.IsSetDescr()) {
        CSeqdesc_CI desc_it(handle, CSeqdesc::e_User);
        while (desc_it) {
            const CSeqdesc& desc = *desc_it;
            if (desc.IsUser()) {
                const CUser_object& user = desc.GetUser();
                const CObject_id& id = user.GetType();
                if (id.IsStr()  &&  id.GetStr() == "FeatureFetchPolicy") {
                    ITERATE ( CUser_object::TData, fit, user.GetData() ) {
                        const CUser_field& field = **fit;
                        const CObject_id& fid = field.GetLabel();
                        if ( !fid.IsStr() || fid.GetStr() != "Depth" ) {
                            continue;
                        }
                        if ( !field.GetData().IsInt() ) {
                            continue;
                        }
                        depth = field.GetData().GetInt();
                        return true;
                    }
                }
            }
            ++desc_it;
        }
    }
    return false;
}

END_NCBI_SCOPE
