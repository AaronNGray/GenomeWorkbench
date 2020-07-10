/*  $Id: convert_set_type_dlg.cpp 37014 2016-11-25 15:32:09Z filippov $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
#include <objects/seqset/Seq_entry.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <objmgr/seqdesc_ci.hpp>
#include <gui/objutils/cmd_change_set_class.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/convert_set_type_dlg.hpp>
#include <gui/widgets/edit/set_class_panel.hpp>

#include <wx/sizer.h>
#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CConvertSetTypeDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CConvertSetTypeDlg, wxDialog )


/*!
 * CConvertSetTypeDlg event table definition
 */

BEGIN_EVENT_TABLE( CConvertSetTypeDlg, wxDialog )

////@begin CConvertSetTypeDlg event table entries
    EVT_TREE_SEL_CHANGED( ID_TREEBOOK, CConvertSetTypeDlg::OnTreebookSelChanged )

    EVT_BUTTON( wxID_OK, CConvertSetTypeDlg::OnOkClick )

////@end CConvertSetTypeDlg event table entries

END_EVENT_TABLE()


/*!
 * CConvertSetTypeDlg constructors
 */

CConvertSetTypeDlg::CConvertSetTypeDlg()
{
    Init();
}

CConvertSetTypeDlg::CConvertSetTypeDlg( wxWindow* parent, const CBioseq_set& set, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_Set.Reset(new CBioseq_set());
    m_Set->Assign(set);
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CConvertSetTypeDlg creator
 */

bool CConvertSetTypeDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConvertSetTypeDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConvertSetTypeDlg creation
    return true;
}


/*!
 * CConvertSetTypeDlg destructor
 */

CConvertSetTypeDlg::~CConvertSetTypeDlg()
{
////@begin CConvertSetTypeDlg destruction
////@end CConvertSetTypeDlg destruction
}


/*!
 * Member initialisation
 */

void CConvertSetTypeDlg::Init()
{
////@begin CConvertSetTypeDlg member initialisation
    m_Tree = NULL;
    m_ClassChoice = NULL;
////@end CConvertSetTypeDlg member initialisation
}


/*!
 * Control creation for CConvertSetTypeDlg
 */

void CConvertSetTypeDlg::CreateControls()
{    
////@begin CConvertSetTypeDlg content construction
    CConvertSetTypeDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Tree = new wxTreeCtrl( itemDialog1, ID_TREEBOOK, wxDefaultPosition, wxSize(300, 300), wxTR_SINGLE );
    itemBoxSizer3->Add(m_Tree, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ClassChoice = new CSetClassPanel( itemDialog1, ID_WINDOW8, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_ClassChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CConvertSetTypeDlg content construction

    objects::CBioseq_set::EClass class_type = m_Set->IsSetClass() ? m_Set->GetClass() : CBioseq_set::eClass_not_set;
    wxTreeItemId root = m_Tree->AddRoot(ToWxString(CSetClassPanel::GetClassName(class_type)));
    m_ItemSets.push_back(TItemSet(root, m_Set.GetPointer()));
    m_ClassChoice->SetClass(class_type);
    if (m_Set->IsSetSeq_set()) {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, it, m_Set->SetSeq_set()) {
            if ((*it)->IsSet()) {
                x_AddSet((*it)->SetSet(), root);
            }
        }
    }
    m_Tree->ExpandAll();
}


void CConvertSetTypeDlg::x_AddSet(objects::CBioseq_set& set, wxTreeItemId parent)
{
    objects::CBioseq_set::EClass class_type = set.IsSetClass() ? set.GetClass() : CBioseq_set::eClass_not_set;
//    CSetClassPanel* pnl = new CSetClassPanel(this);
//    pnl->SetClass(class_type);
    wxTreeItemId this_node = m_Tree->AppendItem(parent, ToWxString(CSetClassPanel::GetClassName(class_type)));
    m_ItemSets.push_back(TItemSet(this_node, &set));
    m_ClassChoice->SetClass(class_type);
    if (set.IsSetSeq_set()) {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, it, set.SetSeq_set()) {
            if ((*it)->IsSet()) {
                x_AddSet((*it)->SetSet(), this_node);
            }
        }
    }
}


/*!
 * Should we show tooltips?
 */

bool CConvertSetTypeDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CConvertSetTypeDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConvertSetTypeDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConvertSetTypeDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CConvertSetTypeDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConvertSetTypeDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConvertSetTypeDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CConvertSetTypeDlg::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CConvertSetTypeDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CConvertSetTypeDlg. 
}



/*!
 * wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREEBOOK
 */

