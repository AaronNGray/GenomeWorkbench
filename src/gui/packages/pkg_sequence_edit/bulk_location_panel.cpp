/*  $Id: bulk_location_panel.cpp 42098 2018-12-19 16:32:59Z filippov $
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
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_point.hpp>
#include <objmgr/util/sequence.hpp>
////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/sizer.h>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CBulkLocationPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBulkLocationPanel, wxPanel )


/*!
 * CBulkLocationPanel event table definition
 */

BEGIN_EVENT_TABLE( CBulkLocationPanel, wxPanel )

////@begin CBulkLocationPanel event table entries
    EVT_RADIOBUTTON( ID_BULKLOC_RBTN1, CBulkLocationPanel::OnBulklocRbtn1Selected )
    EVT_RADIOBUTTON( ID_BULKLOC_RBTN2, CBulkLocationPanel::OnBulklocRbtn2Selected )
////@end CBulkLocationPanel event table entries

END_EVENT_TABLE()


/*!
 * CBulkLocationPanel constructors
 */

CBulkLocationPanel::CBulkLocationPanel()
{
    Init();
}

CBulkLocationPanel::CBulkLocationPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CBulkLocationPanel creator
 */

bool CBulkLocationPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBulkLocationPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBulkLocationPanel creation
    return true;
}


/*!
 * CBulkLocationPanel destructor
 */

CBulkLocationPanel::~CBulkLocationPanel()
{
////@begin CBulkLocationPanel destruction
////@end CBulkLocationPanel destruction
}


/*!
 * Member initialisation
 */

void CBulkLocationPanel::Init()
{
////@begin CBulkLocationPanel member initialisation
    m_Partial5 = NULL;
    m_Partial3 = NULL;
    m_Strand = NULL;
    m_WholeSeq = NULL;
    m_Coordinates = NULL;
    m_From = NULL;
    m_To = NULL;
    m_AddToExisting = NULL;
////@end CBulkLocationPanel member initialisation
}


/*!
 * Control creation for CBulkLocationPanel
 */

