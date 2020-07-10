/*  $Id: edit_tb.cpp 39823 2017-11-08 17:14:23Z katargir $
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

#include "edit_tb.hpp"
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/wx/ibioseq_editor.hpp>

#include <wx/dcclient.h>

BEGIN_NCBI_SCOPE

wxBitmap CEditToolbar::m_EditBmp;
wxBitmap CEditToolbar::m_DeleteBmp;

CEditToolbar::CEditToolbar(ITextItem* item, int editFlags) : m_Item(item), m_EditFlags(editFlags)
{
    SetSize(wxSize(32, 16));

    static bool bitmap_registered = false;
    if (!bitmap_registered) {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("edit_tb::edit"), wxT("pen.png"));
        provider->RegisterFileAlias(wxT("edit_tb::delete"), wxT("delete.png"));
        bitmap_registered = true;
    }

    if (!m_EditBmp.IsOk())
        m_EditBmp = wxArtProvider::GetBitmap(wxT("edit_tb::edit"));

    if (!m_DeleteBmp.IsOk())
        m_DeleteBmp = wxArtProvider::GetBitmap(wxT("edit_tb::delete"));
}

void CEditToolbar::Draw(wxDC& dc, CTextPanelContext* WXUNUSED(context))
{
    if (m_EditBmp.IsOk() && (m_EditFlags & IBioseqEditor::kObjectEditable) != 0)
        dc.DrawBitmap(m_EditBmp, 0, 0);
    if (m_DeleteBmp.IsOk() && (m_EditFlags & IBioseqEditor::kObjectDeletable) != 0)
        dc.DrawBitmap(m_DeleteBmp, 16, 0);
}

void CEditToolbar::MouseEvent(wxMouseEvent& event)
{
    if (event.Moving())
        MouseMove(event);
    else if (event.LeftUp())
        MouseLeftUp(event);
}

void CEditToolbar::MouseLeftUp(wxMouseEvent& event)
{
    CTextItemPanel* panel = (CTextItemPanel*)event.GetEventObject();
    CTextPanelContext* context = panel->GetContext();
    context->SelectItem(m_Item, false, false);

    if (event.m_x < 16) {
        if ((m_EditFlags & IBioseqEditor::kObjectEditable) == 0)
            return;
        context->EditItem(m_Item, false, false);
        //wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, eCmdEditObjects);
        //evt.SetEventObject(panel);
        //panel->GetEventHandler()->AddPendingEvent(evt);
    }
    else {
        if ((m_EditFlags & IBioseqEditor::kObjectDeletable) == 0)
            return;
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, eCmdDeleteObjects);
        evt.SetEventObject(panel);
        panel->GetEventHandler()->AddPendingEvent(evt);
    }
}

void CEditToolbar::MouseMove(wxMouseEvent& event)
{
    CTextItemPanel* panel = static_cast<CTextItemPanel*>(event.GetEventObject());
    if (event.m_x < 16) {
        panel->SetCursor((m_EditFlags & IBioseqEditor::kObjectEditable) != 0 
            ? wxCursor(wxCURSOR_HAND) : wxCursor(wxCURSOR_IBEAM));
    }
    else {
        panel->SetCursor((m_EditFlags & IBioseqEditor::kObjectDeletable) != 0 
            ? wxCursor(wxCURSOR_HAND) : wxCursor(wxCURSOR_IBEAM));
    }
}

END_NCBI_SCOPE
