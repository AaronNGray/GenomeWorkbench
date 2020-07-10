/*  $Id: fingp_seqid.cpp 27635 2013-03-15 19:33:00Z katargir@NCBI.NLM.NIH.GOV $
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
 *  Government do not and cannot warrant the performance or results
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

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/biblio/Id_pat.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Giimport_id.hpp>
#include <objects/seqloc/Patent_seq_id.hpp>
#include <objects/seqloc/PDB_seq_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static void s_Fingerprint_Object_id(
                             CChecksum& checksum, const CObject& object,
                             CScope* /*scope*/, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CObject_id*>(&object));
    const CObject_id& obj = static_cast<const CObject_id&>(object);

    CObject_id::E_Choice choice = obj.Which();
    CKSUM_ADD_INTEGER(checksum, choice);
    switch(choice) {
    case CObject_id::e_Id:
        {
            CObject_id::TId id = obj.GetId();
            CKSUM_ADD_INTEGER(checksum, id);
        }
        break;
    case CObject_id::e_Str:
        checksum.AddLine(obj.GetStr());
        break;
    case CObject_id::e_not_set:
        break;
    }
}

static void s_Fingerprint_Dbtag(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CDbtag*>(&object));
    const CDbtag& obj = static_cast<const CDbtag&>(object);

    checksum.AddLine(obj.GetDb());
    fingerP.Fingerprint(checksum, obj.GetTag(), scope);
}

static void s_Fingerprint_Giimport_id(
                             CChecksum& checksum, const CObject& object,
                             CScope* /*scope*/, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CGiimport_id*>(&object));
    const CGiimport_id& obj = static_cast<const CGiimport_id&>(object);

    CGiimport_id::TId id = obj.GetId();
    CKSUM_ADD_INTEGER(checksum, id);

    if (obj.IsSetDb())
        checksum.AddLine(obj.GetDb());
    if (obj.IsSetRelease())
        checksum.AddLine(obj.GetRelease());
}

static void s_Fingerprint_Textseq_id(
                             CChecksum& checksum, const CObject& object,
                             CScope* /*scope*/, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CTextseq_id*>(&object));
    const CTextseq_id& obj = static_cast<const CTextseq_id&>(object);

    if (obj.IsSetAccession())
        checksum.AddLine(obj.GetAccession());
    if (obj.IsSetRelease())
        checksum.AddLine(obj.GetRelease());

    CTextseq_id::TVersion version = obj.CanGetVersion() ? obj.GetVersion() : 1;
    CKSUM_ADD_INTEGER(checksum, version);
}

static void s_Fingerprint_Patent_seq_id(
                             CChecksum& checksum, const CObject& object,
                             CScope* /*scope*/, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CPatent_seq_id*>(&object));
    const CPatent_seq_id& obj = static_cast<const CPatent_seq_id&>(object);

    const CId_pat& cit = obj.GetCit();

    checksum.AddLine(cit.GetCountry());

    const CId_pat::C_Id& id = cit.GetId();
    CId_pat::C_Id::E_Choice choice = id.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    switch(choice) {
    case CId_pat::C_Id::e_Number:
        checksum.AddLine(id.GetNumber());
        break;
    case CId_pat::C_Id::e_App_number:
        checksum.AddLine(id.GetApp_number());
        break;
    case CId_pat::C_Id::e_not_set:
        break;
    }

    CPatent_seq_id::TSeqid seqid = obj.GetSeqid();
    CKSUM_ADD_INTEGER(checksum, seqid);
}

static void s_Fingerprint_PDB_seq_id(
                             CChecksum& checksum, const CObject& object,
                             CScope* /*scope*/, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CPDB_seq_id*>(&object));
    const CPDB_seq_id& obj = static_cast<const CPDB_seq_id&>(object);

    checksum.AddLine(obj.GetMol());
    CPDB_seq_id::TChain chain = obj.GetChain();
    CKSUM_ADD_INTEGER(checksum, chain);
}

