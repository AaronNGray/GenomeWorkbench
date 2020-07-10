#ifndef GUI_WIDGETS_SEQ___FASTA_SEQHDR_ITEM__HPP
#define GUI_WIDGETS_SEQ___FASTA_SEQHDR_ITEM__HPP

/*  $Id: fasta_seqhdr_item.hpp 32371 2015-02-13 20:22:08Z katargir $
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

#include <objmgr/bioseq_handle.hpp>

#include <gui/widgets/text_widget/text_select_item.hpp>

BEGIN_NCBI_SCOPE

class CFastaTitleItem;

class CFastaSeqHdrItem : public CTextSelectItem
{
public:
    CFastaSeqHdrItem(size_t from, size_t length, CFastaTitleItem& titleItem) :
        m_From(from), m_Length(length), m_TitleItem(titleItem) {}

    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context);

    // Expand object will handle selection
    virtual bool    IsSelectable() const { return false; }

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;

    string x_GetText() const;

    size_t m_From; // 0 based
    size_t m_Length;
    CFastaTitleItem& m_TitleItem;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FASTA_SEQHDR_ITEM__HPP
