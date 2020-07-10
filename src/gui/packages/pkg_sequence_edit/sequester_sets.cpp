  /*  $Id: sequester_sets.cpp 43899 2019-09-17 12:09:19Z choi $
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

////@begin includes
////@end includes
#include <sstream>

#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <objtools/edit/seq_entry_edit.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/core/document.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/propagate_descriptors.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/sequester_sets.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CSequesterSets, wxFrame )


/*!
 * CSequesterSets event table definition
 */

BEGIN_EVENT_TABLE( CSequesterSets, wxFrame )

EVT_BUTTON( ID_SEQUESTER_BUTTON, CSequesterSets::OnSequester)
EVT_BUTTON( ID_CANCEL_BUTTON, CSequesterSets::OnCancel)
EVT_BUTTON( ID_ACCEPT_BUTTON, CSequesterSets::OnAccept)

END_EVENT_TABLE()


/*!
 * CSequesterSets constructors
 */

CSequesterSets::CSequesterSets()
{
    Init();
}

CSequesterSets::CSequesterSets( wxWindow* parent, objects::CSeq_entry_Handle seh, CConstRef<CSeq_submit> seq_submit, IWorkbench* wb,
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    this->m_TopSeqEntry = seh;
    this->m_SeqSubmit = seq_submit;
    this->m_SetSeq.clear();
    m_Workbench = wb;
    ReadBioseq(*(this->m_TopSeqEntry.GetCompleteSeq_entry()));
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


/*!
 * CVectorTrimDlg creator
 */

bool CSequesterSets::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSequesterSets creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );
    wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
    if (attr.colBg != wxNullColour) {
        SetOwnBackgroundColour(attr.colBg);
    }
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    
    Centre();
////@end CSequesterSets creation

    return true;
}


/*!
 * CSequesterSets destructor
 */

CSequesterSets::~CSequesterSets()
{
////@begin CSequesterSets destruction
////@end CSequesterSets destruction
    RestoreTextView();   
}


/*!
 * Member initialisation
 */

void CSequesterSets::Init()
{
}


/*!
 * Control creation for CSequesterSets
 */

