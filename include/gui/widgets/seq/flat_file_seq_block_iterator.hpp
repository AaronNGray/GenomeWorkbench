#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_SEQ_BLOCK_ITERATOR__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_SEQ_BLOCK_ITERATOR__HPP

/*  $Id: flat_file_seq_block_iterator.hpp 30549 2014-06-10 16:27:08Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/text_widget/text_position.hpp>
#include <gui/widgets/seq/flat_file_seq_iterator.hpp>

BEGIN_NCBI_SCOPE

/*
    for (CFlatFileSeqBlockIterator it(*this, cursorPos); it; ++it) {
        if (it.NextSequence()) {
        }
        string line = *it;
    }
 */

class CExpandItem;

class NCBI_GUIWIDGETS_SEQ_EXPORT CFlatFileSeqBlockIterator
{
public:
    CFlatFileSeqBlockIterator(CTextItemPanel& panel, const CTextPosition& pos);

    CFlatFileSeqBlockIterator& operator++ (void); // prefix
    string operator*  (void) const;
    operator bool() const;
    bool NextSequence() const;

    CTextPosition GetPosition(size_t offset) const;

    CFlatFileSeqBlockIterator& operator=(const CFlatFileSeqBlockIterator& it)
    {
        m_SeqIt = it.m_SeqIt;
        m_StartSeqItem = it.m_StartSeqItem;
        m_CurrSeqItem  = it.m_CurrSeqItem;
        m_SeqOffset = it.m_SeqOffset;
        m_NewSequence = it.m_NewSequence;
        m_FullCycle = it.m_FullCycle;
        return *this;
    }

private:
    void x_FixCycle();

    CFlatFileSeqIterator m_StartSeqIt;
    CFlatFileSeqIterator m_SeqIt;
    CExpandItem*   m_StartSeqItem;
    CExpandItem*   m_CurrSeqItem;
    size_t         m_SeqOffset;
    bool           m_NewSequence;
    bool           m_FullCycle;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_SEQ_BLOCK_ITERATOR__HPP
