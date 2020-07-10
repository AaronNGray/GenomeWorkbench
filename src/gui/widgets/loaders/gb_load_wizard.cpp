/*  $Id: gb_load_wizard.cpp 39572 2017-10-12 15:10:36Z katargir $
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

#include <gui/widgets/loaders/gb_load_wizard.hpp>
#include <gui/widgets/loaders/gb_object_loader.hpp>
#include <gui/widgets/loaders/assembly_object_loader.hpp>
#include <gui/widgets/loaders/chain_loader.hpp>
#include <gui/widgets/loaders/gb_load_option_panel.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wizard_dlg.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CGBLoadWizard
CGBLoadWizard::CGBLoadWizard()
:   m_ParentWindow(),
    m_IdsPanel(NULL),
	m_IdsPage(*this)
{
}

wxPanel* CGBLoadWizard::CIdsPage::GetPanel()
{
	return m_Manager.x_GetIdsPanel();
}

bool CGBLoadWizard::CIdsPage::CanLeavePage(bool forward)
{
    return forward ? m_Manager.x_GetIdsPanel()->IsInputValid() : true;
}

//
// IOpenObjectsPanelClient implementation
//

string CGBLoadWizard::GetLabel() const
{
	return "Data from GenBank";
}

IExecuteUnit* CGBLoadWizard::GetExecuteUnit()
{
    if (!m_IdsPanel->GetSeqIds().empty() && !m_IdsPanel->GetGenomicAccessions().empty()) {
        CRef<CChainLoader> chain{ new CChainLoader() };
        chain->Add(new CGBObjectLoader(m_IdsPanel->GetSeqIds(), m_IdsPanel->GetNAs()));
        chain->Add(new CAssemblyObjectLoader(m_IdsPanel->GetGenomicAccessions()));
        return chain;
    }
    else if (!m_IdsPanel->GetSeqIds().empty())
        return new CGBObjectLoader(m_IdsPanel->GetSeqIds(), m_IdsPanel->GetNAs());
    else
        return new CAssemblyObjectLoader(m_IdsPanel->GetGenomicAccessions());
}

//
// IToolWizard implementation
//

void CGBLoadWizard::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

IWizardPage* CGBLoadWizard::GetFirstPage()
{
    x_GetIdsPanel();
    return &m_IdsPage;
}

IWizardPage* CGBLoadWizard::GetOptionsPage()
{ 
	return 0;
}

void CGBLoadWizard::SetPrevPage(IWizardPage* page)
{
    m_IdsPage.SetPrevPage(page);
}

void CGBLoadWizard::SetNextPage(IWizardPage* page)
{
    m_IdsPage.SetNextPage(page);
}

CGenBankLoadOptionPanel* CGBLoadWizard::x_GetIdsPanel()
{
    if (!m_IdsPanel) {
        m_IdsPanel = new CGenBankLoadOptionPanel(m_ParentWindow);
        LoadSettings();
    }
    return m_IdsPanel;
}

void CGBLoadWizard::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

void CGBLoadWizard::SaveSettings() const
{
    if (!m_RegPath.empty() && m_IdsPanel)
        m_IdsPanel->SaveSettings(m_RegPath);
}

void CGBLoadWizard::LoadSettings()
{
    if (!m_RegPath.empty() && m_IdsPanel)
        m_IdsPanel->LoadSettings(m_RegPath);
}


END_NCBI_SCOPE
