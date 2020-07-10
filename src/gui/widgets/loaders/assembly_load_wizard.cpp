/*  $Id: assembly_load_wizard.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/assembly_list_panel.hpp>
#include <gui/widgets/loaders/assembly_load_wizard.hpp>
#include <gui/widgets/loaders/assembly_object_loader.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wizard_dlg.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAssemblyLoadWizard
CAssemblyLoadWizard::CAssemblyLoadWizard()
:   m_ParentWindow(),
    m_AssemblyListPanel(),
	m_AssemblyListPage(*this)
{
}

wxPanel* CAssemblyLoadWizard::CAssemblyListPage::GetPanel()
{
	return m_Manager.x_GetAssemblyListPanel();
}

bool CAssemblyLoadWizard::CAssemblyListPage::CanLeavePage(bool forward)
{
    if (forward) {
        CAssemblyListPanel* panel = m_Manager.x_GetAssemblyListPanel();
        if (!panel->IsInputValid())
            return false;

        panel->SaveSettings();
        return true;
    }

    return true;
}

//
// IOpenObjectsPanelClient implementation
//

string CAssemblyLoadWizard::GetLabel() const
{
	return "Genome";
}

IExecuteUnit* CAssemblyLoadWizard::GetExecuteUnit()
{
    vector<string> assemblies = m_AssemblyListPanel->GetSelectedAssemblies();
    return new CAssemblyObjectLoader(assemblies);
}

//
// IToolWizard implementation
//

void CAssemblyLoadWizard::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

IWizardPage* CAssemblyLoadWizard::GetFirstPage()
{
    x_GetAssemblyListPanel();
    return &m_AssemblyListPage;
}

IWizardPage* CAssemblyLoadWizard::GetOptionsPage()
{ 
	return 0;
}

void CAssemblyLoadWizard::SetPrevPage(IWizardPage* page)
{
    m_AssemblyListPage.SetPrevPage(page);
}

void CAssemblyLoadWizard::SetNextPage(IWizardPage* page)
{
    m_AssemblyListPage.SetNextPage(page);
}

static const string kAssmListTag = ".AssmListPanel";

CAssemblyListPanel* CAssemblyLoadWizard::x_GetAssemblyListPanel()
{
    if (!m_AssemblyListPanel) {
        m_AssemblyListPanel = new CAssemblyListPanel(m_ParentWindow);
        if ( !m_RegPath.empty() ) {
            m_AssemblyListPanel->SetRegistryPath(m_RegPath + kAssmListTag);
            m_AssemblyListPanel->LoadSettings();
        }
    }
    return m_AssemblyListPanel;
}

void CAssemblyLoadWizard::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

void CAssemblyLoadWizard::SaveSettings() const
{
    if (m_RegPath.empty())
		return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    if (m_AssemblyListPanel)
        m_AssemblyListPanel->SaveSettings();
}

void CAssemblyLoadWizard::LoadSettings()
{
	if (m_RegPath.empty())
		return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    if (m_AssemblyListPanel)
        m_AssemblyListPanel->LoadSettings();
}

END_NCBI_SCOPE
