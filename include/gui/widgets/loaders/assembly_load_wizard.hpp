#ifndef GUI_WIDGETS___ASSEMBLY_LOAD_WIZARD__HPP
#define GUI_WIDGETS___ASSEMBLY_LOAD_WIZARD__HPP

/*  $Id: assembly_load_wizard.hpp 29421 2013-12-11 19:06:42Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/loaders/open_objects_panel_client.hpp>
#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/widgets/wx/wizard_page.hpp>

BEGIN_NCBI_SCOPE

class CAssemblyListPanel;

class  NCBI_GUIWIDGETS_LOADERS_EXPORT CAssemblyLoadWizard :
    public CObject,
	public IOpenObjectsPanelClient,
	public IToolWizard,
    public IRegSettings
{
public:
    CAssemblyLoadWizard();

    /// @name IOpenObjectsPanelClient interface implementation
    /// @{
	virtual string        GetLabel() const;
    virtual IExecuteUnit* GetExecuteUnit();
    /// @}

    /// @name IToolWizard interface implementation
    /// @{
    virtual void	      SetParentWindow(wxWindow* parent);
    virtual IWizardPage*  GetFirstPage();
    virtual IWizardPage*  GetOptionsPage();
    virtual void          SetPrevPage(IWizardPage* prevPage);
    virtual void          SetNextPage(IWizardPage* nexPage);
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    class CAssemblyListPage : public IWizardPage
    {
    public:
        CAssemblyListPage(CAssemblyLoadWizard& manager) : m_Manager(manager), m_PrevPage(), m_NextPage() {}
		virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);

		virtual IWizardPage* GetNextPage() { return m_NextPage; }
		virtual IWizardPage* GetPrevPage() { return m_PrevPage; }
		virtual IWizardPage* GetOptionsPage() { return 0; }
		virtual void SetNextPage(IWizardPage* page) { m_NextPage = page; }
		virtual void SetPrevPage(IWizardPage* page) { m_PrevPage = page; }
		virtual void SetOptionsPage(IWizardPage*) {}

    private:
        CAssemblyLoadWizard& m_Manager;
        IWizardPage*   m_PrevPage;
        IWizardPage*   m_NextPage;
    };

    CAssemblyListPanel*         x_GetAssemblyListPanel();

protected:
    wxWindow*			        m_ParentWindow;

    string                      m_RegPath;

    CAssemblyListPanel*         m_AssemblyListPanel;
    CAssemblyListPage           m_AssemblyListPage;
};


END_NCBI_SCOPE


#endif // GUI_WIDGETS___ASSEMBLY_LOAD_WIZARD__HPP
