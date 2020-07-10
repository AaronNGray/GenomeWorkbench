/* $Id: add_linkage_to_gaps.cpp 43378 2019-06-20 19:39:44Z filippov $
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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seq/Seq_gap.hpp>
#include <objects/seq/Linkage_evidence.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seq/IUPACna_.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <objtools/edit/seq_entry_edit.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/packages/pkg_sequence_edit/add_linkage_to_gaps.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

IMPLEMENT_DYNAMIC_CLASS( CAddLinkageToGaps, wxDialog )



BEGIN_EVENT_TABLE( CAddLinkageToGaps, wxDialog )

////@begin CAddLinkageToGaps event table entries
////@end CAddLinkageToGaps event table entries
EVT_CHOICE(ID_ADD_LINKAGE_TO_GAPS_GAP_TYPE, CAddLinkageToGaps::OnGapType)
EVT_CHOICE(ID_ADD_LINKAGE_TO_GAPS_LINKAGE, CAddLinkageToGaps::OnLinkage)
END_EVENT_TABLE()


/*!
 * CAddLinkageToGaps constructors
 */

CAddLinkageToGaps::CAddLinkageToGaps()
{
    Init();
}

CAddLinkageToGaps::CAddLinkageToGaps( wxWindow* parent, 
				wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CAddLinkageToGaps creator
 */

bool CAddLinkageToGaps::Create( wxWindow* parent, 
			     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddLinkageToGaps creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAddLinkageToGaps creation

    return true;
}


/*!
 * CAddLinkageToGaps destructor
 */

CAddLinkageToGaps::~CAddLinkageToGaps()
{
////@begin CAddLinkageToGaps destruction
////@end CAddLinkageToGaps destruction
}


/*!
 * Member initialisation
 */

void CAddLinkageToGaps::Init()
{
    m_GapType = NULL;
    m_Linkage = NULL;
    m_LinkageEvidence = NULL;
}


/*!
 * Control creation for CAddLinkageToGaps
 */

void CAddLinkageToGaps::CreateControls()
{    
////@begin CAddLinkageToGaps content construction
    CAddLinkageToGaps* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2);

    CEnumeratedTypeValues::TValues type_values = objects::CSeq_gap::ENUM_METHOD_NAME(EType)()->GetValues();
    CEnumeratedTypeValues::TValues linkage_values = objects::CSeq_gap::ENUM_METHOD_NAME(ELinkage)()->GetValues();
    CEnumeratedTypeValues::TValues linkage_evidence_values  = objects::CLinkage_evidence::ENUM_METHOD_NAME(EType)()->GetValues();
   
    wxArrayString type_str, linkage_str, linkage_evidence_str;
    type_str.Add("within");
    type_str.Add("between");
    type_str.Add("short-arm");
    type_str.Add("heterochromatin");
    type_str.Add("telomere");
    type_str.Add("centromere");
    type_str.Add("repeat");
    type_str.Add("contamination");
    type_str.Add("unknown");
    type_str.Add("other");

/*    for (CEnumeratedTypeValues::TValues::const_iterator i = type_values.begin(); i != type_values.end(); ++i)
    {
        type_str.Add(wxString(i->first));
    }
*/
    linkage_str.Add(_("Within Scaffolds"));
    linkage_str.Add(_("Between Scaffolds"));

    for (CEnumeratedTypeValues::TValues::const_iterator i = linkage_evidence_values.begin(); i != linkage_evidence_values.end(); ++i)
    {
        linkage_evidence_str.Add(wxString(i->first));
    }

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Gap Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL |wxALL, 5);

    m_GapType = new wxChoice(itemDialog1, ID_ADD_LINKAGE_TO_GAPS_GAP_TYPE, wxDefaultPosition, wxDefaultSize, type_str,0,wxDefaultValidator,"Gap Type");
    m_GapType->SetSelection(0);
    itemFlexGridSizer3->Add(m_GapType, 1, wxALIGN_LEFT | wxGROW | wxALIGN_CENTER_VERTICAL |wxALL, 5);
    m_GapType->Disable();

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Linkage"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText11, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL |wxALL, 5);

    m_Linkage = new wxChoice(itemDialog1, ID_ADD_LINKAGE_TO_GAPS_LINKAGE, wxDefaultPosition, wxDefaultSize, linkage_str,0,wxDefaultValidator,"Linkage");
    m_Linkage->SetSelection(0);
    itemFlexGridSizer3->Add(m_Linkage, 1, wxALIGN_LEFT | wxGROW | wxALIGN_CENTER_VERTICAL |wxALL, 5);
    m_Linkage->Disable();

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Linkage Evidence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL |wxALL, 5);

    m_LinkageEvidence = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, linkage_evidence_str,0,wxDefaultValidator,"Linkage Evidence");
    m_LinkageEvidence->SetSelection(0);
    itemFlexGridSizer3->Add(m_LinkageEvidence, 1, wxALIGN_LEFT | wxGROW | wxALIGN_CENTER_VERTICAL |wxALL, 5);
    m_LinkageEvidence->Disable();

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton2 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAddLinkageToGaps content construction
    m_GapType->Enable();
    int gap_type = m_GapType->GetSelection();
    if (gap_type != wxNOT_FOUND)
    {
        if ( m_GapType->GetString(gap_type) == _("repeat"))
        {
            m_Linkage->Enable();
            m_LinkageEvidence->Enable();
        }
        if ( m_GapType->GetString(gap_type) == _("scaffold") || m_GapType->GetString(gap_type) == _("within"))
        {
            m_LinkageEvidence->Enable();
            }
    }        
}


