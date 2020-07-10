/*  $Id: flat_file_seq_block_iterator.cpp 30550 2014-06-10 17:21:30Z katargir $
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

#include <gui/widgets/seq/flat_file_seq_block_iterator.hpp>

#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>

#include <gui/widgets/seq/flat_file_text_item.hpp>

#include <objtools/format/items/sequence_item.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace {
class CFlatFileSeqBlockIteratorInit : public ICompositeTraverser
{
public:
    CFlatFileSeqBlockIteratorInit(int row) : m_Row(row), m_SeqBlock() {}

    virtual bool ProcessItem(CTextItem& /*textItem*/) { return true; }

    virtual bool ProcessItem(CExpandItem& expandItem)
    {
        CFlatFileTextItem* ffItem = dynamic_cast<CFlatFileTextItem*>(expandItem.GetExpandedItem());
        if (!ffItem) return true;
        const CSequenceItem* seqItem = dynamic_cast<const CSequenceItem*>(ffItem->GetFlatItem());
        if (seqItem) {
            int  startLine = expandItem.GetStartLine();
            size_t lineCount =  expandItem.GetLineCount();
            if (m_Row < startLine + lineCount) {
                m_SeqBlock = &expandItem;
                return false;
            }
        }
        return true;
    }

    CExpandItem*  GetSeqBlock() const { return m_SeqBlock; }

private:
    int m_Row;
    CExpandItem* m_SeqBlock;
};
}


CFlatFileSeqBlockIterator::CFlatFileSeqBlockIterator(CTextItemPanel& panel, const CTextPosition& pos)
: m_StartSeqIt(panel, pos.GetRow()), m_StartSeqItem(), m_CurrSeqItem(), m_SeqOffset(0), m_NewSequence(true), m_FullCycle(false)
{
    for (m_SeqIt = m_StartSeqIt; m_SeqIt; ++m_SeqIt) {
        CCompositeTextItem* composite = *m_SeqIt;
        if (composite && composite->GetStartLine() <= pos.GetRow()) {
            CFlatFileSeqBlockIteratorInit init(pos.GetRow());
            composite->Traverse(init);
            m_StartSeqItem = init.GetSeqBlock();
            if (m_StartSeqItem) break;
        }
    }
    if (!m_SeqIt || !m_StartSeqItem) return;

    m_CurrSeqItem = m_StartSeqItem;

    if (pos.GetRow() >= m_StartSeqItem->GetStartLine()) {
        m_SeqOffset = 60*(pos.GetRow() - m_StartSeqItem->GetStartLine());
        if (pos.GetCol() > 10)
            m_SeqOffset += ((pos.GetCol() - 10)/11)*10 + (pos.GetCol() - 10)%11;
    }
}

CFlatFileSeqBlockIterator& CFlatFileSeqBlockIterator::operator++ (void)
{
    if (!m_CurrSeqItem)
        throw std::out_of_range("Invalid index");

    if (m_FullCycle) {
        m_CurrSeqItem = 0;
        return *this;
    }

    IContainer* container = 0;
    int index = 0;
    m_CurrSeqItem->GetContainer(container, index);
    CCompositeTextItem* composite = dynamic_cast<CCompositeTextItem*>(container);
    if (composite) {
        if (index + 1 < composite->GetItemCount()) {
            CExpandItem* next = dynamic_cast<CExpandItem*>(composite->GetItem(index + 1));
            if (next) {
                CFlatFileTextItem* ffItem = dynamic_cast<CFlatFileTextItem*>(next->GetExpandedItem());
                if (ffItem) {
                    const CSequenceItem* seqItem = dynamic_cast<const CSequenceItem*>(ffItem->GetFlatItem());
                    if (seqItem) {
                        m_CurrSeqItem = next;
                        m_NewSequence = false;
                        x_FixCycle();
                        return *this;
                    }
                }
            }
        }
    }

    m_NewSequence = true;

    if (m_SeqIt) {
        for (++m_SeqIt; m_SeqIt; ++m_SeqIt) {
            CCompositeTextItem* composite = *m_SeqIt;
            CFlatFileSeqBlockIteratorInit init(0);
            composite->Traverse(init);
            m_CurrSeqItem = init.GetSeqBlock();
            if (m_CurrSeqItem) {
                x_FixCycle();
                return *this;
            }
        }
    }

    composite = *m_StartSeqIt;
    CFlatFileSeqBlockIteratorInit init(0);
    composite->Traverse(init);
    m_CurrSeqItem = init.GetSeqBlock();
    if (!m_CurrSeqItem)
        return *this;

    x_FixCycle();
    return *this;
}

void CFlatFileSeqBlockIterator::x_FixCycle()
{
    if (m_CurrSeqItem == m_StartSeqItem) {
        if (!m_FullCycle && m_SeqOffset > 0)
            m_FullCycle = true;
        else
            m_CurrSeqItem = 0;
    }
}

string CFlatFileSeqBlockIterator::operator* (void) const
{
    if (!m_CurrSeqItem)
        throw std::out_of_range("Invalid index");

    string ret;

    CFlatFileTextItem* ffItem = dynamic_cast<CFlatFileTextItem*>(m_CurrSeqItem->GetExpandedItem());
    if (!ffItem)
        return ret;

    const CSequenceItem* seqItem = dynamic_cast<const CSequenceItem*>(ffItem->GetFlatItem());
    if (!seqItem)
        return ret;

    TSeqPos from = seqItem->GetFrom();
    TSeqPos to = seqItem->GetTo();
    seqItem->GetSequence().GetSeqData(from - 1, to, ret);

    if (m_StartSeqItem != m_CurrSeqItem)
        return ret;

    if (m_FullCycle)
        return m_SeqOffset < ret.size() ? ret.substr(0, m_SeqOffset) : ret;
    else
        return m_SeqOffset < ret.size() ? ret.substr(m_SeqOffset) : string();
}

CFlatFileSeqBlockIterator::operator bool() const
{
    return (m_CurrSeqItem != 0);
}

bool CFlatFileSeqBlockIterator::NextSequence() const
{
    return m_NewSequence;
}

CTextPosition CFlatFileSeqBlockIterator::GetPosition(size_t offset) const
{
    if (!m_CurrSeqItem) return CTextPosition();

    CTextPosition pos(m_CurrSeqItem->GetStartLine(), 0);

    if (m_CurrSeqItem == m_StartSeqItem && !m_FullCycle)
        offset += m_SeqOffset;

    pos.ShiftRow(offset/60);
    offset = offset%60;
    int col = offset/10*11 + offset%10;

    pos.ShiftCol(10 + col);
    return pos;
}

END_NCBI_SCOPE
