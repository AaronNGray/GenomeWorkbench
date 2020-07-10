/*  $Id: rearrange_list.cpp 37971 2017-03-09 14:59:46Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */
  

#include <ncbi_pch.hpp>


// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif // WX_PRECOMP

#include "wx/button.h"
#include "wx/stattext.h"
#include "wx/sizer.h"

#include <gui/widgets/edit/rearrange_list.hpp>

BEGIN_NCBI_SCOPE

wxBEGIN_EVENT_TABLE(CRearrangeList, wxCheckListBox)
    EVT_CHECKLISTBOX(wxID_ANY, CRearrangeList::OnCheck)
    EVT_LISTBOX_DCLICK(wxID_ANY, CRearrangeList::OnDoubleClick)
wxEND_EVENT_TABLE()

bool CRearrangeList::Create(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             const wxArrayInt& order,
                             const wxArrayString& items,
                             long style,
                             const wxValidator& validator,
                             const wxString& name)
{
    // construct the array of items in the order in which they should appear in
    // the control
    const size_t count = items.size();
    wxCHECK_MSG( order.size() == count, false, "arrays not in sync" );
    m_checked = 0;
    m_Listener = NULL;
    m_DoubleClickConsumer = NULL;

    wxArrayString itemsInOrder;
    itemsInOrder.reserve(count);
    size_t n;
    for ( n = 0; n < count; n++ )
    {
        int idx = order[n];
        if ( idx < 0 )
            idx = -idx - 1;
        itemsInOrder.push_back(items[idx]);
    }

    // do create the real control
    if ( !wxCheckListBox::Create(parent, id, pos, size, itemsInOrder,
                                 style, validator, name) )
        return false;

    // and now check all the items which should be initially checked
    for ( n = 0; n < count; n++ )
    {
        if ( order[n] >= 0 )
        {
            // Be careful to call the base class version here and not our own
            // which would also update m_order itself.
            wxCheckListBox::Check(n);
            m_checked++;
        }
    }

    m_order = order;

    return true;
}

bool CRearrangeList::CanMoveCurrentUp() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && sel != 0;
}

bool CRearrangeList::CanMoveCurrentDown() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && static_cast<unsigned>(sel) != GetCount() - 1;
}

bool CRearrangeList::MoveCurrentUp()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || sel == 0 )
        return false;

    Swap(sel, sel - 1);
    SetSelection(sel - 1);

    return true;
}

bool CRearrangeList::MoveCurrentDown()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || static_cast<unsigned>(sel) == GetCount() - 1 )
        return false;

    Swap(sel, sel + 1);
    SetSelection(sel + 1);

    return true;
}

void CRearrangeList::Swap(int pos1, int pos2)
{
    // update the internally stored order
    wxSwap(m_order[pos1], m_order[pos2]);


    // and now also swap all the attributes of the items

    // first the label
    const wxString stringTmp = GetString(pos1);
    SetString(pos1, GetString(pos2));
    SetString(pos2, stringTmp);

    // then the checked state
    const bool checkedTmp = IsChecked(pos1);
    wxCheckListBox::Check(pos1, IsChecked(pos2));
    wxCheckListBox::Check(pos2, checkedTmp);

    // and finally the client data, if necessary
    switch ( GetClientDataType() )
    {
        case wxClientData_None:
            // nothing to do
            break;

        case wxClientData_Object:
            {
                wxClientData * const dataTmp = DetachClientObject(pos1);
                SetClientObject(pos1, DetachClientObject(pos2));
                SetClientObject(pos2, dataTmp);
            }
            break;

        case wxClientData_Void:
            {
                void * const dataTmp = GetClientData(pos1);
                SetClientData(pos1, GetClientData(pos2));
                SetClientData(pos2, dataTmp);
            }
            break;
    }
}

void CRearrangeList::Check(unsigned int item, bool check)
{
    if ( check == IsChecked(item) )
        return;

    wxCheckListBox::Check(item, check);

    m_order[item] = ~m_order[item];
    if (check)
        m_checked++;
    else
        m_checked--;

    if (m_Listener)
        m_Listener->UpdateCheckedState(m_checked);
}

