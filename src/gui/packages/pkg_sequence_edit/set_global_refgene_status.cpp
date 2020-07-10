  /*  $Id: set_global_refgene_status.cpp 42191 2019-01-10 16:36:16Z asztalos $
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
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/set_global_refgene_status.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CSetGlobalRefGeneStatus, wxDialog )


/*!
 * CSetGlobalRefGeneStatus event table definition
 */

BEGIN_EVENT_TABLE( CSetGlobalRefGeneStatus, wxDialog )

END_EVENT_TABLE()


/*!
 * CSetGlobalRefGeneStatus constructors
 */

CSetGlobalRefGeneStatus::CSetGlobalRefGeneStatus()
{
    Init();
}

CSetGlobalRefGeneStatus::CSetGlobalRefGeneStatus( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CSetGlobalRefGeneStatus::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSetGlobalRefGeneStatus creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSetGlobalRefGeneStatus creation

    return true;
}


/*!
 * CSetGlobalRefGeneStatus destructor
 */

CSetGlobalRefGeneStatus::~CSetGlobalRefGeneStatus()
{
////@begin CSetGlobalRefGeneStatus destruction
////@end CSetGlobalRefGeneStatus destruction
}


/*!
 * Member initialisation
 */

void CSetGlobalRefGeneStatus::Init()
{
}


/*!
 * Control creation for CSetGlobalRefGeneStatus
 */

void CSetGlobalRefGeneStatus::CreateControls()
{    
    CSetGlobalRefGeneStatus* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
    
    wxArrayString status;
    status.Add(_("Inferred"));
    status.Add(_("Predicted"));
    status.Add(_("Provisional"));
    status.Add(_("Validated"));
    status.Add(_("Reviewed"));
    status.Add(_("Model"));
    status.Add(_("WGS"));
    status.Add(_("Pipeline"));
    m_Status = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, status, 0, wxDefaultValidator, _("Status"));
    m_Status->SetSelection(0);
    itemBoxSizer2->Add(m_Status, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
      
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}



/*!
 * Should we show tooltips?
 */

bool CSetGlobalRefGeneStatus::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSetGlobalRefGeneStatus::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CSetGlobalRefGeneStatus::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CSetGlobalRefGeneStatus::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Set Global RefGene Status") );
    ApplyToCSeq_entry(*(m_TopSeqEntry.GetCompleteSeq_entry()), cmd);
    return cmd;
}

void CSetGlobalRefGeneStatus::ApplyToCSeq_entry(const CSeq_entry& se, CCmdComposite* composite)  
{
    CSeq_entry_Handle seh = m_TopSeqEntry.GetScope().GetSeq_entryHandle(se);
    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsUser() && (*it)->GetUser().IsSetType() && (*it)->GetUser().GetType().IsStr() && (*it)->GetUser().GetType().GetStr() == "RefGeneTracking" && (*it)->GetUser().IsSetData())
        {
            CRef<CSeqdesc> edited_desc(new CSeqdesc);
            edited_desc->Assign(**it);
            CUser_object& edited = edited_desc->SetUser();
            string status = m_Status->GetString(m_Status->GetSelection()).ToStdString();
            if (edited.HasField("Status"))
                edited.SetField("Status").SetData().SetStr(status);
            else
                edited.AddField("Status", status);
          
            CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(seh, **it, *edited_desc));
            composite->AddCommand(*cmd);            
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToCSeq_entry (**it, composite);
        }
    }
}

END_NCBI_SCOPE
