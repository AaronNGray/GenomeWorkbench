/*  $Id: track_list_ctrl.cpp 43790 2019-08-30 16:32:37Z katargir $
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

#include <wx/imaglist.h>

#include "track_list_ctrl.hpp"

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/dcmemory.h>

BEGIN_NCBI_SCOPE

wxDEFINE_EVENT(wxEVT_SET_VISIBILITY, wxCommandEvent);

#define EVT_SET_VISIBILITY(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_SET_VISIBILITY, id, wxID_ANY, \
        wxCommandEventHandler(fn), \
        (wxObject *) NULL \
    ),

BEGIN_EVENT_TABLE(CTrackListCtrl, wxListCtrl)
    EVT_LEFT_DOWN(CTrackListCtrl::OnLeftDown)
    EVT_LIST_COL_CLICK(wxID_ANY, CTrackListCtrl::OnColumnClick)
    EVT_SET_VISIBILITY(wxID_ANY, CTrackListCtrl::OnSetVisibility)
END_EVENT_TABLE()

static wxBitmap s_BitmapFrom7x4(int w, int h, const char* const * bits)
{
    vector<string> strings;

    if (w <= 0 || h <= 0)
        return wxBitmap(w, h);

    strings.push_back(NStr::NumericToString(w) + " " + NStr::NumericToString(h) + " 2 1");
    strings.emplace_back("   c None");
    strings.emplace_back("X  c Gray25");

    for (int i = 0; i < h; ++i) {
        strings.emplace_back(w, ' ');
        int ii = i - (h - 4) / 2;
        if (ii >= 0 && ii < 4) {
            if (w < 7)
                strings.back().replace(0, w, string(bits[ii] + (7 - w) / 2, w));
            else
                strings.back().replace((w - 7) / 2, 7, string(bits[ii], 7));
        }
    }

    vector<const char*> xpm;
    for (const string& s : strings)
        xpm.push_back(s.c_str());
    
    return wxBitmap(&xpm[0]);
}

CTrackListCtrl::CTrackListCtrl(wxWindow *parent, wxWindowID id, const wxPoint&, const wxSize&, long, const wxValidator&, const wxString&)
    : m_ImageList(), m_SortColumn(0)
{
    wxListCtrl::Create(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL);

    m_ImageList = CreateCheckboxImages(this);
    wxSize size = m_ImageList->GetSize();

    static const char * up_xpm[] = {
        "   X   ",
        "  XXX  ",
        " XXXXX ",
        "XXXXXXX"};

    static const char * dn_xpm[] = {
        "XXXXXXX",
        " XXXXX ",
        "  XXX  ",
        "   X   "};

    m_ImageList->Add(s_BitmapFrom7x4(size.x, size.y, up_xpm));
    m_ImageList->Add(s_BitmapFrom7x4(size.x, size.y, dn_xpm));

    SetImageList(m_ImageList, wxIMAGE_LIST_SMALL);

    InsertColumn(0, wxT("Track Name"), wxLIST_FORMAT_LEFT, 250);
    InsertColumn(1, wxT("Accession"), wxLIST_FORMAT_LEFT);
    InsertColumn(2, wxT("Category"), wxLIST_FORMAT_LEFT);
    InsertColumn(3, wxT("Description"), wxLIST_FORMAT_LEFT);

    x_UpdateHeader();
    x_Sort();
}

CTrackListCtrl::~CTrackListCtrl()
{
    delete m_ImageList;
}

void CTrackListCtrl::SetTracks(const TTracks& tracks)
{
    m_Tracks = tracks;
    m_TracksVisible = m_Tracks;

    SetItemCount(m_TracksVisible.size());
    SetColumnWidth(1, wxLIST_AUTOSIZE);
    x_Sort();
    Refresh();
}

void CTrackListCtrl::OnLeftDown(wxMouseEvent& evt)
{
    int flags;
    long item = HitTest(evt.GetPosition(), flags);
    if (item >= 0 && item < (long)m_TracksVisible.size() && (flags & wxLIST_HITTEST_ONITEMICON)) {
        ITrackInfo* track = m_TracksVisible[item];
        wxCommandEvent eventCustom(wxEVT_SET_VISIBILITY);
        eventCustom.SetInt(track->GetVisible() ? 0 : 1);
        wxPostEvent(this, eventCustom);
        if (!GetItemState(item, wxLIST_STATE_SELECTED))
            evt.Skip();
    }
    else {
        evt.Skip();
    }
}

void CTrackListCtrl::OnColumnClick(wxListEvent &evt)
{
    int col = evt.GetColumn();
    if (col < 0)
        m_SortColumn = 0;
    else {
        if (abs(m_SortColumn) == col + 1) {
            m_SortColumn = -m_SortColumn;
            reverse(m_TracksVisible.begin(), m_TracksVisible.end());
        }
        else {
            m_SortColumn = col + 1;
            x_Sort();
        }
    }
    x_UpdateHeader();
    Refresh();
}

void CTrackListCtrl::x_UpdateHeader()
{
    for (int i = 0; i < GetColumnCount(); ++i) {
        wxListItem item;
        item.SetAlign(wxLIST_FORMAT_LEFT);
        if (abs(m_SortColumn) == i + 1)
            item.SetImage(m_SortColumn < 0 ? 5 : 4);
        else
            item.SetImage(-1);
        item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_FORMAT);
        SetColumn(i, item);
    }
}

void CTrackListCtrl::x_Sort()
{
    wxBusyCursor wait;

    if (m_TracksVisible.size() < 2)
        return;

    if (abs(m_SortColumn) == 1)
        sort(m_TracksVisible.begin(), m_TracksVisible.end(),
        [](const ITrackInfo* t1, const ITrackInfo* t2)
        -> bool { return NStr::CompareNocase(t1->GetDisplayName(), t2->GetDisplayName()) < 0; });
    else if (abs(m_SortColumn) == 2)
        sort(m_TracksVisible.begin(), m_TracksVisible.end(),
        [](const ITrackInfo* t1, const ITrackInfo* t2)
        -> bool { string s1 = t1->GetAccession(); if (!NStr::StartsWith(s1, "NA")) s1 = NcbiEmptyString;
                  string s2 = t2->GetAccession(); if (!NStr::StartsWith(s2, "NA")) s2 = NcbiEmptyString;
                  return NStr::CompareNocase(s1, s2) < 0; });
    else if (abs(m_SortColumn) == 3)
        sort(m_TracksVisible.begin(), m_TracksVisible.end(),
        [](const ITrackInfo* t1, const ITrackInfo* t2)
        -> bool { return NStr::CompareNocase(t1->GetCategory(), t2->GetCategory()) < 0; });
    else if (abs(m_SortColumn) == 4)
        sort(m_TracksVisible.begin(), m_TracksVisible.end(),
        [](const ITrackInfo* t1, const ITrackInfo* t2)
        -> bool { return NStr::CompareNocase(t1->GetComments(), t2->GetComments()) < 0; });
    else
        return;

    if (m_SortColumn < 0)
        reverse(m_TracksVisible.begin(), m_TracksVisible.end());
}

void CTrackListCtrl::OnSetVisibility(wxCommandEvent& evt)
{
    bool visible = evt.IsChecked();

    for (long item = -1;;) {
        item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1) break;
        ITrackInfo* track = m_TracksVisible[item];
        track->SetVisible(visible);
        RefreshItem(item);
    }
}

wxString CTrackListCtrl::OnGetItemText(long item, long column) const
{
    if (item < 0 || item >= (long)m_TracksVisible.size())
        return wxEmptyString;

    ITrackInfo* track = m_TracksVisible[item];

    if (column == 0) {
        string trackName = track->GetDisplayName();
        return wxString::FromUTF8(trackName.c_str());
    }

    if (column == 1) {
        string accession = track->GetAccession();
        if (NStr::StartsWith(accession, "NA"))
            return wxString::FromUTF8(accession.c_str());
        else
            return wxEmptyString;
    }

    if (column == 2) {
        string category = track->GetCategory();
        return wxString::FromUTF8(category.c_str());
    }

    if (column == 3) {
        string description = track->GetComments();
        return wxString::FromUTF8(description.c_str());
    }

    return wxEmptyString;
}

int CTrackListCtrl::OnGetItemImage(long item) const
{
    if (item < 0 || item >= (long)m_TracksVisible.size())
        return 0;

    ITrackInfo* track = m_TracksVisible[item];
    return track->GetVisible() ? 1 : 0;
}

void CTrackListCtrl::FilterValues(const string& filter)
{
    if (filter.empty()) {
        m_TracksVisible = m_Tracks;
    }
    else {
        m_TracksVisible.clear();
        for (auto i : m_Tracks) {
            string trackName = i->GetDisplayName();
            string accession = i->GetAccession();
            if (NStr::FindNoCase(trackName, filter) == NPOS &&
                NStr::FindNoCase(accession, filter) == NPOS)
                continue;
            m_TracksVisible.push_back(i);
        }
    }
    x_Sort();
    SetItemCount(m_TracksVisible.size());
    Refresh();
}

static const char* kWidths = "Widths";

void CTrackListCtrl::LoadSettings(const CRegistryReadView& view)
{
    vector<int> widths;
    view.GetIntVec(kWidths, widths);

    if ((int)widths.size() == GetColumnCount()) {
        for (int i = 0; i < GetColumnCount(); ++i) {
            int w = widths[i];
            if (w > 0) SetColumnWidth(i, w);
        }
    }
}

void CTrackListCtrl::SaveSettings(CRegistryWriteView view) const
{
    vector<int> widths;
    for (int i = 0; i < GetColumnCount(); ++i)
        widths.push_back(GetColumnWidth(i));

    if (widths.size() > 0)
        view.Set(kWidths, widths);
}

END_NCBI_SCOPE
