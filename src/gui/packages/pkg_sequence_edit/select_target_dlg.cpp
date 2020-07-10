/*  $Id: select_target_dlg.cpp 42741 2019-04-08 19:39:07Z filippov@NCBI.NLM.NIH.GOV $
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

////@begin includes
////@end includes

#include <objects/submit/Seq_submit.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/seqid_guesser.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/select_target_dlg.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/msgdlg.h> 

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

/*!
 * CSelectTargetDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSelectTargetDlg, wxDialog )


/*!
 * CSelectTargetDlg event table definition
 */

BEGIN_EVENT_TABLE( CSelectTargetDlg, wxDialog )

////@begin CSelectTargetDlg event table entries
    EVT_BUTTON( ID_SEL_TARGET_ACCEPT, CSelectTargetDlg::OnSelTargetAcceptClick )

    EVT_BUTTON( ID_SEL_TARGET_CLEAR, CSelectTargetDlg::OnSelTargetClearClick )

    EVT_BUTTON( ID_SEL_TARGET_CANCEL, CSelectTargetDlg::OnSelTargetCancelClick )

////@end CSelectTargetDlg event table entries

END_EVENT_TABLE()


/*!
 * CSelectTargetDlg constructors
 */

CSelectTargetDlg::CSelectTargetDlg()
{
    Init();
}

CSelectTargetDlg::CSelectTargetDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_Workbench(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CSelectTargetDlg creator
 */

bool CSelectTargetDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSelectTargetDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSelectTargetDlg creation
    return true;
}


/*!
 * CSelectTargetDlg destructor
 */

CSelectTargetDlg::~CSelectTargetDlg()
{
////@begin CSelectTargetDlg destruction
////@end CSelectTargetDlg destruction
}


/*!
 * Member initialisation
 */

void CSelectTargetDlg::Init()
{
////@begin CSelectTargetDlg member initialisation
    m_Text = NULL;
    m_previous_count = 0;
////@end CSelectTargetDlg member initialisation
}


/*!
 * Control creation for CSelectTargetDlg
 */

void CSelectTargetDlg::CreateControls()
{    
////@begin CSelectTargetDlg content construction
    CSelectTargetDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Text = new wxTextCtrl( itemDialog1, ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_Text, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_SEL_TARGET_ACCEPT, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, ID_SEL_TARGET_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_SEL_TARGET_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSelectTargetDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CSelectTargetDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSelectTargetDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSelectTargetDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSelectTargetDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSelectTargetDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSelectTargetDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSelectTargetDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_ACCEPT
 */

void CSelectTargetDlg::OnSelTargetAcceptClick( wxCommandEvent& event )
{
    string acc_text = ToStdString(m_Text->GetValue());
    NStr::TruncateSpacesInPlace(acc_text);
    if (NStr::IsBlank(acc_text)) {
        return;
    }
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) return;
    sel_srv->GetActiveObjects(objects);
    if (objects.empty()) 
    {
        GetViewObjects(m_Workbench, objects);
    }

    if (objects.empty())
        return;

    CSeq_entry_Handle seh;
    CConstRef<CObject> main_object(NULL);
    // look for "real" main object
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const objects::CSeq_submit* seqsubmit = dynamic_cast<const objects::CSeq_submit*>(it->object.GetPointer());
        const objects::CSeq_entry* seqEntry = dynamic_cast<const objects::CSeq_entry*>(it->object.GetPointer());
        const objects::CBioseq_set* bioseqset = dynamic_cast<const objects::CBioseq_set*>(it->object.GetPointer());
        if (seqsubmit) {
            seh = GetTopSeqEntryFromScopedObject(*it);
            main_object = (*it).object;
            break;
        } else if (seqEntry) {
            seh = GetTopSeqEntryFromScopedObject(*it);
            main_object = (*it).object;
        } else if (bioseqset) {
            seh = GetTopSeqEntryFromScopedObject(*it);
            main_object = (*it).object; 
        }
    }

    if (!seh || !main_object) return;

  
    CBioseq_Handle bsh = FindBioseq(seh, acc_text);
    if (!bsh)
    {
        wxMessageBox(_("Target not found"), wxT("Info"), wxOK | wxICON_INFORMATION);        
        return;
    }

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv) return;
    CIRef<IProjectView> textView(srv->FindView(*main_object, "Text View"));
    CIRef<IFlatFileCtrl> FlatFileCtrl(dynamic_cast<IFlatFileCtrl*>(textView.GetPointer()));

    if (!FlatFileCtrl) return;

    FlatFileCtrl->SetPosition(bsh, bsh.GetBioseqCore().GetPointer());
    sel_srv->OnSelectionChanged(textView);   
}

CBioseq_Handle CSelectTargetDlg::FindBioseq(CSeq_entry_Handle seh, const string &acc_text)
{
    if (acc_text != m_previous_acc)
    {
        m_previous_count = 0;
        m_previous_acc = acc_text;
    }
    CBioseq_Handle bsh;
    edit::CSeqIdGuesser guesser(seh);
    CRef<CSeq_id> guess = guesser.Guess(acc_text);
    if (guess)
    {
        bsh = seh.GetScope().GetBioseqHandle(*guess);
    }
    else
    {
        size_t count = 0;
        for (CBioseq_CI b_iter(seh); b_iter; ++b_iter)
        {
            bool found = false;
            ITERATE(CBioseq_Handle::TId, it, b_iter->GetId()) 
            {
                string label;
                CConstRef<CSeq_id> id = (*it).GetSeqId();
                id->GetLabel(&label, CSeq_id::eContent);
                if (NStr::Find(label, acc_text) != NPOS)
                {
                    found = true;
                    break;                   
                }
            }
            if (found)
            {
                count++;
                bsh = *b_iter;
                if (count > m_previous_count)
                {
                    m_previous_count = count;                       
                    break;
                }
            }
        }
    }

    return bsh;
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_CLEAR
 */

void CSelectTargetDlg::OnSelTargetClearClick( wxCommandEvent& event )
{
    m_Text->SetValue(wxEmptyString);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_CANCEL
 */

void CSelectTargetDlg::OnSelTargetCancelClick( wxCommandEvent& event )
{ 
    Destroy();
}


END_NCBI_SCOPE

