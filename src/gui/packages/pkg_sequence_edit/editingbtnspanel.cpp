/*  $Id: editingbtnspanel.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
 * Authors:  Colleen Bollin and Igor Filippov
 */


#include <ncbi_pch.hpp>


////@begin includes
////@end includes

#include <gui/objutils/cmd_factory.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_del_bioseq_set.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/project_item_extra.hpp>
#include <objtools/edit/mail_report.hpp>
#include <gui/core/app_dialogs.hpp>
#include <gui/framework/view_manager_service.hpp>

#include <objmgr/bioseq_ci.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <util/checksum.hpp>
#include <connect/ncbi_util.h>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <wx/icon.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <wx/settings.h>
#include <wx/event.h>
#include <wx/clipbrd.h>
#include <wx/display.h>
#include <wx/hashmap.h>
#include <wx/image.h>
#include <wx/colordlg.h>
#include <wx/stdpaths.h>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include "open_data_source_object_dlg.hpp"
#include <gui/widgets/macro_edit/var_resolver_dlg.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/macro_engine.hpp>

#include <gui/utils/extension_impl.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <gui/packages/pkg_sequence_edit/editingbtnspanel.hpp>
#include <gui/packages/pkg_sequence_edit/remove_desc_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences_from_alignments.hpp>
#include <gui/packages/pkg_sequence_edit/remove_features_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_structured_comment.hpp>
#include <gui/packages/pkg_sequence_edit/remove_annot.hpp>
#include <gui/packages/pkg_sequence_edit/remove_genomeprojectdb.hpp>
#include <gui/packages/pkg_sequence_edit/add_citsubupdate.hpp>
#include <gui/packages/pkg_sequence_edit/pubdesc_editor.hpp>
#include <gui/packages/pkg_sequence_edit/modify_structured_comment.hpp>
#include <gui/packages/pkg_sequence_edit/remove_bankit_comment.hpp>
#include <gui/packages/pkg_sequence_edit/remove_structuredcomment_keyword.hpp>
#include <gui/packages/pkg_sequence_edit/reorder_sequences_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/convert_comment.hpp>
#include <gui/packages/pkg_sequence_edit/modify_feature_id.hpp>
#include <gui/packages/pkg_sequence_edit/pcr-primers-edit.hpp>
#include <gui/packages/pkg_sequence_edit/fix_orgmod_institution.hpp>
#include <gui/packages/pkg_sequence_edit/parse_collectiondate_formats.hpp>
#include <gui/packages/pkg_sequence_edit/country_fixup.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/add_deflinedlg.hpp>
#include <gui/packages/pkg_sequence_edit/vectortrimdlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_feature_add_dlg_std.hpp>
#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/utils/app_popup.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/packages/pkg_sequence_edit/seqid_fix_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_edit_feat_dlg_std.hpp>
#include <gui/packages/pkg_sequence_edit/convert_feat_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/molinfo_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/srceditdialog.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_functions.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/strain_serotype_influenza.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_unverified.hpp>
#include <gui/packages/pkg_sequence_edit/prefix_deflinedlg.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/table_view.hpp>
#include <gui/widgets/edit/init.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/edit_pub_dlg_std.hpp>
#include <gui/packages/pkg_sequence_edit/qual_table_load_manager.hpp>
#include <gui/packages/pkg_sequence_edit/trim_n.hpp>
#include <gui/widgets/edit/edit_object_seq_desc.hpp>
#include <gui/packages/pkg_sequence_edit/mrna_cds.hpp>
#include <gui/packages/pkg_sequence_edit/rem_prot_titles.hpp>
#include <gui/packages/pkg_sequence_edit/correct_rna_strand_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cds_edit.hpp>
#include <gui/packages/pkg_sequence_edit/add_transl_except_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_gene_edit.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_rna_edit.hpp>
#include <gui/packages/pkg_sequence_edit/label_rna.hpp>
#include <gui/packages/pkg_sequence_edit/retranslate_cds.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>
#include <gui/packages/pkg_sequence_edit/macro_worker.hpp>
#include <gui/objutils/cmd_insert_bioseq_set.hpp>
#include <gui/packages/pkg_sequence_edit/choose_set_class_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/suc_refresh_cntrl.hpp>
#include <gui/packages/pkg_sequence_edit/select_target_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/uncul_tax_tool.hpp>
#include <gui/packages/pkg_sequence_edit/remove_srcnotes.hpp>
#include <gui/packages/pkg_sequence_edit/extend_partial_features.hpp>
#include <gui/packages/pkg_sequence_edit/convert_cds_to_misc_feat.hpp>
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/objutils/table_data_base.hpp>
#include <gui/packages/pkg_sequence_edit/lowercase_qualifiers.hpp>
#include <gui/packages/pkg_sequence_edit/correct_genes_dialog.hpp>
#include <objtools/edit/struc_comm_field.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include "edit_table_views.hpp"
#include <gui/packages/pkg_sequence_edit/add_sequences.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/export_table.hpp>
#include <gui/packages/pkg_sequence_edit/sequester_sets.hpp>
#include <gui/widgets/edit/edit_object_submit_block.hpp>
#include <gui/packages/pkg_sequence_edit/wrong_illegal_quals.hpp>
#include <gui/packages/pkg_sequence_edit/group_explode.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_inside_string_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/find_asn1_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq_worker.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/packages/pkg_sequence_edit/discrepancy_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/featuretbl_frame.hpp>
#include <gui/packages/pkg_sequence_edit/import_feat_table.hpp>
#include <gui/packages/pkg_sequence_edit/sequin_desktop_view.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/packages/pkg_sequence_edit/csuc_frame.hpp>
#include <gui/widgets/seq_desktop/desktop_event.hpp>
#include <gui/packages/pkg_sequence_edit/validate_frame.hpp>
#include <gui/packages/pkg_sequence_edit/link_mrna_cds.hpp>
#include <gui/packages/pkg_sequence_edit/barcode_tool.hpp>
#include <gui/widgets/edit/edit_sequence.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/aecr_frame.hpp>
#include "attrib_table_column_id_panel.hpp"

#include <gui/packages/pkg_sequence_edit/parse_text_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/append_mod_to_org.hpp>
#include <gui/packages/pkg_sequence_edit/append_mod_to_org_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/add_macro_button_dlg.hpp>
#include <gui/widgets/edit/macrofloweditor.hpp>
#include "gui_core_helper_impl.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
using namespace objects;

#ifdef NCBI_OS_MSWIN
#define BUTTON_WIDTH 119
#define BUTTON_HEIGHT 23
#else                                // NCBI_OS_LINUX NCBI_OS_DARWIN
#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 23
#endif

#define DIST_BETWEEN_BUTTONS 5    

IMovableButton::IMovableButton(wxPanel* parent, wxWindowID id, const wxString &label, const wxPoint &pos, const wxSize &size, long style) : wxButton(parent,wxID_ANY,label,pos,size,style),
                                                                                                                                            m_OrigId(id), m_Position(pos), m_dragging(false),
                                                                                                                                            m_locked(true),m_removed(false),m_Sizer(parent->GetSizer())
{
    m_menu.Append(ID_POPUP_A, "Tab A");
    m_menu.Append(ID_POPUP_B, "Tab B");
    m_menu.Append(ID_POPUP_C, "Tab C");
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &IMovableButton::onMouseClick, this, wxID_ANY); // workaround for pre-2.9.5 wxWidgets which do not have wxEVT_BUTTON yet.
    Bind(wxEVT_CONTEXT_MENU,&IMovableButton::OnRightClick,this, wxID_ANY);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &IMovableButton::OnPopupClick, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &IMovableButton::OnPopupClickUpdate, this, ID_POPUP_REMOVE);
    Bind(wxEVT_KEY_DOWN,&IMovableButton::OnKeyDown,this, wxID_ANY);
}

void IMovableButton::AddDynamicMenu()
{
    m_menu.Append(ID_POPUP_REMOVE, "Delete Button");
    m_menu.Append(ID_POPUP_EDIT, "Edit Script");
}

void IMovableButton::SetLocked(bool locked)
{
    m_locked = locked;
    if (m_locked)
        m_dragging = false;
}

void IMovableButton::SetRemoved(bool removed)
{
    m_removed = removed;
}

void IMovableButton::SetDragging(bool dragging)
{
    m_dragging = dragging;
}

void IMovableButton::SetSizers( wxWindow *win_top, wxSizer *sizer1, wxSizer *sizer2, wxSizer *sizer3, wxSizer *sizer_removed_actual)
{
    if (win_top)  m_win_top = win_top;
    if (sizer1) m_TopSizer1 = sizer1;
    if (sizer2) m_TopSizer2 = sizer2;
    if (sizer3) m_TopSizer3 = sizer3;
    if (sizer_removed_actual) m_TopSizerRemoved = sizer_removed_actual;
}

bool IMovableButton::IsOccupiedPosition()
{
    bool occupied = false;
    wxRect r = GetRect();

    wxSizerItemList& children = m_Sizer->GetChildren();
    for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
    {
        wxWindow* w = (*child)->GetWindow();
        if (w)
        {
            if (w == dynamic_cast<wxWindow*>(this))
            {
                continue;
            }
            wxRect r1 = w->GetRect();
          
            if (r.Intersects(r1))
            {
                occupied = true;
            }                 
        }
    }
    return occupied;
}

wxPoint IMovableButton::FindUnoccupiedPosition(wxSizer *sizer)
{
    if (sizer == NULL) sizer = m_Sizer;
    int x = DIST_BETWEEN_BUTTONS, y = DIST_BETWEEN_BUTTONS;
    int width = GetRect().GetWidth();
    int height = GetRect().GetHeight();

    int row = 1;           
    wxSizerItemList& children = sizer->GetChildren();
    for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
    {
        wxRect r(x,y,width,height);
        wxWindow* w = (*child)->GetWindow();
        if (w)
        {
            if (w == dynamic_cast<wxWindow*>(this))
            {
                break;
            }
            wxRect r1 = w->GetRect();
            if (r1.GetTop() > r.GetTop())
            {
                x = DIST_BETWEEN_BUTTONS;
                y = r1.GetBottom() + DIST_BETWEEN_BUTTONS;
                row = 1;
                continue;
            }
            if (r.Intersects(r1))
            {
                x = r1.GetRight() + DIST_BETWEEN_BUTTONS;
                row++;
                if (x+width > sizer->GetContainingWindow()->GetRect().GetWidth() || row > 2)
                {
                    x = DIST_BETWEEN_BUTTONS;
                    y = r1.GetBottom() + DIST_BETWEEN_BUTTONS;
                    row = 1;
                }
            }                 
        }
    }
    
    return wxPoint(x,y);
}

void IMovableButton::SetScrollPos(wxPoint scrollpos)
{
    m_ScrollPos = scrollpos;
}

void IMovableButton::CopyButton(wxSizer *sizer, wxPoint p)
{
    CButtonPanel * panel = dynamic_cast<CButtonPanel*>(sizer->GetContainingWindow());
    if (p == wxDefaultPosition)
        p = FindUnoccupiedPosition(sizer);     
    wxPoint scrollpos = panel->GetScrollPos();
    SetScrollPos(scrollpos);
    if (p.x < 0)
        p.x += scrollpos.x;
    if (p.y < 0)
        p.y += scrollpos.y;
    IMovableButton *button = panel->CreateButton(this,p.x,p.y); 
    int bottom = panel->GetVirtualSize().y;
    int button_bottom = button->GetRect().GetBottom() + DIST_BETWEEN_BUTTONS;
    if (button_bottom > bottom)
        panel->SetVirtualSize(panel->GetVirtualSize().x, button_bottom);
    button->SetSizers(m_win_top,m_TopSizer1,m_TopSizer2,m_TopSizer3,m_TopSizerRemoved);
    button->SetLocked(m_locked);
}

void IMovableButton::OnPopupClick(wxCommandEvent &evt)
{
    switch(evt.GetId()) 
    {
    case ID_POPUP_A:
        CopyButton(m_TopSizer1,wxDefaultPosition);
        break;
    case ID_POPUP_B:
        CopyButton(m_TopSizer2,wxDefaultPosition);
        break;
    case ID_POPUP_C:
        CopyButton(m_TopSizer3,wxDefaultPosition);
        break;
    case ID_POPUP_REMOVE:
        CallAfter(&IMovableButton::RemoveButton);
    break;
    case ID_POPUP_EDIT:
        {
        CEditingBtnsPanel* e = dynamic_cast<CEditingBtnsPanel*> (m_win_top);
        if (e)
        e->EditButton(this);
    }
    break;
    }
 }

void IMovableButton::OnPopupClickUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_OrigId >= ID_BUTTON_DYNAMIC);
}

void IMovableButton::RemoveButton()
{
    CEditingBtnsPanel* e = dynamic_cast<CEditingBtnsPanel*> (m_win_top);
    if (e)
    e->RemoveButton(this);
}

void IMovableButton::DeleteButton()
{
    if (!m_locked && !m_removed)
    {
        CButtonPanel * panel = dynamic_cast<CButtonPanel*>(m_Sizer->GetContainingWindow());
        panel->SetMovingButton(NULL,false);
        Destroy();
    }
}

void IMovableButton::OnRightClick(wxContextMenuEvent &evt) 
{
    wxPoint mouseOnScreen = wxGetMousePosition();
    wxRect r = GetScreenRect();
    if (r.Contains(mouseOnScreen.x,mouseOnScreen.y))
    {
        if (!m_locked && !m_removed)
            DeleteButton();   
        else if (m_removed)
            PopupMenu(&m_menu);
        else
            evt.Skip();
    }
}

void IMovableButton::onMouseClick(wxCommandEvent& evt)
{
    if (!IsShownOnScreen())
        return;
    wxPoint mouseOnScreen = wxGetMousePosition();
    wxRect r = GetScreenRect();
    if (r.Contains(mouseOnScreen.x,mouseOnScreen.y))
    {
        if (m_locked)
        {
            CEditingBtnsPanel* e = dynamic_cast<CEditingBtnsPanel*> (m_win_top);
            if (e)
            {
                if (VerifySelection(e))
                {
                    evt.SetId(m_OrigId);
                    evt.Skip(); 
                }
            }
        }
        else if (!m_removed)
        {
            wxPoint mouseOnScreen = wxGetMousePosition();
            m_dragging = !m_dragging;
            CButtonPanel * panel = dynamic_cast<CButtonPanel*>(m_Sizer->GetContainingWindow());
            panel->SetMovingButton(this,m_dragging);
        }
    }
}
    

void IMovableButton::MoveButton(wxPoint mousePos)
{
    if(m_dragging && !m_removed)
    {
        CButtonPanel *w = dynamic_cast<CButtonPanel *>(GetParent());    
        wxPoint scrollpos = w->GetScrollPos();
        SetScrollPos(scrollpos);

        int cx,cy;
        GetPosition(&cx,&cy);
      

// Might have to do the same for mouse coordinates in case of scrolled window
        int sdx,sdy;
        GetSize(&sdx,&sdy);

        int dx = 0;
        int dy = 0;
       

        if (mousePos.x > cx + sdx) dx = sdx+DIST_BETWEEN_BUTTONS;
        if (mousePos.x < cx ) dx = -(sdx+DIST_BETWEEN_BUTTONS);
        if (mousePos.y > cy + sdy) dy = (sdy+DIST_BETWEEN_BUTTONS);
        if (mousePos.y < cy ) dy = -(sdy+DIST_BETWEEN_BUTTONS);

        cx += dx;
        cy += dy;

        wxRect r = w->GetScreenRect();
        if (cx > r.GetWidth())
            cx -= (sdx+DIST_BETWEEN_BUTTONS);
        if (cx < 0)
            cx += (sdx+DIST_BETWEEN_BUTTONS);
        //if (cy+sdy > r.GetHeight())
        //    cy -= (sdy+DIST_BETWEEN_BUTTONS);
        if (cy < 0)
            cy += (sdy+DIST_BETWEEN_BUTTONS);

        SetPosition( wxPoint(cx, cy) );               
        m_Position = wxPoint(w->CalcUnscrolledPosition(GetPosition()));    
        int bottom = w->GetVirtualSize().y;
        int button_bottom = GetRect().GetBottom() + DIST_BETWEEN_BUTTONS;
        if (button_bottom > bottom)
        {
            w->SetVirtualSize(w->GetVirtualSize().x, button_bottom);
        }
    }    
}
 
void IMovableButton::OnKeyDown(wxKeyEvent& event)
{
    OnChar(event);
}

bool IMovableButton::OnChar(wxKeyEvent& event)
{
    bool found = false;
    CButtonPanel *w = dynamic_cast<CButtonPanel *>(GetParent());    
    wxPoint scrollpos = w->GetScrollPos();
    SetScrollPos(scrollpos);

    int cx,cy;
    GetPosition(&cx,&cy);
    wxPoint pos(cx,cy);
    int sdx,sdy;
    GetSize(&sdx,&sdy);
    switch ( event.GetKeyCode() )
    {
    case WXK_LEFT  : pos.x--; MoveButton(pos); found = true; break;
    case WXK_RIGHT : pos.x += sdx+1; MoveButton(pos); found = true; break;
    case WXK_UP    : pos.y--; MoveButton(pos); found = true; break;
    case WXK_DOWN  : pos.y += sdy+1; MoveButton(pos); found = true; break;
    case WXK_RETURN: m_dragging = !m_dragging; found = true; break;
    default: event.Skip(); break;
    }
    return found;
}

IMPLEMENT_ABSTRACT_CLASS( IMovableButton, wxButton )

class CMovableButton : public IMovableButton
{
public:
    CMovableButton() : IMovableButton() {}
    
    CMovableButton(wxPanel* parent, wxWindowID id, const wxString &label=wxEmptyString, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0) : IMovableButton(parent,id,label,pos,size,style) {}
                            
    virtual ~CMovableButton() {}                                                                                                                                                          
    virtual bool VerifySelection(CEditingBtnsPanel* e)
        {
            return e->OnDataChange();
        }
 DECLARE_DYNAMIC_CLASS( CMovableButton )
};

IMPLEMENT_DYNAMIC_CLASS( CMovableButton, IMovableButton )

class CFeatureSubtypeSelector
{
public: 
    CFeatureSubtypeSelector() {}
    CFeatureSubtypeSelector(CSeqFeatData::ESubtype subtype) : m_Subtype(subtype) {}
    virtual ~CFeatureSubtypeSelector() {}
    virtual CSeqFeatData::ESubtype GetSubtype() {return m_Subtype;}
private:
    CSeqFeatData::ESubtype m_Subtype;
};

class CFeatureMovableButton : public IMovableButton, public CFeatureSubtypeSelector
{
public:
    CFeatureMovableButton() : IMovableButton(), CFeatureSubtypeSelector() {}
    
    CFeatureMovableButton(wxPanel* parent, wxWindowID id, CSeqFeatData::ESubtype subtype, const wxString &label=wxEmptyString, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0) : IMovableButton(parent,id,label,pos,size,style), CFeatureSubtypeSelector(subtype) {}
                            
    virtual ~CFeatureMovableButton() {}                                                                                                                                                          
    virtual bool VerifySelection(CEditingBtnsPanel* e)
        {
            return  e->IsLocationSelected();
        }
 DECLARE_DYNAMIC_CLASS( CFeatureMovableButton )
};

IMPLEMENT_DYNAMIC_CLASS( CFeatureMovableButton, IMovableButton )

class CFeatureMovableButtonNa : public IMovableButton, public CFeatureSubtypeSelector
{
public:
    CFeatureMovableButtonNa() : IMovableButton(), CFeatureSubtypeSelector() {}
    
    CFeatureMovableButtonNa(wxPanel* parent, wxWindowID id, CSeqFeatData::ESubtype subtype, const wxString &label=wxEmptyString, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0) : IMovableButton(parent,id,label,pos,size,style), CFeatureSubtypeSelector(subtype) {}
                            
    virtual ~CFeatureMovableButtonNa() {}                                                                                                                                                          
    virtual bool VerifySelection(CEditingBtnsPanel* e)
        {
            return e->IsLocationSelectedNa();
        }
 DECLARE_DYNAMIC_CLASS( CFeatureMovableButtonNa )
};

IMPLEMENT_DYNAMIC_CLASS( CFeatureMovableButtonNa, IMovableButton )

class CFeatureMovableButtonAa : public IMovableButton, public CFeatureSubtypeSelector
{
public:
    CFeatureMovableButtonAa() : IMovableButton(), CFeatureSubtypeSelector() {}
    
    CFeatureMovableButtonAa(wxPanel* parent, wxWindowID id, CSeqFeatData::ESubtype subtype, const wxString &label=wxEmptyString, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0) : IMovableButton(parent,id,label,pos,size,style), CFeatureSubtypeSelector(subtype) {}
                            
    virtual ~CFeatureMovableButtonAa() {}                                                                                                                                                          
    virtual bool VerifySelection(CEditingBtnsPanel* e)
        {
            return e->IsLocationSelectedAa();
        }
 DECLARE_DYNAMIC_CLASS( CFeatureMovableButtonAa )
};

IMPLEMENT_DYNAMIC_CLASS( CFeatureMovableButtonAa, IMovableButton )


CButtonPanel::CButtonPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size) : wxScrolledWindow(parent,id, pos, size)
{
            m_MovingButton = NULL;
            Bind(wxEVT_LEFT_UP, &CButtonPanel::onMouseClick, this, wxID_ANY);
            Bind(wxEVT_KEY_DOWN,&CButtonPanel::OnKeyDown,this, wxID_ANY);
}

CButtonPanel::~CButtonPanel() 
{ 
}

void CButtonPanel::OnKeyDown(wxKeyEvent& event)
{
    OnChar(event);
}

bool CButtonPanel::OnChar(wxKeyEvent& evt)
{
    bool found = false;
    if (m_MovingButton)
    {
        found = m_MovingButton->OnChar(evt);
    }

    if (!found)
        evt.Skip();
    return found;
}

