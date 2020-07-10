/*  $Id: misc_field_panel.cpp 39649 2017-10-24 15:22:12Z asztalos $
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
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/misc_field_panel.hpp>
//#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/widgets/edit/misc_fieldtype.hpp>

BEGIN_NCBI_SCOPE

CMiscFieldPanel::CMiscFieldPanel(wxWindow* parent)
    : CSingleChoicePanel(parent, GetStrings())
{
}


string CMiscFieldPanel::GetFieldName(const bool subfield)
{
    string field = "";
    int val = m_FieldChoice->GetSelection();
    if (val > -1) {
        field = ToStdString(m_FieldChoice->GetString(val));
    }
    return field;
}


bool CMiscFieldPanel::SetFieldName(const string& field)
{
    bool rval = false;
    for (size_t i = 0; i < m_FieldChoice->GetStrings().size(); i++) {
        string misc_field = ToStdString(m_FieldChoice->GetString(i));
        if (QualifierNamesAreEquivalent(misc_field, field)) {
            m_FieldChoice->SetSelection(i);
            rval = true;
            break;
        }
    }
    x_UpdateParent();
    return rval;
}


vector<string> CMiscFieldPanel::GetStrings()
{
    return CMiscFieldType::GetStrings();
    
}

END_NCBI_SCOPE

