/*  $Id: expand_gaps.cpp 43410 2019-06-25 19:30:04Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_vector.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <gui/packages/pkg_sequence_edit/expand_gaps.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

bool CExpandGaps::IsSkipGap(const CDelta_seq& seg)
{
    return seg.IsLiteral() && seg.GetLiteral().IsSetFuzz();
}

void CExpandGaps::AppendLiteral(CDelta_ext &delta, const string& iupac)
{
    if (delta.IsSet() && !delta.Get().empty() && delta.Get().back()->IsLiteral() && delta.Get().back()->GetLiteral().IsSetSeq_data() 
        && delta.Get().back()->GetLiteral().GetSeq_data().IsIupacna())
    {
        string seq = delta.Get().back()->GetLiteral().GetSeq_data().GetIupacna().Get();
        seq += iupac;
        delta.Set().back()->SetLiteral().SetSeq_data().SetIupacna().Set().assign(seq.data(), seq.length());
        delta.Set().back()->SetLiteral().SetLength((CSeq_literal::TLength)seq.size());
    }
    else
    {
        delta.AddLiteral(iupac, CSeq_inst::eMol_na, false); 
    }
}

CRef<CCmdComposite> CExpandGaps::apply(objects::CSeq_entry_Handle tse)
{
    if (!tse) 
        return CRef<CCmdComposite>(NULL);

    CRef<CCmdComposite> composite( new CCmdComposite("Expand Known Gaps to Include Flanking Ns") );   
    for (CBioseq_CI bi(tse, CSeq_inst::eMol_na); bi; ++bi)
    {
        CBioseq_Handle bsh = *bi;
        TSeqPos len = bsh.GetBioseqLength();
        if (len <= 1)
            continue;
        if (!bsh.IsSetInst())
            continue;

        const CSeq_inst &inst = bsh.GetInst();
        if (!inst.IsSetExt() || !inst.GetExt().IsDelta())
            continue;
        bool has_loc(false);

        ITERATE(CDelta_ext::Tdata, iter, inst.GetExt().GetDelta().Get()) 
        {
            if ( (*iter)->IsLoc() ) 
            {
                has_loc = true;
                break;
            }
        }
        if (has_loc)
            continue;
        
        CRef<CSeq_inst> new_inst(new CSeq_inst);
        new_inst->Assign(inst);
        new_inst->ResetExt();
        bool modified(false);
        try 
        {
            TSeqPos pos = 0;
            int prev_finish = 0;
            CSeqVector sv = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
            ITERATE (CDelta_ext::Tdata, delta_i, inst.GetExt().GetDelta().Get()) 
            {
                if (delta_i->Empty()) 
                    continue; // Ignore NULLs, reported separately above.
                const CDelta_seq& seg = **delta_i;
                size_t delta_len = seg.GetLiteral().GetLength();
                if (sv.IsInGap (pos) && !IsSkipGap(seg)) 
                {
                    int cur_finish = pos + delta_len;
                    while (cur_finish < len && sv[cur_finish] == 'N' && !sv.IsInGap(cur_finish)) 
                    {
                        cur_finish++;
                    }
                    cur_finish--;
                    int length = cur_finish - prev_finish + 1;
                    modified |= (length != delta_len);
                    if (length > 0)
                    {
                        CRef<CDelta_seq> new_seg(new CDelta_seq());
                        new_seg->Assign(seg);
                        new_seg->SetLiteral().SetLength(length);
                        new_inst->SetExt().SetDelta().Set().push_back(new_seg);
                    }
                    prev_finish = cur_finish + 1;
                }
                else if (sv.IsInGap (pos) && IsSkipGap(seg)) 
                {
                    if (pos > prev_finish)
                    {
                        string iupac;
                        sv.GetSeqData(prev_finish, pos, iupac);
                        AppendLiteral(new_inst->SetExt().SetDelta(), iupac);
                    }
                    CRef<CDelta_seq> new_seg(new CDelta_seq());
                    new_seg->Assign(seg);
                    new_inst->SetExt().SetDelta().Set().push_back(new_seg);
                    prev_finish = pos + delta_len;
                }
                else
                {
                    int cur_finish = pos + delta_len - 1;
                    while (cur_finish >= prev_finish  && sv[cur_finish] == 'N' && !sv.IsInGap(cur_finish)) 
                    {
                        cur_finish--;
                    }
                    cur_finish++;
                    if (cur_finish > prev_finish)
                    {
                        string iupac;
                        sv.GetSeqData(prev_finish, cur_finish, iupac);
                        new_inst->SetExt().SetDelta().AddLiteral(iupac, CSeq_inst::eMol_na, false);
                    }
                    prev_finish = cur_finish;
                }             
                pos += delta_len;
            }
            
        } catch (CException ) 
        {
            modified = false;
        } 
        catch (std::exception ) 
        {
            modified = false;
        }

        if (modified)
        {
            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, *new_inst));
            composite->AddCommand(*cmd);
        }
    }
 
    return composite;
}


END_NCBI_SCOPE
