#ifndef GUI_WIDGETS_SEQ___FASTA_TITLE_ITEM__HPP
#define GUI_WIDGETS_SEQ___FASTA_TITLE_ITEM__HPP

/*  $Id: fasta_title_item.hpp 32373 2015-02-13 20:38:05Z katargir $
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

class CFastaTitleItem : public CTextSelectItem
{
public:
    CFastaTitleItem(const objects::CBioseq_Handle& h, const objects::CSeq_loc* loc = NULL);

    virtual int     GetIndent() const { return -1; }
    virtual void    CalcSize(wxDC& dc, CTextPanelContext* context);

    virtual const   CConstRef<CObject> GetAssosiatedObject() const;
    // Expand object will handle selection
    virtual bool    IsSelectable() const { return false; }

    const objects::CBioseq_Handle GetBioseqHandle() const { return m_Bioseq; }

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;

    string x_GetTitle() const;

    const objects::CBioseq_Handle m_Bioseq;
    CConstRef<objects::CSeq_loc> m_Loc;
    CConstRef<objects::CSeq_id> m_SeqId;
    string m_Title;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FASTA_TITLE_ITEM__HPP
