/*  $Id: flat_file_text_item.cpp 40287 2018-01-19 18:38:17Z bollin $
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

#include <objmgr/seqdesc_ci.hpp>

#include <objtools/format/text_ostream.hpp>
#include <objtools/format/items/feature_item.hpp>
#include <objtools/format/items/sequence_item.hpp>

#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/text_widget/selection_helper.hpp>

#include <gui/widgets/seq/flat_file_text_item.hpp>
#include <gui/widgets/seq/flat_file_context.hpp>
#include <gui/widgets/seq/flat_file_view_params.hpp>

#include "edit_tb.hpp"

#include <gui/objutils/seqdesc_title_edit.hpp>
#include <gui/objutils/gap_edit.hpp>
#include <gui/objutils/primary_edit.hpp>

#include <objtools/format/items/defline_item.hpp>
#include <objtools/format/items/source_item.hpp>
#include <objtools/format/items/comment_item.hpp>
#include <objtools/format/items/feature_item.hpp>
#include <objtools/format/items/genome_project_item.hpp>
#include <objtools/format/items/gap_item.hpp>
#include <objtools/format/items/primary_item.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace // anonymous
{

class CCountLines : public IFlatTextOStream
{
public:
    CCountLines() : m_Lines(0) {}

    virtual void AddParagraph(const list<string>&  text, const CSerialObject*)
    {
        m_Lines += text.size();
    }
    virtual void AddLine(const CTempString& line,
                         const CSerialObject* obj,
                         EAddNewline add_newline)
    {
        if (eAddNewline_Yes == add_newline)
            ++m_Lines;
    }

    size_t GetLines() const { return m_Lines; }

private:
    size_t m_Lines;
};

class CGetText : public IFlatTextOStream
{
public:
    CGetText() {}

    virtual void AddParagraph(const list<string>&  text, const CSerialObject*)
    {
        ITERATE(list<string>, it, text)
            m_OS << *it << "\n";
    }
    virtual void AddLine(const CTempString& line,
                         const CSerialObject* obj,
                         EAddNewline add_newline)
    {
        m_OS << line;
        if (eAddNewline_Yes == add_newline)
            m_OS << "\n";
    }

    string Text() { return CNcbiOstrstreamToString(m_OS); }

private:
    CNcbiOstrstream m_OS;
};

class CDrawLines : public IFlatTextOStream
{
public:
    CDrawLines(CStyledTextOStream& drawText) : m_DrawText(drawText) {}

    virtual void AddParagraph(const list<string>&  text, const CSerialObject*)
    {
        ITERATE(list<string>, it, text)
            m_DrawText << *it << NewLine();
    }
    virtual void AddLine(const CTempString& line,
                         const CSerialObject* obj,
                         EAddNewline add_newline)
    {
        m_DrawText << line;
        if (eAddNewline_Yes == add_newline)
            m_DrawText << NewLine();
    }

private:
    CStyledTextOStream& m_DrawText;
};

} // anonymous namespace

CFlatFileTextItem::CFlatFileTextItem(CConstRef<objects::IFlatItem> item)
    : m_Item(item), m_EditFlags(0), m_EditToolbar()
{
    if (!m_Item)
        return;

    const CSeqdesc* odesc = dynamic_cast<const CSeqdesc*>(item->GetObject());
    if (odesc) {
        m_AssosiatedObject.Reset(odesc);
        return;
    }

    const CSeq_feat* ofeat = dynamic_cast<const CSeq_feat*>(item->GetObject());
    if (ofeat) {
        m_AssosiatedObject.Reset(ofeat);
    }


    // See s_GetFlatRowType in flat_file_retrieve_job.cpp

    const CGapItem* gap_item = dynamic_cast<const CGapItem*>(m_Item.GetPointer());
    if (gap_item)
    {
        CBioseqContext* ctx = gap_item->GetContext();
        if (!ctx)
            return;

        m_AssosiatedObject.Reset(new CGapEdit(ctx->GetHandle(), gap_item->GetFrom(), gap_item->GetTo() - gap_item->GetFrom() + 1));
        return;
    }

    const CPrimaryItem* primary_item = dynamic_cast<const CPrimaryItem*>(m_Item.GetPointer());
    if (primary_item)
    {
        CBioseqContext* ctx = primary_item->GetContext();
        if (!ctx)
            return;
        m_AssosiatedObject.Reset(new CPrimaryEdit(ctx->GetHandle()));
    }

    const CDeflineItem* defLineItem = dynamic_cast<const CDeflineItem*>(m_Item.GetPointer());
    if (defLineItem) {
        CBioseqContext* ctx = defLineItem->GetContext();
        if (!ctx)
            return;

        m_AssosiatedObject.Reset(new CSeqdescTitleEdit(ctx->GetHandle(), defLineItem->GetDefline()));
        return;
    }
    const CSourceItem* sourceItem = dynamic_cast<const CSourceItem*>(m_Item.GetPointer());
    if (sourceItem) {
        CBioseqContext* ctx = sourceItem->GetContext();
        if (!ctx)
            return;

        CSeqdesc_CI dsrc_it(ctx->GetHandle(), CSeqdesc::e_Source);
        if (dsrc_it) {
            const CSeqdesc& seqDesc = *dsrc_it;
            m_AssosiatedObject.Reset(&seqDesc);
            return;
        }
        const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_Item->GetObject());
        if (seq_feat && seq_feat->GetData().IsBiosrc()) {
            m_AssosiatedObject.Reset(seq_feat);
            return;
        }
    }

    const CSourceFeatureItem* sourceFeatureItem
        = dynamic_cast<const CSourceFeatureItem*>(m_Item.GetPointer());
    if (sourceFeatureItem) {
        CBioseqContext* ctx = sourceFeatureItem->GetContext();
        if (!ctx)
            return;

        const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_Item->GetObject());
        if (!seq_feat)
            return;

        const CSeq_feat::TData& data = seq_feat->GetData();
        if (data.IsBiosrc()) {
            const CBioSource& bioSource = data.GetBiosrc();
            CSeqdesc_CI dsrc_it(ctx->GetHandle(), CSeqdesc::e_Source);
            if (dsrc_it) {
                const CSeqdesc& seqDesc = *dsrc_it;
                if (seqDesc.IsSource()) {
                    const CBioSource& bioSource2 = seqDesc.GetSource();
                    if (&bioSource2 == &bioSource) {
                        m_AssosiatedObject.Reset(&seqDesc);
                        return;
                    }
                }
            }
        }

        //CFeat_CI fsrc_it(ctx->GetHandle(), CSeqFeatData::e_Biosrc);
        //if (fsrc_it) {
        //		m_AssosiatedObject.Reset(fsrc_it->GetOriginalSeq_feat().GetPointer());
        //    return;
        //}
        m_AssosiatedObject.Reset(seq_feat);
        return;
    }

    const CGenomeProjectItem* genomeProjectItem
        = dynamic_cast<const CGenomeProjectItem*>(m_Item.GetPointer());
    if (genomeProjectItem) {
        CBioseqContext* ctx = genomeProjectItem->GetContext();
        if (!ctx)
            return;

        // extract all the useful user objects
        for (CSeqdesc_CI desc(ctx->GetHandle(), CSeqdesc::e_User); desc; ++desc) {
            const CUser_object& uo = desc->GetUser();

            if (!uo.GetType().IsStr()) {
                continue;
            }
            if (NStr::EqualNocase(uo.GetType().GetStr(), "DBLink")) {
                const CSeqdesc& seqDesc = *desc;
                m_AssosiatedObject.Reset(&seqDesc);
                return;
            }
        }
        return;
    }

    const CSerialObject* itemObject = m_Item->GetObject();
    if (!itemObject)
        return;

    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(itemObject);
    const CSequenceItem* seqItem =
        dynamic_cast<const CSequenceItem*>(m_Item.GetPointer());
    if (seqItem && bioseq) {
        CRef<CSeq_loc> loc(new CSeq_loc());
        loc->SetInt().SetFrom(seqItem->GetFrom() - 1);
        loc->SetInt().SetTo(seqItem->GetTo() - 1);
        try {
            loc->SetId(*(bioseq->GetFirstId()));
        } catch (const CObjMgrException&) {
            return;
        }
        m_AssosiatedObject.Reset(loc.GetPointer());
        return;
    } else {
		m_AssosiatedObject.Reset(itemObject);
	}

    if (m_AssosiatedObject)
        SetEditFlags(IBioseqEditor::kObjectEditable);
}

CFlatFileTextItem::~CFlatFileTextItem()
{
    delete m_EditToolbar;
}

void CFlatFileTextItem::InitToolbar()
{
    if (!m_EditToolbar && m_EditFlags != 0)
        m_EditToolbar = new CEditToolbar(this, m_EditFlags);
}

const CConstRef<CObject> CFlatFileTextItem::GetAssosiatedObject() const
{
	return m_AssosiatedObject;
}

void CFlatFileTextItem::CalcSize(wxDC& /*dc*/, CTextPanelContext* context)
{
    x_CalcLineCount(context);
    wxSize size = context->GetWSize();
    m_Size.Set(context->GetLeftMargin() + size.x*80, size.y*m_LineCount);
}

