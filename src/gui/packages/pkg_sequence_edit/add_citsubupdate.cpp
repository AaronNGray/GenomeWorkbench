/*  $Id: add_citsubupdate.cpp 42457 2019-02-28 21:58:28Z asztalos $
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
#include <objmgr/bioseq_ci.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/citsub_updater.hpp>
#include <gui/packages/pkg_sequence_edit/add_citsubupdate.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

bool CAddCitSubForUpdate::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    if (tse) {
        CRef<CCmdComposite> composite(new CCmdComposite(title));    // See CEditObjectPubdesc::GetEditCommand from edit_object_seq_desc.cpp
        apply_to_seq_and_feat(tse,composite);
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    else
        return false;
}


void CAddCitSubForUpdate::apply_to_seq_and_feat(CSeq_entry_Handle tse, CCmdComposite* composite)
{
    ApplyToCSeq_entry (tse, *(tse.GetCompleteSeq_entry()), composite);
    //CFeat_CI feat (tse, SAnnotSelector(CSeqFeatData::e_Pub));   TODO
    //while (feat) {
    //   ApplyToPub (feat->GetData().GetPub(), composite, NULL, NULL,&(feat->GetOriginalFeature())); 
    //       ++feat;
    //}
}


void CAddCitSubForUpdate::ApplyToCSeq_entry(CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)  // s_CollectPubDescriptorLabels in validatorp.cpp
{
    map<CConstRef<CSeqdesc>, CRef<CSeqdesc> > changedSet;
    map<CConstRef<CSeqdesc>, CSeq_entry_Handle> descr_ctxt;
    CConstRef<CSeqdesc> addedSeqdesc;
    string message;
    for (CBioseq_CI b_iter(tse, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        CConstRef<CSeqdesc> changedSeqdesc;
        CSeq_entry_Handle seh_for_desc;
        string msg;
        CRef<CSeqdesc> changedORadded_citsub = CCitSubUpdater::s_GetCitSubForUpdatedSequence(*b_iter, msg, changedSeqdesc, seh_for_desc);
        if (NStr::EqualNocase(msg, CCitSubUpdater::sm_ExistingCitSub)) {
            NcbiInfoBox(msg);
            return;
        }
        if (!changedORadded_citsub) {
            message = msg;
            continue;
        }

        if (changedSeqdesc) {
            changedSet.emplace(changedSeqdesc, changedORadded_citsub);
            descr_ctxt.emplace(changedSeqdesc, seh_for_desc);
            message = msg;
        }
        else {
            addedSeqdesc.Reset(changedORadded_citsub.GetPointer());
            if (changedSet.empty()) {
                message = msg;
            }
        }
    }

    // if changes an existing one, it should not add a new one
    if (!changedSet.empty()) {
        auto desc_it = changedSet.begin();
        auto ctxt_it = descr_ctxt.begin();
        _ASSERT(changedSet.size() == descr_ctxt.size());
        for ( ; desc_it != changedSet.end() && ctxt_it != descr_ctxt.end(); ++desc_it, ctxt_it) {
            CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(ctxt_it->second, *desc_it->first, *desc_it->second));
            composite->AddCommand(*cmd);
        }
    }
    else if (addedSeqdesc) {
        CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(tse, *addedSeqdesc));
        composite->AddCommand(*cmdAddDesc);
    }

    if (!message.empty()) {
        NcbiInfoBox(message);
    }
}


END_NCBI_SCOPE
