/* $Id: Seq_literal.cpp 497282 2016-04-05 18:01:48Z kiryutin $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using the following specifications:
 *   'seq.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>

// generated includes
#include <objects/seq/Seq_literal.hpp>
#include <objects/seq/Seq_gap.hpp>
#include <objects/seq/Linkage_evidence.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CSeq_literal::~CSeq_literal(void)
{
}

CSeq_literal::EBridgeableStatus CSeq_literal::GetBridgeability() const
{
    if (!IsSetSeq_data()) {
        return e_MaybeBridgeable;
    }
    if (!GetSeq_data().IsGap()) {
        return e_NotAGap;
    }

    const CSeq_gap &gap = GetSeq_data().GetGap();
    switch (gap.GetType()) {
    case CSeq_gap::eType_short_arm:
    case CSeq_gap::eType_heterochromatin:
    case CSeq_gap::eType_centromere:
    case CSeq_gap::eType_telomere:
        return e_NotBridgeable;
    default:
        break;
    }

    if (IsSetFuzz() ||
        !gap.IsSetLinkage() ||
        gap.GetLinkage() != CSeq_gap::eLinkage_linked ||
        !gap.IsSetLinkage_evidence())
    {
        return e_MaybeBridgeable;
    }

    /// Gap is linked and of known length; bridgeable if has one of the three
    /// needed types of evidence
    ITERATE (CSeq_gap::TLinkage_evidence, it, gap.GetLinkage_evidence()) {
        switch ((*it)->GetType()) {
        case CLinkage_evidence::eType_paired_ends:
        case CLinkage_evidence::eType_within_clone:
        case CLinkage_evidence::eType_pcr:
            return e_Bridgeable;
        default:
            break;
        }
    }
    return e_MaybeBridgeable;
}

END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 57, chars: 1722, CRC32: 4fb51323 */
