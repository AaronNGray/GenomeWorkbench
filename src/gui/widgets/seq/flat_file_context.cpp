/*  $Id: flat_file_context.cpp 41060 2018-05-16 18:11:00Z katargir $
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

#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/expand_traverser.hpp>
#include <gui/widgets/text_widget/draw_text_stream.hpp>

#include <gui/widgets/seq/flat_file_context.hpp>
#include <gui/widgets/seq/flat_file_view_params.hpp>
#include <gui/widgets/seq/flat_file_header_item.hpp>
#include <gui/widgets/seq/flat_file_locus_item.hpp>
#include <gui/widgets/seq/flat_file_seq_block_iterator.hpp>
#include "text_commands.hpp"

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>

#include <objtools/format/items/locus_item.hpp>
#include <objtools/format/items/defline_item.hpp>
#include <objtools/format/items/source_item.hpp>
#include <objtools/format/items/comment_item.hpp>
#include <objtools/format/items/feature_item.hpp>
#include <objtools/format/items/genome_project_item.hpp>
#include <objtools/format/items/sequence_item.hpp>
#include <objtools/format/items/primary_item.hpp>

#include <gui/objutils/primary_edit.hpp>

#include "features_group.hpp"
#include "sequence_group.hpp"

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "edit_tb.hpp"

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

enum
{
    kExpandAllFeatures = 210,
    kCollapseAllFeatures,
    kExpandSeqBlocks,
    kCollapseSeqBlocks,
    kCollapseSequences
};

BEGIN_EVENT_TABLE( CFlatFileViewContext, CTextPanelContext )
    EVT_MENU(kExpandAllFeatures, CFlatFileViewContext::OnExpandAllFeatures)
    EVT_MENU(kCollapseAllFeatures, CFlatFileViewContext::OnCollapseAllFeatures)
    EVT_MENU(kExpandSeqBlocks, CFlatFileViewContext::OnExpandSeqBlocks)
    EVT_MENU(kCollapseSeqBlocks, CFlatFileViewContext::OnCollapseSeqBlocks)

    EVT_MENU(kCollapseSequences, CFlatFileViewContext::OnCollapseSequences)
    EVT_MENU(kExpandAll, CFlatFileViewContext::OnExpandAll)
END_EVENT_TABLE()

CFlatFileViewContext::CFlatFileViewContext(
    objects::IFormatter& formatter,
    objects::CScope& scope,
    const CSerialObject* so,
    IBioseqEditor* bioseqEditor)
    : CTextPanelContext(3, &scope, so),
      m_Formatter(&formatter),
      m_BioseqEditor(bioseqEditor)
{
    if (m_BioseqEditor)
        m_BioseqEditor->SetCallBack(this);
}

CFlatFileViewContext::~CFlatFileViewContext()
{
    if (m_Panel && m_BioseqEditor) {
        m_Panel->PopEventHandler();
    }
}

int CFlatFileViewContext::GetLeftMargin() const
{
    if (ShowEditIcons())
        return 32 + 2 * GetWWidth();
    else
        return 3 * GetWWidth();
}

bool CFlatFileViewContext::ShowEditIcons() const
{
    return m_BioseqEditor && !RunningInsideNCBI();
}

namespace {
    class CAddToolbarTraverser : public ICompositeTraverser
    {
    public:
        CAddToolbarTraverser(IBioseqEditor& editor, bool showIcons)
            : m_Editor(editor), m_ShowIcons(showIcons) {}

        virtual bool ProcessItem(CTextItem& textItem)
        {
            CFlatFileTextItem* flatTextItem = dynamic_cast<CFlatFileTextItem*>(&textItem);
            if (!flatTextItem)
                return true;

            const IFlatItem* flatItem = flatTextItem->GetFlatItem();
            if (!flatItem) {
                flatTextItem->SetEditFlags(0);
                return true;
            }

            flatTextItem->SetEditFlags(m_Editor.GetEditFlags(*flatItem));
            if (m_ShowIcons)
                flatTextItem->InitToolbar();

            return true;
        }

        virtual bool ProcessItem(CExpandItem& expandItem)
        {
            CFlatFileTextItem* flatTextItem = dynamic_cast<CFlatFileTextItem*>(expandItem.GetExpandedItem());
            if (!flatTextItem)
                return true;

            const IFlatItem* flatItem = flatTextItem->GetFlatItem();
            if (!flatItem)
                return true;

            int flags = m_Editor.GetEditFlags(*flatItem);

            expandItem.SetSelectable(flags != 0);
            if (flags != 0 && m_ShowIcons)
                expandItem.AddGlyph(new CEditToolbar(&expandItem, flags));

            return true;
        }

    private:
        IBioseqEditor& m_Editor;
        bool m_ShowIcons;
    };
}

void CFlatFileViewContext::RootInitialized()
{
    if (!m_Panel || !m_BioseqEditor)
        return;

    CCompositeTextItem* root = dynamic_cast<CCompositeTextItem*>(m_Panel->GetRootItem());
    if (root) {
        CAddToolbarTraverser traverser(*m_BioseqEditor, ShowEditIcons());
        for (size_t i = 0; i < root->GetItemCount(); ++i)
            root->GetItem(i)->Traverse(traverser);
    }
}

void CFlatFileViewContext::SetPanel(CTextItemPanel* panel)
{
    if (m_Panel && m_BioseqEditor) {
        m_Panel->PopEventHandler();
    }

    CTextPanelContext::SetPanel(panel);

    if (m_Panel && m_BioseqEditor) {
        m_Panel->PushEventHandler(m_BioseqEditor);
    }
}

class CFindLocusTraverser : public ICompositeTraverser
{
public:
    CFindLocusTraverser(TConstScopedObjects& objects) : m_SeqLocs(objects) {}

    virtual bool ProcessItem(CTextItem& textItem) {
		CFlatFileLocusItem* locusItem = dynamic_cast<CFlatFileLocusItem*>(&textItem);
        if (locusItem) {
            SConstScopedObject seq_loc = locusItem->GetSeqLoc();
		    if (seq_loc.object)
                m_SeqLocs.push_back(seq_loc);
            return false;
        }
        return true;
	}
    virtual bool ProcessItem(CExpandItem& expandItem) { return true; }

    TConstScopedObjects& m_SeqLocs;
};

void CFlatFileViewContext::GetVisibleObjects(TConstScopedObjects& objects)
{
    CCompositeTextItem* root = dynamic_cast<CCompositeTextItem*>(m_Panel->GetRootItem());
    if (!root) return;
    CFindLocusTraverser finder(objects);
    for (size_t i = 0; i < root->GetItemCount(); ++i) {
        root->GetItem(i)->Traverse(finder);
    }
}

void CFlatFileViewContext::RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider&)
{
    static bool registered = false;
    if (!registered) {
        registered = true;

        cmd_reg.RegisterCommand(kExpandAllFeatures,
            "Expand All Features",
            "Expand All Features",
            NcbiEmptyString,
            NcbiEmptyString,
            "Command expands(opens) All Features of a FlatFile view");
        cmd_reg.RegisterCommand(kCollapseAllFeatures,
            "Collapse All Features",
            "Collapse All Features",
            NcbiEmptyString,
            NcbiEmptyString,
            "Command collapses(closes) All Features of a FlatFile view");
        cmd_reg.RegisterCommand(kExpandSeqBlocks,
            "Expand Sequence",
            "Expand Sequence",
            NcbiEmptyString,
            NcbiEmptyString,
            "Command expands(opens) All Sequence Blocks of a FlatFile view");
        cmd_reg.RegisterCommand(kCollapseSeqBlocks,
            "Collapse Sequence",
            "Collapse Sequence",
            NcbiEmptyString,
            NcbiEmptyString,
            "Command collapses(closes) Sequence Blocks of a FlatFile view");
        cmd_reg.RegisterCommand(kCollapseSequences,
            "Collapse Sequences",
            "Collapse Sequences",
            NcbiEmptyString,
            NcbiEmptyString,
            "Command collapses(closes) Sequence Sections of a FlatFile view");
    }
}

static
WX_DEFINE_MENU(kFlatFileContextMenu)
    WX_MENU_SEPARATOR_L("Edit Selection")
    WX_MENU_SEPARATOR_L("FlatFile Blocks")
    WX_MENU_ITEM(kCollapseSequences)
    WX_MENU_ITEM(kExpandAll)
WX_END_MENU()

wxMenu* CFlatFileViewContext::CreateMenu() const
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* menu = cmd_reg.CreateMenu(kFlatFileContextMenu);
    if (m_BioseqEditor) {
        unique_ptr<wxMenu> editorMenu(m_BioseqEditor->CreateContextMenu());
        if (editorMenu.get()) {
            Merge(*menu, *editorMenu);
        }
    }
    return menu;
}

void CFlatFileViewContext::EditItem(ITextItem* WXUNUSED(item), bool controlDown, bool shiftDown)
{
    if (!m_BioseqEditor || controlDown || shiftDown)
        return;

    m_BioseqEditor->EditSelection();
}

void CFlatFileViewContext::OnExpandAllFeatures(wxCommandEvent& WXUNUSED(event))
{
/*
    CCompositeTextItem& composite = m_Panel->GetDisplayItem();
    bool update = false;
    for (CCompositeIterator sect_it(*composite, typeid(CCompositeTextItem)); sect_it; ++sect_it) {
        for (CCompositeIterator it(dynamic_cast<CCompositeTextItem&>(*sect_it), typeid(CFeaturesGroup));
                it; ++it) {
            dynamic_cast<CFeaturesGroup&>(*it).ExpandAll(this);
            update = true;
        }
    }

    if (update) {
        m_Panel->Layout();
        m_Panel->Refresh();
    }
 */
}

