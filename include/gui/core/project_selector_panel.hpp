#ifndef GUI_CORE___PROJECT_SELECTOR_PANEL__HPP
#define GUI_CORE___PROJECT_SELECTOR_PANEL__HPP
/*  $Id: project_selector_panel.hpp 39666 2017-10-25 16:01:13Z katargir $
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
#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/objects.hpp>

////@begin includes
#include "wx/valgen.h"
#include "wx/statline.h"
////@end includes

#include <wx/panel.h>

////@begin control identifiers
#define SYMBOL_CPROJECTSELECTORPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROJECTSELECTORPANEL_TITLE _("Project Panel")
#define SYMBOL_CPROJECTSELECTORPANEL_IDNAME ID_CPROJECTSELECTORPANEL
#define SYMBOL_CPROJECTSELECTORPANEL_SIZE wxDefaultSize
#define SYMBOL_CPROJECTSELECTORPANEL_POSITION wxDefaultPosition
////@end control identifiers

class wxRadioButton;
class wxListBox;
class wxCheckBox;
class wxTextCtrl;


BEGIN_NCBI_SCOPE

class CProjectService;
class CSelectProjectOptions;

///////////////////////////////////////////////////////////////////////////////
/// SProjectSelectorParams - holds data for CProjectSelectorPanel.

struct  NCBI_GUICORE_EXPORT  SProjectSelectorParams
{
    enum EMode {
        eInvalid = -1,
        eCreateOneProject,
        eCreateSeparateProjects,
        eAddToExistingProject
    };

    bool    m_EnablePackaging;  // input parameter, set to true to enable packaging
    bool    m_EnableDecideLater; // enable "Decide Later" option
    bool    m_EnableCreateSeparate;

    EMode   m_ProjectMode;
    int     m_SelectedProjectId; // project for eAddToExistingProject mode
    bool    m_Package; /// package in a single item
    bool    m_CreateFolder;
    string  m_FolderName;

    SProjectSelectorParams();

    /// is all objects belong to the same project - selects the project
    void    SelectProjectByObjects(TConstScopedObjects& objects, CProjectService* srv);

    void    SaveSettings(CRegistryWriteView& view, const string& section) const;
    void    LoadSettings(CRegistryReadView& view, const string& section);

    void    ToLoadingOptions(CSelectProjectOptions& options);
};


///////////////////////////////////////////////////////////////////////////////
/// CProjectSelectorPanel - a panel that allows the user to specify how the
/// project items created by a plug-in should be packaged.

class NCBI_GUICORE_EXPORT  CProjectSelectorPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CProjectSelectorPanel )
    DECLARE_EVENT_TABLE()

public:
    CProjectSelectorPanel();
    CProjectSelectorPanel( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTSELECTORPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROJECTSELECTORPANEL_POSITION, const wxSize& size = SYMBOL_CPROJECTSELECTORPANEL_SIZE, long style = SYMBOL_CPROJECTSELECTORPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTSELECTORPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROJECTSELECTORPANEL_POSITION, const wxSize& size = SYMBOL_CPROJECTSELECTORPANEL_SIZE, long style = SYMBOL_CPROJECTSELECTORPANEL_STYLE );

    ~CProjectSelectorPanel();

    void Init();

    void CreateControls();

    void    SetParams(const SProjectSelectorParams& params);
    void    GetParams(SProjectSelectorParams& params) const;

    virtual bool    TransferDataToWindow();
    virtual bool    TransferDataFromWindow();

////@begin CProjectSelectorPanel event handler declarations

    void OnInitDialog( wxInitDialogEvent& event );

    void OnCreateNewBtnSelected( wxCommandEvent& event );

    void OnCreateDepBtnSelected( wxCommandEvent& event );

    void OnAddToExistBtnSelected( wxCommandEvent& event );

    void OnProjectListSelected( wxCommandEvent& event );

    void OnCreateFolderCheckClick( wxCommandEvent& event );

////@end CProjectSelectorPanel event handler declarations

////@begin CProjectSelectorPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CProjectSelectorPanel member function declarations

    static bool ShowToolTips();

////@begin CProjectSelectorPanel member variables
    wxRadioButton* m_CreateSeparateRadio;
    wxRadioButton* m_ExistingRadio;
    wxListBox* m_ProjectList;
    wxStaticLine* m_Separator;
    wxCheckBox* m_PackageCheck;
    wxCheckBox* m_FolderCheck;
    wxTextCtrl* m_FolderInput;
    enum {
        ID_CPROJECTSELECTORPANEL = 10008,
        ID_CREATE_NEW_BTN = 10015,
        ID_CREATE_DEP_BTN = 10016,
        ID_ADD_TO_EXIST_BTN = 10017,
        ID_PROJECT_LIST = 10018,
        ID_PACKAGE_CHECK = 10019,
        ID_CREATE_FOLDER_CHECK = 10020,
        ID_FOLDER_INPUT = 10021
    };
////@end CProjectSelectorPanel member variables

public:
    void    SetProjectService(CProjectService* service);
    void    SetSimpleMode();

protected:
    void    x_FillProjectsList();
    void    x_SelectProjectByParams();

protected:
    typedef int TProjectId;
    typedef vector<TProjectId>  TProjectIdVector;

protected:
    /// this member is used for saving projects IDs within the session
    ///static  TProjectId  m_LastSelectedProjectId;

    CProjectService*    m_ProjectService;

    TProjectIdVector    m_ProjectIds;

    SProjectSelectorParams  m_Params;

    bool    m_SimpleMode;

    bool    m_CreateOption;
    bool    m_SeparateOption;
    bool    m_ExistingOption;
    wxString    m_FolderName;
    int     m_SelProjectIndex;
};

END_NCBI_SCOPE


#endif
    // GUI_CORE___PROJECT_SELECTOR_PANEL__HPP
