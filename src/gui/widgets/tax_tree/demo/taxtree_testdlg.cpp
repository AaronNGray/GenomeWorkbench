/*  $Id: taxtree_testdlg.cpp 29680 2014-01-28 14:42:26Z falkrb $
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
 * Authors:
 */


#include <ncbi_pch.hpp>

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>



#include <objects/entrez2/entrez2_client.hpp>
#include <objects/entrez2/Entrez2_eval_boolean.hpp>
#include <objects/entrez2/Entrez2_boolean_exp.hpp>
#include <objects/entrez2/Entrez2_boolean_element.hpp>
#include <objects/entrez2/Entrez2_boolean_reply.hpp>
#include <objects/entrez2/Entrez2_id_list.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>

#include <gui/widgets/tax_tree/tax_tree.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <objects/taxon1/taxon1.hpp>
#include <algorithm>
#include <stack>


////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "taxtree_testdlg.hpp"

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


/*!
 * CTaxTreeTestDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTaxTreeTestDlg, wxDialog )


/*!
 * CTaxTreeTestDlg event table definition
 */

BEGIN_EVENT_TABLE( CTaxTreeTestDlg, wxDialog )

////@begin CTaxTreeTestDlg event table entries
    EVT_BUTTON( ID_BUTTON1, CTaxTreeTestDlg::OnSubmitClick )

    EVT_BUTTON( ID_BUTTON2, CTaxTreeTestDlg::OnRelatedClick )

////@end CTaxTreeTestDlg event table entries

END_EVENT_TABLE()


/*!
 * CTaxTreeTestDlg constructors
 */

CTaxTreeTestDlg::CTaxTreeTestDlg()
{
    Init();
}

CTaxTreeTestDlg::CTaxTreeTestDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CTaxTreeTestDlg creator
 */

bool CTaxTreeTestDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTaxTreeTestDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTaxTreeTestDlg creation
    return true;
}


/*!
 * CTaxTreeTestDlg destructor
 */

CTaxTreeTestDlg::~CTaxTreeTestDlg()
{
////@begin CTaxTreeTestDlg destruction
////@end CTaxTreeTestDlg destruction
}


/*!
 * Member initialisation
 */

void CTaxTreeTestDlg::Init()
{
////@begin CTaxTreeTestDlg member initialisation
    m_Input = NULL;
    m_Tree = NULL;
////@end CTaxTreeTestDlg member initialisation
}


/*!
 * Control creation for CTaxTreeTestDlg
 */

void CTaxTreeTestDlg::CreateControls()
{
////@begin CTaxTreeTestDlg content construction
    CTaxTreeTestDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    m_Input = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Input, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_BUTTON1, _("Submit Query"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, ID_BUTTON2, _("Show Related"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Tree = new CTaxTreeBrowser( itemDialog1, ID_TREECTRL, wxDefaultPosition, wxSize(-1, 200), wxTR_SINGLE );
    itemBoxSizer2->Add(m_Tree, 1, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine8 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine8, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer9 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer9, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton10 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer9->AddButton(itemButton10);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer9->AddButton(itemButton11);

    itemStdDialogButtonSizer9->Realize();

////@end CTaxTreeTestDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CTaxTreeTestDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTaxTreeTestDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTaxTreeTestDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTaxTreeTestDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTaxTreeTestDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTaxTreeTestDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTaxTreeTestDlg icon retrieval
}



/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CTaxTreeTestDlg::OnSubmitClick( wxCommandEvent& event )
{
    string query = m_Input->GetValue().ToStdString();
    if (query.empty()) {
        return;
    }

    //
    // prepare the Entrez query
    //
    CEntrez2_eval_boolean req;
    req.SetReturn_UIDs(true);
    CEntrez2_boolean_exp& exp = req.SetQuery();

    // set the database we're querying
    exp.SetDb().Set("nucleotide");

    // set the query
    CRef<CEntrez2_boolean_element> elem(new CEntrez2_boolean_element());
    elem->SetStr(query);
    exp.SetExp().push_back(elem);

    // set some limits - if num > 0, we assume it's correct
    CRef<CEntrez2_boolean_reply> query_res;
    try {
        // now, submit our query.  this gets us the UIDs we want
        query_res = x_GetClient().AskEval_boolean(req);

        const CEntrez2_id_list& ids = query_res->GetUids();
        vector<int> uids;
        uids.reserve(ids.GetNum());

        _TRACE("query: " << query << " UIDs: " << ids.GetNum());
        CTaxTreeDS_ObjMgr::TUidVec seq_ids;
        seq_ids.reserve(uids.size());

        CEntrez2_id_list::TConstUidIterator iter = ids.GetConstUidIterator();
        for (size_t i = 0;  i < ids.GetNum();  ++i, ++iter) {
            uids.push_back(*iter);

            CRef<CSeq_id> id(new CSeq_id());
            id->SetGi(*iter);
            seq_ids.push_back(id);
        }

        if ( !m_ObjMgr ) {
            m_ObjMgr = CObjectManager::GetInstance();
            CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);
            m_Scope.Reset(new CScope(*m_ObjMgr));
            m_Scope->AddDefaults();
        }

        m_DataSource.Reset(new CTaxTreeDS_ObjMgr(*m_Scope, seq_ids));

        // m_DataSource.Reset(new CTaxTreeDS_Entrez2(uids));

        m_Tree->SetDataSource(*m_DataSource);   
        m_Tree->ExpandAll();
        // m_Tree->SetGis(uids);
    }
    catch (CException& e) {
        LOG_POST(Info << "exception: " << e.what());
        LOG_POST(Info << "query failed; reconnecting...");
    }

//    event.Skip();
}


void CTaxTreeTestDlg::OnRelatedClick( wxCommandEvent& event )
{   
    CTaxTreeDataSource::TUidVec gis;
    m_Tree->GetSelectedUids(gis);
    if (gis.size() != 1) {
        NcbiMessageBox("Please select one and only one sequence");
        return;
    }

    //CStatusBarGuard LOCK(*m_StatusBar, "Submitting query...");

    int gi = gis.front()->GetGi();
    vector<int> gis_n;
    x_GetClient().GetNeighbors(gi, "nucleotide", "nucleotide_nucleotide",
                               gis_n);

    NcbiMessageBox("Found " +
                            NStr::NumericToString(gis_n.size()) +
                            " related sequences");                                                      

 }

CEntrez2Client& CTaxTreeTestDlg::x_GetClient()
{
    if ( !m_Client ) {
        m_Client.Reset(new CEntrez2Client());
    }
    return *m_Client;
}
END_NCBI_SCOPE
