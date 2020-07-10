/*  $Id: import_alignparams_dlg.cpp 36313 2016-09-12 18:40:41Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/loaders/text_align_params_panel.hpp>
#include <gui/packages/pkg_sequence_edit/import_alignparams_dlg.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

BEGIN_NCBI_SCOPE
/*!
 * CImportAlignParamsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CImportAlignParamsDlg, wxDialog )


/*!
 * CImportAlignParamsDlg event table definition
 */

BEGIN_EVENT_TABLE( CImportAlignParamsDlg, wxDialog )

////@begin CImportAlignParamsDlg event table entries
////@end CImportAlignParamsDlg event table entries

END_EVENT_TABLE()


/*!
 * CImportAlignParamsDlg constructors
 */

CImportAlignParamsDlg::CImportAlignParamsDlg()
{
    Init();
}

CImportAlignParamsDlg::CImportAlignParamsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CImportAlignParamsDlg creator
 */

bool CImportAlignParamsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CImportAlignParamsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CImportAlignParamsDlg creation
    return true;
}


/*!
 * CImportAlignParamsDlg destructor
 */

CImportAlignParamsDlg::~CImportAlignParamsDlg()
{
////@begin CImportAlignParamsDlg destruction
////@end CImportAlignParamsDlg destruction
}


/*!
 * Member initialisation
 */

void CImportAlignParamsDlg::Init()
{
////@begin CImportAlignParamsDlg member initialisation
////@end CImportAlignParamsDlg member initialisation
}


/*!
 * Control creation for CImportAlignParamsDlg
 */

void CImportAlignParamsDlg::CreateControls()
{    
////@begin CImportAlignParamsDlg content construction
    CImportAlignParamsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_AlignParamsPanel = new CTextAlignParamsPanel(itemDialog1, ID_FOREIGN); //, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER);
    itemBoxSizer2->Add(m_AlignParamsPanel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    const wxString msg = "\
Beginning Gap : When some of the sequences in an alignment are shorter or longer than others, beginning gap characters \r\n\
are added to the beginning of the sequence to maintain the correct spacing. These will not appear in your sequence file. \r\n\
Middle Gap : These characters are used to maintain the spacing inside an alignment.These are not nucleotides and \r\n\
will not appear as part of your sequence file. \r\n\
End Gap : When some of the sequences in an alignment are shorter or longer than others, end gap characters are added \r\n\
to the end of the sequence to maintain the correct spacing. These will not appear in your sequence file. \r\n\
Ambiguous/Unknown: These characters are used to represent indeterminate/ambiguous nucleotides. These will appear \r\n\
in your sequence file as 'n'. \r\n\
Match: These characters are used to indicate positions where sequences are identical to the first sequence. These will be \r\n\
replaced by the actual characters from the first sequence.";
    wxStaticText* itemStaticText4 = new wxStaticText(itemDialog1, wxID_STATIC, msg, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CImportAlignParamsDlg content construction
}

const CTextAlignParams& CImportAlignParamsDlg::GetData() const
{
    return m_AlignParamsPanel->GetData();
}

void CImportAlignParamsDlg::SetData(const CTextAlignParams& data)
{
    if (m_AlignParamsPanel) {
        m_AlignParamsPanel->SetData(data);
    }
}

bool CImportAlignParamsDlg::TransferDataToWindow()
{
    if (m_AlignParamsPanel) {
        m_AlignParamsPanel->TransferDataToWindow();
    }
    return true;
}

bool CImportAlignParamsDlg::TransferDataFromWindow()
{
    if (m_AlignParamsPanel) {
        m_AlignParamsPanel->TransferDataFromWindow();
    }
    return true;
}


/*!
 * Should we show tooltips?
 */

bool CImportAlignParamsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CImportAlignParamsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CImportAlignParamsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CImportAlignParamsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CImportAlignParamsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CImportAlignParamsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CImportAlignParamsDlg icon retrieval
}

END_NCBI_SCOPE
