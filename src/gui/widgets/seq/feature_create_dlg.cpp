/*  $Id: feature_create_dlg.cpp 43197 2019-05-26 01:34:33Z ucko $
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


#include <ncbi_pch.hpp>

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/srchctrl.h>
#include <wx/choice.h>
#include <wx/grid.h>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Rsite_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Cdregion.hpp>


////@begin includes
////@end includes

#include <gui/widgets/seq/feature_create_dlg.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CFeatureCreateDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFeatureCreateDialog, wxDialog )


/*!
 * CFeatureCreateDialog event table definition
 */

BEGIN_EVENT_TABLE( CFeatureCreateDialog, wxDialog )
    EVT_CHOICE(ID_CHOICE6, CFeatureCreateDialog::OnFeatureTypeChanged)
END_EVENT_TABLE()


/*!
 * CFeatureCreateDialog constructors
 */

CFeatureCreateDialog::CFeatureCreateDialog()
{
    Init();
}

CFeatureCreateDialog::CFeatureCreateDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CFeatureCreateDialog creator
 */

bool CFeatureCreateDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFeatureCreateDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFeatureCreateDialog creation
    return true;
}


/*!
 * CFeatureCreateDialog destructor
 */

CFeatureCreateDialog::~CFeatureCreateDialog()
{
////@begin CFeatureCreateDialog destruction
////@end CFeatureCreateDialog destruction
}


/*!
 * Member initialisation
 */

void CFeatureCreateDialog::Init()
{
////@begin CFeatureCreateDialog member initialisation
////@end CFeatureCreateDialog member initialisation
    m_pIn  = NULL;
    m_pOut = NULL;
    m_pFeatType = NULL;
    m_pQualifiers = NULL;   
}


/*!
 * Control creation for CFeatureCreateDialog
 */

void CFeatureCreateDialog::CreateControls()
{
////@begin CFeatureCreateDialog content construction
    CFeatureCreateDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Feature Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice5Strings;
    wxChoice* itemChoice5 = new wxChoice( itemDialog1, ID_CHOICE6, wxDefaultPosition, wxDefaultSize, itemChoice5Strings, 0 );
    itemFlexGridSizer3->Add(itemChoice5, 1, wxGROW|wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Label"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(-1, 50), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableCol(1);

    wxStaticBox* itemStaticBoxSizer10Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Qualifiers"));
    wxStaticBoxSizer* itemStaticBoxSizer10 = new wxStaticBoxSizer(itemStaticBoxSizer10Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer10, 0, wxGROW|wxALL, 5);

    wxGrid* itemGrid11 = new wxGrid( itemDialog1, ID_GRID1, wxDefaultPosition, wxSize(250, 200), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemGrid11->SetDefaultColSize(25);
    itemGrid11->SetDefaultRowSize(25);
    itemGrid11->SetColLabelSize(25);
    itemGrid11->SetRowLabelSize(50);
    itemGrid11->CreateGrid(1, 2, wxGrid::wxGridSelectCells);
    itemStaticBoxSizer10->Add(itemGrid11, 1, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine12 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine12, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer13 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer13, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer13->AddButton(itemButton14);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer13->AddButton(itemButton15);

    itemStdDialogButtonSizer13->Realize();

    // Set validators
    itemTextCtrl7->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Label) );
    itemTextCtrl9->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Comments) );
////@end CFeatureCreateDialog content construction

    itemGrid11->SetDefaultColSize(110);
    itemGrid11->SetDefaultRowSize(20);
    itemGrid11->SetColLabelSize(20);
    itemGrid11->SetRowLabelSize(0);
        
    m_pFeatType     = itemChoice5;
    m_pQualifiers   = itemGrid11;
    
    m_pQualifiers->SetColLabelValue(0, wxT("Name"));
    m_pQualifiers->SetColLabelValue(1, wxT("Value"));
    
}


/*!
 * Should we show tooltips?
 */

bool CFeatureCreateDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFeatureCreateDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeatureCreateDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeatureCreateDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFeatureCreateDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeatureCreateDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeatureCreateDialog icon retrieval
}

void CFeatureCreateDialog::OnFeatureTypeChanged(wxCommandEvent& event)
{
    x_UpdateQualifiers();
}