bool CButtonPanel::Layout()
{    
    wxWindow* main = GetGrandParent()->GetParent();
    CEditingBtnsPanel* e = dynamic_cast<CEditingBtnsPanel*> (main);
    if (!e) return false;
    Scroll(0,0);
    wxSizer* sizer = GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    int max_y = 0;
    int bottom = GetVirtualSize().y;
    for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
    {
        wxWindow* w = (*child)->GetWindow();
        if (w)
        {
            IMovableButton* button = dynamic_cast<IMovableButton*>(w);
            if (button)
            {
                wxPoint p = button->GetOrigPosition();
                if (p.x < 0 || p.y < 0)
                    p = button->FindUnoccupiedPosition();  
                if (p.y > max_y)
                    max_y = p.y;
                button->SetPosition(CalcScrolledPosition(p));
            }
            else
            {
                w->SetPosition(CalcScrolledPosition(wxPoint(DIST_BETWEEN_BUTTONS, max_y+BUTTON_HEIGHT+DIST_BETWEEN_BUTTONS)));
            }
            bottom = max(bottom, w->GetRect().GetBottom() + DIST_BETWEEN_BUTTONS);
        }
    }
    bool auto_layout = GetAutoLayout();
    SetAutoLayout(false);
    SetVirtualSize(GetVirtualSize().x, bottom);
    SetAutoLayout(auto_layout);
    return true;
}


void CButtonPanel::onMouseClick(wxMouseEvent& evt)
{
    if (m_MovingButton)
    {
        wxPoint p = evt.GetPosition();        
        m_MovingButton->MoveButton(p);
    }
    evt.Skip();
}

wxPoint CButtonPanel::GetScrollPos()
{
    wxPoint scrollpos = GetViewStart();
    int x,y;
    GetScrollPixelsPerUnit(&x,&y);
    scrollpos.x *= x;
    scrollpos.y *= y;
    return scrollpos;
}

void CButtonPanel::SetMovingButton(IMovableButton *btn, bool dragging)
{
    if (m_MovingButton  && btn != m_MovingButton)
        m_MovingButton->SetDragging(false);

    if (dragging)
    {
        m_MovingButton = btn;    
        m_MovingButton->SetScrollPos(GetScrollPos());
    }
    else
    {
        m_MovingButton = NULL;
    }
}

