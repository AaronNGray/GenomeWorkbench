/* $Id: raw_seq_to_delta_by_n.cpp 43609 2019-08-08 16:12:53Z filippov $
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

#include <wx/hyperlink.h>

////@begin includes
////@end includes

#include <objects/seq/Seq_gap.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <objtools/edit/loc_edit.hpp>

#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>
#include <gui/packages/pkg_sequence_edit/raw_seq_to_delta_by_n.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IMPLEMENT_DYNAMIC_CLASS( CRawSeqToDeltaByN, wxDialog )



BEGIN_EVENT_TABLE( CRawSeqToDeltaByN, wxDialog )
END_EVENT_TABLE()


/*!
 * CRawSeqToDeltaByN constructors
 */

CRawSeqToDeltaByN::CRawSeqToDeltaByN()
{
    Init();
}

CRawSeqToDeltaByN::CRawSeqToDeltaByN( wxWindow* parent, 
				wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CRawSeqToDeltaByN creator
 */

bool CRawSeqToDeltaByN::Create( wxWindow* parent, 
			     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRawSeqToDeltaByN creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRawSeqToDeltaByN creation

    return true;
}


/*!
 * CRawSeqToDeltaByN destructor
 */

CRawSeqToDeltaByN::~CRawSeqToDeltaByN()
{
////@begin CRawSeqToDeltaByN destruction
////@end CRawSeqToDeltaByN destruction
}


/*!
 * Member initialisation
 */

void CRawSeqToDeltaByN::Init()
{
    m_Panel = nullptr;
}


/*!
 * Control creation for CRawSeqToDeltaByN
 */

void CRawSeqToDeltaByN::CreateControls()
{    
////@begin CRawSeqToDeltaByN content construction
    CRawSeqToDeltaByN* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Panel = new CRawSeqToDeltaByNPanel(itemDialog1);
    itemBoxSizer2->Add(m_Panel, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer21, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( itemDialog1, wxID_HELP, _("Help"), wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual9/#add-assembly-gaps"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    itemBoxSizer21->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);
////@end CRawSeqToDeltaByN content construction
    
}

/*!
 * Should we show tooltips?
 */

bool CRawSeqToDeltaByN::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRawSeqToDeltaByN::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRawSeqToDeltaByN bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRawSeqToDeltaByN bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRawSeqToDeltaByN::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRawSeqToDeltaByN icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRawSeqToDeltaByN icon retrieval
}

void CRawSeqToDeltaByN::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    long min_known;
    int max_known;
    long min_unknown;
    int max_unknown;
    m_Panel->GetMinMax(min_known, max_known, min_unknown, max_unknown);
    
    bool is_assembly_gap = false;
    int gap_type = CSeq_gap::eType_unknown;
    int linkage = -1;
    int linkage_evidence = -1;
    m_Panel->GetAssemblyParams(is_assembly_gap, gap_type, linkage, linkage_evidence);

    if (tse) 
    {       
        bool remove_alignment = false;
        int count = 0;
        CRef<CCmdComposite> cmd = NRawToDeltaSeq::ConvertRawToDeltaByNsCommand(tse, min_unknown, max_unknown, min_known, max_known,
                                                                               m_Panel->GetAdjustCDS(), m_Panel->GetKeepGapLength(), remove_alignment, count,
                                                                               is_assembly_gap, gap_type, linkage, linkage_evidence);

        if (cmd)
        {
            if (remove_alignment)
            {
                int answer = wxMessageBox(_("Affected alignments will be removed, proceed?"), _("Remove alignments"), wxYES_NO | wxICON_QUESTION);
                if (answer == wxYES) {
                    cmdProcessor->Execute(cmd.GetPointer());
                }
            }
            else {
                cmdProcessor->Execute(cmd.GetPointer());
            }
            
        }
    }
}


void CRawSeqToDeltaByN::DeltaSeqToRaw(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor)
{
 if (tse) 
    {       
        CScope &scope = tse.GetScope();
        CRef<CCmdComposite> composite(new CCmdComposite("Delta Seq To Raw")); 
        bool alignment_affected = false;
        for (CBioseq_CI bi(tse, CSeq_inst::eMol_na); bi; ++bi)
        {
            CBioseq_Handle bsh = *bi;
            CRef<CBioseq> bseq( new CBioseq );
            bseq->Assign(*bsh.GetCompleteBioseq());                              

            CSeq_inst& inst = bseq->SetInst();

            if (!inst.IsSetRepr() || inst.GetRepr() != CSeq_inst::eRepr_delta || !inst.IsSetExt()) 
            {
                continue;
            }

            string iupacna;
            NON_CONST_ITERATE(CSeq_ext::TDelta::Tdata, it, inst.SetExt().SetDelta().Set()) 
            {
                if ((*it)->IsLiteral())
                {
                    if ((*it)->GetLiteral().IsSetSeq_data())
                    {
                        string str;
                        switch((*it)->GetLiteral().GetSeq_data().Which()) 
                        {
                        case CSeq_data::e_Iupacna:
                            str = (*it)->GetLiteral().GetSeq_data().GetIupacna();
                            break;
                        case CSeq_data::e_Ncbi2na:
                            CSeqConvert::Convert((*it)->GetLiteral().GetSeq_data().GetNcbi2na().Get(), CSeqUtil::e_Ncbi2na, 0, (*it)->GetLiteral().GetLength(), str, CSeqUtil::e_Iupacna);
                            break;
                        case CSeq_data::e_Ncbi4na:
                            CSeqConvert::Convert((*it)->GetLiteral().GetSeq_data().GetNcbi4na().Get(), CSeqUtil::e_Ncbi4na, 0, (*it)->GetLiteral().GetLength(), str, CSeqUtil::e_Iupacna);
                            break;
                        case CSeq_data::e_Ncbi8na:
                            CSeqConvert::Convert((*it)->GetLiteral().GetSeq_data().GetNcbi8na().Get(), CSeqUtil::e_Ncbi8na, 0, (*it)->GetLiteral().GetLength(), str, CSeqUtil::e_Iupacna);
                            break;
                        default:
                            break;
                        }
                        iupacna += str;
                    }

                    if (!(*it)->GetLiteral().IsSetSeq_data() || (*it)->GetLiteral().GetSeq_data().IsGap())
                    {                
                        TSeqPos length = (*it)->GetLiteral().GetLength();    
                        iupacna += string(length,'N');
                    }
                }
            }

            inst.SetRepr(CSeq_inst::eRepr_raw);
            inst.SetSeq_data().SetIupacna() = CIUPACna(iupacna);
            inst.ResetExt();
            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, bseq->SetInst()));
            composite->AddCommand(*cmd);            
        }       
        
        cmdProcessor->Execute(composite.GetPointer());
    }
}


END_NCBI_SCOPE
