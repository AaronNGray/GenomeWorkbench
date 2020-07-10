/*  $Id: dialog.cpp 39737 2017-10-31 17:03:07Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/dialog.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/wnd_layout_registry.hpp>

BEGIN_NCBI_SCOPE

CDialog::CDialog()
{
    wxAcceleratorEntry entries[6];

    entries[0].Set(wxACCEL_CMD, (int) 'C', wxID_COPY);
    entries[1].Set(wxACCEL_CMD, (int) 'X', wxID_CUT);
    entries[2].Set(wxACCEL_CMD, (int) 'V', wxID_PASTE);
    entries[3].Set(wxACCEL_CMD, (int) 'A', wxID_SELECTALL);
    entries[4].Set(wxACCEL_CMD,       384, wxID_COPY);
    entries[5].Set(wxACCEL_SHIFT,     384, wxID_PASTE);

    wxAcceleratorTable accel(6, entries);
    SetAcceleratorTable(accel);
}


void CDialog::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kWinRectTag = "WindowRect";

void CDialog::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        SaveWindowRectToRegistry(GetRect(),
            CWndLayoutReg::GetInstance().GetWriteView(m_RegPath + "." + kWinRectTag));

        CRegistryWriteView view = CGuiRegistry::GetInstance().GetWriteView(m_RegPath);
        x_SaveSettings(view);
    }
}


void CDialog::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        wxRect rc = GetRect();
        LoadWindowRectFromRegistry(rc,
            CWndLayoutReg::GetInstance().GetReadView(m_RegPath + "." + kWinRectTag));
        CorrectWindowRect(this, rc);
        long style = GetWindowStyleFlag();
        if (style&wxRESIZE_BORDER)
            SetSize(rc);
        else
            Move(rc.GetLeftTop());

        CRegistryReadView view = CGuiRegistry::GetInstance().GetReadView(m_RegPath);
        x_LoadSettings(view);
    }
}


void CDialog::InitDialog()
{
    LoadSettings();

    wxDialog::InitDialog();
}


void CDialog::EndModal(int retCode)
{
    SaveSettings();

    wxDialog::EndModal(retCode);
}


END_NCBI_SCOPE