void CFlatFileViewContext::OnCollapseAllFeatures(wxCommandEvent& WXUNUSED(event))
{
/*
    CCompositeTextItem& composite = m_Panel->GetDisplayItem();
    bool update = false;
    for (CCompositeIterator sect_it(*composite, typeid(CCompositeTextItem)); sect_it; ++sect_it) {
        for (CCompositeIterator it(dynamic_cast<CCompositeTextItem&>(*sect_it), typeid(CFeaturesGroup));
                it; ++it) {
            dynamic_cast<CFeaturesGroup&>(*it).CollapseAll(this);
            update = true;
        }
    }

    if (update) {
        m_Panel->Layout();
        m_Panel->Refresh();
    }
 */
}

void CFlatFileViewContext::OnExpandSeqBlocks(wxCommandEvent& WXUNUSED(event))
{
/*
    CCompositeTextItem& composite = m_Panel->GetDisplayItem();
    bool update = false;
    for (CCompositeIterator sect_it(*composite, typeid(CCompositeTextItem)); sect_it; ++sect_it) {
        for (CCompositeIterator it(dynamic_cast<CCompositeTextItem&>(*sect_it), typeid(CSequenceGroup));
                it; ++it) {
            dynamic_cast<CSequenceGroup&>(*it).ExpandAll(this);
            update = true;
        }
    }

    if (update) {
        m_Panel->Layout();
        m_Panel->Refresh();
    }
 */
}

