/*  $Id: asn_view_context.cpp 41060 2018-05-16 18:11:00Z katargir $
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

#include <gui/objutils/interface_registry.hpp>
#include <gui/utils/command_processor.hpp>

#include <gui/widgets/seq/asn_view_context.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/seq/asn_header_item.hpp>
#include <gui/widgets/seq/asn_element_block_start.hpp>
#include <gui/widgets/seq/asn_element_item.hpp>

#include <gui/widgets/text_widget/plain_text_item.hpp>

#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CAsnViewContext::CAsnViewContext(objects::CScope& scope, const CSerialObject* so, IBioseqEditor* bioseqEditor, bool tooltpContext)
    : CTextPanelContext(1, &scope, so),
      m_BioseqEditor(bioseqEditor),
      m_TooltipContext(tooltpContext),
      m_ShowAsnTypes(false),
      m_ShowPathToRoot(false)
{
    if (m_BioseqEditor)
        m_BioseqEditor->SetCallBack(this);
}

void CAsnViewContext::SetPanel(CTextItemPanel* panel)
{
    if (m_Panel && m_BioseqEditor) {
        m_Panel->PopEventHandler();
    }

    CTextPanelContext::SetPanel(panel);

    if (m_Panel && m_BioseqEditor) {
        m_Panel->PushEventHandler(m_BioseqEditor);
    }
}

CAsnViewContext::~CAsnViewContext()
{
    if (m_Panel && m_BioseqEditor) {
        m_Panel->PopEventHandler();
    }
}

void CAsnViewContext::SetShowAsnTypes(bool show)
{
    m_ShowAsnTypes = show;
}

void CAsnViewContext::SetShowPathToRoot(bool show)
{
    m_ShowPathToRoot = show;
}

void CAsnViewContext::MouseMoveInContainer(ITextItem* container)
{
    CCompositeTextItem* composite = dynamic_cast<CCompositeTextItem*>(container);
    if (composite == 0 || composite->GetItemCount() == 0)
        return;

    CAsnHeaderItem* headerItem = dynamic_cast<CAsnHeaderItem*>(composite->GetItem(0));
    if (headerItem) {
        AddHier(headerItem);
        return;
    }

    CAsnElementBlockStart* blockStart = dynamic_cast<CAsnElementBlockStart*>(composite->GetItem(0));
    if (blockStart) {
        AddHier(blockStart);
        return;
    }
}

wxWindow* CAsnViewContext::CreateTooltipWindow()
{
    if (!m_ShowPathToRoot || m_MouseOverItem == 0 || m_Hier.empty())
        return NULL;

    CTextItemPanel* tooltipWnd = new CTextItemPanel( m_Panel, wxID_ANY, wxDefaultPosition, wxSize(0,0), wxBORDER_SIMPLE|wxHSCROLL|wxVSCROLL );
    tooltipWnd->Hide();
    tooltipWnd->SetBackgroundColour(wxColor(255, 255, 225));

    CCompositeTextItem* mainItem = new CCompositeTextItem();

    CPlainTextItem* info = new CPlainTextItem();
    info->AddLine("Path to Root:");
    mainItem->AddItem(info);

    for (size_t i = 0; i < m_Hier.size(); ++i) {
        ITextItem* item = m_Hier[i]->Clone();
        if (item) {
            mainItem->AddItem(item);
        }
    }

    CTextPanelContext* context = new CAsnViewContext(*m_Scope, 0, 0, true);

    try {
        wxClientDC dc(tooltipWnd);
        tooltipWnd->InitDC(dc);
        context->CalcWSize(dc);
        mainItem->CalcSize(dc, context);
    } catch (const exception&) {
    }
    wxSize size = mainItem->GetSize();
    tooltipWnd->SetClientSize(size);

    tooltipWnd->SetMainItem(mainItem, context);
    tooltipWnd->Layout();

    return tooltipWnd;
}

namespace {
    class CUpdtateEditingTraverser : public ICompositeTraverser
    {
    public:
        CUpdtateEditingTraverser(IBioseqEditor& editor, CScope& scope)
            : m_Editor(editor), m_Scope(scope) {}

        virtual bool ProcessItem(CTextItem& textItem)
        {
            CAsnElementItem* asnItem = dynamic_cast<CAsnElementItem*>(&textItem);
            if (asnItem)
                x_UpdateEditFlags(*asnItem);
            return true;
        }
        virtual bool ProcessItem(CExpandItem& expandItem)
        {
            int flags = 0;
            CAsnElementItem* asnItem = dynamic_cast<CAsnElementItem*>(expandItem.GetCollapsedItem());
            if (asnItem)
                flags = x_UpdateEditFlags(*asnItem);
            expandItem.SetSelectable(flags != 0);

            return true;
        }

    private:
        int x_UpdateEditFlags(CAsnElementItem& asnItem)
        {
            int flags = 0;
            CEditObject eo = asnItem.GetEditObject(m_Scope);
            if (eo.m_Obj)
                flags = m_Editor.GetEditFlags(eo);
            asnItem.SetEditFlags(flags);
            return flags;
        }

        IBioseqEditor& m_Editor;
        CScope&        m_Scope;
    };
}

void CAsnViewContext::RootInitialized()
{
    if (m_Panel && m_BioseqEditor) {
        CCompositeTextItem* root = dynamic_cast<CCompositeTextItem*>(m_Panel->GetRootItem());
        if (root) {
            CUpdtateEditingTraverser traverser(*m_BioseqEditor, *m_Scope);
            for (size_t i = 0; i < root->GetItemCount(); ++i)
                root->GetItem(i)->Traverse(traverser);
        }
    }
}


wxMenu* CAsnViewContext::CreateMenu() const
{
    if (m_BioseqEditor) {
        return m_BioseqEditor->CreateContextMenu();
    }
    return 0;
}

void CAsnViewContext::GetCurrentSelection(TEditObjectList& objects)
{
    ITERATE (set<ITextItem*>, it, m_SelectedItems) {
        CAsnElementItem* asnItem = 0;
        CExpandItem* expandItem = dynamic_cast<CExpandItem*>(*it);
        if (expandItem) {
            asnItem = dynamic_cast<CAsnElementItem*>(expandItem->GetCollapsedItem());
        }
        else
            asnItem = dynamic_cast<CAsnElementItem*>(*it);

        if (!asnItem)
            continue;

        CEditObject eo = asnItem->GetEditObject(*m_Scope);
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

CBioseq_Handle CAsnViewContext::GetCurrentBioseq()
{
    return CBioseq_Handle();
}

wxString CAsnViewContext::GetWorkDir() const
{
    return m_WorkDir;
}


END_NCBI_SCOPE
