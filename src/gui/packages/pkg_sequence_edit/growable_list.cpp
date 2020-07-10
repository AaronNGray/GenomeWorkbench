/*  $Id: growable_list.cpp 40968 2018-05-04 16:42:02Z katargir $
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
#include <gui/widgets/wx/spin_ctrl.hpp>
#include <gui/widgets/wx/hyperlink.hpp>
#include <wx/statline.h>
#include "growable_list.hpp"

BEGIN_NCBI_SCOPE

/*!
 * CGrowableListCtrl type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGrowableListCtrl, wxPanel )


/*!
 * CGrowableListCtrl event table definition
 */

BEGIN_EVENT_TABLE( CGrowableListCtrl, wxPanel )

////@begin CGrowableListCtrl event table entries

////@end CGrowableListCtrl event table entries

    EVT_HYPERLINK(wxID_ANY, CGrowableListCtrl::OnDelete)
    EVT_COMMAND(wxID_ANY, wxEVT_SPIN_CTRL_EVENT, CGrowableListCtrl::OnSpinCtrl)

END_EVENT_TABLE()

const int kCtrlColNum = 2;

/*!
 * CGrowableListCtrl constructors
 */

CGrowableListCtrl::CGrowableListCtrl()
{
    Init();
}

CGrowableListCtrl::CGrowableListCtrl( wxWindow* parent, const wxArrayString &items,
                                      wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Items(items)
{
    if (m_Items.IsEmpty())
        m_Items.Add(wxEmptyString);
    for (size_t i = 0; i < m_Items.GetCount(); i++)
    {
        SIntData intData;
        intData.m_Item = m_Items[i];
        intData.m_Empty = false;
        if (intData.m_Item.IsEmpty())
            intData.m_Empty = true;
        m_Data.push_back(intData);
    }
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGrowableListCtrl creator
 */

bool CGrowableListCtrl::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGrowableListCtrl creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGrowableListCtrl creation
    return true;
}


/*!
 * CGrowableListCtrl destructor
 */

CGrowableListCtrl::~CGrowableListCtrl()
{
////@begin CGrowableListCtrl destruction
////@end CGrowableListCtrl destruction
}


/*!
 * Member initialisation
 */

void CGrowableListCtrl::Init()
{
////@begin CGrowableListCtrl member initialisation
    m_ScrolledWindow = NULL;
////@end CGrowableListCtrl member initialisation
    m_Sizer = NULL;
}


/*!
 * Control creation for CGrowableListCtrl
 */

void CGrowableListCtrl::CreateControls()
{    
////@begin CGrowableListCtrl content construction
    CGrowableListCtrl* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( itemPanel1, ID_CGROWABLELISTCTRL_PANEL1, wxDefaultPosition, wxSize(-1,100), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel3, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel3, ID_CGROWABLELISTCTRL_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer4->Add(m_ScrolledWindow, 1, wxGROW|wxTOP|wxBOTTOM, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);


    ////@end CGrowableListCtrl content construction

    m_Sizer = new wxFlexGridSizer(0, kCtrlColNum, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);
    m_ScrolledWindow->FitInside();

    //m_AddIntHyperlink->SetVisitedColour(m_AddIntHyperlink->GetNormalColour());
    m_ScrolledWindow->SetScrollRate(0, 5);

    vector<SIntData>::const_iterator it;
    for (it = m_Data.begin(); it != m_Data.end(); ++it) {
        if (it->m_Empty) {
            x_AddEmptyRow ();
        } else {
            x_AddRow(it->m_Item);
        }
    }
    m_ScrolledWindow->FitInside();
}


void CGrowableListCtrl::AddRow(const  wxString& item)
{

    SIntData intData;
    intData.m_Item  = item;
    intData.m_Empty  = false;
    m_Data.push_back(intData);
}

void CGrowableListCtrl::AddEmptyRow()
{
    SIntData intData;
    intData.m_Empty = true;
    m_Data.push_back(intData);
}

void CGrowableListCtrl::ClearRows()
{
    m_Data.clear();
    m_Sizer->Clear(true);
}

static void s_DeleteRow(wxSizerItemList::iterator row, wxSizerItemList& itemList)
{
    wxSizerItemList::iterator node = row;
    for (int i = 0; node != itemList.end() && i < kCtrlColNum; ++i) {
        (**node).DeleteWindows();
        node = itemList.erase(node);
    }
}

static wxSizerItemList::iterator s_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return itemList.end();

    wxSizerItemList::iterator row, it = itemList.begin();

    for(int index = 0; it != itemList.end(); ++it, --index) {
        if (index == 0) {
            row = it;
            index = kCtrlColNum;
        }

        wxSizer* sizer = (**it).GetSizer();
        if (sizer && sizer->GetItem(wnd))
            return row;
        else if ((**it).GetWindow() == wnd)
            return row;
    }

    return it;
}

static wxSizerItemList::iterator s_NextRow(wxSizerItemList::iterator row, wxSizerItemList& itemList)
{
    wxSizerItemList::iterator it = row;
    for (int i = 0; i < kCtrlColNum; ++i, ++it) {
        if (it == itemList.end())
            return itemList.end();
    }
    return it;
}

static wxSizerItemList::iterator s_PrevRow(wxSizerItemList::iterator row, wxSizerItemList& itemList)
{
    wxSizerItemList::iterator it = row;
    for (int i = 0; i < kCtrlColNum; ++i, --it) {
        if (it == itemList.begin())
            return itemList.end();
    }
    return it;
}

static void s_SwapRows(wxSizerItemList::iterator row1,
                       wxSizerItemList::iterator row2,
                       wxSizerItemList& itemList)
{
    for (int i = 0; i < 4; ++i) {
        if (row1 == itemList.end() || row2 == itemList.end())
            break;
        swap(*row1++, *row2++);
    }
}

static void s_GetRowData(wxSizerItemList::iterator row, wxSizerItemList& itemList,
                         wxString& item)
{
    wxSizerItemList::iterator node = row;

    if (node == itemList.end())return;

    wxComboBox* combo = (wxComboBox*)(**node).GetWindow();
    item = combo->GetValue();
}

void CGrowableListCtrl::OnDelete (wxHyperlinkEvent& event)
{
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator row = s_FindRow((wxWindow*)event.GetEventObject(), itemList);
    s_DeleteRow(row, itemList);
    if (m_Sizer->GetChildren().size() == 0) {
        x_AddEmptyRow();
    }
    m_ScrolledWindow->FitInside();
}

void CGrowableListCtrl::OnSpinCtrl ( wxCommandEvent& evt )
{
    wxWindow* spnCtrl = (wxWindow*)evt.GetEventObject();

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator row = s_FindRow(spnCtrl, itemList);
    if (row == itemList.end())
        return;

    wxSizerItemList::iterator row2;

    switch (evt.GetId()) {
        case CSpinControl::kBtnUp:
            row2 = s_PrevRow(row, itemList);
            s_SwapRows(row, row2, itemList);
            m_Sizer->Layout();
            m_ScrolledWindow->Refresh();
            break;
        case CSpinControl::kBtnDn:
            row2 = s_NextRow(row, itemList);
            s_SwapRows(row, row2, itemList);
            m_Sizer->Layout();
            m_ScrolledWindow->Refresh();
            break;
        case 2 + CSpinControl::kBtnUp:
            {{
                size_t rowPos = itemList.IndexOf(*row);               
                wxString item;
                s_GetRowData(row, itemList, item);
              
#if 1
                x_AddEmptyRow(rowPos);
#else
                x_AddRow(item, rowPos);
#endif
                m_Sizer->Layout();
                m_ScrolledWindow->FitInside();
                m_ScrolledWindow->Refresh();
            }}
            break;
        case 2 + CSpinControl::kBtnDn:
            {{               
                wxString item;
                s_GetRowData(row, itemList, item);

                row = s_NextRow(row, itemList);
                size_t rowPos = (row == itemList.end()) ? (size_t)-1 : itemList.IndexOf(*row);
#if 1
                x_AddEmptyRow(rowPos);
#else
                x_AddRow(item, rowPos);
#endif
                m_Sizer->Layout();
                m_ScrolledWindow->FitInside();
                m_ScrolledWindow->Refresh();
            }}
            break;
    }
}


void CGrowableListCtrl::x_AddRow(const wxString& item, size_t rowPos)
{
    wxComboBox *combo = new wxComboBox(m_ScrolledWindow, wxID_ANY, item,
                           wxDefaultPosition,
                           wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(65, -1)).x, -1),
                           m_Items);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(combo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else
        m_Sizer->Insert(rowPos++, combo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(controlsSizer);
    else
        m_Sizer->Insert(rowPos++, controlsSizer);

    wxHyperlinkCtrl* itemHyperLink = new CHyperlink(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT(""));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    controlsSizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    CSpinControl* spinCtrl = new CSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    spinCtrl->SetToolTip(_("Use arrow controls to reorder list"));
    controlsSizer->Add(spinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    spinCtrl = new CSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    spinCtrl->SetToolTip(_("Use plus/plus controls to insert blank elements"));
    spinCtrl->UseImageSet(1);
    controlsSizer->Add(spinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);
}

void CGrowableListCtrl::x_AddEmptyRow(size_t rowPos)
{
    
    wxComboBox *combo = new wxComboBox(m_ScrolledWindow, wxID_ANY, wxT(""),
                           wxDefaultPosition,
                           wxSize(m_ScrolledWindow->ConvertDialogToPixels(wxSize(65, -1)).x, -1),
                           m_Items);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(combo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    else
        m_Sizer->Insert(rowPos++, combo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
    if (rowPos == (size_t)-1)
        m_Sizer->Add(controlsSizer);
    else
        m_Sizer->Insert(rowPos++, controlsSizer);

    wxHyperlinkCtrl* itemHyperLink = new CHyperlink(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT(""));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    controlsSizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    CSpinControl* spinCtrl = new CSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    controlsSizer->Add(spinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    spinCtrl = new CSpinControl(m_ScrolledWindow, wxID_ANY, wxDefaultPosition);
    spinCtrl->UseImageSet(1);
    controlsSizer->Add(spinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);
  
}


bool CGrowableListCtrl::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();
    for (;  node != children.end();  ++node)
        (**node).DeleteWindows();
    children.clear();

    vector<SIntData>::const_iterator it;
    for (it = m_Data.begin(); it != m_Data.end(); ++it) {
        if (it->m_Empty) {
            x_AddEmptyRow ();
        } else {
            x_AddRow(it->m_Item);
        }
    }

    m_ScrolledWindow->FitInside();
    return true;
}

void CGrowableListCtrl::GetItems(vector<string> &items) 
{
    wxSizerItemList& children = m_Sizer->GetChildren();    
    for (wxSizerItemList::iterator node = children.begin();  node != children.end(); ++node) 
    {

        wxComboBox* item_combo = (wxComboBox*)(**node).GetWindow();
        if (item_combo && !item_combo->IsTextEmpty()) 
        {
            items.push_back(item_combo->GetValue().ToStdString());
        }
    }
}

bool CGrowableListCtrl::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_Data.clear();

    wxSizerItemList& children = m_Sizer->GetChildren();
   
    for ( wxSizerItemList::iterator node = children.begin();  node != children.end(); ++node) 
    {
        SIntData intData;

        wxComboBox* item_combo = (wxComboBox*)(**node).GetWindow(); 
        if (!item_combo)
            continue;

        if (item_combo->IsTextEmpty()) 
        {
            intData.m_Empty = true;
        }
        else
        {
            intData.m_Item = item_combo->GetValue();
            intData.m_Empty = false;
        }
        m_Data.push_back(intData);
    }
    return true;
}


/*!
 * Should we show tooltips?
 */

bool CGrowableListCtrl::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGrowableListCtrl::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGrowableListCtrl bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGrowableListCtrl bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGrowableListCtrl::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGrowableListCtrl icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGrowableListCtrl icon retrieval
}


void CGrowableListCtrl::SetItems( const wxArrayString &items)
{
    m_Items = items;
    if (m_Items.IsEmpty())
        m_Items.Add(wxEmptyString);
    m_Data.clear();
    for (size_t i = 0; i < m_Items.GetCount(); i++)
    {
        SIntData intData;
        intData.m_Item = m_Items[i];
        intData.m_Empty = false;
        if (intData.m_Item.IsEmpty())
            intData.m_Empty = true;
        m_Data.push_back(intData);
    }
    TransferDataToWindow();
    Refresh();
}

END_NCBI_SCOPE
