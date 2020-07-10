/*  $Id: add_transl_except_dlg.cpp 38626 2017-06-05 13:53:11Z asztalos $
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

#include <gui/packages/pkg_sequence_edit/add_transl_except_dlg.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CAddTranslExceptDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAddTranslExceptDlg, CBulkCmdDlg )


/*!
 * CAddTranslExceptDlg event table definition
 */

BEGIN_EVENT_TABLE( CAddTranslExceptDlg, CBulkCmdDlg )

////@begin CAddTranslExceptDlg event table entries
////@end CAddTranslExceptDlg event table entries

END_EVENT_TABLE()


/*!
 * CAddTranslExceptDlg constructors
 */

CAddTranslExceptDlg::CAddTranslExceptDlg()
{
    Init();
}

CAddTranslExceptDlg::CAddTranslExceptDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CAddTranslExceptDlg creator
 */

bool CAddTranslExceptDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddTranslExceptDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAddTranslExceptDlg creation
    return true;
}


/*!
 * CAddTranslExceptDlg destructor
 */

CAddTranslExceptDlg::~CAddTranslExceptDlg()
{
////@begin CAddTranslExceptDlg destruction
////@end CAddTranslExceptDlg destruction
}


/*!
 * Member initialisation
 */

void CAddTranslExceptDlg::Init()
{
////@begin CAddTranslExceptDlg member initialisation
    m_CDSCommentTxt = NULL;
    m_OverhangBtn = NULL;
    m_ExtendBtn = NULL;
    m_AdjustGeneBtn = NULL;
    m_OkCancel = NULL;
////@end CAddTranslExceptDlg member initialisation
}


/*!
 * Control creation for CAddTranslExceptDlg
 */

void CAddTranslExceptDlg::CreateControls()
{    
////@begin CAddTranslExceptDlg content construction
    CAddTranslExceptDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("CDS comment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CDSCommentTxt = new wxTextCtrl(itemCBulkCmdDlg1, ID_ATE_CDS_COMMENT_TXT, _("TAA stop codon is completed by the addition of 3' A residues to the mRNA"), wxDefaultPosition, wxSize(320, 50), wxTE_MULTILINE | wxTE_WORDWRAP);
    itemBoxSizer3->Add(m_CDSCommentTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OverhangBtn = new wxCheckBox( itemCBulkCmdDlg1, ID_ATE_OVERHANG_BTN, _("Overhang must be T or A"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OverhangBtn->SetValue(true);
    itemBoxSizer2->Add(m_OverhangBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ExtendBtn = new wxCheckBox( itemCBulkCmdDlg1, ID_ATE_EXTEND_BTN, _("Extend for T/TA overhang"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExtendBtn->SetValue(false);
    itemBoxSizer2->Add(m_ExtendBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AdjustGeneBtn = new wxCheckBox( itemCBulkCmdDlg1, ID_ATE_ADJUST_GENE_BTN, _("Adjust gene to match coding region location"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AdjustGeneBtn->SetValue(true);
    itemBoxSizer2->Add(m_AdjustGeneBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, ID_ATE_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CAddTranslExceptDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CAddTranslExceptDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAddTranslExceptDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddTranslExceptDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddTranslExceptDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAddTranslExceptDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddTranslExceptDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddTranslExceptDlg icon retrieval
}


CRef<CCmdComposite> CAddTranslExceptDlg::GetCommand()
{
    CRef<CCmdComposite> cmd = SetTranslExcept(m_TopSeqEntry,
                                              ToStdString(m_CDSCommentTxt->GetValue()),
                                              m_OverhangBtn->GetValue(),
                                              m_ExtendBtn->GetValue(),
                                              m_AdjustGeneBtn->GetValue());
    if (cmd) {
        m_ErrorMessage = "";
    } else {
        m_ErrorMessage = "Found no coding regions to adjust";
    }
    return cmd;
}


string CAddTranslExceptDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}



END_NCBI_SCOPE
