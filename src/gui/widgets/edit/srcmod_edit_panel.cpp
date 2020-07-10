/*  $Id: srcmod_edit_panel.cpp 42655 2019-03-28 20:17:09Z asztalos $
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
#include <gui/widgets/edit/srcmod_edit_panel.hpp>
#include <gui/widgets/edit/voucher_panel.hpp>
#include <gui/widgets/edit/srcmod_text_panel.hpp>
#include <gui/widgets/edit/srcmod_checkbox_panel.hpp>
#include <gui/widgets/edit/altitude_panel.hpp>
#include <gui/widgets/edit/latlon_panel.hpp>
#include <gui/widgets/edit/srcmod_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CSrcModEditPanel::EEditorType CSrcModEditPanel::GetEditorTypeForSubSource (CSubSource::TSubtype st)
{
    EEditorType rval = eText;
    switch (st) {
        case CSubSource::eSubtype_environmental_sample:
        case CSubSource::eSubtype_germline:
        case CSubSource::eSubtype_metagenomic:
        case CSubSource::eSubtype_rearranged:
        case CSubSource::eSubtype_transgenic:
            rval = eCheckbox;
            break;
        case CSubSource::eSubtype_altitude:
            rval = eAltitude;
            break;
        case CSubSource::eSubtype_lat_lon:
            rval = eLatLon;
            break;
        default:
            break;
    }
    return rval;
}


CSrcModEditPanel::EEditorType CSrcModEditPanel::GetEditorTypeForOrgMod (COrgMod::TSubtype st)
{
    EEditorType rval = eText;
    switch (st) {
        case COrgMod::eSubtype_bio_material:
        case COrgMod::eSubtype_culture_collection:
        case COrgMod::eSubtype_specimen_voucher:
            rval = eVoucher;
            break;
        default:
            break;
    }
    return rval;
}


void CSrcModEditPanel::x_NotifyParent()
{
    wxWindow* parent = this->GetParent();

    CSrcModPanel* mod_panel = dynamic_cast<CSrcModPanel*>(parent);

    while (parent && !mod_panel) {
        parent = parent->GetParent();
        mod_panel = dynamic_cast<CSrcModPanel*>(parent);
    }

    if (mod_panel) {
        mod_panel->OnEditorChange();
    }
}


CSrcModEditPanel*
CSrcModEditFactory::Create(wxWindow* parent, CSrcModEditPanel::EEditorType editor_type)
{
    CSrcModEditPanel* rval = NULL;
    switch (editor_type) {
        case CSrcModEditPanel::eVoucher:
            rval = new CVoucherPanel(parent);
            break;
        case CSrcModEditPanel::eText:
            rval = new CSrcModTextPanel(parent);
            break;
        case CSrcModEditPanel::eCheckbox:
            rval = new CSrcModCheckboxPanel(parent);
            break;
        case CSrcModEditPanel::eAltitude:
            rval = new CAltitudePanel(parent);
            break;
        case CSrcModEditPanel::eLatLon:
            rval = new CLatLonPanel(parent);
            break;
    }
    return rval;
}



END_NCBI_SCOPE
