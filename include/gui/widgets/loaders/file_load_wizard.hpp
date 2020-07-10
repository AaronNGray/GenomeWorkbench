#ifndef GUI_WIDGETS___FILE_LOAD_WIZARD__HPP
#define GUI_WIDGETS___FILE_LOAD_WIZARD__HPP

/*  $Id: file_load_wizard.hpp 44093 2019-10-24 20:19:01Z filippov $
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
#include <gui/widgets/loaders/file_load_panel_client.hpp>
#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/widgets/loaders/file_load_mru_list.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/widgets/wx/wizard_page.hpp>

BEGIN_NCBI_SCOPE

class CFileLoadPanel;

class  NCBI_GUIWIDGETS_LOADERS_EXPORT CFileLoadWizard :
    public CObject,
	public IOpenObjectsPanelClient,
	public IToolWizard,
    public IRegSettings
{
public:
    CFileLoadWizard();

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

	void LoadFormats(const vector<string>& format_ids);

        void SetWorkDir(const wxString& workDir);

	const vector<CIRef<IFileLoadPanelClient> >& GetFormatManagers() const { return m_FormatManagers; }
	CFileLoadMRUList& GetFileMRUList() { return m_FileMRUList; }

    void     SetFilenames(vector<wxString>& filenames);

    vector<wxString> GetFilenames() const { return m_Filenames; }

    void    OnFormatChanged(int format);

protected:
    class CFilePage : public IWizardPage
    {
    public:
        CFilePage(CFileLoadWizard& manager) : m_Manager(manager) {}
		virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward) { return m_Manager.x_CanLeavePage(); }

		virtual IWizardPage* GetNextPage() { return m_Manager.x_GetNextPage(); }
		virtual IWizardPage* GetPrevPage() { return 0; }
		virtual IWizardPage* GetOptionsPage() { return m_Manager.x_GetOptionsPage(); }
		virtual void SetNextPage(IWizardPage*) {}
		virtual void SetPrevPage(IWizardPage*) {}
		virtual void SetOptionsPage(IWizardPage*) {}

    private:
        CFileLoadWizard& m_Manager;
    };

    CFileLoadPanel*       x_GetFileSelectPanel();
    bool				  x_CanLeavePage();
	IWizardPage*	      x_GetNextPage();
	IWizardPage*	      x_GetOptionsPage();
    bool                  x_CheckFormatConflict(const vector<wxString>& filenames);

protected:
    wxWindow*					m_ParentWindow;
	IWizardPage*                m_StartPage;

    string						m_RegPath;

    vector<CIRef<IFileLoadPanelClient> > m_FormatManagers;
    int							m_CurrFormat;

    vector<wxString>			m_Filenames;
    CFileLoadPanel*		        m_FileSelectPanel;
    CFileLoadMRUList			m_FileMRUList;
    CFilePage                   m_FileSelectPage;
    wxString                    m_WorkDir;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS___FILE_LOAD_WIZARD__HPP

