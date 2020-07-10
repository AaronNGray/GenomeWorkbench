/*  $Id: open_data_source_object_dlg.cpp 39737 2017-10-31 17:03:07Z katargir $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>

#include "open_data_source_object_dlg.hpp"
#include <gui/utils/execute_unit.hpp>
#include <gui/utils/object_loader.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/checkbox.h>
#include <wx/valgen.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( COpenDataSourceObjectDlg, CWizardDlg )

BEGIN_EVENT_TABLE( COpenDataSourceObjectDlg, CWizardDlg )

////@begin CWizardDlg event table entries
////@end CWizardDlg event table entries

END_EVENT_TABLE()

COpenDataSourceObjectDlg::COpenDataSourceObjectDlg()
{
    Init();
}

COpenDataSourceObjectDlg::~COpenDataSourceObjectDlg()
{
}


COpenDataSourceObjectDlg::COpenDataSourceObjectDlg( wxWindow* parent, CIRef<IUIToolManager> manager )
{
    SetManager(manager);
    Init();
    Create(parent);
}

bool COpenDataSourceObjectDlg::Create( wxWindow* parent )
{
    CWizardDlg::Create(parent, SYMBOL_CWIZARDDLG_IDNAME, wxT("Open Object from Clipboard in GBench"));

    SetCurrentPage(m_Manager->GetFirstPage());
    return true;
}

void COpenDataSourceObjectDlg::Init()
{
}

void COpenDataSourceObjectDlg::CreatePages()
{
}

void COpenDataSourceObjectDlg::SetManager(CIRef<IUIToolManager> manager)
{
    m_Manager = manager;
    m_Manager->SetParentWindow(this);
}

IObjectLoader* COpenDataSourceObjectDlg::GetObjectLoader()
{
    IObjectLoader* object_loader = 0;
    if (!m_Manager.IsNull()) {
        object_loader = dynamic_cast<IObjectLoader*>(m_Manager->GetExecuteUnit());
    }
    return object_loader;
}

void COpenDataSourceObjectDlg::x_LoadSettings(const CRegistryReadView&)
{
}

void COpenDataSourceObjectDlg::x_SaveSettings(CRegistryWriteView) const
{
}

END_NCBI_SCOPE