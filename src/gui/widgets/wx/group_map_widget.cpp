/*  $Id: group_map_widget.cpp 44279 2019-11-21 20:58:36Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/group_map_widget.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/dcbuffer.h>
#include <wx/sizer.h>
#include <wx/settings.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CGroupMapWidget

CGroupMapWidget::CGroupMapWidget(wxWindow* parent,
                                      wxWindowID id,
                                      const wxPoint& pos,
                                      const wxSize& size,
                                      long style,
                                      const wxString& name)
:   TParent(parent, id, pos, size, style, name)
{
    // register only once
    static bool icons_registered = false;
    if( ! icons_registered)    {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("map::arrow_open"),
                                    wxT("arrow_open.png"));
        provider->RegisterFileAlias(wxT("map::arrow_close"),
                                    wxT("arrow_close.png"));
        icons_registered = true;
    }

    m_OpenIcon = wxArtProvider::GetBitmap(wxT("map::arrow_open"));
    m_ClosedIcon = wxArtProvider::GetBitmap(wxT("map::arrow_close"));

    // Create and setup Map Control
    m_Control = new CMapControl(this);
    m_Control->SetStyle(CMapControl::fSingleSelection);

    // Adjust Map Control settings
    CMapControl::SProperties& props = m_Control->GetProperties();
    props.m_SingleColumn = false;
    props.m_ItemOffsetX = 1;
    props.m_ItemOffsetY = 0;
    props.m_SepLineWidth = 1;
    props.m_ColumnWidth = 200;
    props.m_SeparateGroups = false;
    props.m_SizePolicy = CMapControl::eAdjustVertSize;

    // adjust Map Control Item settings
    SwxMapItemProperties& it_props = m_Control->GetMapItemProperties();
    it_props.m_BackColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    it_props.m_TextColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    it_props.m_VertMargin = 2;
    it_props.m_HorzMargin = 3;

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_Control, 1, wxEXPAND);
    SetSizer(sizer);
}


CGroupMapWidget::~CGroupMapWidget()
{
    Clear();
    //m_Control->RemoveListener(this);
}


void CGroupMapWidget::Init(const TGroupDescrVector& groups)
{
    Clear();

    for(size_t i = 0;  i < groups.size();  i++ )    {
        const SGroupDescr& grp = groups[i];
        AddGroup(grp.m_Label, grp.m_Expanded);

        const TItemDescrVector& items = grp.m_Items;
        for(  size_t j = 0;  j < items.size();  j++ )   {
            const SItemDescr& item = items[j];
            AddItem(grp.m_Label, item.m_Label, item.m_IconAlias, item.m_Description, item.m_ShowDefalt);
        }
    }
    //m_Scroll->UpdateScrollBars();

    int n =  m_Control->GetItemsCount();
    if(n > 0)   {
        m_Control->SelectItem(0);
    }
}


void CGroupMapWidget::Clear()
{
    m_Control->DeleteAllItems();

    for( size_t i = 0;  i < m_Groups.size();  i++ ) {
        delete m_Groups[i];
    }
    m_Groups.clear();
}


void CGroupMapWidget::AddGroup(const string& label, bool expand)
{
    int index = x_GetGroupIndex(label);
    if(index == -1) {
        // add new group
        SGroup* grp = new SGroup();
        grp->m_GroupItem =
            new CGroupItem(label, expand ? m_OpenIcon : m_ClosedIcon, "", CGroupItem::eDefaultHide, *this, true);
        grp->m_GroupItem->Expand(expand);
        m_Groups.push_back(grp);

        m_Control->AddItem(grp->m_GroupItem.GetPointer());
    } else {
        string s = "Group \"" + label + "\" already exist";
        NCBI_THROW(CException, eUnknown, s);
    }
}


typedef CMapControl::TIndex TIndex;

void CGroupMapWidget::AddItem(const string& group_label, const string& label,
                              const string& icon_alias, const string& descr, bool showDefault)
{
    int index = x_GetGroupIndex(group_label);
    if(index != -1) {
        // add new group
        SGroup* grp = m_Groups[index];

        wxBitmap icon = wxArtProvider::GetBitmap(ToWxString(icon_alias));

        CGroupItem::EDefaultState defState = CGroupItem::eDefaultHide;
        if (showDefault) {
            defState = (label == m_DefaultItem) ? CGroupItem::eDefaultYes : CGroupItem::eDefaultNo;
        }

        TItemRef    ref(new CGroupItem(label, icon, descr, defState, *this, false));
        grp->m_Items.push_back(ref);

        // update Control if needed
        if(grp->m_GroupItem->IsExpanded())  {
             TIndex i_grp = m_Control->GetItemIndex(*grp->m_GroupItem);
             TIndex ins = (TIndex)(i_grp + grp->m_Items.size());
             m_Control->InsertItem(ins, ref.GetPointer());
        }
    } else {
        string s = "Group \"" + group_label + "\" does not exist.";
        NCBI_THROW(CException, eUnknown, s);
    }
}


void CGroupMapWidget::GetGroupLabels(vector<string>& groups) const
{
    for( size_t i = 0;  i < m_Groups.size();  i++ ) {
        const SGroup* grp = m_Groups[i];
        groups.push_back(grp->m_GroupItem->GetLabel());
    }
}


bool CGroupMapWidget::IsExpanded(const string& group) const
{
    int index = x_GetGroupIndex(group);
    if(index >= 0)  {
        const SGroup* grp = m_Groups[index];
        return grp->m_GroupItem->IsExpanded();
    } else {
        string s = "Group \"" + group + "\" does not exist.";
        NCBI_THROW(CException, eUnknown, s);
    }
    return false;
}


string CGroupMapWidget::GetHotItem() const
{
    string label = GetItemLabel(m_Control->GetHotIndex());
    return label.empty() ? GetSelectedItem() : label;
}


string CGroupMapWidget::GetSelectedItem() const
{
    int index = m_Control->GetSelectedIndex();
    return GetItemLabel(index);
}

string CGroupMapWidget::GetItemLabel(int index) const
{
    if(index >=0 )  {
        CMapControl::TCItemRef item = m_Control->GetItem(index);
        const CGroupItem* grp_item = dynamic_cast<const CGroupItem*>(item.GetPointer());
        _ASSERT(grp_item);
        if(grp_item  &&  ! grp_item->IsGroupSeparator())    {
            return grp_item->GetLabel();
        }
    }
    return "";
}

CConstRef<CGroupMapWidget::CGroupItem> CGroupMapWidget::GetCurrentItem() const
{
    CConstRef<CGroupItem> curItem;

    int index = m_Control->GetHotIndex();
    if (index < 0)
        index = m_Control->GetSelectedIndex();
    if (index < 0)
        return curItem;

    CMapControl::TCItemRef item = m_Control->GetItem(index);
    const CGroupItem* grp_item = dynamic_cast<const CGroupItem*>(item.GetPointer());

    _ASSERT(grp_item);
    if (grp_item && !grp_item->IsGroupSeparator()) {
        curItem.Reset(grp_item);
    }

    return curItem;
}

void CGroupMapWidget::ExpandGroup(const string& /*label*/)
{
    _ASSERT(false); // TODO implement
}


