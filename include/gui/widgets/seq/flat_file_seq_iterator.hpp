#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_SEQ_ITERATOR__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_SEQ_ITERATOR__HPP

/*  $Id: flat_file_seq_iterator.hpp 30550 2014-06-10 17:21:30Z katargir $
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

BEGIN_NCBI_SCOPE

class CTextItemPanel;
class CCompositeTextItem ;

class NCBI_GUIWIDGETS_SEQ_EXPORT CFlatFileSeqIterator
{
public:
    CFlatFileSeqIterator() : m_Sections(), m_StartIndex(0), m_SeqIndex(0) {}
    CFlatFileSeqIterator(CTextItemPanel& panel, int row = 0);

    CFlatFileSeqIterator& operator++ (void); // prefix
    CCompositeTextItem* operator* (void) const;
    operator bool() const;

    CFlatFileSeqIterator& operator=(const CFlatFileSeqIterator& it)
    {
        m_Sections = it.m_Sections;
        m_StartIndex = it.m_StartIndex;
        m_SeqIndex = it.m_SeqIndex;
        return *this;
    }

private:
    CCompositeTextItem* m_Sections;
    int m_StartIndex;
    int m_SeqIndex;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_SEQ_ITERATOR__HPP