void CFlatFileViewContext::OnCollapseSeqBlocks(wxCommandEvent& WXUNUSED(event))
{
/*
    CCompositeTextItem& composite = m_Panel->GetDisplayItem();
    bool update = false;
    for (CCompositeIterator sect_it(*composite, typeid(CCompositeTextItem)); sect_it; ++sect_it) {
        for (CCompositeIterator it(dynamic_cast<CCompositeTextItem&>(*sect_it), typeid(CSequenceGroup));
                it; ++it) {
            dynamic_cast<CSequenceGroup&>(*it).CollapseAll(this);
            update = true;
        }
    }

    if (update) {
        m_Panel->Layout();
        m_Panel->Refresh();
    }
 */
}

class CExpandSeqsTraverser : public ICompositeTraverser
{
public:
    CExpandSeqsTraverser(CTextPanelContext& context, bool expand) : m_Context(context), m_Expand(expand) {}

    virtual bool ProcessItem(CTextItem& /*textItem*/) { return true; }
    virtual bool ProcessItem(CExpandItem& expandItem)
    {
		CFlatFileSectionHeader* header = dynamic_cast<CFlatFileSectionHeader*>(expandItem.GetCollapsedItem());
		if (header) {
            if ((m_Expand && !expandItem.IsExpanded()) || (!m_Expand && expandItem.IsExpanded())) {
                expandItem.Toggle(&m_Context, false);
            }
            return false;
        }
        return true;
    }
    CTextPanelContext& m_Context;
    bool m_Expand;
};