CMapControl* CGroupMapWidget::GetMapControl()
{
    return m_Control;
}


int CGroupMapWidget::x_GetGroupIndex(const string& label) const
{
    for(size_t i = 0; i < m_Groups.size(); i++ )    {
        CGroupItem* item = m_Groups[i]->m_GroupItem;
        if(item->GetLabel() == label)   {
            return (int)i;
        }
    }
    return -1;
}


void CGroupMapWidget::x_OnToggleGroup(CGroupItem& item)
{
    // find the group descriptor
    int grp_i = x_GetGroupIndex(item.GetLabel());
    _ASSERT(grp_i >= 0);
    TMapItems& v_items = m_Groups[grp_i]->m_Items;

    bool expand = ! item.IsExpanded();//  &&  ! v_item.size();
    item.Expand(expand);
    item.SetIcon(expand ? m_OpenIcon : m_ClosedIcon);

    // get group iem index in the Control
    TIndex index = m_Control->GetItemIndex(item);

    m_Control->LockUpdates(true);

    for( size_t i = 0;  i < v_items.size();  i++  ) {
        if(expand)  {
            TItemRef mp_it = v_items[i];
            m_Control->InsertItem(++index, mp_it.GetPointer());
        } else {
            m_Control->DeleteItem(index + 1);
        }
    }

    m_Control->LockUpdates(false);
    //m_Scroll->UpdateScrollBars();
}


void CGroupMapWidget::x_OnSelectItem(CGroupItem& item)
{
    wxCommandEvent event(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, GetId());
    event.SetEventObject(this);
    event.SetString(ToWxString(item.GetLabel()));

    (void)GetEventHandler()->ProcessEvent(event);
}

