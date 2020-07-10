/*  $Id: resolve_id_dlg.cpp 39737 2017-10-31 17:03:07Z katargir $
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
////@end includes

#include "resolve_id_dlg.hpp"

#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/grid_widget/grid.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/scrolwin.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CResolveIdDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CResolveIdDlg, CDialog )


/*!
 * CResolveIdDlg event table definition
 */

BEGIN_EVENT_TABLE( CResolveIdDlg, CDialog )

////@begin CResolveIdDlg event table entries
////@end CResolveIdDlg event table entries

END_EVENT_TABLE()

class CGridTableSeqIds : public wxGridTableBase
{
public:
    CGridTableSeqIds(const CResolveIdDlg::TData& data, CResolveIdDlg::TIdMap& idMap, CScope& scope);

    virtual int GetNumberRows();
    virtual int GetNumberCols() { return 3; }

    virtual wxString GetValue( int row, int col );
    virtual void SetValue( int row, int col, const wxString& value );


    virtual wxString GetRowLabelValue( int row );
    virtual wxString GetColLabelValue( int col );

private:
    static string x_GetLabel(const CSeq_id_Handle& h);
    string x_GetDescriptionLabel(const CSeq_id_Handle& h);

    const CResolveIdDlg::TData& m_Data;
    CResolveIdDlg::TIdMap& m_IdMap;
    CScope& m_Scope;
};

CGridTableSeqIds::CGridTableSeqIds(const CResolveIdDlg::TData& data, CResolveIdDlg::TIdMap& idMap, CScope& scope)
    : m_Data(data), m_IdMap(idMap), m_Scope(scope)
{
    SetAttrProvider(new wxGridCellAttrProvider);

    wxGridCellAttr* attr = new wxGridCellAttr();
    attr->SetReadOnly();
    wxGridCellAttr* editAttr = new wxGridCellAttr();
    editAttr->SetReadOnly(false);

    wxGridCellAttr* labelAttr = new wxGridCellAttr();
    labelAttr->SetReadOnly();
    //wxFont font = labelAttr->GetFont();
    //font.SetWeight(wxFONTWEIGHT_BOLD);
    //labelAttr->SetFont(font);
    labelAttr->SetTextColour(*wxRED);

    SetColAttr(attr, 0);
    SetColAttr(editAttr, 1);
    attr->IncRef();
    SetColAttr(attr, 2);

    int row = 0;
    ITERATE (CResolveIdDlg::TData, it, m_Data) {
        if (row > 0)
            labelAttr->IncRef();
        SetAttr(labelAttr, row, 0);
        attr->IncRef();
        SetAttr(attr, row, 1);
        row += it->second.size() + 1;
    }
}

wxString CGridTableSeqIds::GetColLabelValue( int col )
{
    if (col == 0) {
        return wxT("Supplied ID");
    }
    else if (col == 1) {
        return wxT("Actual ID");
    }
    else if (col == 2) {
        return wxT("Target Description");
    }
    return wxEmptyString;
}

wxString CGridTableSeqIds::GetRowLabelValue( int row )
{
    int rowNum = 1;
    ITERATE (CResolveIdDlg::TData, it, m_Data) {
        if (row == 0) {
            return wxEmptyString;
        }
        --row;
        if (row < (int)it->second.size()) {
            return wxString::Format(wxT("%d"), rowNum + row);
        }
        rowNum += it->second.size();
        row -= it->second.size();
    }
    return wxEmptyString;
}

int CGridTableSeqIds::GetNumberRows()
{
    int numRows = 0;

    ITERATE (CResolveIdDlg::TData, it, m_Data) {
        numRows += it->second.size() + 1;
    }

    return numRows;
}

string CGridTableSeqIds::x_GetLabel(const CSeq_id_Handle& h)
{
    string label;
    try {
        CConstRef<CSeq_id> seq_id = h.GetSeqId();
        CSeq_id::ELabelType type;
        switch (seq_id->Which()) {
        case CSeq_id::e_Local:
        case CSeq_id::e_Gi:
            type = CSeq_id::eBoth;
            break;
        default:
            type = CSeq_id::eContent;
            break;
        }
        seq_id->GetLabel(&label, type, CSeq_id::fLabel_GeneralDbIsContent|CSeq_id::fLabel_Version);
    } catch (const std::exception&) {
    }
    return label;
}

string CGridTableSeqIds::x_GetDescriptionLabel(const CSeq_id_Handle& h)
{
    CConstRef<CSeq_id> seq_id = h.GetSeqId();
    string label;
    CLabel::GetLabel(*seq_id, &label, CLabel::eDescription, &m_Scope);
    return label;
}

