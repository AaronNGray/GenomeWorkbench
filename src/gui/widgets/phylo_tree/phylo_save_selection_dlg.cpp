/* Id: phylo_save_selection_dlg.cpp 36876 2016-11-09 13:02:26Z falkrb $
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
 * Authors:
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>////@begin includes
////@end includes

#include "phylo_save_selection_dlg.hpp"

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_selection_set.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/clrpicker.h>
#include <wx/bitmap.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/imaglist.h>

////@begin XPM images
////@end XPM images
static const char * checked_xpm[] = {
"16 16 4 1",
"   c None",
".  c #808080",
"X  c Black",
"o  c #c0c0c0",
"                ",
"                ",
"  ............  ",
"  .XXXXXXXXXXo  ",
"  .X         o  ",
"  .X       X o  ",
"  .X      XX o  ",
"  .X X   XXX o  ",
"  .X XX XXX  o  ",
"  .X XXXXX   o  ",
"  .X  XXX    o  ",
"  .X   X     o  ",
"  .X         o  ",
"  .ooooooooooo  ",
"                ",
"                "};
/* XPM */
static const char * unchecked_xpm[] = {
"16 16 4 1",
"   c None",
".  c #808080",
"X  c Black",
"o  c #c0c0c0",
"                ",
"                ",
"  ............  ",
"  .XXXXXXXXXXo  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .ooooooooooo  ",
"                ",
"                "};

static const char * up_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"        X       ",
"       XXX      ",
"      XXXXX     ",
"     XXXXXXX    ",
"    XXXXXXXXX   ",
"   XXXXXXXXXXX  ",
"  XXXXXXXXXXXXX ",
" XXXXXXXXXXXXXXX",
"                ",
"                ",
"                ",
"                "};


static const char * down_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
" XXXXXXXXXXXXXXX",
"  XXXXXXXXXXXXX ",
"   XXXXXXXXXXX  ",
"    XXXXXXXXX   ",
"     XXXXXXX    ",
"      XXXXX     ",
"       XXX      ",
"        X       ",
"                ",
"                ",
"                ",
"                "};

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CPhyloSaveSelectiondlg, wxDialog )

BEGIN_EVENT_TABLE( CPhyloSaveSelectiondlg, wxDialog )

////@begin CPhyloSaveSelectiondlg event table entries
    EVT_LIST_ITEM_SELECTED( ID_SEL_LISTCTRL, CPhyloSaveSelectiondlg::OnSelListctrlSelected )

    EVT_TEXT( ID_SELECTION, CPhyloSaveSelectiondlg::OnSelectionTextUpdated )

    EVT_COLOURPICKER_CHANGED( ID_SEL_COLOR_PICKER, CPhyloSaveSelectiondlg::OnSelColorPickerColourPickerChanged )

    EVT_BUTTON( ID_BITMAPBUTTON_UP, CPhyloSaveSelectiondlg::OnBitmapbuttonUpClick )

    EVT_BUTTON( ID_BITMAPBUTTON_DOWN, CPhyloSaveSelectiondlg::OnBitmapbuttonDownClick )

    EVT_BUTTON( ID_DELETE_SEL_BTN, CPhyloSaveSelectiondlg::OnDeleteSelBtnClick )

    EVT_BUTTON( ID_ADD_CURRENT_SEL_BTN, CPhyloSaveSelectiondlg::OnAddCurrentSelBtnClick )

    EVT_BUTTON( wxID_OK, CPhyloSaveSelectiondlg::OnOkClick )

////@end CPhyloSaveSelectiondlg event table entries

END_EVENT_TABLE()

CPhyloSaveSelectiondlg::CPhyloSaveSelectiondlg()
{
    Init();
}

CPhyloSaveSelectiondlg::CPhyloSaveSelectiondlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CPhyloSaveSelectiondlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPhyloSaveSelectiondlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPhyloSaveSelectiondlg creation

    wxImageList *il = new wxImageList(16, 16, true, 2);
    il->Add( wxBitmap( unchecked_xpm ) );
    il->Add( wxBitmap( checked_xpm ) );
    m_SelList->AssignImageList(il, wxIMAGE_LIST_SMALL);
  
    wxListItem itemCol;
    itemCol.SetText(wxT("Select"));   
    itemCol.SetWidth(60);
    m_SelList->InsertColumn(0, itemCol);

    itemCol.SetText(wxT("Name"));
    itemCol.SetWidth(370);
    m_SelList->InsertColumn(1, itemCol);	

    return true;
}

CPhyloSaveSelectiondlg::~CPhyloSaveSelectiondlg()
{
////@begin CPhyloSaveSelectiondlg destruction
////@end CPhyloSaveSelectiondlg destruction
}