void CBulkLocationPanel::CreateControls()
{    
////@begin CBulkLocationPanel content construction
    CBulkLocationPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Partial5 = new wxCheckBox( itemPanel1, ID_BULKLOC_CHCKBOX1, _("Incomplete at 5' end"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Partial5->SetValue(false);
    itemBoxSizer3->Add(m_Partial5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Partial3 = new wxCheckBox( itemPanel1, ID_BULKLOC_CHCKBOX2, _("Incomplete at 3' end"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Partial3->SetValue(false);
    itemBoxSizer3->Add(m_Partial3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_StrandStrings;
    m_StrandStrings.Add(_("&Plus"));
    m_StrandStrings.Add(_("&Minus"));
    m_Strand = new wxRadioBox( itemPanel1, ID_BULKLOC_RBOX, _("Strand"), wxDefaultPosition, wxDefaultSize, m_StrandStrings, 1, wxRA_SPECIFY_ROWS );
    m_Strand->SetSelection(0);
    itemBoxSizer2->Add(m_Strand, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WholeSeq = new wxRadioButton( itemPanel1, ID_BULKLOC_RBTN1, _("Use Whole Sequence Interval"), wxDefaultPosition, wxSize(-1, 15), 0 );
    m_WholeSeq->SetValue(true);
    itemBoxSizer8->Add(m_WholeSeq, 0, wxALIGN_LEFT|wxALL, 5);

    m_Coordinates = new wxRadioButton( itemPanel1, ID_BULKLOC_RBTN2, _("Use these coordinates:"), wxDefaultPosition, wxSize(-1, 15), 0 );
    m_Coordinates->SetValue(false);
    itemBoxSizer8->Add(m_Coordinates, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemBoxSizer11->Add(5, 15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer11->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_From = new wxTextCtrl( itemPanel1, ID_BULKLOC_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_From->Enable(false);
    itemBoxSizer13->Add(m_From, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_To = new wxTextCtrl( itemPanel1, ID_BULKLOC_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_To->Enable(false);
    itemBoxSizer13->Add(m_To, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AddToExisting = new wxCheckBox( itemPanel1, ID_BULKLOC_CHCKBOX3, _("Also add to sequences that already have a feature like this"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AddToExisting->SetValue(true);
    itemBoxSizer2->Add(m_AddToExisting, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CBulkLocationPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CBulkLocationPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkLocationPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkLocationPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkLocationPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkLocationPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkLocationPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkLocationPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON2
 */

void CBulkLocationPanel::OnBulklocRbtn1Selected( wxCommandEvent& event )
{
    m_From->Enable(false);
    m_To->Enable(false);
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON3
 */

void CBulkLocationPanel::OnBulklocRbtn2Selected( wxCommandEvent& event )
{
    m_From->Enable(true);
    m_To->Enable(true);
}


CRef<CSeq_loc> CBulkLocationPanel::GetLocForBioseq (const CBioseq& bioseq)
{
    int max = bioseq.GetLength() - 1;
    int from = 0;
    int to = max;
    bool fuzz = false;
    if (m_Coordinates->GetValue()) {
        string val = ToStdString(m_From->GetValue());
        if (NStr::IsBlank(val)) {
            CRef<CSeq_loc> empty;
            return empty;
        } else {
            try {
                fuzz |= NStr::StartsWith(val,"^") || NStr::EndsWith(val,"^");
                NStr::TrimPrefixInPlace(val,"^");
                NStr::TrimSuffixInPlace(val,"^");
                from = NStr::StringToInt(val);
                from = from - 1;
            } catch(CException &e) {
                CRef<CSeq_loc> empty;
                return empty;
            }
            catch (exception &e) {
                CRef<CSeq_loc> empty;
                return empty;
            }   
        }

        val = ToStdString(m_To->GetValue());
        if (NStr::IsBlank(val)) {
            CRef<CSeq_loc> empty;
            return empty;
        } else {
            try {
                fuzz |= NStr::StartsWith(val,"^") || NStr::EndsWith(val,"^");
                NStr::TrimPrefixInPlace(val,"^");
                NStr::TrimSuffixInPlace(val,"^");
                to = NStr::StringToInt(val);
                to = to - 1;
            }  catch(CException &e) {
                CRef<CSeq_loc> empty;
                return empty;
            }
            catch (exception &e) {
                CRef<CSeq_loc> empty;
                return empty;
            }   
        }

        if (from > max) {
            from = max;
        }
        if (to > max) {
            to = max;
        }
        if (from > to) {
            int swap = from;
            from = to;
            to = swap;
        }
    }


    CRef<CSeq_loc> loc(new CSeq_loc());

    CSeq_id_Handle best_idh = sequence::GetId(bioseq, sequence::eGetId_Best);
    if (from == to)
    {
        CRef<CSeq_point> seq_pnt(new CSeq_point);
        seq_pnt->SetPoint(from);
        if (m_Strand->GetSelection() == 1) {
            seq_pnt->SetStrand(eNa_strand_minus);
        }   
        seq_pnt->SetId().Assign(*best_idh.GetSeqId());
        loc->SetPnt(*seq_pnt);
    }
    else if (fuzz && abs(to - from) <= 1)
    {
        CRef<CSeq_point> seq_pnt(new CSeq_point);
        seq_pnt->SetPoint(from);
        if (m_Strand->GetSelection() == 1) {
            seq_pnt->SetStrand(eNa_strand_minus);
        }   
        seq_pnt->SetId().Assign(*best_idh.GetSeqId());        
        seq_pnt->SetFuzz().SetLim(CInt_fuzz::eLim_tr);
        loc->SetPnt(*seq_pnt);
    }
    else
    {
        loc->SetInt().SetId().Assign(*best_idh.GetSeqId());
        loc->SetInt().SetFrom(from);
        loc->SetInt().SetTo(to);
    
        if (m_Strand->GetSelection() == 1) {
            loc->SetInt().SetStrand(eNa_strand_minus);
        }
    }

    if (m_Partial5->GetValue()) {
        loc->SetPartialStart(true, eExtreme_Biological);
    }
    if (m_Partial3->GetValue()) {
        loc->SetPartialStop (true, eExtreme_Biological);
    }
    return loc;
}


END_NCBI_SCOPE



