/*  $Id: fasta_retrieve_job.cpp 39642 2017-10-23 16:51:10Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/seq/fasta_view_params.hpp>
#include <gui/widgets/seq/fasta_title_item.hpp>
#include <gui/widgets/seq/fasta_seqhdr_item.hpp>
#include <gui/widgets/seq/fasta_seqblock_item.hpp>

#include <gui/widgets/seq/fasta_retrieve_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CFastaRetrieveJob::CFastaRetrieveJob(CTextPanelContext& context,
                                     const vector<pair<CBioseq_Handle, string> >& handles)
    : CTextRetrieveJob(context), m_Handles(handles)
{
}

CFastaRetrieveJob::~CFastaRetrieveJob()
{
}

IAppJob::EJobState CFastaRetrieveJob::x_Run()
{
    for (auto& h : m_Handles) {
        x_CheckCancelled();
        if (h.first)
            x_DumpBioseq(h.first);
        else
            x_AddItem(nullptr);
    }

    x_CreateResult();

    return eCompleted;
}

void CFastaRetrieveJob::x_DumpBioseq(const CBioseq_Handle& h)
{
    CFastaTitleItem* titleItem = new CFastaTitleItem(h);
    CCompositeTextItem* expanded = new CCompositeTextItem();
    x_AddItem(new CExpandItem(titleItem, expanded, true));

    expanded->AddItem(new CFastaTitleItem(h), false);
    TSeqPos seqLength = h.GetBioseqLength();
    size_t blockSize = kFastaSeqLine*kFastaSeqBlockSize;
    size_t blockCount = (seqLength + blockSize - 1) / blockSize;

    vector<ITextItem*> items(blockCount, 0);

    for (size_t i = 0; i < blockCount; ++i) {
        x_CheckCancelled();
        size_t from = i*blockSize;
        size_t count = (i + 1 == blockCount) ? seqLength - from : blockSize;
        CFastaSeqHdrItem* seqCollapsed = new CFastaSeqHdrItem(from, count, *titleItem);
        CFastaSeqBlockItem* seqExpanded = new CFastaSeqBlockItem(h, from, count);
        expanded->AddItem(new CExpandItem(seqCollapsed, seqExpanded, (blockCount == 1)), false);
    }
}

CFastaRetrieveLocJob::CFastaRetrieveLocJob(CTextPanelContext& context, const CSeq_loc& loc, CScope& scope)
    : CTextRetrieveJob(context), m_Loc(&loc), m_Scope(&scope)
{
}

CFastaRetrieveLocJob::~CFastaRetrieveLocJob()
{
}

IAppJob::EJobState CFastaRetrieveLocJob::x_Run()
{
    if (!m_Loc) return eCompleted;

    for( CSeq_loc_CI it(*m_Loc);  it;  ++it) {
        x_CheckCancelled();

        CBioseq_Handle handle = m_Scope->GetBioseqHandle(it.GetSeq_id());
        if (!handle)
            continue;

        CConstRef<CSeq_loc> loc = it.GetRangeAsSeq_loc();
        CFastaTitleItem* titleItem = new CFastaTitleItem(handle, loc);
        CCompositeTextItem* expanded = new CCompositeTextItem();
        x_AddItem(new CExpandItem(titleItem, expanded));

        expanded->AddItem(new CFastaTitleItem(handle, loc), false);

        CSeq_loc_CI::TRange range = it.GetRange();
        TSeqPos seqLength = range.GetLength();
        size_t blockSize = kFastaSeqLine*kFastaSeqBlockSize;
        size_t blockCount = (seqLength + blockSize - 1)/blockSize;

        for (size_t i = 0; i < blockCount; ++i) {
            x_CheckCancelled();

            size_t from = range.GetFrom() + i*blockSize;
            size_t count = (i + 1 == blockCount) ? seqLength - i*blockSize : blockSize;
            CFastaSeqHdrItem* seqCollapsed = new CFastaSeqHdrItem(from, count, *titleItem);
            CFastaSeqBlockItem* seqExpanded = new CFastaSeqBlockItem(handle, from, count);
            expanded->AddItem(new CExpandItem(seqCollapsed, seqExpanded, (blockCount == 1)), false);
        }
    }

    x_CreateResult();

    return eCompleted;
}

END_NCBI_SCOPE
