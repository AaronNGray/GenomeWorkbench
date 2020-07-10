/*  $Id: flat_file_locus_item.cpp 37250 2016-12-20 18:21:24Z katargir $
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

#include <corelib/ncbistre.hpp>

#include <objtools/format/text_ostream.hpp>
#include <objtools/format/items/locus_item.hpp>

#include <gui/widgets/seq/flat_file_locus_item.hpp>
#include <gui/widgets/text_widget/hyperlink_glyph.hpp>

#include <gui/widgets/seq/flat_file_context.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CFlatFileLocusItem::~CFlatFileLocusItem()
{
    delete m_HyperlinkGlyph;
}


CBioseq_Handle CFlatFileLocusItem::GetBioseqHandle()
{
    const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(m_Item.GetPointerOrNull());
    if (!locusItem)
        return CBioseq_Handle();

    CBioseqContext* ctx = locusItem->GetContext();
    if (ctx)
        return ctx->GetHandle();

    return CBioseq_Handle();
}

SConstScopedObject CFlatFileLocusItem::GetSeqLoc()
{
    if (!m_SeqLoc.object) {
	    CBioseq_Handle handle = GetBioseqHandle();
	    if (handle) {
            CConstRef<CSeq_id> id = handle.GetSeqId();
            CRef<CSeq_loc> seq_loc(new CSeq_loc());
            seq_loc->SetWhole().Assign(*id);
            m_SeqLoc = SConstScopedObject(seq_loc, &handle.GetScope());
	    }
    }
    return m_SeqLoc;
}

void CFlatFileLocusItem::CalcSize(wxDC& dc, CTextPanelContext* context)
{
    CFlatFileTextItem::CalcSize(dc, context);

    const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(m_Item.GetPointerOrNull());
    if (!locusItem)
        return;

    CFlatFileViewContext* ffContext = dynamic_cast<CFlatFileViewContext*>(context);
    if (ffContext && ffContext->IsEditingEnabled()) {
        string seqName = locusItem->GetFullName();
        m_HyperlinkGlyph = new CHyperlinkGlyph(this, ToWxString("Edit \"" + seqName + "\""));

        m_HyperlinkGlyph->CalcSize(dc, context);
        wxSize hSize = m_HyperlinkGlyph->GetSize();
        wxSize size = GetSize();
        wxPoint hPos(size.x + context->GetWWidth(), (size.y - hSize.y)/2);
        m_HyperlinkGlyph->SetPosition(hPos);
        m_Size.x += hSize.x;
    }
}

void CFlatFileLocusItem::Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context)
{
    CFlatFileTextItem::Draw(dc, updateRect, context);
    if (m_HyperlinkGlyph) {
        wxRect rect = m_HyperlinkGlyph->GetRect();
        if (updateRect.Intersects(rect)) {
            wxPoint dcOrigin = context->GetDeviceOrigin();
            context->SetDeviceOrigin(wxPoint(dcOrigin.x + rect.GetLeft(), dcOrigin.y + rect.GetTop()));
            context->SetDeviceOrigin(dc);
            m_HyperlinkGlyph->Draw(dc, context);
            context->SetDeviceOrigin(dcOrigin);
        }
    }
}

void CFlatFileLocusItem::MouseEvent(wxMouseEvent& event, CTextPanelContext& context)
{
    if (m_HyperlinkGlyph) {
        wxRect rect = m_HyperlinkGlyph->GetRect();
        if (rect.Contains(event.m_x, event.m_y)) {
            wxMouseEvent ev(event);
            ev.m_x -= rect.x;
            ev.m_y -= rect.y;
            m_HyperlinkGlyph->MouseEvent(ev);
            return;
        }
    }

    CFlatFileTextItem::MouseEvent(event, context);
}

void CFlatFileLocusItem::OnHyperlinkClick(CTextPanelContext* context)
{
    CFlatFileViewContext* ffContext = dynamic_cast<CFlatFileViewContext*>(context);
    if (!ffContext)
        return;

    const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(m_Item.GetPointerOrNull());
    if (!locusItem)
        return;

    CBioseqContext* ctx = locusItem->GetContext();
    if (!ctx)
        return;

    CBioseq_Handle bh = ctx->GetHandle();
    if (!bh)
        return;

    wxPoint pos = GetPosition();
    pos.x += m_HyperlinkGlyph->GetPosition().x;
    pos.y += m_HyperlinkGlyph->GetRect().GetBottom() + 2;

    ffContext->OnEditBioseq(bh, pos);
}

END_NCBI_SCOPE
