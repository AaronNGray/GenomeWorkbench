/*  $Id: trim_n.cpp 45004 2020-05-06 14:47:12Z asztalos $
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
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/cmd_change_graph.hpp>
#include <gui/objutils/cmd_del_seq_graph.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>

#include <wx/string.h>
#include <gui/objutils/report_trim.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp> 
#include <gui/packages/pkg_sequence_edit/trim_n.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

bool CTrimN::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, wxWindow* parent,  const wxString& workDir, const CSequenceAmbigTrimmer::TTrimRuleVec &trim_rules)
{
    if (tse) {
        CScope &scope = tse.GetScope();        
        map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > product_to_cds;
        GetProductToCDSMap(scope, product_to_cds);
        CRef<CObjectManager> object_manager = CObjectManager::GetInstance();
        CRef<CScope> scope_copy(new CScope(*object_manager));
  

        CRef<CCmdComposite> composite(new CCmdComposite(title));    
        CReportTrim trimmer(CSequenceAmbigTrimmer::eMeaningOfAmbig_OnlyCompletelyUnknown,
                            0, // TODO - put fFlags_TrimAnnot here when it becomes available
                            trim_rules,   
                            50 );   
        wxString msg;
        CBioseq_CI bi(tse, CSeq_inst::eMol_na);
        while (bi) 
        {
            CBioseq_Handle bsh = *bi;
            CRef<CBioseq> bseq( new CBioseq );
            bseq->Assign(*bsh.GetCompleteBioseq());

            CBioseq_Handle edited_bsh = scope_copy->AddBioseq(*bseq);

            TSignedSeqPos from = 0;
            TSignedSeqPos to = bsh.GetInst_Length() - 1;
            CSequenceAmbigTrimmer::EResult eTrimResult = CSequenceAmbigTrimmer::eResult_NoTrimNeeded;
            try
            {
                eTrimResult = trimmer.Trim( edited_bsh, from, to );
            }
            catch(const exception&) {}
            
            if (eTrimResult == CSequenceAmbigTrimmer::eResult_SuccessfullyTrimmed)
            {
                int length = edited_bsh.GetInst_Length();
                string label;
                bsh.GetSeqId()->GetLabel(&label);
                if (from == 0 && to == bsh.GetInst_Length() - 1)
                {
                    composite->AddCommand(*GetDeleteSequenceCommand(bsh));

                    msg << label << ": removed\n";
                }
                else
                {
                    CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, bseq->SetInst()));
                    composite->AddCommand(*cmd);
              
                    if (from != 0)
                        msg << label << ":1-" << from  << "\n";
                    if (to != bsh.GetInst_Length() - 1)
                        msg << label << ":" << to + 2 << "-" << bsh.GetInst_Length() << "\n";
               
                    // Here is a trick - watch the hands carefully. We want to retranslate based on the new Inst, but it's not in scope yet.
                    // We want to keep the original Inst so that Undo command works properly.
                    CRef<objects::CSeq_inst> orig_inst(new objects::CSeq_inst());
                    orig_inst->Assign(bsh.GetInst());
                    CBioseq_EditHandle bseh = bsh.GetEditHandle();
                    bseh.SetInst(bseq->SetInst());
                    trimmer.AdjustAndTranslate(bsh, length, from, to, composite, scope, product_to_cds);
                    bseh.SetInst(*orig_inst);
                    if (bseq->IsSetId())
                    {
                        trimmer.TrimAlignments(composite, bsh, bseq->GetId(), from, to);
                        trimmer.TrimSeqGraphs(composite, bsh, bseq->GetId(), from, to, length);
                    }
                }
            }
            scope_copy->RemoveBioseq(edited_bsh);
            ++bi;
        }
        
        cmdProcessor->Execute(composite.GetPointer());
        if (!msg.IsEmpty())
        {
            CGenericReportDlg* report = new CGenericReportDlg(parent);
            report->SetTitle(wxT("Trimmed Locations"));
            report->SetText(msg);
            report->SetWorkDir(workDir);
            report->Show(true);
        }
        return true;
    }
    else
        return false;
}

END_NCBI_SCOPE