void CConvertSetTypeDlg::OnTreebookSelChanged( wxTreeEvent& event )
{
    wxTreeItemId id = event.GetItem();
    CBioseq_set &bset = x_GetSetFromId(id);
    CBioseq_set::EClass class_type = bset.IsSetClass() ? bset.GetClass() : CBioseq_set::eClass_not_set;
    m_ClassChoice->SetClass(class_type);            
}


void CConvertSetTypeDlg::ChangeNotify()
{
    wxTreeItemId id = m_Tree->GetSelection();
    CBioseq_set &bset = x_GetSetFromId(id);

    CBioseq_set::EClass old_class = bset.GetClass();
    CBioseq_set::EClass class_type = m_ClassChoice->GetClass();        

    if (old_class == CBioseq_set::eClass_genbank && bset.IsSetSeq_set() && !bset.GetSeq_set().empty() &&
        bset.GetSeq_set().front()->IsSet() && bset.GetSeq_set().front()->GetSet().GetClass() != CBioseq_set::eClass_nuc_prot )
    {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, it, bset.SetSeq_set()) 
        {
            (*it)->SetSet().SetClass(class_type);
            wxTreeItemId sub_id = x_GetIdFromSet(&(*it)->SetSet());
            m_Tree->SetItemText(sub_id, ToWxString(CSetClassPanel::GetClassName(class_type)));
        }
    }
    else
    {
        bset.SetClass(class_type);        
        m_Tree->SetItemText(id, ToWxString(CSetClassPanel::GetClassName(class_type)));
    }
}


CBioseq_set &CConvertSetTypeDlg::x_GetSetFromId(wxTreeItemId id)
{
    for (size_t i = 0; i < m_ItemSets.size(); i++) {
        if (m_ItemSets[i].first == id) {
            return *m_ItemSets[i].second;
        }
    }
    return *m_Set;
}

wxTreeItemId CConvertSetTypeDlg::x_GetIdFromSet( CBioseq_set *bset)
{
    wxTreeItemId id;
    for (size_t i = 0; i < m_ItemSets.size(); i++) {
        if (m_ItemSets[i].second == bset) {
            id =  m_ItemSets[i].first;
        }
    }
    return id;
}


CRef<CCmdComposite> CConvertSetTypeDlg::GetCommand(CScope& scope, 
                                                   const objects::CBioseq_set& orig, 
                                                   const objects::CBioseq_set& new_set,
                                                   bool& error)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Change Set Class"));
    bool any = false;

    if (new_set.IsSetClass() &&
        (!orig.IsSetClass() || orig.GetClass() != new_set.GetClass())) {
        CBioseq_set_Handle bh = scope.GetBioseq_setHandle(orig);
        CRef<CCmdChangeSetClass> sub(new CCmdChangeSetClass(bh, new_set.GetClass()));
        cmd->AddCommand(*sub);
        if (!CBioseq_set::NeedsDocsumTitle(new_set.GetClass()))
        {
            CSeq_entry_Handle seh = bh.GetParentEntry();
            for (CSeqdesc_CI desc(seh, CSeqdesc::e_Title, 1); desc; ++desc)
            {
                CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(seh, *desc));
                cmd->AddCommand(*cmdDelDesc);
            }
        }
        any = true;
    }

    if (m_Set->IsSetSeq_set()) {
        if (orig.IsSetSeq_set()) {
            CBioseq_set::TSeq_set::const_iterator orig_i = orig.GetSeq_set().begin();
            CBioseq_set::TSeq_set::const_iterator new_i = new_set.GetSeq_set().begin();
            while (orig_i != orig.GetSeq_set().end()  &&
                   new_i != new_set.GetSeq_set().end()) {
                if ((*orig_i)->IsSet()) {
                    if ((*new_i)->IsSet()) {
                        CRef<CCmdComposite> sub = GetCommand(scope, 
                                                             (*orig_i)->GetSet(), 
                                                             (*new_i)->GetSet(),
                                                             error);
                        if (error) {
                            break;
                        } else if (sub) {
                            cmd->AddCommand(*sub);
                            any = true;
                        }
                    } else {
                        // sets don't match up
                        error = true;
                        break;
                    }
                } else if ((*new_i)->IsSet()) {
                    // sets don't match up
                    error = true;
                    break;
                }
                orig_i++;
                new_i++;
            }
            if (orig_i != orig.GetSeq_set().end() || new_i != new_set.GetSeq_set().end()) {
                // sets don't match up
                error = true;
            }
        } else {
            // sets don't match up
            error = true;
        }
    } else if (orig.IsSetSeq_set()) {
        // sets don't match up
        error = true;
    }

    if (!any || error) {
        cmd.Reset(NULL);
    }
    return cmd;
}


CRef<CCmdComposite> CConvertSetTypeDlg::GetCommand(CScope& scope, const objects::CBioseq_set& orig, bool& error)
{
    error = false;
    return GetCommand(scope, orig, *m_Set, error);
}


END_NCBI_SCOPE