void CGroupMapWidget::x_OnMakeDefaultItem(CGroupItem& item)
{
    wxClientDC dc(this);

    for(size_t i = 0; i < m_Groups.size(); i++ )    {
        SGroup& group = *m_Groups[i];
        for (size_t j = 0; j < group.m_Items.size(); ++j) {
            CGroupItem* pitem = dynamic_cast<CGroupItem*>(group.m_Items[j].GetPointer());
            _ASSERT(pitem);

            if (pitem == &item) {
                if (pitem->GetDefaultState() == CGroupItem::eDefaultYes) {
                    pitem->SetDefaultState(CGroupItem::eDefaultNo);
                    m_DefaultItem = "";
                }
                else {
                    pitem->SetDefaultState(CGroupItem::eDefaultYes);
                    m_DefaultItem = pitem->GetLabel();
                }
                pitem->Layout(dc, m_Control->GetMapItemProperties());
            }
            else {
                if (pitem->GetDefaultState() == CGroupItem::eDefaultYes) {
                    pitem->SetDefaultState(CGroupItem::eDefaultNo);
                    pitem->Layout(dc, m_Control->GetMapItemProperties());
                }
            }
        }
    }

    m_Control->UpdateSelection();

    Refresh();
}

///////////////////////////////////////////////////////////////////////////////
/// CGroupItem

CGroupMapWidget::CGroupItem::CGroupItem(const string& label,
                                              wxBitmap& icon,
                                              const string& descr,
                                              EDefaultState def,
                                              CGroupMapWidget& widget,
                                              bool group)
:   CMapItem(label, icon, descr, group),
    m_Widget(widget),
    m_Expanded(true),
    m_HotLabel(false),
    m_LabelRealW(0),
    m_HotDef(false),
    m_DefWidth(0),
    m_DefaultState(def),
    m_ClickOnSpace(false)
{
}


bool CGroupMapWidget::CGroupItem::IsExpanded() const
{
    return m_Expanded;
}


void CGroupMapWidget::CGroupItem::Expand(bool exp)
{
    m_Expanded = exp;
}

string CGroupMapWidget::CGroupItem::GetFullDescr() const
{
    string descr;

    if (m_HotDef) {
        if (m_DefaultState == eDefaultNo) {
            descr = "Click to make '" + GetLabel() + "' default view.\n";
            descr += "This view will be opened on double click on The Project View '" +
                     m_Widget.GetInputCategory() + "' object.";
        }
        else if (m_DefaultState == eDefaultYes) {
            descr = "Click to clear default view.\n";
            descr += "Double click on The Project View '" +  m_Widget.GetInputCategory() + 
                    "' object will open this dialog.";
        }
    }
    else {
        descr = GetLabel();
        if (!descr.empty())
            descr += "\n";
        descr += GetDescr();
    }

    return descr;
}

int CGroupMapWidget::CGroupItem::PreferredHeight(wxDC& dc,
                                                   SwxMapItemProperties& props,
                                                   int width)
{
    int pref_h = 0;
    if(m_Icon.IsOk()) {
        pref_h = m_Icon.GetHeight();
    }

    int label_h = x_PreferredLabelHeight(dc, props, width);

    pref_h = max(pref_h, label_h);
    if(IsGroupSeparator())  {
        pref_h += 6;
    } else {
        pref_h += props.m_VertMargin * 2;
    }
    if(m_Separator) {
        pref_h += x_PreferredSeparatorHeight(dc);
    }
    return pref_h;
}


int CGroupMapWidget::CGroupItem::x_GetItemShift() const
{
    return IsGroupSeparator() ? 0 : 12;
}

static const char* kSetDefault = "Make Default";
static const char* kUnsetDefault = "Default";

void CGroupMapWidget::CGroupItem::Layout(wxDC& dc, SwxMapItemProperties& props)
{
    CMapItem::Layout(dc, props);

    wxFont font = props.m_Font;
    font.SetWeight(wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);

    m_LabelRealW = x_CalculateRealTextWidth(dc, props);
    m_DefWidth = 0;
    if (m_DefaultState != eDefaultHide) {
        const char* defText =
            (m_DefaultState == eDefaultNo) ? kSetDefault : kUnsetDefault;

        wxSize size = dc.GetTextExtent(ToWxString(defText));
        m_DefWidth = std::min(m_LabelRect.GetWidth() - m_LabelRealW, size.x + 2 * props.m_HorzMargin);
    }
}


