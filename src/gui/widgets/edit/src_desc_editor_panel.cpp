/*  $Id: src_desc_editor_panel.cpp 43089 2019-05-13 21:05:28Z filippov $
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
#include "wx/imaglist.h"
////@end includes
#include <wx/sizer.h>
#include <wx/msgdlg.h> 
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <gui/widgets/edit/src_desc_editor_panel.hpp>

#include <gui/widgets/edit/biosource_autocomplete.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include "loc_gcode_panel.hpp"
#include "biosourcepanel.hpp"
#include "src_other_pnl.hpp"


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSrcDescEditorPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSrcDescEditorPanel, wxPanel )


/*!
 * CSrcDescEditorPanel event table definition
 */

BEGIN_EVENT_TABLE( CSrcDescEditorPanel, wxPanel )

////@begin CSrcDescEditorPanel event table entries
////@end CSrcDescEditorPanel event table entries

END_EVENT_TABLE()


/*!
 * CSrcDescEditorPanel constructors
 */

CSrcDescEditorPanel::CSrcDescEditorPanel()
{
    Init();
}

CSrcDescEditorPanel::CSrcDescEditorPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSrcDescEditorPanel creator
 */

bool CSrcDescEditorPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSrcDescEditorPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSrcDescEditorPanel creation
    return true;
}


/*!
 * CSrcDescEditorPanel destructor
 */

CSrcDescEditorPanel::~CSrcDescEditorPanel()
{
////@begin CSrcDescEditorPanel destruction
////@end CSrcDescEditorPanel destruction
}


/*!
 * Member initialisation
 */

void CSrcDescEditorPanel::Init()
{
////@begin CSrcDescEditorPanel member initialisation
    m_SourceTreebook = NULL;
////@end CSrcDescEditorPanel member initialisation
}


/*!
 * Control creation for CSrcDescEditorPanel
 */

void CSrcDescEditorPanel::CreateControls()
{    
////@begin CSrcDescEditorPanel content construction
    CSrcDescEditorPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_SourceTreebook = new wxTreebook( itemPanel1, ID_TREEBOOK1, wxDefaultPosition, wxSize(-1, 500), wxBK_DEFAULT );
    itemBoxSizer2->Add(m_SourceTreebook, 1, wxGROW|wxALL, 5);

////@end CSrcDescEditorPanel content construction

    wxWindow* panel = new wxPanel(m_SourceTreebook, wxID_ANY, wxDefaultPosition, wxSize(550, 460), wxBK_DEFAULT );
    m_SourceTreebook->AddPage(panel, wxT("Source Modifiers and Extra"));
    panel = new wxPanel(m_SourceTreebook, wxID_ANY, wxDefaultPosition, wxSize(550, 460), wxBK_DEFAULT );
    m_SourceTreebook->AddPage(panel, wxT("other"));
    
}


/*!
 * Should we show tooltips?
 */

bool CSrcDescEditorPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSrcDescEditorPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSrcDescEditorPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSrcDescEditorPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSrcDescEditorPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSrcDescEditorPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSrcDescEditorPanel icon retrieval
}


bool CSrcDescEditorPanel::TransferDataToWindow()
{
    return wxPanel::TransferDataToWindow();
}


bool CSrcDescEditorPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_SrcPanel->PopulateBioSource(*m_BioSource);
    m_LocAndGCodePanel->PopulateLocAndGC(*m_BioSource);
    m_SrcOtherPanel->TransferToOrgRef(m_BioSource->SetOrg());
    return true;
}


void CSrcDescEditorPanel::ChangeSeqdesc(const CSeqdesc& desc)
{
    m_BioSource.Reset(new CBioSource());
    m_BioSource->Assign(desc.GetSource());

    m_SourceTreebook->DeleteAllPages();

    m_SrcPanel = new CBioSourcePanel(m_SourceTreebook, *m_BioSource, this);
    m_SourceTreebook->AddPage(m_SrcPanel, wxT("Source Modifiers"));
    
    m_LocAndGCodePanel = new CLocAndGCodePanel(m_SourceTreebook, *m_BioSource);
    m_SourceTreebook->AddPage(m_LocAndGCodePanel, wxT("Genetic Code"));
                        
    m_SrcOtherPanel = new CSourceOtherPanel(m_SourceTreebook);
    m_SrcOtherPanel->TransferFromOrgRef(m_BioSource->SetOrg());
    m_SourceTreebook->AddPage(m_SrcOtherPanel, wxT("Other"));
                    
    TransferDataToWindow();
    m_SourceTreebook->Refresh();
    Refresh();
}


void CSrcDescEditorPanel::UpdateSeqdesc(CSeqdesc& desc)
{
    m_SrcPanel->PopulateBioSource(desc.SetSource());
    m_LocAndGCodePanel->PopulateLocAndGC(desc.SetSource());
    m_SrcOtherPanel->TransferToOrgRef(desc.SetSource().SetOrg());

    objects::COrg_ref& org = desc.SetSource().SetOrg();
    if (RunningInsideNCBI())
    {
        CBioSourceAutoComplete::AutoFill(org);
    }
    else if ( org.IsSetTaxname() && !NStr::IsBlank (org.GetTaxname()) && org.GetTaxId() == 0 && org.TableLookup(org.GetTaxname()) && 
              wxMessageBox("Would you like to load default lineage and genetic code values for this organism?",  "Please confirm", wxICON_QUESTION | wxYES_NO) == wxYES )
    {
        CBioSourceAutoComplete::AutoFill(org);
    }
    CCleanup cleanup;
    cleanup.BasicCleanup(desc.SetSource());

}

void CSrcDescEditorPanel::UpdateTaxname()
{
    m_LocAndGCodePanel->OnChangedTaxname();
    m_SrcOtherPanel->OnChangedTaxname();
}


END_NCBI_SCOPE