IMovableButton* CButtonPanel::CreateButton(wxWindow *w, int x, int y)
{
    IMovableButton *b = NULL;
    CMovableButton *b1 = dynamic_cast<CMovableButton*>(w);
    if (b1)
    {
        CMovableButton* button = new CMovableButton( this, b1->GetOrigId(), b1->GetLabel(), wxPoint(x,y), wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
        GetSizer()->Add(button, 0, wxALIGN_LEFT|wxALL, DIST_BETWEEN_BUTTONS);
        b = button;
    }
    CFeatureMovableButton *b2 = dynamic_cast<CFeatureMovableButton*>(w);
    if (b2)
    {
        CFeatureMovableButton* button = new CFeatureMovableButton( this, b2->GetOrigId(), b2->GetSubtype(), b2->GetLabel(), wxPoint(x,y), wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
        GetSizer()->Add(button, 0, wxALIGN_LEFT|wxALL, DIST_BETWEEN_BUTTONS);
        b = button;
    }
    CFeatureMovableButtonNa *b3 = dynamic_cast<CFeatureMovableButtonNa*>(w);
    if (b3)
    {
        CFeatureMovableButtonNa* button = new CFeatureMovableButtonNa( this, b3->GetOrigId(), b3->GetSubtype(), b3->GetLabel(), wxPoint(x,y), wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
        GetSizer()->Add(button, 0, wxALIGN_LEFT|wxALL, DIST_BETWEEN_BUTTONS);
        b = button;
    }
    CFeatureMovableButtonAa *b4 = dynamic_cast<CFeatureMovableButtonAa*>(w);
    if (b4)
    {
        CFeatureMovableButtonAa* button = new CFeatureMovableButtonAa( this, b4->GetOrigId(), b4->GetSubtype(), b4->GetLabel(), wxPoint(x,y), wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
        GetSizer()->Add(button, 0, wxALIGN_LEFT|wxALL, DIST_BETWEEN_BUTTONS);
        b = button;
    }
    Refresh();

    return b;
}

static bool comp_buttons(const IMovableButton* a, const IMovableButton* b)
{
    wxString a_label = a->GetLabel();
    wxString b_label = b->GetLabel();
    if (a_label[0].GetValue() == '*' && b_label[0].GetValue() != '*')
        return false;
    if (a_label[0].GetValue() != '*' && b_label[0].GetValue() == '*')
        return true;
    return a_label.CmpNoCase(b_label) < 0;
}

IMPLEMENT_DYNAMIC_CLASS( CButtonPanel, wxScrolledWindow )

/*!
 * CEditingBtnsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditingBtnsPanel, wxFrame )


/*!
 * CEditingBtnsPanel event table definition
 */

BEGIN_EVENT_TABLE( CEditingBtnsPanel, wxFrame)

   EVT_CHECKBOX( ID_CHECKBOX_COLLAPSE, CEditingBtnsPanel::OnClickLock )
   EVT_BUTTON( ID_RESET_TABS, CEditingBtnsPanel::OnResetTabs )
   EVT_BUTTON( ID_IMPORT_TABS, CEditingBtnsPanel::OnImportBtnClick )
   EVT_BUTTON( ID_EXPORT_TABS, CEditingBtnsPanel::OnExportBtnClick )
   EVT_BUTTON( ID_ADD_MACRO, CEditingBtnsPanel::OnAddMacroClick )
   EVT_HYPERLINK(ID_SET_BG_COLOR, CEditingBtnsPanel::OnSetBackground)
   EVT_HYPERLINK(ID_RESET_BG_COLOR, CEditingBtnsPanel::OnResetBackground)
   EVT_SEARCHCTRL_SEARCH_BTN( ID_FIND_BUTTON, CEditingBtnsPanel::OnFindButton )
   EVT_SEARCHCTRL_CANCEL_BTN( ID_FIND_BUTTON, CEditingBtnsPanel::OnCancelFindButton )
   EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK_PANEL, CEditingBtnsPanel::OnNotebookTabChanged)
   EVT_CHAR_HOOK(CEditingBtnsPanel::OnChar)

////@begin CEditingBtnsPanel event table entries
    EVT_BUTTON( ID_BUTTON1, CEditingBtnsPanel::OnReverseAuthorNamesClick )
    EVT_BUTTON( ID_BUTTON2, CEditingBtnsPanel::EditPubs )
    EVT_BUTTON( ID_BUTTON3, CEditingBtnsPanel::ParseLocalIdToSrc )
    EVT_BUTTON( ID_BUTTON4, CEditingBtnsPanel::OnFixCapitalizationAll )
    EVT_BUTTON( ID_BUTTON5, CEditingBtnsPanel::OnFixCapitalizationAuthors )
    EVT_BUTTON( ID_BUTTON6, CEditingBtnsPanel::OnFixCapitalizationTitles )
    EVT_BUTTON( ID_BUTTON7, CEditingBtnsPanel::OnFixCapitalizationAffiliation )
    EVT_BUTTON( ID_BUTTON8, CEditingBtnsPanel::SortUniqueCount )
    EVT_BUTTON( ID_BUTTON9, CEditingBtnsPanel::OnRemoveUnpublishedPublications )
    EVT_BUTTON( ID_BUTTON10, CEditingBtnsPanel::TableReaderFromClipboard )
    EVT_BUTTON( ID_BUTTON11, CEditingBtnsPanel::TaxFixCleanup )
    EVT_BUTTON( ID_BUTTON12, CEditingBtnsPanel::RemoveAllFeatures )
    EVT_BUTTON( ID_BUTTON13, CEditingBtnsPanel::OnStripAuthorSuffixes )
    EVT_BUTTON( ID_BUTTON14, CEditingBtnsPanel::OnRemoveAuthorConsortiums )
    EVT_BUTTON( ID_BUTTON15, CEditingBtnsPanel::CreateFeature )
    EVT_BUTTON( ID_BUTTON16, CEditingBtnsPanel::TrimNsRich )
    EVT_BUTTON( ID_BUTTON17, CEditingBtnsPanel::AnnotateDescriptorComment )
    EVT_BUTTON( ID_BUTTON18, CEditingBtnsPanel::MrnaCds )
    EVT_BUTTON( ID_BUTTON19, CEditingBtnsPanel::OnAddCitSubForUpdate )
    EVT_BUTTON( ID_BUTTON20, CEditingBtnsPanel::OnRemoveSequences )
    EVT_BUTTON( ID_BUTTON21, CEditingBtnsPanel::OnRemoveGenomeProjectsDB )
    EVT_BUTTON( ID_BUTTON22, CEditingBtnsPanel::RemProtTitles )
    EVT_BUTTON( ID_BUTTON23, CEditingBtnsPanel::RmCultureNotes )
    EVT_BUTTON( ID_BUTTON24, CEditingBtnsPanel::RevComp16S )
    EVT_BUTTON( ID_BUTTON25, CEditingBtnsPanel::OnRemoveDescriptors )
    EVT_BUTTON( ID_BUTTON26, CEditingBtnsPanel::StrainToSp )
    EVT_BUTTON( ID_BUTTON27, CEditingBtnsPanel::OnRemoveAllStructuredComments )
    EVT_BUTTON( ID_BUTTON28, CEditingBtnsPanel::BulkEditCDS )
    EVT_BUTTON( ID_BUTTON29, CEditingBtnsPanel::AddTranslExcept )
    EVT_BUTTON( ID_BUTTON30, CEditingBtnsPanel::OnRemoveFeatures )
    EVT_BUTTON( ID_BUTTON31, CEditingBtnsPanel::BulkGeneEdit )
    EVT_BUTTON( ID_BUTTON32, CEditingBtnsPanel::SegregateSets )
    EVT_BUTTON( ID_BUTTON33, CEditingBtnsPanel::AddSet )
    EVT_BUTTON( ID_BUTTON34, CEditingBtnsPanel::OnAddStructuredCommentKeyword )
    EVT_BUTTON( ID_BUTTON36, CEditingBtnsPanel::ClickableSortUniqueCount )
    EVT_BUTTON( ID_BUTTON37, CEditingBtnsPanel::PT_Cleanup )
    EVT_BUTTON( ID_BUTTON38, CEditingBtnsPanel::OnCallerTool )
    EVT_BUTTON( ID_BUTTON39, CEditingBtnsPanel::SelectTarget )
    EVT_BUTTON( ID_BUTTON40, CEditingBtnsPanel::LabelrRNAs )
    EVT_BUTTON( ID_BUTTON41, CEditingBtnsPanel::RetranslateCDS )
    EVT_BUTTON( ID_BUTTON42, CEditingBtnsPanel::RetranslateCDS_NoStp )
    EVT_BUTTON( ID_BUTTON43, CEditingBtnsPanel::BulkEditRNA )
    EVT_BUTTON( ID_BUTTON44, CEditingBtnsPanel::RunMacro)
    EVT_BUTTON( ID_BUTTON48, CEditingBtnsPanel::UnculTaxTool )
    EVT_BUTTON( ID_BUTTON49, CEditingBtnsPanel::OnAppendModToOrg )
    EVT_BUTTON( ID_BUTTON50, CEditingBtnsPanel::RemoveSrcNotes )
    EVT_BUTTON( ID_BUTTON51, CEditingBtnsPanel::AddIsolSource )
    EVT_BUTTON( ID_BUTTON52, CEditingBtnsPanel::ExtendPartialsConstr )
    EVT_BUTTON( ID_BUTTON53, CEditingBtnsPanel::ExtendPartialsAll )
    EVT_BUTTON( ID_BUTTON54, CEditingBtnsPanel::OnAddrRNA16S )
    EVT_BUTTON( ID_BUTTON55, CEditingBtnsPanel::OnAddrRNA18S )
    EVT_BUTTON( ID_BUTTON56, CEditingBtnsPanel::OnAddrRNA23S )
    EVT_BUTTON( ID_BUTTON57, CEditingBtnsPanel::OnRemoveDefLines )
    EVT_BUTTON( ID_BUTTON58, CEditingBtnsPanel::ConvertCdsToMiscFeat )
    EVT_BUTTON( ID_BUTTON59, CEditingBtnsPanel::AddCDS )
    EVT_BUTTON( ID_BUTTON60, CEditingBtnsPanel::AddRNA )
    EVT_BUTTON( ID_BUTTON61, CEditingBtnsPanel::AddOtherFeature )
    EVT_BUTTON( ID_BUTTON62, CEditingBtnsPanel::ValidateSeq )
    EVT_BUTTON( ID_BUTTON63, CEditingBtnsPanel::LowercaseQuals )
    EVT_BUTTON( ID_BUTTON64, CEditingBtnsPanel::DiscrepancyReport )
    EVT_BUTTON( ID_BUTTON65, CEditingBtnsPanel::ApplySourceQual )
    EVT_BUTTON( ID_BUTTON66, CEditingBtnsPanel::EditSourceQual )
    EVT_BUTTON( ID_BUTTON67, CEditingBtnsPanel::RemoveTextOutsideString )
    EVT_BUTTON( ID_BUTTON68, CEditingBtnsPanel::CorrectGenes )
    EVT_BUTTON( ID_BUTTON69, CEditingBtnsPanel::CreateDescGenomeAssemblyComment )
    EVT_BUTTON( ID_BUTTON70, CEditingBtnsPanel::ECNumberCleanup )
    EVT_BUTTON( ID_BUTTON71, CEditingBtnsPanel::EditRNAQual )
    EVT_BUTTON( ID_BUTTON72, CEditingBtnsPanel::Add_rRNA )
    EVT_BUTTON( ID_BUTTON73, CEditingBtnsPanel::Add_misc_feat )
    EVT_BUTTON( ID_BUTTON74, CEditingBtnsPanel::Add_gene )
    EVT_BUTTON( ID_BUTTON75, CEditingBtnsPanel::Save )
    EVT_BUTTON( ID_BUTTON76, CEditingBtnsPanel::EditCGPQual )
    EVT_BUTTON( ID_BUTTON77, CEditingBtnsPanel::ExportTable )
    EVT_BUTTON( ID_BUTTON79, CEditingBtnsPanel::SequesterSets )
    EVT_BUTTON( ID_BUTTON80, CEditingBtnsPanel::AddGSet )
    EVT_BUTTON( ID_BUTTON81, CEditingBtnsPanel::EditFeatQual )
    EVT_BUTTON( ID_BUTTON82, CEditingBtnsPanel::EditSubmitBlock )
    EVT_BUTTON( ID_BUTTON83, CEditingBtnsPanel::rRNAtoDNA )
    EVT_BUTTON( ID_BUTTON84, CEditingBtnsPanel::IllegalQualsToNote )
    EVT_BUTTON( ID_BUTTON85, CEditingBtnsPanel::RmIllegalQuals )
    EVT_BUTTON( ID_BUTTON86, CEditingBtnsPanel::ParseText )
    EVT_BUTTON( ID_BUTTON87, CEditingBtnsPanel::WrongQualsToNote )
    EVT_BUTTON( ID_BUTTON88, CEditingBtnsPanel::ParseTextFromDefline )
    EVT_BUTTON( ID_BUTTON89, CEditingBtnsPanel::EditFeatureLocation )
    EVT_BUTTON( ID_BUTTON90, CEditingBtnsPanel::TableReader )
    EVT_BUTTON( ID_BUTTON91, CEditingBtnsPanel::BulkEdit )
    EVT_BUTTON( ID_BUTTON92, CEditingBtnsPanel::MolInfoEdit )
    EVT_BUTTON( ID_BUTTON93, CEditingBtnsPanel::AutodefDefaultOptions )
    EVT_BUTTON( ID_BUTTON94, CEditingBtnsPanel::AutodefOptions )
    EVT_BUTTON( ID_BUTTON95, CEditingBtnsPanel::AutodefMisc )
    EVT_BUTTON( ID_BUTTON96, CEditingBtnsPanel::AutodefId )
    EVT_BUTTON( ID_BUTTON97, CEditingBtnsPanel::RmWrongQuals )
    EVT_BUTTON( ID_BUTTON98, CEditingBtnsPanel::GroupExplode )
    EVT_BUTTON( ID_BUTTON99, CEditingBtnsPanel::RemoveTextInsideStr )
    EVT_BUTTON( ID_BUTTON100, CEditingBtnsPanel::FindASN1 )
    EVT_BUTTON( ID_BUTTON101, CEditingBtnsPanel::RemoveDupFeats )
    EVT_BUTTON( ID_BUTTON102, CEditingBtnsPanel::RmWrongOrIllegalQuals )
    EVT_BUTTON( ID_BUTTON103, CEditingBtnsPanel::EditStructField )
    EVT_BUTTON( ID_BUTTON110, CEditingBtnsPanel::UpdateSingleSeqClipboard)
    EVT_BUTTON( ID_BUTTON113, CEditingBtnsPanel::RemoveUnverified )
    EVT_BUTTON( ID_BUTTON125, CEditingBtnsPanel::ShowFeatureTable )
    EVT_BUTTON( ID_BUTTON127, CEditingBtnsPanel::UpdateMultSeqClipboard)
    EVT_BUTTON( ID_BUTTON132, CEditingBtnsPanel::RunMacroWithReport)
    EVT_BUTTON( ID_BUTTON133, CEditingBtnsPanel::AutodefRefresh)
    EVT_BUTTON( ID_BUTTON134, CEditingBtnsPanel::OnMegaReport)
    EVT_BUTTON( ID_BUTTON137, CEditingBtnsPanel::RunMacroWithReport)
    EVT_BUTTON( ID_BUTTON138, CEditingBtnsPanel::ImportFeatureTable)
    EVT_BUTTON( ID_BUTTON140, CEditingBtnsPanel::LaunchDesktop)
    EVT_BUTTON( ID_BUTTON142, CEditingBtnsPanel::TaxFixCleanup)
    EVT_BUTTON( ID_BUTTON149, CEditingBtnsPanel::BarcodeTool)
    EVT_BUTTON( ID_BUTTON150, CEditingBtnsPanel::TrimNsTerminal )
    EVT_BUTTON( ID_BUTTON156, CEditingBtnsPanel::EditSequence)
    EVT_BUTTON( ID_BUTTON159, CEditingBtnsPanel::MacroEditor)
    EVT_BUTTON( ID_BUTTON160, CEditingBtnsPanel::ImportFeatureTableClipboard)
    EVT_BUTTON( ID_BUTTON171, CEditingBtnsPanel::RunMacroWithReport)
    EVT_BUTTON( ID_BUTTON172, CEditingBtnsPanel::SaveASN1File)
    EVT_BUTTON( ID_BUTTON173, CEditingBtnsPanel::ApplyDbxrefs)
////@end CEditingBtnsPanel event table entries

END_EVENT_TABLE()


/*!
 * CEditingBtnsPanel constructors
 */

CEditingBtnsPanel *CEditingBtnsPanel::m_Instance = NULL;

void CEditingBtnsPanel::GetInstance( wxWindow* parent, IWorkbench* workbench, bool autostart, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    if (!m_Instance)
        m_Instance = new CEditingBtnsPanel(parent, workbench, id, pos, size, style); 
    // Instantiated on first use. 
    if (autostart && !m_Instance->ShouldAutoStart())
    {
        m_Instance->Destroy();
        m_Instance = NULL;
        return;
    }
    m_Instance->Show(true);    
    m_Instance->SetFocus();     
}


CEditingBtnsPanel::CEditingBtnsPanel()
{
    Init();
}

CEditingBtnsPanel::CEditingBtnsPanel( wxWindow* parent, IWorkbench* workbench, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : m_Workbench(workbench)
{
    Init();
    Create(parent, id, pos, size, style);
    SetRegistryPath("Workbench.Services.Views.Generic.Sequin Buttons");
    LoadSettings();
    SetFrameAndPosition();
    PlaceButtons();
    SetPanelsBackgroundColour();
    string instance = CSysPath::GetInst();
    if (!instance.empty())
    {
        wxString title = GetTitle();
        title << " (" << instance << ")";
        SetTitle(title);
    }
}


/*!
 * CEditingBtnsPanel creator
 */

bool CEditingBtnsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditingBtnsPanel creation
    wxFrame::Create( parent, id, _("Editing Buttons"), pos, size, style );
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditingBtnsPanel creation

    return true;
}


/*!
 * CEditingBtnsPanel destructor
 */

CEditingBtnsPanel::~CEditingBtnsPanel()
{
////@begin CEditingBtnsPanel destruction
////@end CEditingBtnsPanel destruction
    SaveSettings();
    m_Instance = NULL;
}


/*!
 * Member initialisation
 */

void CEditingBtnsPanel::Init()
{
////@begin CEditingBtnsPanel member initialisation
////@end CEditingBtnsPanel member initialisation
    m_CheckBox = NULL;
    m_AutoStart = NULL;
    m_Sizer1 = NULL;
    m_Sizer2 = NULL;
    m_Sizer3 = NULL;
    m_SizerRemoved = NULL;
    m_resetTabsButton = NULL;
    m_importTabsButton = NULL;
    m_exportTabsButton = NULL;
    m_AddMacroButton = NULL;
    m_findButton = NULL;
    m_Notebook = NULL;

    m_FrameSize = wxSize(600,400);
    m_FramePosition = wxPoint(5,5);

    m_bg_color_set = false;
    m_panel1 = NULL;
    m_panel2 = NULL;
    m_panel3 = NULL;
    m_panel_removed = NULL;
}

void CEditingBtnsPanel::SetPanelsBackgroundColour()
{   
/*    unsigned long hash = wxStringHash::stringHash(instance.c_str());
    wxColour old = *wxGREEN; //notebook->GetBackgroundColour();
    wxImage::HSVValue hsv = wxImage::RGBtoHSV(wxImage::RGBValue(old.Red(), old.Green(), old.Blue()));
    hsv.hue = double(hash % 360) / 360;
    wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
    wxColour upd(rgb.red, rgb.green, rgb.blue);
*/
    if (m_bg_color_set)
    {
        CTextPanel::SetTextBackground(m_bg_color);
        SetTextViewBackgroundColour(m_bg_color);
        m_Notebook->SetBackgroundColour(m_bg_color);
        m_panel1->SetBackgroundColour(m_bg_color);
        m_panel2->SetBackgroundColour(m_bg_color);
        m_panel3->SetBackgroundColour(m_bg_color);
        m_panel_removed->SetBackgroundColour(m_bg_color);
    }
    else
    {
        CTextPanel::SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        SetTextViewBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        wxColour bg_color = wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK); 
        m_Notebook->SetBackgroundColour(bg_color);
        m_panel1->SetBackgroundColour(bg_color);
        m_panel2->SetBackgroundColour(bg_color);
        m_panel3->SetBackgroundColour(bg_color);
        m_panel_removed->SetBackgroundColour(bg_color);
    }
    Refresh();
}

void CEditingBtnsPanel::SetTextViewBackgroundColour(const wxColour &color)
{
    if (m_Workbench)
    {
        
        IViewManagerService* view_srv = m_Workbench->GetServiceByType<IViewManagerService>();
        if (view_srv) 
        {
            
            IViewManagerService::TViews views;
            view_srv->GetViews(views);
            NON_CONST_ITERATE(IViewManagerService::TViews, it, views) 
            {
                IProjectView* project_view = dynamic_cast<IProjectView*>((*it).GetPointer());
                if (project_view) // && project_view->GetLabel(IProjectView::eType) == "Text View") 
                {

                    CTextPanel *text_panel = dynamic_cast<CTextPanel*>(project_view->GetWindow());
                    if (text_panel)
                    {
                        text_panel->GetWidget()->SetBackgroundColour(color);                        
                        text_panel->GetWidget()->Refresh();
                    }
                }
            }
        }
    }
}

void CEditingBtnsPanel::OnSetBackground(wxHyperlinkEvent& event)
{
    wxColourDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) 
    {
        m_bg_color = dlg.GetColourData().GetColour();
        m_bg_color_set = true;
        SetPanelsBackgroundColour();
        SaveSettings();
    }
}

void CEditingBtnsPanel::OnResetBackground(wxHyperlinkEvent& event)
{
    m_bg_color_set = false;
    SetPanelsBackgroundColour();
    SaveSettings();
}

/*!
 * Control creation for CEditingBtnsPanel
 */

void CEditingBtnsPanel::CreateControls()
{    
////@begin CEditingBtnsPanel content construction
    // Generated by DialogBlocks, 26/10/2012 15:32:03 (unregistered)

    wxPanel* itemPanel1 = new wxPanel(this, wxID_ANY,wxPoint(0,20),wxSize(600,380));

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    m_Notebook = new wxNotebook(itemPanel1, ID_NOTEBOOK_PANEL,wxDefaultPosition,wxSize(600,280));
    itemBoxSizer1->Add(m_Notebook, 2, wxALL|wxGROW, 0);

    m_panel1 = new CButtonPanel(m_Notebook,ID_TAB1,wxDefaultPosition,wxSize(600,260));
    m_panel1->SetVirtualSize(-1, 1500);
    m_panel1->SetScrollRate(0, 30);
    m_Notebook->AddPage(m_panel1,_("A"));

    m_panel2 = new CButtonPanel(m_Notebook,ID_TAB2,wxDefaultPosition,wxSize(600,260));
    m_panel2->SetVirtualSize(-1, 1500);
    m_panel2->SetScrollRate(0, 30);
    m_Notebook->AddPage(m_panel2,_("B"));

    m_panel3 = new CButtonPanel(m_Notebook,ID_TAB3,wxDefaultPosition,wxSize(600,260));
    m_panel3->SetVirtualSize(-1, 1500);
    m_panel3->SetScrollRate(0, 30);
    m_Notebook->AddPage(m_panel3,_("C"));

    m_panel_removed = new CButtonPanel(m_Notebook,ID_TAB_REMOVED,wxDefaultPosition,wxSize(600,260));
    m_panel_removed->SetVirtualSize(-1, 1500);
    m_panel_removed->SetScrollRate(0, 30);
    m_Notebook->AddPage(m_panel_removed,_("All"));

    m_Sizer1 = new wxBoxSizer(wxVERTICAL);
    m_panel1->SetSizer(m_Sizer1);

    m_Sizer2 = new wxBoxSizer(wxVERTICAL);
    m_panel2->SetSizer(m_Sizer2);

    m_Sizer3 = new wxBoxSizer(wxVERTICAL);
    m_panel3->SetSizer(m_Sizer3);

    m_SizerRemoved = new wxBoxSizer(wxVERTICAL);
    m_panel_removed->SetSizer(m_SizerRemoved); 

    CMovableButton* itemButton1 = new CMovableButton( m_panel_removed, ID_BUTTON1, _("Rev Author Names"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton1, 0, wxALL, 5);
    CMovableButton* itemButton2 = new CMovableButton( m_panel_removed, ID_BUTTON2, _("Edit Pubs"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton2, 0, wxALL, 5);
    CMovableButton* itemButton3 = new CMovableButton( m_panel_removed, ID_BUTTON3, _("Local ID->Src"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton3, 0, wxALL, 5);
    CMovableButton* itemButton4 = new CMovableButton( m_panel_removed, ID_BUTTON4, _("Fix Caps"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton4, 0, wxALL, 5);
    CMovableButton* itemButton5 = new CMovableButton( m_panel_removed, ID_BUTTON5, _("Fix Caps Authors"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton5, 0, wxALL, 5);
    CMovableButton* itemButton6 = new CMovableButton( m_panel_removed, ID_BUTTON6, _("Fix Caps Titles"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton6, 0, wxALL, 5);
    CMovableButton* itemButton7 = new CMovableButton( m_panel_removed, ID_BUTTON7, _("Fix Caps Affil"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton7, 0, wxALL, 5);
    CMovableButton* itemButton8 = new CMovableButton( m_panel_removed, ID_BUTTON8, _("SUC"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton8, 0, wxALL, 5);
    CMovableButton* itemButton9 = new CMovableButton( m_panel_removed, ID_BUTTON9, _("Remove Unpub"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton9, 0, wxALL, 5);
    CMovableButton* itemButton10 = new CMovableButton( m_panel_removed, ID_BUTTON10, _("Table Reader Clip"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton10, 0, wxALL, 5);
    CMovableButton* itemButton11 = new CMovableButton( m_panel_removed, ID_BUTTON11, _("Tax_fix/Clean_up"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton11, 0, wxALL, 5);
    CMovableButton* itemButton12 = new CMovableButton( m_panel_removed, ID_BUTTON12, _("Remove All Features"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton12, 0, wxALL, 5);
    CMovableButton* itemButton13 = new CMovableButton( m_panel_removed, ID_BUTTON13, _("Strip Suffix"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton13, 0, wxALL, 5);
    CMovableButton* itemButton14 = new CMovableButton( m_panel_removed, ID_BUTTON14, _("Rm Consortium"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton14, 0, wxALL, 5);
    CMovableButton* itemButton16 = new CMovableButton( m_panel_removed, ID_BUTTON16, _("Trim N-rich"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton16, 0, wxALL, 5);
    CMovableButton* itemButton17 = new CMovableButton( m_panel_removed, ID_BUTTON17, _("Add Comment"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton17, 0, wxALL, 5);
    CMovableButton* itemButton18 = new CMovableButton( m_panel_removed, ID_BUTTON18, _("mrna_CDS"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton18, 0, wxALL, 5);
    CMovableButton* itemButton19 = new CMovableButton( m_panel_removed, ID_BUTTON19, _("cit-sub-upd"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton19, 0, wxALL, 5);
    CMovableButton* itemButton20 = new CMovableButton( m_panel_removed, ID_BUTTON20, _("Remove Seqs"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton20, 0, wxALL, 5);
    CMovableButton* itemButton21 = new CMovableButton( m_panel_removed, ID_BUTTON21, _("Remove GPID"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton21, 0, wxALL, 5);
    CMovableButton* itemButton22 = new CMovableButton( m_panel_removed, ID_BUTTON22, _("rem_prot_titles"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton22, 0, wxALL, 5);
    CMovableButton* itemButton23 = new CMovableButton( m_panel_removed, ID_BUTTON23, _("Rm Culture Notes"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton23, 0, wxALL, 5);
    CMovableButton* itemButton24 = new CMovableButton( m_panel_removed, ID_BUTTON24, _("Revcomp 16S"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton24, 0, wxALL, 5);
    CMovableButton* itemButton25 = new CMovableButton( m_panel_removed, ID_BUTTON25, _("Remove Descriptors"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); // not found
    m_SizerRemoved->Add(itemButton25, 0, wxALL, 5);
    CMovableButton* itemButton26 = new CMovableButton( m_panel_removed, ID_BUTTON26, _("Strain to sp."), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton26, 0, wxALL, 5);
    CMovableButton* itemButton27 = new CMovableButton( m_panel_removed, ID_BUTTON27, _("RemoveStructCmt"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton27, 0, wxALL, 5);
    CMovableButton* itemButton28 = new CMovableButton( m_panel_removed, ID_BUTTON28, _("Bulk Edit CDS"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton28, 0, wxALL, 5);
    CMovableButton* itemButton29 = new CMovableButton( m_panel_removed, ID_BUTTON29, _("Add Tsl Excepts Cmt"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton29, 0, wxALL, 5);
    CMovableButton* itemButton30 = new CMovableButton( m_panel_removed, ID_BUTTON30, _("RemoveSelectFeats"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton30, 0, wxALL, 5);
    CMovableButton* itemButton31 = new CMovableButton( m_panel_removed, ID_BUTTON31, _("Bulk Edit Gene"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton31, 0, wxALL, 5);
    CMovableButton* itemButton32 = new CMovableButton( m_panel_removed, ID_BUTTON32, _("Segregate"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton32, 0, wxALL, 5);
    CMovableButton* itemButton33 = new CMovableButton( m_panel_removed, ID_BUTTON33, _("Add Set"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton33, 0, wxALL, 5);
    CMovableButton* itemButton34 = new CMovableButton( m_panel_removed, ID_BUTTON34, _("StructKeyword"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton34, 0, wxALL, 5);
    CMovableButton* itemButton35 = new CMovableButton( m_panel_removed, ID_BUTTON35, _("Reorder Strct Cmt"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton35, 0, wxALL, 5);
    CMovableButton* itemButton36 = new CMovableButton( m_panel_removed, ID_BUTTON36, _("cSUC"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton36, 0, wxALL, 5);
    CMovableButton* itemButton37 = new CMovableButton( m_panel_removed, ID_BUTTON37, _("PT Cleanup"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton37, 0, wxALL, 5);
    CMovableButton* itemButton38 = new CMovableButton( m_panel_removed, ID_BUTTON38, _("On-Caller Tool"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton38, 0, wxALL, 5);
    CMovableButton* itemButton39 = new CMovableButton( m_panel_removed, ID_BUTTON39, _("Select Target"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton39, 0, wxALL, 5);
    CMovableButton* itemButton40 = new CMovableButton( m_panel_removed, ID_BUTTON40, _("Label rRNAs"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton40, 0, wxALL, 5);
    CMovableButton* itemButton41 = new CMovableButton( m_panel_removed, ID_BUTTON41, _("Retranslate CDS"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton41, 0, wxALL, 5);
    CMovableButton* itemButton42 = new CMovableButton( m_panel_removed, ID_BUTTON42, _("RetransCDS-NoStp"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton42, 0, wxALL, 5);
    CMovableButton* itemButton43 = new CMovableButton( m_panel_removed, ID_BUTTON43, _("Bulk Edit RNA"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton43, 0, wxALL, 5);
    CMovableButton* itemButton44 = new CMovableButton( m_panel_removed, ID_BUTTON44, _("Autofix"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton44, 0, wxALL, 5);
    CMovableButton* itemButton48 = new CMovableButton( m_panel_removed, ID_BUTTON48, _("Uncul TaxTool"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton48, 0, wxALL, 5);
    CMovableButton* itemButton49 = new CMovableButton( m_panel_removed, ID_BUTTON49, _("Append to Organism"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton49, 0, wxALL, 5);
    CMovableButton* itemButton50 = new CMovableButton( m_panel_removed, ID_BUTTON50, _("Remove SrcNotes"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton50, 0, wxALL, 5);
    CMovableButton* itemButton51 = new CMovableButton( m_panel_removed, ID_BUTTON51, _("Add Isol Source"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton51, 0, wxALL, 5);
    CMovableButton* itemButton52 = new CMovableButton( m_panel_removed, ID_BUTTON52, _("ExtPartialsConstraint"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton52, 0, wxALL, 5);
    CMovableButton* itemButton53 = new CMovableButton( m_panel_removed, ID_BUTTON53, _("Extend Partials Ends"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton53, 0, wxALL, 5);
    CMovableButton* itemButton54 = new CMovableButton( m_panel_removed, ID_BUTTON54, _("Add 16S"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton54, 0, wxALL, 5);
    CMovableButton* itemButton55 = new CMovableButton( m_panel_removed, ID_BUTTON55, _("Add 18S"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton55, 0, wxALL, 5);
    CMovableButton* itemButton56 = new CMovableButton( m_panel_removed, ID_BUTTON56, _("Add 23S"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton56, 0, wxALL, 5);
    CMovableButton* itemButton57 = new CMovableButton( m_panel_removed, ID_BUTTON57, _("Remove DefLines"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton57, 0, wxALL, 5);
    CMovableButton* itemButton58 = new CMovableButton( m_panel_removed, ID_BUTTON58, _("Convert CDS to MiscFeat"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton58, 0, wxALL, 5);
    CMovableButton* itemButton59 = new CMovableButton( m_panel_removed, ID_BUTTON59, _("Add CDS"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton59, 0, wxALL, 5);
    CMovableButton* itemButton60 = new CMovableButton( m_panel_removed, ID_BUTTON60, _("Add RNA"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton60, 0, wxALL, 5);
    CMovableButton* itemButton61 = new CMovableButton( m_panel_removed, ID_BUTTON61, _("Add Other Feature"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton61, 0, wxALL, 5);
    CMovableButton* itemButton62 = new CMovableButton( m_panel_removed, ID_BUTTON62, _("Validate"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton62, 0, wxALL, 5);
    CMovableButton* itemButton63 = new CMovableButton( m_panel_removed, ID_BUTTON63, _("LC Quals"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton63, 0, wxALL, 5);
    CMovableButton* itemButton64 = new CMovableButton( m_panel_removed, ID_BUTTON64, _("Discrepancy Report"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton64, 0, wxALL, 5);
    CMovableButton* itemButton65 = new CMovableButton( m_panel_removed, ID_BUTTON65, _("Apply Source Qual"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton65, 0, wxALL, 5);
    CMovableButton* itemButton66 = new CMovableButton( m_panel_removed, ID_BUTTON66, _("Edit Source Qual"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton66, 0, wxALL, 5);
    CMovableButton* itemButton67 = new CMovableButton( m_panel_removed, ID_BUTTON67, _("Rm Text Out String"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton67, 0, wxALL, 5);
    CMovableButton* itemButton68 = new CMovableButton( m_panel_removed, ID_BUTTON68, _("Correct Gene for CDS"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton68, 0, wxALL, 5);
    CMovableButton* itemButton69 = new CMovableButton( m_panel_removed, ID_BUTTON69, _("Add Gnm Asmb Cmt"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton69, 0, wxALL, 5);
    CMovableButton* itemButton70 = new CMovableButton( m_panel_removed, ID_BUTTON70, _("EC Number Cleanup"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton70, 0, wxALL, 5);
    CMovableButton* itemButton71 = new CMovableButton( m_panel_removed, ID_BUTTON71, _("Edit RNA Qual"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton71, 0, wxALL, 5);
    CMovableButton* itemButton72 = new CMovableButton( m_panel_removed, ID_BUTTON72, _("rRNA"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton72, 0, wxALL, 5);
    CMovableButton* itemButton73 = new CMovableButton( m_panel_removed, ID_BUTTON73, _("misc_feat"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton73, 0, wxALL, 5);
    CMovableButton* itemButton74 = new CMovableButton( m_panel_removed, ID_BUTTON74, _("gene"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton74, 0, wxALL, 5);
    CMovableButton* itemButton75 = new CMovableButton( m_panel_removed, ID_BUTTON75, _("Save"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton75, 0, wxALL, 5);
    CMovableButton* itemButton76 = new CMovableButton( m_panel_removed, ID_BUTTON76, _("Edit CDS qual"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton76, 0, wxALL, 5);
    CMovableButton* itemButton77 = new CMovableButton( m_panel_removed, ID_BUTTON77, _("Export Table"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton77, 0, wxALL, 5);
    CMovableButton* itemButton79 = new CMovableButton( m_panel_removed, ID_BUTTON79, _("Sequester"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton79, 0, wxALL, 5);
    CMovableButton* itemButton80 = new CMovableButton( m_panel_removed, ID_BUTTON80, _("Add G Set"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton80, 0, wxALL, 5);
    CMovableButton* itemButton81 = new CMovableButton( m_panel_removed, ID_BUTTON81, _("Edit Feat Qual"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton81, 0, wxALL, 5);
    CMovableButton* itemButton82 = new CMovableButton( m_panel_removed, ID_BUTTON82, _("sub_affil"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton82, 0, wxALL, 5);
    CMovableButton* itemButton83 = new CMovableButton( m_panel_removed, ID_BUTTON83, _("rRNA->DNA"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton83, 0, wxALL, 5);
    CMovableButton* itemButton84 = new CMovableButton( m_panel_removed, ID_BUTTON84, _("IllegalQuals to Note"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton84, 0, wxALL, 5);
    CMovableButton* itemButton85 = new CMovableButton( m_panel_removed, ID_BUTTON85, _("Remove Illegal Quals"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton85, 0, wxALL, 5);
    CMovableButton* itemButton86 = new CMovableButton( m_panel_removed, ID_BUTTON86, _("Parse Text"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton86, 0, wxALL, 5);
    CMovableButton* itemButton87 = new CMovableButton( m_panel_removed, ID_BUTTON87, _("WrongQuals to Note"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton87, 0, wxALL, 5);
    CMovableButton* itemButton88 = new CMovableButton( m_panel_removed, ID_BUTTON88, _("Parse Defline"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton88, 0, wxALL, 5);
    CMovableButton* itemButton89 = new CMovableButton( m_panel_removed, ID_BUTTON89, _("Edit Location"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton89, 0, wxALL, 5);
    CMovableButton* itemButton90 = new CMovableButton( m_panel_removed, ID_BUTTON90, _("Table Reader"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton90, 0, wxALL, 5);
    CMovableButton* itemButton91 = new CMovableButton( m_panel_removed, ID_BUTTON91, _("Bulk Edit Source"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton91, 0, wxALL, 5);
    CMovableButton* itemButton92 = new CMovableButton( m_panel_removed, ID_BUTTON92, _("Edit Mol Info"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton92, 0, wxALL, 5);
    CMovableButton* itemButton93 = new CMovableButton( m_panel_removed, ID_BUTTON93,_("Auto_Def"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton93, 0, wxALL, 5);
    CMovableButton* itemButton94 = new CMovableButton( m_panel_removed, ID_BUTTON94, _("Auto_Def Options"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton94, 0, wxALL, 5);
    CMovableButton* itemButton95 = new CMovableButton( m_panel_removed, ID_BUTTON95,_("Ad_misc"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton95, 0, wxALL, 5);
    CMovableButton* itemButton96 = new CMovableButton( m_panel_removed, ID_BUTTON96, _("Ad_ID"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton96, 0, wxALL, 5);
    CMovableButton* itemButton97 = new CMovableButton( m_panel_removed, ID_BUTTON97, _("Remove Wrong Quals"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton97, 0, wxALL, 5);
    CMovableButton* itemButton98 = new CMovableButton( m_panel_removed, ID_BUTTON98, _("Group Explode"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton98, 0, wxALL, 5);
    CMovableButton* itemButton99 = new CMovableButton( m_panel_removed, ID_BUTTON99, _("Rm Text In String"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton99, 0, wxALL, 5);
    CMovableButton* itemButton100 = new CMovableButton( m_panel_removed, ID_BUTTON100, _("Find ASN.1"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton100, 0, wxALL, 5);
    CMovableButton* itemButton101 = new CMovableButton( m_panel_removed, ID_BUTTON101, _("Remove Dup Feats"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton101, 0, wxALL, 5);
    CMovableButton* itemButton102 = new CMovableButton( m_panel_removed, ID_BUTTON102, _("Rm WrongIllegal Qual"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton102, 0, wxALL, 5);
    CMovableButton* itemButton103 = new CMovableButton( m_panel_removed, ID_BUTTON103, _("Edit Struct Field"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton103, 0, wxALL, 5);
    CMovableButton* itemButton110 = new CMovableButton( m_panel_removed, ID_BUTTON110, _("Update SeqClip"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton110, 0, wxALL, 5);
    CMovableButton* itemButton113 = new CMovableButton( m_panel_removed, ID_BUTTON113, _("Remove Unverified"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton113, 0, wxALL, 5);
    CMovableButton* itemButton125 = new CMovableButton(m_panel_removed, ID_BUTTON125, _("Feature Table"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton125, 0,  wxALL, 5);
    CMovableButton* itemButton127 = new CMovableButton(m_panel_removed, ID_BUTTON127, _("Update MultSeqClip"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton127, 0,  wxALL, 5);
    CMovableButton* itemButton132 = new CMovableButton(m_panel_removed, ID_BUTTON132, _("*Autofix WGS"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton132, 0,  wxALL, 5);
    CMovableButton* itemButton133 = new CMovableButton(m_panel_removed, ID_BUTTON133, _("Auto_Def Refresh"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton133, 0,  wxALL, 5);
    CMovableButton* itemButton134 = new CMovableButton( m_panel_removed, ID_BUTTON134, _("Mega Report"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton134, 0, wxALL, 5);
    CMovableButton* itemButton137 = new CMovableButton(m_panel_removed, ID_BUTTON137, _("Autofix-R"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton137, 0,  wxALL, 5);
    CMovableButton* itemButton138 = new CMovableButton(m_panel_removed, ID_BUTTON138, _("Import Feature Table"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton138, 0,  wxALL, 5);
    CMovableButton* itemButton140 = new CMovableButton(m_panel_removed, ID_BUTTON140, _("Desktop"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton140, 0,  wxALL, 5);
    CMovableButton* itemButton142 = new CMovableButton(m_panel_removed, ID_BUTTON142, _("Cleanup"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton142, 0, wxALL, 5);
    CMovableButton* itemButton149 = new CMovableButton(m_panel_removed, ID_BUTTON149, _("BARCODE"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton149, 0,  wxALL, 5);
    CMovableButton* itemButton150 = new CMovableButton(m_panel_removed, ID_BUTTON150, _("Trim Term Ns"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) );
    m_SizerRemoved->Add(itemButton150, 0, wxALL, 5);
    CMovableButton* itemButton156 = new CMovableButton(m_panel_removed, ID_BUTTON156, _("Edit Sequence"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton156, 0, wxALL, 5);
    CMovableButton* itemButton159 = new CMovableButton(m_panel_removed, ID_BUTTON159, _("Macro Editor"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton159, 0, wxALL, 5);
    CMovableButton* itemButton160 = new CMovableButton(m_panel_removed, ID_BUTTON160, _("Import Features (Clipboard)"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton160, 0,  wxALL, 5);
    CMovableButton* itemButton171 = new CMovableButton(m_panel_removed, ID_BUTTON171, _("*Autofix TSA"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton171, 0, wxALL, 5);
    CMovableButton* itemButton172 = new CMovableButton(m_panel_removed, ID_BUTTON172, _("Save ASN.1"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton172, 0, wxALL, 5);
    CMovableButton* itemButton173 = new CMovableButton(m_panel_removed, ID_BUTTON173, _("Apply Dbxrefs"), wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_SizerRemoved->Add(itemButton173, 0, wxALL, 5);


    CFeatureMovableButton* itemButton15 = new CFeatureMovableButton( m_panel_removed, ID_BUTTON15, CSeqFeatData::eSubtype_cdregion,_("CDS"), wxDefaultPosition, wxSize(BUTTON_WIDTH,BUTTON_HEIGHT) ); 
    m_SizerRemoved->Add(itemButton15, 0, wxALL, 5);

////@end CEditingBtnsPanel content construction

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALL|wxEXPAND, 0);   

    m_CheckBox = new wxCheckBox( itemPanel1, ID_CHECKBOX_COLLAPSE, _("Lock layout"));
    m_CheckBox->SetValue(true);
    itemBoxSizer2->Add(m_CheckBox, 0, wxALIGN_LEFT|wxALL, 5);    

    m_resetTabsButton = new wxButton( itemPanel1, ID_RESET_TABS, _("Reset Layout"), wxDefaultPosition, wxDefaultSize );
    m_resetTabsButton->Enable(false);
    itemBoxSizer2->Add(m_resetTabsButton, 0, wxALIGN_LEFT|wxALL, 5);

    m_importTabsButton = new wxButton( itemPanel1, ID_IMPORT_TABS, _("Import Layout"), wxDefaultPosition, wxDefaultSize );
    m_importTabsButton->Enable(false);
    itemBoxSizer2->Add(m_importTabsButton, 0, wxALIGN_LEFT|wxALL, 5);

    m_exportTabsButton = new wxButton( itemPanel1, ID_EXPORT_TABS, _("Export Layout"), wxDefaultPosition, wxDefaultSize );
    m_exportTabsButton->Enable(false);
    itemBoxSizer2->Add(m_exportTabsButton, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer3, 0, wxALL|wxEXPAND, 0);   

    m_AutoStart = new wxCheckBox( itemPanel1, ID_CHECKBOX_AUTOSTART, _("Auto start"));
    m_AutoStart->SetValue(false);
    itemBoxSizer3->Add(m_AutoStart, 0, wxALIGN_LEFT|wxALL, 5);

    m_findButton = new wxSearchCtrl( itemPanel1, ID_FIND_BUTTON,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);// _("Find"), wxDefaultPosition, wxDefaultSize );
    m_findButton->Enable(false);
    m_findButton->ShowSearchButton(true);
    m_findButton->ShowCancelButton(true);
    itemBoxSizer3->Add(m_findButton, 0, wxALIGN_LEFT|wxALL, 5);   

    m_AddMacroButton = new wxButton( itemPanel1, ID_ADD_MACRO, _("Add Macro"), wxDefaultPosition, wxDefaultSize );
    m_AddMacroButton->Enable(false);
    itemBoxSizer3->Add(m_AddMacroButton, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer4, 0, wxALL|wxEXPAND, 0);

    wxHyperlinkCtrl* itemHyperLink1 = new wxHyperlinkCtrl(itemPanel1, ID_SET_BG_COLOR, wxT("Set Background"), wxT(""));
    itemHyperLink1->SetVisitedColour(itemHyperLink1->GetNormalColour());
    itemBoxSizer4->Add(itemHyperLink1, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxHyperlinkCtrl* itemHyperLink2 = new wxHyperlinkCtrl(itemPanel1, ID_RESET_BG_COLOR, wxT("Reset Background"), wxT(""));
    itemHyperLink2->SetVisitedColour(itemHyperLink2->GetNormalColour());
    itemBoxSizer4->Add(itemHyperLink2, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);


    wxSizerItemList& children1 = m_SizerRemoved->GetChildren();
    for (wxSizerItemList::iterator child = children1.begin(); child != children1.end(); ++child)
    {
        wxWindow* w = (*child)->GetWindow();
        if (w)
        {
            IMovableButton* button = dynamic_cast<IMovableButton*>(w);
            if (button)
            {
                button->SetSizers(this,m_Sizer1,m_Sizer2,m_Sizer3,m_SizerRemoved);
                button->SetRemoved(true);
                m_all_buttons.push_back(button);
            }
        }
    }
    sort(m_all_buttons.begin(),m_all_buttons.end(),comp_buttons);
    x_ResetRemovedTab();
}


void  CEditingBtnsPanel::x_ResetRemovedTab()
{ 
    m_panel_removed->Scroll(0,0);
    m_SizerRemoved->Clear();
    bool first = true;
    for (unsigned int i=0; i<m_all_buttons.size(); i++)
    {
        if (m_all_buttons[i]->GetLabel()[0].GetValue() == '*' && first)
        {
            wxStaticLine *hl = new wxStaticLine (m_all_buttons[i]->GetParent(),wxID_ANY,wxDefaultPosition,wxSize(2*BUTTON_WIDTH,1),wxLI_HORIZONTAL,wxStaticLineNameStr);
            m_SizerRemoved->Add(hl, 1, wxALL, DIST_BETWEEN_BUTTONS);
            first = false;
        }
    m_all_buttons[i]->Show();
        m_SizerRemoved->Add(m_all_buttons[i], 0, wxALL, DIST_BETWEEN_BUTTONS);
    }
}


/*!
 * Transfer data to the window
 */

bool CEditingBtnsPanel::TransferDataToWindow()
{
    return wxFrame::TransferDataToWindow();
}

void CEditingBtnsPanel::x_LockButtons(wxSizer *sizer, bool locked)
{
    wxSizerItemList& children = sizer->GetChildren();
    for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
    {
        wxWindow* w = (*child)->GetWindow();
        if (w)
        {
            IMovableButton* button = dynamic_cast<IMovableButton*>(w);
            if (button)
            {
                button->SetLocked(locked);
                
            }
        }
    }
}

void CEditingBtnsPanel::OnClickLock( wxCommandEvent& event )
{
    bool locked = m_CheckBox->GetValue();
    x_LockButtons(m_Sizer1,locked);
    x_LockButtons(m_Sizer2,locked);
    x_LockButtons(m_Sizer3,locked);
    x_LockButtons(m_SizerRemoved,locked);   
    m_resetTabsButton->Enable(!locked);
    m_importTabsButton->Enable(!locked);
    m_exportTabsButton->Enable(!locked);

    if (m_Notebook->GetSelection() == m_Notebook->GetPageCount() - 1)
    {
        m_findButton->Enable(!locked);
    m_AddMacroButton->Enable(!locked);
    }
    if (locked)
        SaveSettings();
}

void CEditingBtnsPanel::OnNotebookTabChanged(wxBookCtrlEvent &event) // TODO search window placement on resize/move
{
    if (m_Notebook)
    {
        int old = event.GetOldSelection();

        if (old >=0)
        {
            wxWindow *win = m_Notebook->GetPage(old);
            CButtonPanel * panel = dynamic_cast<CButtonPanel*>(win);
            if (panel)
            {
                panel->Scroll(0,0);
            }
        }
        int current = event.GetSelection();
        if (current >= 0)
        {
            if (current == m_Notebook->GetPageCount() - 1 && m_CheckBox)
            {
                bool locked = m_CheckBox->GetValue();
        if (m_findButton)
            m_findButton->Enable(!locked);
        if (m_AddMacroButton)
            m_AddMacroButton->Enable(!locked);
            }
            else 
            {
        if ( m_findButton )
            m_findButton->Enable(false);
        if ( m_AddMacroButton )
            m_AddMacroButton->Enable(false);
            }
        }
    }
    event.Skip();
}

void CEditingBtnsPanel::OnChar(wxKeyEvent& evt)
{
    bool found = false;
    if (m_Notebook)
    {
        int sel = m_Notebook->GetSelection();
        if (sel != wxNOT_FOUND)
        {
            wxWindow *win = m_Notebook->GetPage(sel);
            CButtonPanel * panel = dynamic_cast<CButtonPanel*>(win);
            if (panel)
                found = panel->OnChar(evt);        
        }
    }
    if (!found)
        evt.Skip();
}


void CEditingBtnsPanel::OnResetTabs ( wxCommandEvent& event) 
{  
    m_MovableButtonPosition.clear();
    ResetTabs();
}

void CEditingBtnsPanel::ResetTabs()
{
    vector<wxSizer*> sizers;
    sizers.push_back(m_Sizer1);
    sizers.push_back(m_Sizer2);
    sizers.push_back(m_Sizer3);
    vector<IMovableButton*> buttons;
    for (unsigned int i=0; i<sizers.size(); i++)
    {
        wxSizer *sz = sizers[i];
        if (sz)
        {
            CButtonPanel * panel = dynamic_cast<CButtonPanel*>(sz->GetContainingWindow());
            if (panel)
            {
                panel->Scroll(0,0);
                panel->SetMovingButton(NULL,false);
            }
            wxSizerItemList& children = sz->GetChildren();
            for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
            {
                wxWindow* w = (*child)->GetWindow();
                if (w)
                {
                    IMovableButton* button = dynamic_cast<IMovableButton*>(w);
                    if (button)
                        buttons.push_back(button);
                }
            }
            
        }
    }

    for (unsigned int i=0; i<buttons.size(); i++)
        buttons[i]->DeleteButton(); 
  
    for (unsigned int i=0; i<m_all_buttons.size(); i++)
        m_all_buttons[i]->Show();
    m_findButton->Clear();
}

void CEditingBtnsPanel::OnFindButton ( wxCommandEvent& event) 
{
    wxString str = event.GetString().Lower();
    if (!str.IsEmpty())
    {
        for (unsigned int i=0; i<m_all_buttons.size(); i++)
        {
            wxString label = m_all_buttons[i]->GetLabel();
            if (label.Lower().Find(str) ==  wxNOT_FOUND)
                m_all_buttons[i]->Hide();
        }               
    }
    event.Skip();
}

void CEditingBtnsPanel::OnCancelFindButton ( wxCommandEvent& event) 
{
    for (unsigned int i=0; i<m_all_buttons.size(); i++)
        m_all_buttons[i]->Show();
    event.Skip();
}

void CEditingBtnsPanel::PlaceButtons()
{
    ResetTabs();

    for (auto btn : m_dynamic_buttons_filename)
    {
    int button_id = btn.first;
    string filename = btn.second;
    wxString button_name = wxString(m_dynamic_buttons_name[button_id]);
    bool report_required = (m_dynamic_buttons_title.find(button_id) != m_dynamic_buttons_title.end());

    CMovableButton* button = new CMovableButton(m_panel_removed, button_id, _("*") + button_name, wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    button->SetSizers(this,m_Sizer1,m_Sizer2,m_Sizer3,m_SizerRemoved);
    button->AddDynamicMenu();
    button->SetRemoved(true);
    button->SetLocked(m_CheckBox->GetValue());
    if (report_required)
    {
        Connect(button_id, wxEVT_BUTTON, wxCommandEventHandler(CEditingBtnsPanel::RunMacroWithReport), NULL, this);
    }
    else
    {
        Connect(button_id, wxEVT_BUTTON, wxCommandEventHandler(CEditingBtnsPanel::RunMacro), NULL, this);
    }	
    m_all_buttons.push_back(button);
    }
    sort(m_all_buttons.begin(),m_all_buttons.end(),comp_buttons);
    x_ResetRemovedTab();
    Layout();
    m_panel_removed->FitInside();

    map<int,IMovableButton*> id_to_win;
    wxSizerItemList& children = m_SizerRemoved->GetChildren();
    for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
    {
        wxWindow* w = (*child)->GetWindow();
        if (w)
        {
            IMovableButton* button = dynamic_cast<IMovableButton*>(w);
            if (button)
                id_to_win[button->GetOrigId()] = button;
        }
    }


    for (map<int,vector<pair<int, wxPoint> > >::const_iterator i = m_MovableButtonPosition.begin(); i != m_MovableButtonPosition.end(); i++) 
    {
        int win_id = i->first;
        CButtonPanel *win = dynamic_cast<CButtonPanel*>(FindWindow(win_id));
        if (win)
        {
            for (vector<pair<int,wxPoint> >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
            {
                int x = j->second.x;
                int y = j->second.y;
                int id = j->first;
                IMovableButton *parent_button = id_to_win[id];
                if (parent_button)
                {
                    IMovableButton *button = win->CreateButton(parent_button,x,y);
                    if (button)
                    {
                        button->SetSizers(this,m_Sizer1,m_Sizer2,m_Sizer3,m_SizerRemoved);
                        button->SetLocked(m_CheckBox->GetValue());
                        if (button->IsOccupiedPosition())
                        {                            
                            wxPoint p = parent_button->FindUnoccupiedPosition(win->GetSizer());
                            button->SetPosition(win->CalcScrolledPosition(p));     
                            button->SetOriginalPosition(p);
                        }
                    }
                }
            }
            win->Layout();
        }
    }
//    m_findButton->Hide();
}

void CEditingBtnsPanel::SetFrameAndPosition()
{
    if (m_FrameSize != wxDefaultSize)
        SetSize(m_FrameSize);
    SetPosition(m_FramePosition);  // For dual display may need wxDisplay and this comments: https://forums.wxwidgets.org/viewtopic.php?t=9100&highlight=dual+monitor
                                   // https://forums.wxwidgets.org/viewtopic.php?t=18353
}

static const char* kSettingsVersion = "VERSION";
static const char* kCoordsArray = "Button Coordinates";
static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";
static const char* kAutoStart = "Auto Start";
static const char* kEditingButtonsPanelLayout = "EditingButtonsLayout";
static const char* kBGColor = "BG Color";
static const char* kBGColorSet = "BG Color Set";
static const char* kDynIdsArray =  "Dynamic Button Ids";
static const char* kFilenamesArray = "Dynamic Button Filenames";
static const char* kNamesArray = "Dynamic Button Names";
static const char* kTitlesArray = "Dynamic Button Report Titles";

void CEditingBtnsPanel::SetRegistryPath(const string& reg_path)
{ 
    m_RegPath = kEditingButtonsPanelLayout;
}
 
void CEditingBtnsPanel::LoadSettings() 
{
    wxString path = CSysPath::ResolvePath( wxT("<home>/toolbar_buttons_layout.asn") );
    LoadSettings(path);
}

void CEditingBtnsPanel::LoadSettings(wxString path)
{
    if (m_RegPath.empty()) return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    string action = "Loading toolbar buttons layout registry - ";

    if( !path.empty() ){
        if( wxFileName::FileExists( path ) ){
            CNcbiIfstream istr( string(path.ToUTF8()) );
            if( istr ){
                gui_reg.AddSite(istr, CGuiRegistry::ePriority_Local - 2);
    
            } else {
                LOG_POST( Error << action << "registry file is invalid " << path );
            }
        } else {
            LOG_POST( Info << action << "skipping, no file " << path );
        }
    } else {
        LOG_POST( Info << action << "skipping, path is empty." );
    }

    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
    
    int version = view.GetInt(kSettingsVersion, 1);
    vector<int> coords;
    view.GetIntVec(kCoordsArray, coords);
    m_MovableButtonPosition.clear();
    if (!coords.empty())
    {
        unsigned int i = 0;
        while ( i < coords.size()-3 )
        {
            int id = coords[i];
            int x = coords[i+1];
            int y = coords[i+2];
            int win = coords[i+3];
            i += 4;
            if (version == 1)
                m_MovableButtonPosition[win].push_back(pair<int,wxPoint>(id,wxPoint(x,y)));
            else 
                m_MovableButtonPosition[win].push_back(pair<int,wxPoint>(id,wxPoint(x * (BUTTON_WIDTH + DIST_BETWEEN_BUTTONS) + DIST_BETWEEN_BUTTONS, y * (BUTTON_HEIGHT + DIST_BETWEEN_BUTTONS) + DIST_BETWEEN_BUTTONS)));
        }
    }


    int width = 600,height = 400;
    width = view.GetInt(kFrameWidth,width);
    height = view.GetInt(kFrameHeight,height);
    m_FrameSize = wxSize(width,height);
    int pos_x = 5, pos_y = 5;
    pos_x = view.GetInt(kFramePosX,pos_x);
    pos_y = view.GetInt(kFramePosY,pos_y);

    int max_x = 0;
    for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
    {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
    }
    if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
    if (pos_x < 0) pos_x = 5;
    if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
    if (pos_y < 0) pos_y = 5;
    m_FramePosition = wxPoint(pos_x,pos_y);

    if (m_AutoStart)
        m_AutoStart->SetValue(view.GetBool(kAutoStart));
    m_bg_color_set = view.GetBool(kBGColorSet, false);
    m_bg_color = wxColour(wxString(view.GetString(kBGColor, "white")));

    vector<int> ids;
    view.GetIntVec(kDynIdsArray, ids);
    vector<string> filenames;
    view.GetStringVec(kFilenamesArray, filenames);
    vector<string> button_names;
    view.GetStringVec(kNamesArray, button_names);
    vector<string> titles;
    view.GetStringVec(kTitlesArray, titles);

    m_dynamic_buttons_filename.clear();
    m_dynamic_buttons_name.clear();
    m_dynamic_buttons_title.clear();

    RemoveStaticButtons(ids, filenames, button_names, titles);
    if (RunningInsideNCBI()) {
        AddDefaultMacroButtons();
    }

    for (size_t i = 0; i < ids.size(); i++)
    {
        int id = ids[i];
        string filename = filenames[i];
        string button_name = button_names[i];
        string title = titles[i];
        m_dynamic_buttons_filename[id] = filename;
        m_dynamic_buttons_name[id] = button_name;
        if (!title.empty())
            m_dynamic_buttons_title[id] = title;
    }
}
 
void CEditingBtnsPanel::RemoveStaticButtons( vector<int> &ids,  vector<string> &filenames,  vector<string> &button_names,  vector<string> &titles)
{
    if (ids.empty())
    return;    
    for (int i = ids.size() - 1; i >= 0; i--)
    {
    int id = ids[i];
    if (id < ID_BUTTON_DYNAMIC)
    {
        ids.erase(ids.begin() + i);
        filenames.erase(filenames.begin() + i);
        button_names.erase(button_names.begin() + i);
        titles.erase(titles.begin() + i);
    }
    }
}

void CEditingBtnsPanel::SaveSettings() const
{
    wxString path = CSysPath::ResolvePath( wxT("<home>/toolbar_buttons_layout.asn") );
    SaveSettings(path);
}


void CEditingBtnsPanel::SaveSettings(wxString path) const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    

    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath, CGuiRegistry::ePriority_Local - 2);
    vector<int> coords;
    
    vector<wxSizer*> sizers;
    sizers.push_back(m_Sizer1);
    sizers.push_back(m_Sizer2);
    sizers.push_back(m_Sizer3);
    for (unsigned int i=0; i<sizers.size(); i++)
    {
        wxSizer *sz = sizers[i];
        if (sz)
        {
            CButtonPanel * panel = dynamic_cast<CButtonPanel*>(sz->GetContainingWindow());
            if (panel)
            {
                wxSizerItemList& children = sz->GetChildren();
                for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
                {
                    wxWindow* w = (*child)->GetWindow();
                    if (w)
                    {
                        IMovableButton* button = dynamic_cast<IMovableButton*>(w);
                        if (button)
                        {
                            coords.push_back(button->GetOrigId());
                            int col = int (0.5 + double(button->GetOrigPosition().x - DIST_BETWEEN_BUTTONS) / (BUTTON_WIDTH + DIST_BETWEEN_BUTTONS));
                            int row = int (0.5 + double(button->GetOrigPosition().y - DIST_BETWEEN_BUTTONS) / (BUTTON_HEIGHT + DIST_BETWEEN_BUTTONS));
                            coords.push_back(col);
                            coords.push_back(row);
                            coords.push_back(panel->GetId());
                        }
                    }
                }
            }
        }
    }

    vector<int> ids;
    vector<string> filenames;
    vector<string> button_names;
    vector<string> titles;
    for (auto btn : m_dynamic_buttons_filename)
    {
    int button_id = btn.first;
    string filename = btn.second;
    string button_name = m_dynamic_buttons_name[button_id];
    string title;
    if (m_dynamic_buttons_title.find(button_id) != m_dynamic_buttons_title.end())
        title = m_dynamic_buttons_title[button_id];
    ids.push_back(button_id);
    filenames.push_back(filename);
    button_names.push_back(button_name);
    titles.push_back(title);
    }

    view.Set(kSettingsVersion, 4);
    view.Set(kCoordsArray, coords);
    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);
    view.Set(kAutoStart, m_AutoStart->GetValue());
    view.Set(kBGColorSet, m_bg_color_set);
    view.Set(kBGColor, m_bg_color.GetAsString().ToStdString());
    view.Set(kDynIdsArray, ids);
    view.Set(kFilenamesArray, filenames);
    view.Set(kNamesArray, button_names);
    view.Set(kTitlesArray, titles);

    if( !path.empty() ){
        CNcbiOfstream ostr( string(path.ToUTF8()) );
        gui_reg.Write( ostr , CGuiRegistry::ePriority_Local - 2 );
    }
    else {
        LOG_POST( Info << "Saving window layout registry - skipping, path is empty" );
    }
}

void CEditingBtnsPanel::OnAddMacroClick( wxCommandEvent& event )
{
    CAddMacroButton dlg(this);
    if (dlg.ShowModal() == wxID_OK) 
    {
    wxString button_name = dlg.GetName();
    bool report_required = dlg.GetReportRequred();
    string filename = dlg.GetFileName();
    if (filename.empty())
        return;
    string title = dlg.GetTitle();
    int button_id = ID_BUTTON_DYNAMIC + m_dynamic_buttons_filename.size();
    CMovableButton* button = new CMovableButton(m_panel_removed, button_id, _("*") + button_name, wxDefaultPosition, wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    button->SetSizers(this,m_Sizer1,m_Sizer2,m_Sizer3,m_SizerRemoved);
    button->AddDynamicMenu();
    button->SetRemoved(true);
    button->SetLocked(m_CheckBox->GetValue());
    if (report_required)
    {
        Connect(button_id, wxEVT_BUTTON, wxCommandEventHandler(CEditingBtnsPanel::RunMacroWithReport), NULL, this);
        m_dynamic_buttons_title[button_id] = title;
    }
    else
    {
        Connect(button_id, wxEVT_BUTTON, wxCommandEventHandler(CEditingBtnsPanel::RunMacro), NULL, this);
    }	
    m_dynamic_buttons_filename[button_id] = filename;	
    m_dynamic_buttons_name[button_id] = button_name;
    m_all_buttons.push_back(button);
    sort(m_all_buttons.begin(),m_all_buttons.end(),comp_buttons);
    x_ResetRemovedTab();
    Layout();
    m_panel_removed->FitInside();
    int y;
    button->GetPosition(NULL, &y);
    int py;
    m_panel_removed->GetScrollPixelsPerUnit(NULL, &py);
    m_panel_removed->Scroll(-1, y / py);
    Refresh();
    Raise();
    button->SetFocus();
    }
}

void CEditingBtnsPanel::RemoveButton(IMovableButton *button)
{
    int id = button->GetOrigId();
    if (m_dynamic_buttons_title.find(id) != m_dynamic_buttons_title.end())
    {
    Disconnect(id, wxEVT_BUTTON, wxCommandEventHandler(CEditingBtnsPanel::RunMacroWithReport), NULL, this);
    }
    else
    {
    Disconnect(id, wxEVT_BUTTON, wxCommandEventHandler(CEditingBtnsPanel::RunMacro), NULL, this);
    }	
    m_dynamic_buttons_filename.erase(id);
    m_dynamic_buttons_title.erase(id);
    m_dynamic_buttons_name.erase(id);
    int pos = -1;
    for (size_t i = 0; i < m_all_buttons.size(); i++)
    {
    if (m_all_buttons[i] == button)
    {
        pos = i;
        break;
    }
    }
    if (pos >= 0)
    m_all_buttons.erase(m_all_buttons.begin() + pos);
    vector<wxSizer*> sizers;
    sizers.push_back(m_Sizer1);
    sizers.push_back(m_Sizer2);
    sizers.push_back(m_Sizer3);
    vector<IMovableButton*> buttons;
    for (unsigned int i=0; i<sizers.size(); i++)
    {
        wxSizer *sz = sizers[i];
        if (sz)
        {
            CButtonPanel * panel = dynamic_cast<CButtonPanel*>(sz->GetContainingWindow());
            if (panel)
            {
                panel->Scroll(0,0);
                panel->SetMovingButton(NULL,false);
            }
            wxSizerItemList& children = sz->GetChildren();
            for (wxSizerItemList::iterator child = children.begin(); child != children.end(); ++child)
            {
                wxWindow* w = (*child)->GetWindow();
                if (w)
                {
                    IMovableButton* btn = dynamic_cast<IMovableButton*>(w);
                    if (btn->GetOrigId() == id)
                        buttons.push_back(btn);
                }
            }
            
        }
    }

    for (size_t i=0; i<buttons.size(); i++)
        buttons[i]->Destroy(); 

    m_SizerRemoved->Detach(button);
    button->Destroy();
    Layout();
    Refresh();
    m_panel_removed->FitInside();
}

void CEditingBtnsPanel::EditButton(IMovableButton *button)
{
    int id = button->GetOrigId();
    if (m_dynamic_buttons_filename.find(id) == m_dynamic_buttons_filename.end())
    return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CRef<IGuiCoreHelper> gui_core_helper(new CGuiCoreHelper(m_Workbench));
    CMacroFlowEditor *frame = CMacroFlowEditor::GetInstance(main_window, gui_core_helper);
    frame->OpenScript(m_dynamic_buttons_filename[id], false, true);
}

void CEditingBtnsPanel::OnExportBtnClick( wxCommandEvent& event )
{
    wxFileDialog asn_save_file(this, wxT("Export settings to file"), wxEmptyString, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (asn_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = asn_save_file.GetPath();
        if( !path.empty() )
            SaveSettings(path);
    }
}

void CEditingBtnsPanel::OnImportBtnClick( wxCommandEvent& event )
{
    wxFileDialog asn_save_file(this, wxT("Import settings from file"), wxEmptyString, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if (asn_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = asn_save_file.GetPath();
        if( !path.empty() )
        {
            LoadSettings(path);
            SetFrameAndPosition();
            PlaceButtons();
        }
    }
}


/*!
 * Should we show tooltips?
 */

bool CEditingBtnsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditingBtnsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditingBtnsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditingBtnsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditingBtnsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditingBtnsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditingBtnsPanel icon retrieval
}


bool CEditingBtnsPanel::OnDataChange()
{
    m_TopSeqEntry.Reset();
    m_WorkDir.Clear();
    m_SeqSubmit.Reset(NULL);
    m_CmdProccessor = NULL;
    m_Locations.clear();
    if (!m_Workbench) return false;
    
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CSeq_submit* const_sub = dynamic_cast<const CSeq_submit*>((*it).object.GetPointer());
        if (const_sub) {
            // CSeq_submit* sub = const_cast<CSeq_submit*>(const_sub);
            m_SeqSubmit.Reset(const_sub);
        }
        
        if (!m_TopSeqEntry) {
            CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);
            if (seh) {
                m_TopSeqEntry = seh;
                if (m_SeqSubmit) {
                    break;
                }
            }
        }
    }

    if (!m_TopSeqEntry) return false;
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
        if (doc) {
            m_CmdProccessor = &doc->GetUndoManager();
            m_WorkDir = doc->GetWorkDir();
        }
    }

    if (!m_CmdProccessor) return false;

    return true;
}

bool CEditingBtnsPanel::IsLocationSelected()
{
    m_Locations.clear();
    m_Locations = x_GetSelectedLocations(); 
    return !m_Locations.empty();
}

bool CEditingBtnsPanel::IsLocationSelectedAa()
{
    vector<CRef<CSeq_loc> > locs = x_GetSelectedLocations();   
    m_Locations.clear();
    ITERATE(vector<CRef<CSeq_loc> >, lit, locs) {
            CScope& scope = m_TopSeqEntry.GetScope();
            CBioseq_Handle bsh = scope.GetBioseqHandle(**lit);
            if (bsh) {
                if (bsh.IsAa()) { // For Prot commands
                    m_Locations.push_back(*lit);
                } 
            }
    }
    return !m_Locations.empty();
}

bool CEditingBtnsPanel::IsLocationSelectedNa()
{
    vector<CRef<CSeq_loc> > locs = x_GetSelectedLocations();   

    m_Locations.clear();
    ITERATE(vector<CRef<CSeq_loc> >, lit, locs) {
            CScope& scope = m_TopSeqEntry.GetScope();
            CBioseq_Handle bsh = scope.GetBioseqHandle(**lit);
            if (bsh) {
                if (bsh.IsNa()) { // For Nuc commands
                    m_Locations.push_back(*lit);
                } 
            }
    }
    return !m_Locations.empty();
}

vector<CRef<CSeq_loc> > CEditingBtnsPanel::x_GetSelectedLocations()
{
    vector<CRef<CSeq_loc> > locs;
    if (!m_Workbench) {
        return locs;
    }

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) return locs;
    sel_srv->GetCurrentSelection(objects);
    if (objects.empty()) 
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }

    if (objects.empty()) return locs;


    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CObject* ptr = it->object.GetPointer();
        
        /// CSeq_entry
        const objects::CSeq_feat* seqfeat = dynamic_cast<const objects::CSeq_feat*>(ptr);
        const objects::CSeq_loc* seqloc = dynamic_cast<const objects::CSeq_loc*>(ptr);
        const objects::CBioseq* bioseq = dynamic_cast<const objects::CBioseq*>(ptr);
        const objects::CSeqdesc* seqdesc = dynamic_cast<const objects::CSeqdesc*>(ptr);
        const objects::CPubdesc* pubdesc = dynamic_cast<const objects::CPubdesc*>(ptr);
        const objects::CSeq_id* seq_id = dynamic_cast<const objects::CSeq_id*>(ptr);

        if (seqfeat) {
            if (seqfeat->IsSetLocation()) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->Assign(seqfeat->GetLocation());
                locs.push_back(loc);
            }
        } else if (seq_id) {
            CBioseq_Handle bsh = it->scope->GetBioseqHandle(*seq_id);
            if (bsh) {
                CRef<CSeq_id> new_id(new CSeq_id());
                new_id->Assign(*seq_id);
                CRef<CSeq_loc> loc(new CSeq_loc(*new_id, 0, bsh.GetInst_Length() - 1));
                locs.push_back(loc);
            }
        }   else if (seqloc) {
            CRef<CSeq_loc> loc(new CSeq_loc());
            loc->Assign(*seqloc);
            locs.push_back(loc);
        } else if (bioseq) {
            CRef<CSeq_loc> loc(new CSeq_loc());
            loc->SetInt().SetId().Assign(*(bioseq->GetId().front()));
            loc->SetInt().SetFrom(0);
            loc->SetInt().SetTo(bioseq->GetLength() - 1);
            locs.push_back(loc);
        } else if (seqdesc) {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(it->scope, *seqdesc);
            CBioseq_CI bi(seh);
            while (bi) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetInt().SetId().Assign(*(bi->GetId().front().GetSeqId()));
                loc->SetInt().SetFrom(0);
                loc->SetInt().SetTo(bi->GetInst_Length() - 1);
                locs.push_back(loc);
                ++bi;
            }
        } else if (pubdesc) {
            CSeq_entry_Handle seh = GetSeqEntryForPubdesc(it->scope, *pubdesc);
            CBioseq_CI bi(seh);
            while (bi) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetInt().SetId().Assign(*(bi->GetId().front().GetSeqId()));
                loc->SetInt().SetFrom(0);
                loc->SetInt().SetTo(bi->GetInst_Length() - 1);
                locs.push_back(loc);
                ++bi;
            }
        }
    }
    if (!m_TopSeqEntry && !locs.empty()) {
        CBioseq_Handle bsh = objects.front().scope->GetBioseqHandle(*(locs.front()->GetId()));
        m_TopSeqEntry = bsh.GetTopLevelEntry();
    }

    if (m_TopSeqEntry && m_Workbench)
    {
        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        if (srv)
        {
            CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
            if (ws) 
            {
                CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
                if (doc)
                    m_CmdProccessor = &doc->GetUndoManager(); 
            }
        }
    }
    return locs;
}



/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CEditingBtnsPanel::OnReverseAuthorNamesClick( wxCommandEvent& event )
{
    CReverseAuthorNames worker;

    CRef<CCmdComposite> composite(new CCmdComposite("Reverse Author Last and First Name"));
    bool found = false;
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv)
    {
        sel_srv->GetCurrentSelection(objects);
        if (objects.empty()) 
        {
            sel_srv->GetActiveObjects(objects);
        }
        if (objects.empty())
        {
            GetViewObjects(m_Workbench, objects);
        }

        if (!objects.empty())
        {
            NON_CONST_ITERATE (TConstScopedObjects, it, objects) 
            {
                found |= worker.ApplyToPubdesc(it->object, m_TopSeqEntry, *composite);
            }
        }
    }
    
    if (found) {
        m_CmdProccessor->Execute(composite.GetPointer());
    }
    else
    {
        CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
        if (cmd) {
            m_CmdProccessor->Execute(cmd);
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void CEditingBtnsPanel::EditPubs( wxCommandEvent& event )
{
    CEditPubDlg * dlg = new CEditPubDlg(NULL, m_Workbench);   
    dlg->Show(true);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void CEditingBtnsPanel::ParseLocalIdToSrc( wxCommandEvent& event )
{
    CParseTextDlg * dlg = new CParseTextDlg(NULL, m_Workbench); 
    dlg->SetFromFieldType(CParseTextDlg::eFieldType_LocalId);
    dlg->SetToFieldType(CParseTextDlg::eFieldType_Source);
    dlg->Show(true);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
 */

void CEditingBtnsPanel::OnFixCapitalizationAll( wxCommandEvent& event )
{
    CFixAllCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5
 */

void CEditingBtnsPanel::OnFixCapitalizationAuthors( wxCommandEvent& event )
{
    CFixAuthorCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON6
 */

void CEditingBtnsPanel::OnFixCapitalizationTitles( wxCommandEvent& event )
{
    CFixTitleCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON7
 */

void CEditingBtnsPanel::OnFixCapitalizationAffiliation( wxCommandEvent& event )
{
    CFixAffilCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON8
 */

void CEditingBtnsPanel::SortUniqueCount( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    IRefreshCntrl * suc_refresh = new CSUCRefreshCntrl(m_Workbench);
    CGenericReportDlg* report = new CGenericReportDlg(main_window, m_Workbench, wxID_ANY, _("SUC"));
    report->SetRefresh(suc_refresh);
    report->RefreshData();
    report->Show(true);    
}

void CEditingBtnsPanel::ClickableSortUniqueCount( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CClickableSortUniqueCount* dlg = new CClickableSortUniqueCount(main_window, m_Workbench);
    dlg->Show(true);   
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON9
 */

void CEditingBtnsPanel::OnRemoveUnpublishedPublications( wxCommandEvent& event )
{
    CRemoveUnpublishedPubs worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON10
 */

static const char* kFrameDlgWidth = "Frame Width";
static const char* kFrameDlgHeight = "Frame Height";
static const char* kFrameDlgPosX = "Frame Position X";
static const char* kFrameDlgPosY = "Frame Position Y";


void CEditingBtnsPanel::SaveSettings(const string &regpath, wxWindow *dlg) const
{
    if (regpath.empty() || !dlg)
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(regpath);

    view.Set(kFrameDlgWidth, dlg->GetScreenRect().GetWidth());
    view.Set(kFrameDlgHeight, dlg->GetScreenRect().GetHeight());
    view.Set(kFrameDlgPosX, dlg->GetScreenPosition().x);
    view.Set(kFrameDlgPosY, dlg->GetScreenPosition().y);
}


void CEditingBtnsPanel::LoadSettings(const string &regpath, wxWindow *dlg)
{
    if (regpath.empty() || !dlg)
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(regpath);

    int width = view.GetInt(kFrameDlgWidth, -1);
    int height = view.GetInt(kFrameDlgHeight, -1);
    if (width >= 0  && height >= 0)
        dlg->SetSize(wxSize(width,height));

    int pos_x = view.GetInt(kFrameDlgPosX, -1);
    int pos_y = view.GetInt(kFrameDlgPosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       dlg->SetPosition(wxPoint(pos_x,pos_y));
   }
   wxTopLevelWindow* top_win = dynamic_cast<wxTopLevelWindow*>(dlg);
   if (top_win)
   {
       wxRect rc = dlg->GetRect();
       CorrectWindowRect(top_win, rc);
       dlg->SetSize(rc);
       dlg->Move(rc.GetLeftTop());
   }
}

void CEditingBtnsPanel::TableReaderFromClipboard( wxCommandEvent& event )
{
   CQualTableLoadManager* qual_mgr = NULL;
   if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    vector< CIRef<IFileLoadPanelClientFactory> > file_formats;
    GetExtensionAsInterface("file_load_panel_client", file_formats);
    CIRef<IFileLoadPanelClient> format_manager;

    for (size_t i = 0; i < file_formats.size(); ++i) {
        if (file_formats[i]->GetFileLoaderId() == "file_loader_qual_table") {
            format_manager.Reset(file_formats[i]->CreateInstance());
        
            if ("file_loader_qual_table" == format_manager->GetFileLoaderId()) {            
                const CQualTableLoadManager* const_qual_mgr = dynamic_cast<const CQualTableLoadManager*>(format_manager.GetPointer());
                if (const_qual_mgr) {
                    qual_mgr = const_cast<CQualTableLoadManager*>(const_qual_mgr);
                    qual_mgr->SetTopLevelEntry(m_TopSeqEntry);
                    qual_mgr->SetServiceLocator(m_Workbench);
                }            
            }
        }
    }


    string fname;
    {
        wxTextDataObject data;
        if (wxTheClipboard->Open())
        {
            if (wxTheClipboard->IsSupported( wxDF_UNICODETEXT ))
                wxTheClipboard->GetData( data );
            wxTheClipboard->Close();
        }            

        if (data.GetText().length() == 0) {
            wxMessageBox("No data in clipboard for table");
            return;
        }

        CTmpFile f(CTmpFile::eNoRemove);
        f.AsOutputFile(CTmpFile::eIfExists_ReturnCurrent) << data.GetText();
        fname = f.GetFileName();
    }
            
    vector<wxString> fnames;
    fnames.push_back(ToWxString(fname));

    // This also causes windows to be created it seems, and so it must happen after 
    // create is called....
    qual_mgr->SetFilenames(fnames);

    COpenDataSourceObjectDlg dlg(NULL, CIRef<IUIToolManager>(qual_mgr));
    dlg.SetSize(760, 940);
    dlg.Centre(wxBOTH | wxCENTRE_ON_SCREEN);
    dlg.SetMinSize(wxSize(760, 750));
    dlg.SetRegistryPath("Dialogs.Edit.OpenTables");
    LoadSettings("Dialogs.Edit.OpenTables", &dlg);

    CFile tmp_file(fname);
    tmp_file.Remove();

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        qual_mgr->x_GetColumnIdPanel()->x_TableReaderCommon(object_loader, wxT("Reading from clipboard(s)..."));
    }
    SaveSettings("Dialogs.Edit.OpenTables", &dlg);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
 */

void CEditingBtnsPanel::TaxFixCleanup( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd = CleanupCommand(m_TopSeqEntry, true, (event.GetId() == ID_BUTTON11));
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
 */

void CEditingBtnsPanel::RemoveAllFeatures( wxCommandEvent& event )
{
    CRemoveFeaturesDlg::RemoveAllFeatures(m_TopSeqEntry, m_CmdProccessor);    
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13
 */

void CEditingBtnsPanel::OnStripAuthorSuffixes( wxCommandEvent& event )
{
    CStripAuthorSuffix worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON14
 */
void CEditingBtnsPanel::OnRemoveAuthorConsortiums( wxCommandEvent& event )
{
    CRemoveAuthorConsortium worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON16
 */

void CEditingBtnsPanel::TrimNsRich( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CTrimN worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Trim Ns Rich", main_window, m_WorkDir))
        event.Skip();
}

void CEditingBtnsPanel::TrimNsTerminal( wxCommandEvent& event )
{
    CSequenceAmbigTrimmer::TTrimRuleVec trim_rules;
    trim_rules.push_back(CSequenceAmbigTrimmer::STrimRule{ 1, 0 });

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CTrimN worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Trim Ns Terminal", main_window, m_WorkDir, trim_rules))
        event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON17
 */

void CEditingBtnsPanel::AnnotateDescriptorComment( wxCommandEvent& event)
{
    CSeq_entry_Handle seh = m_TopSeqEntry;
    if (!m_Locations.empty()) 
    {
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*m_Locations.front());
        seh = bsh.GetSeq_entry_Handle();
    }
    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetComment();
    CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(m_WorkDir);
    edit_dlg->Show(true);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON18
 */

void CEditingBtnsPanel::MrnaCds( wxCommandEvent& event )
{
    CMRnaForCDS worker;
    if (!worker.apply(m_Workbench,m_CmdProccessor,"Create mRNA for CDS",m_TopSeqEntry.GetScope()))
        event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON19
 */

void CEditingBtnsPanel::OnAddCitSubForUpdate( wxCommandEvent& event )
{
    CAddCitSubForUpdate worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Cit-sub for update"))
        event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON20
 */

void CEditingBtnsPanel::OnRemoveSequences( wxCommandEvent& event )
{

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRemoveSequencesDlg dlg(main_window, m_TopSeqEntry);

    if (dlg.ShowModal() == wxID_OK) {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) {
            m_CmdProccessor->Execute(cmd);                       
        }
    }

  
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON21
 */

void CEditingBtnsPanel::OnRemoveGenomeProjectsDB( wxCommandEvent& event )
{
    CRemoveGenomeProjectsDB worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove GenomeProjectsDB"))
        event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON22
 */

void CEditingBtnsPanel::RemProtTitles( wxCommandEvent& event )
{
    CRemProtTitles worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Protein Titles"))
        event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON23
 */

void CEditingBtnsPanel::RmCultureNotes( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd = GetRmCultureNotesCommand(m_TopSeqEntry.GetTopLevelEntry());
    if (cmd)  m_CmdProccessor->Execute(cmd);         
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON26
 */

void CEditingBtnsPanel::StrainToSp( wxCommandEvent& event )
{
    CAppendModToOrg worker(true,COrgMod::eSubtype_strain,"",true,true,false,false,false,false,true);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry);
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON24
 */

void CEditingBtnsPanel::RevComp16S(wxCommandEvent& event)
{
    CCorrectRNAStrandDlg* dlg = new CCorrectRNAStrandDlg(NULL, m_Workbench);
    dlg->Show(true); 
}

void CEditingBtnsPanel::OnRemoveDescriptors( wxCommandEvent& event )
{  
    CRemoveDescDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
        dlg.Close();        
    }
}  


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON27
 */

void CEditingBtnsPanel::OnRemoveAllStructuredComments( wxCommandEvent& event )
{
    CRemoveAllStructuredComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove All Structured Comments"))
        event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON28
 */

void CEditingBtnsPanel::BulkEditCDS( wxCommandEvent& event )
{
    CSeq_entry_Handle seh = m_TopSeqEntry;
    /* vector<CRef<CSeq_loc> > locs = x_GetSelectedLocations();
    if (!locs.empty()) {
        try
        {
            seh = m_TopSeqEntry.GetScope().GetBioseqHandle(*locs.front()).GetParentBioseq_set().GetParentEntry();
        } catch (CException&) {}
    }
    */
    try
    {
        CBulkCDS dlg(NULL, seh, m_Workbench, m_SeqSubmit);
        bool done = false;
        while (!done) {
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) {
                    m_CmdProccessor->Execute(cmd);                       
                    //dlg.Close();        
                    done = true;
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
            } else {
                done = true;
            }
        }
    } catch  (CException&) {}
}


void CEditingBtnsPanel::AddTranslExcept( wxCommandEvent& event )
{
    CAddTranslExceptDlg * dlg = new CAddTranslExceptDlg(NULL, m_Workbench);
    dlg->Show(true);
}

void CEditingBtnsPanel::OnRemoveFeatures( wxCommandEvent& event )
{  
    if (!m_Workbench) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CRemoveFeaturesDlg *dlg = new CRemoveFeaturesDlg(main_window, m_Workbench);
    dlg->Show(true);
}  

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON31
 */

void CEditingBtnsPanel::BulkGeneEdit( wxCommandEvent& event )
{
    CSeq_entry_Handle seh = m_TopSeqEntry;
    
    try
    {
        CBulkGene dlg(NULL, seh,m_Workbench, m_SeqSubmit);
    
        bool done = false;
        while (!done) {
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) {
                    m_CmdProccessor->Execute(cmd);                       
                    //dlg.Close();        
                    done = true;
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
            } else {
                done = true;
            }
        }
    } catch  (CException&) {}
}

void CEditingBtnsPanel::OnRemoveBankitComment( wxCommandEvent& event )
{
    CRemoveBankitComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Bankit Comment"))
        event.Skip();
}

void CEditingBtnsPanel::OnRemoveStructuredCommentKeyword( wxCommandEvent& event )
{
    CRemoveStructuredCommentKeyword worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Structured Comment Keyword"))
        event.Skip();
}

void CEditingBtnsPanel::OnAddStructuredCommentKeyword( wxCommandEvent& event )
{
    CAddStructuredCommentKeyword worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Structured Comment Keyword"))
        event.Skip();
}

void CEditingBtnsPanel::OnReorderStructuredComment( wxCommandEvent& event )
{
    CReorderStructuredComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Reorder Structured Comment"))
        event.Skip();
}

void CEditingBtnsPanel::OnReorderSequences( wxCommandEvent& event )
{  
    CReorderSequencesDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
        dlg.Close();        
    }
}  

void CEditingBtnsPanel::OnConvertComment( wxCommandEvent& event )
{
    CConvertComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Convert Comment to Structured Comment", "::","  "))
        event.Skip();
}

void CEditingBtnsPanel::OnParseComment( wxCommandEvent& event )
{
    CConvertComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Parse Comment to Structured Comment", "=",";"))
        event.Skip();
}

void CEditingBtnsPanel::OnAssignFeatureIds( wxCommandEvent& event )
{
    CAssignFeatureId worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Assign Feature Ids"))
        event.Skip();
}

void CEditingBtnsPanel::LabelrRNAs( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CLabelRna worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Label RNA", main_window))
        event.Skip();
}

void CEditingBtnsPanel::BulkEditRNA( wxCommandEvent& event )
{
    CSeq_entry_Handle seh = m_TopSeqEntry;
       
    try
    {
        CBulkRna dlg(NULL, seh,m_Workbench, m_SeqSubmit);
        
        bool done = false;
        while (!done) {
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) {
                    m_CmdProccessor->Execute(cmd);                       
                    //dlg.Close();        
                    done = true;
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
            } else {
                done = true;
            }
        }
    } catch  (CException&) {}
}

void CEditingBtnsPanel::OnFixiInPrimerSeqs( wxCommandEvent& event )
{
    CPCRPrimerFixI worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnFixOrgModInstitution( wxCommandEvent& event )
{
    CFixOrgModInst worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnSwapPrimerSeqName( wxCommandEvent& event )
{
    CSwapPrimerNameSeq worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnMergePrimerSets(wxCommandEvent& event)
{
    CMergePrimerSets worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnSplitPrimerSets(wxCommandEvent& event)
{
    CSplitPrimerSets worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnAppendModToOrg( wxCommandEvent& event )
{  
    CAppendModToOrgDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
        dlg.Close();        
    }
}  

void CEditingBtnsPanel::OnParseCollectionDateMonthFirst( wxCommandEvent& event )
{
    CParseCollectionDateFormats worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, true);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnParseCollectionDateDayFirst( wxCommandEvent& event )
{
    CParseCollectionDateFormats worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, false);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnCountryFixupCap(wxCommandEvent& event)
{
    CCountryFixup worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, true);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnCountryFixupNoCap(wxCommandEvent& event)
{
    CCountryFixup worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, false);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::OnAddrRNA16S ( wxCommandEvent& event )
{
    bool ispartial5 = true, ispartial3 = true;
    CRef<CCmdComposite> cmd = AddNamedrRna(m_TopSeqEntry, "16S ribosomal RNA", ispartial5, ispartial3);
    if (cmd) m_CmdProccessor->Execute(cmd);
    else
        event.Skip();
        
}

void CEditingBtnsPanel::OnAddrRNA18S ( wxCommandEvent& event )
{
    bool ispartial5 = true, ispartial3 = true;
    CRef<CCmdComposite> cmd = AddNamedrRna(m_TopSeqEntry, "18S ribosomal RNA", ispartial5, ispartial3);
    if (cmd) m_CmdProccessor->Execute(cmd);
    else
        event.Skip();
        
}

void CEditingBtnsPanel::OnAddrRNA23S ( wxCommandEvent& event )
{
    bool ispartial5 = true, ispartial3 = true;
    CRef<CCmdComposite> cmd = AddNamedrRna(m_TopSeqEntry, "23S ribosomal RNA", ispartial5, ispartial3);
    if (cmd) m_CmdProccessor->Execute(cmd);
    else
        event.Skip();
        
}

void CEditingBtnsPanel::OnRemoveDefLines ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Removing DefLines"));
    if (composite){
        RemoveTitlesSeqEntry(m_TopSeqEntry, *(m_TopSeqEntry.GetCompleteSeq_entry()), composite);
        m_CmdProccessor->Execute(composite);
        CAddDefLineDlg *dlg = new CAddDefLineDlg(NULL, m_Workbench);
        dlg->Show(true);
    }else
        event.Skip();
}


// Added 09-30-2013

void CEditingBtnsPanel::VectorTrim( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CVectorTrimDlg* dlg = new CVectorTrimDlg(main_window, m_TopSeqEntry, m_CmdProccessor);
    dlg->Show(true);   
}

void CEditingBtnsPanel::AddCDS( wxCommandEvent& event )
{
    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, m_TopSeqEntry, 
                                                           m_Workbench);
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_cdregion);
    dlg->Show(true);
/*    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) { 
            m_CmdProccessor->Execute(cmd);                       
        } else {
            string errmsg = dlg.GetErrorMessage();
            if (!NStr::IsBlank(errmsg)) {
                wxString error = ToWxString(errmsg);
                wxMessageBox(error, wxT("Error"),
                         wxOK | wxICON_ERROR, NULL);
            }
        }
        dlg.Close();        
    }
*/
}


void CEditingBtnsPanel::AddRNA( wxCommandEvent& event )
{

    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, m_TopSeqEntry, 
                                                           m_Workbench);
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_rRNA);
//    dlg->CenterOnParent();
    dlg->Show(true);
/*
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) { 
            m_CmdProccessor->Execute(cmd);                       
        } else {
            string errmsg = dlg.GetErrorMessage();
            if (!NStr::IsBlank(errmsg)) {
                wxString error = ToWxString(errmsg);
                wxMessageBox(error, wxT("Error"),
                         wxOK | wxICON_ERROR, NULL);
            }
        }
        dlg.Close();        
    }
*/
}


void CEditingBtnsPanel::AddOtherFeature( wxCommandEvent& event )
{

    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, m_TopSeqEntry, 
                                                           m_Workbench);
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_gene);
    //dlg.CenterOnParent();
    dlg->Show(true);
/*
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) { 
            m_CmdProccessor->Execute(cmd);                       
        } else {
            string errmsg = dlg.GetErrorMessage();
            if (!NStr::IsBlank(errmsg)) {
                wxString error = ToWxString(errmsg);
                wxMessageBox(error, wxT("Error"),
                         wxOK | wxICON_ERROR, NULL);
            }
        }
        dlg.Close();        
    }
*/
}


void CEditingBtnsPanel::ValidateSeq( wxCommandEvent& event )
{
    CRef<CUser_object> params(new CUser_object());
    CTableDataValidateParams::SetDoAlignment(*params, true);
    CTableDataValidateParams::SetDoInference(*params, false);

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CValidateFrame::GetInstance(main_window, m_Workbench, params);
}


void CEditingBtnsPanel::DiscrepancyReport ( wxCommandEvent& event )
{
    if (m_TopSeqEntry) {
        CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eNormal, m_TopSeqEntry.GetScope(), m_Workbench, m_WorkDir);
    }
}


void CEditingBtnsPanel::AddSequences( wxCommandEvent& event )
{
    CAddSequences worker;
    worker.apply(m_TopSeqEntry, m_CmdProccessor);
}

void CEditingBtnsPanel::ShowAfter(wxWindow* dlg)
{
    dlg->Show();
}

void CEditingBtnsPanel::ApplySourceQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_Source;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type);

    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::EditSourceQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Edit;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_Source;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type);

    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ConvertSourceQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Convert;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_Source;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type);

    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::SwapSourceQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Swap;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_Source;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type);

    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::RemoveSourceQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Remove;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_Source;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type);
   
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ApplyRNAQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_RNA;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::EditRNAQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Edit;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_RNA;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ConvertRNAQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Convert;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_RNA;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::SwapRNAQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Swap;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_RNA;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::RemoveRNAQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Remove;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_RNA;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ApplyCGPQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_CDSGeneProt;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::EditCGPQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Edit;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_CDSGeneProt;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ConvertCGPQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Convert;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_CDSGeneProt;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::SwapCGPQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Swap;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_CDSGeneProt;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::RemoveCGPQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Remove;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_CDSGeneProt;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ApplyFeatQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_Feature;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::EditFeatQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Edit;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_Feature;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ConvertFeatQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Convert;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_Feature;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::SwapFeatQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Swap;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_Feature;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::RemoveFeatQual( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Remove;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_Feature;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::EditStructField( wxCommandEvent& evt)
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Edit;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_StructuredComment;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type);
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void  CEditingBtnsPanel::EditFeatureEvidence( wxCommandEvent& evt )
{
    try
    {
        CBulkEditFeatDlg * dlg = new CBulkEditFeatDlg(NULL, m_Workbench);
        dlg->SetEditingType(CBulkEditFeatDlg::eEvidence);
        dlg->Show(true);
    }
    catch (CException& e) 
    {
        wxMessageBox(e.GetMsg(), wxT("Error"), wxOK | wxICON_ERROR, NULL);                        
    }      
}

void  CEditingBtnsPanel::EditFeatureLocation( wxCommandEvent& evt )
{
    try
    {
        CBulkEditFeatDlg * dlg = new CBulkEditFeatDlg(NULL, m_Workbench);
        dlg->SetEditingType(CBulkEditFeatDlg::eLocation);
        dlg->Show(true);
    }
    catch (CException& e) 
    {
        wxMessageBox(e.GetMsg(), wxT("Error"), wxOK | wxICON_ERROR, NULL);                        
    }                   
}


void  CEditingBtnsPanel::ParseTextFromDefline( wxCommandEvent& evt )
{
    CParseTextDlg * dlg = new CParseTextDlg(NULL, m_Workbench); 
    dlg->SetFromFieldType(CParseTextDlg::eFieldType_Misc);
    dlg->SetToFieldType(CParseTextDlg::eFieldType_Source);
    dlg->Show(true);
}

void  CEditingBtnsPanel::ParseText( wxCommandEvent& evt )
{
    CParseTextDlg * dlg = new CParseTextDlg(NULL, m_Workbench);
    dlg->Show(true);
}


void  CEditingBtnsPanel::ConvertFeatures( wxCommandEvent& evt )
{
    CConvertFeatDlg * dlg = new CConvertFeatDlg(NULL, m_Workbench);
    dlg->Show(true);
}


void CEditingBtnsPanel::TableReader( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd(NULL);
    CQualTableLoadManager* qual_mgr = NULL;

    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    format_ids.push_back("file_loader_qual_table");
    fileManager->LoadFormats(format_ids);
    fileManager->SetWorkDir(m_WorkDir);

    for( size_t i = 0;  i < fileManager->GetFormatManagers().size();  ++i  )   {
        const IFileLoadPanelClient* mgr = fileManager->GetFormatManagers()[0].GetPointerOrNull();

        if ("file_loader_qual_table" == mgr->GetFileLoaderId()) {
            const CQualTableLoadManager* const_qual_mgr = dynamic_cast<const CQualTableLoadManager*>(mgr);
            if (const_qual_mgr) {
                qual_mgr = const_cast<CQualTableLoadManager*>(const_qual_mgr);
                qual_mgr->SetTopLevelEntry(m_TopSeqEntry);
                qual_mgr->SetServiceLocator(m_Workbench);
            }            
        }
    }

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.OpenTables");
    dlg.SetManagers(loadManagers);
    LoadSettings("Dialogs.Edit.OpenTables", &dlg);
    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        qual_mgr->x_GetColumnIdPanel()->x_TableReaderCommon(object_loader, wxT("Reading file(s)..."));
    }
    SaveSettings("Dialogs.Edit.OpenTables", &dlg);
}


void CEditingBtnsPanel::BulkEdit( wxCommandEvent& event )
{

    try
    {
        SrcEditDialog dlg(NULL, m_TopSeqEntry, m_Workbench, m_SeqSubmit);
        dlg.SetWorkDir(m_WorkDir);
        if (dlg.ShowModal() == wxID_OK) {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            m_CmdProccessor->Execute(cmd);
            // dlg.Close();        
        }    
    } catch  (CException&) {}
}

void CEditingBtnsPanel::MolInfoEdit(wxCommandEvent& evt)
{
    CMolInfoEditDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)
        {
            m_CmdProccessor->Execute(cmd);                       
        }
        dlg.Close();        
    }
}


void CEditingBtnsPanel::AutodefDefaultOptions(wxCommandEvent& evt)
{
    SAutodefParams params;
    ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
    CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
    m_CmdProccessor->Execute(cmd);
}

static bool CompLoc(CRef<CSeq_loc> first, CRef<CSeq_loc> second)
{
    return first->Compare(*second);
}


static bool EqualLoc(CRef<CSeq_loc> first, CRef<CSeq_loc> second)
{
    return first->Equals(*second);
}


void CEditingBtnsPanel::AutodefOptions(wxCommandEvent& evt)
{                
    SAutodefParams params;
    ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
    CAutodefDlg dlg(NULL);
    dlg.SetParams(&params);
    if (dlg.ShowModal() == wxID_OK) {

        dlg.PopulateParams(params);
        CRef<CCmdComposite> cmd(NULL);
        if (dlg.OnlyTargeted()) {
            vector<CRef<CSeq_loc> > orig_locs = x_GetSelectedLocations();
            vector<CRef<CSeq_loc> > cpy = orig_locs;
            vector<CRef<CSeq_loc> >::iterator lit = cpy.begin();
            while (lit != cpy.end()) {
                CBioseq_Handle b = m_TopSeqEntry.GetScope().GetBioseqHandle(**lit);
                if (!b || b.IsAa()) {
                    lit = cpy.erase(lit);
                }
                else {
                    ++lit;
                }
            }

            if (cpy.empty()) {
                dlg.Close();
                return;
            }
            stable_sort(cpy.begin(), cpy.end(), CompLoc);
            cpy.erase(unique(cpy.begin(), cpy.end(), EqualLoc), cpy.end());

            cmd.Reset(new CCmdComposite("Autodef"));
            ITERATE(vector<CRef<CSeq_loc> >, b, cpy) {
                CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(**b);
                CRef<CCmdComposite> subcmd = AutodefSeqEntry(m_TopSeqEntry, bsh, &params);
                if (subcmd) {
                    cmd->AddCommand(*subcmd);
                }
            }
        }
        else {
            cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
        }
        m_CmdProccessor->Execute(cmd);
        dlg.Close();
    }
}

void CEditingBtnsPanel::AutodefMisc(wxCommandEvent& evt)
{
    SAutodefParams params;
    ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
    params.m_MiscFeatRule = 1;
    CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
    m_CmdProccessor->Execute(cmd);
}

void CEditingBtnsPanel::AutodefId(wxCommandEvent& evt)
{
    SAutodefParams params;
    ConfigureAutodefParamsForID(params, m_TopSeqEntry);
    CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
    m_CmdProccessor->Execute(cmd);
}

void CEditingBtnsPanel::AutodefNoMods(wxCommandEvent& evt)
{
    SAutodefParams params;
    CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
    m_CmdProccessor->Execute(cmd);
}


void CEditingBtnsPanel::AutodefPopset(wxCommandEvent& evt)
{
    SAutodefParams params;
    ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
    CAutodefDlg dlg(NULL);
    dlg.SetParams(&params);
    dlg.ShowSourceOptions(false);
    if (dlg.ShowModal() == wxID_OK) 
    {                    
        dlg.PopulateParams(params);
        CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params, true);
        m_CmdProccessor->Execute(cmd);                       
        dlg.Close();        
    }
}

void CEditingBtnsPanel::AutodefRefresh(wxCommandEvent& evt)
{
    CRef<CCmdComposite> cmd = RefreshDeflineCommand(m_TopSeqEntry);
    if (cmd) m_CmdProccessor->Execute(cmd);
}

void CEditingBtnsPanel::ParseStrainSerotypeFromNames (wxCommandEvent& event)
{
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_parse);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Parse Strain Serotype from Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CEditingBtnsPanel::AddStrainSerotypeToNames (wxCommandEvent& event)
{
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_add);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Add Strain Serotype to Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CEditingBtnsPanel::FixupOrganismNames (wxCommandEvent& event)
{
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_fixup);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Fixup Organism Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CEditingBtnsPanel::SplitQualifiersAtCommas (wxCommandEvent& event)
{
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandSplitQualsAtCommas);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Split Quals at Commas");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}

void CEditingBtnsPanel::SplitStructuredCollections (wxCommandEvent& event)
{
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandSplitStructuredCollections);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Split Structured Collections at Semicolon");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CEditingBtnsPanel::TrimOrganismNames (wxCommandEvent& event)
{
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandTrimOrganismNames);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Trim Organism Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CEditingBtnsPanel::AddNamedrRNA12S(wxCommandEvent& event)
{
     bool ispartial5 = true;
     bool ispartial3 = true;
     CRef<CCmdComposite> composite = AddNamedrRna(m_TopSeqEntry, "12S ribosomal RNA", ispartial5, ispartial3);
     if (composite){
         m_CmdProccessor->Execute(composite);
     }
}

void CEditingBtnsPanel::AddNamedrRNA28S(wxCommandEvent& event)
{
     bool ispartial5 = true;
     bool ispartial3 = true;
     CRef<CCmdComposite> composite = AddNamedrRna(m_TopSeqEntry, "28S ribosomal RNA", ispartial5, ispartial3);
     if (composite){
         m_CmdProccessor->Execute(composite);
     }
}

void CEditingBtnsPanel::AddNamedrRNA26S(wxCommandEvent& event)
{
     bool ispartial5 = true;
     bool ispartial3 = true;
     CRef<CCmdComposite> composite = AddNamedrRna(m_TopSeqEntry, "26S ribosomal RNA", ispartial5, ispartial3);
     if (composite){
         m_CmdProccessor->Execute(composite);
     }
}

void CEditingBtnsPanel::AddNamedrRNASmall(wxCommandEvent& event)
{
     bool ispartial5 = true;
     bool ispartial3 = true;
     CRef<CCmdComposite>  composite = AddNamedrRna(m_TopSeqEntry, "small subunit ribosomal RNA", ispartial5, ispartial3);
     if (composite){
         m_CmdProccessor->Execute(composite);
     }
}

void CEditingBtnsPanel::AddNamedrRNALarge(wxCommandEvent& event)
{
     bool ispartial5 = true;
     bool ispartial3 = true;
     CRef<CCmdComposite>  composite = AddNamedrRna(m_TopSeqEntry, "large subunit ribosomal RNA", ispartial5, ispartial3);
     if (composite){
         m_CmdProccessor->Execute(composite);
     }
}


void CEditingBtnsPanel::AddControlRegion ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite = CreateControlRegion(m_TopSeqEntry);
    if (composite){
        m_CmdProccessor->Execute(composite);
    }
}

void CEditingBtnsPanel::AddMicrosatellite ( wxCommandEvent& event)
{
    CRef<CCmdComposite>  composite = CreateMicrosatellite(m_TopSeqEntry);
    if (composite){
        m_CmdProccessor->Execute(composite);
    }
}

void CEditingBtnsPanel::RemoveUnverified( wxCommandEvent& event )
{
    CRemoveUnverified worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Univerified"))
        event.Skip();
}

void CEditingBtnsPanel::AddDefLine ( wxCommandEvent& event)
{
    CAddDefLineDlg *dlg = new CAddDefLineDlg(NULL, m_Workbench);
    dlg->Show(true);
}


void CEditingBtnsPanel::PrefixOrgToDefLine ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Prefix Organism Name to Definition Line"));
    if (AddOrgToDefline(m_TopSeqEntry, composite))
        m_CmdProccessor->Execute(composite);
}

void CEditingBtnsPanel::PrefixStrainToDefLine ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Prefix Strain to Definition Line"));
    CAutoDefAvailableModifier modifier(COrgMod::eSubtype_strain, true);
    bool show_label = true;
    if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
        m_CmdProccessor->Execute(composite);
}

void CEditingBtnsPanel::PrefixCloneToDefLine ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Prefix Clone to Definition Line"));
    CAutoDefAvailableModifier modifier(CSubSource::eSubtype_clone, false);
    bool show_label = true;
    if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
        m_CmdProccessor->Execute(composite);
}

void CEditingBtnsPanel::PrefixIsolateToDefLine ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Prefix Isolate to Definition Line"));
    CAutoDefAvailableModifier modifier(COrgMod::eSubtype_isolate, true);
    bool show_label = true;
    if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
        m_CmdProccessor->Execute(composite);
}

void CEditingBtnsPanel::PrefixHaplotToDefLine ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Prefix Haplotype to Definition Line"));
    CAutoDefAvailableModifier modifier(CSubSource::eSubtype_haplotype, false);
    bool show_label = true;
    if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
        m_CmdProccessor->Execute(composite);
}

void CEditingBtnsPanel::PrefixCultivarToDefLine ( wxCommandEvent& event)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Prefix Cultivar to Definition Line"));
    CAutoDefAvailableModifier modifier(COrgMod::eSubtype_cultivar, true);
    bool show_label = true;
    if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
        m_CmdProccessor->Execute(composite);
}

void CEditingBtnsPanel::PrefixListToDefLine ( wxCommandEvent& event)
{
    SAutodefParams params;
    unsigned int found = ConfigureParamsForPrefixCombo(params, m_TopSeqEntry);
    CPrefixDeflinedlg dlg(NULL, m_TopSeqEntry, found);
    if (found > 0) {
        dlg.SetParams(&params);
        dlg.PopulateModifierCombo();
    }
    if (dlg.ShowModal() == wxID_OK){
        CRef<CCmdComposite> composite = dlg.GetCommand();
        if (composite){
            m_CmdProccessor->Execute(composite);   
        }
        dlg.Close();
    }
}

void CEditingBtnsPanel::CreateFeature ( wxCommandEvent& event)
{
    if (m_Locations.size() != 1)
    {
        wxMessageBox(_("Please select a single Bioseq"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return;
    }

    CRef<CSeq_loc>  lit =  m_Locations.front();
    CFeatureSubtypeSelector *button = dynamic_cast<CFeatureSubtypeSelector*>(event.GetEventObject());
    if (button)
    {
        CSeqFeatData::ESubtype subtype = button->GetSubtype();
        CRef<CSeq_feat> feat = CBioseqEditor::MakeDefaultFeature(subtype);
        feat->SetLocation().Assign(*lit);
            
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*lit);
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        CIRef<IEditObject> editor(new CEditObjectSeq_feat(*feat, seh, seh.GetScope(), true));
        CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
        edit_dlg->SetUndoManager(m_CmdProccessor);
        wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
        edit_dlg->SetEditorWindow(editorWindow);
        edit_dlg->SetEditor(editor);
        edit_dlg->SetWorkDir(m_WorkDir);
        edit_dlg->Show(true);
    }
    
}


void CEditingBtnsPanel::SegregateSets( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CSegregateSets* dlg = new CSegregateSets(main_window, m_TopSeqEntry, m_CmdProccessor);
    dlg->Show(true);
}

void CEditingBtnsPanel::PT_Cleanup (wxCommandEvent& event)
{
    CRef<CCmdComposite> cmd = InstantiateProteinTitles(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

static string s_GetMacroScriptLocation(const string& filename)
{
    wxFileName macro_dir = wxFileName::DirName(wxStandardPaths::Get().GetDocumentsDir() + wxFileName::GetPathSeparator() + "GbenchMacro");

    if (!macro_dir.DirExists()) {
        if (!macro_dir.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
            LOG_POST(Error << "Cannot create directory '" << string(macro_dir.GetPath().ToUTF8()) << "'");
            return kEmptyStr;
        }
    }

    wxFileName macro_file(macro_dir.GetPath(), filename);
    if (!macro_file.FileExists()) {        
        const string orig_location = "<std>/etc/macro_scripts/";
        wxFileName orig_path = CSysPath::ResolvePath(orig_location);
        if (!orig_path.DirExists()) {
            return kEmptyStr;
        }

        wxFileName orig_file(orig_path.GetPath(), filename);
        wxFileName dst_file(macro_dir.GetPath(), filename);
        if (!wxCopyFile(orig_file.GetFullPath(), dst_file.GetFullPath())) {
            LOG_POST(Error << "Cannot copy '" << filename << "' to location '" << string(macro_dir.GetPath().ToUTF8()) << "'");
            return kEmptyStr;
        }
    }
    return string(macro_file.GetFullPath().ToUTF8());
}

static Uint4 s_GetCheckSumForFile(const string& filename)
{
    CChecksum chksum(CChecksum::eCRC32);
    chksum.AddFile(filename);
    return chksum.GetChecksum();
}

static string s_GetUsername()
{
    char username[1024];
    CORE_GetUsername(username, sizeof(username));
    return string(username);
}

static void s_UpdateMacros()
{
    string orig_dir_path{ wxFileName::DirName(CSysPath::ResolvePath(wxT("<std>/etc/macro_scripts"))).GetPath().ToUTF8() };
    string upd_dir_path{ wxFileName::DirName(CSysPath::ResolvePath(wxT("<std>/etc/macro_scripts/Updates"))).GetPath().ToUTF8() };
    
    if (orig_dir_path.empty() || upd_dir_path.empty()) {
        return;
    }

    CDir dir(upd_dir_path);
    CDir::TEntries files(dir.GetEntries("*.mql", CDir::eFile));
    for (auto& it : files) {
        if (!it->IsFile())
            continue;

        string filename = it->GetName();
        string upd_filename = CDirEntry::MakePath(upd_dir_path, filename);
        auto upd_chksum = s_GetCheckSumForFile(upd_filename);

        string orig_filename = CDirEntry::MakePath(orig_dir_path, filename);
        auto orig_chksum = s_GetCheckSumForFile(orig_filename);

        wxFileName lcl_dir = wxFileName::DirName(wxStandardPaths::Get().GetDocumentsDir() + wxFileName::GetPathSeparator() + "GbenchMacro");
        const string lcl_filename = CDirEntry::ConcatPathEx(string(lcl_dir.GetPath().ToUTF8()), filename);
        auto lcl_chksum = s_GetCheckSumForFile(lcl_filename);

        if (lcl_chksum == upd_chksum) {
            // do nothing
        }
        else if (lcl_chksum == orig_chksum) {
            // update
            CFile lcl(lcl_filename);
            lcl.Remove(CDirEntry::eEntryOnly);
            CFile upd(upd_filename);
            if (upd.CopyToDir(string(lcl_dir.GetFullPath().ToUTF8()))) {
                string username = s_GetUsername();
                if (!username.empty()) {
                    NMacroStats::ReportMacroToBeUpdated(username, filename, "updated");
                }
            } else {
                LOG_POST(Error << "Cannot copy '" << upd_filename << "' to location '" << string(lcl_dir.GetFullPath().ToUTF8()) << "'");
            }
        }
        else {
            // report to applog
            string username = s_GetUsername();
            if (!username.empty()) {
                NMacroStats::ReportMacroToBeUpdated(username, filename, "manual_update");
            }
        }
    }
}


void CEditingBtnsPanel::AddDefaultMacroButtons()
{
    m_dynamic_buttons_filename[ID_BUTTON46] = s_GetMacroScriptLocation("flu_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON46] = "Flu Segment";
    m_dynamic_buttons_title[ID_BUTTON46] = "Flu Segment";

    m_dynamic_buttons_filename[ID_BUTTON47] = s_GetMacroScriptLocation("gbmito_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON47] = "GB_mito Macro";
    m_dynamic_buttons_title[ID_BUTTON47] = "GB_mito";

    m_dynamic_buttons_filename[ID_BUTTON104] = s_GetMacroScriptLocation("fixsource_underscores.mql");
    m_dynamic_buttons_name[ID_BUTTON104] = "FixSrcUnderscores";
    m_dynamic_buttons_title[ID_BUTTON104] = "FixSourceUnderscores";

    m_dynamic_buttons_filename[ID_BUTTON106] = s_GetMacroScriptLocation("remove_location.mql");
    m_dynamic_buttons_name[ID_BUTTON106] = "Remove Location";
    m_dynamic_buttons_title[ID_BUTTON106] = "Remove Location";

    m_dynamic_buttons_filename[ID_BUTTON112] = s_GetMacroScriptLocation("remove_fulllength.mql");
    m_dynamic_buttons_name[ID_BUTTON112] = "RemoveFullLength";
    m_dynamic_buttons_title[ID_BUTTON112] = "Remove (CDS, gene, miscRNA, rRNA) Full Length";

    m_dynamic_buttons_filename[ID_BUTTON114] = s_GetMacroScriptLocation("uncultured_taxtool.mql");
    m_dynamic_buttons_name[ID_BUTTON114] = "UnculTax Macro";
    m_dynamic_buttons_title[ID_BUTTON114] = "Uncultured Tax Tool";

    m_dynamic_buttons_filename[ID_BUTTON116] = s_GetMacroScriptLocation("parse2strain.mql");
    m_dynamic_buttons_name[ID_BUTTON116] = "Parse2Strain";
    m_dynamic_buttons_title[ID_BUTTON116] = "Parse to strain";

    m_dynamic_buttons_filename[ID_BUTTON119] = s_GetMacroScriptLocation("remove_xtrageneinfo.mql");
    m_dynamic_buttons_name[ID_BUTTON119] = "RmvXtraGeneInfo";
    m_dynamic_buttons_title[ID_BUTTON119] = "Remove extra gene info";

    m_dynamic_buttons_filename[ID_BUTTON120] = s_GetMacroScriptLocation("remove_molcomplete.mql");
    m_dynamic_buttons_name[ID_BUTTON120] = "RmvMolComplete";
    m_dynamic_buttons_title[ID_BUTTON120] = "RemoveMolComplete";

    m_dynamic_buttons_filename[ID_BUTTON121] = s_GetMacroScriptLocation("apply_gdna.mql");
    m_dynamic_buttons_name[ID_BUTTON121] = "Apply-gDNA";
    m_dynamic_buttons_title[ID_BUTTON121] = "Apply-gDNA";

    m_dynamic_buttons_filename[ID_BUTTON122] = s_GetMacroScriptLocation("cytb_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON122] = "Cytbmacro";
    m_dynamic_buttons_title[ID_BUTTON122] = "CytbMacro";

    m_dynamic_buttons_filename[ID_BUTTON124] = s_GetMacroScriptLocation("join_short_trna.mql");
    m_dynamic_buttons_name[ID_BUTTON124] = "Join Short tRNAs";
    m_dynamic_buttons_title[ID_BUTTON124] = "Join short tRNAs";

    m_dynamic_buttons_filename[ID_BUTTON126] = s_GetMacroScriptLocation("gpipe_uncult_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON126] = "GPIPE Uncultured";
    m_dynamic_buttons_title[ID_BUTTON126] = "Composite Uncultured Tax Tool (GPIPE)";

    m_dynamic_buttons_filename[ID_BUTTON128] = s_GetMacroScriptLocation("rRNA_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON128] = "rRNAmacro";
    m_dynamic_buttons_title[ID_BUTTON128] = "rRNA";

    m_dynamic_buttons_filename[ID_BUTTON129] = s_GetMacroScriptLocation("personal_Susan.mql");
    m_dynamic_buttons_name[ID_BUTTON129] = "Personal macro";
    m_dynamic_buttons_title[ID_BUTTON129] = "Personal macro";

    m_dynamic_buttons_filename[ID_BUTTON131] = s_GetMacroScriptLocation("apply_mito.mql");
    m_dynamic_buttons_name[ID_BUTTON131] = "Apply Mito";
    m_dynamic_buttons_title[ID_BUTTON131] = "Apply Mito";

    m_dynamic_buttons_filename[ID_BUTTON135] = s_GetMacroScriptLocation("gpipe_cultured_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON135] = "GPIPE Cultured";
    m_dynamic_buttons_title[ID_BUTTON135] = "Cultured Tax Tool";

    m_dynamic_buttons_filename[ID_BUTTON136] = s_GetMacroScriptLocation("parse_ATCC.mql");
    m_dynamic_buttons_name[ID_BUTTON136] = "Parse ATCC";
    m_dynamic_buttons_title[ID_BUTTON136] = "Parse ATCC";

    m_dynamic_buttons_filename[ID_BUTTON141] = s_GetMacroScriptLocation("gpipe_ITS_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON141] = "GPIPE ITS";
    m_dynamic_buttons_title[ID_BUTTON141] = "ITS";

    m_dynamic_buttons_filename[ID_BUTTON146] = s_GetMacroScriptLocation("tRNA_codon.mql");
    m_dynamic_buttons_name[ID_BUTTON146] = "tRNAcodon Macro";
    m_dynamic_buttons_title[ID_BUTTON146] = "tRNAcodon";

    m_dynamic_buttons_filename[ID_BUTTON147] = s_GetMacroScriptLocation("tRNA_macro.mql");
    m_dynamic_buttons_name[ID_BUTTON147] = "tRNAmacro";
    m_dynamic_buttons_title[ID_BUTTON147] = "tRNA";

    m_dynamic_buttons_filename[ID_BUTTON148] = s_GetMacroScriptLocation("chloroprod.mql");
    m_dynamic_buttons_name[ID_BUTTON148] = "chloroprod";
    m_dynamic_buttons_title[ID_BUTTON148] = "Chloroprod";

    m_dynamic_buttons_filename[ID_BUTTON45] = s_GetMacroScriptLocation("altitude_fix.mql");
    m_dynamic_buttons_name[ID_BUTTON45] = "Fix Altitude";

    m_dynamic_buttons_filename[ID_BUTTON78] = s_GetMacroScriptLocation("apply_genbank_to_wgs_set.mql");
    m_dynamic_buttons_name[ID_BUTTON78] = "WGS to Genbank";

    m_dynamic_buttons_filename[ID_BUTTON105] = s_GetMacroScriptLocation("lowercase_isolationsrc.mql");
    m_dynamic_buttons_name[ID_BUTTON105] = "LowerIsolSrc";

    m_dynamic_buttons_filename[ID_BUTTON107] = s_GetMacroScriptLocation("localid_to_clone.mql");
    m_dynamic_buttons_name[ID_BUTTON107] = "LocalID_Clone";

    m_dynamic_buttons_filename[ID_BUTTON108] = s_GetMacroScriptLocation("localid_to_isolate.mql");
    m_dynamic_buttons_name[ID_BUTTON108] = "LocalID_Isolate";

    m_dynamic_buttons_filename[ID_BUTTON109] = s_GetMacroScriptLocation("localid_to_strain.mql");
    m_dynamic_buttons_name[ID_BUTTON109] = "LocalID_Strain";

    m_dynamic_buttons_filename[ID_BUTTON111] = s_GetMacroScriptLocation("remove_intronlessgene.mql");
    m_dynamic_buttons_name[ID_BUTTON111] = "RmvIntronlessGene";

    m_dynamic_buttons_filename[ID_BUTTON117] = s_GetMacroScriptLocation("remove_identifiedby.mql");
    m_dynamic_buttons_name[ID_BUTTON117] = "Rmv IdentifiedBy";

    m_dynamic_buttons_filename[ID_BUTTON118] = s_GetMacroScriptLocation("remove_gene.mql");
    m_dynamic_buttons_name[ID_BUTTON118] = "Rmv Gene";

    m_dynamic_buttons_filename[ID_BUTTON123] = s_GetMacroScriptLocation("TSAnote.mql");
    m_dynamic_buttons_name[ID_BUTTON123] = "TSANote";

    m_dynamic_buttons_filename[ID_BUTTON130] = s_GetMacroScriptLocation("flip_host.mql");
    m_dynamic_buttons_name[ID_BUTTON130] = "Flip Host";

    m_dynamic_buttons_filename[ID_BUTTON139] = s_GetMacroScriptLocation("complete_phage.mql");
    m_dynamic_buttons_name[ID_BUTTON139] = "Complete Phage";

    m_dynamic_buttons_filename[ID_BUTTON143] = s_GetMacroScriptLocation("parse_bla_allele.mql");
    m_dynamic_buttons_name[ID_BUTTON143] = "Parse bla allele";

    m_dynamic_buttons_filename[ID_BUTTON144] = s_GetMacroScriptLocation("fix_allauthors_affil.mql");
    m_dynamic_buttons_name[ID_BUTTON144] = "FixAllAuthorsAffil";

    m_dynamic_buttons_filename[ID_BUTTON145] = s_GetMacroScriptLocation("comcirc.mql");
    m_dynamic_buttons_name[ID_BUTTON145] = "ComCirc Macro";

    m_dynamic_buttons_filename[ID_BUTTON151] = s_GetMacroScriptLocation("set_gene_partial.mql");
    m_dynamic_buttons_name[ID_BUTTON151] = "SetGenePartial";

    m_dynamic_buttons_filename[ID_BUTTON152] = s_GetMacroScriptLocation("set_partial_extend.mql");
    m_dynamic_buttons_name[ID_BUTTON152] = "SetPartialAndExtend";

    m_dynamic_buttons_filename[ID_BUTTON153] = s_GetMacroScriptLocation("PB1_genexref.mql");
    m_dynamic_buttons_name[ID_BUTTON153] = "PB1 GeneXref";

    m_dynamic_buttons_filename[ID_BUTTON154] = s_GetMacroScriptLocation("remove_all_add16S.mql");
    m_dynamic_buttons_name[ID_BUTTON154] = "RmvAllAdd16S";

    m_dynamic_buttons_filename[ID_BUTTON155] = s_GetMacroScriptLocation("add16S.mql");
    m_dynamic_buttons_name[ID_BUTTON155] = "Add16S";

    m_dynamic_buttons_filename[ID_BUTTON157] = s_GetMacroScriptLocation("isolate_to_strain.mql");
    m_dynamic_buttons_name[ID_BUTTON157] = "IsolateToStrain";

    m_dynamic_buttons_filename[ID_BUTTON158] = s_GetMacroScriptLocation("remove_assembly_name.mql");
    m_dynamic_buttons_name[ID_BUTTON158] = "RmvAssemblyName";

    m_dynamic_buttons_filename[ID_BUTTON165] = s_GetMacroScriptLocation("extendCDSgene_nearEnds.mql");
    m_dynamic_buttons_name[ID_BUTTON165] = "ExtendCDSgeneNE";

    m_dynamic_buttons_filename[ID_BUTTON166] = s_GetMacroScriptLocation("fixRNAITSpartials.mql");
    m_dynamic_buttons_name[ID_BUTTON166] = "FixRNAITSpartials";

    m_dynamic_buttons_filename[ID_BUTTON161] = s_GetMacroScriptLocation("measles_sourceH.mql");
    m_dynamic_buttons_name[ID_BUTTON161] = "MeaslesSourceH";
    m_dynamic_buttons_title[ID_BUTTON161] = "MeaslesSourceH";

    m_dynamic_buttons_filename[ID_BUTTON162] = s_GetMacroScriptLocation("measles_sourceN.mql");
    m_dynamic_buttons_name[ID_BUTTON162] = "MeaslesSourceN";
    m_dynamic_buttons_title[ID_BUTTON162] = "MeaslesSourceN";

    m_dynamic_buttons_filename[ID_BUTTON163] = s_GetMacroScriptLocation("remove_location_wotaxlookup.mql");
    m_dynamic_buttons_name[ID_BUTTON163] = "RemoveLoc(noTax)";

    m_dynamic_buttons_filename[ID_BUTTON164] = s_GetMacroScriptLocation("rubella_sourceSH.mql");
    m_dynamic_buttons_name[ID_BUTTON164] = "RubellaSourceSH";
    m_dynamic_buttons_title[ID_BUTTON164] = "RubellaSourceSH";

    m_dynamic_buttons_filename[ID_BUTTON167] = s_GetMacroScriptLocation("trimgenelocus2.mql");
    m_dynamic_buttons_name[ID_BUTTON167] = "TrimGeneLocus2";

    m_dynamic_buttons_filename[ID_BUTTON168] = s_GetMacroScriptLocation("trimgenelocus4.mql");
    m_dynamic_buttons_name[ID_BUTTON168] = "TrimGeneLocus4";

    m_dynamic_buttons_filename[ID_BUTTON169] = s_GetMacroScriptLocation("rmvorg_from_strain.mql");
    m_dynamic_buttons_name[ID_BUTTON169] = "RmOrgFromStrain";

    m_dynamic_buttons_filename[ID_BUTTON170] = s_GetMacroScriptLocation("rmvorg_from_isolate.mql");
    m_dynamic_buttons_name[ID_BUTTON170] = "RmOrgFromIsolate";

    s_UpdateMacros();
}

void CEditingBtnsPanel::RunMacroWithReport( wxCommandEvent& event )
{
    string title, filename;
    CMacroWorker::TVecMacroNames macros;
    bool add_user_object = false;

    switch (event.GetId()) {      
    case ID_BUTTON132:
        macros = CMacroWorker::s_GetMacroSteps(CMacroWorker::sm_AutofixWGS);
        title = "Autofix(WGS)";
        break;
    case ID_BUTTON137:
        macros = CMacroWorker::s_GetMacroSteps(CMacroWorker::sm_AutofixGB);
        title = "Autofix";
        add_user_object = true;
        break;
    case ID_BUTTON171:
        macros = CMacroWorker::s_GetMacroSteps(CMacroWorker::sm_AutofixTSA);
        title = "Autofix(TSA)";
        break;
    default:
        break;
    }

    if (m_dynamic_buttons_filename.find(event.GetId()) != m_dynamic_buttons_filename.end())
    {
    filename = m_dynamic_buttons_filename[event.GetId()];
    }
    if (m_dynamic_buttons_title.find(event.GetId()) != m_dynamic_buttons_title.end())
    title = m_dynamic_buttons_title[event.GetId()];

    if (macros.empty() && filename.empty()) {
        NcbiMessageBox("The macro script was not found.");
        return;
    }

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    const string kMacroReport(" Macro Report");
    CGenericReportDlg* report = 0;
    ITERATE(wxWindowList, wit, main_window->GetChildren()){
        wxWindow* child = *wit;
        if (NStr::EndsWith(ToStdString(child->GetLabel()), kMacroReport)) {
            report = wxDynamicCast(child, CGenericReportDlg);
            break;
        }
    }
    
    CNcbiOstrstream log;
    bool status = false;
    CMacroWorker worker(m_TopSeqEntry, m_SeqSubmit, m_CmdProccessor);
    if (!macros.empty()) {
        status = worker.ExecuteMacros(macros, log, add_user_object);
    }
    else if (!filename.empty()) {
        status = worker.ExecuteMacros(filename, log, add_user_object);
    }

    string report_msg = CNcbiOstrstreamToString(log);
    
    if (!status || report_msg.empty()) {
        report_msg.assign("Macro had no effect");
    }

    title.append(kMacroReport);
    if (report) {
        report->SetTitle(ToWxString(title));
        report->SetText(ToWxString(report_msg));
        report->RefreshData();
        report->SetFocus();
    } else {
        report = new CGenericReportDlg(main_window, m_Workbench, wxID_ANY, ToWxString(title));
        report->SetText(ToWxString(report_msg));
        report->Show(true);
    }
}

void CEditingBtnsPanel::RunMacro ( wxCommandEvent& event )
{
    vector<string> macro;
    string filename;
    bool add_user_object = false;
    bool resolve_vars = false;
    switch (event.GetId()) {
    case ID_BUTTON44:
        macro = CMacroWorker::s_GetMacroSteps(CMacroWorker::sm_AutofixGB);
        add_user_object = true;
        break;           
    case ID_BUTTON143: 
        resolve_vars = true;
        break;             
    }

    if (m_dynamic_buttons_filename.find(event.GetId()) != m_dynamic_buttons_filename.end())
    {
    filename = m_dynamic_buttons_filename[event.GetId()];
    }

    if (macro.empty() && filename.empty()) {
        NcbiMessageBox("The macro script was not found.");
        return;
    }
    
    CNcbiOstrstream log;
    CMacroWorker worker(m_TopSeqEntry, m_SeqSubmit, m_CmdProccessor);
    if (!macro.empty()) {
        worker.ExecuteMacros(macro, log, add_user_object);
    }
    else if (!filename.empty()) {
        worker.ExecuteMacros(filename, log, add_user_object, resolve_vars);
    }
}

void CEditingBtnsPanel::AddSet( wxCommandEvent& event )
{
    CChooseSetClassDlg dlg(NULL);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CBioseq_set::EClass new_class = dlg.GetClass();
        CRef<CCmdInsertBioseqSet> cmd(new CCmdInsertBioseqSet(m_TopSeqEntry, new_class));
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
    }        
}

void CEditingBtnsPanel::AddGSet( wxCommandEvent& event )
{
    CBioseq_set::EClass new_class = ncbi::objects::CBioseq_set::eClass_genbank;
    CRef<CCmdInsertBioseqSet> cmd(new CCmdInsertBioseqSet(m_TopSeqEntry, new_class));
    if (cmd)  m_CmdProccessor->Execute(cmd);                       
}

void CEditingBtnsPanel::RetranslateCDS( wxCommandEvent& evt )
{

    CRetranslateCDS worker;
    worker.apply( m_TopSeqEntry, m_CmdProccessor, "Retranslate coding regions", CRetranslateCDS::eRetranslateCDSIgnoreStopExceptEnd); 
}

void CEditingBtnsPanel::RetranslateCDS_NoStp( wxCommandEvent& evt )
{
    CRetranslateCDS worker;
    worker.apply( m_TopSeqEntry, m_CmdProccessor, "Retranslate coding regions", CRetranslateCDS::eRetranslateCDSChooseFrame); 
}


void CEditingBtnsPanel::OnCallerTool(wxCommandEvent& event)
{
    if (m_TopSeqEntry) {
        CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eOncaller, m_TopSeqEntry.GetScope(), m_Workbench, m_WorkDir);
    }
};

void CEditingBtnsPanel::SelectTarget( wxCommandEvent& event )
{
    CSelectTargetDlg * dlg = new CSelectTargetDlg(NULL, m_Workbench);
    dlg->Show();
}

void CEditingBtnsPanel::UnculTaxTool( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    try
    {
        CUnculTaxTool *dlg = new CUnculTaxTool(main_window, m_TopSeqEntry, m_Workbench);
        dlg->Show(true);      
    } catch  (CException&) {}
}

void CEditingBtnsPanel::RemoveSrcNotes( wxCommandEvent& evt )
{
    CRemoveSrcNotes worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CEditingBtnsPanel::AddIsolSource( wxCommandEvent& evt )
{
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_Source;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);

    dlg->SetInitParameters(action_type, field_type, "isolation-source");
    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::ExtendPartialsConstr( wxCommandEvent& evt )
{
    CExtendPartialFeatDlg * dlg = new CExtendPartialFeatDlg(NULL, m_Workbench);
    dlg->Show(true);
}

void CEditingBtnsPanel::ExtendPartialsAll( wxCommandEvent& evt )
{
    CExtendPartialFeatDlg * dlg = new CExtendPartialFeatDlg(NULL, m_Workbench);
    dlg->ApplyToAllAndDie();
}

void CEditingBtnsPanel::ConvertCdsToMiscFeat( wxCommandEvent& evt )
{
    CConvertCdsToMiscFeat * dlg = new CConvertCdsToMiscFeat(NULL, m_Workbench);
    dlg->Show(true);
}

void CEditingBtnsPanel::LowercaseQuals( wxCommandEvent& evt )
{
    CLowerQuals* dlg = new CLowerQuals(NULL, m_Workbench);
    dlg->Show(true);
}


void CEditingBtnsPanel::RemoveTextOutsideString( wxCommandEvent& evt )
{
 
    CAECRFrame::EActionType action_type = CAECRFrame::eActionType_RemoveTextOutside;
    CAECRFrame::EFieldType field_type = CAECRFrame::eFieldType_Misc;
     
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench);
    dlg->SetInitParameters(action_type, field_type);

    CallAfter(&CEditingBtnsPanel::ShowAfter, dlg);
}

void CEditingBtnsPanel::CorrectGenes( wxCommandEvent& event )
{
    CCorrectGenesDialog * dlg = new CCorrectGenesDialog(NULL, m_Workbench);
    dlg->Show(true);
}


void CEditingBtnsPanel::CreateDescGenomeAssemblyComment ( wxCommandEvent& evt)
{
    CSeq_entry_Handle seh = m_TopSeqEntry;
    if (!m_Locations.empty()) 
    {
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*m_Locations.front());
        seh = bsh.GetSeq_entry_Handle();
    }

    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetUser().Assign(*edit::CGenomeAssemblyComment::MakeEmptyUserObject());
   
    CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(m_WorkDir);
    edit_dlg->Show(true);
}

void CEditingBtnsPanel::ECNumberCleanup( wxCommandEvent& evt )
{
    CRef<CCmdComposite> cmd(new CCmdComposite("EC Number Cleanup"));
    for (CFeat_CI feat_ci(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::e_Prot)); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(orig);

        if (new_feat->IsSetData() && new_feat->GetData().IsProt())
        {
            new_feat->SetData().SetProt().AutoFixEC();
            new_feat->SetData().SetProt().RemoveBadEC();
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
        }
    }
    m_CmdProccessor->Execute(cmd);
}

CSeq_entry_Handle CEditingBtnsPanel::x_GetSelectedSeq()
{
    CSeq_entry_Handle seh;

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }
    if (objects.empty()) 
    {
        wxMessageBox(_("Select a single Seq-entry"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return seh;
    }

    vector<CBioseq_Handle> bioseqs;
    CScope &scope = m_TopSeqEntry.GetScope();
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) 
    {
        const CObject* ptr = it->object.GetPointer();
        //cout << "Counter: "  <<  typeid(*ptr).name() << endl;

        const objects::CBioseq* bioseq = dynamic_cast<const objects::CBioseq*>(ptr);
        if (bioseq)
        {
            CBioseq_Handle bsh = scope.GetBioseqHandle(*bioseq,CScope::eMissing_Null);
            if (bsh)
            {
                bioseqs.push_back(bsh);
            }
        }
        const objects::CSeq_loc* loc = dynamic_cast<const objects::CSeq_loc*>(ptr);
        if (loc)
        {
            CBioseq_Handle bsh = scope.GetBioseqHandle(*loc);
            if (bsh)
            {
                bioseqs.push_back(bsh);
            }
        }      
    }

    if (bioseqs.size() == 1)
        seh = bioseqs.front().GetSeq_entry_Handle();

    if (!seh)
    {
        wxMessageBox(_("No Seq-entry selected"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return seh;
    }
    return seh;
}


void CEditingBtnsPanel::Add_rRNA( wxCommandEvent& evt )
{
    CSeq_entry_Handle seh = x_GetSelectedSeq();
    if (!seh)
        return;

    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, seh, m_Workbench);
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_rRNA);
    dlg->Show(true);
}

void CEditingBtnsPanel::Add_misc_feat( wxCommandEvent& evt )
{
    CSeq_entry_Handle seh = x_GetSelectedSeq();
    if (!seh)
        return;

    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, seh, m_Workbench, "misc_feature");
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_misc_feature);
    dlg->Show(true);
}

void CEditingBtnsPanel::Add_gene( wxCommandEvent& evt )
{

    CSeq_entry_Handle seh = x_GetSelectedSeq();
    if (!seh)
        return;

    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, seh, m_Workbench, "Gene");
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_gene);
    dlg->Show(true);
}

void CEditingBtnsPanel::Save( wxCommandEvent& evt ) // same as File -> Export menu
{
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv)
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }
    if (objects.empty()) return;
    CAppDialogs::ExportDialog(m_Workbench);
}

void CEditingBtnsPanel::ExportTable(wxCommandEvent& evt)
{
    CExportTableDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        dlg.SaveTable(m_WorkDir);
    }
}

void CEditingBtnsPanel::SequesterSets( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CSequesterSets * dlg = new CSequesterSets(main_window, m_TopSeqEntry, m_SeqSubmit, m_Workbench);
    dlg->Show(true);      
}

void CEditingBtnsPanel::EditSubmitBlock( wxCommandEvent& event)
{
    if (!m_SeqSubmit || !m_SeqSubmit->IsSetSub())
        return;

    /* TConstScopedObjects objects;
    objects.push_back(SConstScopedObject(m_SeqSubmit->GetSub(), m_TopSeqEntry.GetScope()));
    CSimpleProjectView :: CreateView("Edit Object Dialog View", CEditObjViewDlg::CreateDialogEdit, m_Workbench, objects); 
    */
    CIRef<IEditObject> editor(new CEditObjectSubmit_block(m_SeqSubmit->GetSub(), m_TopSeqEntry.GetScope(), false));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, false);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(m_WorkDir);
    edit_dlg->Show(true);
}

void CEditingBtnsPanel::rRNAtoDNA( wxCommandEvent& event)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("rRNA to DNA") );
    bool modified = false;
    for ( CBioseq_CI seq_it(m_TopSeqEntry, CSeq_inst::eMol_rna); seq_it; ++seq_it ) 
    {
        for ( CSeqdesc_CI desc_it(*seq_it, CSeqdesc::e_Molinfo, 1); desc_it; ++desc_it) 
        {
            if (desc_it->GetMolinfo().IsSetBiomol() && desc_it->GetMolinfo().GetBiomol() == CMolInfo::eBiomol_rRNA)
            {                
                CRef<CSeqdesc> edited_desc(new CSeqdesc);
                edited_desc->Assign(*desc_it);
                edited_desc->SetMolinfo().SetBiomol(CMolInfo::eBiomol_genomic);
                CRef<CCmdChangeSeqdesc> change_molinfo(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *edited_desc));
                cmd->AddCommand(*change_molinfo);
            }
        }

        CRef<CSerialObject> edited_object;
        edited_object.Reset((CSerialObject*)CSeq_inst::GetTypeInfo()->Create());
        edited_object->Assign(seq_it->GetInst());                
        CSeq_inst& edited_inst = dynamic_cast<CSeq_inst&>(*edited_object);
        edited_inst.SetMol(CSeq_inst::eMol_dna);
        edited_inst.ResetStrand();
        edited_inst.SetTopology(CSeq_inst::eTopology_not_set);
        CCmdChangeBioseqInst* change_inst = new CCmdChangeBioseqInst(*seq_it, edited_inst);
        cmd->AddCommand(*change_inst);
        modified = true;
    }

    for ( CBioseq_CI seq_it(m_TopSeqEntry, CSeq_inst::eMol_na); seq_it; ++seq_it ) 
    {
        bool found = false;
        for ( CSeqdesc_CI desc_it(*seq_it, CSeqdesc::e_Molinfo, 1); desc_it; ++desc_it) 
        {
            if (desc_it->GetMolinfo().IsSetBiomol() && desc_it->GetMolinfo().GetBiomol() == CMolInfo::eBiomol_rRNA)
            {                
                CRef<CSeqdesc> edited_desc(new CSeqdesc);
                edited_desc->Assign(*desc_it);
                edited_desc->SetMolinfo().SetBiomol(CMolInfo::eBiomol_genomic);
                CRef<CCmdChangeSeqdesc> change_molinfo(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *edited_desc));
                cmd->AddCommand(*change_molinfo);
                found = true;
            }
        }
        if (found)
        {
            CRef<CSerialObject> edited_object;
            edited_object.Reset((CSerialObject*)CSeq_inst::GetTypeInfo()->Create());
            edited_object->Assign(seq_it->GetInst());                
            CSeq_inst& edited_inst = dynamic_cast<CSeq_inst&>(*edited_object);
            edited_inst.SetMol(CSeq_inst::eMol_dna);
            edited_inst.ResetStrand();
            edited_inst.SetTopology(CSeq_inst::eTopology_not_set);
            CCmdChangeBioseqInst* change_inst = new CCmdChangeBioseqInst(*seq_it, edited_inst);
            cmd->AddCommand(*change_inst);
            modified = true;
        }
    }
    if (modified)
        m_CmdProccessor->Execute(cmd.GetPointer());
}

void CEditingBtnsPanel::WrongQualsToNote( wxCommandEvent& event)
{   
    CWrongIllegalQuals::WrongQualsToNote( m_TopSeqEntry, m_CmdProccessor);  
}

void CEditingBtnsPanel::IllegalQualsToNote( wxCommandEvent& event)
{
    CWrongIllegalQuals::IllegalQualsToNote( m_TopSeqEntry, m_CmdProccessor);  
}

void CEditingBtnsPanel::RmIllegalQuals( wxCommandEvent& event)
{
    CWrongIllegalQuals::RmIllegalQuals( m_TopSeqEntry, m_CmdProccessor);  
}

void CEditingBtnsPanel::RmWrongQuals( wxCommandEvent& event)
{
    CWrongIllegalQuals::RmWrongQuals( m_TopSeqEntry, m_CmdProccessor);  
}

void CEditingBtnsPanel::RmWrongOrIllegalQuals( wxCommandEvent& event)
{
    CWrongIllegalQuals::RmWrongOrIllegalQuals( m_TopSeqEntry, m_CmdProccessor);  
}


void CEditingBtnsPanel::GroupExplode( wxCommandEvent& event)
{
    CGroupExplode::apply( m_Workbench, m_CmdProccessor, m_TopSeqEntry.GetScope());  
}

void CEditingBtnsPanel::RemoveTextInsideStr( wxCommandEvent& evt )
{
    CRemoveTextInsideStrDlg * dlg = new CRemoveTextInsideStrDlg(NULL, m_Workbench);
    dlg->Show(true);
}

void CEditingBtnsPanel::FindASN1( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CFindASN1Dlg * dlg = new CFindASN1Dlg(main_window, m_TopSeqEntry, m_CmdProccessor, m_SeqSubmit);
    dlg->Show(true);
}

void CEditingBtnsPanel::RemoveDupFeats( wxCommandEvent& evt )
{
    CRemoveFeaturesDlg::RemoveDuplicateFeatures(m_TopSeqEntry, m_CmdProccessor);
}

void CEditingBtnsPanel::UpdateSingleSeqClipboard(wxCommandEvent& evt)
{
    CSeqUpdateWorker worker;
    worker.SetSeqUpdateType(CSeqUpdateWorker::eSingle_Clipboard);
    worker.UpdateSingleSequence(m_Workbench, m_TopSeqEntry, m_CmdProccessor);
}

void CEditingBtnsPanel::UpdateMultSeqClipboard(wxCommandEvent& evt)
{
    CSeqUpdateWorker worker;
    worker.SetSeqUpdateType(CSeqUpdateWorker::eMultiple_Clipboard);
    worker.UpdateMultipleSequences(m_Workbench, m_TopSeqEntry, m_CmdProccessor);
}

void CEditingBtnsPanel::ShowFeatureTable(wxCommandEvent& event)
{
    CFeatureTblFr* frame = new CFeatureTblFr(NULL, m_Workbench);
    frame->SetWorkDir(m_WorkDir);
    frame->Show(true);
}

void CEditingBtnsPanel::OnMegaReport(wxCommandEvent& event)
{
    if (m_TopSeqEntry) {
        CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eMega, m_TopSeqEntry.GetScope(), m_Workbench, m_WorkDir);
    }
};

void CEditingBtnsPanel::ImportFeatureTable(wxCommandEvent& event)
{
    CImportFeatTable worker(m_TopSeqEntry);
    CIRef<IEditCommand> cmd = worker.ImportFeaturesFromFile(m_WorkDir, "file_loader_5col");
    if (!cmd)
        return;

    m_CmdProccessor->Execute(cmd);
}

void CEditingBtnsPanel::ImportFeatureTableClipboard(wxCommandEvent& event)
{
    CImportFeatTable worker(m_TopSeqEntry);
    CIRef<IEditCommand> cmd = worker.ImportFeaturesFromClipboard(m_WorkDir);
    if (!cmd)
        return;

    m_CmdProccessor->Execute(cmd);
}

void CEditingBtnsPanel::LaunchDesktop(wxCommandEvent& event)
{
    CRef<CObjectFor<TConstScopedObjects>> objects(new CObjectFor<TConstScopedObjects>);
    if (m_SeqSubmit) {
        objects->GetData().push_back(SConstScopedObject(m_SeqSubmit, &m_TopSeqEntry.GetScope()));
    }
    else {
        objects->GetData().push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteObject(), &m_TopSeqEntry.GetScope()));
    }

    if (!objects->GetData().front().object)
        return;

    static const string sViewType("Sequin Desktop View");


    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    if (prjSrv) {
        prjSrv->AddProjectView(sViewType, objects->GetData(), 0).GetPointer();
    }
    /*
    if (prjSrv) {
        vector<CIRef<IProjectView>> views;
        prjSrv->FindViews(views);
        auto count = 0;
        
        CIRef<IProjectView> desktop_view;
        ITERATE(vector<CIRef<IProjectView> >, it, views) {
            CIRef<IProjectView> view = *it;
            if (view->GetLabel(IProjectView::eType) == sViewType) {
                desktop_view = view;
                count++;
            }
        }
        if (desktop_view && count == 1) {
            prjSrv->ActivateProjectView(desktop_view);
            CSequinDesktopView* desktop = dynamic_cast<CSequinDesktopView*>(desktop_view.GetPointer());
            CDesktopWidgetEvent refresh_evt(CEvent::eEvent_Message, CDesktopWidgetEvent::eRefreshData);
            desktop->Send(&refresh_evt);
        }
        else {
            prjSrv->AddProjectView(sViewType, objects->GetData(), 0).GetPointer();
        }
    }
    */
}

void CEditingBtnsPanel::BarcodeTool( wxCommandEvent& event )
{
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CBarcodeTool *dlg(NULL);
    try
    {
        dlg = new CBarcodeTool(main_window, m_TopSeqEntry, m_CmdProccessor, m_Workbench);
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
    }   
}

void CEditingBtnsPanel::EditSequence( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }   
    
    if(objects.empty()) 
    {
        wxMessageBox(wxT("No objects found"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }

    CRef<CScope> scope(&GetTopSeqEntryFromScopedObject(objects[0]).GetScope());
    if (!scope)
    {
        wxMessageBox(wxT("Cannot determine scope"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }
    const CSeq_loc* loc = NULL;
    size_t count = 0;
    for (auto &o : objects)
    {
        const CSeq_loc* obj_loc = dynamic_cast<const CSeq_loc*>(o.object.GetPointer());
        if (!obj_loc)
            continue;
        if (!loc)
            loc = obj_loc;
        count++;
    }
   
    if (!loc)
    {
        wxMessageBox(wxT("The selection is not a bioseq"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }
    if (count > 1)
    {
        wxMessageBox(wxT("Multiple bioseqs in scope, opening the first one only"), wxT("Warning"), wxOK | wxICON_ERROR);
    }
    int from = 0;
    CRef<CSeq_loc> whole_loc;
    if (!loc->IsWhole())
    {
        from = loc->GetStart(objects::eExtreme_Positional) + 1;
        CBioseq_Handle bsh = scope->GetBioseqHandle(*loc);
        whole_loc = bsh.GetRangeSeq_loc(0,0);
    }
    if (whole_loc)
        loc = whole_loc.GetPointer();
    CEditSequence *dlg = NULL;
    try
    {
        dlg = new CEditSequence( main_window, loc, *scope, m_CmdProccessor, from); 
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
        wxMessageBox(wxT("No bioseq selected"), wxT("Error"), wxOK | wxICON_ERROR);
    }   
}

void CEditingBtnsPanel::MacroEditor( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRef<IGuiCoreHelper> gui_core_helper(new CGuiCoreHelper(m_Workbench));
    CMacroFlowEditor::GetInstance(main_window, gui_core_helper);
}

void CEditingBtnsPanel::SaveASN1File(wxCommandEvent& event)
{
    if (!m_TopSeqEntry) 
        return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CConstRef<CSerialObject> so = m_TopSeqEntry.GetCompleteSeq_entry();
    if (m_SeqSubmit)
    {
        so = m_SeqSubmit;
    }
    if (!so)
        return;

    wxFileDialog asn_save_file(main_window, wxT("Save ASN.1 File"), m_WorkDir, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        
    if (asn_save_file.ShowModal() == wxID_OK)
    {
        wxString path = asn_save_file.GetPath();
        if( !path.IsEmpty())
        {
            ios::openmode mode = ios::out;
            CNcbiOfstream os(string(path.ToUTF8()), mode);
            os << MSerial_AsnText;
            os << *so;
        }
    }
    
}

void CEditingBtnsPanel::ApplyDbxrefs(wxCommandEvent& event)
{
    CRef<CCmdComposite> cmd = CBarcodeTool::ApplyDbxrefs(m_TopSeqEntry);
    m_CmdProccessor->Execute(cmd);
}

END_NCBI_SCOPE