static void s_Fingerprint_Seq_id(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_id*>(&object));
    CSeq_id_Handle idh;
    {{
        const CSeq_id& obj = static_cast<const CSeq_id&>(object);
        idh = sequence::GetId(obj, *scope, sequence::eGetId_Seq_id_BestRank);
    }}
    if ( !idh ) return;

    CConstRef<CSeq_id> seq_id = idh.GetSeqId();
    const CSeq_id& obj = *seq_id;

    CSeq_id::E_Choice choice = obj.Which();
    CKSUM_ADD_INTEGER(checksum, choice);

    switch(choice) {
    case CSeq_id::e_Local:
        s_Fingerprint_Object_id(checksum, obj.GetLocal(), scope, fingerP);
        break;
    case CSeq_id::e_Gibbsq:
        {
            CSeq_id::TGibbsq value = obj.GetGibbsq();
            CKSUM_ADD_INTEGER(checksum, value);
        }
        break;
    case CSeq_id::e_Gibbmt:
        {
            CSeq_id::TGibbmt value = obj.GetGibbmt();
            CKSUM_ADD_INTEGER(checksum, value);
        }
        break;
    case CSeq_id::e_Giim:
        s_Fingerprint_Giimport_id(checksum, obj.GetGiim(), scope, fingerP);
        break;
    case CSeq_id::e_Genbank:
        s_Fingerprint_Textseq_id(checksum, obj.GetGenbank(), scope, fingerP);
        break;
    case CSeq_id::e_Embl:
        s_Fingerprint_Textseq_id(checksum, obj.GetEmbl(), scope, fingerP);
        break;
    case CSeq_id::e_Pir:
        s_Fingerprint_Textseq_id(checksum, obj.GetPir(), scope, fingerP);
        break;
    case CSeq_id::e_Swissprot:
        s_Fingerprint_Textseq_id(checksum, obj.GetSwissprot(), scope, fingerP);
        break;
    case CSeq_id::e_Patent:
        s_Fingerprint_Patent_seq_id(checksum, obj.GetPatent(), scope, fingerP);
        break;
    case CSeq_id::e_Other:
        s_Fingerprint_Textseq_id(checksum, obj.GetOther(), scope, fingerP);
        break;
    case CSeq_id::e_General:
        s_Fingerprint_Dbtag(checksum, obj.GetGeneral(), scope, fingerP);
        break;
    case CSeq_id::e_Gi:
        {
            CSeq_id::TGi value = obj.GetGi();
            CKSUM_ADD_INTEGER(checksum, value);
        }
        break;
    case CSeq_id::e_Ddbj:
        s_Fingerprint_Textseq_id(checksum, obj.GetDdbj(), scope, fingerP);
        break;
    case CSeq_id::e_Prf:
        s_Fingerprint_Textseq_id(checksum, obj.GetPrf(), scope, fingerP);
        break;
    case CSeq_id::e_Pdb:
        s_Fingerprint_PDB_seq_id(checksum, obj.GetPdb(), scope, fingerP);
        break;
    case CSeq_id::e_Tpg:
        s_Fingerprint_Textseq_id(checksum, obj.GetTpg(), scope, fingerP);
        break;
    case CSeq_id::e_Tpe:
        s_Fingerprint_Textseq_id(checksum, obj.GetTpe(), scope, fingerP);
        break;
    case CSeq_id::e_Tpd:
        s_Fingerprint_Textseq_id(checksum, obj.GetTpd(), scope, fingerP);
        break;
    case CSeq_id::e_Gpipe:
        s_Fingerprint_Textseq_id(checksum, obj.GetGpipe(), scope, fingerP);
        break;
    case CSeq_id::e_not_set:
    default:
        break;
    }
}

void init_Seq_idFP()
{
    CObjFingerprint::Register(CObject_id::GetTypeInfo(), s_Fingerprint_Object_id);
    CObjFingerprint::Register(CDbtag::GetTypeInfo(), s_Fingerprint_Dbtag);
    CObjFingerprint::Register(CGiimport_id::GetTypeInfo(), s_Fingerprint_Giimport_id);
    CObjFingerprint::Register(CTextseq_id::GetTypeInfo(), s_Fingerprint_Textseq_id);
    CObjFingerprint::Register(CPatent_seq_id::GetTypeInfo(), s_Fingerprint_Patent_seq_id);
    CObjFingerprint::Register(CPDB_seq_id::GetTypeInfo(), s_Fingerprint_PDB_seq_id);
    CObjFingerprint::Register(CSeq_id::GetTypeInfo(), s_Fingerprint_Seq_id);
}

END_NCBI_SCOPE

