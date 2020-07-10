#ifndef GUI_CORE___UI_FILE_LOAD_MANAGER__HPP
#define GUI_CORE___UI_FILE_LOAD_MANAGER__HPP

/*  $Id: ui_file_load_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <util/format_guess.hpp>

#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/core/ui_tool_manager.hpp>
#include <gui/core/project_selector_panel.hpp>

#include <gui/utils/mru_list.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <wx/panel.h>

#include <gui/widgets/wx/wizard_page_impl.hpp>

BEGIN_NCBI_SCOPE

class CProjectLoadOptionPanel;
class CFileLoadOptionPanel;
class COpenDlg;

///////////////////////////////////////////////////////////////////////////////
/// IFileFormatLoaderManager
///
/// IFileFormatLoaderManager extends IUIToolManager interface.
/// The interface represents a specialized Loader Manager that loads files in a
/// particular format.
class  IFileFormatLoaderManager :
    public IUIToolManager
{
public:
    // return file loader id string to be used in MRU file list instead of label.
    virtual string   GetFileLoaderId() const = 0;

    /// return file wildcard for the format (example - "*.txt;*.tab" )
    virtual wxString GetFormatWildcard() = 0;

    /// checks given filenames and returns true if the manager can handle them,
    /// otherwise - shows an error message
    virtual bool    ValidateFilenames(const vector<wxString>& filenames) = 0;

    /// initilize the manager with the given filenames
    virtual void    SetFilenames(const vector<wxString>& filenames) = 0;
    /// get actual filenames that will be opened
    virtual void    GetFilenames(vector<wxString>& filenames) const = 0;

    /// return true if the current state of the loader UI is the first valid
    /// state (first step in a wizard-like sequence)
    virtual bool    IsInitialState() = 0;

    /// return true if the given file format is supported by this manager
    virtual bool    RecognizeFormat(const wxString& filename) = 0;

    /// return true if the given file format is supported by this manager
    virtual bool    RecognizeFormat(CFormatGuess::EFormat fmt) = 0;

    /// returns true if loader can handle only one file
    virtual bool    SingleFileLoader() const { return false; }

    /// returns true if loader's format can be guessed
    virtual bool    CanGuessFormat() const { return true; }

};


///////////////////////////////////////////////////////////////////////////////
/// Extension Point ID for IFileFormatLoaderManager objects
///
/// Components that need to add support for new File Formats to the application
/// should provide Extensions for this Extension Point. The Extensions shall
/// implement IFileFormatLoaderManager interface.
#define EXT_POINT__FILE_FORMAT_LOADER_MANAGER "file_format_loader_manager"


///////////////////////////////////////////////////////////////////////////////
/// CFileLoadManager
/// CFileLoadManager - a Loader Manager implementing "File" option in "Open" dialog.
/// CFileLoadManager combines IFileFormatLoaderManager for different file formats
/// and acts as a Facade for them.
class  NCBI_GUICORE_EXPORT  CFileLoadManager :
    public CObject,
    public IUIToolManager,
	public IToolWizard,
    public IRegSettings
{
public:
    class CFileDescriptor
    {
    public:
        CFileDescriptor(const wxString& fileName,
                        const wxString& fileLoaderLabel,
                        const string& fileLoaderId)
                        : m_FileName(fileName),
                          m_FileLoaderLabel(fileLoaderLabel),
                          m_FileLoaderId(fileLoaderId) {}

        bool operator==(const CFileDescriptor& fd) 
        { 
            return ((m_FileName == fd.m_FileName) &&
                    (m_FileLoaderId == fd.m_FileLoaderId));
        }

        wxString GetFileName() const { return m_FileName; }
        wxString GetFileLoaderLabel() const { return m_FileLoaderLabel; }
        string   GetFileLoaderId() const { return m_FileLoaderId; }
    private:
        wxString m_FileName;
        wxString m_FileLoaderLabel;
        string   m_FileLoaderId;
    };

    typedef CTimeMRUList<CFileDescriptor> TMRUPathList;

    CFileLoadManager();

    void    LoadRegisteredManagers();

    void    SetDialog(COpenDlg* dlg);

    /// @name IUIToolManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool    CanDo(EAction action);
    virtual bool    IsFinalState();
    virtual bool    IsCompletedState();
    virtual bool    DoTransition(EAction action);
    virtual IAppTask*     GetTask();
    virtual IExecuteUnit* GetExecuteUnit();
    virtual IWizardPage*  GetFirstPage();
    virtual void          SetPrevPage(IWizardPage* prevPage);
    virtual IUIToolManager*  GetAlternateToolManager() { return m_AltToolManager; }
    virtual void    ResetState();
    /// @}

    /// @name IToolWizard interface implementation
    /// @{
	virtual string        GetLabel() const { return GetDescriptor().GetLabel(); }
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    virtual void    AddFileFormat(IFileFormatLoaderManager* manager);
    virtual void    SetFilenames(vector<wxString>& filenames);
    vector<wxString> GetFilenames() const { return m_Filenames; }

    virtual void    OnFormatChanged();

protected:
    enum EState {
        eInvalid = -1,
        eSelectFiles,
        eFormatDefined, // the actual state is defined by the current format manager
        eSelectProjects,
        eCompleted
    };

    typedef CIRef<IFileFormatLoaderManager>  TFormatMgrRef;

    void    x_AutoGuessFormat();

    CFileLoadOptionPanel* x_GetOptionsPanel();
    void    x_UpdateWizardPages();
    bool    x_CanLeavePage(bool forward);

    bool x_CheckFormatConflict(const vector<wxString>& filenames, TFormatMgrRef& manager);


protected:
    class CFilePage : public CWizardPage
    {
    public:
        CFilePage(CFileLoadManager& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward) { return m_Manager.x_CanLeavePage(forward); }
    private:
        CFileLoadManager& m_Manager;
    };


    CUIObject m_Descriptor;
    COpenDlg*   m_OpenDlg;
    IServiceLocator*    m_SrvLocator;
    wxWindow*   m_ParentWindow;
    EState  m_State;

    string m_RegPath;

    TFormatMgrRef m_AutodetectFormat;

    vector<TFormatMgrRef>   m_FormatManagers;
    int m_CurrFormat;
    bool m_CheckFormat;

    /// If a file type is better handled by a different tool manager, set it here
    IUIToolManager* m_AltToolManager;

    vector<wxString>  m_Filenames;

    CFileLoadOptionPanel*    m_OptionPanel;
    CProjectSelectorPanel*   m_ProjectSelPanel;

    SProjectSelectorParams   m_ProjectParams; //TODO

    TMRUPathList m_FileMRUList;

    CFilePage                m_FilePage;
    IWizardPage*             m_OpenObjectsPage;
};


END_NCBI_SCOPE


#endif  // GUI_CORE___UI_FILE_LOAD_MANAGER__HPP