void CSequesterSets::CreateControls()
{    

    CSequesterSets* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
 
    m_Notebook = new wxNotebook(itemDialog1, wxID_ANY,wxDefaultPosition,wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 5);

    CIDSubpanel *panel1 = new CIDSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel1,_("ID"));

    CTextSubpanel *panel2 = new CTextSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel2,_("Text"));
    
    CLengthSubpanel *panel3 = new CLengthSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel3,_("Num Seq Length"));
 
    CNumSubpanel *panel4 = new CNumSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel4,_("Num Sets"));
   
    CFileSubpanel *panel5 = new CFileSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel5,_("File Name"));   

    CFieldSubpanel *panel6 = new CFieldSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel6,_("Field"));   

    CFeatureTypeSubpanel *panel7 = new CFeatureTypeSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel7,_("Feature Type"));
   
    CDescTypeSubpanel *panel8 = new CDescTypeSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel8,_("Desc Type"));
      
    CMolInfoSubpanel *panel9 = new CMolInfoSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel9,_("MolInfo"));
      
    CStructCommentSubpanel *panel10 = new CStructCommentSubpanel( m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_Notebook->AddPage(panel10,_("Struct Comment"));
           
    m_SequesterButton = new wxButton( itemDialog1, ID_SEQUESTER_BUTTON, _("Sequester"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_SequesterButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AcceptButton = new wxButton( itemDialog1, ID_ACCEPT_BUTTON, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_AcceptButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_AcceptButton->Disable();

    wxButton* itemButton14 = new wxButton( itemDialog1, ID_CANCEL_BUTTON, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}



/*!
 * Should we show tooltips?
 */

bool CSequesterSets::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSequesterSets::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CSequesterSets::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}


void CSequesterSets::EnableSmartPackageDone(bool enable)
{ 
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView("Dialogs.Edit.SequesterSets", CGuiRegistry::ePriority_Local - CGuiRegistry::ePriority_Temp_Inc);
    view.Set("EnableDone", enable);
}

void CSequesterSets::OnAccept( wxCommandEvent& event )
{
    if (!m_TextView || !m_SavedEntry || !m_TopSeqEntry)
    {
        EnableSmartPackageDone(true);
        Close();
        return;
    }

    RecursivePropagateDown();

    vector<CRef<CSeq_entry> > updated_entries;
    CSeq_entry_EditHandle eh = m_TopSeqEntry.GetEditHandle();
    for ( CSeq_entry_I it(eh, CSeq_entry_I::fRecursive); it; ++it ) 
    {
        CSeq_entry_EditHandle entry = *it;
        if (entry.HasParentEntry() && entry.GetParentEntry().IsSet() && entry.GetParentEntry().GetSet().IsSetClass() && entry.GetParentEntry().GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
            entry = entry.GetParentEntry();
        if (entry.IsSeq() || (entry.IsSet() && entry.GetSet().IsSetClass() && entry.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot) )
        {
            CRef<CSeq_entry> saved(new CSeq_entry);
            saved->Assign(*entry.GetCompleteSeq_entry());
            updated_entries.push_back(saved);
        }
    }

    eh.SelectNone();
    if (m_SavedEntry->IsSeq())
        eh.SelectSeq(m_SavedEntry->SetSeq());
    else
        eh.SelectSet(m_SavedEntry->SetSet());       
  
    for (size_t i = 0; i < updated_entries.size(); i++)
    {
        CRef<CSeq_entry> saved = updated_entries[i];
        if (saved->IsSeq())
        {
            const CBioseq& bioseq = saved->GetSeq();
            CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*bioseq.GetFirstId());
            if (bsh)
            {
                CSeq_entry_EditHandle seh = bsh.GetSeq_entry_Handle().GetEditHandle();
                CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
                if (bssh && bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_nuc_prot)
                    seh = bssh.GetParentEntry().GetEditHandle();
                seh.SelectNone();
                seh.SelectSeq(saved->SetSeq());
            }
        }
        else
        {
            const CBioseq& bioseq = saved->GetSet().GetNucFromNucProtSet();
            CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*bioseq.GetFirstId());
            if (bsh)
            {
                CSeq_entry_EditHandle seh = bsh.GetSeq_entry_Handle().GetEditHandle();
                CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
                if (bssh && bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_nuc_prot)
                    seh = bssh.GetParentEntry().GetEditHandle();
                seh.SelectNone();
                seh.SelectSet(saved->SetSet());
            }
        }
    }

    RecursivePropagateUp();
    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    if (prjSrv && m_TextView)
    {
        prjSrv->RemoveProjectView(*m_TextView);
        m_TextView.Reset();

        TConstScopedObjects objects;
        if (m_SeqSubmit)
            objects.push_back(SConstScopedObject(m_SeqSubmit, &m_TopSeqEntry.GetScope()));
        else
            objects.push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteSeq_entry(), &m_TopSeqEntry.GetScope()));
        IProjectView* old_view = prjSrv->FindView(*(objects.front().object), "Text View");
        if (!old_view)
            old_view = prjSrv->AddProjectView("Text View", objects, GetParams());
    }
    EnableSmartPackageDone(true);
    Close();
}

vector< vector<int> > &  CSequesterSets::GetSubsets()
{
    int sel = m_Notebook->GetSelection();
    CRBSubpanel *win = dynamic_cast<CRBSubpanel*>(m_Notebook->GetPage(sel));
    return win->GetSubsets();
}

void CSequesterSets::SetSubsets(const set<CBioseq_Handle> &selected)
{
    int sel = m_Notebook->GetSelection();
    CRBSubpanel *win = dynamic_cast<CRBSubpanel*>(m_Notebook->GetPage(sel));
    win->SetSubsets(selected);
}

CRef<CUser_object> CSequesterSets::GetParams()
{
    CRef<CUser_object> params(new CUser_object());
    CRef<CObject_id> type(new CObject_id());
    type->SetStr("TextViewParams");
    params->SetType(*type);
    CUser_object::TData& data = params->SetData();
    {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("TextViewType");
        param->SetLabel(*label);
        param->SetData().SetStr("Flat File");
        data.push_back(param);
    }
    {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("ExpandAll");
        param->SetLabel(*label);
        param->SetData().SetBool(true);
        data.push_back(param);
    }
    {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("TrackSelection");
        param->SetLabel(*label);
        param->SetData().SetBool(true);
        data.push_back(param);
    }
    return params;
}