void CFlatFileViewContext::OnCollapseSequences(wxCommandEvent& event)
{
    CCompositeTextItem* root = dynamic_cast<CCompositeTextItem*>(m_Panel->GetRootItem());
    if (!root) return;

    for (size_t i = 0; i < root->GetItemCount(); ++i) {
        CExpandSeqsTraverser expander(*this, false);
        root->GetItem(i)->Traverse(expander);
    }

    root->UpdatePositions();
    m_Panel->Layout();
    m_Panel->Refresh();
    m_Panel->UpdateCaretPos();
}

void CFlatFileViewContext::OnExpandAll(wxCommandEvent& event)
{
    CCompositeTextItem* root = dynamic_cast<CCompositeTextItem*>(m_Panel->GetRootItem());
    if (!root) return;

    CExpandTraverser expander(*this, true);
    root->Traverse(expander);
    root->UpdatePositions();
    m_Panel->Layout();
    m_Panel->Refresh();

    m_Panel->UpdateCaretPos();
}

static
WX_DEFINE_MENU(kFlatFileBioseqMenu)
    WX_SUBMENU("BioSource")
        WX_MENU_SEPARATOR_L("Edit Bioseq")
    WX_END_SUBMENU()
WX_END_MENU()

void CFlatFileViewContext::OnEditBioseq(objects::CBioseq_Handle& bh, wxPoint pos)
{
    if (!m_BioseqEditor)
        return;

    pos = m_Panel->CalcScrolledPosition(pos);

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    auto_ptr<wxMenu> menu(cmd_reg.CreateMenu(kFlatFileBioseqMenu));

    {{
        auto_ptr<wxMenu> editorMenu(m_BioseqEditor->CreateBioseqMenu());
        if (editorMenu.get()) {
            Merge(*menu, *editorMenu);
        }
    }}

    CleanupSeparators(*menu);

    m_CurBioseq = bh;
    m_Panel->PopupMenu(menu.get(), pos);
}

