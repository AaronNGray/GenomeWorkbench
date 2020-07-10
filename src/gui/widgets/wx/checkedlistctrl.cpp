/*  $Id: checkedlistctrl.cpp 37741 2017-02-13 18:14:02Z katargir $
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

#include <gui/widgets/wx/checkedlistctrl.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/imaglist.h>

#if( wxMAJOR_VERSION == 3 && wxMINOR_VERSION == 0 )
wxDEFINE_EVENT(wxEVT_LIST_ITEM_CHECKED, wxListEvent);
#endif

BEGIN_NCBI_SCOPE

IMPLEMENT_CLASS(wxCheckedListCtrl, wxListCtrl)

BEGIN_EVENT_TABLE(wxCheckedListCtrl, wxListCtrl)
    EVT_LEFT_DOWN(wxCheckedListCtrl::OnMouseEvent)
END_EVENT_TABLE()

wxCheckedListCtrl::wxCheckedListCtrl() : m_ImageList()
{
}

wxCheckedListCtrl::wxCheckedListCtrl(wxWindow *parent, wxWindowID winid, const wxPoint& pos, const wxSize& size,
                                     long style, const wxValidator& validator, const wxString &name)
: m_ImageList()
{
    Create(parent, winid, pos, size, style, validator, name);
}

wxCheckedListCtrl::~wxCheckedListCtrl()
{
    delete m_ImageList;
}

bool wxCheckedListCtrl::Create(wxWindow *parent, wxWindowID winid, const wxPoint& pos, const wxSize& size,
                               long style, const wxValidator& validator, const wxString &name)
{
    if (!wxListCtrl::Create(parent, winid, pos, size, style, validator, name))
        return FALSE;

    m_ImageList = CreateCheckboxImages(this);
    SetImageList(m_ImageList, wxIMAGE_LIST_SMALL);

    return TRUE;
}

void wxCheckedListCtrl::OnMouseEvent(wxMouseEvent& event)
{
    if (event.LeftDown())
    {
        int flags;
        long item = HitTest(event.GetPosition(), flags);
        if (item > -1 && (flags & wxLIST_HITTEST_ONITEMICON))
        {
            SetChecked(item, !IsChecked(item));
            wxListEvent le(wxEVT_LIST_ITEM_CHECKED, GetId());
            le.m_itemIndex = item;
            le.SetEventObject(this);
            GetEventHandler()->ProcessEvent(le);
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

bool wxCheckedListCtrl::IsChecked(long item) const
{
    wxListItem info;
    info.m_mask = wxLIST_MASK_IMAGE;
    info.m_itemId = item;

    if (GetItem(info))
    {
        return (info.m_image == 1);
    }
    else
        return FALSE;
}

void wxCheckedListCtrl::SetChecked(long item, bool checked)
{
    SetItemImage(item, (checked ? 1 : 0), -1);
}

END_NCBI_SCOPE