void CSequesterSets::OnSequester( wxCommandEvent& event )
{
    vector< vector<int> > &subsets = GetSubsets();

    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();

    if (!this->m_TopSeqEntry || subsets.empty() || subsets.front().empty() || !prjSrv)
    {
        event.Skip();
        return;
    }
    
    // remove old view
    {
        TConstScopedObjects objects;
        if (m_SeqSubmit)
            objects.push_back(SConstScopedObject(m_SeqSubmit, &m_TopSeqEntry.GetScope()));
        else
            objects.push_back(SConstScopedObject(m_TopSeqEntry.GetSeq_entryCore(), &m_TopSeqEntry.GetScope()));
        
        IProjectView* old_view = prjSrv->FindView(*(objects.front().object), "Text View");
        if (old_view)
            prjSrv->RemoveProjectView(*old_view);
    }

    RecursivePropagateDown();

    set<CSeq_entry_EditHandle> selected_set;
    for (unsigned int s = 0; s < subsets.size(); s++)
    {    
        for ( unsigned int j = 0; j < subsets[s].size(); j++)
        {
            int i = subsets[s][j];
            CSeq_entry_EditHandle entry = m_SetSeq[i].entry.GetEditHandle();
            if (entry.HasParentEntry() && entry.GetParentEntry().IsSet() && entry.GetParentEntry().GetSet().IsSetClass() && entry.GetParentEntry().GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
                entry = entry.GetParentEntry();
            selected_set.insert(entry);    
        }
    }

    m_SavedEntry.Reset(new CSeq_entry);
    m_SavedEntry->Assign(*m_TopSeqEntry.GetCompleteSeq_entry());
    CSeq_entry_EditHandle eh = m_TopSeqEntry.GetEditHandle();
    for ( CSeq_entry_I it(eh, CSeq_entry_I::fRecursive); it; ++it ) 
    {
        CSeq_entry_EditHandle entry = *it;
        if (entry.HasParentEntry() && entry.GetParentEntry().IsSet() && entry.GetParentEntry().GetSet().IsSetClass() && entry.GetParentEntry().GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
            entry = entry.GetParentEntry();
        if (entry.IsSeq() || (entry.IsSet() && entry.GetSet().IsSetClass() && entry.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot) )
        {
            if ( selected_set.find(entry) == selected_set.end() )
            {
                // detach it
                entry.SelectNone();
                // make entry as empty Bioseq-set
                entry.SelectSet();
            }            
        }
    }

    // Create new Text View
    {
        TConstScopedObjects objects;
        objects.push_back(SConstScopedObject(m_TopSeqEntry.GetSeq_entryCore(), &m_TopSeqEntry.GetScope()));
       
        m_TextView.Reset(prjSrv->FindView(*(objects.front().object), "Text View"));
        if (m_TextView)
        {
            prjSrv->ActivateProjectView(m_TextView);
        }
        else
        {
            m_TextView = prjSrv->AddProjectView("Text View", objects, GetParams());
        }        
    }
    m_AcceptButton->Enable();
    m_SequesterButton->Disable();
    EnableSmartPackageDone(false);
}

void CSequesterSets::OnCancel( wxCommandEvent& event )
{
    RestoreTextView();
    Close();
}

void CSequesterSets::RestoreTextView()
{
    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    if (prjSrv && m_TextView && m_SavedEntry && m_TopSeqEntry)
    {
        prjSrv->RemoveProjectView(*m_TextView);
        m_TextView.Reset();

        RecursivePropagateDown();
        CSeq_entry_EditHandle eh = m_TopSeqEntry.GetEditHandle();
        eh.SelectNone();
        if (m_SavedEntry->IsSeq())
            eh.SelectSeq(m_SavedEntry->SetSeq());
        else
            eh.SelectSet(m_SavedEntry->SetSet());       
        RecursivePropagateUp();
        TConstScopedObjects objects;
        if (m_SeqSubmit)
            objects.push_back(SConstScopedObject(m_SeqSubmit, &m_TopSeqEntry.GetScope()));
        else
            objects.push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteSeq_entry(), &m_TopSeqEntry.GetScope()));
        IProjectView* old_view = prjSrv->FindView(*(objects.front().object), "Text View");
        if (!old_view)
            old_view = prjSrv->AddProjectView("Text View", objects, GetParams());
    }
    EnableSmartPackageDone(true);
}