CEditObject CFlatFileViewContext::GetEditObject(CFlatFileTextItem& ffItem)
{
    const objects::IFlatItem* item = ffItem.m_Item.GetPointer();

    const CSeqdesc* odesc = dynamic_cast<const CSeqdesc*>(item->GetObject());
    if (odesc) {
        const CFlatItem* fitem = dynamic_cast<const CFlatItem*>(item);
        if (fitem) {
            CBioseqContext* ctx = fitem->GetContext();
            if (ctx) {
                return CEditObject(*odesc, ctx->GetHandle().GetSeq_entry_Handle());
            }
        }
    }
    const CSeq_feat* ofeat = dynamic_cast<const CSeq_feat*>(item->GetObject());
    if (ofeat) {
        const CFlatItem* fitem = dynamic_cast<const CFlatItem*>(item);
        if (fitem) {
            CBioseqContext* ctx = fitem->GetContext();
            if (ctx) {
                return CEditObject(*ofeat, ctx->GetHandle().GetSeq_entry_Handle());
            }
        }
    }

    const CSourceItem* sourceItem = dynamic_cast<const CSourceItem*>(item);
    if (sourceItem) {
        CBioseqContext* ctx = sourceItem->GetContext();
        if (!ctx)
            return CEditObject();

        CBioseq_Handle& bh = ctx->GetHandle();
        CSeqdesc_CI dsrc_it(bh, CSeqdesc::e_Source);
        if (dsrc_it) {
            return CEditObject(*dsrc_it, bh.GetSeq_entry_Handle());
        }
        CFeat_CI fsrc_it(bh, CSeqFeatData::e_Biosrc);
        if (fsrc_it) {
            CSeq_feat_Handle fh = *fsrc_it;
            return CEditObject(*fh.GetOriginalSeq_feat(), bh.GetSeq_entry_Handle());
        }
        return CEditObject();
    }

    const CSequenceItem* seqItem = dynamic_cast<const CSequenceItem*>(item);
    if (seqItem) {
        CBioseqContext* ctx = seqItem->GetContext();
        if (!ctx)
            return CEditObject();

        CBioseq_Handle& bh = ctx->GetHandle();
        return CEditObject(*bh.GetRangeSeq_loc(seqItem->GetFrom() - 1, seqItem->GetTo() - 1), bh.GetSeq_entry_Handle());
    }

    const CCommentItem* commentItem = dynamic_cast<const CCommentItem*>(item);
    if (commentItem) {
        const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(commentItem->GetObject());
        if (desc) {
            CBioseqContext* ctx = commentItem->GetContext();
            if (!ctx)
                return CEditObject();

            CBioseq_Handle& bh = ctx->GetHandle();

            return CEditObject(*desc, bh.GetSeq_entry_Handle());
        }
        return CEditObject();
    }
               
    const CSourceFeatureItem* sourceFeatureItem
        = dynamic_cast<const CSourceFeatureItem*>(item);
    if (sourceFeatureItem) {
        const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(item->GetObject());
        if (!seq_feat)
            return CEditObject();

        CBioseqContext* ctx = sourceFeatureItem->GetContext();
        if (!ctx)
            return CEditObject();

        CBioseq_Handle& bh = ctx->GetHandle();

        const CSeq_feat::TData& data = seq_feat->GetData();
        if (data.IsBiosrc()) {
            const CBioSource& bioSource = data.GetBiosrc();
            CSeqdesc_CI dsrc_it(ctx->GetHandle(), CSeqdesc::e_Source);
            if (dsrc_it) {
                const CSeqdesc& seqDesc = *dsrc_it;
                if (seqDesc.IsSource()) {
                    const CBioSource& bioSource2 = seqDesc.GetSource();
                    if (&bioSource2 == &bioSource) {
                        return CEditObject(seqDesc, bh.GetSeq_entry_Handle());
                    }
                }
            }
        }

        //CFeat_CI fsrc_it(ctx->GetHandle(), CSeqFeatData::e_Biosrc);
        //if (fsrc_it) {
        //    CSeq_feat_Handle fh = *fsrc_it;
        //    objects.push_back(CEditObject(*fh.GetOriginalSeq_feat(), bh.GetSeq_entry_Handle()));
        //    continue;
        //}
        return CEditObject(*seq_feat, bh.GetSeq_entry_Handle());
    }

    const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(item);
    if (locusItem) {
        CBioseqContext* ctx = locusItem->GetContext();
        if (!ctx)
            return CEditObject();

        CBioseq_Handle& bh = ctx->GetHandle();

        const CMolInfo* molInfo = dynamic_cast<const CMolInfo*>(item->GetObject());
        if (!molInfo)
            return CEditObject();
        CSeqdesc_CI dsrc_it(ctx->GetHandle(), CSeqdesc::e_Molinfo);
        for (; dsrc_it; ++dsrc_it) {
            const CSeqdesc& seqDesc = *dsrc_it;
            if (&seqDesc.GetMolinfo() == molInfo) {
                return CEditObject(seqDesc, bh.GetSeq_entry_Handle());
            }
        }
        return CEditObject();
    }

    const CDeflineItem* defLineItem = dynamic_cast<const CDeflineItem*>(item);
    if (defLineItem) {
        CBioseqContext* ctx = defLineItem->GetContext();
        if (!ctx)
            return CEditObject();
        const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(item->GetObject());
        if (desc) {
            return CEditObject(*desc, ctx->GetHandle().GetSeq_entry_Handle());
        }

        CBioseq_Handle& bh = ctx->GetHandle();
        CSeqdesc_CI di(bh, CSeqdesc::e_Title);
        if (di) {
            return CEditObject(*di, bh.GetSeq_entry_Handle());
        }
        else {
            CRef<CTitleEdit> titleEdit(new CTitleEdit());
            titleEdit->SetTitle(defLineItem->GetDefline());
            return CEditObject(*titleEdit, bh.GetSeq_entry_Handle());
        }
    }


    const CReferenceItem* refItem = dynamic_cast<const CReferenceItem*>(item);
    if (refItem) {
        CBioseqContext* ctx = refItem->GetContext();
        if (!ctx)
            return CEditObject();

        CBioseq_Handle& bh = ctx->GetHandle();

        const CSubmit_block* submitBlock = dynamic_cast<const CSubmit_block*>(item->GetObject());
        if (submitBlock)
            return CEditObject(*submitBlock, bh.GetSeq_entry_Handle());

        const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(item->GetObject());
        if (seq_feat) {
            CSeq_feat_Handle fh = ctx->GetScope().GetSeq_featHandle(*seq_feat, CScope::eMissing_Null);
            if (fh)
                return CEditObject(*fh.GetOriginalSeq_feat(), bh.GetSeq_entry_Handle());
            return CEditObject();
        }
        const CSeqdesc* seqDesc = dynamic_cast<const CSeqdesc*>(item->GetObject());
        if (seqDesc && seqDesc->IsPub())
            return CEditObject(*seqDesc, bh.GetSeq_entry_Handle());

        return CEditObject();
    }


    const CFeatureItem* featureItem = dynamic_cast<const CFeatureItem*>(item);
    if (featureItem) {
        const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(item->GetObject());
        if (!seq_feat)
            return CEditObject();

        CBioseqContext* ctx = featureItem->GetContext();
        if (!ctx)
            return CEditObject();

        CSeq_feat_Handle fh = ctx->GetScope().GetSeq_featHandle(*seq_feat, CScope::eMissing_Null);
        if (fh) {
            CBioseq_Handle& bh = ctx->GetHandle();
            return CEditObject(*fh.GetOriginalSeq_feat(), bh.GetSeq_entry_Handle());
        }
        return CEditObject();
    }


    const CGenomeProjectItem* genomeProjectItem = dynamic_cast<const CGenomeProjectItem*>(item);
    if (genomeProjectItem) {
        CBioseqContext* ctx = genomeProjectItem->GetContext();
        if (!ctx)
            return CEditObject();

        // extract all the useful user objects
        
        for (CSeqdesc_CI desc(ctx->GetHandle(), CSeqdesc::e_User); desc; ++desc) {
            const CUser_object& uo = desc->GetUser();

            if (!uo.GetType().IsStr())
                return CEditObject();

            if (NStr::EqualNocase(uo.GetType().GetStr(), "DBLink")) {
                CBioseq_Handle& bh = ctx->GetHandle();
                const CSeqdesc& seqDesc = *desc;
                return CEditObject(seqDesc, bh.GetSeq_entry_Handle());
            }
        }
        return CEditObject();
    }


    const CPrimaryItem* primary_item = dynamic_cast<const CPrimaryItem*>(item);
    if (primary_item) {
        CBioseqContext* ctx = primary_item->GetContext();
        if (!ctx)
            return CEditObject();

        for (CSeqdesc_CI desc(ctx->GetHandle(), CSeqdesc::e_User); desc; ++desc) {
            const CUser_object& uo = desc->GetUser();

            if (!uo.GetType().IsStr())
                return CEditObject();

            if (NStr::EqualNocase(uo.GetType().GetStr(), "TpaAssembly")) {
                CBioseq_Handle& bh = ctx->GetHandle();
                const CSeqdesc& seqDesc = *desc;
                return CEditObject(seqDesc, bh.GetSeq_entry_Handle());
            }
        }
        return CEditObject();
    }

    return CEditObject();
}