/*!
 * Should we show tooltips?
 */

bool CAddLinkageToGaps::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAddLinkageToGaps::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddLinkageToGaps bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddLinkageToGaps bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAddLinkageToGaps::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddLinkageToGaps icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddLinkageToGaps icon retrieval
}



CRef<CCmdComposite> CAddLinkageToGaps::GetCommand(objects::CSeq_entry_Handle tse)
{
  
    int gap_type = objects::CSeq_gap::eType_unknown;
    int linkage = -1;
    int linkage_evidence = -1;

    int gap_type_sel = m_GapType->GetSelection();
    int linkage_sel = m_Linkage->GetSelection();
    if (gap_type_sel != wxNOT_FOUND)
    {
        string gap_type_str = m_GapType->GetString(gap_type_sel).ToStdString();
        if (gap_type_str == "within")
            gap_type_str = "scaffold";
        if (gap_type_str == "between")
            gap_type_str = "contig";
        gap_type = objects::CSeq_gap::ENUM_METHOD_NAME(EType)()->FindValue(gap_type_str);
        linkage = objects::CSeq_gap::eLinkage_unlinked;
        if ( gap_type_str == "repeat")
        {
            if (linkage_sel  != wxNOT_FOUND)
            {
                if (m_Linkage->GetString(linkage_sel) == _("Within Scaffolds"))
                {
                    linkage = objects::CSeq_gap::eLinkage_linked;
                    linkage_evidence = objects::CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindValue( m_LinkageEvidence->GetString(m_LinkageEvidence->GetSelection()).ToStdString());
                }
            }
        }
        if ( gap_type_str == "scaffold" )
        {
            linkage = objects::CSeq_gap::eLinkage_linked;
            linkage_evidence = objects::CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindValue( m_LinkageEvidence->GetString(m_LinkageEvidence->GetSelection()).ToStdString());
        }
    }        


    CRef<CCmdComposite> composite( new CCmdComposite("Add Linkage To Gaps") );   
    bool modified(false);
    for (CBioseq_CI bi(tse, CSeq_inst::eMol_na); bi; ++bi)
    {
        CBioseq_Handle bsh = *bi;
        TSeqPos len = bsh.GetBioseqLength();
        if (len <= 1)
            continue;
        if (!bsh.IsSetInst())
            continue;

        const CSeq_inst &inst = bsh.GetInst();
        if (!inst.IsSetExt() || !inst.GetExt().IsDelta())
            continue;
        bool has_loc(false);
        ITERATE(CDelta_ext::Tdata, iter, inst.GetExt().GetDelta().Get()) 
        {
            if ( (*iter)->IsLoc() ) 
            {
                has_loc = true;
                break;
            }
        }
        if (has_loc)
            continue;
        CRef<CSeq_inst> new_inst(new CSeq_inst);
        new_inst->Assign(inst);
        bool found(false);
        try 
        {
            TSeqPos pos = 0;
            CSeqVector sv = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
            NON_CONST_ITERATE(CDelta_ext::Tdata, delta_i, new_inst->SetExt().SetDelta().Set()) 
            {
                if (delta_i->Empty()) 
                    continue; 
                CDelta_seq& seg = **delta_i;
                size_t delta_len = seg.GetLiteral().GetLength();
                if (sv.IsInGap (pos)) 
                {
                    seg.SetLiteral().SetSeq_data().SetGap();
                    seg.SetLiteral().SetSeq_data().SetGap().SetType(gap_type);
                    if (linkage >= 0)
                    {
                        seg.SetLiteral().SetSeq_data().SetGap().SetLinkage(linkage);
                    }
                    if (linkage_evidence >= 0)
                    {
                        CRef<CLinkage_evidence> link_ev(new CLinkage_evidence);
                        link_ev->SetType(linkage_evidence);
                        seg.SetLiteral().SetSeq_data().SetGap().SetLinkage_evidence().push_back(link_ev);
                    } 
                    found = true;
                }               
                pos += delta_len;
            }
        } catch (CException ) {} catch (std::exception ) {}
        if (found)
        {
            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, *new_inst));
            composite->AddCommand(*cmd);                        
            modified = true;
        }
    }

    if (!modified)
        composite.Reset();
        
    return composite;
   
}



void CAddLinkageToGaps::OnGapType(wxCommandEvent& event)
{
    m_Linkage->Disable();
    m_LinkageEvidence->Disable();
    int gap_type = m_GapType->GetSelection();
    int linkage = m_Linkage->GetSelection();
    if (gap_type != wxNOT_FOUND)
    {
        if ( m_GapType->GetString(gap_type) == _("repeat"))
        {
            m_Linkage->Enable();
            if (linkage  != wxNOT_FOUND  && m_Linkage->GetString(linkage) == _("Within Scaffolds"))
            {
                m_LinkageEvidence->Enable();
            }
        }
        if ( m_GapType->GetString(gap_type) == _("scaffold") || m_GapType->GetString(gap_type) == _("within"))
        {
            m_LinkageEvidence->Enable();
        }
    }        
}

void CAddLinkageToGaps::OnLinkage(wxCommandEvent& event)
{
  int gap_type = m_GapType->GetSelection();
  int linkage = m_Linkage->GetSelection();
   if (gap_type != wxNOT_FOUND  && m_GapType->GetString(gap_type) == _("repeat") &&
       linkage  != wxNOT_FOUND  && m_Linkage->GetString(linkage) == _("Within Scaffolds"))
   {
       m_LinkageEvidence->Enable();
   }
   else
   {
       m_LinkageEvidence->Disable();
   }
}

END_NCBI_SCOPE
