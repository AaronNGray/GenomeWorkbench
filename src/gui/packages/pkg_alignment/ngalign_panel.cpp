/*  $Id: ngalign_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>


#include "ngalign_panel.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>

#include "wx/bitmap.h"
#include "wx/icon.h"

#include <gui/widgets/object_list/object_list_widget.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CNGAlignPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CNGAlignPanel, CAlgoToolManagerParamsPanel )

////@begin CNGAlignPanel event table entries
////@end CNGAlignPanel event table entries

END_EVENT_TABLE()

CNGAlignPanel::CNGAlignPanel() 
{
    Init();
}

CNGAlignPanel::CNGAlignPanel( wxWindow* parent )
{
    Init();
    Create(parent);
}

bool CNGAlignPanel::Create( wxWindow* parent, wxWindowID id )
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    CAlgoToolManagerParamsPanel::Create( parent, id, wxDefaultPosition, wxSize(0, 0), SYMBOL_CNGALIGNPANEL_STYLE );

    Hide();

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}

CNGAlignPanel::~CNGAlignPanel()
{
////@begin CNGAlignPanel destruction
////@end CNGAlignPanel destruction
}

void CNGAlignPanel::Init()
{
////@begin CNGAlignPanel member initialisation
    m_QueryListCtrl = NULL;
    m_SubjectListCtrl = NULL;
////@end CNGAlignPanel member initialisation
}

void CNGAlignPanel::CreateControls()
{    
////@begin CNGAlignPanel content construction
    CNGAlignPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Query Sequences"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    m_QueryListCtrl = new CObjectListWidget( itemStaticBoxSizer3->GetStaticBox(), ID_QUERY_LIST, wxDefaultPosition, itemStaticBoxSizer3->GetStaticBox()->ConvertDialogToPixels(wxSize(200, 75)), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_QueryListCtrl, 1, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Subject Sequence"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer5, 1, wxGROW|wxALL, 5);

    m_SubjectListCtrl = new CObjectListWidget( itemStaticBoxSizer5->GetStaticBox(), ID_SUBJ_LIST, wxDefaultPosition, itemStaticBoxSizer5->GetStaticBox()->ConvertDialogToPixels(wxSize(200, 75)), wxLC_REPORT|wxLC_SINGLE_SEL );
    itemStaticBoxSizer5->Add(m_SubjectListCtrl, 1, wxGROW|wxALL, 5);

////@end CNGAlignPanel content construction
}

bool CNGAlignPanel::ShowToolTips()
{
    return true;
}
wxBitmap CNGAlignPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNGAlignPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNGAlignPanel bitmap retrieval
}
wxIcon CNGAlignPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNGAlignPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNGAlignPanel icon retrieval
}

static CObjectList* s_CreateObjList(TConstScopedObjects& objects)
{
    CRef<CObjectList> objList(new CObjectList());
    objList->Init(objects);
    objList->AddColumn(CObjectList::eInteger, "Length");

    int row = 0;
    for (auto& i : objects) {
        TSeqPos length = 0;
        const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(i.object.GetPointer());
        if (seqLoc) {
            try {
                if (seqLoc->IsWhole()) {
                    CBioseq_Handle handle = i.scope->GetBioseqHandle(*seqLoc);
                    if (handle)
                        length = (int)handle.GetBioseqLength();
                }
                else {
                    length = seqLoc->GetTotalRange().GetLength();
                }
            } NCBI_CATCH("CNGAlignPanel get Seq-loc length.");
        }

        objList->SetInteger(0, row, length);
        ++row;
    }

    return objList.Release();
}

void CNGAlignPanel::SetObjects(TConstScopedObjects* objects)
{
    if (objects) {
        CRef<CObjectList> objList(s_CreateObjList(*objects));
        m_QueryListCtrl->Init(*objList, new CObjectListTableModel());

        objList.Reset(s_CreateObjList(*objects));
        m_SubjectListCtrl->Init(*objList, new CObjectListTableModel());

        if (objects->size() > 0) {
            m_QueryListCtrl->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            m_SubjectListCtrl->SetItemState((objects->size() > 1 ? 1 : 0),
                                     wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
        LoadSettings();
    }
}

bool CNGAlignPanel::TransferDataFromWindow()
{
    if (!CAlgoToolManagerParamsPanel::TransferDataFromWindow())
        return false;

    TConstScopedObjects que_sel, sub_sel;

    m_QueryListCtrl->GetSelection(que_sel);
    if( que_sel.empty() ){
        wxMessageBox(
            wxT("Please select Query sequences."), wxT("Error"),
            wxOK | wxICON_ERROR, this
        );
        m_QueryListCtrl->SetFocus();
        return false;
    }

    m_SubjectListCtrl->GetSelection(sub_sel);
    if( sub_sel.empty() ){
        wxMessageBox(
            wxT("Please select Subject sequence."), wxT("Error"),
            wxOK | wxICON_ERROR, this
        );
        m_SubjectListCtrl->SetFocus();
        return false;
    }

    CRef<CScope> scope = sub_sel[0].scope;

    ITERATE( TConstScopedObjects, oit, que_sel ){
        const CRef<CScope> os = oit->scope;
        if( os != scope ){
            wxMessageBox
                (wxT("This tool gives unpredictable results with sequences\nfrom different projects.\n\nPlease move all sequences to the same project\nand try again."), wxT("Error"),
                wxOK | wxICON_ERROR, this
            );
            return false;
        }
    }

    SetQueries() = que_sel;
    SetSubject() = sub_sel[0];

    return true;
}

static const char *kTable1Tag = "Table1", *kTable2Tag = "Table2";

void CNGAlignPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable1Tag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_QueryListCtrl->SaveTableSettings(table_view, false);

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable2Tag);
        table_view = gui_reg.GetWriteView(reg_path);
        m_SubjectListCtrl->SaveTableSettings(table_view, false);
    }
}

void CNGAlignPanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable1Tag);
        table_view = gui_reg.GetReadView(reg_path);
        m_QueryListCtrl->LoadTableSettings(table_view);

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTable2Tag);
        table_view = gui_reg.GetReadView(reg_path);
        m_SubjectListCtrl->LoadTableSettings(table_view);
    }
}

void CNGAlignPanel::RestoreDefaults()
{
}

END_NCBI_SCOPE
