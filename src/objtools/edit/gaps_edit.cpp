/*  $Id: gaps_edit.cpp 598879 2019-12-16 17:02:54Z foleyjp $
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
* Authors:  Sergiy Gotvyanskyy, NCBI
*
* File Description:
*   Converts runs of 'N' or 'n' into a gap
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbiutil.hpp>

#include <objects/seq/Bioseq.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_gap.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/general/Int_fuzz.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_ext.hpp>

#include <util/sequtil/sequtil_convert.hpp>

#include <objtools/edit/gaps_edit.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

namespace
{

bool Make_Iupacna(const CSeq_data& data, string& decoded, TSeqPos len)
{
    CSeqUtil::TCoding src_coding = CSeqUtil::e_not_set;
    CSeqUtil::TCoding dst_coding = CSeqUtil::e_Iupacna;
    CTempString      src;
    switch (data.Which()) {
#define CODING_CASE(x) \
    case CSeq_data::e_##x: \
        src.assign(&data.Get##x().Get()[0], data.Get##x().Get().size()); \
        src_coding = CSeqUtil::e_##x; \
        CSeqConvert::Convert(src, src_coding, 0, len, decoded,  dst_coding); \
        break;
    CODING_CASE(Ncbi2na)
    CODING_CASE(Iupacna)
    CODING_CASE(Iupacaa)
    CODING_CASE(Ncbi4na)
    CODING_CASE(Ncbi8na)
    CODING_CASE(Ncbi8aa)
    CODING_CASE(Ncbieaa)
    CODING_CASE(Ncbistdaa)
#undef CODING_CASE
    default:
//        ERR_POST_X(1, Warning << "PackWithGaps: unsupported encoding "
//                      << CSeq_data::SelectionName(data.Which()));
        return false;
    }
    return true;
}

   
CRef<CDelta_seq> MakeGap(const CSeq_data& data, TSeqPos len, CDelta_ext& ext,  TSeqPos gap_start, TSeqPos gap_length)
{
    string decoded;
    if (!Make_Iupacna(data, decoded, len))
        return CRef<CDelta_seq>();

    if (gap_start>0)
    {
        ext.AddAndSplit(CTempString(decoded, 0, gap_start), CSeq_data::e_Iupacna, gap_start, true, true);
    }

    // here we need to check if segment is gap in fact
    CDelta_seq& gap = ext.AddLiteral(gap_length);

    if (gap_start+gap_length < decoded.length())
    {
        ext.AddAndSplit(
            CTempString(decoded, gap_start+gap_length, TSeqPos(decoded.length())-gap_start-gap_length), 
            CSeq_data::e_Iupacna, TSeqPos(decoded.length())-gap_start-gap_length, true, true);
    }

    return CRef<CDelta_seq>(&gap);
}

CDelta_seq& CloneLiteral(const CDelta_seq& delta, CDelta_ext& ext, TSeqPos len)
{
    CDelta_seq& new_delta = ext.AddLiteral(len);
    if (delta.GetLiteral().IsSetSeq_data())
        new_delta.SetLiteral().SetSeq_data().Assign(delta.GetLiteral().GetSeq_data());

    return new_delta;
}

CRef<CDelta_seq> MakeGap(const CDelta_seq& delta, TSeqPos len, CDelta_ext& ext,  TSeqPos gap_start, TSeqPos gap_length)
{
    if (delta.IsLiteral() && delta.GetLiteral().IsSetSeq_data())
       return MakeGap(delta.GetLiteral().GetSeq_data(), len, ext, gap_start, gap_length);
    else
    {        
        if (gap_start>0)
        {
            CloneLiteral(delta, ext, gap_start);
        }       
        CDelta_seq& new_gap = ext.AddLiteral(gap_length);
        if (len>gap_start+gap_length)
            CloneLiteral(delta, ext, len-gap_start-gap_length);
        return CRef<CDelta_seq>(&new_gap);
    }
}


CRef<CDelta_seq> MakeGap(CBioseq::TInst& inst, TSeqPos gap_start, TSeqPos gap_length)
{
    if (inst.IsAa())
        return CRef<CDelta_seq>();

    if (inst.IsSetExt()) {
        CDelta_ext& ext = inst.SetExt().SetDelta();

        TSeqPos current = 0;
        // let's find addressed literal
        NON_CONST_ITERATE(CDelta_ext::Tdata, it, ext.Set())
        {
            CRef<CDelta_seq> delta = *it;
            if (delta->IsLiteral())
            {
                // literal without length specified break the locup
                if (!delta->GetLiteral().IsSetLength())
                    return CRef<CDelta_seq>();

                TSeqPos lit_len = delta->GetLiteral().GetLength(); 

                if (gap_start < lit_len)
                {
                    if (gap_start == 0 && lit_len == gap_length) // && !delta->GetLiteral().IsSetSeq_data())
                        return delta; // simply return the delta, don't create new
                    else
                    {
                        CDelta_ext new_ext; // temporal container
                        CRef<CDelta_seq> gap_seq = MakeGap(*delta, lit_len, new_ext, gap_start, gap_length);
                        if (gap_seq)
                        {
                            // replace the current delta with created 
                            it = ext.Set().erase(it);
                            ext.Set().insert(it, new_ext.Set().begin(), new_ext.Set().end());
                        }
                        return gap_seq;
                    }
                }
                current += lit_len;
                if (lit_len > gap_start)
                    return CRef<CDelta_seq>();
                gap_start -= lit_len;
            }
            else
            {
                return CRef<CDelta_seq>();
            }
        }
    }
    else 
    if (inst.IsSetSeq_data())
    {
        // convert simple sequence into delta seq
        const CSeq_data& data = inst.GetSeq_data();

        CDelta_ext& ext = inst.SetExt().SetDelta();

        CRef<CDelta_seq> delta = MakeGap(data, inst.GetLength(), ext, gap_start, gap_length);

        // finalize, if delta seq was not successfull
        // revert to the original 
        if (ext.Get().size() > 1) {
            inst.SetRepr(CSeq_inst::eRepr_delta);
            inst.ResetSeq_data();
        } else { // roll back
            inst.ResetExt();
        }
        return delta;
    }

    return CRef<CDelta_seq>();

}

}

CGapsEditor::CGapsEditor(CSeq_gap::EType gap_type, const TEvidenceSet& evidences,
    TSeqPos gapNmin, TSeqPos gap_Unknown_length) :
    m_gap_type(gap_type), 
    m_DefaultEvidence(evidences), 
    m_gapNmin(gapNmin), 
    m_gap_Unknown_length(gap_Unknown_length)
{
}


CGapsEditor::CGapsEditor(CSeq_gap::EType gap_type, 
        const TEvidenceSet& defaultEvidence,
        const TCountToEvidenceMap& countToEvidenceMap,
        TSeqPos gapNmin, 
        TSeqPos gap_Unknown_length)
    :
    m_gap_type(gap_type), 
    m_DefaultEvidence(defaultEvidence), 
    m_GapsizeToEvidenceMap(countToEvidenceMap), 
    m_gapNmin(gapNmin), 
    m_gap_Unknown_length(gap_Unknown_length)
{
}
        

CRef<CDelta_seq> 
CGapsEditor::CreateGap(CBioseq& bioseq, TSeqPos gap_start, TSeqPos gap_length)
{
    if (!bioseq.IsSetInst())
        return CRef<CDelta_seq>();

    CRef<CDelta_seq> seq = MakeGap(bioseq.SetInst(), gap_start, gap_length);
    if (seq.NotEmpty())
    {
        seq->SetLiteral();
        x_SetGapParameters(*seq);
    }
    return seq;
}

void  CGapsEditor::ConvertNs2Gaps(const CSeq_data& data, TSeqPos len, CDelta_ext& ext)
{
    string decoded;
    if (!Make_Iupacna(data, decoded, len))
        return;

    {
        size_t index = 0;
        CTempString current(decoded);
        size_t start;
        while (index + m_gapNmin <= current.length() && ((start = current.find_first_of("Nn", index)) != CTempString::npos))
        {
            size_t end = current.find_first_not_of("Nn", start);
            if (end == CTempString::npos)
                end = current.length();
            if (end - start >= m_gapNmin)
            {
                if (start > 0)
                    ext.AddAndSplit(current, CSeq_data::e_Iupacna, TSeqPos(start), false, true);

                CDelta_seq& gap = ext.AddLiteral(TSeqPos(end-start));
                x_SetGapParameters(gap);
                current.assign(current.data(), end, current.length() - end);
                end = 0;
            }
            index = end;
        }
        if (current.length() > 0)
            ext.AddAndSplit(current, CSeq_data::e_Iupacna, TSeqPos(current.length()), false, true);
    }
}

void CGapsEditor::ConvertNs2Gaps(CBioseq::TInst& inst)
{
    if (inst.IsAa()  ||  !inst.IsSetSeq_data()  ||  inst.IsSetExt()) {
        return;
    }
    const CSeq_data& data = inst.GetSeq_data();

    CDelta_ext& ext = inst.SetExt().SetDelta();

    ConvertNs2Gaps(data, inst.GetLength(), ext);

    if (ext.Get().size() > 1) { // finalize
        inst.SetRepr(CSeq_inst::eRepr_delta);
        inst.ResetSeq_data();
    } else { // roll back
        inst.ResetExt();
    }
}

void CGapsEditor::ConvertNs2Gaps(CBioseq& bioseq)
{
    if (bioseq.IsSetInst() && bioseq.GetInst().IsSetSeq_data() && !bioseq.GetInst().GetSeq_data().IsGap())
    {
        ConvertNs2Gaps(bioseq.SetInst());
    }


    if (!bioseq.IsSetInst() || !bioseq.GetInst().IsSetExt() || !bioseq.GetInst().GetExt().IsDelta())
    {
        return;
    }

    CSeq_inst& inst = bioseq.SetInst();

    // since delta functions allows adding new literal to an end we create a copy of literal array 
    // to iterate over

    CDelta_ext::Tdata src_data = inst.GetExt().GetDelta().Get();
    CDelta_ext& dst_data = inst.SetExt().SetDelta();
    dst_data.Set().clear();

    NON_CONST_ITERATE(CDelta_ext::Tdata, it, src_data)
    {
        if (!(**it).IsLiteral())
        {
            dst_data.Set().push_back(*it);
            continue;
        }

        CDelta_seq::TLiteral& lit = (**it).SetLiteral();

        // split a literal to elements if needed
        if (lit.IsSetSeq_data() && !lit.GetSeq_data().IsGap())
        {
            ConvertNs2Gaps(lit.GetSeq_data(), lit.GetLength(), dst_data);
        }
        else
        {
            // otherwise add it as is and possible update some parameters
            dst_data.Set().push_back(*it);
            x_SetGapParameters(**it);
        }
    }
}

void CGapsEditor::x_SetGapParameters(CDelta_seq& lit)
{
    CDelta_seq::TLiteral& gap = lit.SetLiteral();
    if (gap.IsSetLength() && gap.GetLength() == m_gap_Unknown_length)
    {
        gap.SetFuzz().SetLim(CInt_fuzz::eLim_unk);
    }

    if (gap.IsSetSeq_data() && 
        gap.GetSeq_data().IsGap() && 
        gap.GetSeq_data().GetGap().GetLinkage_evidence().size() > 0)
        return;

    if (!m_DefaultEvidence.empty() || !m_GapsizeToEvidenceMap.empty()) {
        const auto len = gap.GetLength();
        const auto it = m_GapsizeToEvidenceMap.find(len);
        const auto& evidenceSet = 
            (it != m_GapsizeToEvidenceMap.end()) ?
            it->second :
            m_DefaultEvidence;

        if (evidenceSet.empty()) {
            return;
        }

        for (const auto& evidence : evidenceSet) {
            auto pEvidence = Ref(new CLinkage_evidence());
            pEvidence->SetType(evidence);
            gap.SetSeq_data().SetGap().SetLinkage_evidence().emplace_back(move(pEvidence));
        }
        gap.SetSeq_data().SetGap().SetLinkage(CSeq_gap::eLinkage_linked);
        gap.SetSeq_data().SetGap().SetType(m_gap_type);
    }
}

void CGapsEditor::ConvertNs2Gaps(objects::CSeq_entry& entry)
{
    if (m_gapNmin == 0 && m_gap_Unknown_length > 0)
        return;

    switch(entry.Which())
    {
    case CSeq_entry::e_Seq:
        {
            ConvertNs2Gaps(entry.SetSeq());
        }
        break;
    case CSeq_entry::e_Set:
        NON_CONST_ITERATE(CSeq_entry::TSet::TSeq_set, it, entry.SetSet().SetSeq_set())
        {
            ConvertNs2Gaps(**it);
        }
        break;
    default:
        break;
    }
}

void CGapsEditor::ConvertBioseqToDelta(CBioseq& bioseq)
{
    TSeqPos len = bioseq.GetInst().GetLength();
    CDelta_ext& delta_ext = bioseq.SetInst().SetExt().SetDelta();
    CRef<CDelta_seq> delta_seq(new CDelta_seq);
    delta_seq->SetLiteral().SetSeq_data(bioseq.SetInst().SetSeq_data());
    delta_seq->SetLiteral().SetLength(len);
    delta_ext.Set().push_back(delta_seq);
    bioseq.SetInst().ResetSeq_data();
    bioseq.SetInst().SetRepr(CSeq_inst::eRepr_delta);
}

void CGapsEditor::AppendGap(CBioseq& bioseq)
{
    CRef<CDelta_seq> delta_seq(new CDelta_seq);
    CDelta_seq::TLiteral& lit = delta_seq->SetLiteral();
    lit.SetLength(0);
    x_SetGapParameters(*delta_seq);
    const TSeqPos len = 100;
    lit.SetLength(len);
    bioseq.SetInst().SetExt().SetDelta().Set().push_back(delta_seq);
    bioseq.SetInst().SetLength() += len;
}

void CGapsEditor::AddBioseqAsLiteral(CBioseq& parent, CBioseq& bioseq)
{
    auto& delta_set = parent.SetInst().SetExt().SetDelta().Set();
    if (!delta_set.empty() && delta_set.back()->GetLiteral().IsSetSeq_data())
    {
        AppendGap(parent);
    }
    if (bioseq.GetInst().IsSetExt())
    {
        delta_set.splice(delta_set.end(), bioseq.SetInst().SetExt().SetDelta().Set());
    }
    else {
        CRef<CDelta_seq> delta_seq(new CDelta_seq);
        delta_seq->SetLiteral().SetSeq_data(bioseq.SetInst().SetSeq_data());
        delta_seq->SetLiteral().SetLength(bioseq.GetInst().GetLength());
        delta_set.push_back(delta_seq);
    }
    parent.SetInst().SetLength() += bioseq.GetLength();
}

END_SCOPE(objects)
END_NCBI_SCOPE