void CFlatFileViewContext::GetItemSelection(TFlatItemList& itemList)
{
    ITERATE(set<ITextItem*>, it, m_SelectedItems) {
        CFlatFileTextItem* ffItem = 0;
        CExpandItem* expandItem = dynamic_cast<CExpandItem*>(*it);

        if (expandItem)
            ffItem = dynamic_cast<CFlatFileTextItem*>(expandItem->GetExpandedItem());
        else
            ffItem = dynamic_cast<CFlatFileTextItem*>(*it);

        if (!ffItem || ffItem->GetEditFlags() == 0 || ffItem->GetFlatItem() == nullptr)
            continue;

        itemList.emplace_back(ffItem->GetFlatItem());
    }
}

void CFlatFileViewContext::GetCurrentSelection(TEditObjectList& objects)
{
    ITERATE (set<ITextItem*>, it, m_SelectedItems) {
        CFlatFileTextItem* ffItem = 0;
        CExpandItem* expandItem = dynamic_cast<CExpandItem*>(*it);

        if (expandItem)
            ffItem = dynamic_cast<CFlatFileTextItem*>(expandItem->GetExpandedItem());
        else
            ffItem = dynamic_cast<CFlatFileTextItem*>(*it);

        if (!ffItem)
            continue;

        CEditObject eo = GetEditObject(*ffItem);
        if (!eo.m_Obj)
            continue;
        objects.push_back(eo);
    }

	set<const CObject*> objSet;
	for (TEditObjectList::iterator it2 = objects.begin(); it2 != objects.end();) {
		const CObject* pobj = it2->m_Obj.GetPointer();
		if (objSet.find(pobj) != objSet.end()) {
			it2 = objects.erase(it2);
		} else {
			objSet.insert(pobj);
			++it2;
		}
	}
}

