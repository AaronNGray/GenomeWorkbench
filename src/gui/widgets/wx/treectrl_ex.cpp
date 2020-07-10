/*  $Id: treectrl_ex.cpp 36910 2016-11-14 19:48:00Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/treectrl_ex.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <gui/utils/utils_platform.hpp>

#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/imaglist.h>


BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE(CwxTreeCtrlEx, wxTreeCtrl)
    EVT_LEFT_DOWN(CwxTreeCtrlEx::OnLeftDown)
    EVT_LEFT_DCLICK(CwxTreeCtrlEx::OnLeftDown)
    EVT_KEY_DOWN(CwxTreeCtrlEx::OnKeyDown)
    EVT_SET_FOCUS(CwxTreeCtrlEx::OnFocus)
END_EVENT_TABLE()


CwxTreeCtrlEx::CwxTreeCtrlEx(wxWindow *parent, wxWindowID id,
            const wxPoint& pos, const wxSize& size,
            long style, const wxValidator& validator,
            const wxString& name)
{
    style = style & ~wxTR_MULTIPLE;
    wxTreeCtrl::Create(parent, id, pos, size, style, validator, name);
    
    wxImageList * imglist = new wxImageList(13, 13);    
    
    wxFileArtProvider* provider = GetDefaultFileArtProvider();

    provider->RegisterFileAlias(wxT("treectrl_ex::notchecked"),
                                wxT("tree_unchecked_icon.png"));
    provider->RegisterFileAlias(wxT("treectrl_ex::halfchecked"),
                                wxT("tree_tristate_icon.png"));
    provider->RegisterFileAlias(wxT("treectrl_ex::checked"),
                                wxT("tree_checked_icon.png"));
    /*
    provider->RegisterFileAlias(wxT("treectrl_ex::notchecked"),
                                wxT("zoom_in.png"));
    provider->RegisterFileAlias(wxT("treectrl_ex::halfchecked"),
                                wxT("zoom_in.png"));
    provider->RegisterFileAlias(wxT("treectrl_ex::checked"),
                                wxT("zoom_in.png"));

    */
        
    imglist->Add(wxArtProvider::GetBitmap(wxT("treectrl_ex::notchecked")));
    imglist->Add(wxArtProvider::GetBitmap(wxT("treectrl_ex::halfchecked")));
    imglist->Add(wxArtProvider::GetBitmap(wxT("treectrl_ex::checked")));

    //imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));
    //imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));
    //imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));

    wxTreeCtrl::SetImageList(imglist);  
    
}

CwxTreeCtrlEx::~CwxTreeCtrlEx()
{
    delete wxTreeCtrl::GetImageList();
}

void CwxTreeCtrlEx::OnLeftDown(wxMouseEvent &event)
{
    int flags;
    wxTreeItemId id = HitTest(event.GetPosition(), flags);
    if (id.IsOk() && (flags & wxTREE_HITTEST_ONITEMICON)) {
        x_SetState(id);
    }
    else {
        event.Skip();   
    }
}

void CwxTreeCtrlEx::OnKeyDown(wxKeyEvent &event)
{   
    if (event.GetKeyCode() == WXK_SPACE && GetSelection().IsOk()) {
        x_SetState(GetSelection());
    } else {
        event.Skip();
    }
}

void CwxTreeCtrlEx::x_SetState(const wxTreeItemId &id, TTreeState state)
{   
    x_ValidateState(id, x_ModifyState(id, state));
}

CwxTreeCtrlEx::TTreeState CwxTreeCtrlEx::x_ModifyState(const wxTreeItemId &id, TTreeState state)
{
    TTreeState cur_state =
        (TTreeState)wxTreeCtrl::GetItemImage(id);

    switch (state) {
    // toggle
    case TREE_EX_UNDEF:
        {{
            if (cur_state == TREE_EX_UNCHECKED ||
                cur_state == TREE_EX_THREESTATE) {
                    cur_state = TREE_EX_CHECKED;
            }
            else if (cur_state == TREE_EX_CHECKED) {
                cur_state = TREE_EX_UNCHECKED;
            }
            break;
        }}
    case TREE_EX_CHECKED:
    case TREE_EX_UNCHECKED:
        {{
            cur_state = state;
            break;
        }}
    case TREE_EX_THREESTATE:
        break;
    };
    
    return cur_state;
}

