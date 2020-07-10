/*  $Id: cds_product_panel.cpp 25718 2012-04-26 13:31:53Z bollin $
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

#include "cds_product_panel.hpp"

#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Cdregion.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <serial/serialbase.hpp>
#include <serial/typeinfo.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CCDSProductPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCDSProductPanel, wxPanel )


/*!
 * CCDSProductPanel event table definition
 */

BEGIN_EVENT_TABLE( CCDSProductPanel, wxPanel )

////@begin CCDSProductPanel event table entries
////@end CCDSProductPanel event table entries

END_EVENT_TABLE()


/*!
 * CCDSProductPanel constructors
 */

CCDSProductPanel::CCDSProductPanel()
{
    Init();
}

CCDSProductPanel::CCDSProductPanel( wxWindow* parent, CSerialObject& object, CScope&,
                                    wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(0)
{
    m_Object = &object;
    _ASSERT(m_Object->GetThisTypeInfo()->GetName() == "Cdregion");
    Init();
    Create(parent, id, pos, size, style);
}

/*!
 * CCDSProductPanel creator
 */

bool CCDSProductPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCDSProductPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCDSProductPanel creation
    return true;
}


/*!
 * CCDSProductPanel destructor
 */

CCDSProductPanel::~CCDSProductPanel()
{
////@begin CCDSProductPanel destruction
////@end CCDSProductPanel destruction
}


/*!
 * Member initialisation
 */

void CCDSProductPanel::Init()
{
////@begin CCDSProductPanel member initialisation
////@end CCDSProductPanel member initialisation
}


/*!
 * Control creation for CCDSProductPanel
 */

void CCDSProductPanel::CreateControls()
{
////@begin CCDSProductPanel content construction
    CCDSProductPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

////@end CCDSProductPanel content construction

    CCdregion& cds = dynamic_cast<CCdregion&>(*m_Object);

    string codeName;
    int codeId = 1;

    if (cds.IsSetCode()) {
        const CCdregion::TCode& code = cds.GetCode();
        codeId = code.GetId();
        codeName = code.GetName();
    }

    const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
    const CGenetic_code_table::Tdata& codes = code_table.Get();

    int idIndex = -1, idByName = -1;
    ITERATE (CGenetic_code_table::Tdata, it, codes) {
        string str = (*it)->GetName();
        int id = (*it)->GetId();

        string::size_type pos = str.find(';');
        string tmp = (pos != string::npos) ? str.substr(0, pos) + ", etc." : str;
        int index = m_GeneticCode->Append(ToWxString(tmp), (void*)id);

        if (codeId == id)
            idIndex = index;

        if (codeName == str)
            idByName = index;
    }

    if (idIndex < 0)
        idIndex = idByName;
    if (idIndex < 0)
        idIndex = 0;

    m_GeneticCode->SetSelection(idIndex);

    if (cds.IsSetConflict() && cds.GetConflict()) {
        m_ConflictCtrl->SetValue (true);
    }
}


bool CCDSProductPanel::TransferDataFromWindow()
{
    bool result = wxPanel::TransferDataFromWindow();

    int index = m_GeneticCode->GetSelection();
    int id = 1;
    if (index != wxNOT_FOUND)
        id = (long)m_GeneticCode->GetClientData(index);

    CRef< CGenetic_code::C_E > ce(new CGenetic_code::C_E);
    ce->SetId(id);
    CRef<CGenetic_code> code(new CGenetic_code());
    code->Set().push_back(ce);

    CCdregion& cds = dynamic_cast<CCdregion&>(*m_Object);
    cds.SetCode(*code);

    if (m_ConflictCtrl->GetValue ()) {
        cds.SetConflict(true);
    } else {
        cds.ResetConflict();
    }

    return result;
}


void CCDSProductPanel::SetCdregionFields(CCdregion& edit_cds)
{
    TransferDataFromWindow();
    CCdregion& cds = dynamic_cast<CCdregion&>(*m_Object);
    if (cds.IsSetCode()) {
        CRef<CGenetic_code> new_code(new CGenetic_code());
        new_code->Assign(cds.GetCode());
        edit_cds.SetCode(*new_code);
    } else {
        edit_cds.ResetCode();
    }
    if (cds.IsSetFrame()) {
        edit_cds.SetFrame(cds.GetFrame());
    } else {
        edit_cds.ResetFrame();
    }

    if (cds.IsSetConflict() && cds.GetConflict()) {
        edit_cds.SetConflict(true);
    } else {
        edit_cds.ResetConflict();
    }

}


/*!
 * Should we show tooltips?
 */

bool CCDSProductPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCDSProductPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCDSProductPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCDSProductPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCDSProductPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCDSProductPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCDSProductPanel icon retrieval
}

END_NCBI_SCOPE
