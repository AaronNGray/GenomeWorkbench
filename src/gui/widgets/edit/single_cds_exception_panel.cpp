/*  $Id: single_cds_exception_panel.cpp 38073 2017-03-23 16:34:10Z filippov $
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
 * Authors:  Roman Katargin
 */


// Generated by DialogBlocks (unregistered), 22/12/2011 15:18:47

#include <ncbi_pch.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>

////@begin includes
////@end includes

#include "single_cds_exception_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CSingleCDSExceptionPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleCDSExceptionPanel, CQualListItemPanel )


/*!
 * CSingleCDSExceptionPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleCDSExceptionPanel, CQualListItemPanel )

////@begin CSingleCDSExceptionPanel event table entries
    EVT_CHOICE( ID_CHOICE14, CSingleCDSExceptionPanel::OnChoice14Selected )

////@end CSingleCDSExceptionPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleCDSExceptionPanel constructors
 */

CSingleCDSExceptionPanel::CSingleCDSExceptionPanel()
{
    Init();
}

CSingleCDSExceptionPanel::CSingleCDSExceptionPanel( wxWindow* parent, objects::CCode_break& code_break, CSeq_feat& feat, CScope& scope, 
                                                   wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
                                                   : m_CodeBreak(new CCode_break()), m_Scope(&scope), m_EditedFeat(new CSeq_feat())
{
    _ASSERT(feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion);
    Init();
    m_CodeBreak->Assign(code_break);
    m_EditedFeat->Assign (feat);
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleCDSExceptionPanel creator
 */

bool CSingleCDSExceptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleCDSExceptionPanel creation
    CQualListItemPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleCDSExceptionPanel creation
    return true;
}


/*!
 * CSingleCDSExceptionPanel destructor
 */

CSingleCDSExceptionPanel::~CSingleCDSExceptionPanel()
{
}


/*!
 * Member initialisation
 */

void CSingleCDSExceptionPanel::Init()
{
////@begin CSingleCDSExceptionPanel member initialisation
    m_AACtrl = NULL;
    m_AminoAcidCtrl = NULL;
////@end CSingleCDSExceptionPanel member initialisation
}


/*!
 * Control creation for CSingleCDSExceptionPanel
 */

void CSingleCDSExceptionPanel::CreateControls()
{    
////@begin CSingleCDSExceptionPanel content construction
    CSingleCDSExceptionPanel* itemCQualListItemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCQualListItemPanel1->SetSizer(itemBoxSizer2);

    m_AACtrl = new wxTextCtrl( itemCQualListItemPanel1, ID_TEXTCTRL35, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    if (CSingleCDSExceptionPanel::ShowToolTips())
        m_AACtrl->SetToolTip(_("Enter the amino acid position at which the post-translational modification occurs."));
    itemBoxSizer2->Add(m_AACtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxArrayString m_AminoAcidCtrlStrings;
    m_AminoAcidCtrlStrings.Add(wxEmptyString);
    m_AminoAcidCtrlStrings.Add(_("Alanine"));
    m_AminoAcidCtrlStrings.Add(_("Asp or Asn"));
    m_AminoAcidCtrlStrings.Add(_("Cysteine"));
    m_AminoAcidCtrlStrings.Add(_("Aspartic Acid"));
    m_AminoAcidCtrlStrings.Add(_("Glutamic Acid"));
    m_AminoAcidCtrlStrings.Add(_("Phenylalanine"));
    m_AminoAcidCtrlStrings.Add(_("Glycine"));
    m_AminoAcidCtrlStrings.Add(_("Histidine"));
    m_AminoAcidCtrlStrings.Add(_("Isoleucine"));
    m_AminoAcidCtrlStrings.Add(_("Leu or Ile"));
    m_AminoAcidCtrlStrings.Add(_("Lysine"));
    m_AminoAcidCtrlStrings.Add(_("Leucine"));
    m_AminoAcidCtrlStrings.Add(_("Methionine"));
    m_AminoAcidCtrlStrings.Add(_("Asparagine"));
    m_AminoAcidCtrlStrings.Add(_("Pyrrolysine"));
    m_AminoAcidCtrlStrings.Add(_("Proline"));
    m_AminoAcidCtrlStrings.Add(_("Glutamine"));
    m_AminoAcidCtrlStrings.Add(_("Arginine"));
    m_AminoAcidCtrlStrings.Add(_("Serine"));
    m_AminoAcidCtrlStrings.Add(_("Threonine"));
    m_AminoAcidCtrlStrings.Add(_("Selenocysteine"));
    m_AminoAcidCtrlStrings.Add(_("Valine"));
    m_AminoAcidCtrlStrings.Add(_("Tryptophan"));
    m_AminoAcidCtrlStrings.Add(_("Undetermined or atypical"));
    m_AminoAcidCtrlStrings.Add(_("Tyrosine"));
    m_AminoAcidCtrlStrings.Add(_("Glu or Gln"));
    m_AminoAcidCtrlStrings.Add(_("Termination"));
    m_AminoAcidCtrl = new wxChoice( itemCQualListItemPanel1, ID_CHOICE14, wxDefaultPosition, wxDefaultSize, m_AminoAcidCtrlStrings, 0 );
    if (CSingleCDSExceptionPanel::ShowToolTips())
        m_AminoAcidCtrl->SetToolTip(_("Select the amino acid that is actually represented in the protein after post-translational modification at this amino acid position."));
    itemBoxSizer2->Add(m_AminoAcidCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleCDSExceptionPanel content construction
    m_AminoAcidList.clear();
    m_AminoAcidList.push_back(" ");
    m_AminoAcidList.push_back("A");
    m_AminoAcidList.push_back("B");
    m_AminoAcidList.push_back("C");
    m_AminoAcidList.push_back("D");
    m_AminoAcidList.push_back("E");
    m_AminoAcidList.push_back("F");
    m_AminoAcidList.push_back("G");
    m_AminoAcidList.push_back("H");
    m_AminoAcidList.push_back("I");
    m_AminoAcidList.push_back("J");
    m_AminoAcidList.push_back("K");
    m_AminoAcidList.push_back("L");
    m_AminoAcidList.push_back("M");
    m_AminoAcidList.push_back("N");
    m_AminoAcidList.push_back("O");
    m_AminoAcidList.push_back("P");
    m_AminoAcidList.push_back("Q");
    m_AminoAcidList.push_back("R");
    m_AminoAcidList.push_back("S");
    m_AminoAcidList.push_back("T");
    m_AminoAcidList.push_back("U");
    m_AminoAcidList.push_back("V");
    m_AminoAcidList.push_back("W");
    m_AminoAcidList.push_back("X");
    m_AminoAcidList.push_back("Y");
    m_AminoAcidList.push_back("Z");
    m_AminoAcidList.push_back("*");
}


void CSingleCDSExceptionPanel::x_SetAminoAcid ()
{
    unsigned char aa = 0;
    vector<char> seqData;
    string str = "";

    if (!m_CodeBreak->IsSetAa()) {
        m_AminoAcidCtrl->SetSelection(0);
    } else {    
        switch (m_CodeBreak->GetAa().Which()) {
            case CCode_break::C_Aa::e_Ncbi8aa:
                str = m_CodeBreak->GetAa().GetNcbi8aa();
                CSeqConvert::Convert(str, CSeqUtil::e_Ncbi8aa, 0, str.size(), seqData, CSeqUtil::e_Ncbieaa);
                aa = seqData[0];
                break;
            case CCode_break::C_Aa::e_Ncbistdaa:
                str = m_CodeBreak->GetAa().GetNcbi8aa();
                CSeqConvert::Convert(str, CSeqUtil::e_Ncbistdaa, 0, str.size(), seqData, CSeqUtil::e_Ncbieaa);
                aa = seqData[0];
                break;
            case CCode_break::C_Aa::e_Ncbieaa:
                seqData.push_back(m_CodeBreak->GetAa().GetNcbieaa());
                aa = seqData[0];
                break;
            default:
                break;
        }

        m_AminoAcidCtrl->SetSelection(0);
        for (int i = 1; i < m_AminoAcidList.size(); i++) {
            if (aa == m_AminoAcidList[i].c_str()[0]) {
                m_AminoAcidCtrl->SetSelection(i);
                break;
            }
        }                
    }
}


bool CSingleCDSExceptionPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    wxString loc_label = wxEmptyString;
    if (m_CodeBreak->IsSetLoc() && m_CodeBreak->GetLoc().Which() != CSeq_loc::e_not_set) {
        // find offset from beginning of protein
        TSeqPos seq_pos =
                sequence::LocationOffset(m_EditedFeat->GetLocation(), m_CodeBreak->GetLoc(),
                                         sequence::eOffset_FromStart,
                                         m_Scope);
        seq_pos /= 3;
        seq_pos += 1;
        loc_label = ToWxString(NStr::NumericToString(seq_pos));
    }
    m_AACtrl->SetValue (loc_label);

    x_SetAminoAcid();
    return true;
}


bool CSingleCDSExceptionPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    // amino acid
    int aa_pos = m_AminoAcidCtrl->GetSelection();
    if (aa_pos > 0 && aa_pos < m_AminoAcidList.size()) {
        m_CodeBreak->SetAa().SetNcbieaa(m_AminoAcidList[aa_pos].c_str()[0]);
    } else {
        m_CodeBreak->ResetAa();
    }

    string loc_label = ToStdString (m_AACtrl->GetValue());
    if (NStr::IsBlank(loc_label)) {
        m_CodeBreak->ResetLoc();
    } else {
      int start = NStr::StringToInt(loc_label);
        start -= 1;
        start *= 3;
        int frame = 0;
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion() && m_EditedFeat->GetData().GetCdregion().IsSetFrame())
        {
            switch(m_EditedFeat->GetData().GetCdregion().GetFrame())
            {
            case CCdregion::eFrame_not_set :
            case CCdregion::eFrame_one : frame = 0; break;
            case CCdregion::eFrame_two : frame = 1; break;
            case  CCdregion::eFrame_three : frame = 2; break;
            default : frame = 0; break;
            }
        }
        int frame_shift = (start - frame) % 3;
        if (frame_shift == 1)
            start += 2;
        else if (frame_shift == 2)
            start += 1;

        int offset = 0;
        CRef<CSeq_loc> cb_loc (new CSeq_loc());
        for (CSeq_loc_CI loc_iter(m_EditedFeat->GetLocation());  loc_iter;  ++loc_iter) {
            int len = loc_iter.GetRange().GetLength();
            if (offset <= start && offset + len > start) {
                cb_loc->SetInt().SetId().Assign(loc_iter.GetSeq_id());
                if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) {                    
                    cb_loc->SetInt().SetStrand(eNa_strand_minus);
                    cb_loc->SetInt().SetTo(loc_iter.GetRange().GetTo() - (start - offset) );
                } else {
                    cb_loc->SetInt().SetFrom(loc_iter.GetRange().GetFrom() + start - offset);
                }
                if (offset <= start + 2 && offset + len > start + 2) {
                    if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) {                    
                        cb_loc->SetInt().SetFrom(loc_iter.GetRange().GetTo() - (start - offset + 2) );
                    } else {
                        cb_loc->SetInt().SetTo(loc_iter.GetRange().GetFrom() + start - offset + 2);
                    }
                } else {
                    if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) {                    
                        cb_loc->SetInt().SetFrom(loc_iter.GetRange().GetFrom());
                    } else {
                        cb_loc->SetInt().SetTo(loc_iter.GetRange().GetTo());
                    }
                }
            } else if (offset > start && offset <= start + 2) {
                if (cb_loc->IsInt()) {
                    CRef<CSeq_interval> tmp(new CSeq_interval());
                    tmp->Assign (cb_loc->GetInt());
                    cb_loc->SetPacked_int().Set().push_back(tmp);
                }
                // add new interval
                CRef<CSeq_interval> tmp (new CSeq_interval());
                tmp->SetId().Assign(loc_iter.GetSeq_id());
                if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) {                    
                    tmp->SetStrand(eNa_strand_minus);
                    tmp->SetTo(loc_iter.GetRange().GetTo());
                    if (offset + len >= start + 2) {
                        tmp->SetFrom(loc_iter.GetRange().GetTo() - (start - offset + 2) );
                    } else {
                        tmp->SetFrom(loc_iter.GetRange().GetFrom());
                    }
                } else {
                    tmp->SetFrom(loc_iter.GetRange().GetFrom());
                    if (offset + len >= start + 2) {
                        tmp->SetTo(loc_iter.GetRange().GetFrom() + start - offset + 2);
                    } else {
                        tmp->SetTo(loc_iter.GetRange().GetTo());
                    }
                }

                cb_loc->SetPacked_int().Set().push_back(tmp);         
            } 
            offset += len;
        }
        m_CodeBreak->SetLoc(*cb_loc);
    }
    return true;
}


