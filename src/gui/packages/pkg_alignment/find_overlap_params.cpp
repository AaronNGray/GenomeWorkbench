/*  $Id: find_overlap_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/packages/pkg_alignment/find_overlap_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CFindOverlapParams::CFindOverlapParams()
{
    Init();
}
CFindOverlapParams::CFindOverlapParams(const CFindOverlapParams& data)
{
    Init();
    Copy(data);
}
CFindOverlapParams::~CFindOverlapParams()
{
}
void CFindOverlapParams::operator=(const CFindOverlapParams& data)
{
    Copy(data);
}
bool CFindOverlapParams::operator==(const CFindOverlapParams& data) const
{
////@begin CFindOverlapParams equality operator
     if (!(m_BlastParams == data.m_BlastParams)) return false;
     if (!(m_FilterQty == data.m_FilterQty)) return false;
     if (!(m_MaxSlop == data.m_MaxSlop)) return false;
////@end CFindOverlapParams equality operator
    return true;
}
void CFindOverlapParams::Copy(const CFindOverlapParams& data)
{
////@begin CFindOverlapParams copy function
    m_BlastParams = data.m_BlastParams;
    m_FilterQty = data.m_FilterQty;
    m_MaxSlop = data.m_MaxSlop;
////@end CFindOverlapParams copy function
    m_Seq1 = data.m_Seq1;
    m_Seq2 = data.m_Seq2;
}
void CFindOverlapParams::Init()
{
////@begin CFindOverlapParams member initialisation
    m_BlastParams = wxT("-word_size 32 -evalue 0.0001 -dust yes -soft_masking true -best_hit_overhang 0.1 -best_hit_score_edge 0.1");
    m_FilterQty = 4;
    m_MaxSlop = wxT("10");
////@end CFindOverlapParams member initialisation
}

static const char* kBlastParamsTag = "BlastParams";
static const char* kMaxSlopTag = "MaxSlop";
static const char* kFilterTag = "FilterQty";

/// IRegSettings
void CFindOverlapParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kBlastParamsTag, ToStdString( m_BlastParams ));
        view.Set(kMaxSlopTag, ToStdString( m_MaxSlop ));
        view.Set(kFilterTag, m_FilterQty );
    }
}

void CFindOverlapParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_BlastParams = ToWxString(view.GetString(kBlastParamsTag,
                                                  ToStdString(m_BlastParams)));
        m_MaxSlop = ToWxString(view.GetString(kMaxSlopTag,
                                              ToStdString(m_MaxSlop)));
        m_FilterQty = view.GetInt(kFilterTag, m_FilterQty);
    }
}

END_NCBI_SCOPE
