/*  $Id: tmrna_subpanel.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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

#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/seqfeat/RNA_qual_set.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "tmrna_subpanel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CtmRNASubPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CtmRNASubPanel, wxPanel )


/*!
 * CtmRNASubPanel event table definition
 */

BEGIN_EVENT_TABLE( CtmRNASubPanel, wxPanel )

////@begin CtmRNASubPanel event table entries
////@end CtmRNASubPanel event table entries

END_EVENT_TABLE()


/*!
 * CtmRNASubPanel constructors
 */

CtmRNASubPanel::CtmRNASubPanel()
{
    Init();
}

CtmRNASubPanel::CtmRNASubPanel( wxWindow* parent, CRef<CRNA_gen> gen, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_Gen(gen)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CtmRNASubPanel creator
 */

bool CtmRNASubPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CtmRNASubPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CtmRNASubPanel creation
    return true;
}


/*!
 * CtmRNASubPanel destructor
 */

CtmRNASubPanel::~CtmRNASubPanel()
{
////@begin CtmRNASubPanel destruction
////@end CtmRNASubPanel destruction
}


/*!
 * Member initialisation
 */

void CtmRNASubPanel::Init()
{
////@begin CtmRNASubPanel member initialisation
    m_TagPeptideStartCtrl = NULL;
    m_TagPeptideStopCtrl = NULL;
    m_ProductCtrl = NULL;
////@end CtmRNASubPanel member initialisation
}


/*!
 * Control creation for CtmRNASubPanel
 */

void CtmRNASubPanel::CreateControls()
{    
////@begin CtmRNASubPanel content construction
    CtmRNASubPanel* itemPanel1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel1->SetSizer(itemFlexGridSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Tag Peptide"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Start"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TagPeptideStartCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_TagPeptideStartCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TagPeptideStopCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL15, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_TagPeptideStopCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Product"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProductCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL16, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(m_ProductCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CtmRNASubPanel content construction
}


bool CtmRNASubPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }

    string start = "", stop = "";
    if (m_Gen->IsSetQuals()) {
        // find tag peptide qual
        NON_CONST_ITERATE (CRNA_qual_set::Tdata, q, m_Gen->SetQuals().Set()) {
            if ((*q)->IsSetQual() && NStr::Equal((*q)->GetQual(), "tag_peptide")
                && (*q)->IsSetVal()) {
                // parse for start and stop
                vector<string> tokens;
                NStr::Split((*q)->GetVal(), "..", tokens, NStr::fSplit_Tokenize);
                if (tokens.size() == 1) {
                    start =tokens[0];
                } else if (tokens.size() > 1) {
                    start = tokens[0];
                    stop = tokens[1];
                }
            }
        }
    }
    m_TagPeptideStartCtrl->SetValue(ToWxString(start));
    m_TagPeptideStopCtrl->SetValue(ToWxString(stop));

    if (m_Gen->IsSetProduct()) {
        m_ProductCtrl->SetValue(ToWxString(m_Gen->GetProduct()));
    } else {
        m_ProductCtrl->SetValue(wxEmptyString);
    }
    return true;
}


bool CtmRNASubPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }

    string start = ToStdString (m_TagPeptideStartCtrl->GetValue());
    string stop = ToStdString (m_TagPeptideStopCtrl->GetValue());
    string tm = "";
    if (NStr::IsBlank(start)) {
        if (NStr::IsBlank(stop)) {
            // tm stays blank
        } else {
            tm = stop + ".." + stop;
        }
    } else {
        if (NStr::IsBlank (stop)) {
            tm = start + ".." + start;
        } else {
            tm = start + ".." + stop;
        }
    }
    // now find tag_peptide qual and remove or edit
    bool found = false;
    if (m_Gen->IsSetQuals()) {
        CRNA_qual_set::Tdata::iterator q = m_Gen->SetQuals().Set().begin();
        while (q != m_Gen->SetQuals().Set().end()) {
            if ((*q)->IsSetQual() && NStr::Equal((*q)->GetQual(), "tag_peptide")) {
                found = true;
                if (NStr::IsBlank(tm)) {
                    q = m_Gen->SetQuals().Set().erase(q);
                } else {
                    (*q)->SetVal(tm);
                    q++;
                }
            } else {
                q++;
            }
        }
    }
    if (!found && !NStr::IsBlank(tm)) {
        CRef<CRNA_qual> tmqual(new CRNA_qual());
        tmqual->SetQual("tag_peptide");
        tmqual->SetVal(tm);
        m_Gen->SetQuals().Set().push_back(tmqual);
    } else {
        if (m_Gen->IsSetQuals() && m_Gen->GetQuals().Get().empty()) {
            m_Gen->ResetQuals();
        } 
    }
    
    string product = ToStdString (m_ProductCtrl->GetValue());
    if (NStr::IsBlank(product)) {
        m_Gen->ResetProduct();
    } else {
        m_Gen->SetProduct(product);
    }
    return true;
}


/*!
 * Should we show tooltips?
 */

bool CtmRNASubPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CtmRNASubPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CtmRNASubPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CtmRNASubPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CtmRNASubPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CtmRNASubPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CtmRNASubPanel icon retrieval
}

END_NCBI_SCOPE
