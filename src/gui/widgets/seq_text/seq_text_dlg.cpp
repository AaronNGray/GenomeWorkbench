/*  $Id: seq_text_dlg.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <gui/widgets/seq_text/seq_text_dlg.hpp>



BEGIN_NCBI_SCOPE

#include "seq_text_dlg_.cpp"


CSeqTextDlg::CSeqTextDlg(CRef<CSeqTextConfig> cfg, CSeqTextDataSource* pDS)
{
    m_Cfg.Reset(cfg);
    m_Window.reset(x_CreateWindow());

    // set values for display coordinate choices
    m_DisplayCoordinates->add("Absolute");
    m_DisplayCoordinates->add("Relative");

    // Apply values from config to dialog
    m_FontSize->SetFontSize(m_Cfg->GetFontSize());

    if (m_Cfg->GetShowAbsolutePosition()) {
        m_DisplayCoordinates->value(0);
    } else {
        m_DisplayCoordinates->value(1);
    }

//    m_CaseFeature->SetDataSource(*pDS);
//    m_CaseFeature->SetSubType(m_Cfg->GetCaseFeatureSubtype());

    bool show_as_lower = m_Cfg->GetShowFeatAsLower();
    if (show_as_lower) {
        m_Upper->value(0);
        m_Lower->value(1);
    } else {
        m_Upper->value(1);
        m_Lower->value(0);
    }

    m_FeatureColoration->SetDisplayType(m_Cfg->GetFeatureColorationChoice());
    m_CodonDraw->SetDisplayType(m_Cfg->GetCodonDrawChoice());
}


void CSeqTextDlg::x_OnCancel()
{
    CDialog::x_OnCancel();
}


void CSeqTextDlg::x_OnOK()
{
    // Apply values to config and view
    m_Cfg->SetFontSize(m_FontSize->GetFontSize());
    if (m_DisplayCoordinates->value() == 0) {
        m_Cfg->SetShowAbsolutePosition(true);
    } else {
        m_Cfg->SetShowAbsolutePosition(false);
    }
//    int type = 0, subtype = 0;
//    m_CaseFeature->GetTypeSubType (type, subtype);
//    m_Cfg->SetCaseFeature(static_cast<objects::CSeqFeatData::ESubtype>(subtype));
    m_Cfg->SetShowFeatAsLower(m_Lower->value() != 0);
    m_Cfg->SetFeatureColorationChoice(m_FeatureColoration->GetDisplayType());
    m_Cfg->SetCodonDrawChoice(m_CodonDraw->GetDisplayType());

    CDialog::x_OnOK();
}



END_NCBI_SCOPE
