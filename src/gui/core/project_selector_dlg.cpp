/*  $Id: project_selector_dlg.cpp 39749 2017-11-01 14:23:15Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <gui/core/project_selector_dlg.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>

////@begin XPM images
////@end XPM images

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/statbox.h>


BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CProjectSelectorDlg, CDialog )

BEGIN_EVENT_TABLE( CProjectSelectorDlg, CDialog )

////@begin CProjectSelectorDlg event table entries
////@end CProjectSelectorDlg event table entries

END_EVENT_TABLE()


CProjectSelectorDlg::CProjectSelectorDlg() : m_Objects()
{
    Init();
}


CProjectSelectorDlg::CProjectSelectorDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Objects()
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CProjectSelectorDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProjectSelectorDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProjectSelectorDlg creation
    return true;
}


CProjectSelectorDlg::~CProjectSelectorDlg()
{
////@begin CProjectSelectorDlg destruction
////@end CProjectSelectorDlg destruction
}


void CProjectSelectorDlg::Init()
{
////@begin CProjectSelectorDlg member initialisation
    m_Sizer = NULL;
    m_List = NULL;
    m_ProjectSelPanel = NULL;
////@end CProjectSelectorDlg member initialisation
}


void CProjectSelectorDlg::CreateControls()
{
////@begin CProjectSelectorDlg content construction
    CProjectSelectorDlg* itemCDialog1 = this;

    m_Sizer = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(m_Sizer);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Project items to be created"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    m_Sizer->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    m_List = new CObjectListWidget( itemStaticBoxSizer3->GetStaticBox(), ID_COBJECTLISTWIDGET, wxDefaultPosition, itemStaticBoxSizer3->GetStaticBox()->ConvertDialogToPixels(wxSize(300, 50)), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_List, 1, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Project to place new items"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    m_Sizer->Add(itemStaticBoxSizer5, 2, wxGROW|wxALL, 5);

    m_ProjectSelPanel = new CProjectSelectorPanel( itemStaticBoxSizer5->GetStaticBox(), ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer5->Add(m_ProjectSelPanel, 1, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer7 = new wxStdDialogButtonSizer;

    m_Sizer->Add(itemStdDialogButtonSizer7, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton8 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton8->SetDefault();
    itemStdDialogButtonSizer7->AddButton(itemButton8);

    wxButton* itemButton9 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer7->AddButton(itemButton9);

    itemStdDialogButtonSizer7->Realize();

////@end CProjectSelectorDlg content construction

    m_ProjectSelPanel->SetSimpleMode();
    m_Params.m_EnableDecideLater = false;    
}


void CProjectSelectorDlg::SetProjectService(CProjectService* prj_srv)
{
    m_ProjectService = prj_srv;
    m_ProjectSelPanel->SetProjectService(m_ProjectService);    
}


void CProjectSelectorDlg::SetObjects(TConstScopedObjects& objects)
{    
    m_List->SetObjects(objects);    
    m_Objects = &objects;
    m_Params.m_EnableCreateSeparate = (objects.size() > 1);  
}

/*void CProjectSelectorDlg::SetParams(const SProjectSelectorParams& params)
{
    m_Params = params;
}*/


void CProjectSelectorDlg::GetParams(SProjectSelectorParams& params) const
{
    m_ProjectSelPanel->GetParams(params);
    //params = m_Params;
}


bool CProjectSelectorDlg::ShowToolTips()
{
    return true;
}


wxBitmap CProjectSelectorDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectSelectorDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProjectSelectorDlg bitmap retrieval
}


wxIcon CProjectSelectorDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectSelectorDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProjectSelectorDlg icon retrieval
}


bool CProjectSelectorDlg::TransferDataToWindow()
{   
    if(CDialog::TransferDataToWindow()) {      
        // locator
        if (m_ProjectService && m_Objects){
            m_Params.SelectProjectByObjects(*m_Objects, m_ProjectService);
        }        
        m_ProjectSelPanel->SetParams(m_Params);
        
        return m_ProjectSelPanel->TransferDataToWindow();
    }
    return false;
}


bool CProjectSelectorDlg::TransferDataFromWindow()
{
    if(m_ProjectSelPanel->TransferDataFromWindow()) {
        return CDialog::TransferDataFromWindow();
    }
    return false;
}


static const char* kTableTag = "Table";

void CProjectSelectorDlg::x_SaveSettings(CRegistryWriteView view) const
{
    CRegistryWriteView table_view = view.GetWriteView(kTableTag);
    m_List->SaveTableSettings(table_view);
}


void CProjectSelectorDlg::x_LoadSettings(const CRegistryReadView& view)
{
    CRegistryReadView table_view = view.GetReadView(kTableTag);
    m_List->LoadTableSettings(table_view);
}

END_NCBI_SCOPE
