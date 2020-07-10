/*  $Id: trna_subpanel.cpp 35570 2016-05-25 14:38:34Z asztalos $
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
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objmgr/scope.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <util/sequtil/sequtil_convert.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "trna_subpanel.hpp"
#include "trna_product_panel.hpp"
#include "trna_recognized_codons_panel.hpp"
#include "location_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CtRNASubPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CtRNASubPanel, wxPanel )


/*!
 * CtRNASubPanel event table definition
 */

BEGIN_EVENT_TABLE( CtRNASubPanel, wxPanel )

////@begin CtRNASubPanel event table entries
////@end CtRNASubPanel event table entries

END_EVENT_TABLE()


/*!
 * CtRNASubPanel constructors
 */

CtRNASubPanel::CtRNASubPanel()
{
    Init();
}

CtRNASubPanel::CtRNASubPanel(wxWindow* parent, CRef<CTrna_ext> trna, CConstRef<CSeq_feat> feat, CScope& scope, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_trna(trna), m_Feat(feat)
{
    Init();
    m_Scope.Reset(&scope);
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CtRNASubPanel creator
 */

bool CtRNASubPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CtRNASubPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CtRNASubPanel creation
    return true;
}


/*!
 * CtRNASubPanel destructor
 */

CtRNASubPanel::~CtRNASubPanel()
{
////@begin CtRNASubPanel destruction
////@end CtRNASubPanel destruction
}


/*!
 * Member initialisation
 */

void CtRNASubPanel::Init()
{
////@begin CtRNASubPanel member initialisation
    m_tRNATree = NULL;
////@end CtRNASubPanel member initialisation
}


/*!
 * Control creation for CtRNASubPanel
 */

void CtRNASubPanel::CreateControls()
{    
////@begin CtRNASubPanel content construction
    CtRNASubPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_tRNATree = new wxTreebook( itemPanel1, ID_TREEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    itemBoxSizer2->Add(m_tRNATree, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CtRNASubPanel content construction
   
    wxWindow* panel;
    panel = new CtRNAProductPanel (m_tRNATree, m_trna, m_Feat);
    m_tRNATree->AddPage (panel, wxT("Product"));

    panel = new CtRNARecognizedCodonsPanel (m_tRNATree, *m_trna);
    m_tRNATree->AddPage (panel, wxT("Codons Recognized"));

    m_AnticodonLocPanel = new CLocationPanel(m_tRNATree, m_trna->SetAnticodon(), *m_Scope);
    m_tRNATree->AddPage (m_AnticodonLocPanel, wxT("Anticodon"));

}


void CtRNASubPanel::SetGBQualPanel(CGBQualPanel* gbqual_panel)
{
    CtRNAProductPanel* product = 0;
    for (size_t i = 0; i < m_tRNATree->GetPageCount(); ++i) {
        if ((product = dynamic_cast<CtRNAProductPanel*>(m_tRNATree->GetPage(i))) != 0) {
            product->SetGBQualPanel(gbqual_panel);
            break;
        }
    }
}

bool CtRNASubPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    for (size_t i = 0; i < m_tRNATree->GetPageCount(); ++i) {
        m_tRNATree->GetPage(i)->TransferDataToWindow();
    }

    return true;
}


bool CtRNASubPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    for (size_t i = 0; i < m_tRNATree->GetPageCount(); ++i) {
        m_tRNATree->GetPage(i)->TransferDataFromWindow();
    }

    // anticodon
    if (m_AnticodonLocPanel) {
        CRef<objects::CSeq_loc> loc = m_AnticodonLocPanel->GetSeq_loc();
        if (!loc || loc->IsNull() || loc->IsEmpty()) {
            m_trna->ResetAnticodon();
        } else {
            m_trna->SetAnticodon(*loc);
        }
    } else {
        m_trna->ResetAnticodon();
    }

    return true;
}


/*!
 * Should we show tooltips?
 */

bool CtRNASubPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CtRNASubPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CtRNASubPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CtRNASubPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CtRNASubPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CtRNASubPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CtRNASubPanel icon retrieval
}

END_NCBI_SCOPE