CBioseq_Handle CFlatFileViewContext::GetCurrentBioseq()
{
    return m_CurBioseq;
}

wxString CFlatFileViewContext::GetWorkDir() const
{
    return m_WorkDir;
}

bool CFlatFileViewContext::FindSequence(const wxString& findText)
{
    if (!m_Panel) return false;

    CTextPosition cursorPos = m_Panel->GetTextSelection().Normalized().GetStart();
    cursorPos.ShiftCol(1);

    string text = ToStdString(findText);
    NStr::TruncateSpacesInPlace(text);
    if (text.empty()) return false;

    NStr::ToUpper(text);
    size_t findLength = text.length();
    if (findLength > 60) {
        text = text.substr(0, 60);
        findLength = 60;
    }

    for(CFlatFileSeqBlockIterator it(*m_Panel, cursorPos); it;) {
        string block1 = *it, block2;
        string buffer = block1;
        CFlatFileSeqBlockIterator it2 = it;
        ++it2;
        if (it2) {
            block2 = *it2;
            if (findLength > 1 && !it2.NextSequence())
                buffer += block2.substr(0, findLength - 1);
        }
        if (buffer.length() >= findLength) {
            NStr::ToUpper(buffer);
            size_t pos = buffer.find(text);
            if (pos != string::npos) {
                CTextPosition start = it.GetPosition(pos);
                CTextPosition end = it.GetPosition(pos + findLength - 1);
                end.ShiftCol(1);
                m_Panel->MakePosVisible(start);
                m_MarkedSeq = CTextBlock(start, end);
                m_Panel->SetSelection(CTextBlock(start, start));
                return true;
            }
        }
        it = it2;
    }

    return false;
}

namespace {
class CSeqMarkertModifier : public CDrawTextOStream::IStyleModifier
{
public:
    static CDrawTextOStream::IStyleModifier& GetInstance()
    {
        static CSeqMarkertModifier _inst;
        return _inst;
    }
    virtual void SetStyle(wxDC& dc)
    {
        //dc.SetBackgroundMode(wxTRANSPARENT);
        //dc.SetTextForeground(wxColor(222,50,0));

        dc.SetBackgroundMode(wxSOLID);
        dc.SetTextForeground(wxColor(0,0,0));
        dc.SetTextBackground(wxColor(175,238,238));
    }
};
}

void CFlatFileViewContext::InitDrawStream(CDrawTextOStream& stream, int startLine)
{
    if (m_MarkedSeq.IsEmpty()) return;

    CTextBlock mark = m_MarkedSeq;
    mark.ShiftRow(-startLine);

    int startRow = m_MarkedSeq.GetStart().GetRow() - startLine;
    int endRow = m_MarkedSeq.GetEnd().GetRow() - startLine;

    CDrawTextOStream::IStyleModifier& seqMarker = CSeqMarkertModifier::GetInstance();

    if (startRow == endRow) {
        stream.AddMarker(startRow, m_MarkedSeq.GetStart().GetCol(),
                         startRow, m_MarkedSeq.GetEnd().GetCol(), &seqMarker);
        return;
    }
    
    stream.AddMarker(startRow, m_MarkedSeq.GetStart().GetCol(),
                        startRow, 80, &seqMarker);
    for (int i = startRow + 1; i < endRow; ++i)
        stream.AddMarker(i, 10, i, 80, &seqMarker);
    stream.AddMarker(endRow, 10, endRow, m_MarkedSeq.GetEnd().GetCol(), &seqMarker);
}

END_NCBI_SCOPE