void CFeatureCreateDialog::x_InitFeatTypes()
{
    const CFeatList* feat_list = CSeqFeatData::GetFeatList();
    vector <int> allowed;
    ITERATE(vector<CSeqFeatData::ESubtype>, subt, m_Options){   
        allowed.push_back(*subt);
    }

    int selected = 0;
    ITERATE(CFeatList, ft_it, *feat_list) {     
        const CFeatListItem& item = *ft_it;
        string desc = item.GetDescription();
        int    subt = item.GetSubtype();
        if (find(allowed.begin(), allowed.end(), subt) != allowed.end()){
            int idx = m_pFeatType->Append(ToWxString(desc),
                                          new int(item.GetSubtype()));
            if (item.GetSubtype() == m_Selected) {
                selected = idx;
            }
        }
    }   
    m_pFeatType->SetSelection(selected);
}

void CFeatureCreateDialog::x_UpdateQualifiers()
{
    int feat_sel;
    if (m_pFeatType && ((feat_sel=m_pFeatType->GetSelection())!=wxNOT_FOUND)){
        
        if (m_pQualifiers->GetNumberRows()) {
            m_pQualifiers->DeleteRows(0, m_pQualifiers->GetNumberRows());
        }

        int subtype = *((int*)m_pFeatType->GetClientData(feat_sel));
        const auto& quals = CSeqFeatData::GetLegalQualifiers
            ((CSeqFeatData::ESubtype)subtype);

        for (auto q : quals) {
            string qual_name = CSeqFeatData::GetQualifierAsString(q);
            m_pQualifiers->AppendRows();
            m_pQualifiers->SetCellValue(m_pQualifiers->GetNumberRows() - 1, 0,
                                        ToWxString(qual_name));         
            m_pQualifiers->SetReadOnly(m_pQualifiers->GetNumberRows() - 1, 0);
        }
    }

}

void CFeatureCreateDialog::SetIO(TConstScopedObjects & in, TConstScopedObjects & out)
{
    m_pIn  = &in;
    m_pOut = &out;
}

void CFeatureCreateDialog::SetConstraints(vector <CSeqFeatData::ESubtype> & options,
                                          CSeqFeatData::ESubtype selected)
{
    m_Options  = options;
    m_Selected = selected;
}

bool CFeatureCreateDialog::TransferDataToWindow()
{
    x_InitFeatTypes();
    x_UpdateQualifiers();
    return wxDialog::TransferDataToWindow();
}

bool CFeatureCreateDialog::TransferDataFromWindow()
{
    if (m_pIn && m_pOut && m_pFeatType) {
        ITERATE(TConstScopedObjects, cso, *m_pIn) {
            // obtaining location
            const CSeq_loc * seq_loc =
                dynamic_cast<const CSeq_loc*>(cso->object.GetPointer());

            int feat_sel;
            if ((feat_sel=m_pFeatType->GetSelection())==wxNOT_FOUND) continue;      
        
            

            if (seq_loc) {
                // preparing feature
                CRef<CSeq_feat> feat(new CSeq_feat);
                feat->SetLocation(*const_cast<CSeq_loc*>(seq_loc));

                int subtype = *((int*)m_pFeatType->GetClientData(feat_sel));
                switch(subtype) {
                    case CSeqFeatData::eSubtype_gene:     feat->SetData().SetGene(); break;
                    case CSeqFeatData::eSubtype_mRNA:   
                    case CSeqFeatData::eSubtype_tRNA:
                    case CSeqFeatData::eSubtype_rRNA:     feat->SetData().SetRna();    break;   
                    case CSeqFeatData::eSubtype_exon:     feat->SetData().SetRegion(); break;   
                    case CSeqFeatData::eSubtype_region:   feat->SetData().SetRegion(); break;   
                    case CSeqFeatData::eSubtype_site:     feat->SetData().SetRsite();  break;
                    case CSeqFeatData::eSubtype_imp:      feat->SetData().SetImp(); break;
                    case CSeqFeatData::eSubtype_cdregion: {
                        feat->SetExp_ev(CSeq_feat::eExp_ev_not_experimental);
                        feat->SetData().SetCdregion().SetOrf(true);
                        feat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_one);                      
                    }
                }

                // add non-empty qualifiers
                int nmb_rows = m_pQualifiers->GetNumberRows();
                for (int i=0; i<nmb_rows; i++) {
                    string qual_name  = ToStdString(m_pQualifiers->GetCellValue(i, 0));
                    string qual_value = ToStdString(m_pQualifiers->GetCellValue(i, 1));
                    
                    if (!qual_value.empty()){                                               
                        feat->AddQualifier(qual_name, qual_value);
                    }                   
                }
                                
                m_pOut->push_back(SConstScopedObject(feat,
                                                     const_cast<CScope*>(cso->scope.GetPointer())));
            }       
        }
    }
    return wxDialog::TransferDataFromWindow();
}

END_NCBI_SCOPE