void CSingleCDSExceptionPanel::SetCodeBreak(CRef<CCode_break> code_break)
{
    m_CodeBreak.Reset(code_break.GetPointer());
    TransferDataToWindow();
}


CRef<CCode_break> CSingleCDSExceptionPanel::GetCodeBreak()
{
    TransferDataFromWindow();
    if (m_CodeBreak)
        return m_CodeBreak;

    return CRef<CCode_break>();
}


void CSingleCDSExceptionPanel::GetDimensions(int *width, int *height)
{
      if (height != NULL) {
            int win_width, win_height;
            m_AminoAcidCtrl->GetSize(&win_width, &win_height);
          *height = win_height;
      }
      if (width != NULL) {
            int win_width, win_height;
            m_AminoAcidCtrl->GetSize(&win_width, &win_height);
        *width = win_width;
        m_AACtrl->GetSize(&win_width, &win_height);
        *width += win_width;
      }
}


/*!
 * Should we show tooltips?
 */

bool CSingleCDSExceptionPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleCDSExceptionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleCDSExceptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleCDSExceptionPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleCDSExceptionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleCDSExceptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleCDSExceptionPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE14
 */

void CSingleCDSExceptionPanel::OnChoice14Selected( wxCommandEvent& event )
{
    wxChoice* item = (wxChoice*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetStringSelection()))) {
        return;
    }
    NotifyParentOfChange();
}




END_NCBI_SCOPE