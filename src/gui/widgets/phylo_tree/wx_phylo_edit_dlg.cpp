/*  $Id: wx_phylo_edit_dlg.cpp 29104 2013-10-22 18:01:10Z falkrb $
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
 * Authors: Vladimir Tereshkov
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>////@begin includes
////@end includes


#include <gui/widgets/phylo_tree/wx_phylo_edit_dlg.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_label.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>

#include <gui/widgets/wx/dialog_utils.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/rgba_color.hpp>

#include <algo/phy_tree/bio_tree.hpp>

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/clrpicker.h>
#include <wx/checklst.h>
#include <wx/valgen.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/listbook.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CwxPhyloEditDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxPhyloEditDlg, wxDialog )

////@begin CwxPhyloEditDlg event table entries
    EVT_LIST_ITEM_SELECTED( ID_LISTCTRL1, CwxPhyloEditDlg::OnListctrl1Selected )
    EVT_LIST_ITEM_DESELECTED( ID_LISTCTRL1, CwxPhyloEditDlg::OnListctrl1Deselected )

    EVT_BUTTON( ID_BUTTON4, CwxPhyloEditDlg::OnButton4Click )

    EVT_BUTTON( ID_BUTTON5, CwxPhyloEditDlg::OnButton5Click )

    EVT_BUTTON( ID_BUTTON, CwxPhyloEditDlg::OnButtonClick )

    EVT_COLOURPICKER_CHANGED( ID_COLOURCTRL, CwxPhyloEditDlg::OnColourctrlColourPickerChanged )

    EVT_BUTTON( wxID_OK, CwxPhyloEditDlg::OnOkClick )

////@end CwxPhyloEditDlg event table entries

END_EVENT_TABLE()

CwxPhyloEditDlg::CwxPhyloEditDlg()
: m_Tree(NULL)
, m_NodeIdx(CPhyloTree::Null())
, m_UpdatedFeature(NULL)
{
    Init();
}

CwxPhyloEditDlg::CwxPhyloEditDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_Tree(NULL)
, m_NodeIdx(CPhyloTree::Null())
, m_UpdatedFeature(NULL)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CwxPhyloEditDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxPhyloEditDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxPhyloEditDlg creation
    return true;
}

CwxPhyloEditDlg::~CwxPhyloEditDlg()
{
////@begin CwxPhyloEditDlg destruction
////@end CwxPhyloEditDlg destruction
}

void CwxPhyloEditDlg::Init()
{
////@begin CwxPhyloEditDlg member initialisation
    m_List = NULL;
    m_Name = NULL;
    m_Value = NULL;
    m_ColorPicker = NULL;
////@end CwxPhyloEditDlg member initialisation
}

void CwxPhyloEditDlg::CreateControls()
{
////@begin CwxPhyloEditDlg content construction
    CwxPhyloEditDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    m_List = new wxListCtrl( itemDialog1, ID_LISTCTRL1, wxDefaultPosition, wxSize(300, 120), wxLC_REPORT );
    itemBoxSizer3->Add(m_List, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, ID_BUTTON4, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_BUTTON5, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, ID_BUTTON, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox(itemDialog1, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(itemStaticBoxSizer9Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer9, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer9->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Name = new wxTextCtrl( itemDialog1, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    itemBoxSizer10->Add(m_Name, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer9->Add(itemBoxSizer13, 1, wxGROW|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer13->Add(itemBoxSizer14, 1, wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Value"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Value = new wxTextCtrl( itemDialog1, ID_TEXTCTRL15, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(m_Value, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemDialog1, wxID_STATIC, _("Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ColorPicker = new wxColourPickerCtrl( itemDialog1, ID_COLOURCTRL, wxColour(0, 255, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemBoxSizer13->Add(m_ColorPicker, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine19 = new wxStaticLine( itemDialog1, ID_STATICLINE1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine19, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer20 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer20, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton21 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer20->AddButton(itemButton21);

    wxButton* itemButton22 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer20->AddButton(itemButton22);

    itemStdDialogButtonSizer20->Realize();

////@end CwxPhyloEditDlg content construction
}

void CwxPhyloEditDlg::OnButton4Click( wxCommandEvent& event )
{
    // validate data: not blank, does not duplicate other items
    wxString n = m_Name->GetValue();
    n.Trim(true);
    n.Trim(false);
    if (n.Length() == 0)
        return;
     
    std::string n_lower = ToStdString(n);
    NStr::ToLower(n_lower);

    // iterate through listctrl so that we don't add duplicates
    long item_num = -1;
    for ( ;; ) {
        item_num = m_List->GetNextItem(item_num,
                                       wxLIST_NEXT_ALL,
                                       wxLIST_STATE_DONTCARE);
        if ( item_num == -1 )
            break;

        wxListItem info;
        info.m_itemId = item_num;
        info.m_mask = wxLIST_MASK_TEXT;
        info.m_col = 0;
        string name = m_List->GetItem(info) ? ToStdString(info.m_text) : string();

        if (NStr::ToLower(name) == NStr::ToLower(n_lower)) {
            wxString msg = wxT("Item '");
            msg += n;
            msg += wxT("' is already a property.\n");
            msg += wxT("Select the property from the list to update it.");

            wxMessageBox(msg, wxT("Duplicate Property Name"),
                         wxOK | wxICON_WARNING);

            return;
        }
    }

    // add property:
    wxListItem * item = new wxListItem();
    int itemId = m_List->GetItemCount();
    m_List->InsertItem(itemId, *item);
    m_List->SetItem(itemId, 0, m_Name->GetValue());
    m_List->SetItem(itemId, 1, m_Value->GetValue());    
    event.Skip();
}

void CwxPhyloEditDlg::OnButton5Click( wxCommandEvent& event )
{
    // delete button
    if (m_List->GetSelectedItemCount() == 0)
        return;

    // find selected item index
    int index = m_List->GetNextItem(-1,
                                 wxLIST_NEXT_ALL,
                                 wxLIST_STATE_SELECTED);

    m_List->DeleteItem(index);

    event.Skip();
}

void CwxPhyloEditDlg::OnButtonClick( wxCommandEvent& event )
{
    // update
    x_UpdateSelectedRow();
    event.Skip();
}

void CwxPhyloEditDlg::OnOkClick( wxCommandEvent& event )
{
    // iterate through listctrl    
    long item = -1;
    for ( ;; ) {
        item = m_List->GetNextItem(item,
                                     wxLIST_NEXT_ALL,
                                     wxLIST_STATE_DONTCARE);
        if ( item == -1 )
            break;

        wxListItem info;
        info.m_itemId = item;
        info.m_mask = wxLIST_MASK_TEXT;
        info.m_col = 0;
        string name = m_List->GetItem(info) ? ToStdString(info.m_text) : string();
        info.m_col = 1;
        string value = m_List->GetItem(info) ? ToStdString(info.m_text) : string();

        TBioTreeFeatureId   id;

        if (m_UpdatedFeature->GetDictionary().HasFeature(name)){
            id = m_UpdatedFeature->GetDictionary().GetId(name);
        }
        else { // find smallest available id
            for (id=0; id < 255; id++){
                if (!m_UpdatedFeature->GetDictionary().HasFeature(id)) {
                    m_UpdatedFeature->GetDictionary().Register(id, name);
                    break;
                }
            }
            // should probably put the 255 in bio_tree if we are serious about it
            if (id==255) {
                NCBI_THROW(CException, eUnknown,
                    "CPhyloTreeNode::SetFeature - maximum number of features exceeded");
            }
        }
        m_UpdatedFeature->GetUpdated()[0].GetFeatures().SetFeature(id, value);
    }

    event.Skip();
}

bool CwxPhyloEditDlg::ShowToolTips()
{
    return true;
}
wxBitmap CwxPhyloEditDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxPhyloEditDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxPhyloEditDlg bitmap retrieval
}
wxIcon CwxPhyloEditDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxPhyloEditDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxPhyloEditDlg icon retrieval
}

void CwxPhyloEditDlg::SetParams(CPhyloTree* tree,
                                CPhyloTree::TTreeIdx node_idx,
                                CFeatureEdit* updated_feature)
{
    m_Tree = tree;
    m_NodeIdx = node_idx;
    m_UpdatedFeature = updated_feature;

    int w = m_List->GetSize().GetWidth();

    // make value 2/3 of list width, and name 1/3.
    m_List->InsertColumn(0, wxT("Name"), wxLIST_FORMAT_LEFT, w/3 );
    m_List->InsertColumn(1, wxT("Value"), wxLIST_FORMAT_LEFT, (w*2)/3 );

    const CBioTreeFeatureDictionary & dict  = m_Tree->GetFeatureDict();
    const CBioTreeFeatureList       & flist = (*m_Tree)[m_NodeIdx]->GetBioTreeFeatureList();

    int itemId = 0;
    ITERATE(CBioTreeFeatureList::TFeatureList, it, flist.GetFeatureList()) {
        if (dict.HasFeature(it->id)){
            string sName  = dict.GetFeatureDict().find(it->id)->second;
            string sValue = it->value;
            
            wxListItem * item = new wxListItem();
            m_List->InsertItem(itemId, *item);

            m_List->SetItem(itemId, 0, ToWxString(sName));
            m_List->SetItem(itemId, 1, ToWxString(sValue));

            itemId++;
        }
    }
}


void CwxPhyloEditDlg::OnListctrl1Selected( wxListEvent& event )
{
    if (m_List->GetSelectedItemCount() == 0)
        return;

    // find selected item index
    int index = m_List->GetNextItem(-1,
                                 wxLIST_NEXT_ALL,
                                 wxLIST_STATE_SELECTED);

    wxListItem info;
    info.m_itemId = index;
    info.m_mask = wxLIST_MASK_TEXT;
    info.m_col = 0;
    wxString name = m_List->GetItem(info) ? info.m_text : wxT("");
    info.m_col = 1;
    wxString value = m_List->GetItem(info) ? info.m_text : wxT("");


    // If there is already a color string in the properties put that color
    // on the color picker control   
    string current_value = ToStdString(value);

    string::size_type idx1 = current_value.find_first_of('[');
    if (idx1 != string::npos) {
        string::size_type idx2 = current_value.find_first_of(']');
        if (idx2 != string::npos && idx2 > idx1) {
            // We have found a color (maybe - could also be other text between [ and ]...)
            try {
                string color_str = current_value.substr(idx1, idx2-idx1 + 1);
                CRgbaColor color(color_str);

                /// Set color in color picker
                m_ColorPicker->SetColour(wxColour(color.GetRedUC(),
                                                  color.GetGreenUC(), 
                                                  color.GetBlueUC()));
            }
            catch (CException&) {
                // Not a color - probably other string text (don't worry about it)
            }
        }
    }

    m_Name->SetValue(name);
    m_Value->SetValue(value);

    event.Skip();
}

void CwxPhyloEditDlg::OnListctrl1Deselected( wxListEvent& event )
{
    x_UpdateSelectedRow();
    event.Skip();
}

void CwxPhyloEditDlg::x_UpdateSelectedRow()
{
    if (m_List->GetSelectedItemCount() == 0)
        return;

    // find selected item index
    int index = m_List->GetNextItem(-1,
                                 wxLIST_NEXT_ALL,
                                 wxLIST_STATE_SELECTED);

    // Can't set the name to be blank:
    wxString n = m_Name->GetValue();
    n.Trim(true);
    n.Trim(false);
    if (n.Length() == 0)
        return;
    
    m_List->SetItem(index, 0, m_Name->GetValue());
    m_List->SetItem(index, 1, m_Value->GetValue());

}

void CwxPhyloEditDlg::OnColourctrlColourPickerChanged( wxColourPickerEvent& event )
{
    // Convert wxColor to string format [r,g,b] where r,g,b are 0..255.
    CRgbaColor  new_color(ToStdString(event.GetColour().GetAsString(wxC2S_HTML_SYNTAX))); 

    // If there is already a color string in the properties replace it.  Otherwise
    // insert the color value at the beginning of the text.
    string current_value = ToStdString(m_Value->GetValue());

    string::size_type idx1 = current_value.find_first_of('[');
    if (idx1 != string::npos) {
        string::size_type idx2 = current_value.find_first_of(']');
        if (idx2 != string::npos && idx2 > idx1) {
            // We have found a color.  Replace it.
            string old_color_str = current_value.substr(idx1, idx2-idx1 + 1);

            try {
                CRgbaColor old_color(old_color_str);

                // Retain the current alpha value since color picker does not let you
                // choose that.
                new_color.SetAlpha(old_color.GetAlpha());   
                string new_color_str = std::string("[") + new_color.ToString(true,true) +  std::string("]");

                current_value.replace(idx1, idx2-idx1 + 1, new_color_str);
                m_Value->SetValue(ToWxString(current_value));            
                return;
            }
            catch (CException&) {
                // Not a color - probably other string text (don't worry about it)
            }            
        }
    }
    
    // Alpha of 255 is unreasonable (since you can't see nodes) so set default to 25%:
    new_color.SetAlpha((unsigned char)64);
    string new_color_str = std::string("[") + new_color.ToString(true,true) +  std::string("]");
    m_Value->SetValue(ToWxString(new_color_str + " " + current_value));

    event.Skip();
}

END_NCBI_SCOPE



