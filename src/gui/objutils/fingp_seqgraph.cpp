/*  $Id: fingp_seqgraph.cpp 27635 2013-03-15 19:33:00Z katargir@NCBI.NLM.NIH.GOV $
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

#include <objects/seqres/Byte_graph.hpp>
#include <objects/seqres/Int_graph.hpp>
#include <objects/seqres/Real_graph.hpp>
#include <objects/seqres/Seq_graph.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static void s_Fingerprint_Byte_graph(
                             CChecksum& checksum, const CObject& object,
                             CScope*, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CByte_graph*>(&object));
    const CByte_graph& obj = static_cast<const CByte_graph&>(object);

    CKSUM_INTEGER_FIELD2(checksum, CByte_graph, Max);
    CKSUM_INTEGER_FIELD2(checksum, CByte_graph, Min);
    CKSUM_INTEGER_FIELD2(checksum, CByte_graph, Axis);

    CByte_graph::TValues values = obj.GetValues();

    if (values.size() > 0)
        checksum.AddLine(&values[0], values.size());
}

static void s_Fingerprint_Int_graph(
                             CChecksum& checksum, const CObject& object,
                             CScope*, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CInt_graph*>(&object));
    const CInt_graph& obj = static_cast<const CInt_graph&>(object);

    CKSUM_INTEGER_FIELD2(checksum, CInt_graph, Max);
    CKSUM_INTEGER_FIELD2(checksum, CInt_graph, Min);
    CKSUM_INTEGER_FIELD2(checksum, CInt_graph, Axis);

    ITERATE(CInt_graph::TValues, it, obj.GetValues())
        CKSUM_ADD_INTEGER(checksum, *it);
}

static void s_Fingerprint_Real_graph(
                             CChecksum& checksum, const CObject& object,
                             CScope*, const CObjFingerprint&)
{
    _ASSERT(dynamic_cast<const CReal_graph*>(&object));
    const CReal_graph& obj = static_cast<const CReal_graph&>(object);

    CObjFingerprint::AddDouble(checksum, obj.GetMax());
    CObjFingerprint::AddDouble(checksum, obj.GetMin());
    CObjFingerprint::AddDouble(checksum, obj.GetAxis());

    ITERATE(CReal_graph::TValues, it, obj.GetValues())
        CObjFingerprint::AddDouble(checksum, *it);
}

static void s_Fingerprint_Seq_graph(
                             CChecksum& checksum, const CObject& object,
                             CScope* scope, const CObjFingerprint& fingerP)
{
    _ASSERT(dynamic_cast<const CSeq_graph*>(&object));
    const CSeq_graph& obj = static_cast<const CSeq_graph&>(object);

    if (obj.IsSetTitle()) checksum.AddLine(obj.GetTitle());
    if (obj.IsSetComment()) checksum.AddLine(obj.GetComment());
    fingerP.Fingerprint(checksum, obj.GetLoc(), scope);
    if (obj.IsSetTitle_x()) checksum.AddLine(obj.GetTitle_x());
    if (obj.IsSetTitle_y()) checksum.AddLine(obj.GetTitle_y());
    CKSUM_INTEGER_FIELD(checksum, CSeq_graph, Comp);
    if (obj.IsSetA()) CObjFingerprint::AddDouble(checksum, obj.GetA());
    if (obj.IsSetB()) CObjFingerprint::AddDouble(checksum, obj.GetB());
    CKSUM_INTEGER_FIELD(checksum, CSeq_graph, Numval);

    const CSeq_graph::TGraph& graph = obj.GetGraph();
    switch(graph.Which()) {
    case CSeq_graph::TGraph::e_Real :
        s_Fingerprint_Byte_graph(checksum, graph.GetReal(), scope, fingerP);
        break;
    case CSeq_graph::TGraph::e_Int :
        s_Fingerprint_Byte_graph(checksum, graph.GetInt(), scope, fingerP);
        break;
    case CSeq_graph::TGraph::e_Byte :
        s_Fingerprint_Byte_graph(checksum, graph.GetByte(), scope, fingerP);
        break;
    default:
        break;
    }
}

void init_Seq_graphFP()
{
    CObjFingerprint::Register(CByte_graph::GetTypeInfo(), s_Fingerprint_Byte_graph);
    CObjFingerprint::Register(CInt_graph::GetTypeInfo(), s_Fingerprint_Int_graph);
    CObjFingerprint::Register(CReal_graph::GetTypeInfo(), s_Fingerprint_Real_graph);
    CObjFingerprint::Register(CSeq_graph::GetTypeInfo(), s_Fingerprint_Seq_graph);
}

END_NCBI_SCOPE

