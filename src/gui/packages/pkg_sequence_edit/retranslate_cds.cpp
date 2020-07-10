/*  $Id: retranslate_cds.cpp 43629 2019-08-09 19:00:33Z filippov $
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
#include <objmgr/feat_ci.hpp>
#include <objects/seqtable/seq_table_exception.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>
#include <wx/msgdlg.h>

#include <gui/packages/pkg_sequence_edit/retranslate_cds.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CRef<CCmdComposite> CRetranslateCDS::GetCommand( objects::CSeq_entry_Handle seh, string title, ERetranslateMethod method, string &error)
{
    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn("CDS comment"));
    vector<CConstRef<CObject> > objs = col->GetObjects(seh, "", CRef<edit::CStringConstraint>(NULL));
    if (objs.empty()) {
        error = "No coding regions found!";       
        return CRef<CCmdComposite>(NULL);
    }
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(seh);
    bool any_change = false;
    int offset = 1;
    CRef<CCmdComposite> cmd(new CCmdComposite(title)); // "Retranslate coding regions"
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        // skip if pseudo
        if (sequence::IsPseudo(*f, seh.GetScope())) {
            continue;
        }
        if (f->IsSetExcept_text() && NStr::Find(f->GetExcept_text(), "RNA editing") != string::npos) 
        {
            continue;
        }

        CRef<CSeq_feat> new_cds(new CSeq_feat());
        new_cds->Assign(*f);
        bool cds_change = false;
        switch (method) {
            case eRetranslateCDSIgnoreStopExceptEnd:
                // do nothing
                break;
            case eRetranslateCDSObeyStop:
                // truncate at stop
                if (edit::TruncateCDSAtStop(*new_cds, seh.GetScope())) {
                    cds_change = true;
                }
                break;
            case eRetranslateCDSChooseFrame:                
                {{  
                    CCdregion::TFrame orig_frame = CCdregion::eFrame_not_set;
                    if (new_cds->GetData().GetCdregion().IsSetFrame()) {
                        orig_frame = new_cds->GetData().GetCdregion().GetFrame();
                    }
                    new_cds->SetData().SetCdregion().SetFrame(CSeqTranslator::FindBestFrame(*f, seh.GetScope()));
                    CCdregion::TFrame new_frame = CCdregion::eFrame_not_set;
                    if (new_cds->GetData().GetCdregion().IsSetFrame()) {
                        new_frame = new_cds->GetData().GetCdregion().GetFrame();
                    }
                    if (orig_frame != new_frame) {
                        cds_change = true;
                    }
                }}
                break;
            default:
                // should never happen
                return CRef<CCmdComposite>(NULL);
                break;
        }
        bool transl_change = false;
        CRef<CCmdComposite> subcmd = RetranslateCDSCommand(seh.GetScope(), *f, *new_cds, transl_change, offset, create_general_only);
        if (subcmd) {
            cmd->AddCommand(*subcmd);
            if (cds_change || transl_change) {
                CSeq_feat_Handle fh = seh.GetScope().GetSeq_featHandle(*f);
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(fh, *new_cds, true));
                cmd->AddCommand(*chgFeat);
            }
            any_change = true;
        }
    }
    if (!any_change)
        cmd.Reset();
    return cmd;
}

void CRetranslateCDS::apply( objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, string title, ERetranslateMethod method )
{
    string error;
    CRef<CCmdComposite> cmd = GetCommand(seh, title, method, error);
    if (cmd) {
        cmdProcessor->Execute(cmd); 
    } else if (!error.empty()) {
        wxMessageBox(wxString(error), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    } else {
        wxMessageBox(wxT("No effect!"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }
}

CRef<CCmdComposite> CRetranslateCDS::RetranslateCDSCommand(CScope& scope, const CSeq_feat& old_cds, CSeq_feat& cds, bool& cds_change, int& offset, bool create_general_only)
{
    // feature must be cds
    if (!cds.IsSetData() && !cds.GetData().IsCdregion()) {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }
    cds_change = false;

    if (!cds.IsSetProduct()) {
        string id_label;
        CBioseq_Handle bsh = scope.GetBioseqHandle(cds.GetLocation());
        CRef<CSeq_id> new_prot_id = objects::edit::GetNewProtId(bsh, offset, id_label, create_general_only);
        cds.SetProduct().SetWhole().Assign(*new_prot_id);
    }
    // Use Cdregion.Product to get handle to protein bioseq 
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds.GetProduct());

    if (!prot_bsh) {
        return GetRetranslateCDSCommand(scope, cds, cds_change, offset, create_general_only);
    }
    // Should be a protein!
    if (!prot_bsh.IsProtein())
    {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }

    // Make a copy of existing CSeq_inst
    CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
    new_inst->Assign(prot_bsh.GetInst());

    // Make edits to the CSeq_inst copy
    //CRef<CBioseq> new_protein_bioseq;
    if (new_inst->IsSetSeq_data())
    {
        new_inst->ResetSeq_data();
    }

    // Generate new protein sequence data and length
    string prot;
    CSeqTranslator::Translate(cds, scope, prot);
    if (NStr::EndsWith(prot, "*")) 
    {
        prot = prot.substr(0, prot.length() - 1);
    }
    new_inst->ResetExt();
    new_inst->SetRepr(objects::CSeq_inst::eRepr_raw); 
    new_inst->SetSeq_data().SetNcbieaa().Set(prot);
    new_inst->SetLength(TSeqPos(prot.length()));
    new_inst->SetMol(CSeq_inst::eMol_aa);

      
    CRef<CCmdComposite> cmd(new CCmdComposite("Retranslate CDS"));

    // Update protein sequence data and length
    CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(prot_bsh, *new_inst));
    cmd->AddCommand(*chgInst);

    // change molinfo on bioseq
    CRef<CCmdComposite> synch_molinfo = GetSynchronizeProductMolInfoCommand(scope, cds);
    if (synch_molinfo) 
    {
        cmd->AddCommand(*synch_molinfo);
    }

    
    for ( CFeat_CI prot_feat_ci(prot_bsh, CSeqFeatData::e_Prot); prot_feat_ci; ++prot_feat_ci ) 
    {
        if (prot_feat_ci->GetFeatSubtype() != CSeqFeatData::eSubtype_prot)
            continue;
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(prot_feat_ci->GetOriginalFeature());
        
        if ( new_feat->CanGetLocation() &&
             new_feat->GetLocation().IsInt() &&
             new_feat->GetLocation().GetInt().CanGetTo() )
        {
            new_feat->SetLocation().SetInt().SetTo(new_inst->GetLength() - 1);
            
            edit::AdjustProteinFeaturePartialsToMatchCDS(*new_feat, cds);
            
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*prot_feat_ci, *new_feat));
            cmd->AddCommand(*chgFeat);
        }
    }
    bool any_actions = false;
    try
    {
        NRawToDeltaSeq::RemapOtherProtFeats(old_cds, cds, prot_bsh, cmd, any_actions);
    } catch(const CUtilException&) {}
    return cmd;
}

END_NCBI_SCOPE
