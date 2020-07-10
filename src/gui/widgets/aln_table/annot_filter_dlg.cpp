/*  $Id: annot_filter_dlg.cpp 14892 2007-08-29 14:14:42Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include "annot_filter_dlg.hpp"

#include <gui/widgets/fl/input.hpp>
#include <gui/widgets/fl/button.hpp>
#include <gui/widgets/fl/return_button.hpp>
#include <gui/widgets/fl/checkbutton.hpp>
#include <gui/widgets/fl/dialog_back.hpp>
#include <gui/widgets/fl/box.hpp>

#include <gui/widgets/seq/feature_types_dlg.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#include "annot_filter_dlg_.cpp"

CAnnotCompareFilterDlg::CAnnotCompareFilterDlg(SAnnotCompareFilter& params)
    : m_Params(params)
{
    m_Window.reset(x_CreateWindow());
    CenterOnActive();
}


void CAnnotCompareFilterDlg::LoadSettings()
{
}

void CAnnotCompareFilterDlg::SaveSettings()
{
}


void CAnnotCompareFilterDlg::x_UpdateData(EDDXDirection dir)
{
    DDX_CheckButton(m_ShowIdenticalLoc,  m_Params.show_identical_loc,  dir);
    DDX_CheckButton(m_ShowIdenticalSeq,  m_Params.show_identical_seq,  dir);
    DDX_CheckButton(m_ShowIdenticalProd, m_Params.show_identical_prod, dir);
    DDX_CheckButton(m_ShowNotFound,      m_Params.show_not_found,      dir);
    DDX_CheckButton(m_ShowMissingExons,  m_Params.show_missing_exons,  dir);
    DDX_CheckButton(m_Show5PrimeExt,     m_Params.show_5prime_ext,     dir);
    DDX_CheckButton(m_Show3PrimeExt,     m_Params.show_3prime_ext,     dir);
    DDX_CheckButton(m_ShowComplex,       m_Params.show_complex,        dir);
}


void CAnnotCompareFilterDlg::x_OnChooseFeatTypes()
{
    set<CFeatListItem> feat_types;
    CFeatureTypesDlg dlg(feat_types, "Select Features to Show");
    dlg.CenterOnActive();

    if (dlg.ShowModal() == eOK)  {
    }
}


END_NCBI_SCOPE
