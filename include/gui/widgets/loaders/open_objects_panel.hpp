#ifndef GUI_WIDGETS___LOADERS___OPEN_OBJECTS_PANEL__HPP
#define GUI_WIDGETS___LOADERS___OPEN_OBJECTS_PANEL__HPP

/*  $Id: open_objects_panel.hpp 28939 2013-09-17 22:03:53Z katargir $
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
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/loaders/open_objects_panel_client.hpp>

#include <gui/widgets/wx/splitter.hpp>
#include <gui/widgets/wx/wizard_page_impl.hpp>

#include <gui/objutils/reg_settings.hpp>

class wxSimpleHtmlListBox;

#define SYMBOL_OPENOBJECTSPANEL_STYLE wxFULL_REPAINT_ON_RESIZE
#define SYMBOL_OPENOBJECTSPANEL_TITLE _("Open Objects Panel")
#define SYMBOL_OPENOBJECTSPANEL_IDNAME ID_SPLITTER_PANEL
#define SYMBOL_OPENOBJECTSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_OPENOBJECTSPANEL_POSITION wxDefaultPosition

BEGIN_NCBI_SCOPE

class IObjectLoader;

class NCBI_GUIWIDGETS_LOADERS_EXPORT  COpenObjectsPanel:
    public CSplitter, public IRegSettings
{
    DECLARE_DYNAMIC_CLASS( COpenObjectsPanel )
    DECLARE_EVENT_TABLE()

public:
    COpenObjectsPanel();
    COpenObjectsPanel( wxWindow* parent, wxWindowID id = SYMBOL_OPENOBJECTSPANEL_IDNAME, const wxPoint& pos = SYMBOL_OPENOBJECTSPANEL_POSITION, const wxSize& size = SYMBOL_OPENOBJECTSPANEL_SIZE, long style = SYMBOL_OPENOBJECTSPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_OPENOBJECTSPANEL_IDNAME, const wxPoint& pos = SYMBOL_OPENOBJECTSPANEL_POSITION, const wxSize& size = SYMBOL_OPENOBJECTSPANEL_SIZE, long style = SYMBOL_OPENOBJECTSPANEL_STYLE );

    ~COpenObjectsPanel();

    void Init();

    void CreateControls();

    IWizardPage* GetWizardPage() { return &m_WizardPage; }

    void OnLoaderSelected( wxCommandEvent& event );

    void    SetManagers(vector<CIRef<IOpenObjectsPanelClient> >& managers);

    IObjectLoader*  GetObjectLoader();

    /// @name IRegSettings interface
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    class CPage : public IWizardPage
    {
    public:
        CPage(COpenObjectsPanel& panel) : m_OpenObjectPanel(panel) {}
        virtual bool CanLeavePage(bool forward) { return m_OpenObjectPanel.x_CanLeavePage(forward); }
        virtual wxPanel* GetPanel() { return m_OpenObjectPanel.x_GetPanel(); }

		virtual IWizardPage* GetNextPage() { return m_OpenObjectPanel.x_GetNextPage(); }
		virtual IWizardPage* GetPrevPage() { return 0; }
		virtual IWizardPage* GetOptionsPage() { return m_OpenObjectPanel.x_GetOptionsPage(); }
		virtual void SetNextPage(IWizardPage*) {}
		virtual void SetPrevPage(IWizardPage*) {}
		virtual void SetOptionsPage(IWizardPage*) {}

    private:
        COpenObjectsPanel& m_OpenObjectPanel;
    };

    bool		 x_CanLeavePage(bool forward);
    wxPanel*     x_GetPanel();
	IWizardPage* x_GetNextPage();
	IWizardPage* x_GetOptionsPage();

    void    x_AppendOptionForManager(IOpenObjectsPanelClient& manager);
    void    x_SelectManager(int index);

    enum {
        ID_SPLITTER_PANEL = 10003,
        ID_LOADER_LIST,
        ID_OPTION_PANEL
    };

    string  m_RegPath;

    wxSimpleHtmlListBox* m_LoadersListBox;
    wxPanel*             m_OptionPanel;

    int                 m_CurrManager;
    /// Load Managers corresponding to Loading Options
    vector<CIRef<IOpenObjectsPanelClient> > m_Managers;

    CPage               m_WizardPage;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___OPEN_OBJECTS_PANEL__HPP
