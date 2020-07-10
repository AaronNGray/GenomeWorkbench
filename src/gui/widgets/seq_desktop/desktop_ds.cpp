/*  $Id: desktop_ds.cpp 44457 2019-12-19 19:25:12Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>

#include <gui/widgets/seq_desktop/desktop_typed_items.hpp>
#include <gui/widgets/seq_desktop/desktop_ds.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/settings.h>
#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
 
CDesktopDataBuilder::CDesktopDataBuilder(const CSerialObject& so, objects::CScope& scope)
{
    m_Scope = Ref(&scope);
    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(&so);
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(&so);
    const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*>(&so);
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(&so);
    const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set*>(&so);
    const CSeq_submit* seq_submit = dynamic_cast<const CSeq_submit*>(&so);
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&so);

    if (seq_entry) {
        m_Seh = scope.GetSeq_entryHandle(*seq_entry);
    }
    else if (bioseq) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(*bioseq);
        if (bsh)
            m_Seh = bsh.GetTopLevelEntry();
    }
    else if (bioseq_set) {
        CBioseq_set_Handle bssh = scope.GetBioseq_setHandle(*bioseq_set);
        if (bssh)
            m_Seh = bssh.GetTopLevelEntry();
    }
    else if (seq_id) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(*seq_id);
        if (bsh)
            m_Seh = bsh.GetTopLevelEntry();
    }
    else if (seq_loc) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(*seq_loc);
        if (bsh)
            m_Seh = bsh.GetTopLevelEntry();
    }
    else if (seq_submit) {
        m_SeqSubmit = ConstRef(seq_submit);
        if (m_SeqSubmit->IsSetData()) {
            const CSeq_submit::TData& data = m_SeqSubmit->GetData();
            if (data.IsEntrys()) {
                const CSeq_submit::TData::TEntrys& entries = data.GetEntrys();
                if (!entries.empty()) {
                    m_Seh = scope.GetSeq_entryHandle(**entries.begin());
                }
            }
        }
    }  else if (annot) {
        m_AnnotHandle = scope.GetSeq_annotHandle(*annot);
    }  else {
        ReportIDError("", true, "Sequin Desktop View");
        return;
    }

    x_CreateDesktopItems();
}

void CDesktopDataBuilder::x_CreateDesktopItems()
{    
    if (m_Seh) {

        if (m_SeqSubmit) {
            x_CreateSeqSubmit();
        }
        else {
            CConstRef<CSeq_entry> entry = m_Seh.GetCompleteSeq_entry();
            if (m_Seh.IsSeq()) {
                CConstRef<IDesktopDataItem> item(new CDesktopBioseqItem(m_Seh.GetSeq()));
                m_RootItem = Ref(new CRootDesktopItem(item));
                x_CreateSeq(m_Seh.GetSeq(), m_RootItem);
            }
            else {
                CConstRef<IDesktopDataItem> item(new CDesktopBioseqsetItem(m_Seh.GetSet()));
                m_RootItem = Ref(new CRootDesktopItem(item));
                x_CreateSet(m_Seh.GetSet(), m_RootItem);
            }
        
        }
    } else if (m_AnnotHandle) {
        CConstRef<IDesktopDataItem> item(new CDesktopAnnotItem(m_AnnotHandle));
        m_RootItem = Ref(new CRootDesktopItem(item));
        x_CreateAnnot(m_AnnotHandle, m_RootItem);
    }
    else {
        _ASSERT(false);
    }
}

void CDesktopDataBuilder::x_CreateSeqSubmit()
{
    CConstRef<IDesktopDataItem> item(new CDesktopSeqSubmitItem(*m_SeqSubmit, m_Scope.GetNCObject()));
    m_RootItem = Ref(new CRootDesktopItem(item));

    if (m_SeqSubmit->IsSetSub()) {
        const CSubmit_block& block = m_SeqSubmit->GetSub();
        if (block.IsSetContact()) {
            CConstRef<IDesktopDataItem> item(new CDesktopContactInfoItem(*m_SeqSubmit, block.GetContact(), m_Scope.GetNCObject()));
            IDesktopItem* contact_item = new CSimpleDesktopItem(item);
            m_RootItem->Add(*contact_item);
        }
        if (block.IsSetCit()) {
            CConstRef<IDesktopDataItem> item(new CDesktopCitSubItem(*m_SeqSubmit, block.GetCit(), m_Scope.GetNCObject()));
            IDesktopItem* cit_item = new CSimpleDesktopItem(item);
            m_RootItem->Add(*cit_item);
        }
    }

    if (m_SeqSubmit->IsEntrys()) {
        if (m_Seh.IsSeq()) {
            x_CreateSeq(m_Seh.GetSeq(), m_RootItem);
        }
        else {
            x_CreateSet(m_Seh.GetSet(), m_RootItem);
        }
    }
    else if (m_SeqSubmit->IsAnnots()) {
        FOR_EACH_SEQANNOT_ON_SEQSUBMIT(annot_it, *m_SeqSubmit) {
            x_CreateAnnot(m_Scope->GetSeq_annotHandle(**annot_it), m_RootItem);
        }
    }
}

void CDesktopDataBuilder::x_CreateSeq(const CBioseq_Handle& bsh, CIRef<IDesktopItem> parent)
{
    CConstRef<IDesktopDataItem> item(new CDesktopBioseqItem(bsh));
    IDesktopItem* bseq_item = new CCompositeDesktopItem(item);
    parent->Add(*bseq_item);

    const CBioseq& bseq = bsh.GetCompleteBioseq().GetObject();
    FOR_EACH_SEQDESC_ON_BIOSEQ(desc_it, bseq) {
        CConstRef<IDesktopDataItem> item(new CDesktopSeqdescItem(**desc_it, bsh.GetSeq_entry_Handle()));
        IDesktopItem* desc_item = new CSimpleDesktopItem(item);
        bseq_item->Add(*desc_item);
    }

    FOR_EACH_SEQANNOT_ON_BIOSEQ(annot_it, bseq) {
        x_CreateAnnot(m_Scope->GetSeq_annotHandle(**annot_it), CIRef<IDesktopItem>(bseq_item));
    }
}


void CDesktopDataBuilder::x_CreateAnnot(const CSeq_annot_Handle& ah, CIRef<IDesktopItem> parent)
{
    CConstRef<IDesktopDataItem> item (new CDesktopAnnotItem(ah));
    IDesktopItem* annot_item = new CCompositeDesktopItem(item);
    const wxBrush& bkgd_brush = parent->GetBkgdBrush();
    annot_item->SetBkgdBrush(bkgd_brush);
    parent->Add(*annot_item);

    
    if (ah.IsFtable()) {
        for (CFeat_CI feat_it(ah); feat_it; ++feat_it) {
            CConstRef<IDesktopDataItem> item(new CDesktopFeatItem(feat_it->GetSeq_feat_Handle()));
            IDesktopItem* feat_item = new CSimpleDesktopItem(item);
            feat_item->SetBkgdBrush(bkgd_brush);
            feat_item->SetFrameColor(bkgd_brush.GetColour());
            annot_item->Add(*feat_item);
        }
    }
    else if (ah.IsAlign()) {
        for (CAlign_CI align_it(ah); align_it; ++align_it) {
            CConstRef<IDesktopDataItem> item(new CDesktopAlignItem(align_it.GetSeq_align_Handle()));
            IDesktopItem* align_item = new CSimpleDesktopItem(item);
            align_item->SetBkgdBrush(bkgd_brush);
            align_item->SetFrameColor(bkgd_brush.GetColour());
            annot_item->Add(*align_item);
        }
    }
    else if (ah.IsGraph()) {
        for (CGraph_CI graph_it(ah); graph_it; ++graph_it) {
            CConstRef<IDesktopDataItem> item(new CDesktopGraphItem(graph_it->GetSeq_graph_Handle()));
            IDesktopItem* graph_item = new CSimpleDesktopItem(item);
            graph_item->SetBkgdBrush(bkgd_brush);
            graph_item->SetFrameColor(bkgd_brush.GetColour());
            annot_item->Add(*graph_item);
        }
    }
}

void CDesktopDataBuilder::x_CreateSet(const CBioseq_set_Handle& bssh, CIRef<IDesktopItem> parent)
{
    CConstRef<IDesktopDataItem> item(new CDesktopBioseqsetItem(bssh));
    IDesktopItem* bset_item = new CCompositeDesktopItem(item);
    parent->Add(*bset_item);

    const CBioseq_set& bset = bssh.GetCompleteBioseq_set().GetObject();
    const wxBrush& bkgd_brush = bset_item->GetBkgdBrush();
    FOR_EACH_SEQDESC_ON_SEQSET(desc_it, bset) {
        CConstRef<IDesktopDataItem> item(new CDesktopSeqdescItem(**desc_it, bssh.GetParentEntry()));
        IDesktopItem* desc_item = new CSimpleDesktopItem(item);
        desc_item->SetBkgdBrush(bkgd_brush);
        desc_item->SetFrameColor(bkgd_brush.GetColour());
        bset_item->Add(*desc_item);
    }

    FOR_EACH_SEQANNOT_ON_SEQSET(annot_it, bset) {
        x_CreateAnnot(m_Scope->GetSeq_annotHandle(**annot_it), CIRef<IDesktopItem>(bset_item));
    }

    FOR_EACH_SEQENTRY_ON_SEQSET(entry_it, bset) {
        const CSeq_entry& subentry = **entry_it;
        if (subentry.IsSet()) {
            CBioseq_set_Handle sub_bssh = m_Scope->GetBioseq_setHandle(subentry.GetSet());
            x_CreateSet(sub_bssh, CIRef<IDesktopItem>(bset_item));
        }
        else {
            CBioseq_Handle sub_bsh = m_Scope->GetBioseqHandle(subentry.GetSeq());
            x_CreateSeq(sub_bsh, CIRef<IDesktopItem>(bset_item));
        }
    }
}
  
END_NCBI_SCOPE
