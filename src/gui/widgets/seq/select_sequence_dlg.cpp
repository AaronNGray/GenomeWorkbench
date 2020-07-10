/*  $Id: select_sequence_dlg.cpp 35537 2016-05-18 17:18:09Z katargir $
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
////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/button.h>

#include "select_sequence_dlg.hpp"

#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CSelectSequenceDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSelectSequenceDlg, CDialog )


/*!
 * CSelectSequenceDlg event table definition
 */

BEGIN_EVENT_TABLE( CSelectSequenceDlg, CDialog )

////@begin CSelectSequenceDlg event table entries
    EVT_LIST_ITEM_ACTIVATED( ID_LISTCTRL, CSelectSequenceDlg::OnListctrlItemActivated )

////@end CSelectSequenceDlg event table entries

END_EVENT_TABLE()


/*!
 * CSelectSequenceDlg constructors
 */

 CSelectSequenceDlg::CSelectSequenceDlg() : m_Seqs()
{
    Init();
}

CSelectSequenceDlg::CSelectSequenceDlg(wxWindow* parent, vector<CFlatFileSeq>& seqs, const wxString& select)
    : m_SequenceId(select), m_Seqs(&seqs)
{
    Init();
    Create(parent, SYMBOL_CSELECTSEQUENCEDLG_IDNAME, SYMBOL_CSELECTSEQUENCEDLG_TITLE, SYMBOL_CSELECTSEQUENCEDLG_POSITION, SYMBOL_CSELECTSEQUENCEDLG_SIZE, SYMBOL_CSELECTSEQUENCEDLG_STYLE);
}



/*!
 * CSelectSequenceDlg creator
 */

bool CSelectSequenceDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSelectSequenceDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSelectSequenceDlg creation
    return true;
}


/*!
 * CSelectSequenceDlg destructor
 */

CSelectSequenceDlg::~CSelectSequenceDlg()
{
////@begin CSelectSequenceDlg destruction
////@end CSelectSequenceDlg destruction
}


/*!
 * Member initialisation
 */

void CSelectSequenceDlg::Init()
{
////@begin CSelectSequenceDlg member initialisation
    m_SeqList = NULL;
////@end CSelectSequenceDlg member initialisation
}

class CListCtrlSeqs : public wxListCtrl
{
    DECLARE_EVENT_TABLE()

public:
    CListCtrlSeqs(wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos,
        const wxSize& size,
        long style);

    virtual wxString OnGetItemText(long item, long column) const;
    void OnSize(wxSizeEvent& event);

    void SetSeqs(vector<CFlatFileSeq>& seqs);

private:
    vector<CFlatFileSeq>* m_Seqs;
};

BEGIN_EVENT_TABLE( CListCtrlSeqs, wxListCtrl )    
    EVT_SIZE( CListCtrlSeqs::OnSize )
END_EVENT_TABLE()


CListCtrlSeqs::CListCtrlSeqs(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxListCtrl(parent, id, pos, size, style)
{
    AppendColumn(wxT("Sequence Id"));
}

void CListCtrlSeqs::OnSize(wxSizeEvent& event)
{
    wxRect rect = GetClientRect();
    SetColumnWidth(0, rect.GetWidth());
    event.Skip();
}

void CListCtrlSeqs::SetSeqs(vector<CFlatFileSeq>& seqs)
{
    m_Seqs = &seqs;
    SetItemCount((long)m_Seqs->size());
}

wxString CListCtrlSeqs::OnGetItemText(long item, long column) const
{
    if (column != 0 || !m_Seqs || (size_t)item >= m_Seqs->size())
        return wxEmptyString;

    return ToWxString((*m_Seqs)[item].m_Name);
}


/*!
 * Control creation for CSelectSequenceDlg
 */

void CSelectSequenceDlg::CreateControls()
{    
////@begin CSelectSequenceDlg content construction
    CSelectSequenceDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_SeqList = new CListCtrlSeqs( itemCDialog1, ID_LISTCTRL, wxDefaultPosition, itemCDialog1->ConvertDialogToPixels(wxSize(100, 150)), wxLC_REPORT|wxLC_VIRTUAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_SeqList, 1, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer4 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer4, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton5 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer4->AddButton(itemButton5);

    wxButton* itemButton6 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer4->AddButton(itemButton6);

    itemStdDialogButtonSizer4->Realize();

////@end CSelectSequenceDlg content construction

    m_SeqList->SetSeqs(*m_Seqs);
    if (!m_SequenceId.empty()) {
        for (size_t i = 0; i < m_Seqs->size(); ++i) {
            if ((*m_Seqs)[i].m_Name == m_SequenceId) {
                m_SeqList->SetItemState((long)i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                m_SeqList->EnsureVisible((long)i);
            }
        }
    }
}

bool CSelectSequenceDlg::TransferDataFromWindow()
{
    if (m_SeqList->GetSelectedItemCount() != 1) {
        wxMessageBox(wxT("Please, select sequence id."), wxT("Error"),
                        wxOK | wxICON_ERROR, this);
        return false;
    }

    long item = -1;
    item = m_SeqList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    m_SequenceId = m_SeqList->GetItemText(item);
    return CDialog::TransferDataFromWindow();
}

/*!
 * Should we show tooltips?
 */

bool CSelectSequenceDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSelectSequenceDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSelectSequenceDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSelectSequenceDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSelectSequenceDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSelectSequenceDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSelectSequenceDlg icon retrieval
}

void CSelectSequenceDlg::OnListctrlItemActivated(wxListEvent& event)
{
    if (TransferDataFromWindow())
        EndModal(wxID_OK);
}

END_NCBI_SCOPE