void CFlatFileTextItem::Draw(wxDC& dc, const wxRect& updateRect, CTextPanelContext* context)
{
    CTextSelectItem::Draw(dc, updateRect, context);

    if (m_EditToolbar) {
        wxRect rect = m_EditToolbar->GetRect();
        if (updateRect.Intersects(rect)) {
            wxPoint dcOrigin = context->GetDeviceOrigin();
            context->SetDeviceOrigin(wxPoint(dcOrigin.x + rect.GetLeft(), dcOrigin.y + rect.GetTop()));
            context->SetDeviceOrigin(dc);
            m_EditToolbar->Draw(dc, context);
            context->SetDeviceOrigin(dcOrigin);
        }
    }
}

void CFlatFileTextItem::MouseEvent(wxMouseEvent& event, CTextPanelContext& context)
{
    if (m_EditToolbar) {
        wxRect rect = m_EditToolbar->GetRect();
        if (rect.Contains(event.m_x, event.m_y)) {
            wxMouseEvent ev(event);
            ev.m_x -= rect.x;
            ev.m_y -= rect.y;
            m_EditToolbar->MouseEvent(ev);
            return;
        }
    }
    CTextSelectItem::MouseEvent(event, context);
}

void CFlatFileTextItem::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const
{
    CFlatFileViewContext* pcntx = static_cast<CFlatFileViewContext*>(context);
    CDrawLines draw(ostream);
    if (!m_Item->Skip())
        m_Item->Format(pcntx->GetFormatter(), draw);
}

void CFlatFileTextItem::x_CalcLineCount(CTextPanelContext* context)
{
    CFlatFileViewContext* pcntx = (CFlatFileViewContext*)context;
    _ASSERT(pcntx);
    if (!pcntx) {
        m_LineCount = 1;
        return;
    }

    CCountLines os;
    if (!m_Item->Skip()) {
        m_Item->Format(pcntx->GetFormatter(), os);
        m_LineCount = os.GetLines();
    }
    else
        m_LineCount = 1;
}

string CFlatFileTextItem::GetToolTipText(CTextPanelContext* context) const
{
    if (dynamic_cast<const CFeatureItemBase*>(m_Item.GetPointer()) == NULL)
        return "";

    CFlatFileViewContext* pcntx = static_cast<CFlatFileViewContext*>(context);
    CGetText os;
    if (!m_Item->Skip())
        m_Item->Format(pcntx->GetFormatter(), os);
    return os.Text();
}

END_NCBI_SCOPE