void CGroupMapWidget::CGroupItem::Draw(wxDC& dc/*, const wxRect& rc*/, int state,
                                           SwxMapItemProperties& props)
{
    wxRect rc_back(m_Rect);
    rc_back.Inflate(-props.m_Border, -props.m_Border);

    // draw focus rectangle
    //x_DrawFocusRect(dc, rc_back, state, props); //rc

    // Fill background
    x_DrawBackground(dc, rc_back, state, props);

    bool selected = (state & CSelectionControl::fItemSelected) != 0;

    // Draw Icon
    if(m_Icon.Ok()) {
        dc.DrawBitmap(m_Icon, m_IconPos.x, m_IconPos.y, true);
    }

    if( ! m_Label.empty()  &&  m_LabelRect.width > 0  &&  m_LabelRect.height > 0)  {
        bool widget_focused = (state & CSelectionControl::fWidgetFocused) != 0;

        if(IsGroupSeparator())  {
            dc.SetFont(props.m_Font);
        } else {
            // TODO this can be optimized
            wxFont font = props.m_Font;
            font.SetWeight(wxFONTWEIGHT_NORMAL);
            dc.SetFont(font);
        }

        x_DrawText(dc, m_Label, m_LabelRect, selected, widget_focused, m_HotLabel, props);

        if (m_DefaultState != eDefaultHide && m_DefWidth > 0) {
            const char* defText =
                (m_DefaultState == eDefaultNo) ? kSetDefault : kUnsetDefault;
            wxRect defRect(m_LabelRect);
            defRect.SetX(defRect.GetRight() - m_DefWidth);

            wxFont font = props.m_Font;
            font.SetWeight(wxFONTWEIGHT_NORMAL);
            dc.SetFont(font);
            x_DrawText(dc, defText, defRect, selected, widget_focused, m_HotDef, props);
        }
    }
}


int CGroupMapWidget::CGroupItem::x_PreferredSeparatorHeight(wxDC& dc) const
{
    return 0;
}

void CGroupMapWidget::CGroupItem::x_DrawBackground(wxDC& dc, const wxRect& rc,
                                                       int state, SwxMapItemProperties& props)
{
    bool selected = (state & CSelectionControl::fItemSelected) != 0;
    wxColour cl_back = selected ? props.m_SelBackColor :
        (IsGroupSeparator() ? GetAverage(props.m_TextColor, props.m_BackColor, 0.10f)
                            : props.m_BackColor);
    
    /*
    if(!IsGroupSeparator() && m_HotLabel)  {
        const float     kBlueAdjustment = 0.1f;
        unsigned char   adjustedRed = (unsigned char)(cl_back.Red() * (1 - kBlueAdjustment));
        unsigned char   adjustedGreen = (unsigned char)(cl_back.Green() * (1 - kBlueAdjustment));
        cl_back.Set(adjustedRed, adjustedGreen, cl_back.Blue(), cl_back.Alpha());
    }
    */

    wxBrush brush(cl_back);
    dc.SetBrush(brush);
    dc.DrawRectangle(rc.x, rc.y, rc.width, rc.height);
}


void CGroupMapWidget::CGroupItem::OnDefaultAction()
{
    if(IsGroupSeparator())  {
        m_Widget.x_OnToggleGroup(*this);
    } else {
        m_Widget.x_OnSelectItem(*this);
    }
}


bool CGroupMapWidget::CGroupItem::OnHotTrack(const wxPoint& ms_pos)
{
    if (IsGroupSeparator()) 
    {
        // make all separator hot clickable item (including icon)
        return true;
    }
    wxRect rc(m_LabelRect);
    rc.width = m_LabelRealW;
    m_HotLabel = rc.Contains(ms_pos);

    m_HotDef = false;

    if (!m_HotLabel && m_DefaultState != eDefaultHide && m_DefWidth > 0) {
        wxRect defRect(m_LabelRect);
        defRect.SetX(defRect.GetRight() - m_DefWidth);
        m_HotDef = defRect.Contains(ms_pos);
    }

    return m_HotLabel || m_HotDef;
}

void CGroupMapWidget::CGroupItem::OnMouseDown(const wxPoint& ms_pos)
{
    m_ClickOnSpace = !m_HotLabel && !m_HotDef;

    if (IsGroupSeparator() || m_HotLabel) {
        OnDefaultAction();
    }
    else if (m_HotDef) {
        m_Widget.x_OnMakeDefaultItem(*this);
    }
}

void CGroupMapWidget::CGroupItem::OnLeftDoubleClick(const wxPoint& ms_pos)
{
    if (!m_HotDef && !IsGroupSeparator() && m_ClickOnSpace)
        OnDefaultAction();
}

END_NCBI_SCOPE
