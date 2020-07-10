/*  $Id: flat_file_seq_iterator.cpp 35541 2016-05-18 19:29:42Z katargir $
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

#include <gui/widgets/seq/flat_file_seq_iterator.hpp>

#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

BEGIN_NCBI_SCOPE

CFlatFileSeqIterator::CFlatFileSeqIterator(CTextItemPanel& panel, int row)
: m_Sections(), m_StartIndex(-1), m_SeqIndex(-1)
{
    m_Sections = dynamic_cast<CCompositeTextItem*>(panel.GetRootItem());
    if (!m_Sections) return;

    if (row < 0 || row >= (int)m_Sections->GetLineCount())
        return;

    for (size_t i = 0; i < m_Sections->GetItemCount(); ++i) {
        ITextItem* item = m_Sections->GetItem(i);
        int  startLine = item->GetStartLine();
        size_t lineCount =  item->GetLineCount();
        if (row >= startLine && row < startLine + lineCount) {
            m_StartIndex = m_SeqIndex = (int)i;
            break;
        }
    }
}

CCompositeTextItem* CFlatFileSeqIterator::operator* (void) const
{
    if (!m_Sections)
        throw std::invalid_argument ("Iterator not initialized");

    if (m_SeqIndex >= 0 && m_SeqIndex < m_Sections->GetItemCount()) {
        ITextItem* item = m_Sections->GetItem((size_t)m_SeqIndex);
		CExpandItem* expandItem = dynamic_cast<CExpandItem*>(item);
		if (expandItem)
		    return dynamic_cast<CCompositeTextItem*>(expandItem->GetExpandedItem());
        else
            return dynamic_cast<CCompositeTextItem*>(item);
    }
    return 0;
}

CFlatFileSeqIterator::operator bool() const
{
    return (m_Sections && m_SeqIndex >= 0);
}

CFlatFileSeqIterator& CFlatFileSeqIterator::operator++ (void)
{
    if (!m_Sections)
        throw std::invalid_argument ("Iterator not initialized");

    if (m_SeqIndex < 0)
        throw std::out_of_range("Invalid index");

    if (++m_SeqIndex >= m_Sections->GetItemCount())
        m_SeqIndex = 0;

    if (m_SeqIndex == m_StartIndex)
        m_SeqIndex = -1;

    return *this;
}

END_NCBI_SCOPE