void CRearrangeList::OnCheck(wxCommandEvent& event)
{
    // update the internal state to match the new item state
    const int n = event.GetInt();

    if ( (m_order[n] >= 0) != IsChecked(n) )
    {
        m_order[n] = ~m_order[n];
        if (m_order[n] >= 0 )
            m_checked++;
        else
            m_checked--;

        if (m_Listener)
            m_Listener->UpdateCheckedState(m_checked);
    }
}

void CRearrangeList::OnDoubleClick(wxCommandEvent& event)
{
    // update the internal state to match the new item state
    const int n = event.GetInt();

    if (m_DoubleClickConsumer)
        m_DoubleClickConsumer->DoubleClick(n);
}

void CRearrangeList::Clear()
{
    wxCheckListBox::Clear();
    m_order.clear();
    m_checked = 0;
}

void CRearrangeList::Set(const wxArrayInt& order, const wxArrayString& items)
{
    const size_t count = items.size();
    if ( order.size() != count)
        return;

    Clear();    
    wxArrayString itemsInOrder;
    itemsInOrder.reserve(count);
    size_t n;
    for ( n = 0; n < count; n++ )
    {
        int idx = order[n];
        if ( idx < 0 )
            idx = -idx - 1;
        itemsInOrder.push_back(items[idx]);
    }
    
    // do create the real control
    wxCheckListBox::Set(itemsInOrder);
    m_checked = 0;
    // and now check all the items which should be initially checked
    for ( n = 0; n < count; n++ )
    {
        if ( order[n] >= 0 )
        {
            // Be careful to call the base class version here and not our own
            // which would also update m_order itself.
            wxCheckListBox::Check(n);
            m_checked++;
        }
    }

    m_order = order;
}

// ============================================================================
// CRearrangeCtrl implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(CRearrangeCtrl, wxPanel)
    EVT_UPDATE_UI(wxID_UP, CRearrangeCtrl::OnUpdateButtonUI)
    EVT_UPDATE_UI(wxID_DOWN, CRearrangeCtrl::OnUpdateButtonUI)

    EVT_BUTTON(wxID_UP, CRearrangeCtrl::OnButton)
    EVT_BUTTON(wxID_DOWN, CRearrangeCtrl::OnButton)
wxEND_EVENT_TABLE()

void CRearrangeCtrl::Init()
{
    m_list = NULL;
}

bool
CRearrangeCtrl::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        const wxArrayInt& order,
                        const wxArrayString& items,
                        long style,
                        const wxValidator& validator,
                        const wxString& name)
{
    // create all the windows
    if ( !wxPanel::Create(parent, id, pos, size, wxTAB_TRAVERSAL, name) )
        return false;

    m_list = new CRearrangeList(this, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 order, items,
                                 style, validator);
    wxButton * const btnUp = new wxButton(this, wxID_UP);
    wxButton * const btnDown = new wxButton(this, wxID_DOWN);

    // arrange them in a sizer
    wxSizer * const sizerBtns = new wxBoxSizer(wxVERTICAL);
    sizerBtns->Add(btnUp, wxSizerFlags().Centre().Border(wxBOTTOM));
    sizerBtns->Add(btnDown, wxSizerFlags().Centre().Border(wxTOP));

    wxSizer * const sizerTop = new wxBoxSizer(wxHORIZONTAL);
    sizerTop->Add(m_list, wxSizerFlags(1).Expand().Border(wxRIGHT));
    sizerTop->Add(sizerBtns, wxSizerFlags(0).Centre().Border(wxLEFT));
    SetSizer(sizerTop);

    m_list->SetFocus();

    return true;
}

void CRearrangeCtrl::OnUpdateButtonUI(wxUpdateUIEvent& event)
{
    event.Enable( event.GetId() == wxID_UP ? m_list->CanMoveCurrentUp()
                                           : m_list->CanMoveCurrentDown() );
}

void CRearrangeCtrl::OnButton(wxCommandEvent& event)
{
    if ( event.GetId() == wxID_UP )
        m_list->MoveCurrentUp();
    else
        m_list->MoveCurrentDown();
}


END_NCBI_SCOPE

