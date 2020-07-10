/*  $Id: open_objects_dlg.cpp 39737 2017-10-31 17:03:07Z katargir $
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
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/open_objects_panel.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/checkbox.h>
#include <wx/valgen.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( COpenObjectsDlg, CWizardDlg )

BEGIN_EVENT_TABLE( COpenObjectsDlg, CWizardDlg )

////@begin CWizardDlg event table entries
////@end CWizardDlg event table entries

END_EVENT_TABLE()

COpenObjectsDlg::COpenObjectsDlg()
{
    Init();
}

COpenObjectsDlg::COpenObjectsDlg( wxWindow* parent )
{
    Init();
    Create(parent);
}

bool COpenObjectsDlg::Create( wxWindow* parent )
{
    return CWizardDlg::Create(parent, SYMBOL_CWIZARDDLG_IDNAME, wxT("Open Objects in GBench"));
}

void COpenObjectsDlg::Init()
{
    m_OpenObjectsPage = NULL;
}

COpenObjectsDlg::~COpenObjectsDlg()
{
}

void COpenObjectsDlg::CreatePages()
{
    m_OpenObjectsPage = new COpenObjectsPanel( this, ID_OPEN_OBJECTS_PANEL );
}

void COpenObjectsDlg::SetManagers(vector<CIRef<IOpenObjectsPanelClient> >& managers)
{
    if (m_OpenObjectsPage) {
        m_OpenObjectsPage->SetManagers(managers);
        SetCurrentPage(m_OpenObjectsPage->GetWizardPage());
    }
}

IObjectLoader* COpenObjectsDlg::GetObjectLoader()
{
    IObjectLoader* object_loader = 0;
    if (m_OpenObjectsPage)
        object_loader = m_OpenObjectsPage->GetObjectLoader();
    return object_loader;
}

void COpenObjectsDlg::x_LoadSettings(const CRegistryReadView&)
{
    if (m_OpenObjectsPage) {
        m_OpenObjectsPage->SetRegistryPath(m_RegPath);
        m_OpenObjectsPage->LoadSettings();
    }
}

void COpenObjectsDlg::x_SaveSettings(CRegistryWriteView) const
{
    if (m_OpenObjectsPage) {
        m_OpenObjectsPage->SetRegistryPath(m_RegPath);
        m_OpenObjectsPage->SaveSettings();
    }
}

END_NCBI_SCOPE