void CSequesterSets::RecursiveCollectSets(const CBioseq_set_Handle &bssh, vector<CBioseq_set_Handle> &set_vec, set<CBioseq_set_Handle> &unique_set)
{
    CBioseq_set_Handle parent = bssh.GetParentBioseq_set();
    if (bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_genbank)
        return;

    if (unique_set.find(bssh) == unique_set.end())
    {
        set_vec.push_back(bssh);
        unique_set.insert(bssh);
    }
    if (parent)
        RecursiveCollectSets(parent, set_vec, unique_set);
}

void CSequesterSets::RecursivePropagateDown()
{
    vector<CBioseq_set_Handle> set_vec;
    set<CBioseq_set_Handle> unique_set;
    CSeq_entry_EditHandle eh = m_TopSeqEntry.GetEditHandle();
    for ( CSeq_entry_I it(eh, CSeq_entry_I::fRecursive); it; ++it ) 
    {
        CSeq_entry_EditHandle entry = *it;
        if (entry.HasParentEntry() && entry.GetParentEntry().IsSet() && entry.GetParentEntry().GetSet().IsSetClass() && entry.GetParentEntry().GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
            entry = entry.GetParentEntry();
        if (entry.IsSeq() || (entry.IsSet() && entry.GetSet().IsSetClass() && entry.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot) )
        {
            if (entry.HasParentEntry() && entry.GetParentEntry().IsSet())
            {
                RecursiveCollectSets(entry.GetParentEntry().GetSet(), set_vec, unique_set);
            }
        }
    }

    for (int i = set_vec.size() - 1; i >= 0; i--)
    {
        BioseqSetDescriptorPropagateDown(set_vec[i]);
    }
}

void CSequesterSets::RecursivePropagateUp()
{
    vector<CBioseq_set_Handle> set_vec;
    set<CBioseq_set_Handle> unique_set;
    CSeq_entry_EditHandle eh = m_TopSeqEntry.GetEditHandle();
    for ( CSeq_entry_I it(eh, CSeq_entry_I::fRecursive); it; ++it ) 
    {
        CSeq_entry_EditHandle entry = *it;
        if (entry.HasParentEntry() && entry.GetParentEntry().IsSet() && entry.GetParentEntry().GetSet().IsSetClass() && entry.GetParentEntry().GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
            entry = entry.GetParentEntry();
        if (entry.IsSeq() || (entry.IsSet() && entry.GetSet().IsSetClass() && entry.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot) )
        {
            if (entry.HasParentEntry() && entry.GetParentEntry().IsSet())
            {
                RecursiveCollectSets(entry.GetParentEntry().GetSet(), set_vec, unique_set);
            }
        }
    }

    for (int i = 0; i < set_vec.size(); i++)
    {
        BioseqSetDescriptorPropagateUp(set_vec[i]);
    }
}

void CSequesterSets::BioseqSetDescriptorPropagateUp(CBioseq_set_Handle set)
{
    if (set.IsEmptySeq_set()) 
    {
        return;
    }
    CConstRef<CBioseq_set> top_set = set.GetCompleteBioseq_set();
    CRef<CSeq_descr> master(new CSeq_descr());
    bool first = true;
    ITERATE(CBioseq_set::TSeq_set, it, top_set->GetSeq_set()) 
    {
        if ((*it)->IsSetDescr()) 
        {
            if (first) 
            {
                ITERATE(CSeq_descr::Tdata, d, (*it)->GetDescr().Get()) 
                {
                    if (!CPropagateDescriptors::ShouldStayInPlace(**d))
                    {
                        // add to master list
                        CRef<CSeqdesc> cpy(new CSeqdesc());
                        cpy->Assign(**d);
                        master->Set().push_back(cpy);
                    }
                }
                first = false;
            } 
            else 
            {
                // remove from master any descriptor not on member
                CSeq_descr::Tdata::iterator d = master->Set().begin();
                while (d != master->Set().end()) 
                {
                    bool found = false;
                    ITERATE(CSeq_descr::Tdata, s, (*it)->GetDescr().Get()) 
                    {
                        if ((*d)->Equals(**s)) 
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found) 
                    {
                        ++d;
                    } 
                    else 
                    {
                        d = master->Set().erase(d);
                    }
                }
            }
        } 
        else 
        {
            master->Reset();
            break;
        }
    }
    if (master->IsSet() && !master->Set().empty()) 
    {
        // copy each descriptor to master, remove from member
        CBioseq_set_EditHandle etop(set);
        ITERATE(CSeq_descr::Tdata, d, master->Get()) 
        {
            // remove from components
            ITERATE(CBioseq_set::TSeq_set, it, top_set->GetSeq_set()) 
            {
                if ((*it)->IsSeq()) 
                {
                    CBioseq_Handle bs = set.GetScope().GetBioseqHandle((*it)->GetSeq());
                    CBioseq_EditHandle bse(bs);
                    CBioseq::TDescr::Tdata::iterator di = bse.SetDescr().Set().begin();
                    while (di != bse.SetDescr().Set().end() && !(*di)->Equals(**d)) 
                    {
                        ++di;
                    }
                    if (di != bse.SetDescr().Set().end()) 
                    {
                        bse.RemoveSeqdesc(**di);
                    }
                } 
                else  if ((*it)->IsSet()) 
                {
                    CBioseq_set_Handle bss = set.GetScope().GetBioseq_setHandle((*it)->GetSet());
                    CBioseq_set_EditHandle bsse(bss);
                    CBioseq_set::TDescr::Tdata::iterator di = bsse.SetDescr().Set().begin();
                    while (di != bsse.SetDescr().Set().end() && !(*di)->Equals(**d)) 
                    {
                        ++di;
                    }
                    if (di != bsse.SetDescr().Set().end()) 
                    {
                        bsse.RemoveSeqdesc(**di);
                    }
                }
            }
            CRef<CSeqdesc> cpy(new CSeqdesc());
            cpy->Assign(**d);
            etop.AddSeqdesc(*cpy);
        }        
    }
}


void CSequesterSets::BioseqSetDescriptorPropagateDown( const CBioseq_set_Handle & bioseq_set_h)
{
    if( ! bioseq_set_h ) 
    {
        return;
    }

    // retrieve all the CSeqdescs that we will have to copy
    CConstRef<CSeq_descr> pSeqDescrToCopy;
    {
        // we have this pSeqDescrWithChosenDescs variable because 
        // we want pSeqDescrToCopy to be protected
        // once it's set
        CRef<CSeq_descr> pSeqDescrWithChosenDescs( new CSeq_descr );
        
        for(CSeqdesc_CI desc_ci( bioseq_set_h.GetParentEntry(), CSeqdesc::e_not_set, 1); desc_ci; ++desc_ci ) 
        {
            if (!CPropagateDescriptors::ShouldStayInPlace(*desc_ci)) 
                pSeqDescrWithChosenDescs->Set().push_back( CRef<CSeqdesc>( SerialClone(*desc_ci) ) );
        }
        pSeqDescrToCopy = pSeqDescrWithChosenDescs;
    }

    if (!pSeqDescrToCopy->IsSet() || pSeqDescrToCopy->Get().empty())
        return;

    // copy to all immediate children
    
    for(CSeq_entry_CI direct_child_ci( bioseq_set_h, CSeq_entry_CI::eNonRecursive ); direct_child_ci; ++direct_child_ci ) 
    {
        CRef<CSeq_descr> pNewDescr( SerialClone(*pSeqDescrToCopy) );
        direct_child_ci->GetEditHandle().AddDescr(*SerialClone(*pSeqDescrToCopy) );
    }
    
    // remove copied descs from the parent
  ITERATE(CSeq_descr::Tdata, d, pSeqDescrToCopy->Get()) 
  {
      CBioseq_set_EditHandle bsse( bioseq_set_h);
      CBioseq_set::TDescr::Tdata::iterator di = bsse.SetDescr().Set().begin();
      while (di != bsse.SetDescr().Set().end() && !(*di)->Equals(**d)) 
      {
          ++di;
      }
      if (di != bsse.SetDescr().Set().end()) 
      {
          bsse.RemoveSeqdesc(**di);
      }
  }
}

END_NCBI_SCOPE