CwxTreeCtrlEx::TTreeState CwxTreeCtrlEx::x_ValidateState(const wxTreeItemId &id, TTreeState state)
{   
    x_ProcessDownstream(id, state);
    x_ProcessUpstream(id,   state);
    return state;
}

CwxTreeCtrlEx::TTreeState CwxTreeCtrlEx::x_ProcessDownstream(const wxTreeItemId &id, TTreeState state)
{
    // update downstream
    if (state==TREE_EX_CHECKED ||  state==TREE_EX_UNCHECKED) {
        wxTreeItemIdValue cookie;
        for (wxTreeItemId cid = GetFirstChild(id, cookie); cid.IsOk(); cid=GetNextChild(id, cookie)){
            x_ModifyState(cid,  state);
            x_ProcessDownstream(cid, state);
        }
        wxTreeCtrl::SetItemImage(id, state);
    }   
    return state;
}

CwxTreeCtrlEx::TTreeState CwxTreeCtrlEx::x_ProcessUpstream(const wxTreeItemId &id, TTreeState state)
{
    wxTreeItemId parent = GetItemParent(id);
    TTreeState parent_state = TREE_EX_UNCHECKED;

    if (parent.IsOk()) {        
        
        wxTreeItemIdValue cookie;
        int               nmb_checked = 0;
        int               nmb_all     = 0;
        bool              bSomethingChecked = false;
    
        for (wxTreeItemId cid = GetFirstChild(parent, cookie); cid.IsOk(); cid=GetNextChild(parent, cookie)){
            bSomethingChecked |= (wxTreeCtrl::GetItemImage(cid) == TREE_EX_CHECKED);
            bSomethingChecked |= (wxTreeCtrl::GetItemImage(cid) == TREE_EX_THREESTATE);

            if (wxTreeCtrl::GetItemImage(cid) == TREE_EX_CHECKED)  nmb_checked++;
            nmb_all++;
        }

        if (bSomethingChecked) {
            parent_state = ((nmb_checked == nmb_all) ? TREE_EX_CHECKED : TREE_EX_THREESTATE);
        }
        wxTreeCtrl::SetItemImage(parent, parent_state);
        x_ProcessUpstream(parent, parent_state);
    }
    return parent_state;
}


void CwxTreeCtrlEx::OnFocus(wxFocusEvent &event)
{
    event.Skip();
}


void CwxTreeCtrlEx::CheckAll(bool bcheck)
{
    x_SetState(GetRootItem(), bcheck ? TREE_EX_CHECKED : TREE_EX_UNCHECKED);
}

void CwxTreeCtrlEx::SetCheck(const wxTreeItemId& item, bool bcheck)
{
    x_SetState(item, bcheck ? TREE_EX_CHECKED : TREE_EX_UNCHECKED);
}

bool CwxTreeCtrlEx::GetCheck(const wxTreeItemId& item)
{
    return wxTreeCtrl::GetItemImage(item) == TREE_EX_CHECKED;
}

void CwxTreeCtrlEx::SetCheckMultiple(const wxArrayTreeItemIds& ids)
{
    for( size_t ix = 0; ix < ids.size(); ix++ ){
        x_SetState( ids[ix], TREE_EX_CHECKED);
    }
}

size_t CwxTreeCtrlEx::GetCheckedMultiple(wxArrayTreeItemIds& ids) const
{
    ids.empty();
    wxTreeItemId root =  GetRootItem();
    x_GetCheckedMultiple(ids, root);
    return ids.size();
}

void CwxTreeCtrlEx::x_GetCheckedMultiple(wxArrayTreeItemIds& ids, wxTreeItemId& item) const
{
    if (wxTreeCtrl::GetItemImage(item) == TREE_EX_CHECKED) {
        ids.Add(item);
    }
    
    wxTreeItemIdValue cookie;
    for (wxTreeItemId cid = GetFirstChild(item, cookie); cid.IsOk(); cid=GetNextChild(item, cookie)){
        x_GetCheckedMultiple(ids, cid);
    }   
}

END_NCBI_SCOPE

