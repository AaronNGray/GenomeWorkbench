/*  $Id: desktop_view_context.cpp 37872 2017-02-24 21:11:01Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/cmd_del_bioseq_set.hpp>
#include <gui/objutils/cmd_del_seq_graph.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/descriptor_change.hpp>

#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_create_align.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/cmd_add_seq_annot.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/seq_desktop/desktop_canvas.hpp>
#include <gui/widgets/seq_desktop/desktop_view_context.hpp>
#include <wx/menu.h>
#include <wx/window.h>
#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static
WX_DEFINE_MENU(kDesktopEditMenu)
    WX_MENU_ITEM(wxID_CUT)
    WX_MENU_ITEM(wxID_COPY)
    WX_MENU_ITEM(wxID_PASTE)
WX_END_MENU()

BEGIN_EVENT_TABLE(CDesktopViewContext, wxEvtHandler)
    EVT_MENU(wxID_CUT, CDesktopViewContext::OnCutSelection)
    EVT_UPDATE_UI(wxID_CUT, CDesktopViewContext::OnUpdateCutSelection)

    EVT_MENU(wxID_COPY, CDesktopViewContext::OnCopySelection)
    EVT_UPDATE_UI(wxID_COPY, CDesktopViewContext::OnUpdateCopySelection)
    
    EVT_MENU(wxID_PASTE, CDesktopViewContext::OnPasteSelection)
    EVT_UPDATE_UI(wxID_PASTE, CDesktopViewContext::OnUpdatePasteSelection)
END_EVENT_TABLE()


CDesktopViewContext::CDesktopViewContext(IBioseqEditor* bioseqEditor, ICommandProccessor& cmdProccessor,
    const CSerialObject* saved_object, ESavedObjectType saved_type)
    : m_BioseqEditor(bioseqEditor), m_CmdProccessor(cmdProccessor), m_Canvas(nullptr), m_SavedType(saved_type)
{
    if (m_BioseqEditor) {
        m_BioseqEditor->SetCallBack(this);
    }

    m_SavedObject.Reset(saved_object);
}

CDesktopViewContext::~CDesktopViewContext()
{
    if (m_Canvas) {
        m_Canvas->PopEventHandler();
        if (m_BioseqEditor) {
            m_Canvas->PopEventHandler();
        }
    }
}

wxMenu* CDesktopViewContext::CreateMenu() const
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* menu = cmd_reg.CreateMenu(kDesktopEditMenu);

    if (m_BioseqEditor) {
        wxMenu* editorMenu(m_BioseqEditor->CreateContextMenu(true));
        if (editorMenu) {
            Merge(*menu, *editorMenu);
        }
    }
    return menu;
}

void CDesktopViewContext::SetCanvas(CDesktopCanvas* canvas)
{
    if (m_Canvas) {
        m_Canvas->PopEventHandler();
        if (m_BioseqEditor) {
            m_Canvas->PopEventHandler();
        }
    }

    m_Canvas = canvas;

    if (m_Canvas) {
        m_Canvas->PushEventHandler(this);
        if (m_BioseqEditor) {
            m_Canvas->PushEventHandler(m_BioseqEditor);
        }
    }
}

void CDesktopViewContext::GetCurrentSelection(TEditObjectList& objects)
{
    CConstRef<IDesktopDataItem> data_item = x_GetSelectedDataItem();
    if (!data_item) return;

    const CDesktopCitSubItem* citsub_item = dynamic_cast<const CDesktopCitSubItem*>(data_item.GetPointer());
    const CDesktopContactInfoItem* contact_item = dynamic_cast<const CDesktopContactInfoItem*>(data_item.GetPointer());
    // if the selected item is a Cit-sub or a Contact-info object, the object passed to the editor is a Submit-block
    if (citsub_item) {
        objects.push_back(CEditObject(citsub_item->GetSubmitBlock(), data_item->GetSeqentryHandle()));
    }
    else if (contact_item) {
        objects.push_back(CEditObject(contact_item->GetSubmitBlock(), data_item->GetSeqentryHandle()));
    }
    else {
        // for the rest of objects
        objects.push_back(CEditObject(*data_item->GetObject(), data_item->GetSeqentryHandle()));
    }
    return;
}

CBioseq_Handle CDesktopViewContext::GetCurrentBioseq()
{
    return CBioseq_Handle();
}

CConstRef<IDesktopDataItem> CDesktopViewContext::x_GetSelectedDataItem() const
{
    const IDesktopItem* item = m_Canvas->GetSelection();
    if (!item) return CConstRef<IDesktopDataItem>();

    return item->GetDataItem();
}


void CDesktopViewContext::OnCutSelection(wxCommandEvent& event)
{
    CConstRef<IDesktopDataItem> data_item = x_GetSelectedDataItem();
    if (!data_item) return;

    if (auto item = dynamic_cast<const CDesktopSeqdescItem*>(data_item.GetPointer())) {
        CIRef<IEditCommand> deldesc_cmd(new CCmdDelDesc(item->GetSeqentryHandle(), item->GetSeqdesc()));
        if (deldesc_cmd) {
            m_SavedObject.Reset(&item->GetSeqdesc());
            m_SavedType = eSavedObjectType_Seqdesc;
            m_CmdProccessor.Execute(deldesc_cmd);
        }
    }
    else if (auto item = dynamic_cast<const CDesktopFeatItem*>(data_item.GetPointer())) {

        bool remove_protein = true;
        CSeq_feat_Handle fh = item->GetFeatHandle();
        if (fh.IsSetData() && fh.GetData().IsCdregion()) {
            wxMessageDialog dlg(NULL, _("Would you like to remove the protein as well?"), _("Question"),
                wxYES_DEFAULT | wxYES_NO | wxCANCEL | wxICON_QUESTION);
            switch (dlg.ShowModal()) {
            case wxID_YES:
                remove_protein = true;
                break;
            case wxID_NO:
                remove_protein = false;
                break;
            case wxID_CANCEL:
                return;
            }
        }

        CRef<CCmdComposite> delfeat_cmd = GetDeleteFeatureCommand(item->GetFeatHandle(), remove_protein);
        if (delfeat_cmd) {
            m_SavedObject.Reset(item->GetFeatHandle().GetOriginalSeq_feat());
            m_SavedType = eSavedObjectType_SeqFeat;
            m_CmdProccessor.Execute(delfeat_cmd);
        }
    }
    else if (auto item = dynamic_cast<const CDesktopAnnotItem*>(data_item.GetPointer())) {
        CIRef<IEditCommand> delannot_cmd(new CCmdDelSeq_annot(item->GetAnnotHandle()));
        if (delannot_cmd) {
            m_SavedObject.Reset(item->GetAnnotHandle().GetCompleteSeq_annot());
            m_SavedType = eSavedObjectType_SeqAnnot;
            m_CmdProccessor.Execute(delannot_cmd);
        }
    }
    else if (auto item = dynamic_cast<const CDesktopAlignItem*>(data_item.GetPointer())) {
        CIRef<IEditCommand> delalign_cmd(new CCmdDelSeq_align(item->GetAlignHandle()));
        if (delalign_cmd) {
            m_SavedObject.Reset(item->GetAlignHandle().GetSeq_align());
            m_SavedType = eSavedObjectType_SeqAlign;
            m_CmdProccessor.Execute(delalign_cmd);
        }
    }
    else if (auto item = dynamic_cast<const CDesktopBioseqItem*>(data_item.GetPointer())) {
        CRef<CCmdComposite> delseq_cmd = GetDeleteSequenceCommand(item->GetBioseqHandle());
        if (delseq_cmd) {
            m_SavedObject.Reset(item->GetBioseqHandle().GetCompleteBioseq());
            m_SavedType = eSavedObjectType_Bioseq;
            m_CmdProccessor.Execute(delseq_cmd);
        }
    }
    else if (auto item = dynamic_cast<const CDesktopContactInfoItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(&item->GetContactInfo());
        m_SavedType = eSavedObjectType_ContactInfo;

        const CSubmit_block& submit_block = item->GetSubmitBlock();
        CRef<CSerialObject> edited_subblock;
        edited_subblock.Reset((CSerialObject*)CSubmit_block::GetTypeInfo()->Create());
        edited_subblock->Assign(submit_block);
        CSubmit_block& new_subblock = dynamic_cast<CSubmit_block&>(*edited_subblock);

        new_subblock.ResetContact();

        CChangeSubmitBlockCommand* chg_subblock = new CChangeSubmitBlockCommand();
        CObject* actual = (CObject*)&submit_block;
        chg_subblock->Add(actual, CConstRef<CObject>(edited_subblock));
        CRef<CCmdComposite> cmd(new CCmdComposite("Remove contact info"));
        cmd->AddCommand(*chg_subblock);
        m_CmdProccessor.Execute(cmd);
    }
    else if (auto item = dynamic_cast<const CDesktopCitSubItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(&item->GetCitSub());
        m_SavedType = eSavedObjectType_CitSub;

        const CSubmit_block& submit_block = item->GetSubmitBlock();
        CRef<CSerialObject> edited_subblock;
        edited_subblock.Reset((CSerialObject*)CSubmit_block::GetTypeInfo()->Create());
        edited_subblock->Assign(submit_block);
        CSubmit_block& new_subblock = dynamic_cast<CSubmit_block&>(*edited_subblock);

        new_subblock.ResetCit();

        CChangeSubmitBlockCommand* chg_subblock = new CChangeSubmitBlockCommand();
        CObject* actual = (CObject*)&submit_block;
        chg_subblock->Add(actual, CConstRef<CObject>(edited_subblock));
        CRef<CCmdComposite> cmd(new CCmdComposite("Remove citsub"));
        cmd->AddCommand(*chg_subblock);
        m_CmdProccessor.Execute(cmd);
    }
    else if (auto item = dynamic_cast<const CDesktopBioseqsetItem*>(data_item.GetPointer())) {
        CBioseq_set_Handle bssh = item->GetBioseqsetHandle();
        CIRef<IEditCommand> delset_cmd(new CCmdDelBioseqSet(bssh));
        if (delset_cmd) {
            m_SavedObject.Reset(item->GetBioseqsetHandle().GetCompleteBioseq_set());
            m_SavedType = eSavedObjectType_Bioseqset;
            m_CmdProccessor.Execute(delset_cmd);
        }
    }
    else if (auto item = dynamic_cast<const CDesktopGraphItem*>(data_item.GetPointer())) {
        CIRef<IEditCommand> delgraph_cmd(new CCmdDelSeq_graph(item->GetGraphHandle()));
        if (delgraph_cmd) {
            m_SavedObject.Reset(item->GetGraphHandle().GetSeq_graph());
            m_SavedType = eSavedObjectType_SeqGraph;
            m_CmdProccessor.Execute(delgraph_cmd);
        }
    }

    event.Skip();
}

void CDesktopViewContext::OnCopySelection(wxCommandEvent& event)
{
    CConstRef<IDesktopDataItem> data_item = x_GetSelectedDataItem();
    if (!data_item) return;

    if (auto item = dynamic_cast<const CDesktopSeqdescItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(&item->GetSeqdesc());
        m_SavedType = eSavedObjectType_Seqdesc;
    }
    else if (auto item = dynamic_cast<const CDesktopFeatItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(item->GetFeatHandle().GetOriginalSeq_feat());
        m_SavedType = eSavedObjectType_SeqFeat;
    }
    else if (auto item = dynamic_cast<const CDesktopAnnotItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(item->GetAnnotHandle().GetCompleteSeq_annot());
        m_SavedType = eSavedObjectType_SeqAnnot;
    }
    else if (auto item = dynamic_cast<const CDesktopAlignItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(item->GetAlignHandle().GetSeq_align());
        m_SavedType = eSavedObjectType_SeqAlign;
    }
    else if (auto item = dynamic_cast<const CDesktopBioseqItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(item->GetBioseqHandle().GetCompleteBioseq());
        m_SavedType = eSavedObjectType_Bioseq;
    }
    else if (auto item = dynamic_cast<const CDesktopContactInfoItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(&item->GetContactInfo());
        m_SavedType = eSavedObjectType_ContactInfo;
    }
    else if (auto item = dynamic_cast<const CDesktopCitSubItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(&item->GetCitSub());
        m_SavedType = eSavedObjectType_CitSub;
    }
    else if (auto item = dynamic_cast<const CDesktopBioseqsetItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(item->GetBioseqsetHandle().GetCompleteBioseq_set());
        m_SavedType = eSavedObjectType_Bioseqset;
    }
    else if (auto item = dynamic_cast<const CDesktopGraphItem*>(data_item.GetPointer())) {
        m_SavedObject.Reset(item->GetGraphHandle().GetSeq_graph());
        m_SavedType = eSavedObjectType_SeqGraph;
    }

    event.Skip();
}

void CDesktopViewContext::OnPasteSelection(wxCommandEvent& event)
{
    CConstRef<IDesktopDataItem> data_item = x_GetSelectedDataItem();
    if (!data_item || !m_SavedObject || m_SavedType == eSavedObjectType_not_set) return;

    const string& type = data_item->GetType();

    switch (m_SavedType){
    case eSavedObjectType_Seqdesc:
    {
        const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(m_SavedObject.GetPointer());
        _ASSERT(desc);
        if (type == CBioseq::GetTypeInfo()->GetName()
            || type == CBioseq_set::GetTypeInfo()->GetName()) {
            CIRef<IEditCommand> add_desc(new CCmdCreateDesc(data_item->GetSeqentryHandle(), *desc));
            if (add_desc) {
                m_CmdProccessor.Execute(add_desc);
            }
        }
        break;
    }
    case eSavedObjectType_SeqFeat:
    {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(m_SavedObject.GetPointer());
        _ASSERT(feat);
        if (type == CBioseq::GetTypeInfo()->GetName()
            || type == CBioseq_set::GetTypeInfo()->GetName()
            || type == CSeq_annot::GetTypeInfo()->GetName()) {
            CSeq_entry_Handle seh = data_item->GetSeqentryHandle();
            CIRef<IEditCommand> add_feat(new CCmdCreateFeat(seh, *feat));
            if (add_feat) {
                m_CmdProccessor.Execute(add_feat);
            }
        }
        break;
    }
    case eSavedObjectType_SeqAnnot:
    {
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(m_SavedObject.GetPointer());
        _ASSERT(annot);
        if (type == CBioseq::GetTypeInfo()->GetName()
            || type == CBioseq_set::GetTypeInfo()->GetName()) {
            CSeq_entry_Handle seh = data_item->GetSeqentryHandle();
            CIRef<IEditCommand> add_annot(new CCmdCreateSeq_annot(seh, const_cast<CSeq_annot&>(*annot)));
            if (add_annot) {
                m_CmdProccessor.Execute(add_annot);
            }
        }
        break;
    }
    case eSavedObjectType_SeqAlign:
    {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(m_SavedObject.GetPointer());
        _ASSERT(align);
        if (type == CBioseq::GetTypeInfo()->GetName()
            || type == CBioseq_set::GetTypeInfo()->GetName()
            || type == CSeq_annot::GetTypeInfo()->GetName()) {
            CSeq_entry_Handle seh = data_item->GetSeqentryHandle();
            CIRef<IEditCommand> add_align(new CCmdCreateAlign(seh, *align));
            if (add_align) {
                m_CmdProccessor.Execute(add_align);
            }
        }
        break;
    }
    case eSavedObjectType_Bioseq:
    {
        const CBioseq* bseq = dynamic_cast<const CBioseq*>(m_SavedObject.GetPointer());
        _ASSERT(bseq);
        if (type == CBioseq_set::GetTypeInfo()->GetName()) {
            CRef<CSeq_entry> entry(new CSeq_entry);
            entry->SetSeq().Assign(*bseq);
            CIRef<IEditCommand> add_bseq(new CCmdAddSeqEntry(entry, data_item->GetSeqentryHandle()));
            if (add_bseq) {
                m_CmdProccessor.Execute(add_bseq);
            }
        }
        break;
    }
    case eSavedObjectType_ContactInfo:
    {
        const CContact_info* contact = dynamic_cast<const CContact_info*>(m_SavedObject.GetPointer());
        _ASSERT(contact);
        if (type == CSeq_submit::GetTypeInfo()->GetName()) {
            if (auto item = dynamic_cast<const CDesktopSeqSubmitItem*>(data_item.GetPointer())) {
                const CSeq_submit& submit = item->GetSeqSubmit();
                // don't paste it if there isn't a submiblock present already
                if (!submit.IsSetSub()) return; 

                const CSubmit_block& submit_block = submit.GetSub();
                CRef<CSerialObject> edited_subblock;
                edited_subblock.Reset((CSerialObject*)CSubmit_block::GetTypeInfo()->Create());
                edited_subblock->Assign(submit_block);
                CSubmit_block& new_subblock = dynamic_cast<CSubmit_block&>(*edited_subblock);

                new_subblock.SetContact().Assign(*contact);

                CChangeSubmitBlockCommand* chg_subblock = new CChangeSubmitBlockCommand();
                CObject* actual = (CObject*)&submit_block;
                chg_subblock->Add(actual, CConstRef<CObject>(edited_subblock));
                CRef<CCmdComposite> cmd(new CCmdComposite("Change contact info"));
                cmd->AddCommand(*chg_subblock);
                m_CmdProccessor.Execute(cmd);
            }
        }
        break;
    }
    case eSavedObjectType_CitSub:
    {
        const CCit_sub* citsub = dynamic_cast<const CCit_sub*>(m_SavedObject.GetPointer());
        _ASSERT(citsub);
        if (type == CBioseq::GetTypeInfo()->GetName()
            || type == CBioseq_set::GetTypeInfo()->GetName()) {
            CRef<CPub> new_pub(new CPub);
            new_pub->SetSub().Assign(*citsub);
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->SetPub().SetPub().Set().push_back(new_pub);
            CIRef<IEditCommand> add_desc(new CCmdCreateDesc(data_item->GetSeqentryHandle(), *new_desc));
            if (add_desc) {
                m_CmdProccessor.Execute(add_desc);
            }
        }
        else if (type == CSeq_submit::GetTypeInfo()->GetName()) {
            if (auto item = dynamic_cast<const CDesktopSeqSubmitItem*>(data_item.GetPointer())) {
                const CSeq_submit& submit = item->GetSeqSubmit();
                // don't paste it if there isn't a submiblock present already
                if (!submit.IsSetSub()) return;

                const CSubmit_block& submit_block = submit.GetSub();
                CRef<CSerialObject> edited_subblock;
                edited_subblock.Reset((CSerialObject*)CSubmit_block::GetTypeInfo()->Create());
                edited_subblock->Assign(submit_block);
                CSubmit_block& new_subblock = dynamic_cast<CSubmit_block&>(*edited_subblock);

                new_subblock.SetCit().Assign(*citsub);

                CChangeSubmitBlockCommand* chg_subblock = new CChangeSubmitBlockCommand();
                CObject* actual = (CObject*)&submit_block;
                chg_subblock->Add(actual, CConstRef<CObject>(edited_subblock));
                CRef<CCmdComposite> cmd(new CCmdComposite("Change cit-sub"));
                cmd->AddCommand(*chg_subblock);
                m_CmdProccessor.Execute(cmd);
            }
        }
        break;
    }
    case eSavedObjectType_Bioseqset:
    {
        const CBioseq_set* bset = dynamic_cast<const CBioseq_set*>(m_SavedObject.GetPointer());
        _ASSERT(bset);
        if (type == CBioseq_set::GetTypeInfo()->GetName()) {
            CRef<CSeq_entry> entry(new CSeq_entry);
            entry->SetSet().Assign(*bset);
            CIRef<IEditCommand> add_bset(new CCmdAddSeqEntry(entry, data_item->GetSeqentryHandle()));
            if (add_bset) {
                m_CmdProccessor.Execute(add_bset);
            }
        }
        break;
    }
    case eSavedObjectType_SeqGraph:
    {
        // not implemented yet
    }
    default:
        break;
    } 
}

void CDesktopViewContext::OnUpdateCutSelection(wxUpdateUIEvent& event)
{
    x_UpdateCutCopySelection(event);
}
void CDesktopViewContext::OnUpdateCopySelection(wxUpdateUIEvent& event)
{
    x_UpdateCutCopySelection(event);
}
void CDesktopViewContext::OnUpdatePasteSelection(wxUpdateUIEvent& event)
{
    (m_SavedObject) ? event.Enable(true) : event.Enable(false);
}

void CDesktopViewContext::x_UpdateCutCopySelection(wxUpdateUIEvent& event)
{
    event.Enable(false);
    CConstRef<IDesktopDataItem> data_item = x_GetSelectedDataItem();
    if (!data_item) return;

    if (!dynamic_cast<const CDesktopSeqSubmitItem*>(data_item.GetPointer())) {
        event.Enable(true);
    }
}

void CDesktopViewContext::EditSelection()
{
    CConstRef<IDesktopDataItem> data_item = x_GetSelectedDataItem();
    if (!data_item || !m_BioseqEditor) return;

    m_BioseqEditor->EditSelection();
}

END_NCBI_SCOPE