void CPhyloSaveSelectiondlg::Init()
{
////@begin CPhyloSaveSelectiondlg member initialisation
    m_SelList = NULL;
    m_SelectionName = NULL;
    m_SelColorPicker = NULL;
    m_ButtonUp = NULL;
    m_ButtonDown = NULL;
    m_DeleteBtn = NULL;
    m_AddSelectionBtn = NULL;
////@end CPhyloSaveSelectiondlg member initialisation

    m_CurrentItem = -1;
}

void CPhyloSaveSelectiondlg::CreateControls()
{    
////@begin CPhyloSaveSelectiondlg content construction
    CPhyloSaveSelectiondlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_LEFT|wxALL, 5);

    m_SelList = new wxListCtrl( itemStaticBoxSizer3->GetStaticBox(), ID_SEL_LISTCTRL, wxDefaultPosition, wxSize(540, 200), wxLC_REPORT|wxLC_NO_SORT_HEADER|wxLC_SINGLE_SEL|wxLC_HRULES|wxLC_VRULES );
    m_SelList->SetHelpText(_("List of saved selections"));
    if (CPhyloSaveSelectiondlg::ShowToolTips())
        m_SelList->SetToolTip(_("List of saved selections"));
    itemBoxSizer5->Add(m_SelList, 1, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemStaticBoxSizer3->GetStaticBox(), wxID_ANY, _("Update Selection Name, Color or Order"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxHORIZONTAL);
    itemBoxSizer4->Add(itemStaticBoxSizer7, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemStaticBoxSizer7->GetStaticBox(), wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_SelectionName = new wxTextCtrl( itemStaticBoxSizer7->GetStaticBox(), ID_SELECTION, wxEmptyString, wxDefaultPosition, wxSize(180, -1), 0 );
    m_SelectionName->Enable(false);
    itemStaticBoxSizer7->Add(m_SelectionName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SelColorPicker = new wxColourPickerCtrl( itemStaticBoxSizer7->GetStaticBox(), ID_SEL_COLOR_PICKER, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    m_SelColorPicker->Enable(false);
    itemStaticBoxSizer7->Add(m_SelColorPicker, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ButtonUp = new wxButton( itemStaticBoxSizer7->GetStaticBox(), ID_BITMAPBUTTON_UP, wxEmptyString, wxDefaultPosition, wxSize(24, -1), 0 );
    m_ButtonUp->Enable(false);
    itemStaticBoxSizer7->Add(m_ButtonUp, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    m_ButtonDown = new wxButton( itemStaticBoxSizer7->GetStaticBox(), ID_BITMAPBUTTON_DOWN, wxEmptyString, wxDefaultPosition, wxSize(24, -1), 0 );
    m_ButtonDown->Enable(false);
    itemStaticBoxSizer7->Add(m_ButtonDown, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_DeleteBtn = new wxButton( itemStaticBoxSizer7->GetStaticBox(), ID_DELETE_SEL_BTN, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DeleteBtn->Enable(false);
    itemStaticBoxSizer7->Add(m_DeleteBtn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    m_AddSelectionBtn = new wxButton( itemStaticBoxSizer7->GetStaticBox(), ID_ADD_CURRENT_SEL_BTN, _("Add New"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer7->Add(m_AddSelectionBtn, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Connect events and objects
    m_SelList->Connect(ID_SEL_LISTCTRL, wxEVT_LEFT_DOWN, wxMouseEventHandler(CPhyloSaveSelectiondlg::OnLeftDown), NULL, this);
////@end CPhyloSaveSelectiondlg content construction

    m_ButtonUp->SetBitmap(wxBitmap(up_xpm));
    m_ButtonDown->SetBitmap(wxBitmap(down_xpm));
}

bool CPhyloSaveSelectiondlg::ShowToolTips()
{
    return true;
}
wxBitmap CPhyloSaveSelectiondlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPhyloSaveSelectiondlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPhyloSaveSelectiondlg bitmap retrieval
}
wxIcon CPhyloSaveSelectiondlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPhyloSaveSelectiondlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPhyloSaveSelectiondlg icon retrieval
}

void CPhyloSaveSelectiondlg::x_EnableCurrentSelection(bool b)
{
    m_SelectionName->Enable(b);
    m_SelColorPicker->Enable(b);
    m_ButtonUp->Enable(b);
    m_ButtonDown->Enable(b);
    m_DeleteBtn->Enable(b);
    //m_AddSelectionBtn->Enable(b);
}

void CPhyloSaveSelectiondlg::SetSelections(CRef<CPhyloTreeDataSource> ds, string name) 
{
    // Get currently selected nodes (the current selection set)
    vector<CPhyloTree::TTreeIdx> currently_selected;
    ds->GetTree()->GetSelected(currently_selected);
    std::sort(currently_selected.begin(), currently_selected.end());

    // Get the selection set that results when we apply the results
    // from the last query. Note that when we call SetSelection()
    // parent or child nodes not in the query results may become part of
    // the selection set. 
    vector<CPhyloTree::TTreeIdx> last_query_results;
    ds->GetTree()->GetExplicitlySelected(last_query_results);

    ds->GetTree()->SetSelection(last_query_results);
    vector<CPhyloTree::TTreeIdx> last_query_selection;
    ds->GetTree()->GetSelected(last_query_selection);
    std::sort(last_query_selection.begin(), last_query_selection.end());

    // reset the current selection
    ds->GetTree()->ClearSelection();
    ds->GetTree()->SetSelection(currently_selected);

    // 'name' is the last query string.  If the current selection set does
    // not match the last query, set name to blank to avoid confusion
    if (last_query_selection != currently_selected)
        name = "";

    m_DS = ds;

    // Need to collect all colors in clusters and in selection sets that
    // are not currently active.  We will use these colors when we try to
    // find a new color that is sufficiently different from the current colors.
    std::vector<CRgbaColor> current_colors;

    m_AddSelectionBtn->Enable(false);

    x_EnableCurrentSelection(false);
    if (currently_selected.size() != 0) {
        // Don't put in a duplicate name (names are the most recent selection
        // query, but they do not necessarily match the current selection set -
        // we give the user a chance here to set the name).
        x_EnableCurrentSelection(true);
        m_AddSelectionBtn->Enable(true);
        m_DeleteBtn->Enable(false);  // can't delete a set that has not been added.

        m_SelectionName->SetValue(name);
        vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();

        // If it's a duplicate name AND the nodes are the same as the entry already in the
        // table disable the controls and do not give the opportunity to add a new set.
        // Otherwise generate a name
        vector<CPhyloTree::TID> currently_selected;
        ds->GetTree()->GetSelectedIDs(currently_selected);
        std::sort(currently_selected.begin(), currently_selected.end());
        for (size_t i = 0; i < sel_sets.size(); ++i) {
            if (!sel_sets[i].GetSelected())
                current_colors.push_back(sel_sets[i].GetColor());

            if (name == sel_sets[i].GetName()) {
                vector<CPhyloTree::TID>  already_selected = sel_sets[i].GetSelectionSet();
                std::sort(already_selected.begin(), already_selected.end());

                if (already_selected == currently_selected) {
                    m_SelectionName->SetValue("");
                    x_EnableCurrentSelection(false);
                    m_AddSelectionBtn->Enable(false);
                }
                else {
                    name = "";
                }
                break;
            }
        }
    }

    /// If the user can add this selection, pick a reasonable color for it
    if (m_AddSelectionBtn->IsEnabled()) {
        if (name == "") {
            m_SelectionName->SetValue("<<Enter Selection Name>>");
            m_SelectionName->SelectAll();
        }

        const CPhyloTreeDataSource::TClusterToColorMap&  ctcmap = ds->GetClusterToColorMap();

        ITERATE(CPhyloTreeDataSource::TClusterToColorMap, citer, ctcmap) {
            CRgbaColor c = ds->GetModel().GetColorTable()->GetColor(citer->second);
            current_colors.push_back(c);
        }

        CRgbaColor  color(102, 153, 204);
        CRgbaColor  white(255, 255, 255, 255);

        float num_colors = (float)ctcmap.size();
        float color_step = num_colors ? (180.0f / num_colors) : 0;
        float rotate_angle = color_step + 100.0f;

        float max_color_dist = 0.0f;
        CRgbaColor best_color = color;

        // Find a color that is as far away visually as possible from the other
        // selection set colors and colors of normal clusters in the tree.
        for (size_t i = 0; i < 2 * current_colors.size(); ++i) {
            // protect from bright colors (almost white) since they are harder to see
            float c_dist = CRgbaColor::ColorDistance(white, color);
            while (c_dist < 0.7f) {
                color.Darken(0.1f);
                c_dist = CRgbaColor::ColorDistance(white, color);
            }

            float min_color_dist = 1e10f;

            // Find the closest color from among all current selection/cluster colors in use
            for (size_t j = 0; j < current_colors.size(); ++j) {
                float dist = CRgbaColor::ColorDistance(current_colors[j], color);
                if (dist < min_color_dist) {
                    min_color_dist = dist;
                }
            }

            // If the closet color is further away than the closest color
            // for all previous colors wever tried, save the current color.
            if (min_color_dist > max_color_dist) {
                max_color_dist = min_color_dist;
                best_color = color;
            }

            color = CRgbaColor::RotateColor(color, rotate_angle);
        }

        m_SelColorPicker->SetColour(wxColour(best_color.GetRedUC(),
            best_color.GetGreenUC(), best_color.GetBlueUC()));
    }

    UpdateSelections();
}

void CPhyloSaveSelectiondlg::UpdateSelections() 
{
    m_SelList->DeleteAllItems();
    vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();
    
    for (size_t i=0; i<sel_sets.size(); ++i) {
        wxListItem item;
        item.SetId(i);
        item.SetText( sel_sets[i].GetName() ); 
        CRgbaColor c = sel_sets[i].GetColor();
        item.SetTextColour(wxColour(c.GetRedUC(), c.GetGreenUC(), c.GetBlueUC(), c.GetAlphaUC()));
        m_SelList->InsertItem( item );

        // set value in first column
        wxListItem info;
        info.SetImage(-1);
        info.SetId(i);
        info.SetColumn(1);
        info.SetText(sel_sets[i].GetName());
        m_SelList->SetItem(info);

        info.SetImage(sel_sets[i].GetSelected() ? 1 : 0);
        //info.SetId(i);
        info.SetColumn(0);
        info.SetText(""); 
        m_SelList->SetItem(info);
    }
}

void CPhyloSaveSelectiondlg::OnOkClick( wxCommandEvent& event )
{
    x_SetClusterIds(); 
    event.Skip();
}


void CPhyloSaveSelectiondlg::OnSelListctrlSelected( wxListEvent& event )
{
    // single selection list so this should always be true.
    if (m_SelList->GetSelectedItemCount() == 1) {
        wxListItem info;
        x_EnableCurrentSelection(true);
        // Selecting existing set takes away any chance to add current set
        m_AddSelectionBtn->Enable(false);  
        if (m_DS->GetSelectionSets().GetSets().size() < 2) {
            m_ButtonDown->Enable(false);
            m_ButtonUp->Enable(false);
        }

        m_CurrentItem = event.GetIndex();
        info.SetId(event.GetIndex());
        info.m_mask = wxLIST_MASK_TEXT;
        info.m_col = 1;
        m_SelList->GetItem(info);
        m_SelectionName->ChangeValue(info.GetText());

        vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();
        CRgbaColor c = sel_sets[m_CurrentItem].GetColor();
        m_SelColorPicker->SetColour(wxColour(c.GetRedUC(), c.GetGreenUC(), c.GetBlueUC(), c.GetAlphaUC()));
    }
}

void CPhyloSaveSelectiondlg::OnLeftDown( wxMouseEvent& event )
{
    if (event.LeftDown())
    {
        int flags;
        long item = m_SelList->HitTest(event.GetPosition(), flags);
        if (item > -1 && (flags & wxLIST_HITTEST_ONITEMICON))
        {
            bool is_checked;
            wxListItem info;
            info.m_mask = wxLIST_MASK_IMAGE ;
            info.m_itemId = item;

            if ( m_SelList->GetItem(info))           
                is_checked = (info.m_image == 1);            
            else
                is_checked = false;

            m_SelList->SetItemImage(item, (is_checked ? 0 : 1));
            
            // Update datasource
            vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();
            sel_sets[item].SetSelected((is_checked ? 0 : 1));

            return;
        }
    } 

    event.Skip();
}


void CPhyloSaveSelectiondlg::OnBitmapbuttonUpClick( wxCommandEvent& /*event*/ )
{
    vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();

    if (m_CurrentItem >= 1 && m_CurrentItem < long(sel_sets.size())) {
        std::iter_swap(sel_sets.begin() + m_CurrentItem, sel_sets.begin() + (m_CurrentItem - 1));       

        // Re-create displayed list
        UpdateSelections();

        // Keep selection on same item
        m_CurrentItem -= 1;
        m_SelList->SetItemState(m_CurrentItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void CPhyloSaveSelectiondlg::OnBitmapbuttonDownClick( wxCommandEvent& /*event*/ )
{
    vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();

    if (m_CurrentItem >= 0 && m_CurrentItem < long(sel_sets.size()-1) ) {
        std::iter_swap(sel_sets.begin() + m_CurrentItem, sel_sets.begin() + (m_CurrentItem +1));       

        // Re-create displayed list
        UpdateSelections();

        // Keep selection on same item
        m_CurrentItem += 1;
        m_SelList->SetItemState(m_CurrentItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void CPhyloSaveSelectiondlg::OnSelColorPickerColourPickerChanged( wxColourPickerEvent& event )
{
    vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();

    if (m_CurrentItem >= 0 && m_CurrentItem < long(sel_sets.size())) {
        wxColour c = event.GetColour();
      
        sel_sets[m_CurrentItem].SetColor(CRgbaColor(c.Red(), c.Green(), c.Blue(), c.Alpha()));

        // Update the text color of the item in the list.
        m_SelList->SetItemTextColour(m_CurrentItem, c);
        m_SelList->Refresh();
    }
}

void CPhyloSaveSelectiondlg::OnSelectionTextUpdated( wxCommandEvent& /*event*/ )
{
    vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();

    if (m_CurrentItem >= 0 && m_CurrentItem < long(sel_sets.size())) {
        wxListItem info;
        info.SetImage(-1);
        info.SetId(m_CurrentItem);
        info.m_mask = wxLIST_MASK_TEXT;
        info.SetColumn(1);
        info.SetText(m_SelectionName->GetValue());
        m_SelList->SetItem(info);

        sel_sets[m_CurrentItem].SetName(ToStdString(m_SelectionName->GetValue()));
    }
}


void CPhyloSaveSelectiondlg::OnDeleteSelBtnClick( wxCommandEvent& /*event*/ )
{
    vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();

    if (m_CurrentItem >= 0 && m_CurrentItem < long(sel_sets.size())) {
        sel_sets.erase(sel_sets.begin() + m_CurrentItem);
        m_CurrentItem -= 1;

        x_EnableCurrentSelection(false);

        // Re-create displayed list
        UpdateSelections();
    }
}


void CPhyloSaveSelectiondlg::OnAddCurrentSelBtnClick( wxCommandEvent& /*event*/ )
{
    vector<CPhyloTree::TID> currently_selected;
    m_DS->GetTree()->GetSelectedIDs(currently_selected);
    std::sort(currently_selected.begin(), currently_selected.end());
    int max_cluster_id = 500;

    vector<CPhyloSelectionSet>& sel_sets = m_DS->GetSelectionSets().GetSets();
    if (currently_selected.size() > 0) {

        m_SelectionName->TransferDataFromWindow();

        // If it's a duplicate - don't add and give the user a message.
        for (size_t i=0; i<sel_sets.size(); ++i) {
            vector<CPhyloTree::TID> selected = sel_sets[i].GetSelectionSet();
            std::sort(selected.begin(), selected.end());
            if (selected == currently_selected) {
                wxMessageBox("Current selection set is identical to existing set:\n" + sel_sets[i].GetName(), "Unable to Add Selection", wxOK | wxICON_WARNING);

                return;
            }
            if (sel_sets[i].GetClusterID() >= max_cluster_id)
                max_cluster_id = sel_sets[i].GetClusterID()+1;
        }

        // Insert new selection set at beginning of list.
        // the cluster id here will be overridden when we save since we have to keep cluster ids in order.
        sel_sets.push_back(CPhyloSelectionSet(currently_selected, ToStdString(m_SelectionName->GetValue()), max_cluster_id));
        
        // Set color 
        wxColour c = m_SelColorPicker->GetColour();      
        sel_sets[sel_sets.size()-1].SetColor(CRgbaColor(c.Red(), c.Green(), c.Blue(), c.Alpha()));
        // default to on (new set should be selected)
        sel_sets[sel_sets.size()-1].SetSelected(true);

        UpdateSelections();
        m_SelectionName->SetValue("");
        m_SelList->Refresh();
    }

    x_EnableCurrentSelection(false);
    m_AddSelectionBtn->Enable(false);
}

// Cluster IDs for these selection have to do two things:
// 1. avoid overlap with existing IDs (picking a higher number)
// 2. be in numerical order with the elements at the beginning of 
//    the list having higher numbers and sets lower in the list 
//    having lower numbers. 'higher priority' sets color to overlay 
//    colors of lower priority sets when we propogate set colors 
//    back toward the root of the tree.
void CPhyloSaveSelectiondlg::x_SetClusterIds()
{

    CPhyloTree* t = m_DS->GetTree();
    int max_cluster_id = -1;
    size_t i;

    for (i=0; i<t->GetSize(); ++i) {
        max_cluster_id = std::max(max_cluster_id, t->GetNodeValue(i).GetClusterID());
    }

    // pick cluster id much larger than existing ids:
    max_cluster_id += 500 + t->GetSelectionSets().GetSets().size();

    // set ids, first highest then lower.
    t->GetSelectionSets().RenumberClusterIDs(max_cluster_id);
}


END_NCBI_SCOPE