wxString CGridTableSeqIds::GetValue( int row, int col )
{
    if (col < 0 || col > 2)
        return wxEmptyString;

    ITERATE (CResolveIdDlg::TData, it, m_Data) {
        if (row == 0) {
            return (col == 0) ? ToWxString(it->first) : wxString();
        }
        --row;
        if (row < (int)it->second.size()) {
            if (col == 0) {
                return ToWxString(x_GetLabel(it->second[row]));
            }
            else {
                CResolveIdDlg::TIdMap::iterator it2 = m_IdMap.find(it->second[row]);
                if (it2 == m_IdMap.end()) {
                    if (col == 1)
                        return ToWxString(x_GetLabel(it->second[row]));
                    else 
                        return wxEmptyString;
                }
                else {
                    CSeq_id_Handle& h = it2->second;
                    if (col == 1)
                        return ToWxString(x_GetLabel(h));
                    else 
                        return ToWxString(x_GetDescriptionLabel(h));
                }
            }
            break;
        }
        row -= it->second.size();
    }
    return wxEmptyString;
}

void CGridTableSeqIds::SetValue( int row, int col, const wxString& value )
{
    if (col != 1)
        return;

    ITERATE (CResolveIdDlg::TData, it, m_Data) {
        if (row <= 0) {
            return;
        }
        --row;
        if (row < (int)it->second.size()) {
            const CSeq_id_Handle& h = it->second[row];

            string id_str = ToStdString(value);
            CRef<CSeq_id> id(new CSeq_id);
            try {
                id->Set(id_str);
            }
            catch (CSeqIdException&) {
                id->SetLocal().SetStr(id_str);
            }

            CSeq_id_Handle handle = CSeq_id_Handle::GetHandle(*id);
            if (handle != h) {
                m_IdMap[h] = handle;
            }
            break;
        }
        row -= it->second.size();
    }
}


/*!
 * CResolveIdDlg constructors
 */

CResolveIdDlg::CResolveIdDlg()
{
    Init();
}

CResolveIdDlg::CResolveIdDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CResolveIdDlg creator
 */

bool CResolveIdDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CResolveIdDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CResolveIdDlg creation
    return true;
}


/*!
 * CResolveIdDlg destructor
 */

CResolveIdDlg::~CResolveIdDlg()
{
////@begin CResolveIdDlg destruction
////@end CResolveIdDlg destruction
}


/*!
 * Member initialisation
 */

void CResolveIdDlg::Init()
{
////@begin CResolveIdDlg member initialisation
    m_Grid = NULL;
////@end CResolveIdDlg member initialisation
}


/*!
 * Control creation for CResolveIdDlg
 */

void CResolveIdDlg::CreateControls()
{
////@begin CResolveIdDlg content construction
    CResolveIdDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCDialog1, wxID_STATIC, _("The following IDs were found in the file.\nPlease confirm that they are correct.\nEdit Actual ID if incorrect."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Grid = new wxGrid( itemCDialog1, ID_GRID, wxDefaultPosition, itemCDialog1->ConvertDialogToPixels(wxSize(200, 150)), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_Grid->SetDefaultColSize(50);
    m_Grid->SetDefaultRowSize(25);
    m_Grid->SetColLabelSize(25);
    m_Grid->SetRowLabelSize(50);
    itemBoxSizer2->Add(m_Grid, 1, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

////@end CResolveIdDlg content construction
    wxSize size = m_Grid->GetSize();
    m_Grid->SetDefaultColSize(size.x/2 - m_Grid->GetRowLabelSize());
}

void CResolveIdDlg::SetData(const TData& data, TIdMap& map, CScope& scope)
{
    m_Grid->SetTable(new CGridTableSeqIds(data, map, scope), true);
}

static const char* kCol0Width = "Col0Width";
static const char* kCol1Width = "Col1Width";
static const char* kCol2Width = "Col2Width";

void CResolveIdDlg::x_LoadSettings(const CRegistryReadView& view)
{
    int width = view.GetInt(kCol0Width, m_Grid->GetColSize(0));
    m_Grid->SetColSize(0, width);
    width = view.GetInt(kCol1Width, m_Grid->GetColSize(1));
    m_Grid->SetColSize(1, width);
    width = view.GetInt(kCol2Width, m_Grid->GetColSize(2));
    m_Grid->SetColSize(2, width);
}

void CResolveIdDlg::x_SaveSettings(CRegistryWriteView view) const
{
    view.Set(kCol0Width, m_Grid->GetColSize(0));
    view.Set(kCol1Width, m_Grid->GetColSize(1));
    view.Set(kCol2Width, m_Grid->GetColSize(2));
}


/*!
 * Should we show tooltips?
 */

bool CResolveIdDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CResolveIdDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CResolveIdDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CResolveIdDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CResolveIdDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CResolveIdDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CResolveIdDlg icon retrieval
}

END_NCBI_SCOPE
