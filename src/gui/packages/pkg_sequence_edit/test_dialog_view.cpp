/*  $Id: test_dialog_view.cpp 30384 2014-05-08 19:29:29Z katargir $
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

#include <gui/utils/view_event.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include "test_dialog_view.hpp"

#include <wx/sizer.h>
#include <wx/button.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CTestDialogView type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTestDialogView, CGuiWidgetDlg )


/*!
 * CTestDialogView event table definition
 */

BEGIN_EVENT_TABLE( CTestDialogView, CGuiWidgetDlg )

////@begin CTestDialogView event table entries
////@end CTestDialogView event table entries

END_EVENT_TABLE()


wxWindow* CTestDialogView::CreateDialog(wxWindow* parent)
{
    return new CTestDialogView(parent);
}

/*!
 * CTestDialogView constructors
 */

CTestDialogView::CTestDialogView()
{
    Init();
}

CTestDialogView::CTestDialogView( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CTestDialogView creator
 */

bool CTestDialogView::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTestDialogView creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CGuiWidgetDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTestDialogView creation
    return true;
}


/*!
 * CTestDialogView destructor
 */

CTestDialogView::~CTestDialogView()
{
////@begin CTestDialogView destruction
////@end CTestDialogView destruction

    if (m_TextPanel) {
        m_TextPanel->SetHost(0);
        m_TextPanel->GetWidget()->SetHost(0);
    }
}


/*!
 * Member initialisation
 */

void CTestDialogView::Init()
{
////@begin CTestDialogView member initialisation
    m_TextPanel = NULL;
////@end CTestDialogView member initialisation
}


/*!
 * Control creation for CTestDialogView
 */

void CTestDialogView::CreateControls()
{    
////@begin CTestDialogView content construction
    CTestDialogView* itemCGuiWidgetDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCGuiWidgetDlg1->SetSizer(itemBoxSizer2);

    m_TextPanel = new CTextPanel( itemCGuiWidgetDlg1, ID_PANEL1, wxDefaultPosition, wxSize(350, 200), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_TextPanel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemCGuiWidgetDlg1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CTestDialogView content construction

    m_TextPanel->Show();
}

/// ITextWidgetHost implementation

void CTestDialogView::WidgetSelectionChanged()
{
    if (m_WidgetHost) m_WidgetHost->WidgetSelectionChanged();
}

/// IGuiWidgetHost implementation

void CTestDialogView::SetHost(IGuiWidgetHost* host)
{
    CGuiWidgetDlg::SetHost(host);
    if (m_TextPanel) m_TextPanel->SetHost(host);
}

bool CTestDialogView::InitWidget(TConstScopedObjects& objects)
{
    if (!m_TextPanel || !m_TextPanel->InitWidget(objects)) return false;
    m_TextPanel->GetWidget()->SetHost(this);
    return true;
}

void CTestDialogView::GetSelectedObjects(TConstObjects& objects) const
{
    if (m_TextPanel) m_TextPanel->GetSelectedObjects(objects);
}

void CTestDialogView::SetSelectedObjects(const TConstObjects& objects)
{
    if (m_TextPanel) m_TextPanel->SetSelectedObjects(objects);
}

const CObject* CTestDialogView::GetOrigObject() const
{
    return m_TextPanel ? m_TextPanel->GetOrigObject() : 0;
}

void CTestDialogView::SetUndoManager(ICommandProccessor* cmdProccessor)
{
    if (m_TextPanel) m_TextPanel->SetUndoManager(cmdProccessor);
}

void CTestDialogView::DataChanging()
{
    if (m_TextPanel) m_TextPanel->DataChanging();
}

void CTestDialogView::DataChanged()
{
    if (m_TextPanel) m_TextPanel->DataChanged();
}

void CTestDialogView::GetActiveObjects(vector<TConstScopedObjects>& objects)
{
    if (m_TextPanel) m_TextPanel->GetActiveObjects(objects);
}

/*!
 * Should we show tooltips?
 */

bool CTestDialogView::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTestDialogView::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTestDialogView bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTestDialogView bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTestDialogView::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTestDialogView icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTestDialogView icon retrieval
}

END_NCBI_SCOPE
